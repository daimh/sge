#ifndef _QMON_INIT_H_
#define _QMON_INIT_H_
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
#include <Xmt/Color.h>
#include "qmon_proto.h"
#include "gdi/sge_gdi_ctx.h"

void qmonLoadIcons(void); 
Pixmap qmonGetIcon(String name);
void qmonInitSge(sge_gdi_ctx_class_t **ctx_ref, char *progname, int usage);
void qmonAllocColor(Widget w);
void qmonCreateGC(Widget top);
void qmonExitCB(Widget w, XtPointer cld, XtPointer cad);
void qmonExitFunc(int i);
Widget XmtInitialize(XtAppContext *app, String app_class, XrmOptionDescList options, Cardinal num_options, int *argc_in_out, String *argv_in_out, String *fallbacks, ArgList args, Cardinal num_args);


#endif /* _QMON_INIT_H_ */
