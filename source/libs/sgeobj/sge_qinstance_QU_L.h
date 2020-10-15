#ifndef __SGE_QINSTANCEL_H
#define __SGE_QINSTANCEL_H

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

/* *INDENT-OFF* */

enum {
   QU_qhostname = QU_LOWERBOUND, /* host where the queue instance resides */
   QU_qname,                     /* cluster queue the queue instance belongs to */
   QU_full_name,                 /* name of the queue instance, e.g. "all.q@powermachine" */
   QU_tag,                       /* tag used by scheduling code */
   QU_available_at, /* scheduling code only */

   QU_seq_no,
   QU_nsuspend,
   QU_qtype,
   QU_job_slots,

   QU_tmpdir,
   QU_shell,
   QU_calendar,
   QU_priority,
   QU_processors,
   QU_prolog,
   QU_epilog,
   QU_shell_start_mode,
   QU_starter_method,
   QU_suspend_method,
   QU_resume_method,
   QU_terminate_method,
   QU_initial_state,

   QU_rerun,

   QU_s_rt,
   QU_h_rt,
   QU_s_cpu,
   QU_h_cpu,
   QU_s_fsize,
   QU_h_fsize,
   QU_s_data,
   QU_h_data,
   QU_s_stack,
   QU_h_stack,
   QU_s_core,
   QU_h_core,
   QU_s_rss,
   QU_h_rss,
   QU_s_vmem,
   QU_h_vmem,

   QU_suspend_interval,
   QU_min_cpu_interval,
   QU_notify,
   
   QU_ckpt_list,
   QU_pe_list,

   QU_owner_list,
   QU_acl,
   QU_xacl,

   QU_projects,
   QU_xprojects,

   QU_consumable_config_list,   /* sublist with entry for each resource specified with "complex_values" */
   QU_load_thresholds,
   QU_suspend_thresholds,

   QU_subordinate_list,

   QU_state,
   QU_tagged4schedule, /* used for parallel job scheduling only */
                       /* 0 - can be used only as slave queue */
                       /* 1 - can be used as slave queue for now assignment,
                              however as master for reservation */
                       /* 2 - can be used as master for now and reservation */
   QU_cache_version,
   QU_pending_signal,
   QU_pending_signal_delivery_time,
   QU_version,
   QU_suspended_on_subordinate,
   QU_last_suspend_threshold_ckeck,
   QU_job_cnt,
   QU_pending_job_cnt,
   QU_soft_violation,
   QU_host_seq_no,
   QU_resource_utilization,     /* sublist with resource utilization entry for each consumable */
   QU_message_list,
   QU_gdi_do_later,

/* new state variables, these are needed for the scheduler, to dispach jobs into suitable queues */
   QU_state_changes
};


LISTDEF(QU_Type)
   JGDI_OBJ(QueueInstance)
   JGDI_EVENT_OBJ(ADD(sgeE_QINSTANCE_ADD) | MODIFY(sgeE_QINSTANCE_MOD) | DELETE(sgeE_QINSTANCE_DEL))
   SGE_HOST(QU_qhostname, CULL_HASH | CULL_SPOOL | CULL_JGDI_RO) 
   SGE_STRING(QU_qname, CULL_SPOOL | CULL_JGDI_RO)
   SGE_STRING(QU_full_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_JGDI_RO)
   SGE_ULONG(QU_tag, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_available_at, CULL_DEFAULT | CULL_JGDI_RO)

   SGE_ULONG(QU_seq_no, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_ULONG(QU_nsuspend, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_ULONG(QU_qtype, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_ULONG(QU_job_slots, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_STRING(QU_tmpdir, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_shell, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_calendar, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_priority, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_processors, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_prolog, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_epilog, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_shell_start_mode, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_starter_method, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_suspend_method, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_resume_method, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_terminate_method, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_initial_state, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_BOOL(QU_rerun, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_STRING(QU_s_rt, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_rt, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_cpu, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_cpu, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_fsize, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_fsize, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_data, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_data, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_stack, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_stack, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_core, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_core, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_rss, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_rss, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_s_vmem, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_h_vmem, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_STRING(QU_suspend_interval, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_min_cpu_interval, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_STRING(QU_notify, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_LIST(QU_ckpt_list, ST_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_pe_list, ST_Type, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_LIST(QU_owner_list, US_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_acl, US_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_xacl, US_Type, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_LIST(QU_projects, PR_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_xprojects, PR_Type, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_LIST(QU_consumable_config_list, CE_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_load_thresholds, CE_Type, CULL_CONFIGURE | CULL_JGDI_RO)
   SGE_LIST(QU_suspend_thresholds, CE_Type, CULL_CONFIGURE | CULL_JGDI_RO)

   SGE_LIST(QU_subordinate_list, SO_Type, CULL_CONFIGURE | CULL_JGDI_RO)

/* EB:*/

   SGE_ULONG(QU_state, CULL_SPOOL | CULL_JGDI_HIDDEN)
   SGE_ULONG(QU_tagged4schedule, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_cache_version, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_pending_signal, CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(QU_pending_signal_delivery_time, CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(QU_version, CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(QU_suspended_on_subordinate, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_last_suspend_threshold_ckeck, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_job_cnt, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_pending_job_cnt, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_soft_violation, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_ULONG(QU_host_seq_no, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_LIST(QU_resource_utilization, RUE_Type, CULL_DEFAULT | CULL_JGDI_RO)
   SGE_LIST(QU_message_list, QIM_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
   SGE_ULONG(QU_gdi_do_later, CULL_DEFAULT | CULL_JGDI_RO)

   SGE_LIST(QU_state_changes, CQU_Type, CULL_DEFAULT | CULL_JGDI_RO)
 
LISTEND 

NAMEDEF(QUN)
   NAME("QU_qhostname")
   NAME("QU_qname")
   NAME("QU_full_name")
   NAME("QU_tag")
   NAME("QU_available_at")

   NAME("QU_seq_no")
   NAME("QU_nsuspend")
   NAME("QU_qtype")
   NAME("QU_job_slots")

   NAME("QU_tmpdir")
   NAME("QU_shell")
   NAME("QU_calendar")
   NAME("QU_priority")
   NAME("QU_processors")
   NAME("QU_prolog")
   NAME("QU_epilog")
   NAME("QU_shell_start_mode")
   NAME("QU_starter_method")
   NAME("QU_suspend_method")
   NAME("QU_resume_method")
   NAME("QU_terminate_method")
   NAME("QU_initial_state")

   NAME("QU_rerun")

   NAME("QU_s_rt")
   NAME("QU_h_rt")
   NAME("QU_s_cpu")
   NAME("QU_h_cpu")
   NAME("QU_s_fsize")
   NAME("QU_h_fsize")
   NAME("QU_s_data")
   NAME("QU_h_data")
   NAME("QU_s_stack")
   NAME("QU_h_stack")
   NAME("QU_s_core")
   NAME("QU_h_core")
   NAME("QU_s_rss")
   NAME("QU_h_rss")
   NAME("QU_s_vmem")
   NAME("QU_h_vmem")

   NAME("QU_suspend_interval")
   NAME("QU_min_cpu_interval")
   NAME("QU_notify")

   NAME("QU_ckpt_list")
   NAME("QU_pe_list")

   NAME("QU_owner_list")
   NAME("QU_acl")
   NAME("QU_xacl")

   NAME("QU_projects")
   NAME("QU_xprojects")

   NAME("QU_consumable_config_list") 
   NAME("QU_load_thresholds") 
   NAME("QU_suspend_thresholds") 

   NAME("QU_subordinate_list") 

   NAME("QU_state")
   NAME("QU_tagged4schedule")
   NAME("QU_cache_version")
   NAME("QU_pending_signal")
   NAME("QU_pending_signal_delivery_time")
   NAME("QU_version")
   NAME("QU_suspended_on_subordinate")
   NAME("QU_last_suspend_threshold_ckeck")
   NAME("QU_job_cnt")
   NAME("QU_pending_job_cnt")
   NAME("QU_soft_violation")
   NAME("QU_host_seq_no")
   NAME("QU_resource_utilization")
   NAME("QU_message_list")
   NAME("QU_gdi_do_later")
   NAME("QU_state_changes")
 
NAMEEND

#define QUS sizeof(QUN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_QINSTANCEL_H */
