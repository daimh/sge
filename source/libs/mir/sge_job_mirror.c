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

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_job.h"

#include "mir/msg_mirlib.h"
#include "mir/sge_mirror.h"
#include "mir/sge_job_mirror.h"
#include "mir/sge_ja_task_mirror.h"
#include "mir/sge_pe_task_mirror.h"

#include "sge.h"

static bool job_update_master_list_usage(lList *job_list, lListElem *event);

/****** Eventmirror/job/job_update_master_list_usage() *************************
*  NAME
*     job_update_master_list_usage() -- update usage for a jobs tasks
*
*  SYNOPSIS
*     int job_update_master_list_usage(lListElem *event)
*
*  FUNCTION
*     Events containing usage reports are sent for a jobs tasks.
*     This can be array tasks (where a non array job has a single
*     array task) or tasks of a parallel job.
*     This function decides which type of task has to receive
*     the updated usage report and passes the event
*     information to the corresponding update functions.
*
*  INPUTS
*     lListElem *event - event object containing the new usage list
*     lList *job_list  - master job list
*
*  RESULT
*     bool - true, if the operation succeeds, else false
*
*  SEE ALSO
*     Eventmirror/ja_task/pe_task_update_master_list_usage()
*     Eventmirror/pe_task/pe_task_update_master_list_usage()
*******************************************************************************/
static bool job_update_master_list_usage(lList *job_list, lListElem *event)
{
   bool ret = true;
   u_long32 job_id, ja_task_id;
   const char *pe_task_id;

   DENTER(TOP_LAYER, "job_update_master_list_usage");

   job_id = lGetUlong(event, ET_intkey);
   ja_task_id = lGetUlong(event, ET_intkey2);
   pe_task_id = lGetString(event, ET_strkey);

   if(ja_task_id == 0) {
      dstring id_dstring = DSTRING_INIT;
      ERROR((SGE_EVENT, MSG_JOB_RECEIVEDINVALIDUSAGEEVENTFORJOB_S, job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring)));
      sge_dstring_free(&id_dstring);
      ret = false;
   }

   if(pe_task_id != NULL) {
      ret = (pe_task_update_master_list_usage(job_list, event) != SGE_EMA_FAILURE)?true:false;   /* usage for a pe task */
   } else {
      ret = (ja_task_update_master_list_usage(job_list, event) != SGE_EMA_FAILURE)?true:false;   /* usage for a ja task */
   }

   DEXIT;
   return ret;
}

/****** Eventmirror/job/job_update_master_list() *****************************
*  NAME
*     job_update_master_list() -- update the master list of jobs
*
*  SYNOPSIS
*     bool job_update_master_list(sge_object_type type,
*                                     sge_event_action action,
*                                     lListElem *event, void *clientdata)
*
*  FUNCTION
*     Update the global master list of jobs
*     based on an event.
*     The function is called from the event mirroring interface.
*
*     A jobs array tasks are not updated by this function,
*     as they are maintained by separate events.
*     In addition, some scheduler specific attributes, that
*     are only used in scheduler, are not updated.
*
*  INPUTS
*     sge_object_type type     - event type
*     sge_event_action action - action to perform
*     lListElem *event        - the raw event
*     void *clientdata        - client data
*
*  RESULT
*     bool - true, if update is successfull, else false
*
*  NOTES
*     The function should only be called from the event mirror interface.
*
*  SEE ALSO
*     Eventmirror/--Eventmirror
*     Eventmirror/sge_mirror_update_master_list()
*     Eventmirror/job/job_update_master_list_usage()
*******************************************************************************/
sge_callback_result
job_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                       sge_event_action action, lListElem *event, void *clientdata)
{
   lList **list;
   const lDescr *list_descr;
   u_long32 job_id;
   lListElem *job = NULL;
   lList *ja_tasks = NULL;

   char id_buffer[MAX_STRING_SIZE];
   dstring id_dstring;

   DENTER(TOP_LAYER, "job_update_master_list");

   sge_dstring_init(&id_dstring, id_buffer, MAX_STRING_SIZE);

   list = sge_master_list(object_base, SGE_TYPE_JOB);
   list_descr = lGetListDescr(lGetList(event, ET_new_version)); 
   job_id = lGetUlong(event, ET_intkey);
   job = job_list_locate(*list, job_id);

   if (action == SGE_EMA_MOD) {
      u_long32 event_type = lGetUlong(event, ET_type);

      if (job == NULL) {
         ERROR((SGE_EVENT, MSG_JOB_CANTFINDJOBFORUPDATEIN_SS,
                job_get_id_string(job_id, 0, NULL, &id_dstring), "job_update_master_list"));
         DRETURN(SGE_EMA_FAILURE);
      }

      if (event_type == sgeE_JOB_USAGE || event_type == sgeE_JOB_FINAL_USAGE ) {
         /* special handling needed for JOB_USAGE and JOB_FINAL_USAGE events.
         * they are sent for jobs, ja_tasks and pe_tasks and only contain
         * the usage list.
         * Preferable would probably be to send MOD events for the different
         * object types.
         */
         bool ret = job_update_master_list_usage(*list, event);
         DRETURN(ret?SGE_EMA_OK:SGE_EMA_FAILURE);
      } else {
         /* this is the true modify event.
          * we may not update several fields:
          * - JB_ja_tasks is the task list - it is maintained by JATASK events
          * - JB_host and JB_category are scheduler internal attributes
          *   they may not be overwritten.
          *   Better would be to move them from JB_Type to some scheduler specific
          *   object.
          */

          lListElem *modified_job;

          modified_job = lFirst(lGetList(event, ET_new_version));
          if(job != NULL && modified_job != NULL) {
            /* we want to preserve the old ja_tasks, since job update events to not contain them */
            lXchgList(job, JB_ja_tasks, &ja_tasks);
            lSetHost(modified_job, JB_host, lGetHost(job, JB_host));
            lSetRef(modified_job, JB_category, lGetRef(job, JB_category));
          }
      }
   }

   if (sge_mirror_update_master_list(list, list_descr, job, job_get_id_string(job_id, 0, NULL, &id_dstring), action, event) != SGE_EM_OK) {
      lFreeList(&ja_tasks);
      DRETURN(SGE_EMA_FAILURE);
   }

   /* restore ja_task list after modify event */
   if (action == SGE_EMA_MOD && ja_tasks != NULL) {
      /* we have to search the replaced job */
      job = job_list_locate(*list, job_id);
      if(job == NULL) {
         ERROR((SGE_EVENT, MSG_JOB_CANTFINDJOBFORUPDATEIN_SS,
                job_get_id_string(job_id, 0, NULL, &id_dstring), "job_update_master_list"));
         lFreeList(&ja_tasks);
         DRETURN(SGE_EMA_FAILURE);
      }

      lXchgList(job, JB_ja_tasks, &ja_tasks);
      lFreeList(&ja_tasks);
   }

   DRETURN(SGE_EMA_OK);
}

sge_callback_result
job_schedd_info_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                                   sge_event_action action, lListElem *event, void *clientdata)
{
   lList **list = NULL;
   const lDescr *list_descr = NULL;

   lList *data_list;
   lListElem *ep = NULL;
   
   DENTER(TOP_LAYER, "job_schedd_info_update_master_list");

   list = sge_master_list(object_base, type); 
   list_descr = lGetListDescr(lGetList(event, ET_new_version));

   /* We always update the whole list (consisting of one list element) */
   lFreeList(list);

   if((data_list = lGetList(event, ET_new_version)) != NULL) {
      if((ep = lFirst(data_list)) != NULL) {
         ep = lDechainElem(data_list, ep);
      }
   }

   /* if neccessary, create list and copy schedd info */
   if(ep != NULL) {
      *list = lCreateList("job schedd info", list_descr);
      lAppendElem(*list, ep);
   }

   DEXIT;
   return SGE_EMA_OK;
}

