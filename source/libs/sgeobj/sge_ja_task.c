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
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_range.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_mesobj.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"

/****** sgeobj/ja_task/ja_task_search_pe_task()*********************************
*  NAME
*     ja_task_search_pe_task() -- Find a certain PE Task 
*
*  SYNOPSIS
*     lListElem* ja_task_search_pe_task(const lListElem *ja_task, 
*                                       const char **pe_task_id) 
*
*  FUNCTION
*     Find a certain PE Task with "pe_task_id" in "ja_task" 
*
*  INPUTS
*     const lListElem *ja_task - JAT_Type element 
*     const char *pe_task_id   - PE task id string (e.g. "1.speedy") 
*
*  RESULT
*     lListElem* - PET_Type
*******************************************************************************/
lListElem *ja_task_search_pe_task(const lListElem *ja_task, 
                                  const char *pe_task_id)
{
   if(ja_task != NULL) {
      lList *pe_tasks = lGetList(ja_task, JAT_task_list);

      if(pe_tasks != NULL) {
         return lGetElemStr(pe_tasks, PET_id, pe_task_id);
      }
   }

   return NULL;
}

/****** sgeobj/ja_task/ja_task_list_print_to_string() **************************
*  NAME
*     ja_task_list_print_to_string() -- print task id ranges into string 
*
*  SYNOPSIS
*     void ja_task_list_print_to_string(const lList *ja_task_list, 
*                                       dstring *range_string) 
*
*  FUNCTION
*     The ids of all tasks contained in 'ja_task_list' will be printed
*     into 'range_string'. 
*
*  INPUTS
*     const lList *ja_task_list - JAT_Type list 
*     dstring *range_string     - dynamic string 
******************************************************************************/
void ja_task_list_print_to_string(const lList *ja_task_list, 
                                  dstring *range_string)
{
   lListElem *ja_task = NULL;    /* JAT_Type */
   lList *range_list = NULL;     /* RN_Type */

   DENTER(TOP_LAYER, "ja_task_list_print_to_string");
   for_each(ja_task, ja_task_list) {
      u_long32 tid = lGetUlong(ja_task, JAT_task_number);

      range_list_insert_id(&range_list, NULL, tid);      
   } 
   range_list_sort_uniq_compress(range_list, NULL, true); 
   range_list_print_to_string(range_list, range_string, false, false, false); 
   lFreeList(&range_list);
   DRETURN_VOID;
}

/****** sgeobj/ja_task/ja_task_list_split_group() ******************************
*  NAME
*     ja_task_list_split_group() -- Splits a list into two parts
*
*  SYNOPSIS
*     lList* ja_task_list_split_group(lList **ja_task_list)
*
*  FUNCTION
*     All tasks which have the same state (JAT_status, JAT_state) like
*     the first element of 'ja_task_list' will be removed from i
*     'ja_task_list' and returned by this function.
*
*  INPUTS
*     lList **ja_task_list - JAT_Type list
*
*  RESULT
*     lList* - JAT_Type list (elements with equivalent state)
*
*  SEE ALSO
*     sgeobj/range/RN_Type
******************************************************************************/
lList* ja_task_list_split_group(lList **ja_task_list)
{
   lList *ret_list = NULL;

   if (ja_task_list && *ja_task_list) {
      lListElem *first_task = lFirst(*ja_task_list);

      if (first_task) {
         u_long32 status = lGetUlong(first_task, JAT_status); 
         u_long32 state = lGetUlong(first_task, JAT_state); 
         u_long32 hold = lGetUlong(first_task, JAT_hold);
         const lDescr *descr = lGetElemDescr(first_task);
         lCondition *where = NULL;

         where = lWhere("%T(%I != %u || %I != %u || %I != %u)", descr,
                        JAT_status, status, JAT_state, state,
                        JAT_hold, hold);
         lSplit(ja_task_list, &ret_list, NULL, where);
         lFreeWhere(&where);
      }
   }
   return ret_list;
}                     

/****** sgeobj/ja_task/ja_task_add_finished_pe_task() **************************
*  NAME
*     ja_task_add_finished_pe_task() -- remember finished parallel task
*
*  SYNOPSIS
*     bool 
*     ja_task_add_finished_pe_task(lListElem *ja_task, 
*                                  const char *pe_task_id) 
*
*  FUNCTION
*     To avoid duplicate handling of finished parallel tasks (which 
*     could be triggered by sge_execd sending task end reports multiple 
*     times until it receives an ack from qmaster), the ja_task object 
*     (JAT_Type) contains a list of finished parallel tasks.
*    
*     ja_task_add_finished_pe_task tries to add a new parallel task to 
*     this list.
*
*     If an entry with the given pe_task_id already exists, the function 
*     returns false, else true.
*
*  INPUTS
*     lListElem *ja_task     - the ja_task to check/modify
*     const char *pe_task_id - the pe_task_id to check/insert
*
*  RESULT
*     bool - error state
*        true  - if the pe_task_id did not yet exist and could be inserted,
*        false - if the pe_task_id already existed.
*
*  SEE ALSO
*     sgeobj/ja_task/ja_task_clear_finished_pe_tasks()
*******************************************************************************/
bool ja_task_add_finished_pe_task(lListElem *ja_task, const char *pe_task_id)
{
   lListElem *pe_task;

   DENTER(TOP_LAYER, "ja_task_add_finished_pe_task");

   pe_task = lGetSubStr(ja_task, FPET_id, pe_task_id, JAT_finished_task_list);
   if (pe_task != NULL) {
      INFO((SGE_EVENT, "already handled exit of pe task "SFQ" in ja_task "
         sge_U32CFormat"\n", pe_task_id, sge_u32c(lGetUlong(ja_task, JAT_task_number))));
      DRETURN(false);
   }

   if (lAddSubStr(ja_task, FPET_id, pe_task_id, JAT_finished_task_list,
                  FPET_Type))
      DRETURN(true);
   else {
      ERROR((SGE_EVENT, "ja_task_add_finished_pe_task: lAddSubStr failed, task "
             SFQ, pe_task_id));
      DRETURN(false);
   }
}

/****** sgeobj/ja_task/ja_task_clear_finished_pe_tasks() ***********************
*  NAME
*     ja_task_clear_finished_pe_tasks() -- clear finished task list 
*
*  SYNOPSIS
*     bool 
*     ja_task_clear_finished_pe_tasks(lListElem *ja_task) 
*
*  FUNCTION
*     A ja_task contains a list of all finished parallel tasks (see also
*     sgeobj/ja_task/ja_task_add_finished_pe_task()).
*
*     In certain circumstances (e.g. if a ja_task is rescheduled), it is 
*     necessary to clear this list.
*
*     ja_task_clear_finished_pe_tasks removes the complete sublist including
*     the contained task ids.
*
*  INPUTS
*     lListElem *ja_task - the ja_task to modify
*
*  RESULT
*     bool - true, if the list could be cleared,
*            false, if no list of finished pe_tasks existed.
*
*  SEE ALSO
*     sgeobj/ja_task/ja_task_add_finished_pe_task()
*******************************************************************************/
bool ja_task_clear_finished_pe_tasks(lListElem *ja_task)
{
   lList *pe_task_list;

   DENTER(TOP_LAYER, "ja_task_clear_finished_pe_tasks");

   /* get list of finished pe tasks */
   pe_task_list = lGetList(ja_task, JAT_finished_task_list);
   if (pe_task_list == NULL) {
      DPRINTF(("no finished pe task list to clear in ja_task "sge_u32"\n",
               lGetUlong(ja_task, JAT_task_number)));
      DRETURN(false);
   }

   /* if we have such a list, delete it (lSetList will free the list) */
   lSetList(ja_task, JAT_finished_task_list, NULL);

   DPRINTF(("cleared finished pe task list in ja_task "sge_u32"\n",
            lGetUlong(ja_task, JAT_task_number)));

   DRETURN(true);
}

/****** parse/sge_parse_jobtasks() *********************************************
*  NAME
*     sge_parse_jobtasks() -- parse array task ranges 
*
*  SYNOPSIS
*     int sge_parse_jobtasks(lList **ipp, lListElem **idp, const char 
*     *str_jobtask, lList **alpp, bool include_names, lList *arrayDefList) 
*
*  FUNCTION
*    parses a job ids with or without task ranges following this pattern: 
*     Digit = '0' | '1' | ... | '9' .
*     JobId = Digit { Digit } .
*     TaskIdRange = TaskId [ '-' TaskId [  ':' Digit ] ] .
*     JobTasks = JobId [ '.' TaskIdRange ] .
*
*   in case of a job name, the task range has to be specified extra. This
*   will be colleced in an extra list and handed in as the arrayDefList
*
*  INPUTS
*     lList **ipp             - ID_Type List, target list
*     lListElem **idp         - New ID_Type-Elem parsed from str_jobtask 
*     const char *str_jobtask - job id with task range or job name 
*     lList **alpp            - answer list 
*     bool include_names      - true: job names are allowed
*     lList *arrayDefList     - in case of job names, a list of array taskes
*
*  RESULT
*     int - -1 no valid JobTask-Identifier
*           0 everything went fine
*
*
*  NOTES
*     MT-NOTE: sge_parse_jobtasks() is MT safe 
*
*******************************************************************************/
int sge_parse_jobtasks(lList **ipp, lListElem **idp, const char *str_jobtask,   
                       lList **alpp, bool include_names, lList *arrayDefList) {
   char *token;
   char *job_str;
   lList *task_id_range_list = NULL;
   int ret = 1;

   DENTER(TOP_LAYER, "sge_parse_jobtasks");
   job_str = strdup(str_jobtask);

   /* An empty job id string is a bad job id string! */
   if (strcmp (job_str, "") == 0) {
      ret = -1;
   }
   /*
   ** dup the input string for tokenizing
   */
   else if(isdigit(job_str[0])) {
      const double epsilon = 1.0E-12;
      char *end_ptr = NULL;
      double dbl_value;
      u_long32 ulng_value;

      if ((token = strchr(job_str, '.')) != NULL){
         token[0] = '\0';
         token++;
         if (!range_list_parse_from_string(&task_id_range_list, alpp, token,
                                           false, true, INF_NOT_ALLOWED) ||
             !task_id_range_list) {
            ret = -1;
         }
      }

      dbl_value = strtod(job_str, &end_ptr);
      ulng_value = dbl_value;

      if ((dbl_value < 1) || (dbl_value - ulng_value > epsilon) ||
          (end_ptr == NULL) || (end_ptr[0] != '\0')) {
         ret = -1;
      }
   }

   if (arrayDefList != NULL) {
      if (task_id_range_list == NULL) {
         task_id_range_list = lCopyList(lGetListName(arrayDefList), arrayDefList);
      }
      else {
         lList *copy = lCopyList("", arrayDefList);
         lAddList(task_id_range_list, &copy);
      }
   }

   if (ret == 1) {
      if (!include_names && !isdigit(job_str[0]) && (strcmp(job_str, "\"*\"") != 0)) {
         ret = -1;
      } else {
         *idp = lAddElemStr(ipp, ID_str, job_str, ID_Type);
         
         if (*idp) {
            range_list_sort_uniq_compress(task_id_range_list, alpp, true);
            lSetList(*idp, ID_ja_structure, task_id_range_list);
         }
      }
   }
   /*
   ** free the dupped string
   */
   sge_free(&job_str); 
   DRETURN(ret);
}

/****** sgeobj/ja_task/ja_task_message_add() **********************************
*  NAME
*     ja_task_message_add() -- add a message to the message list of a task 
*
*  SYNOPSIS
*     bool 
*     ja_task_message_add(lListElem *this_elem, u_long32 type, 
*                         const char *message) 
*
*  FUNCTION
*     Adds a message in the "JAT_message_list"-message list of "this_elem"
*     "type" will be the message type. "message" is the text string stored
*     int the new element of the sublist.  
*
*  INPUTS
*     lListElem *this_elem - JAT_Type element 
*     u_long32 type        - message type id 
*     const char *message  - message 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: ja_task_message_add() is MT safe 
*
*  SEE ALSO
*     sgeobj/ja_task/ja_task_message_trash_all_of_type_X()
*******************************************************************************/
bool 
ja_task_message_add(lListElem *this_elem, u_long32 type, const char *message)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_message_add");
   ret = object_message_add(this_elem, JAT_message_list, type, message);
   DRETURN(ret);
}

/****** sgeobj/ja_task/ja_task_message_trash_all_of_type_X() ******************
*  NAME
*     ja_task_message_trash_all_of_type_X() -- Trash messages of certain type 
*
*  SYNOPSIS
*     bool 
*     ja_task_message_trash_all_of_type_X(lListElem *this_elem, 
*                                         u_long32 type) 
*
*  FUNCTION
*     Trash all messages from the sublist of JAT_message_list which are of
*     the given "type". 
*     
*
*  INPUTS
*     lListElem *this_elem - JAT_Type element 
*     u_long32 type        - type id 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: ja_task_message_trash_all_of_type_X() is MT safe 
*******************************************************************************/
bool
ja_task_message_trash_all_of_type_X(lListElem *this_elem, u_long32 type)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_message_trash_all_of_type_X");
   ret = object_message_trash_all_of_type_X(this_elem, JAT_message_list, type);
   DRETURN(ret);
}

/****** sge_ja_task/ja_task_verify() *******************************************
*  NAME
*     ja_task_verify() -- verify a ja_task object
*
*  SYNOPSIS
*     bool 
*     ja_task_verify(const lListElem *ja_task, lList **answer_list) 
*
*  FUNCTION
*     Verify correctness of a ja_task object.
*
*  INPUTS
*     const lListElem *ja_task - the ja_task to verify
*     lList **answer_list      - answer list to pass back error messages
*
*  RESULT
*     bool - true: everything ok, else false
*
*  NOTES
*     MT-NOTE: ja_task_verify() is MT safe 
*
*  SEE ALSO
*     sge_ja_task/ja_task_verify_execd_job()
*******************************************************************************/
bool 
ja_task_verify(const lListElem *ja_task, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_verify");

   ret = object_verify_ulong_not_null(ja_task, answer_list, JAT_task_number);

   /* 
    * TODO: JAT_status,
    * TODO: JAT_start_time,
    * TODO: JAT_end_time,
    * TODO: JAT_hold,
    * TODO: JAT_granted_pe,
    * TODO: JAT_job_restarted,
    * TODO: JAT_master_queue,
    * TODO: JAT_state,
    * TODO: JAT_pvm_ckpt_pid,

    * TODO: JAT_pending_signal,
    * TODO: JAT_pending_signal_delivery_time,
    * TODO: JAT_pid,
    * TODO: JAT_osjobid,
    * TODO: JAT_usage_list,

    * TODO: JAT_scaled_usage_list,
    * TODO: JAT_reported_usage_list,
    * TODO: JAT_fshare,
    * TODO: JAT_tix,
    * TODO: JAT_oticket,

    * TODO: JAT_fticket,
    * TODO: JAT_sticket,
    * TODO: JAT_share,
    * TODO: JAT_suitable,
    * TODO: JAT_task_list,
    * TODO: JAT_finished_task_list,

    * TODO: JAT_previous_usage_list,

    * TODO: JAT_pe_object,
    * TODO: JAT_next_pe_task_id,
    * TODO: JAT_stop_initiate_time,
    * TODO: JAT_prio,
    * TODO: JAT_ntix,

    * TODO: JAT_message_list
    */

   DRETURN(ret);
}

/****** sge_ja_task/ja_task_verify_execd_job() *********************************
*  NAME
*     ja_task_verify_execd_job() -- verify a ja_task object for execd
*
*  SYNOPSIS
*     bool 
*     ja_task_verify_execd_job(const lListElem *ja_task, lList **answer_list) 
*
*  FUNCTION
*     Verify a ja_task object that has been sent to execd.
*
*  INPUTS
*     const lListElem *ja_task - the ja_task object to verify
*     lList **answer_list      - answer list to pass back error messages
*
*  RESULT
*     bool - true: everything ok, else false
*
*  NOTES
*     MT-NOTE: ja_task_verify_execd_job() is MT safe 
*
*  SEE ALSO
*     sge_ja_task/ja_task_verify()
*******************************************************************************/
bool 
ja_task_verify_execd_job(const lListElem *ja_task, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_verify_execd_job");

   ret = ja_task_verify(ja_task, answer_list);

   if (ret) {
      ret = ja_task_verify_granted_destin_identifier_list(lGetList(ja_task, JAT_granted_destin_identifier_list), answer_list);
   }

   DRETURN(ret);
}

/****** sge_ja_task/ja_task_verify_granted_destin_identifier_list() ************
*  NAME
*     ja_task_verify_granted_destin_identifier_list() -- verify granted destination identifier list
*
*  SYNOPSIS
*     bool 
*     ja_task_verify_granted_destin_identifier_list(const lList *gdil, 
*                                                   lList **answer_list) 
*
*  FUNCTION
*     Verify correctness of a granted destination identifier list being part 
*     of a scheduled ja_task.
*
*  INPUTS
*     const lList *gdil   - the list to verify
*     lList **answer_list - answer list to pass back error messages
*
*  RESULT
*     bool - true: everything ok, else false
*
*  NOTES
*     MT-NOTE: ja_task_verify_granted_destin_identifier_list() is MT safe 
*
*  SEE ALSO
*     sge_ja_task/ja_task_verify_granted_destin_identifier()
*******************************************************************************/
bool 
ja_task_verify_granted_destin_identifier_list(const lList *gdil, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_verify_granted_destin_identifier_list");

   if (gdil == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              "%s", MSG_INVALID_GDIL);
      ret = false;
   }

   if (ret) {
      const lListElem *ep;

      for_each (ep, gdil) {
         ret = ja_task_verify_granted_destin_identifier(ep, answer_list);
         if (!ret) {
            break;
         }
      }
   }

   DRETURN(ret);
}

/****** sge_ja_task/ja_task_verify_granted_destin_identifier() *****************
*  NAME
*     ja_task_verify_granted_destin_identifier() -- verify a granted destination identifier
*
*  SYNOPSIS
*     bool 
*     ja_task_verify_granted_destin_identifier(const lListElem *ep, 
*                                              lList **answer_list) 
*
*  FUNCTION
*     Verify a single element of a granted destination identifier list
*     (a granted queue with a certain number of slots).
*
*  INPUTS
*     const lListElem *ep - the element to verify
*     lList **answer_list - answer list to pass back error messages
*
*  RESULT
*     bool - true: everything ok, else false
*
*  NOTES
*     MT-NOTE: ja_task_verify_granted_destin_identifier() is MT safe 
*
*  SEE ALSO
*     sge_ja_task/ja_task_verify_granted_destin_identifier_list()
*******************************************************************************/
bool 
ja_task_verify_granted_destin_identifier(const lListElem *ep, lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "ja_task_verify_granted_destin_identifier");

   if (ep == NULL) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR, 
                              MSG_NULLELEMENTPASSEDTO_S, SGE_FUNC);
      ret = false;
   }

   if (ret) {
      ret = qinstance_verify_full_name(answer_list, lGetString(ep, JG_qname));
   }

   if (ret) {
      ret = verify_host_name(answer_list, lGetHost(ep, JG_qhostname));
   }

   if (ret) {
      ret = object_verify_ulong_not_null(ep, answer_list, JG_slots);
   }

   if (ret) {
      const lListElem *queue = lGetObject(ep, JG_queue);

      if (queue != NULL) {
         ret = qinstance_verify(queue, answer_list);
      }
   }

   DRETURN(ret);
}

/****** sge_ja_task/ja_task_is_tightly_integrated() ****************************
*  NAME
*     ja_task_is_tightly_integrated() -- is this a tightly integrated job?
*
*  SYNOPSIS
*     bool ja_task_is_tightly_integrated(const lListElem *ja_task) 
*
*  FUNCTION
*     Figures out, if a running ja task belongs to a tightly integrated
*     parallel job.
*
*  INPUTS
*     const lListElem *ja_task - the ja task to test
*
*  RESULT
*     bool - true if it is a tightly integrated parallel job, else false
*
*  NOTES
*     MT-NOTE: ja_task_is_tightly_integrated() is MT safe, the caller must
*              hold a read lock on the SGE_TYPE_PE list.
*******************************************************************************/
bool ja_task_is_tightly_integrated(const lListElem *ja_task)
{
   bool ret = false;
   object_description *object_base = object_type_get_object_description();

   if (ja_task != NULL) {
      const char *pe_name = lGetString(ja_task, JAT_granted_pe);
      if (pe_name != NULL) {
         const lListElem *pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, pe_name);
         if (pe != NULL) {
            if (lGetBool(pe, PE_control_slaves)) {
               ret = true;
            }
         }
      }
   }

   return ret;
}

