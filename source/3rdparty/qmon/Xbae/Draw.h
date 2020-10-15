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
 * $Id: Draw.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Draw.h created by Andrew Lister (30 October, 1995)
 */

#ifndef _Xbae_Draw_h
#define _Xbae_Draw_h

#include "Macros.h"

void xbaeComputeCellColors P((XbaeMatrixWidget, int, int, Pixel *, Pixel *));

void xbaeDrawCell P((XbaeMatrixWidget, int, int));
XbaeCellType xbaeGetDrawCellValue P((XbaeMatrixWidget, int, int, String *,
                                      Pixmap *, Pixmap *, int *, int *,
                                      Pixel *, Pixel *, int *));
void xbaeDrawString P((XbaeMatrixWidget mw, Window win, GC gc, String string,
                        int length, int x, int y, int maxlen,
                        unsigned char alignment, Boolean highlight,
                        Boolean bold, Boolean rowLabel,
                        Boolean colLabel, Pixel color));
void xbaeDrawColumnLabel P((XbaeMatrixWidget mw, int column,
                             Boolean pressed));
void xbaeDrawRowLabel P((XbaeMatrixWidget mw, int row, Boolean pressed));


#endif

