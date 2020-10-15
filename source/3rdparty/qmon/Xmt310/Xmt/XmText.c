/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmText.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmText.c,v $
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
#include <Xm/Text.h>

/* ARGSUSED */
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
    if (type == XmtQString)
	XmTextSetString(w, *(String *)address);
    else if (type == XmtQBuffer)
	XmTextSetString(w, (char *)address);
    else
	XmtWarningMsg("XmtDialogSetDialogValues", "xmtext",
		      "Type mismatch:\n\tCan't set value from resource of type '%s'.  String or Buffer expected.",
		   XrmQuarkToString(type));
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
    if (type == XmtQString)
	*(String *)address = XmTextGetString(w);
    else if (type == XmtQBuffer) {
	String s = XmTextGetString(w);
	int len = strlen(s);

	strncpy(address, s, size-1);
	((char *)address)[size-1] = '\0';
	if (len >= size)
	    XmtWarningMsg("XmtDialogGetDialogValues", "xmtextTrunc",
			  "The input value is %d characters long\n\tand does not fit into a buffer %d characters long.\n\tThe trailing characters have been truncated.",
			  len+1, size);
	XtFree(s);
    }
    else
	XmtWarningMsg("XmtDialogGetDialogValues", "xmtextType",
		      "Type mismatch:\n\tCan't set input value on a resource of type '%s'.  String or Buffer expected.",
		      XrmQuarkToString(type));
}

static XmtWidgetType text = {
    "XmText",
    NULL,
    XmCreateText,
    setvalue,
    getvalue,
};

static XmtWidgetType stext = {
    "XmScrolledText",
    NULL,
    XmCreateScrolledText,
    setvalue,
    getvalue,
};

#if NeedFunctionPrototypes
void XmtRegisterXmText(void)
#else
void XmtRegisterXmText()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&text, 1);
}

#if NeedFunctionPrototypes
void XmtRegisterXmScrolledText(void)
#else
void XmtRegisterXmScrolledText()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&stext, 1);
}
