/* 
 * Motif Tools Library, Version 3.1
 * $Id: Screen.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Screen.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/AppResP.h>
#include <Xmt/Hash.h>
#include <X11/IntrinsicP.h>


/* ARGSUSED */
#if NeedFunctionPrototypes
static void DestroyCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void DestroyCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    XmtAppResources *app = XmtGetApplicationResources(w);

    /* remove the info structure from the per-screen hash table */
    XmtHashTableDelete(app->screen_table, (XtPointer)XtScreen(w));

    /*
     * Free array of dialogs.
     * The dialogs themselves will be destroyed by whatever destroyed
     * the shell.
     */
    XtFree((char *)info->help_dialog_cache.dialogs);

    /* free the info structure itself */
    XtFree((char *)info);
}

/*
 * We cache dialogs by appshell and by screen.
 * XXX a 2D hashtable would be nice here, so we don't do a double lookup.
 */
#if NeedFunctionPrototypes
XmtPerScreenInfo *XmtGetPerScreenInfo(Widget w)
#else
XmtPerScreenInfo *XmtGetPerScreenInfo(w)
Widget w;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);
    XmtPerScreenInfo *info;
    Screen *screen = XtScreenOfObject(w);
    Widget shell;
    Boolean status;

    status = XmtHashTableLookup(app->screen_table,
				(XtPointer) screen,
				(XtPointer *) &info);

    if (!status) {
	info = (XmtPerScreenInfo *) XtCalloc(1, sizeof(XmtPerScreenInfo));
	/*
	 * the info->topmost_shell field is the highest level shell on this
	 * screen within the widget hierarchy.  It might be the root shell or
	 * it might not be.  I assert that the algorithm below always yields
	 * a shell widget, but it doesn't really matter.
	 */
	for(shell=NULL; w; shell = w, w = XtParent(w)) {
	    if (!XtIsWidget(w)) continue;
	    if (w->core.screen != screen) break;
	}

	info->topmost_shell = shell;
	XtAddCallback(shell, XtNdestroyCallback,
		      DestroyCallback, (XtPointer) info);
	XmtHashTableStore(app->screen_table,
			  (XtPointer) screen,
			  (XtPointer) info);
    }

    return info;
}
