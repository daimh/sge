#ifndef __PTF_H
#define __PTF_H
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

/*----------------------------------------------------
 * ptf.h
 *
 *--------------------------------------------------*/
  


#include <sys/types.h>

#if defined(IRIX)
#  include <sys/types.h>
#  include <sys/time.h>

   /* define some IRIX 6.[23] stuff not available in IRIX 6.4 */
   struct sched_deadline {
        timespec_t             dl_period;      /* size of deadline interval */
        timespec_t             dl_alloc;       /* cpu wanted in interval */
        /*
         * These fields are placeholders for future functionality.
         * They currently should be set to zero.
         */
        timespec_t              dl_deadline;    /* if deadline/period differ */
        int                     dl_flags;       /* control flags */
   };
#  define DEADLINE 15
#endif

#define PTF_COMPENSATION_FACTOR 2.0

#if defined(IRIX)
   typedef ash_t osjobid_t;
   typedef unsigned long long u_osjobid_t;
#  define OSJOBID_FMT "%lld"
/*#elif defined(ALPHA5)
   typedef unsigned long osjobid_t;
   typedef unsigned long u_osjobid_t;
#  define OSJOBID_FMT "%ld"       */
#else
   typedef pid_t osjobid_t;
   typedef unsigned int u_osjobid_t;
#  define OSJOBID_FMT pid_t_fmt
#endif

typedef gid_t addgrpid_t;
#define ADDGRPID_FMT gid_t_fmt  

/* job states */

#define JL_JOB_ACTIVE	0x00
#define JL_JOB_COMPLETE	0x01
#define JL_JOB_DELETED	0x02

/*-----------------------------------------------------

    PTF constants

*/

#define PTF_SCHEDULE_TIME 2

#define PTF_USAGE_DECAY_FACTOR 1.0

/* #define PTF_MIN_JOB_USAGE 0.001 */
#define PTF_MIN_JOB_USAGE 1.0

#define PTF_NICE_BASED

#define PTF_DIFF_DECAY_CONSTANT 0.8

#ifdef PTF_NICE_BASED
#  if defined(IRIX)
#    define ENFORCE_PRI_RANGE     1
#    define PTF_MIN_PRIORITY      (TS_PRIO_MIN)
#    define PTF_MAX_PRIORITY      (TS_PRIO_MAX)
#    define PTF_OS_MIN_PRIORITY   (TS_PRIO_MIN)
#    define PTF_OS_MAX_PRIORITY   (TS_PRIO_MAX)
#    define PTF_BACKGROUND_NICE_VALUE 20
#  elif defined(SOLARIS) || defined(ALPHA) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD)
#    define ENFORCE_PRI_RANGE     1
#    define PTF_MIN_PRIORITY      20
#    define PTF_MAX_PRIORITY     -10
#    define PTF_OS_MIN_PRIORITY   20
#    define PTF_OS_MAX_PRIORITY  -20
#  elif defined(LINUX) || defined(INTERIX) || defined(AIX)
#    define ENFORCE_PRI_RANGE     1
#    define PTF_MIN_PRIORITY      20
#    define PTF_MAX_PRIORITY      0
#    define PTF_OS_MIN_PRIORITY   20
#    define PTF_OS_MAX_PRIORITY  -20
#  elif defined(HP1164)
#    define ENFORCE_PRI_RANGE     1
#    define PTF_MIN_PRIORITY      39
#    define PTF_MAX_PRIORITY      0
#    define PTF_OS_MIN_PRIORITY   39
#    define PTF_OS_MAX_PRIORITY   0
#  endif
#  define PTF_BACKGROUND_JOB_PROPORTION 0.015
#  define PTF_BACKGROUND_JOB_PRIORITY NDPLOMAX
#endif

#ifdef IRIX
   /*
    * transform the priority value which will be accepted by
    * setpriority into a value which will be accepted by
    * schedctl
    */
#  define PTF_PRIORITY_TO_NATIVE_PRIORITY(priority) (priority+20)
#else
#  define PTF_PRIORITY_TO_NATIVE_PRIORITY(priority) (priority)
#endif


#ifdef PTF_NDPRI_BASED
#define PTF_MIN_PRIORITY NDPNORMMIN
/* #define PTF_MAX_PRIORITY NDPNORMMAX */
#define PTF_MAX_PRIORITY (NDPNORMMIN-10)
/*
#define PTF_MIN_PRIORITY NDPLOMIN
#define PTF_MAX_PRIORITY NDPLOMAX
*/
#endif


#ifdef PTF_SLICE_BASED
#define PTF_MIN_PRIORITY 1
#define PTF_MAX_PRIORITY 1000
#define PTF_TIME_TO_SLICE_UP 500   /* milliseconds */
#define PTF_MIN_SLICE        0.1    /* milliseconds */
#endif


/*-----------------------------------------------------

   PTF library

*/

int ptf_init(void);

void ptf_start(void); 

void ptf_stop(void); 

int ptf_is_running(void); 

void ptf_unregister_registered_job(u_long32 job_id, u_long32 ja_task_id );

void ptf_reinit_queue_priority(u_long32 job_id, u_long32 ja_task_idr, 
                               const char *pe_task_id_str, int priority);

int ptf_job_started(osjobid_t os_jobid, const char *task_id_str, 
                    lListElem *job, u_long32 jataskid);

int ptf_get_usage(lList **jobs);

lList *ptf_get_job_usage(u_long job_id, u_long ja_task_id, 
                         const char *task_id);

int ptf_process_job_ticket_list(lList *jobs);

int ptf_job_complete(u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, lList **usage);

void ptf_update_job_usage(void);

int ptf_adjust_job_priorities(void);

const char *ptf_errstr(int ptf_error_code);

void ptf_show_registered_jobs(void);

/*-----------------------------------------------------

    PTF errors
*/

/* #define PTF_Exxxx 1 */

#define PTF_ERROR_NONE                  0
#define PTF_ERROR_JOB_NOT_FOUND         1
#define PTF_ERROR_INVALID_ARGUMENT      2

#endif /* __PTF_H */

