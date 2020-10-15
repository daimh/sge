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
#include <fcntl.h>

#include <sys/resource.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_unistd.h"
#include "uti/sge_profiling.h"
#include "uti/sge_time.h"
#include "uti/sge_monitor.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_thread_ctrl.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_manop.h"

#include "comm/cl_commlib.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_thread_main.h"

#include "basis_types.h"
#include "sge_qmaster_heartbeat.h"
#include "sge_thread_jvm.h"
#include "sge_thread_listener.h"
#include "sge_thread_signaler.h"
#include "sge_thread_scheduler.h"
#include "sge_thread_timer.h"
#include "sge_thread_worker.h"
#include "sge_thread_event_master.h"
#include "setup_qmaster.h"
#include "sge_qmaster_process_message.h"
#include "sge_event_master.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "qmaster_heartbeat.h"
#include "shutdown.h"
#include "sge.h"
#include "sge_qmaster_threads.h"
#include "msg_qmaster.h"
#include "setup_qmaster.h"

#if defined(SOLARIS)
#   include "sge_smf.h"
#endif

static void init_sig_action_and_mask(void);

/****** qmaster/sge_qmaster_main/sge_qmaster_application_status() ************
*  NAME
*     sge_qmaster_application_status() -- commlib status callback function  
*
*  SYNOPSIS
*      unsigned long sge_qmaster_application_status(char** info_message) 
*
*  FUNCTION
*      This is the implementation of the commlib application status callback
*      function. This function is called from the commlib when a connected
*      client wants to get a SIRM (Status Information Response Message).
*      The standard client for this action is the qping command.
*
*      The callback function is set with cl_com_set_status_func() after
*      commlib initalization.
*
*      The function is called by a commlib thread which is not in the
*      context of the qmaster application. This means no qmaster specific
*      functions should be called (locking of global variables).
*
*      status 0:  no errors
*      status 1:  one or more threads has reached warning timeout
*      status 2:  one or more threads has reached error timeout
*      status 3:  thread alive timeout struct not initalized
*
*  INPUTS
*     char** info_message - pointer to an char* inside commlib.
*                           info message must be malloced, commlib will
*                           free this memory. 
*  RESULT
*     unsigned long status - status of application
*
*  NOTES
*     This function is MT save
* 
*******************************************************************************/
unsigned long sge_qmaster_application_status(char** info_message) 
{

   return sge_monitor_status(info_message, mconf_get_monitor_time());
}


/****** qmaster/sge_qmaster_main/main() ****************************************
*  NAME
*     main() -- qmaster entry point 
*
*  SYNOPSIS
*     int main(int argc, char* argv[]) 
*
*  FUNCTION
*     Qmaster entry point.
*
*     NOTE: The main thread must block all signals before any additional thread
*     is created. Failure to do so will ruin signal handling!
*
*  INPUTS
*     int argc     - number of commandline arguments 
*     char* argv[] - commandline arguments 
*
*  RESULT
*     0 - success 
*
*  NOTES
*     We check whether 'SGE_ROOT' is set before we daemonize. Once qmaster is
*     a daemon, we are no longer connected to a terminal and hence can not
*     output an error message to stdout or stderr.
*
*     We need to inovke 'prepare_enroll()' *before* the user id is switched via
*     'become_admin_user()'. This is because qmaster must be able to bind a so
*     called reserved port (requires root privileges) if configured to do so.
*
*******************************************************************************/
int main(int argc, char* argv[])
{
   int max_enroll_tries;
   int ret_val;
   bool has_daemonized = false;
   sge_gdi_ctx_class_t *ctx = NULL;
   u_long32 start_time = sge_get_gmt();
   monitoring_t monitor;
   const char *pidfile = getenv("SGE_QMASTER_PIDFILE");

   DENTER_MAIN(TOP_LAYER, "qmaster");

   sge_maybe_set_dumpable();
   sge_monitor_init(&monitor, "MAIN", NONE_EXT, MT_WARNING, MT_ERROR);
   prof_mt_init();

   sge_get_root_dir(true, NULL, 0, true);
   
#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   sge_init_language_func((gettext_func_type)gettext, (setlocale_func_type)setlocale, (bindtextdomain_func_type)bindtextdomain, (textdomain_func_type)textdomain);
   sge_init_language(NULL,NULL);   
#endif 

   /* 
    * qmaster doesn't support any commandline anymore,
    * but we should show version string and -help option 
    */
   if (argc != 1) {
      sigset_t sig_set;
      sigfillset(&sig_set);
      pthread_sigmask(SIG_SETMASK, &sig_set, NULL);
      sge_qmaster_thread_init(&ctx, QMASTER, MAIN_THREAD, true);
      sge_process_qmaster_cmdline(argv);
      SGE_EXIT((void**)&ctx, 1);
   }

   /*
    * daemonize qmaster
    * set filedescripto limits
    * and initialize librarrays to be used in multi threaded environment
    * also take care that finished child processed of this process become
    * zombie jobs
    */
   has_daemonized = sge_daemonize_qmaster();
   init_sig_action_and_mask();

   /* init qmaster threads without becomming admin user */
   sge_qmaster_thread_init(&ctx, QMASTER, MAIN_THREAD, false);

   ctx->set_daemonized(ctx, has_daemonized);

   /* this must be done as root user to be able to bind ports < 1024 */
   max_enroll_tries = 30;
   while (cl_com_get_handle(prognames[QMASTER],1) == NULL) {
      ctx->prepare_enroll(ctx);
      max_enroll_tries--;
      if (max_enroll_tries <= 0) {
         /* exit after 30 seconds */
         CRITICAL((SGE_EVENT, SFNMAX, MSG_QMASTER_COMMUNICATION_ERRORS));
         SGE_EXIT((void**)&ctx, 1);
      }
      if (cl_com_get_handle(prognames[QMASTER],1) == NULL) {
        /* sleep when prepare_enroll() failed */
        sleep(1);
      }
   }

   /*
    * now the commlib up and running. Set qmaster application status function 
    * (commlib callback function for qping status information response 
    *  messages (SIRM))
    */
   ret_val = cl_com_set_status_func(sge_qmaster_application_status);
   if (ret_val != CL_RETVAL_OK) {
      ERROR((SGE_EVENT, SFNMAX, cl_get_error_text(ret_val)));
   }

   /* 
    * now we become admin user change into the correct root directory set the
    * the target for logging messages
    */
   sge_become_admin_user(ctx->get_admin_user(ctx));
   sge_chdir_exit(ctx->get_qmaster_spool_dir(ctx), 1);
   log_state_set_log_file(ERR_FILE);
   ctx->set_exit_func(ctx, sge_exit_func);

#if defined(SOLARIS)
   /* Init shared SMF libs if necessary */
   if (sge_smf_used() == 1 && sge_smf_init_libs() != 0) {
       SGE_EXIT((void**)&ctx, 1);
   }
#endif

   /*
    * We do increment the heartbeat manually here. This is the 'startup heartbeat'. 
    * The first time the hearbeat will be incremented through the heartbeat event 
    * handler is after about HEARTBEAT_INTERVAL seconds. The hardbeat event handler
    * is setup during the initialisazion of the timer thread.
    */
   inc_qmaster_heartbeat(QMASTER_HEARTBEAT_FILE, HEARTBEAT_INTERVAL, NULL);
     
   /*
    * Event master module has to be initialized already here because
    * sge_setup_qmaster() might already access it although event delivery
    * thread is not running.
    *
    * Corresponding shutdown is done in sge_event_master_terminate();
    *
    * EB: In my opinion the init function should called in
    * sge_event_master_initialize(). Is it possible to move that call?
    */ 
   sge_event_master_init();

   sge_setup_qmaster(ctx, argv);

   /*
    * Setup all threads and initialize corresponding modules. 
    * Order is important!
    */
   sge_signaler_initialize(ctx);
   sge_event_master_initialize(ctx);
   sge_timer_initialize(ctx, &monitor);
   sge_worker_initialize(ctx);
#if 0
   sge_test_initialize(ctx);
#endif
   sge_listener_initialize(ctx);
   sge_scheduler_initialize(ctx, NULL);
#ifndef NO_JNI
   sge_jvm_initialize(ctx, NULL);
#endif

   INFO((SGE_EVENT, "qmaster startup took "sge_u32" seconds", sge_get_gmt() - start_time));

   /*
    * Block till signal from signal thread arrives us
    */
   sge_thread_wait_for_signal();

   /* 
    * Shutdown all threads and shutdown corresponding modules.
    * Order is important!
    */
#ifndef NO_JNI
   sge_jvm_terminate(ctx, NULL);
#endif
   sge_scheduler_terminate(ctx, NULL);
   sge_listener_terminate();
#if 0
   sge_test_terminate(ctx);
#endif
   sge_worker_terminate(ctx);
   sge_timer_terminate();
   sge_event_master_terminate();
   sge_signaler_terminate();

   /*
    * Remaining shutdown operations
    */
   sge_clean_lists();
   sge_monitor_free(&monitor);

   sge_shutdown((void**)&ctx, sge_qmaster_get_exit_state());
   sge_prof_cleanup();

   if (!pidfile) pidfile = QMASTER_PID_FILE;
   unlink(pidfile);

   DRETURN(0);
} /* main() */


/****** qmaster/sge_qmaster_main/init_sig_action_and_mask() *******************
*  NAME
*     init_sig_action_and_mask() -- initialize signal action and mask 
*
*  SYNOPSIS
*     static void init_sig_action_and_mask(void)
*
*  FUNCTION
*     Initialize signal action and mask.
*
*     NOTE: We ignore SIGCHLD. This, together with the 'SA_NOCLDWAIT' flag,
*     does make sure, that an unwaited for child process will not become
*     a zombie process.
*
*  INPUTS
*     none 
*
*  RESULT
*     none
*
*******************************************************************************/
static void init_sig_action_and_mask(void)
{
#if !defined(INTERIX) && !defined(__CYGWIN__)
   struct sigaction sa;
   sigset_t sig_set;
   
   sa.sa_handler = SIG_IGN;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_NOCLDWAIT;
   sigaction(SIGCHLD, &sa, NULL);
   
   sigfillset(&sig_set);
   pthread_sigmask(SIG_SETMASK, &sig_set, NULL);

#endif
   return;
}

