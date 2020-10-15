#ifndef __SGE_JATASKL_H
#define __SGE_JATASKL_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****** sgeobj/ja_task/--JAT_Type *********************************************
*  NAME
*     JAT_Type - CULL array task 
*
*  ELEMENTS
*     SGE_ULONG(JAT_task_number) --->  JAT_id
*        Unique task number assigned during task creation.
*
*     SGE_ULONG(JAT_status) ---> merge status, state and hold
*        First part of the state (see also JAT_hold, JAT_state)
*
*     SGE_ULONG(JAT_start_time)
*        Tasks start time. 
*
*     SGE_ULONG(JAT_end_time)
*        Tasks end time. 
*        Only used in Execd to simulate job runs.
*
*     SGE_ULONG(JAT_hold) 
*        Second part of the state (user, operator, system hold) 
*        (see also JAT_status, JAT_state)
*
*     SGE_STRING(JAT_granted_pe)
*        Name of a granted parallel environment
*        
*     SGE_ULONG(JAT_job_restarted)
*        Was the task restarted (due to reschedule/migrate)?
*        JG: TODO: it is no boolean, but misused for other information!
*
*     SGE_LIST(JAT_granted_destin_identifier_list)
*        Granted destination identifier list (JG_Type)
*
*     SGE_STRING(JAT_master_queue) ---> == first element of JAT_granted_destin_identifier_list?
*        Master queue
* 
*     SGE_ULONG(JAT_state)
*        Third part of state (see also JAT_hold, JAT_status) 
*
*     SGE_ULONG(JAT_pvm_ckpt_pid) --->  still used? Any reference to PVM?
*
*     SGE_ULONG(JAT_pending_signal)
*
*     SGE_ULONG(JAT_pending_signal_delivery_time)
*
*     SGE_ULONG(JAT_pid) ---> move up
*
*     SGE_STRING(JAT_osjobid)
*        Unique id which applies to all os processes started 
*        on behalf of this task. Set during the startup phase of the
*        jobscript. Meaning depends on the architecture of that 
*        host were the task is started. 
*           SOLARIS/LINUX/ALPHA  - supplementary group id
*           IRIX                 - array session id
*
*     SGE_LIST(JAT_usage_list)
*        Raw usage from data collector. No longer used by schedd.
*        Scaled by qmaster to JAT_scaled_usage_list. 'UA_Type' list.
*        Not spooled.
*
*     SGE_LIST(JAT_scaled_usage_list)  
*        Scaled usage set by qmaster, used by schedd. 'UA_Type' list. 
*        Not spooled.
*
*     SGE_ULONG(JAT_fshare)
*        Functional shares associated with the job. Set and
*        used by schedd. Stored to qmaster for displaying.
*        Not spooled.
*
*     SGE_DOUBLE(JAT_tix)
*        Total tickets. Set by schedd, saved to qmaster.
*        Sent to PTF. Not Spooled
*
*     SGE_DOUBLE(JAT_oticket)
*        Override tickets set by schedd. Saved to qmaster and
*        sent to PTF. Not spooled.
*
*     SGE_DOUBLE(JAT_fticket)
*        Functional tickets set bye schedd. Saved to qmaster and 
*        sent to PTF. Not spooled.
*
*     SGE_DOUBLE(JAT_sticket)
*        Share-tree tickets set by schedd. Saved to qmaster.
*        Not spooled.
*
*     SGE_DOUBLE(JAT_share)
*        Job targeted proportion set by schedd. Saved to qmaster.
*        Not spooled.
*
*     SGE_ULONG(JAT_suitable) ---> only for output.
*        
*     SGE_LIST(JAT_task_list, PET_Type)
*        List of running parallel tasks
*
*     SGE_LIST(JAT_finished_task_list, FPET_Type)
*        List of finished parallel tasks - contains only task names.
*
*     SGE_LIST(JAT_previous_usage_list)
*
*     SGE_OBJECT(JAT_pe_object)
*        PE object granted to this task (PE_Type), only used in execd
*
*     SGE_ULONG(JAT_next_pe_task_id)         
*        Used locally in execd to store next pe task id for this jatask on this execd.
*
*     SGE_DOUBLE(JAT_progress_in_former_runs)         
*        The proportion of the total job run time earlier runs achieved and secured. 
*        No where used.
*
*     SGE_ULONG(JAT_stop_initiate_time)         
*        The time when the action to initiate a job finish (i.e. job termination,
*        or checkpoint creation) was triggered or re-triggered.
*
*     SGE_DOUBLE(JAT_prio )         
*        The GE priority derived from weighted normalized tickets and 
*        weighted normalized static urgency. Changes with task due to GE ticket 
*        dependency. Not spooled.
*
*     SGE_DOUBLE(JAT_ntix )         
*        Relative importance due to JAT_tix amount in the range between
*        0.0 and 1.0.
*        Not spooled.
*
*     SGE_ULONG(JAT_wallclock_limit)         
*        Resulting wallclock_limit. If not requested the minimum by queue h_rt's.
*
*  FUNCTION
*     JAT_Type elements make sense only in conjunction with JB_Type
*     elements.  One element of each type is necessary to hold all 
*     data for the execution of one job. One JB_Type element and 
*     x JAT_Type elements are needed to execute an array job with 
*     x tasks.
*
*              -----------       1:x        ------------
*              | JB_Type |<---------------->| JAT_Type |
*              -----------                  ------------
*
*     The relation between these two elements is defined in the 
*     'JB_ja_tasks' sublist of a 'JB_Type' element. This list will
*     contain all belonging JAT_Type elements. 
*
*     The 'JAT_Type' CULL element contains all attributes in which
*     one array task may differ from another array task of the 
*     same array job. The 'JB_Type' element defines all attributes
*     which are equivalent for all tasks of an array job.
*     A job and an array job with one task are equivalent 
*     concerning their data structures. Both consist of one 'JB_Type' 
*     and one 'JAT_Type' element.
*
*     'JAT_Type' elements contain dynamic data which accrue during the
*     execution of a job. Therefore it is not necessary to create
*     these elements during the submission of a (array) job, but
*     after the job has been dispatched.
*        
*  SEE ALSO 
*     gdi/job/--JB_Type
******************************************************************************/

/* *INDENT-OFF* */

enum {
   JAT_task_number = JAT_LOWERBOUND,
   JAT_status,
   JAT_start_time,
   JAT_end_time,
   JAT_hold,
   JAT_granted_pe,

   JAT_job_restarted,
   JAT_granted_destin_identifier_list,
   JAT_master_queue,
   JAT_state,
   JAT_pvm_ckpt_pid,

   JAT_pending_signal,
   JAT_pending_signal_delivery_time,
   JAT_pid,
   JAT_osjobid,
   JAT_usage_list,

   JAT_scaled_usage_list,
   JAT_reported_usage_list,
   JAT_fshare,
   JAT_tix,
   JAT_oticket,

   JAT_fticket,
   JAT_sticket,
   JAT_share,
   JAT_suitable,
   JAT_task_list,
   JAT_finished_task_list,

   JAT_previous_usage_list,

   JAT_pe_object,
   JAT_next_pe_task_id,
   /*JAT_progress_in_former_runs,*/
   JAT_stop_initiate_time,
   JAT_prio,
   JAT_ntix,

   JAT_wallclock_limit,

   JAT_message_list
};

LISTDEF(JAT_Type)
   JGDI_OBJ(JobTask)
   JGDI_EVENT_OBJ(ADD(sgeE_JATASK_ADD) | MODIFY(sgeE_JATASK_MOD) | DELETE(sgeE_JATASK_DEL))
   SGE_ULONG(JAT_task_number, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)
   SGE_ULONG(JAT_status, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_start_time, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_end_time, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_hold, CULL_DEFAULT | CULL_SUBLIST)
   SGE_STRING(JAT_granted_pe, CULL_DEFAULT | CULL_SUBLIST)

   SGE_ULONG(JAT_job_restarted, CULL_DEFAULT | CULL_SUBLIST)
   SGE_LIST(JAT_granted_destin_identifier_list, JG_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_STRING(JAT_master_queue, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_state, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_pvm_ckpt_pid, CULL_DEFAULT | CULL_SUBLIST)

   SGE_ULONG(JAT_pending_signal, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_pending_signal_delivery_time, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_pid, CULL_DEFAULT | CULL_SUBLIST)
   SGE_STRING(JAT_osjobid, CULL_DEFAULT | CULL_SUBLIST)
   SGE_MAP(JAT_usage_list, UA_Type, CULL_DEFAULT | CULL_SUBLIST)

   SGE_MAP(JAT_scaled_usage_list, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_MAP(JAT_reported_usage_list, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_fshare, CULL_DEFAULT | CULL_SUBLIST)
   SGE_DOUBLE(JAT_tix, CULL_DEFAULT | CULL_SUBLIST)
   SGE_DOUBLE(JAT_oticket, CULL_DEFAULT | CULL_SUBLIST)

   SGE_DOUBLE(JAT_fticket, CULL_DEFAULT | CULL_SUBLIST)
   SGE_DOUBLE(JAT_sticket, CULL_DEFAULT | CULL_SUBLIST)
   SGE_DOUBLE(JAT_share, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(JAT_suitable, CULL_DEFAULT)
   SGE_LIST(JAT_task_list, PET_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_LIST(JAT_finished_task_list, FPET_Type, CULL_DEFAULT | CULL_SUBLIST)

   SGE_MAP(JAT_previous_usage_list, UA_Type, CULL_DEFAULT)

   SGE_OBJECT(JAT_pe_object, PE_Type, CULL_DEFAULT)   
   SGE_ULONG(JAT_next_pe_task_id, CULL_DEFAULT)
/*   SGE_DOUBLE(JAT_progress_in_former_runs, CULL_DEFAULT | CULL_SUBLIST)         */
   SGE_ULONG(JAT_stop_initiate_time, CULL_DEFAULT | CULL_SUBLIST)         
   SGE_DOUBLE(JAT_prio, CULL_DEFAULT)         
   SGE_DOUBLE(JAT_ntix, CULL_DEFAULT)         

   SGE_ULONG(JAT_wallclock_limit, CULL_DEFAULT | CULL_SPOOL)

   SGE_LIST(JAT_message_list, QIM_Type, CULL_DEFAULT | CULL_SPOOL)
LISTEND 

NAMEDEF(JATN)
   NAME("JAT_task_number")
   NAME("JAT_status")
   NAME("JAT_start_time")
   NAME("JAT_end_time")
   NAME("JAT_hold")
   NAME("JAT_granted_pe")
   NAME("JAT_job_restarted")
   NAME("JAT_granted_destin_identifier_list")
   NAME("JAT_master_queue")
   NAME("JAT_state")
   NAME("JAT_pvm_ckpt_pid")
   NAME("JAT_pending_signal")
   NAME("JAT_pending_signal_delivery_time")
   NAME("JAT_pid")
   NAME("JAT_osjobid")
   NAME("JAT_usage_list")
   NAME("JAT_scaled_usage_list")
   NAME("JAT_reported_usage_list")
   NAME("JAT_fshare")

   NAME("JAT_tix")
   NAME("JAT_oticket")
   NAME("JAT_fticket")
   NAME("JAT_sticket")
   NAME("JAT_share")

   NAME("JAT_suitable")
   NAME("JAT_task_list")
   NAME("JAT_finished_task_list")
   NAME("JAT_previous_usage_list")

   NAME("JAT_pe_object")
   NAME("JAT_next_pe_task_id")
/*   NAME("JAT_progress_in_former_runs")*/
   NAME("JAT_stop_initiate_time")
   NAME("JAT_prio")
   NAME("JAT_ntix")

   NAME("JAT_wallclock_limit")

   NAME("JAT_message_list")
NAMEEND

/* *INDENT-ON* */

#define JATS sizeof(JATN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_JATASKL_H */
