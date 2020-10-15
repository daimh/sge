/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2008 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sge.h"

#include <time.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_binding_parse.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_jsv.h"
#include "sgeobj/sge_jsv_script.h"
#include "sgeobj/sge_mailrec.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_binding.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "symbols.h"
#include "msg_common.h"

/*
 * defines the timeout how long a client/qmaster would wait maximally for
 * a response from a JSV script after a command string has been send
 */
#define JSV_CMD_TIMEOUT (10) 

typedef bool (*jsv_command_f)(sge_gdi_ctx_class_t *ctx, lListElem *jsv, lList **answer_list, 
                              dstring *c, dstring *s, dstring *a);

typedef struct jsv_command_t_ jsv_command_t;

struct jsv_command_t_ {
   const char *command;
   jsv_command_f func;
};

static bool
jsv_split_commandline(const char *input, dstring *command, dstring *subcommand, dstring *args)
{
   bool ret = true;

   DENTER(TOP_LAYER, "jsv_split_commandline");
   if (input != NULL) {
      struct saved_vars_s *cntx = NULL;
      const char *token1 = sge_strtok_r(input, " ", &cntx);

      if (token1 != NULL) {
         const char *token2;

         sge_dstring_append(command, token1);
         token2 = sge_strtok_r(NULL, " ", &cntx);
         if (token2 != NULL) {
            bool first = true;    
            const char *arg = NULL;

            sge_dstring_append(subcommand, token2);
            arg = sge_strtok_r(NULL, " ", &cntx);
            while (arg != NULL) {
               if (first) {
                  first = false;
               } else {
                  sge_dstring_append(args, " ");
               }
               sge_dstring_append(args, arg);
               arg = sge_strtok_r(NULL, " ", &cntx);
            }
         }
      }
      sge_free_saved_vars(cntx);
   }
   DRETURN(ret);
}

static bool
jsv_split_token(dstring *input, dstring *token, dstring *args) 
{
   const char *i = sge_dstring_get_string(input);
   bool ret = true;

   DENTER(TOP_LAYER, "jsv_split_token");
   if (i != NULL) {
      struct saved_vars_s *cntx = NULL;
      const char *token1 = sge_strtok_r(i, " ", &cntx);

      if (token1 != NULL) {
         bool first = true;
         const char *arg = NULL;

         sge_dstring_append(token, token1);
         arg = sge_strtok_r(NULL, " ", &cntx);
         while (arg != NULL) {
            if (first) {
               first = false;
            } else {
               sge_dstring_append(args, " ");
            }
            sge_dstring_append(args, arg);
            arg = sge_strtok_r(NULL, " ", &cntx);
         }
      }
      sge_free_saved_vars(cntx);
   }
   DRETURN(ret);
}

static bool
jsv_handle_param_command(sge_gdi_ctx_class_t *ctx _UNUSED, lListElem *jsv,
                         lList **answer_list, dstring *c _UNUSED, dstring *s,
                         dstring *a)
{
   bool ret = true;
   const char *param = sge_dstring_get_string(s);
   const char *value = sge_dstring_get_string(a);

   DENTER(TOP_LAYER, "jsv_handle_param_command");
   if (param != NULL) {
      bool skip_check = false;
      lList *local_answer_list = NULL;
      lListElem *new_job = lGetRef(jsv, JSV_new_job);

      /*
       * If we get a "__JSV_TEST_RESULT" then this code is triggered as part of a
       * testsuite test. We store that we are in test mode and we store the
       * expected result which will be tested after we did the parameter
       * modification.
       */
      if (strcmp(param, "__JSV_TEST_RESULT") == 0) {
         lSetBool(jsv, JSV_test, true);
         lSetUlong(jsv, JSV_test_pos, 0);
         lSetString(jsv, JSV_result, value);
         skip_check = true;
      }

      /*
       * Reject read-only parameter
       */
      {
         int i = 0;
         const char *read_only_param[] = {
            "CLIENT", "CONTEXT", "GROUP", "JOB_ID", "USER", "VERSION",
            NULL
         };

         while (read_only_param[i] != NULL) {
            if (strcmp(param, read_only_param[i]) == 0) {
               answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                       MSG_JSV_PARSE_READ_S, param);
               ret = false;
               break;
            }
            i++;
         }
      }

      /*
       * Handle boolean parameter.
       * -b -j -notify -shell -R -r
       */
      if (ret) {
         int i = 0;
         const char *read_only_param[] = {
            "b", "j", "notify", "shell", "R", "r",
            NULL
         };
         bool is_readonly = false;

         while (read_only_param[i] != NULL) {
            if (strcmp(param, read_only_param[i]) == 0) {
               is_readonly = true;
               if (value == NULL) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_BOOL_S, param);
                  ret = false;
               } else if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
               break;
            }
            i++;
         }
         if (ret && is_readonly) {
            if (strcmp(param, "b") == 0) {
               job_set_binary(new_job, strcmp(value, "y") == 0 ? true : false);
            } else if (strcmp(param, "j") == 0) {
               lSetBool(new_job, JB_merge_stderr, strcmp(value, "y") == 0 ? true : false);
            } else if (strcmp(param, "notify") == 0) {
               lSetBool(new_job, JB_notify, strcmp(value, "y") == 0 ? true : false);
            } else if (strcmp(param, "R") == 0) {
               lSetBool(new_job, JB_reserve, strcmp(value, "y") == 0 ? true : false);
            } else if (strcmp(param, "r") == 0) {
               lSetUlong(new_job, JB_restart, strcmp(value, "y") == 0 ? 1 : 0);
            } else if (strcmp(param, "shell") == 0) {
               job_set_no_shell(new_job, strcmp(value, "y") == 0 ? true : false);
            } else {
               answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                       MSG_JSV_PARSE_VAL_SS, param, value);
               ret = false;
            }
         }
      }

      /* 
       * Handle string parameter 
       *    -A -ckpt -cwd -N -pe <pe_name> -P 
       */
      if (ret) {
         int i = 0;
         const char *string_param[] = {
            "A", "ckpt", "cwd", "N", "pe_name", "P",
            NULL
         };
         const int string_attribute[] = {
            JB_account, JB_checkpoint_name, JB_cwd, JB_job_name, JB_pe, JB_project,
            0
         };

         while (string_param[i] != NULL) {
            int attribute = string_attribute[i];

            if (strcmp(param, string_param[i]) == 0) {
               if (value == NULL) {
                  /* 
                   * - jobs without job name are rejected 
                   * - resetting ckpt name also resets ckpt attribute
                   */
                  if (strcmp(param, "N") == 0) {
                     answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, 
                                             ANSWER_QUALITY_ERROR, MSG_JSV_PARSE_NAME_S, param);
                     ret = false;
                  } else if (strcmp(param, "ckpt") == 0) {
                     lSetUlong(new_job, JB_checkpoint_attr, NO_CHECKPOINT);
                     lSetString(new_job, attribute, NULL); 
                  } else {
                     lSetString(new_job, attribute, NULL); 
                  }
               } else { 
                  lSetString(new_job, attribute, value);
               }
               break;
            }
            i++;
         }
      }

      /*
       * Handle path list parameters
       *    -o -i -e -S
       */
      if (ret) {
         int i = 0;
         const char *path_list_param[] = {
            "o", "i", "e", "S",
            NULL
         };
         const int path_list_attribute[] = {
            JB_stdout_path_list, JB_stdin_path_list, JB_stderr_path_list, JB_shell_list,
            0
         };

         while (path_list_param[i] != NULL) {
            if (strcmp(path_list_param[i], param) == 0) {
               lList *path_list = NULL;
               int attribute = path_list_attribute[i];

               if (value != NULL) {
                  int lret = cull_parse_path_list(&path_list, value);

                  if (lret) {
                     answer_list_add_sprintf(&local_answer_list, STATUS_DENIED, 
                                             ANSWER_QUALITY_ERROR, MSG_JSV_PARSE_VAL_SS, 
                                             param, value);
                     ret = false;
                  }
               }
               if (ret) {
                  lSetList(new_job, attribute, path_list); 
               }
               break;
            }
            i++;
         }
      }

      /* CMDARG<id> */
      {         
         if (ret && strncmp("CMDARG", param, 6) == 0) {
            lList *arg_list = lGetList(new_job, JB_job_args);
            lListElem *elem;
            u_long32 id = 0;
            u_long32 length;
            int i;
            const char *id_string = param + 6;
            if (!isdigit(id_string[0])) {
               if (value) {
                  u_long32 to_create = 0;
                  u_long32 to_remove = 0;

                  length = lGetNumberOfElem(arg_list);
                  ret &= ulong_parse_from_string(&id, &local_answer_list, value);
                  if (ret) {
                     if (id > length) {
                        to_create = id - length;
                        while (to_create > 0) {
                           lAddElemStr(&arg_list, ST_name, "", ST_Type);
                           to_create--;
                        }
                     } else {
                        to_remove = length - id;
                        while (to_remove > 0) {
                           lListElem *tmp = lLast(arg_list);
                           lRemoveElem(arg_list, &tmp);
                           to_remove--;
                        }
                     }
                  }
               }
            } else {
               u_long32 to_create = 0;
               length = lGetNumberOfElem(arg_list);
               ret &= ulong_parse_from_string(&id, &local_answer_list, id_string);

               if (id > length) {
                  to_create = id - length + 1;
                  while (to_create > 0) {
                     lAddElemStr(&arg_list, ST_name, "", ST_Type);
                     to_create--;
                  }
               }

               length = lGetNumberOfElem(arg_list);
               elem = lFirst(arg_list);
               for (i = 0; i <= length - 1; i++) {
                  if (i == id) {
                     lSetString(elem, ST_name, (value != NULL) ? value : "");
                     break;
                  }
                  elem = lNext(elem);
               }
            }
         } 
      }

      /* -a */
      {
         if (ret && strcmp("a", param) == 0) {
            u_long32 timeval = 0;

            if (value != NULL) {
               int lret = ulong_parse_date_time_from_string(&timeval, &local_answer_list, value);

               if (!lret) {
                  ret = false;
               }
            }
            if (ret) {
               lSetUlong(new_job, JB_execution_time, timeval);
            }
         }
      }


      /* -ac */
      {
         if (ret && strcmp("ac", param) == 0) {
            lList *context_list = NULL;

            if (value != NULL) {
               int lret = var_list_parse_from_string(&context_list, value, 0);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_context, context_list);
            }
         }
      }

      /* -ar */
      {
         if (ret && strcmp("ar", param) == 0) {
            u_long32 id = 0;

            if (value != NULL) {
               lList *ar_id_list = NULL;
               ret &= ulong_list_parse_from_string(&ar_id_list, &local_answer_list, value, ",");
               if (ret) {
                  lListElem *first = lFirst(ar_id_list);

                  if (first != NULL) {
                     id = lGetUlong(first, ULNG_value);
                  }
               }
               lFreeList(&ar_id_list);
            }
            if (ret) {
               lSetUlong(new_job, JB_ar, id);
            }
         }
      }

      /* -c <interval> */
      /* -c <occasion> */
      {
         if (ret && strcmp("c_interval", param) == 0) {
            u_long32 timeval = 0;

            if (value != NULL) {
               int lret = ulong_parse_date_time_from_string(&timeval, &local_answer_list, value);

               if (!lret) {
                  ret = false;
               }
            }
            if (ret) {
               lSetUlong(new_job, JB_checkpoint_interval, timeval);
            }
         }
         if (ret && strcmp("c_occasion", param) == 0) {
            int lret = sge_parse_checkpoint_attr(value);

            if (lret != 0) {
               lSetUlong(new_job, JB_checkpoint_interval, lret);
            } else {
               answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                       MSG_JSV_PARSE_VAL_SS, param, value);
               ret = false;
            }
         }
      }

      /* -display */
      {
         if (ret && strcmp("display", param) == 0) {
            lList *env_list = lGetList(new_job, JB_env_list);
            lListElem *display = lGetElemStr(env_list, VA_variable, "DISPLAY"); 

            if (value != NULL) {
               if (display == NULL) {
                  display = lAddSubStr(new_job, VA_variable, "DISPLAY", JB_env_list, VA_Type);
               } 
               lSetString(display, VA_value, value);
            } else {
               if (display != NULL) {
                  lRemoveElem(env_list, &display);
               }
            }
         }
      }

      /* -dl */
      {
         if (ret && strcmp("dl", param) == 0) {
            u_long32 timeval = 0;

            if (value != NULL) {
               int lret = ulong_parse_date_time_from_string(&timeval, &local_answer_list, value);

               if (!lret) {
                  ret = false;
               }
            }
            if (ret) {
               lSetUlong(new_job, JB_deadline, timeval);
            }
         }
      }

      /* -h */
      {
         if (ret && strcmp("h", param) == 0) {
            int hold_state = MINUS_H_TGT_NONE;  
            lList *n_hold = lGetList(new_job, JB_ja_u_h_ids);
            lList *u_hold = lGetList(new_job, JB_ja_n_h_ids);
            lList *id_list = (n_hold != NULL) ? n_hold : u_hold;

            if (value != NULL) {
               hold_state = sge_parse_hold_list(value, QSUB);
               if (hold_state == -1) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (hold_state == MINUS_H_TGT_NONE) {
               lSetList(new_job, JB_ja_n_h_ids, lCopyList("", id_list));
               lSetList(new_job, JB_ja_u_h_ids, NULL);
            } else {
               lSetList(new_job, JB_ja_u_h_ids, lCopyList("", id_list));
               lSetList(new_job, JB_ja_n_h_ids, NULL);
            }
         }
      }

      /* -hold_jid */
      {
         if (ret && strcmp("hold_jid", param) == 0) {
            lList *hold_list = NULL;
            lList *jref_list = NULL;
            lListElem *jid_str;

            if (value != NULL) {
               int lret = cull_parse_jid_hold_list(&hold_list, value);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            for_each(jid_str, hold_list) {
               lAddElemStr(&jref_list, JRE_job_name, lGetString(jid_str, ST_name), JRE_Type);
            }
            lSetList(new_job, JB_jid_request_list, jref_list);
            lFreeList(&hold_list);
         }
      }

      /* -hold_jid_ad */
      {
         if (ret && strcmp("hold_jid_ad", param) == 0) {
            lList *hold_list = NULL;
            lList *jref_list = NULL;
            lListElem *jid_str;

            if (value != NULL) {
               int lret = cull_parse_jid_hold_list(&hold_list, value);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            for_each(jid_str, hold_list) {
               lAddElemStr(&jref_list, JRE_job_name, lGetString(jid_str, ST_name), JRE_Type);
            }
            lSetList(new_job, JB_ja_ad_request_list, jref_list);
            lFreeList(&hold_list);
         }
      }

      /* -js */
      {
         if (ret && strcmp("js", param) == 0) {
            u_long32 shares = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &shares, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lSetUlong(new_job, JB_jobshare, shares);
            }
         }
      }

      /* -l */
      {  
         if (ret && strcmp("l_hard", param) == 0) {
            lList *resource_list = NULL;

            if (value != NULL) {
               resource_list = centry_list_parse_from_string(NULL, value, false);
               
               if (!resource_list) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_hard_resource_list, resource_list);
            }
         }
         if (ret && strcmp("l_soft", param) == 0) {
            lList *resource_list = NULL;

            if (value != NULL) {
               resource_list = centry_list_parse_from_string(NULL, value, false);
               
               if (!resource_list) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_soft_resource_list, resource_list);
            }
         }
      }

      /* -m */
      {
         if (ret && strcmp("m", param) == 0) {
            int mail_options = NO_MAIL;

            if (value != NULL) {
               mail_options = sge_parse_mail_options(&local_answer_list, value, QSUB);
               if (!mail_options) {
                  ret = false;
               }
            }
            if (ret) {
               if (mail_options & NO_MAIL) {
                  lSetUlong(new_job, JB_mail_options, 0);
               } else {
                  lSetUlong(new_job, JB_mail_options, mail_options);
               }
            }
         }
      }

      /* -masterq ; -soft -q ; -hard -q*/
      {
         if (ret && strcmp("masterq", param) == 0) {
            lList *id_list = NULL;

            if (value != NULL) {
               int lret = cull_parse_destination_identifier_list(&id_list, value);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_PARSE_WRONGDESTIDLISTFORMATXSPECTOXOPTION_SS, 
                                          value, "-masterq");
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_master_hard_queue_list, id_list);
            }
         } else if (ret && strcmp("q_hard", param) == 0) {
            lList *id_list = NULL;

            if (value != NULL) {
               int lret = cull_parse_destination_identifier_list(&id_list, value);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_PARSE_WRONGDESTIDLISTFORMATXSPECTOXOPTION_SS, 
                                          value, "-q");
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_hard_queue_list, id_list);
            }
         } else if (ret && strcmp("q_soft", param) == 0) {
            lList *id_list = NULL;

            if (value != NULL) {
               int lret = cull_parse_destination_identifier_list(&id_list, value);

               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_PARSE_WRONGDESTIDLISTFORMATXSPECTOXOPTION_SS, 
                                          value, "-q");
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_soft_queue_list, id_list);
            }
         }
      }

      /* -M */
      {
         if (ret && strcmp("M", param) == 0) {
            lList *mail_list = NULL;

            if (value != NULL) {
               int lret = mailrec_parse(&mail_list, value);
   
               if (lret) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lSetList(new_job, JB_mail_list, mail_list); 
            }
         }
      }

      /* -p */
      {
         if (ret && strcmp("p", param) == 0) {
            int priority = 0;

            if (value != NULL) {
               ret = ulong_parse_priority(&local_answer_list, &priority, value);
            }
            if (ret) {
               lSetUlong(new_job, JB_priority, BASE_PRIORITY + priority);
            }
         }
      }

      /* -tc */
      {
         if (ret && strcmp("tc", param) == 0) {
            int max_tasks = 0;

            if (value != NULL) {
               ret = ulong_parse_task_concurrency(&local_answer_list, &max_tasks, value);
            }
            if (ret) {
               lSetUlong(new_job, JB_ja_task_concurrency, max_tasks);
            }
         }
      }

      /*    
       * -binding 
       *    <type> linear_automatic:<amount>
       *    <type> linear:<amount>:<socket>,<core>
       *    <type> striding_automatic:<amount>:<step>
       *    <type> striding:<amount>:<step>:<socket>,<core>
       *    <type> explicit:<socket_core_list>
       * 
       * <type> := set | env | pe
       * <socket_core_list> := <socket>,<core>[:<socket>,<core>]
       */
      {
         lList *binding_list = lGetList(new_job, JB_binding);
         lListElem *binding_elem = lFirst(binding_list);
  
         /* 
          * initialize binding CULL structure as if there was no binding
          * specified if there is none till now
          */ 
         if (binding_elem == NULL) {
            ret &= job_init_binding_elem(new_job);
            if (!ret) {
               answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, 
                                       ANSWER_QUALITY_ERROR,
                                       "%s", MSG_JSV_MEMBINDING);
               ret = false;
            }
            binding_list = lGetList(new_job, JB_binding);
            binding_elem = lFirst(binding_list);
         }
         /* 
          * parse JSV binding parameter and overwrite previous setting
          */
         if (ret && strcmp("binding_strategy", param) == 0) {
            if (value) {
               lSetString(binding_elem, BN_strategy, value);
               if ((strcmp("explicit", value)) != 0)
                  lSetString(binding_elem, BN_parameter_explicit, NULL);
            } else {
               lSetString(binding_elem, BN_strategy, "no_job_binding");
            }
         }
         if (ret && strcmp("binding_type", param) == 0) {

            if (value) {
               binding_type_t type = binding_type_to_enum(value);

               lSetUlong(binding_elem, BN_type, type);
            } else {
               lSetUlong(binding_elem, BN_type, BINDING_TYPE_NONE);
            }
         }
         if (ret && strcmp("binding_amount", param) == 0) {
            u_long32 amount = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &amount, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lSetUlong(binding_elem, BN_parameter_n, amount);
               }
            }
         }
         if (ret && strcmp("binding_step", param) == 0) {
            u_long32 step = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &step, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lSetUlong(binding_elem, BN_parameter_striding_step_size, step);
               }
            } 
         }
         if (ret && strcmp("binding_socket", param) == 0) {
            u_long32 socket = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &socket, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lSetUlong(binding_elem, BN_parameter_socket_offset, socket);
               }
            } 
         }
         if (ret && strcmp("binding_core", param) == 0) {
            u_long32 core = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &core, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lSetUlong(binding_elem, BN_parameter_core_offset, core);
               }
            } 
         }
         /*
          * Following section handles the explicit socket/core list
          *    1) fist we check if we received socket 
          *    2) then we check if we received core 
          *    3) then we check if the exp_n value has changed
          *    4) if either the socket or core value addresses a position behind the
          *       existing list or if the list length should be increased then
          *       we increase the arrays holding socket and core values
          *    5) after that we write the new values and store them
          * 
          */
         if (ret && strncmp("binding_exp_", param, strlen("binding_exp_")) == 0) {
            bool has_new_length = false;
            bool has_new_socket = false;
            bool has_new_core = false;
            u_long32 new_length = 0;
            u_long32 new_socket = 0;
            u_long32 new_socket_id = 0;
            u_long32 new_core = 0;
            u_long32 new_core_id = 0;
            int *socket_array = NULL;
            int *core_array = NULL;
            int sockets = 0;
            int cores = 0;

            /* 1) */
            if (ret && strncmp("binding_exp_socket", param, strlen("binding_exp_socket")) == 0) {
               const char *number = param + strlen("binding_exp_socket");

               if (value != NULL) {
                  if (!parse_ulong_val(NULL, &new_socket_id, TYPE_INT, number, NULL, 0)) {
                     answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, 
                                             ANSWER_QUALITY_ERROR, MSG_JSV_PARSE_VAL_SS, 
                                             param, value);
                     ret = false;
                  } else {
                     if (!parse_ulong_val(NULL, &new_socket, TYPE_INT, value, NULL, 0)) {
                        answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                                MSG_JSV_PARSE_VAL_SS, param, value);
                        ret = false;
                     } else {
                        has_new_socket = true;
                     }
                  }
               } 
            }
            /* 2) */
            if (ret && strncmp("binding_exp_core", param, strlen("binding_exp_core")) == 0) {
               const char *number = param + strlen("binding_exp_core");

               if (value != NULL) {
                  if (!parse_ulong_val(NULL, &new_core_id, TYPE_INT, number, NULL, 0)) {
                     answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, 
                                             ANSWER_QUALITY_ERROR, MSG_JSV_PARSE_VAL_SS, 
                                             param, value);
                     ret = false;
                  } else {
                     if (!parse_ulong_val(NULL, &new_core, TYPE_INT, value, NULL, 0)) {
                        answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                                MSG_JSV_PARSE_VAL_SS, param, value);
                        ret = false;
                     } else {
                        has_new_core = true;
                     }
                  }
               } 
            }
            /* 3) */
            if (ret && strcmp("binding_exp_n", param) == 0) {

               if (value != NULL) {
                  if (!parse_ulong_val(NULL, &new_length, TYPE_INT, value, NULL, 0)) {
                     answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                             MSG_JSV_PARSE_VAL_SS, param, value);
                     ret = false;
                  } else {
                     has_new_length = true;
                  }
               } 
            }
            /* 4) */
            if (ret) {
               bool do_resize = false;
               const char *old_param_exp_value = lGetString(binding_elem, BN_parameter_explicit);

               ret &= binding_explicit_extract_sockets_cores(old_param_exp_value,
                         &socket_array, &sockets, &core_array, &cores);

               if (!ret) {
                  /* 
                   * parsing will only fail if explicit binding list contains
                   * string 'no_explicit_binding' and should now be changed to 
                   * explicit binding
                   */
                  socket_array = NULL;
                  sockets = 0;
                  core_array = NULL;
                  cores = 0;
                  ret = true;
               } 
               if (ret) {
                  if (has_new_length) {
                     do_resize = true;
                  }
                  if (has_new_socket && new_socket_id + 1 > sockets) {
                     do_resize = true;
                     new_length = new_socket_id + 1;
                  }
                  if (has_new_core && new_core_id + 1 > cores) {
                     do_resize = true;
                     new_length = new_core_id + 1;
                  }

                  if (do_resize) {
                     int i;

                     socket_array = sge_realloc(socket_array, new_length * sizeof(int), 1);
                     core_array = sge_realloc(core_array, new_length * sizeof(int), 1);
                     for (i = sockets; i < new_length; i++) {
                        socket_array[i] = 0;
                     }
                     for (i = cores; i < new_length; i++) {
                        core_array[i] = 0;
                     }
                     sockets = new_length;
                     cores = new_length;
                  }

                  /* 5) */
                  if (has_new_socket) {
                     socket_array[new_socket_id] = new_socket;
                  }
                  if (has_new_core) {
                     core_array[new_core_id] = new_core;
                  }
               }
               if (ret) {
                  dstring socket_core_string = DSTRING_INIT;

                  binding_printf_explicit_sockets_cores(&socket_core_string, 
                                                        socket_array, sockets, 
                                                        core_array, cores);
                  lSetString(binding_elem, BN_parameter_explicit, 
                             sge_dstring_get_string(&socket_core_string));
                  sge_dstring_free(&socket_core_string);
               }

               sge_free(&socket_array);
               sge_free(&core_array);
            }
         }
      }

      /*
       * -pe name n-m
       */
      {
         if (ret && strcmp("pe_name", param) == 0) {
            if (value) {
               lSetString(new_job, JB_pe, value);
            } else {
               lSetString(new_job, JB_pe, NULL);
            }
         }
         if (ret && strcmp("pe_min", param) == 0) {
            u_long32 min = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &min, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lList *range_list = lGetList(new_job, JB_pe_range);
               lListElem *range = lFirst(range_list);

               if (range == NULL) {
                  range = lAddSubUlong(new_job, RN_min, min, JB_pe_range, RN_Type);
               }
               if (range != NULL) {
                  lSetUlong(range, RN_min, min);
               }
            }
         }
         if (ret && strcmp("pe_max", param) == 0) {
            u_long32 max = 0;

            if (value != NULL) {
               if (!parse_ulong_val(NULL, &max, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               }
            }
            if (ret) {
               lList *range_list = lGetList(new_job, JB_pe_range);
               lListElem *range = lFirst(range_list);

               if (range == NULL) {
                  range = lAddSubUlong(new_job, RN_max, max, JB_pe_range, RN_Type);
               }
               if (range != NULL) {
                  lSetUlong(range, RN_max, max);
               }
            }
         }
      }

      /*
       *-t n-m:s
       */
      {
         if (ret && strcmp("t_min", param) == 0) {
            if (value != NULL) {
               u_long32 min;
               if (!parse_ulong_val(NULL, &min, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          "invalid t_min "SFQ" was passed by JSV", value);
                  ret = false;
               } else {
                  lList *range_list = lGetList(new_job, JB_ja_structure);
                  lListElem *range = lFirst(range_list);

                  if (range == NULL) {
                     range = lAddSubUlong(new_job, RN_min, min, JB_ja_structure, RN_Type);
                  }
                  if (range != NULL) {
                     lSetUlong(range, RN_min, min);
                  }
               }
            }
         }
         if (ret && strcmp("t_max", param) == 0) {
            if (value != NULL) {
               u_long32 max;
               if (!parse_ulong_val(NULL, &max, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lList *range_list = lGetList(new_job, JB_ja_structure);
                  lListElem *range = lFirst(range_list);

                  if (range == NULL) {
                     range = lAddSubUlong(new_job, RN_max, max, JB_ja_structure, RN_Type);
                  }
                  if (range != NULL) {
                     lSetUlong(range, RN_max, max);
                  }
               }
            }
         }
         if (ret && strcmp("t_step", param) == 0) {
            if (value != NULL) {
               u_long32 step;
               if (!parse_ulong_val(NULL, &step, TYPE_INT, value, NULL, 0)) {
                  answer_list_add_sprintf(&local_answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                          MSG_JSV_PARSE_VAL_SS, param, value);
                  ret = false;
               } else {
                  lList *range_list = lGetList(new_job, JB_ja_structure);
                  lListElem *range = lFirst(range_list);

                  if (range == NULL) {
                     range = lAddSubUlong(new_job, RN_step, step, JB_ja_structure, RN_Type);
                  }
                  if (range != NULL) {
                     lSetUlong(range, RN_step, step);
                  }
               }
            }
         }
      }

      /* -w */
      {
         if (ret && strcmp("w", param) == 0) {
            int level = 0;

            if (value != NULL) {
               ret = job_parse_validation_level(&level, value, QSUB, &local_answer_list);

               DPRINTF(("result of parsing is %d\n", ret));
   
            }
            if (ret) {
               lSetUlong(new_job, JB_verify_suitable_queues, level);             
            }
         }

      }

      /*
       * if we are in test mode the we have to check the expected result.
       * in test mode we will reject jobs if we did not get the expected
       * result otherwise
       * we will accept the job with the ret value set above including
       * the created error messages.
       */
      if (lGetBool(jsv, JSV_test) && !skip_check) {
         const char *result_string = lGetString(jsv, JSV_result);
         u_long32 result_pos = lGetUlong(jsv, JSV_test_pos);

         if (strlen(result_string) > result_pos) {
            char result_char = result_string[result_pos];
            bool result = (result_char == '1') ? true : false;

            if (result != ret) {
               answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                       "\"PARAM %s %s\" was unexpectedly %s\n",
                                       param, value != NULL ? value : " ",
                                       ret ? "accepted" : "rejected");
               ret = false;
            } else {
               ret = true;
            }
         }
         lSetUlong(jsv, JSV_test_pos, lGetUlong(jsv, JSV_test_pos) + 1);
      }
      answer_list_append_list(answer_list, &local_answer_list);
   }
   DRETURN(ret);
}

static bool
jsv_handle_send_command(sge_gdi_ctx_class_t *ctx _UNUSED, lListElem *jsv,
                        lList **answer_list, dstring *c _UNUSED, dstring *s,
                        dstring *a _UNUSED)
{
   bool ret = true;
   const char *subcommand = sge_dstring_get_string(s);

   DENTER(TOP_LAYER, "jsv_handle_send_command");
   if (strcmp(subcommand, "ENV") == 0) {
      lSetBool(jsv, JSV_send_env, true);
   } else {
      /*
       * Invalid subcommand. JSV seems to wait for information which
       * is not available in this version. Job will be rejected.
       */
      answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                              MSG_JSV_PARSE_OBJECT_S, sge_dstring_get_string(s));

      lSetBool(jsv, JSV_send_env, false);
      ret = false;
   }
   DRETURN(ret);
}

static bool
jsv_handle_result_command(sge_gdi_ctx_class_t *ctx _UNUSED, lListElem *jsv,
                          lList **answer_list, dstring *c _UNUSED,
                          dstring *s, dstring *a)
{
   bool ret = true;
   dstring m = DSTRING_INIT;
   dstring st = DSTRING_INIT; 
   const char *sub_command = NULL;
   const char *state = NULL;
   const char *message = NULL;

   DENTER(TOP_LAYER, "jsv_handle_result_command");
   sub_command = sge_dstring_get_string(s);
   jsv_split_token(a, &st, &m);
   state = sge_dstring_get_string(&st);
   message = sge_dstring_get_string(&m);
   if (sub_command != NULL && strcmp(sub_command, "STATE") == 0 && state != NULL) {
      if (strcmp(state, "ACCEPT") == 0) {
         DPRINTF(("Job is accepted\n"));
         lSetBool(jsv, JSV_accept, true);
         lSetBool(jsv, JSV_done, true);
      } else if (strcmp(state, "CORRECT") == 0) {
         DPRINTF(("Job is corrected\n"));
         lSetBool(jsv, JSV_accept, true);
         lSetBool(jsv, JSV_done, true);
      } else if (strcmp(state, "REJECT") == 0) {
         DPRINTF(("Job is rejected\n"));
         if (message != NULL) {
            answer_list_add_sprintf(answer_list, STATUS_DENIED,
                                    ANSWER_QUALITY_ERROR, "%s", message);
         } else {
            answer_list_add_sprintf(answer_list, STATUS_DENIED, 
                                    ANSWER_QUALITY_ERROR, "%s", MSG_JSV_REJECTED);
         }
         lSetBool(jsv, JSV_accept, false);
         lSetBool(jsv, JSV_done, true);
      } else if (strcmp(state, "REJECT_WAIT") == 0) {
         DPRINTF(("Job is rejected temporarily\n"));
         if (message != NULL) {
            answer_list_add_sprintf(answer_list, STATUS_DENIED, 
                                    ANSWER_QUALITY_ERROR, "%s", message);
         } else {
            answer_list_add_sprintf(answer_list, STATUS_DENIED, 
                                    ANSWER_QUALITY_ERROR, "%s", MSG_JSV_TMPREJECT);
         }
         lSetBool(jsv, JSV_accept, false);
         lSetBool(jsv, JSV_done, true);
      } else {
         answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                 MSG_JSV_STATE_S, sge_dstring_get_string(a));
         ret = false;
      }
   } else {
      answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                              MSG_JSV_COMMAND_S, sub_command);
      ret = false;
   }
   
   /* disable sending of env variables for the next job verification */
   lSetBool(jsv, JSV_send_env, false);

   sge_dstring_free(&m);
   sge_dstring_free(&st);
   DRETURN(ret);
}

static bool
jsv_handle_started_command(sge_gdi_ctx_class_t *ctx, lListElem *jsv, lList **answer_list,
                           dstring *c _UNUSED, dstring *s _UNUSED, dstring *a _UNUSED)
{
   const char *prefix = "PARAM";
   dstring buffer = DSTRING_INIT;
   bool ret = true;
   lListElem *old_job = lGetRef(jsv, JSV_old_job);

   DENTER(TOP_LAYER, "jsv_handle_started_command");

   /* reset variables which are only used in test cases */
   lSetBool(jsv, JSV_test, false);
   lSetString(jsv, JSV_result, "");

   /* 
    * JSV VERSION <major>.<minor> 
    * read-only   
    */
   sge_dstring_clear(&buffer);
   sge_dstring_sprintf(&buffer, "%s VERSION 1.0", prefix);
   jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

   /* 
    * JSV CONTEXT "client"|"server" 
    * read-only
    */
   sge_dstring_clear(&buffer);
   sge_dstring_sprintf(&buffer, "%s CONTEXT %s", prefix,
                 (strcmp(lGetString(jsv, JSV_context), JSV_CONTEXT_CLIENT) == 0 ? "client" : "server"));
   jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   
   /* 
    * JSV CLIENT <program_name>
    * read-only
    */
   {
      u_long32 progid = ctx->get_who(ctx);

      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s CLIENT %s", prefix, prognames[progid]);
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /* 
    * JSV USER <user_name> 
    * read-only
    */
   sge_dstring_clear(&buffer);
   sge_dstring_sprintf(&buffer, "%s USER %s", prefix, lGetString(old_job, JB_owner));
   jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

   /* 
    * JSV GROUP <group_name> 
    * read-only
    */
   sge_dstring_clear(&buffer);
   sge_dstring_sprintf(&buffer, "%s GROUP %s", prefix, lGetString(old_job, JB_group));
   jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

   /* 
    * JSV JOB_ID <jid> 
    * optional; read-only 
    */
   {
      u_long32 jid = lGetUlong(old_job, JB_job_number);

      if (jid > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s JOB_ID "sge_u32, prefix, jid);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /*
    * JSV COMMAND 
    *    -b y ... <command>      => format := <command>
    *    -b n ... <job_script>   => format := <file> 
    *    -b n                    => format := "STDIN"
    */
   {
      const char *script_name = lGetString(old_job, JB_script_file);

      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s CMDNAME %s", prefix, (script_name != NULL) ? script_name : "NONE");
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /* 
    * JSV SCRIPT_ARGS 
    */
   {
      lList *list = lGetList(old_job, JB_job_args);
      lListElem *elem;
      int i = 0;

      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s CMDARGS %d", prefix, lGetNumberOfElem(list));
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

      for_each(elem, list) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s CMDARG%d %s", prefix, i, lGetString(elem, ST_name));
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         i++;
      }
   }
   
   /* 
    * -a 
    * PARAM a <date_time> (optional; <date_time> := CCYYMMDDhhmm.SS)
    */
   {
      time_t clocks = (time_t) lGetUlong(old_job, JB_execution_time);

      if (clocks > 0) {
         struct tm time_struct;

         localtime_r(&clocks, &time_struct);
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s a %04d%02d%02d%02d%02d.%02d", prefix,
                             time_struct.tm_year + 1900, time_struct.tm_mon, 
                             time_struct.tm_mday, time_struct.tm_hour, 
                             time_struct.tm_min, time_struct.tm_sec);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -ac variable[=value],... (optional; also contains result of -dc and -sc options) 
    */
   {
      lList *context_list = lGetList(old_job, JB_context);

      if (context_list != NULL) {
         lListElem *tmp_job = lCopyElem(old_job);
         lListElem *context = NULL;
         lList* tmp = NULL;

         lXchgList(tmp_job, JB_context, &tmp);
         set_context(tmp, tmp_job);
         context_list = lGetList(tmp_job, JB_context);
         if (context_list != NULL) {
            bool first = true;

            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "PARAM ac");
            for_each(context, context_list) {
               const char *name = lGetString(context, VA_variable);
               const char *value = lGetString(context, VA_value);

               sge_dstring_sprintf_append(&buffer, (first) ? " " : ",");
               first = false;
               if (value != NULL) {
                  sge_dstring_sprintf_append(&buffer, "%s=%s", name, value);
               } else {
                  sge_dstring_sprintf_append(&buffer, "%s", name);
               }
            }
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }
         lFreeList(&tmp);
         lFreeElem(&tmp_job);
      }
   }

   /* 
    * -ar 
    * PARAM ar <ar_id> 
    * optional 
    */
   {
      u_long32 ar_id = lGetUlong(old_job, JB_ar);
   
      if (ar_id > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s ar "sge_u32, prefix, ar_id);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -A <account_string> (optional) 
    */
   {
      const char *account_string = lGetString(old_job, JB_account);

      if (account_string != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s A %s", prefix, account_string);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -b y|n 
    * PARAM b y|n (optional; only available if -b y was specified)
    */
   if (job_is_binary(old_job)) {
      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s b %c", prefix, job_is_binary(old_job) ? 'y' : 'n');
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /*
    * -c n|s|m|x   or   
    * -c <interval>
    *
    * PARAM c_occasion <occasion_string> (optional; <occasion_string> := ['n']['s']['m']['x']
    * PARAM c_interval <interval> (optional; <interval> := <2_digits>:<2_digits>:<2_digits>)
    */
   {
      u_long32 interval = lGetUlong(old_job, JB_checkpoint_interval);
      u_long32 attr = lGetUlong(old_job, JB_checkpoint_attr);

      if (interval > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s c_interval ", prefix);
         double_print_time_to_dstring((double)interval, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
      if (attr > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s c_occasion ", prefix);
         job_get_ckpt_attr(attr, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -ckpt name
    * PARAM ckpt <name> (optional);
    */
   {
      const char *ckpt = lGetString(old_job, JB_checkpoint_name);

      if (ckpt != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s ckpt %s", prefix, ckpt);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -cwd 
    *
    * Different to commandline. If -cwd was specified it will be exported to the
    * JSV by passing the complete path. To remove the path the JSV has to
    * pass an empty path.
    *
    * PARAM cwd <working_directory> (optional)
    */
   {
      const char *cwd = lGetString(old_job, JB_cwd);

      if (cwd != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s cwd %s", prefix, cwd);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -C (not handled in JSV) */
   /* -dc (handled as part of -ac parameter) */
   /* -display <display_name> (handled below where -v/-V is handled) */

   /* 
    * -dl <date_time> 
    * optional
    */
   {
      time_t clocks = (time_t) lGetUlong(old_job, JB_deadline);

      if (clocks > 0) {
         struct tm time_struct;

         localtime_r(&clocks, &time_struct);
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s dl %04d%02d%02d%02d%02d.%02d", prefix,
                             time_struct.tm_year + 1900, time_struct.tm_mon, 
                             time_struct.tm_mday, time_struct.tm_hour, 
                             time_struct.tm_min, time_struct.tm_sec);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -e <output_path>
    */
   {
      lList *shell_list = lGetList(old_job, JB_stderr_path_list);

      if (shell_list != NULL) {
         lListElem *shell;
         bool first = true;
 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s e", prefix);
         for_each(shell, shell_list) {
            const char *hostname = lGetHost(shell, PN_host);
            const char *path = lGetString(shell, PN_path);
           
             
            sge_dstring_append_char(&buffer, first ? ' ' : ',');
            if (hostname != NULL) {
               sge_dstring_append(&buffer, hostname); 
               sge_dstring_append_char(&buffer, ':');
               sge_dstring_append(&buffer, path);
            } else {
               sge_dstring_append(&buffer, path);
            }
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -hard (handled as l_hard and q_hard below) */

   /* 
    * -hold_jid wc_job_list 
    * optional
    */
   {
      lList *hold_jid_list = lGetList(old_job, JB_jid_request_list);
     
      if (hold_jid_list != NULL) {
         lListElem *hold_jid;
         bool first = true;

         sge_dstring_clear(&buffer); 
         sge_dstring_sprintf(&buffer, "%s hold_jid", prefix);
         for_each(hold_jid, hold_jid_list) {
            const char *name = lGetString(hold_jid, JRE_job_name);
            u_long32 jid = lGetUlong(hold_jid, JRE_job_number);

            sge_dstring_append_char(&buffer, first ? ' ' : ',');
            if (name != NULL) {
               sge_dstring_sprintf_append(&buffer, "%s", name);
            } else {
               sge_dstring_sprintf_append(&buffer, sge_U32CFormat, sge_u32c(jid));
            }
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -hold_jid_ad wc_job_list 
    * optional 
    */
   {
      lList *hold_jid_list = lGetList(old_job, JB_ja_ad_request_list);
    
      if (hold_jid_list != NULL) {
         lListElem *hold_jid;
         bool first = true;

         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s hold_jid_ad", prefix);
         for_each(hold_jid, hold_jid_list) {
            const char *name = lGetString(hold_jid, JRE_job_name);
            u_long32 jid = lGetUlong(hold_jid, JRE_job_number);

            sge_dstring_append_char(&buffer, first ? ' ' : ',');
            if (name != NULL) {
               sge_dstring_sprintf_append(&buffer, "%s", name);
            } else {
               sge_dstring_sprintf_append(&buffer, sge_U32CFormat, sge_u32c(jid));
            }
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -h (optional; only available if job is in user hold)
    * 
    * in difference with the qsub -h switch the setting is provided as
    *
    * PARAM h u|n
    *
    * where 'u' means "user hold"
    * and 'n' would mean "no hold"
    */
   {
      lList *hold_list = lGetList(old_job, JB_ja_u_h_ids);

      if (hold_list != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s h %c", prefix, (hold_list != NULL) ? 'u' : 'n');
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -i <input_path> (optional) 
    */
   {
      lList *shell_list = lGetList(old_job, JB_stdin_path_list);

      if (shell_list != NULL) {
         lListElem *shell;
         bool first = true;
 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s i", prefix);
         for_each(shell, shell_list) {
            const char *hostname = lGetHost(shell, PN_host);
            const char *path = lGetString(shell, PN_path);
           
             
            sge_dstring_append_char(&buffer, first ? ' ' : ',');
            if (hostname != NULL) {
               sge_dstring_append(&buffer, hostname); 
               sge_dstring_append_char(&buffer, ':');
               sge_dstring_append(&buffer, path);
            } else {
               sge_dstring_append(&buffer, path);
            }
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -inherit (not handled in JSV) */

   /* 
    * -j y | n 
    * PARAM j y | n 
    * optional; only available when -j y was specified
    */
   if (lGetBool(old_job, JB_merge_stderr) == true) {
      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s j %c", prefix, lGetBool(old_job, JB_merge_stderr) ? 'y' : 'n');
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /* 
    * -js job_share (optional) 
    */
   {
      u_long32 job_share = lGetUlong(old_job, JB_jobshare);

      if (job_share > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s js "sge_U32CFormat, prefix, sge_u32c(job_share));
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -l (optional)
    *
    * -soft -l =>
    * PARAM l_soft <centry_list> 
    *
    * -hard -l =>
    * PARAM l_hard <centry_list>
    */
   {
      lList *l_hard_list = lGetList(old_job, JB_hard_resource_list);

      if (l_hard_list != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s l_hard ", prefix);
         centry_list_append_to_dstring(l_hard_list, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }
   {
      lList *l_soft_list = lGetList(old_job, JB_soft_resource_list);

      if (l_soft_list != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s l_soft ", prefix);
         centry_list_append_to_dstring(l_soft_list, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -m [b][e][a][s] or n (optional; only provided to JSV script if != 'n'
    */
   {
      u_long32 mail_options = lGetUlong(old_job, JB_mail_options);

      if (mail_options > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s m ", prefix);
         sge_mailopt_to_dstring(mail_options, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -M <mail_addr>, ... 
    * optional
    */
   {
      lList *mail_list = lGetList(old_job, JB_mail_list);
      lListElem *mail;
      bool first = true;
      
      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s M", prefix);
      for_each(mail, mail_list) {
         const char *user = lGetString(mail, MR_user);
         const char *host = lGetHost(mail, MR_host); 

         sge_dstring_append_char(&buffer, first ? ' ' : ',');
         sge_dstring_append(&buffer, user);
         if (host != NULL) {
            sge_dstring_sprintf_append(&buffer, "@%s", host);
         }
         first = false;
      }
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /* 
    * -masterq wc_queue_list (optional) 
    */
   {
      lList *master_hard_queue_list = lGetList(old_job, JB_master_hard_queue_list);

      if (master_hard_queue_list != NULL) {
         lListElem *queue;
         bool first = true;

         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s masterq", prefix);
         for_each(queue, master_hard_queue_list) {
            const char *queue_pattern = lGetString(queue, QR_name);

            sge_dstring_append_char(&buffer, first ? ' ' : ','); 
            sge_dstring_sprintf_append(&buffer, "%s", queue_pattern);  
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -notify y|n 
    * PARAM notify y|n (optional; only available when -notify y was specified)
    */
   if (lGetBool(old_job, JB_notify) == true) {
      sge_dstring_clear(&buffer);
      sge_dstring_sprintf(&buffer, "%s notify %c", prefix, lGetBool(old_job, JB_notify) ? 'y' : 'n');
      jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
   }

   /* -now y|n (not available in JSV) */

   /* 
    * -N <job_name> 
    * optional; only available if specified during job submission
    */
   {
      const char *name = lGetString(old_job, JB_job_name);

      if (name != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s N %s", prefix, name);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -noshell y | n (not handled in JSV) */
   /* -nostdin (not handled in JSV) */

   /* 
    * -o <output_path> (optional) 
    * TODO: EB: summarize with -S -e -i 
    */
   {
      lList *shell_list = lGetList(old_job, JB_stdout_path_list);

      if (shell_list != NULL) {
         lListElem *shell;
 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s o ", prefix);
         for_each(shell, shell_list) {
            const char *hostname = lGetHost(shell, PN_host);
            const char *path = lGetString(shell, PN_path);
           
            if (hostname != NULL) {
               sge_dstring_append(&buffer, hostname); 
               sge_dstring_append_char(&buffer, ':');
               sge_dstring_append(&buffer, path);
            } else {
               sge_dstring_append(&buffer, path);
            }
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -ot override_tickets 
    * (handled in JSV) 
    */

   /* 
    * -P project_name (optional; only available if specified during submission) 
    */
   {
      const char *project = lGetString(old_job, JB_project);

      if (project != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s P %s", prefix, project);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }
   
   /* -p priority 
    * optional; only provided if specified during submission and != 0) 
    */
   {
      int priority = (int) lGetUlong(old_job, JB_priority) - 1024; 

      if (priority != 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s p %d", prefix, priority);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -tc task concurrency
    * optional; only provided if specified during submission
    */
   {
      int max_tasks = (int) lGetUlong(old_job, JB_ja_task_concurrency);

      if (max_tasks != 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s tc %d", prefix, max_tasks);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /*
    * -binding 
    *    <type> linear_automatic:<amount>
    *    <type> linear:<amount>:<socket>,<core>
    *    <type> striding_automatic:<amount>:<step>
    *    <type> striding:<amount>:<step>:<socket>,<core>
    *    <type> explicit:<socket_core_list>
    *
    * <type> := set | env | pe
    * <socket_core_list> := <socket>,<core>[:<socket>,<core>]
    */
   {
      lList *list = lGetList(old_job, JB_binding);
      lListElem *binding = ((list != NULL) ? lFirst(list) : NULL);
      const char *strategy = ((binding != NULL) ? lGetString(binding, BN_strategy) : NULL);

      if (strategy != NULL && strcmp(strategy, "no_job_binding") != 0) {
         const char *strategy_without_automatic = strategy;

         /* binding_strategy */
         if (strcmp(strategy, "linear_automatic") == 0) {
            strategy_without_automatic = "linear";
         } else if (strcmp(strategy, "striding_automatic") == 0) {
            strategy_without_automatic = "striding";
         }
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s binding_strategy %s", 
                             prefix, strategy_without_automatic);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

         /* binding_type */
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s binding_type ", prefix);
         binding_type_to_string((binding_type_t)lGetUlong(binding, BN_type), &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

         if (strcmp("linear", strategy_without_automatic) == 0 || strcmp("striding", strategy_without_automatic) == 0) {
            /* binding_amount */
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s binding_amount "sge_U32CFormat, prefix, 
                                sge_u32c(lGetUlong(binding, BN_parameter_n)));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }

         /*
          * socket and core will only be sent for linear and striding strategy
          */
         if (strcmp("linear", strategy) == 0 || strcmp("striding", strategy) == 0) {
            /* binding_socket */
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s binding_socket "sge_U32CFormat, prefix, 
                                sge_u32c(lGetUlong(binding, BN_parameter_socket_offset)));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

            /* binding_core */
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s binding_core "sge_U32CFormat, prefix, 
                                sge_u32c(lGetUlong(binding, BN_parameter_core_offset)));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }

         /*
          * Only within striding strategy stepsize parameter is allowed
          */ 
         if (strcmp("striding", strategy_without_automatic) == 0) {
            /* binding_step */
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s binding_step "sge_U32CFormat, prefix, 
                                sge_u32c(lGetUlong(binding, BN_parameter_striding_step_size)));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }

         /*
          * "explicit" strategy requires a socket/core list
          */
         if (strcmp("explicit", strategy) == 0) {
            int *socket_array = NULL;
            int *core_array = NULL;
            int socket = 0;
            int core = 0;
            int i;

            binding_explicit_extract_sockets_cores(
               lGetString(binding, BN_parameter_explicit), 
               &socket_array, &socket, &core_array, &core);    

            /* binding_strategy */
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s binding_exp_n "sge_U32CFormat, prefix, sge_u32c(socket));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));

            for (i = 0; i < socket; i++) {
               sge_dstring_clear(&buffer);
               sge_dstring_sprintf(&buffer, "%s binding_exp_socket%d %d", prefix, i, socket_array[i]);
               jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
               sge_dstring_clear(&buffer);
               sge_dstring_sprintf(&buffer, "%s binding_exp_core%d %d", prefix, i, core_array[i]);
               jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
            }

            sge_free(&socket_array);
            sge_free(&core_array);
         }
      }
   }

   /* 
    * -pe name n[-[m]] (optional)
    *
    * PARAM pe_name <pe_name>
    * PARAM pe_min <min_number>
    * PARAM pe_max <max_number>
    *
    *                min_number  max_number
    *    -pe pe 4    4           4
    *    -pe pe 4-8  4           8
    *    -pe pe 4-   4           9999999
    *    -pe pe -8   1           8
    */
   {
      const char *pe_name = lGetString(old_job, JB_pe);
      lList *range_list = lGetList(old_job, JB_pe_range);
      lListElem *range = lFirst(range_list);

      if (pe_name != NULL) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s pe_name %s", prefix, pe_name);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
      if (range != NULL) {
         u_long32 min = lGetUlong(range, RN_min);
         u_long32 max = lGetUlong(range, RN_max);

         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s pe_min "sge_U32CFormat, prefix, sge_u32c(min));
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s pe_max "sge_U32CFormat, prefix, sge_u32c(max));
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -pty y|n (not available in JSV) */

   /* 
    * -q wc_queue_list (optional; see man page sge_types(1) for wc_queue_list specification)
    *
    * -soft -q =>
    * PARAM q_soft <wc_queue_list> (see man page sge_types(1) for wc_queue_list specification)
    *
    * -hard -q =>
    * PARAM q_hard <wc_queue_list> 
    *
    * TODO: EB: CLEANUP: make a function for the code blocks of -soft -q, -hard -q and -masterq
    */
   {
      lList *hard_queue_list = lGetList(old_job, JB_hard_queue_list);

      if (hard_queue_list != NULL) {
         lListElem *queue;
         bool first = true;

         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s q_hard", prefix);
         for_each(queue, hard_queue_list) {
            const char *queue_pattern = lGetString(queue, QR_name);

            sge_dstring_append_char(&buffer, first ? ' ' : ','); 
            sge_dstring_sprintf_append(&buffer, "%s", queue_pattern);  
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }
   {
      lList *soft_queue_list = lGetList(old_job, JB_soft_queue_list);

      if (soft_queue_list != NULL) {
         lListElem *queue;
         bool first = true;

         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s q_soft", prefix);
         for_each(queue, soft_queue_list) {
            const char *queue_pattern = lGetString(queue, QR_name);

            sge_dstring_append_char(&buffer, first ? ' ' : ','); 
            sge_dstring_sprintf_append(&buffer, "%s", queue_pattern);  
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -R y|n 
    * optional; only available if specified during submission and value is y
    */
   {
      bool reserve = lGetBool(old_job, JB_reserve) ? true : false;
  
      if (reserve) { 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s R %c", prefix, reserve ? 'y' : 'n');
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -r y|n 
    * optional; only available if specified during submission and value is y 
    */
   {
      u_long32 restart = lGetUlong(old_job, JB_restart);
  
      if (restart == 1) { 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s r %c", prefix, (restart == 1) ? 'y' : 'n');
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -sc (handled as part of -ac) */

   /* 
    * -shell y|n 
    * optional; only available if -shell n was specified    
    */
   {
      bool no_shell = job_is_no_shell(old_job);

      if (no_shell) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s shell %c", prefix, !no_shell ? 'y' : 'n');
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* -soft (handled as l_soft and q_soft) */

   /* -sync y|n (not available in JSV) */

   /* 
    * -S shell_path_list (optional) 
    * PARAM S [hostname:]path,...
    */
   {
      lList *shell_list = lGetList(old_job, JB_shell_list);

      if (shell_list != NULL) {
         lListElem *shell;
         bool first = true;
 
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s S", prefix);
         for_each(shell, shell_list) {
            const char *hostname = lGetHost(shell, PN_host);
            const char *path = lGetString(shell, PN_path);
           
             
            sge_dstring_append_char(&buffer, first ? ' ' : ',');
            if (hostname != NULL) {
               sge_dstring_append(&buffer, hostname); 
               sge_dstring_append_char(&buffer, ':');
               sge_dstring_append(&buffer, path);
            } else {
               sge_dstring_append(&buffer, path);
            }
            first = false;
         }
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* 
    * -t min[-max[:step]] (optional; only available if specified during submission 
    * and if values differ from "1-1:1") 
    * PARAM t_min <number>
    * PARAM t_max <number>
    * PARAM t_step <number>
    */
   {
      lList *ja_structure_list = lGetList(old_job, JB_ja_structure); 
      lListElem *ja_structure = lFirst(ja_structure_list);

      if (ja_structure != NULL) {
         u_long32 min, max, step;

         range_get_all_ids(ja_structure, &min, &max, &step);

         /*
          * if -t is not specified then all values will be 1 therefore we have to 
          * provide the values to JSV only if one value differs from 1
          */
         if (max != 1 || min != 1 || step != 1) {
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s t_min "sge_U32CFormat, prefix, sge_u32c(min));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s t_max "sge_U32CFormat, prefix, sge_u32c(max));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "%s t_step "sge_U32CFormat, prefix, sge_u32c(step));
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }
      }
   }

   /* -terse (ignored in JSV. it is just too late to change this) */
   /* -u username,... (not handled in JSV because only available for qalter) */
   /* -v variable[=value],... (handles also -V; done below after all params are handled */
   /* -verbose (not available in JSV) */
   /* -V (handled as part of -v) */

   /* 
    * -w e|w|n|v|p 
    * optional; only sent to JSV if != 'n') 
    */
   {
      u_long32 verify = lGetUlong(old_job, JB_verify_suitable_queues);
   
      if (verify > 0) {
         sge_dstring_clear(&buffer);
         sge_dstring_sprintf(&buffer, "%s w ", prefix);
         job_get_verify_attr(verify, &buffer);
         jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
      }
   }

   /* command (handled as PARAM SCRIPT NAME above) */
   /* command_args (handled as PARAM SCRIPT above) */
   /* xterm_args (handled as PARAM SCRIPT above) */

   /* 
    * handle -v -V and -display here 
    * TODO: EB: JSV: PARSING
    */  
   {
      lList *env_list = NULL;
      lListElem *env = NULL;
      lListElem *display = NULL;

      /* make a copy of the environment */
      var_list_copy_env_vars_and_value(&env_list, lGetList(old_job, JB_env_list));

      /* remove certain variables which don't come from the user environment */
      var_list_remove_prefix_vars(&env_list, VAR_PREFIX);
      var_list_remove_prefix_vars(&env_list, VAR_PREFIX_NR);

      /* 
       * if there is a DISPLAY variable and if the client is qsh/qrsh
       * then we will send the DISPLAY value as if it originally came 
       * from -display switch.
       */
      display = lGetElemStr(env_list, VA_variable, "DISPLAY"); 
      if (display != NULL) {
         const char *value = lGetString(display, VA_value);
         u_long32 progid = ctx->get_who(ctx);

         if (value != NULL && 
             (strcmp(prognames[progid], "qsh") == 0 || strcmp(prognames[progid], "qrsh") == 0)) {
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "PARAM display %s", value);
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         } 
      }

      /* send the variable to the JSV but only if it was requested */
      if (lGetBool(jsv, JSV_send_env) == true) {
         for_each(env, env_list) {
            const char *value = lGetString(env, VA_value);
            const char *name = lGetString(env, VA_variable);
            size_t length, i;
      
            sge_dstring_clear(&buffer);
            sge_dstring_sprintf(&buffer, "ENV ADD %s ", name);
            length = (value != NULL) ? strlen(value) : 0;
            for (i = 0; i < length; i++) {
               char in[] = {  
                  '\\', '\n', '\r', '\t', '\a', '\b', '\v', '\0'
               };
               char *out[] = {
                  "\\", "\\n", "\\r", "\\t", "\\a", "\\b", "\\v", ""
               };
               int j = 0;
               bool already_handled = false;
   
               while (in[j] != '\0') {
                  if (in[j] == value[i]) {
                     sge_dstring_append(&buffer, out[j]);
                     already_handled = true;
                  }
                  j++;
               }
               if (!already_handled) {
                  sge_dstring_append_char(&buffer, value[i]); 
               }
            }
            jsv_send_command(jsv, answer_list, sge_dstring_get_string(&buffer));
         }
      }
      lFreeList(&env_list);
   }

   /* script got all parameters. now verification can start */
   if (ret) {
      ret &= jsv_send_command(jsv, answer_list, "BEGIN");
   }

   /* cleanup */
   sge_dstring_free(&buffer);

   DRETURN(ret);
}

static bool
jsv_handle_log_command(sge_gdi_ctx_class_t *ctx _UNUSED, lListElem *jsv,
                       lList **answer_list _UNUSED,
                       dstring *c _UNUSED, dstring *s, dstring *a)
{
   bool ret = true;
   const char *command = sge_dstring_get_string(s);
   const char *sub_command = sge_dstring_get_string(s);
   const char *args = sge_dstring_get_string(a);

   DENTER(TOP_LAYER, "jsv_handle_log_command");
   if (args == NULL) {
      /* empty message will print a empty line (only newline) */
      args = "";
   }
   if (strcmp(lGetString(jsv, JSV_context), JSV_CONTEXT_CLIENT) != 0) {
      if (strcmp(sub_command, "INFO") == 0) {
         INFO((SGE_EVENT, "%s", args));
      } else if (strcmp(sub_command, "WARNING") == 0) {
         WARNING((SGE_EVENT, "%s", args));
      } else if (strcmp(sub_command, "ERROR") == 0) {
         ERROR((SGE_EVENT, "%s", args));
      } else {
         WARNING((SGE_EVENT, MSG_JSV_LOG_SS, command, sub_command));
      }
   } else {
      printf("%s\n", args);
   }
   DRETURN(ret);
}

static bool
jsv_handle_env_command(sge_gdi_ctx_class_t *ctx _UNUSED, lListElem *jsv, lList **answer_list,
                       dstring *c _UNUSED, dstring *s, dstring *a)
{
   bool ret = true;
   dstring variable = DSTRING_INIT;
   dstring value = DSTRING_INIT;
   const char *mod;
   const char *var;
   const char *val;
   bool skip_check = false;
   lList *local_answer_list = NULL;
   lListElem *new_job = lGetRef(jsv, JSV_new_job);

   DENTER(TOP_LAYER, "jsv_handle_env_command");

   jsv_split_token(a, &variable, &value);
   mod = sge_dstring_get_string(s);
   var = sge_dstring_get_string(&variable);
   val = sge_dstring_get_string(&value);

   DPRINTF(("got from JSV \"%s %s %s\"", mod, var, (val != NULL) ? val : ""));

   if (strcmp(var, "__JSV_TEST_RESULT") == 0) {
      lSetBool(jsv, JSV_test, true);
      lSetUlong(jsv, JSV_test_pos, 0);
      lSetString(jsv, JSV_result, val);
      skip_check = true;
   }

   if (skip_check != true) {
      if (mod != NULL && var != NULL &&
          (((strcmp(mod, "MOD") == 0 || strcmp(mod, "ADD") == 0) && val != NULL) ||
            (strcmp(mod, "DEL") == 0 && val == NULL))) {
         lList *env_list = lGetList(new_job, JB_env_list);
         lListElem *env_variable = NULL;

         if (var != NULL) {
            env_variable = lGetElemStr(env_list, VA_variable, var); 
         }
         if (strcmp("ADD", mod) == 0 || strcmp("MOD", mod) == 0) {
            if (env_variable == NULL) {
               env_variable = lAddSubStr(new_job, VA_variable, var, JB_env_list, VA_Type);
            }
            lSetString(env_variable, VA_value, val);
         } else if (strcmp("DEL", mod) == 0) {
            if (env_variable != NULL) {
               lRemoveElem(env_list, &env_variable);
            }
         } else {
            answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                    "\"ENV %s %s %s\" is invalid\n",
                                    (mod != NULL) ? mod : "<null>", 
                                    (var != NULL) ? var : "<null>", 
                                    (val != NULL) ? val : "<null>");
            ret = false;
         }
      } else {
         answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                 "\"ENV %s %s %s\" is invalid\n",
                                 (mod != NULL) ? mod : "<null>", 
                                 (var != NULL) ? var : "<null>", 
                                 (val != NULL) ? val : "<null>");
         ret = false;
      }
   }

   /*
    * if we are in test mode the we have to check the expected result.
    * in test mode we will reject jobs if we did not get the expected
    * result otherwise
    * we will accept the job with the ret value set above including
    * the created error messages.
    */
   if (lGetBool(jsv, JSV_test) && !skip_check) {
      const char *result_string = lGetString(jsv, JSV_result);
      u_long32 result_pos = lGetUlong(jsv, JSV_test_pos);

      if (strlen(result_string) > result_pos) {
         char result_char = result_string[result_pos];
         bool result = (result_char == '1') ? true : false;

         if (result != ret) {
            answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                    "\"ENV %s %s %s\" was unexpectedly %s\n",
                                    (mod != NULL) ? mod : "<null>", 
                                    (var != NULL) ? var : "<null>", 
                                    (val != NULL) ? val : "<null>",
                                    ret ? "accepted" : "rejected");
            ret = false;
         } else {
            ret = true;
         }
      }
      lSetUlong(jsv, JSV_test_pos, lGetUlong(jsv, JSV_test_pos) + 1);
   }
   answer_list_append_list(answer_list, &local_answer_list);

   sge_dstring_free(&variable);
   sge_dstring_free(&value);
   DRETURN(ret);
}

/****** sgeobj/jsv/jsv_do_communication() **************************************
*  NAME
*     jsv_do_communication() -- Starts communicating with a JSV script 
*
*  SYNOPSIS
*     bool 
*     jsv_do_communication(sge_gdi_ctx_class_t *ctx, lListElem *jsv, 
*                          lList **answer_list) 
*
*  FUNCTION
*     Start a communication cycle to verify one job. The job to
*     be verified has to be store in 'jsv' as attribute 'JSV_new_job'.
*     Depending on the response of the JSV instance certain attributes
*     in the 'jsv' will be changes (JSV_restart, JSV_soft_shutdown, 
*     JSV_done, JSV_new_job)
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - GE context 
*     lListElem *jsv           - JSV_Type instance
*     lList **answer_list      - AN_Type list 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: jsv_do_communication() is MT safe 
*******************************************************************************/
bool 
jsv_do_communication(sge_gdi_ctx_class_t *ctx, lListElem *jsv, lList **answer_list)
{
   bool ret = true;
   char input[10000];

   DENTER(TOP_LAYER, "jsv_do_communication");
   if (ret) {
      /* 
       * Try to read some error messages from stderr. There still has no command been send
       * to JSV but the initialization code might also cause errors....
       */
      while (fscanf(lGetRef(jsv, JSV_err), "%[^\n]\n", input) == 1) {
         ERROR((SGE_EVENT, MSG_JSV_LOGMSG_S, input));
         answer_list_add_sprintf(answer_list, STATUS_DENIED,
                                 ANSWER_QUALITY_ERROR, "%s", SGE_EVENT);
         ret = false; 
      }
   }
   if (ret) {
      DPRINTF(("JSV - START will be sent\n"));
      ret &= jsv_send_command(jsv, answer_list, "START");
   }
   if (ret) {
      u_long32 start_time = sge_get_gmt();
      bool do_retry = true;
      int jsv_timeout = 10;
      
      if (strcmp(lGetString(jsv, JSV_context), JSV_CONTEXT_CLIENT) == 0 && getenv("SGE_JSV_TIMEOUT") != NULL) {
         if (atoi(getenv("SGE_JSV_TIMEOUT")) > 0) {
            jsv_timeout = atoi(getenv("SGE_JSV_TIMEOUT")); 
            DPRINTF(("JSV_TIMEOUT value of %d s being used from environment variable\n", jsv_timeout))
         }         
      } else {
         jsv_timeout = mconf_get_jsv_timeout();
         DPRINTF(("JSV_TIMEOUT value of %d s being used from qmaster parameter\n", jsv_timeout))
      }

      lSetBool(jsv, JSV_done, false);
      lSetBool(jsv, JSV_soft_shutdown, true);
      while (!lGetBool(jsv, JSV_done)) {
         if (sge_get_gmt() - start_time > jsv_timeout) {
            DPRINTF(("JSV - master waited longer than %d s to get response from JSV\n", jsv_timeout));
            /*
             * In case of a timeout we try it a second time. In that case we kill
             * the old instance and start a new one before we continue
             * with the verification. Otherwise we report an error which will
             * automatically reject the job which should be verified.
             */
            if (do_retry) {
               DPRINTF(("JSV - will retry verification\n")); 
               lSetBool(jsv, JSV_restart, false);
               lSetBool(jsv, JSV_accept, false);
               lSetBool(jsv, JSV_done, false);
               DPRINTF(("JSV process will be stopped now\n"));
               ret &= jsv_stop(jsv, answer_list, false);
               if (ret) {
                  DPRINTF(("JSV process will be started now\n"));
                  ret &= jsv_start(jsv, answer_list);
               }
               if (ret) {
                  DPRINTF(("JSV process gets START command\n"));
                  while (fscanf(lGetRef(jsv, JSV_err), "%[^\n]\n", input) == 1) {
                     ERROR((SGE_EVENT, MSG_JSV_LOGMSG_S, input));  
                  }
                  ret &= jsv_send_command(jsv, answer_list, "START");
               }
               start_time = sge_get_gmt();
               do_retry = false;
            } else {
               DPRINTF(("JSV - reject due to timeout in communication process\n")); 
               answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                       "got no response from JSV script "SFQ, 
                                       lGetString(jsv, JSV_command));
               lSetBool(jsv, JSV_restart, true);
               lSetBool(jsv, JSV_soft_shutdown, false);
               lSetBool(jsv, JSV_done, true);
            }
         } else {
            FILE *err_stream = lGetRef(jsv, JSV_err);
            FILE *out_stream = lGetRef(jsv, JSV_out);

            /* 
             * read a line from the script or wait some time before you try again
             * but do this only if there was no message an the stderr stream.
             */
            if (ret) {
               if (fscanf(out_stream, "%[^\n]\n", input) == 1) {
                  dstring sub_command = DSTRING_INIT;
                  dstring command = DSTRING_INIT;
                  dstring args = DSTRING_INIT;
                  jsv_command_t commands[] = {
                     {"PARAM", jsv_handle_param_command},
                     {"ENV", jsv_handle_env_command},
                     {"LOG", jsv_handle_log_command},
                     {"RESULT", jsv_handle_result_command},
                     {"SEND", jsv_handle_send_command},
                     {"STARTED", jsv_handle_started_command},
                     {NULL, NULL}
                  };
                  bool handled = false;
                  const char *c;
                  int i = -1;

                  DPRINTF(("JSV << \"%s\"\n", input));

                  jsv_split_commandline(input, &command, &sub_command, &args);
                  c = sge_dstring_get_string(&command);
      
                  while(commands[++i].command != NULL) {
                     if (strcmp(c, commands[i].command) == 0) {
                        handled = true;
                        ret &= commands[i].func(ctx, jsv, answer_list, 
                                                &command, &sub_command, &args);

                        if (ret == false || 
                            lGetBool(jsv, JSV_restart) == true || 
                            lGetBool(jsv, JSV_accept) == true) {
                           lSetBool(jsv, JSV_done, true);
                        }
                        break;
                     }
                  }

                  if (!handled) {
                     answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                             MSG_JSV_JCOMMAND_S, c);
                     lSetBool(jsv, JSV_accept, false);
                     lSetBool(jsv, JSV_restart, true);
                     lSetBool(jsv, JSV_done, true);
                  }

                  /*
                   * set start time for one iteration
                   */
                  start_time = sge_get_gmt();
                  sge_dstring_free(&sub_command);
                  sge_dstring_free(&command);
                  sge_dstring_free(&args);
               } else {
                  sge_usleep(10000);
               }
            }

            /* 
             * try to read a line from the error stream. If there is something then
             * restart the script before next check, do not communicate with script 
             * anymore during shutdown. The last message in the stderr stream will be
             * send as answer to the calling function.
             */
            while (fscanf(err_stream, "%[^\n]\n", input) == 1) {
               ERROR((SGE_EVENT, MSG_JSV_LOGMSG_S, input));  
               answer_list_add_sprintf(answer_list, STATUS_DENIED,
                                       ANSWER_QUALITY_ERROR, "%s", SGE_EVENT);
               ret = false;
            }
            if (!ret && lGetBool(jsv, JSV_done) == false) {
               answer_list_add_sprintf(answer_list, STATUS_DENIED, ANSWER_QUALITY_ERROR,
                                       "JSV stderr is - %s", input);
               lSetBool(jsv, JSV_restart, true);
               lSetBool(jsv, JSV_soft_shutdown, false);
               lSetBool(jsv, JSV_done, true);
            }
         }
      }
   }
   return ret;
}

static char *
jsv_cull_attr2switch_name(int cull_attr, lListElem *job) 
{
   char *ret = NULL;

   DENTER(TOP_LAYER, "jsv_cull_attr2switch_name");
   if (cull_attr == JB_execution_time) {
      ret = "a";
   } else if (cull_attr == JB_context) {
      ret = "ac"; /* although it might also be a ds or sc request we return ac here */
   } else if (cull_attr == JB_ar) {
      ret = "ar";
   } else if (cull_attr == JB_account) {
      ret = "A";
   } else if (cull_attr == JB_binding) {
      ret = "binding";
   } else if (cull_attr == JB_checkpoint_interval) {
      ret = "c_interval";
   } else if (cull_attr == JB_checkpoint_attr) {
      ret = "c_occasion";
   } else if (cull_attr == JB_checkpoint_name) {
      ret = "ckpt";
   } else if (cull_attr == JB_cwd) {
      ret = "cwd";
   } else if (cull_attr == JB_deadline) {
      ret = "dl";
   } else if (cull_attr == JB_stderr_path_list) {
      ret = "e";
   } else if (cull_attr == JB_jid_request_list) {
      ret = "hold_jid";
   } else if (cull_attr == JB_ja_ad_request_list) {
      ret = "hold_jid_ad";
   } else if (cull_attr == JB_ja_tasks) {
      ret = "h";
   } else if (cull_attr == JB_stdin_path_list) {
      ret = "i";
   } else if (cull_attr == JB_merge_stderr) {
      ret = "j";
   } else if (cull_attr == JB_jobshare) {
      ret = "js";
   } else if (cull_attr == JB_hard_resource_list) {
      ret = "l_hard";
   } else if (cull_attr == JB_soft_resource_list) {
      ret = "l_soft";
   } else if (cull_attr == JB_mail_options) {
      ret = "m";
   } else if (cull_attr == JB_master_hard_queue_list) {
      ret = "masterq";
   } else if (cull_attr == JB_notify) {
      ret = "notify";
   } else if (cull_attr == JB_mail_list) {
      ret = "M";
   } else if (cull_attr == JB_job_name) {
      /*
       * This is a special case for JB_job_name parameter. 
       * A) null
       *    qalter ... <jid>         => qalter with job id
       * B) <string>
       *    qalter -N <string> <jid> => qalter with job id using -N option 
       *                                to change name
       * C) :<job_name>:
       *    qalter ... <job_name>    => qalter with job name instead
       *                                of job id
       * D) :<job_name>:<string2>             
       *    qalter -N <string2> <job_name> => qalter with job name instead of
       *                                      job id using -N option to change name
       */
      #define JOB_NAME_DEL ':'
      const char *job_name = lGetString(job, JB_job_name);
      if (job_name != NULL) {
         if (job_name[0] == JOB_NAME_DEL) {
            const char *help_str = strchr(&(job_name[1]), JOB_NAME_DEL);
            if (help_str != NULL && help_str[1] != '\0') {
               /* This is case D) */
               ret = "N";
            }
         } else {
              /* This is case B) */
            ret = "N";
         }
      }
   } else if (cull_attr == JB_stdout_path_list) {
      ret = "o";
   } else if (cull_attr == JB_project) {
      ret = "P";
   } else if (cull_attr == JB_priority) {
      ret = "p";
   } else if (cull_attr == JB_pe) {
      ret = "pe_name";
   } else if (cull_attr == JB_pe_range) {
      ret = "pe_min";
   } else if (cull_attr == JB_hard_queue_list) {
      ret = "q_hard";
   } else if (cull_attr == JB_soft_queue_list) {
      ret = "q_soft";
   } else if (cull_attr == JB_reserve) {
      ret = "R";
   } else if (cull_attr == JB_restart) {
      ret = "r";
   } else if (cull_attr == JB_shell_list) {
      ret = "S";
   } else if (cull_attr == JB_ja_structure) {
      ret = "t";
   } else if (cull_attr == JB_env_list) {
      ret = "v"; /* v will be returned even if V was specified */
   } else if (cull_attr == JB_verify_suitable_queues) {
      ret = "w";
   } else if (cull_attr == JB_script_file) {
      ret = "CMDNAME";
   }
   DRETURN(ret);
}

bool
jsv_is_modify_rejected(sge_gdi_ctx_class_t *context, lList **answer_list, lListElem *job) 
{
   bool ret = false;

   DENTER(TOP_LAYER, "jsv_is_modify_rejected");
   if (job != NULL) {
      const char *jsv_allowed_mod = mconf_get_jsv_allowed_mod();
      const char *jsv_url = mconf_get_jsv_url();

      if (jsv_url && strcasecmp(jsv_url, "none") != 0) {

         /*
          * Check now if there are allowed modifications.
          */
         if (jsv_allowed_mod && strcmp(jsv_allowed_mod, "none") != 0) {
            const lDescr *descr = lGetElemDescr(job);
            const lDescr *pointer = NULL;
            lList *allowed_switches = NULL;
            lList *got_switches = NULL;
            lListElem *allowed = NULL;

            /*
             * Transform the cull fields into a list of corresponding 
             * qalter switch names.
             */
            str_list_parse_from_string(&allowed_switches, jsv_allowed_mod, ",");
            for (pointer = descr; pointer->nm != NoName; pointer++) {
               const char *swch = jsv_cull_attr2switch_name(pointer->nm, job);
             
               if (swch != NULL) {
                  lAddElemStr(&got_switches, ST_name, swch, ST_Type);
               }
            }

            /*
             * Even if not specified on commandline. The information of the
             * -w switch is always passed to qalter. We must allow it even if it
             * was not specified.
             */
            allowed = lGetElemStr(allowed_switches, ST_name, "w");
            if (allowed == NULL) {
               lAddElemStr(&allowed_switches, ST_name, "w", ST_Type);
            }

            /*
             * Allow -t switch automatically if -h is used. The corresponding
             * information of -t in the CULL data structure is used to
             * send the information of -h.
             */
            allowed = lGetElemStr(allowed_switches, ST_name, "h");
            if (allowed != NULL) {
               allowed = lGetElemStr(allowed_switches, ST_name, "t");

               if (allowed == NULL) {
                  lAddElemStr(&allowed_switches, ST_name, "t", ST_Type);
               }
            }

            /*
             * Remove the allowed switches from the list of switches which were
             * applied to the job we got.
             */
            for_each(allowed, allowed_switches) {
               const char *name = lGetString(allowed, ST_name);
               const void *iterator = NULL;
               lListElem *got;
               lListElem *got_next;

               got_next = lGetElemStrFirst(got_switches, ST_name, name, &iterator);
               while ((got = got_next) != NULL) {
                  got_next = lGetElemStrNext(got_switches, ST_name, name, &iterator);

                  lRemoveElem(got_switches, &got);
               }
            }

            /*
             * If there are no remaining switches then the request will not be rejected.
             */
            if (lGetNumberOfElem(got_switches) == 0) {
               ret = false;
            } else {
               lListElem *not_allowed;
               dstring switches = DSTRING_INIT;
               bool first = true;

               for_each (not_allowed, got_switches) {
                  if (first) {
                     first = false;
                  } else {
                     sge_dstring_append_char(&switches, ',');
                  } 
                  sge_dstring_append(&switches, lGetString(not_allowed, ST_name));
               }
               ERROR((SGE_EVENT, MSG_JSV_SWITCH_S, sge_dstring_get_string(&switches)));
               answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               sge_dstring_free(&switches);
               ret = true;
            }

            if (allowed_switches) {
               lFreeList(&allowed_switches);
            }
            if (got_switches) {
               lFreeList(&got_switches);
            }
         } else {
            /*
             * JSV is active but no modification allowed
             */
            ERROR((SGE_EVENT, SFNMAX, MSG_JSV_ALLOWED));
            answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = true;
         }
      }
      sge_free(&jsv_allowed_mod);
      sge_free(&jsv_url);
   }
   DRETURN(ret);
}


