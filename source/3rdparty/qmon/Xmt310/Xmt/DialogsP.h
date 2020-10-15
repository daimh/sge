/* 
 * Motif Tools Library, Version 3.1
 * $Id: DialogsP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: DialogsP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtDialogsP_h
#define _XmtDialogsP_h

#include <Xmt/Dialogs.h>

typedef struct {
    Widget *dialogs;
    short num, max, in_use;
} XmtDialogCache;


_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void _XmtOkCallback(Widget, XtPointer, XtPointer);
extern void _XmtYesCallback(Widget, XtPointer, XtPointer);
extern void _XmtNoCallback(Widget, XtPointer, XtPointer);
extern void _XmtCancelCallback(Widget, XtPointer, XtPointer);
extern void _XmtHelpCallback(Widget, XtPointer, XtPointer);
extern void _XmtDialogPopdownCallback(Widget, XtPointer, XtPointer);
extern Boolean * _XmtDisplayMessage(Widget, StringConst, StringConst,
				    StringConst, va_list *,
				    StringConst, StringConst, Pixmap,
				    int, int);
#else
extern void _XmtOkCallback();
extern void _XmtYesCallback();
extern void _XmtNoCallback();
extern void _XmtCancelCallback();
extern void _XmtHelpCallback();
extern void _XmtDialogPopdownCallback();
extern Boolean *_XmtDisplayMessage();
#endif
_XFUNCPROTOEND
    
#endif  /* _XmtDialogsP_h */
