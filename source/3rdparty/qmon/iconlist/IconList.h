/* $Id: IconList.h,v 1.1 2001-07-18 11:06:03 root Exp $ */
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

#ifndef _IconList_h
#define _IconList_h

#include <Xm/Xm.h>

extern WidgetClass xmIconListWidgetClass;

typedef struct _XmIconListRec *XmIconListWidget;

#ifndef XmIsIconList
#define XmIsIconList(a) (XtIsSubclass((Widget)a, xmIconListWidgetClass))
#endif /* XmIsIconList */

#define XmNiconWidth "iconWidth"
#define XmNiconHeight "iconHeight"
#define XmCIconDimension "IconDimension"

#define XmNusingBitmaps "usingBitmaps"
#define XmCUsingBitmaps "UsingBitmaps"

#define XmNclipWidget "clipWidget"
#define XmCClipWidget "ClipWidget"

#define XmNremoveCallback "removeCallback"

typedef enum _IconListReasonType {
         XmCR_REMOVE = 200
}  IconListReasonType;

typedef struct _IconListPixmap
{
	Pixmap pixmap;
	Pixmap mask;
	Boolean isBitmap;
	int width, height;
} IconListPixmap;

typedef struct _IconListElement
{
	String *string;
   int  numStrings;
	IconListPixmap iconPixmap;
} IconListElement;

typedef struct _XmIconListCallbackStruct
{
	int reason;
	XEvent *event;
	int row;
	IconListElement *element;
} XmIconListCallbackStruct;

#if NeedFunctionPrototypes
extern Widget XmCreateScrolledIconList(Widget _parent, char *_name, ArgList _args, Cardinal _numArgs);
extern void XmIconListSetItems(Widget _iconList, IconListElement *_elements, int _count);
extern void XmIconListGetItems(Widget _iconList, IconListElement **_elements, int *_count);
extern Widget XmIconListGetScrolledWindow(Widget _w);
extern void XmIconListScrollToRow(Widget _w, int _row, Boolean _toTop, Boolean _makeFocus, Boolean _callCallbacks);
#else
extern Widget XmCreateScrolledIconList();
extern void XmIconListSetItems();
extern void XmIconListGetItems();
extern Widget XmIconListGetScrolledWindow();
extern void XmIconListScrollToRow();
#endif

#endif /* _IconList_h */
