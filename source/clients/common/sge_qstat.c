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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_prog.h"
#include "uti/sge_parse_num_par.h"
#include "uti/setup_path.h"
#include "uti/sge_time.h"
#include "uti/sge_stdlib.h"

#include "cull/cull_sort.h"

#include "cull/cull_sort.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_conf.h" 
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_binding.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance_type.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_usage.h"

#include "sched/sge_sched.h"
#include "sched/sge_urgency.h"
#include "sched/sge_support.h"
#include "sched/load_correction.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi.h"

#include "sge_cqueue_qstat.h"
#include "sge_qstat.h"
#include "qstat_printing.h"
#include "sge.h"

#include "msg_clients_common.h"

static int qstat_env_get_all_lists(qstat_env_t *qstat_env, bool need_job_list, lList** alpp);

int qstat_env_filter_queues(qstat_env_t *qstat_env, lList **alpp);
static int filter_jobs(qstat_env_t *qstat_env);
static void calc_longest_queue_length(qstat_env_t *qstat_env);
static int qstat_env_prepare(qstat_env_t* qstat_env, bool need_job_list, lList **alpp);

static void remove_tagged_jobs(lList *job_list);
static int qstat_handle_running_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);


void qstat_env_destroy(qstat_env_t* qstat_env) {
   /* Free the lLists */ 
   lFreeList(&qstat_env->resource_list);
   lFreeList(&qstat_env->qresource_list);
   lFreeList(&qstat_env->queueref_list);
   lFreeList(&qstat_env->peref_list);
   lFreeList(&qstat_env->user_list);
   lFreeList(&qstat_env->queue_user_list);
   lFreeList(&qstat_env->queue_list);
   lFreeList(&qstat_env->centry_list);
   lFreeList(&qstat_env->exechost_list);
   lFreeList(&qstat_env->schedd_config);
   lFreeList(&qstat_env->pe_list);
   lFreeList(&qstat_env->ckpt_list);
   lFreeList(&qstat_env->acl_list);
   lFreeList(&qstat_env->zombie_list);
   lFreeList(&qstat_env->job_list);
   lFreeList(&qstat_env->hgrp_list);
   lFreeList(&qstat_env->project_list);
   /* Free the lEnumerations */
   lFreeWhat(&qstat_env->what_JB_Type);
   lFreeWhat(&qstat_env->what_JAT_Type_list);
   lFreeWhat(&qstat_env->what_JAT_Type_template);
   /* Do not free the context - it's a reference */
   qstat_env->ctx = NULL;
}

static int handle_queue(lListElem *q, qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);
static int handle_jobs_queue(lListElem *qep, qstat_env_t* qstat_env, int print_jobs_of_queue, 
                             qstat_handler_t *handler, lList **alpp);

static int sge_handle_job(lListElem *job, lListElem *jatep, lListElem *qep, lListElem *gdil_ep, bool print_jobid,
                          char *master, dstring *dyn_task_str,
                          int slots, int slot, int slots_per_line,
                          qstat_env_t *qstat_env, job_handler_t *handler, lList **alpp );

static int job_handle_subtask(lListElem *job, lListElem *ja_task, lListElem *pe_task,
                              job_handler_t *handler, lList **alpp );
                              
static int handle_pending_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);
static int handle_finished_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);
static int handle_error_jobs(qstat_env_t *qstat_env, qstat_handler_t* handler, lList **alpp);

static int handle_zombie_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);

static int handle_jobs_not_enrolled(lListElem *job, bool print_jobid, char *master,
                                    int slots, int slot, int *count,
                                    qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp);
                       
static int job_handle_resources(lList* cel, lList* centry_list, int slots,
                                job_handler_t *handler,
                                int(*start_func)(job_handler_t* handler, lList **alpp),
                                int(*resource_func)(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp),
                                int(*finish_func)(job_handler_t* handler, lList **alpp),
                                lList **alpp);                                    

static void print_qstat_env_to(qstat_env_t *qstat_env, FILE* file);

int qselect(qstat_env_t* qstat_env, qselect_handler_t* handler, lList **alpp) {
 
   lListElem *cqueue = NULL;
   lListElem *qep = NULL;
   
   DENTER(TOP_LAYER,"qselect");
   
   /* we need the queue list in any case */
   qstat_env->need_queues = true;

   if (qstat_env_prepare(qstat_env, false, alpp) != 0) {
      DRETURN(1);
   }
   
   if (qstat_env_filter_queues(qstat_env, alpp) <= 0) {
      DRETURN(1);
   }

   /* Do output */
   if (handler->report_started != NULL) {
      handler->report_started(handler, alpp);
   }
   for_each(cqueue, qstat_env->queue_list) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);

      for_each(qep, qinstance_list) {
         if ((lGetUlong(qep, QU_tag) & TAG_SHOW_IT)!=0) {
            if (handler->report_queue != NULL) {
               handler->report_queue(handler, lGetString(qep, QU_full_name), alpp);
            }   
         }
      }
   }
   if (handler->report_finished != NULL) {
      handler->report_finished(handler, alpp);
   }
   
   DRETURN(0);
}

int qstat_cqueue_summary(qstat_env_t *qstat_env, cqueue_summary_handler_t *handler, lList **alpp) {
 
   int ret = 0;
   lListElem *cqueue = NULL;
   
   DENTER(TOP_LAYER,"qstat_cqueue_summary");
   
   if ((ret = qstat_env_prepare(qstat_env, true, alpp)) != 0 ) {
      DPRINTF(("qstat_env_prepare failed\n"));
      DRETURN(ret);
   }
   
   if ((ret = qstat_env_filter_queues(qstat_env, alpp)) < 0) {
      DPRINTF(("qstat_env_filter_queues failed\n"));
      DRETURN(ret);
   }
   
   if ((ret = filter_jobs(qstat_env)) != 0) {
      DPRINTF(("filter_jobs failed\n"));
      DRETURN(ret);
   }

   calc_longest_queue_length(qstat_env);
   
   correct_capacities(qstat_env->exechost_list, qstat_env->centry_list);
   
   handler->qstat_env = qstat_env;
   
   if (handler->report_started != NULL) {
      ret = handler->report_started(handler, alpp);
      if (ret) {
         DRETURN(ret);
      }
   }
   

   for_each (cqueue, qstat_env->queue_list) {
      if (lGetUlong(cqueue, CQ_tag) != TAG_DEFAULT) {
         cqueue_summary_t summary;
         
         memset(&summary, 0, sizeof(cqueue_summary_t));
         
         cqueue_calculate_summary(cqueue,
                                  qstat_env->exechost_list,
                                  qstat_env->centry_list,
                                  &(summary.load),
                                  &(summary.is_load_available),
                                  &(summary.used),
                                  &(summary.resv),
                                  &(summary.total),
                                  &(summary.suspend_manual),
                                  &(summary.suspend_threshold),
                                  &(summary.suspend_on_subordinate),
                                  &(summary.suspend_calendar),
                                  &(summary.unknown),
                                  &(summary.load_alarm),
                                  &(summary.disabled_manual),
                                  &(summary.disabled_calendar),
                                  &(summary.ambiguous),
                                  &(summary.orphaned),
                                  &(summary.error),
                                  &(summary.available),
                                  &(summary.temp_disabled),
                                  &(summary.manual_intervention));
                                  
         if (handler->report_cqueue != NULL && (ret = handler->report_cqueue(handler, lGetString(cqueue, CQ_name), &summary, alpp))) {
            DRETURN(ret);
         }
      }
   }
   
   if (handler->report_finished != NULL) {
      ret = handler->report_finished(handler, alpp); 
      if (ret) {
         DRETURN(ret);
      }
   }
   handler->qstat_env = NULL;
   
   DRETURN(0);
}

int qstat_no_group(qstat_env_t* qstat_env, qstat_handler_t* handler, lList **alpp) {
 
   int ret = 0;

   DENTER(TOP_LAYER,"qstat_no_group");

   if (getenv("SGE_QSTAT_ENV_DEBUG") != NULL) {
      print_qstat_env_to(qstat_env, stdout);
      qstat_env->global_showjobs = 1;
      qstat_env->global_showqueues = 1;
   }
   
   if ((ret = qstat_env_prepare(qstat_env, true, alpp)) != 0 ) {
      DRETURN(ret);
   }

   if ((ret = qstat_env_filter_queues(qstat_env, alpp)) < 0 ) {
      DRETURN(ret);
   }

   if ((ret = filter_jobs(qstat_env)) != 0 ) {
      DRETURN(ret);
   }
   
   calc_longest_queue_length(qstat_env);

   correct_capacities(qstat_env->exechost_list, qstat_env->centry_list);
   
   handler->qstat_env = qstat_env;
   handler->job_handler.qstat_env = qstat_env;
   
   
   if (handler->report_started && (ret = handler->report_started(handler, alpp))) {
      DPRINTF(("report_started failed\n"));
      DRETURN(ret);
   }
   
   if ((ret = qstat_handle_running_jobs(qstat_env, handler, alpp))) {
      DPRINTF(("qstat_handle_running_jobs failed\n"));
      DRETURN(ret);
   }
   remove_tagged_jobs(qstat_env->job_list);
 
   /* sort pending jobs */
   if (lGetNumberOfElem(qstat_env->job_list)>0 ) {
      sgeee_sort_jobs(&(qstat_env->job_list));
   }

   /* 
    *
    * step 4: iterate over jobs that are pending;
    *         tag them with TAG_FOUND_IT
    *
    *         print the jobs that run in these queues 
    *
    */
    if ((ret = handle_pending_jobs(qstat_env, handler, alpp))) {
       DPRINTF(("handle_pending_jobs failed\n"));
       DRETURN(ret);
    }
    
   /* 
    *
    * step 5:  in case of SGE look for finished jobs and view them as
    *          finished  a non SGE-qstat will show them as error jobs
    *
    */
    if ((ret=handle_finished_jobs(qstat_env, handler, alpp))) {
       DPRINTF(("handle_finished_jobs failed\n"));
       DRETURN(ret);
    }
    
   /*
    *
    * step 6:  look for jobs not found. This should not happen, cause each
    *          job is running in a queue, or pending. But if there is
    *          s.th. wrong we have
    *          to ensure to print this job just to give hints whats wrong
    *
    */
    if ((ret=handle_error_jobs(qstat_env, handler, alpp))) {
       DPRINTF(("handle_error_jobs failed\n"));
       DRETURN(ret);
    }

   /*
    *
    * step 7:  print recently finished jobs ('zombies')
    *
    */
    if ((ret=handle_zombie_jobs(qstat_env, handler, alpp))) {
       DPRINTF(("handle_zombie_jobs failed\n"));
       DRETURN(ret);
    }

   if (handler->report_finished && (ret = handler->report_finished(handler, alpp))) {
         DPRINTF(("report_finished failed\n"));
         DRETURN(ret);
   }
   handler->qstat_env = NULL;
   handler->job_handler.qstat_env = NULL;
   
   DRETURN(0);
}


static void calc_longest_queue_length(qstat_env_t *qstat_env) {
   u_long32 name;
   char *env;
   lListElem *qep = NULL;
   
   if ((qstat_env->group_opt & GROUP_CQ_SUMMARY) == 0) { 
      name = QU_full_name;
   }
   else {
      name = CQ_name;
   }
   if ((env = getenv("SGE_LONG_QNAMES")) != NULL){
      qstat_env->longest_queue_length = atoi(env);
      if (qstat_env->longest_queue_length == -1) {
         for_each(qep, qstat_env->queue_list) {
            int length;
            const char *queue_name =lGetString(qep, name);
            if ((length = strlen(queue_name)) > qstat_env->longest_queue_length){
               qstat_env->longest_queue_length = length;
            }
         }
      }
      else {
         if (qstat_env->longest_queue_length < 10) {
            qstat_env->longest_queue_length = 10;
         }
      }
   }
}
   


static int qstat_env_prepare(qstat_env_t* qstat_env, bool need_job_list, lList **alpp) 
{
   int ret = 0;

   DENTER(TOP_LAYER,"qstat_env_prepare");
   
   ret = qstat_env_get_all_lists(qstat_env, need_job_list, alpp);
   if (ret) {
      DRETURN(ret);
   } else {
      lFreeList(alpp);
   }

   ret = sconf_set_config(&(qstat_env->schedd_config), alpp);
   if (!ret){
      DPRINTF(("sconf_set_config failed\n"));
      DRETURN(ret);
   }
   
   centry_list_init_double(qstat_env->centry_list);

   sge_stopwatch_log(0, "Time for getting all lists");
   
   if (getenv("MORE_INFO")) {
      if (qstat_env->global_showjobs) {
         lWriteListTo(qstat_env->job_list, stdout);
         DRETURN(0);
      }

      if (qstat_env->global_showqueues) {
         lWriteListTo(qstat_env->queue_list, stdout);
         DRETURN(0);
      }
   }

   DRETURN(0);
}



static void remove_tagged_jobs(lList *job_list) {
   
   lListElem *jep = lFirst(job_list);
   lListElem *tmp = NULL;
   
   while (jep) {
      lList *task_list;
      lListElem *jatep, *tmp_jatep;

      tmp = lNext(jep);
      task_list = lGetList(jep, JB_ja_tasks);
      jatep = lFirst(task_list);
      while (jatep) {
         tmp_jatep = lNext(jatep);
         if ((lGetUlong(jatep, JAT_suitable) & TAG_FOUND_IT)) {
            lRemoveElem(task_list, &jatep);
         }
         jatep = tmp_jatep;
      }
      jep = tmp;
   }
   
}

static int qstat_handle_running_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp) 
{
   lListElem *qep = NULL;
   int ret = 0;
   
   DENTER(TOP_LAYER,"qstat_handle_running_jobs");

   /* no need to iterate through queues if queues are not printed */
   if (!qstat_env->need_queues) {
      if ((ret = handle_jobs_queue(NULL, qstat_env, 1, handler, alpp))) {
         DPRINTF(("handle_jobs_queue failed\n"));
      }
      DRETURN(ret);
   }
   
   /* handle running jobs of a queue */ 
   for_each(qep, qstat_env->queue_list) {

      const char* queue_name = lGetString(qep, QU_full_name);
      
      /* here we have the queue */
      if (lGetUlong(qep, QU_tag) & TAG_SHOW_IT) {
         
         
         if ((qstat_env->full_listing & QSTAT_DISPLAY_NOEMPTYQ) && 
             !qinstance_slots_used(qep)) {
            continue;
         }
         
         if (handler->report_queue_started && (ret=handler->report_queue_started(handler, queue_name, alpp))) {
            DPRINTF(("report_queue_started failed\n"));
            break;
         }
         
         if ((ret=handle_queue(qep, qstat_env, handler, alpp))) {
            DPRINTF(("handle_queue failed\n"));
            break;
         }

         if (qstat_env->shut_me_down != NULL && qstat_env->shut_me_down()) {
            DPRINTF(("shut_me_down\n"));
            ret = 1;
            break;
         }

         if ((ret = handle_jobs_queue(qep, qstat_env, 1, handler, alpp))) {
            DPRINTF(("handle_jobs_queue failed\n"));
            break;
         }
         if (handler->report_queue_finished && (ret=handler->report_queue_finished(handler, queue_name, alpp))) {
            DPRINTF(("report_queue_finished failed\n"));
            break;
         }
         
      }
   }

   DRETURN(ret);
}

static int handle_jobs_queue(lListElem *qep, qstat_env_t* qstat_env, int print_jobs_of_queue, 
                             qstat_handler_t *handler, lList **alpp) {
   lListElem *jlep;
   lListElem *jatep;
   lListElem *gdilep, *old_gdilep = NULL;
   u_long32 job_tag;
   u_long32 jid = 0, old_jid;
   u_long32 jataskid = 0, old_jataskid;
   const char *qnm = qep?lGetString(qep, QU_full_name):NULL;
   int ret = 0;
   dstring dyn_task_str = DSTRING_INIT;

   DENTER(TOP_LAYER, "handle_jobs_queue");

   if (handler->report_queue_jobs_started && (ret=handler->report_queue_jobs_started(handler, qnm, alpp)) ) {
      DPRINTF(("report_queue_jobs_started failed\n"));
      goto error;
   }
   
   for_each(jlep, qstat_env->job_list) {
      int master, i;

      for_each(jatep, lGetList(jlep, JB_ja_tasks)) {
         u_long32 jstate = lGetUlong(jatep, JAT_state);

         if (qstat_env->shut_me_down && qstat_env->shut_me_down()) {
            DPRINTF(("shut_me_down\n"));
            ret = 1;
            goto error;
         }

         if (ISSET(jstate, JSUSPENDED_ON_SUBORDINATE) ||
             ISSET(jstate, JSUSPENDED_ON_SLOTWISE_SUBORDINATE)) {
            lSetUlong(jatep, JAT_state, jstate & ~JRUNNING);
         }
            
         for_each (gdilep, lGetList(jatep, JAT_granted_destin_identifier_list)) {

            if (!qep || !strcmp(lGetString(gdilep, JG_qname), qnm)) {
               int slot_adjust = 0;
               int lines_to_print;
               int slots_per_line = 0;
               int slots_in_queue = lGetUlong(gdilep, JG_slots); 

               if (!qep)
                  qnm = lGetString(gdilep, JG_qname);

               job_tag = lGetUlong(jatep, JAT_suitable);
               job_tag |= TAG_FOUND_IT;
               lSetUlong(jatep, JAT_suitable, job_tag);

               master = !strcmp(qnm, 
                     lGetString(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_qname));

               if (master) {
                  const char *pe_name;
                  lListElem *pe;
                  if (((pe_name=lGetString(jatep, JAT_granted_pe))) &&
                      ((pe=pe_list_locate(qstat_env->pe_list, pe_name))) &&
                      !lGetBool(pe, PE_job_is_first_task))

                      slot_adjust = 1;
               }

               /* job distribution view ? */
               if (!(qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)) {
                  /* no - condensed ouput format */
                  if (!master && !(qstat_env->full_listing & QSTAT_DISPLAY_FULL)) {
                     /* skip all slave outputs except in full display mode */
                     continue;
                  }

                  /* print only on line per job for this queue */
                  lines_to_print = 1;

                  /* always only show the number of job slots represented by the line */
                  if ((qstat_env->full_listing & QSTAT_DISPLAY_FULL)) {
                     slots_per_line = slots_in_queue;
                  } else {
                     slots_per_line = sge_granted_slots(lGetList(jatep, JAT_granted_destin_identifier_list));
                  }

               } else {
                  /* yes */
                  lines_to_print = (int)slots_in_queue+slot_adjust;
                  slots_per_line = 1;
               }

               for (i=0; i<lines_to_print ;i++) {
                  bool print_jobid = false;
                  bool print_it = false;
                  int different;

                  old_jid = jid;
                  jid = lGetUlong(jlep, JB_job_number);
                  old_jataskid = jataskid;
                  jataskid = lGetUlong(jatep, JAT_task_number);

                  different = (jid != old_jid) || (jataskid != old_jataskid) || (gdilep != old_gdilep);
                  old_gdilep = gdilep;
                  
                  if (different) {
                     print_jobid = true;
                  } else {
                     if (!(qstat_env->full_listing & QSTAT_DISPLAY_RUNNING)) {
                        print_jobid = ((master && (i==0)) ? true : false);
                     } else {
                        print_jobid = false;
                     }
                  }

                  if (!lGetNumberOfElem(qstat_env->user_list) || 
                     (lGetNumberOfElem(qstat_env->user_list) && (lGetUlong(jatep, JAT_suitable)&TAG_SELECT_IT))) {
                     if (print_jobs_of_queue && (job_tag & TAG_SHOW_IT)) {
                        if ((qstat_env->full_listing & QSTAT_DISPLAY_RUNNING) &&
                            (lGetUlong(jatep, JAT_state) & JRUNNING) ) {
                           print_it = true;
                        } else if ((qstat_env->full_listing & QSTAT_DISPLAY_SUSPENDED) &&
                           ((lGetUlong(jatep, JAT_state)&JSUSPENDED) ||
                           (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_THRESHOLD) ||
                           (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_SUBORDINATE) ||
                           (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_SLOTWISE_SUBORDINATE))) {
                           print_it = true;
                        } else if ((qstat_env->full_listing & QSTAT_DISPLAY_USERHOLD) &&
                            (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_USER)) {
                           print_it = true;
                        } else if ((qstat_env->full_listing & QSTAT_DISPLAY_OPERATORHOLD) &&
                            (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_OPERATOR))  {
                           print_it = true;
                        } else if ((qstat_env->full_listing & QSTAT_DISPLAY_SYSTEMHOLD) &&
                            (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_SYSTEM)) {
                           print_it = true;
                        } else if ((qstat_env->full_listing & QSTAT_DISPLAY_JOBARRAYHOLD) &&
                            (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_JA_AD)) {
                           print_it = true;
                        } else {
                           print_it = false;
                        }       
                        if (print_it) {
                           sge_dstring_sprintf(&dyn_task_str, sge_u32, jataskid);
                           ret = sge_handle_job(jlep, jatep, qep, gdilep, print_jobid,
                                                (master && different && (i==0))?"MASTER":"SLAVE",
                                                &dyn_task_str,
                                                slots_in_queue+slot_adjust, i, slots_per_line,
                                                qstat_env, &(handler->job_handler), alpp );
                           if (ret) {
                              goto error;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   if (handler->report_queue_jobs_finished && (ret=handler->report_queue_jobs_finished(handler, qnm, alpp)) ) {
      DPRINTF(("report_queue_jobs_finished failed\n"));
      goto error;
   }
   
   
error:
   sge_dstring_free(&dyn_task_str);                     
   DRETURN(ret);
}


static int filter_jobs(qstat_env_t *qstat_env) {
   
   lListElem *jep = NULL;
   lListElem *jatep = NULL;
   lListElem *up = NULL;
   
   DENTER(TOP_LAYER,"filter_jobs");

   /* 
   ** all jobs are selected 
   */
   for_each (jep, qstat_env->job_list) {
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         if (!(lGetUlong(jatep, JAT_status) & JFINISHED))
            lSetUlong(jatep, JAT_suitable, TAG_SHOW_IT);
      }
   }

   /*
   ** tag only jobs which satisfy the user list
   */
   if (lGetNumberOfElem(qstat_env->user_list)) {
      DPRINTF(("------- selecting jobs -----------\n"));

      /* ok, now we untag the jobs if the user_list was specified */ 
      for_each(up, qstat_env->user_list) 
         for_each (jep, qstat_env->job_list) {
            if (up && lGetString(up, ST_name) && 
                  !fnmatch(lGetString(up, ST_name), 
                              lGetString(jep, JB_owner), 0)) {
               for_each (jatep, lGetList(jep, JB_ja_tasks)) {
                  lSetUlong(jatep, JAT_suitable, 
                     lGetUlong(jatep, JAT_suitable)|TAG_SHOW_IT|TAG_SELECT_IT);
               }
            }
         }
   }


   if (lGetNumberOfElem(qstat_env->peref_list) || lGetNumberOfElem(qstat_env->queueref_list) || 
       lGetNumberOfElem(qstat_env->resource_list) || lGetNumberOfElem(qstat_env->queue_user_list)) {
          
      lListElem *cqueue = NULL;
      lListElem *qep = NULL;
      /*
      ** unselect all pending jobs that fit in none of the selected queues
      ** that way the pending jobs are really pending jobs for the queues 
      ** printed
      */

      sconf_set_qs_state(QS_STATE_EMPTY);
      for_each(jep, qstat_env->job_list) {
         int ret, show_job;

         show_job = 0;

         for_each(cqueue, qstat_env->queue_list) {
            const lList *qinstance_list = lGetList(cqueue, CQ_qinstances);

            for_each(qep, qinstance_list) {
               lListElem *host = NULL;

               if (!(lGetUlong(qep, QU_tag) & TAG_SHOW_IT)) {
                  continue;
               }
               
               host = host_list_locate(qstat_env->exechost_list, lGetHost(qep, QU_qhostname));
               
               if (host != NULL) {
                  ret = sge_select_queue(lGetList(jep, JB_hard_resource_list), qep, 
                                         host, qstat_env->exechost_list, qstat_env->centry_list, 
                                         true, 1, qstat_env->queue_user_list, qstat_env->acl_list, jep);

                  if (ret==1) {
                     show_job = 1;
                     break;
                  }
               }
               /* we should have an error message here, even so it should not happen, that
                 we have queue instances without a host, but.... */
            }
         }   

         for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            if (!show_job && !(lGetUlong(jatep, JAT_status) == JRUNNING || (lGetUlong(jatep, JAT_status) == JTRANSFERING))) {
               DPRINTF(("show task "sge_u32"."sge_u32"\n",
                       lGetUlong(jep, JB_job_number),
                       lGetUlong(jatep, JAT_task_number)));
               lSetUlong(jatep, JAT_suitable, lGetUlong(jatep, JAT_suitable) & ~TAG_SHOW_IT);
            }
         }
         if (!show_job) {
            lSetList(jep, JB_ja_n_h_ids, NULL);
            lSetList(jep, JB_ja_u_h_ids, NULL);
            lSetList(jep, JB_ja_o_h_ids, NULL);
            lSetList(jep, JB_ja_s_h_ids, NULL);
         }
      }
      sconf_set_qs_state(QS_STATE_FULL);
   }

   /*
    * step 2.5: reconstruct queue stata structure
    */
   if ((qstat_env->group_opt & GROUP_CQ_SUMMARY) != 0) {
      lPSortList(qstat_env->queue_list, "%I+ ", CQ_name);
   } else {
      lList *tmp_queue_list = NULL;
      lListElem *cqueue = NULL;

      tmp_queue_list = lCreateList("", QU_Type);

      for_each(cqueue, qstat_env->queue_list) {
         lList *qinstances = NULL;

         lXchgList(cqueue, CQ_qinstances, &qinstances);
         lAddList(tmp_queue_list, &qinstances);
      }
      
      lFreeList(&(qstat_env->queue_list));
      qstat_env->queue_list = tmp_queue_list;
      tmp_queue_list = NULL;

      lPSortList(qstat_env->queue_list, "%I+ %I+ %I+", QU_seq_no, QU_qname, QU_qhostname);
   }
   DRETURN(0);
}


/*-------------------------------------------------------------------------*/
int qstat_env_filter_queues(qstat_env_t *qstat_env, lList **alpp) {
   
   int ret = 0;

   DENTER(TOP_LAYER, "qstat_env_filter_queues");

   ret = filter_queues(NULL,
                        qstat_env->queue_list,
                        qstat_env->centry_list,
                        qstat_env->hgrp_list,
                        qstat_env->exechost_list,
                        qstat_env->acl_list,
                        qstat_env->project_list,
                        qstat_env->pe_list,
                        qstat_env->resource_list, 
                        qstat_env->queueref_list, 
                        qstat_env->peref_list, 
                        qstat_env->queue_user_list,
                        qstat_env->queue_state,
                        alpp);
   DRETURN(ret);
}

int filter_queues(lList **filtered_queue_list,
                  lList *queue_list, 
                  lList *centry_list,
                  lList *hgrp_list,
                  lList *exechost_list,
                  lList *acl_list,
                  lList *prj_list,
                  lList *pe_list,
                  lList *resource_list, 
                  lList *queueref_list, 
                  lList *peref_list, 
                  lList *queue_user_list,
                  u_long32 queue_states,
                  lList **alpp)
{
   int nqueues = 0;
/*   u_long32 empty_qs = 0; */
   u_long32 empty_qs = 1;

   DENTER(TOP_LAYER, "filter_queues");

   centry_list_init_double(centry_list);

   DPRINTF(("------- selecting queues -----------\n"));
   /* all queues are selected */
   cqueue_list_set_tag(queue_list, TAG_SHOW_IT, true);

   /* unseclect all queues not selected by a -q (if exist) */
   if (lGetNumberOfElem(queueref_list)>0) {
      
      if ((nqueues=select_by_qref_list(queue_list, hgrp_list, queueref_list))<0) {
         DRETURN(-1);
      }

      if (nqueues==0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_QSTAT_NOQUEUESREMAININGAFTERXQUEUESELECTION_S, "-q");
         if (filtered_queue_list != NULL) {
            *filtered_queue_list = NULL;
         }
         DRETURN(0);
      }
   }

   /* unselect all queues not selected by -qs */
   select_by_queue_state(queue_states, exechost_list, queue_list, centry_list);
  
   /* unselect all queues not selected by a -U (if exist) */
   if (lGetNumberOfElem(queue_user_list)>0) {
      if ((nqueues=select_by_queue_user_list(exechost_list, queue_list, 
                                             queue_user_list, acl_list, prj_list))<0) {
         DRETURN(-1);
      }

      if (nqueues==0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_QSTAT_NOQUEUESREMAININGAFTERXQUEUESELECTION_S, "-U");
         if (filtered_queue_list != NULL) {
            *filtered_queue_list = NULL;
         }
         DRETURN(0);
      }
   }

   /* unselect all queues not selected by a -pe (if exist) */
   if (lGetNumberOfElem(peref_list)>0) {
      if ((nqueues=select_by_pe_list(queue_list, peref_list, pe_list))<0) {
         DRETURN(-1);
      }

      if (nqueues==0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_QSTAT_NOQUEUESREMAININGAFTERXQUEUESELECTION_S, "-pe");
         if (filtered_queue_list != NULL) {
            *filtered_queue_list = NULL;
         }
         DRETURN(0);
      }
   }
   /* unselect all queues not selected by a -l (if exist) */
   if (lGetNumberOfElem(resource_list)) {
      if (select_by_resource_list(resource_list, exechost_list, 
                                  queue_list, centry_list, empty_qs)<0) {
         DRETURN(-1);
      }
   }   

   if (rmon_mlgetl(&RMON_DEBUG_ON, GDI_LAYER) & INFOPRINT) {
      lListElem *cqueue;
      for_each(cqueue, queue_list) {
         lListElem *qep;
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);

         for_each(qep, qinstance_list) {
            if ((lGetUlong(qep, QU_tag) & TAG_SHOW_IT)!=0) {
               DPRINTF(("++ %s\n", lGetString(qep, QU_full_name)));
            } else {
               DPRINTF(("-- %s\n", lGetString(qep, QU_full_name)));
            }
         }
      }
   }


   if (!is_cqueue_selected(queue_list)) {
      if (filtered_queue_list != NULL) {
         *filtered_queue_list = NULL;
      }
      DRETURN(0);
   } 

   if (filtered_queue_list != NULL) {
      static lCondition *tagged_queues = NULL;
      static lEnumeration *all_fields = NULL;
      if (!tagged_queues) {
         tagged_queues = lWhere("%T(%I == %u)", CQ_Type, CQ_tag, TAG_SHOW_IT);
         all_fields = lWhat("%T(ALL)", CQ_Type);
      }
      *filtered_queue_list = lSelect("FQL", queue_list, tagged_queues, all_fields);  
   }

   DRETURN(1);
}

static int qstat_env_get_all_lists(qstat_env_t* qstat_env, bool need_job_list, lList** alpp) 
{
   lList **queue_l = qstat_env->need_queues ? &(qstat_env->queue_list) : NULL;
   lList **job_l = need_job_list ? &(qstat_env->job_list) : NULL;
   lList **centry_l = &(qstat_env->centry_list);
   lList **exechost_l = &(qstat_env->exechost_list);
   lList **sc_l = &(qstat_env->schedd_config);
   lList **pe_l = &(qstat_env->pe_list);
   lList **ckpt_l = &(qstat_env->ckpt_list);
   lList **acl_l = &(qstat_env->acl_list);
   lList **zombie_l = &(qstat_env->zombie_list);
   lList **hgrp_l = &(qstat_env->hgrp_list);
   /*lList *queueref_list = qstat_env->queueref_list;
   lList *peref_list = qstat_env->peref_list;*/
   lList *user_list = qstat_env->user_list;
   lList **project_l = &(qstat_env->project_list);
   u_long32 show = qstat_env->full_listing;
   
   lCondition *where= NULL, *nw = NULL; 
   lCondition *zw = NULL, *gc_where = NULL;
   lEnumeration *q_all, *pe_all, *ckpt_all, *acl_all, *ce_all, *up_all;
   lEnumeration *eh_all, *sc_what, *gc_what, *hgrp_what;
   lListElem *ep = NULL;
   lList *conf_l = NULL;
   lList *mal = NULL;
   int q_id = 0, j_id = 0, pe_id = 0, ckpt_id = 0, acl_id = 0, z_id = 0, up_id = 0;
   int ce_id, eh_id, sc_id, gc_id, hgrp_id = 0;
   int show_zombies = (show & QSTAT_DISPLAY_ZOMBIES) ? 1 : 0;
   state_gdi_multi state = STATE_GDI_MULTI_INIT;
   const char *cell_root = qstat_env->ctx->get_cell_root(qstat_env->ctx);
   u_long32 progid = qstat_env->ctx->get_who(qstat_env->ctx);

   DENTER(TOP_LAYER, "qstat_env_get_all_lists");

   if (queue_l) {
      DPRINTF(("need queues\n"));
      q_all = lWhat("%T(ALL)", CQ_Type);
      
      q_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                       SGE_CQ_LIST, SGE_GDI_GET, NULL, NULL, 
                                       q_all, false, &state, true);
      lFreeWhat(&q_all);
    
      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   } else {
      DPRINTF(("queues not needed\n"));
   }

   /* 
   ** jobs
   */
   if (job_l) {
      lCondition *where = qstat_get_JB_Type_selection(user_list, show);
      lEnumeration *what = qstat_get_JB_Type_filter(qstat_env);

      j_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                       SGE_JB_LIST, SGE_GDI_GET, NULL, where, 
                                       what, false, &state, true);
      lFreeWhere(&where);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

   /* 
   ** job zombies
   */
   if (zombie_l && show_zombies) {
      for_each(ep, user_list) {
         nw = lWhere("%T(%I p= %s)", JB_Type, JB_owner, lGetString(ep, ST_name));
         if (!zw)
            zw = nw;
         else
            zw = lOrWhere(zw, nw);
      }

      z_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                       SGE_ZOMBIE_LIST, SGE_GDI_GET, NULL, zw, 
                                       qstat_get_JB_Type_filter(qstat_env), false,  
                                       &state, true);
      lFreeWhere(&zw);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

   /*
   ** complexes
   */
   ce_all = lWhat("%T(ALL)", CE_Type);
   ce_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                     SGE_CE_LIST, SGE_GDI_GET, NULL, NULL, 
                                     ce_all, false, &state, true); 
   lFreeWhat(&ce_all);

   if (answer_list_has_error(alpp)) {
      DRETURN(1);
   }

   /*
   ** exechosts
   */
   where = lWhere("%T(%I!=%s)", EH_Type, EH_name, SGE_TEMPLATE_NAME);
   eh_all = lWhat("%T(ALL)", EH_Type);
   eh_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                     SGE_EH_LIST, SGE_GDI_GET,
                                     NULL, where, eh_all, false, &state, true);
   lFreeWhat(&eh_all);
   lFreeWhere(&where);

   if (answer_list_has_error(alpp)) {
      DRETURN(1);
   }

   /* 
   ** pe list 
   */ 
   if (pe_l) {   
      pe_all = lWhat("%T(%I%I%I%I%I)", PE_Type, PE_name, PE_slots, PE_job_is_first_task, PE_control_slaves, PE_urgency_slots);
      pe_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                        SGE_PE_LIST, SGE_GDI_GET, NULL, NULL, pe_all, 
                                        false, &state, true);
      lFreeWhat(&pe_all);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

  /* 
   ** ckpt list 
   */ 
   if (ckpt_l) {
      ckpt_all = lWhat("%T(%I)", CK_Type, CK_name);
      ckpt_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                          SGE_CK_LIST, SGE_GDI_GET, NULL, NULL, 
                                          ckpt_all, false, &state, true);
      lFreeWhat(&ckpt_all);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

   /* 
   ** acl list 
   */ 
   if (acl_l) {
      acl_all = lWhat("%T(ALL)", US_Type);
      acl_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                         SGE_US_LIST, SGE_GDI_GET, 
                                         NULL, NULL, acl_all, false, &state, true);
      lFreeWhat(&acl_all);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

   /* 
   ** project list 
   */ 
   if (project_l) {
      up_all = lWhat("%T(ALL)", PR_Type);
      up_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                        SGE_PR_LIST, SGE_GDI_GET, 
                                        NULL, NULL, up_all, false, &state, true);
      lFreeWhat(&up_all);

      if (answer_list_has_error(alpp)) {
         DRETURN(1);
      }
   }

   /*
   ** scheduler configuration
   */

   /* might be enough, but I am not sure */
   /*sc_what = lWhat("%T(%I %I)", SC_Type, SC_user_sort, SC_job_load_adjustments);*/ 
   sc_what = lWhat("%T(ALL)", SC_Type);

   sc_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                     SGE_SC_LIST, SGE_GDI_GET, NULL, NULL, 
                                     sc_what, false, &state, true);
   lFreeWhat(&sc_what);

   if (answer_list_has_error(alpp)) {
      DRETURN(1);
   }

   /*
   ** hgroup 
   */
   hgrp_what = lWhat("%T(ALL)", HGRP_Type);
   hgrp_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_RECORD, 
                                       SGE_HGRP_LIST, SGE_GDI_GET, NULL, NULL, 
                                       hgrp_what, false, &state, true);
   lFreeWhat(&hgrp_what);

   if (answer_list_has_error(alpp)) {
      DRETURN(1);
   }

   /*
   ** global cluster configuration
   */
   gc_where = lWhere("%T(%I c= %s)", CONF_Type, CONF_name, SGE_GLOBAL_NAME);
   gc_what = lWhat("%T(ALL)", CONF_Type);
   gc_id = qstat_env->ctx->gdi_multi(qstat_env->ctx, alpp, SGE_GDI_SEND, 
                                     SGE_CONF_LIST, SGE_GDI_GET,
                                     NULL, gc_where, gc_what, false, &state, true);
   qstat_env->ctx->gdi_wait(qstat_env->ctx, alpp, &mal, &state);
   lFreeWhat(&gc_what);
   lFreeWhere(&gc_where);

   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   /*
   ** handle results
   */
   if (queue_l) {
      /* --- queue */
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_CQ_LIST, q_id, 
                                    mal, queue_l);

      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }

   /* --- job */
   if (job_l) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_JB_LIST, j_id, mal, job_l);

#if 0 /* EB: debug */
      {
         lListElem *elem = NULL;

         for_each(elem, *job_l) {
            lListElem *task = lFirst(lGetList(elem, JB_ja_tasks));

            fprintf(stderr, "jid="sge_u32" ", lGetUlong(elem, JB_job_number));
            if (task) {
               dstring string = DSTRING_INIT;

               fprintf(stderr, "state=%s status=%s job_restarted="sge_u32"\n", sge_dstring_ulong_to_binstring(&string, lGetUlong(task, JAT_state)), sge_dstring_ulong_to_binstring(&string, lGetUlong(task, JAT_status)), lGetUlong(task, JAT_job_restarted));
               sge_dstring_free(&string);
            } else {
               fprintf(stderr, "\n");
            }
         }
      }
#endif
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }

      /*
       * debug output to perform testsuite tests
       */
      if (getenv("_SGE_TEST_QSTAT_JOB_STATES") != NULL) {
         fprintf(stderr, "_SGE_TEST_QSTAT_JOB_STATES: jobs_received=%d\n", 
                 lGetNumberOfElem(*job_l));
      }
   }

   /* --- job zombies */
   if (zombie_l && show_zombies) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_ZOMBIE_LIST, z_id, mal,
         zombie_l);
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }

   /* --- complex */
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_CE_LIST, ce_id,
                                mal, centry_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   /* --- exec host */
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_EH_LIST, eh_id, 
                                 mal, exechost_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   /* --- pe */
   if (pe_l) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_PE_LIST, pe_id, 
                                    mal, pe_l);
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }

   /* --- ckpt */
   if (ckpt_l) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_CK_LIST, ckpt_id, 
                                    mal, ckpt_l);
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }

   /* --- acl */
   if (acl_l) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_US_LIST, acl_id, 
                                    mal, acl_l);
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }

   /* --- project */
   if (project_l) {
      sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_PR_LIST, up_id, 
                                    mal, project_l);
      if (answer_list_has_error(alpp)) {
         lFreeList(&mal);
         DRETURN(1);
      }
   }


   /* --- scheduler configuration */
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_SC_LIST, sc_id, mal, sc_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   /* --- hgrp */
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_HGRP_LIST, hgrp_id, mal, 
                                hgrp_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   /* -- apply global configuration for sge_hostcmp() scheme */
   sge_gdi_extract_answer(alpp, SGE_GDI_GET, SGE_CONF_LIST, gc_id, mal, &conf_l);
   if (answer_list_has_error(alpp)) {
      lFreeList(&mal);
      DRETURN(1);
   }

   if (lFirst(conf_l)) {
      lListElem *local = NULL;
      merge_configuration(NULL, progid, cell_root, lFirst(conf_l), local, NULL);
   }
   lFreeList(&conf_l);
   lFreeList(&mal);

   DRETURN(0);
}


/* ------------------- Queue Handler ---------------------------------------- */

static int handle_queue(lListElem *q, qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp) {
   char arch_string[80];
   char *load_avg_str;
   char load_alarm_reason[MAX_STRING_SIZE];
   char suspend_alarm_reason[MAX_STRING_SIZE];
   const char *queue_name = NULL;
   u_long32 interval;
   
   queue_summary_t summary;
   dstring type_string = DSTRING_INIT;
   dstring state_string = DSTRING_INIT;
   int ret = 0;
   
   DENTER(TOP_LAYER, "handle_queue");

   memset(&summary, 0, sizeof(queue_summary_t));
   
   *load_alarm_reason = 0;
   *suspend_alarm_reason = 0;

   /* make it possible to display any load value in qstat output */
   if (!(load_avg_str=getenv("SGE_LOAD_AVG")) || !strlen(load_avg_str))
      load_avg_str = LOAD_ATTR_LOAD_AVG;
   
   summary.load_avg_str = load_avg_str;
   
   if (!(qstat_env->full_listing & QSTAT_DISPLAY_FULL)) {
      DRETURN(0);
   }

   queue_name = lGetString(q, QU_full_name);

   /* compute the load and check for alarm states */

   summary.has_load_value = sge_get_double_qattr(&(summary.load_avg), load_avg_str, q, 
                                                 qstat_env->exechost_list, qstat_env->centry_list, 
                                                 &(summary.has_load_value_from_object)) ? true : false;

   if (sge_load_alarm(NULL, q, lGetList(q, QU_load_thresholds), qstat_env->exechost_list, qstat_env->centry_list, NULL, true)) {
      qinstance_state_set_alarm(q, true);
      sge_load_alarm_reason(q, lGetList(q, QU_load_thresholds), qstat_env->exechost_list, 
                            qstat_env->centry_list, load_alarm_reason, 
                            MAX_STRING_SIZE - 1, "load");
   }
   
   parse_ulong_val(NULL, &interval, TYPE_TIM,
                   lGetString(q, QU_suspend_interval), NULL, 0);
   if (lGetUlong(q, QU_nsuspend) != 0 &&
       interval != 0 &&
       sge_load_alarm(NULL, q, lGetList(q, QU_suspend_thresholds), qstat_env->exechost_list, qstat_env->centry_list, NULL, false)) {
      qinstance_state_set_suspend_alarm(q, true);
      sge_load_alarm_reason(q, lGetList(q, QU_suspend_thresholds), 
                            qstat_env->exechost_list, qstat_env->centry_list, suspend_alarm_reason, 
                            MAX_STRING_SIZE - 1, "suspend");
   }

   qinstance_print_qtype_to_dstring(q, &type_string, true);
   summary.queue_type = sge_dstring_get_string(&type_string);

   summary.resv_slots = qinstance_slots_reserved_now(q);
   summary.used_slots = qinstance_slots_used(q);
   summary.total_slots = (int)lGetUlong(q, QU_job_slots);

   /* arch */
   if (!sge_get_string_qattr(arch_string, sizeof(arch_string)-1, LOAD_ATTR_ARCH, 
       q, qstat_env->exechost_list, qstat_env->centry_list)) {
      summary.arch = arch_string;
   } else {
      summary.arch = NULL;
   }
   qinstance_state_append_to_dstring(q, &state_string);
   summary.state = sge_dstring_get_string(&state_string);
   
   if (handler->report_queue_summary && (ret=handler->report_queue_summary(handler, queue_name, &summary, alpp))) {
      DPRINTF(("report_queue_summary failed\n"));
      goto error;
   }
   

   if ((qstat_env->full_listing & QSTAT_DISPLAY_ALARMREASON)) {
      if (*load_alarm_reason) {
         if (handler->report_queue_load_alarm) {
            if ((ret=handler->report_queue_load_alarm(handler, queue_name, load_alarm_reason, alpp))) {
               DPRINTF(("report_queue_load_alarm failed\n"));
               goto error;
            }
         }
      }
      if (*suspend_alarm_reason) {
         if (handler->report_queue_suspend_alarm) {
            if ((ret=handler->report_queue_suspend_alarm(handler, queue_name, suspend_alarm_reason, alpp))) {
               DPRINTF(("report_queue_suspend_alarm failed\n"));
               goto error;
            }
         }
      }
   }

   if ((qstat_env->explain_bits & QI_ALARM) > 0) {
      if (*load_alarm_reason) {
         if (handler->report_queue_load_alarm) {
            if ((ret=handler->report_queue_load_alarm(handler, queue_name, load_alarm_reason, alpp))) {
               DPRINTF(("report_queue_load_alarm failed\n"));
               goto error;
            }
         }
      }
   }
   if ((qstat_env->explain_bits & QI_SUSPEND_ALARM) > 0) {
      if (*suspend_alarm_reason) {
         if (handler->report_queue_suspend_alarm) {
            if ((ret=handler->report_queue_suspend_alarm(handler, queue_name, suspend_alarm_reason, alpp))) {
               DPRINTF(("report_queue_suspend_alarm failed\n"));
               goto error;
            }
         }
      }
   }
   if (qstat_env->explain_bits != QI_DEFAULT && handler->report_queue_message) {
      lList *qim_list = lGetList(q, QU_message_list);
      lListElem *qim = NULL;

      for_each(qim, qim_list) {
         u_long32 type = lGetUlong(qim, QIM_type);

         if ((qstat_env->explain_bits & QI_AMBIGUOUS) == type || 
             (qstat_env->explain_bits & QI_ERROR) == type) {
            const char *message = lGetString(qim, QIM_message);

            if ((ret=handler->report_queue_message(handler, queue_name, message, alpp))) {
               DPRINTF(("report_queue_message failed\n"));
               goto error;
            }
         }
      }
   }

   /* view (selected) resources of queue in case of -F [attr,attr,..] */ 
   if (((qstat_env->full_listing & QSTAT_DISPLAY_QRESOURCES)) &&
        handler->report_queue_resource) {
      dstring resource_string = DSTRING_INIT;
      lList *rlp;
      lListElem *rep;
      char dom[5];
      u_long32 dominant = 0;
      const char *s;

      rlp = NULL;

      queue_complexes2scheduler(&rlp, q, qstat_env->exechost_list, qstat_env->centry_list);

      for_each (rep , rlp) {

         /* we had a -F request */
         if (qstat_env->qresource_list) {
            lListElem *qres;

            qres = lGetElemStr(qstat_env->qresource_list, CE_name, 
                               lGetString(rep, CE_name));
            if (qres == NULL) {
               qres = lGetElemStr(qstat_env->qresource_list, CE_name,
                               lGetString(rep, CE_shortcut));
            }

            /* if this complex variable wasn't requested with -F, skip it */
            if (qres == NULL) {
               continue ;
            }
         }
         sge_dstring_clear(&resource_string);
         s = sge_get_dominant_stringval(rep, &dominant, &resource_string);
         monitor_dominance(dom, dominant); 
         
         if ((ret=handler->report_queue_resource(handler, dom, lGetString(rep, CE_name), s, alpp))) {
            DPRINTF(("report_queue_resource failed\n"));
            break;
         }
      }

      lFreeList(&rlp);
      sge_dstring_free(&resource_string);

   }

error:
   sge_dstring_free(&type_string);
   sge_dstring_free(&state_string);
   
   DRETURN(ret);
}

/* ------------------- Job Handler ------------------------------------------ */

static int handle_pending_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp) {
   
   lListElem *nxt, *jep, *jatep, *nxt_jatep;
   lList* ja_task_list = NULL;
   int FoundTasks;
   int ret = 0;
   int count = 0;
   dstring dyn_task_str = DSTRING_INIT;
   
   DENTER(TOP_LAYER, "handle_pending_jobs");

   nxt = lFirst(qstat_env->job_list);
   while ((jep=nxt)) {
      nxt = lNext(jep);
      nxt_jatep = lFirst(lGetList(jep, JB_ja_tasks));
      FoundTasks = 0;

      while ((jatep = nxt_jatep)) { 
         if (qstat_env->shut_me_down && qstat_env->shut_me_down() ) {
            DPRINTF(("shut_me_down\n"));
            break;
         }   
         nxt_jatep = lNext(jatep);

         if (!(((qstat_env->full_listing & QSTAT_DISPLAY_OPERATORHOLD) && (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_OPERATOR))  
               ||
             ((qstat_env->full_listing & QSTAT_DISPLAY_USERHOLD) && (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_USER)) 
               ||
             ((qstat_env->full_listing & QSTAT_DISPLAY_SYSTEMHOLD) && (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_SYSTEM)) 
               ||
             ((qstat_env->full_listing & QSTAT_DISPLAY_JOBARRAYHOLD) && (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_JA_AD)) 
               ||
             ((qstat_env->full_listing & QSTAT_DISPLAY_JOBHOLD) && lGetList(jep, JB_jid_predecessor_list))
               ||
             ((qstat_env->full_listing & QSTAT_DISPLAY_STARTTIMEHOLD) && lGetUlong(jep, JB_execution_time))
               ||
             !(qstat_env->full_listing & QSTAT_DISPLAY_HOLD))
            ) {
            break;
         }

         if (!(lGetUlong(jatep, JAT_suitable) & TAG_FOUND_IT) && 
            VALID(JQUEUED, lGetUlong(jatep, JAT_state)) &&
            !VALID(JFINISHED, lGetUlong(jatep, JAT_status))) {
            lSetUlong(jatep, JAT_suitable, 
            lGetUlong(jatep, JAT_suitable)|TAG_FOUND_IT);

            if ((!lGetNumberOfElem(qstat_env->user_list) || 
               (lGetNumberOfElem(qstat_env->user_list) && 
               (lGetUlong(jatep, JAT_suitable)&TAG_SELECT_IT))) &&
               (lGetUlong(jatep, JAT_suitable)&TAG_SHOW_IT)) {
                  
               if ((qstat_env->full_listing & QSTAT_DISPLAY_PENDING) && 
                   (qstat_env->group_opt & GROUP_NO_TASK_GROUPS) > 0) {

                  sge_dstring_sprintf(&dyn_task_str, sge_u32, 
                                    lGetUlong(jatep, JAT_task_number));

                  if (count == 0 && handler->report_pending_jobs_started && (ret=handler->report_pending_jobs_started(handler, alpp))) {
                     DPRINTF(("report_pending_jobs_started failed\n"));
                     goto error;
                  }
                  ret = sge_handle_job(jep, jatep, NULL, NULL, true, NULL, &dyn_task_str,
                                       0, 0, 0,
                                       qstat_env, &(handler->job_handler), alpp);

                  if (ret) {
                     DPRINTF(("sge_handle_job failed\n"));
                     goto error;
                  }
                  count++;
               } else {
                  if (!ja_task_list) {
                     ja_task_list = lCreateList("", lGetElemDescr(jatep));
                  }
                  lAppendElem(ja_task_list, lCopyElem(jatep));
                  FoundTasks = 1;
               }
            }
         }
      }
      if ((qstat_env->full_listing & QSTAT_DISPLAY_PENDING)  && 
          (qstat_env->group_opt & GROUP_NO_TASK_GROUPS) == 0 && 
          FoundTasks && 
          ja_task_list) {
         lList *task_group = NULL;

         while ((task_group = ja_task_list_split_group(&ja_task_list))) {
            sge_dstring_clear(&dyn_task_str);
            ja_task_list_print_to_string(task_group, &dyn_task_str);

            if (count == 0 && handler->report_pending_jobs_started && (ret=handler->report_pending_jobs_started(handler, alpp))) {
               DPRINTF(("report_pending_jobs_started failed\n"));
               goto error;
            }
            ret = sge_handle_job(jep, lFirst(task_group), NULL, NULL, true, NULL, &dyn_task_str,
                                 0, 0, 0,
                                 qstat_env, &(handler->job_handler), alpp);
            
            lFreeList(&task_group);
            
            if (ret) {
               DPRINTF(("sge_handle_job failed\n"));
               goto error;
            }
            count++;
         }
      }
      if (jep != nxt && (qstat_env->full_listing & QSTAT_DISPLAY_PENDING)) {
         ret = handle_jobs_not_enrolled(jep, true, NULL,
                                        0, 0, &count, qstat_env, handler, alpp);
      }
   }
   
   if (count > 0 && handler->report_pending_jobs_finished && (ret=handler->report_pending_jobs_finished(handler, alpp))) {
      DPRINTF(("report_pending_jobs_finished failed\n"));
      goto error;
   }
   
error:
   sge_dstring_free(&dyn_task_str);
   lFreeList(&ja_task_list);

   DRETURN(ret);
}


static int handle_finished_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp) {
   lListElem *jep, *jatep;
   int ret = 0;
   int count = 0;
   dstring dyn_task_str = DSTRING_INIT;

   DENTER(TOP_LAYER, "handle_finished_jobs");

   for_each (jep, qstat_env->job_list) {
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         if (qstat_env->shut_me_down && qstat_env->shut_me_down()) {
            DPRINTF(("shut_me_down\n"));
            ret = -1;
            break;
         }   
         if (lGetUlong(jatep, JAT_status) == JFINISHED) {
            lSetUlong(jatep, JAT_suitable, lGetUlong(jatep, JAT_suitable)|TAG_FOUND_IT);

            if (!getenv("MORE_INFO"))
               continue;

            if (!lGetNumberOfElem(qstat_env->user_list) || (lGetNumberOfElem(qstat_env->user_list) && 
                  (lGetUlong(jatep, JAT_suitable)&TAG_SELECT_IT))) {
                     
               if (count == 0) {
                  if (handler->report_finished_jobs_started && (ret=handler->report_finished_jobs_started(handler, alpp))) {
                     DPRINTF(("report_finished_jobs_started failed\n"));
                     break;
                  }
               }
               sge_dstring_sprintf(&dyn_task_str, sge_u32, 
                                 lGetUlong(jatep, JAT_task_number));
                                 
               ret = sge_handle_job(jep, jatep, NULL, NULL, true, NULL, &dyn_task_str, 
                                    0, 0, 0,
                                    qstat_env, &(handler->job_handler), alpp);

               if (ret) {
                  break;
               }
               count++;
            }
         }
      }
   }

   if (ret == 0 && count > 0) {
      if (handler->report_finished_jobs_finished && (ret=handler->report_finished_jobs_finished(handler, alpp))) {
         DPRINTF(("report_finished_jobs_finished failed\n"));
      }
   }

   sge_dstring_free(&dyn_task_str);
   DRETURN(ret);
}


static int handle_error_jobs(qstat_env_t *qstat_env, qstat_handler_t* handler, lList **alpp) {

   lListElem *jep, *jatep;
   int ret = 0;
   int count = 0;
   dstring dyn_task_str = DSTRING_INIT;
   
   DENTER(TOP_LAYER, "handle_error_jobs");

   for_each(jep, qstat_env->job_list) {
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         if (!(lGetUlong(jatep, JAT_suitable) & TAG_FOUND_IT) && lGetUlong(jatep, JAT_status) == JERROR) {
            lSetUlong(jatep, JAT_suitable, lGetUlong(jatep, JAT_suitable)|TAG_FOUND_IT);

            if (!lGetNumberOfElem(qstat_env->user_list) || (lGetNumberOfElem(qstat_env->user_list) && 
                  (lGetUlong(jatep, JAT_suitable)&TAG_SELECT_IT))) {
               sge_dstring_sprintf(&dyn_task_str, sge_u32, lGetUlong(jatep, JAT_task_number));
               
               if (count == 0) {
                   if (handler->report_error_jobs_started && (ret=handler->report_error_jobs_started(handler, alpp))) {
                      DPRINTF(("report_error_jobs_started failed\n"));
                      goto error;
                   }
               }
               ret = sge_handle_job(jep, jatep, NULL, NULL, true, NULL, &dyn_task_str,
                                    0, 0, 0,
                                    qstat_env, &(handler->job_handler), alpp);

               if (ret) {
                  goto error;
               }
               count++;
            }
         }
      }
   }
   if (ret == 0 && count > 0 ) {
       if (handler->report_error_jobs_finished && (ret=handler->report_error_jobs_finished(handler, alpp))) {
          DPRINTF(("report_error_jobs_started failed\n"));
       }
   }
   
error:   
   sge_dstring_free(&dyn_task_str);
   DRETURN(ret);
}

static int handle_zombie_jobs(qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp) {
   
   lListElem *jep;
   int ret = 0;
   int count = 0;
   dstring dyn_task_str = DSTRING_INIT; 
   
   DENTER(TOP_LAYER, "handle_zombie_jobs");
   
   if (!(qstat_env->full_listing & QSTAT_DISPLAY_ZOMBIES)) {
      sge_dstring_free(&dyn_task_str);
      DRETURN(0);
   }

   for_each (jep, qstat_env->zombie_list) { 
      lList *z_ids = NULL;
      z_ids = lGetList(jep, JB_ja_z_ids);
      if (z_ids != NULL) {
         lListElem *ja_task = NULL;
         u_long32 first_task_id = range_list_get_first_id(z_ids, NULL);

         sge_dstring_clear(&dyn_task_str);

         ja_task = job_get_ja_task_template_pending(jep, first_task_id);
         range_list_print_to_string(z_ids, &dyn_task_str, false, false, false);
         
         if (count == 0 && handler->report_zombie_jobs_started && (ret=handler->report_zombie_jobs_started(handler, alpp))) {
            DPRINTF(("report_zombie_jobs_started failed\n"));
            break;
         }
         ret = sge_handle_job(jep, ja_task, NULL, NULL, true, NULL, &dyn_task_str,
                              0,0, 0,
                              qstat_env, &(handler->job_handler), alpp);
         if (ret) {
            break;                            
         }
         count++;
      }
   }

   if (ret == 0 && count > 0 && handler->report_zombie_jobs_finished && (ret=handler->report_zombie_jobs_finished(handler, alpp))) {
      DPRINTF(("report_zombie_jobs_finished failed\n"));
   }

   sge_dstring_free(&dyn_task_str);
   DRETURN(ret);
}


static int handle_jobs_not_enrolled(lListElem *job, bool print_jobid, char *master,
                                    int slots, int slot, int *count,
                                    qstat_env_t *qstat_env, qstat_handler_t *handler, lList **alpp)
{
   lList *range_list[16];         /* RN_Type */
   u_long32 hold_state[16];
   int i;
   dstring ja_task_id_string = DSTRING_INIT;
   int ret = 0;

   DENTER(TOP_LAYER, "handle_jobs_not_enrolled");

   job_create_hold_id_lists(job, range_list, hold_state); 
   for (i = 0; i <= 15; i++) {
      lList *answer_list = NULL;
      u_long32 first_id;
      int show = 0;

      if (((qstat_env->full_listing & QSTAT_DISPLAY_USERHOLD) && (hold_state[i] & MINUS_H_TGT_USER)) ||
          ((qstat_env->full_listing & QSTAT_DISPLAY_OPERATORHOLD) && (hold_state[i] & MINUS_H_TGT_OPERATOR)) ||
          ((qstat_env->full_listing & QSTAT_DISPLAY_SYSTEMHOLD) && (hold_state[i] & MINUS_H_TGT_SYSTEM)) ||
          ((qstat_env->full_listing & QSTAT_DISPLAY_JOBARRAYHOLD) && (hold_state[i] & MINUS_H_TGT_JA_AD)) ||
          ((qstat_env->full_listing & QSTAT_DISPLAY_STARTTIMEHOLD) && (lGetUlong(job, JB_execution_time) > 0)) ||
          ((qstat_env->full_listing & QSTAT_DISPLAY_JOBHOLD) && (lGetList(job, JB_jid_predecessor_list) != 0)) ||
          (!(qstat_env->full_listing & QSTAT_DISPLAY_HOLD))
         ) {
         show = 1;
      }
      if (range_list[i] != NULL && show) { 
         if ((qstat_env->group_opt & GROUP_NO_TASK_GROUPS) == 0) {
            sge_dstring_clear(&ja_task_id_string);
            range_list_print_to_string(range_list[i], &ja_task_id_string, false, false, false);
            first_id = range_list_get_first_id(range_list[i], &answer_list);
            if (answer_list_has_error(&answer_list) != 1) {
               lListElem *ja_task = job_get_ja_task_template_hold(job, 
                                                      first_id, hold_state[i]);
               lList *n_h_ids = NULL;
               lList *u_h_ids = NULL;
               lList *o_h_ids = NULL;
               lList *s_h_ids = NULL;
               lList *a_h_ids = NULL;

               lXchgList(job, JB_ja_n_h_ids, &n_h_ids);
               lXchgList(job, JB_ja_u_h_ids, &u_h_ids);
               lXchgList(job, JB_ja_o_h_ids, &o_h_ids);
               lXchgList(job, JB_ja_s_h_ids, &s_h_ids);
               lXchgList(job, JB_ja_a_h_ids, &a_h_ids);
               
               if (*count == 0 && handler->report_pending_jobs_started && (ret=handler->report_pending_jobs_started(handler, alpp))) {
                  DPRINTF(("report_pending_jobs_started failed\n"));
                  ret = 1;
                  break;
               }
               ret = sge_handle_job(job, ja_task, NULL, NULL, print_jobid, master, &ja_task_id_string,
                                    slots, slot, 0,
                                    qstat_env, &(handler->job_handler), alpp);
               if (ret) {
                  DPRINTF(("sge_handle_job failed\n"));
                  break;
               }
               lXchgList(job, JB_ja_n_h_ids, &n_h_ids);
               lXchgList(job, JB_ja_u_h_ids, &u_h_ids);
               lXchgList(job, JB_ja_o_h_ids, &o_h_ids);
               lXchgList(job, JB_ja_s_h_ids, &s_h_ids);
               lXchgList(job, JB_ja_a_h_ids, &a_h_ids);
               (*count)++;
            }
         } else {
            lListElem *range; /* RN_Type */
            
            for_each(range, range_list[i]) {
               u_long32 start, end, step;
               range_get_all_ids(range, &start, &end, &step);
               for (; start <= end; start += step) { 
                  lListElem *ja_task = job_get_ja_task_template_hold(job,
                                                          start, hold_state[i]);
                  sge_dstring_sprintf(&ja_task_id_string, sge_u32, start);
                  
                  if (*count == 0 && handler->report_pending_jobs_started && (ret=handler->report_pending_jobs_started(handler, alpp))) {
                     DPRINTF(("report_pending_jobs_started failed\n"));
                     ret = 1;
                     break;
                  }
                  ret = sge_handle_job(job, ja_task, NULL, NULL, print_jobid, NULL, &ja_task_id_string,
                                       slots, slot, 0,
                                       qstat_env, &(handler->job_handler), alpp);
                  if (ret) {
                     DPRINTF(("sge_handle_job failed\n"));
                     break;
                  }
                  (*count)++;
               }
            }
         }
      }
   }

   job_destroy_hold_id_lists(range_list);
   sge_dstring_free(&ja_task_id_string);
   DRETURN(ret);
}                 


static int sge_handle_job(lListElem *job, lListElem *jatep, lListElem *qep, lListElem *gdil_ep, 
                          bool print_jobid, char *master, dstring *dyn_task_str,
                          int slots, int slot, int slots_per_line,
                          qstat_env_t *qstat_env, job_handler_t *handler, lList **alpp ) 
{
   u_long32 jstate;
   int sge_ext, tsk_ext, sge_urg, sge_pri, sge_time;
   lList *ql = NULL;
   lListElem *qrep;
   
   job_summary_t summary;
   u_long32 ret = 0;

   DENTER(TOP_LAYER, "sge_handle_job");

   memset(&summary, 0, sizeof(job_summary_t));
   
   summary.print_jobid = print_jobid;
   summary.is_zombie = job_is_zombie_job(job);

   if (gdil_ep)
      summary.queue = lGetString(gdil_ep, JG_qname);

   sge_ext = ((qstat_env->full_listing & QSTAT_DISPLAY_EXTENDED) == QSTAT_DISPLAY_EXTENDED);
   tsk_ext = (qstat_env->full_listing & QSTAT_DISPLAY_TASKS);
   sge_urg = (qstat_env->full_listing & QSTAT_DISPLAY_URGENCY);
   sge_pri = (qstat_env->full_listing & QSTAT_DISPLAY_PRIORITY);
   sge_time = !sge_ext;
   sge_time = sge_time | tsk_ext | sge_urg | sge_pri;

   summary.nprior = lGetDouble(jatep, JAT_prio);
   if (sge_pri || sge_urg) {
      summary.nurg = lGetDouble(job, JB_nurg);
   }
   if (sge_pri) {
      summary.nppri = lGetDouble(job, JB_nppri);
   }
   if (sge_pri || sge_ext) {
      summary.ntckts =  lGetDouble(jatep, JAT_ntix);
   }
   if (sge_urg) {
      summary.urg = lGetDouble(job, JB_urg);
      summary.rrcontr = lGetDouble(job, JB_rrcontr);
      summary.wtcontr = lGetDouble(job, JB_wtcontr);
      summary.dlcontr = lGetDouble(job, JB_dlcontr);
   }
   if (sge_pri) {
      summary.priority = lGetUlong(job, JB_priority)-BASE_PRIORITY;
   }
   summary.name = lGetString(job, JB_job_name);
   summary.user = lGetString(job, JB_owner);
   if (sge_ext) {
      summary.project = lGetString(job, JB_project);
      summary.department = lGetString(job, JB_department);
   }

   /* move status info into state info */
   jstate = lGetUlong(jatep, JAT_state);
   if (lGetUlong(jatep, JAT_status)==JTRANSFERING) {
      jstate |= JTRANSFERING;
      jstate &= ~JRUNNING;
   }

   if (lGetList(job, JB_jid_predecessor_list) || lGetUlong(jatep, JAT_hold)) {
      jstate |= JHELD;
   }

   if (lGetUlong(jatep, JAT_job_restarted)) {
      jstate &= ~JWAITING;
      jstate |= JMIGRATING;
   }

   job_get_state_string(summary.state, jstate);
   if (summary.is_zombie)
      strncpy(summary.state, "z", 2);

   if (sge_time) {
      summary.submit_time = (time_t)lGetUlong(job, JB_submission_time);
      summary.start_time = (time_t)lGetUlong(jatep, JAT_start_time);
   }
   
   if (lGetUlong(jatep, JAT_status)==JRUNNING || lGetUlong(jatep, JAT_status)==JTRANSFERING) {
      summary.is_running = true;
   } else {
      summary.is_running = false;
   }

   if (sge_urg) {
      summary.deadline = (time_t)lGetUlong(job, JB_deadline);
   }
   
   if (sge_ext) {
      lListElem *up, *pe, *task;
      lList *job_usage_list;
      const char *pe_name;
      bool sum_pe_tasks = false;
      
      if (master == NULL || strcmp(master, "MASTER") == 0) {
         if (!(qstat_env->group_opt & GROUP_NO_PETASK_GROUPS)) {
            sum_pe_tasks = true;
         }
         job_usage_list = lCopyList(NULL, lGetList(jatep, JAT_scaled_usage_list));
      } else {
         job_usage_list = lCreateList("", UA_Type);
      }

      /* sum pe-task usage based on queue slots */
      if (job_usage_list) {
         int subtask_ndx=1;
         for_each(task, lGetList(jatep, JAT_task_list)) {
            lListElem *dst, *src, *ep;
            const char *qname;

            if (sum_pe_tasks ||
                (summary.queue && 
                 ((ep=lFirst(lGetList(task, PET_granted_destin_identifier_list)))) &&
                 ((qname=lGetString(ep, JG_qname))) &&
                 !strcmp(qname, summary.queue) && ((subtask_ndx++%slots)==slot))) {
               for_each(src, lGetList(task, PET_scaled_usage)) {
                  if ((dst=lGetElemStr(job_usage_list, UA_name, lGetString(src, UA_name))))
                     lSetDouble(dst, UA_value, lGetDouble(dst, UA_value) + lGetDouble(src, UA_value));
                  else
                     lAppendElem(job_usage_list, lCopyElem(src));
               }
            }
         }
      }


      /* scaled cpu usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_CPU))) { 
         summary.has_cpu_usage = false;
      } else {
         summary.has_cpu_usage = true;
         summary.cpu_usage = lGetDouble(up, UA_value);
      }
      /* scaled mem usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_MEM))) { 
         summary.has_mem_usage = false;
      } else {
         summary.has_mem_usage = true;
         summary.mem_usage = lGetDouble(up, UA_value);
      }
      /* scaled io usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_IO))) {
         summary.has_io_usage = false;
      } else {
         summary.has_io_usage = true;
         summary.io_usage = lGetDouble(up, UA_value); 
      }
      lFreeList(&job_usage_list);

      /* get tickets for job/slot */
      
      summary.override_tickets = lGetUlong(job, JB_override_tickets);
      summary.share = lGetDouble(jatep, JAT_share);
      if (sge_ext) {
         summary.is_queue_assigned = false;
      } else {
         if (lGetList(jatep, JAT_granted_destin_identifier_list) != NULL) {
            summary.is_queue_assigned = true;
         } else {
            summary.is_queue_assigned = false;
         }
      }

      /* braces needed to suppress compiler warnings */
      if ((pe_name=lGetString(jatep, JAT_granted_pe)) &&
           (pe=pe_list_locate(qstat_env->pe_list, pe_name)) &&
           lGetBool(pe, PE_control_slaves) && slots) {
         if (slot == 0) {
            summary.tickets = (u_long)lGetDouble(gdil_ep, JG_ticket);
            summary.otickets = (u_long)lGetDouble(gdil_ep, JG_oticket);
            summary.ftickets = (u_long)lGetDouble(gdil_ep, JG_fticket);
            summary.stickets = (u_long)lGetDouble(gdil_ep, JG_sticket);
         }
         else {
            if (slots) {
               summary.tickets = (u_long)(lGetDouble(gdil_ep, JG_ticket) / slots);
               summary.otickets = (u_long)(lGetDouble(gdil_ep, JG_oticket) / slots);
               summary.ftickets = (u_long)(lGetDouble(gdil_ep, JG_fticket) / slots);
               summary.stickets = (u_long)(lGetDouble(gdil_ep, JG_sticket) / slots);
            } 
            else {
               summary.tickets = summary.otickets = summary.ftickets = summary.stickets = 0;
            }
         }
      }
      else {
         summary.tickets = (u_long)lGetDouble(jatep, JAT_tix);
         summary.otickets = (u_long)lGetDouble(jatep, JAT_oticket);
         summary.ftickets = (u_long)lGetDouble(jatep, JAT_fticket);
         summary.stickets = (u_long)lGetDouble(jatep, JAT_sticket);
      }

   }

   summary.master = master;
   if (slots_per_line == 0) {
      summary.slots = sge_job_slot_request(job, qstat_env->pe_list);
   } else {
      summary.slots = slots_per_line;
   }
   
   summary.is_array = job_is_array(job);
   
   if (summary.is_array) {
      summary.task_id = sge_dstring_get_string(dyn_task_str); 
   } else {
      summary.task_id = NULL;
   }
   
   if ((ret = handler->report_job(handler, lGetUlong(job, JB_job_number), &summary, alpp))) {
      DPRINTF(("handler->report_job failed\n"));
      goto error;
   }

   if (tsk_ext) {
      lList *task_list = lGetList(jatep, JAT_task_list);
      lListElem *task, *ep;
      const char *qname;
      int subtask_ndx=1;
      
      if (handler->report_sub_tasks_started) {
         if ((ret = handler->report_sub_tasks_started(handler, alpp))) {
            DPRINTF(("(handler->report_sub_tasks_started failed\n"));
            goto error;
         }
      }

      /* print master sub-task belonging to this queue */
      if (!slot && task_list && summary.queue &&
          ((ep=lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)))) &&
          ((qname=lGetString(ep, JG_qname))) &&
          !strcmp(qname, summary.queue)) {
             
          if ((ret=job_handle_subtask(job, jatep, NULL, handler, alpp))) {
             DPRINTF(("sge_handle_subtask failed\n"));
             goto error;
          }      
      }
         
      /* print sub-tasks belonging to this queue */
      for_each(task, task_list) {
         if (!slots || (summary.queue && 
              ((ep=lFirst(lGetList(task, PET_granted_destin_identifier_list)))) &&
              ((qname=lGetString(ep, JG_qname))) &&
              !strcmp(qname, summary.queue) && ((subtask_ndx++%slots)==slot))) {
             if ((ret=job_handle_subtask(job, jatep, task, handler, alpp))) {
                DPRINTF(("sge_handle_subtask failed\n"));
                goto error;
             }      
         }
      }
      
      if (handler->report_sub_tasks_finished) {
         if ((ret=handler->report_sub_tasks_finished(handler, alpp))) {
            DPRINTF(("(handler->report_sub_tasks_finished failed\n"));
            goto error;
         }
      }
      
   } 

   /* print additional job info if requested */
   if ((qstat_env->full_listing & QSTAT_DISPLAY_RESOURCES)) {
      
      if (handler->report_additional_info) {
         if ((ret=handler->report_additional_info(handler, FULL_JOB_NAME, lGetString(job, JB_job_name), alpp))) {
            DPRINTF(("handler->report_additional_info(Full jobname) failed"));
            goto error;
         }
      }
      if (summary.queue && handler->report_additional_info) {
         if ((ret=handler->report_additional_info(handler, MASTER_QUEUE, summary.queue, alpp))) {
            DPRINTF(("handler->report_additional_info(Master queue) failed"));
            goto error;
         }
      }

      if (lGetString(job, JB_pe) && handler->report_requested_pe) {
         dstring range_string = DSTRING_INIT;

         range_list_print_to_string(lGetList(job, JB_pe_range), 
                                    &range_string, true, false, false);
                                    
         ret = handler->report_requested_pe(handler, lGetString(job, JB_pe), sge_dstring_get_string(&range_string), alpp);
                                    
         sge_dstring_free(&range_string);
         
         if (ret) {
            DPRINTF(("handler->report_requested_pe failed\n"));
            goto error;
         }
      }
      
      if (lGetString(jatep, JAT_granted_pe) && handler->report_granted_pe) {
         lListElem *gdil_ep;
         u_long32 pe_slots = 0;
         for_each (gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list)) {
            pe_slots += lGetUlong(gdil_ep, JG_slots);
         }
         
         if ((ret=handler->report_granted_pe(handler, lGetString(jatep, JAT_granted_pe), pe_slots, alpp))) {
            DPRINTF(("handler->report_granted_pe failed\n"));
            goto error;
         }
      }
      if (lGetString(job, JB_checkpoint_name) && handler->report_additional_info) { 
         if ((ret=handler->report_additional_info(handler, CHECKPOINT_ENV, lGetString(job, JB_checkpoint_name), alpp))) {
            DPRINTF(("handler->report_additional_info(CHECKPOINT_ENV) failed\n"));
            goto error;
         }
      }

      /* Handle the Hard Resources */
      ret = job_handle_resources(lGetList(job, JB_hard_resource_list), qstat_env->centry_list, 
                                 sge_job_slot_request(job, qstat_env->pe_list),
                                 handler,
                                 handler->report_hard_resources_started,
                                 handler->report_hard_resource,
                                 handler->report_hard_resources_finished, alpp);
      if (ret) {
         DPRINTF(("handle_resources for hard resources failed\n"));
         goto error;
      }

      /* display default requests if necessary */
      if (handler->report_request) {
         lList *attributes = NULL;
         lListElem *ce;
         const char *name;
         lListElem *hep;

         queue_complexes2scheduler(&attributes, qep, qstat_env->exechost_list, qstat_env->centry_list);
         for_each (ce, attributes) {
            double dval;

            name = lGetString(ce, CE_name);
            if (!lGetUlong(ce, CE_consumable) || !strcmp(name, "slots") || 
                job_get_request(job, name)) {
               continue;
            }

            parse_ulong_val(&dval, NULL, lGetUlong(ce, CE_valtype), lGetString(ce, CE_default), NULL, 0); 
            if (dval == 0.0) {
               continue;
            }

            /* For pending jobs (no queue/no exec host) we may print default request only
               if the consumable is specified in the global host. For running we print it
               if the resource is managed at this node/queue */
            if ((qep && lGetSubStr(qep, CE_name, name, QU_consumable_config_list)) ||
                (qep && (hep=host_list_locate(qstat_env->exechost_list, lGetHost(qep, QU_qhostname))) &&
                 lGetSubStr(hep, CE_name, name, EH_consumable_config_list)) ||
                  ((hep=host_list_locate(qstat_env->exechost_list, SGE_GLOBAL_NAME)) &&
                  lGetSubStr(hep, CE_name, name, EH_consumable_config_list))) {

                     if ((ret=handler->report_request(handler, name, lGetString(ce, CE_default), alpp)) ) {
                        DPRINTF(("handler->report_request failed\n"));
                        break;
                     }
            }
         }
         lFreeList(&attributes);
         if (ret) {
            goto error;
         }
      }
      
      /* Handle the Soft Resources */
      ret = job_handle_resources(lGetList(job, JB_soft_resource_list), qstat_env->centry_list, 
                                 sge_job_slot_request(job, qstat_env->pe_list),
                                 handler,
                                 handler->report_soft_resources_started,
                                 handler->report_soft_resource,
                                 handler->report_soft_resources_finished, alpp);
      if (ret) {
         DPRINTF(("handle_resources for soft resources failed\n"));
         goto error;
      }
      
      if (handler->report_hard_requested_queue) {
         ql = lGetList(job, JB_hard_queue_list);
         if (ql) {
            if (handler->report_hard_requested_queues_started && (ret=handler->report_hard_requested_queues_started(handler, alpp))) {
               DPRINTF(("handler->report_hard_requested_queues_started failed\n"));
               goto error;
            }
            for_each(qrep, ql) {
               if ((ret=handler->report_hard_requested_queue(handler, lGetString(qrep, QR_name), alpp))) {
                  DPRINTF(("handler->report_hard_requested_queue failed\n"));
                  goto error;
               }
            }
            if (handler->report_hard_requested_queues_finished && (ret=handler->report_hard_requested_queues_finished(handler, alpp))) {
               DPRINTF(("handler->report_hard_requested_queues_finished failed\n"));
               goto error;
            }
         }
      }
      
      if (handler->report_soft_requested_queue) {
         ql = lGetList(job, JB_soft_queue_list);
         if (ql) {
            if (handler->report_soft_requested_queues_started && (ret=handler->report_soft_requested_queues_started(handler, alpp))) {
               DPRINTF(("handler->report_soft_requested_queue_started failed\n"));
               goto error;
            }
            for_each(qrep, ql) {
               if ((ret=handler->report_soft_requested_queue(handler, lGetString(qrep, QR_name), alpp))) {
                  DPRINTF(("handler->report_soft_requested_queue failed\n"));
                  goto error;
               }
            }
            if (handler->report_soft_requested_queues_finished && (ret=handler->report_soft_requested_queues_finished(handler, alpp))) {
               DPRINTF(("handler->report_soft_requested_queues_finished failed\n"));
               goto error;
            }
         }
      }
      
      if (handler->report_master_hard_requested_queue) {
         ql = lGetList(job, JB_master_hard_queue_list);
         if (ql){
            if (handler->report_master_hard_requested_queues_started && (ret=handler->report_master_hard_requested_queues_started(handler, alpp))) {
               DPRINTF(("handler->report_master_hard_requested_queues_started failed\n"));
               goto error;
            }
            for_each(qrep, ql) {
               if ((ret=handler->report_master_hard_requested_queue(handler, lGetString(qrep, QR_name), alpp))) {
                  DPRINTF(("handler->report_master_hard_requested_queue failed\n"));
                  goto error;
               }
            }
            if (handler->report_master_hard_requested_queues_finished && (ret=handler->report_master_hard_requested_queues_finished(handler, alpp))) {
               DPRINTF(("handler->report_master_hard_requested_queues_finished failed\n"));
               goto error;
            }
         }
      }

      if (handler->report_predecessor_requested) {
         ql = lGetList(job, JB_jid_request_list );
         if (ql) {
            if (handler->report_predecessors_requested_started && 
                (ret=handler->report_predecessors_requested_started(handler, alpp))) {
               DPRINTF(("handler->report_predecessors_requested_started failed\n"));
               goto error;
            }
            
            for_each(qrep, ql) {
               if ((ret=handler->report_predecessor_requested(handler, lGetString(qrep, JRE_job_name), alpp))) {
                  DPRINTF(("handler->report_predecessor_requested failed\n"));
                  goto error;
               }
            }
            
            if (handler->report_predecessors_requested_finished && 
                (ret=handler->report_predecessors_requested_finished(handler, alpp))) {
               DPRINTF(("handler->report_predecessors_requested_finished failed\n"));
               goto error;
            }
         }
      }
      if (handler->report_predecessor) {
         ql = lGetList(job, JB_jid_predecessor_list);
         if (ql) {
            if (handler->report_predecessors_started && 
                (ret=handler->report_predecessors_started(handler, alpp))) {
               DPRINTF(("handler->report_predecessors_started failed\n"));
               goto error;
            }
            
            for_each(qrep, ql) {
               if ((ret=handler->report_predecessor(handler, lGetUlong(qrep, JRE_job_number), alpp))) {
                  DPRINTF(("handler->report_predecessor failed\n"));
                  goto error;
               }
            }
            if (handler->report_predecessors_finished && 
                (ret=handler->report_predecessors_finished(handler, alpp))) {
               DPRINTF(("handler->report_predecessors_finished failed\n"));
               goto error;
            }
         }
      }

      if (handler->report_ad_predecessor_requested) {
         ql = lGetList(job, JB_ja_ad_request_list );
         if (ql) {
            if (handler->report_ad_predecessors_requested_started && 
                (ret=handler->report_ad_predecessors_requested_started(handler, alpp))) {
               DPRINTF(("handler->report_ad_predecessors_requested_started failed\n"));
               goto error;
            }
            
            for_each(qrep, ql) {
               if ((ret=handler->report_ad_predecessor_requested(handler, lGetString(qrep, JRE_job_name), alpp))) {
                  DPRINTF(("handler->report_ad_predecessor_requested failed\n"));
                  goto error;
               }
            }
            
            if (handler->report_ad_predecessors_requested_finished && 
                (ret=handler->report_ad_predecessors_requested_finished(handler, alpp))) {
               DPRINTF(("handler->report_ad_predecessors_requested_finished failed\n"));
               goto error;
            }
         }
      }
      if (handler->report_ad_predecessor) {
         ql = lGetList(job, JB_ja_ad_predecessor_list);
         if (ql) {
            if (handler->report_ad_predecessors_started && 
                (ret=handler->report_ad_predecessors_started(handler, alpp))) {
               DPRINTF(("handler->report_ad_predecessors_started failed\n"));
               goto error;
            }
            
            for_each(qrep, ql) {
               if ((ret=handler->report_ad_predecessor(handler, lGetUlong(qrep, JRE_job_number), alpp))) {
                  DPRINTF(("handler->report_ad_predecessor failed\n"));
                  goto error;
               }
            }
            if (handler->report_ad_predecessors_finished && 
                (ret=handler->report_ad_predecessors_finished(handler, alpp))) {
               DPRINTF(("handler->report_ad_predecessors_finished failed\n"));
               goto error;
            }
         }
      }
      if (handler->report_binding && (qstat_env->full_listing & QSTAT_DISPLAY_BINDING) != 0) {
         lList *binding_list = lGetList(job, JB_binding);

         if (binding_list != NULL) {
            lListElem *binding_elem = lFirst(binding_list);
            dstring binding_param = DSTRING_INIT;

            binding_print_to_string(binding_elem, &binding_param);
            if (handler->report_binding_started && 
                (ret=handler->report_binding_started(handler, alpp))) {
               DPRINTF(("handler->report_binding_started failed\n"));
               goto error;
            }
            if ((ret=handler->report_binding(handler, sge_dstring_get_string(&binding_param), alpp))) {
               DPRINTF(("handler->report_binding failed\n"));
               goto error;
            }
            if (handler->report_binding_finished && 
                (ret=handler->report_binding_finished(handler, alpp))) {
               DPRINTF(("handler->report_binding_finished failed\n"));
               goto error;
            }
            sge_dstring_free(&binding_param);
         }
      }
   }
   
   if (handler->report_job_finished && (ret=handler->report_job_finished(handler, lGetUlong(job, JB_job_number), alpp))) {
      DPRINTF(("handler->report_job_finished failed\n"));
      goto error;
   }
   
#undef QSTAT_INDENT
#undef QSTAT_INDENT2

error:
   DRETURN(ret);
}


static int job_handle_resources(lList* cel, lList* centry_list, int slots,
                                job_handler_t *handler,
                                int(*start_func)(job_handler_t* handler, lList **alpp),
                                int(*resource_func)(job_handler_t* handler, const char* name, const char* value, double uc, lList **alpp),
                                int(*finish_func)(job_handler_t* handler, lList **alpp),
                                lList **alpp) {                                  
                                               
   int ret = 0;
   const lListElem *ce, *centry;
   const char *s, *name;
   double uc;
   DENTER(TOP_LAYER,"job_handle_requests_from_ce_type");
   
   if (start_func && (ret=start_func(handler, alpp))) {
      DPRINTF(("start_func failed\n"));
      DRETURN(ret);
   }
   /* walk through complex entries */
   for_each (ce, cel) {
      name = lGetString(ce, CE_name);
      if ((centry = centry_list_locate(centry_list, name))) {
         uc = centry_urgency_contribution(slots, name, lGetDouble(ce, CE_doubleval), centry);
      } else {
         uc = 0.0;
      }

      s = lGetString(ce, CE_stringval);
      if ((ret=resource_func(handler, name, s, uc, alpp))) {
         DPRINTF(("resource_func failed\n"));
         break;
      }
   }
   if (ret == 0 && finish_func ) {
      if ((ret=finish_func(handler, alpp))) {
         DPRINTF(("finish_func failed"));
      }
   }
   DRETURN(ret);
}

static int job_handle_subtask(lListElem *job, lListElem *ja_task, lListElem *pe_task,
                              job_handler_t *handler, lList **alpp ) {
   char task_state_string[8];
   u_long32 tstate, tstatus;
   lListElem *ep;
   lList *usage_list;
   lList *scaled_usage_list;
   
   task_summary_t summary;
   int ret = 0;

   DENTER(TOP_LAYER, "job_handle_subtask");

   /* is sub-task logically running */
   if (pe_task == NULL) {
      tstatus = lGetUlong(ja_task, JAT_status);
      usage_list = lGetList(ja_task, JAT_usage_list);
      scaled_usage_list = lGetList(ja_task, JAT_scaled_usage_list);
   } else {
      tstatus = lGetUlong(pe_task, PET_status);
      usage_list = lGetList(pe_task, PET_usage);
      scaled_usage_list = lGetList(pe_task, PET_scaled_usage);
   }

   if (pe_task == NULL) {
      summary.task_id = "";
   } else {
      summary.task_id = lGetString(pe_task, PET_id);
   }

   /* move status info into state info */
   tstate = lGetUlong(ja_task, JAT_state);
   if (tstatus==JRUNNING) {
      tstate |= JRUNNING;
      tstate &= ~JTRANSFERING;
   } else if (tstatus==JTRANSFERING) {
      tstate |= JTRANSFERING;
      tstate &= ~JRUNNING;
   } else if (tstatus==JFINISHED) {
      tstate |= JEXITING;
      tstate &= ~(JRUNNING|JTRANSFERING);
   }

   if (lGetList(job, JB_jid_predecessor_list) || lGetUlong(ja_task, JAT_hold)) {
      tstate |= JHELD;
   }

   if (lGetUlong(ja_task, JAT_job_restarted)) {
      tstate &= ~JWAITING;
      tstate |= JMIGRATING;
   }

   /* write states into string */ 
   job_get_state_string(task_state_string, tstate);
   summary.state = task_state_string;

   {
      lListElem *up;

      /* scaled cpu usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_CPU))) {
         summary.has_cpu_usage = false;
      } else {
         summary.has_cpu_usage = true;
         summary.cpu_usage = lGetDouble(up, UA_value);
      }

      /* scaled mem usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_MEM))) {
         summary.has_mem_usage = false;
      } else {
         summary.has_mem_usage = true;
         summary.mem_usage = lGetDouble(up, UA_value);
      }
  
      /* scaled io usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_IO))) {
         summary.has_io_usage = false;
      } else {
         summary.has_io_usage = true;
         summary.io_usage = lGetDouble(up, UA_value);
      }
   }

   if (tstatus==JFINISHED) {
      ep=lGetElemStr(usage_list, UA_name, "exit_status");
      if (ep) {
         summary.has_exit_status = true;
         summary.exit_status = (int)lGetDouble(ep, UA_value);
      } else {
         summary.has_exit_status = false;
      }
   } else {
      summary.has_exit_status = false;
   }
   
   ret = handler->report_sub_task(handler, &summary, alpp);

   DRETURN(ret);
}

/* ----------- functions from qstat_filter ---------------------------------- */
lCondition *qstat_get_JB_Type_selection(lList *user_list, u_long32 show)
{
   lCondition *jw = NULL;
   lCondition *nw = NULL;

   DENTER(TOP_LAYER, "qstat_get_JB_Type_selection");

   /*
    * Retrieve jobs only for those users specified via -u switch
    */
   {
      lListElem *ep = NULL;
      lCondition *tmp_nw = NULL;

      for_each(ep, user_list) {
         tmp_nw = lWhere("%T(%I p= %s)", JB_Type, JB_owner, lGetString(ep, ST_name));
         if (jw == NULL) {
            jw = tmp_nw;
         } else {
            jw = lOrWhere(jw, tmp_nw);
         }
      }
   }

   /*
    * Select jobs according to current state
    */
   {
      lCondition *tmp_nw = NULL;

      /*
       * Pending jobs (all that are not running) 
       */
      if ((show & QSTAT_DISPLAY_PENDING) == QSTAT_DISPLAY_PENDING) {
         const u_long32 all_pending_flags = (QSTAT_DISPLAY_USERHOLD|QSTAT_DISPLAY_OPERATORHOLD|
                    QSTAT_DISPLAY_SYSTEMHOLD|QSTAT_DISPLAY_JOBARRAYHOLD|QSTAT_DISPLAY_JOBHOLD|
                    QSTAT_DISPLAY_STARTTIMEHOLD|QSTAT_DISPLAY_PEND_REMAIN);
         /*
          * Fine grained stated selection for pending jobs
          * or simply all pending jobs
          */
         if (((show & all_pending_flags) == all_pending_flags) ||
             ((show & all_pending_flags) == 0)) {
            /*
             * All jobs not running (= all pending)
             */
            tmp_nw = lWhere("%T(!(%I -> %T((%I m= %u))))", JB_Type, JB_ja_tasks,
                        JAT_Type, JAT_status, JRUNNING);
            if (nw == NULL) {
               nw = tmp_nw;
            } else {
               nw = lOrWhere(nw, tmp_nw);
            }
            /*
             * Array Jobs with one or more tasks pending
             */
            tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_n_h_ids, 
                        RN_Type, RN_min, 0);
            if (nw == NULL) {
               nw = tmp_nw;
            } else {
               nw = lOrWhere(nw, tmp_nw);
            } 
            tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_u_h_ids, 
                        RN_Type, RN_min, 0);
            if (nw == NULL) {
               nw = tmp_nw;
            } else {
               nw = lOrWhere(nw, tmp_nw);
            } 
            tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_s_h_ids, 
                        RN_Type, RN_min, 0);
            if (nw == NULL) {
               nw = tmp_nw;
            } else {
               nw = lOrWhere(nw, tmp_nw);
            } 
            tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_o_h_ids, 
                        RN_Type, RN_min, 0);
            if (nw == NULL) {
               nw = tmp_nw;
            } else {
               nw = lOrWhere(nw, tmp_nw);
            } 
         } else {
            /*
             * User Hold 
             */
            if ((show & QSTAT_DISPLAY_USERHOLD) == QSTAT_DISPLAY_USERHOLD) {
               /* unenrolled jobs in user hold state ... */
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_u_h_ids, 
                           RN_Type, RN_min, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               /* ... or enrolled jobs with an user  hold */
               tmp_nw = lWhere("%T((%I -> %T(%I m= %u)))", JB_Type,
                               JB_ja_tasks, JAT_Type, JAT_hold, MINUS_H_TGT_USER);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               }
            }
            /*
             * Operator Hold 
             */
            if ((show & QSTAT_DISPLAY_OPERATORHOLD) == QSTAT_DISPLAY_OPERATORHOLD) {
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_o_h_ids, 
                           RN_Type, RN_min, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               tmp_nw = lWhere("%T((%I -> %T(%I m= %u)))", JB_Type,
                               JB_ja_tasks, JAT_Type, JAT_hold, MINUS_H_TGT_OPERATOR);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               }
            }
            /*
             * System Hold 
             */
            if ((show & QSTAT_DISPLAY_SYSTEMHOLD) == QSTAT_DISPLAY_SYSTEMHOLD) {
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_s_h_ids, 
                           RN_Type, RN_min, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               tmp_nw = lWhere("%T((%I -> %T(%I m= %u)))", JB_Type,
                               JB_ja_tasks, JAT_Type, JAT_hold, MINUS_H_TGT_SYSTEM);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               }
            }
            /*
             * Job Array Dependency Hold 
             */
            if ((show & QSTAT_DISPLAY_JOBARRAYHOLD) == QSTAT_DISPLAY_JOBARRAYHOLD) {
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_a_h_ids, 
                           RN_Type, RN_min, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               tmp_nw = lWhere("%T((%I -> %T(%I m= %u)))", JB_Type,
                               JB_ja_tasks, JAT_Type, JAT_hold, MINUS_H_TGT_JA_AD);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               }
            }
            /*
             * Start Time Hold 
             */
            if ((show & QSTAT_DISPLAY_STARTTIMEHOLD) == QSTAT_DISPLAY_STARTTIMEHOLD) {
               tmp_nw = lWhere("%T(%I > %u)", JB_Type, JB_execution_time, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
            }
            /*
             * Job Dependency Hold 
             */
            if ((show & QSTAT_DISPLAY_JOBHOLD) == QSTAT_DISPLAY_JOBHOLD) {
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_jid_predecessor_list, JRE_Type, JRE_job_number, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
            }
            /*
             * Rescheduled and jobs in error state (not in hold/no start time/no dependency) 
             * and regular pending jobs
             */
            if ((show & QSTAT_DISPLAY_PEND_REMAIN) == QSTAT_DISPLAY_PEND_REMAIN) {
               tmp_nw = lWhere("%T(%I -> %T((%I != %u)))", JB_Type, JB_ja_tasks, 
                           JAT_Type, JAT_job_restarted, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               tmp_nw = lWhere("%T(%I -> %T((%I m= %u)))", JB_Type, JB_ja_tasks, 
                           JAT_Type, JAT_state, JERROR);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
               tmp_nw = lWhere("%T(%I -> %T((%I > %u)))", JB_Type, JB_ja_n_h_ids, 
                           RN_Type, RN_min, 0);
               if (nw == NULL) {
                  nw = tmp_nw;
               } else {
                  nw = lOrWhere(nw, tmp_nw);
               } 
            }
         }
      }
      /*
       * Running jobs (which are not suspended) 
       *
       * NOTE: 
       *    This code is not quite correct. It select jobs
       *    which are running and not suspended (qmod -s)
       * 
       *    Jobs which are suspended due to other mechanisms
       *    (suspend on subordinate, thresholds, calendar)
       *    should be rejected too, but this is not possible
       *    because this information is not stored within
       *    job or job array task.
       *    
       *    As a result to many jobs will be requested by qsub.
       */   
      if ((show & QSTAT_DISPLAY_RUNNING) == QSTAT_DISPLAY_RUNNING) {
         tmp_nw = lWhere("%T(((%I -> %T(%I m= %u)) || (%I -> %T(%I m= %u))) && !(%I -> %T((%I m= %u))))", JB_Type, 
                         JB_ja_tasks, JAT_Type, JAT_status, JRUNNING,
                         JB_ja_tasks, JAT_Type, JAT_status, JTRANSFERING,
                         JB_ja_tasks, JAT_Type, JAT_state, JSUSPENDED);
         if (nw == NULL) {
            nw = tmp_nw;
         } else {
            nw = lOrWhere(nw, tmp_nw);
         } 
      }

      /*
       * Suspended jobs
       *
       * NOTE:
       *    see comment above
       */
      if ((show & QSTAT_DISPLAY_SUSPENDED) == QSTAT_DISPLAY_SUSPENDED) {
         tmp_nw = lWhere("%T((%I -> %T(%I m= %u)) || (%I -> %T(%I m= %u)) || (%I -> %T(%I m= %u)))", JB_Type, 
                         JB_ja_tasks, JAT_Type, JAT_status, JRUNNING,
                         JB_ja_tasks, JAT_Type, JAT_status, JTRANSFERING,
                         JB_ja_tasks, JAT_Type, JAT_state, JSUSPENDED);
         if (nw == NULL) {
            nw = tmp_nw;
         } else {
            nw = lOrWhere(nw, tmp_nw);
         } 
      }
   }

   if (nw != NULL) {
      if (jw == NULL) {
         jw = nw;
      } else {
         jw = lAndWhere(jw, nw);
      }
   }

   DRETURN(jw);
}

lEnumeration *qstat_get_JB_Type_filter(qstat_env_t* qstat_env) 
{
   DENTER(TOP_LAYER, "qstat_get_JB_Type_filter");

   if (qstat_env->what_JAT_Type_template != NULL) {
      lWhatSetSubWhat(qstat_env->what_JB_Type, JB_ja_template, &(qstat_env->what_JAT_Type_template));
   }
   if (qstat_env->what_JAT_Type_list != NULL) {
      lWhatSetSubWhat(qstat_env->what_JB_Type, JB_ja_tasks, &(qstat_env->what_JAT_Type_list));
   }

   DRETURN(qstat_env->what_JB_Type);
}


void qstat_filter_add_core_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_job_number,
      JB_owner,
      JB_type,
      JB_pe,
      JB_jid_predecessor_list,
      JB_ja_ad_predecessor_list,
      JB_job_name,
      JB_submission_time,
      JB_pe_range,
      JB_ja_structure,
      JB_ja_tasks,
      JB_ja_n_h_ids,
      JB_ja_u_h_ids,
      JB_ja_o_h_ids,
      JB_ja_s_h_ids,
      JB_ja_a_h_ids,
      JB_ja_z_ids,
      JB_ja_template,
      JB_execution_time,
      JB_hard_queue_list,
      JB_project,
      NoName
   };
   const int nm_JAT_Type_template[] = {
      JAT_task_number,
      JAT_prio,
      JAT_hold,
      JAT_state,
      JAT_status,
      JAT_job_restarted,
      JAT_start_time,
      NoName
   };
   const int nm_JAT_Type_list[] = {
      JAT_task_number,
      JAT_status,
      JAT_granted_destin_identifier_list,
      JAT_suitable,
      JAT_granted_pe,
      JAT_state,
      JAT_prio,
      JAT_hold,
      JAT_job_restarted,
      JAT_start_time,
      NoName
   }; 
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);

   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_template); 
   lMergeWhat(&(qstat_env->what_JAT_Type_template), &tmp_what);
   
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_list); 
   lMergeWhat(&(qstat_env->what_JAT_Type_list), &tmp_what);
}

void qstat_filter_add_ext_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_department,
      JB_override_tickets,
      NoName
   };
   const int nm_JAT_Type_template[] = {
      JAT_ntix,
      JAT_scaled_usage_list,
      JAT_granted_pe,
      JAT_tix,
      JAT_oticket,
      JAT_fticket,
      JAT_sticket,
      JAT_share,
      NoName
   };
   const int nm_JAT_Type_list[] = {
      JAT_ntix,
      JAT_scaled_usage_list,
      JAT_task_list,
      JAT_tix,
      JAT_oticket,
      JAT_fticket,
      JAT_sticket,
      JAT_share,
      NoName
   };
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
   
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_template); 
   lMergeWhat(&(qstat_env->what_JAT_Type_template), &tmp_what);
   
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_list); 
   lMergeWhat(&(qstat_env->what_JAT_Type_list), &tmp_what);
}

void qstat_filter_add_pri_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_nppri,
      JB_nurg,
      JB_priority,
      NoName
   };
   const int nm_JAT_Type_template[] = {
      JAT_ntix,
      NoName
   };
   const int nm_JAT_Type_list[] = {
      JAT_ntix,
      NoName
   };

   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_template); 
   lMergeWhat(&(qstat_env->what_JAT_Type_template), &tmp_what);
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_list); 
   lMergeWhat(&(qstat_env->what_JAT_Type_list), &tmp_what);
}

void qstat_filter_add_urg_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_deadline,
      JB_nurg,
      JB_urg,
      JB_rrcontr,
      JB_dlcontr,
      JB_wtcontr,
      NoName
   };
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}

void qstat_filter_add_l_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_checkpoint_name,
      JB_hard_resource_list,
      JB_soft_resource_list,
      NoName
   };
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}

void qstat_filter_add_pe_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_checkpoint_name,
      JB_hard_resource_list,
      JB_soft_resource_list,
      NoName
   };
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}


void qstat_filter_add_q_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_checkpoint_name,
      JB_hard_resource_list,
      NoName
   };
   
   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}

void qstat_filter_add_r_attributes(qstat_env_t *qstat_env) {
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_checkpoint_name,
      JB_hard_resource_list,
      JB_soft_resource_list,
      JB_hard_queue_list,
      JB_soft_queue_list,
      JB_master_hard_queue_list,
      JB_jid_request_list,
      JB_ja_ad_request_list,
      JB_binding,
      NoName
   };
   const int nm_JAT_Type_template[] = {
      JAT_granted_pe,
      NoName
   };

   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);

   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_template); 
   lMergeWhat(&(qstat_env->what_JAT_Type_template), &tmp_what);
}

void qstat_filter_add_xml_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   const int nm_JB_Type[] = {
      JB_jobshare,
      NoName
   };

   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}

void qstat_filter_add_U_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;

   const int nm_JB_Type[] = {
      JB_checkpoint_name,
      JB_hard_resource_list,
      NoName
   };

   tmp_what = lIntVector2What(JB_Type, nm_JB_Type);
   lMergeWhat(&(qstat_env->what_JB_Type), &tmp_what);
}

void qstat_filter_add_t_attributes(qstat_env_t *qstat_env) 
{
   lEnumeration *tmp_what = NULL;
   
   const int nm_JAT_Type_list[] = {
      JAT_task_list,
      JAT_usage_list,
      JAT_scaled_usage_list,
      NoName
   };

   const int nm_JAT_Type_template[] = {
      JAT_task_list,
      JAT_usage_list,
      JAT_scaled_usage_list,
      NoName
   };
   
   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_list); 
   lMergeWhat(&(qstat_env->what_JAT_Type_list), &tmp_what);

   tmp_what = lIntVector2What(JAT_Type, nm_JAT_Type_template); 
   lMergeWhat(&(qstat_env->what_JAT_Type_template), &tmp_what); 
}

const char* sge_get_dominant_stringval(lListElem *rep, u_long32 *dominant_p, dstring *resource_string_p)
{ 
   const char *s = NULL;
   u_long32 type = lGetUlong(rep, CE_valtype);

   DENTER(TOP_LAYER, "sge_get_dominant_stringval");

   switch (type) {
   case TYPE_HOST:   
   case TYPE_STR:   
   case TYPE_CSTR:  
   case TYPE_RESTR:
      if (!(lGetUlong(rep, CE_pj_dominant)&DOMINANT_TYPE_VALUE)) {
         *dominant_p = lGetUlong(rep, CE_pj_dominant);
         s = lGetString(rep, CE_pj_stringval);
      } else {
         *dominant_p = lGetUlong(rep, CE_dominant);
         s = lGetString(rep, CE_stringval);
      }
      break;
   case TYPE_TIM: 

      if (!(lGetUlong(rep, CE_pj_dominant)&DOMINANT_TYPE_VALUE)) {
         double val = lGetDouble(rep, CE_pj_doubleval);

         *dominant_p = lGetUlong(rep, CE_pj_dominant);
         double_print_time_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      } else {
         double val = lGetDouble(rep, CE_doubleval);

         *dominant_p = lGetUlong(rep, CE_dominant);
         double_print_time_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      }
      break;
   case TYPE_MEM:

      if (!(lGetUlong(rep, CE_pj_dominant)&DOMINANT_TYPE_VALUE)) {
         double val = lGetDouble(rep, CE_pj_doubleval);

         *dominant_p = lGetUlong(rep, CE_pj_dominant);
         double_print_memory_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      } else {
         double val = lGetDouble(rep, CE_doubleval);

         *dominant_p = lGetUlong(rep, CE_dominant);
         double_print_memory_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      }
      break;
   case TYPE_INT:
      if (!(lGetUlong(rep, CE_pj_dominant)&DOMINANT_TYPE_VALUE)) {
         double val = lGetDouble(rep, CE_pj_doubleval);
         *dominant_p = lGetUlong(rep, CE_pj_dominant);
         double_print_int_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      } else {
         double val = lGetDouble(rep, CE_doubleval);

         *dominant_p = lGetUlong(rep, CE_dominant);
         double_print_int_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      }
      break;
   default:   

      if (!(lGetUlong(rep, CE_pj_dominant)&DOMINANT_TYPE_VALUE)) {
         double val = lGetDouble(rep, CE_pj_doubleval);

         *dominant_p = lGetUlong(rep, CE_pj_dominant);
         double_print_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      } else {
         double val = lGetDouble(rep, CE_doubleval);

         *dominant_p = lGetUlong(rep, CE_dominant);
         double_print_to_dstring(val, resource_string_p);
         s = sge_dstring_get_string(resource_string_p);
      }
      break;
   }

   DRETURN(s);
}


/*-------------------------------------------------------------------------*
 * NAME
 *   build_job_state_filter - set the full_listing flags in the qstat_env
 *                            according to job_state
 *
 * PARAMETER
 *  qstat_env - the qstat_env
 *  job_state - the job_state
 *  alpp      - answer list for error reporting
 *
 * RETURN
 *  0    - full_listing flags in qstat_env set
 *  else - error, reason has been reported in alpp.
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
int build_job_state_filter(qstat_env_t *qstat_env, const char* job_state, lList **alpp) {
   int ret = 0;
   
   DENTER(TOP_LAYER, "build_job_state_filter");

   if (job_state != NULL) {
      /* 
       * list of options for the -s switch
       * when you add options, make sure that single byte options (e.g. "h")
       * come after multi byte options starting with the same character (e.g. "hs")!
       */
      static char* flags[] = {
         "hu", "hs", "ho", "hd", "hj", "ha", "h", "p", "r", "s", "z", "a", NULL
      };
      static u_long32 bits[] = {
         (QSTAT_DISPLAY_USERHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_SYSTEMHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_OPERATORHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_JOBARRAYHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_JOBHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_STARTTIMEHOLD|QSTAT_DISPLAY_PENDING), 
         (QSTAT_DISPLAY_HOLD|QSTAT_DISPLAY_PENDING), 
         QSTAT_DISPLAY_PENDING,
         QSTAT_DISPLAY_RUNNING, 
         QSTAT_DISPLAY_SUSPENDED, 
         QSTAT_DISPLAY_ZOMBIES,
         (QSTAT_DISPLAY_PENDING|QSTAT_DISPLAY_RUNNING|QSTAT_DISPLAY_SUSPENDED),
         0 
      };
      int i;
      const char *s;
      u_long32 rm_bits = 0;
      
      /* initialize bitmask */
      for (i =0 ; flags[i] != 0; i++) {
         rm_bits |= bits[i];
      }
      qstat_env->full_listing &= ~rm_bits;

      /* 
       * search each 'flag' in argstr
       * if we find the whole string we will set the corresponding 
       * bits in '*qstat_env->full_listing'
       */
      s = job_state;
      while (*s != '\0') {
         bool matched = false;
         for (i = 0; flags[i] != NULL; i++) {
            if (strncmp(s, flags[i], strlen(flags[i])) == 0) {
               qstat_env->full_listing |= bits[i];
               s += strlen(flags[i]);
               matched = true;
            }
         }

         if (!matched) {
            answer_list_add_sprintf(alpp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                                    "%s", MSG_OPTIONS_WRONGARGUMENTTOSOPT); 
            ret = -1;
            break;
         }
      }
   }

   DRETURN(ret);
}

static void print_qstat_env_to(qstat_env_t *qstat_env, FILE* file) {

   lInit(nmv);
   fprintf(file, "======================================================\n");
   fprintf(file, "QSTAT_ENV\n");
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "resource_list:\n");
   if (qstat_env->resource_list != NULL) {
      lWriteListTo(qstat_env->resource_list, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "qresource_list:\n");
   if (qstat_env->resource_list != NULL) {
      lWriteListTo(qstat_env->resource_list, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "queueref_list:\n");
   if (qstat_env->resource_list != NULL) {
      lWriteListTo(qstat_env->resource_list, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "user_list:\n");
   if (qstat_env->resource_list != NULL) {
      lWriteListTo(qstat_env->resource_list, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "what_JB_Type:\n");
   if (qstat_env->what_JB_Type != NULL) {
      lWriteWhatTo(qstat_env->what_JB_Type, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "what_JAT_Type_template:\n");
   if (qstat_env->what_JAT_Type_template != NULL) {
      lWriteWhatTo(qstat_env->what_JAT_Type_template, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "what_JAT_Type_list:\n");
   if (qstat_env->what_JAT_Type_list != NULL) {
      lWriteWhatTo(qstat_env->what_JAT_Type_list, stdout);
   } else {
      fprintf(file, "NULL\n");
   }
   fprintf(file, "------------------------------------------------------\n");
   fprintf(file, "Params:\n");
   fprintf(file,"full_listing = %x\n", (int)qstat_env->full_listing);
   {
      int masks [] = {
         QSTAT_DISPLAY_EXTENDED,
         QSTAT_DISPLAY_RESOURCES,
         QSTAT_DISPLAY_QRESOURCES,
         QSTAT_DISPLAY_TASKS,
         QSTAT_DISPLAY_NOEMPTYQ,
         QSTAT_DISPLAY_PENDING,
         QSTAT_DISPLAY_SUSPENDED,
         QSTAT_DISPLAY_RUNNING,
         QSTAT_DISPLAY_FINISHED,
         QSTAT_DISPLAY_ZOMBIES,
         QSTAT_DISPLAY_ALARMREASON,
         QSTAT_DISPLAY_USERHOLD,
         QSTAT_DISPLAY_SYSTEMHOLD,
         QSTAT_DISPLAY_OPERATORHOLD,
         QSTAT_DISPLAY_JOBARRAYHOLD,
         QSTAT_DISPLAY_JOBHOLD,
         QSTAT_DISPLAY_STARTTIMEHOLD,
         QSTAT_DISPLAY_URGENCY,
         QSTAT_DISPLAY_PRIORITY,
         QSTAT_DISPLAY_PEND_REMAIN
      };
      
      const char* text [] = {
         "QSTAT_DISPLAY_EXTENDED",
         "QSTAT_DISPLAY_RESOURCES",
         "QSTAT_DISPLAY_QRESOURCES",
         "QSTAT_DISPLAY_TASKS",
         "QSTAT_DISPLAY_NOEMPTYQ",
         "QSTAT_DISPLAY_PENDING",
         "QSTAT_DISPLAY_SUSPENDED",
         "QSTAT_DISPLAY_RUNNING",
         "QSTAT_DISPLAY_FINISHED",
         "QSTAT_DISPLAY_ZOMBIES",
         "QSTAT_DISPLAY_ALARMREASON",
         "QSTAT_DISPLAY_USERHOLD",
         "QSTAT_DISPLAY_SYSTEMHOLD",
         "QSTAT_DISPLAY_OPERATORHOLD",
         "QSTAT_DISPLAY_JOBARRAYHOLD",
         "QSTAT_DISPLAY_JOBHOLD",
         "QSTAT_DISPLAY_STARTTIMEHOLD",
         "QSTAT_DISPLAY_URGENCY",
         "QSTAT_DISPLAY_PRIORITY",
         "QSTAT_DISPLAY_PEND_REMAIN",
         NULL
      };
      int i=0;
      
      while (text[i] != NULL) {
         if (qstat_env->full_listing & masks[i]) {
            fprintf(file,"              =  %s\n", text[i]);
         }
         i++;
      }
   }
   fprintf(file, "qselect_mode = %x\n", (int)qstat_env->qselect_mode);
   fprintf(file, "group_opt = %x\n", (int)qstat_env->group_opt);
   fprintf(file, "queue_state = %x\n", (int)qstat_env->queue_state);
   fprintf(file, "explain_bits = %x\n", (int)qstat_env->explain_bits);
   fprintf(file, "job_info = %x\n", (int)qstat_env->job_info);
   fprintf(file, "======================================================\n");
   
   
}

