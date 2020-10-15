#ifndef _QMON_REQUEST_H_
#define _QMON_REQUEST_H_
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

#include <Xm/Xm.h>

#include "qmon_proto.h"
#include "qmon_cull.h"

enum resource_type {
   ALL_RESOURCES,
   REQUESTABLE_RESOURCES
};
   

void qmonRequestPopup(Widget w, XtPointer cld, XtPointer cad);
lList *qmonRequestHardResources(void);
lList *qmonRequestSoftResources(void);
void qmonRequestDraw(Widget w, lList *lp, int how);
lList *qmonGetResources(lList *cx_list, int how);
Boolean qmonRequestInput(Widget w, int type, String resource, String stringval, int maxlen);

#endif /* _QMON_REQUEST_H_ */

