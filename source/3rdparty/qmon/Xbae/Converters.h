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
 * $Id: Converters.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifndef _Xbae_Converters_h
#define _Xbae_Converters_h

/*
 * Converters.h created by Andrew Lister (6 August, 1995)
 */

#include <Xm/Xm.h>
#include "Macros.h"
/*
 * Type converters
 */
Boolean CvtStringToStringArray P((Display *, XrmValuePtr, Cardinal *,
                                   XrmValuePtr, XrmValuePtr, XtPointer *));
void StringArrayDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                               XrmValuePtr, Cardinal *));
Boolean CvtStringToCellTable P((Display *, XrmValuePtr, Cardinal *,
                                 XrmValuePtr, XrmValuePtr, XtPointer *));
void StringCellDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                              XrmValuePtr, Cardinal *));
Boolean CvtStringToWidthArray P((Display *, XrmValuePtr, Cardinal *,
                                  XrmValuePtr, XrmValuePtr, XtPointer *));
void WidthArrayDestructor  P((XtAppContext, XrmValuePtr, XtPointer,
                               XrmValuePtr, Cardinal *));
Boolean CvtStringToMaxLengthArray P((Display *, XrmValuePtr, Cardinal *,
                                      XrmValuePtr, XrmValuePtr, XtPointer *));
void MaxLengthArrayDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                                  XrmValuePtr, Cardinal *));
Boolean CvtStringToBooleanArray P((Display *, XrmValuePtr, Cardinal *,
                                    XrmValuePtr, XrmValuePtr, XtPointer *));
void BooleanArrayDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                                XrmValuePtr, Cardinal *));
Boolean CvtStringToAlignmentArray P((Display *, XrmValuePtr, Cardinal *,
                                      XrmValuePtr, XrmValuePtr, XtPointer *));
void AlignmentArrayDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                                  XrmValuePtr, Cardinal *num_args));
Boolean CvtStringToPixelTable P((Display *, XrmValuePtr, Cardinal *,
                                  XrmValuePtr, XrmValuePtr, XtPointer *));
void PixelTableDestructor P((XtAppContext, XrmValuePtr, XtPointer,
                                  XrmValuePtr, Cardinal *));
Boolean CvtStringToGridType P((Display *, XrmValuePtr, Cardinal *,
                                XrmValuePtr, XrmValuePtr, XtPointer *));

Boolean
#ifdef __VMS
CvtStringToMatrixScrollBarDisp
#else
CvtStringToMatrixScrollBarDisplayPolicy
#endif
                                        P((Display *, XrmValuePtr,
                                           Cardinal *, XrmValuePtr,
                                           XrmValuePtr,
                                           XtPointer *));

#endif
