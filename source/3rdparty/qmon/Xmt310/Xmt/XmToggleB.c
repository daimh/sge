/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmToggleB.c,v 1.2 2004-02-04 14:10:22 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmToggleB.c,v $
 * Revision 1.2  2004-02-04 14:10:22  andre
 * AA-2004-02-04-1: Enhancem.: Cluster Queues qmon
 *                             - Xmt/XmToggleB.c -> callback is called now !!!
 *                             - explain, q filters still missing
 *                             - reuse of some qconf, qstat functionality in qmon
 *                  Review:    Pending
 *                  Changed:   qmon, qstat, qconf, qhost
 *
 * Revision 1.1.1.1  2001/07/18 11:06:03  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>
#include <Xmt/QuarksP.h>
#include <Xm/ToggleB.h>

#if NeedFunctionPrototypes
static void setvalue(Widget w, XtPointer address, XrmQuark type, Cardinal size)
#else
static void setvalue(w, address, type, size)
Widget w;
XtPointer address;
XrmQuark type;
Cardinal size;
#endif
{
    int state;

    if ((type != XmtQBoolean) &&
	(type != XmtQBool) &&
	(type != XmtQCardinal) &&
	(type != XmtQEnum) &&
	(type != XmtQInt) &&
	(type != XmtQShort) &&
	(type != XmtQUnsignedChar)) {
	XmtWarningMsg("XmtDialogSetDialogValues", "toggleb",
		      "Type mismatch: Widget '%s':\n\tCan't set widget value from a resource of type '%s'; scalar type expected.",
		      XtName(w), XrmQuarkToString(type));
	return;
    }

    if (size == sizeof(unsigned char))
	state = *(unsigned char *)address;
    else if (size == sizeof(unsigned short))
	state = *(unsigned short *)address;
    else if (size == sizeof(unsigned int))
	state = *(unsigned int *)address;
    else return;
    
    XmToggleButtonSetState(w, state, True);
}

#if NeedFunctionPrototypes
static void getvalue(Widget w, XtPointer address, XrmQuark type, Cardinal size)
#else
static void getvalue(w, address, type, size)
Widget w;
XtPointer address;
XrmQuark type;
Cardinal size;
#endif
{
    Boolean state = XmToggleButtonGetState(w);

    if ((type != XmtQBoolean) &&
	(type != XmtQBool) &&	
	(type != XmtQCardinal) &&
	(type != XmtQEnum) &&
	(type != XmtQInt) &&
	(type != XmtQShort) &&
	(type != XmtQUnsignedChar)) {
	XmtWarningMsg("XmtDialogGetDialogValues", "toggleb",
		      "Type mismatch: Widget '%s':\n\tCan't set state on resource of type '%s'; scalar type expected.",
		      XtName(w), XrmQuarkToString(type));
	return;
    }

    if (size == sizeof(unsigned char))
	*(unsigned char *)address = state;
    else if (size == sizeof(unsigned short))
	*(unsigned short *)address = state;
    else if (size == sizeof(unsigned int))
	*(unsigned int *)address = state;
}

static XmtWidgetType widget = {
    "XmToggleButton",
    NULL,
    XmCreateToggleButton,
    setvalue,
    getvalue
};

#if NeedFunctionPrototypes
void XmtRegisterXmToggleButton(void)
#else
void XmtRegisterXmToggleButton()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&widget, 1);
}

