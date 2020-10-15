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
 *  Portions of this code are Copyright 2011 Univa Inc.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/setup_path.h"
#include "uti/sge_prog.h"
#include "uti/sge_string.h"
#include "uti/sge_dstring.h"
#include "uti/sge_unistd.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_ulong.h"

#include "sched/sge_sched.h"
#include "sched/sge_schedd_text.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sig_handlers.h"
#include "show_job.h"
#include "qstat_printing.h"
#include "sge.h"
#include "basis_types.h"
#include "read_defaults.h"
#include "sge_qstat.h"
#include "qstat_xml.h"
#include "qstat_cmdline.h"
#include "msg_common.h"
#include "msg_clients_common.h"
#include "msg_qstat.h"


#define FORMAT_I_20 "%I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I %I "
#define FORMAT_I_10 "%I %I %I %I %I %I %I %I %I %I "
#define FORMAT_I_5 "%I %I %I %I %I "
#define FORMAT_I_2 "%I %I "
#define FORMAT_I_1 "%I "

static lList *sge_parse_qstat(sge_gdi_ctx_class_t *ctx, lList **ppcmdline, qstat_env_t *qstat_env,  
                              char **hostname, lList **ppljid, u_long32 *isXML);
static int qstat_show_job(sge_gdi_ctx_class_t *ctx, lList *jid, u_long32 isXML, qstat_env_t *qstat_env);
static int qstat_show_job_info(sge_gdi_ctx_class_t *ctx, u_long32 isXML, qstat_env_t *qstat_env);

typedef struct qstat_stdout_ctx_str qstat_stdout_ctx_t;

struct qstat_stdout_ctx_str {
   bool  header_printed;
   bool  job_header_printed;
   
   /* id of the last reported job */
   u_long32 last_job_id;
   dstring  last_queue_name;
   
   int  sub_task_count;
   int  hard_resource_count;
   int  soft_resource_count;
   int  hard_requested_queue_count;
   int  soft_requested_queue_count;
   int  master_hard_requested_queue_count;
   int  predecessor_requested_count;
   int  predecessor_count;
   int  ad_predecessor_requested_count;
   int  ad_predecessor_count;
};

static int qstat_stdout_init(qstat_handler_t *handler, lList **alpp);
static int qstat_stdout_queue_summary(qstat_handler_t* handler, const char* qname, queue_summary_t *summary, lList **alpp);
static int qstat_stdout_queue_finished(qstat_handler_t* handler, const char* qname, lList** alpp);

static int qstat_stdout_queue_load_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
static int qstat_stdout_queue_suspend_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
static int qstat_stdout_queue_message(qstat_handler_t* handler, const char* qname, const char *message, lList **alpp);
static int qstat_stdout_queue_resource(qstat_handler_t* handler, const char* dom, const char* name, const char* value, lList **alpp);
static int qstat_stdout_pending_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_stdout_finished_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_stdout_error_jobs_started(qstat_handler_t *handler, lList **alpp);
static int qstat_stdout_destroy(qstat_handler_t *handler);

static int job_stdout_init(job_handler_t *handler, lList** alpp);
static int job_stdout_job(job_handler_t* handler, u_long32 jid, job_summary_t *summary, lList **alpp);
static int job_stdout_sub_tasks_started(job_handler_t* handler, lList **alpp);
static int job_stdout_sub_task(job_handler_t* handler, task_summary_t *summary, lList **alpp);
static int job_stdout_sub_tasks_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_requested_pe(job_handler_t *handler, const char* pe_name, const char* pe_range, lList **alpp);
static int job_stdout_granted_pe(job_handler_t *handler, const char* pe_name, int pe_slots, lList **alpp);

static int job_stdout_additional_info(job_handler_t* handler, job_additional_info_t name, const char* value, lList **alpp);
static int job_stdout_request(job_handler_t* handler, const char* name, const char* value, lList **alpp);

static int job_stdout_hard_resources_started(job_handler_t* handler, lList **alpp);
static int job_stdout_hard_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
static int job_stdout_hard_resources_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_soft_resources_started(job_handler_t* handler, lList **alpp);
static int job_stdout_soft_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
static int job_stdout_soft_resources_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_hard_requested_queues_started(job_handler_t* handler, lList **alpp);
static int job_stdout_hard_requested_queue(job_handler_t* handler, const char* qname, lList **alpp);
static int job_stdout_hard_requested_queues_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_soft_requested_queues_started(job_handler_t* handler, lList **alpp);
static int job_stdout_soft_requested_queue(job_handler_t* handler, const char* qname, lList **alpp);
static int job_stdout_soft_requested_queues_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_master_hard_requested_queues_started(job_handler_t* handler, lList **alpp);
static int job_stdout_master_hard_request_queue(job_handler_t* handler, const char* qname, lList **alpp);
static int job_stdout_master_hard_requested_queues_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_predecessors_requested_started(job_handler_t* handler, lList **alpp);
static int job_stdout_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp);
static int job_stdout_predecessors_requested_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_predecessors_started(job_handler_t* handler, lList **alpp);
static int job_stdout_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp);
static int job_stdout_predecessors_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_ad_predecessors_requested_started(job_handler_t* handler, lList **alpp);
static int job_stdout_ad_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp);
static int job_stdout_ad_predecessors_requested_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_ad_predecessors_started(job_handler_t* handler, lList **alpp);
static int job_stdout_ad_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp);
static int job_stdout_ad_predecessors_finished(job_handler_t* handler, lList **alpp);

static int job_stdout_binding_started(job_handler_t* handler, lList **alpp);
static int job_stdout_binding(job_handler_t* handler, const char *binding, lList **alpp);
static int job_stdout_binding_finished(job_handler_t* handler, lList **alpp);

static void qselect_stdout_init(qselect_handler_t* handler, lList **alpp);
static int qselect_stdout_report_queue(qselect_handler_t* handler, const char* qname, lList **alpp);

static int cqueue_summary_stdout_init(cqueue_summary_handler_t *handler, lList **alpp);
static int cqueue_summary_stdout_report_started(cqueue_summary_handler_t *handler, lList **alpp);
static int cqueue_summary_stdout_report_cqueue(cqueue_summary_handler_t *handler, const char* cqname, cqueue_summary_t *summary, lList **alpp);

int main(int argc, char *argv[]);

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int main(
int argc,
char **argv 
) {
   lList *alp = NULL;
   lList *pcmdline = NULL;
   lList *pfile = NULL;
   lList *jid_list = NULL;
   lList *ref_list = NULL;
   lListElem *aep = NULL;
   lListElem *ep_1 = NULL;
   lListElem *ep_2 = NULL;
   char *hostname = NULL;
   const char *username = NULL;
   const char *cell_root = NULL;
   qstat_env_t qstat_env;
   u_long32 isXML = 0;
   sge_gdi_ctx_class_t *ctx = NULL;
   bool more = true;

   DENTER_MAIN(TOP_LAYER, "qstat");

   /* initialize the qstat_env */
   memset(&qstat_env, 0, sizeof(qstat_env_t));
   qstat_env.full_listing = QSTAT_DISPLAY_ALL;
   qstat_env.explain_bits = QI_DEFAULT;
   qstat_env.job_info = 0;
   qstat_env.group_opt = 0;
   qstat_env.queue_state = U_LONG32_MAX;
   qstat_env.longest_queue_length=30;
   qstat_env.need_queues = true;

   sge_sig_handler_in_main_loop = 0;
   sge_setup_sig_handlers(QSTAT);
   log_state_set_log_gui(true);

   if (sge_gdi2_setup(&ctx, QSTAT, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   username = ctx->get_username(ctx);
   cell_root = ctx->get_cell_root(ctx);
   lInit(nmv);      
   qstat_env.ctx = ctx;

   if (!strcmp(sge_basename(*argv++, '/'), "qselect")) {
      qstat_env.qselect_mode = 1;
   } else {
      qstat_env.qselect_mode = 0;
   }


   {
      dstring file = DSTRING_INIT;
      if (qstat_env.qselect_mode == 0) { /* the .sge_qstat file should only be used in the qstat mode */
         get_root_file_path(&file, cell_root, SGE_COMMON_DEF_QSTAT_FILE);
         /*
          * Support of local/global profile for qstat command: we
          * parse command options from both global and local qstat
          * profile. Each command option is represented by an
          * object of type SPA hanging off list 'pfile'.
          *
          * Next, parse command line options. Options from here are
          * put in a separate list 'pcmdline' to allow checking for
          * conflicting/duplicate options. This is required to
          * obey the override semantic defined for the command line.
          */
         switch_list_qstat_parse_from_file(&pfile, &alp, qstat_env.qselect_mode, 
                                           sge_dstring_get_string(&file));
         if (get_user_home_file_path(&file, SGE_HOME_DEF_QSTAT_FILE, username,
                                              &alp)) {
            switch_list_qstat_parse_from_file(&pfile, &alp, qstat_env.qselect_mode, 
                                           sge_dstring_get_string(&file));
         }
      }                                  
      switch_list_qstat_parse_from_cmdline(&pcmdline, &alp, qstat_env.qselect_mode, argv);
      /*
       * Walk option list given by command line and check
       * for matching options from file.
       */
      for_each(ep_1, pcmdline) {
         do {
            /*
             * Need that logic to handle multiple SPA
             * objects representing the same option.
             */
            more = false;
            for_each(ep_2, pfile) {
               if (strcmp(lGetString(ep_1, SPA_switch),
                       lGetString(ep_2, SPA_switch)) == 0) {
                  /*
                   * Bingo: remove dup.
                   */
                  lRemoveElem(pfile, &ep_2);
                  /*
                   * Start over again. We assume that the list
                   * is not that huge. The next iteration we
                   * may encounter another entry for the option
                   * just removed.
                   */
                  more = true;
                  break;
               }
            }
         } while(more);
      }
      /*
       * With dups removed we can now safely merge both lists.
       * Note that we can only append to a non-empty list.
       */
      if (lGetNumberOfElem(pcmdline) > 0) {
         lAppendList(pcmdline, pfile);
         lFreeList(&pfile);
      } else if (lGetNumberOfElem(pfile) > 0) {
         lAppendList(pfile, pcmdline);
         lFreeList(&pcmdline);
         pcmdline = pfile;
      }
      sge_dstring_free(&file);
   }
 
   if (alp) {
      /*
      ** high level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      qstat_env_destroy(&qstat_env);
      SGE_EXIT((void**)&ctx, 1);
   }

   alp = sge_parse_qstat(ctx, &pcmdline, &qstat_env, &hostname,   
                         &jid_list, &isXML);

   if (alp) {
      /*
      ** low level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      lFreeList(&ref_list);
      lFreeList(&jid_list);
      qstat_env_destroy(&qstat_env);
      SGE_EXIT((void**)&ctx, 1);
   }

   /* if -j, then only print job info and leave */
   if (qstat_env.job_info) {
      int ret = 0;

      if (lGetNumberOfElem(jid_list) > 0) {
         /* RH TODO: implement the qstat_show_job_info with and handler */
         ret = qstat_show_job(ctx, jid_list, isXML, &qstat_env);
      } else {
         /* RH TODO: implement the qstat_show_job_info with and handler */
         ret = qstat_show_job_info(ctx, isXML, &qstat_env);
      }
      qstat_env_destroy(&qstat_env);
      SGE_EXIT((void**)&ctx, ret);
   }

   {
      lList *answer_list = NULL;
      int ret = 0;

      str_list_transform_user_list(&(qstat_env.user_list), &answer_list, username);
      
      if (qstat_env.qselect_mode) {
         qselect_handler_t handler;
         if (isXML) {
            if(qselect_xml_init(&handler, &answer_list)) {
               for_each(aep, answer_list) {
                  fprintf(stderr, "%s\n", lGetString(aep, AN_text));
               }
               lFreeList(&answer_list);
               qstat_env_destroy(&qstat_env);
               SGE_EXIT((void**)&ctx, 1);
               return 1;
            }
         } else {
            qselect_stdout_init(&handler, &answer_list);
         }
         ret = qselect(&qstat_env, &handler, &answer_list);
         if (handler.destroy != NULL) {
            handler.destroy(&handler, &answer_list);
         }
      } else if (qstat_env.group_opt & GROUP_CQ_SUMMARY) {
         cqueue_summary_handler_t handler;
         if (isXML) {
            ret = cqueue_summary_xml_handler_init(&handler, &answer_list);
         } else {
            ret = cqueue_summary_stdout_init(&handler, &answer_list);
         }
         if (ret == 0) {
            ret = qstat_cqueue_summary(&qstat_env, &handler, &answer_list);
         }
         if (handler.destroy != NULL) {
            handler.destroy(&handler);
         }
      } else {
         qstat_handler_t handler;
         
         if (isXML) {
            ret = qstat_xml_handler_init(&handler, &answer_list);
         } else {
            ret = qstat_stdout_init(&handler, &answer_list);
         }
         
         if (ret == 0) {
            ret = qstat_no_group(&qstat_env, &handler, &answer_list);
         }
         
         if (handler.destroy != NULL ) {
            DPRINTF(("Destroy handler\n"));
            handler.destroy(&handler);
         }
      }

      answer_list_output(&answer_list);

      if (ret != 0) {
         qstat_env_destroy(&qstat_env);
         SGE_EXIT((void**)&ctx, 1);
         return 1;
      }
   }
   SGE_EXIT((void**)&ctx, 0);
   return 0;
}


/****
 **** sge_parse_qstat (static)
 ****
 **** 'stage 2' parsing of qstat-options. Gets the options from
 **** ppcmdline, sets the full and empry_qs flags and puts the
 **** queue/res/user-arguments into the lists.
 ****/
static lList *
sge_parse_qstat(sge_gdi_ctx_class_t *ctx, lList **ppcmdline, qstat_env_t *qstat_env,
                char **hostname, lList **ppljid, u_long32 *isXML)
{
   stringT str;
   lList *alp = NULL;
   u_long32 helpflag;
   int usageshowed = 0;
   char *argstr;
   u_long32 full = 0;
   lList *plstringopt = NULL; 


   DENTER(TOP_LAYER, "sge_parse_qstat");

   qstat_env->need_queues = false;
   qstat_filter_add_core_attributes(qstat_env);


   /* Loop over all options. Only valid options can be in the
      ppcmdline list. 
   */

   /* make core binding related output to default output */
   qstat_env->full_listing |= QSTAT_DISPLAY_BINDING;

   while (lGetNumberOfElem(*ppcmdline)) {
      if (parse_flag(ppcmdline, "-help",  &alp, &helpflag)) {
         usageshowed = qstat_usage(qstat_env->qselect_mode, stdout, NULL);
         DEXIT;
         SGE_EXIT((void**)&ctx, 0);
         break;
      }

      while (parse_flag(ppcmdline, "-ncb", &alp, &(qstat_env->is_binding_format))) {
         /* disable qstat binding output bit */
         qstat_env->full_listing ^= QSTAT_DISPLAY_BINDING;
         continue;
      }

      while (parse_string(ppcmdline, "-j", &alp, &argstr)) {
         qstat_env->job_info = 1;
         if (argstr) {
            if (*ppljid) {
               lFreeList(ppljid);
            }
            str_list_parse_from_string(ppljid, argstr, ",");
            sge_free(&argstr);
         }
         continue;
      }

      while (parse_flag(ppcmdline, "-xml", &alp, isXML)){
         qstat_filter_add_xml_attributes(qstat_env);
         continue;
      }
      
      /*
      ** Two additional flags only if MORE_INFO is set:
      ** -dj   dump jobs:  displays full global_job_list 
      ** -dq   dump queue: displays full global_queue_list
      */
      if (getenv("MORE_INFO")) {
         while (parse_flag(ppcmdline, "-dj", &alp, &(qstat_env->global_showjobs)))
            ;
         
         while (parse_flag(ppcmdline, "-dq", &alp, &(qstat_env->global_showqueues)))
            ;
      }

      while (parse_flag(ppcmdline, "-ne", &alp, &full)) {
         if (full) {
            qstat_env->full_listing |= QSTAT_DISPLAY_NOEMPTYQ;
            full = 0;
         }
         continue;
      }


      while (parse_flag(ppcmdline, "-f", &alp, &full)) {
         if (full) {
            qstat_env->full_listing |= QSTAT_DISPLAY_FULL;
            full = 0;
         }
         qstat_env->need_queues = true;
         continue;
      }

      while (parse_string(ppcmdline, "-s", &alp, &argstr)) {
         
         if (argstr != NULL) {
            if (build_job_state_filter(qstat_env, argstr, &alp)) {
               if (!usageshowed) {
                  qstat_usage(qstat_env->qselect_mode, stderr, NULL);
               }
               sge_free(&argstr);
               DEXIT;
               return alp;
            }
            sge_free(&argstr);
         }
         continue;
      }

      while (parse_string(ppcmdline, "-explain", &alp, &argstr)) {
         u_long32 filter = QI_AMBIGUOUS | QI_ALARM | QI_SUSPEND_ALARM | QI_ERROR;
         qstat_env->explain_bits = qinstance_state_from_string(argstr, &alp, filter);
         qstat_env->full_listing |= QSTAT_DISPLAY_FULL;
         qstat_env->need_queues = true;
         sge_free(&argstr);
         continue;
      }
       
      while (parse_string(ppcmdline, "-F", &alp, &argstr)) {
         qstat_env->full_listing |= QSTAT_DISPLAY_QRESOURCES|QSTAT_DISPLAY_FULL;
         qstat_env->need_queues = true;
         if (argstr) {
            if (qstat_env->qresource_list) {
               lFreeList(&(qstat_env->qresource_list));
            }
            qstat_env->qresource_list = centry_list_parse_from_string(qstat_env->qresource_list, argstr, false);
            sge_free(&argstr);
         }
         continue;
      }

      while (parse_flag(ppcmdline, "-ext", &alp, &full)) {
         qstat_filter_add_ext_attributes(qstat_env);
         if (full) {
            qstat_env->full_listing |= QSTAT_DISPLAY_EXTENDED;
            full = 0;
         }
         continue;
      }

      if (!qstat_env->qselect_mode ) {
         while (parse_flag(ppcmdline, "-urg", &alp, &full)) {
            qstat_filter_add_urg_attributes(qstat_env); 
            qstat_env->need_queues = true;
            if (full) {
               qstat_env->full_listing |= QSTAT_DISPLAY_URGENCY;
               full = 0;
            }
            continue;
         }
      }

      if (!qstat_env->qselect_mode ) {
         while (parse_flag(ppcmdline, "-pri", &alp, &full)) {
            qstat_filter_add_pri_attributes(qstat_env);
            if (full) {
               qstat_env->full_listing |= QSTAT_DISPLAY_PRIORITY;
               full = 0;
            }
            continue;
         }
      }

      while (parse_flag(ppcmdline, "-r", &alp, &full)) {
         qstat_filter_add_r_attributes(qstat_env);
         if (full) {
            qstat_env->full_listing |= QSTAT_DISPLAY_RESOURCES;
            full = 0;
         }
         continue;
      }

      while (parse_flag(ppcmdline, "-t", &alp, &full)) {
         qstat_filter_add_t_attributes(qstat_env);
         if (full) {
            qstat_env->full_listing |= QSTAT_DISPLAY_TASKS;
            qstat_env->group_opt |= GROUP_NO_PETASK_GROUPS;
            full = 0;
         }
         continue;
      }

      while (parse_string(ppcmdline, "-qs", &alp, &argstr)) {
         u_long32 filter = 0xFFFFFFFF;
         qstat_env->queue_state = qinstance_state_from_string(argstr, &alp, filter);
         qstat_env->need_queues = true;
         sge_free(&argstr);
         continue;
      }

      while (parse_string(ppcmdline, "-l", &alp, &argstr)) {
         qstat_filter_add_l_attributes(qstat_env);
         qstat_env->resource_list = centry_list_parse_from_string(qstat_env->resource_list, argstr, false);
         qstat_env->need_queues = true;
         sge_free(&argstr);
         continue;
      }

      while (parse_multi_stringlist(ppcmdline, "-u", &alp, &(qstat_env->user_list), ST_Type, ST_name)) {
         continue;
      }
      
      while (parse_multi_stringlist(ppcmdline, "-U", &alp, &(qstat_env->queue_user_list), ST_Type, ST_name)) {
         qstat_filter_add_U_attributes(qstat_env);
         qstat_env->need_queues = true;
         continue;
      }   
      
      while (parse_multi_stringlist(ppcmdline, "-pe", &alp, &(qstat_env->peref_list), ST_Type, ST_name)) {
         qstat_filter_add_pe_attributes(qstat_env);
         qstat_env->need_queues = true;
         continue;
      }   

      while (parse_multi_stringlist(ppcmdline, "-q", &alp, &(qstat_env->queueref_list), QR_Type, QR_name)) {
         qstat_filter_add_q_attributes(qstat_env);
         qstat_env->need_queues = true;
         continue;
      }

      while (parse_multi_stringlist(ppcmdline, "-g", &alp, &plstringopt, ST_Type, ST_name)) {
         qstat_env->group_opt |= parse_group_options(plstringopt, &alp);
         qstat_env->need_queues = true;
         lFreeList(&plstringopt);    
         continue;
      }
   }

   if (lGetNumberOfElem(*ppcmdline)) {
     sprintf(str, "%s\n", MSG_PARSE_TOOMANYOPTIONS);
     if (!usageshowed)
        qstat_usage(qstat_env->qselect_mode, stderr, NULL);
     answer_list_add(&alp, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
     DEXIT;
     return alp;
   }

   DEXIT;
   return alp;
}

/* --------------- qstat stdout handler --------------------------------------*/


static int qstat_stdout_init(qstat_handler_t *handler, lList **alpp) 
{
   int ret = 0;
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)sge_malloc(sizeof(qstat_stdout_ctx_t));
   
   DENTER(TOP_LAYER, "qstat_stdout_init");
   
   if (ctx == NULL) {
      answer_list_add(alpp, "malloc of qstat_stdout_ctx failed",
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      ret = -1;
      goto error;
   }
   memset(handler,0, sizeof(qstat_handler_t));
   memset(ctx,0,sizeof(qstat_stdout_ctx_t));
   
   handler->ctx = ctx; 
   
   /* initialze report handler methods */
   handler->report_queue_summary = qstat_stdout_queue_summary;
   handler->report_queue_finished = qstat_stdout_queue_finished;
   handler->report_queue_load_alarm = qstat_stdout_queue_load_alarm;
   handler->report_queue_suspend_alarm = qstat_stdout_queue_suspend_alarm;
   handler->report_queue_message = qstat_stdout_queue_message;
   handler->report_queue_resource = qstat_stdout_queue_resource;
   
   handler->report_pending_jobs_started = qstat_stdout_pending_jobs_started;
   handler->report_finished_jobs_started = qstat_stdout_finished_jobs_started;
   handler->report_error_jobs_started = qstat_stdout_error_jobs_started;
   
   handler->destroy = qstat_stdout_destroy;
   
   if((ret=job_stdout_init(&(handler->job_handler), alpp))) {
      DPRINTF(("job_stdout_init failed\n"));
      goto error;
   }
   
   handler->job_handler.ctx = ctx;
   
   /* internal context initializing */
   ctx->header_printed = false;
   ctx->job_header_printed = false;

error:
   if (ret != 0 ) {
      if(ctx != NULL) {
         sge_free(&ctx);
      }
   }
   DEXIT;
   return ret;
}

static int qstat_stdout_destroy(qstat_handler_t *handler) 
{
   DENTER(TOP_LAYER, "qstat_stdout_destroy");

   if (handler->ctx) {
      sge_dstring_free(&(((qstat_stdout_ctx_t*)(handler->ctx))->last_queue_name));
      sge_free(&(handler->ctx));
   }

   DEXIT;
   return 0;
}


static int job_stdout_init(job_handler_t *handler, lList** alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_init");

   if (handler == NULL) {
      DEXIT;
      return -1;
   }

   handler->report_job = job_stdout_job;
   handler->report_sub_tasks_started = job_stdout_sub_tasks_started;
   handler->report_sub_task = job_stdout_sub_task;
   handler->report_sub_tasks_finished = job_stdout_sub_tasks_finished;
   
   handler->report_requested_pe = job_stdout_requested_pe;
   handler->report_granted_pe = job_stdout_granted_pe;
   handler->report_additional_info = job_stdout_additional_info;
   
   handler->report_request = job_stdout_request;
   
   handler->report_hard_resources_started = job_stdout_hard_resources_started;
   handler->report_hard_resource = job_stdout_hard_resource;
   handler->report_hard_resources_finished = job_stdout_hard_resources_finished;
   
   handler->report_soft_resources_started = job_stdout_soft_resources_started;
   handler->report_soft_resource = job_stdout_soft_resource;
   handler->report_soft_resources_finished = job_stdout_soft_resources_finished;
   
   handler->report_hard_requested_queues_started = job_stdout_hard_requested_queues_started;
   handler->report_hard_requested_queue = job_stdout_hard_requested_queue;
   handler->report_hard_requested_queues_finished = job_stdout_hard_requested_queues_finished;
   
   handler->report_soft_requested_queues_started = job_stdout_soft_requested_queues_started;
   handler->report_soft_requested_queue = job_stdout_soft_requested_queue;
   handler->report_soft_requested_queues_finished = job_stdout_soft_requested_queues_finished;
   
   handler->report_master_hard_requested_queues_started = job_stdout_master_hard_requested_queues_started;
   handler->report_master_hard_requested_queue = job_stdout_master_hard_request_queue;
   handler->report_master_hard_requested_queues_finished = job_stdout_master_hard_requested_queues_finished;
   
   handler->report_predecessors_requested_started = job_stdout_predecessors_requested_started;
   handler->report_predecessor_requested = job_stdout_predecessor_requested;
   handler->report_predecessors_requested_finished = job_stdout_predecessors_requested_finished;
   
   handler->report_predecessors_started = job_stdout_predecessors_started;
   handler->report_predecessor = job_stdout_predecessor;
   handler->report_predecessors_finished = job_stdout_predecessors_finished;

   handler->report_ad_predecessors_requested_started = job_stdout_ad_predecessors_requested_started;
   handler->report_ad_predecessor_requested = job_stdout_ad_predecessor_requested;
   handler->report_ad_predecessors_requested_finished = job_stdout_ad_predecessors_requested_finished;

   handler->report_ad_predecessors_started = job_stdout_ad_predecessors_started;
   handler->report_ad_predecessor = job_stdout_ad_predecessor;
   handler->report_ad_predecessors_finished = job_stdout_ad_predecessors_finished;

   handler->report_binding_started = job_stdout_binding_started;
   handler->report_binding = job_stdout_binding;
   handler->report_binding_finished = job_stdout_binding_finished;

   DEXIT;
   return 0;
}

static char hashes[] = "##############################################################################################################";

/* regular output */
static char jhul1[] = "---------------------------------------------------------------------------------------------";
/* -g t */
static char jhul2[] = "-";
/* -ext */
static char jhul3[] = "-------------------------------------------------------------------------------";
/* -t */
static char jhul4[] = "-----------------------------------------------------";
/* -urg */
static char jhul5[] = "----------------------------------------------------------------";
/* -pri */
static char jhul6[] = "-----------------------------------";

#define OPTI_PRINT8(value) \
   if (value > 99999999 ) \
      printf("%8.3g ", value); \
   else  \
      printf("%8.0f ", value)

static int job_stdout_job(job_handler_t* handler, u_long32 jid, job_summary_t *summary, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;
   const char* indent = "";
   int sge_urg, sge_pri, sge_ext, sge_time, tsk_ext;
   bool print_job_id;
   dstring ds = DSTRING_INIT;
   
   DENTER(TOP_LAYER, "job_stdout_job");
   
   sge_ext = ((qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) == QSTAT_DISPLAY_EXTENDED);
   tsk_ext = (qstat_env->full_listing & QSTAT_DISPLAY_TASKS);
   sge_urg = (qstat_env->full_listing & QSTAT_DISPLAY_URGENCY);
   sge_pri = (qstat_env->full_listing & QSTAT_DISPLAY_PRIORITY);
   sge_time = !sge_ext;
   sge_time = sge_time | tsk_ext | sge_urg | sge_pri;

   if ((qstat_env->full_listing & QSTAT_DISPLAY_FULL) == QSTAT_DISPLAY_FULL) {
      ctx->job_header_printed = true;
   }

#if 0
   if (ctx->last_job_id != jid) {
     print_job_id = true;
   } else if ( summary->queue != NULL &&
               strcmp(sge_dstring_get_string(&(ctx->last_queue_name)), summary->queue) != 0 ) {
     print_job_id = true;
   } else {
     print_job_id = false;
   }    
#else
   print_job_id = summary->print_jobid;
#endif
   
   ctx->last_job_id = jid;
   if (summary->queue == NULL) {
      sge_dstring_clear(&(ctx->last_queue_name));
   } else {
      sge_dstring_copy_string(&(ctx->last_queue_name), summary->queue); 
   }
   
   if (!ctx->job_header_printed) {
      int i;
      int line_length = qstat_env->longest_queue_length-10+1;
      char * seperator = sge_malloc(line_length);
      const char *part1 = "%s%-7.7s %s %s%s%s%s%s %-10.10s %-12.12s %s%-5.5s %s%s%s%s%s%s%s%s%s%-";
      const char *part3 = ".";
	   const char *part5 = "s %s %s%s%s%s%s%s";
		char *part6 = sge_malloc(strlen(part1) + strlen(part3) + strlen(part5) + 20);
      
      ctx->job_header_printed = true;
      
      for (i=0; i<line_length; i++) {
         seperator[i] = '-';
      }
      seperator[line_length-1] = '\0';
      sprintf(part6, "%s%d%s%d%s", part1, qstat_env->longest_queue_length, part3, qstat_env->longest_queue_length, part5);
   
      printf(part6,
               indent,
               "job-ID",
               "prior ",
            (sge_pri||sge_urg)?" nurg   ":"",
            sge_pri?" npprior":"",
            (sge_pri||sge_ext)?" ntckts ":"",
            sge_urg?" urg      rrcontr  wtcontr  dlcontr ":"",
            sge_pri?"  ppri":"",
               "name",
               "user",
            sge_ext?"project          department ":"",
               "state",
            sge_time?"submit/start at     ":"",
            sge_urg?" deadline           " : "",
            sge_ext?USAGE_ATTR_CPU "        " USAGE_ATTR_MEM "     " USAGE_ATTR_IO "      " : "",
            sge_ext?"tckts ":"",
            sge_ext?"ovrts ":"",
            sge_ext?"otckt ":"",
            sge_ext?"ftckt ":"",
            sge_ext?"stckt ":"",
            sge_ext?"share ":"",
               "queue",
            (qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)?"master":"slots",
               "ja-task-ID ", 
            tsk_ext?"task-ID ":"",
            tsk_ext?"state ":"",
            tsk_ext?USAGE_ATTR_CPU "        " USAGE_ATTR_MEM "     " USAGE_ATTR_IO "      " : "",
            tsk_ext?"stat ":"",
            tsk_ext?"failed ":"" );

      printf("\n%s%s%s%s%s%s%s%s\n", indent, 
            jhul1, 
            seperator,
            (qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)?jhul2:"",
            sge_ext ? jhul3 : "", 
            tsk_ext ? jhul4 : "",
            sge_urg ? jhul5 : "",
            sge_pri ? jhul6 : "");
            
      sge_free(&part6);
      sge_free(&seperator);               
   }
   
   if (summary->is_zombie) {
      sge_printf_header(qstat_env->full_listing & 
                        (QSTAT_DISPLAY_ZOMBIES | QSTAT_DISPLAY_FULL), 
                        sge_ext);
   }
   
   /* job id */
   /* job number / ja task id */
   if (print_job_id) {
      printf("%7d ", (int)jid); 
   } else {
      printf("        ");
   }
   
   if (print_job_id) {
      printf("%7.5f ", summary->nprior); /* nprio 0.0 - 1.0 */
   } else {
      printf("        ");
   }
   if (sge_pri || sge_urg) {
      if (print_job_id)
         printf("%7.5f ", summary->nurg); /* nurg 0.0 - 1.0 */
      else
         printf("        ");
   }
   if (sge_pri) {
      if (print_job_id)
         printf("%7.5f ", summary->nppri); /* nppri 0.0 - 1.0 */
      else
         printf("        ");
   }
   if (sge_pri || sge_ext) {
      if (print_job_id)
         printf("%7.5f ", summary->ntckts); /* ntix 0.0 - 1.0 */
      else
         printf("        ");
   }

   if (sge_urg) {
      if (print_job_id) {
         OPTI_PRINT8(summary->urg);
         OPTI_PRINT8(summary->rrcontr);
         OPTI_PRINT8(summary->wtcontr);
         OPTI_PRINT8(summary->dlcontr);
      } else {
         printf("         "
                "         "
                "         "
                "         ");
      }
   } 

   if (sge_pri) {
      if (print_job_id) {
         printf("%5d ", (int)summary->priority); 
      } else {
         printf("         "
                "         ");
      }
   }

   if (print_job_id) {
      /* job name */
      printf("%-10.10s ", summary->name); 

      /* job owner */
      printf("%-12.12s ", summary->user); 
   } else {
      printf("           "); 
      printf("             "); 
   }

   if (sge_ext) {
      if (print_job_id) {
         /* job project */
         printf("%-16.16s ", summary->project?summary->project:"NA"); 
         /* job department */
         printf("%-10.10s ", summary->department?summary->department:"NA"); 
      } else {
         printf("                 "); 
         printf("           "); 
      }
   }

   if (print_job_id) {
      printf("%-5.5s ", summary->state); 
   } else {
      printf("      "); 
   }

   if (sge_time) {
      if (print_job_id) {
         /* start/submit time */
         if (summary->is_running) {
            printf("%s ", sge_ctime(summary->start_time, &ds));
         } else {
            printf("%s ", sge_ctime(summary->submit_time, &ds));
         }
      } else {
         printf("                    "); 
      }
   }

   /* deadline time */
   if (sge_urg) {
      if (print_job_id) { 
         if (!summary->deadline )
            printf("                    ");
         else
            printf("%s ", sge_ctime(summary->deadline, &ds));
      } else {
         printf("                    "); 
      }
   }

   if (sge_ext) {
      /* scaled cpu usage */
      if (!summary->has_cpu_usage) 
         printf("%-10.10s ", summary->is_running?"NA":""); 
      else {
         int secs, minutes, hours, days;

         secs = summary->cpu_usage;

         days    = secs/(60*60*24);
         secs   -= days*(60*60*24);

         hours   = secs/(60*60);
         secs   -= hours*(60*60);

         minutes = secs/60;
         secs   -= minutes*60;
      
         printf("%d:%2.2d:%2.2d:%2.2d ", days, hours, minutes, secs); 
      } 
      /* scaled mem usage */
      if (!summary->has_mem_usage) 
         printf("%-7.7s ", summary->is_running?"NA":""); 
      else
         printf("%-5.5f ", summary->mem_usage); 
  
      /* scaled io usage */
      if (!summary->has_io_usage) 
         printf("%-7.7s ", summary->is_running?"NA":""); 
      else
         printf("%-5.5f ", summary->io_usage); 

      /* report jobs dynamic scheduling attributes */
      /* only scheduled have these attribute */
      /* Pending jobs can also have tickets */
      if (summary->is_zombie) {
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
      } else {
         if (sge_ext || summary->is_queue_assigned) {
            printf("%5ld ", (unsigned long)summary->tickets),
            printf("%5ld ", (unsigned long)summary->override_tickets);
            printf("%5ld ", (unsigned long)summary->otickets);
            printf("%5ld ", (unsigned long)summary->ftickets);
            printf("%5ld ", (unsigned long)summary->stickets);
            printf("%-5.2f ", summary->share); 
         } else {
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
         }
      }
   }
   /* if not full listing we need the queue's name in each line */
   if (!(qstat_env->full_listing & QSTAT_DISPLAY_FULL)) {
      char temp[20];
	   sprintf(temp,"%%-%d.%ds ", qstat_env->longest_queue_length, qstat_env->longest_queue_length);
      printf(temp, summary->queue?summary->queue:"");
   }
   
   if ((qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)) {
      /* MASTER/SLAVE information needed only to show parallel job distribution */
      if (summary->master)
         printf("%-7.6s", summary->master);
      else
         printf("       ");
   } else {
      /* job slots requested/granted */
      printf("%5d ", (int)summary->slots);
   }
   
   if (summary->task_id && summary->is_array)
      printf("%s", summary->task_id); 
   else
      printf("       ");
   
   if (!tsk_ext) {
      putchar('\n');
   }   
   
   sge_dstring_free(&ds);
   
   DEXIT;
   return 0;
}

static int job_stdout_sub_tasks_started(job_handler_t* handler, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_sub_tasks_started");

   ctx->sub_task_count = 0;
   
   DEXIT;
   return 0;
}


static int job_stdout_sub_task(job_handler_t* handler, task_summary_t *summary, lList **alpp)
{
   bool indent = false;
   DENTER(TOP_LAYER, "job_stdout_sub_task");
   
   printf("   %s%-12s ", indent ? QSTAT_INDENT2: "", (summary->task_id == NULL)? "" : summary->task_id );
   printf("%-5.5s ", summary->state); 
   
   if (summary->has_cpu_usage) {
      dstring resource_string = DSTRING_INIT;

      double_print_time_to_dstring(summary->cpu_usage,
                                   &resource_string);
      printf("%s ", sge_dstring_get_string(&resource_string));
      sge_dstring_free(&resource_string);
   } else {
      printf("%-10.10s ", summary->is_running?"NA":"");
   }
   if (summary->has_mem_usage) { 
      printf("%-5.5f ", summary->mem_usage); 
   } else {
      printf("%-7.7s ", summary->is_running?"NA":"");
   }
   
   /* scaled io usage */
   if (summary->has_io_usage) { 
      printf("%-5.5f ", summary->io_usage); 
   } else {
      printf("%-7.7s ", summary->is_running?"NA":"");
   }
   
   if (summary->has_exit_status) {
      printf("%-4d", (int)summary->exit_status);
   }

   DEXIT;
   return 0;
}

static int job_stdout_sub_tasks_finished(job_handler_t* handler, lList **alpp)
{
   DENTER(TOP_LAYER, "job_stdout_sub_tasks_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_requested_pe(job_handler_t *handler, const char* pe_name, const char* pe_range, lList **alpp) 
{
   const char* name = "Requested PE";
   int len = MAX(1,17 - strlen(name));
   DENTER(TOP_LAYER, "job_stdout_requested_pe");
   printf("%s%s:%*s%s %s\n", QSTAT_INDENT, name, len, " ", pe_name, pe_range);
   DEXIT;
   return 0;
   
}

static int job_stdout_granted_pe(job_handler_t *handler, const char* pe_name, int pe_slots, lList **alpp) 
{
   const char* name = "Granted PE";
   int len = MAX(1,17 - strlen(name));   
   DENTER(TOP_LAYER, "job_stdout_granted_pe");
   printf("%s%s:%*s%s %d\n", QSTAT_INDENT, name, len, " ", pe_name, pe_slots);
   DEXIT;
   return 0;
}

static int job_stdout_additional_info(job_handler_t* handler, job_additional_info_t name, const char* value, lList **alpp)
{
   dstring ds = DSTRING_INIT;
   
   DENTER(TOP_LAYER, "job_stdout_additional_info");

   switch(name) {
      case CHECKPOINT_ENV: sge_dstring_copy_string(&ds, "Checkpoint Env."); break;
      case MASTER_QUEUE:   sge_dstring_copy_string(&ds, "Master Queue"); break;
      case FULL_JOB_NAME:  sge_dstring_copy_string(&ds, "Full jobname"); break;
      default:
           DPRINTF(("Unknown additional info(%d)\n", name));
           abort();
   }
   {
      const char* name_str = sge_dstring_get_string(&ds);
      int len = MAX(1,17 - strlen(name_str));
      printf("%s%s:%*s%s\n", QSTAT_INDENT, name_str, len, " ",
             value == NULL ? "" : value);
   }
   sge_dstring_free(&ds);
   DEXIT;
   return 0;
}

static int job_stdout_request(job_handler_t* handler, const char* name, const char* value, lList **alpp)
{
   DENTER(TOP_LAYER, "job_stdout_request");
   printf("%s%s=%s (default)\n", QSTAT_INDENT, name, value);  
   DEXIT;
   return 0;
}

static int job_stdout_hard_requested_queues_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_hard_requested_queues_started");

   printf(QSTAT_INDENT "Hard requested queues: ");
   ctx->hard_requested_queue_count = 0;

   DEXIT;
   return 0;
}

static int job_stdout_hard_requested_queue(job_handler_t* handler, const char* qname, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_hard_requested_queue");

   if(ctx->hard_requested_queue_count > 0 ) {
      printf(", %s", qname);
   } else {
      printf("%s", qname);
   }
   ctx->hard_requested_queue_count++;
   DEXIT;
   return 0;
}

static int job_stdout_hard_requested_queues_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_hard_requested_queues_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_soft_requested_queues_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_soft_requested_queues_started");
   
   printf(QSTAT_INDENT "Soft requested queues: ");
   ctx->soft_requested_queue_count = 0;

   DEXIT;
   return 0;
}

static int job_stdout_soft_requested_queue(job_handler_t* handler, const char* qname, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_soft_requested_queue");
   
   if(ctx->soft_requested_queue_count > 0 ) {
      printf(", %s", qname);
   } else {
      printf("%s", qname);
   }
   ctx->soft_requested_queue_count++;

   DEXIT;
   return 0;
}

static int job_stdout_soft_requested_queues_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_soft_requested_queues_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_master_hard_requested_queues_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_master_hard_requested_queues_started");

   printf(QSTAT_INDENT "Master task hard requested queues: ");
   ctx->master_hard_requested_queue_count = 0;

   DEXIT;
   return 0;
}

static int job_stdout_master_hard_request_queue(job_handler_t* handler, const char* qname, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_master_hard_request_queue");

   if(ctx->master_hard_requested_queue_count > 0 ) {
      printf(", %s", qname);
   } else {
      printf("%s", qname);
   }
   ctx->master_hard_requested_queue_count++;
   
   DEXIT;
   return 0;
}

static int job_stdout_master_hard_requested_queues_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_master_hard_requested_queues_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_hard_resources_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_hard_resources_started");

   ctx->hard_resource_count = 0;
   printf("       Hard Resources:   ");

   DEXIT;
   return 0;
}

static int job_stdout_hard_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_hard_resource");

   if(ctx->hard_resource_count > 0 ) {
      printf("                         ");
   }
   printf("%s=%s (%f)\n", name, value == NULL ? "" : value, uc);
   ctx->hard_resource_count++;

   DEXIT;
   return 0;
}

static int job_stdout_hard_resources_finished(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_hard_resources_finished");
   
   if (ctx->hard_resource_count == 0) {
      putchar('\n');
   }

   DEXIT;
   return 0;
}

static int job_stdout_soft_resources_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_soft_resources_started");

   ctx->soft_resource_count = 0;
   printf("       Soft Resources:   ");

   DEXIT;
   return 0;
}

static int job_stdout_soft_resource(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_soft_resource");

   if (ctx->soft_resource_count > 0 ) {
      printf("                         ");
   }
   printf("%s=%s\n", name, value == NULL ? "" : value);
   ctx->soft_resource_count++;

   DEXIT;
   return 0;
}

static int job_stdout_soft_resources_finished(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_soft_resources_finished");

   if (ctx->soft_resource_count == 0) {
      putchar('\n');
   }

   DEXIT;
   return 0;
}

static int job_stdout_predecessors_requested_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_predecessors_requested_started");

   ctx->predecessor_requested_count = 0;
   printf("       Predecessor Jobs (request): ");

   DEXIT;
   return 0;
}

static int job_stdout_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_predecessor_requested");

   if(ctx->predecessor_requested_count > 0 ) {
      printf(", %s", name);
   } else {
      printf("%s", name);
   }
   ctx->predecessor_requested_count++;
   
   DEXIT;
   return 0;
}

static int job_stdout_predecessors_requested_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_predecessors_requested_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_predecessors_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_predecessors_started");

   ctx->predecessor_count = 0;
   printf("       Predecessor Jobs: ");

   DEXIT;
   return 0;
}

static int job_stdout_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_predecessor");

   if (ctx->predecessor_count > 0 ) {
      printf(", "sge_u32, jid);
   } else {
      printf(sge_u32, jid);
   }
   ctx->predecessor_count++;

   DEXIT;
   return 0;
}

static int job_stdout_predecessors_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_predecessors_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessors_requested_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_ad_predecessors_requested_started");

   ctx->ad_predecessor_requested_count = 0;
   printf("       Predecessor Array Jobs (request): ");

   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessor_requested(job_handler_t* handler, const char* name, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_ad_predecessor_requested");

   if(ctx->ad_predecessor_requested_count > 0) {
      printf(", %s", name);
   } else {
      printf("%s", name);
   }
   ctx->ad_predecessor_requested_count++;
   
   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessors_requested_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_ad_predecessors_requested_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessors_started(job_handler_t* handler, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_ad_predecessors_started");

   ctx->ad_predecessor_count = 0;
   printf("       Predecessor Array Jobs: ");

   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessor(job_handler_t* handler, u_long32 jid, lList **alpp)
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;

   DENTER(TOP_LAYER, "job_stdout_ad_predecessor");

   if (ctx->ad_predecessor_count > 0) {
      printf(", "sge_u32, jid);
   } else {
      printf(sge_u32, jid);
   }
   ctx->ad_predecessor_count++;

   DEXIT;
   return 0;
}

static int job_stdout_ad_predecessors_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_ad_predecessors_finished");
   putchar('\n');
   DEXIT;
   return 0;
}

static int job_stdout_binding_started(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_binding_started");

   printf("       Binding:          ");

   DEXIT;
   return 0;
}

static int job_stdout_binding(job_handler_t *handler, const char* binding, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_binding");

   printf("%s", binding);

   DEXIT;
   return 0;
}

static int job_stdout_binding_finished(job_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "job_stdout_binding_finished");
   
   putchar('\n');

   DEXIT;
   return 0;
}

static int qstat_stdout_queue_summary(qstat_handler_t* handler, const char* qname, queue_summary_t *summary, lList **alpp) 
{
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;
   qstat_env_t *qstat_env = handler->qstat_env;
   int sge_ext = qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED;
   char to_print[80];
   
   DENTER(TOP_LAYER, "qstat_stdout_queue_summary");

   if (ctx->header_printed == false) {
      char temp[20];
      ctx->header_printed = true;
      
      sprintf(temp, "%%-%d.%ds", qstat_env->longest_queue_length, qstat_env->longest_queue_length);

      printf(temp,MSG_QSTAT_PRT_QUEUENAME); 
      
      printf(" %-5.5s %-14.14s %-8.8s %-13.13s %s\n", 
            MSG_QSTAT_PRT_QTYPE, 
            MSG_QSTAT_PRT_RESVUSEDTOT,
            summary->load_avg_str,
            LOAD_ATTR_ARCH,
            MSG_QSTAT_PRT_STATES);
   }
   
   printf("---------------------------------------------------------------------------------%s", 
      sge_ext?"------------------------------------------------------------------------------------------------------------":"");
   {
      int i;
      for(i=0; i< qstat_env->longest_queue_length - 30; i++)
         printf("-");
      printf("\n");
   }

   {
      char temp[20];
      sprintf(temp, "%%-%d.%ds ", qstat_env->longest_queue_length, qstat_env->longest_queue_length);
      printf(temp, qname);
   }

   printf("%-5.5s ", summary->queue_type); 

   /* number of used/total slots */
   sprintf(to_print, "%d/%d/%d ", (int)summary->resv_slots, (int)summary->used_slots, (int)summary->total_slots); 
   printf("%-14.14s ", to_print);   

   /* load avg */
   if (!summary->has_load_value) {
      if (summary->has_load_value_from_object) {
         sprintf(to_print, "%2.2f ", summary->load_avg);
      } else {
         sprintf(to_print, "---  ");
      }
   } else {
      sprintf(to_print, "-NA- ");
   }
   
   printf("%-8.8s ", to_print);   

   /* arch */
   if (summary->arch != NULL) {
      sprintf(to_print, "%s ", summary->arch);
   } else {
      sprintf(to_print, "-NA- ");
   }
   printf("%-13.13s ", to_print);   
   printf("%s", summary->state ? summary->state : "NA"); 

   printf("\n");
   
   DRETURN(0);
}

static int qstat_stdout_queue_load_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp) 
{
   DENTER(TOP_LAYER, "qstat_stdout_queue_load_alarm");
   printf("\t%s\n", reason != NULL ? reason : "no alarm reason given");
   DEXIT;
   return 0;
}

static int qstat_stdout_queue_suspend_alarm(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp) 
{
   DENTER(TOP_LAYER, "qstat_stdout_queue_suspend_alarm");
   printf("\t%s\n", reason != NULL ? reason : "no alarm reason given");
   DEXIT;
   return 0;
}

static int qstat_stdout_queue_message(qstat_handler_t* handler, const char* qname, const char *message, lList **alpp) 
{
   DENTER(TOP_LAYER, "qstat_stdout_queue_message");
   printf("\t%s\n", message != NULL ? message : "no queue message given");
   DEXIT;
   return 0;
}


static int qstat_stdout_queue_finished(qstat_handler_t* handler, const char *qname, lList** alpp) 
{
   DENTER(TOP_LAYER, "qstat_stdout_queue_finished");
/*    printf("\n"); */
   DEXIT;
   return 0;
}

static int qstat_stdout_queue_resource(qstat_handler_t* handler, const char* dom, 
                                       const char* name, const char* value, lList **alpp) 
{
   DENTER(TOP_LAYER, "qstat_stdout_queue_resource");
   printf("\t%s:%s=%s\n", dom, name, value);
   DEXIT;
   return 0;
}

static int qstat_stdout_pending_jobs_started(qstat_handler_t *handler, lList **alpp) 
{
   qstat_env_t *qstat_env = handler->qstat_env;
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;
   
   DENTER(TOP_LAYER, "qstat_stdout_pending_jobs_started");
   
   ctx->last_job_id = 0;
   sge_printf_header((qstat_env->full_listing & QSTAT_DISPLAY_FULL) |
                     (qstat_env->full_listing & QSTAT_DISPLAY_PENDING), 
                     (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) == QSTAT_DISPLAY_EXTENDED);
   DEXIT;
   return 0;
}

static int qstat_stdout_finished_jobs_started(qstat_handler_t *handler, lList **alpp) 
{
   qstat_env_t *qstat_env = handler->qstat_env;
   qstat_stdout_ctx_t *ctx = (qstat_stdout_ctx_t*)handler->ctx;
   int sge_ext = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED);
   
   DENTER(TOP_LAYER,"qstat_stdout_finished_jobs_started");
   
   ctx->last_job_id = 0;

   printf("\n################################################################################%s\n", sge_ext?hashes:"");
   printf("%s\n", MSG_QSTAT_PRT_JOBSWAITINGFORACCOUNTING);
   printf(  "################################################################################%s\n", sge_ext?hashes:"");

   DEXIT;
   return 0;
}

static int qstat_stdout_error_jobs_started(qstat_handler_t *handler, lList **alpp) 
{
   qstat_env_t *qstat_env = handler->qstat_env;
   int sge_ext = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED);
   
   DENTER(TOP_LAYER,"qstat_stdout_error_jobs_started");
   
   printf("\n################################################################################%s\n", sge_ext?hashes:"");
   printf("%s\n", MSG_QSTAT_PRT_ERRORJOBS);
   printf(  "################################################################################%s\n", sge_ext?hashes:"");

   DEXIT;
   return 0;
}

/* --------------- Cluster Queue Summary To Stdout Handler -------------------*/

static int cqueue_summary_stdout_init(cqueue_summary_handler_t *handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "cqueue_summary_stdout_init");

   memset(handler, 0, sizeof(cqueue_summary_handler_t));
   
   handler->report_started = cqueue_summary_stdout_report_started;
   handler->report_cqueue = cqueue_summary_stdout_report_cqueue;

   DEXIT;
   return 0;
}


static int cqueue_summary_stdout_report_started(cqueue_summary_handler_t *handler, lList **alpp) 
{
   int i;
   qstat_env_t *qstat_env = handler->qstat_env;
   
   bool show_states = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) ? true : false;
   
   char queue_def[50];
   char fields[] = "%7s %6s %6s %6s %6s %6s %6s ";

   DENTER(TOP_LAYER, "cqueue_summary_stdout_report_started");

   sprintf(queue_def, "%%-%d.%ds %s ", qstat_env->longest_queue_length, qstat_env->longest_queue_length, fields);                         
   printf( queue_def,
          "CLUSTER QUEUE", "CQLOAD", 
          "USED", "RES", "AVAIL", "TOTAL", "aoACDS", "cdsuE");
   if (show_states) {
      printf("%5s %5s %5s %5s %5s %5s %5s %5s %5s %5s %5s", 
             "s", "A", "S", "C", "u", "a", "d", "D", "c", "o", "E");
   }
   printf("\n");

   printf("--------------------");
   printf("--------------------");
   printf("--------------------");
   printf("--------------------");
   if (show_states) {
      printf("--------------------");
      printf("--------------------");
      printf("--------------------");
      printf("------");
   }
   for(i=0; i< qstat_env->longest_queue_length - 36; i++) {
      printf("-");
   }   
   printf("\n");

   DEXIT;
   return 0;
}


static int cqueue_summary_stdout_report_cqueue(cqueue_summary_handler_t *handler, 
                                               const char* cqname, cqueue_summary_t *summary,
                                               lList **alpp) 
{
   qstat_env_t *qstat_env = handler->qstat_env;
   bool show_states = (qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) ? true : false;
   char queue_def[50];

   DENTER(TOP_LAYER, "cqueue_summary_stdout_report_cqueue");

   sprintf(queue_def, "%%-%d.%ds ", qstat_env->longest_queue_length, qstat_env->longest_queue_length);

   printf(queue_def, cqname);

   if (summary->is_load_available) {
      printf("%7.2f ", summary->load);
   } else {
      printf("%7s ", "-NA-");
   }
   
   printf("%6d ", (int)summary->used);
   printf("%6d ", (int)summary->resv);
   printf("%6d ", (int)summary->available);
   printf("%6d ", (int)summary->total);
   printf("%6d ", (int)summary->temp_disabled);
   printf("%6d ", (int)summary->manual_intervention);
   if (show_states) {
      printf("%5d ", (int)summary->suspend_manual);
      printf("%5d ", (int)summary->suspend_threshold);
      printf("%5d ", (int)summary->suspend_on_subordinate);
      printf("%5d ", (int)summary->suspend_calendar);
      printf("%5d ", (int)summary->unknown);
      printf("%5d ", (int)summary->load_alarm);
      printf("%5d ", (int)summary->disabled_manual);
      printf("%5d ", (int)summary->disabled_calendar);
      printf("%5d ", (int)summary->ambiguous);
      printf("%5d ", (int)summary->orphaned);
      printf("%5d ", (int)summary->error);
   }
   printf("\n");

   DEXIT;
   return 0;
}



/* ----------------------- qselect stdout handler --------------------------- */

static void qselect_stdout_init(qselect_handler_t* handler, lList **alpp) 
{
   DENTER(TOP_LAYER, "qselect_stdout_init");

   memset(handler, 0, sizeof(qselect_handler_t));
   handler->report_queue = qselect_stdout_report_queue;

   DEXIT;
}

static int qselect_stdout_report_queue(qselect_handler_t* handler, const char* qname, lList **alpp) 
{
   DENTER(TOP_LAYER, "qselect_stdout_report_queue");

   printf("%s\n", qname);

   DEXIT;
   return 0;
}



/*
** qstat_show_job
** displays information about a given job
** to be extended
**
** returns 0 on success, non-zero on failure
*/
static int 
qstat_show_job(sge_gdi_ctx_class_t *ctx, lList *jid_list, u_long32 isXML, qstat_env_t *qstat_env) {
   lListElem *j_elem = 0;
   lList* jlp = NULL;
   lList* ilp = NULL;
   lListElem* aep = NULL;
   lCondition *where = NULL, *newcp = NULL;
   lEnumeration* what = NULL;
   lList* alp = NULL;
   bool schedd_info = true;
   bool jobs_exist = true;
   lListElem* mes;
   lListElem *tmpElem;

   DENTER(TOP_LAYER, "qstat_show_job");

   /* get job scheduling information */
   what = lWhat("%T(ALL)", SME_Type);
   alp = ctx->gdi(ctx, SGE_SME_LIST, SGE_GDI_GET, &ilp, NULL, what, false);
   lFreeWhat(&what);

   if (!isXML){
      for_each(aep, alp) {
         if (lGetUlong(aep, AN_status) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            schedd_info = false;
         }
      }
   }
   lFreeList(&alp);

   /* build 'where' for all jobs */
   where = NULL;
   for_each(j_elem, jid_list) {
      const char *job_name = lGetString(j_elem, ST_name);

      if (isdigit(job_name[0])) {
         u_long32 jid = atol(lGetString(j_elem, ST_name));
         newcp = lWhere("%T(%I==%u)", JB_Type, JB_job_number, jid);
      } else {
         newcp = lWhere("%T(%I p= %s)", JB_Type, JB_job_name, job_name);
      }
      if (newcp) {
         if (!where) {
            where = newcp;
         } else {
            where = lOrWhere(where, newcp);
         }
      }
   }
   what = lWhat("%T(%I%I%I%I%I%I%I%I%I%I%I%I%I%I%I->%T%I%I%I%I%I%I->%T%I%I%I%I->%T(%I%I%I%I%I%I%I)"
            "%I%I%I%I%I%I%I->%T(%I)%I%I->%T(%I)%I%I%I%I%I%I%I%I%I%I%I%I%I%I%I->%T%I%I%I%I%I%I%I%I%I%I%I%I)",
            JB_Type, JB_job_number, JB_ar, JB_exec_file, JB_submission_time, 
            JB_owner, JB_uid, JB_group, JB_gid, JB_account, JB_merge_stderr, 
            JB_mail_list, JB_project, JB_notify, JB_job_name, 
            JB_stdout_path_list, 
            /**/
            PN_Type, JB_jobshare, JB_hard_resource_list, JB_soft_resource_list,
            JB_hard_queue_list, JB_soft_queue_list, JB_shell_list, 
            /**/
            PN_Type, JB_env_list, JB_job_args, JB_script_file, JB_ja_tasks,
            /**/
            JAT_Type, JAT_status, JAT_task_number, JAT_scaled_usage_list,
            JAT_task_list, JAT_message_list, JAT_start_time, JAT_ntix,
            /**/
            JB_context, JB_cwd, JB_stderr_path_list, JB_jid_predecessor_list,
            JB_ja_ad_predecessor_list, JB_jid_request_list,
            JB_ja_ad_request_list,
            /**/
            JRE_Type, JRE_job_number, 
            /**/
            JB_jid_successor_list,
            JB_ja_ad_successor_list,
            /**/
            JRE_Type, JRE_job_number, 
            /**/
            JB_deadline, JB_execution_time, JB_checkpoint_name, 
            JB_checkpoint_attr, JB_checkpoint_interval, JB_directive_prefix, 
            JB_reserve, JB_mail_options, JB_stdin_path_list, JB_priority, 
            JB_restart, JB_verify, JB_master_hard_queue_list, JB_script_size, 
            JB_pe, 
            RN_Type, JB_pe_range, JB_jid_request_list, 
            JB_verify_suitable_queues, JB_soft_wallclock_gmt,
            JB_hard_wallclock_gmt, JB_override_tickets, JB_version,
            JB_ja_structure, JB_type, JB_binding, JB_ja_task_concurrency,
            JB_nurg);
   /* get job list */
   alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &jlp, where, what, false);
   lFreeWhere(&where);
   lFreeWhat(&what);

   if (isXML) {
      /* filter the message list to contain only jobs that have been requested.
         First remove all entries in the job_number_list that are not in the
         jbList. Then remove all entries (job_number_list, message_number and 
         message) from the message_list that have no jobs in them. 
      */
      for_each (tmpElem, ilp) {
         lList *msgList = NULL;
         lListElem *msgElem = NULL;
         lListElem *tmp_msgElem = NULL;
         msgList = lGetList(tmpElem, SME_message_list);
         msgElem = lFirst(msgList);
         while (msgElem) {            
            lList *jbList = NULL;
            lListElem *jbElem = NULL;
            lListElem *tmp_jbElem = NULL;
            
            tmp_msgElem = lNext(msgElem);
            jbList = lGetList(msgElem, MES_job_number_list);
            jbElem = lFirst(jbList);
            
            while (jbElem) {
               tmp_jbElem = lNext(jbElem);
               if (lGetElemUlong(jlp, JB_job_number, lGetUlong(jbElem, ULNG_value)) == NULL) {
                  lRemoveElem(jbList, &jbElem);
               }
               jbElem = tmp_jbElem;
            }
            if (lGetNumberOfElem(lGetList(msgElem, MES_job_number_list)) == 0) {
               lRemoveElem(msgList, &msgElem);
            }
            msgElem = tmp_msgElem;
         }         
      }
      
      xml_qstat_show_job(&jlp, &ilp,  &alp, &jid_list, qstat_env);
   
      lFreeList(&jlp);
      lFreeList(&alp);
      lFreeList(&jid_list);
      DRETURN(0);
   }

   for_each(aep, alp) {
      if (lGetUlong(aep, AN_status) != STATUS_OK) {
         fprintf(stderr, "%s\n", lGetString(aep, AN_text));
         jobs_exist = false;
      }
   }
   lFreeList(&alp);
   if (!jobs_exist) {
      DRETURN(1);
   }

   /* does jlp contain all information we requested? */
   if (lGetNumberOfElem(jlp) == 0) {
      lListElem *elem1, *elem2;
      int first_time = 1;

      for_each(elem1, jlp) {
         char buffer[256];
 
         sprintf(buffer, sge_U32CFormat, sge_u32c(lGetUlong(elem1, JB_job_number)));   
         elem2 = lGetElemStr(jid_list, ST_name, buffer);     
         
         if (elem2) {
            lDechainElem(jid_list, elem2);
            lFreeElem(&elem2);
         }    
      }
      fprintf(stderr, "%s\n", MSG_QSTAT_FOLLOWINGDONOTEXIST);
      for_each(elem1, jid_list) {
         if (!first_time) {
            fprintf(stderr, ", "); 
         }
         first_time = 0;
         fprintf(stderr, "%s", lGetString(elem1, ST_name));
      }
      fprintf(stderr, "\n");
      DEXIT;
      SGE_EXIT((void**)&ctx, 1);
   }

   /* print scheduler job information and global scheduler info */
   for_each (j_elem, jlp) {
      u_long32 jid = lGetUlong(j_elem, JB_job_number);
      lListElem *sme;

      printf("==============================================================\n");
      /* print job information */
      cull_show_job(j_elem, 0, (qstat_env->full_listing & QSTAT_DISPLAY_BINDING) != 0 ? true : false);
      
      /* print scheduling information */
      if (schedd_info && (sme = lFirst(ilp))) {
         int first_run = 1;

         if (sme) {
            /* global schduling info */
            for_each (mes, lGetList(sme, SME_global_message_list)) {
               if (first_run) {
                  printf("%s:            ",MSG_SCHEDD_SCHEDULINGINFO);
                  first_run = 0;
               } else {
                  printf("%s", "                            ");
               }
               printf("%s\n", lGetString(mes, MES_message));
            }

            /* job scheduling info */
            for_each(mes, lGetList(sme, SME_message_list)) {
               lListElem *mes_jid;

               for_each(mes_jid, lGetList(mes, MES_job_number_list)) {
                  if (lGetUlong(mes_jid, ULNG_value) == jid) {
                     if (first_run) {
                        printf("%s:            ",MSG_SCHEDD_SCHEDULINGINFO);
                        first_run = 0;
                     } else {
                        printf("%s", "                            ");
                     }
                     printf("%s\n", lGetString(mes, MES_message));
                  }
               }
            }
         }
      }
   }

   lFreeList(&ilp);
   lFreeList(&jlp);
   DRETURN(0);
}

static int qstat_show_job_info(sge_gdi_ctx_class_t *ctx, u_long32 isXML, qstat_env_t *qstat_env)
{
   lList *ilp = NULL, *mlp = NULL;
   lListElem* aep = NULL;
   lEnumeration* what = NULL;
   lList* alp = NULL;
   bool schedd_info = true;
   lListElem* mes;
   int initialized = 0;
   u_long32 last_jid = 0;
   u_long32 last_mid = 0;
   char text[256], ltext[256];
   int ids_per_line = 0;
   int first_run = 1;
   int first_row = 1;
   lListElem *sme;
   lListElem *jid_ulng = NULL; 

   DENTER(TOP_LAYER, "qstat_show_job_info");

   /* get job scheduling information */
   what = lWhat("%T(ALL)", SME_Type);
   alp = ctx->gdi(ctx, SGE_SME_LIST, SGE_GDI_GET, &ilp, NULL, what, false);
   lFreeWhat(&what);
   if (isXML){
      xml_qstat_show_job_info(&ilp, &alp, qstat_env);
   }
   else {
      for_each(aep, alp) {
         if (lGetUlong(aep, AN_status) != STATUS_OK) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            schedd_info = false;
         }
      }
      lFreeList(&alp);
      if (!schedd_info) {
         DEXIT;
         return 1;
      }

      sme = lFirst(ilp);
      if (sme) {
         /* print global schduling info */
         first_run = 1;
         for_each (mes, lGetList(sme, SME_global_message_list)) {
            if (first_run) {
               printf("%s:            ",MSG_SCHEDD_SCHEDULINGINFO);
               first_run = 0;
            }
            else
               printf("%s", "                            ");
            printf("%s\n", lGetString(mes, MES_message));
         }
         if (!first_run)
            printf("\n");

         first_run = 1;

         mlp = lGetList(sme, SME_message_list);
         lPSortList (mlp, "I+", MES_message_number);

         /* 
          * Remove all jids which have more than one entry for a MES_message_number
          * After this step the MES_messages are not correct anymore
          * We do not need this messages for the summary output
          */
         {
            lListElem *flt_msg, *flt_nxt_msg;
            lList *new_list;
            lListElem *ref_msg, *ref_jid;

            new_list = lCreateList("filtered message list", MES_Type);

            flt_nxt_msg = lFirst(mlp);
            while ((flt_msg = flt_nxt_msg)) {
               lListElem *flt_jid, * flt_nxt_jid;
               int found_msg, found_jid;

               flt_nxt_msg = lNext(flt_msg);
               found_msg = 0;
               for_each(ref_msg, new_list) {
                  if (lGetUlong(ref_msg, MES_message_number) == 
                      lGetUlong(flt_msg, MES_message_number)) {
                 
                  flt_nxt_jid = lFirst(lGetList(flt_msg, MES_job_number_list));
                  while ((flt_jid = flt_nxt_jid)) {
                     flt_nxt_jid = lNext(flt_jid);
                    
                     found_jid = 0; 
                     for_each(ref_jid, lGetList(ref_msg, MES_job_number_list)) {
                        if (lGetUlong(ref_jid, ULNG_value) == 
                            lGetUlong(flt_jid, ULNG_value)) {
                           lRemoveElem(lGetList(flt_msg, MES_job_number_list), &flt_jid);
                           found_jid = 1;
                           break;
                        }
                     }
                     if (!found_jid) { 
                        lDechainElem(lGetList(flt_msg, MES_job_number_list), flt_jid);
                        lAppendElem(lGetList(ref_msg, MES_job_number_list), flt_jid);
                     } 
                  }
                  found_msg = 1;
               }
            }
            if (!found_msg) {
               lDechainElem(mlp, flt_msg);
               lAppendElem(new_list, flt_msg);
            }
         }
         lSetList(sme, SME_message_list, new_list);
         mlp = new_list;
      }

      text[0]=0;
      for_each(mes, mlp) {
         lPSortList (lGetList(mes, MES_job_number_list), "I+", ULNG_value);

         for_each(jid_ulng, lGetList(mes, MES_job_number_list)) {
            u_long32 mid;
            u_long32 jid = 0;
            int skip = 0;
            int header = 0;

            mid = lGetUlong(mes, MES_message_number);
            jid = lGetUlong(jid_ulng, ULNG_value);

            if (initialized) {
               if (last_mid == mid && last_jid == jid)
                  skip = 1;
               else if (last_mid != mid)
                     header = 1;
               }
               else {
                  initialized = 1;
                  header = 1;
            }

               if (strlen(text) >= MAX_LINE_LEN || ids_per_line >= MAX_IDS_PER_LINE || header) {
                  printf("%s", text);
                  text[0] = 0;
                  ids_per_line = 0;
                  first_row = 0;
               }

               if (header) {
                  if (!first_run)
                     printf("\n\n");
                  else
                     first_run = 0;
                  printf("%s\n", sge_schedd_text(mid+SCHEDD_INFO_OFFSET));
                  first_row = 1;
               }

               if (!skip) {
                  if (ids_per_line == 0)
                     if (first_row)
                        strcat(text, "\t");
                     else
                        strcat(text, ",\n\t");
                  else
                     strcat(text, ",\t");
                  sprintf(ltext, sge_u32, jid);
                  strcat(text, ltext);
                  ids_per_line++;
               }

               last_jid = jid;
               last_mid = mid;
            }
         }
         if (text[0] != 0)
            printf("%s\n", text);
      }
   }

   lFreeList(&ilp);
   
   DEXIT;
   return 0;
}

