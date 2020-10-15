
#ifndef __SGE_PE_TASKL_H
#define __SGE_PE_TASKL_H
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

/****** sgeobj/pe_task/--PET_Type *********************************************
*  NAME
*     PET_Type - CULL pe task element 
*
*  ELEMENTS
*     Job identification
*     ==================
*     SGE_STRING(PET_id)
*        The pe task id. It is unique per job.
*
*     SGE_STRING(PET_name)
*        Optional name of a pe task. Not yet completely implemented, but
*        it could be used to pass information to be shown by qstat.
*   
*     Runtime control information
*     ===================
*     SGE_ULONG(PET_status)
*        Status of the pe job, see defines in libs/gdi/sge_jobL.h.
*
*     SGE_LIST(PET_granted_destin_identifier_list)
*        Granted destination identifier list. Will contain one
*        entry specifying the queue the pe task runns in.
*
*     SGE_ULONG(PET_pid)
*        Pid of a running pe task (process group id).
*
*     SGE_STRING(PET_osjobid)
*        os jobid  of a running pe task.
*
*     Usage information
*     =================
*     SGE_LIST(PET_usage)
*     SGE_LIST(PET_scaled_usage)
*     SGE_LIST(PET_previous_usage)
*
*     Time information
*     ================
*     SGE_ULONG(PET_submission_time)
*     SGE_ULONG(PET_start_time)
*     SGE_ULONG(PET_end_time)
*
*     Submission information
*     ======================
*     SGE_STRING(PET_cwd)
*        Current working directory of the pe task. If not set, the 
*        cwd from the ja task is inherited.
*
*     SGE_LIST(PET_path_aliases)
*        Path alias list for the pe task.
*
*     SGE_LIST(PET_environment)
*        Environment variables exported to the pe task.
*        They will overwrite inherited variables from the ja task.
*
*  FUNCTION
*     PET_Type objects are used to store information about tasks of
*     tightly integrated parallel jobs (started with qrsh -inherit).
*     
*     Parallel tasks are sub objects of array tasks (even for non 
*     array jobs one pseudo array task is created).
*
*         +---------+   1:x   +----------+   1:x   +----------+
*         | JB_Type |<------->| JAT_Type |<------->| PET_Type |
*         +---------+         +----------+         +----------+
*
*  SEE ALSO
*     gdi/job/--JB_Type
*     gdi/job_jatask/JAT_Type                             
******************************************************************************/
enum {
   PET_id = PET_LOWERBOUND,
   PET_name,

   PET_status,

   PET_granted_destin_identifier_list,

   PET_pid,
   PET_osjobid,
   PET_usage,
   PET_scaled_usage,
   PET_reported_usage,
   PET_previous_usage,

   PET_submission_time,
   PET_start_time,
   PET_end_time,

   PET_cwd,
   PET_path_aliases,
   PET_environment,

   PET_do_contact
};

LISTDEF(PET_Type)
   JGDI_OBJ(PETask)
   JGDI_EVENT_OBJ(ADD(sgeE_PETASK_ADD) | DELETE(sgeE_PETASK_DEL))
   SGE_STRING(PET_id, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)
   SGE_STRING(PET_name, CULL_DEFAULT | CULL_SUBLIST)
   
   SGE_ULONG(PET_status, CULL_DEFAULT | CULL_SUBLIST)

   SGE_LIST(PET_granted_destin_identifier_list, JG_Type, CULL_DEFAULT | CULL_SUBLIST)

   SGE_ULONG(PET_pid, CULL_DEFAULT)
   SGE_STRING(PET_osjobid, CULL_DEFAULT)
   SGE_MAP(PET_usage, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_MAP(PET_scaled_usage, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_MAP(PET_reported_usage, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_MAP(PET_previous_usage, UA_Type, CULL_DEFAULT)

   SGE_ULONG(PET_submission_time, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(PET_start_time, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(PET_end_time, CULL_DEFAULT | CULL_SUBLIST)

   SGE_STRING(PET_cwd, CULL_DEFAULT | CULL_SUBLIST)
   SGE_LIST(PET_path_aliases, PA_Type, CULL_DEFAULT)
   SGE_MAP(PET_environment, VA_Type, CULL_DEFAULT)
   SGE_BOOL(PET_do_contact, CULL_DEFAULT | CULL_SUBLIST)
LISTEND

NAMEDEF(PETN)
   NAME("PET_id")
   NAME("PET_name")
   
   NAME("PET_status")
   
   NAME("PET_granted_destin_identifier_list")

   NAME("PET_pid")
   NAME("PET_osjobid")
   NAME("PET_usage")
   NAME("PET_scaled_usage")
   NAME("PET_reported_usage")
   NAME("PET_previous_usage")
   
   NAME("PET_submission_time")
   NAME("PET_start_time")
   NAME("PET_end_time")
   
   NAME("PET_cwd")
   NAME("PET_path_aliases")
   NAME("PET_environment")
   NAME("PET_do_contact")
NAMEEND


#define PETS sizeof(PETN)/sizeof(char*)

/****** sgeobj/pe_task/--PETR_Type ********************************************
*  NAME
*     PET_Type - CULL pe task request element 
*
*  ELEMENTS
*     Job identification
*     ==================
*     SGE_ULONG(PETR_jobid)
*
*     SGE_ULONG(PETR_jataskid)
*
*     Submission information
*     ======================
*     SGE_STRING(PETR_queuename)
*
*     SGE_STRING(PETR_owner)
*
*     SGE_STRING(PETR_cwd)
*
*     SGE_LIST(PETR_path_aliases)
*
*     SGE_LIST(PETR_environment)
*
*     Time information
*     ================
*     SGE_ULONG(PETR_submission_time)
*
*  FUNCTION
*     Objects of PETR_Type are used to request the start of a task in a 
*     tightly integrated parallel job.
*     
*  SEE ALSO
*     gdi/job_jatask/PET_Type
******************************************************************************/
enum {
   PETR_jobid = PETR_LOWERBOUND,
   PETR_jataskid,
   PETR_queuename,
   PETR_owner,
   PETR_cwd,
   PETR_path_aliases,
   PETR_environment,
   PETR_submission_time
};

LISTDEF(PETR_Type)
   SGE_ULONG(PETR_jobid, CULL_DEFAULT)
   SGE_ULONG(PETR_jataskid, CULL_DEFAULT)
   SGE_STRING(PETR_queuename, CULL_DEFAULT)
   SGE_STRING(PETR_owner, CULL_DEFAULT)
   SGE_STRING(PETR_cwd, CULL_DEFAULT)
   SGE_LIST(PETR_path_aliases, PA_Type, CULL_DEFAULT)
   SGE_MAPLIST(PETR_environment, VA_Type, CULL_DEFAULT)
   SGE_ULONG(PETR_submission_time, CULL_DEFAULT)
LISTEND


NAMEDEF(PETRN)
   NAME("PETR_jobid")
   NAME("PETR_jataskid")
   NAME("PETR_queuename")
   NAME("PETR_owner")
   NAME("PETR_cwd")
   NAME("PETR_path_aliases")
   NAME("PETR_environment")
   NAME("PETR_submission_time")
NAMEEND

#define PETRS sizeof(PETRN)/sizeof(char*)

/*
 * We need to store information about finished pe tasks to avoid
 * duplicate accounting records (see IZ 438).
 * A ja task will contain a list of finished pe tasks.
 * Only the task id of finished tasks will be stored.
 */

/* *INDENT-OFF* */

enum {
   FPET_id = FPET_LOWERBOUND
};

LISTDEF(FPET_Type)
   SGE_STRING(FPET_id, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)
LISTEND

NAMEDEF(FPETN)
   NAME("FPET_id")
NAMEEND

/* *INDENT-ON* */

#define FPETS sizeof(FPETN)/sizeof(char *)


#ifdef  __cplusplus
}
#endif

#endif /* __SGE_PE_TASKL_H */

