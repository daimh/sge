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

#include <strings.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/config_file.h"
#include "uti/sge_signal.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "symbols.h"
#include "msg_common.h"

/****** sgeobj/ckpt/ckpt_is_referenced() **************************************
*  NAME
*     ckpt_is_referenced() -- Is a given CKPT referenced in other objects? 
*
*  SYNOPSIS
*     bool ckpt_is_referenced(const lListElem *ckpt, lList **answer_list, 
*                             const lList *master_job_list,
*                             const lList *master_cqueue_list) 
*
*  FUNCTION
*     This function returns true if the given "ckpt" is referenced
*     in at least one of the objects contained in "master_job_list" or
*     "master_cqueue_list". If this is the case than
*     a corresponding message will be added to the "answer_list". 
*
*  INPUTS
*     const lListElem *ckpt           - CK_Type object 
*     lList **answer_list             - AN_Type list 
*     const lList *master_job_list    - JB_Type list 
*     const lList *master_cqueue_list - CQ_Type list
*
*  RESULT
*     bool - true or false  
******************************************************************************/
bool ckpt_is_referenced(const lListElem *ckpt, lList **answer_list,
                        const lList *master_job_list, 
                        const lList *master_cqueue_list)
{
   bool ret = false;

   {
      lListElem *job = NULL;

      for_each(job, master_job_list) {
         if (job_is_ckpt_referenced(job, ckpt)) {
            const char *ckpt_name = lGetString(ckpt, CK_name);
            u_long32 job_id = lGetUlong(job, JB_job_number);

            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                    ANSWER_QUALITY_INFO, MSG_CKPTREFINJOB_SU,
                                    ckpt_name, sge_u32c(job_id));
            ret = true;
            break;
         }
      } 
   }
   if (!ret) {
      lListElem *queue = NULL, *ckl = NULL;
 
      /* fix for bug 6422335
       * check the cq configuration for ckpt references instead of qinstances
       */
      const char *ckpt_name = lGetString(ckpt, CK_name);

      for_each(queue, master_cqueue_list) {
         for_each(ckl, lGetList(queue, CQ_ckpt_list)){
            if (lGetSubStr(ckl, ST_name, ckpt_name, ASTRLIST_value))  {
               answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN,
                                       ANSWER_QUALITY_INFO, 
                                       MSG_CKPTREFINQUEUE_SS,
                                       ckpt_name, lGetString(queue, CQ_name));
               ret = true;
               break;
            }
         }
      }
   }
   return ret;
}

/****** sgeobj/ckpt/ckpt_list_locate() ***************************************
*  NAME
*     ckpt_list_locate -- find a ckpt object in a list 
*
*  SYNOPSIS
*     lListElem *ckpt_list_locate(lList *ckpt_list, const char *ckpt_name)
*
*  FUNCTION
*     This function will return a ckpt object by name if it exists.
*
*
*  INPUTS
*     lList *ckpt_list      - CK_Type object
*     const char *ckpt_name - name of the ckpt object. 
*
*  RESULT
*     NULL - ckpt object with name "ckpt_name" does not exist
*     !NULL - pointer to the cull element (CK_Type) 
******************************************************************************/
lListElem *ckpt_list_locate(const lList *ckpt_list, const char *ckpt_name)
{
   return lGetElemStr(ckpt_list, CK_name, ckpt_name);
}

/****** sgeobj/ckpt/sge_parse_checkpoint_attr() *******************************
*  NAME
*     sge_parse_checkpoint_attr() -- make "when" bitmask from string 
*
*  SYNOPSIS
*     int sge_parse_checkpoint_attr(const char *attr_str) 
*
*  FUNCTION
*     Parse checkpoint "when" string and return a bitmask. 
*
*  INPUTS
*     const char *attr_str - when string 
*
*  RESULT
*     int - bitmask of checkpoint specifers
*           0 if attr_str == NULL or nothing set or value 
*           may be a time value 
*
*  NOTES
*     MT-NOTE: sge_parse_checkpoint_attr() is MT safe
*******************************************************************************/
int sge_parse_checkpoint_attr(const char *attr_str)
{
   int opr;

   if (attr_str == NULL) {
      return 0;
   }

   /* May be it's a time value */
   if (isdigit((int) *attr_str) || (*attr_str == ':')) {
      return 0;
   }

   opr = 0;
   while (*attr_str) {
      if (*attr_str == CHECKPOINT_AT_MINIMUM_INTERVAL_SYM)
         opr = opr | CHECKPOINT_AT_MINIMUM_INTERVAL;
      else if (*attr_str == CHECKPOINT_AT_SHUTDOWN_SYM)
         opr = opr | CHECKPOINT_AT_SHUTDOWN;
      else if (*attr_str == CHECKPOINT_SUSPEND_SYM)
         opr = opr | CHECKPOINT_SUSPEND;
      else if (*attr_str == NO_CHECKPOINT_SYM)
         opr = opr | NO_CHECKPOINT;
      else if (*attr_str == CHECKPOINT_AT_AUTO_RES_SYM)
         opr = opr | CHECKPOINT_AT_AUTO_RES;
      else {
         opr = -1;
         break;
      }
      attr_str++;
   }

   return opr;
}

/****** sgeobj/ckpt/ckpt_validate() ******************************************
*  NAME
*     ckpt_validate -- validate all ckpt interface parameters
*
*  SYNOPSIS
*     int ckpt_validate(lListElem *ep, lList **alpp);
*
*  FUNCTION
*     This function will test all ckpt interface parameters.
*     If all are valid then it will return successfull.
*
*  INPUTS
*     ep     - element which sould be verified.
*     answer - answer list where the function stored error messages
*
*  RESULT
*     [answer] - error messages will be added to this list
*     STATUS_OK - success
*     STATUS_EUNKNOWN or STATUS_EEXIST - error
*
*  NOTES
*     MT-NOTE: ckpt_validate() is not MT safe
******************************************************************************/
int ckpt_validate(const lListElem *this_elem, lList **alpp)
{
   static const char* ckpt_interfaces[] = {
      "USERDEFINED",
      "HIBERNATOR",
      "TRANSPARENT",
      "APPLICATION-LEVEL",
      "CPR"
   };
   static struct attr {
      int nm;
      char *text;
   } ckpt_commands[] = {
      { CK_ckpt_command, "ckpt_command" },
      { CK_migr_command, "migr_command" },
      { CK_rest_command, "restart_command"},
      { CK_clean_command, "clean_command"},
      { NoName,           NULL} };

   unsigned i;
   int found = 0;
   const char *s, *interface;

   DENTER(TOP_LAYER, "ckpt_validate");

   if (!this_elem) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   /* -------- CK_name */
   if (verify_str_key(
         alpp, lGetString(this_elem, CK_name), 
         MAX_VERIFY_STRING, "checkpoint interface", KEY_TABLE) != STATUS_OK) {
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   /*
    * check if ckpt obj can be added
    * check allowed interfaces and license
    */
   interface = lGetString(this_elem, CK_interface);
   found = 0;
   /* handle NULL pointer for interface name */
   if (interface == NULL) {
      interface = "<null>";
   } else {

      for (i=0; i < (sizeof(ckpt_interfaces)/sizeof(char*)); i++) {
         if (!strcasecmp(interface, ckpt_interfaces[i])) {
            found = 1;
            break;
         }
      }
   }

   if (!found) {
      ERROR((SGE_EVENT, MSG_SGETEXT_NO_INTERFACE_S, interface));
      answer_list_add(alpp, SGE_EVENT, 
                      STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EEXIST;
   }

   for (i = 0; ckpt_commands[i].nm != NoName; i++) {
      if (replace_params(lGetString(this_elem, ckpt_commands[i].nm),
               NULL, 0, ckpt_variables)) {
         ERROR((SGE_EVENT, MSG_OBJ_CKPTENV_SSS,
               ckpt_commands[i].text, lGetString(this_elem, CK_name), err_msg));
         answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
         DEXIT;
         return STATUS_EEXIST;
      }
   }

   /* -------- CK_signal */
   if ((s=lGetString(this_elem, CK_signal)) &&
         strcasecmp(s, "none") &&
         sge_sys_str2signal(s)==-1) {
      ERROR((SGE_EVENT, MSG_CKPT_XISNOTASIGNALSTRING_S , s));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EEXIST;
   }

   DEXIT;
   return STATUS_OK;
}

/****** sgeobj/ckpt/ckpt_list_get_master_list() *******************************
*  NAME
*     ckpt_list_get_master_list() -- Return pointer to master ckpt list 
*
*  SYNOPSIS
*     lList ** ckpt_list_get_master_list(void) 
*
*  FUNCTION
*     Return pointer to master ckpt list 
*
*  RESULT
*     lList ** - master ckpt list
*******************************************************************************/
lList **
ckpt_list_get_master_list(void)
{
   return object_type_get_master_list(SGE_TYPE_CKPT);
}

/****** sgeobj/ckpt/ckpt_list_do_all_exist() **********************************
*  NAME
*     ckpt_list_do_all_exist() -- Do all ckpt's exist? 
*
*  SYNOPSIS
*     bool 
*     ckpt_list_do_all_exist(const lList *ckpt_list, 
*                            lList **answer_list, 
*                            const lList *ckpt_ref_list) 
*
*  FUNCTION
*     Test if the checkpointing objects whose name is contained in
*     "ckpt_ref_list" is contained in "ckpt_list". 
*
*  INPUTS
*     const lList *ckpt_list     - CK_Type list 
*     lList **answer_list        - AN_Type list 
*     const lList *ckpt_ref_list - ST_Type list containing ckpt names 
*
*  RESULT
*     bool - true if all ckpt objects exist 
*******************************************************************************/
bool
ckpt_list_do_all_exist(const lList *ckpt_list, lList **answer_list,
                       const lList *ckpt_ref_list)
{
   bool ret = true;
   lListElem *ckpt_ref_elem = NULL;

   DENTER(TOP_LAYER, "ckpt_list_do_all_exist");
   for_each(ckpt_ref_elem, ckpt_ref_list) {
      const char *ckpt_ref_string = lGetString(ckpt_ref_elem, ST_name);

      if (ckpt_list_locate(ckpt_list, ckpt_ref_string) == NULL) {
         answer_list_add_sprintf(answer_list, STATUS_EEXIST,
                                 ANSWER_QUALITY_ERROR,
                                 MSG_CKPTREFDOESNOTEXIST_S, ckpt_ref_string);
         ret = false;
         break;
      }
   }
   DEXIT;
   return ret;
}


/****** src/sge_generic_ckpt() **********************************************
*
*  NAME
*     sge_generic_ckpt -- build up a generic ckpt object
*
*  SYNOPSIS
*     lListElem* sge_generic_ckpt(
*        char *ckpt_name
*     );
*
*  FUNCTION
*     build up a generic ckpt object
*
*  INPUTS
*     ckpt_name - name used for the CK_name attribute of the generic
*               pe object. If NULL then "template" is the default name.
*
*  RESULT
*     !NULL - Pointer to a new CULL object of type CK_Type
*     NULL - Error
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*
*****************************************************************************/   
lListElem* sge_generic_ckpt(char *ckpt_name) 
{
   lListElem *ep;

   DENTER(TOP_LAYER, "sge_generic_ckpt");

   ep = lCreateElem(CK_Type);

   if (ckpt_name)
      lSetString(ep, CK_name, ckpt_name);
   else
      lSetString(ep, CK_name, "template");

   lSetString(ep, CK_interface, "userdefined");
   lSetString(ep, CK_ckpt_command, "none");
   lSetString(ep, CK_migr_command, "none");
   lSetString(ep, CK_rest_command, "none");
   lSetString(ep, CK_clean_command, "none");
   lSetString(ep, CK_ckpt_dir, "/tmp");
   lSetString(ep, CK_when, "sx");
   lSetString(ep, CK_signal, "none");
   lSetUlong(ep, CK_job_pid, 0);

   DEXIT;
   return ep;
}

