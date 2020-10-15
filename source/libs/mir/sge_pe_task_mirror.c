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
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"

#include "mir/sge_mirror.h"
#include "mir/msg_mirlib.h"
#include "mir/sge_pe_task_mirror.h"

#include "sge.h"

/****** Eventmirror/pe_task/pe_task_update_master_list_usage() *****************
*  NAME
*     pe_task_update_master_list_usage() -- update a parallel tasks usage
*
*  SYNOPSIS
*     bool 
*     pe_task_update_master_list_usage(lList *job_list, lListElem *event) 
*
*  FUNCTION
*     Updates the scaled usage of a parallel task.
*
*  INPUTS
*     lListElem *job_list - the master job list
*     lListElem *event    - event object containing the new usage list
*
*  RESULT
*     bool - true, if the operation succeeds, else false
*
*  SEE ALSO
*     Eventmirror/job/job_update_master_list_usage()
*     Eventmirror/ja_task/ja_task_update_master_list_usage()
*******************************************************************************/
sge_callback_result
pe_task_update_master_list_usage(lList *job_list, lListElem *event)
{
   lList *tmp = NULL;
   u_long32 job_id, ja_task_id;
   const char *pe_task_id;
   lListElem *job, *ja_task, *pe_task;

   DENTER(TOP_LAYER, "pe_task_update_master_list_usage");

   job_id = lGetUlong(event, ET_intkey);
   ja_task_id = lGetUlong(event, ET_intkey2);
   pe_task_id = lGetString(event, ET_strkey);
   
   job = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), job_id);
   if (job == NULL) {
      dstring id_dstring = DSTRING_INIT;
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDJOBFORUPDATEIN_SS, 
             job_get_id_string(job_id, 0, NULL, &id_dstring), SGE_FUNC));
      sge_dstring_free(&id_dstring);
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   
   ja_task = job_search_task(job, NULL, ja_task_id);
   if (ja_task == NULL) {
      dstring id_dstring = DSTRING_INIT;
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDJATASKFORUPDATEIN_SS, 
             job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), SGE_FUNC));
      sge_dstring_free(&id_dstring);
      DEXIT;
      return SGE_EMA_FAILURE;
   }

   pe_task = ja_task_search_pe_task(ja_task, pe_task_id);
   if (pe_task == NULL) {
      dstring id_dstring = DSTRING_INIT;
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDPETASKFORUPDATEIN_SS, 
             job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), SGE_FUNC));
      sge_dstring_free(&id_dstring);
      DEXIT;
      return SGE_EMA_FAILURE;
   }

   lXchgList(event, ET_new_version, &tmp);
   lXchgList(pe_task, PET_scaled_usage, &tmp);
   lXchgList(event, ET_new_version, &tmp);
   
   DEXIT;
   return SGE_EMA_OK;
}

/****** Eventmirror/pe_task/pe_task_update_master_list() ***********************
*  NAME
*     pe_task_update_master_list() -- update parallel tasks of an array task
*
*  SYNOPSIS
*     bool 
*     pe_task_update_master_list(sge_object_type type, sge_event_action action, 
*                                lListElem *event, void *clientdata) 
*
*  FUNCTION
*     Update the list of parallel tasks of an array task
*     based on an event.
*     The function is called from the event mirroring interface.
*
*     The scaled usage list of a parallel task is not updated
*     by this function, as this data is maintained by a 
*     separate event.
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
*******************************************************************************/
sge_callback_result
pe_task_update_master_list(sge_evc_class_t *evc, object_description *object_base, sge_object_type type, 
                           sge_event_action action, lListElem *event, void *clientdata)
{
   u_long32 job_id; 
   lListElem *job = NULL; 

   const char *pe_task_id = NULL;     
   lListElem *pe_task = NULL;

   u_long32 ja_task_id;
   lListElem *ja_task = NULL; 
   lList *pe_task_list = NULL;
   const lDescr *pe_task_descr = NULL;

   lList *usage = NULL;

   char id_buffer[MAX_STRING_SIZE];
   dstring id_dstring;

   DENTER(TOP_LAYER, "pe_task_update_master_list");

   sge_dstring_init(&id_dstring, id_buffer, MAX_STRING_SIZE);

   job_id = lGetUlong(event, ET_intkey);
   ja_task_id = lGetUlong(event, ET_intkey2);
   pe_task_id = lGetString(event, ET_strkey);
   
   job = job_list_locate(*sge_master_list(object_base, SGE_TYPE_JOB), job_id);
   if (job == NULL) {
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDJOBFORUPDATEIN_SS, 
             job_get_id_string(job_id, 0, NULL, &id_dstring), SGE_FUNC));
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   
   ja_task = job_search_task(job, NULL, ja_task_id);
   if (ja_task == NULL) {
      ERROR((SGE_EVENT, MSG_JOB_CANTFINDJATASKFORUPDATEIN_SS, 
             job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), SGE_FUNC));
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   
   pe_task = ja_task_search_pe_task(ja_task, pe_task_id);

   pe_task_list = lGetList(ja_task, JAT_task_list);
   pe_task_descr = lGetListDescr(lGetList(event, ET_new_version)); 
  
   if (action == SGE_EMA_MOD) {
      /* modify event for pe_task.
       * we may not update
       * - PET_scaled_usage - it is maintained by JOB_USAGE events
       */
      if (pe_task == NULL) {
         ERROR((SGE_EVENT, MSG_JOB_CANTFINDPETASKFORUPDATEIN_SS, 
                job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), SGE_FUNC));
         DEXIT;
         return SGE_EMA_FAILURE;
      }
      lXchgList(pe_task, PET_scaled_usage, &usage);
   }
 
   if (sge_mirror_update_master_list(&pe_task_list, pe_task_descr, pe_task, 
                                     job_get_id_string(job_id, ja_task_id, 
                                                       pe_task_id, &id_dstring),
                                     action, event) != SGE_EM_OK) {
      lFreeList(&usage);
      DEXIT;
      return SGE_EMA_FAILURE;
   }

   /* restore pe_task list after modify event */
   if (action == SGE_EMA_MOD) {
      pe_task = ja_task_search_pe_task(ja_task, pe_task_id);
      if (pe_task == NULL) {
         ERROR((SGE_EVENT, MSG_JOB_CANTFINDPETASKFORUPDATEIN_SS, 
                job_get_id_string(job_id, ja_task_id, pe_task_id, &id_dstring), SGE_FUNC));
         lFreeList(&usage);       
         DEXIT;
         return SGE_EMA_FAILURE;
      }

      lXchgList(pe_task, PET_scaled_usage, &usage);
      lFreeList(&usage);
   }

   /* first petask add event could have created new pe_task list for job */
   if (lGetList(ja_task, JAT_task_list) == NULL && pe_task_list != NULL) {
      lSetList(ja_task, JAT_task_list, pe_task_list);
   }

   DEXIT;
   return SGE_EMA_OK;
}

