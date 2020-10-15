#ifndef __SGE_PEL_H
#define __SGE_PEL_H

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

/****** sgeobj/pe/--PE_Type ***************************************************
*  NAME
*     PE_Type -- CULL
*
*  ELEMENTS
*     SGE_STRING(PE_name)
*        name of the pe 
*
*     SGE_ULONG(PE_slots)
*        number of total slots
*
*     SGE_LIST(PE_user_list)   
*        US_Type; list of allowed users
*
*     SGE_LIST(PE_xuser_list)  
*        US_Type; list of not allowed users
*
*     SGE_STRING(PE_start_proc_args)
*        cmd line sequence for starting the pe
*
*     SGE_STRING(PE_stop_proc_args)
*        cmd line sequence for stopping the pe
*
*     SGE_STRING(PE_allocation_rule)
*        number of processors per machine
*  
*     SGE_BOOL(PE_control_slaves)
*        whether slave tasks get fed into execd
*
*     SGE_BOOL(PE_job_is_first_task)
*        whether the job script also starts first
*        task like with pvm or job script is just a 
*        starter doing no work like with mpi, dmake 
*        --> has only a meaning when
*        PE_control_slaves is true
*
*     SGE_LIST(PE_resource_utilization, RUE_Type)
*        internal field; used only to store number used slots
*        this field gets not spooled, updated dynamically
*  
*     SGE_STRING(PE_urgency_slots)
*        Specifies what slot amount shall be used when computing jobs 
*        static urgency in case of jobs with slot range PE requests. 
*        The actual problem is that when determining the urgency number 
*        the number of slots finally assigned is not yet known. The following
*        settings are supported: min/max/avg/<fixed integer> 
*  
*     SGE_STRING(PE_qsort_args)
*        Specifies the dynamic library, function name, and string arguments
*        that should be called instead of the queue sort function once
*        the candidate queues for a job have been located.  The format is:
*        library_name function_name [strings_arguments ...]. The entire
*        string is passed to the function as the second argument.
*        NOTE: This is only available when compiled with -DSGE_PQS_API
*
*     SGE_BOOL(PE_accounting_summary)
*        For tightly integrated parallel jobs.
*        Specifies if a single accounting record is written for the whole job,
*        or if every task gets an individual accounting record.
*
******************************************************************************/
enum {
   PE_name = PE_LOWERBOUND,  
   PE_slots,               
   PE_user_list,          
   PE_xuser_list,         
   PE_start_proc_args,    
   PE_stop_proc_args,     
   PE_allocation_rule,    
   PE_control_slaves,    
   PE_job_is_first_task,
   PE_resource_utilization,
   PE_urgency_slots,
   PE_accounting_summary,
#ifdef SGE_PQS_API
   PE_qsort_args
#endif
};


LISTDEF(PE_Type)
   JGDI_ROOT_OBJ(ParallelEnvironment, SGE_PE_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_PE_ADD) | MODIFY(sgeE_PE_MOD) | DELETE(sgeE_PE_DEL) | GET_LIST(sgeE_PE_LIST))
   SGE_STRING_D(PE_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "template")
   SGE_ULONG(PE_slots, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_LIST(PE_user_list, US_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF) 
   SGE_LIST(PE_xuser_list, US_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF) 
   SGE_STRING_D(PE_start_proc_args, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "NONE")
   SGE_STRING_D(PE_stop_proc_args, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "NONE")
   SGE_STRING_D(PE_allocation_rule, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "$pe_slots")
   SGE_BOOL_D(PE_control_slaves, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "FALSE")
   SGE_BOOL_D(PE_job_is_first_task, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "TRUE")
   SGE_LIST(PE_resource_utilization, RUE_Type, CULL_DEFAULT | CULL_JGDI_HIDDEN)
   SGE_STRING_D(PE_urgency_slots, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "min")
   SGE_BOOL_D(PE_accounting_summary, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, "FALSE")
#ifdef SGE_PQS_API
   SGE_STRING(PE_qsort_args, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
#endif
LISTEND 

NAMEDEF(PEN)
   NAME("PE_name")
   NAME("PE_slots")
   NAME("PE_user_list")
   NAME("PE_xuser_list")
   NAME("PE_start_proc_args")
   NAME("PE_stop_proc_args")
   NAME("PE_allocation_rule")
   NAME("PE_control_slaves")
   NAME("PE_job_is_first_task")
   NAME("PE_resource_utilization")
   NAME("PE_urgency_slots")
   NAME("PE_accounting_summary")
#ifdef SGE_PQS_API
   NAME("PE_qsort_args")
#endif
NAMEEND

/* *INDENT-ON* */ 

#define PES sizeof(PEN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_PEL_H */
