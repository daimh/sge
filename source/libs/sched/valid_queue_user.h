#ifndef __VALID_QUEUE_USER_H
#define __VALID_QUEUE_USER_H
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

int sge_contained_in_access_list_(const char *user, const char *group, 
                                         const lList *acl, const lList *acl_list);

int sge_has_access(const char *user, const char *group, lListElem *q, 
                   const lList *acl_list);

int sge_has_access_(const char *user, const char *group, const lList *q_acl, 
                    const lList *q_xacl, const lList *acl_list);

bool sge_ar_have_users_access(lList **alpp, lListElem *ar, const char *name, const lList *acl_list, const lList *xacl_list, const lList *master_userset_list);

#endif /* __VALID_QUEUE_USER_H */

