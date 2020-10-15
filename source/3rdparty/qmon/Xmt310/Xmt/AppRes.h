/* 
 * Motif Tools Library, Version 3.1
 * $Id: AppRes.h,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AppRes.h,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#ifndef _XmtAppRes_h
#define _XmtAppRes_h

/*
 * names and classes for each of these resources
 */
#ifndef XmtNbusyCursor
#define XmtNbusyCursor		"busyCursor"
#endif
#ifndef XmtCBusyCursor
#define XmtCBusyCursor		"BusyCursor"
#endif
#ifndef XmtNbusyCursorForeground
#define XmtNbusyCursorForeground	"busyCursorForeground"
#endif
#ifndef XmtNbusyCursorBackground
#define XmtNbusyCursorBackground	"busyCursorBackground"
#endif
#ifndef XmtNhelpCursor
#define XmtNhelpCursor  	"helpCursor"
#endif
#ifndef XmtCHelpCursor
#define XmtCHelpCursor 		"HelpCursor"
#endif
#ifndef XmtNhelpCursorForeground
#define XmtNhelpCursorForeground  	"helpCursorForeground"
#endif
#ifndef XmtNhelpCursorBackground
#define XmtNhelpCursorBackground	"helpCursorBackground"
#endif
#ifndef XmtNcontextHelpPixmap
#define XmtNcontextHelpPixmap	"contextHelpPixmap"
#endif
#ifndef XmtCContextHelpPixmap
#define XmtCContextHelpPixmap	"ContextHelpPixmap"
#endif
#ifndef XmtNhelpFile
#define XmtNhelpFile		"helpFile"
#endif
#ifndef XmtCHelpFile
#define XmtCHelpFile		"HelpFile"
#endif
#ifndef XmtNcontextHelpFile
#define XmtNcontextHelpFile     "contextHelpFile"
#endif
#ifndef XmtCContextHelpFile
#define XmtCContextHelpFile     "ContextHelpFile"
#endif
#ifndef XmtNconfigDir
#define XmtNconfigDir		"configDir"
#endif
#ifndef XmtCConfigDir
#define XmtCConfigDir		"ConfigDir"
#endif
#ifndef XmtNhelpFilePath
#define XmtNhelpFilePath	"helpFilePath"
#endif
#ifndef XmtNresourceFilePath
#define XmtNresourceFilePath	"resourceFilePath"
#endif
#ifndef XmtNbitmapFilePath
#define XmtNbitmapFilePath	"bitmapFilePath"
#endif
#ifndef XmtNpixmapFilePath
#define XmtNpixmapFilePath	"pixmapFilePath"
#endif
#ifndef XmtCHelpFilePath
#define XmtCHelpFilePath	"HelpFilePath"
#endif
#ifndef XmtCResourceFilePath
#define XmtCResourceFilePath	"ResourceFilePath"
#endif
#ifndef XmtCBitmapFilePath
#define XmtCBitmapFilePath	"BitmapFilePath"
#endif
#ifndef XmtCPixmapFilePath
#define XmtCPixmapFilePath	"PixmapFilePath"
#endif
#ifndef XmtNconfigPath
#define XmtNconfigPath	        "configPath"
#endif
#ifndef XmtNuserConfigPath
#define XmtNuserConfigPath	"userConfigPath"
#endif
#ifndef XmtCConfigPath
#define XmtCConfigPath	"ConfigPath"
#endif
#ifndef XmtNcolorTable
#define XmtNcolorTable "colorTable"
#endif
#ifndef XmtCColorTable
#define XmtCColorTable "ColorTable"
#endif
#ifndef XmtNdefaultColorTable
#define XmtNdefaultColorTable "defaultColorTable"
#endif
#ifndef XmtCDefaultColorTable
#define XmtCDefaultColorTable "DefaultColorTable"
#endif
#ifndef XmtNreverseVideo
#define XmtNreverseVideo "reverseVideo"
#endif
#ifndef XmtCReverseVideo
#define XmtCReverseVideo "ReverseVideo"
#endif
#ifndef XmtNcursor
#define XmtNcursor "cursor"
#endif
#ifndef XmtCCursor
#define XmtCCursor "Cursor"
#endif
#ifndef XmtNcursorForeground
#define XmtNcursorForeground "cursorForeground"
#endif
#ifndef XmtCCursorForeground
#define XmtCCursorForeground "CursorForeground"
#endif
#ifndef XmtNcursorBackground
#define XmtNcursorBackground "cursorBackground"
#endif
#ifndef XmtCCursorBackground
#define XmtCCursorBackground "CursorBackground"
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtParseCommandLine(Widget, int *, char **);
extern void XmtInitializeApplicationShell(Widget, ArgList, Cardinal);
extern void XmtSetApplicationValues(Widget, ArgList, Cardinal);
extern void XmtGetApplicationValues(Widget, ArgList, Cardinal);
extern Widget XmtLookupApplicationShell(XrmQuark);
#else
extern void XmtParseCommandLine();
extern void XmtInitializeApplicationShell();
extern void XmtSetApplicationValues();
extern void XmtGetApplicationValues();
extern Widget XmtLookupApplicationShell();
#endif
_XFUNCPROTOEND    

#endif
