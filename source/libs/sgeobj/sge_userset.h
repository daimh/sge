#ifndef _SGE_USERSET_H
#define _SGE_USERSET_H
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

#include "sgeobj/sge_userset_US_L.h"
#include "sgeobj/sge_userset_UE_L.h"
#include "sgeobj/sge_userset_JC_L.h"

#define US_ACL       (1<<0)
#define US_DEPT      (1<<1)

/* 
 *  This is the list type we use to hold the 
 *  user set lists in the qmaster. These are also used as 
 *  (x)access lists.
 */

/* special list element */
#define DEADLINE_USERS     "deadlineusers"
#define DEFAULT_DEPARTMENT "defaultdepartment"
#define AR_USERS           "arusers"


extern const char *userset_types[];

lList **
userset_list_get_master_list(void);

bool userset_is_deadline_user(lList *lp, const char *username);

bool userset_is_ar_user(lList *lp, const char *username);

lListElem *
userset_list_locate(lList *lp, const char *name);

int 
userset_validate_entries(lListElem *userset, lList **alpp, int start_up);

int userset_list_validate_acl_list(lList *acl_list, lList **alpp);

int userset_list_validate_access(lList *acl_list, int nm, lList **alpp);

const char *
userset_get_type_string(const lListElem *userset, lList **answer_list,
                        dstring *buffer);

bool 
userset_set_type_string(lListElem *userset, lList **answer_list, 
                        const char *value);

const char *
userset_list_append_to_dstring(const lList *this_list, dstring *string);

int sge_contained_in_access_list(const char *user, const char *group, 
                                 const lListElem *acl, lList **alpp);

#endif /* _SGE_USERSET_H */



