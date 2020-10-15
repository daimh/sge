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
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_io.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_str.h"

#include "basis_types.h"
#include "sge.h"
#include "read_defaults.h"
#include "parse_qsub.h"
#include "usage.h"
#include "sig_handlers.h"
#include "qrstat.h"
#include "qrstat_report_handler.h"
#include "qrstat_report_handler_xml.h"
#include "qrstat_report_handler_stdout.h"
#include "qrstat_filter.h"
#include "msg_common.h"

extern char **environ;

static bool
sge_parse_from_file_qrstat(const char *file, lList **ppcmdline, lList **alpp);

static bool 
sge_parse_qrstat(sge_gdi_ctx_class_t *ctx, lList **answer_list,
                 qrstat_env_t *qrstat_env, lList **cmdline)
{
   bool ret = true;
   
   DENTER(TOP_LAYER, "sge_parse_qrstat");

   qrstat_env->is_summary = true;
   while (lGetNumberOfElem(*cmdline)) {
      u_long32 value;
   
      /* -help */
      if (opt_list_has_X(*cmdline, "-help")) {
         sge_usage(QRSTAT, stdout);
         DEXIT;
         SGE_EXIT((void**)&ctx, 0);
      }

      /* -u */
      while (parse_multi_stringlist(cmdline, "-u", answer_list, 
                                    &(qrstat_env->user_list), ST_Type, ST_name)) {
         continue;
      }

      /* -explain */
      while (parse_flag(cmdline, "-explain", answer_list, &value)) {
         qrstat_filter_add_core_attributes(qrstat_env);
         qrstat_filter_add_explain_attributes(qrstat_env);
         qrstat_env->is_explain = (value > 0) ? true : false;
         continue;
      }

      /* -xml */
      while (parse_flag(cmdline, "-xml", answer_list, &value)) {
         qrstat_filter_add_core_attributes(qrstat_env);
         qrstat_filter_add_xml_attributes(qrstat_env);
         qrstat_env->is_xml = (value > 0) ? true : false;
         continue;
      }

      /* -ar */
      while (parse_u_longlist(cmdline, "-ar", answer_list, &(qrstat_env->ar_id_list))) {         
         qrstat_filter_add_core_attributes(qrstat_env);
         qrstat_filter_add_ar_attributes(qrstat_env);
         qrstat_filter_add_ar_where(qrstat_env);
         qrstat_env->is_summary = false;
         continue;      
      }

      if (lGetNumberOfElem(*cmdline)) {
         sge_usage(QRSTAT, stdout);
         answer_list_add(answer_list, MSG_PARSE_TOOMANYOPTIONS, 
                         STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         ret = false;
         break;
      }
   } 

   if (qrstat_env->is_summary) {
      char user[128] = "";
      if (sge_uid2user(geteuid(), user, sizeof(user), MAX_NIS_RETRIES)) {
         answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC,
                                 ANSWER_QUALITY_CRITICAL,
                                 MSG_SYSTEM_RESOLVEUSER_U, (u_long32)geteuid());
         ret = false;
      } else {
         str_list_transform_user_list(&(qrstat_env->user_list), answer_list, user);
         qrstat_filter_add_core_attributes(qrstat_env);
         qrstat_filter_add_u_where(qrstat_env);
      }
   }

   DRETURN(ret);
}

/************************************************************************/
int main(int argc, char **argv) {
   int ret = 0;
   lList *pcmdline = NULL;
   lList *answer_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   qrstat_env_t qrstat_env;

   DENTER_MAIN(TOP_LAYER, "qrsub");

   /* Set up the program information name */
   sge_setup_sig_handlers(QRSTAT);

   log_state_set_log_gui(1);

   if (sge_gdi2_setup(&ctx, QRSTAT, MAIN_THREAD, &answer_list) != AE_OK) {
      answer_list_output(&answer_list);
      goto error_exit;
   }

   qrstat_filter_init(&qrstat_env);
   qrstat_filter_set_ctx(&qrstat_env, ctx);

   /*
    * stage 1: commandline parsing
    */
   {
      dstring file = DSTRING_INIT;
      const char *user = ctx->get_username(ctx);
      const char *cell_root = ctx->get_cell_root(ctx);

      /* arguments from SGE_ROOT/common/sge_qrstat file */
      get_root_file_path(&file, cell_root, SGE_COMMON_DEF_QRSTAT_FILE);
      if (sge_parse_from_file_qrstat(sge_dstring_get_string(&file), &pcmdline, &answer_list) == true) {
         /* arguments from $HOME/.sge_qrstat file */
         if (get_user_home_file_path(&file, SGE_HOME_DEF_QRSTAT_FILE, user, &answer_list)) {
            sge_parse_from_file_qrstat(sge_dstring_get_string(&file), &pcmdline, &answer_list);
         }
      }
      sge_dstring_free(&file); 

      if (answer_list) {
         answer_list_output(&answer_list);
         lFreeList(&pcmdline);
         sge_prof_cleanup();
         SGE_EXIT((void**)&ctx, 1);
      }
   }

   answer_list = cull_parse_cmdline(QRSTAT, argv+1, environ, &pcmdline, FLG_USE_PSEUDOS);
   if (answer_list != NULL) {
      answer_list_output(&answer_list);
      lFreeList(&pcmdline);
      goto error_exit;
   }
  
   /* 
    * stage 2: evalutate switches and modify qrstat_env
    */
   if (!sge_parse_qrstat(ctx, &answer_list, &qrstat_env, &pcmdline)) {
      answer_list_output(&answer_list);
      lFreeList(&pcmdline);
      goto error_exit;
   }

   /* 
    * stage 3: fetch data from master 
    */
   {
      answer_list = ctx->gdi(ctx, SGE_AR_LIST, SGE_GDI_GET, &qrstat_env.ar_list, 
                     qrstat_env.where_AR_Type, qrstat_env.what_AR_Type, false);

      if (answer_list_has_error(&answer_list)) {
         answer_list_output(&answer_list);
         goto error_exit;
      }
   }

   /*
    * stage 4: create output in correct format
    */
   {
      qrstat_report_handler_t *handler = NULL;

      if (qrstat_env.is_xml) {
         handler = qrstat_create_report_handler_xml(&qrstat_env, &answer_list);
      } else {
         handler = qrstat_create_report_handler_stdout(&qrstat_env, &answer_list);
      }
      if (!qrstat_print(&answer_list, handler, &qrstat_env)) {
         ret = 1;
      }
      if (qrstat_env.is_xml) {
         qrstat_destroy_report_handler_xml(&handler, &answer_list); 
      } else {
         qrstat_destroy_report_handler_stdout(&handler, &answer_list); 
      }
   }

   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   DRETURN(ret);

error_exit:
   sge_gdi2_shutdown((void**)&ctx);
   sge_prof_cleanup();
   SGE_EXIT((void**)&ctx, 1);
   DRETURN(1);
}

static bool
sge_parse_from_file_qrstat(const char *file, lList **ppcmdline, lList **alpp)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_parse_from_file_qrstat");

   if (ppcmdline == NULL) {
      ret = false;
   } else {
      if (!sge_is_file(file)) {
         /*
          * This is no error
          */
         DPRINTF(("file "SFQ" does not exist\n", file));
      } else {
         char *file_as_string = NULL;
         int file_as_string_length;

         file_as_string = sge_file2string(file, &file_as_string_length);
         if (file_as_string == NULL) {
            answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, 
                                    ANSWER_QUALITY_ERROR,
                                    MSG_ANSWER_ERRORREADINGFROMFILEX_S, file);
            ret = false;
         } else {
            char **token = NULL;

            token = stra_from_str(file_as_string, " \n\t");
            *alpp = cull_parse_cmdline(QRSTAT, token, environ, ppcmdline, FLG_USE_PSEUDOS);
         }
      }
   }  
   DRETURN(ret); 
}
