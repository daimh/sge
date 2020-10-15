/* 
 * Motif Tools Library, Version 3.1
 * $Id: DECHack.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: DECHack.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * The procedures in this file are only needed with DECWindows Xt
 * libraries (VMS and Ultrix) to work around the fact that calling
 * XtRemoveCallback causes a core dump.
 * This is undeniably gross.
 * See also Xmt.h, where we #define XtAddCallback and XtRemoveCallback
 * to be these functions here.
 * 
 * Note that these are not fully general replacements for the real functions;
 * they don't work if a procedure is registered more than once, with
 * different data on the same list in the same widget.  But this doesn't
 * happen in the Xmt library.
 */

#ifdef DECWINDOWS_CALLBACK_HACK

#include <Xmt/Xmt.h>

/* remove our redefinitions, so we get the real thing here. */
#undef XtAddCallback
#undef XtRemoveCallback

/*
 * We use this no-op procedure as a placeholder, adding it
 * whenever we remove a real callback procedure.  This seems
 * to prevent whatever corruption of the callback list occurs
 */

static void noop(Widget w, XtPointer tag, XtPointer data)
{
    /* do nothing */
}

void _XmtDECAddCallback(Widget w, String name,
			XtCallbackProc proc, XtPointer data)

{
    XtRemoveCallback(w, name, noop, proc);
    XtAddCallback(w, name, proc, data);
}

void _XmtDECRemoveCallback(Widget w, String name,
			   XtCallbackProc proc, XtPointer data)
{
    XtRemoveCallback(w, name, proc, data);
    XtAddCallback(w, name, noop, proc);
}

#else
/* just so the compiler doesn't complain about an empty file. */
static char XmtDummy;
#endif
