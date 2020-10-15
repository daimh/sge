/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBox.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBox.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHelpBox_h
#define _XmtHelpBox_h    

#include <Xmt/Layout.h>

externalref WidgetClass xmtHelpBoxWidgetClass;
typedef struct _XmtHelpBoxClassRec *XmtHelpBoxWidgetClass;
typedef struct _XmtHelpBoxRec *XmtHelpBoxWidget;

externalref _Xconst char XmtHelpBoxStrings[];
#ifndef XmtNhelpBackground
#define XmtNhelpBackground ((char*)&XmtHelpBoxStrings[0])
#endif
#ifndef XmtNhelpFontList
#define XmtNhelpFontList ((char*)&XmtHelpBoxStrings[15])
#endif
#ifndef XmtNhelpForeground
#define XmtNhelpForeground ((char*)&XmtHelpBoxStrings[28])
#endif
#ifndef XmtNhelpNode
#define XmtNhelpNode ((char*)&XmtHelpBoxStrings[43])
#endif
#ifndef XmtNhelpPixmap
#define XmtNhelpPixmap ((char*)&XmtHelpBoxStrings[52])
#endif
#ifndef XmtNhelpText
#define XmtNhelpText ((char*)&XmtHelpBoxStrings[63])
#endif
#ifndef XmtNhelpTitle
#define XmtNhelpTitle ((char*)&XmtHelpBoxStrings[72])
#endif
#ifndef XmtNtitleFontList
#define XmtNtitleFontList ((char*)&XmtHelpBoxStrings[82])
#endif
#ifndef XmtNtitleForeground
#define XmtNtitleForeground ((char*)&XmtHelpBoxStrings[96])
#endif
#ifndef XmtNvisibleLines
#define XmtNvisibleLines ((char*)&XmtHelpBoxStrings[112])
#endif
#ifndef XmtCHelpNode
#define XmtCHelpNode ((char*)&XmtHelpBoxStrings[125])
#endif
#ifndef XmtCHelpPixmap
#define XmtCHelpPixmap ((char*)&XmtHelpBoxStrings[134])
#endif
#ifndef XmtCHelpText
#define XmtCHelpText ((char*)&XmtHelpBoxStrings[145])
#endif
#ifndef XmtCHelpTitle
#define XmtCHelpTitle ((char*)&XmtHelpBoxStrings[154])
#endif
#ifndef XmtCVisibleLines
#define XmtCVisibleLines ((char*)&XmtHelpBoxStrings[164])
#endif

#define XmtHELP_BOX_PIXMAP_NAME  "_xmt_help"

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateHelpBox(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateHelpDialog(Widget, String, ArgList, Cardinal);
#else
extern Widget XmtCreateHelpBox();
extern Widget XmtCreateHelpDialog();
#endif
_XFUNCPROTOEND
    
#endif /* _XmtHelpBox_h */
