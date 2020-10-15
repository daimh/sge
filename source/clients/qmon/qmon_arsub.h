#ifndef _QMON_ARSUB_H_
#define _QMON_ARSUB_H_
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

typedef struct _tARSubMode {
   int mode;
   int sub_mode;
   u_long32 ar_id;
} tARSubMode;

enum _tARSubSensitivityMode {
   ARSUB_NORMAL           = 0x01,
   ARSUB_QSH              = 0x02,
   ARSUB_BINARY           = 0x04,
   ARSUB_SCRIPT           = 0x08,
   ARSUB_QALTER_PENDING   = 0x10,
   ARSUB_QALTER_RUNNING   = 0x20
};

void qmonARSubPopup(Widget w, XtPointer cld, XtPointer cad);
void qmonARSubSetResources(lList **hr, lList **sr);
lList *qmonARSubHR(void);
lList *qmonARSubSR(void);
String qmonARSubRequestType(void);

#endif /* _QMON_ARSUB_H_ */

