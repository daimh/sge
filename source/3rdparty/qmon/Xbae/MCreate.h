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
 * $Id: MCreate.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Create.h created by Andrew Lister (28 January, 1996)
 */

#ifndef _Xbae_Create_h
#define _Xbae_Create_h

#include "Macros.h"

void xbaeCopyBackground P((Widget, int, XrmValue *));
void xbaeCopyForeground P((Widget, int, XrmValue *));
void xbaeCopyDoubleClick P((Widget, int, XrmValue *));
void xbaeCopyCellShadowTypes P((XbaeMatrixWidget));
void xbaeCopyRowShadowTypes P((XbaeMatrixWidget));
void xbaeCopyColumnShadowTypes P((XbaeMatrixWidget));
void xbaeCopyCellUserData P((XbaeMatrixWidget));
void xbaeCopyRowUserData P((XbaeMatrixWidget));
void xbaeCopyColumnUserData P((XbaeMatrixWidget));
void xbaeCopySelectedCells P((XbaeMatrixWidget));
void xbaeCopyRowLabels P((XbaeMatrixWidget));
void xbaeCopyColumnLabels P((XbaeMatrixWidget));
void xbaeCopyCells P((XbaeMatrixWidget));
#if CELL_WIDGETS
void xbaeCopyCellWidgets P((XbaeMatrixWidget));
#endif
void xbaeCopyColumnWidths  P((XbaeMatrixWidget));
void xbaeCopyColumnMaxLengths P((XbaeMatrixWidget));
void xbaeCopyBackgrounds P((XbaeMatrixWidget));
void xbaeCopyColumnAlignments P((XbaeMatrixWidget));
void xbaeCopyColumnLabelAlignments P((XbaeMatrixWidget));
void xbaeCopyColumnButtonLabels P((XbaeMatrixWidget));
void xbaeCopyRowButtonLabels P((XbaeMatrixWidget));
void xbaeCopyColors P((XbaeMatrixWidget));
#if XmVersion >= 1002
void xbaeCopyHighlightedCells P((XbaeMatrixWidget));
#endif
void xbaeCreateDrawGC P((XbaeMatrixWidget));
void xbaeCreatePixmapGC P((XbaeMatrixWidget));
void xbaeCreateLabelGC P((XbaeMatrixWidget));
void xbaeCreateLabelClipGC P((XbaeMatrixWidget));
void xbaeCreateGridLineGC P((XbaeMatrixWidget));
void xbaeCreateTopShadowClipGC P((XbaeMatrixWidget));
void xbaeCreateBottomShadowClipGC P((XbaeMatrixWidget));
void xbaeNewFont P((XbaeMatrixWidget));
void xbaeNewLabelFont P((XbaeMatrixWidget));
void xbaeFreeCells P((XbaeMatrixWidget));
#if CELL_WIDGETS
void xbaeFreeCellWidgets P((XbaeMatrixWidget));
#endif
void xbaeFreeRowLabels P((XbaeMatrixWidget));
void xbaeFreeColumnLabels P((XbaeMatrixWidget));
void xbaeFreeColors P((XbaeMatrixWidget));
void xbaeFreeBackgrounds P((XbaeMatrixWidget));
void xbaeFreeSelectedCells P((XbaeMatrixWidget));
void xbaeFreeCellUserData P((XbaeMatrixWidget));
void xbaeFreeCellShadowTypes P((XbaeMatrixWidget));
#if XmVersion >= 1002
void xbaeFreeHighlightedCells P((XbaeMatrixWidget));
#endif
void xbaeCreateColors P((XbaeMatrixWidget));

#endif
