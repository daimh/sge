/* $Id: IconListP.h,v 1.1 2001-07-18 11:06:03 root Exp $ */
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

#ifndef _IconListP_h
#define _IconListP_h

#include <Xm/XmP.h>

#if XmVersion > 1001
#include <Xm/PrimitiveP.h>
#endif

#include "IconList.h"

typedef struct _XmIconListClassPart
{
	int blammo; /* make's compiler happy */
} XmIconListClassPart;

typedef struct _XmIconListClassRec
{
	CoreClassPart core_class;
	XmPrimitiveClassPart primitive_class;
	XmIconListClassPart iconList_class;
} XmIconListClassRec;

extern XmIconListClassRec xmIconListClassRec;

typedef struct _XmIconListPart
{
	Boolean highlightOnEnter;
	Boolean usingBitmaps;
	Boolean hasFocus;
	Boolean callCallbacks;

	int startRow;
	int endRow;
	int numberOfElements;
	int visibleItemCount;
	int focusElement;

	short clickCount;

	IconListElement *elements;

	XtPointer userData;

	XmFontList fontList;

	Pixel foreground;
	Pixel selectColor;

	Dimension marginWidth;
	Dimension marginHeight;
	Dimension iconWidth;
	Dimension iconHeight;

	Dimension textHeight;
	Dimension fullElementHeight;
	Dimension textOffset;

	Time lastClickTime;
	int lastClickRow;

	XtCallbackList activateCallback;

	XFontStruct *font;

	Dimension highlightThickness;

	XtCallbackList valueChangedCallback;
	XtCallbackList focusChangeCallback;
	XtCallbackList removeCallback;

	Widget clipWidget;
	Widget verticalScrollBarWidget;
	Widget horizontalScrollBarWidget;
	Widget scrolledWindowWidget;

	GC gc;
	GC copyGC;
	GC eraseGC;
	GC selectGC;
} XmIconListPart;

typedef struct _XmIconListRec
{
	CorePart core;
	XmPrimitivePart primitive;
	XmIconListPart iconList;
} XmIconListRec, *XmIconListPtr;

#endif /* _IconListP_h */
