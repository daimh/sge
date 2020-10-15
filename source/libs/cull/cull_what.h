#ifndef __CULL_WHAT_H
#define __CULL_WHAT_H
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

#ifdef  __cplusplus
extern "C" {
#endif

#include "uti/sge_dstring.h"

#define WHAT_ALL                        -1
#define WHAT_NONE                       -2

lEnumeration *lWhat(const char *fmt, ...);

lEnumeration *_lWhat(const char *fmt, const lDescr *dp, const int *nm_list, int nm);
lEnumeration *lWhatAll(void);
void lFreeWhat(lEnumeration **ep);
lEnumeration *lCopyWhat(const lEnumeration *ep);
int lCountWhat(const lEnumeration *ep, const lDescr *dp);

int lReduceDescr(lDescr **dst_dpp, lDescr *src_dp, lEnumeration *enp);
lEnumeration *lIntVector2What(const lDescr *dp, const int intv[]);
void nm_set(int job_field[], int nm);

int lMergeWhat(lEnumeration **what1, lEnumeration **what2);
int lWhatSetSubWhat(lEnumeration *what1, int nm, lEnumeration **what2);

#ifdef  __cplusplus
}
#endif

#endif /* __CULL_WHAT_H */

