/* 
 * Motif Tools Library, Version 3.1
 * $Id: WorkingBox.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: WorkingBox.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtWorkingBox_h
#define _XmtWorkingBox_h    

#include <Xmt/Layout.h>

externalref WidgetClass xmtWorkingBoxWidgetClass;
typedef struct _XmtWorkingBoxClassRec *XmtWorkingBoxWidgetClass;
typedef struct _XmtWorkingBoxRec *XmtWorkingBoxWidget;

externalref _Xconst char XmtWorkingBoxStrings[];
#ifndef XmtNbuttonLabel
#define XmtNbuttonLabel ((char*)&XmtWorkingBoxStrings[0])
#endif
#ifndef XmtNbuttonWidget
#define XmtNbuttonWidget ((char*)&XmtWorkingBoxStrings[12])
#endif
#ifndef XmtNicon
#define XmtNicon ((char*)&XmtWorkingBoxStrings[25])
#endif
#ifndef XmtNmessage
#define XmtNmessage ((char*)&XmtWorkingBoxStrings[30])
#endif
#ifndef XmtNscaleLabel
#define XmtNscaleLabel ((char*)&XmtWorkingBoxStrings[38])
#endif
#ifndef XmtNscaleMax
#define XmtNscaleMax ((char*)&XmtWorkingBoxStrings[49])
#endif
#ifndef XmtNscaleMin
#define XmtNscaleMin ((char*)&XmtWorkingBoxStrings[58])
#endif
#ifndef XmtNscaleValue
#define XmtNscaleValue ((char*)&XmtWorkingBoxStrings[67])
#endif
#ifndef XmtNscaleWidget
#define XmtNscaleWidget ((char*)&XmtWorkingBoxStrings[78])
#endif
#ifndef XmtNshowButton
#define XmtNshowButton ((char*)&XmtWorkingBoxStrings[90])
#endif
#ifndef XmtNshowScale
#define XmtNshowScale ((char*)&XmtWorkingBoxStrings[101])
#endif
#ifndef XmtCButtonLabel
#define XmtCButtonLabel ((char*)&XmtWorkingBoxStrings[111])
#endif
#ifndef XmtCIcon
#define XmtCIcon ((char*)&XmtWorkingBoxStrings[123])
#endif
#ifndef XmtCMessage
#define XmtCMessage ((char*)&XmtWorkingBoxStrings[128])
#endif
#ifndef XmtCScaleLabel
#define XmtCScaleLabel ((char*)&XmtWorkingBoxStrings[136])
#endif
#ifndef XmtCScaleMax
#define XmtCScaleMax ((char*)&XmtWorkingBoxStrings[147])
#endif
#ifndef XmtCScaleMin
#define XmtCScaleMin ((char*)&XmtWorkingBoxStrings[156])
#endif
#ifndef XmtCScaleValue
#define XmtCScaleValue ((char*)&XmtWorkingBoxStrings[165])
#endif
#ifndef XmtCShowButton
#define XmtCShowButton ((char*)&XmtWorkingBoxStrings[176])
#endif
#ifndef XmtCShowScale
#define XmtCShowScale ((char*)&XmtWorkingBoxStrings[187])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateWorkingBox(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateWorkingDialog(Widget, String, ArgList, Cardinal);
extern void XmtWorkingBoxSetScaleValue(Widget, int);
extern Boolean XmtWorkingBoxHandleEvents(Widget);
#else
extern Widget XmtCreateWorkingBox();
extern Widget XmtCreateWorkingDialog();
extern void XmtWorkingBoxSetScaleValue();
extern Boolean XmtWorkingBoxHandleEvents();
#endif
_XFUNCPROTOEND
    
#endif /* _XmtWorkingBox_h */
