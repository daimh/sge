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

#include "uti/sge_rmon.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_lock.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_bootstrap.h"
#include "uti/msg_utilib.h"  /* remove once 'sge_daemonize_qmaster' did become 'sge_daemonize' */
#include "uti/sge_profiling.h"
#include "uti/setup_path.h"
#include "uti/sge_os.h"
#include "uti/sge_time.h"
#include "uti/sge_spool.h"
#include "uti/sge_thread_ctrl.h"

#include "sgeobj/sge_jsv.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h"

#include "gdi/sge_security.h"
#include "gdi/qm_name.h"
#include "gdi/sge_gdi_packet.h"
#include "gdi/sge_gdi_packet_internal.h"
#include "gdi/sge_gdi2.h"

#include "evm/sge_event_master.h"

#include "comm/cl_commlib.h"

#include "basis_types.h"
#include "setup_qmaster.h"
#include "sge_qmaster_process_message.h"
#include "sge_mt_init.h"
#include "sge_qmaster_threads.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "sge_userprj_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_calendar_qmaster.h"
#include "lock.h"
#include "qmaster_heartbeat.h"
#include "shutdown.h"
#include "sge.h"
#include "sge_qmod_qmaster.h"
#include "reschedule.h"
#include "sge_job_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_sched_process_events.h"
#include "sge_follow.h"
#include "sge_c_report.h"
#include "sge_thread_main.h"
#include "sge_thread_worker.h"
#include "sge_qmaster_process_message.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_daemons_common.h"

static void
sge_worker_cleanup_monitor(monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_worker_cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

void 
sge_worker_initialize(sge_gdi_ctx_class_t *ctx)
{
   const u_long32 max_initial_worker_threads = ctx->get_worker_thread_count(ctx);
   cl_thread_settings_t* dummy_thread_p = NULL;
   int i;   

   DENTER(TOP_LAYER, "sge_worker_initialize");

   /*
    * TODO: EB: corresponding destroy function is missing during shutdown
    */
   sge_tq_create(&Master_Task_Queue);

   sge_init_job_number();
   sge_init_ar_id();
   DPRINTF(("job/ar counter have been initialized\n"));

   reporting_initialize(NULL);
   DPRINTF(("accounting and reporting module has been initialized\n"));

   INFO((SGE_EVENT, MSG_QMASTER_THREADCOUNT_US, 
         sge_u32c(max_initial_worker_threads), threadnames[WORKER_THREAD]));
   cl_thread_list_setup(&(Main_Control.worker_thread_pool), "thread pool");
   for (i = 0; i < max_initial_worker_threads; i++) {
      dstring thread_name = DSTRING_INIT;

      sge_dstring_sprintf(&thread_name, "%s%03d", threadnames[WORKER_THREAD], i);
      cl_thread_list_create_thread(Main_Control.worker_thread_pool, &dummy_thread_p,
                                   cl_com_get_log_list(), sge_dstring_get_string(&thread_name), i, 
                                   sge_worker_main, NULL, NULL, CL_TT_WORKER);
      sge_dstring_free(&thread_name);
   }
   DRETURN_VOID;
}

void
sge_worker_terminate(sge_gdi_ctx_class_t *ctx)
{
   bool do_final_spooling;

   DENTER(TOP_LAYER, "sge_worker_terminate");

   sge_tq_wakeup_waiting(Master_Task_Queue);

   /*
    * trigger pthread_cancel for each thread so that further 
    * shutdown process will be faster
    */
   {
      cl_thread_list_elem_t *thr = NULL;
      cl_thread_list_elem_t *thr_nxt = NULL;

      thr_nxt = cl_thread_list_get_first_elem(Main_Control.worker_thread_pool);
      while ((thr = thr_nxt) != NULL) {
         thr_nxt = cl_thread_list_get_next_elem(thr);

         cl_thread_shutdown(thr->thread_config); 
      }
   }

   /*
    * Shutdown/delete the threads and wait for termination
    */
   {
      cl_thread_settings_t *thread = NULL;

      thread = cl_thread_list_get_first_thread(Main_Control.worker_thread_pool);
      while (thread != NULL) {
         DPRINTF(("gets canceled\n"));
         cl_thread_list_delete_thread(Main_Control.worker_thread_pool, thread);

         thread = cl_thread_list_get_first_thread(Main_Control.worker_thread_pool);
      }  
      DPRINTF(("all "SFN" threads terminated\n", threadnames[WORKER_THREAD]));
   }

   do_final_spooling = sge_qmaster_do_final_spooling();

   /* shutdown and remove JSV instances */
   jsv_list_remove_all();

   reporting_shutdown(ctx, NULL, do_final_spooling);
   DPRINTF(("accounting and reporting module has been shutdown\n"));

   /*
    * final spooling is only done if the shutdown of the current instance
    * of this master process is not triggered due to the fact that
    * shadowed started another instance which is currently running ...
    *
    * ... in that case we would overwrite data which might have already
    * changed in the second instance of the master
    */
   if (do_final_spooling == true) {
      sge_store_job_number(ctx, NULL, NULL);
      sge_store_ar_id(ctx, NULL, NULL);
      DPRINTF(("job/ar counter made persistent\n"));
      sge_job_spool(ctx);     /* store qmaster jobs to database */
      sge_userprj_spool(ctx); /* spool the latest usage */
      DPRINTF(("final job and user/project spooling has been triggered\n"));
   }

   sge_shutdown_persistence(NULL);
   DPRINTF(("persistence module has been shutdown\n"));

   DRETURN_VOID;
}

void *
sge_worker_main(void *arg)
{
   bool do_endlessly = true;
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   sge_gdi_ctx_class_t *ctx = NULL;
   monitoring_t monitor;
   monitoring_t *monitorp = &monitor;
   time_t next_prof_output = 0;

   DENTER(TOP_LAYER, "sge_worker_main");

   DPRINTF(("started"));
   cl_thread_func_startup(thread_config);
   sge_monitor_init(&monitor, thread_config->thread_name, GDI_EXT, MT_WARNING, MT_ERROR);
   sge_qmaster_thread_init(&ctx, QMASTER, WORKER_THREAD, true);

   /* register at profiling module */
   set_thread_name(pthread_self(), "Worker Thread");
   conf_update_thread_profiling("Worker Thread");
 
   while (do_endlessly) {
      sge_gdi_packet_class_t *packet = NULL;

      /*
       * Wait for packets. As long as packets are available cancelation 
       * of this thread is ignored. The shutdown procedure in the main 
       * thread takes care that packet producers will be terminated 
       * before all worker threads so that this won't be a problem.
       */
      MONITOR_IDLE_TIME(
         sge_tq_wait_for_task(Master_Task_Queue, 1, SGE_TQ_GDI_PACKET, (void *)&packet),
         &monitor, mconf_get_monitor_time(), mconf_is_monitor_message());

      MONITOR_SET_QLEN((monitorp), sge_tq_get_task_count(Master_Task_Queue));

      if (packet != NULL) {
         sge_gdi_task_class_t *task = packet->first_task;
         bool is_only_read_request = true;

         thread_start_stop_profiling();

#ifdef SEND_ANSWER_IN_LISTENER
#else
         /*
          * prepare buffer for sending an answer 
          */
         if (packet->is_intern_request == false && packet->is_gdi_request == true) {
            init_packbuffer(&(packet->pb), 0, 0);
         }
#endif

         MONITOR_MESSAGES((monitorp));

         if (packet->is_gdi_request == true) {
            /*
             * test if a write lock is necessary
             */
            task = packet->first_task;
            while (task != NULL) {
               u_long32 command = SGE_GDI_GET_OPERATION(task->command); 

               if (command != SGE_GDI_GET) {
                  is_only_read_request = false;
                  break;
               }
               task = task->next;            
            }
         } else {
            is_only_read_request = false;
         }

         /*
          * acquire the correct lock
          */
         if (is_only_read_request) {
            MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_READ), monitorp);
         } else {
            MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitorp);
         }

         if (packet->is_gdi_request == true) {
            /*
             * do the GDI request
             */
            task = packet->first_task;
            while (task != NULL) {
               sge_c_gdi(ctx, packet, task, &(task->answer_list), &monitor);

               task = task->next;
            }
         } else {
            task = packet->first_task;
            sge_c_report(ctx, packet->host, packet->commproc, packet->commproc_id, 
                         task->data_list, &monitor);
         }

         /*
          * do unlock
          */
         if (is_only_read_request) {
            SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ)
         } else {
            SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE)
         }

         if (packet->is_gdi_request == true) {
#ifdef SEND_ANSWER_IN_LISTENER
            sge_gdi_packet_broadcast_that_handled(packet);
#else
            /*
             * Send the answer to the client
             */
            if (packet->is_intern_request == false) {
               MONITOR_MESSAGES_OUT(monitorp);
               sge_gdi2_send_any_request(ctx, 0, NULL,
                                         packet->host, packet->commproc, packet->commproc_id, 
                                         &(packet->pb), TAG_GDI_REQUEST, 
                                         packet->response_id, NULL);
               clear_packbuffer(&(packet->pb));
#  ifdef BLOCK_LISTENER
               sge_gdi_packet_broadcast_that_handled(packet);
#  else
               sge_gdi_packet_free(&packet);
#  endif
               /*
                * Code only for TS: 
                *
                * Following if-block will only be executed in testsuite if the qmaster
                * parameter __TEST_SLEEP_AFTER_REQUEST is defined. This will block the
                * worker thread if it handled a request. Only this makes sure that
                * other worker threads can handle incoming requests. Otherwise
                * it might be possible that one worker threads handles all requests
                * on fast qmaster hosts if testsuite is not fast enough to generate
                * gdi requests.
                */
               if (mconf_get_enable_test_sleep_after_request() == true) {
                  sleep(5);
               }
            } else {
               sge_gdi_packet_broadcast_that_handled(packet);
               /* this is an internal request, packet will get destroyed later,
                * where the caller waits for the answer
                * make sure it is no longer accessed here
                */
               packet = NULL;
            }
#endif
         } else {
            sge_gdi_packet_free(&packet);
         }
     
         thread_output_profiling("worker thread profiling summary:\n",
                                 &next_prof_output);

         sge_monitor_output(&monitor);
      } else { 
         int execute = 0;

         /* 
          * pthread cancelation point
          */
         pthread_cleanup_push((void (*)(void *))sge_worker_cleanup_monitor,
                              (void *)&monitor);
         cl_thread_func_testcancel(thread_config);
         pthread_cleanup_pop(execute); 
      }
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before 
    * and after the call of cl_thread_func_testcancel()
    */

   DRETURN(NULL);
}
