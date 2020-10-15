#ifndef __SGE_PE_H 
#define __SGE_PE_H 
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

#include "sgeobj/sge_pe_PE_L.h"

bool
pe_name_is_matching(const char *pe_name, const char *wildcard);

bool 
pe_is_matching(const lListElem *pe, const char *wildcard);

lListElem *
pe_list_find_matching(const lList *pe_list, const char *wildcard);

lListElem *
pe_list_locate(const lList *pe_list, const char *pe_name);

bool 
pe_is_referenced(const lListElem *pe, lList **answer_list,
                 const lList *master_job_list,
                 const lList *master_queue_list);

int 
pe_validate(lListElem *pep, lList **alpp, int startup);

int 
pe_validate_slots(lList **alpp, u_long32 slots);

int 
pe_validate_urgency_slots(lList **alpp, const char *s);

int 
pe_urgency_slots(const lListElem *pe, 
                 const char *urgency_slot_setting, 
                 const lList* range_list);

bool 
pe_list_do_all_exist(const lList *pe_list, lList **answer_list, 
                     const lList *pe_ref_list, bool ignore_make_pe);

lListElem* 
pe_create_template(char *pe_name);

int 
pe_get_slots_used(const lListElem *pe);

int 
pe_set_slots_used(lListElem *pe, int slots);

void 
pe_debit_slots(lListElem *pep, int slots, u_long32 job_id);

#ifdef SGE_PQS_API
int
pe_validate_qsort_args(lList **alpp, const char *qsort_args,
                       lListElem *pe, void **lib, void **fn);
#endif

bool
pe_do_accounting_summary(const lListElem *pe);

#endif /* __SGE_PE_H */
