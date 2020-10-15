/* 
 * Motif Tools Library, Version 3.1
 * $Id: Cursor.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Cursor.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/AppResP.h>

#if NeedFunctionPrototypes
void XmtDisplayBusyCursor(Widget w)
#else
void XmtDisplayBusyCursor(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    XmtAppResources *app = XmtGetApplicationResources(shell);
    
    if (!XtIsRealized(shell)) return;
    XDefineCursor(dpy, XtWindow(shell), app->busy_cursor);
    XFlush(dpy);
}

#if NeedFunctionPrototypes
void XmtDisplayDefaultCursor(Widget w)
#else
void XmtDisplayDefaultCursor(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    XmtAppResources *app = XmtGetApplicationResources(shell);
    
    if (!XtIsRealized(shell)) return;
    XDefineCursor(dpy, XtWindow(shell), app->cursor);
    XFlush(dpy);
}


#if NeedFunctionPrototypes
void XmtDisplayCursor(Widget w, Cursor c)
#else
void XmtDisplayCursor(w, c)
Widget w;
Cursor c;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    
    if (!XtIsRealized(shell)) return;
    XDefineCursor(dpy, XtWindow(shell), c);
    XFlush(dpy);
}

