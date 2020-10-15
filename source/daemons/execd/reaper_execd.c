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
 *  Portions of this software are Copyright (c) 2011-2012 Univa Corporation
 *  Copyright (C) 2012 Mark Dixon, University of Leeds
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <float.h>

#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <grp.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_afsutil.h"
#include "uti/sge_parse_num_par.h"
#include "uti/setup_path.h"
#include "uti/sge_spool.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_binding_hlp.h"
#include "uti/sge_os.h"
#include "uti/sge_log.h"
#include "uti/config_file.h"
#include "uti/sge_signal.h"
#include "uti/sge_prog.h"
#include "uti/sge_time.h"
#include "uti/sge_stdio.h"
#include "uti2/sge_cgroup.h"

#include "sgeobj/sge_config.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_binding.h"

#include "gdi/sge_qexec.h"
#include "gdi/sge_security.h" 

#include "spool/classic/read_write_job.h"

#include "sge.h"
#include "symbols.h"
#include "admin_mail.h"
#include "mail.h"
#include "exec_job.h"
#include "dispatcher.h"
#include "tmpdir.h"
#include "sge_job_qmaster.h"
#include "execution_states.h"
#include "sge_load_sensor.h"
#include "execd.h"
#include "reaper_execd.h"
#include "execd_signal_queue.h"
#include "job_report_execd.h"
#include "get_path.h"
#include "load_avg.h"
#include "msg_common.h"
#include "msg_daemons_common.h"
#include "msg_execd.h"
#include "binding_support.h"

#ifdef COMPILE_DC
#  include "ptf.h"
static void unregister_from_ptf(u_long32 jobid, u_long32 jataskid, const char *pe_task_id, lListElem *jr);
#endif

static int clean_up_job(lListElem *jr, int failed, int signal, int is_array, const char *job_owner);
static void convert_attribute(lList **cflpp, lListElem *jr, char *name, u_long32 udefau);
static int extract_ulong_attribute(lList **cflpp, char *name, u_long32 *valuep); 

static lListElem *execd_job_failure(lListElem *jep, lListElem *jatep, lListElem *petep, const char *error_string, int general, int failed);
static int read_dusage(lListElem *jr, const char *jobdir, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, int failed);
static void build_derived_final_usage(lListElem *jr, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id);

static void examine_job_task_from_file(sge_gdi_ctx_class_t *ctx, int startup, char *dir, lListElem *jep, lListElem *jatep, lListElem *petep, pid_t *pids, int npids);

static void update_used_cores(const char* path_to_config, lListElem** jr);

static void clean_up_binding(char* binding);

static void reaper_sendmail(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jr);
/* TODO: global c file with #define JAPI_SINGLE_SESSION_KEY "JAPI_SSK" */
static const char *JAPI_SINGLE_SESSION_KEY = "JAPI_SSK";

/*****************************************************************************
 This code is used only by execd.
 Execd has gotten a SIGCLD or execd starts up.
 Look whether there are finished jobs. Report them to the qmaster.
 Our input is what shepherd wrote to the directory of the job:
 "<execd_spool>/active_jobs/<jobnumber>/".

 "error"-file  errors detected by shepherd
 "trace"-file  information only interesting for sge programmers
 "usage"-file  usage of job (man getrusage)
 "pid"-file    pid of shepherd; can be used to look into the process table

 If everything is done we can remove the job directory.
 We also enable or disable delayed reporting for a job here. (Ref CR6888406 for why).
 It is enabled for jobs which:
  - have been started via qsub -sync/DRMAA and
  - submission time is before the qmaster restarted or if qmaster is currently down
  
 ****************************************************************************/
int sge_reap_children_execd(int max_count, bool is_qmaster_down)
{
   int pid = 999;
   int exit_status, child_signal, core_dumped, failed;
   u_long32 jobid, jataskid;
   lListElem *jep, *petep = NULL, *jatep = NULL;

   int status;
   int reap_count = 0;

   DENTER(TOP_LAYER, "sge_reap_children_execd");
   DPRINTF(("========================REAPER======================\n"));

   pid = 999;
   if (max_count < 1) {
      max_count = 1;
   }

   while (pid > 0) {
      if (reap_count >= max_count) {
         DPRINTF(("max. reap count is reached - returning. reaped "sge_U32CFormat" childs.\n", sge_u32c(reap_count)));
         return 1;
      }

      exit_status = child_signal = core_dumped = failed = 0;

      pid = waitpid(-1, &status, WNOHANG);

      if (pid == 0) {
         DPRINTF(("pid==0 - no stopped or exited children\n"));
         break;
      }

      if (pid == -1) {
         DPRINTF(("pid==-1 - no children not previously waited for\n"));
         break;
      }

      if (WIFSTOPPED(status)) {
         DPRINTF(("PID %d WIFSTOPPED\n", pid));
         continue;
      }

#ifdef WIFCONTINUED
      if (WIFCONTINUED(status)) {
         DPRINTF(("PID %d WIFCONTINUED\n", pid));
         continue;
      }
#endif

      if (WIFSIGNALED(status)) {
         child_signal = WTERMSIG(status);
#ifdef WCOREDUMP
         core_dumped = WCOREDUMP(status);
#else
         core_dumped = status & 80;
#endif
         failed = ESSTATE_DIED_THRU_SIGNAL;
      } else if (WIFEXITED(status)) {
         exit_status = WEXITSTATUS(status);
         if (exit_status)
            failed = ESSTATE_SHEPHERD_EXIT;
      } else {
         /* not signaled and not exited - so what else happend with this guy? */
         WARNING((SGE_EVENT, MSG_WAITPIDNOSIGNOEXIT_UI, sge_u32c(pid), status));
         continue;
      }

      /* increase reaped job counter */ 
      reap_count++;

      /* search whether it was a job or one of its tasks */
      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         int Break = 0;
   
         petep = NULL;
         for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            if (lGetUlong(jatep, JAT_pid) == pid) {
               petep = NULL;
               Break = 1;
               break;
            }
            
            for_each(petep, lGetList(jatep, JAT_task_list)) {
               if (lGetUlong(petep, PET_pid) == pid) {
                  break;
               }
            }
            
            if (petep) {
               Break = 1;
               break;
            }
         }
         if (Break) {
            break;
         }
      }

      /* was it a job ? */
      if (jep && jatep) {
         lListElem *jr;

         jobid = lGetUlong(jep, JB_job_number);
         jataskid = lGetUlong(jatep, JAT_task_number);
   
         if (child_signal)
            ERROR((SGE_EVENT, MSG_SHEPHERD_VSHEPHERDOFJOBWXDIEDTHROUGHSIGNALYZ_SUUSI,
                     (petep ? MSG_SLAVE : "" ),
                     sge_u32c(jobid), 
                     sge_u32c(jataskid),
                     core_dumped ? MSG_COREDUMPED: "",
                     child_signal));

         if (exit_status) {
            ERROR((SGE_EVENT, MSG_SHEPHERD_WSHEPHERDOFJOBXYEXITEDWITHSTATUSZ_SUUI, 
                  (petep ? MSG_SLAVE : "" ), 
                  sge_u32c(jobid), 
                  sge_u32c(jataskid), 
                  exit_status));
         }

         /* 
          *  seek job report for this job it must be contained in job report
          *  if not it should be a job kept with SGE_KEEP_ACTIVE (without
          *  keeping job object itself)
          */
         DPRINTF(("Job: "sge_u32", JA-Task: "sge_u32", PE-Task: %s\n", jobid, jataskid, 
            petep != NULL ? lGetString(petep, PET_id) : ""));
         if (!(jr=get_job_report(jobid, jataskid, petep != NULL ? lGetString(petep, PET_id) : NULL))) {
            ERROR((SGE_EVENT, MSG_JOB_MISSINGJOBXYINJOBREPORTFOREXITINGJOBADDINGIT_UU, 
                   sge_u32c(jobid), sge_u32c(jataskid)));
            if (petep != NULL) {
               jr = add_job_report(jobid, jataskid, lGetString(petep, PET_id), jep);
            } else {
               jr = add_job_report(jobid, jataskid, NULL, jep);
            }
         }

         /* when restarting execd it happens that cleanup_old_jobs()
            has already cleaned up this job */
         if (lGetUlong(jr, JR_state) == JEXITING) {
            DPRINTF(("State of job "sge_u32" already changed to JEXITING\n", jobid));
            continue;
         }

         lSetUlong(jr, JR_state, JEXITING);

         if(petep != NULL) {
            lSetUlong(petep, PET_status, JEXITING);
         } else {
            lSetUlong(jatep, JAT_status, JEXITING);
         }

         clean_up_job(jr, failed, exit_status, job_is_array(jep),
                      lGetString(jep, JB_owner));

         /* set JR_delay_report true if the job is a JAPI/DRMAA job ... 
          * -sync/DRMAA jobs have unique session key, qsub jobs have JAPI_SSK
          *  and other clients have null keys as they are not set anywhere
          * We also check if the sync client was started before the qmaster
          * recovered or qmaster is down now, which means the qsub -sync/DRMAA client needs to yet
          * collect the exit status!
          */
         if (lGetString(jep, JB_session) && strncmp(lGetString(jep, JB_session), JAPI_SINGLE_SESSION_KEY, 8) != 0 && 
               (is_qmaster_down || lGetUlong(jep, JB_submission_time) < sge_get_qmrestart_time())) {
            lSetBool(jr, JR_delay_report, true);
            INFO((SGE_EVENT, MSG_EXECD_ENABLEDELEAYDREPORTINGFORJOB_U, sge_u32c(jobid)));
         } else {
            lSetBool(jr, JR_delay_report, false);
         }

         flush_job_report(jr);

      } else  if (sge_ls_stop_if_pid(pid)) {
         if (child_signal) {
            ERROR((SGE_EVENT, MSG_STATUS_LOADSENSORDIEDWITHSIGNALXY_SI,
                  core_dumped ? MSG_COREDUMPED: "",
                  child_signal));
         } else {
            WARNING((SGE_EVENT, MSG_STATUS_LOADSENSOREXITEDWITHEXITSTATUS_I,
                    exit_status));
         }
      } else {
         if (child_signal)
            ERROR((SGE_EVENT, MSG_STATUS_MAILERDIEDTHROUGHSIGNALXY_SI,
                  core_dumped ? MSG_COREDUMPED: "",
                  child_signal));
         else if (exit_status)
            ERROR((SGE_EVENT, MSG_STATUS_MAILEREXITEDWITHEXITSTATUS_I,
                  exit_status));
      }
   }
   DPRINTF(("reaped "sge_U32CFormat" childs - no child remaining\n", sge_u32c(reap_count)));

   DEXIT;
   return 0;
}

#ifdef COMPILE_DC
/* 

DESC

   unregisters job from ptf and fills usage values
   into appropriate job report element

*/
static void unregister_from_ptf(u_long32 job_id, u_long32 ja_task_id,
                               const char *pe_task_id, lListElem *jr) 
{
   lList* usage = NULL;
   int ptf_error;

   DENTER(TOP_LAYER, "unregister_from_ptf");

   ptf_error = ptf_job_complete(job_id, ja_task_id, pe_task_id, &usage);
   if (ptf_error) {
      lListElem *ja_task = NULL;
      lListElem *job = NULL; 

      /* if the job was a 'short-runner' omit the warning */
      if (execd_get_job_ja_task(job_id, ja_task_id, &job, &ja_task)) {
         /* check if the job was a short-runner */  
         u_long32 time_since_started = sge_get_gmt() - lGetUlong(ja_task, JAT_start_time);
         if (time_since_started <= 2) {
            /* the job was started <= 2 seconds before and ended already 
               hence no warning has to be printed because of bug CR 6326191 */ 
            DRETURN_VOID;
         }
      }

      WARNING((SGE_EVENT, MSG_JOB_REAPINGJOBXPTFCOMPLAINSY_USS,
               sge_u32c(job_id), sge_u32c(ja_task_id), ptf_errstr(ptf_error)));
   } else {
      if (usage) {
         lXchgList(jr, JR_usage, &usage);
         lFreeList(&usage);
      }
   }

   DRETURN_VOID;
}
#endif

/************************************************************************
   This is the job clean up and report function. We design this function
   to be as independent from the execd as possible. Maybe we want to 
   make an extra executable later. We have another function cleaning up jobs
   (execd_job_start_failure). This function is called if the job starting 
   failed. At this time there is no config-file present.

   jobid = id of job to reap
   failed = indicates a failure of job execution, see shepherd_states.h
 ************************************************************************/
static int clean_up_job(lListElem *jr, int failed, int shepherd_exit_status, 
                        int is_array, const char* job_owner)
{
   char* binding;
   dstring jobdir = DSTRING_INIT;
   dstring fname  = DSTRING_INIT;

   char id_buffer[MAX_STRING_SIZE];
   dstring id_dstring;

   SGE_STRUCT_STAT statbuf;
   char error[10000];
   FILE *fp;
   u_long32 job_id, job_pid = 0, ckpt_arena, general_failure = 0, ja_task_id;
   const char *pe_task_id = NULL;
   lListElem *du;

   DENTER(TOP_LAYER, "clean_up_job");

   sge_dstring_init(&id_dstring, id_buffer, MAX_STRING_SIZE);

   if (!jr) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_JOB_CLEANUPJOBCALLEDWITHINVALIDPARAMETERS));
      DEXIT;
      return -1;
   }

   job_id = lGetUlong(jr, JR_job_number);
   ja_task_id = lGetUlong(jr, JR_ja_task_number);
   pe_task_id = lGetString(jr, JR_pe_task_id_str);

   DPRINTF(("cleanup for job %s\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));

#ifdef COMPILE_DC
   unregister_from_ptf(job_id, ja_task_id, pe_task_id, jr);
#else
   lDelSubStr(jr, UA_name, USAGE_ATTR_CPU, JR_usage);
   lDelSubStr(jr, UA_name, USAGE_ATTR_MEM, JR_usage);
   lDelSubStr(jr, UA_name, USAGE_ATTR_IO, JR_usage);
   lDelSubStr(jr, UA_name, USAGE_ATTR_IOW, JR_usage);
   lDelSubStr(jr, UA_name, USAGE_ATTR_VMEM, JR_usage);
   lDelSubStr(jr, UA_name, USAGE_ATTR_MAXVMEM, JR_usage);
#endif

   /* set directory for job */
   sge_get_active_job_file_path(&jobdir, job_id, ja_task_id, pe_task_id, NULL);

   if (SGE_STAT(sge_dstring_get_string(&jobdir), &statbuf)) {
      /* This never should happen, cause if we cant create this directory on
         startup we report the job finish immediately */
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDDIRXFORREAPINGJOBYZ_SS, 
             sge_dstring_get_string(&jobdir), 
             job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      sge_dstring_free(&jobdir);       
      return -1;        /* nothing can be done without information */
   }

   /* read config written by exec_job */
   sge_get_active_job_file_path(&fname, job_id, ja_task_id, pe_task_id, 
                                "config");
   if (read_config(sge_dstring_get_string(&fname))) {
      /* This should happen very rarely. exec_job() should avoid this 
         condition as far as possible. One possibility for this case is, 
         that the execd dies just after making the jobs active directory. 
         The pain with this case is, that we have not much information
         to report this job to qmaster. */
      ERROR((SGE_EVENT, MSG_JOB_CANTREADCONFIGFILEFORJOBXY_S, 
         job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      lSetUlong(jr, JR_failed, ESSTATE_NO_CONFIG);
      lSetString(jr, JR_err_str, (char *) MSG_SHEPHERD_EXECDWENTDOWNDURINGJOBSTART);

      sge_dstring_free(&fname);
      sge_dstring_free(&jobdir);
      DEXIT;
      return 0;
   }

   *error = '\0';

   DTRACE;

   /* job to core binding: on Solaris the processor set have to be deleted 
      and the cores have to be freed */ 
      
   binding = get_conf_val("binding");
   
   clean_up_binding(binding);

   /*
    * look for exit status of shepherd This is the last file the shepherd
    * creates. So if we can find this shepherd terminated normal.
    */
   sge_get_active_job_file_path(&fname,
                                job_id, ja_task_id, pe_task_id, "exit_status");
   if (!(fp = fopen(sge_dstring_get_string(&fname), "r"))) {
      /* 
       * we trust the exit status of the shepherd if it exited regularly
       * otherwise we assume it died before starting the job (if died through signal or
       * job_dir was found during execd startup)
       */
      if (failed == ESSTATE_SHEPHERD_EXIT) {
         failed = shepherd_exit_status;
      } else {
         failed = SSTATE_BEFORE_PROLOG;
      }

      snprintf(error, sizeof(error), MSG_STATUS_ABNORMALTERMINATIONOFSHEPHERDFORJOBXY_S,
              job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring));
      ERROR((SGE_EVENT, SFNMAX, error));
 
      /* 
       * failed = ESSTATE_SHEPHERD_EXIT or exit status of shepherd if we are
       * parent in case we can't open the exit_status file
       */
   } else {
      int fscanf_count, shepherd_exit_status_file;
         
      fscanf_count = fscanf(fp, "%d", &shepherd_exit_status_file);
      FCLOSE_IGNORE_ERROR(fp);
      if (fscanf_count != 1) {
         snprintf(error, sizeof(error), MSG_STATUS_ABNORMALTERMINATIONFOSHEPHERDFORJOBXYEXITSTATEFILEISEMPTY_S,
                 job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring));
         ERROR((SGE_EVENT, SFNMAX, error));
         /* 
          * If shepherd died through signal assume job was started, else
          * trust exit status
          */
         if (failed == ESSTATE_SHEPHERD_EXIT)
            failed = shepherd_exit_status;
         else
            failed = ESSTATE_NO_EXITSTATUS;
      } else if (failed != ESSTATE_NO_PID)  /* only set during execd startup */
         failed = shepherd_exit_status_file;
         
      /* 
       * failed is content of exit_status file or ESSTATE_NO_PID or real
       * exit status if file can't be read
       */   
   }


   /*
    * failed is:
    *    exit status of shepherd (one of SSTATE_* values)
    *    ESSTATE_DIED_THRU_SIGNAL
    *    ESSTATE_NO_PID 
    */

   if (failed) {
      if (failed == ESSTATE_DIED_THRU_SIGNAL)
         sprintf(error, SFNMAX, MSG_SHEPHERD_DIEDTHROUGHSIGNAL);
      else if (failed == ESSTATE_NO_PID)
         sprintf(error, SFNMAX, MSG_SHEPHERD_NOPIDFILE);
      else
         sprintf(error, MSG_SHEPHERD_EXITEDWISSTATUS_IS, failed, 
                 get_sstate_description(failed));
   }

   /* look for error file this overrules errors found yet */
   sge_get_active_job_file_path(&fname,
                                job_id, ja_task_id, pe_task_id, "error");
   if ((fp = fopen(sge_dstring_get_string(&fname), "r"))) {
      int n;
      char *new_line;
      if ((n = fread(error, 1, sizeof(error), fp))) {
         /* Non empty error file. The shepherd encounterd a problem. */ 
         if (!failed)
            failed = ESSTATE_UNEXP_ERRORFILE;
         error[n] = '\0';
         /* ensure only first line of error file is in 'error' */
         if ((new_line=strchr(error, '\n')))
            *new_line = '\0';
         DPRINTF(("ERRORFILE: %256s\n", error));
      }
      else if (feof(fp)) {
         DPRINTF(("empty error file\n"));
      } else {
         ERROR((SGE_EVENT, MSG_JOB_CANTREADERRORFILEFORJOBXY_S, 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      }      
      FCLOSE_IGNORE_ERROR(fp);
   }
   else {
      ERROR((SGE_EVENT, MSG_FILE_NOOPEN_SS, sge_dstring_get_string(&fname), strerror(errno)));
      /* There is no error file. */
   }

   DTRACE;

   /*
   ** now that shepherd stops on errors there is no usage file
   ** if the job never ran
   ** but we need a dusage struct because information that has nothing 
   ** to do with the job also goes in there, this should be changed
   ** read_dusage gets the failed parameter to decide what should be there
   */
   if (read_dusage(jr, sge_dstring_get_string(&jobdir), job_id, ja_task_id, pe_task_id, failed)) {
      if (error[0] == '\0') {
         snprintf(error, sizeof(error), MSG_JOB_CANTREADUSAGEFILEFORJOBXY_S,
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring));
      }
      
      ERROR((SGE_EVENT, SFNMAX, error));
      
      if (!failed) {
         failed = SSTATE_FAILURE_AFTER_JOB;
      }
      
      DTRACE;
   }

   

   /* map system signals into sge signals to make signo's exchangable */
   du=lGetSubStr(jr, UA_name, "signal", JR_usage);
   if (du) {
      int signo = (int)lGetDouble(du, UA_value);

      if (signo) {
         u_long32 sge_signo;

         /* Job died through a signal */
         snprintf(error, sizeof(error), MSG_JOB_WXDIEDTHROUGHSIGNALYZ_SSI,
                 job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
                 sge_sys_sig2str(signo), signo);

         DPRINTF(("%s\n", error));
         failed = SSTATE_FAILURE_AFTER_JOB;

         if ((sge_signo=sge_map_signal(signo)) != -1)
            lSetDouble(du, UA_value, (double)sge_signo);
      }

      if ((u_long32)lGetDouble(du, UA_value) == 0xffffffff) {
         if (!failed) {
            failed = SSTATE_FAILURE_AFTER_JOB;
            if (!*error)
               snprintf(error, sizeof(error), SFNMAX, MSG_JOB_CANTREADUSEDRESOURCESFORJOB);
         }
      }
   }
      
   DTRACE;

   /* Be careful: the checkpointing checking is done at the end. It will
    * often override other failure states.
    * If the job finishes, the shepherd must remove the "checkpointed" file
    */  

   sge_get_active_job_file_path(&fname,
                                job_id, ja_task_id, pe_task_id, "checkpointed");
   ckpt_arena = 1;   /* 1 job will be restarted in case of failure *
                      * 2 job will be restarted from ckpt arena    */
   if (!SGE_STAT(sge_dstring_get_string(&fname), &statbuf)) {
      int dummy;

      failed = SSTATE_MIGRATE;
      if ((fp = fopen(sge_dstring_get_string(&fname), "r"))) {
         DPRINTF(("found checkpointed file\n"));
         if (fscanf(fp, "%d", &dummy)==1) {
            DPRINTF(("need restart from ckpt arena\n"));
            ckpt_arena = 2;
         }
         FCLOSE_IGNORE_ERROR(fp);
      }   

      sge_get_active_job_file_path(&fname, job_id, ja_task_id, pe_task_id, 
                                "job_pid");
      if (!SGE_STAT(sge_dstring_get_string(&fname), &statbuf)) {
         if ((fp = fopen(sge_dstring_get_string(&fname), "r"))) {
            if (!fscanf(fp, sge_u32 , &job_pid))
               job_pid = 0;
            FCLOSE_IGNORE_ERROR(fp);
         }
         else {
            job_pid = 0;
            ERROR((SGE_EVENT, MSG_JOB_CANTOPENJOBPIDFILEFORJOBXY_S, 
                   job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
         }
      }
   }

   lSetUlong(jr, JR_job_pid, job_pid);
   
   /* Only used for ckpt jobs: 1 checkpointed, 2 checkpoint in the arena */
   lSetUlong(jr, JR_ckpt_arena, ckpt_arena);

   /* Currently the shepherd doesn't create this file */
   sge_get_active_job_file_path(&fname, job_id, ja_task_id, pe_task_id, 
                                "noresources");
   if (!SGE_STAT(sge_dstring_get_string(&fname), &statbuf))
      failed = SSTATE_AGAIN;
   
   /* failed */
   if (SSTATE_QMASTER_ENFORCED_LIMIT != lGetUlong(jr, JR_failed))
      lSetUlong(jr, JR_failed, failed);
   DPRINTF(("job report for job "SFN": failed = %ld\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), 
            (long) failed));
   /* err_str */
   if (*error) {
      lSetString(jr, JR_err_str, error);
      DPRINTF(("job report for job "SFN": err_str = %s\n", 
               job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring),
               error));
   }

   /* general_failure */
   switch (failed) {
   case SSTATE_BEFORE_PROLOG:
   case SSTATE_HELPER_SERVICE_ERROR:
   case SSTATE_CHECK_DAEMON_CONFIG:

   /* for CR 6252457 this block should be removed, but for a real fix the
      host or queue error detection must be improved
   */
      general_failure = GFSTATE_HOST;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   case SSTATE_PROCSET_NOTSET:
   case SSTATE_READ_CONFIG:
   case SSTATE_PROLOG_FAILED:
   case SSTATE_BEFORE_PESTART:
   case SSTATE_PESTART_FAILED:
   case SSTATE_EPILOG_FAILED:
      general_failure = GFSTATE_QUEUE;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   case SSTATE_ADD_GRP_SET_ERROR:
   case SSTATE_BEFORE_JOB:
   case SSTATE_NO_SHELL:
      {
         int job_caused_failure = 0;
         lListElem *job = NULL; 
         lListElem *ja_task = NULL;
         lListElem *master_queue = NULL;

         /* search job and ja_task */
         if (execd_get_job_ja_task(job_id, ja_task_id, &job, &ja_task)) {
            master_queue = responsible_queue(ja_task, NULL);
         }

         if ((failed == SSTATE_NO_SHELL) && (job != NULL) && 
             ((lGetList(job, JB_shell_list) != NULL) ||
              ((master_queue != NULL) &&
               JOB_TYPE_IS_BINARY(lGetUlong(job, JB_type)) &&
               JOB_TYPE_IS_NO_SHELL(lGetUlong(job, JB_type))))) {
            job_caused_failure = 1;
         } else if ((failed == SSTATE_NO_SHELL) && (master_queue != NULL)) {
            char* shell_start_mode = mconf_get_shell_start_mode();
            const char *mode = job_get_shell_start_mode(master_queue,
                                                        shell_start_mode);
            sge_free(&shell_start_mode);

            if (!strcmp(mode, "unix_behavior") != 0) {
               job_caused_failure = 1;
            }
         } else if ((failed == SSTATE_BEFORE_JOB) && (job != NULL) &&
                  JOB_TYPE_IS_BINARY(lGetUlong(job, JB_type)) &&
                  !sge_is_file(lGetString(job, JB_script_file))) {
            job_caused_failure = 1;
         } else if (failed == SSTATE_ADD_GRP_SET_ERROR) {
            job_caused_failure = 1;
         }

         general_failure = job_caused_failure ? GFSTATE_JOB : GFSTATE_QUEUE;
         job_related_adminmail(EXECD, jr, is_array, job_owner);
      }
      break;
   /*
   ** if an error occurred where the job 
   ** is source of the failure 
   */
   case SSTATE_OPEN_OUTPUT:
   case SSTATE_NO_CWD:
   case SSTATE_AFS_PROBLEM:
   case SSTATE_APPERROR:
   case SSTATE_PASSWD_MISSING:
   case SSTATE_PASSWD_WRONG:
   case SSTATE_HELPER_SERVICE_BEFORE_JOB:
      general_failure = GFSTATE_JOB;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   /*
   ** if an error occurred after the job has been run
   ** it is not as serious (unless in the epilog, as above)
   */
   case SSTATE_PESTOP_FAILED:
      general_failure = GFSTATE_QUEUE_NO_RERUN;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   case SSTATE_BEFORE_PESTOP:
   case SSTATE_BEFORE_EPILOG:
   case SSTATE_PROCSET_NOTFREED:
      general_failure = GFSTATE_NO_HALT;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   /*
   ** these are shepherd error conditions met by the execd
   */
   case ESSTATE_NO_CONFIG:
   case ESSTATE_NO_PID:
   case ESSTATE_DIED_THRU_SIGNAL:
   case ESSTATE_SHEPHERD_EXIT:
   case ESSTATE_NO_EXITSTATUS:
   case ESSTATE_UNEXP_ERRORFILE:
   case ESSTATE_UNKNOWN_JOB:
      /*
      ** test for admin mail here
      */
      general_failure = GFSTATE_NO_HALT;
      job_related_adminmail(EXECD, jr, is_array, job_owner);
      break;
   default: 
      general_failure = GFSTATE_NO_HALT;
      /*
      ** this is no error, because not all failures apply to general_failure
      */
      break;
   } /* switch */
   
   lSetUlong(jr, JR_general_failure, general_failure);
   DPRINTF(("job report for job "SFN": general_failure = %ld\n", 
            job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring),
            (long) general_failure));

   sge_dstring_free(&fname);
   sge_dstring_free(&jobdir);

   DEXIT;
   return 0;
}

/* Add usage value if one doesn't already exist.  */
static int
maybe_add_usage(lListElem *jr, const char *name, const char *val_as_str,
                double val)
{
   if (!lGetSubStr(jr, UA_name, name, JR_usage))
      return add_usage(jr, name, val_as_str, val);
   return 0;
}

/* ------------------------- */
void remove_acked_job_exit(sge_gdi_ctx_class_t *ctx, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, lListElem *jr)
{
   char *exec_file, *script_file, *tmpdir, *job_owner, *qname; 
   dstring jobdir = DSTRING_INIT;
   char fname[SGE_PATH_MAX];
   char err_str_buffer[1024];
   dstring err_str;
   SGE_STRUCT_STAT statbuf;
   lListElem *jep = NULL, *petep = NULL, *jatep = NULL;
   const char *pe_task_id_str; 
   const char *sge_root = ctx->get_sge_root(ctx);

   DENTER(TOP_LAYER, "remove_acked_job_exit");

   sge_dstring_init(&err_str, err_str_buffer, sizeof(err_str_buffer));

   if (ja_task_id == 0) {
      ERROR((SGE_EVENT, MSG_SHEPHERD_REMOVEACKEDJOBEXITCALLEDWITHX_U, sge_u32c(job_id)));
      DRETURN_VOID;
   }

   pe_task_id_str = jr?lGetString(jr, JR_pe_task_id_str):NULL;

   /* try to find this job in our job list */ 
   if (execd_get_job_ja_task(job_id, ja_task_id, &jep, &jatep)) {
      lListElem *master_q;
      int used_slots;
   
      DPRINTF(("REMOVING WITH jep && jatep\n"));
      if (pe_task_id_str) {
         petep = lGetElemStr(lGetList(jatep, JAT_task_list), PET_id, pe_task_id_str);

         if (petep == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_XYHASNOTASKZ_UUS, 
                   sge_u32c(job_id), sge_u32c(ja_task_id), pe_task_id_str));
            del_job_report(jr);
            DRETURN_VOID;
         }

         if (lGetUlong(jr, JR_state) != JEXITING) {
            WARNING((SGE_EVENT, MSG_EXECD_GOTACKFORPETASKBUTISNOTINSTATEEXITING_S, pe_task_id_str));
            /* job report has to be deleted */
            del_job_report(jr);
            DRETURN_VOID;
         }

         master_q = responsible_queue(jatep, petep);
      } else { 
         master_q = responsible_queue(jatep, NULL);
      }   
     
      /* use mail list of job instead of tasks one */
      if (jr && lGetUlong(jr, JR_state) != JSLAVE) {
         reaper_sendmail(ctx, jep, jr); 
      }


      /*
      ** security hook
      */
#ifdef KERBEROS
      /* destroy credentials cache of job */
      if (!pe_task_id_str)
         krb_destroy_forwarded_tgt(job_id);
#endif
      /*
      ** Execute command to delete the client's DCE or Kerberos credentials.
      */
      if (mconf_get_do_credentials())
         delete_credentials(sge_root, jep);

      /* remove job/task active dir */
      if (!mconf_get_keep_active() && !getenv("SGE_KEEP_ACTIVE")) {
         sge_get_active_job_file_path(&jobdir,
                                      job_id, ja_task_id, pe_task_id,
                                      NULL);
         DPRINTF(("removing active dir: %s\n", sge_dstring_get_string(&jobdir)));
         if (sge_rmdir(sge_dstring_get_string(&jobdir), &err_str)) {
            ERROR((SGE_EVENT, MSG_FILE_CANTREMOVEDIRECTORY_SS,
                   sge_dstring_get_string(&jobdir), err_str_buffer));
         }
      }

      /* cgroup/cpuset.  */
      if (have_cgroup_job_dir(cg_cpuset, job_id, ja_task_id))
         remove_job_cpuset (job_id, ja_task_id); /* writes own warings */

      /* increment # of free slots. In case no slot is used any longer 
         we have to remove queues tmpdir for this job */
      /* decrement the number of used slots.
       * not done for the slave container of tightly integrated parallel jobs
       */
      used_slots = qinstance_slots_used(master_q);
      if (lGetUlong(jatep, JAT_status) == JSLAVE && pe_task_id_str == NULL) {
         DPRINTF(("remove_acked_job_exit for slave container - not decreasing used slots"));
      } else {
         used_slots--;
         qinstance_set_slots_used(master_q, used_slots);
         DPRINTF(("%s: used slots decreased to %d\n", lGetString(master_q, QU_full_name), used_slots));
      }

      /* 
       * when the job finished / the last task of a pe job finished
       * delete the tmpdir for this job
       * the slave container of a tightly integrated parallel job 
       * does not have a tmpdir
       */
      if (used_slots == 0) {
         if (lGetUlong(jatep, JAT_status) != JSLAVE || pe_task_id_str != NULL) {
         sge_remove_tmpdir(lGetString(master_q, QU_tmpdir), 
                           lGetString(jep, JB_owner), lGetUlong(jep, JB_job_number), 
                           ja_task_id, lGetString(master_q, QU_qname));
         }
      }

      if (pe_task_id_str == NULL) {
         /*
          * Clean-up the job container (master and slave tasks)
          */

         if (!mconf_get_simulate_jobs()) {
            job_remove_spool_file(job_id, ja_task_id, NULL, SPOOL_WITHIN_EXECD);

            if (!JOB_TYPE_IS_BINARY(lGetUlong(jep, JB_type)) &&
                lGetString(jep, JB_exec_file) &&
                lGetUlong(jatep, JAT_status) != JSLAVE) {

               /*
                * Unlink job_script if there are no other master tasks
                * (e.g. task arrays) for this job present on execd
                */
               if (count_master_tasks(*(object_type_get_master_list(SGE_TYPE_JOB)), job_id) <= 1) {
                  DPRINTF(("unlinking script file %s\n", lGetString(jep, JB_exec_file)));
                  unlink(lGetString(jep, JB_exec_file));
               }
            }
         }
      } else {
         DPRINTF(("not removing job file: pe_task_id_str = %s\n", pe_task_id_str));
      }

      if (pe_task_id_str != NULL) {
         /*
          * Clean-up the PE task (a second pass cleans the container)
          */

         /* unchain pe task element from task list */
         lRemoveElem(lGetList(jatep, JAT_task_list), &petep);

         /*
          * if this was the last pe_task of an exiting slave job
          * send the slave job final report
          */
         if (lGetNumberOfElem(lGetList(jatep, JAT_task_list)) == 0) {
            lListElem *jr = get_job_report(job_id, ja_task_id, NULL);
            if (jr != NULL && lGetUlong(jr, JR_state) == JSLAVE && ISSET(lGetUlong(jatep, JAT_state), JDELETED)) {
              /* Fixme:  Repalced out on the same basis as the one
                 inexecd_slave_job_exit -- why was it there?  */
               /* add_usage(jr, "exit_status", NULL, 0); */
               maybe_add_usage(jr, "exit_status", NULL, 0);
               flush_job_report(jr);
            }
         }
      } else {
         /* check if job has queue limits and decrease global flag if necessary */
         modify_queue_limits_flag_for_job(ctx->get_qualified_hostname(ctx), jep, false);

         if (used_slots == 0 || mconf_get_simulate_jobs()) {
            /* remove the jep element only if all slave tasks are gone, we need to job object to remove the tmpdir */
            lRemoveElem(*(object_type_get_master_list(SGE_TYPE_JOB)), &jep);
         }
      }

      /* delete the job report */
      del_job_report(jr);

   } else {
      /* must be an ack of an ask job request from qmaster */
      DPRINTF(("REMOVING WITHOUT jep && jatep\n"));
      /* clean up active jobs entry etc. */
      if (pe_task_id_str == NULL) {
         ERROR((SGE_EVENT, MSG_SHEPHERD_ACKNOWLEDGEFORUNKNOWNJOBXYZ_UUS, 
                sge_u32c(job_id),  sge_u32c(ja_task_id), 
                (pe_task_id_str ? pe_task_id_str : MSG_MASTER)));

      /*
      ** security hook
      */
#ifdef KERBEROS
         krb_destroy_forwarded_tgt(job_id);
#endif
         sge_get_active_job_file_path(&jobdir,
                                      job_id, ja_task_id, pe_task_id,
                                      NULL);
         if (SGE_STAT(sge_dstring_get_string(&jobdir), &statbuf)) {
            ERROR((SGE_EVENT, MSG_SHEPHERD_CANTFINDACTIVEJOBSDIRXFORREAPINGJOBY_SU, 
                  sge_dstring_get_string(&jobdir), sge_u32c(job_id)));
         } else {
            /*** read config file written by exec_job ***/ 
            snprintf(fname, sizeof(fname), "%s/config", sge_dstring_get_string(&jobdir));
            if (read_config(fname)) {
               /* This should happen very rarely. exec_job() should avoid this 
                  condition as far as possible. One possibility for this case is, 
                  that the execd dies just after making the jobs active directory. 
                  The pain with this case is, that we have not much information
                  to report this job to qmaster. */

               if (sge_rmdir(sge_dstring_get_string(&jobdir), &err_str)) {
                  ERROR((SGE_EVENT, MSG_FILE_CANTREMOVEDIRECTORY_SS,
                         sge_dstring_get_string(&jobdir), err_str_buffer));
               }
            }

            /* do not remove xterm or qlogin starter ! */
            if ((script_file = get_conf_val("script_file")) 
                  && strcasecmp(script_file, "INTERACTIVE")
                  && strcasecmp(script_file, "QLOGIN")) {
               if ((exec_file = get_conf_val("exec_file"))) {
                  DPRINTF(("removing exec_file %s\n", exec_file));
                  unlink(exec_file);
               }
            }
            
            /* tmpdir */
            if ((!(tmpdir = get_conf_val("queue_tmpdir"))) || 
                 (!(qname = get_conf_val("queue"))) || 
             (!(job_owner = get_conf_val("job_owner")))) {
               ERROR((SGE_EVENT, MSG_SHEPHERD_INCORRECTCONFIGFILEFORJOBXY_UU, 
                     sge_u32c(job_id), sge_u32c(ja_task_id)));
            } else {
               DPRINTF(("removing queue_tmpdir %s\n", tmpdir));
               sge_remove_tmpdir(tmpdir, job_owner, job_id, ja_task_id, qname);
            }
         }

         /* job */
         if (!mconf_get_simulate_jobs()) {
            job_remove_spool_file(job_id, ja_task_id, NULL, SPOOL_WITHIN_EXECD);
         }

         /* active dir */
         if (!mconf_get_keep_active() && !getenv("SGE_KEEP_ACTIVE")) {
            DPRINTF(("removing active dir: %s\n", sge_dstring_get_string(&jobdir)));
            if (sge_rmdir(sge_dstring_get_string(&jobdir), &err_str)) {
               ERROR((SGE_EVENT, MSG_FILE_CANTREMOVEDIRECTORY_SS,
                      sge_dstring_get_string(&jobdir), err_str_buffer));
            }
         }

         /* cgroup/cpuset.  */
         if (have_cgroup_job_dir(cg_cpuset, job_id, ja_task_id))
            remove_job_cpuset (job_id, ja_task_id); /* writes own warings */
      }
      del_job_report(jr);
      cleanup_job_report(job_id, ja_task_id);
   }

   sge_dstring_free(&jobdir);

   DRETURN_VOID;
}

/**************************************************************************
 This function is called if we fail to start a job. 
 Returns the appropriate job report.

 We clean up the job.
 We report the job_number and the error to the master.
 If general is set we have a general problems starting jobs.
 The master should not give us jobs in the future and the administrator
 should be informed about the problem (e.g. we cant write to a filesystem).
 **************************************************************************/
lListElem *execd_job_start_failure(lListElem *jep, lListElem *jatep,
                                   lListElem *petep, const char *error_string,
                                   int general)
{
   return execd_job_failure(jep, jatep, petep, error_string, general, SSTATE_FAILURE_BEFORE_JOB);
}

lListElem *execd_job_run_failure(lListElem *jep, lListElem *jatep,
                                 lListElem *petep, const char *error_string,
                                 int general)
{
   return execd_job_failure(jep, jatep, petep, error_string, general, SSTATE_FAILURE_AFTER_JOB);
}

static lListElem *execd_job_failure(lListElem *jep, lListElem *jatep, lListElem *petep,
                                    const char *error_string, int general, int failed)
{
   lListElem *jr, *ep = NULL;
   u_long32 jobid, jataskid = 0;
   const char *petaskid = NULL;

   DENTER(TOP_LAYER, "execd_job_failure");

   jobid = lGetUlong(jep, JB_job_number);
   if (jatep != NULL) {
      jataskid = lGetUlong(jatep, JAT_task_number);
   }
   if (petep != NULL) {
      petaskid = lGetString(petep, PET_id);
   }

   ERROR((SGE_EVENT, 
      (failed==SSTATE_FAILURE_BEFORE_JOB)?
         MSG_SHEPHERD_CANTSTARTJOBXY_US:
         MSG_SHEPHERD_PROBLEMSAFTERSTART_DS, sge_u32c(jobid), error_string));

   jr = get_job_report(jobid, jataskid, petaskid);
   if (!jr) {
      DPRINTF(("no job report found to report job start failure!\n"));
      jr = add_job_report(jobid, jataskid, petaskid, jep);
   }
   
   if (petep != NULL) {
      ep = lFirst(lGetList(petep, PET_granted_destin_identifier_list));
   } else if (jatep != NULL) {
      ep = lFirst(lGetList(jatep, JAT_granted_destin_identifier_list));
   }

   if (ep != NULL) {
      lSetString(jr, JR_queue_name, lGetString(ep, JG_qname));
   }
      
   lSetUlong(jr, JR_failed, failed);
   lSetUlong(jr, JR_general_failure, general);
   lSetString(jr, JR_err_str, error_string);
  
   lSetUlong(jr, JR_state, JEXITING);

   job_related_adminmail(EXECD, jr, job_is_array(jep), lGetString(jep, JB_owner));

   DRETURN(jr);
}


/**************************************************************************
 This function is called if we are asked by the master concerning a job
 we dont know anything about. We have to tell this to the qmaster, so that
 he can clean up this job.
 This is done very like the normal job finish and runs into the same
 functions in the qmaster.
 **************************************************************************/
void job_unknown(u_long32 jobid, u_long32 jataskid, char *qname)
{
   lListElem *jr;

   DENTER(TOP_LAYER, "job_unknown");

   ERROR((SGE_EVENT, MSG_SHEPHERD_JATASKXYISKNOWNREPORTINGITTOQMASTER, 
     sge_u32c(jobid), sge_u32c(jataskid)));

   jr = add_job_report(jobid, jataskid, NULL, NULL);
   if (jr) {
      lSetString(jr, JR_queue_name, qname);
      lSetUlong(jr, JR_failed, ESSTATE_UNKNOWN_JOB); 
      lSetUlong(jr, JR_state, JEXITING);
      lSetString(jr, JR_err_str, (char*) MSG_JR_ERRSTR_EXECDDONTKNOWJOB);
   }

   DEXIT;
   return;
}

/************************************************************************
 Look for old jobs hanging around on disk and report them to the qmaster.
 This is used at execd startup time.
 We have to call it cyclic cause there may be jobs alive while execd went
 down and up. If such a job exits we get no SIGCLD from shepherd.
 If startup is true this is the first call of the execd. We produce more
 output for the administrator the first time.
 ************************************************************************/

/* fixme: deal with old cpusets/cgroups too */

int clean_up_old_jobs(sge_gdi_ctx_class_t *ctx, int startup)
{
   SGE_STRUCT_DIRENT *dent = NULL;
   DIR *cwd = NULL;
   char dir[SGE_PATH_MAX];
   pid_t pids[10000];   /* a bunch of processes */
   int npids;           /* number of running processes */
   char *jobdir;
   u_long32 jobid, jataskid;
   static int lost_children = 1;
   lListElem *jep, *petep, *jatep = NULL;

   DENTER(TOP_LAYER, "clean_up_old_jobs");

   if (startup) {
      INFO((SGE_EVENT, SFNMAX, MSG_SHEPHERD_CKECKINGFOROLDJOBS));
   }

   /* 
      If we get an empty Master_Job_List we know that 
      it is no longer necessary to pass this code 
     
      Getting job information by parsing ps-output 
      is very expensive. The aim is to get informed
      about jobs that were started by the execd 
      running before we were started. Jobs that
      were started by us are our childs and we
      get a cheap SIGCLD informing us about the
      jobs exit. 
       
      So if we arrive here and an empty Master_Job_List
      we know all jobs that were our "lost children" 
      exited and there is no need for ps-commands.

   */
   if (mconf_get_simulate_jobs() ||
       lGetNumberOfElem(*(object_type_get_master_list(SGE_TYPE_JOB))) == 0 ||
       !lost_children) {
      if (lost_children) {
         if (startup) {
            INFO((SGE_EVENT, SFNMAX, MSG_SHEPHERD_NOOLDJOBSATSTARTUP));
         } else {
            INFO((SGE_EVENT, SFNMAX, MSG_SHEPHERD_NOMOREOLDJOBSAFTERSTARTUP));
         }
         /* 
          * Now setting lost_children to 0 which disables further pid checking
          * for the whole runtime of the execd
          */
         lost_children = 0;
      }
      /* all children exited */
      DEXIT;
      return 0;
   }

   /* Get pids of running jobs. So we can look for running shepherds. */
   npids = sge_get_pids(pids, 10000, SGE_SHEPHERD, PSCMD);
   if (npids == -1) {
      ERROR((SGE_EVENT, SFNMAX, MSG_SHEPHERD_CANTGETPROCESSESFROMPSCOMMAND));
      DEXIT;
      return -1;
   }

   DPRINTF(("found %d running processes\n", npids));

   /* We read the job information from active dir. There is one subdir for each
      started job. Shepherd writes exit_status and usage to this directory.
      Cause maybe shepherd was killed while execd was down we have to look into
      the process table too. */

   if (!(cwd=opendir(ACTIVE_DIR))) {
      ERROR((SGE_EVENT, MSG_FILE_CANTOPENDIRECTORYX_SS, ACTIVE_DIR, strerror(errno)));
      DEXIT;
      return -1;
   }

   while ((dent=SGE_READDIR(cwd))) {
      char string[256], *token, *endp;
      u_long32 tmp_id;

      jobdir = dent->d_name;    /* jobdir is the jobid.jataskid converted to string */
      sge_strlcpy(string, jobdir, sizeof(string));

      if (!strcmp(jobdir, ".") || !strcmp(jobdir, "..") )
         continue;

      jobid = jataskid = 0;
      if ((token = strtok(string, " ."))) {
         tmp_id = strtol(token, &endp, 10);
         if (*endp == '\0') {
            jobid = tmp_id;
            if ((token = strtok(NULL, " \n\t"))) {
               tmp_id = strtol(token, &endp, 10);
               if (*endp == '\0') {
                  jataskid = tmp_id;
               }
            }
         }
      }

      if (!jobid || !jataskid) {
         /* someone left his garbage in our directory */
         WARNING((SGE_EVENT, MSG_SHEPHERD_XISNOTAJOBDIRECTORY_S, jobdir)); 
         continue;
      }

      /* seek job to this jobdir */
      if (!execd_get_job_ja_task(jobid, jataskid, &jep, &jatep)) {
         /* missing job in job dir but not in active job dir */
         if (startup) {
            ERROR((SGE_EVENT, MSG_SHEPHERD_FOUNDACTIVEJOBDIRXWHILEMISSINGJOBDIRREMOVING_S, jobdir)); 
         }
         /* remove active jobs directory */
         DPRINTF(("+++++++++++++++++++ remove active jobs directory ++++++++++++++++++\n"));
         {
            char path[SGE_PATH_MAX];
            snprintf(path, sizeof(path), ACTIVE_DIR"/%s", jobdir);
            sge_rmdir(path, NULL);
         }
         continue;
      }
      if (lGetUlong(jatep, JAT_status) != JSLAVE) {
         snprintf(dir, sizeof(dir), "%s/%s", ACTIVE_DIR, jobdir);
         examine_job_task_from_file(ctx, startup, dir, jep, jatep, NULL, pids, npids);
      }
      for_each(petep, lGetList(jatep, JAT_task_list)) {
         snprintf(dir, sizeof(dir), "%s/%s/%s", ACTIVE_DIR, jobdir,
                  lGetString(petep, PET_id));
         examine_job_task_from_file(ctx, startup, dir, jep, jatep, petep, pids, npids);
      }
   }    /* while (dent=SGE_READDIR(cwd)) */

   closedir(cwd);

   DEXIT;
   return 0;
}

static void 
examine_job_task_from_file(sge_gdi_ctx_class_t *ctx, int startup, char *dir, lListElem *jep,
                           lListElem *jatep, lListElem *petep, pid_t *pids, 
                           int npids) 
{
   lListElem *jr = NULL;
   int shepherd_alive;  /* =1 -> this shepherd is in the process table */
   FILE *fp = NULL;
   SGE_STRUCT_STAT statbuf;
   char fname[SGE_PATH_MAX];
   pid_t pid;           /* pid of shepherd */
   char err_str[1024];
   u_long32 jobid, jataskid;
   const char *pe_task_id_str = NULL;
   static u_long32 startup_time = 0;

   DENTER(TOP_LAYER, "examine_job_task_from_file");
   
   if (!startup_time) {
      startup_time = sge_get_gmt();
   }   

   jobid = lGetUlong(jep, JB_job_number);
   jataskid = lGetUlong(jatep, JAT_task_number);
   if (petep != NULL) {
      pe_task_id_str = lGetString(petep, PET_id);
   }

   if (SGE_STAT(dir, &statbuf)) {
      ERROR((SGE_EVENT, MSG_SHEPHERD_CANTSTATXY_SS, dir, strerror(errno)));
      DEXIT;
      return;
   }

   if (!S_ISDIR(statbuf.st_mode)) {
      ERROR((SGE_EVENT, MSG_FILE_XISNOTADIRECTORY_S, dir));
      DEXIT;
      return;
   }

   DPRINTF(("Found job directory: %s\n", dir));
   if (startup) {
      INFO((SGE_EVENT, MSG_SHEPHERD_FOUNDDIROFJOBX_S, dir));
   }   
   
   /* Look for pid of shepherd */
   snprintf(fname, sizeof(fname), "%s/pid", dir);
   if (!(fp = fopen(fname, "r"))) {
      /* Is it 
            1. a job started before startup of execd
               In this case the job was started by the old execd 
               and there must be a pid fild -> Logging
         or is it 
            2. a newly started job
               In this case the shepherd had not enough time 
               to write the pid file -> No Logging */
      if (startup && startup_time >= lGetUlong(jatep, JAT_start_time)) {
         ERROR((SGE_EVENT, MSG_SHEPHERD_CANTREADPIDFILEXFORJOBYSTARTTIMEZX_SSUS,
                fname, dir, sge_u32c(lGetUlong(jatep, JAT_start_time)), strerror(errno)));
         /* seek job report for this job - it must be contained in job report
            If this is a newly started execd we can assume the execd was broken
            in the interval between making the jobs active directory and writing
            the shepherds pid (done by the started shepherd). So we just remove
            and report this job. */
         if (!(jr=get_job_report(jobid, jataskid, pe_task_id_str))) {
            CRITICAL((SGE_EVENT, MSG_SHEPHERD_MISSINGJOBXINJOBREPORTFOREXITINGJOB_U, sge_u32c(jobid)));
            jr = add_job_report(jobid, jataskid, pe_task_id_str, NULL);
         }
         lSetUlong(jr, JR_state, JEXITING);
         clean_up_job(jr, ESSTATE_NO_PID, 0, job_is_array(jep),
                      lGetString(jep, JB_owner)); /* failed before execution */
      }
      DRETURN_VOID;
   }
   
   if (fscanf(fp, pid_t_fmt, &pid) != 1) {
      /* most probably a newly started job
         shepherd usually just had not enough time for writing the pid file 
         if these warnings do appear frequently one might consider having
         execd (and thus the shepherds) do local spooling instead of via NFS */
      WARNING((SGE_EVENT, MSG_SHEPHERD_CANTREADPIDFROMPIDFILEXFORJOBY_SS,
                  fname, dir));
      FCLOSE_IGNORE_ERROR(fp);
      DEXIT;
      return;
   }
   FCLOSE_IGNORE_ERROR(fp);

   /* look whether shepherd is still alive */ 
   shepherd_alive = sge_contains_pid(pid, pids, npids);

   /* report this information */
   if (shepherd_alive) {
      snprintf(err_str, sizeof(err_str),
               MSG_SHEPHERD_SHEPHERDFORJOBXHASPIDYANDISZALIVE_SU,
               dir, sge_u32c(pid));
   } else {
      snprintf(err_str, sizeof(err_str),
               MSG_SHEPHERD_SHEPHERDFORJOBXHASPIDYANDISNOTALIVE_SU,
               dir, sge_u32c(pid));
   }
   if (startup) {
      INFO((SGE_EVENT, SFNMAX, err_str));
      modify_queue_limits_flag_for_job(ctx->get_qualified_hostname(ctx), jep, true);
   } else {
      DPRINTF(("%s", err_str));
   }

   if (shepherd_alive) {     /* shepherd alive -> nothing to do */

      if (startup) {
         /*  
            at startup we need to change the 
            state of not exited jobs from JWRITTEN 
            to JRUNNING
         */ 
         if ((jr=get_job_report(jobid, jataskid, pe_task_id_str))) {

            lSetUlong(jr, JR_state, JRUNNING);
            /* here we will call a ptf function to get */
            /* the first usage data after restart      */

            if (HAVE_HWLOC) {
               /* do accounting of bound cores */ 
               dstring fconfig = DSTRING_INIT;

               sge_get_active_job_file_path(&fconfig, jobid, jataskid, pe_task_id_str, "config");
               
               if (sge_dstring_get_string(&fconfig) == NULL) {
                  DPRINTF(("couldn't find config file for running job\n"));
               } else {   
                  DPRINTF(("path to config file %s\n", sge_dstring_get_string(&fconfig)));
                  update_used_cores(sge_dstring_get_string(&fconfig), &jr);
               }
               
               sge_dstring_free(&fconfig);
                  
            }            

         } else {
            /* found job in active jobs directory 
               but not in spool directory of execd */
            ERROR((SGE_EVENT, MSG_SHEPHERD_INCONSISTENTDATAFORJOBX_U, sge_u32c(jobid)));
            jr = add_job_report(jobid, jataskid, pe_task_id_str, NULL);
            lSetUlong(jr, JR_state, JEXITING);
         }
      }
      DEXIT;
      return;
   }

   /* seek job report for this job - it must be contained in job report */
   if (!(jr=get_job_report(jobid, jataskid, pe_task_id_str))) {
      CRITICAL((SGE_EVENT, MSG_SHEPHERD_MISSINGJOBXYINJOBREPORT_UU, sge_u32c(jobid), sge_u32c(jataskid)));
      jr = add_job_report(jobid, jataskid, pe_task_id_str, jep);
      DEXIT;
      return;
   }

   /* if the state is already JEXITING work is done  
      for this job and we wait for ACK from qmaster */
   if (lGetUlong(jr, JR_state)==JEXITING) {
      DPRINTF(("State of job "sge_u32"."sge_u32" already changed to JEXITING\n", jobid, jataskid));
      DRETURN_VOID;
   }

   clean_up_job(jr, 0, 0, job_is_array(jep), lGetString(jep, JB_owner));
   lSetUlong(jr, JR_state, JEXITING);
   

   flush_job_report(jr);

   DRETURN_VOID;
}

static void update_used_cores(const char* path_to_config, lListElem** jr)
{
  if (HAVE_HWLOC) {
   const char* binding_cfg;
   
   DENTER(TOP_LAYER, "update_used_cores");
  
   DPRINTF(("update used cores: %s\n", path_to_config));

   if (!read_config(path_to_config)) {

      binding_cfg = get_conf_val("binding");

      if (binding_cfg == NULL) {
         DPRINTF(("couldn't get binding element from config file!\n"));
      } else {

         DPRINTF(("BINDING bindingcfg %s\n", binding_cfg));

         if (binding_cfg != NULL) {
            /* extract the job binding string and account it */
            const char* jobtopo = binding_get_topology_for_job(binding_cfg);
         
            if (jobtopo != NULL) {
               /* usage in job report */
               dstring pseudo_usage = DSTRING_INIT; 

               /* account the used cores on execd global */
               DPRINTF(("account cores used by job: %s\n", jobtopo));
               account_job(jobtopo);

               /* add to job report (for qstat -j x -cb) */
               sge_dstring_sprintf(&pseudo_usage, "binding_inuse!%s", jobtopo); 
               
               add_usage(*jr, sge_dstring_get_string(&pseudo_usage), NULL, 0);
               sge_dstring_free(&pseudo_usage); 

            } else {
               DPRINTF(("topology not found\n"));
            }   
         } else {
            DPRINTF(("binding_cfg is NULL\n"));
         }
      
      } /* binding_cfg found */

   } else {
      DPRINTF(("couldnt read config in\n"));
   }
   DRETURN_VOID;
  }
}

/************************************************************/
/* fill dusage with:                                        */ 
/* - data retrieved from config - file                      */
/* - data retrieved from "usage" as shepherd has written it */
/*   if we cant read "usage" exit_status = 0xffffffff       */ 
/************************************************************/
static int 
read_dusage(lListElem *jr, const char *jobdir, u_long32 jobid, u_long32 jataskid, const char *pe_task_id, int failed)
{
   char pid_file[SGE_PATH_MAX];
   FILE *fp;
   u_long32 pid;

   DENTER(TOP_LAYER, "read_dusage");

   pid = 0xffffffff;

   snprintf(pid_file, sizeof(pid_file), "%s/pid", jobdir);

   if (failed != ESSTATE_NO_PID) {
      fp = fopen(pid_file, "r");
      if (fp != NULL) {
         if (fscanf(fp, sge_u32 , &pid) != 1) {
            ERROR((SGE_EVENT, MSG_EXECD_ERRORREADINGPIDOFJOB_UU,
                   sge_u32c(jobid), sge_u32c(jataskid)));
         }
         FCLOSE(fp);
      } else {
         ERROR((SGE_EVENT, MSG_SHEPHERD_CANTOPENPIDFILEXFORJOBYZ_SUU,
                pid_file, sge_u32c(jobid), sge_u32c(jataskid)));
      }
   }

   if (failed != ESSTATE_NO_CONFIG) {
      dstring buffer = DSTRING_INIT;
      const char *qinstance_name = NULL;
   
      qinstance_name = sge_dstring_sprintf(&buffer, SFN"@"SFN,
                                           get_conf_val("queue"),
                                           get_conf_val("host"));
      lSetString(jr, JR_queue_name, qinstance_name);
      qinstance_name = NULL;
      sge_dstring_free(&buffer);

      add_usage(jr, "submission_time", get_conf_val("submission_time"), (double)0);
      add_usage(jr, "priority", get_conf_val("priority"),        (double)0);
   }

   /* read "usage" file */
   /*
   ** there may be more failure states where there is no usage file
   ** but it is best if we try to read it and ignore the error if reading failed
   ** if we have an error already
   */
   if (!failed || (failed > SSTATE_BEFORE_JOB)) {
      char usage_file[SGE_PATH_MAX];
      snprintf(usage_file, sizeof(usage_file), "%s/usage", jobdir);
      fp = fopen(usage_file, "r");
      if (fp) {
         char buf[10000];
         lList *cflp = NULL;
         u_long32 wait_status;

         read_config_list(fp, &cflp, NULL, CF_Type, CF_name, CF_value, 0, "=", 0, buf, sizeof(buf));
         FCLOSE(fp);

         if (extract_ulong_attribute(&cflp, "wait_status", &wait_status)==0)
            lSetUlong(jr, JR_wait_status, wait_status);

         convert_attribute(&cflp, jr, "exit_status",   1);
         convert_attribute(&cflp, jr, "signal",        0);
         convert_attribute(&cflp, jr, "start_time",    0);
         convert_attribute(&cflp, jr, "end_time",      0);

         convert_attribute(&cflp, jr, "ru_wallclock",  0);

         convert_attribute(&cflp, jr, "ru_utime",      0);
         convert_attribute(&cflp, jr, "ru_stime",      0);
         convert_attribute(&cflp, jr, "ru_maxrss",     0);
         convert_attribute(&cflp, jr, "ru_ixrss",      0);
         convert_attribute(&cflp, jr, "ru_ismrss",     0);
         convert_attribute(&cflp, jr, "ru_idrss",      0);
         convert_attribute(&cflp, jr, "ru_isrss",      0);
         convert_attribute(&cflp, jr, "ru_minflt",     0);
         convert_attribute(&cflp, jr, "ru_majflt",     0);
         convert_attribute(&cflp, jr, "ru_nswap",      0);
         convert_attribute(&cflp, jr, "ru_inblock",    0);
         convert_attribute(&cflp, jr, "ru_oublock",    0);
         convert_attribute(&cflp, jr, "ru_msgsnd",     0);
         convert_attribute(&cflp, jr, "ru_msgrcv",     0);
         convert_attribute(&cflp, jr, "ru_nsignals",   0);
         convert_attribute(&cflp, jr, "ru_nvcsw",      0);
         convert_attribute(&cflp, jr, "ru_nivcsw",     0);

         build_derived_final_usage(jr, jobid, jataskid, pe_task_id);
         lFreeList(&cflp);
      } else {
         ERROR((SGE_EVENT, MSG_SHEPHERD_CANTOPENUSAGEFILEXFORJOBYZX_SUUS,
                usage_file, sge_u32c(jobid), sge_u32c(jataskid), strerror(errno)));
         DRETURN(-1);
      }
   }


#if 0
   {
      lListElem *ep;

      if (lGetList(jr, JR_usage)) {
         DPRINTF(("resulting usage attributes:\n"));
      } else {
         DPRINTF(("empty usage list\n"));
      }   

      for_each (ep, lGetList(jr, JR_usage)) {
         DPRINTF(("    \"%s\" = %f\n",
            lGetString(ep, UA_name),
            lGetDouble(ep, UA_value)));
      }
   }
#endif
   DRETURN(0);
FCLOSE_ERROR:
   ERROR((SGE_EVENT, MSG_FILE_NOCLOSE_SS, "usage or pid",
          strerror(errno)));
   DRETURN(-1);
}


static void build_derived_final_usage(lListElem *jr, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id)
{
   lList *usage_list;
   double ru_cpu, pdc_cpu;
   double cpu, r_cpu,
          mem, r_mem,
          io, iow, r_io, r_iow, maxvmem, r_maxvmem;

   lListElem *job = NULL;
   lListElem *ja_task = NULL;
   lListElem *pe_task = NULL;

   bool accounting_summary = false;

   DENTER(TOP_LAYER, "build_derived_final_usage");

   usage_list = lGetList(jr, JR_usage);
   
   /* cpu    = MAX(sum of "ru_utime" and "ru_stime" , PDC "cpu" usage) */
   ru_cpu = usage_list_get_double_usage(usage_list, "ru_utime", 0) +
            usage_list_get_double_usage(usage_list, "ru_stime", 0);
   pdc_cpu = usage_list_get_double_usage(usage_list, USAGE_ATTR_CPU, 0);
   cpu = MAX(ru_cpu, pdc_cpu);

   /* build reserved usage if required */ 
   r_cpu = r_mem = r_maxvmem = 0.0;
   if (mconf_get_acct_reserved_usage() || mconf_get_sharetree_reserved_usage()) {
      if (execd_get_job_ja_task(job_id, ja_task_id, &job, &ja_task)) {
         double wallclock;
         u_long32 end_time = usage_list_get_ulong_usage(usage_list, "end_time", 0);
         const lListElem *pe = lGetObject(ja_task, JAT_pe_object);
         if (pe != NULL && lGetBool(pe, PE_accounting_summary)) {
            accounting_summary = true;
         }

         if (pe_task_id != NULL) {
            pe_task = lGetSubStr(ja_task, PET_id, pe_task_id, JAT_task_list);
         }

         build_reserved_usage(end_time, ja_task, pe_task, &wallclock, &r_cpu, &r_mem, &r_maxvmem);
      }
   }

   /* mem    = PDC "mem" usage or zero */
   mem = usage_list_get_double_usage(usage_list, USAGE_ATTR_MEM, 0);

   /* io     = PDC "io" usage or zero */
   io = usage_list_get_double_usage(usage_list, USAGE_ATTR_IO, 0);

   /* r_io   = 0 */
   r_io = io;

   /* iow     = PDC "io wait time" or zero */
   iow = usage_list_get_double_usage(usage_list, USAGE_ATTR_IOW, 0);

   /* r_iow  = 0 */
   r_iow = iow;

   /* maxvmem */
   maxvmem = usage_list_get_double_usage(usage_list, USAGE_ATTR_MAXVMEM, 0);

   DPRINTF(("CPU/MEM/IO: M(%f/%f/%f) R(%f/%f/%f) acct: %s stree: %s\n",
         cpu, mem, io, r_cpu, r_mem, r_io,
         mconf_get_acct_reserved_usage()?"R":"M", mconf_get_sharetree_reserved_usage()?"R":"M"));

   /* Report reserved usage or real usage.
    * If accounting_summary is activated for a tightly integrated
    * parallel job, we do not send any usage for the pe tasks.
    */
   if (mconf_get_acct_reserved_usage()) {
      if (!(accounting_summary && pe_task_id != NULL)) {
         add_usage(jr, USAGE_ATTR_CPU_ACCT, NULL, r_cpu);
         add_usage(jr, USAGE_ATTR_MEM_ACCT, NULL, r_mem);
         add_usage(jr, USAGE_ATTR_IO_ACCT,  NULL, r_io);
         add_usage(jr, USAGE_ATTR_IOW_ACCT, NULL, r_iow);
         if (r_maxvmem != DBL_MAX) {
            add_usage(jr, USAGE_ATTR_MAXVMEM_ACCT, NULL, r_maxvmem);
         }
      }
   } else {
      add_usage(jr, USAGE_ATTR_CPU_ACCT, NULL, cpu);
      add_usage(jr, USAGE_ATTR_MEM_ACCT, NULL, mem);
      add_usage(jr, USAGE_ATTR_IO_ACCT,  NULL, io);
      add_usage(jr, USAGE_ATTR_IOW_ACCT, NULL, iow);
      if (maxvmem != DBL_MAX) {
         add_usage(jr, USAGE_ATTR_MAXVMEM_ACCT, NULL, maxvmem);
      }

      /* We might have switched off sending of pe task job reports
       * when starting the task with accounting_summary & sharetree_reserved_usage.
       * But if we want to see *real* acct usage, enable sending this final report.
       */
      if (pe_task_id != NULL && lGetBool(jr, JR_no_send)) {
         lSetBool(jr, JR_no_send, false);
      }
   }

   /* Report reserved usage or real usage.
    * If accounting_summary is activated for a tightly integrated
    * parallel job, we do not send any usage for the pe tasks.
    */
   if (mconf_get_sharetree_reserved_usage()) {
      if (!(accounting_summary && pe_task_id != NULL)) {
         add_usage(jr, USAGE_ATTR_CPU, NULL, r_cpu);
         add_usage(jr, USAGE_ATTR_MEM, NULL, r_mem);
         add_usage(jr, USAGE_ATTR_IO,  NULL, r_io);
         add_usage(jr, USAGE_ATTR_IOW, NULL, r_iow);
         if (r_maxvmem != DBL_MAX) {
            add_usage(jr, USAGE_ATTR_MAXVMEM, NULL, r_maxvmem);
         }
      }
   } else {
      add_usage(jr, USAGE_ATTR_CPU, NULL, cpu);
      add_usage(jr, USAGE_ATTR_MEM, NULL, mem);
      add_usage(jr, USAGE_ATTR_IO,  NULL, io);
      add_usage(jr, USAGE_ATTR_IOW, NULL, iow);
      if (maxvmem != DBL_MAX) {
         add_usage(jr, USAGE_ATTR_MAXVMEM, NULL, maxvmem);
      }
   }

   DRETURN_VOID;
}

/*****************************************************************/
static void convert_attribute(
lList **cflpp,
lListElem *jr,
char *name,
u_long32 udefault 
) {
   const char *s;

   s = get_conf_value(NULL, *cflpp, CF_name, CF_value, name);
   add_usage(jr, name, s, (double)udefault);
   lDelElemStr(cflpp, CF_name, name);
}


/*****************************************************************/

static int extract_ulong_attribute(
lList **cflpp,
char *name,
u_long32 *valuep
) {
   const char *s;
   int ret;

   if (!(s = get_conf_value(NULL, *cflpp, CF_name, CF_value, name)))
      return -1;
   ret = sscanf(s, sge_u32, valuep);
   lDelElemStr(cflpp, CF_name, name);
   return (ret == 1)?0:-1;
}


/* send mail to users if requested */
static void reaper_sendmail(sge_gdi_ctx_class_t *ctx, lListElem *jep,
                            lListElem *jr) {
   lList *mail_users; 
   u_long32 mail_options; 
   char sge_mail_subj[1024];
   char sge_mail_body[10*2048];
   char sge_mail_start[128];
   char sge_mail_end[128];
   u_long32 jobid, taskid, failed;
   double ru_utime, ru_stime, ru_wallclock;
   double ru_cpu = 0.0, ru_maxvmem = 0.0;
   int exit_status = -1, signo = -1;
   const char *q, *h, *u;
   lListElem *ep;
   const char *pe_task_id_str;
   dstring ds;
   char buffer[128];
   dstring cpu_string = DSTRING_INIT;
   dstring maxvmem_string = DSTRING_INIT;
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "reaper_sendmail");

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   mail_users = lGetList(jep, JB_mail_list);
   mail_options = lGetUlong(jep, JB_mail_options); 
   pe_task_id_str = lGetString(jr, JR_pe_task_id_str);

   if (!(q=lGetString(jr, JR_queue_name))) {
      q = MSG_MAIL_UNKNOWN_NAME;
   }

   h = qualified_hostname;

   if (!(u=lGetString(jep, JB_owner)))
      u = MSG_MAIL_UNKNOWN_NAME;

   /* JG: TODO (397): Extend usage module: usage_list_get_ctime_usage() 
    *                 and use the other usage_list_get functions.
    */

   if ((ep=lGetSubStr(jr, UA_name, "start_time", JR_usage)))
      strcpy(sge_mail_start, sge_ctime((time_t)lGetDouble(ep, UA_value), &ds));
   else   
      strcpy(sge_mail_start, MSG_MAIL_UNKNOWN_NAME);

   if ((ep=lGetSubStr(jr, UA_name, "end_time", JR_usage)))
      strcpy(sge_mail_end, sge_ctime((time_t)lGetDouble(ep, UA_value), &ds));
   else   
      strcpy(sge_mail_end, MSG_MAIL_UNKNOWN_NAME);

   if ((ep=lGetSubStr(jr, UA_name, "ru_utime", JR_usage)))
      ru_utime = lGetDouble(ep, UA_value); 
   else
      ru_utime = 0;

   if ((ep=lGetSubStr(jr, UA_name, "ru_stime", JR_usage)))
      ru_stime = lGetDouble(ep, UA_value); 
   else
      ru_stime = 0;

   if ((ep=lGetSubStr(jr, UA_name, "ru_wallclock", JR_usage)))
      ru_wallclock = lGetDouble(ep, UA_value); 
   else
      ru_wallclock = 0;

   if ((ep=lGetSubStr(jr, UA_name, USAGE_ATTR_CPU_ACCT, JR_usage)))
      ru_cpu = lGetDouble(ep, UA_value);
   if ((ep=lGetSubStr(jr, UA_name, USAGE_ATTR_MAXVMEM_ACCT, JR_usage)))
      ru_maxvmem = lGetDouble(ep, UA_value);

   jobid = lGetUlong(jr, JR_job_number);
   taskid = lGetUlong(jr, JR_ja_task_number);

   failed = lGetUlong(jr, JR_failed);
   
   if ((ep=lGetSubStr(jr, UA_name, "exit_status", JR_usage)))
      exit_status = (int)lGetDouble(ep, UA_value);
   
   double_print_time_to_dstring(ru_cpu, &cpu_string);
   double_print_memory_to_dstring(ru_maxvmem, &maxvmem_string);

        /* send job exit mail only for master task */
   if ((VALID(MAIL_AT_EXIT, mail_options)) && !failed && !pe_task_id_str) {
      dstring utime_string = DSTRING_INIT;
      dstring stime_string = DSTRING_INIT;
      dstring wtime_string = DSTRING_INIT;

      DPRINTF(("mail VALID at EXIT\n"));
      double_print_time_to_dstring(ru_utime, &utime_string);
      double_print_time_to_dstring(ru_stime, &stime_string);
      double_print_time_to_dstring(ru_wallclock, &wtime_string);
      if (job_is_array(jep)) {
         snprintf(sge_mail_subj, sizeof(sge_mail_subj),
                  MSG_MAIL_SUBJECT_JA_TASK_COMP_UUS, sge_u32c(jobid),
                  sge_u32c(taskid), lGetString(jep, JB_job_name));
         snprintf(sge_mail_body, sizeof(sge_mail_body),
                 MSG_MAIL_BODY_COMP_SSSSSSSSSSSI, 
                 sge_mail_subj,
                 u,
                 q, 
                 h,
                 sge_mail_start, 
                 sge_mail_end,
                 sge_dstring_get_string(&utime_string),
                 sge_dstring_get_string(&stime_string),
                 sge_dstring_get_string(&wtime_string),
                 (ru_cpu     == 0.0) ? "NA":sge_dstring_get_string(&cpu_string),
                 (ru_maxvmem == 0.0) ? "NA":sge_dstring_get_string(&maxvmem_string),
                 exit_status);
      } else {
         snprintf(sge_mail_subj, sizeof(sge_mail_subj), MSG_MAIL_SUBJECT_JOB_COMP_US,
                  sge_u32c(jobid), lGetString(jep, JB_job_name));
         snprintf(sge_mail_body, sizeof(sge_mail_body),
                 MSG_MAIL_BODY_COMP_SSSSSSSSSSSI,
                 sge_mail_subj,
                 u,
                 q, 
                 h,
                 sge_mail_start, 
                 sge_mail_end,
                 sge_dstring_get_string(&utime_string),
                 sge_dstring_get_string(&stime_string),
                 sge_dstring_get_string(&wtime_string),
                 (ru_cpu     == 0.0) ? "NA":sge_dstring_get_string(&cpu_string),
                 (ru_maxvmem == 0.0) ? "NA":sge_dstring_get_string(&maxvmem_string),
                 exit_status);
      }

      cull_mail(EXECD, mail_users, sge_mail_subj, sge_mail_body, MSG_MAIL_TYPE_COMP);
      sge_dstring_free(&utime_string);
      sge_dstring_free(&stime_string);
      sge_dstring_free(&wtime_string);
   }

   if (((VALID(MAIL_AT_ABORT, mail_options)) 
       || (VALID(MAIL_AT_EXIT, mail_options))) && 
       (failed || lGetUlong(jr, JR_general_failure)==GFSTATE_JOB)) {
      char exitstr[256];
      const char *err_str;
      const char *action, *comment = "";

      if (failed==SSTATE_MIGRATE) {
         action = MSG_MAIL_ACTION_MIGR;
      } else if (failed==SSTATE_AGAIN) {
         action = MSG_MAIL_ACTION_RESCH;
      } else if (failed==SSTATE_APPERROR) {
         action = MSG_MAIL_ACTION_APPERROR;
      } else if (lGetUlong(jr, JR_general_failure)==GFSTATE_JOB) {
         action = MSG_MAIL_ACTION_ERR;
         comment = MSG_MAIL_ACTION_ERR_COMMENT;
      } else {
         action = MSG_MAIL_ACTION_ABORT;
      }

      if ((ep=lGetSubStr(jr, UA_name, "signal", JR_usage)))
         signo = (u_long32)lGetDouble(ep, UA_value);

      if (!(err_str=lGetString(jr, JR_err_str)))
         err_str = MSG_UNKNOWNREASON;

      DPRINTF(("MAIL VALID at ABORT\n"));
      sprintf(exitstr, "%d", exit_status);
      if (pe_task_id_str == NULL) {
         if (job_is_array(jep)) {
            snprintf(sge_mail_subj, sizeof(sge_mail_subj),
                    MSG_MAIL_SUBJECT_JA_TASK_STATE_UUSS,
                    sge_u32c(jobid),
                    sge_u32c(taskid),
                    lGetString(jep, JB_job_name),
                    action);
            snprintf(sge_mail_body, sizeof(sge_mail_body),
                    MSG_MAIL_BODY_STATE_SSSSSSSSSSSSS,
                    sge_mail_subj, 
                    exitstr, 
                    sge_sig2str(signo),
                    u, q, h, sge_mail_start, sge_mail_end,
                    (ru_cpu     == 0.0) ? "NA":sge_dstring_get_string(&cpu_string),
                    (ru_maxvmem == 0.0) ? "NA":sge_dstring_get_string(&maxvmem_string),
                    get_sstate_description(failed), 
                    err_str, 
                    comment);
         } else {
            snprintf(sge_mail_subj, sizeof(sge_mail_subj),
                    MSG_MAIL_SUBJECT_JOB_STATE_USS,
                    sge_u32c(jobid),
                    lGetString(jep, JB_job_name),
                    action);
            snprintf(sge_mail_body, sizeof(sge_mail_body),
                    MSG_MAIL_BODY_STATE_SSSSSSSSSSSSS,
                    sge_mail_subj,
                    exitstr, 
                    sge_sig2str(signo),
                    u, q, h, sge_mail_start, sge_mail_end,
                    (ru_cpu     == 0.0) ? "NA":sge_dstring_get_string(&cpu_string),
                    (ru_maxvmem == 0.0) ? "NA":sge_dstring_get_string(&maxvmem_string),
                    get_sstate_description(failed), 
                    err_str, 
                    comment);
         }
         cull_mail(EXECD, mail_users, sge_mail_subj, 
                   sge_mail_body, MSG_MAIL_TYPE_STATE);
      }
   }

   sge_dstring_free(&cpu_string);
   sge_dstring_free(&maxvmem_string);
   DEXIT;
   return ;
}

/****** reaper_execd/execd_slave_job_exit() ************************************
*  NAME
*     execd_slave_job_exit() -- make pe slave job report exit
*
*  SYNOPSIS
*     void execd_slave_job_exit(u_long32 job_id, u_long32 ja_task_id) 
*
*  FUNCTION
*     When the master task of a tightly integrated pe job exited,
*     qmaster sends a request to all slave execds to report 
*     the exit of the slave job container once all pe tasks
*     have finished.
*     This function sets the slave job to status JEXITING,
*     if all pe tasks already exited, it triggers sending
*     of the final slave job report.
*
*  INPUTS
*     u_long32 job_id     - job id of the slave job
*     u_long32 ja_task_id - ja_task id of the slave job
*
*  NOTES
*     MT-NOTE: execd_slave_job_exit() is MT safe 
*******************************************************************************/
void execd_slave_job_exit(u_long32 job_id, u_long32 ja_task_id)
{
   lListElem *job = NULL;
   lListElem *ja_task = NULL;

   if (execd_get_job_ja_task(job_id, ja_task_id, &job, &ja_task)) {
      /* kill possibly still running tasks */
      signal_job(job_id, ja_task_id, SGE_SIGKILL);

      /* if there are no pe_tasks running
       * we can finish the slave job right away
       */
      if (lGetNumberOfElem(lGetList(ja_task, JAT_task_list)) == 0) {
         lListElem *jr = get_job_report(job_id, ja_task_id, NULL);
         /* Fixme: The commented line is part of the "JG-2009-01-02-0"
            change (which added the whole function).  It causes the
            exit status of tightly integrated jobs always to be 0.
            Why was it there?
            Replaced by a conditional version in case the status could
            be missing.  */
         /* add_usage(jr, "exit_status", NULL, 0); */
         maybe_add_usage(jr, "exit_status", NULL, 0);
         flush_job_report(jr);
      }
   }
}

/****** reaper_execd/clean_up_binding() ****************************************
*  NAME
*     clean_up_binding() -- Releases the resources used by a job for core binding. 
*
*  SYNOPSIS
*     static void clean_up_binding(char* binding) 
*
*  FUNCTION
*     Checks which cores the job was bound to and releases them (i.e. 
*     the as used marked cores where marked as free so that other jobs 
*     can be bound to any of these cores). 
*
*  INPUTS
*     char* binding - Pointer to the "binding" line out of the config file. 
*
*  RESULT
*     static void - void
*
*
*  NOTES
*     MT-NOTE: clean_up_binding() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static void clean_up_binding(char* binding)
{
   DENTER(TOP_LAYER, "clean_up_binding");

   if (binding == NULL || strcasecmp("NULL", binding) == 0 
      || strcasecmp("no_job_binding", binding) == 0) {
      /* no binding was instructed */
      DRETURN_VOID;
   }


   if (HAVE_HWLOC) {
      /* The topology used can be found just after the last ":" */
      /* -> find the topology used and release it */
   
      if (strstr(binding, ":") != NULL) {
         /* there was an order from execd to shepherd for binding */
         /* seach the string after the last ":" -> this
            is the topology used by the job */
         char* topo = NULL;
         topo = strrchr(binding, ':');
         free_topology(++topo, -1);
         DPRINTF((SGE_EVENT, "topology used by job freed"));
      } else {
         /* couldn't find valid topology string in config file */
         WARNING((SGE_EVENT, "No resource string found in config entry binding"));
      }
   }

   DRETURN_VOID;
}

/****** reaper_execd/count_master_tasks() ****************************************
*  NAME
*     count_master_tasks() -- Counts the number of master tasks present for a job
*
*  SYNOPSIS
*     int count_master_tasks(const lList *lp, u_long32 job_id)
*
*  FUNCTION
*     Counts the number of master tasks for a specific job that are present
*     on the execd.
*
*  INPUTS
*     const lList *lp - Pointer to the job list
*     u_long32 job_id - Job number to count master tasks for
*
*  RESULT
*     int             - Number of master tasks found
*
*  NOTES
*     MT-NOTE: count_master_tasks() is MT safe
*******************************************************************************/
int count_master_tasks(const lList *lp, u_long32 job_id)
{
   const void *iterator = NULL;
   int master_jobs = 0;
   const lListElem *jep;

   DENTER(TOP_LAYER, "count_master_tasks");

   jep = lGetElemUlongFirst(lp, JB_job_number, job_id, &iterator);
   while (jep != NULL) {
      lListElem *jatep = lFirst(lGetList(jep, JB_ja_tasks));
      while (jatep != NULL) {
         if (lGetUlong(jatep, JAT_status) != JSLAVE) {
            master_jobs++;
         }
         jatep = lNext(jatep);
      }
      jep = lGetElemUlongNext(lp, JB_job_number, job_id, &iterator);
   }

   DPRINTF(("Found %d master jobs for "sge_u32, master_jobs, job_id));

   DRETURN(master_jobs);
}
