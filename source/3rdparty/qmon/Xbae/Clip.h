/*
 * Copyright(c) 1992 Bell Communications Research, Inc. (Bellcore)
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
 * ClipWidget Author: Andrew Wason, Bellcore, aw@bae.bellcore.com
 *
 * $Id: Clip.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * Clip.h - Public definitions for Clip widget
 */

#ifndef _Xbae_Clip_h
#define _Xbae_Clip_h

#include <Xm/Xm.h>
#include "patchlevel.h"


/* Resources:
 * Name                        Class                        RepType                Default Value
 * ----                        -----                        -------                -------------
 * exposeProc                Function                Function        NULL
 * focusCallback        Callback                Callback        NULL
 */

#define XmNexposeProc "exposeProc"


/* Class record constants */

extern WidgetClass xbaeClipWidgetClass;

typedef struct _XbaeClipClassRec *XbaeClipWidgetClass;
typedef struct _XbaeClipRec *XbaeClipWidget;


/*
 * External interfaces to class methods
 */


#if defined (__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern void XbaeClipRedraw(
#if NeedFunctionPrototypes
                           Widget        /* w */
#endif
                           );

#if defined (__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _Xbae_Clip_h */
