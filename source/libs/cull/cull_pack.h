#ifndef __CULL_PACK_H
#define __CULL_PACK_H
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

#include "cull/cull_list.h" 
#include "cull/pack.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
 *
 *   lListElem
 *
 */
int cull_unpack_elem(sge_pack_buffer *pb, lListElem **epp, const lDescr *dp);

int cull_pack_elem(sge_pack_buffer *pb, const lListElem *ep);

int 
cull_unpack_elem_partial(sge_pack_buffer *pb, lListElem **epp, 
                         const lDescr *dp, int flags);

int
cull_pack_list_summary(sge_pack_buffer *pb, const lList *lp,
                       const lEnumeration *what, const char *name, 
                       size_t *offset, size_t *used);

int
cull_pack_elem_partial(sge_pack_buffer *pb, const lListElem *ep,
                       const lEnumeration *what, int flags);

void setByteArray(const char *byteArray, int size, lListElem *elem, int name);

int getByteArray(char **byte, const lListElem *elem, int name);

/*
 *   lList
 */
int cull_unpack_list(sge_pack_buffer *pb, lList **lpp);
int cull_pack_list(sge_pack_buffer *pb, const lList *lp);

int cull_unpack_list_partial(sge_pack_buffer *pb, lList **lpp, int flags);
int cull_pack_list_partial(sge_pack_buffer *pb, const lList *lp, 
                           lEnumeration *what, int flags);

/*
 *
 *   lEnumeration
 *
 */
int cull_unpack_enum(sge_pack_buffer *pb, lEnumeration **epp);
int cull_pack_enum(sge_pack_buffer *pb, const lEnumeration *ep);

/*
 *
 *   lCondition
 *
 */
int cull_unpack_cond(sge_pack_buffer *pb, lCondition **cpp);
int cull_pack_cond(sge_pack_buffer *pb, const lCondition *cp);

void cull_dump_pack_buffer(sge_pack_buffer *pb, FILE *fp);


#ifdef  __cplusplus
}
#endif

#endif /* #ifndef __CULL_PACK_H */

