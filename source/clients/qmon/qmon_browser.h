#ifndef _QMON_BROWSER_H_
#define _QMON_BROWSER_H_
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

#include "qmon_proto.h"


/* 
 * !!!! Attention this depends on the ordering of the strings in
 * !!!! the checkbox/palette
 */
#define BROWSE_STDOUT            (1<<0)
#define BROWSE_STDERR            (1<<1)
#define BROWSE_QUEUE             (1<<2)
#define BROWSE_JOB               (1<<3)
#define BROWSE_MSG               (1<<4)
#define BROWSE_AR                (1<<5)



void qmonBrowserOpen(Widget w, XtPointer cld, XtPointer cad);
void qmonBrowserShow(const char *s);
void qmonBrowserMessages(Widget w, XtPointer cld, XtPointer cad);
Boolean qmonBrowserObjectEnabled(int obj_id);

#endif /* _QMON_BROWSER_H_ */
