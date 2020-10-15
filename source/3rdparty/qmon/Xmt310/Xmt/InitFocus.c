/* 
 * Motif Tools Library, Version 3.1
 * $Id: InitFocus.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: InitFocus.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Util.h>

#if XmVersion >= 1002  /* Motif 1.2 or later */

/* this is the easy case */
#if NeedFunctionPrototypes
void XmtSetInitialFocus(Widget dialog, Widget initial)
#else
void XmtSetInitialFocus(dialog, initial)
Widget dialog;
Widget initial;
#endif
{
    XtVaSetValues(dialog, XmNinitialFocus, initial, NULL);
}

#else  /* Motif 1.1 or before */

/*
 * This is the hard case.  Prior to Motif 1.2 the initial focus in
 * PromptDialogs goes to the default button, by default.  We really
 * want it on the text widget.  Hitting return still does the right
 * thing (activates the default button) even if we switch focus.
 *
 * The problem is that XmProcessTraversal doesn't work unless the dialog
 * already has the focus, and has done whatever it does when it gets the
 * focus.  Other problems are due to window managers: there is an
 * asyncronous delay between managing a dialog and when it pops up.  Also,
 * under twm, eg. dialogs don't automatically get focus like they do under
 * mwm.
 *
 * According to Kee Hinckley and the Motif FAQ list, the procedures
 * below will work.  They wait until the dialog gets the focus, and then
 * install a zero-length timer (which will be invoked sometime later, after
 * the dialog box has finished responding to the focus in event.  When the
 * timer goes off, things are stable enough that a call to XmProcessTraversal
 * should work.  It is gross, but that is why I'm putting it in a
 * convenence routine.
 */

/* ARGSUSED */
#if NeedFunctionPrototypes
static void got_timer(XtPointer data, XtIntervalId *interval)
#else
static void got_timer(data, interval)
XtPointer data;
XtIntervalId *interval;
#endif
{
    XmProcessTraversal((Widget) data, XmTRAVERSE_CURRENT);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void got_focus(Widget w, XtPointer tag, XtPointer data)
#else
static void got_focus(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XtRemoveCallback(w, XmNfocusCallback, got_focus, tag);
    XtAppAddTimeOut(XtWidgetToApplicationContext(w), 0, got_timer, tag);
}

#if NeedFunctionPrototypes
void XmtSetInitialFocus(Widget dialog, Widget initial)
#else
void XmtSetInitialFocus(dialog, initial)
Widget dialog, initial;
#endif
{
    XtAddCallback(dialog, XmNfocusCallback, got_focus, (XtPointer)initial);
}

#endif
