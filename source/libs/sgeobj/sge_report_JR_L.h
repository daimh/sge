#ifndef __SGE_REPORT_JR_L_H
#define __SGE_REPORT_JR_L_H

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
/*
 * definition for job report
 */
enum {
   JR_job_number = JR_LOWERBOUND,
   JR_ja_task_number,
   JR_queue_name,
   JR_state,
   JR_failed,
   JR_general_failure,
   JR_err_str,
   JR_usage,                 /* UA_Type */
   JR_job_pid,
   JR_ckpt_arena,
   JR_pe_task_id_str,
   JR_osjobid,
   JR_wait_status,
   JR_flush,
   JR_no_send,
   JR_delay_report
};

LISTDEF(JR_Type)
   SGE_ULONG(JR_job_number, CULL_HASH)      /* Job to report */
   SGE_ULONG(JR_ja_task_number, CULL_DEFAULT) /* JobArray task to report */
   SGE_STRING(JR_queue_name, CULL_DEFAULT)  /* Queue this job (tried to) run in */
   SGE_ULONG(JR_state, CULL_DEFAULT)        /* either JRUNNING or JEXITING, JRUNNING sent 
                                               as ack for jobdelivery and cyclic */
   SGE_ULONG(JR_failed, CULL_DEFAULT)       /* FAILED_... */
   SGE_ULONG(JR_general_failure, CULL_DEFAULT) /* 1 -> general problem */
   SGE_STRING(JR_err_str, CULL_DEFAULT)     /* describes failure */
   SGE_MAP(JR_usage, UA_Type, CULL_DEFAULT) /* used resources UA_Type */
   SGE_ULONG(JR_job_pid, CULL_DEFAULT)      /* pid of job script */
   SGE_ULONG(JR_ckpt_arena, CULL_DEFAULT)   /* if there is a checkpoint in the arena */
   SGE_STRING(JR_pe_task_id_str, CULL_DEFAULT) /* string describing
                                                  task from view of PE.  If
                                                  non-null this is a PE task */
   SGE_STRING(JR_osjobid, CULL_DEFAULT)     /* string containing osjobid for ckpt jobs */
   SGE_ULONG(JR_wait_status, CULL_DEFAULT)  /* japi_wait() 'status' information  */
   SGE_BOOL(JR_flush, CULL_DEFAULT)
   SGE_BOOL(JR_no_send, CULL_DEFAULT)       /* do not send this job report - used for pe tasks & accounting_summary */
   SGE_BOOL(JR_delay_report, CULL_DEFAULT)  /* do not send this job report - used for qsub -sync/DRMAA jobs during a qmaster failover*/
LISTEND

NAMEDEF(JRN)
   NAME("JR_job_number")
   NAME("JR_ja_task_number")
   NAME("JR_queue_name")
   NAME("JR_state")
   NAME("JR_failed")
   NAME("JR_general_failure")
   NAME("JR_err_str")
   NAME("JR_usage")
   NAME("JR_job_pid")
   NAME("JR_ckpt_arena")
   NAME("JR_pe_task_id_str")
   NAME("JR_osjobid")
   NAME("JR_wait_status")
   NAME("JR_flush")
   NAME("JR_no_send")
   NAME("JR_delay_report")
NAMEEND

#define JRS sizeof(JRN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef __cplusplus
}
#endif

#endif                          /* __SGE_REPORTL_H */
