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
#include <errno.h>
#include <limits.h>
#include <fnmatch.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_stdio.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "sge.h"
#include "symbols.h"
#include "msg_qmaster.h"

/****** sge_advance_reservation/ar_list_locate() *******************************
*  NAME
*     ar_list_locate() -- locate an advance reservation by id
*
*  SYNOPSIS
*     lListElem* ar_list_locate(lList *ar_list, u_long32 ar_id) 
*
*  FUNCTION
*     This function returns an AR object with the selected id from the
*     given list.
*
*  INPUTS
*     lList *ar_list - list to be searched in
*     u_long32 ar_id - id of interest
*
*  RESULT
*     lListElem* - if found the reference to the AR object, else NULL
*
*  NOTES
*     MT-NOTE: ar_list_locate() is MT safe 
*******************************************************************************/
lListElem *ar_list_locate(lList *ar_list, u_long32 ar_id)
{
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "ar_list_locate");

   ep = lGetElemUlong(ar_list, AR_id, ar_id);

   DRETURN(ep);
}

/****** sge_advance_reservation/ar_validate() **********************************
*  NAME
*     ar_validate() -- validate an advance reservation
*
*  SYNOPSIS
*     bool ar_validate(lListElem *ar, lList **alpp, bool in_master)
*
*  FUNCTION
*     Ensures a new AR has valid start and end times
*
*  INPUTS
*     lListElem *ar   - the AR to check
*     lList **alpp    - answer list pointer
*     bool in_master  - are we in qmaster?
*     bool is_spool   - do we validate for spooling? 
*
*  RESULT
*     bool - true if OK, else false
*
*  NOTES
*     MT-NOTE: ar_validate() is MT safe
*******************************************************************************/
bool ar_validate(lListElem *ar, lList **alpp, bool in_master, bool is_spool)
{
   u_long32 start_time;
   u_long32 end_time;
   u_long32 duration;
   u_long32 now = sge_get_gmt();
   object_description *object_base = object_type_get_object_description();
   
   DENTER(TOP_LAYER, "ar_validate");

   /*   AR_start_time, SGE_ULONG        */
   if ((start_time = lGetUlong(ar, AR_start_time)) == 0) {
      start_time = now;
      lSetUlong(ar, AR_start_time, start_time);
   }

   /*   AR_end_time, SGE_ULONG        */
   end_time = lGetUlong(ar, AR_end_time);
   duration = lGetUlong(ar, AR_duration);
   
   if (end_time == 0 && duration == 0) {
      answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                              MSG_AR_MISSING_VALUE_S, "end time or duration");
      goto ERROR;
   } else if (end_time == 0) {
      end_time = duration_add_offset(start_time, duration);
      duration = end_time  - start_time;
      lSetUlong(ar, AR_end_time, end_time);
      lSetUlong(ar, AR_duration, duration);
   } else if (duration == 0) {
      duration = end_time - start_time;
      lSetUlong(ar, AR_duration, duration);
   }

   if ((end_time - start_time) != duration) {
      answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                              "%s", MSG_AR_START_END_DURATION_INVALID);
      goto ERROR;
   }

   if (start_time > end_time) {
      answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                              "%s", MSG_AR_START_LATER_THAN_END);
      goto ERROR;
   }
   
   if (!is_spool) {
      if (start_time < now) {
         answer_list_add_sprintf(alpp, STATUS_EEXIST, ANSWER_QUALITY_ERROR,
                                 "%s", MSG_AR_START_IN_PAST);
         goto ERROR;
      }
   }
   /*   AR_owner, SGE_STRING */
   
   if (in_master) {
      /*    AR_name, SGE_STRING */
      NULL_OUT_NONE(ar, AR_name);
      if (object_verify_name(ar, alpp, AR_name, SGE_OBJ_AR)) {
         goto ERROR;
      }
      /*   AR_account, SGE_STRING */
      NULL_OUT_NONE(ar, AR_account);
      if (!lGetString(ar, AR_account)) {
         lSetString(ar, AR_account, DEFAULT_ACCOUNT);
      } else {
         if (verify_str_key(alpp, lGetString(ar, AR_account), MAX_VERIFY_STRING,
         "account string", QSUB_TABLE) != STATUS_OK) {
            goto ERROR;
         }
      }
      /*   AR_verify, SGE_ULONG              just verify the reservation or final case */
      /*   AR_error_handling, SGE_ULONG      how to deal with soft and hard exceptions */
      /*   AR_checkpoint_name, SGE_STRING    Named checkpoint */
      NULL_OUT_NONE(ar, AR_checkpoint_name);
      {
         /* request for non existing ckpt object will be refused */
         const char *ckpt_name = NULL;

         ckpt_name = lGetString(ar, AR_checkpoint_name);
         if (ckpt_name != NULL) {
            lList *master_ckpt_list = *object_base[SGE_TYPE_CKPT].list;
            lListElem *ckpt_ep = ckpt_list_locate(master_ckpt_list, ckpt_name);
            if (!ckpt_ep) {
               ERROR((SGE_EVENT, MSG_JOB_CKPTUNKNOWN_S, ckpt_name));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               goto ERROR;
            }
          }
      }
      /*   AR_resource_list, SGE_LIST */
      {
         lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;

         if (centry_list_fill_request(lGetList(ar, AR_resource_list),
                                      alpp, master_centry_list, false, true,
                                      false)) {
            goto ERROR;
         }
         if (compress_ressources(alpp, lGetList(ar, AR_resource_list), SGE_OBJ_AR)) {
            goto ERROR;
         }
         
         if (!centry_list_is_correct(lGetList(ar, AR_resource_list), alpp)) {
            goto ERROR;
         }
      }
      /*   AR_queue_list, SGE_LIST */
      if (!qref_list_is_valid(lGetList(ar, AR_queue_list), alpp)) {
         goto ERROR;
      }
      /*   AR_mail_options, SGE_ULONG   */
      /*   AR_mail_list, SGE_LIST */
      
      /*   AR_master_queue_list  -masterq wc_queue_list, SGE_LIST bind master task to queue(s) */
      if (!qref_list_is_valid(lGetList(ar, AR_master_queue_list), alpp)) {
         goto ERROR;
      }
       
      
      /*   AR_pe, SGE_STRING,  AR_pe_range, SGE_LIST */
      NULL_OUT_NONE(ar, AR_pe);
      {
         const char *pe_name = NULL;
         lList *pe_range = NULL;
         
         pe_name = lGetString(ar, AR_pe);
         if (pe_name) {
            const lListElem *pep;
            pep = pe_list_find_matching(*object_base[SGE_TYPE_PE].list, pe_name);
            if (!pep) {
               ERROR((SGE_EVENT, MSG_JOB_PEUNKNOWN_S, pe_name));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               goto ERROR;
            }
            /* check pe_range */
            pe_range = lGetList(ar, AR_pe_range);
            if (object_verify_pe_range(alpp, pe_name, pe_range, SGE_OBJ_AR)!=STATUS_OK) {
               goto ERROR;
            }
         }
      }

      /*   AR_acl_list, SGE_LIST */
      if (userset_list_validate_access(lGetList(ar, AR_acl_list), ARA_name, alpp) != STATUS_OK) {
         goto ERROR;
      }
      
      /*   AR_xacl_list, SGE_LIST */
      if (userset_list_validate_access(lGetList(ar, AR_xacl_list), ARA_name, alpp) != STATUS_OK) {
         goto ERROR;
      }

      if (is_spool) {
         lListElem *jg;
         dstring cqueue_buffer = DSTRING_INIT;
         dstring hostname_buffer = DSTRING_INIT;
         for_each(jg, lGetList(ar, AR_granted_slots)){
            const char *hostname = NULL;
            const char *qname = lGetString(jg, JG_qname);
            bool has_hostname = false;
            bool has_domain = false;

            cqueue_name_split(qname, &cqueue_buffer, &hostname_buffer,
                              &has_hostname, &has_domain);
            hostname = sge_dstring_get_string(&hostname_buffer);
            lSetHost(jg, JG_qhostname, hostname);
         }
         sge_dstring_free(&cqueue_buffer);
         sge_dstring_free(&hostname_buffer);
      }
      /*   AR_type,  SGE_ULONG     */
      /*   AR_state, SGE_ULONG               state of the AR */
      if(lGetUlong(ar, AR_state) == ARL_UNKNOWN){
         lSetUlong(ar, AR_state, ARL_CREATION);  
      }
   }
   DRETURN(true);

ERROR:
   DRETURN(false);
}

/****** libs/sge_obj/ar_get_event_from_string() ******************************
*  NAME
*     ar_get_event_from_string() -- converts a string to a event id 
*
*  SYNOPSIS
*     ar_state_event_t ar_get_event_from_string(const char *string) 
*
*  FUNCTION
*     Converts a human readable event string to the corresponding
*     event if. 
*
*  INPUTS
*     const char *string - string 
*
*  RESULT
*     ar_state_event_t - the event id 
*
*  NOTES
*     MT-NOTE: ar_get_event_from_string() is not MT safe 
*******************************************************************************/
ar_state_event_t
ar_get_event_from_string(const char *string)
{
   ar_state_event_t ret = ARL_UNKNOWN;

   DENTER(TOP_LAYER, "ar_get_event_from_string");
   if (string != NULL) {
      if (!strcmp(MSG_AR_EVENT_STATE_UNKNOWN, string)) {
         ret = ARL_UNKNOWN;
      } else if (!strcmp(MSG_AR_EVENT_STATE_CREATION, string)) {
         ret = ARL_CREATION;
      } else if (!strcmp(MSG_AR_EVENT_STATE_STARTIME_REACHED, string)) {
         ret = ARL_STARTTIME_REACHED;
      } else if (!strcmp(MSG_AR_EVENT_STATE_ENDTIME_REACHED, string)) {
         ret = ARL_ENDTIME_REACHED;
      } else if (!strcmp(MSG_AR_EVENT_STATE_UNSATISFIED, string)) {
         ret = ARL_UNSATISFIED;
      } else if (!strcmp(MSG_AR_EVENT_STATE_OK, string)) {
         ret = ARL_OK;
      } else if (!strcmp(MSG_AR_EVENT_STATE_TERMINATED, string)) {
         ret = ARL_TERMINATED;
      } 
   } 
   DRETURN(ret);
}

/****** libs/sgeobj/ar_get_string_from_event() ********************************
*  NAME
*     ar_get_string_from_event() -- converts a state event to a string 
*
*  SYNOPSIS
*     const char * ar_get_string_from_event(ar_state_event_t event) 
*
*  FUNCTION
*     Converts a state event id to a human readable string. 
*
*  INPUTS
*     ar_state_event_t event - state event id 
*
*  RESULT
*     const char * - string
*
*  NOTES
*     MT-NOTE: ar_get_string_from_event() is not MT safe 
*******************************************************************************/
const char *
ar_get_string_from_event(ar_state_event_t event)
{
   const char *ret = MSG_AR_EVENT_STATE_UNKNOWN;
   DENTER(TOP_LAYER, "ar_get_string_from_event");
   switch(event) {
      case ARL_UNKNOWN:
         ret = MSG_AR_EVENT_STATE_UNKNOWN;
         break;
      case ARL_CREATION:
         ret = MSG_AR_EVENT_STATE_CREATION;
         break;
      case ARL_STARTTIME_REACHED:
         ret = MSG_AR_EVENT_STATE_STARTIME_REACHED;
         break;
      case ARL_ENDTIME_REACHED:
         ret = MSG_AR_EVENT_STATE_ENDTIME_REACHED;
         break;
      case ARL_UNSATISFIED:
         ret = MSG_AR_EVENT_STATE_UNSATISFIED;
         break;
      case ARL_OK:
         ret = MSG_AR_EVENT_STATE_OK;
         break;
      case ARL_TERMINATED:
         ret = MSG_AR_EVENT_STATE_TERMINATED;
         break;
      case ARL_DELETED:
         ret = MSG_AR_EVENT_STATE_DELETED;
         break;
      default:
         /* should never happen */
         DTRACE;
         break;
   }
   DRETURN(ret);
}

/****** libs/sgeobj/ar_state2dstring() ***************************************
*  NAME
*     ar_state2dstring() -- writes the AR state as letter combination
*
*  SYNOPSIS
*     void ar_state2dstring(ar_state_t state, dstring *state_as_string) 
*
*  FUNCTION
*     This function writes the given state of an advance reservation as a
*     letter into the given dstring. The letter will be appended.
*
*  INPUTS
*     ar_state_t state         - ar state 
*     dstring *state_as_string - dstring
*
*  RESULT
*     void 
*
*  NOTES
*     MT-NOTE: ar_get_string_from_event() is MT safe 
*******************************************************************************/
void 
ar_state2dstring(ar_state_t state, dstring *state_as_string)
{
   const char *letter = "u";
   switch (state) {
      case AR_WAITING:
         letter = "w";
         break;
      case AR_RUNNING:
         letter = "r";
         break;
      case AR_EXITED:
         letter = "x";
         break;
      case AR_DELETED:
         letter = "d";
         break;
      case AR_ERROR:
         letter = "E";
         break;
      case AR_WARNING:
         letter = "W";
         break;
      default:
         break;
   }
   sge_dstring_append(state_as_string, letter);
}

/****** sge_advance_reservation/sge_ar_has_errors() ****************************
*  NAME
*     sge_ar_has_errors() -- Has AR errors?
*
*  SYNOPSIS
*     bool sge_ar_has_errors(lListElem *ar) 
*
*  FUNCTION
*     Check if one of the reserved queues is in state where jobs can not be
*     running
*
*  INPUTS
*     lListElem *ar - advance reservation object (AR_Type)
*
*  RESULT
*     bool - true if has errors
*            false if has no errors
*
*  NOTES
*     MT-NOTE: sge_ar_has_errors() is MT safe 
*******************************************************************************/
bool sge_ar_has_errors(lListElem *ar) {
   bool ret = false;

   DENTER(TOP_LAYER, "sge_ar_has_errors");

   if (lGetUlong(ar, AR_qi_errors) != 0) {
      ret = true;
   }

   DRETURN(ret);
}

/****** sge_advance_reservation/ar_list_sort() ********************************
*  NAME
*     ar_list_sort() -- Sort a AR_Type list
*
*  SYNOPSIS
*     int ar_list_sort(lList *this_list)
*
*  FUNCTION
*     Sort a AR_Type list
*
*  INPUTS
*     lList *this_list - AR_Type list
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*******************************************************************************/
int ar_list_sort(lList *this_list) {
   DENTER(BASIS_LAYER, "ar_list_sort");

   int ret = lPSortList(this_list, "%I+", AR_id);

   DRETURN(ret);
}
