#ifndef __SGE_JOB_SCHEDD_H
#define __SGE_JOB_SCHEDD_H
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

#define running_status(x) ((x)!=0 && (x)!=JFINISHED)
#define finished_status(x) ((x)==JFINISHED)

bool job_get_duration(u_long32 *duration, const lListElem *jep);
bool task_get_duration(u_long32 *duration, const lListElem *ja_task);

/* 
 * get order used for job sorting
 */
void sge_inc_jc(lList** jcpp, const char *name, int slots);

void sge_dec_jc(lList** jcpp, const char *name, int slots);

int job_get_next_task(lListElem *job, lListElem **task_ret, u_long32 *id_ret);

/*
 * drop all running jobs into the running list 
 *
 */
int sge_split_job_running(lList **jobs, lList **running, const char *running_name);

/*
 * move first task in job to running list 
 *
 */

int sge_move_to_running(lList **jobs, lList **running, lListElem *job);

/*
 * drop all finished jobs into the finished list 
 *
 */
int sge_split_job_finished(lList **jobs, lList **finished, const char *finished_name);

/*
 * drop all jobs waiting for -a into the waiting list 
 *
 */
int sge_split_job_wait_at_time(lList **jobs, lList **waiting, const char *waiting_name, u_long32 now);

/*
 * drop all jobs in error state
 *
 */
int sge_split_job_error(lList **jobs, lList **error, const char *error_name);

/*
 * drop all jobs in hold state
 *
 */
int sge_split_job_hold(lList **jobs, lList **hold, const char *hold_name);

/*
 * drop all jobs waiting for a predecessor into the waiting list 
 *
 */
int sge_split_job_wait_predecessor(lList **jobs, lList **waiting, const char *waiting_name);

/*
 * drop all jobs restricted by a ckpt environment
 *
 */
int sge_split_job_ckpt_restricted(lList **jobs, lList **restricted, const char *restricted_name, lList *ckpt_list);

lList *filter_max_running_1step(lList *pending_jobs, lList *running_jobs, lList **jct_list, int max_jobs, int elem);

lList *filter_max_running(lList *pending_jobs, lList *jct_list, int max_jobs, int elem);

int nslots_granted(lList *granted, const char *qhostname);

int active_subtasks(lListElem *job, const char *qname);

int active_nslots_granted(lListElem *job, lList *granted, const char *qhostname);

lListElem *explicit_job_request(lListElem *jep, const char *name);

int sge_granted_slots(lList *gdil);

const char *get_name_of_split_value(int value);

/****** sched/sge_job_schedd/SPLIT_-Constants *********************************
*  NAME
*     SPLIT_-Constants -- Constants used for split_jobs() 
*
*  SYNOPSIS
*     enum {
*        SPLIT_FIRST,
*        SPLIT_PENDING = SPLIT_FIRST,
*        SPLIT_PENDING_EXCLUDED,
*        SPLIT_PENDING_EXCLUDED_INSTANCES,
*        SPLIT_SUSPENDED,
*        SPLIT_WAITING_DUE_TO_PREDECESSOR,
*        SPLIT_HOLD,
*        SPLIT_ERROR,
*        SPLIT_WAITING_DUE_TO_TIME,
*        SPLIT_RUNNING,
*        SPLIT_FINISHED,
*        SPLIT_LAST
*     };             
*
*  FUNCTION
*     SPLIT_PENDING     - Pending jobs/tasks which may be dispatched 
*     SPLIT_PENDING_EXCLUDED     - Pending jobs/tasks which won't 
*                         be dispatched because this whould exceed 
*                         'max_u_jobs'
*     SPLIT_PENDING_EXCLUDED_INSTANCES    - Pending jobs/tasks which 
*                         won't be dispatched because this whould 
*                         exceed 'max_aj_instances' 
*     SPLIT_SUSPENDED   - Suspended jobs/tasks 
*     SPLIT_WAITING_DUE_TO_PREDECESSOR    - Jobs/Tasks waiting for 
*                         others to finish
*     SPLIT_HOLD        - Jobs/Tasks in user/operator/system hold
*     SPLIT_ERROR       - Jobs/Tasks which are in error state
*     SPLIT_WAITING_DUE_TO_TIME  - These jobs/tasks are not 
*                         dispatched because start time is in future
*     SPLIT_RUNNING     - These Jobs/Tasks won't be dispatched 
*                         because they are already running
*     SPLIT_FINISHED    - Already finished jobs/tasks   
*
*     SPLIT_NOT_STARTED - jobs that could not be dispatched in one scheduling
*                         run
*
*     SPLIT_FIRST and SPLIT_LAST might be used to build loops.
*
*  SEE ALSO
*     sched/sge_job_schedd/split_jobs() 
*     sched/sge_job_schedd/trash_splitted_jobs()
*******************************************************************************/
enum {
   SPLIT_FIRST,

   SPLIT_PENDING = SPLIT_FIRST,
   SPLIT_PENDING_EXCLUDED,
   SPLIT_PENDING_EXCLUDED_INSTANCES,
   SPLIT_SUSPENDED,
   SPLIT_WAITING_DUE_TO_PREDECESSOR,
   SPLIT_HOLD,
   SPLIT_ERROR,
   SPLIT_WAITING_DUE_TO_TIME,
   SPLIT_RUNNING,
   SPLIT_FINISHED,
   SPLIT_NOT_STARTED,
   SPLIT_DEFERRED,
   SPLIT_LAST
};

void 
split_jobs(lList **job_list, u_long32 max_aj_instances,
           lList **result_lists[], bool do_copy); 

void 
job_lists_split_with_reference_to_max_running(bool monitor_next_run, lList **job_lists[],
                                              lList **user_list,
                                              const char *user_name,
                                              int max_jobs_per_user);

bool
job_move_first_pending_to_running(lListElem **pending_job,
                                  lList **result_lists[]);

void 
trash_splitted_jobs(bool monitor_next_run, lList **job_list[]);

void 
job_lists_print(lList **job_list[]);

void 
user_list_init_jc(lList **user_list, lList **splitted_job_lists[]);

#endif /* __SGE_JOB_SCHEDD_H */

