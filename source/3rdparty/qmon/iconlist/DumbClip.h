/* $Id: DumbClip.h,v 1.1 2001-07-18 11:06:03 root Exp $ */
/*
 * Copyright 1996 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.    John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose. It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

#ifndef _DumbClip_h
#define _DumbClip_h

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>

extern WidgetClass xmDumbClipWidgetClass;

typedef struct _XmDumbClipRec *XmDumbClipWidget;
typedef struct _XmDumbClipConstraintRec *XmDumbClipConstraint;

#ifndef XmIsDumbClip
#define XmIsDumbClip(a) (XtIsSubclass(a, xmDumbClipWidgetClass))
#endif

typedef struct _XmDumbClipCallbackStruct
{
	int reason;
	XEvent *event;
	Dimension width, height;
} XmDumbClipCallbackStruct;

#ifdef _NO_PROTO
Widget XmCreateDumbClip();
#else
Widget XmCreateDumbClip(Widget _parent, char *_name, ArgList _warg, Cardinal _numWarg);
#endif /* _NO_PROTO */

#endif /* _DumpClip_h */
