#ifndef __SGE_PETASK_H 
#define __SGE_PETASK_H 
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

#include "sgeobj/sge_pe_task_PET_L.h"
#include "sgeobj/sge_pe_task_PETR_L.h"
#include "sgeobj/sge_pe_task_FPET_L.h"

#define PE_TASK_PAST_USAGE_CONTAINER "past_usage"

lListElem *pe_task_sum_past_usage(lListElem *container, 
                                  const lListElem *pe_task);

lListElem *pe_task_sum_past_usage_all(lList *pe_task_list);

lListElem *pe_task_sum_past_usage_list(lList *pe_task_list, 
                                       const lListElem *pe_task);

bool pe_task_verify_request(const lListElem *petr, lList **answer_list);

#endif /* __SGE_PETASK_H */
