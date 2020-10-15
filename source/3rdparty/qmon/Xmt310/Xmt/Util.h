/* 
 * Motif Tools Library, Version 3.1
 * $Id: Util.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Util.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtUtil_h
#define _XmtUtil_h

/*
 * constants for XmtFindFile()
 */
#define XmtSearchPathOnly    0x00
#define XmtSearchUserPath    0x01
#define XmtSearchAppPath     0x02
#define XmtSearchSysPath     0x04
#define XmtSearchEverywhere  \
    (XmtSearchUserPath | XmtSearchAppPath | XmtSearchSysPath)

_XFUNCPROTOBEGIN    
#if NeedFunctionPrototypes
extern void XmtRegisterAll(void);
extern Widget XmtInitialize(XtAppContext *, String,
			    XrmOptionDescList, Cardinal, int *, String *,
			    String *, ArgList, Cardinal);
extern void XmtBlock(Widget, Boolean *);
extern Widget XmtGetApplicationShell(Widget) gcc_const_attribute;
extern Widget XmtGetTopLevelShell(Widget) gcc_const_attribute;
extern Widget XmtGetShell(Widget) gcc_const_attribute;
extern String XmtFindFile(Widget, StringConst, StringConst,
			  StringConst, StringConst, StringConst, int);
extern XmString XmtCreateXmString(StringConst);
extern XmString XmtCreateLocalizedXmString(Widget, StringConst);
extern Widget XmtNameToWidget(Widget, StringConst);
extern void XmtWaitUntilMapped(Widget);
extern void XmtDisplayBusyCursor(Widget);
extern void XmtDisplayDefaultCursor(Widget);
extern void XmtDisplayCursor(Widget, Cursor);
extern void XmtDiscardButtonEvents(Widget);
extern void XmtDiscardKeyPressEvents(Widget);
extern void XmtSetInitialFocus(Widget, Widget);
extern int XmtBSearch(StringConst, String *, int);
extern String XmtGetHomeDir(void);
extern void XmtAddDeleteCallback(Widget, int, XtCallbackProc, XtPointer);
extern void XmtAddSaveYourselfCallback(Widget, XtCallbackProc, XtPointer);
extern Boolean XmtCheckPrintfFormat(StringConst, StringConst);
extern void XmtIconifyShell(Widget);
extern void XmtDeiconifyShell(Widget);
extern void XmtRaiseShell(Widget);
extern void XmtLowerShell(Widget);
extern void XmtFocusShell(Widget);
extern void XmtSetFocusToShell(Widget);
extern void XmtWarpToShell(Widget);
extern void XmtMoveShellToPointer(Widget);
extern String _XmtLocalize(Screen* , StringConst, StringConst, StringConst);
extern String XmtLocalize(Widget, StringConst, StringConst);
extern String XmtLocalizeWidget(Widget, StringConst, StringConst);
extern String XmtLocalize2(Widget, StringConst, StringConst, StringConst);
extern Visual *XmtGetVisual(Widget);
extern void XmtPatchVisualInheritance(void);
#else
extern void XmtRegisterAll();
extern Widget XmtInitialize();
extern void XmtBlock();
extern Widget XmtGetApplicationShell();
extern Widget XmtGetTopLevelShell();
extern Widget XmtGetShell();
extern String XmtFindFile();
extern XmString XmtCreateXmString();
extern XmString XmtCreateLocalizedXmString();
extern Widget XmtNameToWidget();
extern void XmtWaitUntilMapped();
extern void XmtDisplayBusyCursor();
extern void XmtDisplayDefaultCursor();
extern void XmtDisplayCursor();
extern void XmtDiscardButtonEvents();
extern void XmtDiscardKeyPressEvents();
extern void XmtSetInitialFocus();
extern int XmtBSearch();
extern String XmtGetHomeDir();
extern void XmtAddDeleteCallback();
extern void XmtAddSaveYourselfCallback();
extern Boolean XmtCheckPrintfFormat();
extern void XmtIconifyShell();
extern void XmtDeiconifyShell();
extern void XmtRaiseShell();
extern void XmtLowerShell();
extern void XmtFocusShell();
extern void XmtSetFocusToShell();
extern void XmtWarpToShell();
extern void XmtMoveShellToPointer();
extern String _XmtLocalize();
extern String XmtLocalize();
extern String XmtLocalizeWidget();
extern String XmtLocalize2();
extern Visual *XmtGetVisual();
extern void XmtPatchVisualInheritance();
#endif

#if NeedVarargsPrototypes
extern void XmtErrorMsg(String, String, String, ...) gcc_printf_func(3,4);
extern void XmtWarningMsg(String, String, String, ...) gcc_printf_func(3,4);
#else
extern void XmtErrorMsg();
extern void XmtWarningMsg();
#endif

/*
 * like assert(), XmtAssertWidgetClass() is only defined ifndef NDEBUG
 */
#ifndef NDEBUG
#if NeedFunctionPrototypes
extern void XmtAssertWidgetClass(Widget, WidgetClass, String);
#else
extern void XmtAssertWidgetClass();
#endif
#else
#define XmtAssertWidgetClass(w, c, msg)
#endif /* NDEBUG */
_XFUNCPROTOEND

#endif /* _XmtUtil_h */
