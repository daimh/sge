/* 
 * Motif Tools Library, Version 3.1
 * $Id: Visual.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Visual.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

#if NeedFunctionPrototypes
Visual *XmtGetVisual(Widget w)
#else
Visual *XmtGetVisual(w)
Widget w;
#endif
{
    while(!XtIsShell(w)) w = XtParent(w);
    if (((ShellWidget)w)->shell.visual != (Visual *)CopyFromParent) 
	return ((ShellWidget)w)->shell.visual;
    else 
	return DefaultVisualOfScreen(XtScreen(w));
}

/*
 * A function used by some resource converters
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtFetchVisual(Widget w, Cardinal *size, XrmValue *value)
#else
void _XmtFetchVisual(w, size, value)
Widget w;
Cardinal *size;
XrmValue *value;
#endif
{
    while(!XtIsShell(w)) w = XtParent(w);
    value->size = sizeof(Visual *);
    if (((ShellWidget)w)->shell.visual != (Visual *)CopyFromParent) 
	value->addr = (XPointer) &((ShellWidget)w)->shell.visual;
    else 
	value->addr = (XPointer) &DefaultVisualOfScreen(XtScreen(w));
}


static XtInitProc orig_shell_init_proc;

#if NeedFunctionPrototypes
static void XmtShellVisualInheritancePatch(Widget req, Widget new,
					   ArgList args, Cardinal *num_args)
#else
static void XmtShellVisualInheritancePatch(req, new, args, num_args)
Widget req, new;
ArgList args;
Cardinal *num_args;
#endif
{
    ShellWidget w = (ShellWidget) new;

    /* first, call the original init method */
    (*orig_shell_init_proc)(req, new, args, num_args);

    /*
     * then, if visual is still CopyFromParent, then inherit from our
     * closest shell ancestor, if we have one.
     */
    if ((w->shell.visual == CopyFromParent) && XtParent(w)) {
	Widget p;
	for(p = XtParent(w); !XtIsShell(p); p = XtParent(p));
	w->shell.visual = ((ShellWidget)p)->shell.visual;
    }
}


/*
 * Call this procedure before XtAppInitialize, or however you
 * create your toplevel shell. It will munge the Shell widget class
 * so that it handles visual inheritance in the same way it handles
 * depth and colormap inheritance.  This kind of inheritance is not
 * always the desired behavior, but often makes it a lot easier
 * to write applications with a non-default visual.
 */
#if NeedFunctionPrototypes
void XmtPatchVisualInheritance(void)
#else
void XmtPatchVisualInheritance()
#endif
{
    orig_shell_init_proc = shellClassRec.core_class.initialize;
    shellClassRec.core_class.initialize = XmtShellVisualInheritancePatch;
}
