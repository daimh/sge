/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmScale.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmScale.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>
#include <Xmt/QuarksP.h>
#include <Xm/Scale.h>

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
    int value;

    if ((type != XmtQCardinal) &&
	(type != XmtQDimension) &&
	(type != XmtQPosition) &&
	(type != XmtQInt) &&
	(type != XmtQShort) &&
	(type != XmtQUnsignedChar)) {
	XmtWarningMsg("XmtDialogSetDialogValues", "scale",
		      "Type mismatch: Widget '%s':\n\tCan't set widget value from a resource of type '%s'; scalar type expected.",
		      XtName(w), XrmQuarkToString(type));
	return;
    }

    if (size == sizeof(unsigned char))
	value = *(unsigned char *)address;
    else if (size == sizeof(unsigned short))
	value = *(unsigned short *)address;
    else if (size == sizeof(unsigned int))
	value = *(unsigned int *)address;
    else return;

    XmScaleSetValue(w, value);
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
    int value;

    if ((type != XmtQCardinal) &&
	(type != XmtQDimension) &&
	(type != XmtQPosition) &&
	(type != XmtQInt) &&
	(type != XmtQShort) &&
	(type != XmtQUnsignedChar)) {
	XmtWarningMsg("XmtDialogGetDialogValues", "scale",
		      "Type mismatch: Widget '%s':\n\tCan't set value on resource of type '%s'; scalar type expected.",
		      XtName(w), XrmQuarkToString(type));
	return;
    }

    XmScaleGetValue(w, &value);

    if (size == sizeof(unsigned char))
	*(unsigned char *)address = value;
    else if (size == sizeof(unsigned short))
	*(unsigned short *)address = value;
    else if (size == sizeof(unsigned int))
	*(unsigned int *)address = value;
}

static XmtWidgetType widget = {
    "XmScale",
    NULL,
    XmCreateScale,
    setvalue,
    getvalue
};

#if NeedFunctionPrototypes
void XmtRegisterXmScale(void)
#else
void XmtRegisterXmScale()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&widget, 1);
}



