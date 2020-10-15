#ifndef __SGE_PTF_JO_L_H
#define __SGE_PTF_JO_L_H

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
 * This is the list type we use to hold the list of
 * O.S. jobs being tracked for each PTF job. There
 * will normally only be one O.S. job per PTF job,
 * except in the case of parallel jobs or jobs with
 * multiple tasks.
 */

enum {
   JO_OS_job_ID,
   JO_OS_job_ID2,
   JO_ja_task_ID,
   JO_task_id_str,
   JO_state,
   JO_usage_list,
   JO_pid_list
};

LISTDEF(JO_Type)
   SGE_ULONG(JO_OS_job_ID, CULL_DEFAULT)    /* O.S. job id (lower 32 bits) */
   SGE_ULONG(JO_OS_job_ID2, CULL_DEFAULT)   /* O.S. job id (upper 32 bits) */
   SGE_ULONG(JO_ja_task_ID, CULL_DEFAULT)   /* job array task id */
   SGE_STRING(JO_task_id_str, CULL_DEFAULT) /* task ID string */
   SGE_ULONG(JO_state, CULL_DEFAULT)        /* job state (JL_JOB_*) */
   SGE_MAP(JO_usage_list, UA_Type, CULL_DEFAULT)    /* ptf interval usage */
   SGE_LIST(JO_pid_list, JP_Type, CULL_DEFAULT)      /* process ID list */
LISTEND 

NAMEDEF(JON)
   NAME("JO_OS_job_ID")
   NAME("JO_OS_job_ID2")
   NAME("JO_ja_task_ID")
   NAME("JO_task_id_str")
   NAME("JO_state")
   NAME("JO_usage_list")
   NAME("JO_pid_list")
NAMEEND

#define JOS sizeof(JON)/sizeof(char*)


/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_PTFL_H */
