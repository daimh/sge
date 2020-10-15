/* 
 * Motif Tools Library, Version 3.1
 * $Id: Dialog.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Dialog.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtDialog_h
#define _XmtDialog_h

/* name and class of special subpart searched for dialog default values */
#define XmtNdefaults		"xmtDefaults"
#define XmtCDefaults		"XmtDefaults"

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtDialogDo(Widget, XtPointer);
extern Boolean XmtDialogDoSync(Widget, XtPointer);

extern void XmtDialogGetDialogValues(Widget, XtPointer);
extern void XmtDialogSetDialogValues(Widget, XtPointer);
extern void XmtDialogGetDefaultValues(Widget, XtPointer, ArgList, Cardinal);

extern XtPointer XmtDialogGetDataAddress(Widget);
extern void XmtDialogSetReturnValue(Widget, XmtWideBoolean);

extern void XmtDialogOkayCallback(Widget, XtPointer, XtPointer);
extern void XmtDialogCancelCallback(Widget, XtPointer, XtPointer);
extern void XmtDialogApplyCallback(Widget, XtPointer, XtPointer);
extern void XmtDialogResetCallback(Widget, XtPointer, XtPointer);
extern void XmtDialogDoneCallback(Widget, XtPointer, XtPointer);

extern void XmtDialogBindResourceList(Widget, XtResourceList, Cardinal);
#else
extern void XmtDialogDo();
extern Boolean XmtDialogDoSync();

extern void XmtDialogGetDialogValues();
extern void XmtDialogSetDialogValues();
extern void XmtDialogGetDefaultValues();

extern XtPointer XmtDialogGetDataAddress();
extern void XmtDialogSetReturnValue();

extern void XmtDialogOkayCallback();
extern void XmtDialogCancelCallback();
extern void XmtDialogApplyCallback();
extern void XmtDialogResetCallback();
extern void XmtDialogDoneCallback();

extern void XmtDialogBindResourceList();
#endif

_XFUNCPROTOEND

#endif    
