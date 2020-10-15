#ifndef __SGE_JA_TASK_H 
#define __SGE_JA_TASK_H 
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

#include "uti/sge_dstring.h"

#include "sgeobj/sge_ja_task_JAT_L.h"

lListElem *ja_task_search_pe_task(const lListElem *ja_task,
                                  const char *pe_task_id);

void ja_task_list_print_to_string(const lList *ja_task_list, 
                                  dstring *range_string);

lList* ja_task_list_split_group(lList **ja_task_list);

bool 
ja_task_add_finished_pe_task(lListElem *ja_task, const char *pe_task_id);

bool 
ja_task_clear_finished_pe_tasks(lListElem *ja_task);

int sge_parse_jobtasks(lList **lp, lListElem **idp, const char *str, 
                       lList **alpp, bool include_names, lList *arrayDefList);

bool
ja_task_message_add(lListElem *this_elem, u_long32 type, const char *message);

bool
ja_task_message_trash_all_of_type_X(lListElem *this_elem, u_long32 type);

bool
ja_task_verify(const lListElem *ja_task, lList **answer_list);

bool
ja_task_verify_execd_job(const lListElem *ja_task, lList **answer_list);

bool
ja_task_verify_granted_destin_identifier_list(const lList *gdil, lList **answer_list);

bool
ja_task_verify_granted_destin_identifier(const lListElem *ep, lList **answer_list);

bool
ja_task_is_tightly_integrated(const lListElem *ja_task);
#endif /* __SGE_JA_TASK_H */
