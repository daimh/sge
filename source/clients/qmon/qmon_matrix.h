#ifndef __QMON_MATRIX_H
#define __QMON_MATRIX_H
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

#include "Matrix.h"
#include "cull/cull_list.h"

#define CE_TYPE_REDUCED       0
#define CE_TYPE_FULL          1

#define CE_MAX                8

enum {
   CE_NAME,
   CE_SHORTCUT,
   CE_TYPE,
   CE_RELOP,
   CE_REQUEST,
   CE_CONSUMABLE,
   CE_DEFAULT,
   CE_URGENCY
};


void QmonRegisterMatrixWidgets(void);

void qmonSetCE_Type(Widget w, lList *lp, int full);
lList* qmonGetCE_Type(Widget w);
void qmonSetNxN(Widget w, lList *lp, int num_fields, ...);
lList* qmonGetNxN(Widget w, lDescr *dp, int num_fields, ...);
void qmonSet2xN(Widget w, lList *lp, int field1, int field2);
lList* qmonGet2xN(Widget w, lDescr *dp, int field1, int field2);
void qmonMatrixSelect(Widget w, XtPointer cld, XtPointer cad);

#endif /* __QMON_MATRIX_H */
