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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "comm/cl_commlib.h"

#include "uti/sge_rmon.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_bootstrap.h"
#include "uti/setup_path.h"
#include "uti/sge_os.h"
#include "uti/sge_stdio.h"
#include "uti/sge_profiling.h"

#include "evm/sge_event_master.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_schedd_conf.h"

#include "gdi/sge_security.h"
#include "gdi/sge_gdi_packet.h"

#include "sched/sge_serf.h"
#include "sched/schedd_monitor.h"

#include "basis_types.h"
#include "sge_sched_job_category.h"
#include "sge_sched_order.h"
#include "sge_thread_main.h"
#include "sge_thread_scheduler.h"
#include "setup_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_sched_process_events.h"

#include "msg_common.h"
#include "msg_qmaster.h"

#define SCHEDULER_TIMEOUT_S 10
#define SCHEDULER_TIMEOUT_N 0

static char schedule_log_path[SGE_PATH_MAX];
static const char *schedule_log_file = "schedule";
static int SGE_TEST_DELAY_SCHEDULING = 0;

master_scheduler_class_t Master_Scheduler = {
   PTHREAD_MUTEX_INITIALIZER,
   false,
   0,
   true 
};


static void schedd_set_serf_log_file(sge_gdi_ctx_class_t *ctx)
{
   const char *cell_root = ctx->get_cell_root(ctx);

   DENTER(TOP_LAYER, "set_schedd_serf_log_path");

   if (!*schedule_log_path) {
      snprintf(schedule_log_path, sizeof(schedule_log_path), "%s/%s/%s", cell_root, "common", schedule_log_file);
      DPRINTF(("schedule log path >>%s<<\n", schedule_log_path));
   }

   DRETURN_VOID;
}

/* MT-NOTE: schedd_serf_record_func() is not MT safe */
static void
schedd_serf_record_func(u_long32 job_id, u_long32 ja_taskid, const char *state,
                        u_long32 start_time, u_long32 end_time,
                        char level_char, const char *object_name,
                        const char *name, double utilization)
{
   FILE *fp;

   DENTER(TOP_LAYER, "schedd_serf_record_func");

   if (!(fp = fopen(schedule_log_path, "a"))) {
      DRETURN_VOID;
   }

   /* a new record */
   fprintf(fp, sge_U32CFormat":"sge_U32CFormat":%s:"sge_U32CFormat":"
           sge_U32CFormat":%c:%s:%s:%f\n", sge_u32c(job_id),
           sge_u32c(ja_taskid), state, sge_u32c(start_time),
           sge_u32c(end_time - start_time),
           level_char, object_name, name, utilization);
   FCLOSE(fp);

   DRETURN_VOID;
FCLOSE_ERROR:
   DPRINTF((MSG_FILE_NOCLOSE_SS, schedule_log_path, strerror(errno)));
   DRETURN_VOID;
}

/* MT-NOTE: schedd_serf_newline() is not MT safe */
static void schedd_serf_newline(u_long32 time)
{
   FILE *fp;

   DENTER(TOP_LAYER, "schedd_serf_newline");
   fp = fopen(schedule_log_path, "a");
   if (fp) {
      /* well, some kind of new line indicating a new schedule run */
      fprintf(fp, "::::::::\n");
      FCLOSE(fp);
   }
   DRETURN_VOID;
FCLOSE_ERROR:
   DPRINTF((MSG_FILE_NOCLOSE_SS, schedule_log_path, strerror(errno)));
   DRETURN_VOID;
}


static void
sge_scheduler_cleanup_monitor(monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_scheduler_cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

static void
sge_scheduler_cleanup_event_client(sge_evc_class_t *evc)
{
   DENTER(TOP_LAYER, "sge_scheduler_cleanup_event_client");
   sge_mirror_shutdown(evc);
   DRETURN_VOID;
}

static void sge_scheduler_wait_for_event(sge_evc_class_t *evc, lList **event_list)
{
   int wait_ret;
   bool do_ack = false;

   DENTER(TOP_LAYER, "sge_scheduler_wait_for_event");


   sge_mutex_lock("event_control_mutex", SGE_FUNC, __LINE__, &Scheduler_Control.mutex);

   if (!Scheduler_Control.triggered) {
      struct timespec ts;
      u_long32 current_time = sge_get_gmt();
      ts.tv_sec = (long) current_time + SCHEDULER_TIMEOUT_S;
      ts.tv_nsec = SCHEDULER_TIMEOUT_N;

      wait_ret = pthread_cond_timedwait(&Scheduler_Control.cond_var, &Scheduler_Control.mutex, &ts);

      /*
       * if pthread_cond_timedwait returns 0, we were triggered by event master
       * otherwise we ran into a timeout or an error
       */
      if (wait_ret != 0) {
         DPRINTF(("pthread_cond_timedwait for events failed %d\n", wait_ret));
      }
   }

   if (Scheduler_Control.triggered) {
      *event_list = Scheduler_Control.new_events;
      Scheduler_Control.new_events = NULL;
      Scheduler_Control.triggered = false;
      do_ack = true;
   }

   sge_mutex_unlock("event_control_mutex", SGE_FUNC, __LINE__, &Scheduler_Control.mutex);

   if (do_ack) {
      if (lGetElemUlong(*event_list, ET_type, sgeE_ACK_TIMEOUT) != NULL) {
         evc->ec_mark4registration(evc);
      }
      evc->ec_ack(evc);
   }

   DRETURN_VOID;
}

/****** qmaster/threads/sge_scheduler_initialize() ***************************
*  NAME
*     sge_scheduler_initialize() -- setup and start the scheduler thread 
*
*  SYNOPSIS
*     void sge_scheduler_initialize(sge_gdi_ctx_class_t *ctx) 
*
*  FUNCTION
*     A call to this function initializes the scheduler thread if it is
*     not already running. 
*
*     The first call to this function (during qmaster qstart) starts
*     the scheduler thread only if it is enabled in the bootstrap file.
*     Otherwise the scheduler will not be started.
*
*     Each subsequent call (triggered from GDI) will definitely start
*     the scheduler thread if it is not running.
*
*     Main routine for the created thread is sge_scheduler_main().
*
*     'Master_Scheduler' is accessed by this function.
*     
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context object 
*     lList **answer_list      - answer list
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_scheduler_initialize() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_scheduler_initialize() 
*     qmaster/threads/sge_scheduler_cleanup_thread() 
*     qmaster/threads/sge_scheduler_terminate() 
*     qmaster/threads/sge_scheduler_main() 
*******************************************************************************/
void 
sge_scheduler_initialize(sge_gdi_ctx_class_t *ctx, lList **answer_list)
{
   DENTER(TOP_LAYER, "sge_scheduler_initialize");

   /* initialize debugging instrumentation */
   {
      char *debug = getenv("SGE_TEST_DELAY_SCHEDULING");
      if (debug != NULL) {
         SGE_TEST_DELAY_SCHEDULING = atoi(debug);
      }
   }

   sge_mutex_lock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

   if (Master_Scheduler.is_running == false) {
      bool start_thread = true;

      /* 
       * when this function is called the first time we will use the setting from
       * the bootstrap file to identify if the scheduler should be started or not
       * otherwise we have to start the thread due to a manual request through GDI.
       * There is no option. We have to start it.
       */
      if (Master_Scheduler.use_bootstrap == true) { 
         start_thread = ((ctx->get_scheduler_thread_count(ctx) > 0) ? true : false);
         Master_Scheduler.use_bootstrap = false; 
      } 

      if (start_thread == true) { 
         cl_thread_settings_t* dummy_thread_p = NULL;
         dstring thread_name = DSTRING_INIT;

         /*
          * initialize the thread pool
          */
         cl_thread_list_setup(&(Main_Control.scheduler_thread_pool), "thread pool");
   
         /* 
          * prepare a unique scheduler thread name for each instance of an scheduler 
          */
         sge_dstring_sprintf(&thread_name, "%s%03d", threadnames[SCHEDD_THREAD], 
                             Master_Scheduler.thread_id);

         /*
          * start the scheduler
          */
         cl_thread_list_create_thread(Main_Control.scheduler_thread_pool, &dummy_thread_p,
                                      cl_com_get_log_list(), sge_dstring_get_string(&thread_name), 
                                      Master_Scheduler.thread_id, sge_scheduler_main, NULL, NULL, CL_TT_SCHEDULER);
         sge_dstring_free(&thread_name);

         /*
          * Increase the thread id so that the next instance of a scheduler will have a 
          * different name and flag that scheduler is running
          */
         Master_Scheduler.thread_id++;
         Master_Scheduler.is_running = true;

         INFO((SGE_EVENT, MSG_THREAD_XHASSTARTED_S, threadnames[SCHEDD_THREAD]));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
      } else {
         INFO((SGE_EVENT, MSG_THREAD_XSTARTDISABLED_S, threadnames[SCHEDD_THREAD]));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
      }
   } else {
      ERROR((SGE_EVENT, MSG_THREAD_XISRUNNING_S, threadnames[SCHEDD_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   }
   sge_mutex_unlock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));
   DRETURN_VOID;
}

/****** qmaster/threads/sge_scheduler_cleanup_thread() ********************
*  NAME
*     sge_scheduler_cleanup_thread() -- cleanup the scheduler thread 
*
*  SYNOPSIS
*     void sge_scheduler_cleanup_thread(void) 
*
*  FUNCTION
*     Cleanup the scheduler thread. 
*
*     This function has to be executed only by the scheduler thread.
*     Ideally it should be the last function executed when the
*     pthread cancelation point is passed. 
*
*     'Master_Scheduler' is accessed by this function.
*     
*  INPUTS
*     void - None 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_scheduler_cleanup_thread() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_scheduler_initialize() 
*     qmaster/threads/sge_scheduler_cleanup_thread() 
*     qmaster/threads/sge_scheduler_terminate() 
*     qmaster/threads/sge_scheduler_main() 
*******************************************************************************/
void
sge_scheduler_cleanup_thread(void *ctx_ref)
{
   DENTER(TOP_LAYER, "sge_scheduler_cleanup_thread");
   
   sge_mutex_lock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

   if (Master_Scheduler.is_running) {
      cl_thread_settings_t* thread = NULL;

      /* 
       * The scheduler thread itself executes this function (sge_scheduler_cleanup_thread())
       * at the cancelation point as part of the cleanup. 
       * Therefore it has to unset the thread config before the
       * cl_thread is deleted. Otherwise we might run into a race condition when logging
       * is used after the call of cl_thread_list_delete_thread_without_join() 
       */
      cl_thread_unset_thread_config();

      /*
       * Delete the scheduler thread but don't wait for termination
       */
      thread = cl_thread_list_get_first_thread(Main_Control.scheduler_thread_pool);
      cl_thread_list_delete_thread_without_join(Main_Control.scheduler_thread_pool, thread);
      
      /* 
       * Trash the thread pool 
       */
      cl_thread_list_cleanup(&Main_Control.scheduler_thread_pool);

      /*
       * now a new scheduler can start
       */
      Master_Scheduler.is_running = false;

      /*
      ** free the ctx too
      */
      sge_gdi_ctx_class_destroy((sge_gdi_ctx_class_t **)ctx_ref);
   }

   sge_mutex_unlock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

   DRETURN_VOID;
}

/****** qmaster/threads/sge_scheduler_terminate() ****************************
*  NAME
*     sge_scheduler_terminate() -- terminate the scheduler 
*
*  SYNOPSIS
*     void sge_scheduler_terminate(sge_gdi_ctx_class_t *ctx) 
*
*  FUNCTION
*     Terminates the scheduler if it was started previousely. This 
*     function will return only when it is sure that the pthread canceled. 
*
*     'Master_Scheduler' is accessed by this function.
*     
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context object 
*     lList **answer_list      - answer list
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_scheduler_terminate() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_scheduler_initialize() 
*     qmaster/threads/sge_scheduler_cleanup_thread() 
*     qmaster/threads/sge_scheduler_terminate() 
*     qmaster/threads/sge_scheduler_main() 
*******************************************************************************/
void
sge_scheduler_terminate(sge_gdi_ctx_class_t *ctx, lList **answer_list)
{
   DENTER(TOP_LAYER, "sge_scheduler_terminate");
   
   sge_mutex_lock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

   if (Master_Scheduler.is_running) {
      pthread_t thread_id;
      cl_thread_settings_t* thread = NULL;

      /* 
       * store thread id to use it later on 
       */
      thread = cl_thread_list_get_first_thread(Main_Control.scheduler_thread_pool);
      thread_id = *(thread->thread_pointer);

      /* 
       * send cancel signal 
       */
      pthread_cancel(thread_id);
     
      /*
       * wakeup scheduler thread which might be blocked by wait for events
       */
      pthread_cond_signal(&Scheduler_Control.cond_var);

      /*
       * cl_thread deletion and cl_thread_pool deletion will be done at 
       * schedulers cancelation point in sge_scheduler_cleanup_thread() ...
       * ... therefore we have nothing more to do.
       */
      ;

      sge_mutex_unlock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

      INFO((SGE_EVENT, MSG_THREAD_XTERMINATED_S, threadnames[SCHEDD_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
   } else {
      sge_mutex_unlock("master scheduler struct", SGE_FUNC, __LINE__, &(Master_Scheduler.mutex));

      ERROR((SGE_EVENT, MSG_THREAD_XNOTRUNNING_S, threadnames[SCHEDD_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   }

   DRETURN_VOID;
}

/****** qmaster/threads/sge_scheduler_main() **********************************
*  NAME
*     sge_scheduler_main() -- main function of the scheduler thread 
*
*  SYNOPSIS
*     void * sge_scheduler_main(void *arg) 
*
*  FUNCTION
*     Main function of the scheduler thread, 
*
*  INPUTS
*     void *arg - pointer to the thread function (type cl_thread_settings_t*) 
*
*  RESULT
*     void * - always NULL 
*
*  NOTES
*     MT-NOTE: sge_scheduler_main() is MT safe 
*
*     MT-NOTE: this is a thread function. Do NOT use this function
*     MT-NOTE: in any other way!
*
*  SEE ALSO
*     qmaster/threads/sge_scheduler_initialize() 
*     qmaster/threads/sge_scheduler_cleanup_thread() 
*     qmaster/threads/sge_scheduler_terminate() 
*     qmaster/threads/sge_scheduler_main() 
*******************************************************************************/
void *
sge_scheduler_main(void *arg)
{
   time_t next_prof_output = 0;
   monitoring_t monitor;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_evc_class_t *evc = NULL;
   lList *alp = NULL;
   sge_where_what_t where_what;
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   bool do_shutdown = false;
   bool do_endlessly = true;
   bool local_ret = true;

   DENTER(TOP_LAYER, "sge_scheduler_main");

   memset(&where_what, 0, sizeof(where_what));

   /*
    * startup
    */
   if (local_ret) {
      /* initialize commlib thread */
      cl_thread_func_startup(thread_config);

      /* initialize monitoring */
      sge_monitor_init(&monitor, thread_config->thread_name, SCH_EXT, SCT_WARNING, SCT_ERROR);
      sge_qmaster_thread_init(&ctx, SCHEDD, SCHEDD_THREAD, true);

      /* register at profiling module */
      set_thread_name(pthread_self(), "Scheduler Thread");
      conf_update_thread_profiling("Scheduler Thread");
      DPRINTF((SFN" started\n", thread_config->thread_name));

      /* initialize schedd_runnlog logging */
      schedd_set_schedd_log_file(ctx);
   }

   /* set profiling parameters */
   prof_set_level_name(SGE_PROF_EVENTMASTER, NULL, NULL);
   prof_set_level_name(SGE_PROF_SPOOLING, NULL, NULL);
   prof_set_level_name(SGE_PROF_CUSTOM0, "scheduler", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM1, "pending ticket calculation", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM3, "job sorting", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM4, "job dispatching", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM5, "send orders", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM6, "scheduler event loop", NULL);
   prof_set_level_name(SGE_PROF_CUSTOM7, "copy lists", NULL);
   prof_set_level_name(SGE_PROF_SCHEDLIB4, NULL, NULL);

   /* set-up needed for 'schedule' file */
   serf_init(schedd_serf_record_func, schedd_serf_newline);
   schedd_set_serf_log_file(ctx);

   /*
    * prepare event client/mirror mechanism
    */
   if (local_ret) {
      local_ret = sge_gdi2_evc_setup(&evc, ctx, EV_ID_SCHEDD, &alp, "scheduler");
      DPRINTF(("prepared event client/mirror mechanism\n"));
   }

   /*
    * register as event mirror
    */
   if (local_ret) {
      sge_mirror_initialize(evc, EV_ID_SCHEDD, "scheduler",
                            false, &event_update_func, &sge_mod_event_client,
                            &sge_add_event_client, &sge_remove_event_client,
                            &sge_handle_event_ack);
      evc->ec_register(evc, false, NULL, &monitor);
      evc->ec_set_busy_handling(evc, EV_BUSY_UNTIL_RELEASED);
      DPRINTF(("registered at event mirror\n"));
   }

   /*
    * subscribe necessary data
    */
   if (local_ret) {
      ensure_valid_what_and_where(&where_what);
      subscribe_scheduler(evc, &where_what);
      DPRINTF(("subscribed necessary data from event master\n"));
   }

   /* 
    * schedulers main loop
    */
   if (local_ret) {
      while (do_endlessly) {
         bool handled_events = false;
         lList *event_list = NULL;
         int execute = 0;
         double prof_copy = 0.0;
         double prof_total = 0.0;
         double prof_init = 0.0;
         double prof_free = 0.0;
         double prof_run = 0.0;
         lList *orders = NULL;

         if (sconf_get_profiling()) {
            prof_start(SGE_PROF_OTHER, NULL);
            prof_start(SGE_PROF_PACKING, NULL);
            prof_start(SGE_PROF_EVENTCLIENT, NULL);
            prof_start(SGE_PROF_MIRROR, NULL);
            prof_start(SGE_PROF_GDI, NULL);
            prof_start(SGE_PROF_HT_RESIZE, NULL);
            prof_start(SGE_PROF_CUSTOM0, NULL);
            prof_start(SGE_PROF_CUSTOM1, NULL);
            prof_start(SGE_PROF_CUSTOM3, NULL);
            prof_start(SGE_PROF_CUSTOM4, NULL);
            prof_start(SGE_PROF_CUSTOM5, NULL);
            prof_start(SGE_PROF_CUSTOM6, NULL);
            prof_start(SGE_PROF_CUSTOM7, NULL);
            prof_start(SGE_PROF_SCHEDLIB4, NULL);
         } else {
            prof_stop(SGE_PROF_OTHER, NULL);
            prof_stop(SGE_PROF_PACKING, NULL);
            prof_stop(SGE_PROF_EVENTCLIENT, NULL);
            prof_stop(SGE_PROF_MIRROR, NULL);
            prof_stop(SGE_PROF_GDI, NULL);
            prof_stop(SGE_PROF_HT_RESIZE, NULL);
            prof_stop(SGE_PROF_CUSTOM0, NULL);
            prof_stop(SGE_PROF_CUSTOM1, NULL);
            prof_stop(SGE_PROF_CUSTOM3, NULL);
            prof_stop(SGE_PROF_CUSTOM4, NULL);
            prof_stop(SGE_PROF_CUSTOM5, NULL);
            prof_stop(SGE_PROF_CUSTOM6, NULL);
            prof_stop(SGE_PROF_CUSTOM7, NULL);
            prof_stop(SGE_PROF_SCHEDLIB4, NULL);
         }

         /*
          * Wait for new events
          */
         MONITOR_IDLE_TIME(sge_scheduler_wait_for_event(evc, &event_list), (&monitor), mconf_get_monitor_time(), 
                           mconf_is_monitor_message());

         /* If we lost connection we have to register again */
         if (evc->ec_need_new_registration(evc)) {
            lFreeList(&event_list);
            if (evc->ec_register(evc, false, NULL, &monitor) == true) {
               DPRINTF(("re-registered at event master!\n"));
            }
         }

         if (event_list != NULL) {
            /* check for shutdown */
            do_shutdown = (lGetElemUlong(event_list, ET_type, sgeE_SHUTDOWN) != NULL) ? true : false;

            /* update mirror and free data */
            if (do_shutdown == false && sge_mirror_process_event_list(evc, event_list) == SGE_EM_OK) {
               handled_events = true;
               DPRINTF(("events handled\n"));
            } else {
               DPRINTF(("events contain shutdown event - ignoring events\n"));
            }
            lFreeList(&event_list);
         }
 
         /* if we actually got events, start the scheduling run and further event processing */
         if (handled_events == true) {
            lList *answer_list = NULL;
            scheduler_all_data_t copy;
            lList *master_cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
            lList *master_job_list = *object_type_get_master_list(SGE_TYPE_JOB);
            lList *master_userset_list = *object_type_get_master_list(SGE_TYPE_USERSET);
            lList *master_project_list = *object_type_get_master_list(SGE_TYPE_PROJECT);
            lList *master_exechost_list= *object_type_get_master_list(SGE_TYPE_EXECHOST);
            lList *master_rqs_list= *object_type_get_master_list(SGE_TYPE_RQS);
            lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);
            lList *master_ckpt_list = *object_type_get_master_list(SGE_TYPE_CKPT);
            lList *master_user_list = *object_type_get_master_list(SGE_TYPE_USER);
            lList *master_ar_list = *object_type_get_master_list(SGE_TYPE_AR);
            lList *master_pe_list = *object_type_get_master_list(SGE_TYPE_PE);
            lList *master_hgrp_list = *object_type_get_master_list(SGE_TYPE_HGROUP);
            lList *master_sharetree_list = *object_type_get_master_list(SGE_TYPE_SHARETREE);

            /* delay scheduling for test purposes, see issue GE-3306 */
            if (SGE_TEST_DELAY_SCHEDULING > 0) {
               sleep(SGE_TEST_DELAY_SCHEDULING);
            }

            PROF_START_MEASUREMENT(SGE_PROF_CUSTOM6);
            PROF_START_MEASUREMENT(SGE_PROF_CUSTOM7);

            if (__CONDITION(INFOPRINT)) {
               dstring ds;
               char buffer[128];

               sge_dstring_init(&ds, buffer, sizeof(buffer));
               DPRINTF(("================[SCHEDULING-EPOCH %s]==================\n",
                        sge_at_time(0, &ds)));
               sge_dstring_free(&ds);
            }

            /*
             * If there were new events then
             * copy/filter data necessary for the scheduler run
             * and run the scheduler method
             */
            memset(&copy, 0, sizeof(copy));

            copy.dept_list = lSelect("", master_userset_list, where_what.where_dept, where_what.what_acldept);
            copy.acl_list = lSelect("", master_userset_list, where_what.where_acl, where_what.what_acldept);

            DPRINTF(("RAW CQ:%d, J:%d, H:%d, C:%d, A:%d, D:%d, P:%d, CKPT:%d,"
                     " US:%d, PR:%d, RQS:%d, AR:%d, S:nd:%d/lf:%d\n",
               lGetNumberOfElem(master_cqueue_list),
               lGetNumberOfElem(master_job_list),
               lGetNumberOfElem(master_exechost_list),
               lGetNumberOfElem(master_centry_list),
               lGetNumberOfElem(copy.acl_list),
               lGetNumberOfElem(copy.dept_list),
               lGetNumberOfElem(master_project_list),
               lGetNumberOfElem(master_ckpt_list),
               lGetNumberOfElem(master_user_list),
               lGetNumberOfElem(master_project_list),
               lGetNumberOfElem(master_rqs_list),
               lGetNumberOfElem(master_ar_list),
               lGetNumberOfNodes(NULL, master_sharetree_list, STN_children),
               lGetNumberOfLeafs(NULL, master_sharetree_list, STN_children)
            ));

            sge_rebuild_job_category(master_job_list, master_userset_list,
                                     master_project_list, master_rqs_list);

            PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM7);
            prof_init = prof_get_measurement_wallclock(SGE_PROF_CUSTOM7, true, NULL);
            PROF_START_MEASUREMENT(SGE_PROF_CUSTOM7);

            sge_before_dispatch(evc);

            /* prepare data for the scheduler itself */
            copy.host_list = lCopyList("", master_exechost_list);

            /*
             * Within the scheduler we do only need QIs
             */
            {
               lListElem *cqueue = NULL;
               lEnumeration *what_queue3 = NULL;

               for_each(cqueue, master_cqueue_list) {
                  lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
                  lList *t;

                  if (!qinstance_list) {
                     continue;
                  }

                  /* all_queue_list contains all queue instances with state and full queue name only */
                  if (!what_queue3) {
                     what_queue3 = lWhat("%T(%I%I)", lGetListDescr(qinstance_list), QU_full_name, QU_state);
                  }
                  t = lSelect("t", qinstance_list, NULL, what_queue3);
                  if (t) {
                     if (copy.all_queue_list == NULL) {
                        copy.all_queue_list = lCreateList("all", lGetListDescr(t));
                     }
                     lAppendList(copy.all_queue_list, t);
                     lFreeList (&t);
                  }

                  t = lSelect("t", qinstance_list, where_what.where_queue, where_what.what_queue2);
                  if (t) {
                     if (copy.queue_list == NULL) {
                        copy.queue_list = lCreateList("enabled", lGetListDescr(t));
                     }
                     lAppendList(copy.queue_list, t);
                     lFreeList (&t);
                  }

                  t = lSelect("t", qinstance_list, where_what.where_queue2, where_what.what_queue2);
                  if (t) {
                     if (copy.dis_queue_list == NULL) {
                        copy.dis_queue_list = lCreateList("disabled", lGetListDescr(t));
                     }
                     lAppendList(copy.dis_queue_list, t);
                     lFreeList (&t);
                  }
               }
               if (what_queue3) {
                  lFreeWhat(&what_queue3);
               }
            }

            if (sconf_is_job_category_filtering()) {
               copy.job_list = sge_category_job_copy(copy.queue_list, &orders, evc->monitor_next_run);
            } else {
               copy.job_list = lCopyList("", master_job_list);
            }

            /* no need to copy these lists, they are read only used */
            copy.centry_list = master_centry_list;
            copy.ckpt_list = master_ckpt_list;
            copy.hgrp_list = master_hgrp_list;

            /* these lists need to be copied because they are modified during scheduling run */
            copy.share_tree = lCopyList("", master_sharetree_list);
            copy.pe_list = lCopyList("", master_pe_list);
            copy.user_list = lCopyList("", master_user_list);
            copy.project_list = lCopyList("", master_project_list);
            copy.rqs_list = lCopyList("", master_rqs_list);
            copy.ar_list = lCopyList("", master_ar_list);

            /* report number of reduced and raw (in brackets) lists */
            DPRINTF(("Q:%d, AQ:%d J:%d(%d), H:%d(%d), C:%d, A:%d, D:%d, P:%d, CKPT:%d,"
                     " US:%d, PR:%d, RQS:%d, AR:%d, S:nd:%d/lf:%d \n",
               lGetNumberOfElem(copy.queue_list),
               lGetNumberOfElem(copy.all_queue_list),
               lGetNumberOfElem(copy.job_list),
               lGetNumberOfElem(master_job_list),
               lGetNumberOfElem(copy.host_list),
               lGetNumberOfElem(master_exechost_list),
               lGetNumberOfElem(copy.centry_list),
               lGetNumberOfElem(copy.acl_list),
               lGetNumberOfElem(copy.dept_list),
               lGetNumberOfElem(copy.pe_list),
               lGetNumberOfElem(copy.ckpt_list),
               lGetNumberOfElem(copy.user_list),
               lGetNumberOfElem(copy.project_list),
               lGetNumberOfElem(copy.rqs_list),
               lGetNumberOfElem(copy.ar_list),
               lGetNumberOfNodes(NULL, copy.share_tree, STN_children),
               lGetNumberOfLeafs(NULL, copy.share_tree, STN_children)
            ));

            if (getenv("SGE_ND")) {
               printf("Q:%d, AQ:%d J:%d(%d), H:%d(%d), C:%d, A:%d, D:%d, "
                  "P:%d, CKPT:%d, US:%d, PR:%d, RQS:%d, AR:%d, S:nd:%d/lf:%d \n",
                  lGetNumberOfElem(copy.queue_list),
                  lGetNumberOfElem(copy.all_queue_list),
                  lGetNumberOfElem(copy.job_list),
                  lGetNumberOfElem(master_job_list),
                  lGetNumberOfElem(copy.host_list),
                  lGetNumberOfElem(master_exechost_list),
                  lGetNumberOfElem(copy.centry_list),
                  lGetNumberOfElem(copy.acl_list),
                  lGetNumberOfElem(copy.dept_list),
                  lGetNumberOfElem(copy.pe_list),
                  lGetNumberOfElem(copy.ckpt_list),
                  lGetNumberOfElem(copy.user_list),
                  lGetNumberOfElem(copy.project_list),
                  lGetNumberOfElem(copy.rqs_list),
                  lGetNumberOfElem(copy.ar_list),
                  lGetNumberOfNodes(NULL, copy.share_tree, STN_children),
                  lGetNumberOfLeafs(NULL, copy.share_tree, STN_children)
                 );
            } else {
               schedd_log("-------------START-SCHEDULER-RUN-------------", NULL, evc->monitor_next_run);
            }

            PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM7);
            prof_copy = prof_get_measurement_wallclock(SGE_PROF_CUSTOM7, true, NULL);
            PROF_START_MEASUREMENT(SGE_PROF_CUSTOM7);

            scheduler_method(evc, &answer_list, &copy, &orders);
            answer_list_output(&answer_list);

            PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM7);
            prof_run = prof_get_measurement_wallclock(SGE_PROF_CUSTOM7, true, NULL);
            PROF_START_MEASUREMENT(SGE_PROF_CUSTOM7);

            /* .. which gets deleted after using */
            lFreeList(&(copy.host_list));
            lFreeList(&(copy.queue_list));
            lFreeList(&(copy.dis_queue_list));
            lFreeList(&(copy.all_queue_list));
            lFreeList(&(copy.job_list));
            lFreeList(&(copy.acl_list));
            lFreeList(&(copy.dept_list));
            lFreeList(&(copy.pe_list));
            lFreeList(&(copy.share_tree));
            lFreeList(&(copy.user_list));
            lFreeList(&(copy.project_list));
            lFreeList(&(copy.rqs_list));
            lFreeList(&(copy.ar_list));

            PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM7);
            prof_free = prof_get_measurement_wallclock(SGE_PROF_CUSTOM7, true, NULL);

            /* 
             * need to sync with event master thread
             * if schedd configuration changed then settings in evm can be adjusted
             */
            if (sconf_is_new_config()) {
               /* set scheduler interval / event delivery interval */
               u_long32 interval = sconf_get_schedule_interval();
               if (evc->ec_get_edtime(evc) != interval) {
                  evc->ec_set_edtime(evc, interval);
               }

               /* set job / ja_task event flushing */
               set_job_flushing(evc);

               /* no need to ec_commit here - we do it when resetting the busy state */

               /* now we handled the new schedd config - no need to do it twice */
               sconf_reset_new_config();
            }

            /* block till master handled all GDI orders */
            sge_schedd_block_until_orders_processed(evc->get_gdi_ctx(evc), NULL);
            schedd_order_destroy();

            /*
             * Stop profiling for "schedd run total" and the subcategories
             */
            PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM6);
            prof_total = prof_get_measurement_wallclock(SGE_PROF_CUSTOM6, true, NULL);

            if (prof_is_active(SGE_PROF_CUSTOM6)) {
               PROFILING((SGE_EVENT, "PROF: schedd run took: %.3f s (init: %.3f s, copy: %.3f s, "
                          "run:%.3f, free: %.3f s, jobs: %d, categories: %d/%d)",
                           prof_total, prof_init, prof_copy, prof_run, prof_free,
                           lGetNumberOfElem(*object_type_get_master_list(SGE_TYPE_JOB)), sge_category_count(),
                           sge_cs_category_count() ));
            }
            if (getenv("SGE_ND") != NULL) {
               printf("--------------STOP-SCHEDULER-RUN-------------\n");
            } else {
               schedd_log("--------------STOP-SCHEDULER-RUN-------------", NULL, evc->monitor_next_run);
            }

            thread_output_profiling("scheduler thread profiling summary:\n", &next_prof_output);

            sge_monitor_output(&monitor);
         }

         /* reset the busy state */
         evc->ec_set_busy(evc, 0);
         evc->ec_commit(evc, NULL);

         /* stop logging into schedd_runlog (enabled via -tsm) */
         evc->monitor_next_run = false;

         /*
          * pthread cancelation point
          *
          * sge_scheduler_cleanup_thread() is the last function which should
          * be called so it is pushed first
          */
         pthread_cleanup_push(sge_scheduler_cleanup_thread, (void *) &ctx);
         pthread_cleanup_push((void (*)(void *))sge_scheduler_cleanup_monitor,
                              (void *)&monitor);
         pthread_cleanup_push((void (*)(void *))sge_scheduler_cleanup_event_client,
                              (void *)evc);
         cl_thread_func_testcancel(thread_config);
         pthread_cleanup_pop(execute);
         pthread_cleanup_pop(execute);
         pthread_cleanup_pop(execute);
         DPRINTF(("passed cancelation point\n"));
      }
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before
    * the call of cl_thread_func_testcancel()
    */

   DRETURN(NULL);
}

