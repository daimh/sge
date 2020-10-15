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
#include <fcntl.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_thread_ctrl.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_profiling.h"
#include "uti/setup_path.h"

#include "comm/cl_commlib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_conf.h"

#include "gdi/sge_security.h"
#include "gdi/sge_gdi_packet.h"

#include "msg_qmaster.h"
#include "basis_types.h"
#include "sge_qmaster_process_message.h"
#include "sge_event_master.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "setup_qmaster.h"
#include "sge_qmaster_threads.h"
#include "sge_thread_main.h"
#include "sge_thread_listener.h"

static void
sge_listener_cleanup_monitor(monitoring_t *monitor) 
{
   DENTER(TOP_LAYER, "sge_listener_cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

void 
sge_listener_initialize(sge_gdi_ctx_class_t *ctx)
{
   const u_long32 max_initial_listener_threads = ctx->get_listener_thread_count(ctx);
   cl_thread_settings_t* dummy_thread_p = NULL;
   u_long32 i;  

   DENTER(TOP_LAYER, "sge_listener_initialize");

   INFO((SGE_EVENT, MSG_QMASTER_THREADCOUNT_US, sge_u32c(max_initial_listener_threads), threadnames[LISTENER_THREAD]));
   cl_thread_list_setup(&(Main_Control.listener_thread_pool), "thread pool");
   for (i = 0; i < max_initial_listener_threads; i++) {
      dstring thread_name = DSTRING_INIT;      
      
      sge_dstring_sprintf(&thread_name, "%s%03"sge_U32CLetter, threadnames[LISTENER_THREAD], i);
      cl_thread_list_create_thread(Main_Control.listener_thread_pool, &dummy_thread_p,
                                   cl_com_get_log_list(), sge_dstring_get_string(&thread_name), i, 
                                   sge_listener_main, NULL, NULL, CL_TT_LISTENER); 
      sge_dstring_free(&thread_name);
   }
   DRETURN_VOID;
}

void
sge_listener_terminate(void)
{
   cl_thread_settings_t* thread = NULL;
   DENTER(TOP_LAYER, "sge_listener_terminate");

   /*
    * Currently the event master (EDE) does need a working gdi thread to 
    * successfully deliver events. For this reason we need to add the
    * 'sgeE_QMASTER_GOES_DOWN' event *before* the listener threads 
    * are terminated.
    */
   sge_add_event(0, sgeE_QMASTER_GOES_DOWN, 0, 0, NULL, NULL, NULL, NULL);
   DPRINTF(("triggered shutdown event for event master module\n"));

   /*
    * trigger pthread_cancel for each thread so that further 
    * shutdown process will be faster
    */
   {
      cl_thread_list_elem_t *thr = NULL;
      cl_thread_list_elem_t *thr_nxt = NULL;

      thr_nxt = cl_thread_list_get_first_elem(Main_Control.listener_thread_pool);
      while ((thr = thr_nxt) != NULL) {
         thr_nxt = cl_thread_list_get_next_elem(thr);

         cl_thread_shutdown(thr->thread_config);
      }
   }

   /*
    * delete all threads and wait for termination
    */
   thread = cl_thread_list_get_first_thread(Main_Control.listener_thread_pool);
   while (thread != NULL) {
      DPRINTF((SFN" gets canceled\n", thread->thread_name));
      cl_thread_list_delete_thread(Main_Control.listener_thread_pool, thread);
      thread = cl_thread_list_get_first_thread(Main_Control.listener_thread_pool);
   } 
   DPRINTF(("all "SFN" threads exited\n", threadnames[LISTENER_THREAD]));
   DRETURN_VOID;
}

void *
sge_listener_main(void *arg)
{
   bool do_endlessly = true;
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   monitoring_t monitor;
   sge_gdi_ctx_class_t *ctx = NULL;
   time_t next_prof_output = 0;

   DENTER(TOP_LAYER, "sge_listener_main");

   DPRINTF(("started\n"));
   cl_thread_func_startup(thread_config);
   sge_monitor_init(&monitor, thread_config->thread_name, LIS_EXT, MT_WARNING, MT_ERROR);
   sge_qmaster_thread_init(&ctx, QMASTER, LISTENER_THREAD, true);

   /* register at profiling module */
   set_thread_name(pthread_self(), "Listener Thread");
   conf_update_thread_profiling("Listener Thread");

   DPRINTF(("entering main loop\n"));
   while (do_endlessly) {
      int execute = 0; 

      if (sge_thread_has_shutdown_started() == false) { 
         thread_start_stop_profiling();

         sge_qmaster_process_message(ctx, &monitor);
       
         thread_output_profiling("listener thread profiling summary:\n",
                                 &next_prof_output);

         sge_monitor_output(&monitor);
      } 

      /* pthread cancelation point */
      do {
         pthread_cleanup_push((void (*)(void *))sge_listener_cleanup_monitor, 
                              (void *)&monitor);
         cl_thread_func_testcancel(thread_config);
         pthread_cleanup_pop(execute); 
         if (sge_thread_has_shutdown_started()) {
            DPRINTF(("waiting for termination\n"));
            sleep(1);
         }
      } while (sge_thread_has_shutdown_started());
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before 
    * the call of cl_thread_func_testcancel()
    */
   DRETURN(NULL);
}

