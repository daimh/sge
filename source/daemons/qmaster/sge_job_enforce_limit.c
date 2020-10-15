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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"
#include "sge.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_time.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_lock.h"

#include "gdi/sge_gdi_ctx.h"

#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_conf.h"

#include "sge_qmaster_timed_event.h"
#include "sge_job_enforce_limit.h"
#include "sge_reporting_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_host_qmaster.h"
#include "dispatcher.h" /* execd header file, but only need for timeout define */

#include "msg_common.h"
#include "msg_qmaster.h"

static bool is_module_enabled(void) 
{
   static bool old_setting = false;
   bool ret = mconf_get_enable_enforce_master_limit();

   /*
    * if new setting is false and old one is true then delete old one-time events
    */
   if (ret == false && old_setting == true) {
      te_delete_all_one_time_events(TYPE_ENFORCE_LIMIT_EVENT);
   }
   old_setting = ret;
   return ret;
}


/****** qmaster/qmaster-execd/sge_host_add_remove_enforce_limit_trigger() ******
*  NAME
*     sge_host_add_remove_enforce_limit_trigger() -- add/remove a trigger 
*
*  SYNOPSIS
*     static void 
*     sge_host_add_remove_enforce_limit_trigger(const char *hostname, 
*                                               bool add) 
*
*  FUNCTION
*     This functions adds events to the list of events within the timer thread. 
*     Each of those events is sent when the hard wallclock limit of a job is 
*     reached. Trigger events will only be registered for those jobs which
*     are currently running on the host provided via "hostname".
*  
*     Additionally tasks of a pe job currently running on the host with
*     "hostname" will be tagged. The protorcol would otherwise assume that
*     hosts where the pe tasks are running would send reports about the
*     final usage when the pe task ends. Pe jobs would then stuck in the
*     queue on that host endlessly. 
*
*     When the event is triggered the function sge_job_enfoce_limit_handler()
*     is executed.
*
*     If "add" is false then all steps are reversed.
*
*     This function does nothing if "ENABLE_ENFORCE_MASTER_LIMIT" is not
*     defined as qmaster_param or set to false.
*
*     The functions sge_host_add_enforce_limit_trigger() and
*     sge_host_remove_enforce_limit_trigger() are wrapper function 
*     for this function.
*
*  INPUTS
*     const char *hostname - hostname of a host which is not contactable
*                            anymore. 
*     bool add             - true or false depending whether a trigger
*                            should be added or removed 
*
*  RESULT
*     static void - NONE
*
*  NOTES
*     MT-NOTE: sge_host_add_remove_enforce_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_job_enfoce_limit_handler
*     qmaster/qmaster-execd/sge_host_add_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_remove_enforce_limit_trigger() 
*******************************************************************************/
static void 
sge_host_add_remove_enforce_limit_trigger(const char *hostname, bool add) 
{
   lListElem *job;
   lListElem *ja_task;

   DENTER(TOP_LAYER, "sge_host_add_remove_enforce_limit_trigger");

   /*
    * is the limit enforcment module enabled?
    */
   if (is_module_enabled()) {

      /*
       * Add/Remove a timer for those jobs/ja_tasks currently running on the given host 
       * and remove a flag which prevents the qmaster<->execd protocol from waiting
       * for a certain pe task waiting on that host
       */ 
      for_each (job, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         for_each (ja_task, lGetList(job, JB_ja_tasks)) {
            bool do_action = false;
            lList *gdil = lGetList(ja_task, JAT_granted_destin_identifier_list);
            lListElem *gdil_ep = lFirst(gdil);

            /* 
             * Is the job really running?
             */
            if (gdil_ep != NULL) { 
               
               /*
                * Either we got a hostname. Than we can add triggers for jobs running on that host.
                * Or we get NULL as hostname. In that case we have to check the qinstance state where the
                * job is running.
                */
               if (hostname != NULL) {
                  /*
                   * check if there is a need to trigger a limit enforcement. This is the case if
                   *
                   *    - the ja_task (master task pf a pe job) is running on the host we received or if
                   *    - one of the pe_tasks part of the ja_task is running on that host
                   *
                   * in these cases "do_action" will be set to "true"
                   */
                  if (sge_hostcmp(lGetHost(gdil_ep, JG_qhostname), hostname) == 0) {
                     do_action = true;
                  } else {
                     lListElem *pe_task;

                     for_each (pe_task, lGetList(ja_task, JAT_task_list)) {
                        lList *gdil = lGetList(pe_task, PET_granted_destin_identifier_list);
                        lListElem *gdil_ep = lFirst(gdil);
         
                        if (gdil_ep != NULL && sge_hostcmp(lGetHost(gdil_ep, JG_qhostname), hostname) == 0) {
                           do_action = true;
                           break;
                        }
                     }
                  }
               } else {
                  lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
                  lListElem *qinstance = cqueue_list_locate_qinstance(master_cqueue_list,
                                                                      lGetString(gdil_ep, JG_qname));
           
                  /*
                   * is master queue or at least one of the slave queues in unknown state?
                   */ 
                  if (qinstance != NULL) {
                     if (qinstance_state_is_unknown(qinstance) == true) {
                        do_action = true;
                     } else {
                        lListElem *pe_task;

                        for_each (pe_task, lGetList(ja_task, JAT_task_list)) {
                           lList *gdil = lGetList(pe_task, PET_granted_destin_identifier_list);
                           lListElem *gdil_ep;

                           for_each(gdil_ep, gdil) {
                              qinstance = cqueue_list_locate_qinstance(master_cqueue_list, lGetString(gdil_ep, JG_qname));
                              if (qinstance != NULL && qinstance_state_is_unknown(qinstance)) {
                                 do_action = true;
                                 break;
                              }
                           }
                        }
                     }
                  }
               }

               /*
                * add/remove the trigger event
                */
               if (do_action) {
                  if (add) {
                     sge_job_add_enforce_limit_trigger(job, ja_task);
                  } else {
                     u_long32 job_id = lGetUlong(job, JB_job_number);
                     u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number); 

                     sge_job_remove_enforce_limit_trigger(job_id, ja_task_id);
                  }
               } 
            }
         }
      } 
   }
   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_add_check_limit_trigger() *****************
*  NAME
*     sge_add_check_limit_trigger() -- check limits for unknown host 
*
*  SYNOPSIS
*     void sge_add_check_limit_trigger(void) 
*
*  FUNCTION
*     Add a timer thread event trigger which will be fired when the
*     double of the maximum of all load report intervals of all execution
*     hosts is reached. 
*
*     When the trigger function is executed then all jobs running on hosts
*     still in unknwon state will be checked how long they have till the
*     corresponding runtime limit is reached. In that case an additional 
*     trigger event is registered which will be fired, when the real limit
*     is reached.
*
*     Please note that the same trigger function is used for job specific 
*     triggers and for limit triggers of all jobs running on unknown hosts.
*     The only difference is that job and ja_task == 0 will be passed to
*     sge_job_enfoce_limit_handler().
*
*  INPUTS
*     void - NONE 
*
*  NOTES
*     MT-NOTE: sge_add_check_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_job_enfoce_limit_handler() 
*******************************************************************************/
void
sge_add_check_limit_trigger(void)
{
   lList *master_host_list = *(object_type_get_master_list(SGE_TYPE_EXECHOST));
   u_long32 now = sge_get_gmt();
   u_long32 max_time = 0;
   u_long32 reconnect_timeout = EXECD_MAX_RECONNECT_TIMEOUT;
   lListElem *host;
   te_event_t ev;

   DENTER(TOP_LAYER, "sge_add_check_limit_trigger");

   for_each (host, master_host_list) {
      max_time = MAX(max_time,  2 * load_report_interval(host));
   }

   ev = te_new_event((time_t)(now + max_time + reconnect_timeout),
                     TYPE_ENFORCE_LIMIT_EVENT, ONE_TIME_EVENT,
                     0, 0, NULL);

   te_add_event(ev);
   te_free_event(&ev);

   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_job_enfoce_limit_handler() *******************
*  NAME
*     sge_job_enfoce_limit_handler() -- enforces wallclock limits in master 
*
*  SYNOPSIS
*     void 
*     sge_job_enfoce_limit_handler(sge_gdi_ctx_class_t *ctx, 
*                                  te_event_t event, monitoring_t *monitor) 
*
*  FUNCTION
*     This handler is triggered by the timed event thread when the 
*     hr_t limit of a job is reached. The job and ja_task id are provided
*     as numeric parameters. 
*
*     The function will check if the job is still running and if the
*     host where the job is running is still in unknwon state. Before the
*     function terminates the job like "qdel -f" the data structures are
*     manipulated in a way so that the online unsage of the job which was
*     reported in the past will be written to accounting records.
*
*     To add a event which will trigger the execution of this function
*     sge_host_add_enforce_limit_trigger() can be used. Events which have 
*     been added can be removed by sge_host_remove_enforce_limit_trigger().
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context object 
*     te_event_t event         - timed event structure 
*     monitoring_t *monitor    - monitoring object 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_job_enfoce_limit_handler() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_host_add_remove_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_add_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_remove_enforce_limit_trigger() 
*******************************************************************************/
void 
sge_job_enfoce_limit_handler(sge_gdi_ctx_class_t *ctx, te_event_t event, monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_job_enfoce_limit_handler");

   if (is_module_enabled()) {
      u_long32 job_id = te_get_first_numeric_key(event);
      u_long32 ja_task_id = te_get_second_numeric_key(event);

      MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

      /*
       * Either we will receive a valid job and task id or the value 0 for both.
       * Valid ids mean that we have to handle a certain job or task
       * 0 for both means that some time has been elapsed since the master process has been started
       * and now we have to check whether a host in still in "unknwon" state.
       */
      if (job_id == 0 && ja_task_id == 0) {
         sge_host_add_enforce_limit_trigger(NULL);
      } else {
         lList *master_job_list = *(object_type_get_master_list(SGE_TYPE_JOB));
         lListElem *job = job_list_locate(master_job_list, job_id);
         lListElem *ja_task = job_search_task(job, NULL, ja_task_id);

         /*
          * does the job and the task structure still exist. The job might have been deleted by qdel -f
          */
         if (job != NULL && ja_task != NULL) {
            lList *gdil = lGetList(ja_task, JAT_granted_destin_identifier_list); 
            lListElem *gdil_ep = lFirst(gdil);

            /*
             * is it a running job?
             */
            if (gdil_ep != NULL) {
               bool do_action = false;
               u_long32 now = sge_get_gmt();
               lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
               lList *master_pe_list = *(object_type_get_master_list(SGE_TYPE_PE));
               lListElem *qinstance = cqueue_list_locate_qinstance(master_cqueue_list,
                                                                   lGetString(gdil_ep, JG_qname));

               /*
                * is one of the queues still in unknown state where the job is running
                */
               if (qinstance != NULL) {
                  if (qinstance_state_is_unknown(qinstance) == true) {
                     do_action = true;
                  } else {
                     /*
                      * Accounting for tight pe tasks running on unknown hosts
                      */
                     if (job_is_tight_parallel(job, master_pe_list) == true) {
                        lList *pe_tasks = lGetList(ja_task, JAT_task_list);
                        lListElem *pe_task;

                        for_each(pe_task, pe_tasks) {
                           lList *gdil = NULL;
                           lListElem *gdil_ep;
                           lListElem *qinstance;

                           gdil = lGetList(pe_task, PET_granted_destin_identifier_list);
                           gdil_ep = lFirst(gdil);
                           if (gdil_ep != NULL) {
                              qinstance = cqueue_list_locate_qinstance(master_cqueue_list, lGetString(gdil_ep, JG_qname));

                              if (qinstance != NULL && qinstance_state_is_unknown(qinstance) == true) {
                                 do_action = true;   
                                 break;
                              }
                           }
                        }
                     }
                  }
               }

               if (do_action) {
                  lListElem *dummy_jr;

                  /*
                   * Accounting for tight pe tasks running on unknown hosts
                   */
                  if (job_is_tight_parallel(job, master_pe_list) == true) {
                     lList *pe_task_list = lGetList(ja_task, JAT_task_list);
                     lListElem *pe_task;

                     for_each (pe_task, pe_task_list) {
                        if (lGetBool(pe_task, PET_do_contact) == false) {
                           lListElem *dummy_jr = lCreateElem(JR_Type);

                           job_report_init_from_job_with_usage(dummy_jr, job, ja_task, pe_task, now);
                           reporting_create_acct_record(ctx, NULL, dummy_jr, job, ja_task, false);
                           lFreeElem(&dummy_jr);
                        } 
                     }
                  }

                  /*
                   * Accounting for the job itself
                   */
                  dummy_jr = lCreateElem(JR_Type);
                  job_report_init_from_job_with_usage(dummy_jr, job, ja_task, NULL, now);
                  reporting_create_acct_record(ctx, NULL, dummy_jr, job, ja_task, false);
                  lFreeElem(&dummy_jr);
                  reporting_create_job_log(NULL, now, JL_DELETED, MSG_SCHEDD, 
                                           lGetHost(gdil_ep, JG_qhostname), 
                                           NULL, job, ja_task, NULL, MSG_LOG_DELFORCED);

                  /*
                   * Assassinate the job (qdel -f)
                   */
                  sge_commit_job(ctx, job, ja_task, NULL, COMMIT_ST_FINISHED_FAILED_EE, 
                                 COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
                  job = NULL;
                  ja_task = NULL;

                  /*
                   * Cleanup
                   */
                  INFO((SGE_EVENT, MSG_JOB_TERMJOBDUETOLIMIT_UU, sge_u32c(job_id), sge_u32c(ja_task_id)));
                  cancel_job_resend(job_id, ja_task_id);
               }
            } 
         } 
      }

      SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
   }
   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_host_add_enforce_limit_trigger() *************
*  NAME
*     sge_host_add_enforce_limit_trigger() -- adds trigger events
*
*  SYNOPSIS
*     void sge_host_add_enforce_limit_trigger(const char *hostname) 
*
*  FUNCTION
*     Adds trigger events to the list of events in the timed event thread. 
*     Added events can be removed via sge_host_remove_enforce_limit_trigger().
*
*     Find a more detailed description what this function does in the
*     ADOC comment of sge_host_add_remove_enforce_limit_trigger().
*
*  INPUTS
*     const char *hostname - hostname of a host in unknwon state.
*
*  RESULT
*     void - None 
*
*  NOTES
*     MT-NOTE: sge_host_add_enforce_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_host_add_remove_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_add_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_remove_enforce_limit_trigger() 
*******************************************************************************/
void 
sge_host_add_enforce_limit_trigger(const char *hostname) 
{
   DENTER(TOP_LAYER, "sge_host_add_enforce_limit_trigger");
   sge_host_add_remove_enforce_limit_trigger(hostname, true);
   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_host_remove_enforce_limit_trigger() **********
*  NAME
*     sge_host_remove_enforce_limit_trigger() -- removes trigger events
*
*  SYNOPSIS
*     void sge_host_remove_enforce_limit_trigger(const char *hostname) 
*
*  FUNCTION
*     Removes trigger events from the list of events in the timed event thread. 
*     Events have to be added via sge_host_remove_enforce_limit_trigger()
*     before they can be removed.
*
*     Find a more detailed description what this function does in the
*     ADOC comment of sge_host_add_remove_enforce_limit_trigger().
*
*  INPUTS
*     const char *hostname - hostname of a host which is again in known state 
*
*  RESULT
*     void - None 
*
*  NOTES
*     MT-NOTE: sge_host_remove_enforce_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_host_add_remove_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_add_enforce_limit_trigger() 
*     qmaster/qmaster-execd/sge_host_remove_enforce_limit_trigger() 
*******************************************************************************/
void
sge_host_remove_enforce_limit_trigger(const char *hostname) 
{
   DENTER(TOP_LAYER, "sge_host_remove_enforce_limit_trigger");
   sge_host_add_remove_enforce_limit_trigger(hostname, false);
   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_job_add_enforce_limit_trigger() ************
*  NAME
*     sge_job_add_enforce_limit_trigger() -- add a trigger event for a job 
*
*  SYNOPSIS
*     void 
*     sge_job_add_enforce_limit_trigger(lListElem *job, lListElem *ja_task);
*
*  FUNCTION
*     Adds a trigger event for the given job/task. The job must be
*     already in the transfering or running state.
*
*     The added event will trigger the forced removal of the job when
*     h_rt limit of the job or of one of the pe tasks part of the
*     job is reached. 
*
*     Counterpart for this function is 
*     sge_job_remove_enforce_limit_trigger()
*
*  INPUTS
*     lListElem *job       - job structure 
*     lListElem *ja_task   - job array task structure
*                            the jobs MUST be running!
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_job_add_enforce_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_job_remove_enforce_limit_trigger() 
*******************************************************************************/
void 
sge_job_add_enforce_limit_trigger(lListElem *job, lListElem *ja_task) 
{
   DENTER(TOP_LAYER, "sge_job_add_enforce_limit_trigger");

   /*
    * is the limit enforcment enabled?
    */
   if (is_module_enabled()) {

      /*
       * Find queue instance and job h_rt limits.
       * and add a new timer which will trigger enforcement of limit
       */
      if (job != NULL && ja_task != NULL) {
         lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
         lList *master_pe_list = *(object_type_get_master_list(SGE_TYPE_PE));

         /*
          * If the job is a tightly integrated parallel job than we have to take care
          * that the qmaster<->execd protocol does not try to contact or wait for pe tasks
          * which are running on the host currently changing to unknwon state.
          */
         if (job_is_tight_parallel(job, master_pe_list)) {
            lList *pe_tasks = lGetList(ja_task, JAT_task_list);
            lListElem *pe_task;

            for_each(pe_task, pe_tasks) {
               lList *gdil = NULL;
               lListElem *gdil_ep;
               lListElem *qinstance;

               gdil = lGetList(pe_task, PET_granted_destin_identifier_list);
               gdil_ep = lFirst(gdil);
               if (gdil_ep != NULL) {
                  qinstance = cqueue_list_locate_qinstance(master_cqueue_list, lGetString(gdil_ep, JG_qname));
   
                  if (qinstance != NULL && qinstance_state_is_unknown(qinstance) == true) {
                     lSetBool(pe_task, PET_do_contact, false);
                  }
               }
            }
         }

         /*
          * add the trigger
          */
         {
            u_long32 now = sge_get_gmt();
            u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
            u_long32 job_id = lGetUlong(job, JB_job_number);
            u_long32 duration_offset = sconf_get_duration_offset();
            u_long32 delta_seconds = 0;
            u_long32 already_running = 0;
            bool has_rt_limit = false;

            /*
             * Calculate how long the job still might run till is has to
             * be terminated because of exceeding the wallclock limits
             */
            {
               u_long32 job_h_rt = U_LONG32_MAX;
               u_long32 qi_h_rt = U_LONG32_MAX;
               u_long32 max_running = 0; 

               /*
                * Find the jobs wallclock limit
                */
               {
                  lList *cplxl = lGetList(job, JB_hard_resource_list);
                  lListElem *cple = lGetElemStr(cplxl, CE_name, SGE_ATTR_H_RT);

                  if (cple != NULL) {
                     const char *job_limit = lGetString(cple, CE_stringval);

                     if (job_limit != NULL && strcasecmp(job_limit, "infinity") != 0) {
                        parse_ulong_val(NULL, &job_h_rt, TYPE_TIM, job_limit, NULL, 0);
                        has_rt_limit = true;
                     }
                  }
               }

               /*
                * find the smallest queue limit where the job is running 
                */
               if (has_rt_limit == false) {
                  lList *gdil = lGetList(ja_task, JAT_granted_destin_identifier_list);
                  u_long32 current_qi_h_rt = U_LONG32_MAX;
                  lListElem *gdil_ep;

                  for_each(gdil_ep, gdil) {
                     const char *qname = lGetString(gdil_ep, JG_qname);
                     lListElem *qi = cqueue_list_locate_qinstance(master_cqueue_list, qname);

                     if (qi != NULL) {
                        const char *qi_limit = lGetString(qi, QU_h_rt);

                        if (qi_limit != NULL && strcasecmp(qi_limit, "infinity") != 0) {
                           parse_ulong_val(NULL, &current_qi_h_rt, TYPE_TIM, qi_limit, NULL, 0);
                           has_rt_limit = true;
                           qi_h_rt = MIN(current_qi_h_rt, qi_h_rt);
                        } 
                     }
                  }
               }

               max_running = MIN(qi_h_rt, job_h_rt); 
               already_running = now - lGetUlong(ja_task, JAT_start_time);
               if (already_running <= max_running) {
                  delta_seconds = MAX(max_running - already_running, 0);
               } 
            }

            /*
             * add the event to the timed event thread if there was a limit defined in queue or job
             */
            if (has_rt_limit) {
               te_event_t ev = te_new_event((time_t)(now + delta_seconds + duration_offset), 
                                            TYPE_ENFORCE_LIMIT_EVENT, ONE_TIME_EVENT, 
                                            job_id, ja_task_id, NULL); 
               te_add_event(ev);
               te_free_event(&ev);

               INFO((SGE_EVENT, MSG_JOB_ADDJOBTRIGGER_UUUU, 
                     sge_u32c(job_id), sge_u32c(ja_task_id), 
                     sge_u32c(delta_seconds), sge_u32c(duration_offset)));
            }
         }
      }
   }
   DRETURN_VOID;
}

/****** qmaster/qmaster-execd/sge_job_remove_enforce_limit_trigger() ***********
*  NAME
*     sge_job_remove_enforce_limit_trigger() -- remove a event for a job 
*
*  SYNOPSIS
*     void 
*     sge_job_remove_enforce_limit_trigger(u_long32 job_id, 
*                                          u_long32 ja_task_id) 
*
*  FUNCTION
*     Counterpart for the function sge_job_add_enforce_limit_trigger().
*     Find a detailed description there.
*
*  INPUTS
*     u_long32 job_id     - job id  
*     u_long32 ja_task_id - ja task id 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_job_remove_enforce_limit_trigger() is MT safe 
*
*  SEE ALSO
*     qmaster/qmaster-execd/sge_job_add_enforce_limit_trigger()
*******************************************************************************/
void 
sge_job_remove_enforce_limit_trigger(u_long32 job_id, u_long32 ja_task_id)
{
   lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
   lList *master_job_list = *(object_type_get_master_list(SGE_TYPE_JOB));
   lList *master_pe_list = *(object_type_get_master_list(SGE_TYPE_PE));
   lListElem *job = job_list_locate(master_job_list, job_id);
   lListElem *ja_task = job_search_task(job, NULL, ja_task_id);
   bool delete_trigger = false;

   DENTER(TOP_LAYER, "sge_job_remove_enforce_limit_trigger");

   /*
    * Delete pe task flag which prevents communication with unknwon 
    * hosts in qmaster<->execd protocol
    */
   if (job != NULL && ja_task != NULL) {
      if (job_is_tight_parallel(job, master_pe_list)) {
         lList *pe_tasks = lGetList(ja_task, JAT_task_list);
         lListElem *pe_task;
         bool all_are_known = true;

         for_each (pe_task, pe_tasks) {
            lList *gdil = NULL;
            lListElem *gdil_ep;
            lListElem *qinstance;

            gdil = lGetList(pe_task, PET_granted_destin_identifier_list);
            gdil_ep = lFirst(gdil);
            if (gdil_ep != NULL) {
               qinstance = cqueue_list_locate_qinstance(master_cqueue_list, lGetString(gdil_ep, JG_qname));

               if (qinstance != NULL) {
                  if (qinstance_state_is_unknown(qinstance) == true) {
                     lSetBool(pe_task, PET_do_contact, false);
                     all_are_known = false;
                  } else {
                     lSetBool(pe_task, PET_do_contact, true);
                  }
               }
            }
         }
         if (all_are_known) {
            delete_trigger = true;
         }
      } else {
         delete_trigger = true;
      }
   }

   /*
    * Delete the time which triggers the job removal
    */
   if (delete_trigger) {
      INFO((SGE_EVENT, MSG_JOB_DELJOBTRIGGER_UU, sge_u32c(job_id), sge_u32c(ja_task_id)));
      te_delete_one_time_event(TYPE_ENFORCE_LIMIT_EVENT, job_id, ja_task_id, NULL);
   }

   DRETURN_VOID;
}

