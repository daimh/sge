#ifndef __SGE_CQUEUE_VERIFY_H
#define __SGE_CQUEUE_VERIFY_H

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

#include "sgeobj/sge_cqueue.h"

bool
cqueue_verify_calendar(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem);

bool
cqueue_verify_ckpt_list(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem);

bool
cqueue_verify_consumable_config_list(lListElem *cqueue, lList **answer_list,
                                     lListElem *attr_elem);

bool
cqueue_verify_initial_state(lListElem *cqueue, lList **answer_list,
                            lListElem *attr_elem);

bool
cqueue_verify_pe_list(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem);

bool
cqueue_verify_priority(lListElem *cqueue, lList **answer_list,
                       lListElem *attr_elem);

bool
cqueue_verify_processors(lListElem *cqueue, lList **answer_list,
                         lListElem *attr_elem);

bool
cqueue_verify_project_list(lListElem *cqueue, lList **answer_list,
                           lListElem *attr_elem);

bool
cqueue_verify_shell_start_mode(lListElem *cqueue, lList **answer_list,
                               lListElem *attr_elem);

bool
cqueue_verify_shell(lListElem *cqueue, lList **answer_list,
                               lListElem *attr_elem);

bool
cqueue_verify_subordinate_list(lListElem *cqueue, lList **answer_list,
                               lListElem *attr_elem);

bool
cqueue_verify_user_list(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem);

bool
cqueue_verify_job_slots(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem);

bool
cqueue_verify_memory_value(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem);

bool
cqueue_verify_time_value(lListElem *cqueue, lList **answer_list,
                        lListElem *attr_elem);
#endif
