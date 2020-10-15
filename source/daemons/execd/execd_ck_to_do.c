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
#include <sys/types.h>
#include <signal.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_hostname.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_signal.h"
#include "uti/sge_string.h"
#include "uti/sge_stdio.h"
#include "uti/sge_uidgid.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_str.h"

#include "sched/sge_job_schedd.h"

#include "gdi/sge_security.h"

#include "spool/classic/read_write_job.h"

#include "dispatcher.h"
#include "reaper_execd.h"
#include "job_report_execd.h"
#include "load_avg.h"
#include "execd_ck_to_do.h"
#include "execd_signal_queue.h"
#include "symbols.h"
#include "exec_job.h"
#include "execution_states.h"
#include "msg_execd.h"
#include "get_path.h"
#include "sig_handlers.h"
#include "sge.h"
#include "msg_common.h"

#ifdef COMPILE_DC
#  include "ptf.h"
#  ifdef DEBUG_DC
#     include "sgedefs.h"
#     include "exec_ifm.h"
#  endif
#endif

extern volatile int waiting4osjid;

static bool 
sge_execd_ja_task_is_tightly_integrated(const lListElem *ja_task);
static bool
sge_kill_petasks(const lListElem *job, const lListElem *ja_task);

static int sge_start_jobs(sge_gdi_ctx_class_t *ctx);
static int exec_job_or_task(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jatep, lListElem *petep);

#define ADDGRPID "addgrpid"
#define OSJOBID "osjobid"

#ifdef COMPILE_DC
static void force_job_rlimit(const char* qualified_hostname);
#endif

extern volatile int jobs_to_start;

extern lList *jr_list;

#ifdef COMPILE_DC
static void notify_ptf(void)
{
   lListElem *jep, *tep;
   int write_job = -1;

   DENTER(TOP_LAYER, "notify_ptf");

#ifdef DEBUG_DC
   ptf_show_registered_jobs();
#endif

   /* ck joblist if there are still jobs waiting for osjobid */
   if (waiting4osjid) {
      waiting4osjid = 0;

      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         lListElem* jatep;
         
         for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            write_job = 0;
            if (lGetUlong(jatep, JAT_status) == JWAITING4OSJID) {
               switch (register_at_ptf(jep, jatep, NULL)) {
                  case 0:   
                     /* succeeded */
                     lSetUlong(jatep, JAT_status, JRUNNING);
                  
                     /* spool state transition */ 
                     write_job = 1;
       
                     break;
                  case 1: 
                     /* still waiting for osjobid - nothing changes */
                     waiting4osjid = 1;
                     break;
                  default:
                     /* should add here some cleanup code to remove job */
                     break;
               }
            }

            for_each(tep, lGetList(jatep, JAT_task_list)) {
               if (lGetUlong(tep, PET_status) == JWAITING4OSJID) {
                  switch (register_at_ptf(jep, jatep, tep)) {
                     case 0:   
                        /* succeeded */
                        lSetUlong(tep, PET_status, JRUNNING);

                        /* spool state transition */ 
                        write_job = 1;
                        break;

                     case 1: 
                        /* still waiting for osjobid - nothing changes */
                        waiting4osjid = 1;
                        break;

                     default:
                        /* should add here some cleanup code to remove sub-task */
                        break;
                   }
               }
            }
            if (write_job && !mconf_get_simulate_jobs())
               job_write_spool_file(jep, lGetUlong(jatep, JAT_task_number), 
                                    NULL, SPOOL_WITHIN_EXECD);
         }
      }

      if (waiting4osjid) {
         DPRINTF(("still waiting for osjobids\n"));
      } else {
         DPRINTF(("got all osjobids\n"));
      }   
   }

   DEXIT;
   return;
}

/* force job resource limits */
static void force_job_rlimit(const char* qualified_hostname)
{
   lListElem *jep;

   DENTER(TOP_LAYER, "force_job_rlimit");

   for_each (jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
      lListElem *jatep;

      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         lListElem *q=NULL, *cpu_ep, *vmem_ep, *gdil_ep;
         double cpu_val, vmem_val;
         double s_cpu, h_cpu;
         double s_vmem, h_vmem;
         int cpu_exceeded;
         lList *usage_list;
         u_long32 jobid, jataskid;

         jobid = lGetUlong(jep, JB_job_number);
         jataskid = lGetUlong(jatep, JAT_task_number);

         cpu_val = vmem_val = s_cpu = h_cpu = s_vmem = h_vmem = 0;

         /* retrieve cpu and vmem usage */
         usage_list = ptf_get_job_usage(jobid, jataskid, "*");
         if (usage_list == NULL) {
            continue;
         }

         if ((cpu_ep = lGetElemStr(usage_list, UA_name, USAGE_ATTR_CPU))) {
            cpu_val = lGetDouble(cpu_ep, UA_value);
         }

         if ((vmem_ep = lGetElemStr(usage_list, UA_name, USAGE_ATTR_MAXVMEM))) {
            vmem_val = lGetDouble(vmem_ep, UA_value);
         }
            
         DPRINTF(("JOB "sge_u32" %s %10.5f %s %10.5f\n", jobid,
            cpu_ep != NULL ? USAGE_ATTR_CPU : "("USAGE_ATTR_CPU")", cpu_val,
            vmem_ep != NULL ? USAGE_ATTR_VMEM : "("USAGE_ATTR_MAXVMEM")", vmem_val));

         /* free no longer needed usage_list */
         lFreeList(&usage_list);
         cpu_ep = vmem_ep = NULL;

         for_each (gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list)) {
            int nslots=0;
            double lim;
            char err_str[128];
            size_t err_size = sizeof(err_str) - 1;

            if (sge_hostcmp(qualified_hostname, lGetHost(gdil_ep, JG_qhostname))
                || !(q = lGetObject(gdil_ep, JG_queue))) {
               continue;
            }

            nslots = lGetUlong(gdil_ep, JG_slots);

            parse_ulong_val(&lim, NULL, TYPE_TIM, lGetString(q, QU_s_cpu), err_str, err_size);
            if (lim == DBL_MAX) {
               s_cpu = DBL_MAX;
            } else {
               s_cpu += lim * nslots; 
            }

            parse_ulong_val(&lim, NULL, TYPE_TIM, lGetString(q, QU_h_cpu), err_str, err_size);
            if (lim == DBL_MAX) {
               h_cpu = DBL_MAX;
            } else {
               h_cpu += lim * nslots; 
            }

            parse_ulong_val(&lim, NULL, TYPE_TIM, lGetString(q, QU_s_vmem), err_str, err_size);
            if (lim == DBL_MAX) {
               s_vmem = DBL_MAX;
            } else {
               s_vmem += lim * nslots; 
            }

            parse_ulong_val(&lim, NULL, TYPE_TIM, lGetString(q, QU_h_vmem), err_str, err_size);
            if (lim == DBL_MAX) {
               h_vmem = DBL_MAX;
            } else {
               h_vmem += lim * nslots; 
            }
         } /* foreach gdil_ep */

         if (h_cpu < cpu_val || h_vmem < vmem_val) {
            cpu_exceeded = (h_cpu < cpu_val);
            WARNING((SGE_EVENT, MSG_JOB_EXCEEDHLIM_USSFF, 
                     sge_u32c(jobid), sge_u32c(jataskid),
                     cpu_exceeded ? "h_cpu" : "h_vmem",
                     q?lGetString(q, QU_full_name) : "-",
                     cpu_exceeded ? cpu_val : vmem_val,
                     cpu_exceeded ? h_cpu : h_vmem));
            signal_job(jobid, jataskid, SGE_SIGKILL);
            lSetUlong(get_job_report(jobid, jataskid, NULL),
                      JR_failed, SSTATE_QMASTER_ENFORCED_LIMIT);
            continue;
         }

         if (s_cpu < cpu_val || s_vmem < vmem_val) {
            cpu_exceeded = (s_cpu < cpu_val);
            WARNING((SGE_EVENT, MSG_JOB_EXCEEDSLIM_USSFF,
                     sge_u32c(jobid), sge_u32c(jataskid),
                     cpu_exceeded ? "s_cpu" : "s_vmem",
                     q?lGetString(q, QU_full_name) : "-",
                     cpu_exceeded ? cpu_val : vmem_val,
                     cpu_exceeded ? s_cpu : s_vmem));
            signal_job(jobid, jataskid, SGE_SIGXCPU);
            continue;
         }
      } /* foreach jatep */
   } /* foreach jep */

   DRETURN_VOID;
}
#endif

static u_long32 
execd_get_wallclock_limit(const char *qualified_hostname, lList *gdil_list, int limit_nm, u_long32 now) 
{
   u_long32 ret = U_LONG32_MAX;
   const lListElem *gdil;
   const void *iterator;

   gdil = lGetElemHostFirst(gdil_list, JG_qhostname, qualified_hostname, &iterator);
   while (gdil != NULL) {
      const lListElem *queue;
      const char *limit;
      u_long32 clock_val;

      queue = lGetObject(gdil, JG_queue);
      if (queue != NULL) {
         limit = lGetString(queue, limit_nm);

         if (strcasecmp(limit, "infinity") == 0) {
            clock_val = U_LONG32_MAX;
         } else {   
            parse_ulong_val(NULL, &clock_val, TYPE_TIM, limit, NULL, 0);
         }   

         ret = MIN(ret, clock_val);
      }

      gdil = lGetElemHostNext(gdil_list, JG_qhostname, qualified_hostname, &iterator);
   }

   if (ret != U_LONG32_MAX) {
      ret += now;
   }

   return ret;
}

/******************************************************
 EXECD function

 called by dispatcher on a cyclic basis 
 cyclic means on every received message and on timeout


   as all dispatcher called function returns
   -  0 on success (currently the only value is 0)
   -  1 if we want to quit the dispacher loop
   -  2 if we want to reconnect to qmaster
   
 do cyclic jobs
 ******************************************************/
/* TODO: what are the intended intervals? */
#define SIGNAL_RESEND_INTERVAL 1
#define OLD_JOB_INTERVAL 60

int do_ck_to_do(sge_gdi_ctx_class_t *ctx, bool is_qmaster_down) {
   u_long32 now;
   u_long32 pdc_interval = U_LONG32_MAX;
   static u_long next_pdc = 0;
   static u_long next_signal = 0;
   static u_long next_old_job = 0;
   static u_long next_report = 0;
   static u_long last_report_send = 0;
   lListElem *jep, *jatep;
   int return_value = 0;
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "do_ck_to_do");

   /*
    *  get current time (now)
    *  ( don't update the now time inside this function )
    */
   now = sge_get_gmt();

#ifdef KERBEROS
   krb_renew_tgts(*(object_type_get_master_list(SGE_TYPE_JOB)));
#endif

   /* start jobs if present */
   if (jobs_to_start) {
      /* reset jobs_to_start before starting jobs. We may loose
       * a job start if we reset jobs_to_start after sge_start_jobs()
       */
      jobs_to_start = 0;
      sge_start_jobs(ctx);
   }

#ifdef COMPILE_DC
   /*
    * Collecting usage is only necessary if there are
    * jobs/tasks on this execution host and if it isn't
    * disabled by the execd_param PDC_INTERVAL set to NEVER. 
    */
   pdc_interval = mconf_get_pdc_interval();
   if (lGetNumberOfElem(*(object_type_get_master_list(SGE_TYPE_JOB))) > 0 &&
       pdc_interval != U_LONG32_MAX &&
       next_pdc <= now ) {
      next_pdc = now + pdc_interval;

      notify_ptf();

      errno = 0;
      if (sge_switch2start_user()) { /* fixme: exit? */
         CRITICAL((SGE_EVENT, MSG_SWITCH_USER_S, strerror(errno)));
      } else {
         ptf_update_job_usage();
         sge_switch2admin_user();
      }

      /* check for job limits */
      if (check_for_queue_limits()) {
         force_job_rlimit(qualified_hostname);
      }
   }
#endif

   if (sge_sig_handler_dead_children != 0) {
      /* reap max. 10 jobs which generated a SIGCLD */

      /* SIGCHILD signal is blocked from dispatcher(), so
       * we can be sure that sge_sig_handler_dead_children is untouched here
       */
      sge_sig_handler_dead_children = sge_reap_children_execd(10, is_qmaster_down);
   }

   if (next_signal <= now) {
      next_signal = now + SIGNAL_RESEND_INTERVAL;
      /* resend signals to shepherds */
      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         for_each(jatep, lGetList(jep, JB_ja_tasks)) {

            /* don't start wallclock before job acutally started */
            if (lGetUlong(jatep, JAT_status) == JWAITING4OSJID ||
                  lGetUlong(jatep, JAT_status) == JEXITING)
               continue;

            if (!lGetUlong(jep, JB_hard_wallclock_gmt)) {
               u_long32 task_wallclock_limit = lGetUlong(jatep, JAT_wallclock_limit);
               lList *gdil_list = lGetList(jatep,
                                           JAT_granted_destin_identifier_list);

               lSetUlong(jep, JB_soft_wallclock_gmt, 
                         execd_get_wallclock_limit(qualified_hostname, gdil_list, QU_s_rt, now));
               lSetUlong(jep, JB_hard_wallclock_gmt, 
                         execd_get_wallclock_limit(qualified_hostname, gdil_list, QU_h_rt, now));

               if (task_wallclock_limit != 0) {
                  lSetUlong(jep, JB_hard_wallclock_gmt, 
                            MIN(lGetUlong(jep, JB_hard_wallclock_gmt), duration_add_offset(now, task_wallclock_limit)));
               }
               if (!mconf_get_simulate_jobs()) {
                  job_write_spool_file(jep, lGetUlong(jatep, JAT_task_number), 
                                       NULL, SPOOL_WITHIN_EXECD);
               }
            }
            
            if (now >= lGetUlong(jep, JB_hard_wallclock_gmt) ) {
               if (!(lGetUlong(jatep, JAT_pending_signal_delivery_time)) ||
                   (now > lGetUlong(jatep, JAT_pending_signal_delivery_time))) {
                  lUlong job_id = lGetUlong(jep, JB_job_number);
                  lUlong ja_task_id = lGetUlong(jatep, JAT_task_number);
                  lListElem *jr = get_job_report(job_id, ja_task_id, NULL);

                  WARNING((SGE_EVENT, MSG_EXECD_EXCEEDHWALLCLOCK_UU,
                           sge_u32c(job_id), sge_u32c(ja_task_id)));
                  if (sge_execd_ja_task_is_tightly_integrated(jatep)) {
                     sge_kill_petasks(jep, jatep);
                  }
                  if (lGetUlong(jatep, JAT_pid) != 0) {
                     sge_kill(lGetUlong(jatep, JAT_pid), SGE_SIGKILL, job_id,
                              ja_task_id, NULL);
                  }
                  lSetUlong(jatep, JAT_pending_signal_delivery_time, now+90);
                  lSetUlong(jr, JR_failed, SSTATE_QMASTER_ENFORCED_LIMIT);
               }    
               continue;
            }

            if (now >= lGetUlong(jep, JB_soft_wallclock_gmt)) {
               if (!(lGetUlong(jatep, JAT_pending_signal_delivery_time)) ||
                   (now > lGetUlong(jatep, JAT_pending_signal_delivery_time))) {
                  WARNING((SGE_EVENT, MSG_EXECD_EXCEEDSWALLCLOCK_UU,
                       sge_u32c(lGetUlong(jep, JB_job_number)), sge_u32c(lGetUlong(jatep, JAT_task_number))));  
                  if (sge_execd_ja_task_is_tightly_integrated(jatep)) {
                     sge_kill_petasks(jep, jatep);
                  }
                  if (lGetUlong(jatep, JAT_pid) != 0) {
                     sge_kill(lGetUlong(jatep, JAT_pid), SGE_SIGUSR1, 
                              lGetUlong(jep, JB_job_number),
                              lGetUlong(jatep, JAT_task_number),
                              NULL);
                  }
                  lSetUlong(jatep, JAT_pending_signal_delivery_time, now+90);         
               }
               continue;
            }   
         }
      }
   }

   if (next_old_job <= now) {
      next_old_job = now + OLD_JOB_INTERVAL;
      clean_up_old_jobs(ctx, 0);
   }

   /* check for end of simulated jobs */
   if (mconf_get_simulate_jobs()) {
      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            if(lGetUlong(jatep, JAT_end_time) <= now) {
               lListElem *jr = NULL;
               u_long32 jobid, jataskid;
               u_long32 wallclock;

               jobid = lGetUlong(jep, JB_job_number);
               jataskid = lGetUlong(jatep, JAT_task_number);

               DPRINTF(("Simulated job "sge_u32"."sge_u32" is exiting\n", jobid, jataskid));

               if ((jr=get_job_report(jobid, jataskid, NULL)) == NULL) {
                  ERROR((SGE_EVENT, MSG_JOB_MISSINGJOBXYINJOBREPORTFOREXITINGJOBADDINGIT_UU, 
                         sge_u32c(jobid), sge_u32c(jataskid)));
                  jr = add_job_report(jobid, jataskid, NULL, jep);
               }

               lSetUlong(jr, JR_state, JEXITING);
               add_usage(jr, "submission_time", NULL, lGetUlong(jep, JB_submission_time));
               add_usage(jr, "start_time", NULL, lGetUlong(jatep, JAT_start_time));
               add_usage(jr, "end_time", NULL, lGetUlong(jatep, JAT_end_time));
               wallclock = lGetUlong(jatep, JAT_end_time) - lGetUlong(jatep, JAT_start_time);
               add_usage(jr, "ru_wallclock", NULL, wallclock);
               add_usage(jr, USAGE_ATTR_CPU_ACCT, NULL, wallclock * 0.5);
               add_usage(jr, "ru_utime", NULL, wallclock * 0.4);
               add_usage(jr, "ru_stime", NULL, wallclock * 0.1);
               add_usage(jr, "exit_status", NULL, 0);
            
               
               lSetUlong(jatep, JAT_status, JEXITING);
               flush_job_report(jr);
            }
         }
      }   
   }

   /* do timeout calculation */
   if (sge_get_flush_jr_flag() || next_report <= now || sge_get_flush_lr_flag()) {
      if (next_report <= now) {
         next_report = now + mconf_get_load_report_time();

         /* if pdc_interval is equals load_report time syncronize both calls to
            make the online usage acurate as possible */
         if (mconf_get_load_report_time() == mconf_get_pdc_interval()) {
            next_pdc = next_report;
         }
      }

      /* send only 1 load report per second, unequal because system time could be set back */
      if (last_report_send != now) {
         last_report_send = now;

         update_job_usage(qualified_hostname);

         /* send all reports */
         if (sge_send_all_reports(ctx, now, 0, execd_report_sources) == 1) {
            return_value = 2;
         }
      }
   }

   /* handle shutdown */
   if (shut_me_down != 0) {
      return_value = 1;
   }
   DRETURN(return_value);
}

/****** execd_ck_to_do/sge_execd_ja_task_is_tightly_integrated() ***************
*  NAME
*     sge_execd_ja_task_is_tightly_integrated() -- is it a tightly integr. parallel job?
*
*  SYNOPSIS
*     static bool 
*     sge_execd_ja_task_is_tightly_integrated(const lListElem *ja_task) 
*
*  FUNCTION
*     Checks if a certain job (ja_task) is running in a tightly integrated
*     parallel environment.
*
*  INPUTS
*     const lListElem *ja_task - ja_task (in execd context)
*
*  RESULT
*     static bool - true, if it is a tightly integrated job, else false
*
*******************************************************************************/
static bool 
sge_execd_ja_task_is_tightly_integrated(const lListElem *ja_task)
{
   bool ret = false;

   if (ja_task != NULL) {
      const lListElem *pe = lGetObject(ja_task, JAT_pe_object);
      if (pe != NULL && lGetBool(pe, PE_control_slaves)) {
         ret = true;
      }
   }

   return ret;
}

/****** execd_ck_to_do/sge_kill_petasks() **************************************
*  NAME
*     sge_kill_petasks() -- kill all pe tasks of a tightly integr. parallel job
*
*  SYNOPSIS
*     static bool 
*     sge_kill_petasks(const lListElem *job, const lListElem *ja_task) 
*
*  FUNCTION
*     Kills all tasks of a tightly integrated parallel job/array task.
*
*  INPUTS
*     const lListElem *job     - the job
*     const lListElem *ja_task - the array task
*
*  RESULT
*     static bool - true, if any task was found and could be signalled, 
*                   else false
*
*******************************************************************************/
static bool
sge_kill_petasks(const lListElem *job, const lListElem *ja_task)
{
   bool ret = false;

   if (job != NULL && ja_task != NULL) {
      lListElem *pe_task;

      for_each(pe_task, lGetList(ja_task, JAT_task_list)) {
         if (sge_kill(lGetUlong(pe_task, PET_pid), SGE_SIGKILL,
                      lGetUlong(job, JB_job_number),
                      lGetUlong(ja_task, JAT_task_number),
                      lGetString(pe_task, PET_id)) == 0) {
            ret = true;
         }
      }
   }

   return ret;
}


/*****************************************************************************/
static int sge_start_jobs(sge_gdi_ctx_class_t *ctx)
{
   lListElem *jep, *jatep, *petep;
   int state_changed;
   int jobs_started = 0;

   DENTER(TOP_LAYER, "sge_start_jobs");

   if (lGetNumberOfElem(*(object_type_get_master_list(SGE_TYPE_JOB))) == 0) {
      DPRINTF(("No jobs to start\n"));
      DRETURN(0);
   }

   for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
      for_each(jatep, lGetList(jep, JB_ja_tasks)) {
         state_changed = exec_job_or_task(ctx, jep, jatep, NULL);

         /* visit all tasks */
         for_each(petep, lGetList(jatep, JAT_task_list)) {
            state_changed |= exec_job_or_task(ctx, jep, jatep, petep);
         }

         /* now save this job so we are up to date on restart */
         if (state_changed) {
            if (!mconf_get_simulate_jobs()) {
               job_write_spool_file(jep, lGetUlong(jatep, JAT_task_number), 
                                    NULL, SPOOL_WITHIN_EXECD);
            }
            jobs_started++;
         }
      }
   }
   DPRINTF(("execd_ck_to_do: started "sge_U32CFormat" jobs\n", sge_u32c(jobs_started)));

   DRETURN(0);
}


static int exec_job_or_task(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jatep, lListElem *petep)
{
   char err_str[256];
   int pid;
   u_long32 now;
   u_long32 job_id, ja_task_id;
   const char *pe_task_id = NULL;
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "exec_job_or_task");

   /* retrieve ids - we need them later on */
   job_id = lGetUlong(jep, JB_job_number);
   ja_task_id = lGetUlong(jatep, JAT_task_number);
   if (petep != NULL) {
      pe_task_id = lGetString(petep, PET_id);
   }

   /* we only handle idle jobs or tasks */
   /* JG: TODO: make a function is_task_idle(jep, jatep, petep) */
   {
      u_long32 status;

      if (petep != NULL) {
         status = lGetUlong(petep, PET_status);
      } else {
         status = lGetUlong(jatep, JAT_status);
      }

      if (status != JIDLE) {
         DRETURN(0);
      }
   }   

   now = sge_get_gmt();

   /* JG: TODO: make a function simulate_start_job_or_task() */
   if (mconf_get_simulate_jobs()) {
      lList *job_args;
      u_long32 duration = 60;

      DPRINTF(("Simulating job "sge_u32"."sge_u32"\n", 
               job_id, ja_task_id));
      lSetUlong(jatep, JAT_start_time, now);
      lSetUlong(jatep, JAT_status, JRUNNING);

      /* set time when job shall be reported as finished */
      job_args = lGetList(jep, JB_job_args);
      if (lGetNumberOfElem(job_args) == 1) {
         const char *arg = NULL;
         char *endptr = NULL;
         u_long32 duration_in;

         arg = lGetString(lFirst(job_args), ST_name);
         if (arg != NULL) {
            DPRINTF(("Trying to use first argument ("SFQ") as duration for simulated job\n", arg));
            
            duration_in = strtol(arg, &endptr, 0);
            if (arg != endptr) {
               duration = duration_in;
            }
         }   
      }

      lSetUlong(jatep, JAT_end_time, now + duration);
      DRETURN(1);
   }

   if (petep != NULL) {
      lSetUlong(petep, PET_start_time, now);
#ifdef COMPILE_DC
      lSetUlong(petep, PET_status, JWAITING4OSJID);
      waiting4osjid = 1;
#else
      lSetUlong(petep, PET_status, JRUNNING);
#endif
   } else {
      lSetUlong(jatep, JAT_start_time, now);
#ifdef COMPILE_DC
      lSetUlong(jatep, JAT_status, JWAITING4OSJID);
      waiting4osjid = 1;
#else
      lSetUlong(jatep, JAT_status, JRUNNING);
#endif
   }

   if (getenv("FAILURE_BEFORE_EXEC")) {
      pid = -1; 
      strcpy(err_str, "FAILURE_BEFORE_EXEC");
   } else {
      pid = sge_exec_job(ctx, jep, jatep, petep, err_str, 256);
   }   

   if (pid < 0) {
      switch (pid) {
         case -1:
            execd_job_start_failure(jep, jatep, petep, err_str, GFSTATE_NO_HALT);
            break;
         case -2:
            execd_job_start_failure(jep, jatep, petep, err_str, GFSTATE_QUEUE);
            break;
         case -3:
            execd_job_start_failure(jep, jatep, petep, err_str, GFSTATE_JOB);
            break;
         default:
            execd_job_start_failure(jep, jatep, petep, err_str, GFSTATE_HOST);
            break;
      }
      DRETURN(0);
   }
   DTIMEPRINTF(("TIME IN EXECD FOR STARTING THE JOB: " sge_u32 "\n",
                sge_get_gmt()-now));
   
   if (petep != NULL) {
      lSetUlong(petep, PET_pid, pid);
   } else {
      lSetUlong(jatep, JAT_pid, pid);
   }

   DPRINTF(("***EXECING "sge_u32"."sge_u32" on %s (tid = %s) (pid = %d)\n",
            job_id, ja_task_id, 
            qualified_hostname, pe_task_id != NULL ? pe_task_id : "null", pid));

   /* when a ja_task or pe_task has been started, flush the job report */
   {
      lListElem *jr = get_job_report(job_id, ja_task_id, pe_task_id);
      if (jr == NULL) {
         jr = add_job_report(job_id, ja_task_id, pe_task_id, jep);
         flush_job_report(jr);
      }
   }

   DRETURN(1);
}

#ifdef COMPILE_DC
int register_at_ptf(
lListElem *job,
lListElem *ja_task,
lListElem *pe_task 
) {
   u_long32 job_id;   
   u_long32 ja_task_id;   
   const char *pe_task_id = NULL;

   int success;
   FILE *fp;
   SGE_STRUCT_STAT sb;

   char id_buffer[MAX_STRING_SIZE]; /* static dstring for job id string */
   dstring id_dstring;

#if defined(SOLARIS) || defined(ALPHA) || defined(LINUX) || defined(HP1164) || defined(AIX) || defined(FREEBSD) || defined(DARWIN)
   gid_t addgrpid;
   dstring addgrpid_path = DSTRING_INIT;
#else   
   dstring osjobid_path = DSTRING_INIT;
   osjobid_t osjobid;   
#endif   
   DENTER(TOP_LAYER, "register_at_ptf");

   sge_dstring_init(&id_dstring, id_buffer, MAX_STRING_SIZE);

   job_id = lGetUlong(job, JB_job_number);
   ja_task_id = lGetUlong(ja_task, JAT_task_number);
   if (pe_task != NULL) { 
      pe_task_id = lGetString(pe_task, PET_id);
   }

#if defined(SOLARIS) || defined(ALPHA) || defined(LINUX) || defined(HP1164) || defined(AIX) || defined(FREEBSD) || defined(DARWIN)
   /**
    ** read additional group id and use it as osjobid 
    **/

   /**
    ** we use the process group ID as osjobid on HP-UX and AIX
    **/
   
   /* open addgrpid file */
   sge_get_active_job_file_path(&addgrpid_path,
                                job_id, ja_task_id, pe_task_id, ADDGRPID);
   DPRINTF(("Registering job %s with PTF\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));

   if (SGE_STAT(sge_dstring_get_string(&addgrpid_path), &sb) && errno == ENOENT) {
      DPRINTF(("still waiting for addgrpid of job %s\n", 
         job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      sge_dstring_free(&addgrpid_path);
      DEXIT;
      return(1);
   }  

   if (!(fp = fopen(sge_dstring_get_string(&addgrpid_path), "r"))) {
      ERROR((SGE_EVENT, MSG_EXECD_NOADDGIDOPEN_SSS, sge_dstring_get_string(&addgrpid_path), 
             job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), strerror(errno)));
      sge_dstring_free(&addgrpid_path);
      DEXIT;
      return(-1);
   }
  
   sge_dstring_free(&addgrpid_path);       

   /* read addgrpid */
   success = (fscanf(fp, gid_t_fmt, &addgrpid)==1);
   FCLOSE(fp);
   if (!success) {
      /* can happen that shepherd has opend the file but not written */
      DEXIT;
      return (1);
   }
   {
      int ptf_error;

      DPRINTF(("Register job with AddGrpId at "gid_t_fmt" PTF\n", addgrpid));
      if ((ptf_error = ptf_job_started(addgrpid, pe_task_id, job, ja_task_id))) {
         ERROR((SGE_EVENT, MSG_JOB_NOREGISTERPTF_SS, 
                job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
                ptf_errstr(ptf_error)));
         DEXIT;
         return (1);
      }
   }

   /* store addgrpid in job report to be sent to qmaster later on */
   {
      char addgrpid_str[64];
      lListElem *jr;

      sprintf(addgrpid_str, gid_t_fmt, addgrpid);
      if ((jr=get_job_report(job_id, ja_task_id, pe_task_id)))
         lSetString(jr, JR_osjobid, addgrpid_str); 
      DPRINTF(("job %s: addgrpid = %s\n", 
               job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), addgrpid_str));
   }
#else
   /* read osjobid if possible */
   sge_get_active_job_file_path(&osjobid_path,
                                job_id, ja_task_id, pe_task_id, OSJOBID);
   
   DPRINTF(("Registering job %s with PTF\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));

   if (SGE_STAT(sge_dstring_get_string(&osjobid_path), &sb) && errno == ENOENT) {
      DPRINTF(("still waiting for osjobid of job %s\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      sge_dstring_free(&osjobid_path);      
      DEXIT;
      return 1;
   } 

   if (!(fp=fopen(sge_dstring_get_string(&osjobid_path), "r"))) {
      ERROR((SGE_EVENT, MSG_EXECD_NOOSJOBIDOPEN_SSS, sge_dstring_get_string(&osjobid_path), 
             job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
             strerror(errno)));
      sge_dstring_free(&osjobid_path);      
      DEXIT;
      return -1;
   }

   sge_dstring_free(&osjobid_path);      

   success = (fscanf(fp, OSJOBID_FMT, &osjobid)==1);
   FCLOSE(fp);
   if (!success) {
      /* can happen that shepherd has opend the file but not written */
      DEXIT;
      return 1;
   }

   {
      int ptf_error;
      if ((ptf_error = ptf_job_started(osjobid, pe_task_id, job, ja_task_id))) {
         ERROR((SGE_EVENT, MSG_JOB_NOREGISTERPTF_SS,  
                job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
                ptf_errstr(ptf_error)));
         DEXIT;
         return -1;
      }
   }

   /* store osjobid in job report to be sent to qmaster later on */
   {
      char osjobid_str[64];
      lListElem *jr;

      sprintf(osjobid_str, OSJOBID_FMT, osjobid);
      if ((jr=get_job_report(job_id, ja_task_id, pe_task_id)))
         lSetString(jr, JR_osjobid, osjobid_str); 
      DPRINTF(("job %s: osjobid = %s\n", 
               job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring),
               osjobid_str));
   }
#endif

   DEXIT;
   return 0;
FCLOSE_ERROR:
   DEXIT;
   return 1;
}
#endif
