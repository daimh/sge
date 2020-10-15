#ifndef __SGE_RUSAGE_H
#define __SGE_RUSAGE_H
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
 ************************************************************************/
/*___INFO__MARK_END__*/
/****************************************************************
 Structures for architecture depended data
 ****************************************************************/

#include "uti/sge_dstring.h"

struct all_drsuage {
   char        *arch;
};

typedef struct all_drsuage sge_all_rusage_type;

/****************************************************************
 Structure used for reporting about the end of a job.
 ****************************************************************/
struct drusage {
   char     *qname;
   char     *hostname;
   char     *group;   /* the user's UNIX group on the exec host */
   char     *owner;    /* owner of the job we report about */
   char     *project;  /* project of the job we report about */
   char     *department; /* department of the job we report about */
   char     *job_name; /* -N switch or script_file or "STDIN" */
   char     *account;         /* accounting string see -A switch */
   u_long32 failed;           /* != 0 -> this job failed, 
                                 see states in execution_states.h */
   u_long32 general_failure;  /* != 0 execd reports "can execute no job", 
                                 also see above header */
   char     *err_str;         /* error string if this job is canceled
                                 abnormaly */
   u_long32 priority;         /* priority of job */
   u_long32 job_number;
   u_long32 submission_time;
   u_long32 start_time;
   u_long32 end_time;
   u_long32 exit_status;
   u_long32 signal;
   double   ru_wallclock;
   double   ru_utime;      /* user time used */
   double   ru_stime;      /* system time used */
   u_long64 ru_maxrss;
   u_long64 ru_ixrss;      /* integral shared text size */
   u_long64 ru_ismrss;     /* integral shared memory size*/
   u_long64 ru_idrss;      /* integral unshared data " */
   u_long64 ru_isrss;      /* integral unshared stack " */
   u_long64 ru_minflt;     /* page reclaims */
   u_long64 ru_majflt;     /* page faults */
   u_long64 ru_nswap;      /* swaps */
   u_long64 ru_inblock;    /* block input operations */
   u_long64 ru_oublock;    /* block output operations */
   u_long64 ru_msgsnd;     /* messages sent */
   u_long64 ru_msgrcv;     /* messages received */
   u_long64 ru_nsignals;   /* signals received */
   u_long64 ru_nvcsw;      /* voluntary context switches */
   u_long64 ru_nivcsw;     /* involuntary " */
   u_long32 pid;
   char     *granted_pe;
   u_long32 slots;
   char     *hard_resources_list;
   char     *hard_queue_list;
   u_long32 task_number;   /* job-array task number */
   double   cpu;
   double   mem;
   double   io;
   double   iow;
   char     *pe_taskid;
   double   maxvmem;
   u_long32 ar;
   u_long32 ar_sub_time;
   sge_all_rusage_type *arch_dep_usage;/* pointer to a structure with
                                          architecture dependend usage
                                          information */
                                              
};

typedef struct drusage sge_rusage_type;

/* 
 * name of the usage record that will hold the time when the last intermediate 
 * record has been written 
 */
#define LAST_INTERMEDIATE "im_acct_time"

/* 
 * time window in minutes after midnight in which intermediate usage will be 
 * written - we needn't do all the checks for intermediate usage reporting
 * at any time of the day.
 */
#define INTERMEDIATE_ACCT_WINDOW 10

/*
 * minimum runtime of a job in seconds as prerequisit for the writing of 
 * intermediate usage reporting - it's not worth writing an intermediate usage 
 * record for jobs that have started some seconds before midnight.
 */
#define INTERMEDIATE_MIN_RUNTIME 60

const char *
sge_write_rusage(dstring *buffer, 
                 lListElem *jr, lListElem *job, lListElem *ja_task, 
                 const char *category_str, const char delimiter,
                 bool intermediate);

#endif /* __SGE_RUSAGE_H */
