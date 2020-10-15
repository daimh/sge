/* 
 * Motif Tools Library, Version 3.1
 * $Id: Dialogs.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Dialogs.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <X11/IntrinsicP.h>
#include <X11/CompositeP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtOkCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
void _XmtOkCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    info->blocked = False;
    info->button = XmtOkButton;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtYesCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
void _XmtYesCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    info->blocked = False;
    info->button = XmtYesButton;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtNoCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
void _XmtNoCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    info->blocked = False;
    info->button = XmtNoButton;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtCancelCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
void _XmtCancelCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    
    info->blocked = False;
    info->button = XmtCancelButton;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtHelpCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
void _XmtHelpCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;

    /*
     * Display a help dialog.
     * Use NULL dialog name and NULL title so we get the default.
     * We make this a modal dialog, even though info dialogs are generally
     * modeless because if we use a modeless one the user can't pop it down
     * until the modal dialog underneath is popped down.  I don't know if
     * this is a bug in motif, or is because the help dialog is not a child
     * of the modal dialog underneath, but it is bad behavior, so we work
     * around it by using a modal help dialog.  Also, this is good because
     * we don't have to arrange to automatically pop down the help dialog when
     * the modal underneath is popped down; the user can't pop down the modal
     * until he dismisses the help.
     */
    (void) _XmtDisplayMessage(w, NULL, NULL, info->help_text, NULL,
			      NULL, NULL, None,
			      XmDIALOG_INFORMATION,
			      XmDIALOG_PRIMARY_APPLICATION_MODAL);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtDialogPopdownCallback(Widget w, XtPointer tag, XtPointer data)
#else
void _XmtDialogPopdownCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtDialogCache *cache = (XmtDialogCache *)tag;
    Widget dialog;

    /*
     * if there is already a free dialog, destroy this one
     * and remove it from the cache.  Otherwise, just note that
     * we now have one free.
     */
    if (cache->in_use < cache->num) {
	int i;

	/*
	 * w is the shell.  Get the dialog child
	 */
	dialog = ((CompositeWidget)w)->composite.children[0];
	for(i=0; i < cache->num; i++)
	    if (cache->dialogs[i] == dialog) break;

	XtDestroyWidget(dialog);

	/* now compress the remaining array elements */
	for(; i < cache->num-1; i++) cache->dialogs[i] = cache->dialogs[i+1];

	cache->num--;
    }
    
    cache->in_use--;
}

