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
 * $Id: Methods.h,v 1.1 2001-07-18 11:06:00 root Exp $
 */


/*
 * Methods.h created by Andrew Lister (7 August, 1995)
 */
#ifndef _Xbae_Methods_h
#define _Xbae_Methods_h

#include "Macros.h"

void xbaeResize P((XbaeMatrixWidget));

/*
 * New Matrix methods
 */
void xbaeSetCell P((XbaeMatrixWidget, int, int, const String, Boolean));
void xbaeModifyVerifyCB P((Widget, XtPointer, XtPointer));
void xbaeEditCell P((XbaeMatrixWidget, XEvent *, int, int,
                     String *, Cardinal));
void xbaeSelectCell P((XbaeMatrixWidget, int, int));
void xbaeSelectRow P((XbaeMatrixWidget, int));
void xbaeSelectColumn P((XbaeMatrixWidget, int));
void xbaeDeselectAll P((XbaeMatrixWidget));
void xbaeSelectAll P((XbaeMatrixWidget));
void xbaeDeselectCell P((XbaeMatrixWidget, int, int));
void xbaeDeselectRow P((XbaeMatrixWidget, int));
void xbaeDeselectColumn P((XbaeMatrixWidget, int));
String xbaeGetCell P((XbaeMatrixWidget, int, int));
Boolean xbaeCommitEdit P((XbaeMatrixWidget, XEvent *, Boolean));
void xbaeCancelEdit P((XbaeMatrixWidget, Boolean));
void xbaeAddRows P((XbaeMatrixWidget, int, String *, String *, Pixel *,
                    Pixel *, int));
void xbaeDeleteRows P((XbaeMatrixWidget, int, int));
void xbaeAddColumns P((XbaeMatrixWidget, int, String *, String *, short *,
                        int *, unsigned char *, unsigned char *, Pixel *,
                        Pixel *, int));
void xbaeDeleteColumns P((XbaeMatrixWidget, int, int));
void xbaeSetRowColors P((XbaeMatrixWidget, int, Pixel *, int, Boolean));
void xbaeSetColumnColors P((XbaeMatrixWidget, int,
                             Pixel *, int, Boolean));
void xbaeSetCellColor P((XbaeMatrixWidget, int, int, Pixel, Boolean));

#endif
