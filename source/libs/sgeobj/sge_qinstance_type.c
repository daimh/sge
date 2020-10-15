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

#include "cull/cull.h"

#include "sgeobj/cull_parse_util.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_type.h"
#include "sgeobj/sge_utility.h"

#include "msg_common.h"

#define QINSTANCE_TYPE_LAYER TOP_LAYER

const char *queue_types[] = {
   "BATCH",
   "INTERACTIVE",
   NULL
};

static bool
qinstance_has_type(const lListElem *this_elem, u_long32 type);

static bool
qinstance_has_type(const lListElem *this_elem, u_long32 type)
{
   bool ret = false;

   if (lGetUlong(this_elem, QU_qtype) & type) {
      ret = true;
   }
   return ret;
}

/****** sgeobj/qinstance/qtype_append_to_dstring() ****************************
*  NAME
*     qtype_append_to_dstring() -- Creates qtype bitmask as string 
*
*  SYNOPSIS
*     const char * qtype_append_to_dstring(u_long32 qtype, dstring *string) 
*
*  FUNCTION
*     This functions expects a "qtype" bitmask. Each bit represents a 
*     certain queue type. If it is set to 1 the corresponding type name 
*     will be appended to "string". If no bit is 1 than "NONE" will be
*     appended to "string".
*
*  INPUTS
*     u_long32 qtype  - bitmask 
*     dstring *string - string 
*
*  RESULT
*     const char * - pointer to the internal buffer of string
*
*  NOTES
*     MT-NOTE: qtype_append_to_dstring() is MT safe 
*******************************************************************************/
const char *
qtype_append_to_dstring(u_long32 qtype, dstring *string)
{
   const char *ret = NULL;

   DENTER(QINSTANCE_TYPE_LAYER, "qtype_append_to_dstring");
   if (string != NULL) {
      const char **ptr = NULL;
      u_long32 bitmask = 1;
      bool qtype_defined = false;

      for (ptr = queue_types; *ptr != NULL; ptr++) {
         if (bitmask & qtype) {
            if (qtype_defined) {
               sge_dstring_append(string, " ");
            }
            sge_dstring_append(string, *ptr);
            qtype_defined = true;
         }
         bitmask <<= 1;
      };
      if (!qtype_defined) {
         sge_dstring_append(string, "NONE");
      }
      ret = sge_dstring_get_string(string);
   }
   DEXIT;
   return ret;
}

bool
qinstance_print_qtype_to_dstring(const lListElem *this_elem,
                                 dstring *string, bool only_first_char)
{
   bool ret = true;

   DENTER(QINSTANCE_TYPE_LAYER, "qinstance_print_qtype_to_dstring");
   if (this_elem != NULL && string != NULL) {
      const char **ptr = NULL;
      u_long32 bitmask = 1;
      bool qtype_defined = false;

      for (ptr = queue_types; *ptr != NULL; ptr++) {
         if (bitmask & lGetUlong(this_elem, QU_qtype)) {
            qtype_defined = true;
            if (only_first_char) {
               sge_dstring_sprintf_append(string, "%c", (*ptr)[0]);
            } else {
               sge_dstring_sprintf_append(string, "%s ", *ptr);
            }
         }
         bitmask <<= 1;
      };
      if (only_first_char) {
         if (qinstance_is_parallel_queue(this_elem)) {
            sge_dstring_sprintf_append(string, "%c", 'P');
            qtype_defined = true;
         }
         if (qinstance_is_checkpointing_queue(this_elem)) {
            sge_dstring_sprintf_append(string, "%c", 'C');
            qtype_defined = true;
         }
      }
      if (!qtype_defined) {
         if (only_first_char) {
            sge_dstring_append(string, "N");
         } else {
            sge_dstring_append(string, "NONE");
         }
      }
   }
   DRETURN(ret);
}

bool
qinstance_parse_qtype_from_string(lListElem *this_elem, lList **answer_list,
                                  const char *value)
{
   bool ret = true;
   u_long32 type = 0;

   DENTER(QINSTANCE_TYPE_LAYER, "qinstance_parse_qtype_from_string");
   SGE_CHECK_POINTER_FALSE(this_elem, answer_list);
   if (value != NULL && *value != 0) {
      if (!sge_parse_bitfield_str(value, queue_types, &type,
                                  "queue type", NULL, true)) {
         ret = false;
      }
   }

   lSetUlong(this_elem, QU_qtype, type);
   DEXIT;
   return ret;
}

bool qinstance_is_batch_queue(const lListElem *this_elem)
{
   return qinstance_has_type(this_elem, BQ);
}

bool qinstance_is_interactive_queue(const lListElem *this_elem)
{
   return qinstance_has_type(this_elem, IQ);
}

bool qinstance_is_checkpointing_queue(const lListElem *this_elem)
{
   return qinstance_is_a_ckpt_referenced(this_elem);
}

bool qinstance_is_parallel_queue(const lListElem *this_elem)
{
   return qinstance_is_a_pe_referenced(this_elem);
}

