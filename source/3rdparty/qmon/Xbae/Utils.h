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
 * $Id: Utils.h,v 1.1 2001-07-18 11:06:00 root Exp $
 */

/*
 * Utils.h created by Andrew Lister (6 August, 1995)
 */
#ifndef _Xbae_Utils_h
#define _Xbae_Utils_h

#include "Macros.h"

void xbaeGetVisibleRows P((XbaeMatrixWidget, int *, int *));
void xbaeGetVisibleColumns P((XbaeMatrixWidget, int *, int *));
void xbaeGetVisibleCells P((XbaeMatrixWidget mw, int *, int *, int *,
                            int *));
void xbaeClearCell P((XbaeMatrixWidget, int, int));
void xbaeMakeRowVisible P((XbaeMatrixWidget, int));
void xbaeMakeColumnVisible P((XbaeMatrixWidget, int));
void xbaeMakeCellVisible P((XbaeMatrixWidget, int, int));
void xbaeAdjustTopRow P((XbaeMatrixWidget));
void xbaeAdjustLeftColumn P((XbaeMatrixWidget));
Boolean xbaeIsRowVisible P((XbaeMatrixWidget, int));
Boolean xbaeIsColumnVisible P((XbaeMatrixWidget, int));
Boolean xbaeIsCellVisible P((XbaeMatrixWidget, int, int));
void xbaeSetClipMask P((XbaeMatrixWidget, unsigned int));
void xbaeGetCellTotalWidth P((XbaeMatrixWidget));
void xbaeGetColumnPositions P((XbaeMatrixWidget));
void xbaeComputeSize P((XbaeMatrixWidget, Boolean, Boolean));
short xbaeMaxRowLabel P((XbaeMatrixWidget));
void xbaeParseColumnLabel P((String, ColumnLabelLines));
Boolean xbaeEventToXY P((XbaeMatrixWidget, XEvent *, int *, int *,
                         CellType *));
Boolean xbaeXYToRowCol P((XbaeMatrixWidget, int *, int *, int *, int *,
                          CellType));
int xbaeXtoCol P((XbaeMatrixWidget, int));
int xbaeXtoTrailingCol P((XbaeMatrixWidget, int));
void xbaeRowColToXY P((XbaeMatrixWidget, int, int, int *, int *));
Window xbaeGetCellWindow P((XbaeMatrixWidget, Widget *, int, int));
void xbaeCalcVertFill P((XbaeMatrixWidget, Window, int, int, int, int,
                         int *, int *, int *, int *));
void xbaeCalcHorizFill P((XbaeMatrixWidget, Window, int, int, int, int,
                          int *, int *, int *, int *));
#endif
