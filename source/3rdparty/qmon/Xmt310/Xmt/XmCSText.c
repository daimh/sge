/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmCSText.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmCSText.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * This file courtesy of Tony Hefner.
 */

#include <stdlib.h>
#include <Xmt/Xmt.h>

#if XmVersion == 2000

#include <Xmt/WidgetType.h>
#include <Xmt/QuarksP.h>
#include <Xm/CSText.h>

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
    XmString xmString = NULL;

    if (type == XmtQString) {
        /* Convert the value to an XmString. */
        xmString = XmtCreateLocalizedXmString(w, *(String *)address);
	XmCSTextSetString(w, xmString);
    }
    else if (type == XmtQBuffer) {
        /* Convert the value to an XmString. */
        xmString = XmtCreateLocalizedXmString(w, (char *)address);
	XmCSTextSetString(w, xmString);
    }
    else
	XmtWarningMsg("XmtDialogSetDialogValues", "xmcstext",
		      "Type mismatch:\n\tCan't set value from resource of type '%s'.  String or Buffer expected.",
		   XrmQuarkToString(type));

    if (xmString) XmStringFree(xmString);
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
    int nChars;
    int bufferSize;

    if (type == XmtQString) {
        String buffer;

        /* Find out how many characters are in the widget buffer. */
        nChars = XmCSTextGetLastPosition(w);

        /* Calculate the buffer size for the entire contents. */
        bufferSize = (nChars * MB_CUR_MAX) + 1;

        buffer = XtMalloc(bufferSize);

        /* Now read out the contents. */
        XmCSTextGetSubstring(w, 0, nChars, bufferSize, buffer);

	*(String *)address = buffer;
    }
    else if (type == XmtQBuffer) {
        /* Find out how many characters are in the widget buffer. */
        nChars = XmCSTextGetLastPosition(w);

        /* Calculate the buffer size for the entire contents. */
        bufferSize = (nChars * MB_CUR_MAX) + 1;

        if (size >= nChars)
            XmCSTextGetSubstring(w, 0, nChars, size, (char *)address);
        else {
            XmCSTextGetSubstring(w, 0, size, size, (char *)address);

	    XmtWarningMsg("XmtDialogGetDialogValues", "xmcstextTrunc",
			  "The input value is %d characters long\n\tand does not fit into a buffer %d characters long.\n\tThe trailing characters have been truncated.",
			  nChars, size);
        }
    }
    else
	XmtWarningMsg("XmtDialogGetDialogValues", "xmtextType",
		      "Type mismatch:\n\tCan't set input value on a resource of type '%s'.  String or Buffer expected.",
		      XrmQuarkToString(type));
}

static XmtWidgetType cstext = {
    "XmCSText",
    NULL,
    (XmtWidgetConstructor) XmCreateCSText,
    setvalue,
    getvalue,
};

static XmtWidgetType scstext = {
    "XmScrolledCSText",
    NULL,
    (XmtWidgetConstructor) XmCreateScrolledCSText,
    setvalue,
    getvalue,
};

#if NeedFunctionPrototypes
void XmtRegisterXmCSText(void)
#else
void XmtRegisterXmCSText()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&cstext, 1);
}

#if NeedFunctionPrototypes
void XmtRegisterXmScrolledCSText(void)
#else
void XmtRegisterXmScrolledCSText()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&scstext, 1);
}

#else  /* if XmVersion = 2000 */
/* just so the compiler doesn't complain about an empty file. */
static char XmtDummy;
#endif
