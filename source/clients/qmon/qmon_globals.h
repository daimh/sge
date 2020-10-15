#ifndef _QMON_GLOBALS_H_
#define _QMON_GLOBALS_H_
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

extern char*         SGE_ROOT;
extern XtAppContext  AppContext;
extern Widget        AppShell;
extern GC            fg_gc, bg_gc, qb_gc, alarm_gc,
                     error_gc, suspend_gc, running_gc, disable_gc,
                     caldisable_gc, calsuspend_gc;
extern Pixel         WarningPixel;
extern Pixel         QueueSelectedPixel;
extern Pixel         JobSuspPixel;
extern Pixel         JobSosPixel;
extern Pixel         JobDelPixel;
extern Pixel         JobHoldPixel;
extern Pixel         JobErrPixel;
extern Pixel         TooltipForeground;
extern Pixel         TooltipBackground;
extern int           nologo;
extern int           helpset;
extern int           qmon_debug;

/* this function is used almost everywhere */
void qmonMainControlRaise(Widget w, XtPointer cld, XtPointer cad);

#endif /* _QMON_GLOBALS_H_ */
