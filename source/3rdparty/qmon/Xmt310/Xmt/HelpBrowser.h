/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBrowser.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBrowser.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHelpBrowser_h
#define _XmtHelpBrowser_h

#include <Xmt/Layout.h>

externalref WidgetClass xmtHelpBrowserWidgetClass;
typedef struct _XmtHelpBrowserClassRec *XmtHelpBrowserWidgetClass;
typedef struct _XmtHelpBrowserRec *XmtHelpBrowserWidget;

externalref _Xconst char XmtHelpBrowserStrings[];
#ifndef XmtNcrossrefLabel
#define XmtNcrossrefLabel ((char*)&XmtHelpBrowserStrings[0])
#endif
#ifndef XmtNcurrentNode
#define XmtNcurrentNode ((char*)&XmtHelpBrowserStrings[14])
#endif
#ifndef XmtNdoneLabel
#define XmtNdoneLabel ((char*)&XmtHelpBrowserStrings[26])
#endif
#ifndef XmtNindexFontList
#define XmtNindexFontList ((char*)&XmtHelpBrowserStrings[36])
#endif
#ifndef XmtNindexLabel
#define XmtNindexLabel ((char*)&XmtHelpBrowserStrings[50])
#endif
#ifndef XmtNnextLabel
#define XmtNnextLabel ((char*)&XmtHelpBrowserStrings[61])
#endif
#ifndef XmtNprevLabel
#define XmtNprevLabel ((char*)&XmtHelpBrowserStrings[71])
#endif
#ifndef XmtNsectionFontList
#define XmtNsectionFontList ((char*)&XmtHelpBrowserStrings[81])
#endif
#ifndef XmtNtextBackground
#define XmtNtextBackground ((char*)&XmtHelpBrowserStrings[97])
#endif
#ifndef XmtNtextColumns
#define XmtNtextColumns ((char*)&XmtHelpBrowserStrings[112])
#endif
#ifndef XmtNtextFontList
#define XmtNtextFontList ((char*)&XmtHelpBrowserStrings[124])
#endif
#ifndef XmtNtextForeground
#define XmtNtextForeground ((char*)&XmtHelpBrowserStrings[137])
#endif
#ifndef XmtNtextRows
#define XmtNtextRows ((char*)&XmtHelpBrowserStrings[152])
#endif
#ifndef XmtNtitleLabel
#define XmtNtitleLabel ((char*)&XmtHelpBrowserStrings[161])
#endif
#ifndef XmtNtitlePixmap
#define XmtNtitlePixmap ((char*)&XmtHelpBrowserStrings[172])
#endif
#ifndef XmtNtocFontList
#define XmtNtocFontList ((char*)&XmtHelpBrowserStrings[184])
#endif
#ifndef XmtNtocLabel
#define XmtNtocLabel ((char*)&XmtHelpBrowserStrings[196])
#endif
#ifndef XmtNtocRows
#define XmtNtocRows ((char*)&XmtHelpBrowserStrings[205])
#endif
#ifndef XmtNtopNode
#define XmtNtopNode ((char*)&XmtHelpBrowserStrings[213])
#endif
#ifndef XmtCCrossrefLabel
#define XmtCCrossrefLabel ((char*)&XmtHelpBrowserStrings[221])
#endif
#ifndef XmtCCurrentNode
#define XmtCCurrentNode ((char*)&XmtHelpBrowserStrings[235])
#endif
#ifndef XmtCDoneLabel
#define XmtCDoneLabel ((char*)&XmtHelpBrowserStrings[247])
#endif
#ifndef XmtCIndexLabel
#define XmtCIndexLabel ((char*)&XmtHelpBrowserStrings[257])
#endif
#ifndef XmtCNextLabel
#define XmtCNextLabel ((char*)&XmtHelpBrowserStrings[268])
#endif
#ifndef XmtCPrevLabel
#define XmtCPrevLabel ((char*)&XmtHelpBrowserStrings[278])
#endif
#ifndef XmtCTitleLabel
#define XmtCTitleLabel ((char*)&XmtHelpBrowserStrings[288])
#endif
#ifndef XmtCTitlePixmap
#define XmtCTitlePixmap ((char*)&XmtHelpBrowserStrings[299])
#endif
#ifndef XmtCTocLabel
#define XmtCTocLabel ((char*)&XmtHelpBrowserStrings[311])
#endif
#ifndef XmtCTocRows
#define XmtCTocRows ((char*)&XmtHelpBrowserStrings[320])
#endif
#ifndef XmtCTopNode
#define XmtCTopNode ((char*)&XmtHelpBrowserStrings[328])
#endif

#define XmtHELP_BROWSER_PIXMAP_NAME "_xmt_help_browser"

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateHelpBrowser(Widget, String, ArgList, Cardinal);
#else
extern Widget XmtCreateHelpBrowser();
#endif
_XFUNCPROTOEND


#endif
