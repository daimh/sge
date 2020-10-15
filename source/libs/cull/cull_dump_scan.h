#ifndef __CULL_DUMP_SCAN_H
#define __CULL_DUMP_SCAN_H
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

#include "cull/cull_multitype.h"

#ifdef  __cplusplus
extern "C" {
#endif

int lDumpElemFp(FILE *fp, const lListElem *ep, int indent);
int lDumpElem(const char *fname, const lListElem *ep, int indent);
int lDumpObject(FILE *fp, const lListElem *ep, int indent);
int lDumpList(FILE *fp, const lList *lp, int indent);
int lDumpDescr(FILE *fp, const lDescr *dp, int indent);

lListElem *lUndumpElem(const char *fname, const lDescr *dp);
lListElem *lUndumpElemFp(FILE *fp, const lDescr *dp);
lListElem *lUndumpObject(FILE *fp);
lList *lUndumpList(FILE *fp, const char *name, const lDescr *dp);
lDescr *lUndumpDescr(FILE *fp);

#ifdef  __cplusplus
}
#endif

#endif /* __CULL_DUMP_SCAN_H */
