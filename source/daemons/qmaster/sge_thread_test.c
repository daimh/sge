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
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_time.h"
#include "uti/sge_spool.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_bootstrap.h"
#include "uti/msg_utilib.h"  /* remove once 'sge_daemonize_qmaster' did become 'sge_daemonize' */
#include "uti/sge_profiling.h"
#include "uti/sge_thread_ctrl.h"
#include "uti/sge_dstring.h"
#include "uti/setup_path.h"
#include "uti/sge_os.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"

#include "gdi/sge_security.h"
#include "gdi/qm_name.h"
#include "gdi/sge_gdi_packet.h"

#include "comm/cl_commlib.h"

#include "sge.h"
#include "setup_qmaster.h"
#include "sge_mt_init.h"
#include "lock.h"
#include "sge_calendar_qmaster.h"
#include "qmaster_heartbeat.h"
#include "shutdown.h"
#include "sge_job_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "reschedule.h"
#include "basis_types.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_qmaster_process_message.h"
#include "sge_event_master.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "sge_userprj_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_sched_process_events.h"
#include "sge_follow.h"
#include "sge_qmaster_threads.h"
#include "sge_thread_main.h"
#include "sge_thread_test.h"
#include "sge_qmaster_process_message.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_daemons_common.h"

static void
sge_test_cleanup_monitor(monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_test_cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

void 
sge_test_initialize(sge_gdi_ctx_class_t *ctx)
{
   const u_long32 max_initial_test_threads = 1;
   cl_thread_settings_t* dummy_thread_p = NULL;
   int i;   

   DENTER(TOP_LAYER, "sge_test_initialize");

   INFO((SGE_EVENT, MSG_QMASTER_THREADCOUNT_US, 
         sge_u32c(max_initial_test_threads), threadnames[TESTER_THREAD]));
   cl_thread_list_setup(&(Main_Control.test_thread_pool), "thread pool");
   for (i = 0; i < max_initial_test_threads; i++) {
      dstring thread_name = DSTRING_INIT;

      sge_dstring_sprintf(&thread_name, "%s%03d", threadnames[TESTER_THREAD], i);
      cl_thread_list_create_thread(Main_Control.test_thread_pool, &dummy_thread_p,
                                   cl_com_get_log_list(), sge_dstring_get_string(&thread_name), i, 
                                   sge_test_main, NULL, NULL, CL_TT_TESTER);
      sge_dstring_free(&thread_name);
   }
   DRETURN_VOID;
}

void
sge_test_terminate(sge_gdi_ctx_class_t *ctx)
{
   cl_thread_settings_t* thread = NULL;

   DENTER(TOP_LAYER, "sge_test_terminate");

   thread = cl_thread_list_get_first_thread(Main_Control.test_thread_pool);
   while (thread != NULL) {
      DPRINTF((SFN" gets canceled\n", thread->thread_name));
      cl_thread_list_delete_thread(Main_Control.test_thread_pool, thread);

      thread = cl_thread_list_get_first_thread(Main_Control.test_thread_pool);
   }  
   DPRINTF(("all "SFN" threads terminated\n", threadnames[TESTER_THREAD]));

   DRETURN_VOID;
}

void *
sge_test_main(void *arg)
{
   bool do_endlessly = true;
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   sge_gdi_ctx_class_t *ctx = NULL;
   monitoring_t monitor;

   DENTER(TOP_LAYER, "sge_test_main");

   DPRINTF((SFN" started", thread_config->thread_name));
   cl_thread_func_startup(thread_config);
   sge_monitor_init(&monitor, thread_config->thread_name, NONE_EXT, MT_WARNING, MT_ERROR);
   sge_qmaster_thread_init(&ctx, QMASTER, TESTER_THREAD, true);

   /* register at profiling module */
   set_thread_name(pthread_self(), thread_config->thread_name);
   conf_update_thread_profiling(thread_config->thread_name);
 
   while (do_endlessly) {
      int execute = 0;
     
      DPRINTF((SFN " is looping\n", thread_config->thread_name)); 

      {
         lList *answer_list = NULL;
         lEnumeration *what = lWhat("%T(ALL)", CQ_Type);
         lCondition *where = NULL;
         lList *data_list = NULL;

         answer_list = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_GET, &data_list,
                                where, what, false);

         if (answer_list_has_error(&answer_list)) {
            lWriteListTo(answer_list, stderr);
         } else {
            DPRINTF(("got data list with %d and answer list with %d elements.\n",
                     lGetNumberOfElem(data_list), lGetNumberOfElem(answer_list)));
         }

         lFreeWhat(&what);
         lFreeWhere(&where);
      }

      {
         state_gdi_multi state = STATE_GDI_MULTI_INIT;
         lEnumeration *what_cqueue = lWhat("%T(ALL)", CQ_Type);
         lCondition *where_cqueue = NULL;
         lEnumeration *what_job = lWhat("%T(ALL)", JB_Type);
         lCondition *where_job = NULL;
         lList *local_answer_list = NULL;
         int cqueue_request_id;
         int job_request_id;

         cqueue_request_id = ctx->gdi_multi(ctx, &local_answer_list, SGE_GDI_RECORD, 
                                           SGE_CQ_LIST, SGE_GDI_GET, NULL,
                                           where_cqueue, what_cqueue, false, &state, true);
         job_request_id = ctx->gdi_multi(ctx, &local_answer_list, SGE_GDI_SEND, 
                                         SGE_JB_LIST, SGE_GDI_GET, NULL,
                                         where_job, what_job, false, &state, true);
         if (cqueue_request_id != -1 && job_request_id != -1 && 
             answer_list_has_error(&local_answer_list) == false) {
            lList *multi_answer_list = NULL;
            lList *list_cqueue = NULL;
            lList *list_job = NULL;
            lList *answer_cqueue = NULL;
            lList *answer_job = NULL;
     
            ctx->gdi_wait(ctx, &local_answer_list, &multi_answer_list, &state);
            sge_gdi_extract_answer(&answer_cqueue, SGE_GDI_GET, SGE_CQ_LIST, 
                                   cqueue_request_id, multi_answer_list, &list_cqueue);
            sge_gdi_extract_answer(&answer_job, SGE_GDI_GET, SGE_CQ_LIST, 
                                   job_request_id, multi_answer_list, &list_job);

            if (answer_list_has_error(&answer_cqueue) || answer_list_has_error(&answer_job) ||
                answer_list_has_error(&local_answer_list)) {
               ERROR((SGE_EVENT, "QMASTER INTERNAL MULTI GDI TEST FAILED"));
               ERROR((SGE_EVENT, "get response for intern gdi request but request had error")); 
            } else {
               INFO((SGE_EVENT, "QMASTER INTERNAL MULTI GDI TEST WAS SUCCESSFULL"));
               INFO((SGE_EVENT, "got cqueue list with "sge_U32CFormat" and cqueue answer "
                     "list with "sge_U32CFormat" elements.", sge_u32c(lGetNumberOfElem(list_cqueue)), 
                     sge_u32c(lGetNumberOfElem(answer_cqueue))));
               INFO((SGE_EVENT, "got job list with "sge_U32CFormat" and job answer "
                     "list with "sge_U32CFormat" elements.", sge_u32c(lGetNumberOfElem(list_job)), 
                     sge_u32c(lGetNumberOfElem(answer_job))));
            }
            lFreeList(&multi_answer_list);
            lFreeList(&answer_cqueue);
            lFreeList(&answer_job);
            lFreeList(&list_cqueue);
            lFreeList(&list_job);
         } else {
            ERROR((SGE_EVENT, "QMASTER INTERNAL MULTI GDI TEST FAILED"));
            ERROR((SGE_EVENT, "unable to send intern gdi request (cqueue_request_id=%d, "
                   "job_request_id=%d", cqueue_request_id, job_request_id));
         }

         lFreeList(&local_answer_list);
      }

      do {
         pthread_cleanup_push((void (*)(void *))sge_test_cleanup_monitor,
                              (void *)&monitor);
         cl_thread_func_testcancel(thread_config);
         pthread_cleanup_pop(execute); 
         if (sge_thread_has_shutdown_started()) {
            DPRINTF((SFN" is waiting for termination\n", thread_config->thread_name));
            sleep(1);   
         }
      } while (sge_thread_has_shutdown_started());
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before 
    * and after the call of cl_thread_func_testcancel()
    */

   DRETURN(NULL);
}

