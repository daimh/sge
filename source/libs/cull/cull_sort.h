#ifndef __CULL_SORT_H
#define __CULL_SORT_H
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

#include <stdarg.h>

#include "cull/cull_list.h"

#ifdef  __cplusplus
extern "C" {
#endif

int lSortCompare(const lListElem *ep0, const lListElem *ep1, const lSortOrder *sp);

int lInsertSorted(const lSortOrder *so, lListElem *ep, lList *lp);
int lResortElem(const lSortOrder *so, lListElem *ep, lList *lp);

lSortOrder *lParseSortOrderVarArg(const lDescr *dp, const char *fmt, ...);
lSortOrder *lParseSortOrder(const lDescr *dp, const char *fmt, va_list ap);
void lFreeSortOrder(lSortOrder **so);

/* for debugging purposes */
void lWriteSortOrder(const lSortOrder *sp);


lSortOrder * lCreateSortOrder(int n);
int lAddSortCriteria(const lDescr *dp, lSortOrder *so, int nm, int up_down_flag);

/* ------------ */
#ifdef  __cplusplus
}
#endif

#endif /* __CULL_SORT_H */

