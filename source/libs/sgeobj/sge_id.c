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
#include "uti/sge_string.h"

#include "cull/cull_list.h"

#include "sgeobj/parse.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/msg_sgeobjlib.h"

#define ID_LAYER BASIS_LAYER

/* EB: ADOC: add commets */

bool
id_list_build_from_str_list(lList **id_list, 
                            lList **answer_list,
                            const lList *str_list,
                            u_long32 transition,
                            u_long32 option) 
{
   bool ret = true;
   lListElem *elem;

   DENTER(ID_LAYER, "id_list_build_from_str_list");

   if (transition_is_valid_for_qinstance(transition, answer_list) &&
       transition_option_is_valid_for_qinstance(option, answer_list) &&
       str_list_is_valid(str_list, answer_list)) {
      for_each(elem, str_list) {
         const char *string = lGetString(elem, ST_name);
         lListElem *new_id = NULL;

         /*
          * Try to parse and add jid/taskid
          * or add string (queue pattern) 
          */

         if ((transition & QUEUE_DO_ACTION) == 0) { 
            sge_parse_jobtasks(id_list, &new_id, string, answer_list, false, NULL);
         }   

         if (new_id == NULL) {
            new_id = lAddElemStr(id_list, ID_str, string, ID_Type);
         }
         if (new_id != NULL) {
            lSetUlong(new_id, ID_action, transition);
            lSetUlong(new_id, ID_force, option);
         } else {
            answer_list_add(answer_list, MSG_ID_UNABLETOCREATE, 
                            STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            lFreeList(id_list);
            break;
         }
         
      }
   }

   DEXIT;
   return ret;
}


