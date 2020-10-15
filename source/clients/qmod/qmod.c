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
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_language.h"
#include "uti/sge_unistd.h"
#include "uti/sge_profiling.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/msg_sgeobjlib.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_qinstance_state.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sge_options.h"
#include "usage.h"
#include "sig_handlers.h"
#include "parse_qsub.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qmod.h"


static lList *sge_parse_cmdline_qmod(char **argv, char **envp, lList **ppcmdline);
static lList *sge_parse_qmod(lList **ppcmdline, lList **ppreflist, u_long32 *pforce);

static int qmod_usage(FILE *fp, char *what);

static bool answer_list_has_exit_code_error(lList **answer_list);

extern char **environ;


int main(
int argc,
char **argv
) {
   u_long32 force = 0;
   lList *ref_list = NULL;
   lList *alp = NULL, *pcmdline = NULL;
   lListElem *aep;
   sge_gdi_ctx_class_t *ctx = NULL;
   bool answ_list_has_err = false;

   DENTER_MAIN(TOP_LAYER, "qmod");

   prof_mt_init();

   log_state_set_log_gui(1);
   sge_setup_sig_handlers(QMOD);

   if (sge_gdi2_setup(&ctx, QMOD, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   /*
   ** static func for parsing all qmod specific switches
   ** here we get action, force, ref_list
   */
   alp = sge_parse_cmdline_qmod(++argv, environ, &pcmdline);
   if(alp) {
      /*
      ** high level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      SGE_EXIT((void**)&ctx, 1);
   }

   alp = sge_parse_qmod(&pcmdline, &ref_list, &force);

   if(alp) {
      /*
      ** low level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      lFreeList(&ref_list);
      SGE_EXIT((void**)&ctx, 1);
   }

   {
      lListElem *idep = NULL;
      for_each(idep, ref_list) {
         lSetUlong(idep, ID_force, force);
      }
   }

   if (ref_list) {
      alp = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_TRIGGER, &ref_list, NULL, NULL, false);
   }

   answ_list_has_err = answer_list_has_exit_code_error(&alp); 

   /*
   ** show answer list
   */
   for_each(aep, alp) {
      fprintf(stdout, "%s\n", lGetString(aep, AN_text));
   }

   lFreeList(&alp);
   lFreeList(&ref_list);
   lFreeList(&pcmdline);

   sge_prof_cleanup();

   if(answ_list_has_err) {
      SGE_EXIT((void**)&ctx, 1);
   }
   else {
      SGE_EXIT((void**)&ctx, 0);
   }
   DEXIT;
   return 0;
}


/****** qmod/answer_list_has_exit_code_error() *********************************
*  NAME
*     answer_list_has_exit_code_error() -- Returns if there was a critical error 
*                                          or when the return status was not OK. 
*
*  SYNOPSIS
*     static bool answer_list_has_exit_code_error(lList **answer_list) 
*
*  FUNCTION
*     Checks the answer list if there was any critical error or in the other 
*     cases if there was a status other than ok. 
*
*  INPUTS
*     lList **answer_list - AN_Type list
*
*  RESULT
*     static bool - "true" if an error is found "false" otherwise 
*
*  NOTES
*     MT-NOTE: answer_list_has_exit_code_error() is not MT safe 
*
*******************************************************************************/
static bool answer_list_has_exit_code_error(lList **answer_list)
{
   bool ret = false;

   DENTER(TOP_LAYER, "answer_list_has_exit_code_error");

   if (answer_list_has_quality(answer_list, ANSWER_QUALITY_CRITICAL) == true) {
      ret = true;
   } else {
      lListElem *answer;   /* AN_Type */
      /* check each ERROR if the status is really != 1 (STATUS_OK) */
      u_long32 status;
      for_each(answer, *answer_list) {
         if (answer_has_quality(answer, ANSWER_QUALITY_ERROR)) {
            status = answer_get_status(answer);       
            if (status != STATUS_OK) {
               ret = true;
            }
         }   
      }
  } 

   DRETURN(ret);
}            


/****
 **** sge_parse_cmdline_qmod (static)
 ****
 **** 'stage 1' parsing of qmod-options. Parses options
 **** with their arguments and stores them in ppcmdline.
 ****/
static lList *sge_parse_cmdline_qmod(
char **argv,
char **envp,
lList **ppcmdline
) {
char **sp;
char **rp;
lList *alp = NULL;

   DENTER(TOP_LAYER, "sge_parse_cmdline_qmod");

   rp = argv;

   if (*rp == NULL) {
      /* no command line argument: print help on error */
      qmod_usage(stderr, NULL);
      answer_list_add_sprintf(&alp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                              "%s", MSG_PARSE_NOOPTIONARGUMENT);
   }

   while(*(sp=rp)) {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", "--help", ppcmdline, &alp)) != sp)
         continue;

      /* -f option */
      if ((rp = parse_noopt(sp, "-f", "--force", ppcmdline, &alp)) != sp)
         continue;

      /* -c option */
      if ((rp = parse_until_next_opt(sp, "-c", "--clear", ppcmdline, &alp)) != sp)
         continue;

      /* -cj option */
      if ((rp = parse_until_next_opt(sp, "-cj", "--clearjob", ppcmdline, &alp)) != sp)
         continue;

      /* -cq option */
      if ((rp = parse_until_next_opt(sp, "-cq", "--clearqueue", ppcmdline, &alp)) != sp)
         continue;

      /* -s option */
      if ((rp = parse_until_next_opt(sp, "-s", "--suspend", ppcmdline, &alp)) != sp)
         continue;

      /* -sj option */
      if ((rp = parse_until_next_opt(sp, "-sj", "--suspendjob", ppcmdline, &alp)) != sp)
         continue;

      /* -sq option */
      if ((rp = parse_until_next_opt(sp, "-sq", "--suspendqueue", ppcmdline, &alp)) != sp)
         continue;

      /* -us option */
      if ((rp = parse_until_next_opt(sp, "-us", "--unsuspend", ppcmdline, &alp)) != sp)
         continue;

      /* -usj option */
      if ((rp = parse_until_next_opt(sp, "-usj", "--unsuspendjob", ppcmdline, &alp)) != sp)
         continue;

      /* -usq option */
      if ((rp = parse_until_next_opt(sp, "-usq", "--unsuspendqueue", ppcmdline, &alp)) != sp)
         continue;

      /* -d option */
      if ((rp = parse_until_next_opt(sp, "-d", "--disable", ppcmdline, &alp)) != sp)
         continue;

      /* -rj option */
      if ((rp = parse_until_next_opt(sp, "-rj", "--reschedulejob", ppcmdline, &alp)) != sp)
         continue;

      /* -rq option */
      if ((rp = parse_until_next_opt(sp, "-rq", "--reschedulequeue", ppcmdline, &alp)) != sp)
         continue;

      /* -r option */
      if ((rp = parse_until_next_opt(sp, "-r", "--reschedule", ppcmdline, &alp)) != sp)
         continue;

      /* -e option */
      if ((rp = parse_until_next_opt(sp, "-e", "--enable", ppcmdline, &alp)) != sp)
         continue;

      /* -t */
      if (!strcmp("-t", *sp)) {
         lList *task_id_range_list = NULL;
         lListElem *ep_opt;

         /* next field is path_name */
         sp++;
         if (*sp == NULL) {
             answer_list_add(&alp, MSG_PARSE_TOPTIONMUSTHAVEALISTOFTASKIDRANGES, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
             goto error;
         }

         DPRINTF(("\"-t %s\"\n", *sp));

         range_list_parse_from_string(&task_id_range_list, &alp, *sp,
                                      false, true, INF_NOT_ALLOWED);
         if (!task_id_range_list) {
            goto error;
         }

         range_list_sort_uniq_compress(task_id_range_list, &alp, true);
         if (lGetNumberOfElem(task_id_range_list) > 1) {
            answer_list_add(&alp, MSG_QCONF_ONLYONERANGE, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            goto error;
         }

         ep_opt = sge_add_arg(ppcmdline, t_OPT, lListT, *(sp - 1), *sp);
         lSetList(ep_opt, SPA_argval_lListT, task_id_range_list);

         sp++;
         rp = sp;
         continue;
      }

#ifdef __SGE_QINSTANCE_STATE_DEBUG__
      if ((rp = parse_until_next_opt(sp, "-_e", "--_error", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_o", "--_orphaned", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_do", "--_dorphaned", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_u", "--_unknown", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_du", "--_dunknown", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_c", "--_confambiguous", ppcmdline, &alp)) != sp)
         continue;
      if ((rp = parse_until_next_opt(sp, "-_dc", "--_dconfambiguous", ppcmdline, &alp)) != sp)
         continue;
#endif

      /* oops */
      answer_list_add_sprintf(&alp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR, MSG_PARSE_INVALIDOPTIONARGUMENTX_S, *sp);
error:
      qmod_usage(stderr, NULL);
      DEXIT;
      return alp;
   }
   DEXIT;
   return alp;
}

/****
 **** sge_parse_qmod (static)
 ****
 **** 'stage 2' parsing of qmod-options. Gets the options from
 **** ppcmdline, sets the force and action flags and puts the
 **** queue/job-names/numbers in ppreflist.
 ****/
static lList *sge_parse_qmod(lList **ppcmdline, lList **ppreflist, u_long32 *pforce)
{
   lList *alp = NULL;
   u_long32 helpflag;
   int usageshowed = 0;

   DENTER(TOP_LAYER, "sge_parse_qmod");

   /* Loop over all options. Only valid options can be in the
      ppcmdline list. Except f_OPT all options are exclusive.
   */
   while(lGetNumberOfElem(*ppcmdline)) {
      lListElem *ep;
      static const char *options[] = {
         "-c",
         "-cj",
         "-cq",
         "-d",
         "-r",
         "-rj",
         "-rq",
         "-e",
         "-s",
         "-sj",
         "-sq",
         "-us",
         "-usj",
         "-usq",
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
         "-_e",
         "-_o",
         "-_do",
         "-_u",
         "-_du",
         "-_c",
         "-_dc",
#endif
         NULL
      };
      static const u_long32 transitions[] = {
         QI_DO_CLEARERROR,
         QI_DO_CLEARERROR | JOB_DO_ACTION,
         QI_DO_CLEARERROR | QUEUE_DO_ACTION,
         QI_DO_DISABLE | QUEUE_DO_ACTION,
         QI_DO_RESCHEDULE,
         QI_DO_RESCHEDULE | JOB_DO_ACTION,
         QI_DO_RESCHEDULE | QUEUE_DO_ACTION,
         QI_DO_ENABLE | QUEUE_DO_ACTION,
         QI_DO_SUSPEND,
         QI_DO_SUSPEND | JOB_DO_ACTION,
         QI_DO_SUSPEND | QUEUE_DO_ACTION,
         QI_DO_UNSUSPEND,
         QI_DO_UNSUSPEND | JOB_DO_ACTION,
         QI_DO_UNSUSPEND | QUEUE_DO_ACTION,
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
         QI_DO_SETERROR,
         QI_DO_SETORPHANED,
         QI_DO_CLEARORPHANED,
         QI_DO_SETUNKNOWN,
         QI_DO_CLEARUNKNOWN,
         QI_DO_SETAMBIGUOUS,
         QI_DO_CLEARAMBIGUOUS,
#endif
         QI_DO_NOTHING
      };
      int i;

      if (parse_flag(ppcmdline, "-help",  &alp, &helpflag)) {
         usageshowed = qmod_usage(stdout, NULL);
         break;
      }

      if (parse_flag(ppcmdline, "-f", &alp, pforce)) {
         continue;
      }

      i = 0;
      while (options[i] != NULL) {
         if ((transitions[i] & QUEUE_DO_ACTION) == 0) {
            parse_multi_jobtaskslist(ppcmdline, options[i], &alp, ppreflist, true, transitions[i]);
         } else {
            lList *queueList = NULL;
            if (parse_multi_stringlist( ppcmdline, options[i], &alp, &queueList, ST_Type, ST_name)){
               id_list_build_from_str_list(ppreflist, &alp, queueList, transitions[i], *pforce);
            }
            lFreeList(&queueList);
         }
         i++;
      }

      /* we get to this point, than there are -t options without job names. We have to write an error message */
      if ((ep = lGetElemStr(*ppcmdline, SPA_switch, "-t")) != NULL) {
         answer_list_add_sprintf(&alp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                                 MSG_JOB_LONELY_TOPTION_S, lGetString(ep, SPA_switch_arg));

         break;
      }

   }

   if (lGetNumberOfElem(*ppcmdline)) {
      if (!usageshowed) {
         qmod_usage(stderr, NULL);
      }
      answer_list_add_sprintf(&alp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                              "%s", MSG_PARSE_TOOMANYOPTIONS);
   }

   DRETURN(alp);
}

/****
 **** qmod_usage (static)
 ****
 **** displays usage of qmod on file fp.
 **** Is what NULL, full usage will be displayed.
 ****
 **** Returns always 1.
 ****
 **** If what is a pointer to an option-string,
 **** only usage for that option will be displayed.
 ****   ** not implemented yet! **
 ****/
static int qmod_usage(
FILE *fp,
char *what
) {
   dstring ds;
   char buffer[256];

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   fprintf(fp, "%s\n", feature_get_product_name(FS_SHORT_VERSION, &ds));

   if(!what) {
      /* display full usage */
      fprintf(fp,"%s qmod [options]\n", MSG_SRC_USAGE);
      fprintf(fp, "   [-c job_wc_queue_list]  %s\n", MSG_QMOD_c_OPT_USAGE);
      fprintf(fp, "   [-cj job_list]          %s\n", MSG_QMOD_c_OPT_USAGE_J);
      fprintf(fp, "   [-cq wc_queue_list]     %s\n", MSG_QMOD_c_OPT_USAGE_Q);

      fprintf(fp, "   [-d wc_queue_list]      %s\n", MSG_QMOD_d_OPT_USAGE);
      fprintf(fp, "   [-e wc_queue_list]      %s\n", MSG_QMOD_e_OPT_USAGE);
      fprintf(fp, "   [-f]                    %s\n", MSG_QMOD_f_OPT_USAGE);
      fprintf(fp, "   [-help]                 %s\n", MSG_COMMON_help_OPT_USAGE);
      fprintf(fp, "   [-r job_wc_queue_list]  %s\n", MSG_QMOD_r_OPT_USAGE);
      fprintf(fp, "   [-rj job_list]          %s\n", MSG_QMOD_r_OPT_USAGE_J);
      fprintf(fp, "   [-rq wc_queue_list]     %s\n", MSG_QMOD_r_OPT_USAGE_Q);

      fprintf(fp, "   [-s job_wc_queue_list]  %s\n", MSG_QMOD_s_OPT_USAGE);
      fprintf(fp, "   [-sj job_list]          %s\n", MSG_QMOD_s_OPT_USAGE_J);
      fprintf(fp, "   [-sq wc_queue_list]     %s\n", MSG_QMOD_s_OPT_USAGE_Q);
      fprintf(fp, "   [-us job_wc_queue_list] %s\n", MSG_QMOD_us_OPT_USAGE);
      fprintf(fp, "   [-usj job_list]         %s\n", MSG_QMOD_us_OPT_USAGE_J);
      fprintf(fp, "   [-usq wc_queue_list]    %s\n", MSG_QMOD_us_OPT_USAGE_Q);

#ifdef __SGE_QINSTANCE_STATE_DEBUG__
      fprintf(fp, "   [-_e queue_list]        %s\n", MSG_QMOD_err_OPT_ISAGE);
      fprintf(fp, "   [-_o queue_list]        %s\n", MSG_QMOD_o_OPT_ISAGE);
      fprintf(fp, "   [-_do queue_list]       %s\n", MSG_QMOD_do_OPT_ISAGE);
      fprintf(fp, "   [-_u queue_list]        %s\n", MSG_QMOD_u_OPT_ISAGE);
      fprintf(fp, "   [-_du queue_list]       %s\n", MSG_QMOD_du_OPT_ISAGE);
      fprintf(fp, "   [-_c queue_list]        %s\n", MSG_QMOD_c_OPT_ISAGE);
      fprintf(fp, "   [-_dc queue_list]       %s\n", MSG_QMOD_dc_OPT_ISAGE);
#endif
      fprintf(fp, "\n");
      fprintf(fp, "job_wc_queue_list          {job_tasks|wc_queue}[{','|' '}{job_tasks|wc_queue}[{','|' '}...]]\n");
      fprintf(fp, "job_list                   {job_tasks}[{','|' '}job_tasks[{','|' '}...]]\n");
      fprintf(fp, "job_tasks                  {{job_id'.'task_id_range}|job_name|pattern}[' -t 'task_id_range]\n");
      fprintf(fp, "task_id_range              task_id['-'task_id[':'step]]\n");
      fprintf(fp, "wc_cqueue                  %s\n", MSG_QSTAT_HELP_WCCQ);
      fprintf(fp, "wc_host                    %s\n", MSG_QSTAT_HELP_WCHOST);
      fprintf(fp, "wc_hostgroup               %s\n", MSG_QSTAT_HELP_WCHG);
      fprintf(fp, "wc_qinstance               wc_cqueue@wc_host\n");
      fprintf(fp, "wc_qdomain                 wc_cqueue@wc_hostgroup\n");
      fprintf(fp, "wc_queue                   wc_cqueue|wc_qdomain|wc_qinstance\n");
      fprintf(fp, "wc_queue_list              wc_queue[','wc_queue[','...]]\n");
   } else {
      /* display option usage */
      fprintf(fp, MSG_QDEL_not_available_OPT_USAGE_S,what);
      fprintf(fp, "\n");
   }
   return 1;
}

