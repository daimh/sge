/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-99 Andrew Lister
 *                        All rights reserved
 * Permission to use, copy, modify and distribute this material for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Bellcore not be used in advertising
 * or publicity pertaining to this material without the specific,
 * prior written permission of an authorized representative of
 * Bellcore.
 *
 * BELLCORE MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES, EX-
 * PRESS OR IMPLIED, WITH RESPECT TO THE SOFTWARE, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR ANY PARTICULAR PURPOSE, AND THE WARRANTY AGAINST IN-
 * FRINGEMENT OF PATENTS OR OTHER INTELLECTUAL PROPERTY RIGHTS.  THE
 * SOFTWARE IS PROVIDED "AS IS", AND IN NO EVENT SHALL BELLCORE OR
 * ANY OF ITS AFFILIATES BE LIABLE FOR ANY DAMAGES, INCLUDING ANY
 * LOST PROFITS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES RELAT-
 * ING TO THE SOFTWARE.
 *
 * $Id: ScrollMgr.h,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * ScrollMgr.h created by Andrew Lister (6 August, 1995)
 */

#ifndef _Xbae_ScrollMgr_h
#define _Xbae_ScrollMgr_h

#include "Macros.h"
/*
 * ScrollMgr implementation
 */

SmScrollMgr xbaeSmCreateScrollMgr P((void));
void xbaeSmDestroyScrollMgr P((SmScrollMgr)); 
void xbaeSmAddScroll P((SmScrollMgr, int, int));
void xbaeSmRemoveScroll P((SmScrollMgr));
void xbaeSmScrollEvent P((SmScrollMgr, XEvent *));

/*
 * Scrollbar callbacks
 */
void xbaeScrollVertCB P((Widget, XtPointer, XmScrollBarCallbackStruct *));
void xbaeScrollHorizCB P((Widget, XtPointer , XmScrollBarCallbackStruct *));

void xbaeRedrawCells P((XbaeMatrixWidget, Rectangle *));
void xbaeRedrawLabelsAndFixed P((XbaeMatrixWidget, Rectangle *));

#endif
