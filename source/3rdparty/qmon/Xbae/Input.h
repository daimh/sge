/*
 * Copyright(c) 1999 Andrew Lister
 *
 *                        All rights reserved
 * Permission to use, copy, modify and distribute this material for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of the author not be used in advertising
 * or publicity pertaining to this material without the specific,
 * prior written permission of the author.
 *
 * THE AUTHOR MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES, EX-
 * PRESS OR IMPLIED, WITH RESPECT TO THE SOFTWARE, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR ANY PARTICULAR PURPOSE, AND THE WARRANTY AGAINST IN-
 * FRINGEMENT OF PATENTS OR OTHER INTELLECTUAL PROPERTY RIGHTS.  THE
 * SOFTWARE IS PROVIDED "AS IS", AND IN NO EVENT SHALL THE AUTHOR OR
 * ANY OF ITS AFFILIATES BE LIABLE FOR ANY DAMAGES, INCLUDING ANY
 * LOST PROFITS OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES RELAT-
 * ING TO THE SOFTWARE.
 *
 * Author: Andrew Lister
 *
 * $Id: Input.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifndef _Xbae_Input_h
#define _Xbae_Input_h

#include <Xm/Xm.h>
#include <Xm/Text.h>

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

typedef struct _XbaeInputClassRec *XbaeInputWidgetClass;
typedef struct _XbaeInputRec *XbaeInputWidget;

externalref WidgetClass xbaeInputWidgetClass;

#ifndef XmNautoFill
#define XmNautoFill "autoFill"
#endif
#ifndef XmNoverwriteMode
#define XmNoverwriteMode "overwriteMode"
#endif
#ifndef XmNpattern
#define XmNpattern "pattern"
#endif
#ifndef XmNvalidateCallback
#define XmNvalidateCallback "validateCallback"
#endif
#ifndef XmNoutputFormat
#define XmNoutputFormat "outputFormat"
#endif    
#ifndef XmNconvertCase
#define XmNconvertCase "convertCase"
#endif    

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

_XFUNCPROTOBEGIN
extern Widget XbaeCreateInput P((Widget, String, ArgList, Cardinal));
_XFUNCPROTOEND

typedef struct _XbaeInputValidateCallbackStruct
{
    int reason;
    XEvent *event;
    String pattern;
    String value;
    Boolean doit;
}
XbaeInputValidateCallbackStruct;

#ifndef XbaeIsXbaeInput
#define XbaeIsXbaeInput(w)    XtIsSubclass(w, xbaeInputWidgetClass)
#endif /* XbaeIsXbaeInput */
    
#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif


