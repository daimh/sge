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

#if defined(ALPHA) || defined(SOLARIS)
#  include <sys/param.h>        /* for MAX() macro */
#endif

#include <sys/types.h>

#if defined(COMPILE_DC) || defined(MODULE_TEST)

#if defined(IRIX) || defined(ALPHA) || defined(LINUX) || defined(SOLARIS) || !defined(MODULE_TEST) || defined(HP1164) || defined(HP1164) || defined(FREEBSD) || defined(DARWIN)
#   define USE_DC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

#ifdef __sgi
#  include <sys/resource.h>
#  include <sys/systeminfo.h>
#  include <sys/sched.h>
#  include <sys/sysmp.h>
#  include <sys/schedctl.h>
#endif

#if defined(ALPHA) || defined(SOLARIS) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)
#  include <sys/resource.h>
#endif

#ifdef MODULE_TEST
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <sys/signal.h>
#   if ( IRIX || ALPHA || SOLARIS )
#      include <sys/fault.h>
#   endif
#   include <sys/syscall.h>
#   include <sys/procfs.h>
#endif

#include "comm/commlib.h"

#include "uti/sge_stdio.h"
#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_language.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_usage.h"

#include "ptf.h"
#include "sge.h"
#include "basis_types.h"
#include "msg_common.h"
#include "msg_execd.h"
#include "sgedefs.h"
#include "exec_ifm.h"
#include "pdc.h"

/*
 *
 * PTF Data Structures
 *
 *    job_ticket_list     list of jobs and their associated tickets which is
 *                        sent from the SGE scheduler to the PTF.
 *
 *    job_ticket_entry    contains the job ID and job tickets for each job.
 *
 *    job_list            local list of jobs and all the associated attributes
 *
 *    job                 entry contained in the job list.
 *
 *
 * Notes:
 *
 *   Make sure JL_usage is always set to at least PTF_MIN_JOB_USAGE
 *
 *   Where do we create the usage list?  Probably in the routine that
 *   we get the usage from the Data Collector.
 *
 *   When does usage get reset to PTF_MIN_JOB_USAGE?  Possibly, upon
 *   receiving a new job tickets list.
 *
 *   When do we update the usage in the qmaster?  Possibly, after
 *   receiving a new job tickets list.
 *
 *   Make sure we don't delete a job from the job list until all the
 *   usage has been collected and reported back to the qmaster.
 *
 */

#define INCPTR(type, ptr, nbyte) ptr = (type *)((char *)ptr + nbyte)

#define INCJOBPTR(ptr, nbyte) INCPTR(struct psJob_s, ptr, nbyte)

#define INCPROCPTR(ptr, nbyte) INCPTR(struct psProc_s, ptr, nbyte)

static void ptf_calc_job_proportion_pass0(lListElem *job,
                                          u_long * sum_of_job_tickets,
                                          double *sum_of_last_usage);

static void ptf_calc_job_proportion_pass1(lListElem *job,
                                          u_long sum_of_job_tickets,
                                          double sum_of_last_usage,
                                          double *sum_proportion);

static void ptf_calc_job_proportion_pass2(lListElem *job,
                                          u_long sum_of_job_tickets,
                                          double sum_proportion,
                                          double *sum_adjusted_proportion,
                                          double *sum_last_interval_usage);

static void ptf_calc_job_proportion_pass3(lListElem *job,
                                          double sum_adjusted_proportion,
                                          double sum_last_interval_usage,
                                          double *min_share,
                                          double *max_share,
                                          double *max_ticket_share);

static void ptf_set_OS_scheduling_parameters(lList *job_list, double min_share,
                                             double max_share,
                                             double max_ticket_share);

static void ptf_get_usage_from_data_collector(void);

static lListElem *ptf_process_job(osjobid_t os_job_id,
                                  const char *task_id_str,
                                  lListElem *new_job, u_long32 jataskid);

static lListElem *ptf_get_job_os(lList *job_list, osjobid_t os_job_id,
                                 lListElem **job_elem);

static void ptf_set_job_priority(lListElem *job);

static lList *_ptf_get_job_usage(lListElem *job, u_long ja_task_id,
                                 const char *task_id);

static osjobid_t ptf_get_osjobid(lListElem *osjob);


static void ptf_set_osjobid(lListElem *osjob, osjobid_t osjobid);

static void ptf_set_native_job_priority(lListElem *job, lListElem *osjob,
                                        long pri);

static lList *ptf_build_usage_list(char *name, lList *old_usage_list);

static lListElem *ptf_get_job(u_long job_id);

static void ptf_unregister_registered_jobs(void);

#if defined(__sgi)

static void ptf_setpriority_ash(lListElem *job, lListElem *osjob, long pri);

#elif defined(ALPHA) || defined(SOLARIS) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)

static void ptf_setpriority_addgrpid(lListElem *job, lListElem *osjob,
                                     long pri);

#endif

static lList *ptf_jobs = NULL;

static int is_ptf_running = 0;

#if defined(__sgi)

static char irix_release[257] = "6.2";

static int got_release = 0;

#endif

/****** execd/ptf/ptf_get_osjobid() *******************************************
*  NAME
*     ptf_get_osjobid() -- return the job id 
*
*  SYNOPSIS
*     static osjobid_t ptf_get_osjobid(lListElem *osjob) 
*
*  FUNCTION
*     The function returns the os job id contained in the CULL element
*
*  INPUTS
*     lListElem *osjob - element of type JO_Type 
*
*  RESULT
*     static osjobid_t - os job id (job id / ash / supplementary gid)
******************************************************************************/
static osjobid_t ptf_get_osjobid(lListElem *osjob)
{
   osjobid_t osjobid;

#if !defined(LINUX) && !defined(SOLARIS) && !defined(ALPHA5) && !defined(DARWIN) && !defined(FREEBSD) && !defined(NETBSD) && !defined(INTERIX) && !defined(HP1164) && !defined(AIX)

   osjobid = lGetUlong(osjob, JO_OS_job_ID2);
   osjobid = (osjobid << 32) + lGetUlong(osjob, JO_OS_job_ID);

#else

   osjobid = lGetUlong(osjob, JO_OS_job_ID);

#endif

   return osjobid;
}

/****** execd/ptf/ptf_set_osjobid() *******************************************
*  NAME
*     ptf_set_osjobid() -- set os job id 
*
*  SYNOPSIS
*     static void ptf_set_osjobid(lListElem *osjob, osjobid_t osjobid) 
*
*  FUNCTION
*     Set the attribute of "osjob" containing the os job id
*
*  INPUTS
*     lListElem *osjob  - element of type JO_Type 
*     osjobid_t osjobid - os job id (job id / ash / supplementary gid) 
******************************************************************************/
static void ptf_set_osjobid(lListElem *osjob, osjobid_t osjobid)
{
#if !defined(LINUX) && !defined(SOLARIS) && !defined(ALPHA5) && !defined(DARWIN) && !defined(FREEBSD) && !defined(NETBSD) && !defined(INTERIX) && !defined(HP1164) && !defined(AIX)

   lSetUlong(osjob, JO_OS_job_ID2, ((u_osjobid_t) osjobid) >> 32);
   lSetUlong(osjob, JO_OS_job_ID, osjobid & 0xffffffff);

#else

   lSetUlong(osjob, JO_OS_job_ID, osjobid);

#endif
}

/****** execd/ptf/ptf_build_usage_list() **************************************
*  NAME
*     ptf_build_usage_list() -- create a new usage list from an existing list 
*
*  SYNOPSIS
*     static lList* ptf_build_usage_list(char *name, lList *old_usage_list) 
*
*  FUNCTION
*     This method creates a new usage list or makes a copy of a old
*     usage list and zeros out the usage values. 
*
*  INPUTS
*     char *name            - name of the new list 
*     lList *old_usage_list - old usage list or NULL 
*
*  RESULT
*     static lList* - copy of "old_usage_list" or a real new one 
******************************************************************************/
static lList *ptf_build_usage_list(char *name, lList *old_usage_list)
{
   lList *usage_list;
   lListElem *usage;

   DENTER(TOP_LAYER, "ptf_build_usage_list");

   if (old_usage_list) {
      usage_list = lCopyList(name, old_usage_list);
      for_each(usage, usage_list) {
         lSetDouble(usage, UA_value, 0);
      }
   } else {
      usage_list = lCreateList(name, UA_Type);

      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_IO);
      lSetDouble(usage, UA_value, 0);
      lAppendElem(usage_list, usage);

      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_IOW);
      lSetDouble(usage, UA_value, 0);
      lAppendElem(usage_list, usage);

      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_MEM);
      lSetDouble(usage, UA_value, 0);
      lAppendElem(usage_list, usage);

      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_CPU);
      lSetDouble(usage, UA_value, 0);
      lAppendElem(usage_list, usage);

#if defined(ALPHA) || defined(LINUX) || defined(SOLARIS) || defined(HP1164) || defined(AIX) || defined(FREEBSD) || defined(DARWIN)
      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_VMEM);
      lSetDouble(usage, UA_value, 0);
      DPRINTF(("adding usage attribute %s\n", USAGE_ATTR_VMEM));
      lAppendElem(usage_list, usage);

      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, USAGE_ATTR_MAXVMEM);
      lSetDouble(usage, UA_value, 0);
      DPRINTF(("adding usage attribute %s\n", USAGE_ATTR_MAXVMEM));
      lAppendElem(usage_list, usage);
#endif
   }

   DEXIT;
   return usage_list;
}

/****** execd/ptf/ptf_reinit_queue_priority() *********************************
*  NAME
*     ptf_reinit_queue_priority() -- set static priority 
*
*  SYNOPSIS
*     void ptf_reinit_queue_priority(u_long32 job_id, u_long32 ja_task_id, 
*                                    char *pe_task_id_str, u_long32 priority) 
*
*  FUNCTION
*     If execd gets a new configuration this function is used to
*     reinitialize static priorities of all jobs currently running.
*
*  INPUTS
*     u_long32 job_id      - job id 
*     u_long32 ja_task_id  - task number 
*     char *pe_task_id_str - pe task id string or NULL 
*     u_long32 priority    - new static priority 
******************************************************************************/
void ptf_reinit_queue_priority(u_long32 job_id, u_long32 ja_task_id,
                               const char *pe_task_id_str, int priority)
{
   lListElem *job_elem;
   DENTER(TOP_LAYER, "ptf_reinit_queue_priority");

   if (!job_id || !ja_task_id) {
      DEXIT;
      return;
   }

   for_each(job_elem, ptf_jobs) {
      lList *os_job_list;
      lListElem *os_job;

      if (lGetUlong(job_elem, JL_job_ID) == job_id) {
         DPRINTF(("\tjob id: " sge_u32 "\n", lGetUlong(job_elem, JL_job_ID)));
         os_job_list = lGetList(job_elem, JL_OS_job_list);
         for_each(os_job, os_job_list) {
            if (lGetUlong(os_job, JO_ja_task_ID) == ja_task_id &&
                ((!pe_task_id_str && !lGetString(os_job, JO_task_id_str))
                 || (pe_task_id_str && lGetString(os_job, JO_task_id_str) &&
                     !strcmp(pe_task_id_str,
                             lGetString(os_job, JO_task_id_str))))) {

               DPRINTF(("\t\tChanging priority for osjobid: " sge_u32 " jatask "
                        sge_u32 " petask %s\n",
                        lGetUlong(os_job, JO_OS_job_ID),
                        lGetUlong(os_job, JO_ja_task_ID),
                        pe_task_id_str ? pe_task_id_str : ""));

               ptf_set_native_job_priority(job_elem, os_job,
                                           PTF_PRIORITY_TO_NATIVE_PRIORITY
                                           (priority));
            }
         }
      }
   }
   DEXIT;
}

/****** execd/ptf/ptf_set_job_priority() **************************************
*  NAME
*     ptf_set_job_priority() -- Update job priority 
*
*  SYNOPSIS
*     static void ptf_set_job_priority(lListElem *job) 
*
*  FUNCTION
*     The funktion updates the priority of each process which belongs
*     to "job". The attribute JL_pri of "job" specifies the new priority.
*
*  INPUTS
*     lListElem *job - JL_Type 
******************************************************************************/
static void ptf_set_job_priority(lListElem *job)
{
   lListElem *osjob;
   long pri = lGetLong(job, JL_pri);

   DENTER(TOP_LAYER, "ptf_set_job_priority");

   for_each(osjob, lGetList(job, JL_OS_job_list)) {
      ptf_set_native_job_priority(job, osjob, pri);
   }
   DEXIT;
}

/****** execd/ptf/ptf_set_native_job_priority() *******************************
*  NAME
*     ptf_set_native_job_priority() -- Change job priority  
*
*  SYNOPSIS
*     static void ptf_set_native_job_priority(lListElem *job, lListElem *osjob, 
*                                             long pri) 
*
*  FUNCTION
*     The function updates the priority of each process which belongs
*     to "job" and "osjob".
*
*  INPUTS
*     lListElem *job   - job 
*     lListElem *osjob - one of the os jobs of "job" 
*     long pri         - new priority value 
******************************************************************************/
static void ptf_set_native_job_priority(lListElem *job, lListElem *osjob, 
                                        long pri)
{
#if defined(__sgi)
   ptf_setpriority_ash(job, osjob, pri);
#elif defined(ALPHA) || defined(SOLARIS) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)
   ptf_setpriority_addgrpid(job, osjob, pri);
#endif
}

#if defined(__sgi)

/****** execd/ptf/ptf_setpriority_ash() ***************************************
*  NAME
*     ptf_setpriority_ash() -- Change priority of processes 
*
*  SYNOPSIS
*     static void ptf_setpriority_ash(lListElem *job, lListElem *osjob, 
*        long *pri) 
*
*  FUNCTION
*     This function is only available for the architecture IRIX.
*     All processes belonging to "job" and "osjob" will get a new priority.
*
*  INPUTS
*     lListElem *job   - job  
*     lListElem *osjob - one of the os jobs of "job" 
*     long pri         - new priority 
******************************************************************************/
static void ptf_setpriority_ash(lListElem *job, lListElem *osjob, long pri)
{
   static int got_bg_flag = 0;
   static int bg_flag;
   lListElem *pid;

#  if defined(IRIX)
   int nprocs;
   static int first = 1;
#  endif

   DENTER(TOP_LAYER, "ptf_setpriority_ash");

#  if defined(IRIX)
   if (first) {
      nprocs = sysmp(MP_NPROCS);
      if (nprocs <= 0)
         nprocs = 1;
      first = 0;
   }
#  endif

   if (!got_bg_flag) {
      bg_flag = (getenv("PTF_NO_BACKGROUND_PRI") == NULL);
      got_bg_flag = 1;
   }
#  if defined(__sgi)

   if (!got_release) {
      if (sysinfo(SI_RELEASE, irix_release, sizeof(irix_release)) < 0) {
         ERROR((SGE_EVENT, MSG_SYSTEM_SYSINFO_SI_RELEASE_CALL_FAILED_S,
                strerror(errno)));
      }
      got_release = 1;
   }
#  endif
#  if defined(__sgi)
   for_each(pid, lGetList(osjob, JO_pid_list)) {
#     ifdef PTF_NICE_BASED
      /* IRIX 6.4 also has some scheduler bugs.  Namely, when you use
       * setpriority and assign a "weightless" priority of 20, setting
       * a new priority doesn't bring the process out of the "weightless"
       * class.  The only way to bring the process out is to force a
       * reset of its scheduling class using sched_setscheduler. 
       */
      if (bg_flag && lGetUlong(job, JL_interactive) == 0 &&
          lGetDouble(job, JL_adjusted_current_proportion) <
          (PTF_BACKGROUND_JOB_PROPORTION / (double) nprocs)) {

         /* set the background "weightless" priority */

         if (setpriority(PRIO_PROCESS, (id_t)lGetUlong(pid, JP_pid),
                         PTF_BACKGROUND_NICE_VALUE) < 0 && errno != ESRCH) {

            ERROR((SGE_EVENT, MSG_PRIO_JOBXPIDYSETPRIORITYFAILURE_UUS,
                   sge_u32c(lGetUlong(job, JL_job_ID)),
                   sge_u32c(lGetUlong(pid, JP_pid)), strerror(errno)));
         }

         lSetUlong(pid, JP_background, 1);

      } else {

         struct sched_param sp;

         sp.sched_priority = pri;

         if (sched_setscheduler((id_t)lGetUlong(pid, JP_pid), SCHED_TS, &sp) < 0 
             && errno != ESRCH) {
            ERROR((SGE_EVENT, MSG_SCHEDD_JOBXPIDYSCHEDSETSCHEDULERFAILURE_UUS,
                   sge_u32c(lGetUlong(job, JL_job_ID)),
                   sge_u32c(lGetUlong(pid, JP_pid)), strerror(errno)));
         }

         lSetUlong(pid, JP_background, 0);
      }
#     endif
#     ifdef PTF_NDPRI_BASED
      if (schedctl(NDPRI, lGetUlong(pid, JP_pid), pri) < 0 && errno != ESRCH) {
         ERROR((SGE_EVENT, MSG_SCHEDD_JOBXPIDYSCHEDCTLFAILUREX_UUS,
                sge_u32c(lGetUlong(job, JL_job_ID)), sge_u32c(lGetUlong(pid, JP_pid)),
                strerror(errno)));
      }
#     endif
   }
#  endif
   DEXIT;
}

#elif defined(ALPHA) || defined(SOLARIS) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)

/****** execd/ptf/ptf_setpriority_addgrpid() **********************************
*  NAME
*     ptf_setpriority_addgrpid() -- Change priority of processes
*
*  SYNOPSIS
*     static void ptf_setpriority_jobid(lListElem *job, lListElem *osjob,
*                                       long *pri)
*
*  FUNCTION
*     This function is only available for the architecture SOLARIS, ALPHA,
*     LINUX, DARWIN and FREEBSD. All processes belonging to "job" and "osjob" will
*     get a new priority.
*
*     This function assumes the the "max" priority is smaller than the "min"
*     priority.
*
*  INPUTS
*     lListElem *job   - job
*     lListElem *osjob - one of the os jobs of "job"
*     long pri         - new priority
******************************************************************************/
static void ptf_setpriority_addgrpid(lListElem *job, lListElem *osjob, 
                                     long pri)
{
   lListElem *pid;

   DENTER(TOP_LAYER, "ptf_setpriority_addgrpid");

#  ifdef USE_ALPHA_PGRPS

   /*
    * set the priority for the entire process group
    */
   if (setpriority(PRIO_PGRP, ptf_get_osjobid(osjob), pri) < 0 &&
       errno != ESRCH) {
      ERROR((SGE_EVENT, MSG_PRIO_JOBXSETPRIORITYFAILURE_DS,
             sge_u32c(lGetUlong(job, JL_job_ID)), strerror(errno)));
   }
   DEXIT;
#  else

   /*
    * set the priority for each active process
    */
   for_each(pid, lGetList(osjob, JO_pid_list)) {
      if (setpriority(PRIO_PROCESS, lGetUlong(pid, JP_pid), pri) < 0 &&
          errno != ESRCH) {
         ERROR((SGE_EVENT, MSG_PRIO_JOBXPIDYSETPRIORITYFAILURE_UUS,
                sge_u32c(lGetUlong(job, JL_job_ID)), sge_u32c(lGetUlong(pid, JP_pid)),
                strerror(errno)));
      } else {
         DPRINTF(("Changing Priority of process " sge_u32 " to " sge_u32 "\n",
                  sge_u32c(lGetUlong(pid, JP_pid)), sge_u32c((u_long32) pri)));
      }
   }
   DEXIT;
#  endif
}

#endif

/****** execd/ptf/ptf_get_job() ***********************************************
*  NAME
*     ptf_get_job() -- look up the job for the supplied job_id and return it 
*
*  SYNOPSIS
*     static lListElem* ptf_get_job(u_long job_id) 
*
*  FUNCTION
*     look up the job for the supplied job_id and return it 
*
*  INPUTS
*     u_long job_id - SGE job id 
*
*  RESULT
*     static lListElem* - JL_Type
******************************************************************************/
static lListElem *ptf_get_job(u_long job_id)
{
   lListElem *job;
   lCondition *where;

   where = lWhere("%T(%I == %u)", JL_Type, JL_job_ID, job_id);
   job = lFindFirst(ptf_jobs, where);
   lFreeWhere(&where);
   return job;
}

/****** execd/ptf/ptf_get_job_os() ********************************************
*  NAME
*     ptf_get_job_os() -- look up the job for the supplied OS job_id 
*
*  SYNOPSIS
*     static lListElem* ptf_get_job_os(lList *job_list, 
*                                      osjobid_t os_job_id, 
*                                      lListElem **job_elem) 
*
*  FUNCTION
*     This functions tries to find a os job element (JO_Type) with
*     "os_job_id" within the list of os jobs of "job_elem". If "job_elem"
*     is not provided the function will look up the whole "job_list".
*
*  INPUTS
*     lList *job_list      - List of all known jobs (JL_Type) 
*     osjobid_t os_job_id  - os job id (ash, job id, supplementary gid) 
*     lListElem **job_elem - Pointer to a job element pointer (JL_Type)
*                          - pointer to a NULL pointer
*                            => *job_elem will contain the corresponding
*                               job element pointer when the function 
*                               returns successfully
*                          - NULL 
*
*  RESULT
*     static lListElem* - osjob (JO_Type) 
*                         or NULL if it was not found.
******************************************************************************/
static lListElem *ptf_get_job_os(lList *job_list, osjobid_t os_job_id, 
                                 lListElem **job_elem)
{
   lListElem *job, *osjob = NULL;
   lCondition *where;

   DENTER(TOP_LAYER, "ptf_get_job_os");

#if defined(LINUX) || defined(SOLARIS) || defined(ALPHA5) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(INTERIX) || defined(HP1164) || defined(AIX)
   where = lWhere("%T(%I == %u)", JO_Type, JO_OS_job_ID, (u_long32) os_job_id);
#else
   where = lWhere("%T(%I == %u && %I == %u)", JO_Type,
                  JO_OS_job_ID, (u_long) (os_job_id & 0xffffffff),
                  JO_OS_job_ID2, (u_long) (((u_osjobid_t) os_job_id) >> 32));
#endif

   if (!where) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_WHERE_FAILEDTOBUILDWHERECONDITION));
      DRETURN(NULL);
   }

   if (job_elem && (*job_elem)) {
      osjob = lFindFirst(lGetList(*job_elem, JL_OS_job_list), where);
   } else {
      for_each(job, job_list) {
         osjob = lFindFirst(lGetList(job, JL_OS_job_list), where);
         if (osjob) {
            break;
         }
      }
      if (job_elem) {
         *job_elem = osjob ? job : NULL;
      }
   }

   lFreeWhere(&where);
   DRETURN(osjob);
}


/*--------------------------------------------------------------------
 * ptf_process_job - process a job received from the SGE scheduler.
 * This assumes that new jobs can be received after a job ticket
 * list has been sent.  The new jobs will have an associated number
 * of tickets which will be updated in the job_list maintained by
 * the PTF.
 *--------------------------------------------------------------------*/

static lListElem *ptf_process_job(osjobid_t os_job_id, const char *task_id_str,
                                  lListElem *new_job, u_long32 jataskid)
{
   lListElem *job, *osjob;
   lList *job_list = ptf_jobs;
   u_long job_id = lGetUlong(new_job, JB_job_number);
   double job_tickets =
      lGetDouble(lFirst(lGetList(new_job, JB_ja_tasks)), JAT_tix);
   u_long interactive = (lGetString(new_job, JB_script_file) == NULL);

   DENTER(TOP_LAYER, "ptf_process_job");

   /*
    * Add the job to the job list, if it does not already exist
    */

/*
 * cases:
 *
 * job == NULL && osjobid == 0
 *    return
 *    job == NULL && osjobid > 0
 *    add osjob job && osjobid > 0 search by osjobid if osjob
 *    found skip
 *    else
 *    add osjob job && osjobid == 0 skip
 */
   job = ptf_get_job(job_id);
   if (os_job_id == 0) {
      if (job == NULL) {
         DEXIT;
         return NULL;
      }
   } else {
      lList *osjoblist;

      if (job == NULL) {
         job = lCreateElem(JL_Type);
         lAppendElem(job_list, job);
         lSetUlong(job, JL_job_ID, job_id);
      }
      osjoblist = lGetList(job, JL_OS_job_list);
      osjob = ptf_get_job_os(osjoblist, os_job_id, &job);
      if (!osjob) {
         if (!osjoblist) {
            osjoblist = lCreateList("osjoblist", JO_Type);
            lSetList(job, JL_OS_job_list, osjoblist);
         }
         osjob = lCreateElem(JO_Type);
         lSetUlong(osjob, JO_ja_task_ID, jataskid);
         lAppendElem(osjoblist, osjob);
         lSetList(osjob, JO_usage_list,
                  ptf_build_usage_list("usagelist", NULL));
         ptf_set_osjobid(osjob, os_job_id);
      }
      if (task_id_str) {
         lSetString(osjob, JO_task_id_str, task_id_str);
      }
   }

   /*
    * set number of tickets in job entry
    */
   lSetUlong(job, JL_tickets, (u_long32)MAX(job_tickets, 1));

   /*
    * set interactive job flag
    */
   if (interactive) {
      lSetUlong(job, JL_interactive, 1);
   }

   DEXIT;
   return job;
}

/****** execd/ptf/ptf_get_usage_from_data_collector() *************************
*  NAME
*     ptf_get_usage_from_data_collector() -- get usage from PDC 
*
*  SYNOPSIS
*     static void ptf_get_usage_from_data_collector(void) 
*
*  FUNCTION
*     get the usage for all the jobs in the job ticket list and update 
*     the job list 
*
*     call data collector routine with a list of all the jobs in the job_list
*     for each job in the job_list
*        update job.usage with data collector info
*        update list of process IDs associated with job end     
*     do
******************************************************************************/
static void ptf_get_usage_from_data_collector(void)
{
#ifdef USE_DC

   lListElem *job, *osjob;
   lList *pidlist, *oldpidlist;
   struct psJob_s *jobs, *ojobs, *tmp_jobs;
   struct psProc_s *procs;
   uint64 jobcount;
   int proccount;
   const char *tid;
   int i, j;

   DENTER(TOP_LAYER, "ptf_get_usage_from_data_collector");

   ojobs = jobs = psGetAllJobs();
   if (jobs) {
      jobcount = *(uint64 *) jobs;

#ifndef SOLARIS
      INCJOBPTR(jobs, sizeof(uint64));
#else
      INCJOBPTR(jobs, 8);
#endif

      for (i = 0; i < jobcount; i++) {
         lList *usage_list;
         lListElem *usage;
         double cpu_usage_value;

         /* look up job in job list */

         job = NULL;
         osjob = ptf_get_job_os(ptf_jobs, jobs->jd_jid, &job);

         if (osjob) {
            u_long job_state = lGetUlong(osjob, JO_state);

            tmp_jobs = jobs;

            /* fill in job completion state */
            lSetUlong(osjob, JO_state, jobs->jd_refcnt ?
                      (job_state & ~JL_JOB_COMPLETE) : (job_state |
                                                        JL_JOB_COMPLETE));

            /* fill in usage for job */
            usage_list = lGetList(osjob, JO_usage_list);
            if (!usage_list) {
               usage_list = ptf_build_usage_list("usagelist", NULL);
               lSetList(osjob, JO_usage_list, usage_list);
            }

            /* set CPU usage */
            cpu_usage_value = jobs->jd_utime_c + jobs->jd_utime_a +
               jobs->jd_stime_c + jobs->jd_stime_a;
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_CPU))) {
               lSetDouble(usage, UA_value, 
                          MAX(cpu_usage_value, lGetDouble(usage, UA_value)));
            }

            /* set mem usage (in GB seconds) */
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_MEM))) {
               lSetDouble(usage, UA_value, (double) jobs->jd_mem / 1048576.0);
            }

            /* set I/O usage (in GB) */
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_IO))) {
               lSetDouble(usage, UA_value,
                          (double) jobs->jd_chars / 1073741824.0);
            }

            /* set I/O wait time */
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_IOW))) {
               lSetDouble(usage, UA_value,
                          (double) jobs->jd_bwtime_c + jobs->jd_bwtime_a +
                          jobs->jd_rwtime_c + jobs->jd_rwtime_a);
            }

            /* set vmem usage */
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_VMEM))) {
               lSetDouble(usage, UA_value, jobs->jd_vmem);
            }

            /* set himem usage */
            if ((usage = lGetElemStr(usage_list, UA_name, USAGE_ATTR_MAXVMEM))) {
               lSetDouble(usage, UA_value, jobs->jd_himem);
            }

            /* build new pid list */
            proccount = jobs->jd_proccount;
            INCJOBPTR(jobs, jobs->jd_length);

            if (proccount > 0) {
               oldpidlist = lGetList(osjob, JO_pid_list);
               pidlist = lCreateList("pidlist", JP_Type);

               procs = (struct psProc_s *) jobs;
               for (j = 0; j < proccount; j++) {
                  lListElem *pid;

                  if (procs->pd_state == 1) {
                     if ((pid = lGetElemUlong(oldpidlist, JP_pid,
                                              procs->pd_pid))) {
                        lAppendElem(pidlist, lCopyElem(pid));
                     } else {
                        pid = lCreateElem(JP_Type);

                        lSetUlong(pid, JP_pid, procs->pd_pid);
                        lAppendElem(pidlist, pid);
                     }
                  }
                  INCPROCPTR(procs, procs->pd_length);
               }

               jobs = (struct psJob_s *)procs;
               lSetList(osjob, JO_pid_list, pidlist);
            } else {
               lSetList(osjob, JO_pid_list, NULL);
            }

            tid = lGetString(osjob, JO_task_id_str);
            DPRINTF(("JOB " sge_u32 "." sge_u32 ": %s: (cpu = %8.3lf / mem = "
                     UINT64_FMT " / io = " UINT64_FMT " / vmem = %10.5f  / himem = %10.5f)\n",
                     lGetUlong(job, JL_job_ID), 
                     lGetUlong(osjob, JO_ja_task_ID), tid ? tid : "",
                     tmp_jobs->jd_utime_c + tmp_jobs->jd_utime_a +
                     tmp_jobs->jd_stime_c + tmp_jobs->jd_stime_a,
                     tmp_jobs->jd_mem, tmp_jobs->jd_chars,
                     (double) tmp_jobs->jd_vmem, (double) tmp_jobs->jd_himem));
         } else {
            /* 
             * NOTE: Under what conditions would DC have a job
             * that the PTF doesn't know about? 
             */
            psIgnoreJob(jobs->jd_jid);

            proccount = jobs->jd_proccount;
            INCJOBPTR(jobs, jobs->jd_length);
            procs = (struct psProc_s *) jobs;
            for (j = 0; j < proccount; j++) {
               INCPROCPTR(procs, procs->pd_length);
            }
            jobs = (struct psJob_s *) procs;
         }
      }
      sge_free(&ojobs);

      for_each(job, ptf_jobs) {
         double usage_value, old_usage_value;
         double cpu_usage_value = 0;
         int active_jobs = 0;

         for_each(osjob, lGetList(job, JL_OS_job_list)) {
            lListElem *usage;

            if ((usage = lGetElemStr(lGetList(osjob, JO_usage_list), 
                                     UA_name, USAGE_ATTR_CPU))) {
               cpu_usage_value += lGetDouble(usage, UA_value);
            }
            if (!(lGetUlong(osjob, JO_state) & JL_JOB_COMPLETE)) {
               active_jobs = 1;
            }
         }

         /* calculate JL_usage */
         usage_value = cpu_usage_value;
         old_usage_value = lGetDouble(job, JL_old_usage_value);
         lSetDouble(job, JL_old_usage_value, usage_value);
         lSetDouble(job, JL_usage, 
                    MAX(PTF_MIN_JOB_USAGE, lGetDouble(job, JL_usage) *
                    PTF_USAGE_DECAY_FACTOR + (usage_value - old_usage_value)));
         lSetDouble(job, JL_last_usage, usage_value - old_usage_value);

         /* set job state */
         if (!active_jobs) {
            lSetUlong(job, JL_state, lGetUlong(job, JL_state) 
                      & JL_JOB_COMPLETE);
         }
      }
   }

#else

# ifdef MODULE_TEST

   lListElem *job, *proc, *usage;
   lList *usage_list;
   lList *pid_list;
   int j;

   DENTER(TOP_LAYER, "ptf_get_usage_from_data_collector");

   j = 0;
   for_each(job, ptf_jobs) {

      lListElem *osjob = lFirst(lGetList(job, JL_OS_job_list));

      usage_list = lGetList(osjob, JO_usage_list);
      if (!usage_list) {
         usage_list = ptf_build_usage_list("usagelist", NULL);
         lSetList(osjob, JO_usage_list, usage_list);
      }

      /* add to usage */

      for_each(usage, usage_list) {
         double value;

         value = lGetDouble(usage, UA_value);
         value += drand48();
         lSetDouble(usage, UA_value, value);
      }

      /* calculate JL_usage */
      {
         prstatus_t pinfo;
         int procfd;

         procfd = lGetUlong(job, JL_procfd);
         if (procfd > 0) {

            double usage_value, old_usage_value;

            if (ioctl(procfd, PIOCSTATUS, &pinfo) < 0) {
               perror("ioctl on /proc");
            }

            usage_value =
               pinfo.pr_utime.tv_sec +
               pinfo.pr_utime.tv_nsec / 1000000000.0L +
               pinfo.pr_stime.tv_sec +
               pinfo.pr_stime.tv_nsec / 1000000000.0L +
               pinfo.pr_cutime.tv_sec +
               pinfo.pr_cutime.tv_nsec / 1000000000.0L +
               pinfo.pr_cstime.tv_sec +
               pinfo.pr_cstime.tv_nsec / 1000000000.0L;

            old_usage_value = lGetDouble(job, JL_old_usage_value);

            lSetDouble(job, JL_old_usage_value, usage_value);

            lSetDouble(job, JL_usage,
                       lGetDouble(job, JL_usage) *
                       PTF_USAGE_DECAY_FACTOR +
                       (usage_value - old_usage_value));

            lSetDouble(job, JL_last_usage, usage_value - old_usage_value);
         }
      }

      /* build a fake pid list with the OS job ID as the pid */

      pid_list = lGetList(osjob, JO_pid_list);
      if (!pid_list) {
         pid_list = lCreateList("pidlist", JP_Type);
         lSetList(osjob, JO_pid_list, pid_list);
         proc = lCreateElem(JP_Type);
         lSetUlong(proc, JP_pid, lGetUlong(osjob, JO_OS_job_ID));
         lAppendElem(pid_list, proc);
      }

      j++;
   }

# endif /* MODULE_TEST */

   DEXIT;

#endif /* USE_DC */

}

/*--------------------------------------------------------------------
 * ptf_calc_job_proporiton_pass0
 *--------------------------------------------------------------------*/

static void ptf_calc_job_proportion_pass0(lListElem *job,
                                          u_long *sum_of_job_tickets,
                                          double *sum_of_last_usage)
{
   *sum_of_job_tickets += lGetUlong(job, JL_tickets);
   *sum_of_last_usage += lGetDouble(job, JL_last_usage);
}


/*--------------------------------------------------------------------
 * ptf_calc_job_proportion_pass1
 *--------------------------------------------------------------------*/

static void ptf_calc_job_proportion_pass1(lListElem *job,
                                          u_long sum_of_job_tickets,
                                          double sum_of_last_usage,
                                          double *sum_proportion)
{
   double share, job_proportion;
   double u;

   share = ((double) lGetUlong(job, JL_tickets) / sum_of_job_tickets) *
      1000.0 + 1.0;
   lSetDouble(job, JL_share, share);
   lSetDouble(job, JL_ticket_share, (double) lGetUlong(job, JL_tickets) / sum_of_job_tickets);

   /*
    * Calculate each jobs proportion value based on the job's tickets
    * and recent usage:
    * job.proportion = (job.tickets/sum_of_job_tickets)^2 / job.usage
    */
   u = MAX(lGetDouble(job, JL_usage), PTF_MIN_JOB_USAGE);
   job_proportion = share * share / u;
   *sum_proportion += job_proportion;
   lSetDouble(job, JL_proportion, job_proportion);

}

/*--------------------------------------------------------------------
 * ptf_calc_job_proportion_pass2
 *--------------------------------------------------------------------*/

static void ptf_calc_job_proportion_pass2(lListElem *job,
                                          u_long sum_of_job_tickets,
                                          double sum_proportion, double
                                          *sum_adjusted_proportion, double
                                          *sum_last_interval_usage)
{
   double job_current_proportion = 0;
   double compensate_proportion;
   double job_targetted_proportion, job_adjusted_usage, job_adjusted_proportion, share;

   job_targetted_proportion = (double) lGetUlong(job, JL_tickets) /
                               sum_of_job_tickets;

   if (sum_proportion > 0) {
      job_current_proportion = lGetDouble(job, JL_proportion) / sum_proportion;
   }

   compensate_proportion = PTF_COMPENSATION_FACTOR * job_targetted_proportion;
   if (job_current_proportion > compensate_proportion) {
      job_adjusted_usage = lGetDouble(job, JL_usage) * 
          (job_current_proportion / compensate_proportion);
   } else {
      job_adjusted_usage = lGetDouble(job, JL_usage);
   }
   lSetDouble(job, JL_adjusted_usage, job_adjusted_usage);

   /*
    * Recalculate proportions based on adjusted job usage
    */
   share = lGetDouble(job, JL_share);
   job_adjusted_usage = MAX(job_adjusted_usage, PTF_MIN_JOB_USAGE);
   job_adjusted_proportion = share * share / job_adjusted_usage;
   *sum_adjusted_proportion += job_adjusted_proportion;
   lSetDouble(job, JL_adjusted_proportion, job_adjusted_proportion);
}

/*--------------------------------------------------------------------
 * ptf_calc_job_proportion_pass3
 *--------------------------------------------------------------------*/

static void ptf_calc_job_proportion_pass3(lListElem *job,
                                          double sum_adjusted_proportion,
                                          double sum_last_interval_usage,
                                          double *min_share, double *max_share,
                                          double *max_ticket_share)
{
   double job_adjusted_current_proportion = 0;

   if (sum_adjusted_proportion > 0)
      job_adjusted_current_proportion =
         lGetDouble(job, JL_adjusted_proportion) / sum_adjusted_proportion;

   lSetDouble(job, JL_last_proportion,
              lGetDouble(job, JL_adjusted_current_proportion));

   lSetDouble(job, JL_adjusted_current_proportion,
              job_adjusted_current_proportion);

   *max_share = MAX(*max_share, job_adjusted_current_proportion);
   *min_share = MIN(*min_share, job_adjusted_current_proportion);
   *max_ticket_share = MAX(*max_ticket_share, lGetDouble(job, JL_ticket_share));

#if 0
    DPRINTF(("XXXXXXX  minshare: %f, max_share: %f XXXXXXX\n", *min_share, 
             *max_share)); 
#endif
}

/*--------------------------------------------------------------------
 * ptf_set_OS_scheduling_parameters
 *
 * This function assumes the the "max" priority is smaller than the "min"
 * priority.
 *--------------------------------------------------------------------*/
static void ptf_set_OS_scheduling_parameters(lList *job_list, double min_share,
                                             double max_share,
                                             double max_ticket_share)
{
   lListElem *job;
   static long pri_range, pri_min = -999, pri_max = -999;
   long pri_min_tmp, pri_max_tmp;
   long pri;
   
   DENTER(TOP_LAYER, "ptf_set_OS_scheduling_parameters");

   
   pri_min_tmp = mconf_get_ptf_min_priority();
   if (pri_min_tmp == -999) {
      pri_min_tmp = PTF_MIN_PRIORITY;   
   }

   pri_max_tmp = mconf_get_ptf_max_priority();
   if (pri_max_tmp == -999) {
      pri_max_tmp = PTF_MAX_PRIORITY;   
   }

   /* 
    * For OS'es where we enforce the values of max and min priority verify
    * the the values are in the boundaries of PTF_OS_MAX_PRIORITY and
    * PTF_OS_MIN_PRIORITY.
    */
#if ENFORCE_PRI_RANGE
   pri_max_tmp = MAX(pri_max_tmp, PTF_OS_MAX_PRIORITY);
   pri_min_tmp = MIN(pri_min_tmp, PTF_OS_MIN_PRIORITY);
#endif 

   /*
    * Ensure that the max priority can't get a bigger value than
    * the min priority (otherwise the "range" gets wrongly calculated
    */         
   if (pri_max_tmp > pri_min_tmp) {
      pri_max_tmp = pri_min_tmp;
   }
      
   /* If the value has changed set pri_max/pri_min/pri_range and log */
   if (pri_max != pri_max_tmp || pri_min != pri_min_tmp) {
      u_long32 old_ll = log_state_get_log_level();
      
      pri_max = pri_max_tmp;
      pri_min = pri_min_tmp;
      pri_range = pri_min - pri_max;
   
      log_state_set_log_level(LOG_INFO);   
      INFO((SGE_EVENT, MSG_PRIO_PTFMINMAX_II, (int) pri_max, (int) pri_min));
      log_state_set_log_level(old_ll);   
   }

   /* Set the priority for each job */
   for_each(job, job_list) {

      pri = pri_max + (long)(pri_range * (1.0 -
                       lGetDouble(job, JL_adjusted_current_proportion)));

      /*
       * Note: Should calculate targetted proportion and if it is below
       * a certain % then set a background priority.  This is because
       * nice seems to always give at least some minimal % to all
       * processes.
       */

#ifdef notdef
      pri = pri_max + (pri_range * (lGetDouble(job, JL_curr_pri) / max_share));
#endif

      if (max_share > 0) {
         pri = pri_max + (long)(pri_range * ((lGetDouble(job, JL_curr_pri) 
                                              - min_share) / max_share));
      } else {
         pri = pri_min;
      }

      if (pri_min > 50 && pri_max > 50) {
         if (max_ticket_share > 0) {
  	         lSetDouble(job, JL_timeslice, 
                       MAX(pri_max, lGetDouble(job, JL_ticket_share) *
                                    pri_min / max_ticket_share));
         } else {
            lSetDouble(job, JL_timeslice, pri_min);
         }
      }
      lSetLong(job, JL_pri, pri);
      ptf_set_job_priority(job);
   }

   DEXIT;
}


/*--------------------------------------------------------------------
 * ptf_job_started - process new job
 *--------------------------------------------------------------------*/
int ptf_job_started(osjobid_t os_job_id, const char *task_id_str, 
                    lListElem *new_job, u_long32 jataskid)
{
   DENTER(TOP_LAYER, "ptf_job_started");

   /*
    * Add new job to job list
    */
   ptf_process_job(os_job_id, task_id_str, new_job, jataskid);

   /*
    * Tell data collector to start collecting data for this job
    */
#ifdef USE_DC
   if (os_job_id > 0) {
      psWatchJob(os_job_id);
   }
#else

# ifdef MODULE_TEST
   if (os_job_id > 0) {
      int fd;
      char fname[32];

      sprintf(fname, "/proc/%05d", os_job_id);

      fd = open(fname, O_RDONLY);
      if (fd < 0)
         fprintf(stderr, "opening of %s failed\n", fname);
      else
         lSetUlong(job, JL_procfd, fd);
   }
# endif

#endif

   DEXIT;
   return 0;
}

/*--------------------------------------------------------------------
 * ptf_job_complete - process completed job
 *--------------------------------------------------------------------*/

int ptf_job_complete(u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, lList **usage)
{
   lListElem *ptf_job, *osjob;
   lList *osjobs;

   DENTER(TOP_LAYER, "ptf_job_complete");

   ptf_job = ptf_get_job(job_id);

   if (ptf_job == NULL) {
      DRETURN(PTF_ERROR_JOB_NOT_FOUND);
   }

   osjobs = lGetList(ptf_job, JL_OS_job_list);

   /*
    * if job is not complete, go get latest job usage info
    */
   if (!(lGetUlong(ptf_job, JL_state) & JL_JOB_COMPLETE)) {
      sge_switch2start_user();
      ptf_get_usage_from_data_collector();
      sge_switch2admin_user();
   }

   /*
    * Get usage for completed job
    *    (If this for a sub-task then we return the sub-task usage)
    */
   *usage = _ptf_get_job_usage(ptf_job, ja_task_id, pe_task_id); 

   /* Search ja/pe ptf task */
   if (pe_task_id == NULL) {
      osjob = lFirst(osjobs);
   } else {
      for_each(osjob, osjobs) {
         if (lGetUlong(osjob, JO_ja_task_ID) == ja_task_id) {
            const char *osjob_pe_task_id = lGetString(osjob, JO_task_id_str);

            if (osjob_pe_task_id != NULL &&
                strcmp(pe_task_id, osjob_pe_task_id) == 0) {
               break;
            } 
         }
      }
   }

   if (osjob == NULL) {
      DRETURN(PTF_ERROR_JOB_NOT_FOUND);
   } 
   
   /*
    * Mark osjob as complete and see if all tracked osjobs are done
    * Tell data collector to stop collecting data for this job
    */
   lSetUlong(osjob, JO_state, lGetUlong(osjob, JO_state) | JL_JOB_DELETED);
#ifdef USE_DC
   psIgnoreJob(ptf_get_osjobid(osjob));
#endif

#ifndef USE_DC
# ifdef MODULE_TEST

   {
      int procfd = lGetUlong(ptf_job, JL_procfd);

      if (procfd > 0)
         close(procfd);
   }
# endif
#endif

   /*
    * Remove job/task from job/task list
    */

   DPRINTF(("PTF: Removing job " sge_u32 "." sge_u32 ", petask %s\n", 
            job_id, ja_task_id, pe_task_id == NULL ? "none" : pe_task_id));
   lRemoveElem(osjobs, &osjob);

   if (lGetNumberOfElem(osjobs) == 0) {
      DPRINTF(("PTF: Removing job\n"));
      lRemoveElem(ptf_jobs, &ptf_job);
   }

   DRETURN(0);
}


/*--------------------------------------------------------------------
 * ptf_process_job_ticket_list - Process the job ticket list sent
 * from the SGE scheduler.
 *--------------------------------------------------------------------*/

int ptf_process_job_ticket_list(lList *job_ticket_list)
{
   lListElem *jte, *job;

   DENTER(TOP_LAYER, "ptf_process_job_ticket_list");

    /*
     * Update the job entries in the job list with the number of
     * tickets from the job ticket list.  Reset the usage to the
     * minimum usage value.
     */
   for_each(jte, job_ticket_list) {

      /* 
       * set JB_script_file because we don't know if this is
       * an interactive job 
       */
      lSetString(jte, JB_script_file, "dummy");

      job = ptf_process_job(0, NULL, jte,
                            lGetUlong(lFirst(lGetList(jte, JB_ja_tasks)),
                            JAT_task_number));
      if (job) {
         /* reset temporary usage and priority */
         lSetDouble(job, JL_usage, MAX(PTF_MIN_JOB_USAGE,
                                       lGetDouble(job, JL_usage) * 0.1));

         lSetDouble(job, JL_curr_pri, 0);
      }
   }

   DEXIT;
   return 0;
}

void ptf_update_job_usage()
{
   DENTER(TOP_LAYER, "ptf_update_job_usage");

   ptf_get_usage_from_data_collector();
   DEXIT;
}


/*--------------------------------------------------------------------
 * ptf_adjust_job_priorities - routine to adjust the priorities of
 * executing jobs. Called whenever the PTF interval timer expires.
 *--------------------------------------------------------------------*/

int ptf_adjust_job_priorities(void)
{
   static u_long32 next = 0;
   u_long32 now;
   lList *job_list, *pid_list;
   lListElem *job, *osjob;
   u_long sum_of_job_tickets = 0;
   int num_procs = 0;
   double sum_proportion = 0;
   double sum_adjusted_proportion = 0;
   double min_share = 100.0;
   double max_share = 0;
   double max_ticket_share = 0;
   double sum_interval_usage = 0;
   double sum_of_last_usage = 0;

   DENTER(TOP_LAYER, "ptf_adjust_job_priorities");

   if ((now = sge_get_gmt()) < next) {
      DRETURN(0);
   }

   job_list = ptf_jobs;

   /*
    * Do pass 0 of calculating job proportional share of resources
    */
   for_each(job, job_list) {
      ptf_calc_job_proportion_pass0(job, &sum_of_job_tickets,
                                    &sum_of_last_usage);
   }
   if (sum_of_job_tickets == 0) {
      sum_of_job_tickets = 1;
   }

   /*
    * Do pass 1 of calculating job proportional share of resources
    */
   for_each(job, job_list) {
      ptf_calc_job_proportion_pass1(job, sum_of_job_tickets,
                                    sum_of_last_usage, &sum_proportion);
   }

   /*
    * Do pass 2 of calculating job proportional share of resources
    */
   for_each(job, job_list) {
      ptf_calc_job_proportion_pass2(job, sum_of_job_tickets,
                                    sum_proportion, 
                                    &sum_adjusted_proportion,
                                    &sum_interval_usage);
   }

   /*
    * Do pass 3 of calculating job proportional share of resources
    */
   for_each(job, job_list) {
      ptf_calc_job_proportion_pass3(job, sum_proportion,
                                    sum_interval_usage, &min_share,
                                    &max_share, &max_ticket_share);
   }

   max_share = 0;
   min_share = -1;

   for_each(job, job_list) {
      double shr;

      /*
       * calculate share based on tickets only
       */
      shr = lGetDouble(job, JL_share);

      num_procs = 0;
      for_each(osjob, lGetList(job, JL_OS_job_list)) {
         if ((pid_list = lGetList(osjob, JO_pid_list))) {
            num_procs += lGetNumberOfElem(pid_list);
         }
      }
      num_procs = MAX(1, num_procs);

      /* 
       * NOTE: share algo only adjusts priority when a process runs 
       */
      lSetDouble(job, JL_curr_pri, lGetDouble(job, JL_curr_pri) 
                 + ((lGetDouble(job, JL_adjusted_usage) * num_procs) 
                    / (shr * shr)));

      max_share = MAX(max_share, lGetDouble(job, JL_curr_pri));
      if (min_share < 0) {
         min_share = lGetDouble(job, JL_curr_pri);
      } else {
         min_share = MIN(min_share, lGetDouble(job, JL_curr_pri));
      } 
   }

   /*
    * Set the O.S. scheduling parameters for the jobs
    */
   ptf_set_OS_scheduling_parameters(job_list, min_share, max_share,
                                    max_ticket_share);
   
   next = now + PTF_SCHEDULE_TIME;

   DEXIT;
   return 0;
}

/*--------------------------------------------------------------------
 * ptf_get_job_usage - routine to return a single usage list for the
 * entire job.
 *--------------------------------------------------------------------*/
lList *ptf_get_job_usage(u_long job_id, u_long ja_task_id, 
                         const char *task_id)
{
   return _ptf_get_job_usage(ptf_get_job(job_id), ja_task_id, task_id);
}

static lList *_ptf_get_job_usage(lListElem *job, u_long ja_task_id,
                                 const char *task_id)
{
   lListElem *osjob, *usrc, *udst;
   lList *job_usage = NULL;
   const char *task_id_str;

   if (job == NULL) {
      return NULL;
   }

   for_each(osjob, lGetList(job, JL_OS_job_list)) {
      task_id_str = lGetString(osjob, JO_task_id_str);

      if ((((!task_id || !task_id[0]) && !task_id_str) 
           || (task_id && !strcmp(task_id, "*")) 
           || (task_id && task_id_str && !strcmp(task_id, task_id_str)))
          && (lGetUlong(osjob, JO_ja_task_ID) == ja_task_id)) {

         if (job_usage) {
            for_each(usrc, lGetList(osjob, JO_usage_list)) {
               if ((udst = lGetElemStr(job_usage, UA_name,
                                       lGetString(usrc, UA_name)))) {
                  lSetDouble(udst, UA_value, lGetDouble(udst, UA_value)
                                             + lGetDouble(usrc, UA_value));
               } else {
                  lAppendElem(job_usage, lCopyElem(usrc));
               }
            }
         } else {
            job_usage = lCopyList(NULL, lGetList(osjob, JO_usage_list));
         }
      }
   }
   return job_usage;
}

/*--------------------------------------------------------------------
 * ptf_get_usage - routine to return the current job usage for
 * all executing jobs.  Returns a job list with the job_ID and usage
 * filled in. A separate job entry is returned for each tracked
 * sub-task.
 *--------------------------------------------------------------------*/

int ptf_get_usage(lList **job_usage_list)
{
   lList *job_list, *temp_usage_list;
   lListElem *job, *osjob;
   lEnumeration *what;

   DENTER(TOP_LAYER, "ptf_get_usage");

   what = lWhat("%T(%I %I)", JB_Type, JB_job_number, JB_ja_tasks);

   temp_usage_list = lCreateList("PtfJobUsageList", JB_Type);
   job_list = ptf_jobs;
   for_each(job, job_list) {
      lListElem *tmp_job = NULL;
      u_long32 job_id = lGetUlong(job, JL_job_ID);

      for_each(osjob, lGetList(job, JL_OS_job_list)) {
         lListElem *tmp_ja_task;
         lListElem *tmp_pe_task;
         u_long32 ja_task_id = lGetUlong(osjob, JO_ja_task_ID);
         const char *pe_task_id = lGetString(osjob, JO_task_id_str);

         if (lGetUlong(osjob, JO_state) & JL_JOB_DELETED) {
            continue;
         }

         tmp_job = job_list_locate(temp_usage_list, job_id);
         if (tmp_job == NULL) {
            tmp_job = lCreateElem(JB_Type);
            lSetUlong(tmp_job, JB_job_number, job_id);
            lAppendElem(temp_usage_list, tmp_job);
         }

         tmp_ja_task = job_search_task(tmp_job, NULL, ja_task_id);
         if (tmp_ja_task == NULL) {
            tmp_ja_task = lAddSubUlong(tmp_job, JAT_task_number, ja_task_id, JB_ja_tasks, JAT_Type);
         }

         if (pe_task_id != NULL) {
            tmp_pe_task = ja_task_search_pe_task(tmp_ja_task, pe_task_id);
            if (tmp_pe_task == NULL) {
               tmp_pe_task = lAddSubStr(tmp_ja_task, PET_id, pe_task_id, JAT_task_list, PET_Type);
            }
            lSetList(tmp_pe_task, PET_usage, lCopyList(NULL, lGetList(osjob, JO_usage_list)));
         } else {
            lSetList(tmp_ja_task, JAT_usage_list, lCopyList(NULL, lGetList(osjob, JO_usage_list)));
         }
      } /* for each osjob */
   } /* for each ptf job */

   *job_usage_list = lSelect("PtfJobUsageList", temp_usage_list, NULL, what);

   lFreeList(&temp_usage_list);
   lFreeWhat(&what);

   DRETURN(0);
}


/*--------------------------------------------------------------------
 * ptf_init - initialize the Priority Translation Facility
 *--------------------------------------------------------------------*/

int ptf_init(void)
{
   DENTER(TOP_LAYER, "ptf_init");
   lInit(nmv);

   ptf_jobs = lCreateList("ptf_job_list", JL_Type);
   if (ptf_jobs == NULL) {
      DEXIT;
      return -1; 
   }

   sge_switch2start_user();
   if (psStartCollector()) {
      sge_switch2admin_user();
      DEXIT;
      return -1;
   }
#if defined(__sgi)
   schedctl(RENICE, 0, 0);
#elif defined(ALPHA) || defined(SOLARIS) || defined(LINUX) || defined(FREEBSD) || defined(DARWIN)
   if (getuid() == 0) {
      if (setpriority(PRIO_PROCESS, getpid(), PTF_MAX_PRIORITY) < 0) {
         ERROR((SGE_EVENT, MSG_PRIO_SETPRIOFAILED_S, strerror(errno)));
      }
   }
#endif
   sge_switch2admin_user();
   DEXIT;
   return 0;
}

void ptf_start(void)
{
   DENTER(TOP_LAYER, "ptf_start");
   if (!is_ptf_running) {
      ptf_init();
      is_ptf_running = 1;
   }
   DEXIT;
}

void ptf_stop(void)
{
   DENTER(TOP_LAYER, "ptf_stop");
   if (is_ptf_running) {
      ptf_unregister_registered_jobs();
      psStopCollector();
      is_ptf_running = 0;
   }

   lFreeList(&ptf_jobs);
   DEXIT;
}

void ptf_show_registered_jobs(void)
{
   lList *job_list;
   lListElem *job_elem;

   DENTER(TOP_LAYER, "ptf_show_registered_jobs");

   job_list = ptf_jobs;
   for_each(job_elem, job_list) {
      lList *os_job_list;
      lListElem *os_job;

      DPRINTF(("\tjob id: " sge_u32 "\n", lGetUlong(job_elem, JL_job_ID)));
      os_job_list = lGetList(job_elem, JL_OS_job_list);
      for_each(os_job, os_job_list) {
         lList *process_list;
         lListElem *process;
         const char *pe_task_id_str;
         u_long32 ja_task_id;

         pe_task_id_str = lGetString(os_job, JO_task_id_str);
         pe_task_id_str = pe_task_id_str ? pe_task_id_str : "<null>";
         ja_task_id = lGetUlong(os_job, JO_ja_task_ID);

         DPRINTF(("\t\tosjobid: "sge_u32" ja_task_id: "sge_u32" petaskid: %s\n",
                  lGetUlong(os_job, JO_OS_job_ID), ja_task_id,
                  pe_task_id_str));
         process_list = lGetList(os_job, JO_pid_list);
         for_each(process, process_list) {
            u_long32 pid;

            pid = lGetUlong(process, JP_pid);
            DPRINTF(("\t\t\tpid: " sge_u32 "\n", pid));
         }
      }
   }
   DEXIT;
}

void ptf_unregister_registered_job(u_long32 job_id, u_long32 ja_task_id ) {
  lListElem *job;
   lListElem *next_job;

   DENTER(TOP_LAYER, "ptf_unregister_registered_job");

   next_job = lFirst(ptf_jobs);
   while ((job = next_job)) {
      lList *os_job_list;
      lListElem *os_job;
      lListElem *next_os_job;

      next_job = lNext(job);

      if (lGetUlong(job, JL_job_ID) == job_id) {
         DPRINTF(("PTF: found job id "sge_U32CFormat"\n", job_id));
         os_job_list = lGetList(job, JL_OS_job_list);
         next_os_job = lFirst(os_job_list);
         while ((os_job = next_os_job)) {
            next_os_job = lNext(os_job);
            if (lGetUlong(os_job, JO_ja_task_ID ) == ja_task_id) {
               DPRINTF(("PTF: found job task id "sge_U32CFormat"\n", ja_task_id));
               psIgnoreJob(ptf_get_osjobid(os_job));
               DPRINTF(("PTF: Notify PDC to remove data for osjobid " sge_u32 "\n",
                        lGetUlong(os_job, JO_OS_job_ID)));
               lRemoveElem(os_job_list, &os_job);
            }
         }

         if (lFirst(os_job_list) == NULL) {
            DPRINTF(("PTF: No more os_job_list entries, removing job\n"));
            DPRINTF(("PTF: Removing job " sge_u32 "\n", lGetUlong(job, JL_job_ID)));
            lRemoveElem(ptf_jobs, &job);
         }
      }
   }
   DEXIT;
}

static void ptf_unregister_registered_jobs(void)
{
   lListElem *job;

   DENTER(TOP_LAYER, "ptf_unregister_registered_jobs");

   for_each(job, ptf_jobs) {
      lListElem *os_job;
      for_each(os_job, lGetList(job, JL_OS_job_list)) {
         psIgnoreJob(ptf_get_osjobid(os_job));
         DPRINTF(("PTF: Notify PDC to remove data for osjobid " sge_u32 "\n",
                  lGetUlong(os_job, JO_OS_job_ID)));
      }
   }

   lFreeList(&ptf_jobs);
   DPRINTF(("PTF: All jobs unregistered from PTF\n"));
   DEXIT;
}

int ptf_is_running(void)
{
   return is_ptf_running;
}

/*--------------------------------------------------------------------
 * ptf_errstr - return PTF error string
 *--------------------------------------------------------------------*/

const char *ptf_errstr(int ptf_error_code)
{
   const char *errmsg = MSG_ERROR_UNKNOWNERRORCODE;

   DENTER(TOP_LAYER, "ptf_errstr");

   switch (ptf_error_code) {
   case PTF_ERROR_NONE:
      errmsg = MSG_ERROR_NOERROROCCURED;
      break;

   case PTF_ERROR_INVALID_ARGUMENT:
      errmsg = MSG_ERROR_INVALIDARGUMENT;
      break;

   case PTF_ERROR_JOB_NOT_FOUND:
      errmsg = MSG_ERROR_JOBDOESNOTEXIST;
      break;

   default:
      break;
   }

   DRETURN(errmsg);
}

#ifdef MODULE_TEST

#define TESTJOB "./cpu_bound"

int main(int argc, char **argv)
{
   int i;
   FILE *f;
   lList *job_ticket_list, *job_usage_list;
   lListElem *jte, *job;
   u_long job_id = 0;
   osjobid_t os_job_id = 100;

#ifdef __SGE_COMPILE_WITH_GETTEXT__
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type) gettext,
                         (setlocale_func_type) setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type) textdomain);
   sge_init_language(NULL, NULL);
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */


#if defined(ALPHA) && defined(notdef)
   if (setsid() < 0) {
      perror("setsid");
   }
#endif

   ptf_init();

#ifdef USE_DC

   psStartCollector();

#endif

   /* build job ticket list */

   job_ticket_list = lCreateList("jobticketlist", JB_Type);

   if (argc < 2) {
      jte = lCreateElem(JB_Type);
      lSetUlong(jte, JB_job_number, ++job_id);
      lSetUlong(jte, JB_ticket, 100);
      lSetString(jte, JB_script_file, TESTJOB);
      lAppendElem(job_ticket_list, jte);

      jte = lCreateElem(JB_Type);
      lSetUlong(jte, JB_job_number, ++job_id);
      lSetUlong(jte, JB_ticket, 500);
      lSetString(jte, JB_script_file, TESTJOB);
      lAppendElem(job_ticket_list, jte);

      jte = lCreateElem(JB_Type);
      lSetUlong(jte, JB_job_number, ++job_id);
      lSetUlong(jte, JB_ticket, 300);
      lSetString(jte, JB_script_file, TESTJOB);
      lAppendElem(job_ticket_list, jte);

      jte = lCreateElem(JB_Type);
      lSetUlong(jte, JB_job_number, ++job_id);
      lSetUlong(jte, JB_ticket, 100);
      lSetString(jte, JB_script_file, TESTJOB);
      lAppendElem(job_ticket_list, jte);
   } else {
      int argn;

      for (argn = 1; argn < argc; argn++) {
         int tickets = atoi(argv[argn]);

         if (tickets <= 0)
            tickets = 1;
         jte = lCreateElem(JB_Type);
         lSetUlong(jte, JB_job_number, ++job_id);
         lSetUlong(jte, JB_ticket, tickets);
         lSetString(jte, JB_script_file, TESTJOB);
         lAppendElem(job_ticket_list, jte);
      }
   }

   /* Start jobs and tell PTF jobs have started */

   for_each(jte, job_ticket_list) {
      pid_t pid;

#if defined(IRIX)

      if (newarraysess() < 0) {
         perror("newarraysess");
         exit(1);
      }

      os_job_id = getash();
      printf(MSG_JOB_THEASHFORJOBXISY_DX,
             sge_u32c(lGetUlong(jte, JB_job_number)), u64c(os_job_id));

#endif

      pid = fork();
      if (pid == 0) {
         char *jobname = lGetString(jte, JB_script_file);

         /* schedctl(NDPRI, 0, 0); */
#if defined(ALPHA)
         if (setsid() < 0) {
            perror("setsid");
            exit(1);
         }
#endif
         execl(jobname, jobname, (char *) NULL);
         perror("exec");
         exit(1);
      } else {
#if defined(ALPHA)
         os_job_id = pid;
#endif
         ptf_job_started(os_job_id, jte, 0);
      }
   }

#if defined(IRIX)

   if (newarraysess() < 0) {
      perror("newarraysess");
      exit(2);
   }
   printf("My ash is "u64"\n", u64c(getash()));

#endif

   ptf_process_job_ticket_list(job_ticket_list);

   /* dump job_list */

   if (!(f = fdopen(1, "w"))) {
      fprintf(stderr, MSG_ERROR_COULDNOTOPENSTDOUTASFILE);
      exit(1);
   }

   if (lDumpList(f, ptf_jobs, 0) == EOF) {
      fprintf(stderr, MSG_ERROR_UNABLETODUMPJOBLIST);
      exit(2);
   }

   /* get job usage */

   ptf_get_usage(&job_usage_list);

   for (i = 0; i < 100; i++) {
      u_long sum_of_tickets = 0;
      double sum_of_usage = 0;
      double sum_of_last_usage = 0;

      /* adjust priorities */

      ptf_adjust_job_priorities();

      for_each(job, ptf_jobs) {
         sum_of_tickets += lGetUlong(job, JL_tickets);
         sum_of_usage += lGetDouble(job, JL_usage);
         sum_of_last_usage += lGetDouble(job, JL_last_usage);
      }

#       if defined(ALPHA)
#         define XFMT "%20d"
#       else
#         define XFMT "%20lld"
#       endif

      puts("                                           adj    total     curr"
           "                      last     next     prev");
      puts("job_id              os_job_id tickets    usage    usage    usage"
           "  target%  actual%  actual%  target%    diff% curr_pri   pri");
      for_each(job, ptf_jobs) {
         printf("%6d   " XFMT " %7d %8.3lf %8.3lf %8.3lf %8.3lf"
                " %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %5d\n",
                lGetUlong(job, JL_job_ID), ptf_get_osjobid(job),
                lGetUlong(job, JL_tickets), lGetDouble(job, JL_usage),
                lGetDouble(job, JL_adjusted_usage), 
                lGetDouble(job, JL_last_usage),
                sum_of_tickets ? lGetUlong(job, JL_tickets) /
                (double) sum_of_tickets : 0,
                sum_of_usage ? lGetDouble(job, JL_usage) / sum_of_usage : 0,
                sum_of_last_usage ? lGetDouble(job, JL_last_usage) /
                sum_of_last_usage : 0, 
                lGetDouble(job, JL_adjusted_current_proportion),
                lGetDouble(job, JL_diff_proportion), 
                lGetDouble(job, JL_curr_pri), lGetLong(job, JL_pri));
      }

#if 0
      /* 
       * dump job_list 
       */
      if (!(f = fdopen(1, "w"))) {
         fprintf(stderr, MSG_ERROR_COULDNOTOPENSTDOUTASFILE);
         exit(1);
      }

      if (lDumpList(f, ptf_jobs, 0) == EOF) {
         fprintf(stderr, MSG_ERROR_UNABLETODUMPJOBLIST);
         exit(2);
      }

      /* dump job usage list */

      if (lDumpList(f, job_usage_list, 0) == EOF) {
         fprintf(stderr, "%s\n", MSG_ERROR_UNABLETODUMPJOBUSAGELIST);
         exit(2);
      }
#endif

      sleep(10);
   }

   for_each(job, ptf_jobs) {
      ptf_kill(ptf_get_jobid(job), SIGTERM);
   }

#ifdef USE_DC
   psStopCollector();
#endif

   return 0;
}

#endif /* MODULE_TEST */

#endif
