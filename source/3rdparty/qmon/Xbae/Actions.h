/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1995-99 Andrew Lister
 *
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
 * $Id: Actions.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Actions.h created by Andrew Lister (6 August, 1995)
 */

#ifndef _Xbae_Actions_h
#define _Xbae_Actions_h

#include "Macros.h"

/*
 * Actions
 */

void xbaeEditCellACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeCancelEditACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeCommitEditACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeSelectCellACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeDefaultActionACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeResizeColumnsACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeTraverseNextACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeTraversePrevACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeProcessDragACT P((Widget, XEvent *, String *, Cardinal *));
void xbaeHandleClick P((Widget, XtPointer, XEvent *, Boolean *));
void xbaeHandleMotionACT P((Widget, XEvent *, String *, Cardinal *));
void xbaePageDownACT P((Widget, XEvent *, String *, Cardinal *));
void xbaePageUpACT P((Widget, XEvent *, String *, Cardinal *));
#endif
