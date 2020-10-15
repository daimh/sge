/* 
 * Motif Tools Library, Version 3.1
 * $Id: ConvertData.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ConvertData.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Converters.h>
#include <X11/IntrinsicP.h>

/*
 * strings for some representation types
 */
externaldef(xmtconverterstrings)_Xconst char XmtConverterStrings[] = {
    'S','t','r','i','n','g','L','i','s','t',0,
    'P','i','x','m','a','p','L','i','s','t',0,
    'X','m','t','C','o','l','o','r','T','a','b','l','e',0,
    'X','m','t','B','u','t','t','o','n','T','y','p','e',0,
    'B','i','t','m','a','s','k',0
};

/*
 * An XtConvertArgList useful for a number of converters
 */
externaldef(_xmtwidgetconvertarg) XtConvertArgRec _XmtWidgetConvertArg[] = {
  {XtBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.self), sizeof(Widget)}
};

/*
 * another place to register converters so they can be called directly
 * without automatically linking them in.
 */
externaldef(_xmtcallbackconverter) XtTypeConverter _XmtCallbackConverter;
externaldef(_xmtcolortableconverter) XtTypeConverter _XmtColorTableConverter;

#include <X11/CoreP.h>
#include <X11/CompositeP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtFetchRootWidget(Widget w, Cardinal *size, XrmValue *value)
#else
void _XmtFetchRootWidget(w, size, value)
Widget w;
Cardinal *size;
XrmValue *value;
#endif
{
    w = XmtGetApplicationShell(w);
    value->addr = (XPointer) &w->core.self;
    value->size = sizeof(Widget);
}
