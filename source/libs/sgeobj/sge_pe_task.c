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

#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_path_alias.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"

/****** sgeobj/pe_task/pe_task_sum_past_usage() *******************************
*  NAME
*     pe_task_sum_past_usage() -- sum up pe tasks past usage
*
*  SYNOPSIS
*     lListElem* pe_task_sum_past_usage(lListElem *container, 
*                                       const lListElem *pe_task) 
*
*  FUNCTION
*     The pe task list of a ja task can contain one container 
*     element to hold the usage of finished pe tasks no longer 
*     stored in the task list.
*
*     This function adds the usage of pe_task to the usage of container.
*
*  INPUTS
*     lListElem *container     - container object to hold past usage
*     const lListElem *pe_task - the pe task from which to copy usage
*
*  RESULT
*     lListElem* - the container object
*
*  SEE ALSO
*     sgeobj/pe_task/pe_task_sum_past_usage_all()
*     sgeobj/pe_task/pe_task_sum_past_usage_list()
*******************************************************************************/
lListElem *
pe_task_sum_past_usage(lListElem *container, const lListElem *pe_task)
{
   lList *container_usage;
   const lList *pe_task_usage;
   lList *container_reported_usage;
   const lList *pe_task_reported_usage;
   
   DENTER(TOP_LAYER, "pe_task_sum_past_usage");

   /* invalid input - nothing to do */
   if (container == NULL || pe_task == NULL) {
      DRETURN(NULL);
   }

   if (container == pe_task) {
      /* container and pe_task are the same element - nothing to do */
      DRETURN(container);
   }

   /* we sum up the scaled_usage */
   container_usage = lGetOrCreateList(container, PET_scaled_usage, "reported_usage", UA_Type);
   pe_task_usage = lGetList(pe_task, PET_scaled_usage);
   if (pe_task_usage != NULL) {
      usage_list_sum(container_usage, pe_task_usage);
   }

   /* 
    * and we sum up the already reported usage
    * (for long running jobs having intermediate usage records) 
    */
   container_reported_usage = lGetOrCreateList(container, PET_reported_usage, "reported_usage", UA_Type);
   pe_task_reported_usage = lGetList(pe_task, PET_reported_usage);
   if (pe_task_reported_usage != NULL) {
      usage_list_sum(container_reported_usage, pe_task_reported_usage);
   }

   DRETURN(container);
}

/****** sgeobj/pe_task/pe_task_sum_past_usage_all() ***************************
*  NAME
*     pe_task_sum_past_usage_all() -- sum up all pe tasks past usage
*
*  SYNOPSIS
*     lListElem* pe_task_sum_past_usage_all(lList *pe_task_list)
*
*  FUNCTION
*     Similar to pe_task_sum_past_usage, but will sum up the usage of 
*     all pe tasks in a task list to the container object in this list.
*
*     If the container object does not yet exist, it will be created.
*
*  INPUTS
*     lList *pe_task_list - the pe task list to process
*
*  RESULT
*     lListElem* - the container object
*
*  SEE ALSO
*     sgeobj/pe_task/pe_task_sum_past_usage()
*     sgeobj/pe_task/pe_task_sum_past_usage_list()
*******************************************************************************/
lListElem *pe_task_sum_past_usage_all(lList *pe_task_list)
{
   lListElem *container = NULL;
   const lListElem *pe_task;

   DENTER(TOP_LAYER, "pe_task_sum_past_usage_all");

   /* no pe task list - nothing to do */
   if (pe_task_list == NULL) {
      DRETURN(NULL);
   }

   /* loop over all pe tasks and sum up usage */
   for_each(pe_task, pe_task_list) {
      if (container == NULL) {
         container = pe_task_sum_past_usage_list(pe_task_list, pe_task);
      } else {
         pe_task_sum_past_usage(container, pe_task);
      }
   }

   DRETURN(container);
}

/****** sgeobj/pe_task/pe_task_sum_past_usage_list() **************************
*  NAME
*     pe_task_sum_past_usage_list() -- sum up pe tasks past usage
*
*  SYNOPSIS
*     lListElem* 
*     pe_task_sum_past_usage_list(lList *pe_task_list, 
*                                 const lListElem *pe_task) 
*
*  FUNCTION
*     Similar to pe_task_sum_past_usage.
*     The container is retrieved from pe_task_list, if it does not 
*     yet exist it is created and inserted into pe_task_list as 
*     first element.
*
*  INPUTS
*     lList *pe_task_list      - list containing the container object
*     const lListElem *pe_task - the pe task from which to copy usage
*
*  RESULT
*     lListElem* - the container object
*
*  SEE ALSO
*     sgeobj/pe_task/pe_task_sum_past_usage()
*     sgeobj/pe_task/pe_task_sum_past_usage_all()
******************************************************************************/
lListElem *
pe_task_sum_past_usage_list(lList *pe_task_list, const lListElem *pe_task)
{
   lListElem *container;

   DENTER(TOP_LAYER, "pe_task_sum_past_usage_list");

   /* no pe task list - nothing to do */
   if (pe_task_list == NULL) {
      DRETURN(NULL);
   }

   /* get container - if it does not yet exist, create it as first element in pe task list */
   container = lGetElemStr(pe_task_list, PET_id, PE_TASK_PAST_USAGE_CONTAINER);
   if (container == NULL) {
      container = lCreateElem(PET_Type);
      lSetString(container, PET_id, PE_TASK_PAST_USAGE_CONTAINER);
      lSetBool(container, PET_do_contact, true);
      lInsertElem(pe_task_list, NULL, container);
   }

   /* sum up usage */
   pe_task_sum_past_usage(container, pe_task);

   DRETURN(container);
}

/****** sge_pe_task/pe_task_verify_request() ***********************************
*  NAME
*     pe_task_verify_request() -- verify a pe task request object
*
*  SYNOPSIS
*     bool 
*     pe_task_verify_request(const lListElem *petr, lList **answer_list) 
*
*  FUNCTION
*     Verifies structure and contents of a pe task request.
*     A pe task request is sent to sge_execd by qrsh -inherit.
*
*  INPUTS
*     const lListElem *petr - the object to verify
*     lList **answer_list   - answer list to pass back error messages
*
*  RESULT
*     bool - true on success,
*            false on error with error message in answer_list
*
*  NOTES
*     MT-NOTE: pe_task_verify_request() is MT safe 
*
*  BUGS
*     The function is far from being complete.
*     Currently, only the CULL structure is verified, not the contents.
*
*  SEE ALSO
*     sge_object/object_verify_cull()
*******************************************************************************/
bool 
pe_task_verify_request(const lListElem *petr, lList **answer_list) {
   bool ret = true;

   DENTER(TOP_LAYER, "pe_task_verify_request");

   if (petr == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_NULLELEMENTPASSEDTO_S, SGE_FUNC);
      ret = false;
   }

   /* 
    * A pe task request entering execd must have some additional properties: 
    *    - PETR_jobid > 0
    *    - PETR_jataskid > 0
    *    - PETR_owner != NULL
    *    - verify PETR_queuename, if != NULL
    *    - verify PETR_cwd, if it is != NULL
    *    - verify PETR_path_aliases, if they are != NULL
    *    - verify PETR_environment, if it is != NULL
    *    - PETR_submission_time (currently unused)
    */

   if (ret) {
      ret = object_verify_ulong_not_null(petr, answer_list, PETR_jobid);
   }

   if (ret) {
      ret = object_verify_ulong_not_null(petr, answer_list, PETR_jataskid);
   }

   if (ret) {
      ret = object_verify_string_not_null(petr, answer_list, PETR_owner);
   }

   if (ret) {
      const char *queue_name = lGetString(petr, PETR_queuename);

      if (queue_name != NULL) {
         if (verify_str_key(
               answer_list, queue_name, MAX_VERIFY_STRING,
               lNm2Str(PETR_queuename), KEY_TABLE) != STATUS_OK) {
            ret = false;
         }
      }
   }

   if (ret) {
      const char *cwd = lGetString(petr, PETR_cwd);

      if (cwd != NULL) {
         ret = path_verify(cwd, answer_list, "cwd", true);
      }
   }

   if (ret) {
      const lList *path_aliases = lGetList(petr, PETR_path_aliases);

      if (path_aliases != NULL) {
         ret = path_alias_verify(path_aliases, answer_list);
      }
   } 

   if (ret) {
      const lList *env_list = lGetList(petr, PETR_environment);

      if (env_list != NULL) {
         ret = var_list_verify(env_list, answer_list);
      }
   } 

   DRETURN(ret);
}
