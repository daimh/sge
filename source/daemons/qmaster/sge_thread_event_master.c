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

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_report.h"

#include "gdi/sge_security.h"

#include "comm/cl_commlib.h"

#include "gdi/sge_gdi_packet.h"

#include "basis_types.h"
#include "setup_qmaster.h"
#include "sgeobj/sge_manop.h"
#include "sge_event_master.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "sge_userprj_qmaster.h"
#include "sge_qmaster_threads.h"
#include "sge_thread_main.h"
#include "sge_thread_event_master.h"
#include "sge_qmaster_timed_event.h"
#include "sge_persistence_qmaster.h"

static void
sge_event_master_cleanup_monitor(monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "sge_event_master_cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

static void
sge_event_master_cleanup_report_list(lList **list)
{
   DENTER(TOP_LAYER, "sge_event_master_cleanup_monitor");
   lFreeList(list);
   DRETURN_VOID;
}

void 
sge_event_master_initialize(sge_gdi_ctx_class_t *ctx)
{
   cl_thread_settings_t* dummy_thread_p = NULL;
   dstring thread_name = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_event_master_initialize");

   DPRINTF(("event master functionality has been initialized\n"));

   sge_dstring_sprintf(&thread_name, "%s%03d", threadnames[DELIVERER_THREAD], 0);
   cl_thread_list_setup(&(Main_Control.event_master_thread_pool), "event master thread pool");
   cl_thread_list_create_thread(Main_Control.event_master_thread_pool, &dummy_thread_p,
                                cl_com_get_log_list(), sge_dstring_get_string(&thread_name), 0, 
                                sge_event_master_main, NULL, NULL, CL_TT_DELIVERER);
   sge_dstring_free(&thread_name);
   DRETURN_VOID;
}

void
sge_event_master_terminate(void)
{
   cl_thread_settings_t* thread = NULL;

   DENTER(TOP_LAYER, "sge_event_master_terminate");

   thread = cl_thread_list_get_first_thread(Main_Control.event_master_thread_pool);
   while (thread != NULL) {
      DPRINTF((SFN" gets canceled\n", thread->thread_name));
      cl_thread_list_delete_thread(Main_Control.event_master_thread_pool, thread);
      thread = cl_thread_list_get_first_thread(Main_Control.event_master_thread_pool);
   }  
   DPRINTF(("all "SFN" threads terminated\n", threadnames[DELIVERER_THREAD]));

   DRETURN_VOID;
}

void *
sge_event_master_main(void *arg)
{
   bool do_endlessly = true;
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   sge_gdi_ctx_class_t *ctx = NULL;
   monitoring_t monitor;
   monitoring_t *monitorp = &monitor;

   lListElem *report = NULL;
   lList *report_list = NULL;
   time_t next_prof_output = 0;

   DENTER(TOP_LAYER, "sge_event_master_main");

   DPRINTF(("started"));
   cl_thread_func_startup(thread_config);
   sge_monitor_init(&monitor, thread_config->thread_name, EDT_EXT, EMT_WARNING, EMT_ERROR);
   sge_qmaster_thread_init(&ctx, QMASTER, DELIVERER_THREAD, true);

   /* register at profiling module */
   set_thread_name(pthread_self(), "Deliver Thread");
   conf_update_thread_profiling("Deliver Thread");

   report_list = lCreateListHash("report list", REP_Type, false);
   report = lCreateElem(REP_Type);
   lSetUlong(report, REP_type, NUM_REP_REPORT_EVENTS);
   lSetHost(report, REP_host, ctx->get_qualified_hostname(ctx));
   lAppendElem(report_list, report);
 
   while (do_endlessly) {
      int execute = 0;

      thread_start_stop_profiling();

      /*
       * did a new event arrive which has a flush time of 0 seconds?
       */
      MONITOR_IDLE_TIME(sge_event_master_wait_next(), (&monitor), mconf_get_monitor_time(), 
                        mconf_is_monitor_message());

      MONITOR_MESSAGES((monitorp));
      MONITOR_EDT_COUNT((&monitor));
      MONITOR_CLIENT_COUNT((&monitor), lGetNumberOfElem(Event_Master_Control.clients));

      sge_event_master_process_requests(&monitor);
      sge_event_master_send_events(ctx, report, report_list, &monitor);
      sge_monitor_output(&monitor);

      thread_output_profiling("event master thread profiling summary:\n",
                              &next_prof_output);

      /* pthread cancelation point */
      pthread_cleanup_push((void (*)(void *))sge_event_master_cleanup_monitor,
                           (void *)&monitor);
      pthread_cleanup_push((void (*)(void *))sge_event_master_cleanup_report_list,
                           (void *)&report_list);
      cl_thread_func_testcancel(thread_config);
      pthread_cleanup_pop(execute); 
      pthread_cleanup_pop(execute); 
      if (sge_thread_has_shutdown_started()) {
         DPRINTF(("waiting for termination\n"));
         sleep(1);
      }
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before 
    * and after the call of cl_thread_func_testcancel()
    */

   DRETURN(NULL);
}

