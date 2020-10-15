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
#include "uti/sge_dstring.h"

#include "cull/cull_list.h"

#include "gdi/sge_gdi.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_cqueue_verify.h"
#include "sgeobj/sge_path_alias.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "symbols.h"
#include "sge.h"

#define CQUEUE_VERIFY_LAYER TOP_LAYER

/* EB: ADOC: add commets */

bool
cqueue_verify_calendar(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_calendar");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *name = lGetString(attr_elem, ASTR_value);

      if (name != NULL && strcasecmp("none", name)) {
         lListElem *calendar = calendar_list_locate(*object_type_get_master_list(SGE_TYPE_CALENDAR), name);
         if (calendar == NULL) {
            sprintf(SGE_EVENT, MSG_CQUEUE_UNKNOWNCALENDAR_S, name);
            answer_list_add(answer_list, SGE_EVENT,
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_ckpt_list(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_ckpt_list");
   if (cqueue != NULL && attr_elem != NULL) {
      lList *ckpt_list = lGetList(attr_elem, ASTRLIST_value);

      if (ckpt_list != NULL) {
         const lList *master_list = *(ckpt_list_get_master_list());

         if (!ckpt_list_do_all_exist(master_list, answer_list, ckpt_list)) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_consumable_config_list(lListElem *cqueue, lList **answer_list,
                                     lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_project_list");
   if (cqueue != NULL && attr_elem != NULL) {
      lList *centry_list = lGetList(attr_elem, ACELIST_value);

      if (centry_list != NULL) {
         const lList *master_list = *(centry_list_get_master_list());

         if (!centry_list_do_all_exists(master_list, answer_list, centry_list)) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_initial_state(lListElem *cqueue, lList **answer_list,
                            lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_initial_state");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *names[] = {"default", "enabled", "disabled", NULL};
      const char *name = lGetString(attr_elem, ASTR_value);
      bool found = false;
      int i = 0;

      while (names[i] != NULL) {
         if (!strcasecmp(name, names[i])) {
            found = true;
         }
         i++;
      }
      if (!found) {
         sprintf(SGE_EVENT, MSG_CQUEUE_UNKNOWNINITSTATE_S, name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = false;
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_pe_list(lListElem *cqueue, lList **answer_list,
                      lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_pe_list");
   if (cqueue != NULL && attr_elem != NULL) {
      lList *pe_list = lGetList(attr_elem, ASTRLIST_value);

      if (pe_list != NULL) {
         const lList *master_list = *(object_type_get_master_list(SGE_TYPE_PE));
         if (!pe_list_do_all_exist(master_list, answer_list, pe_list, true)) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_priority(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_priority");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *priority_string = lGetString(attr_elem, ASTR_value);

      if (priority_string != NULL) {
         const int priority = atoi(priority_string);

         if (priority == 0 && priority_string[0] != '0') {
            answer_list_add(answer_list, MSG_CQUEUE_WRONGCHARINPRIO,
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = false;
         } else if (priority < -20 || priority > 20 ) {
            answer_list_add(answer_list, MSG_CQUEUE_PRIORITYNOTINRANGE,
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_processors(lListElem *cqueue, lList **answer_list,
                         lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_priority");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *processors_string = lGetString(attr_elem, ASTR_value);

      if (processors_string != NULL) {
         lList *range_list = NULL;

         range_list_parse_from_string(&range_list, answer_list,
                                      processors_string,
                                      JUST_PARSE, false, INF_ALLOWED);
         if (*answer_list) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_project_list(lListElem *cqueue, lList **answer_list,
                           lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_project_list");
   if (cqueue != NULL && attr_elem != NULL) {
      lList *project_list = lGetList(attr_elem, APRJLIST_value);

      if (project_list != NULL) {
         const lList *master_list = *object_type_get_master_list(SGE_TYPE_PROJECT);

         if (!prj_list_do_all_exist(master_list, answer_list, project_list)) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_shell_start_mode(lListElem *cqueue, lList **answer_list,
                               lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_shell_start_mode");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *names[] = {
         "unix_behavior", "posix_compliant", "script_from_stdin",
         NULL
      };
      const char *name = lGetString(attr_elem, ASTR_value);
      bool found = false;
      int i = 0;

      while (names[i] != NULL) {
         if (!strcasecmp(name, names[i])) {
            found = true;
         }
         i++;
      }
      if (!found) {
         sprintf(SGE_EVENT, MSG_CQUEUE_UNKNOWNSTARTMODE_S, name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = false;
      }
   }
   DEXIT;
   return ret;
}
bool
cqueue_verify_shell(lListElem *cqueue _UNUSED, lList **answer_list,
                    lListElem *attr_elem)
    {
       bool ret = true;
       bool path_found = true;

       const char *name = lGetString(attr_elem, ASTR_value);

       DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_shell");

       /* Check also if it is an absolute valid path */
       path_found = path_verify(name, answer_list, "shell", true);

           if (!path_found) {
               sprintf(SGE_EVENT, MSG_CQUEUE_UNKNOWNSHELL_S, name);
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               ret = false;
           }
        
        DEXIT;
        return ret;
   }

bool
cqueue_verify_subordinate_list(lListElem *cqueue, lList **answer_list,
                               lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_subordinate_list");
   if (cqueue != NULL && attr_elem != NULL) {
      const lList *master_list = 
                           *(object_type_get_master_list(SGE_TYPE_CQUEUE));
      const char *cqueue_name = lGetString(cqueue, CQ_name);
      lList *so_list = lGetList(attr_elem, ASOLIST_value);
      lListElem *so;

      for_each(so, so_list) {
         const char *so_name = lGetString(so, SO_name);
 
         /*
          * Check for recursions to ourself
          */
         if (strcmp(cqueue_name, so_name) != 0) {
            const lListElem *cqueue = NULL;

            /*
             * Check if cqueue exists
             */
            cqueue = cqueue_list_locate(master_list, so_name);
            if (cqueue != NULL) {
               /*
                * Success
                */
               ;
            } else {
               ERROR((SGE_EVENT, MSG_CQUEUE_UNKNOWNSUB_SS,
                      so_name, cqueue_name));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               ret = false;
            }
         } else {
            ERROR((SGE_EVENT, MSG_CQUEUE_SUBITSELF_S, cqueue_name));
            answer_list_add(answer_list, SGE_EVENT,
                            STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}

bool
cqueue_verify_user_list(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_user_list");
   if (cqueue != NULL && attr_elem != NULL) {
      lList *user_list = lGetList(attr_elem, AUSRLIST_value);

      if (user_list != NULL) {
         if (userset_list_validate_acl_list(user_list, 
                                            answer_list) == STATUS_EUNKNOWN) {
            ret = false;
         }
      }
   }
   DEXIT;
   return ret;
}



/****** sge_cqueue_verify/cqueue_verify_job_slots() ****************************
*  NAME
*     cqueue_verify_job_slots() -- verify the queue slots attribute
*
*  SYNOPSIS
*     bool 
*     cqueue_verify_job_slots(lListElem *cqueue, lList **answer_list, 
*                             lListElem *attr_elem)
*
*  FUNCTION
*     Verifies if the slots attribute of a queue is in the expected range
*     (0 .. MAX_SEQNUM). MAX_SEQNUM is 9999999.
*
*  INPUTS
*     lListElem *cqueue    - The queue to verify.
*     lList **answer_list  - answer list to report errors
*     lListElem *attr_elem - the attribute to verify
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: cqueue_verify_job_slots() is MT safe 
*******************************************************************************/
bool 
cqueue_verify_job_slots(lListElem *cqueue, lList **answer_list, lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_job_slots");
   if (cqueue != NULL && attr_elem != NULL) {
      u_long32 slots = lGetUlong(attr_elem, AULNG_value);

      if (slots > MAX_SEQNUM) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 MSG_ATTR_INVALID_ULONGVALUE_USUU, sge_u32c(slots), "slots",
                                 sge_u32c(0), sge_u32c(MAX_SEQNUM));
         ret = false;
      }
   }

   DEXIT;
   return ret;
}

/****** sge_cqueue_verify/cqueue_verify_memory_value() ****************************
*  NAME
*     cqueue_verify_memory_value() -- verify a queue memory attribute like h_vmem
*
*  SYNOPSIS
*     bool 
*     cqueue_verify_memory_value(lListElem *cqueue, lList **answer_list, 
*                             lListElem *attr_elem)
*
*  FUNCTION
*     Verifies if a memory attribute of a queue is in the expected range
*     (0 .. INFINITY) NONE is no allowed value.
*
*  INPUTS
*     lListElem *cqueue    - The queue to verify.
*     lList **answer_list  - answer list to report errors
*     lListElem *attr_elem - the attribute to verify
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: cqueue_verify_memory_value() is MT safe 
*******************************************************************************/
bool
cqueue_verify_memory_value(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_memory_value");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *memory_string = lGetString(attr_elem, AMEM_value);

#if 1
      lListElem *copy = lCopyElem(attr_elem);
      if (!object_parse_field_from_string(copy, answer_list, AMEM_value, memory_string)) {
         ret = false;
      }
      lFreeElem(&copy);
#else
      if (memory_string == NULL || !strcasecmp(memory_string, "none")) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                         MSG_NONE_NOT_ALLOWED_S, "memory values");
         ret = false;
      }
#endif      
   }
   DEXIT;
   return ret;
}

/****** sge_cqueue_verify/cqueue_verify_time_value() ****************************
*  NAME
*     cqueue_verify_time_value() -- verify a queue time attribute like h_cpu
*
*  SYNOPSIS
*     bool 
*     cqueue_verify_time_value(lListElem *cqueue, lList **answer_list, 
*                             lListElem *attr_elem)
*
*  FUNCTION
*     Verifies if a time attribute of a queue is in the expected range
*     (0:0:0 .. INFINITY) NONE is no allowed value.
*
*  INPUTS
*     lListElem *cqueue    - The queue to verify.
*     lList **answer_list  - answer list to report errors
*     lListElem *attr_elem - the attribute to verify
*
*  RESULT
*     bool - true on success, false on error
*
*  NOTES
*     MT-NOTE: cqueue_verify_time_value() is MT safe 
*******************************************************************************/
bool
cqueue_verify_time_value(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem)
{
   bool ret = true;

   DENTER(CQUEUE_VERIFY_LAYER, "cqueue_verify_time_value");
   if (cqueue != NULL && attr_elem != NULL) {
      const char *time_string = lGetString(attr_elem, ATIME_value);

      if (time_string == NULL || !strcasecmp(time_string, "none")) {
         answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                         MSG_NONE_NOT_ALLOWED_S, "time values");
         ret = false;
      }
   }
   DEXIT;
   return ret;
}

