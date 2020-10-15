/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
 * Copyright(c) 1999 Andrew Lister
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
 * CaptionWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 * 
 * $Id: Caption.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifndef _Xbae_Caption_h
#define _Xbae_Caption_h

/*
 * Caption Widget public include file
 */

#include <Xm/Xm.h>
#include <X11/Core.h>
#include "patchlevel.h"


#ifndef XlibSpecificationRelease
# ifndef _XFUNCPROTOBEGIN
#   ifdef __cplusplus                      /* for C++ V2.0 */
#     define _XFUNCPROTOBEGIN extern "C" {   /* do not leave open across includes */
#     define _XFUNCPROTOEND }
#   else
#     define _XFUNCPROTOBEGIN
#     define _XFUNCPROTOEND
#   endif
# endif /* _XFUNCPROTOBEGIN */
#else
#include <X11/Xfuncproto.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Resources:
 * Name                        Class                RepType                Default Value
 * ----                        -----                -------                -------------
 * fontList                FontList        FontList        dynamic
 * labelAlignment        LabelAlignment        LabelAlignment        AlignmentCenter
 * labelOffset                LabelOffset        int                0
 * labelPixmap                LabelPixmap        PrimForegroundPixmap
 *                                                        XmUNSPECIFIED_PIXMAP
 * labelPosition        LabelPosition        LabelPosition        PositionLeft
 * labelString                XmString        XmString        widget name
 * labelTextAlignment        Alignment        Alignment        XmALIGNMENT_CENTER
 * labelType                LabelType        LabelType        XmSTRING
 */

/*
 * New resource constants
 */
#ifndef XmNlabelPosition
#define XmNlabelPosition "labelPosition"
#endif
#ifndef XmCLabelPosition
#define XmCLabelPosition "LabelPosition"
#endif
#ifndef XmNlabelAlignment
#define XmNlabelAlignment "labelAlignment"
#endif
#ifndef XmCLabelAlignment
#define XmCLabelAlignment "LabelAlignment"
#endif
#ifndef XmNlabelTextAlignment
#define XmNlabelTextAlignment "labelTextAlignment"
#endif
#ifndef XmNlabelOffset
#define XmNlabelOffset "labelOffset"
#endif
#ifndef XmCLabelOffset
#define XmCLabelOffset "LabelOffset"
#endif

#ifndef XmRLabelPosition
#define XmRLabelPosition "LabelPosition"
#endif
#ifndef XmRLabelAlignment
#define XmRLabelAlignment "LabelAlignment"
#endif

/* Class record constants */

extern WidgetClass xbaeCaptionWidgetClass;

typedef struct _XbaeCaptionClassRec *XbaeCaptionWidgetClass;
typedef struct _XbaeCaptionRec *XbaeCaptionWidget;

/*
 * Prototype wrapper
 */
#ifndef P
#if defined(__STDC__) || defined (__cplusplus)
#define P(x)                x
#else
#define P(x)                ()
#define const
#define volatile
#endif
#endif

/*
 * External interfaces to class methods
 */
_XFUNCPROTOBEGIN
extern Widget XbaeCreateCaption P((Widget, String, ArgList, Cardinal));
_XFUNCPROTOEND

/*
 * Type for XmNlabelPosition resource
 */
typedef enum _XbaeLabelPosition {
    XbaePositionLeft,
    XbaePositionRight,
    XbaePositionTop,
    XbaePositionBottom
} XbaeLabelPosition;

/*
 * Type for XmNlabelAlignment resource
 */
typedef enum _XbaeLabelAlignment {
    XbaeAlignmentTopOrLeft,
    XbaeAlignmentCenter,
    XbaeAlignmentBottomOrRight
} XbaeLabelAlignment;

/* provide clean-up for those with R4 */
#ifndef XlibSpecificationRelease
# undef _Xconst
# undef _XFUNCPROTOBEGIN
# undef _XFUNCPROTOEND
#endif

#ifndef XbaeIsXbaeCaption
#define XbaeIsXbaeCaption(w)    XtIsSubclass(w, xbaeCaptionWidgetClass)
#endif /* XbaeIsXbaeInput */

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _Xbae_Caption_h */


