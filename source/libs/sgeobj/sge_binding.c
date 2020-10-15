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
 *   Portions of this code are Copyright 2011 Univa Inc.
 *   Copyright (C) 2011 Dave Love, University of Liverpool
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_binding.h" 
#include "sgeobj/sge_answer.h"

#include "msg_common.h"

#define BINDING_LAYER TOP_LAYER

/****** sge_binding/binding_print_to_string() **********************************
*  NAME
*     binding_print_to_string() -- Prints the content of a binding list to a string
*
*  SYNOPSIS
*     bool binding_print_to_string(const lListElem *this_elem, dstring *string)
*
*  FUNCTION
*     Prints the binding type and binding strategy of a binding list element 
*     into a string.
*
*  INPUTS
*     const lListElem* this_elem - Binding list element
*
*  OUTPUTS
*     const dstring *string      - Output string which must be initialized.
*
*  RESULT
*     bool - true in all cases
*
*  NOTES
*     MT-NOTE: is_starting_point() is MT safe
*
*******************************************************************************/
bool
binding_print_to_string(const lListElem *this_elem, dstring *string) {
   bool ret = true;

   DENTER(BINDING_LAYER, "binding_print_to_string");
   if (this_elem != NULL && string != NULL) {
      const char *const strategy = lGetString(this_elem, BN_strategy);
      binding_type_t type = (binding_type_t)lGetUlong(this_elem, BN_type);

      switch (type) {
         case BINDING_TYPE_SET:
            sge_dstring_append(string, "set ");
            break;
         case BINDING_TYPE_PE:
            sge_dstring_append(string, "pe ");
            break;
         case BINDING_TYPE_ENV:
            sge_dstring_append(string, "env ");
            break;
         case BINDING_TYPE_NONE:
            sge_dstring_append(string, "NONE");
      }

      if (strcmp(strategy, "linear_automatic") == 0) {
         u_long32 n = sge_u32c(lGetUlong(this_elem, BN_parameter_n));
         if (BIND_INFINITY == n)
            sge_dstring_sprintf_append(string, "linear:slots");
         else
            sge_dstring_sprintf_append(string, "linear:"sge_U32CFormat, n);
      } else if (strcmp(strategy, "linear") == 0) {
        sge_dstring_sprintf_append(string, "%s:"sge_U32CFormat":"sge_U32CFormat","sge_U32CFormat,
               "linear", sge_u32c(lGetUlong(this_elem, BN_parameter_n)),
               sge_u32c(lGetUlong(this_elem, BN_parameter_socket_offset)),
               sge_u32c(lGetUlong(this_elem, BN_parameter_core_offset)));
      } else if (strcmp(strategy, "striding_automatic") == 0) {
         sge_dstring_sprintf_append(string, "%s:"sge_U32CFormat":"sge_U32CFormat,
            "striding", sge_u32c(lGetUlong(this_elem, BN_parameter_n)),
            sge_u32c(lGetUlong(this_elem, BN_parameter_striding_step_size)));
      } else if (strcmp(strategy, "striding") == 0) {
         sge_dstring_sprintf_append(string, "%s:"sge_U32CFormat":"sge_U32CFormat":"sge_U32CFormat","sge_U32CFormat,
            "striding", sge_u32c(lGetUlong(this_elem, BN_parameter_n)),
            sge_u32c(lGetUlong(this_elem, BN_parameter_striding_step_size)),
            sge_u32c(lGetUlong(this_elem, BN_parameter_socket_offset)),
            sge_u32c(lGetUlong(this_elem, BN_parameter_core_offset)));
      } else if (strcmp(strategy, "explicit") == 0) {
         sge_dstring_sprintf_append(string, "%s", lGetString(this_elem, BN_parameter_explicit));
      }
   }
   DRETURN(ret);
}

bool
binding_parse_from_string(lListElem *this_elem, lList **answer_list, dstring *string) 
{
   bool ret = true;

   DENTER(BINDING_LAYER, "binding_parse_from_string");

   if (this_elem != NULL && string != NULL) {
      int amount = 0;
      int stepsize = 0;
      int firstsocket = 0;
      int firstcore = 0;
      binding_type_t type = BINDING_TYPE_NONE; 
      dstring strategy = DSTRING_INIT;
      dstring socketcorelist = DSTRING_INIT;
      dstring error = DSTRING_INIT;

      if (parse_binding_parameter_string(sge_dstring_get_string(string), 
               &type, &strategy, &amount, &stepsize, &firstsocket, &firstcore, 
               &socketcorelist, &error) != true) {
         dstring parse_binding_error = DSTRING_INIT;

         sge_dstring_append_dstring(&parse_binding_error, &error);

         answer_list_add_sprintf(answer_list, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR,
                                 MSG_PARSE_XOPTIONWRONGARGUMENT_SS, "-binding",  
                                 sge_dstring_get_string(&parse_binding_error));

         sge_dstring_free(&parse_binding_error);
         ret = false;
      } else {
         lSetString(this_elem, BN_strategy, sge_dstring_get_string(&strategy));
         
         lSetUlong(this_elem, BN_type, type);
         lSetUlong(this_elem, BN_parameter_socket_offset, (firstsocket >= 0) ? firstsocket : 0);
         lSetUlong(this_elem, BN_parameter_core_offset, (firstcore >= 0) ? firstcore : 0);
         lSetUlong(this_elem, BN_parameter_n, (amount >= 0) ? amount : 0);
         lSetUlong(this_elem, BN_parameter_striding_step_size, (stepsize >= 0) ? stepsize : 0);
         
         if (strstr(sge_dstring_get_string(&strategy), "explicit") != NULL) {
            lSetString(this_elem, BN_parameter_explicit, sge_dstring_get_string(&socketcorelist));
         }
      }

      sge_dstring_free(&strategy);
      sge_dstring_free(&socketcorelist);
      sge_dstring_free(&error);
   }

   DRETURN(ret);
}

