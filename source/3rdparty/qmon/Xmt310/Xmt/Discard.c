/* 
 * Motif Tools Library, Version 3.1
 * $Id: Discard.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Discard.c,v $
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

#if NeedFunctionPrototypes
void XmtDiscardButtonEvents(Widget w)
#else
void XmtDiscardButtonEvents(w)
Widget w;
#endif
{
    Display *dpy = XtDisplay(w);
    XEvent event;
    
    XSync(dpy, False);  /* get any events from the server */
    while (XCheckMaskEvent(dpy,
			   ButtonPressMask | ButtonReleaseMask |
			   ButtonMotionMask | PointerMotionMask,
			   &event))
	;
}


#if NeedFunctionPrototypes
void XmtDiscardKeyPressEvents(Widget w)
#else
void XmtDiscardKeyPressEvents(w)
Widget w;
#endif
{
    Display *dpy = XtDisplay(w);
    XEvent event;
    
    XSync(dpy, False);  /* get any events from the server */
    while (XCheckMaskEvent(dpy, KeyPressMask, &event))
	;
}

