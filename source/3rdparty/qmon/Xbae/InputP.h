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
 * $Id: InputP.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

#ifndef _Xbae_InputP_h
#define _Xbae_InputP_h

#include "Input.h"
#if XmVersion <= 1001
#include <Xm/XmP.h>
#else
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/TextP.h>

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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XbaeInputClassPart
{
    void *extension;
}
XbaeInputClassPart;

typedef struct _XbaeInputClassRec
{
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmTextClassPart text_class;
    XbaeInputClassPart input_class;
}
XbaeInputClassRec;

externalref XbaeInputClassRec xbaeInputClassRec;

typedef struct _XbaeInputPart
{
    /* Public resources */
    unsigned char alignment;
    Boolean auto_fill;
    Boolean overwrite_mode;
    String pattern;
    XtCallbackList validate_callback;
    Boolean convert_case;
    /* Internal resources */
    int pattern_length;
    XmTextPosition last_insert;
    char *literal_pending;
    int literal_count;
}
XbaeInputPart;

typedef struct _XbaeInputRec
{
    CorePart core;
    XmPrimitivePart primitive;
    XmTextPart text;
    XbaeInputPart input;
}
XbaeInputRec;
    
#ifdef __cplusplus
}
#endif

#endif




