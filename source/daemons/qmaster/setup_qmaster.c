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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <sys/resource.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_os.h"
#include "uti/sge_hostname.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_spool.h"
#include "uti/setup_path.h"
#include "uti/config_file.h"
#include "uti/sge_time.h"

#include "evm/sge_event_master.h"

#include "spool/sge_spooling.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_resource_quota.h"

#include "gdi/qm_name.h"
#include "gdi/sge_gdi2.h"

#include "sched/sge_sched.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "sge_resource_quota_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_qinstance_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_sharetree_qmaster.h"
#include "sge_host_qmaster.h"
#include "sge_pe_qmaster.h"
#include "sge_cqueue_qmaster.h"
#include "sge_job_qmaster.h"
#include "sge_task_depend.h"
#include "sched_conf_qmaster.h"
#include "configuration_qmaster.h"
#include "lock.h"
#include "usage.h"
#include "shutdown.h"
#include "sge_give_jobs.h"
#include "sge_resource_quota_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_qinstance_qmaster.h"
#include "setup_qmaster.h"

#include "msg_daemons_common.h"
#include "msg_qmaster.h"
#include "msg_common.h"
   
struct cmplx_tmp {
   char *name;
   char *shortcut;
   u_long32 valtype;
   u_long32 relop;
   u_long32 consumable;
   char *valdefault;
   u_long32 requestable;
   char *urgency_weight;
};

static void   process_cmdline(char**);
static lList* parse_cmdline_qmaster(char**, lList**);
static lList* parse_qmaster(lList**, u_long32*);
static void   qmaster_init(sge_gdi_ctx_class_t *ctx, char**);
static void   communication_setup(sge_gdi_ctx_class_t *ctx);
static bool   is_qmaster_already_running(const char *qmaster_spool_dir);
static void   qmaster_lock_and_shutdown(void **ctx_ref, int);
static int    setup_qmaster(sge_gdi_ctx_class_t *ctx);

static int    
remove_invalid_job_references(bool job_spooling, int user, object_description *object_base);

static int    debit_all_jobs_from_qs(void);
static void   init_categories(void);


/****** qmaster/setup_qmaster/sge_setup_qmaster() ******************************
*  NAME
*     sge_setup_qmaster() -- setup qmaster 
*
*  SYNOPSIS
*     int sge_setup_qmaster(char* anArgv[]) 
*
*  FUNCTION
*     Process command line arguments. Remove qmaster lock file. Write qmaster
*     host to the 'act_qmaster' file. Initialize qmaster and reporting. Write
*     qmaster PID file.  
*
*     NOTE: Before this function is invoked, qmaster must become admin user.
*
*  INPUTS
*     char* anArgv[] - command line argument vector
*
*  RESULT
*     0 - success 
*
*  NOTES
*     MT-NOTE: sge_setup_qmaster() is NOT MT safe! 
*     MT-NOTE:
*     MT-NOTE: This function must be called exclusively, with the qmaster main
*     MT-NOTE: thread being the *only* active thread. In other words, do not
*     MT-NOTE: invoke this function after any additional thread (directly or
*     MT-NOTE: indirectly) has been created.
*
*     Do *not* write the qmaster pid file before 'qmaster_init()' returned
*     successfully. Otherwise, if we do have a running qmaster and a second
*     qmaster is started (illegally) on the same host, the second qmaster will
*     overwrite the pid of the qmaster started first. The second qmaster will
*     detect its insubordinate doing and terminate itself, thus leaving behind
*     a useless pid.
*
*******************************************************************************/
int sge_setup_qmaster(sge_gdi_ctx_class_t *ctx, char* anArgv[])
{
   char err_str[MAX_STRING_SIZE];
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   const char *act_qmaster_file = ctx->get_act_qmaster_file(ctx);
   const char *pidfile = getenv("SGE_QMASTER_PIDFILE");

   DENTER(TOP_LAYER, "sge_setup_qmaster");

   umask(022); /* this needs a better solution */

   process_cmdline(anArgv);

   INFO((SGE_EVENT, SFNMAX, MSG_STARTUP_BEGINWITHSTARTUP));

   qmaster_unlock(QMASTER_LOCK_FILE);

   if (write_qm_name(qualified_hostname, act_qmaster_file, err_str, sizeof(err_str))) {
      ERROR((SGE_EVENT, "%s", err_str));
      SGE_EXIT(NULL, 1);
   }

   qmaster_init(ctx, anArgv);

   if (!pidfile) pidfile = QMASTER_PID_FILE;
   sge_write_pid(pidfile);

   DEXIT;
   return 0;
} /* sge_setup_qmaster() */

/****** qmaster/setup_qmaster/sge_qmaster_thread_init() ************************
*  NAME
*     sge_qmaster_thread_init() -- Initialize a qmaster thread.
*
*  SYNOPSIS
*     int sge_qmaster_thread_init(sge_gdi_ctx_class_t **ctx_ref,
*                                 u_long32 prog_id, u_long32 thread_id,
*                                 bool switch_to_admin_user)
*
*  FUNCTION
*     Subsume functions which need to be called immediately after thread
*     startup. This function does make sure that the thread local data
*     structures do contain reasonable values.
*
*  INPUTS
*     bool switch_to_admin_user - become admin user if set to true
*
*  RESULT
*     0 - success 
*
*  NOTES
*     MT-NOTE: sge_qmaster_thread_init() is MT safe 
*     MT-NOTE:
*     MT-NOTE: sge_qmaster_thread_init() should be invoked at the beginning
*     MT-NOTE: of a thread function.
*
*******************************************************************************/
int 
sge_qmaster_thread_init(sge_gdi_ctx_class_t **ctx_ref, u_long32 prog_id, 
                        u_long32 thread_id, bool switch_to_admin_user)
{
   const char *admin_user = NULL;
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER(TOP_LAYER, "sge_qmaster_thread_init");

   lInit(nmv);

   if (sge_setup2(ctx_ref, prog_id, thread_id, &alp, true) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)ctx_ref, 1);
   }   
   ctx = *ctx_ref; 
   ctx->reresolve_qualified_hostname(ctx);
   DEBUG((SGE_EVENT,"%s: qualified hostname \"%s\"\n", SGE_FUNC, ctx->get_qualified_hostname(ctx)));
   admin_user = ctx->get_admin_user(ctx);
  
   if (switch_to_admin_user == true) {
      char str[MAX_STRING_SIZE];
      if (sge_set_admin_username(admin_user, str, sizeof(str)) == -1) {
         CRITICAL((SGE_EVENT, SFNMAX, str));
         SGE_EXIT((void**)ctx_ref, 1);
      }

      if (sge_switch2admin_user()) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_ERROR_CANTSWITCHTOADMINUSER));
         SGE_EXIT((void**)ctx_ref, 1);
      }
   }

   DEXIT;
   return 0;
} /* sge_qmaster_thread_init() */

/****** qmaster/setup_qmaster/sge_setup_job_resend() ***************************
*  NAME
*     sge_setup_job_resend() -- Setup job resend events.
*
*  SYNOPSIS
*     void sge_setup_job_resend(void) 
*
*  FUNCTION
*     Register a job resend event for each job or array task which does have a
*     'JTRANSFERING' status.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: sge_setup_job_resend() is not MT safe 
*
*******************************************************************************/
void sge_setup_job_resend(void)
{
   lListElem *job = NULL;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_setup_job_resend");

   job = lFirst(*object_base[SGE_TYPE_JOB].list);

   while (NULL != job)
   {
      lListElem *task;
      u_long32 job_num;

      job_num = lGetUlong(job, JB_job_number);

      task = lFirst(lGetList(job, JB_ja_tasks));
      
      while (NULL != task)
      {
         if (lGetUlong(task, JAT_status) == JTRANSFERING)
         {
            lListElem *granted_queue, *qinstance, *host;
            const char *qname;
            u_long32 task_num, when;
            te_event_t ev;

            task_num = lGetUlong(task, JAT_task_number);

            granted_queue = lFirst(lGetList(task, JAT_granted_destin_identifier_list));

            qname = lGetString(granted_queue, JG_qname);

            qinstance = cqueue_list_locate_qinstance(*object_base[SGE_TYPE_CQUEUE].list, qname);

            host = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, lGetHost(qinstance, QU_qhostname)); 

            when = lGetUlong(task, JAT_start_time);

            when += MAX(load_report_interval(host), MAX_JOB_DELIVER_TIME);

            ev = te_new_event((time_t)when, TYPE_JOB_RESEND_EVENT, ONE_TIME_EVENT, job_num, task_num, "job-resend_event");           
            te_add_event(ev);
            te_free_event(&ev);

            DPRINTF(("Did add job resend for "sge_u32"/"sge_u32" at "sge_u32"\n", job_num, task_num, when));
         }

         task = lNext(task);
      }

      job = lNext(job);
   }

   DEXIT;
   return;
} /* sge_setup_job_resend() */

/****** setup_qmaster/sge_process_qmaster_cmdline() ****************************
*  NAME
*     sge_process_qmaster_cmdline() -- global available function for qmaster
*
*  SYNOPSIS
*     void sge_process_qmaster_cmdline(char**anArgv) 
*
*  FUNCTION
*     This function simply calls the static function process_cmdline()
*
*  INPUTS
*     char**anArgv - command line arguments from main()
*
*  NOTES
*     MT-NOTE: sge_process_qmaster_cmdline() is NOT MT safe 
*
*  SEE ALSO
*     qmaster/setup_qmaster/process_cmdline()
*******************************************************************************/
void sge_process_qmaster_cmdline(char**anArgv) {
   process_cmdline(anArgv);
}
/****** qmaster/setup_qmaster/process_cmdline() ********************************
*  NAME
*     process_cmdline() -- Handle command line arguments 
*
*  SYNOPSIS
*     static void process_cmdline(char **anArgv) 
*
*  FUNCTION
*     Handle command line arguments. Parse argument vector and handle options.
*
*  INPUTS
*     char **anArgv - pointer to argument vector
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: process_cmdline() is NOT MT safe. 
*
*******************************************************************************/
static void process_cmdline(char **anArgv)
{
   lList *alp, *pcmdline;
   lListElem *aep;
   u_long32 help = 0;

   DENTER(TOP_LAYER, "process_cmdline");

   alp = pcmdline = NULL;

   alp = parse_cmdline_qmaster(&anArgv[1], &pcmdline);
   if(alp) {
      /*
      ** high level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      SGE_EXIT(NULL, 1);
   }

   alp = parse_qmaster(&pcmdline, &help);
   if(alp) {
      /*
      ** low level parsing error! show answer list
      */
      for_each(aep, alp) {
         fprintf(stderr, "%s", lGetString(aep, AN_text));
      }
      lFreeList(&alp);
      lFreeList(&pcmdline);
      SGE_EXIT(NULL, 1);
   }

   if(help) {
      /* user wanted to see help. we can exit */
      lFreeList(&pcmdline);
      SGE_EXIT(NULL, 0);
   }

   DEXIT;
   return;
} /* process_cmdline */

/****** qmaster/setup_qmaster/parse_cmdline_qmaster() **************************
*  NAME
*     parse_cmdline_qmaster() -- Parse command line arguments
*
*  SYNOPSIS
*     static lList* parse_cmdline_qmaster(char **argv, lList **ppcmdline) 
*
*  FUNCTION
*     Decompose argument vector. Handle options and option arguments. 
*
*  INPUTS
*     char **argv       - pointer to argument vector 
*     lList **ppcmdline - pointer to lList pointer which does contain the 
*                         command line arguments upon return. 
*
*  RESULT
*     lList* - pointer to answer list 
*
*  NOTES
*     MT-NOTE: parse_cmdline_qmaster() is MT safe. 
*
*******************************************************************************/
static lList *parse_cmdline_qmaster(char **argv, lList **ppcmdline )
{
   char **sp;
   char **rp;
   stringT str;
   lList *alp = NULL;

   DENTER(TOP_LAYER, "parse_cmdline_qmaster");

   rp = argv;
   while(*(sp=rp))
   {
      /* -help */
      if ((rp = parse_noopt(sp, "-help", NULL, ppcmdline, &alp)) != sp)
         continue;

      /* oops */
      snprintf(str, sizeof(str), MSG_PARSE_INVALIDOPTIONARGUMENTX_S, *sp);
      printf("%s\n", *sp);
      sge_usage(QMASTER, stderr);
      answer_list_add(&alp, str, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return alp;
   }

   DEXIT;
   return alp;
} /* parse_cmdline_qmaster() */

/****** qmaster/setup_qmaster/parse_qmaster() **********************************
*  NAME
*     parse_qmaster() -- Process options. 
*
*  SYNOPSIS
*     static lList* parse_qmaster(lList **ppcmdline, u_long32 *help) 
*
*  FUNCTION
*     Process options 
*
*  INPUTS
*     lList **ppcmdline - list of options
*     u_long32 *help    - flag is set upon return if help has been requested
*
*  RESULT
*     lList* - answer list 
*
*  NOTES
*     MT-NOTE: parse_qmaster() is not MT safe. 
*
*******************************************************************************/
static lList *parse_qmaster(lList **ppcmdline, u_long32 *help )
{
   lList *alp = NULL;
   int usageshowed = 0;

   DENTER(TOP_LAYER, "parse_qmaster");

   /* Loop over all options. Only valid options can be in the 
      ppcmdline list.
   */
   while(lGetNumberOfElem(*ppcmdline))
   {
      /* -help */
      if(parse_flag(ppcmdline, "-help", &alp, help)) {
         usageshowed = 1;
         sge_usage(QMASTER, stdout);
         break;
      }
   }

   if(lGetNumberOfElem(*ppcmdline)) {
      if(!usageshowed) {
         sge_usage(QMASTER, stderr);
      }
      answer_list_add(&alp, MSG_PARSE_TOOMANYOPTIONS, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return alp;
   }
   DEXIT;
   return alp;
} /* parse_qmaster() */

/****** qmaster/setup_qmaster/qmaster_init() ***********************************
*  NAME
*     qmaster_init() -- Initialize qmaster 
*
*  SYNOPSIS
*     static void qmaster_init(char **anArgv) 
*
*  FUNCTION
*     Initialize qmaster. Do general setup and communication setup. 
*
*  INPUTS
*     char **anArgv - process argument vector 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: qmaster_init() is NOT MT safe. 
*
*******************************************************************************/
static void qmaster_init(sge_gdi_ctx_class_t *ctx, char **anArgv)
{

   DENTER(TOP_LAYER, "qmaster_init");

   if (setup_qmaster(ctx)) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_STARTUP_SETUPFAILED));
      SGE_EXIT(NULL, 1);
   }

   ctx->set_exit_func(ctx, qmaster_lock_and_shutdown);   
  
   communication_setup(ctx);

   starting_up(); /* write startup info message to message file */

   DRETURN_VOID;
} /* qmaster_init() */

/****** qmaster/setup_qmaster/communication_setup() ****************************
*  NAME
*     communication_setup() -- set up communication
*
*  SYNOPSIS
*     static void communication_setup(void) 
*
*  FUNCTION
*     Initialize qmaster communication. 
*
*     This function will fail, if the configured qmaster port is already in
*     use.
*
*     This could happen if either qmaster has been terminated shortly before
*     and the operating system did not get around to free the port or there
*     is a qmaster already running.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: communication_setup() is NOT MT safe 
*
*******************************************************************************/
static void communication_setup(sge_gdi_ctx_class_t *ctx)
{
   cl_com_handle_t* com_handle = NULL;
   char* qmaster_params = NULL;
#if defined(IRIX)
   struct rlimit64 qmaster_rlimits;
#else
   struct rlimit qmaster_rlimits;
#endif

   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   u_long32 qmaster_port = ctx->get_sge_qmaster_port(ctx);
   const char *qmaster_spool_dir = ctx->get_qmaster_spool_dir(ctx);

   DENTER(TOP_LAYER, "communication_setup");

   DEBUG((SGE_EVENT,"my resolved hostname name is: \"%s\"\n", qualified_hostname));

   com_handle = cl_com_get_handle(prognames[QMASTER], 1);

   if (com_handle == NULL)
   {
      ERROR((SGE_EVENT, "port "sge_u32" already bound", qmaster_port));

      if (is_qmaster_already_running(qmaster_spool_dir) == true)
      {
         char *host = NULL;
         int res = -1; 

         res = cl_com_gethostname(&host, NULL, NULL,NULL);

         CRITICAL((SGE_EVENT, MSG_QMASTER_FOUNDRUNNINGQMASTERONHOSTXNOTSTARTING_S, ((CL_RETVAL_OK == res ) ? host : "unknown")));

         if (CL_RETVAL_OK == res) { 
            sge_free(&host); 
         }
      }

      SGE_EXIT(NULL, 1);
   }

   if (com_handle) {
      unsigned long max_connections = 0;
      u_long32 old_ll = 0;

      /* 
       * re-check file descriptor limits for qmaster 
       */
#if defined(IRIX)
      getrlimit64(RLIMIT_NOFILE, &qmaster_rlimits);
#else
      getrlimit(RLIMIT_NOFILE, &qmaster_rlimits);
#endif

      /* save old debug log level and set log level to INFO */
      old_ll = log_state_get_log_level();

      /* enable max connection close mode */
      cl_com_set_max_connection_close_mode(com_handle, CL_ON_MAX_COUNT_CLOSE_AUTOCLOSE_CLIENTS);

      cl_com_get_max_connections(com_handle, &max_connections);

      /* add local host to allowed host list */
      cl_com_add_allowed_host(com_handle,com_handle->local->comp_host);

      /* check dynamic event client count */
      mconf_set_max_dynamic_event_clients(sge_set_max_dynamic_event_clients(mconf_get_max_dynamic_event_clients())); 

      /* log startup info into qmaster messages file */
      log_state_set_log_level(LOG_INFO);
      INFO((SGE_EVENT, MSG_QMASTER_FD_HARD_LIMIT_SETTINGS_U, sge_u32c(qmaster_rlimits.rlim_max)));
      INFO((SGE_EVENT, MSG_QMASTER_FD_SOFT_LIMIT_SETTINGS_U, sge_u32c(qmaster_rlimits.rlim_cur)));
      INFO((SGE_EVENT, MSG_QMASTER_MAX_FILE_DESCRIPTORS_LIMIT_U, sge_u32c(max_connections)));
      INFO((SGE_EVENT, MSG_QMASTER_MAX_EVC_LIMIT_U, sge_u32c(mconf_get_max_dynamic_event_clients())));
      log_state_set_log_level(old_ll);
   }

   cl_commlib_set_connection_param(cl_com_get_handle(prognames[QMASTER], 1), HEARD_FROM_TIMEOUT, mconf_get_max_unheard());

   /* fetching qmaster_params and begin to parse */
   qmaster_params = mconf_get_qmaster_params();

   /* updating the commlib parameter list with new or changed parameters */
   cl_com_update_parameter_list(qmaster_params);
   DPRINTF(("received qmaster_params are: %s\n", qmaster_params));
   sge_free(&qmaster_params);

   /* now enable qmaster communication */
   cl_commlib_set_global_param(CL_COMMLIB_DELAYED_LISTEN, false);
   
   DEXIT;
   return;
} /* communication_setup() */

/****** qmaster/setup_qmaster/is_qmaster_already_running() *********************
*  NAME
*     is_qmaster_already_running() -- is qmaster already running 
*
*  SYNOPSIS
*     static bool is_qmaster_already_running(void) 
*
*  FUNCTION
*     Check whether there is a running qmaster already.
*
*  INPUTS
*     void - none 
*
*  RESULT
*     true  - running qmaster detected. 
*     false - otherwise
*
*  NOTES
*     MT-NOTE: is_qmaster_already_running() is not MT safe 
*
*  BUGS
*     This function will only work if the PID found in the qmaster PID file
*     either does belong to a running qmaster or no process at all.
*
*     Of course PIDs will be reused. This is, however, not a problem because
*     of the very specific situation in which this function is called.
*
*******************************************************************************/
static bool is_qmaster_already_running(const char *qmaster_spool_dir)
{
   enum { NULL_SIGNAL = 0 };

   bool res = true;
   char pidfile[SGE_PATH_MAX] = { '\0' };
   pid_t pid = 0;
   char *env_pidfile = getenv("SGE_QMASTER_PIDFILE");

   DENTER(TOP_LAYER, "is_qmaster_already_running");

   if (env_pidfile)
      snprintf(pidfile, sizeof(pidfile), "%s", env_pidfile);
   else
      snprintf(pidfile, sizeof(pidfile), "%s/%s", qmaster_spool_dir,
               QMASTER_PID_FILE);

   if ((pid = sge_readpid(pidfile)) == 0)
   {
      DEXIT;
      return false;
   }

   res = (kill(pid, NULL_SIGNAL) == 0) ? true: false;

   DEXIT;
   return res;
} /* is_qmaster_already_running() */


static void sge_propagate_queue_suspension(object_description *object_base, lListElem *jep,
                                           dstring *cqueue_name, dstring *host_domain)
{
   const lListElem *gdil_ep, *cq, *qi;
   lListElem *jatep;

   for_each (jatep, lGetList(jep, JB_ja_tasks)) {
      u_long32 jstate = lGetUlong(jatep, JAT_state); 
      bool is_suspended = false;

      for_each (gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list)) {

         if (!cqueue_name_split(lGetString(gdil_ep, JG_qname), cqueue_name, host_domain, NULL, NULL)) {
            continue;
         } 

         if (!(cq = lGetElemStr(*object_base[SGE_TYPE_CQUEUE].list, CQ_name, sge_dstring_get_string(cqueue_name))) ||
             !(qi = lGetElemHost(lGetList(cq, CQ_qinstances), QU_qhostname, sge_dstring_get_string(host_domain)))) 
            continue;

         if (qinstance_state_is_manual_suspended(qi) ||
             qinstance_state_is_susp_on_sub(qi) ||
             qinstance_state_is_cal_suspended(qi)) {
             is_suspended = true;
             break;
         }
      }

      if (is_suspended)
         jstate |= JSUSPENDED_ON_SUBORDINATE;
      else
         jstate &= ~JSUSPENDED_ON_SUBORDINATE;
      lSetUlong(jatep, JAT_state, jstate);
   }
}

/****** qmaster/setup_qmaster/qmaster_lock_and_shutdown() ***************************
*  NAME
*     qmaster_lock_and_shutdown() -- Acquire qmaster lock file and shutdown 
*
*  SYNOPSIS
*     static void qmaster_lock_and_shutdown(int anExitValue) 
*
*  FUNCTION
*     qmaster exit function. This version MUST NOT be used, if the current
*     working   directory is NOT the spool directory. Other components do rely
*     on finding the lock file in the spool directory.
*
*  INPUTS
*     int anExitValue - exit value 
*
*  RESULT
*     void - none
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: qmaster_lock_and_shutdown() is MT safe 
*
*******************************************************************************/
static void qmaster_lock_and_shutdown(void **ctx_ref, int anExitValue)
{
   DENTER(TOP_LAYER, "qmaster_lock_and_shutdown");
   
   if (anExitValue == 0) {
      if (qmaster_lock(QMASTER_LOCK_FILE) == -1) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_QMASTER_LOCKFILE_ALREADY_EXISTS));
      }
   }
   sge_gdi2_shutdown(ctx_ref);

   DEXIT;
   return;
} /* qmaster_lock_and_shutdown() */


static int setup_qmaster(sge_gdi_ctx_class_t *ctx)
{
   lListElem *jep, *ep, *tmpqep;
   static bool first = true;
   lListElem *spooling_context = NULL;
   lList *answer_list = NULL;
   time_t time_start, time_end;
   monitoring_t monitor;
   object_description *object_base = object_type_get_object_description();
   const char *qualified_hostname = NULL;

   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "setup_qmaster");

   if (first) {
      first = false;
   } else {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SETUP_SETUPMAYBECALLEDONLYATSTARTUP));
      DEXIT;
      return -1;
   }   

   /* register our error function for use in replace_params() */
   config_errfunc = set_error;

   /*
    * Initialize Master lists and hash tables, if necessary 
    */

/** This part is making the scheduler a
  * lot slower that it was before. This was an enhancement introduced
  * in cvs revision 1.35
  * It might be added again, when the hashing problem on large job lists
  * with only a few owners is solved.
  */
#if 0
    if (*(object_base[SGE_TYPE_JOB].list) == NULL) {
       *(object_base[SGE_TYPE_JOB].list) = lCreateList("Master_Job_List", JB_Type);
    }
    cull_hash_new(*(object_base[SGE_TYPE_JOB].list), JB_owner, 0);
#endif

   if (!sge_initialize_persistence(ctx, &answer_list)) {
      answer_list_output(&answer_list);
      DRETURN(-1);
   } else {
      answer_list_output(&answer_list);
      spooling_context = spool_get_default_context();
   }

   if (sge_read_configuration(ctx, spooling_context, object_base[SGE_TYPE_CONFIG].list, answer_list) != 0) {
      DRETURN(-1);
   }
   
   mconf_set_new_config(true);

   /* get aliased hostname from commd */
   ctx->reresolve_qualified_hostname(ctx);
   qualified_hostname = ctx->get_qualified_hostname(ctx);
   DEBUG((SGE_EVENT,"ctx->get_qualified_hostname(ctx) returned \"%s\"\n", qualified_hostname));

   /*
   ** read in all objects and check for correctness
   */
   DPRINTF(("Complex Attributes----------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_CENTRY].list, SGE_TYPE_CENTRY);
   answer_list_output(&answer_list);

   /*
    * for release 6.2u5 the "job to core"- binding feature has been added 
    * that needs some additional complex entries. We check here if those
    * entries exist and create them silently if they are not there. Only
    * this prevents from creating a update procedure.
    *
    * TODO: As soon as there is a release where an update procedure is 
    *       available we should put that code there and remove it here.
    */
   {
      struct cmplx_tmp new_complexes[] = {
         {"m_thread", "thread", 1, CMPLXLE_OP, CONSUMABLE_NO, "0", REQU_YES, "0"},
         {"m_core", "core", 1, CMPLXLE_OP, CONSUMABLE_NO, "0", REQU_YES, "0"},
         {"m_socket", "socket", 1, CMPLXLE_OP, CONSUMABLE_NO, "0", REQU_YES, "0"},
         {"m_topology", "topo", 9, CMPLXEQ_OP, CONSUMABLE_NO, NULL, REQU_YES, "0"},
         {"m_topology_inuse", "utopo", 9, CMPLXEQ_OP, CONSUMABLE_NO, NULL, REQU_YES, "0"},
         {NULL, NULL, 0, 0, 0, NULL, 0, 0}
      };
      int i;

      for (i = 0; new_complexes[i].name != NULL; i++) {
         lList *centry_list = *(object_base[SGE_TYPE_CENTRY].list);
         lListElem *entry_long = lGetElemStr(centry_list, CE_name, new_complexes[i].name);
         lListElem *entry_short = lGetElemStr(centry_list, CE_shortcut, new_complexes[i].shortcut);

         if (entry_long == NULL && entry_short == NULL) {
            lListElem *new_centry = lCreateElem(CE_Type);

            lSetString(new_centry, CE_name, new_complexes[i].name);
            lSetString(new_centry, CE_shortcut, new_complexes[i].shortcut);
            lSetString(new_centry, CE_default, new_complexes[i].valdefault);
            lSetString(new_centry, CE_urgency_weight, new_complexes[i].urgency_weight);
            lSetUlong(new_centry, CE_valtype, new_complexes[i].valtype);
            lSetUlong(new_centry, CE_relop, new_complexes[i].relop);
            lSetUlong(new_centry, CE_consumable, new_complexes[i].consumable);
            lSetUlong(new_centry, CE_requestable, new_complexes[i].requestable);

            /* append and spool the object */
            lAppendElem(centry_list, new_centry);
            spool_write_object(NULL, spool_get_default_context(), new_centry,
                               lGetString(new_centry, CE_name), SGE_TYPE_CENTRY, false);

         }
      }
   }

   DPRINTF(("host_list----------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_EXECHOST].list, SGE_TYPE_EXECHOST);
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_ADMINHOST].list, SGE_TYPE_ADMINHOST);
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_SUBMITHOST].list, SGE_TYPE_SUBMITHOST);
   answer_list_output(&answer_list);

   if (!host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, SGE_TEMPLATE_NAME)) {
      /* add an exec host "template" */
      if (sge_add_host_of_type(ctx, SGE_TEMPLATE_NAME, SGE_EH_LIST, &monitor))
         ERROR((SGE_EVENT, SFNMAX, MSG_CONFIG_ADDINGHOSTTEMPLATETOEXECHOSTLIST));
   }

   /* add host "global" to Master_Exechost_List as an exec host */
   if (!host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, SGE_GLOBAL_NAME)) {
      /* add an exec host "global" */
      if (sge_add_host_of_type(ctx, SGE_GLOBAL_NAME, SGE_EH_LIST, &monitor))
         ERROR((SGE_EVENT, SFNMAX, MSG_CONFIG_ADDINGHOSTGLOBALTOEXECHOSTLIST));
   }

   /* add qmaster host to Master_Adminhost_List as an administrative host */
   if (!host_list_locate(*object_base[SGE_TYPE_ADMINHOST].list, qualified_hostname)) {
      if (sge_add_host_of_type(ctx, qualified_hostname, SGE_AH_LIST, &monitor)) {
         DRETURN(-1);
      }
   }

   DPRINTF(("manager_list----------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_MANAGER].list, SGE_TYPE_MANAGER);
   answer_list_output(&answer_list);
   if (!manop_is_manager("root")) {
      ep = lAddElemStr(object_base[SGE_TYPE_MANAGER].list, UM_name, "root", UM_Type);

      if (!spool_write_object(&answer_list, spooling_context, ep, "root", SGE_TYPE_MANAGER, job_spooling)) {
         answer_list_output(&answer_list);
         CRITICAL((SGE_EVENT, SFNMAX, MSG_CONFIG_CANTWRITEMANAGERLIST));
         DRETURN(-1);
      }
   }
   for_each(ep, *object_base[SGE_TYPE_MANAGER].list) {
      DPRINTF(("%s\n", lGetString(ep, UM_name)));
   }   

   DPRINTF(("host group definitions-----------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_HGROUP].list, SGE_TYPE_HGROUP);
   answer_list_output(&answer_list);

   DPRINTF(("operator_list----------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_OPERATOR].list, SGE_TYPE_OPERATOR);
   answer_list_output(&answer_list);
   if (!manop_is_operator("root")) {
      ep = lAddElemStr(object_base[SGE_TYPE_OPERATOR].list, UO_name, "root", UO_Type);

      if (!spool_write_object(&answer_list, spooling_context, ep, "root", SGE_TYPE_OPERATOR, job_spooling)) {
         answer_list_output(&answer_list);
         CRITICAL((SGE_EVENT, SFNMAX, MSG_CONFIG_CANTWRITEOPERATORLIST));
         DEXIT;
         return -1;
      }
   }
   for_each(ep, *object_base[SGE_TYPE_OPERATOR].list) {
      DPRINTF(("%s\n", lGetString(ep, UO_name)));
   }   


   DPRINTF(("userset_list------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_USERSET].list, SGE_TYPE_USERSET);
   answer_list_output(&answer_list);

   DPRINTF(("calendar list ------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_CALENDAR].list, SGE_TYPE_CALENDAR);
   answer_list_output(&answer_list);

   DPRINTF(("resource quota list -----------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_RQS].list, SGE_TYPE_RQS);
   answer_list_output(&answer_list);

#ifndef __SGE_NO_USERMAPPING__
   DPRINTF(("administrator user mapping-----------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_CUSER].list, SGE_TYPE_CUSER);
   answer_list_output(&answer_list);
#endif

   DPRINTF(("cluster_queue_list---------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_CQUEUE].list, SGE_TYPE_CQUEUE);
   answer_list_output(&answer_list);
   cqueue_list_set_unknown_state(*(object_base[SGE_TYPE_CQUEUE].list), NULL, false, true);

   /*
    * Initialize cached values for each qinstance:
    *    - fullname
    *    - suspend_on_subordinate
    */
   for_each(tmpqep, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      lList     *qinstance_list = lGetList(tmpqep, CQ_qinstances);
      lListElem *qinstance = NULL;
      lList     *aso_list = NULL;
      lListElem *aso = NULL;

      /*
       * Update cluster queue configuration from pre-6.2u5 to 6.2u5, i.e. from
       * cluster queues without slotwise suspend on subordinate to such with 
       * slotwise ssos.
       */
      aso_list = lGetList(tmpqep, CQ_subordinate_list);
      if (aso_list != NULL) {
         /* This cluster queue has a list of subordinate lists (possibly one
          * for each host).*/
         for_each (aso, aso_list) {
            lListElem *elem;
            int   pos;
            lList *so_list = lGetList(aso, ASOLIST_value);
            
            /* Every element of the ASOLIST should have a SOLIST, but we
             * check it to be sure.
             */
            if (so_list != NULL) {
               /* In each subordinate list, all elements must have the same
                * SO_slots_sum value, so it's enough to look in the first
                * element.
                */
               elem = lFirst(so_list);
               pos = lGetPosViaElem(elem, SO_slots_sum, SGE_NO_ABORT);
               if (pos == -1) {
                  /* In the subordinate list of the cluster queue there is no
                   * SO_slots_sum field yet, so we have to create a new
                   * subordinate list, copy the values from the old one and
                   * initialize the new fields, remove the old subordinate list
                   * from the cluster queue and add the new one instead.
                   */
                  lList      *new_so_list = NULL;
                  lListElem  *so = NULL;
                  lListElem  *new_so = NULL;
                  const char *so_list_name = NULL;

                  so_list_name = lGetListName(so_list);
                  new_so_list = lCreateList(so_list_name, SO_Type);

                  for_each (so, so_list) {
                     new_so = lCreateElem(SO_Type);
                     lSetString(new_so, SO_name, lGetString(so, SO_name));
                     lSetUlong(new_so, SO_threshold, lGetUlong(so, SO_threshold));
                     lSetUlong(new_so, SO_slots_sum, 0);
                     lSetUlong(new_so, SO_seq_no, 0);
                     lSetUlong(new_so, SO_action, 0);
                     lAppendElem(new_so_list, new_so);
                  }
                  lSetList(aso, ASOLIST_value, new_so_list);
               }
            }
         }
      }
      for_each(qinstance, qinstance_list) {
         qinstance_set_full_name(qinstance);
         sge_qmaster_qinstance_state_set_susp_on_sub(qinstance, false);
      }
   }

   DPRINTF(("pe_list---------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_PE].list, SGE_TYPE_PE);
   answer_list_output(&answer_list);

   DPRINTF(("ckpt_list---------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_CKPT].list, SGE_TYPE_CKPT);
   answer_list_output(&answer_list);

   DPRINTF(("advance reservation list -----------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_AR].list, SGE_TYPE_AR);
   answer_list_output(&answer_list);

   /* initialize cached advance reservations structures */
   {
      lListElem *ar;
      for_each(ar, *object_base[SGE_TYPE_AR].list) {
         ar_initialize_reserved_queue_list(ar);
      }
   }

   DPRINTF(("job_list-----------------------------------\n"));
   /* measure time needed to read job database */
   time_start = time(0);
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_JOB].list, SGE_TYPE_JOB);
   time_end = time(0);
   answer_list_output(&answer_list);

   {
      u_long32 saved_logginglevel = log_state_get_log_level();
      log_state_set_log_level(LOG_INFO);
      INFO((SGE_EVENT, MSG_QMASTER_READ_JDB_WITH_X_ENTR_IN_Y_SECS_UU,
            sge_u32c(lGetNumberOfElem(*object_base[SGE_TYPE_JOB].list)), 
            sge_u32c(time_end - time_start)));
      log_state_set_log_level(saved_logginglevel);
   }

   {
      dstring cqueue_name = DSTRING_INIT;
      dstring host_domain = DSTRING_INIT;

      for_each(jep, *(object_base[SGE_TYPE_JOB].list)) {

         DPRINTF(("JOB "sge_u32" PRIORITY %d\n", lGetUlong(jep, JB_job_number), 
               (int)lGetUlong(jep, JB_priority) - BASE_PRIORITY));

         /* doing this operation we need the complete job list read in */
         job_suc_pre(jep);
         
         /* also do this for array dependency predecessors */
         job_suc_pre_ad(jep);

         /* array successor jobs need to have their cache rebuilt. this will
            do nothing spectacular if the AD request list for this job is empty. */
         sge_task_depend_init(jep, &answer_list);

         centry_list_fill_request(lGetList(jep, JB_hard_resource_list), 
                     NULL, *object_base[SGE_TYPE_CENTRY].list, false, true, false);

         /* need to update JSUSPENDED_ON_SUBORDINATE since task spooling is not 
            triggered upon queue un/-suspension */
         sge_propagate_queue_suspension(object_base, jep, &cqueue_name, &host_domain);
      }
      sge_dstring_free(&cqueue_name);
      sge_dstring_free(&host_domain);
   }

   if (!ctx->get_job_spooling(ctx)) {
      lList *answer_list = NULL;
      dstring buffer = DSTRING_INIT;

      INFO((SGE_EVENT, "job spooling is disabled - removing spooled jobs"));

      ctx->set_job_spooling(ctx, true);
      
      for_each(jep, *object_base[SGE_TYPE_JOB].list) {
         u_long32 job_id = lGetUlong(jep, JB_job_number);
         sge_dstring_clear(&buffer);

         if (lGetString(jep, JB_exec_file) != NULL) {
            if (spool_read_script(&answer_list, job_id, jep) == true) {
               spool_delete_script(&answer_list, job_id, jep);
            }  else {
               printf("could not read in script file\n");
            }
         }
         spool_delete_object(&answer_list, spool_get_default_context(), 
                             SGE_TYPE_JOB, 
                             job_get_key(job_id, 0, NULL, &buffer),
                             job_spooling);                     
      }
      answer_list_output(&answer_list);
      sge_dstring_free(&buffer);
      ctx->set_job_spooling(ctx, true);
   }

   /* 
      if the job is in state running 
      we have to register each slot 
      in a queue, in the resource quota sets
      and in the parallel 
      environment if the job is a 
      parallel one
   */
   debit_all_jobs_from_qs(); 
   debit_all_jobs_from_pes(*object_base[SGE_TYPE_PE].list); 

   /*
    * Initialize cached values for each qinstance:
    *    - update suspend on subordinate state according to running jobs
    *    - update cached QI values.
    */
   for_each(tmpqep, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      cqueue_mod_qinstances(ctx, tmpqep, NULL, tmpqep, true, false, &monitor);
   }

   /* rebuild signal resend events */
   rebuild_signal_events();


   DPRINTF(("user list-----------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_USER].list, SGE_TYPE_USER);
   answer_list_output(&answer_list);

   remove_invalid_job_references(job_spooling, 1, object_base);

   DPRINTF(("project list-----------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_PROJECT].list, SGE_TYPE_PROJECT);
   answer_list_output(&answer_list);

   remove_invalid_job_references(job_spooling, 0, object_base);
   
   DPRINTF(("scheduler config -----------------------------------\n"));
   
   sge_read_sched_configuration(ctx, spooling_context, &answer_list);
   answer_list_output(&answer_list);

   DPRINTF(("share tree list-----------------------------------\n"));
   spool_read_list(&answer_list, spooling_context, object_base[SGE_TYPE_SHARETREE].list, SGE_TYPE_SHARETREE);
   answer_list_output(&answer_list);
   ep = lFirst(*object_base[SGE_TYPE_SHARETREE].list);
   if (ep) {
      lList *alp = NULL;
      lList *found = NULL;
      check_sharetree(&alp, ep, *object_base[SGE_TYPE_USER].list, 
                      *object_base[SGE_TYPE_PROJECT].list, NULL, &found);
      lFreeList(&found);
      lFreeList(&alp); 
   }


   init_categories();

   DRETURN(0);
}

/****** setup_qmaster/remove_invalid_job_references() **************************
*  NAME
*     remove_invalid_job_references() -- ??? 
*
*  SYNOPSIS
*     static int remove_invalid_job_references(int user, object_description 
*     *object_base) 
*
*  FUNCTION
*   get rid of still debited per job usage contained 
*   in user or project object if the job is no longer existing
*
*  INPUTS
*     int user                        - work on users
*     object_description *object_base - master list table
*
*  RESULT
*     static int -  always 0
*
*  NOTES
*     MT-NOTE: remove_invalid_job_references() is not MT safe 
*
*******************************************************************************/
static int 
remove_invalid_job_references(bool job_spooling, int user, object_description *object_base) 
{
   lListElem *up, *upu, *next;
   u_long32 jobid;
   int object_key = user ? UU_name : PR_name;
   lList *object_list = user ? *object_base[SGE_TYPE_USER].list : *object_base[SGE_TYPE_PROJECT].list;
   sge_object_type object_type = user ? SGE_TYPE_USER : SGE_TYPE_PROJECT;
   const char *object_name = user ? MSG_OBJ_USER : MSG_OBJ_PRJ;
   int debited_job_usage_key = user ? UU_debited_job_usage : PR_debited_job_usage;

   DENTER(TOP_LAYER, "remove_invalid_job_references");

   for_each(up, object_list) {
      int spool_me = 0;
      next = lFirst(lGetList(up, debited_job_usage_key));
      while ((upu=next)) {
         next = lNext(upu);

         jobid = lGetUlong(upu, UPU_job_number);
         if (!job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), jobid)) {
            lRemoveElem(lGetList(up, debited_job_usage_key), &upu);
            WARNING((SGE_EVENT, "removing reference to no longer existing job "sge_u32" of %s "SFQ,
                           jobid, object_name, lGetString(up, object_key)));
            spool_me = 1;
         }
      }

      if (spool_me) {
         lList *answer_list = NULL;
         spool_write_object(&answer_list, spool_get_default_context(), up, 
                            lGetString(up, object_key), object_type, job_spooling);
         answer_list_output(&answer_list);
      }
   }

   DEXIT;
   return 0;
}

static int debit_all_jobs_from_qs()
{
   lListElem *gdi;
   u_long32 slots;
   const char *queue_name;
   lListElem *next_jep  = NULL;
   lListElem *jep  = NULL;
   lListElem *qep  = NULL;
   lListElem *next_jatep = NULL;
   lListElem *jatep = NULL;
   int ret = 0;
   object_description *object_base = object_type_get_object_description();
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   lList *master_ar_list = *object_base[SGE_TYPE_AR].list;
   lList *master_rqs_list = *object_base[SGE_TYPE_RQS].list;

   DENTER(TOP_LAYER, "debit_all_jobs_from_qs");

   next_jep = lFirst(*(object_type_get_master_list(SGE_TYPE_JOB)));
   while ((jep=next_jep)) {
   
      /* may be we have to delete this job */   
      next_jep = lNext(jep);
      
      next_jatep = lFirst(lGetList(jep, JB_ja_tasks));
      while ((jatep = next_jatep)) {
         bool master_task = true;
         next_jatep = lNext(jatep);

         /* don't look at states - we only trust in 
            "granted destin. ident. list" */

         for_each (gdi, lGetList(jatep, JAT_granted_destin_identifier_list)) {
            u_long32 ar_id = lGetUlong(jep, JB_ar);
            lListElem *ar   = NULL;

            queue_name = lGetString(gdi, JG_qname);
            slots = lGetUlong(gdi, JG_slots);
            
            if (!(qep = cqueue_list_locate_qinstance(master_cqueue_list, queue_name))) {
               ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDQUEUEXREFERENCEDINJOBY_SU,  
                      queue_name, sge_u32c(lGetUlong(jep, JB_job_number))));
               lRemoveElem(lGetList(jep, JB_ja_tasks), &jatep);
            } else if (ar_id != 0 && (ar = lGetElemUlong(master_ar_list, AR_id, ar_id)) == NULL) {
               ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDARXREFERENCEDINJOBY_UU,  
                      sge_u32c(ar_id), sge_u32c(lGetUlong(jep, JB_job_number))));
               lRemoveElem(lGetList(jep, JB_ja_tasks), &jatep);
            } else {
               /* debit in all layers */
               lListElem *rqs = NULL;
               debit_host_consumable(jep, host_list_locate(*object_base[SGE_TYPE_EXECHOST].list,
                                     "global"), master_centry_list, slots, master_task, NULL);
               debit_host_consumable(jep, host_list_locate(
                        *object_base[SGE_TYPE_EXECHOST].list, lGetHost(qep, QU_qhostname)),
                        master_centry_list, slots, master_task, NULL);
               qinstance_debit_consumable(qep, jep, master_centry_list, slots, master_task, NULL);
               for_each (rqs, master_rqs_list) {
                  rqs_debit_consumable(rqs, jep, gdi, lGetString(jatep, JAT_granted_pe), master_centry_list, 
                                        *object_base[SGE_TYPE_USERSET].list, *object_base[SGE_TYPE_HGROUP].list, slots, master_task);
               }
               if (ar != NULL) {
                  lListElem *queue = lGetSubStr(ar, QU_full_name, lGetString(gdi, JG_qname), AR_reserved_queues);
                  if (queue != NULL) {
                     qinstance_debit_consumable(queue, jep, master_centry_list, slots, master_task, NULL);
                  } else {
                     ERROR((SGE_EVENT, "job "sge_U32CFormat" runs in queue "SFQ" not reserved by AR "sge_U32CFormat,  
                            sge_u32c(lGetUlong(jep, JB_job_number)), lGetString(gdi, JG_qname), sge_u32c(ar_id)));
                  }
               }
            }
            master_task = false;
         }
      }
   }

   DRETURN(ret);
}

/****** setup_qmaster/init_categories() ****************************************
*  NAME
*     init_categories() -- Initialize usersets/projects wrts categories
*
*  SYNOPSIS
*     static void init_categories(void)
*
*  FUNCTION
*     Initialize usersets/projects wrts categories.
*
*  NOTES
*     MT-NOTE: init_categories() is not MT safe
*******************************************************************************/
static void init_categories(void)
{
   const lListElem *cq, *pe, *hep, *ep;
   lListElem *acl, *prj, *rqs;
   lList *u_list = NULL, *p_list = NULL;
   lList *master_project_list = (*object_type_get_master_list(SGE_TYPE_PROJECT));
   lList *master_userset_list = (*object_type_get_master_list(SGE_TYPE_USERSET));
   bool all_projects = false;
   bool all_usersets = false;

   /*
    * collect a list of references to usersets/projects used in
    * the resource quota sets
    */
   for_each (rqs, *object_type_get_master_list(SGE_TYPE_RQS)) {
      if (!all_projects && !rqs_diff_projects(rqs, NULL, &p_list, NULL, master_project_list)) {
         all_projects = true;
      }
      if (!all_usersets && !rqs_diff_usersets(rqs, NULL, &u_list, NULL, master_userset_list)) {
         all_usersets = true;
      }
      if (all_usersets && all_projects) {
         break;
      }
   }

   /*
    * collect list of references to usersets/projects used as ACL
    * with queue_conf(5), host_conf(5) and sge_pe(5)
    */
   for_each (cq, *object_type_get_master_list(SGE_TYPE_CQUEUE)) {
      cqueue_diff_projects(cq, NULL, &p_list, NULL);
      cqueue_diff_usersets(cq, NULL, &u_list, NULL);
   }

   for_each (pe, *object_type_get_master_list(SGE_TYPE_PE)) {
      pe_diff_usersets(pe, NULL, &u_list, NULL);
   }

   for_each (hep, *object_type_get_master_list(SGE_TYPE_EXECHOST)) {
      host_diff_projects(hep, NULL, &p_list, NULL);
      host_diff_usersets(hep, NULL, &u_list, NULL);
   }

   /*
    * now set categories flag with usersets/projects used as ACL
    */
   for_each(ep, p_list)
      if ((prj = prj_list_locate(master_project_list, lGetString(ep, PR_name))))
         lSetBool(prj, PR_consider_with_categories, true);

   for_each(ep, u_list)
      if ((acl = userset_list_locate(master_userset_list, lGetString(ep, US_name))))
         lSetBool(acl, US_consider_with_categories, true);
   
   lFreeList(&p_list);
   lFreeList(&u_list);
}
