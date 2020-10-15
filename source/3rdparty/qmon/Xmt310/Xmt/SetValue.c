/* 
 * Motif Tools Library, Version 3.1
 * $Id: SetValue.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: SetValue.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/SetValue.h>

#if NeedFunctionPrototypes
void XmtSetValue(Widget w, StringConst res, StringConst val)
#else
void XmtSetValue(w, res, val)
Widget w;
StringConst res;
StringConst val;
#endif
{
    XtVaSetValues(w, XtVaTypedArg, res, XtRString, val, strlen(val)+1, NULL);
}

#if NeedFunctionPrototypes
void XmtSetTypedValue(Widget w, StringConst resource,
		      StringConst type, StringConst value)
#else
void XmtSetTypedValue(w, resource, type, value)
Widget w;
StringConst resource;
StringConst type;
StringConst value;
#endif
{
    XrmValue from, to;
    Arg arg;

    from.addr = (XtPointer) value;
    from.size = strlen(value) + 1;
    to.addr = NULL;
    to.size = 0;

    if (XtConvertAndStore(w, XtRString, &from, type, &to)) {
	arg.name = (String) resource;
	switch (to.size) {
	case sizeof(char):   arg.value = *(char *)to.addr; break;
	case sizeof(short):  arg.value = *(short *)to.addr; break;
	case sizeof(int):    arg.value = *(int *)to.addr; break;
	default:             arg.value = (XtArgVal)to.addr; break;
	}
	XtSetValues(w, &arg, 1);
    }
    else {
	XmtWarningMsg("XmtSetTypedValue", "convert",
		      "Can't set resource '%s' of widget '%s':\n\t conversion failed.",
		      resource, XtName(w));
    }
}
