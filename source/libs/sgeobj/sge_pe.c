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

#include <fnmatch.h>
#include <strings.h>
#include <ctype.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/config_file.h"
#include "uti/sge_string.h"

#include "cull/cull_list.h"

#include "sched/sge_resource_utilization.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_resource_utilization_RUE_L.h"
#include "sgeobj/msg_sgeobjlib.h"
#ifdef SGE_PQS_API
#include "uti/sge_dlopen.h"
#endif

#include "sge.h"
#include "msg_common.h"
#include "msg_qmaster.h"

#define SGE_ATTRVAL_MIN                "min"
#define SGE_ATTRVAL_MAX                "max"
#define SGE_ATTRVAL_AVG                "avg"

bool pe_name_is_matching(const char *pe_name, const char *wildcard)
{
   return fnmatch(wildcard, pe_name, 0) == 0 ? true : false;
}

/****** sgeobj/pe/pe_is_matching() ********************************************
*  NAME
*     pe_is_matching() -- Does Pe name match the wildcard? 
*
*  SYNOPSIS
*     bool pe_is_matching(const lListElem *pe, const char *wildcard) 
*
*  FUNCTION
*     The function returns true (1) if the name of the given
*     "pe" matches the "wildcard". 
*
*  INPUTS
*     const lListElem *pe  - PE_Type element 
*     const char *wildcard - wildcard 
*
*  RESULT
*     bool - true or false 
******************************************************************************/
bool pe_is_matching(const lListElem *pe, const char *wildcard) 
{
   return pe_name_is_matching(lGetString(pe, PE_name), wildcard);
}

/****** sgeobj/pe/pe_list_find_matching() *************************************
*  NAME
*     pe_list_find_matching() -- Find a PE matching  wildcard expr 
*
*  SYNOPSIS
*     const lListElem* pe_list_find_matching(lList *pe_list, 
*                                      const char *wildcard) 
*
*  FUNCTION
*     Try to find a PE that matches the given "wildcard" expression.
*
*  INPUTS
*     const lList *pe_list       - PE_Type list
*     const char *wildcard - Wildcard expression 
*
*  RESULT
*     lListElem* - PE_Type object or NULL
*******************************************************************************/
lListElem *pe_list_find_matching(const lList *pe_list, const char *wildcard) 
{
   lListElem *ret = NULL;

   for_each (ret, pe_list) {
      if (pe_is_matching(ret, wildcard)) {
         break;
      }
   }
   return ret;
}

/****** sgeobj/pe/pe_list_locate() ********************************************
*  NAME
*     pe_list_locate() -- Locate a certain PE 
*
*  SYNOPSIS
*     lListElem* pe_list_locate(lList *pe_list, const char *pe_name) 
*
*  FUNCTION
*     Locate the PE with the name "pe_name". 
*
*  INPUTS
*     lList *pe_list      - PE_Type list
*     const char *pe_name - PE name 
*
*  RESULT
*     lListElem* - PE_Type object or NULL
* 
*  NOTES
*     MT-NOTE: pe_list_locate() is MT safe
******************************************************************************/
lListElem *pe_list_locate(const lList *pe_list, const char *pe_name) 
{
   return lGetElemStr(pe_list, PE_name, pe_name);
}

/****** sgeobj/pe/pe_is_referenced() ******************************************
*  NAME
*     pe_is_referenced() -- Is a given PE referenced in other objects? 
*
*  SYNOPSIS
*     bool pe_is_referenced(const lListElem *pe, lList **answer_list, 
*                           const lList *master_job_list,
*                           const lList *master_cqueue_list) 
*
*  FUNCTION
*     This function returns true (1) if the given "pe" is referenced
*     in at least one of the objects contained in "master_job_list"
*     or "master_cqueue_list". If this is the case than
*     a corresponding message will be added to the "answer_list".
*
*  INPUTS
*     const lListElem *pe             - PE_Type object 
*     lList **answer_list             - AN_Type list 
*     const lList *master_job_list    - JB_Type list 
*     const lList *master_cqueue_list - CQ_Type list
*
*  RESULT
*     bool - true or false  
******************************************************************************/
bool pe_is_referenced(const lListElem *pe, lList **answer_list,
                      const lList *master_job_list,
                      const lList *master_cqueue_list)
{
   bool ret = false;

   {
      lListElem *job = NULL;

      for_each(job, master_job_list) {
         if (job_is_pe_referenced(job, pe)) {
            const char *pe_name = lGetString(pe, PE_name);
            u_long32 job_id = lGetUlong(job, JB_job_number);

            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                    ANSWER_QUALITY_INFO, MSG_PEREFINJOB_SU,
                                    pe_name, sge_u32c(job_id));
            ret = true;
            break;
         }
      } 
   }
   if (!ret) {
      lListElem *cqueue = NULL, *cpl = NULL;

      /* fix for bug 6422335
       * check cq configuration for pe references instead of qinstances
       */
      const char *pe_name = lGetString(pe, PE_name);

      for_each(cqueue, master_cqueue_list) {
         for_each(cpl, lGetList(cqueue, CQ_pe_list)){
            if (lGetSubStr(cpl, ST_name, pe_name, ASTRLIST_value))  {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_INFO, 
                                       MSG_PEREFINQUEUE_SS, 
                                       pe_name, lGetString(cqueue, CQ_name));
               ret = true;
               break;
            }
         }
      }
   }
   return ret;
}

/****** sgeobj/pe/pe_validate() ***********************************************
*  NAME
*     pe_validate() -- validate a parallel environment
*
*  SYNOPSIS
*     int pe_validate(int startup, lListElem *pep, lList **alpp)
*
*  FUNCTION
*     Ensures that a new pe is not a duplicate of an already existing one
*     and checks consistency of the parallel environment:
*        - pseudo parameters in start and stop proc
*        - validity of the allocation rule
*        - correctness of the queue list, the user list and the xuser list
*
*
*  INPUTS
*     lListElem *pep - the pe to check
*     lList **alpp   - answer list pointer, if an answer shall be created, else
*                      NULL - errors will in any case be output using the
*                      Grid Engine error logging macros.
*     int startup    - are we in qmaster startup phase?
*
*  RESULT
*     int - STATUS_OK, if everything is ok, else other status values,
*           see libs/gdi/sge_answer.h
*
*  NOTES
*     MT-NOTE: pe_validate() is not MT safe
*******************************************************************************/
int pe_validate(lListElem *pep, lList **alpp, int startup)
{
   const char *s;
   const char *pe_name;
   int ret;

   DENTER(TOP_LAYER, "pe_validate");
   pe_name = lGetString(pep, PE_name);
   if (pe_name != NULL && verify_str_key(alpp, pe_name, MAX_VERIFY_STRING, MSG_OBJ_PE, KEY_TABLE) != STATUS_OK) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_PE_INVALIDCHARACTERINPE_S, pe_name));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_PE_INVALIDCHARACTERINPE_S, pe_name);
      }
      DEXIT;
      return STATUS_EEXIST;
   }

   /* register our error function for use in replace_params() */
   config_errfunc = set_error;

   /* -------- start_proc_args */
   NULL_OUT_NONE(pep, PE_start_proc_args);
   s = lGetString(pep, PE_start_proc_args);
   if (s != NULL && replace_params(s, NULL, 0, pe_variables)) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_PE_STARTPROCARGS_SS, pe_name, err_msg));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_PE_STARTPROCARGS_SS, pe_name, err_msg);
      }
      DEXIT;
      return STATUS_EEXIST;
   }

   /* -------- stop_proc_args */
   NULL_OUT_NONE(pep, PE_stop_proc_args);
   s = lGetString(pep, PE_stop_proc_args);
   if (s != NULL && replace_params(s, NULL, 0, pe_variables)) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_PE_STOPPROCARGS_SS, pe_name, err_msg));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_PE_STOPPROCARGS_SS, pe_name, err_msg); 
      }
      DEXIT;
      return STATUS_EEXIST;
   }

   /* -------- slots */
   if ((ret=pe_validate_slots(alpp, lGetUlong(pep, PE_slots))) != STATUS_OK) {
      DEXIT;
      return ret;
   }

   /* -------- allocation_rule */
   s = lGetString(pep, PE_allocation_rule);
   if (s == NULL) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
               lNm2Str(PE_allocation_rule), "validate_pe"));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_SGETEXT_MISSINGCULLFIELD_SS,
                                 lNm2Str(PE_allocation_rule), "validate_pe");
      }
      DEXIT;
      return STATUS_EEXIST;
   }

   if (replace_params(s, NULL, 0, pe_alloc_rule_variables)) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_PE_ALLOCRULE_SS, pe_name, err_msg));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_PE_ALLOCRULE_SS, pe_name, err_msg);
      }
      DEXIT;
      return STATUS_EEXIST;
   }

   /* do this only in qmaster. we don't have the usersets in qconf */
   if (startup) {
      /* -------- PE_user_list */
      if ((ret=userset_list_validate_acl_list(lGetList(pep, PE_user_list), alpp))!=STATUS_OK) {
         DEXIT;
         return ret;
      }

      /* -------- PE_xuser_list */
      if ((ret=userset_list_validate_acl_list(lGetList(pep, PE_xuser_list), alpp))!=STATUS_OK) {
         DEXIT;
         return ret;
      }
   }

   /* -------- PE_urgency_slots */
   if ((ret=pe_validate_urgency_slots(alpp, lGetString(pep, PE_urgency_slots)))!=STATUS_OK) {
      DEXIT;
      return ret;
   }

#ifdef SGE_PQS_API
   /* -------- PE_qsort_args */
   NULL_OUT_NONE(pep, PE_qsort_args);
   if (startup) {
      void *handle=NULL, *fn=NULL;
      const char *qsort_args = lGetString(pep, PE_qsort_args);
      if (qsort_args) {
         if ((ret=pe_validate_qsort_args(alpp, qsort_args, pep,
                     &handle, &fn))!=STATUS_OK) {
            DEXIT;
            return ret;
         }
         /* lSetRef(pep, PE_qsort_validated, 1); */
      }
   }
#endif

   DEXIT;
   return STATUS_OK;
}

/****** sgeobj/pe/pe_validate_slots() *********************************
*  NAME
*     pe_validate_slots() -- Ensure urgency slot setting is valid.
*
*  SYNOPSIS
*     int pe_validate_slots(lList **alpp, u_long32 slots) 
*
*  FUNCTION
*     Validates slot setting.
*
*  INPUTS
*     lList **alpp   - On error a context message is returned.
*     u_long32 slots - The slots value.
*
*  RESULT
*     int - values other than STATUS_OK indicate error condition 
*
*  NOTES
*     MT-NOTE: pe_validate_slots() is MT safe
*******************************************************************************/
int pe_validate_slots(lList **alpp, u_long32 slots)
{
   DENTER(TOP_LAYER, "pe_validate_slots");

   if (slots > RANGE_INFINITY) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, MSG_ATTR_INVALID_ULONGVALUE_USUU, sge_u32c(slots), 
                "slots", sge_u32c(0), sge_u32c(RANGE_INFINITY)));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 MSG_ATTR_INVALID_ULONGVALUE_USUU, sge_u32c(slots), 
                                 "slots", sge_u32c(0), sge_u32c(RANGE_INFINITY));
      }
      DEXIT;
      return STATUS_ESEMANTIC;
   }

   DEXIT;
   return STATUS_OK;
}
/****** sgeobj/pe/pe_validate_urgency_slots() *********************************
*  NAME
*     pe_validate_urgency_slots() -- Ensure urgency slot setting is valid.
*
*  SYNOPSIS
*     int pe_validate_urgency_slots(lList **alpp, const char *s) 
*
*  FUNCTION
*     Validates urgency slot setting.
*
*  INPUTS
*     lList **alpp  - On error a context message is returned.
*     const char *s - The urgency slot string to be validated.
*
*  RESULT
*     int - values other than STATUS_OK indicate error condition 
*
*  NOTES
*     MT-NOTE: pe_validate_urgency_slots() is MT safe
*******************************************************************************/
int pe_validate_urgency_slots(lList **answer_list, const char *s)
{
   DENTER(TOP_LAYER, "pe_validate_urgency_slots");

   if (strcasecmp(s, SGE_ATTRVAL_MIN) &&
       strcasecmp(s, SGE_ATTRVAL_MAX) &&
       strcasecmp(s, SGE_ATTRVAL_AVG) &&
       !isdigit(s[0])) {
      if (answer_list == NULL) {
         ERROR((SGE_EVENT, "rejecting invalid urgency_slots setting \"%s\"\n", 
                s));
      } else {
         answer_list_add_sprintf(answer_list, STATUS_EEXIST, ANSWER_QUALITY_ERROR, MSG_PE_REJECTINGURGENCYSLOTS_S, s); 
      }
      DEXIT;
      return STATUS_ESEMANTIC;
   }

   DEXIT;
   return STATUS_OK;
}

/****** sgeobj/pe/pe_list_do_all_exist() **************************************
*  NAME
*     pe_list_do_all_exist() -- Check if a list of PE's really exists 
*
*  SYNOPSIS
*     bool 
*     pe_list_do_all_exist(const lList *pe_list, 
*                          lList **answer_list, 
*                          const lList *pe_ref_list, 
*                          bool ignore_make_pe) 
*
*  FUNCTION
*     Check if all PE's in "pe_ref_list" really exist in "pe_list".
*     If "ignore_make_pe" is 'true' than the test for the PE with the 
*     name "make" will not be done.
*
*  INPUTS
*     const lList *pe_list     - PE_Type list 
*     lList **answer_list      - AN_Type list 
*     const lList *pe_ref_list - ST_Type list of PE names 
*     bool ignore_make_pe      - bool 
*
*  RESULT
*     bool 
*        true  - if all PE's exist
*        false - if at least one PE does not exist
*
*  NOTES
*     MT-NOTE: pe_urgency_slots() is MT safe
*******************************************************************************/
bool pe_list_do_all_exist(const lList *pe_list, lList **answer_list,
                          const lList *pe_ref_list, bool ignore_make_pe)
{
   bool ret = true;
   lListElem *pe_ref_elem = NULL;

   DENTER(TOP_LAYER, "pe_list_do_all_exist");
   for_each(pe_ref_elem, pe_ref_list) {
      const char *pe_ref_string = lGetString(pe_ref_elem, ST_name);

      if (ignore_make_pe && !strcmp(pe_ref_string, "make")) { 
         continue;
      }
      if (pe_list_locate(pe_list, pe_ref_string) == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EEXIST, 
                                 ANSWER_QUALITY_ERROR, 
                                 MSG_PEREFDOESNOTEXIST_S, pe_ref_string);
         ret = false; 
         break;
      }
   }
   DEXIT;
   return ret;
}

/****** sgeobj/pe/pe_urgency_slots() ******************************************
*  NAME
*     pe_urgency_slots() -- Compute PEs urgency slot amount for a slot range
*
*  SYNOPSIS
*     int pe_urgency_slots(const lListElem *pe, 
*                          const char *urgency_slot_setting, 
*                          const lList* range_list) 
*
*  FUNCTION
*     Compute PEs urgency slot amount for a slot range. The urgency slot
*     amount is the amount that is assumed for a job with a slot range
*     before an assignment.
*
*  INPUTS
*     const lListElem *pe              - PE_Type object.
*     const char *urgency_slot_setting - Ugency slot setting as in sge_pe(5)
*     const lList* range_list          - RN_Type list.
*
*  RESULT
*     int - The slot amount.
*
*  NOTES
*     MT-NOTE: pe_urgency_slots() is MT safe
*******************************************************************************/
int 
pe_urgency_slots(const lListElem *pe, const char *urgency_slot_setting, 
                 const lList* range_list)
{
   int n;

   DENTER(TOP_LAYER, "pe_urgency_slots");

   if (!strcasecmp(urgency_slot_setting, SGE_ATTRVAL_MIN)) {
      n = range_list_get_first_id(range_list, NULL);
   } else if (!strcasecmp(urgency_slot_setting, SGE_ATTRVAL_MAX)) {
      /* 
       * in case of an infinity slot range we use the 
       * maximum PE slot number instead 
       */
      n = range_list_get_last_id(range_list, NULL);
      if (n == RANGE_INFINITY) {
         n = lGetUlong(pe, PE_slots);
      }
   } else if (!strcasecmp(urgency_slot_setting, SGE_ATTRVAL_AVG)) {
      /* 
       * to handle infinity slot ranges we use the maximum PE 
       * slot number as upper bound when determining the average 
       */
      n = range_list_get_average(range_list, lGetUlong(pe, PE_slots));
   } else if (isdigit(urgency_slot_setting[0])) {
      n = atoi(urgency_slot_setting);
   } else {
      CRITICAL((SGE_EVENT, MSG_PE_UNKNOWN_URGENCY_SLOT_SS, urgency_slot_setting, lGetString(pe, PE_name)));
      n = 1;
   }
   DEXIT;
   return n;
}

/****** sgeobj/pe/pe_create_template() ****************************************
*
*  NAME
*     pe_create_template -- build up a generic pe object 
*
*  SYNOPSIS
*     lListElem *pe_create_template(char *pe_name);
*
*  FUNCTION
*     build up a generic pe object
*
*  INPUTS
*     pe_name - name used for the PE_name attribute of the generic
*               pe object. If NULL then "template" is the default name.
*
*  RESULT
*     !NULL - Pointer to a new CULL object of type PE_Type
*     NULL - Error
*
*  NOTES
*     MT-NOTE: pe_set_slots_used() is MT safe 
*******************************************************************************/
lListElem* pe_create_template(char *pe_name)
{
   lListElem *pep;

   DENTER(TOP_LAYER, "pe_create_template");

   pep = lCreateElem(PE_Type);

   if (pe_name) {
      lSetString(pep, PE_name, pe_name);
   } else {
      lSetString(pep, PE_name, "template");
   }

   lSetString(pep, PE_allocation_rule, "$pe_slots");
   lSetString(pep, PE_start_proc_args, "NONE");
   lSetString(pep, PE_stop_proc_args, "NONE");

   /* Fixme:  This should probably be flipped.  */
   /* PE_control_slaves initialized implicitly to false */
   lSetBool(pep, PE_job_is_first_task, TRUE);

   lSetString(pep, PE_urgency_slots, SGE_ATTRVAL_MIN);

#ifdef SGE_PQS_API
   lSetString(pep, PE_qsort_args, NULL);
#endif

   DEXIT;
   return pep;
}

/****** sgeobj/pe/pe_slots_used() *********************************************
*  NAME
*     pe_get_slots_used() -- Returns used PE slots
*
*  SYNOPSIS
*     int pe_get_slots_used(const lListElem *pe) 
*
*  FUNCTION
*     Returns the number of currently used PE slots.
*
*  INPUTS
*     const lListElem *pe - The PE object (PE_Type)
*
*  RESULT
*     int - number of currently used PE slots or -1 on error
*
*  NOTES
*     MT-NOTE: pe_get_slots_used() is MT safe 
*******************************************************************************/
int pe_get_slots_used(const lListElem *pe)
{
   int ret = -1;
   const lListElem *actual = lGetSubStr(pe, RUE_name, SGE_ATTR_SLOTS, 
                                        PE_resource_utilization);

   if (actual) {
      ret = lGetDouble(actual, RUE_utilized_now);
   }
   return ret; 
}

/****** sgeobj/pe/pe_set_slots_used() *****************************************
*  NAME
*     pe_set_slots_used() -- Set number of used PE slots
*
*  SYNOPSIS
*     int pe_set_slots_used(lListElem *pe, int slots) 
*
*  FUNCTION
*     Sets the number of used PE slots.
*
*  INPUTS
*     lListElem *pe - The pe object (PE_Type) 
*     int slots     - Number of slots.
*
*  RESULT
*     int - 0 on success -1 on error
*
*  NOTES
*     MT-NOTE: pe_set_slots_used() is MT safe 
*******************************************************************************/
int pe_set_slots_used(lListElem *pe, int slots)
{
   lListElem *actual = lGetSubStr(pe, RUE_name, SGE_ATTR_SLOTS, 
                                  PE_resource_utilization);
   if (!actual && (!(actual = 
         lAddSubStr(pe, RUE_name, SGE_ATTR_SLOTS, PE_resource_utilization, RUE_Type))))
      return -1;
   lSetDouble(actual, RUE_utilized_now, slots); 
   return 0;
}


/****** sgeobj/pe/pe_debit_slots() ********************************************
*  NAME
*     pe_debit_slots() -- Debit pos/neg number of slots from PE
*
*  SYNOPSIS
*     void pe_debit_slots(lListElem *pep, int slots, u_long32 job_id) 
*
*  FUNCTION
*     Increases or decreses the number of slots used with a PE.
*
*  INPUTS
*     lListElem *pep  - The PE (PE_Type)
*     int slots       - Pos/neg number of slots.
*     u_long32 job_id - Job id for monitoring purposes.
*
*  NOTES
*     MT-NOTE: pe_debit_slots() is MT safe 
*******************************************************************************/
void pe_debit_slots(lListElem *pep, int slots, u_long32 job_id) 
{
   int n;

   DENTER(TOP_LAYER, "pe_debit_slots");

   if (pep) {
      n = pe_get_slots_used(pep);
      n += slots;
      if (n < 0) {
         ERROR((SGE_EVENT, MSG_PE_USEDSLOTSTOOBIG_S, lGetString(pep, PE_name)));
      }
      pe_set_slots_used(pep, n);
   }
   DEXIT;
   return;
}


#ifdef SGE_PQS_API
/****** sgeobj/pe/pe_validate_qsort_args() *********************************
*  NAME
*     pe_validate_qsort_args() -- Ensures the qsort_args setting is valid
*     by verifying that the dynamic link library can be loaded. This function
*     is presumably called during startup and whenever a PE is created
*     or modified.
*
*  SYNOPSIS
*     int pe_validate_qsort_args(lList **alpp, const char *qsort_args, 
*           lListElem *pe, void **lib, void **fn) 
*
*  FUNCTION
*     Validates urgency slot setting.
*
*  INPUTS
*     lList **alpp  - On error a context message is returned.
*     const char *qsort_args - The qsort_args string
*     lListElem *pe - The new or existing PE
*     void **lib - The returned handle of the dynamically linked library
*     void **fn - The returned address of the dynamically linked function
*
*  RESULT
*     int - values other than STATUS_OK indicate error condition 
*
*  NOTES
*     MT-NOTE: pe_validate_qsort_args() is not MT safe
*******************************************************************************/
int pe_validate_qsort_args(lList **alpp, const char *qsort_args, lListElem *pe,
      void **lib, void **fn)
{
   const char *old_qsort_args = lGetString(pe, PE_qsort_args);
   char *lib_name, *fn_name;
   struct saved_vars_s *cntx = NULL;
   void *lib_handle=NULL;
   int ret = STATUS_OK;
   const char *error;

   DENTER(TOP_LAYER, "pe_validate_qsort_args");

   /*
    * If we already have already validated the function and the old and new qsort_args
    * are the same, then we're done
    */
   if ( /* lGetUlong(pe, PE_qsort_validated) && */
             old_qsort_args && qsort_args && strcmp(old_qsort_args, qsort_args)==0) {
      goto cleanup;
   }

   /* if new args are blank, then we go close the old library */
   if (!qsort_args)
      goto cleanup;

   /* get library name */
   lib_name = sge_strtok_r(qsort_args, " ", &cntx); 
   if (!lib_name) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, "No d2yyynamic library specified for pe_qsort_args for PE %s\n",
                lGetString(pe, PE_name)));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR, 
                                 MSG_PQS_NODYNAMICLIBRARY_S, lGetString(pe, PE_name));
      }
      ret = STATUS_EEXIST;
      goto cleanup;
   }

   /* open library */
   lib_handle = sge_dlopen(lib_name, NULL);
   if (!lib_handle) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, "Unable to open %s library in pe_qsort_args for PE %s - %s\n",
              lib_name, lGetString(pe, PE_name), dlerror()));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR, 
                                 MSG_PQS_UNABLETOOPENLIBRARY_SSS, lib_name, 
                                 lGetString(pe, PE_name), dlerror());
      }
      ret = STATUS_EEXIST;
      goto cleanup;
   }

   /* get function name */
   fn_name = sge_strtok_r(NULL, " ", &cntx);
   if (!fn_name) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, "No function name specified in pe_qsort_args for PE %s \n",
              lGetString(pe, PE_name)));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR, 
                                 MSG_PQS_NOFUNCTIONNAME_S, lGetString(pe, PE_name));
      }
      ret = STATUS_EEXIST;
      goto cleanup;
   }

   /* lookup function address */
   error = dlerror();           /* clear status */
   dlsym(lib_handle, fn_name);
   if ((error = dlerror()) != NULL) {
      if (alpp == NULL) {
         ERROR((SGE_EVENT, "Unable to locate %s symbol in %s library for pe_qsort_args in PE %s - %s\n",
              fn_name, lib_name, lGetString(pe, PE_name), error));
      } else {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR, 
                                 MSG_PQS_UNABLELOCATESYMBOL_SSSS, fn_name, 
                                 lib_name, lGetString(pe, PE_name), error);
      }
      ret = STATUS_EEXIST;
      goto cleanup;
   }

cleanup:

   if (cntx)
      sge_free_saved_vars(cntx);

   if (lib_handle)
      dlclose(lib_handle);

   return ret;
}
#endif

/****** sge_pe/pe_do_accounting_summary() **************************************
*  NAME
*     pe_do_accounting_summary() -- do accounting summary?
*
*  SYNOPSIS
*     bool 
*     pe_do_accounting_summary(const lListElem *pe)
*
*  FUNCTION
*     Returns whether for tightly integrated jobs
*     - a single accounting record shall be created (true), or
*     - an individual accounting record shall be created for every task,
*       and an accounting record for the master task (job script).
*
*  INPUTS
*     const lListElem *pe - the parallel environment
*
*  RESULT
*     bool - true (do summary), or false (many records)
*
*  NOTES
*     MT-NOTE: pe_do_accounting_summary() is MT safe 
*******************************************************************************/
bool
pe_do_accounting_summary(const lListElem *pe)
{
   bool ret = false;

   /*
    * the accounting_summary attribute has only a meaning
    * for tightly integrated jobs
    */
   if (pe != NULL && lGetBool(pe, PE_control_slaves)) {
      ret = lGetBool(pe, PE_accounting_summary) ? true : false;
   }

   return ret;
}

