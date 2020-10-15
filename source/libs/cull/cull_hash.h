#ifndef __CULL_HASH_H
#define __CULL_HASH_H
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
#include "uti/sge_htable.h"

#include "cull/cull_list.h"
#include "cull/cull_hashP.h"

#ifdef  __cplusplus
extern "C" {
#endif

cull_htable cull_hash_create(const lDescr *descr, int size);
int cull_hash_new(lList *lp, int name, bool unique);
int cull_hash_new_check(lList *lp, int nm, bool unique);
void cull_hash_insert(const lListElem *ep, void *key, cull_htable ht, bool unique);
void cull_hash_remove(const lListElem *ep, const int pos);
void cull_hash_elem(const lListElem *ep);
lListElem *cull_hash_first(cull_htable ht, const void *key, bool unique, 
                           const void **iterator);
lListElem *cull_hash_next(cull_htable ht, const void **iterator);
void cull_hash_free_descr(lDescr *descr);
void cull_hash_create_hashtables(lList *lp);
void *cull_hash_key(const lListElem *ep, int pos, char *host_key);

const char *cull_hash_statistics(cull_htable ht, dstring *buffer);
void cull_hash_recreate_after_sort(lList *lp);
#ifdef  __cplusplus
}
#endif

#endif /* #ifndef __CULL_HASH_H */

