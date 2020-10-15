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
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_os.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_binding_hlp.h"
#include "uti2/sge_cgroup.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_proc.h"

#include "spool/classic/read_write_job.h"

#include "gdi/msg_gdilib.h"

#include "sge_load_sensor.h"
#include "dispatcher.h"
#include "load_avg.h"
#include "reaper_execd.h"
#include "setup_execd.h"
#include "shutdown.h"
#include "sig_handlers.h"
#include "startprog.h"
#include "usage.h"
#include "execd.h"
#include "sge.h"
#include "msg_common.h"
#include "msg_execd.h"
#include "binding_support.h"

#ifdef COMPILE_DC
#   include "ptf.h"
#   include "sgedefs.h"
#endif

#if defined(SOLARIS)
#   include "sge_smf.h"
#   include "sge_string.h"
#endif

#include "sge_proc.h"

#define EXECD_PID_FILE            "execd.pid"

volatile int jobs_to_start = 1;

/* only used when running as SGE execd */
volatile int waiting4osjid = 1;

/* Store the directory the execd runs in when in normal operation.
 * avoid calling getcwd, cause this catches zombies on sun and is a !?GRML call!
 */
char execd_spool_dir[SGE_PATH_MAX];

static void execd_exit_func(void **ctx, int i);
static void parse_cmdline_execd(char **argv);
static lList *sge_parse_cmdline_execd(char **argv, lList **ppcmdline);
static lList *sge_parse_execd(lList **ppcmdline, u_long32 *help);

static u_long32 last_qmaster_registration_time = 0;


u_long32 get_last_qmaster_register_time(void) {
   return last_qmaster_registration_time;
}


/****** execd/sge_execd_application_status() ***********************************
*  NAME
*     sge_execd_application_status() -- commlib status callback function
*
*  SYNOPSIS
*     unsigned long sge_execd_application_status(char** info_message) 
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
*      The function is called by a commlib function which may not run in the
*      context of the execd application. This means no execd specific
*      functions should be called (e.g. locking of global variables).
*
*      status 0:  no errors
*      status 1:  dispatcher has reached warning timeout
*      status 2:  dispatcher has reached error timeout
*      status 3:  dispatcher alive timeout struct not initalized
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
*******************************************************************************/
unsigned long sge_execd_application_status(char** info_message) 
{
   return sge_monitor_status(info_message, 0);
}

/*-------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
   int ret;
   int ret_val;
   int printed_points = 0;
   int max_enroll_tries;
   time_t next_prof_output = 0;
   int execd_exit_state = 0;
   lList **master_job_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   lList *alp = NULL;
   const char *pidfile = NULL;

   DENTER_MAIN(TOP_LAYER, "execd");

   gen_procList ();
   init_topology();
   sge_maybe_set_dumpable();
   init_cgroups();
   prof_mt_init();

   set_thread_name(pthread_self(),"Execd Thread");

   pidfile = getenv("SGE_EXECD_PIDFILE");
   if (!pidfile)
       pidfile = EXECD_PID_FILE;

   prof_set_level_name(SGE_PROF_CUSTOM1, "Execd Thread", NULL); 
   prof_set_level_name(SGE_PROF_CUSTOM2, "Execd Dispatch", NULL); 

#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);   
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */

   /* This needs a better solution */
   umask(022);

   /* exit func for SGE_EXIT() */
   sge_sig_handler_in_main_loop = 0;
   sge_setup_sig_handlers(EXECD);

   if (sge_setup2(&ctx, EXECD, MAIN_THREAD, &alp, false) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }
   ctx->set_exit_func(ctx, execd_exit_func);
   
#if defined(SOLARIS)
   /* Init shared SMF libs if necessary */
   if (sge_smf_used() == 1 && sge_smf_init_libs() != 0) {
       SGE_EXIT((void**)&ctx, 1);
   }
#endif

   /* prepare daemonize */
   if (!getenv("SGE_ND")) {
      sge_daemonize_prepare(ctx);
   }

   if ((ret=sge_occupy_first_three())>=0) {
      CRITICAL((SGE_EVENT, MSG_FILE_REDIRECTFD_I, ret));
      SGE_EXIT((void**)&ctx, 1);
   }

   lInit(nmv);

   /* unset XAUTHORITY if set */
   if (getenv("XAUTHORITY") != NULL) {
      unsetenv("XAUTHORITY");
   }

   parse_cmdline_execd(argv);   
   
   /* exit if we can't get communication handle (bind port) */
   max_enroll_tries = 30;
   while (cl_com_get_handle(prognames[EXECD],1) == NULL) {
      ctx->prepare_enroll(ctx);
      max_enroll_tries--;

      if (max_enroll_tries <= 0 || shut_me_down) {
         /* exit after 30 seconds */
         if (printed_points != 0) {
            printf("\n");
         }
         CRITICAL((SGE_EVENT, SFNMAX, MSG_COM_ERROR));
         SGE_EXIT((void**)&ctx, 1);
      }
      if (cl_com_get_handle(prognames[EXECD],1) == NULL) {
        /* sleep when prepare_enroll() failed */
        sleep(1);
        if (max_enroll_tries < 27) {
           printf(".");
           printed_points++;
           fflush(stdout);
        }
      }
   }

   if (printed_points != 0) {
      printf("\n");
   }

   /*
    * now the commlib up and running. Set execd application status function 
    * ( commlib callback function for qping status information response 
    *   messages (SIRM) )
    */
   ret_val = cl_com_set_status_func(sge_execd_application_status);
   if (ret_val != CL_RETVAL_OK) {
      ERROR((SGE_EVENT, SFNMAX, cl_get_error_text(ret_val)));
   }

   /* test connection */
   {
      cl_com_SIRM_t* status = NULL;
      ret_val = cl_commlib_get_endpoint_status(ctx->get_com_handle(ctx),
                                               (char *)ctx->get_master(ctx, true),
                                               (char*)prognames[QMASTER], 1, &status);
      if (ret_val != CL_RETVAL_OK) {
         ERROR((SGE_EVENT, SFNMAX, cl_get_error_text(ret_val)));
         ERROR((SGE_EVENT, SFNMAX, MSG_CONF_NOCONFBG));
      }
      cl_com_free_sirm_message(&status);
   }
   
   /* finalize daeamonize */
   if (!getenv("SGE_ND")) {
      sge_daemonize_finalize(ctx);
   }

   /* daemonizes if qmaster is unreachable */   
   sge_setup_sge_execd(ctx);

   /* are we using qidle or not */
   sge_ls_qidle(mconf_get_use_qidle());
   sge_ls_qls(1);
   
   DPRINTF(("use_qidle: %d\n", mconf_get_use_qidle()));

   /* test load sensor (internal or external) */
   {
      lList *report_list = sge_build_load_report(ctx->get_qualified_hostname(ctx), ctx->get_binary_path(ctx));
      lFreeList(&report_list);
   }
   
   /* here we have to wait for qmaster registration */
   while (sge_execd_register_at_qmaster(ctx, false) != 0) {
      if (sge_get_com_error_flag(EXECD, SGE_COM_ACCESS_DENIED, true)) {
         /* This is no error */
         DPRINTF(("*****  got SGE_COM_ACCESS_DENIED from qmaster  *****\n"));
      }
      if (sge_get_com_error_flag(EXECD, SGE_COM_ENDPOINT_NOT_UNIQUE, false)) {
         execd_exit_state = SGE_COM_ENDPOINT_NOT_UNIQUE;
         break;
      }
      if (shut_me_down != 0) {
         break;
      }
      sleep(30);
   }

   /* 
    * Terminate on SIGTERM or hard communication error
    */
   if (execd_exit_state != 0 || shut_me_down != 0) {
      sge_shutdown((void**)&ctx, execd_exit_state);
      DRETURN(execd_exit_state);
   }

   /*
    * We write pid file when we are connected to qmaster. Otherwise an old
    * execd might overwrite our pidfile.
    */
   sge_write_pid(pidfile);

   /*
    * At this point we are sure we are the only sge_execd and we are connected
    * to the current qmaster. First we have to report any reaped children
    * that might exist.
    */
   starting_up();

   /*
    * Log a warning message if execd hasn't been started by a superuser
    */
   if (!sge_is_start_user_superuser()) {
      WARNING((SGE_EVENT, SFNMAX, MSG_SWITCH_USER_NOT_ROOT));
   }   

#ifdef COMPILE_DC
   if (ptf_init()) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_EXECD_NOSTARTPTF));
      SGE_EXIT((void**)&ctx, 1);
   }
   INFO((SGE_EVENT, SFNMAX, MSG_EXECD_STARTPDCANDPTF));
#endif

   master_job_list = object_type_get_master_list(SGE_TYPE_JOB);
   *master_job_list = lCreateList("Master_Job_List", JB_Type);
   job_list_read_from_disk(master_job_list, "Master_Job_List",
                           0, SPOOL_WITHIN_EXECD, 
                          job_initialize_job);
   

   /* clean up jobs hanging around (look in active_dir) */
   clean_up_old_jobs(ctx, 1);
   execd_trash_load_report();
   sge_set_flush_lr_flag(true);

   sge_sig_handler_in_main_loop = 1;

   if (thread_prof_active_by_id()) {
      prof_start(SGE_PROF_CUSTOM1, NULL);
      prof_start(SGE_PROF_CUSTOM2, NULL);
      prof_start(SGE_PROF_GDI_REQUEST, NULL);
   } else {
      prof_stop(SGE_PROF_CUSTOM1, NULL);
      prof_stop(SGE_PROF_CUSTOM2, NULL);
      prof_stop(SGE_PROF_GDI_REQUEST, NULL);
   }

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   /* Start dispatching */
   execd_exit_state = sge_execd_process_messages(ctx);


   /*
    * This code is only reached when dispatcher terminates and execd goes down.
    */

   /* log if we received SIGPIPE signal */
   if (sge_sig_handler_sigpipe_received) {
       sge_sig_handler_sigpipe_received = 0;
       INFO((SGE_EVENT, "SIGPIPE received\n"));
   }

#if defined(LINUX)
   free_procList();
#endif
   lFreeList(master_job_list);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);
   if (prof_is_active(SGE_PROF_ALL)) {
     time_t now = (time_t)sge_get_gmt();

      if (now > next_prof_output) {
         prof_output_info(SGE_PROF_ALL, false, "profiling summary:\n");
         prof_reset(SGE_PROF_ALL,NULL);
         next_prof_output = now + 60;
      }
   }
   sge_prof_cleanup();
   sge_shutdown((void**)&ctx, execd_exit_state);

   unlink(pidfile);

   DRETURN(execd_exit_state);
}


/*-------------------------------------------------------------
 * Function installed to be called just before exit() is called.
 * clean up
 *-------------------------------------------------------------*/
static void execd_exit_func(void **ctx_ref, int i _UNUSED)
{
   DENTER(TOP_LAYER, "execd_exit_func");

   sge_gdi2_shutdown(ctx_ref);

   /* trigger load sensors shutdown */
   sge_ls_stop(0);

#ifdef COMPILE_DC
   ptf_stop();
#endif
   
#if defined(SOLARIS)
   if (sge_smf_used() == 1) {
      /* We don't do disable on svcadm restart */
      if (sge_strnullcmp(sge_smf_get_instance_state(), SCF_STATE_STRING_ONLINE) == 0 &&
          sge_strnullcmp(sge_smf_get_instance_next_state(), SCF_STATE_STRING_NONE) == 0) {      
         sge_smf_temporary_disable_instance();
      }
   }
#endif  
   DEXIT;
}

/****** execd/sge_execd_register_at_qmaster() **********************************
*  NAME
*     sge_execd_register_at_qmaster() -- modify execd list at qmaster site
*
*  SYNOPSIS
*     int sge_execd_register_at_qmaster(void) 
*
*  FUNCTION
*     add local execd name to SGE_EH_LIST in order to register at
*     qmaster
*
*  INPUTS
*     void - no input
*
*  RESULT
*     int - 0 = success / 1 = error
*
*  NOTES
*     MT-NOTE: sge_execd_register_at_qmaster() is not MT safe 
*
*******************************************************************************/
int sge_execd_register_at_qmaster(sge_gdi_ctx_class_t *ctx, bool is_restart) {
   int return_value = 0;
   static int sge_last_register_error_flag = 0;
   lList *alp = NULL;

   /* 
    * If it is a reconnect (is_restart == true) the act_qmaster file must be
    * re-read in order to update ctx qmaster cache when master migrates. 
    */
   const char *master_host = ctx->get_master(ctx, is_restart);

   DENTER(TOP_LAYER, "sge_execd_register_at_qmaster");

   /* We will not try to make a gdi request when qmaster is not alive. The
    * gdi will return with timeout after one minute. If qmaster is not alive
    * we will not try a gdi request!
    */
   if (master_host != NULL && ctx->is_alive(ctx) == CL_RETVAL_OK) {
      lList *hlp = lCreateList("exechost starting", EH_Type);
      lListElem *hep = lCreateElem(EH_Type);
      lSetUlong(hep, EH_featureset_id, feature_get_active_featureset());
      lAppendElem(hlp, hep);

      /* register at qmaster */
      DPRINTF(("*****  Register at qmaster   *****\n"));
      if (!is_restart) {
         /*
          * This is a regular startup.
          */
         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_ADD, &hlp, NULL, NULL, false);
      } else {
         /*
          * Indicate this is a restart to qmaster.
          * This is used for the initial_state of queue_configuration implementation.
          */
         alp = ctx->gdi(ctx, SGE_EH_LIST, SGE_GDI_ADD | SGE_GDI_EXECD_RESTART,
                        &hlp, NULL, NULL, false);
      }
      lFreeList(&hlp);
   } else {
      DPRINTF(("*****  Register at qmaster - qmaster not alive!  *****\n"));
   }

   if (alp == NULL) {
      if (sge_last_register_error_flag == 0) {
         WARNING((SGE_EVENT, MSG_COM_CANTREGISTER_SS, master_host?master_host:"", MSG_COM_ERROR));
         sge_last_register_error_flag = 1;
      }
      return_value = 1;
   } else {
      lListElem *aep = lFirst(alp);
      if (lGetUlong(aep, AN_status) != STATUS_OK) {
         if (sge_last_register_error_flag == 0) {
            WARNING((SGE_EVENT, MSG_COM_CANTREGISTER_SS, master_host?master_host:"", lGetString(aep, AN_text)));
            sge_last_register_error_flag = 1;
         }
         return_value = 1;
      }
   }
 
   if (return_value == 0) {
      sge_last_register_error_flag = 0;
      INFO((SGE_EVENT, MSG_EXECD_REGISTERED_AT_QMASTER_S, master_host?master_host:""));
      last_qmaster_registration_time = sge_get_gmt();
   }
   lFreeList(&alp);
   DRETURN(return_value);
}


/*---------------------------------------------------------------------
 * parse_cmdline_execd
 *---------------------------------------------------------------------*/
static void parse_cmdline_execd(char **argv)
{
   lList *ref_list = NULL, *alp = NULL, *pcmdline = NULL;
   lListElem *aep;
   u_long32 help = 0;

   DENTER(TOP_LAYER, "parse_cmdline_execd");
            
   alp = sge_parse_cmdline_execd(argv+1, &pcmdline);
   if(alp) {
      /* 
      ** high level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      /* TODO: replace with alpp and DRETURN */
      SGE_EXIT(NULL, 1);
   }

   alp = sge_parse_execd(&pcmdline, &help);
   lFreeList(&pcmdline);
   lFreeList(&ref_list);

   if(alp) {
      /*
      ** low level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      /* TODO: replace with alpp and DRETURN */
      SGE_EXIT(NULL, 1);
   }
   lFreeList(&alp);

   if(help) {
      /*
      ** user wanted only help. we can exit!
      */
      /* TODO: replace with alpp and DRETURN */
      SGE_EXIT(NULL, 0);
   }
   DEXIT;
}


/*-------------------------------------------------------------
 * sge_parse_cmdline_execd
 *
 *-------------------------------------------------------------*/ 
static lList *sge_parse_cmdline_execd(char **argv, lList **ppcmdline)
{
char **sp;
char **rp;
stringT str;
lList *alp = NULL;

   DENTER(TOP_LAYER, "sge_parse_cmdline_execd");

   rp = argv;
   while(*(sp=rp)) {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", NULL, ppcmdline, &alp)) != sp)
         continue;

      /* -nostart-commd */
      if ((rp = parse_noopt(sp, "-nostart-commd", NULL, ppcmdline, &alp)) != sp)
         continue;

      /* -lj */
      if ((rp = parse_until_next_opt(sp, "-lj", NULL, ppcmdline, &alp)) != sp)
         continue;

      /* oops */
      snprintf(str, sizeof(str), MSG_PARSE_INVALIDARG_S, *sp);
      sge_usage(EXECD, stderr);
      answer_list_add(&alp, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return alp;
   }
   DEXIT;
   return alp;
}


/*-------------------------------------------------------------
 * sge_parse_execd
 *
 *-------------------------------------------------------------*/
static lList *sge_parse_execd(lList **ppcmdline, u_long32 *help) 
{
   lList *alp = NULL;
   int usageshowed = 0;

   DENTER(TOP_LAYER, "sge_parse_execd");

   /* Loop over all options. Only valid options can be in the
      ppcmdline list.
   */
   while(lGetNumberOfElem(*ppcmdline)) {
      /* -help */
      if(parse_flag(ppcmdline, "-help", &alp, help)) {
         usageshowed = 1;
         sge_usage(EXECD, stdout);
         break;
      }
   }
   
   if(lGetNumberOfElem(*ppcmdline)) {
      if(!usageshowed) {
         sge_usage(EXECD, stderr);
      }
      answer_list_add(&alp, MSG_PARSE_TOOMANYARGS, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return alp;
   }

   DEXIT;
   return alp;
}

/* JG: TODO: we have this searching code in many places!! */
/****** execd/execd_get_job_ja_task() ******************************************
*  NAME
*     execd_get_job_ja_task() -- search job and ja_task by id
*
*  SYNOPSIS
*     bool
*     execd_get_job_ja_task(u_long32 job_id, u_long32 ja_task_id,
*                           lListElem **job, lListElem **ja_task) 
*
*  FUNCTION
*     Searches the execd master lists for job and ja_task
*     defined by job_id and ja_task_id.
*
*  INPUTS
*     u_long32 job_id     - job id
*     u_long32 ja_task_id - ja_task id
*     lListElem **job     - returns job or NULL if not found
*     lListElem **ja_task - returns ja_task or NULL if not found
*
*  RESULT
*     bool - true if both job and ja_task are found, else false
*
*  NOTES
*     MT-NOTE: execd_get_job_ja_task() is MT safe 
*******************************************************************************/
bool execd_get_job_ja_task(u_long32 job_id, u_long32 ja_task_id, lListElem **job, lListElem **ja_task)
{
   const void *iterator = NULL;

   DENTER(TOP_LAYER, "execd_get_job_ja_task");

   *job = lGetElemUlongFirst(*(object_type_get_master_list(SGE_TYPE_JOB)),
                            JB_job_number, job_id, &iterator);
   while (*job != NULL) {
      *ja_task = job_search_task(*job, NULL, ja_task_id);
      if (*ja_task != NULL) {
         DRETURN(true);
      }

      /* in execd, we have exactly one ja_task per job,
       * therefore we can have multiple jobs with the same job_id
       */
      *job = lGetElemUlongNext(*(object_type_get_master_list(SGE_TYPE_JOB)),
                               JB_job_number, job_id, &iterator);
   }
   
   if (*job == NULL) {
      ERROR((SGE_EVENT, MSG_JOB_TASKWITHOUTJOB_U, sge_u32c(job_id))); 
   } else if (*ja_task == NULL) { 
      ERROR((SGE_EVENT, MSG_JOB_TASKNOTASKINJOB_UU, sge_u32c(job_id), sge_u32c(ja_task_id)));
   }

   *job = NULL;
   *ja_task = NULL;
   DRETURN(false);
}
