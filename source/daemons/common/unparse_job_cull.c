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
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"
#include "uti/sge_language.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/parse.h"
#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_answer.h" 
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_mailrec.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_qref.h"

#include "sched/valid_queue_user.h"

#include "parse_qsub.h"
#include "parse_job_cull.h"
#include "unparse_job_cull.h"
#include "sge.h"
#include "symbols.h"
#include "sge_options.h"
#include "msg_daemons_common.h"


static char *sge_unparse_checkpoint_attr(int opr, char *string);
/* static char *sge_unparse_hold_list(u_long32 hold); */
static char *sge_unparse_mail_options(u_long32 mail_opt);
static int sge_unparse_checkpoint_option(lListElem *job, lList **pcmdline, lList **alpp);
static int sge_unparse_account_string(lListElem *job, lList **pcmdline, lList **alpp);
static int sge_unparse_path_list(lListElem *job, int nm, char *option, lList **pcmdline, lList **alpp);
static int sge_unparse_pe(lListElem *job, lList **pcmdline, lList **alpp);

static int sge_unparse_resource_list(lListElem *job, int nm, lList **pcmdline, lList **alpp);
static int sge_unparse_string_option(lListElem *job, int nm, char *option, lList **pcmdline, lList **alpp);

lList *cull_unparse_job_parameter(sge_gdi_ctx_class_t *ctx, lList **pcmdline, lListElem *job, int flags)
{
   const char *cp;
   u_long32 ul;
   lList *answer = NULL;
   char str[MAX_STRING_SIZE];
   lList *lp;
   int ret;
   lListElem *ep_opt;
   const char *username = ctx->get_username(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "cull_unparse_job_parameter");

   /*
   ** -a
   ** problem with submission time, but that is not a good
   ** default option anyway, is not unparsed
   */

   /*
   ** -A
   */
   if (sge_unparse_account_string(job, pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }

   /*
   ** -c
   */
   if (sge_unparse_checkpoint_option(job, pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }
  
   /*
    * -ckpt 
    */
   if (sge_unparse_string_option(job, JB_checkpoint_name, "-ckpt", 
            pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }


   /*
   ** -cwd
   */
   if (lGetString(job, JB_cwd)) {
      ep_opt = sge_add_noarg(pcmdline, cwd_OPT, "-cwd", NULL);
   }

   /*
    * -P
    */
   if (sge_unparse_string_option(job, JB_project, "-P",
            pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }

#if 0
   /*
   ** -C
   */
   if (sge_unparse_string_option(job, JB_directive_prefix, "-C", 
            pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }
#endif   

   /*
   ** -e
   */
   if (sge_unparse_path_list(job, JB_stderr_path_list, "-e", pcmdline, 
                     &answer) != 0) {
      DEXIT;
      return answer;
   }

   /*
   ** -h, here only user hold supported at the moment
   */
   if  ((ul = lGetUlong(lFirst(lGetList(job, JB_ja_tasks)), JAT_hold))) {
      ep_opt = sge_add_noarg(pcmdline, h_OPT, "-h", NULL);
   }

   /*
   ** -hold_jid
   */
   if ((lp = lGetList(job, JB_jid_request_list))) {
      int fields[] = { JRE_job_name, 0 };
      const char *delis[] = {NULL, ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 0);
      if (ret) {
         DPRINTF(("Error %d formatting jid_request_list as -hold_jid\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGJIDPREDECESSORLISTASHOLDJID);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, hold_jid_OPT, lListT, "-hold_jid", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("hold_jid list", lp));      
   }

   /*
   ** -hold_jid_ad
   */
   if ((lp = lGetList(job, JB_ja_ad_request_list))) {
      int fields[] = { JRE_job_name, 0 };
      const char *delis[] = {NULL, ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 0);
      if (ret) {
         DPRINTF(("Error %d formatting ja_ad_request_list as -hold_jid_ad\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGJIDPREDECESSORLISTASHOLDJIDAD);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, hold_jid_ad_OPT, lListT, "-hold_jid_ad", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("hold_jid_ad list", lp));      
   }

   /*
   ** -i
   */
   if (sge_unparse_path_list(job, JB_stdin_path_list, "-i", pcmdline, 
                     &answer) != 0) {
      DEXIT;
      return answer;
   }

   /*
   ** -j
   */
   if ((ul = lGetBool(job, JB_merge_stderr))) {
      ep_opt = sge_add_arg(pcmdline, j_OPT, lIntT, "-j", "y");
      lSetInt(ep_opt, SPA_argval_lIntT, true);
   }
   
   /*
   ** -jid
   */
   if ((lp = lGetList(job, JB_job_identifier_list))) {
      int fields[] = { JRE_job_number, 0};
      const char *delis[] = {"", ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 
         0);
      if (ret) {
         DPRINTF(("Error %d formatting job_identifier_list as -jid\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGJOBIDENTIFIERLISTASJID);
         DRETURN(answer);
      }
      ep_opt = sge_add_arg(pcmdline, jid_OPT, lListT, "-jid", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("jid list", lp));      
   }

   /*
   ** -js
   */
   if ((ul = lGetUlong(job, JB_jobshare)) != 0)  {
      sprintf(str, sge_u32, ul);
      ep_opt = sge_add_arg(pcmdline, js_OPT, lUlongT, "-js", str);
      lSetUlong(ep_opt, SPA_argval_lUlongT, ul);
   }

   /*
   ** -lj is in parsing but can't be unparsed here
   */

   /*
   ** -l
   */
   if (sge_unparse_resource_list(job, JB_hard_resource_list,
            pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }
   if (sge_unparse_resource_list(job, JB_soft_resource_list,
            pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }






   /*
   ** -m
   */
   if ((ul = lGetUlong(job, JB_mail_options))) {
      cp = sge_unparse_mail_options(ul);
      if (!cp) {
         DPRINTF(("Error unparsing mail options\n"));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_PARSE_ERRORUNPARSINGMAILOPTIONS);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, m_OPT, lIntT, "-m", cp);
      lSetInt(ep_opt, SPA_argval_lIntT, ul);
   }

   /*
   ** -M obviously a problem!!!
   ** not unparsed at the moment
   ** does it make sense as a default, after all?
   */
   if ((lp = lGetList(job, JB_mail_list))) {
      lList *lp_new = NULL;
      lListElem *ep_new = NULL;
      lListElem *ep = NULL;
      const char *host;
      const char *user;

      /*
      ** or rather take all if there are more than one elements?
      */
      for_each(ep, lp) {
         user = lGetString(ep, MR_user);
         host = lGetHost(ep, MR_host);
         if (sge_strnullcmp(user, username) || 
             sge_hostcmp(host, qualified_hostname)) {
            lp_new = lCreateList("mail list", MR_Type);
            ep_new = lAddElemStr(&lp_new, MR_user, user, MR_Type);
            lSetHost(ep_new, MR_host, host);
         }
      }
      if (lp_new) {
         int fields[] = { MR_user, MR_host, 0 };
         const char *delis[] = {"@", ",", NULL};

         ret = uni_print_list(NULL, str, sizeof(str) - 1, lp_new, fields, delis, FLG_NO_DELIS_STRINGS);
         if (ret) {
            DPRINTF(("Error %d formatting mail list as -M\n", ret));
            answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                    "%s", MSG_LIST_ERRORFORMATTINGMAILLISTASM);
            return answer;
         }
         ep_opt = sge_add_arg(pcmdline, M_OPT, lListT, "-M", str);
         lSetList(ep_opt, SPA_argval_lListT, lp_new);
 
      }
   }

   /*
   ** -N
   ** dont unparse the job name!
   ** each job has a jobname, but not each defaults file wants a -N
   */
#if 0
   if ((cp = lGetString(job, JB_job_name))) {
      ep_opt = sge_add_arg(pcmdline, N_OPT, lStringT, "-N", cp);
      lSetString(ep_opt, SPA_argval_lStringT, cp);
   }
#endif
   /*
   ** -notify
   */
   if  ((ul = lGetBool(job, JB_notify))) {
      ep_opt = sge_add_noarg(pcmdline, notify_OPT, "-notify", NULL);
   }

   /*
   ** -o
   */
   if (sge_unparse_path_list(job, JB_stdout_path_list, "-o", pcmdline, 
                     &answer) != 0) {
      DEXIT;
      return answer;
   }

   /*
   ** -p
   */
   if ((ul = lGetUlong(job, JB_priority)) != BASE_PRIORITY)  {
      int prty;

      prty = ul - BASE_PRIORITY;
      if (prty > 1024) {
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_PROC_INVALIDPROIRITYMUSTBELESSTHAN1025);
         DEXIT;
         return answer;
      }
      if (prty < -1023) {
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_PROC_INVALIDPRIORITYMUSTBEGREATERTHANMINUS1024);
         DEXIT;
         return answer;
      }
      sprintf(str, "%d", prty);
      ep_opt = sge_add_arg(pcmdline, p_OPT, lIntT, "-p", str);
      lSetInt(ep_opt, SPA_argval_lIntT, prty);
   }

   /*
   ** -pe
   */
   if (sge_unparse_pe(job, pcmdline, &answer) != 0) {
      DEXIT;
      return answer;
   }


#if 1
   /*
   ** -q
   ** exec-string is suppressed because uni_print_list can't do this
   ** is not in the manual anyway
   */
   if ((lp = lGetList(job, JB_hard_queue_list))) {
      int fields[] = { QR_name, 0 };
      const char *delis[] = {"@", ",", NULL};

      ep_opt = sge_add_noarg(pcmdline, hard_OPT, "-hard", NULL);
      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 
         FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting hard_queue_list as -q\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGHARDQUEUELISTASQ);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, q_OPT, lListT, "-q", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("hard queue list", lp));      
      lSetInt(ep_opt, SPA_argval_lIntT, 1); /* means hard */
   }
   if ((lp = lGetList(job, JB_soft_queue_list))) {
      int fields[] = { QR_name, 0 };
      const char *delis[] = {"@", ",", NULL};

      ep_opt = sge_add_noarg(pcmdline, soft_OPT, "-soft", NULL);
      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 
         FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting soft_queue_list as -q\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGSOFTQUEUELISTASQ);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, q_OPT, lListT, "-q", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("soft queue list", lp));      
      lSetInt(ep_opt, SPA_argval_lIntT, 2); /* means soft */
   }
#endif
   
   /*
   ** -R
   */
   if ((ul = lGetBool(job, JB_reserve))) {
      ep_opt = sge_add_arg(pcmdline, R_OPT, lIntT, "-R", "y");
      lSetInt(ep_opt, SPA_argval_lIntT, true);
   }

   /*
   ** -r
   */
   if ((lGetUlong(job, JB_restart) == 1)) {
      ep_opt = sge_add_arg(pcmdline, r_OPT, lIntT, "-r", "y");
      lSetInt(ep_opt, SPA_argval_lIntT, 1);
   } else if ((lGetUlong(job, JB_restart) == 2)) {
      ep_opt = sge_add_arg(pcmdline, r_OPT, lIntT, "-r", "n");
      lSetInt(ep_opt, SPA_argval_lIntT, 2);
   }

   /*
   ** -S
   */
#if 1   
   if (sge_unparse_path_list(job, JB_shell_list, "-S", pcmdline, 
                     &answer) != 0) {
      DEXIT;
      return answer;
   }
#else
   if ((lp = lGetList(job, JB_shell_list))) {
      int fields[] = { PN_host, PN_file_host, PN_path, PN_file_staging, 0 };
      const char *delis[] = {":", ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting shell_list\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 MSG_LIST_ERRORFORMATINGSHELLLIST);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, S_OPT, lListT, "-S", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("shell list", lp));
   }
#endif

   /*
   ** -v, -V
   ** we always generate a -v statement, which means the user should not
   ** declare a job generated with -V as default
   */
   if ((lp = lGetList(job, JB_env_list))) {
      int fields[] = { VA_variable, VA_value, 0};
      const char *delis[] = {"=", ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 
         FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting environment list as -v\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGENVIRONMENTLISTASV);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, v_OPT, lListT, "-v", str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("env list", lp));      
   }

   /*
   ** -verify is not unparsed
   */

   /*
   ** for full cmdline, script is needed
   */
   if ((flags & FLG_FULL_CMDLINE) &&
      (cp = lGetString(job, JB_script_file))) {
      if (!strcmp(cp, "STDIN")) {
         if (lGetList(job, JB_job_args)) {
         ep_opt = sge_add_arg(pcmdline, 0, lStringT, STR_PSEUDO_SCRIPT, "--");
         lSetString(ep_opt, SPA_argval_lStringT, "--");
         }
      }
      else {
      }
   }

   /*
   ** for full cmdline, job args are also needed
   */
   if ((flags & FLG_FULL_CMDLINE) &&
      (lp = lGetList(job, JB_job_args))) {
      int fields[] = { ST_name, 0};
      const char *delis[] = {NULL, " ", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, 
         FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting job arguments\n", ret));
         answer_list_add_sprintf(&answer, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGJOBARGUMENTS);
         return answer;
      }
      ep_opt = sge_add_arg(pcmdline, 0, lListT, STR_PSEUDO_JOBARG, str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("job arguments", lp));      
   }




   DEXIT;
   return answer;
}


static char *sge_unparse_checkpoint_attr(int opr, char *str)
{
   int i = 0;

   if (opr & CHECKPOINT_AT_MINIMUM_INTERVAL)
      str[i++] = CHECKPOINT_AT_MINIMUM_INTERVAL_SYM;
   if (opr & CHECKPOINT_AT_SHUTDOWN)
      str[i++] = CHECKPOINT_AT_SHUTDOWN_SYM;
   if (opr & CHECKPOINT_SUSPEND)
      str[i++] = CHECKPOINT_SUSPEND_SYM;
   if (opr & NO_CHECKPOINT)
      str[i++] = NO_CHECKPOINT_SYM;

   str[i] = '\0';

   return str;
}

/*-------------------------------------------------------------------------*/
static char *sge_unparse_mail_options(
u_long32 mail_opt 
) {
   static char mail_str[5 + 1];
   char *pc;

   DENTER(TOP_LAYER, "sge_unparse_mail_options");

   memset(mail_str, 0, sizeof(mail_str));
   pc = mail_str;
   if (VALID(MAIL_AT_ABORT, mail_opt)) {
      *pc++ = 'a';
   }
   if (VALID(MAIL_AT_BEGINNING, mail_opt)) {
      *pc++ = 'b';
   }
   if (VALID(MAIL_AT_EXIT, mail_opt)) {
      *pc++ = 'e';
   }
   if (VALID(NO_MAIL, mail_opt)) {
      *pc++ = 'n';
   }
   if (VALID(MAIL_AT_SUSPENSION, mail_opt)) {
      *pc++ = 's';
   }
   
   if  (!*mail_str) {
      DEXIT;
      return NULL;
   }

   DEXIT;
   return (mail_str);
}

/*-------------------------------------------------------------------------*/
static int sge_unparse_account_string(
lListElem *job,
lList **pcmdline,
lList **alpp 
) {
   const char *cp;
   lListElem *ep_opt;

   DENTER(TOP_LAYER, "sge_unparse_account_string");
   
   if ((cp = lGetString(job, JB_account))) {
      if (strcmp(cp, DEFAULT_ACCOUNT)) {
         ep_opt = sge_add_arg(pcmdline, A_OPT, lStringT, "-A", cp);
         lSetString(ep_opt, SPA_argval_lStringT, cp);
      }
   }
   DEXIT;
   return 0;
}


/*-------------------------------------------------------------------------*/
static int sge_unparse_checkpoint_option(lListElem *job, lList **pcmdline, lList **alpp)
{
   lListElem *ep_opt = NULL;
   char *cp;
   int i;
   u_long32 ul;
   char str[256];

   DENTER(TOP_LAYER, "sge_unparse_checkpoint_option");

   if ((i = lGetUlong(job, JB_checkpoint_attr))) {
      if ((cp = sge_unparse_checkpoint_attr(i, str))) {
         ep_opt = sge_add_arg(pcmdline, 0, lIntT, "-c", cp);
         lSetInt(ep_opt, SPA_argval_lIntT, i);
      } else {
         answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 MSG_JOB_INVALIDVALUEFORCHECKPOINTATTRIBINJOB_U,
                                 sge_u32c(lGetUlong(job, JB_job_number)));
         DRETURN(-1);
      }
   }

   if ((ul = lGetUlong(job, JB_checkpoint_interval))) {
      snprintf(str, sizeof(str), sge_uu32, ul);
      ep_opt = sge_add_arg(pcmdline, c_OPT, lLongT, "-c", str);
      lSetLong(ep_opt, SPA_argval_lLongT, (long) ul);
   }

   DRETURN(0);
}


/*-------------------------------------------------------------------------*/
static int sge_unparse_string_option(
lListElem *job,
int nm,
char *option,
lList **pcmdline,
lList **alpp 
) {
   lListElem *ep_opt = NULL;
   const char *cp;

   DENTER(TOP_LAYER, "sge_unparse_string_option");
   
   if ((cp = lGetString(job, nm))) {
      ep_opt = sge_add_arg(pcmdline, 0, lStringT, option, cp);
      lSetString(ep_opt, SPA_argval_lStringT, cp);
   }
   DEXIT;
   return 0;
}


/*-------------------------------------------------------------------------*/
static int sge_unparse_resource_list(lListElem *job, int nm, lList **pcmdline, lList **alpp)
{
   lList *lp;
   int ret = 0;
   char str[MAX_STRING_SIZE];

   DENTER(TOP_LAYER, "sge_unparse_resource_list");

   if ((lp = lGetList(job, nm))) {
      lListElem *ep_opt;
      int hard = (nm == JB_hard_resource_list);

      if (hard) {
         ep_opt = sge_add_noarg(pcmdline, hard_OPT, "-hard", NULL);
      }
      else {
         ep_opt = sge_add_noarg(pcmdline, soft_OPT, "-soft", NULL);
      }

      ret = centry_list_append_to_string(lp, str, sizeof(str) - 1);
      if (ret) {
         DPRINTF(("Error %d formatting hard_resource_list as -l\n", ret));
         answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGHARDRESOURCELISTASL);
         return ret;
      }
      if (*str && (str[strlen(str) - 1] == '\n')) {
         str[strlen(str) - 1] = 0;
      }
      ep_opt = sge_add_arg(pcmdline, l_OPT, lListT, "-l", str);

      if (hard)
         lSetList(ep_opt, SPA_argval_lListT, lCopyList("hard res", lp));
      else
         lSetList(ep_opt, SPA_argval_lListT, lCopyList("soft res", lp));

      if (hard) 
         lSetInt(ep_opt, SPA_argval_lIntT, 1); /* means hard */
      else
         lSetInt(ep_opt, SPA_argval_lIntT, 2); /* means soft */
   }
   DEXIT;
   return ret;
}

/*-------------------------------------------------------------------------*/
static int sge_unparse_pe(
lListElem *job,
lList **pcmdline,
lList **alpp 
) {
   const char *cp;
   lList *lp = NULL;
   lListElem *ep_opt;
   dstring string_buffer = DSTRING_INIT;
   int ret = 0;

   DENTER(TOP_LAYER, "sge_unparse_pe");

   if ((cp = lGetString(job, JB_pe))) {
      sge_dstring_append(&string_buffer, cp);
      sge_dstring_append(&string_buffer, " ");
      if (!(lp = lGetList(job, JB_pe_range))) {
         DPRINTF(("Job has parallel environment with no ranges\n"));
         answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_JOB_JOBHASPEWITHNORANGES);
         sge_dstring_free(&string_buffer);
         DEXIT;
         return -1;
      }
      {
         dstring range_string = DSTRING_INIT;

         range_list_print_to_string(lp, &range_string, true, false, false);
         sge_dstring_append(&string_buffer, 
                            sge_dstring_get_string(&range_string));
         sge_dstring_free(&range_string);
      }
      if (ret) {
         DPRINTF(("Error %d formatting ranges in -pe\n", ret));
         answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGRANGESINPE);
         sge_dstring_free(&string_buffer);
         DEXIT;
         return ret;
      }
      ep_opt = sge_add_arg(pcmdline, pe_OPT, lStringT, "-pe", 
                           sge_dstring_get_string(&string_buffer));
      lSetString(ep_opt, SPA_argval_lStringT, cp);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList("pe ranges", lp));
   }
   sge_dstring_free(&string_buffer);
   DEXIT;
   return ret;
}

/*-------------------------------------------------------------------------*/
static int sge_unparse_path_list(lListElem *job, int nm, char *option, lList **pcmdline, lList **alpp)
{
   lList *lp = NULL;
   int ret = 0;
   char str[MAX_STRING_SIZE];
   lListElem *ep_opt;

   DENTER(TOP_LAYER, "sge_unparse_path_list");

   if ((lp = lGetList(job, nm))) {
      int fields[] = { PN_host, PN_path, 0 };
      const char *delis[] = {":", ",", NULL};

      ret = uni_print_list(NULL, str, sizeof(str) - 1, lp, fields, delis, FLG_NO_DELIS_STRINGS);
      if (ret) {
         DPRINTF(("Error %d formatting path_list\n", ret));
         answer_list_add_sprintf(alpp, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_LIST_ERRORFORMATINGPATHLIST);
         return ret;
      }
      ep_opt = sge_add_arg(pcmdline, e_OPT, lListT, option, str);
      lSetList(ep_opt, SPA_argval_lListT, lCopyList(option, lp));
   }
   DEXIT;
   return ret;
}

