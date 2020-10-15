#ifndef __SGE_JOBL_H
#define __SGE_JOBL_H
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

/****** sgeobj/job/--JB_Type **************************************************
*  NAME
*     JB_Type - CULL job element
*
*  ELEMENTS
*     Job identification and dependencies
*     ===================================
*
*     SGE_ULONG(JB_job_number) ---> JB_id
*        Unique job number.
*
*     SGE_STRING(JB_job_name)
*        Job name ("qsub/qalter -N job_name")
*
*     SGE_ULONG(JB_version)
*        The number of times the job has been modified.
*
*     SGE_LIST(JB_jid_request_list)
*        job requested dependencies (JRE_Type only JRE_job_name)
*
*     SGE_LIST(JB_jid_predecessor_list)
*        Predecessor jobs (JRE_Type only JRE_job_name)
*
*     SGE_LIST(JB_jid_successor_list)
*        Successor jobs (JRE_Type only JRE_job_number)
*
*     SGE_LIST(JB_ja_ad_request_list)
*        job requested array dependencies (JRE_Type only JRE_job_name)
*
*     SGE_LIST(JB_ja_ad_predecessor_list)
*        Predecessor array jobs (JRE_Type only JRE_job_name)
*
*     SGE_LIST(JB_ja_ad_successor_list)
*        Successor array jobs (JRE_Type only JRE_job_number)
*
*     SGE_STRING(JB_session)
*        Job's session (JAPI session tag for job event selection)
*
*     Project/Department
*     ==================
*
*     SGE_STRING(JB_project)
*        Project name (qsub -P project_name)
*
*     SGE_STRING(JB_department)
*        Department name. Set by scheduler.
*
*     Data related to job script
*     ===========================
*
*     SGE_STRING(JB_directive_prefix)
*        Command prefix for jobscript ("qsub -C prefix") for parsing
*        special comments in the script file.
*
*     SGE_STRING(JB_exec_file)
*     ---> The path to the locally spooled copy on the execution daemon,
*          It is script what actually gets executed,
*          In the case of a binary, is unused.
*
*     SGE_STRING(JB_script_file)
*     ---> The path to the job as sent from the CLI, is the path on the submit host
*          In the case of a binary, is the path to the binary
*
*     SGE_ULONG(JB_script_size)
*     ---> really needed?
*
*     SGE_STRING(JB_script_ptr)
*     ---> the pointer to the character area of the job script
*
*     Time information
*     ================
*
*     SGE_RULONG(JB_submission_time)
*
*     SGE_ULONG(JB_execution_time)
*        When should the job start ("qsub/qalter -a date_time")
*
*     SGE_ULONG(JB_deadline)
*        Deadline initiation time. (qsub -dl date_time)
*
*     User related information
*     ========================
*
*     SGE_RSTRING(JB_owner) ---> rename to JB_user to be consistent?
*
*     SGE_RULONG(JB_uid)
*
*     SGE_RSTRING(JB_group)
*
*     SGE_RULONG(JB_gid)
*
*     SGE_STRING(JB_account)
*        Account string ("qsub/qalter -A account string")
*
*     Submission environment
*     ======================
*
*     SGE_STRING(JB_cwd)
*        Current working directory during qsub ("qsub -cwd")
*
*     SGE_BOOL(JB_notify)
*        Notify job of impending kill/stop signal. ("qsub -notify")
*
*     SGE_ULONG(JB_type)
*        Start job immediately or not at all. ("qsub -now")
*        JG: TODO: it is no boolean, but misused for other information!
*
*     SGE_BOOL(JB_reserve)
*        Specifies a reservation is desired by the user ("-R y|n").
*        Available for non-immediate job submissions. Irrespective
*        of the users desire a job reservation is made
*
*        o only in reservation scheduling mode
*        o only when the maximum number of reservations during a
*          scheduling run is greater than the position of the job in
*          priority order. The maximum number (SC_max_reservation) can be
*          specified in sched_conf(5).
*        o only for non-immediate jobs
*
*        Default is 'n'.
*
*     SGE_ULONG(JB_ar) ---> JB_ar
*        Unique advance reservation number.
*
*     SGE_ULONG(JB_priority)
*        Priority ("qsub/qalter -p priority")
*
*     SGE_ULONG(JB_jobshare)
*        Priority ("qsub/qalter -js jobshare")
*
*     SGE_LIST(JB_shell_list, PN_Type)
*        Command interpreter to be used (PN_Type).
*        ("qsub/qalter -S shell")
*
*     SGE_ULONG(JB_verify)
*        Triggers "verify" messages. (qsub -verify)
*
*     SGE_LIST(JB_env_list)
*        Export these environment variables (VA_Type). ("qsub -V").
*
*     SGE_TLIST(JB_context, VA_Type)
*        Custom attributes (name,val) pairs (VA_Type).
*        ("qsub/qalter -ac/-dc context_list")
*
*     SGE_LIST(JB_job_args)
*        Job arguments (ST_Type).
*
*     SGE_LIST(JB_binding)
*        Binding strategy for execution host (and later scheduler)
*
*     Checkpointing/Restart
*     =====================
*     SGE_ULONG(JB_checkpoint_attr)  ----> merge all checkpointing
*                                          stuff to one object?
*        Checkpoint attributes ("qsub/qalter -c interval_flags")
*
*     SGE_STRING(JB_checkpoint_name)
*        Name of checkpoint object ("qsub/qalter -ckpt ckpt_name")
*
*     SGE_OBJECT(JB_checkpoint_object, CK_Type)
*        Checkpoint object which will be sent from qmaster to execd.
*
*     SGE_ULONG(JB_checkpoint_interval)
*        Checkpoint frequency ("qsub/qalter -c seconds")
*
*     SGE_ULONG(JB_restart)
*        Is job rerunable? ("qsub/qalter -r y/n")
*        JG: TODO: it is no boolean, but misused for other information!
*
*     Job I/O
*     =======
*
*     SGE_LIST(JB_stdout_path_list)
*        Pathname for stdout (PN_Type). ("qsub/qalter -o path_name")
*
*     SGE_LIST(JB_stderr_path_list)
*        Standard error path streams (PN_Type). ("qsub/qalter "-e path_name")
*
*     SGE_LIST(JB_stdin_path_list)
*        Standard input path streams (PN_Type). ("qsub/qalter "-i path_name")
*
*     SGE_BOOL(JB_merge_stderr)
*        Merge stdout and stderr? ("qsub/qalter -j y|n")
*
*     Resource requests
*     =================
*
*     SGE_LIST(JB_hard_resource_list, CE_Type)
*        Hard resource requirements/limits/restrictions (CE_Type).
*        ("qsub -l resource_list")
*
*     SGE_LIST(JB_soft_resource_list, CE_Type)
*        Soft resource requirements/limits/restrictions (CE_Type).
*        ("qsub -l resource_list")
*
*     SGE_LIST(JB_hard_queue_list)
*        ----> why separated from other requests?
*        Hard queue list (QR_Type). ("qsub -q dest_identifier")
*
*     SGE_LIST(JB_soft_queue_list)
*        ----> why separated from other requests?
*        Soft queue list (QR_Type). ("qsub/qselect -q dest_identifier")
*
*     Mail options
*     ============
*
*     SGE_ULONG(JB_mail_options)
*        Mail options  ("qsub/qalter -m mail_options")
*
*     SGE_LIST(JB_mail_list)
*        Mail recipients (MR_Type). ("qsub/qalter -M mail_list)
*
*     Parallel Job info
*     =================
*
*     SGE_STRING(JB_pe)
*        Name of requested PE or wildcard expression for matching PEs
*
*     SGE_LIST(JB_pe_range)
*        PE slot range (RN_Type). Qmaster ensures it is ascending and
*        normalized.
*
*     SGE_LIST(JB_master_hard_queue_list)
*        Master queue list (QR_Type). ("qsub -masterq queue_list")
*
*     Security related data
*     =====================
*
*     SGE_STRING(JB_tgt)
*        Kerberos client TGT
*
*     SGE_STRING(JB_cred)
*        DCE/Kerberos credentials
*
*     Data related to array jobs
*     ==========================
*
*     SGE_LIST(JB_ja_structure)
*        Elements describing task id range structure during the
*        submission time of a (array) job (RN_Type).
*        ("qsub -t tid_range")
*
*     SGE_LIST(JB_ja_n_h_ids)
*        Just submitted array task without hold state (RN_Type).
*        ("qsub -t tid_range")
*
*     SGE_LIST(JB_ja_u_h_ids)
*        Just submitted and user hold applied (RN_Type).
*        ("qsub -h -t tid_range")
*        ("qalter -h u/U jid.tid1-tid2:step")
*
*     SGE_LIST(JB_ja_s_h_ids)
*        Just submitted and system hold applied (RN_Type).
*        ("qalter -h s/S jid.tid1-tid2:step")
*
*     SGE_LIST(JB_ja_o_h_ids)
*        Just submitted and operator hold applied (RN_Type).
*        ("qalter -h o/O jid.tid1-tid2:step")
*
*     SGE_LIST(JB_ja_a_h_ids)
*        Just submitted and array hold applied (RN_Type).
*        ("qalter -hold_jid_ad wc_job_list")
*
*     SGE_LIST(JB_ja_z_ids)
*        Zombie task ids (RN_Type).
*
*     SGE_LIST(JB_ja_template)
*        Template for new tasks.  The schedd thread
*        stores initial tickets in this element. (JAT_Type)
*
*     SGE_LIST(JB_ja_tasks)
*        List of array tasks (in case of array jobs) or one task
*        (in case of a job) (JAT_Type).
*
*     Data used only in scheduler
*     ===========================
*
*     SGE_HOST(JB_host)
*        Host job is executing on. Local to scheduler.
*        Not spooled.
*
*     SGE_REF(JB_category)
*        Category string reference used in scheduler.
*
*     Misc
*     ====
*
*     SGE_LIST(JB_user_list)
*        List of usernames (qsub/qalter -u username_list).
*        ---> qsub -u does not exist. Not part of a job, but only
*             userd for qalter request as where condition. Could most
*             probably be passed via lCondition.
*
*     SGE_LIST(JB_job_identifier_list)
*        ---> condition for qalter? Then it should better be passed
*             via condition.
*        (ID_Type)
*
*     SGE_ULONG(JB_verify_suitable_queues)   ---> qalter?
*
*     SGE_ULONG(JB_soft_wallclock_gmt)
*        Time at which the job's s_rt expires.
*
*     SGE_ULONG(JB_hard_wallclock_gmt)
*        Time at which the job's h_rt expires.
*
*     SGE_DOUBLE(JB_urg )
*        Absolute static urgency importance. The admin can use arbitrary
*        weighting factors in the formula used to determine this number. So any
*        value is possible. Needed only when scheduling code is run.
*        Not spooled.
*
*     SGE_DOUBLE(JB_nurg )
*        Relative importance due to static urgency in the range between 0.0
*        and 1.0. No need to make this a per task attribute as long as waiting time
*        and deadline remain job attributes.
*        Not spooled.
*
*     SGE_DOUBLE(JB_nppri )
*        Relative importance due to Posix priority in the range between 0.0
*        and 1.0. No need to make this a per task attribute as long as the POSIX
*        priority remains a job attribute.
*        Not spooled.
*
*     SGE_DOUBLE(JB_rrcontr )
*        Combined contribution to static urgency from all resources. This can
*        be any value. Actually this is a property of job category. This field is
*        needed only to provide it for diagnosis purposes it as per job information
*        via GDI.
*        Not spooled.
*
*     SGE_DOUBLE(JB_dlcontr )
*        Contribution to static urgency from job deadline. This can be any
*        value. No need to make this a per task attribute as long a deadline is a
*        job attribute. Increases over time.
*        Not spooled.
*
*     SGE_DOUBLE(JB_wtcontr )
*        Contribution to static urgency from waiting time. This can be any
*        value. No need to make this a per task attribute as long as waiting time
*        is a job attribute. Increases over time.
*        Not spooled.
*
*     SGE_ULONG(JB_override_tickets)
*        - override tickets assigned by admin.
*        (qalter -ot tickets).
*
*     SGE_LIST(JB_qs_args) ---> qsi? 
*        Arguments for foreign queuing system (ST_Type).
*        Either delete it, or recycle it to be used with starter_method.
*
*     SGE_LIST(JB_path_aliases)
*        Path aliases list (PA_Type).
*
*     SGE_ULONG(JB_pty)
*        Interactive job should be started in a pty. 0=no, 1=yes, 2=use default.
*
*     SGE_ULONG(JB_ja_task_concurrency)
*        The number of concurrent array tasks executing at any given time.
*
*
*  FUNCTION
*     JB_Type elements only make sense in conjunction with JAT_Type
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
*  SEE ALSO
*     gdi/job/--JAT_Type
******************************************************************************/
enum {
   JB_job_number = JB_LOWERBOUND,
   JB_job_name,     
   JB_version,
   JB_jid_request_list,
   JB_jid_predecessor_list,
   JB_jid_successor_list,
   JB_ja_ad_request_list,
   JB_ja_ad_predecessor_list,
   JB_ja_ad_successor_list,
   JB_session,

   JB_project,
   JB_department,
   
   JB_directive_prefix, 
   JB_exec_file,
   JB_script_file,
   JB_script_size,
   JB_script_ptr,
   
   JB_submission_time,
   JB_execution_time,
   JB_deadline,

   JB_owner,
   JB_uid,
   JB_group,
   JB_gid,
   JB_account,

   JB_cwd,                
   JB_notify,        
   JB_type,
   JB_reserve,
   JB_priority,         
   JB_jobshare,         
   JB_shell_list,
   JB_verify,      
   JB_env_list,
   JB_context,
   JB_job_args,
 
   JB_checkpoint_attr,
   JB_checkpoint_name,
   JB_checkpoint_object,
   JB_checkpoint_interval, 
   JB_restart,      

   JB_stdout_path_list,
   JB_stderr_path_list,
   JB_stdin_path_list,
   JB_merge_stderr, 

   JB_hard_resource_list,
   JB_soft_resource_list,
   JB_hard_queue_list,
   JB_soft_queue_list,

   JB_mail_options,     
   JB_mail_list,

   JB_pe,
   JB_pe_range,
   JB_master_hard_queue_list,
  
   JB_tgt,
   JB_cred,

   JB_ja_structure,
   JB_ja_n_h_ids,
   JB_ja_u_h_ids,
   JB_ja_s_h_ids,
   JB_ja_o_h_ids,
   JB_ja_a_h_ids,
   JB_ja_z_ids,
   JB_ja_template,
   JB_ja_tasks,

   JB_host,
   JB_category,

   JB_user_list, 
   JB_job_identifier_list,
   JB_verify_suitable_queues,
   JB_soft_wallclock_gmt,
   JB_hard_wallclock_gmt,
   JB_override_tickets,
   JB_qs_args,
   JB_path_aliases,
   JB_urg,
   JB_nurg,
   JB_nppri,
   JB_rrcontr,
   JB_dlcontr,
   JB_wtcontr,
   JB_ar,
   JB_pty,
   JB_ja_task_concurrency,
   JB_binding 
};

/* 
 * IF YOU CHANGE SOMETHING HERE THEN CHANGE ALSO THE ADOC COMMENT ABOVE 
 */
   
LISTDEF(JB_Type)
   JGDI_ROOT_OBJ(Job, SGE_JB_LIST, ADD | MODIFY| DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_JOB_ADD) | MODIFY(sgeE_JOB_MOD) | DELETE(sgeE_JOB_DEL) | GET_LIST(sgeE_JOB_LIST))
   SGE_ULONG(JB_job_number, CULL_PRIMARY_KEY | CULL_HASH | CULL_SPOOL) 
   SGE_STRING(JB_job_name, CULL_DEFAULT | CULL_SPOOL)
   SGE_ULONG(JB_version, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_LIST(JB_jid_request_list, JRE_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(JB_jid_predecessor_list, JRE_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO) 
   SGE_LIST(JB_jid_successor_list, JRE_Type, CULL_DEFAULT | CULL_JGDI_RO) 
   SGE_LIST(JB_ja_ad_request_list, JRE_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(JB_ja_ad_predecessor_list, JRE_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO) 
   SGE_LIST(JB_ja_ad_successor_list, JRE_Type, CULL_DEFAULT | CULL_JGDI_RO) 
   SGE_STRING(JB_session, CULL_DEFAULT | CULL_SPOOL) 

   SGE_STRING(JB_project, CULL_DEFAULT | CULL_SPOOL)             
   SGE_STRING(JB_department, CULL_DEFAULT | CULL_SPOOL)  

   SGE_STRING(JB_directive_prefix, CULL_DEFAULT | CULL_SPOOL)     
   SGE_STRING(JB_exec_file, CULL_DEFAULT | CULL_SPOOL)
   SGE_STRING(JB_script_file, CULL_DEFAULT | CULL_SPOOL)
   SGE_ULONG(JB_script_size, CULL_DEFAULT | CULL_SPOOL)
   SGE_STRING(JB_script_ptr, CULL_DEFAULT)

   SGE_ULONG(JB_submission_time, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(JB_execution_time, CULL_DEFAULT | CULL_SPOOL)  
   SGE_ULONG(JB_deadline, CULL_DEFAULT | CULL_SPOOL) 

   SGE_STRING(JB_owner, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(JB_uid, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_STRING(JB_group, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(JB_gid, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_STRING(JB_account, CULL_DEFAULT | CULL_SPOOL)      

   SGE_STRING(JB_cwd, CULL_DEFAULT | CULL_SPOOL)     
   SGE_BOOL(JB_notify, CULL_DEFAULT | CULL_SPOOL)  
   SGE_ULONG(JB_type, CULL_DEFAULT | CULL_SPOOL)     
   SGE_BOOL(JB_reserve, CULL_DEFAULT | CULL_SPOOL)
   SGE_ULONG(JB_priority, CULL_DEFAULT | CULL_SPOOL)       
   SGE_ULONG(JB_jobshare, CULL_DEFAULT | CULL_SPOOL)       
   SGE_LIST(JB_shell_list, PN_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_ULONG(JB_verify, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO) 
   SGE_MAP(JB_env_list, VA_Type, CULL_DEFAULT | CULL_SPOOL)  
   SGE_MAP(JB_context, VA_Type, CULL_DEFAULT | CULL_SPOOL)  
   SGE_LIST(JB_job_args, ST_Type, CULL_DEFAULT | CULL_SPOOL)  

   SGE_ULONG(JB_checkpoint_attr, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)     
   SGE_STRING(JB_checkpoint_name, CULL_DEFAULT | CULL_SPOOL)   
   SGE_OBJECT(JB_checkpoint_object, CK_Type, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(JB_checkpoint_interval, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)   
   SGE_ULONG(JB_restart, CULL_DEFAULT | CULL_SPOOL)  

   SGE_LIST(JB_stdout_path_list, PN_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_LIST(JB_stderr_path_list, PN_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_LIST(JB_stdin_path_list, PN_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_BOOL(JB_merge_stderr, CULL_DEFAULT | CULL_SPOOL)     

   SGE_LIST(JB_hard_resource_list, CE_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(JB_soft_resource_list, CE_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(JB_hard_queue_list, QR_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_LIST(JB_soft_queue_list, QR_Type, CULL_DEFAULT | CULL_SPOOL) 
   
   SGE_ULONG(JB_mail_options, CULL_DEFAULT | CULL_SPOOL) 
   SGE_LIST(JB_mail_list, MR_Type, CULL_DEFAULT | CULL_SPOOL)  

   SGE_STRING(JB_pe, CULL_DEFAULT | CULL_SPOOL)              
   SGE_LIST(JB_pe_range, RN_Type, CULL_DEFAULT | CULL_SPOOL)     
   SGE_LIST(JB_master_hard_queue_list, QR_Type, CULL_DEFAULT | CULL_SPOOL)  

   SGE_STRING(JB_tgt, CULL_DEFAULT)      
   SGE_STRING(JB_cred, CULL_DEFAULT)   

   SGE_LIST(JB_ja_structure, RN_Type, CULL_DEFAULT | CULL_SPOOL)  
   SGE_LIST(JB_ja_n_h_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL) 
   SGE_LIST(JB_ja_u_h_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL)   
   SGE_LIST(JB_ja_s_h_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL)    
   SGE_LIST(JB_ja_o_h_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL)   
   SGE_LIST(JB_ja_a_h_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL)   
   SGE_LIST(JB_ja_z_ids, RN_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)   
   SGE_LIST(JB_ja_template, JAT_Type, CULL_DEFAULT | CULL_SPOOL)  
   SGE_LIST(JB_ja_tasks, JAT_Type, CULL_DEFAULT | CULL_SPOOL)

   SGE_HOST(JB_host, CULL_DEFAULT | CULL_JGDI_RO)       
   SGE_REF(JB_category, CT_Type, CULL_DEFAULT)    

   SGE_LIST(JB_user_list, ST_Type, CULL_DEFAULT)  
   SGE_LIST(JB_job_identifier_list, ID_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)    
   SGE_ULONG(JB_verify_suitable_queues, CULL_DEFAULT)
   SGE_ULONG(JB_soft_wallclock_gmt, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)
   SGE_ULONG(JB_hard_wallclock_gmt, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_HIDDEN)
   SGE_ULONG(JB_override_tickets, CULL_DEFAULT | CULL_SPOOL)   
   SGE_LIST(JB_qs_args, ST_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)   
   SGE_LIST(JB_path_aliases, PA_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_DOUBLE(JB_urg, CULL_DEFAULT)         
   SGE_DOUBLE(JB_nurg, CULL_DEFAULT | CULL_JGDI_RO)         
   SGE_DOUBLE(JB_nppri, CULL_DEFAULT | CULL_JGDI_RO)         
   SGE_DOUBLE(JB_rrcontr, CULL_DEFAULT | CULL_JGDI_RO)         
   SGE_DOUBLE(JB_dlcontr, CULL_DEFAULT | CULL_JGDI_RO)         
   SGE_DOUBLE(JB_wtcontr, CULL_DEFAULT | CULL_JGDI_RO)         
   SGE_ULONG(JB_ar, CULL_DEFAULT | CULL_SPOOL)     
   SGE_ULONG(JB_pty, CULL_DEFAULT | CULL_SPOOL)     
   SGE_ULONG(JB_ja_task_concurrency, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(JB_binding, BN_Type, CULL_DEFAULT | CULL_SPOOL)
   /* 
    * IF YOU ADD SOMETHING HERE THEN CHANGE ALSO THE ADOC COMMENT ABOVE 
    */
LISTEND

NAMEDEF(JBN)
   NAME("JB_job_number")
   NAME("JB_job_name")
   NAME("JB_version")
   NAME("JB_jid_request_list")
   NAME("JB_jid_predecessor_list")
   NAME("JB_jid_successor_list")
   NAME("JB_ja_ad_request_list")
   NAME("JB_ja_ad_predecessor_list")
   NAME("JB_ja_ad_successor_list")
   NAME("JB_session")

   NAME("JB_project")
   NAME("JB_department")
   
   NAME("JB_directive_prefix")
   NAME("JB_exec_file")
   NAME("JB_script_file")
   NAME("JB_script_size")
   NAME("JB_script_ptr")
   
   NAME("JB_submission_time")
   NAME("JB_execution_time")
   NAME("JB_deadline")
   
   NAME("JB_owner")
   NAME("JB_uid")
   NAME("JB_group")
   NAME("JB_gid")
   NAME("JB_account")

   NAME("JB_cwd")
   NAME("JB_notify")
   NAME("JB_type")
   NAME("JB_reserve")
   NAME("JB_priority")
   NAME("JB_jobshare")
   NAME("JB_shell_list")
   NAME("JB_verify")
   NAME("JB_env_list")
   NAME("JB_context")
   NAME("JB_job_args")

   NAME("JB_checkpoint_attr")
   NAME("JB_checkpoint_name")
   NAME("JB_checkpoint_object")
   NAME("JB_checkpoint_interval")
   NAME("JB_restart")

   NAME("JB_stdout_path_list")
   NAME("JB_stderr_path_list")
   NAME("JB_stdin_path_list")
   NAME("JB_merge_stderr")

   NAME("JB_hard_resource_list")
   NAME("JB_soft_resource_list")
   NAME("JB_hard_queue_list")
   NAME("JB_soft_queue_list")
   
   NAME("JB_mail_options")
   NAME("JB_mail_list")

   NAME("JB_pe")
   NAME("JB_pe_range")
   NAME("JB_master_hard_queue_list")
   
   NAME("JB_tgt")
   NAME("JB_cred")

   NAME("JB_ja_structure")
   NAME("JB_ja_n_h_ids")
   NAME("JB_ja_u_h_ids")
   NAME("JB_ja_s_h_ids")
   NAME("JB_ja_o_h_ids")
   NAME("JB_ja_a_h_ids")
   NAME("JB_ja_z_ids")
   NAME("JB_ja_template")
   NAME("JB_ja_tasks")

   NAME("JB_host")
   NAME("JB_category")
   
   NAME("JB_user_list")
   NAME("JB_job_identifier_list")
   NAME("JB_verify_suitable_queues")
   NAME("JB_soft_wallclock_gmt")
   NAME("JB_hard_wallclock_gmt")
   NAME("JB_override_tickets")
   NAME("JB_qs_args")
   NAME("JB_path_aliases")
   NAME("JB_urg")
   NAME("JB_nurg")
   NAME("JB_nppri")
   NAME("JB_rrcontr")
   NAME("JB_dlcontr")
   NAME("JB_wtcontr")
   NAME("JB_ar")
   NAME("JB_pty")
   NAME("JB_ja_task_concurrency")
   NAME("JB_binding")
NAMEEND

#define JBS sizeof(JBN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_JOBL_H */
