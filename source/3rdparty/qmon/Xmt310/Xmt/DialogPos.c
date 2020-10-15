/* 
 * Motif Tools Library, Version 3.1
 * $Id: DialogPos.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: DialogPos.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Dialogs.h>
#include <X11/IntrinsicP.h>

static Arg position_args[2] = {
   {XtNx, 0}, {XtNy, 0}
};

#if NeedFunctionPrototypes
void XmtDialogPosition(Widget dialog, Widget ref)
#else
void XmtDialogPosition(dialog, ref)
Widget dialog;
Widget ref;
#endif
{
    Position refx, refy;
    int x, y;
    int sw = WidthOfScreen(XtScreen(dialog));
    int sh = HeightOfScreen(XtScreen(dialog));

    if (!XtIsRealized(dialog)) XtRealizeWidget(dialog);

    /* get the absolute position of the center of the ref widget. */
    XtTranslateCoords(ref,ref->core.width/2, ref->core.height/2, &refx, &refy);

    /* get the absolute position of the ul of the centered dialog */
    x = refx - dialog->core.width/2;
    y = refy - dialog->core.height/2;

    /* adjust if it goes off the screen. */
    if (x < 0) x = 0;
    else if (x + dialog->core.width > sw)
	x = sw - dialog->core.width;

    if (y < 0) y = 0;
    else if (y + dialog->core.height > sh)
	y = sh - dialog->core.height;

    position_args[0].value = (XtArgVal)x;
    position_args[1].value = (XtArgVal)y;
    XtSetValues(dialog, position_args, 2);
}
