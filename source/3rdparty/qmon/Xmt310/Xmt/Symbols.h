/* 
 * Motif Tools Library, Version 3.1
 * $Id: Symbols.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Symbols.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtSymbols_h
#define _XmtSymbols_h    

#include <Xmt/Hash.h>

typedef struct _XmtSymbolRec *XmtSymbol;

typedef void (*XmtSymbolCallbackProc)(
#if NeedFunctionPrototypes
				      XmtSymbol, XtPointer, XtArgVal
#endif
				      );

typedef struct _XmtSymbolCallbackRec {
    XmtSymbolCallbackProc proc;
    XtPointer closure;
    struct _XmtSymbolCallbackRec *next;
} XmtSymbolCallbackRec;

typedef struct _XmtSymbolRec {       /* app programmer declares these */
    String name;
    String type;
    Cardinal size;
    XtPointer addr;
    String resource_name;
    short num_values;
    XtPointer *values;
    /* private, don't initialize */
    unsigned char mode;
    XmtSymbolCallbackRec *callbacks;
    unsigned short index;
} XmtSymbolRec;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtRegisterSymbol(XmtSymbol);
extern void XmtRegisterSymbols(XmtSymbolRec *, Cardinal);
extern void XmtRegisterResourceList(XtPointer, XtResourceList, Cardinal);
extern void XmtRegisterWidgetResource(StringConst, Widget, StringConst);
extern XmtSymbol XmtLookupSymbol(StringConst);
extern void XmtSymbolSetValue(XmtSymbol, XtArgVal);
extern void XmtSymbolGetValue(XmtSymbol, XtArgVal *);
extern Boolean XmtSymbolSetTypedValue(Widget, XmtSymbol, String,
				      XtArgVal, Cardinal);
extern String XmtSymbolName(XmtSymbol);
extern int XmtSymbolSize(XmtSymbol);
extern XrmQuark XmtSymbolTypeQuark(XmtSymbol);
extern String XmtSymbolType(XmtSymbol);
extern Boolean XmtSymbolIsEnumerated(XmtSymbol);
extern void XmtSymbolAddCallback(XmtSymbol, XmtSymbolCallbackProc, XtPointer);
extern void XmtSymbolRemoveCallback(XmtSymbol,XmtSymbolCallbackProc,XtPointer);
#else
extern void XmtRegisterSymbol();
extern void XmtRegisterSymbols();
extern void XmtRegisterResourceList();
extern void XmtRegisterWidgetResource();
extern XmtSymbol XmtLookupSymbol();
extern void XmtSymbolSetValue();
extern void XmtSymbolGetValue();
extern Boolean XmtSymbolSetTypedValue();
extern String XmtSymbolName();
extern int XmtSymbolSize();
extern XrmQuark XmtSymbolTypeQuark();
extern String XmtSymbolType();
extern Boolean XmtSymbolIsEnumerated();
extern void XmtSymbolAddCallback();
extern void XmtSymbolRemoveCallback();
#endif

#if NeedVarargsPrototypes
extern void XmtVaRegisterSymbols(StringConst, ...);
#else
extern void XmtVaRegisterSymbols();
#endif
_XFUNCPROTOEND

#endif /* _XmtSymbols_h */
