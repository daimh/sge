/* 
 * Motif Tools Library, Version 3.1
 * $Id: Quarks.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Quarks.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/QuarksP.h>

externaldef(xmtqbool) XrmQuark XmtQBool;
externaldef(xmtqboolean) XrmQuark XmtQBoolean;
externaldef(xmtqcardinal) XrmQuark XmtQCardinal;
externaldef(xmtqdimension) XrmQuark XmtQDimension;
externaldef(xmtqenum) XrmQuark XmtQEnum;
externaldef(xmtqint) XrmQuark XmtQInt;
externaldef(xmtqposition) XrmQuark XmtQPosition;
externaldef(xmtqshort) XrmQuark XmtQShort;
externaldef(xmtqunsignedchar) XrmQuark XmtQUnsignedChar;
externaldef(xmtqdouble) XrmQuark XmtQDouble;
externaldef(xmtqfloat) XrmQuark XmtQFloat;
externaldef(xmtqstring) XrmQuark XmtQString;
externaldef(xmtqbuffer) XrmQuark XmtQBuffer;

#if NeedFunctionPrototypes
void _XmtInitQuarks(void)
#else
void _XmtInitQuarks()
#endif
{
    static Boolean inited = False;

    if (inited == False) {
	inited = True;

	XmtQBool = XrmPermStringToQuark(XtRBool);
	XmtQBoolean = XrmPermStringToQuark(XtRBoolean);
	XmtQCardinal = XrmPermStringToQuark(XtRCardinal);
	XmtQDimension = XrmPermStringToQuark(XtRDimension);
	XmtQEnum = XrmPermStringToQuark(XtREnum);
	XmtQInt = XrmPermStringToQuark(XtRInt);
	XmtQPosition = XrmPermStringToQuark(XtRPosition);
	XmtQShort = XrmPermStringToQuark(XtRShort);
	XmtQUnsignedChar = XrmPermStringToQuark(XtRUnsignedChar);
	XmtQDouble = XrmPermStringToQuark(XmtRDouble);
	XmtQFloat = XrmPermStringToQuark(XtRFloat);
	XmtQString = XrmPermStringToQuark(XtRString);
	XmtQBuffer = XrmPermStringToQuark(XmtRBuffer);
    }
}
