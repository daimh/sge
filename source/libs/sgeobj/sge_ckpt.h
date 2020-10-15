#ifndef __SGE_CKPT_H 
#define __SGE_CKPT_H 
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

#include "sgeobj/sge_ckpt_CK_L.h"

bool 
ckpt_is_referenced(const lListElem *ckpt, lList **answer_list,
                   const lList *master_job_list,
                   const lList *master_queue_list);

lListElem *
ckpt_list_locate(const lList *ckpt_list, const char *ckpt_name);

int 
sge_parse_checkpoint_attr(const char *attr_str);

int ckpt_validate(const lListElem *this_elem, lList **alpp);

lList **
ckpt_list_get_master_list(void);

bool
ckpt_list_do_all_exist(const lList *ckpt_list, lList **answer_list,
                       const lList *ckpt_ref_list);

lListElem* sge_generic_ckpt(char *ckpt_name);

#endif /* __SGE_CKPT_H */
