#ifndef __SGE_TASK_DEPEND_H
#define __SGE_TASK_DEPEND_H
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2007 by Rising Sun Pictures
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sgeobj/sge_object.h"

bool
sge_task_depend_is_same_range(const lListElem *pre_jep, 
                              const lListElem *suc_jep);

int
sge_task_depend_get_range(lListElem **range, lList **alpp, 
                          const lListElem *pre_jep, 
                          const lListElem *suc_jep, u_long32 task_id);

bool
sge_task_depend_update(lListElem *jep, lList **alpp, u_long32 task_id);

bool
sge_task_depend_init(lListElem *jep, lList **alpp);

bool
sge_task_depend_flush(lListElem *jep, lList **alpp);

#endif /* __SGE_TASK_DEPEND_H */
