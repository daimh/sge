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

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "cull/cull.h"

#include "gdi/sge_gdi.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_order.h"

#include "sge_orders.h"
#include "sge_ssi.h"
#include "msg_schedd.h"

/* MT-NOTE: parse_job_identifier() is not MT safe */
static bool parse_job_identifier(const char *id, u_long32 *job_id, u_long32 *ja_task_id)
{
   char *copy = NULL;

   DENTER(TOP_LAYER, "parse_job_identifier");

   copy = strdup(id);
   *job_id = atoi(strtok(copy, "."));
   *ja_task_id = atoi(strtok(NULL, "."));
   sge_free(&copy);

   if(*job_id > 0 && *ja_task_id > 0) {
      DRETURN(true);
   }

   WARNING((SGE_EVENT, MSG_SSI_ERRORPARSINGJOBIDENTIFIER_S, id));

   DRETURN(false);
}

/****** schedlib/ssi/sge_ssi_job_cancel() **************************************
*  NAME
*     sge_ssi_job_cancel() -- delete or restart a job
*
*  SYNOPSIS
*     bool sge_ssi_job_cancel(const char *job_identifier, bool reschedule) 
*
*  FUNCTION
*     Delete the given job.
*     If reschedule is set to true, reschedule the job.
*
*  INPUTS
*     const char *job_identifier - job identifier in the form 
*                                  <jobid>.<ja_task_id>, e.g. 123.1
*     bool reschedule            - if true, reschedule job
*
*  RESULT
*     bool - true, if the job could be successfully deleted (rescheduled),
*           else false.
*
*  NOTES
*     The reschedule parameter is igored in the current implementation.
*
*  SEE ALSO
*     schedlib/ssi/sge_ssi/job_start()
*
*  MT-NOTE: sge_ssi_job_cancel() is not MT safe
*******************************************************************************/
bool sge_ssi_job_cancel(sge_evc_class_t *evc, const char *job_identifier, bool reschedule) 
{
   u_long32 job_id, ja_task_id;
   lList *ref_list = NULL, *alp;
   lListElem *ref_ep;
   char job_id_str[100];
   sge_gdi_ctx_class_t *ctx = evc->get_gdi_ctx(evc);

   DENTER(TOP_LAYER, "sge_ssi_job_cancel");

   /* reschedule not yet implemented */
   if(reschedule) {
      DRETURN(false);
   }

   if(!parse_job_identifier(job_identifier, &job_id, &ja_task_id)) {
      DRETURN(false);
   }

   /* create id structure */
   sprintf(job_id_str, sge_U32CFormat, sge_u32c(job_id));
   ref_ep = lAddElemStr(&ref_list, ID_str, job_id_str, ID_Type);
   ref_ep = lAddSubUlong(ref_ep, RN_min, ja_task_id, ID_ja_structure, RN_Type);
   lSetUlong(ref_ep, RN_max, ja_task_id);
   lSetUlong(ref_ep, RN_step, 1);

   /* send delete request */
   alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_DEL, &ref_list, NULL, NULL, false);

   answer_list_on_error_print_or_exit(&alp, stderr);

   DRETURN(true);
}


/****** schedlib/ssi/sge_ssi_job_start() ***************************************
*  NAME
*     sge_ssi_job_start() -- start a job
*
*  SYNOPSIS
*     bool sge_ssi_job_start(const char *job_identifier, const char *pe, 
*                           task_map tasks[]) 
*
*  FUNCTION
*     Start the job described by job_identifier, pe and tasks.
*     job_identifier has to be given in the form "<job_id>.<ja_task_id>",
*     e.g. "123.1" and must reference a pending job/array task.
*     For parallel jobs, pe has to be the name of an existing parallel
*     environment.
*     tasks describes how many tasks are to be started per host.
*
*     The function creates a scheduling order and sends it to qmaster.
*
*  INPUTS
*     const char *job_identifier - unique job identifier
*     const char *pe             - name of a parallel environment 
*                                  or NULL for sequential jobs
*     task_map tasks[]           - mapping host->number of tasks
*
*  RESULT
*     bool - true on success, else false
*
*  SEE ALSO
*     libsched/ssi/--Simple-Scheduler-Interface
*     libsched/ssi/-Simple-Scheduler-Interface-Typedefs
*******************************************************************************/
bool sge_ssi_job_start(sge_evc_class_t *evc, const char *job_identifier, const char *pe, task_map tasks[])
{
   u_long32 job_id, ja_task_id;
   lListElem *job, *ja_task;
   lList *order_list = NULL; /* list to be sent to qmaster */
   lList *granted = NULL;    /* granted queues */
   int i;

   DENTER(TOP_LAYER, "sge_ssi_job_start");

   if(!parse_job_identifier(job_identifier, &job_id, &ja_task_id)) {
      DRETURN(false);
   }

   /* create job element */
   job = lCreateElem(JB_Type);
   lSetUlong(job, JB_job_number, job_id);

   /* create array task */
   ja_task = lCreateElem(JAT_Type);
   lSetUlong(ja_task, JAT_task_number, ja_task_id);
   if(pe != NULL) {
      lSetString(ja_task, JAT_granted_pe, pe);
   }

   /* create granted queue list 
    * we expect exactly one queue per host to exist
    */
   for(i = 0; tasks[i].procs != 0; i++) {
      lListElem *queue, *granted_queue;

      if(tasks[i].host_name == NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_SSI_MISSINGHOSTNAMEINTASKLIST));
         DRETURN(false);
      }

      DPRINTF(("job requests %d slots on host %s\n", tasks[i].procs, tasks[i].host_name));
  
      queue = lGetElemHost(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), 
                           QU_qhostname, tasks[i].host_name);
      if(queue == NULL) {
         ERROR((SGE_EVENT, MSG_SSI_COULDNOTFINDQUEUEFORHOST_S, tasks[i].host_name));
         DRETURN(false);
      }

      granted_queue = lAddElemStr(&granted, JG_qname, lGetString(queue, QU_full_name), JG_Type);
      lSetUlong(granted_queue, JG_qversion, lGetUlong(queue, QU_version));
      lSetHost(granted_queue, JG_qhostname, lGetHost(queue, QU_qhostname));
      lSetUlong(granted_queue, JG_slots, tasks[i].procs);
   }

   /* create and send order */
   order_list = sge_create_orders(order_list, ORT_start_job, job, ja_task, granted, true);

   sge_send_orders2master(evc, &order_list);

   if (order_list != NULL) {
      lFreeList(&order_list);
   }   

   DRETURN(true);
}

