/* 
 * Motif Tools Library, Version 3.1
 * $Id: Icon.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Icon.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Icon.h>
#include <X11/extensions/shape.h>

#if NeedFunctionPrototypes
static void DestroyIconWindow(Widget w, XtPointer tag, XtPointer data)
#else
static void DestroyIconWindow(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XDestroyWindow(XtDisplay(w), (Window)tag);
}

static Boolean shape_supported, shape_queried;

#if NeedFunctionPrototypes
void XmtCreatePixmapIcon(Widget w, Pixmap icon, Pixmap shape)
#else
void XmtCreatePixmapIcon(w, icon, shape)
Widget w;
Pixmap icon;
Pixmap shape;
#endif
{
    Widget shell;
    Display *dpy;
    Window win, root;
    int x, y;
    unsigned width, height, border, depth;
    
    if (!icon) return;
    
    shell = XmtGetShell(w);
    dpy = XtDisplay(shell);
    
    if (!shape_queried) {
	int dummy;
	shape_supported = XShapeQueryExtension(dpy, &dummy, &dummy); 
	shape_queried = True;
    }

    XGetGeometry(dpy, icon, &root, &x, &y, &width, &height, &border, &depth);
    win = XCreateSimpleWindow(dpy, root, 0, 0, width, height,
			      (unsigned) 0, CopyFromParent, CopyFromParent);
    XSetWindowBackgroundPixmap(dpy, win, icon);

    if (shape && shape_supported) {
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, shape, ShapeSet);
	XShapeCombineMask(dpy, win, ShapeClip, 0, 0, shape, ShapeSet);
    }

    XtVaSetValues(shell, XtNiconWindow, win, NULL);
    XtAddCallback(shell, XtNdestroyCallback, DestroyIconWindow,(XtPointer)win);
}

#if NeedFunctionPrototypes
void XmtDestroyPixmapIcon(Widget w)
#else
void XmtDestroyPixmapIcon(w)
Widget w;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    Window win;

    XtVaGetValues(shell, XtNiconWindow, &win, NULL);
    if (win == None) return;

    XtVaSetValues(shell, XtNiconWindow, None, NULL);
    XtRemoveCallback(shell, XtNdestroyCallback,
		     DestroyIconWindow, (XtPointer)win);
    XDestroyWindow(dpy, win);
}

#if NeedFunctionPrototypes
void XmtChangePixmapIcon(Widget w, Pixmap icon, Pixmap shape)
#else
void XmtChangePixmapIcon(w, icon, shape)
Widget w;
Pixmap icon;
Pixmap shape;
#endif
{
    Widget shell = XmtGetShell(w);
    Display *dpy = XtDisplay(shell);
    Window win;

    XtVaGetValues(shell, XtNiconWindow, &win, NULL);
    if (win == None) return;

    if (icon) XSetWindowBackgroundPixmap(dpy, win, icon);
    if (shape && shape_supported) {
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, shape, ShapeSet);
	XShapeCombineMask(dpy, win, ShapeClip, 0, 0, shape, ShapeSet);
    }
    XClearWindow(dpy, win);
}

