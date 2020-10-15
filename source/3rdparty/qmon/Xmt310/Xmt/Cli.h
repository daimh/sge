/* 
 * Motif Tools Library, Version 3.1
 * $Id: Cli.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Cli.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtCli_h
#define _XmtCli_h    

#include <Xm/Text.h>

externalref WidgetClass xmtCliWidgetClass;
typedef struct _XmtCliClassRec *XmtCliWidgetClass;
typedef struct _XmtCliRec *XmtCliWidget;

externalref _Xconst char XmtCliStrings[];
#ifndef XmtNcliTranslations
#define XmtNcliTranslations ((char*)&XmtCliStrings[0])
#endif
#ifndef XmtNdisplayStderr
#define XmtNdisplayStderr ((char*)&XmtCliStrings[16])
#endif
#ifndef XmtNdisplayStdout
#define XmtNdisplayStdout ((char*)&XmtCliStrings[30])
#endif
#ifndef XmtNescapeNewlines
#define XmtNescapeNewlines ((char*)&XmtCliStrings[44])
#endif
#ifndef XmtNfildes
#define XmtNfildes ((char*)&XmtCliStrings[59])
#endif
#ifndef XmtNhistory
#define XmtNhistory ((char*)&XmtCliStrings[66])
#endif
#ifndef XmtNhistoryMaxItems
#define XmtNhistoryMaxItems ((char*)&XmtCliStrings[74])
#endif
#ifndef XmtNhistoryNumItems
#define XmtNhistoryNumItems ((char*)&XmtCliStrings[90])
#endif
#ifndef XmtNinputCallback
#define XmtNinputCallback ((char*)&XmtCliStrings[106])
#endif
#ifndef XmtNpageMode
#define XmtNpageMode ((char*)&XmtCliStrings[120])
#endif
#ifndef XmtNpageString
#define XmtNpageString ((char*)&XmtCliStrings[129])
#endif
#ifndef XmtNpageWidget
#define XmtNpageWidget ((char*)&XmtCliStrings[140])
#endif
#ifndef XmtNprompt
#define XmtNprompt ((char*)&XmtCliStrings[151])
#endif
#ifndef XmtNsaveHistory
#define XmtNsaveHistory ((char*)&XmtCliStrings[158])
#endif
#ifndef XmtNsaveLines
#define XmtNsaveLines ((char*)&XmtCliStrings[170])
#endif
#ifndef XmtCCliTranslations
#define XmtCCliTranslations ((char*)&XmtCliStrings[180])
#endif
#ifndef XmtCDisplayStderr
#define XmtCDisplayStderr ((char*)&XmtCliStrings[196])
#endif
#ifndef XmtCDisplayStdout
#define XmtCDisplayStdout ((char*)&XmtCliStrings[210])
#endif
#ifndef XmtCEscapeNewlines
#define XmtCEscapeNewlines ((char*)&XmtCliStrings[224])
#endif
#ifndef XmtCFildes
#define XmtCFildes ((char*)&XmtCliStrings[239])
#endif
#ifndef XmtCHistoryMaxItems
#define XmtCHistoryMaxItems ((char*)&XmtCliStrings[246])
#endif
#ifndef XmtCHistoryNumItems
#define XmtCHistoryNumItems ((char*)&XmtCliStrings[262])
#endif
#ifndef XmtCPageMode
#define XmtCPageMode ((char*)&XmtCliStrings[278])
#endif
#ifndef XmtCPageString
#define XmtCPageString ((char*)&XmtCliStrings[287])
#endif
#ifndef XmtCPrompt
#define XmtCPrompt ((char*)&XmtCliStrings[298])
#endif
#ifndef XmtCSaveHistory
#define XmtCSaveHistory ((char*)&XmtCliStrings[305])
#endif
#ifndef XmtCSaveLines
#define XmtCSaveLines ((char*)&XmtCliStrings[317])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtCliClear(Widget);
extern void XmtCliPuts(StringConst, Widget);
extern char *XmtCliGets(char *, int, Widget);
extern void XmtCliFlush(Widget);
extern Widget XmtCreateCli(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateScrolledCli(Widget, String, ArgList, Cardinal);
#else
extern void XmtCliClear();
extern void XmtCliPuts();
extern char *XmtCliGets();
extern void XmtCliFlush();
extern Widget XmtCreateCli();
extern Widget XmtCreateScrolledCli();
#endif

#if NeedVarargsPrototypes
extern void XmtCliPrintf(Widget, StringConst, ...) gcc_printf_func(2,3);
#else
extern void XmtCliPrintf();
#endif
_XFUNCPROTOEND

#endif /* _XmtCli_h */
