/* 
 * Motif Tools Library, Version 3.1
 * $Id: MsgLine.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MsgLine.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtMsgLine_h
#define _XmtMsgLine_h    

#include <Xm/Text.h>

externalref WidgetClass xmtMsgLineWidgetClass;
typedef struct _XmtMsgLineClassRec *XmtMsgLineWidgetClass;
typedef struct _XmtMsgLineRec *XmtMsgLineWidget;

/*
 * values for the when argument to XmtMsgLineClear and XmtMsgLinePop
 */
#define XmtMsgLineNow 0
#define XmtMsgLineOnAction -1
 

externalref _Xconst char XmtMsgLineStrings[];
#ifndef XmtNallowAsyncInput
#define XmtNallowAsyncInput ((char*)&XmtMsgLineStrings[0])
#endif
#ifndef XmtNinputCallback
#define XmtNinputCallback ((char*)&XmtMsgLineStrings[16])
#endif
#ifndef XmtNmsgLineTranslations
#define XmtNmsgLineTranslations ((char*)&XmtMsgLineStrings[30])
#endif
#ifndef XmtCAllowAsyncInput
#define XmtCAllowAsyncInput ((char*)&XmtMsgLineStrings[50])
#endif
#ifndef XmtCMsgLineTranslations
#define XmtCMsgLineTranslations ((char*)&XmtMsgLineStrings[66])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtMsgLineClear(Widget, int);
extern void XmtMsgLineSet(Widget, StringConst);
extern void XmtMsgLineAppend(Widget, StringConst);
extern void XmtMsgLinePush(Widget);
extern void XmtMsgLinePop(Widget, int);
extern void XmtMsgLineSetInput(Widget, StringConst);
extern String XmtMsgLineGetInput(Widget);
extern String XmtMsgLineGetString(Widget, char *, int);
extern int XmtMsgLineGetChar(Widget);
extern Boolean XmtMsgLineGetUnsigned(Widget, unsigned int *);
extern Boolean XmtMsgLineGetInt(Widget, int *);
extern Boolean XmtMsgLineGetDouble(Widget, double *);
extern Widget XmtCreateMsgLine(Widget, String, ArgList, Cardinal);
#else
extern void XmtMsgLineClear();
extern void XmtMsgLineSet();
extern void XmtMsgLineAppend();
extern void XmtMsgLinePush();
extern void XmtMsgLinePop();
extern void XmtMsgLineSetInput();
extern String XmtMsgLineGetInput();
extern String XmtMsgLineGetString();
extern int XmtMsgLineGetChar();
extern Boolean XmtMsgLineGetUnsigned();
extern Boolean XmtMsgLineGetInt();
extern Boolean XmtMsgLineGetDouble();
extern Widget XmtCreateMsgLine();
#endif

#if NeedVarargsPrototypes
extern void XmtMsgLinePrintf(Widget w,StringConst fmt,...)gcc_printf_func(2,3);
#else
extern void XmtMsgLinePrintf();
#endif

_XFUNCPROTOEND

#endif /* _XmtMsgLine_h */
