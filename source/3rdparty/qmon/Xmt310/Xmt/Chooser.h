/* 
 * Motif Tools Library, Version 3.1
 * $Id: Chooser.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Chooser.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtChooser_h
#define _XmtChooser_h

#include <Xm/RowColumn.h>

typedef enum {
    XmtChooserRadioBox,     /* one-of-many toggles */
    XmtChooserCheckBox,     /* n-of-many toggles */
    XmtChooserRadioPalette, /* one-of-many toggle buttons w/o indicators */
    XmtChooserCheckPalette, /* n-of-many toggle buttons w/o indicators */
    XmtChooserRadioList,    /* XmList widget in single select mode */
    XmtChooserCheckList,    /* XmList widget in multi-select mode */
    XmtChooserOption,       /* option menu */
    XmtChooserButtonBox     /* Push buttons in a box */
#if XmVersion >= 2000
    , XmtChooserComboBox    /* drop-down single-select list */
#endif	
} XmtChooserType;

typedef struct {
    int state;        /* selected item or OR of items */
    Cardinal item;         /* whichever item just was clicked */
    XtPointer valuep; /* address of value from XmtNvalues array */
} XmtChooserCallbackStruct;

externalref WidgetClass xmtChooserWidgetClass;
typedef struct _XmtChooserClassRec *XmtChooserWidgetClass;
typedef struct _XmtChooserRec *XmtChooserWidget;


externalref _Xconst char XmtChooserStrings[];
#ifndef XmtNchooserType
#define XmtNchooserType ((char*)&XmtChooserStrings[0])
#endif
#ifndef XmtNfontList
#define XmtNfontList ((char*)&XmtChooserStrings[12])
#endif
#ifndef XmtNinsensitivePixmaps
#define XmtNinsensitivePixmaps ((char*)&XmtChooserStrings[21])
#endif
#ifndef XmtNitemWidgets
#define XmtNitemWidgets ((char*)&XmtChooserStrings[40])
#endif
#ifndef XmtNlabelType
#define XmtNlabelType ((char*)&XmtChooserStrings[52])
#endif
#ifndef XmtNnumItems
#define XmtNnumItems ((char*)&XmtChooserStrings[62])
#endif
#ifndef XmtNpixmaps
#define XmtNpixmaps ((char*)&XmtChooserStrings[71])
#endif
#ifndef XmtNselectPixmaps
#define XmtNselectPixmaps ((char*)&XmtChooserStrings[79])
#endif
#ifndef XmtNstate
#define XmtNstate ((char*)&XmtChooserStrings[93])
#endif
#ifndef XmtNstrings
#define XmtNstrings ((char*)&XmtChooserStrings[99])
#endif
#ifndef XmtNsymbolName
#define XmtNsymbolName ((char*)&XmtChooserStrings[107])
#endif
#ifndef XmtNvalueChangedCallback
#define XmtNvalueChangedCallback ((char*)&XmtChooserStrings[118])
#endif
#ifndef XmtNvalueSize
#define XmtNvalueSize ((char*)&XmtChooserStrings[139])
#endif
#ifndef XmtNvalueStrings
#define XmtNvalueStrings ((char*)&XmtChooserStrings[149])
#endif
#ifndef XmtNvalueType
#define XmtNvalueType ((char*)&XmtChooserStrings[162])
#endif
#ifndef XmtNvalues
#define XmtNvalues ((char*)&XmtChooserStrings[172])
#endif
#ifndef XmtNvisibleItems
#define XmtNvisibleItems ((char*)&XmtChooserStrings[179])
#endif
#ifndef XmtCChooserType
#define XmtCChooserType ((char*)&XmtChooserStrings[192])
#endif
#ifndef XmtCInsensitivePixmaps
#define XmtCInsensitivePixmaps ((char*)&XmtChooserStrings[204])
#endif
#ifndef XmtCLabelType
#define XmtCLabelType ((char*)&XmtChooserStrings[223])
#endif
#ifndef XmtCNumItems
#define XmtCNumItems ((char*)&XmtChooserStrings[233])
#endif
#ifndef XmtCPixmaps
#define XmtCPixmaps ((char*)&XmtChooserStrings[242])
#endif
#ifndef XmtCSelectPixmaps
#define XmtCSelectPixmaps ((char*)&XmtChooserStrings[250])
#endif
#ifndef XmtCState
#define XmtCState ((char*)&XmtChooserStrings[264])
#endif
#ifndef XmtCStrings
#define XmtCStrings ((char*)&XmtChooserStrings[270])
#endif
#ifndef XmtCSymbolName
#define XmtCSymbolName ((char*)&XmtChooserStrings[278])
#endif
#ifndef XmtCValueSize
#define XmtCValueSize ((char*)&XmtChooserStrings[289])
#endif
#ifndef XmtCValueStrings
#define XmtCValueStrings ((char*)&XmtChooserStrings[299])
#endif
#ifndef XmtCValueType
#define XmtCValueType ((char*)&XmtChooserStrings[312])
#endif
#ifndef XmtCValues
#define XmtCValues ((char*)&XmtChooserStrings[322])
#endif
#ifndef XmtCVisibleItems
#define XmtCVisibleItems ((char*)&XmtChooserStrings[329])
#endif
#ifndef XmtRXmtChooserType
#define XmtRXmtChooserType ((char*)&XmtChooserStrings[342])
#endif

#if XmVersion >= 2000
#ifndef XmtNrenderTable
#define XmtNrenderTable "renderTable"
#endif
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateChooser(Widget, StringConst, ArgList, Cardinal);
extern void XmtRegisterChooser(void);
extern int XmtChooserGetState(Widget);
extern void XmtChooserSetState(Widget, int, XmtWideBoolean);
extern XtPointer XmtChooserGetValue(Widget);
extern int XmtChooserLookupItemByName(Widget, StringConst);
extern int XmtChooserLookupItemByValue(Widget, XtPointer);
extern String XmtChooserLookupItemName(Widget, int);
extern XtPointer XmtChooserLookupItemValue(Widget, int);
extern void XmtChooserSetItemValue(Widget, int, XtPointer);
extern void XmtChooserSetSensitive(Widget, int, XmtWideBoolean);
extern Boolean XmtChooserGetSensitivity(Widget, int);
#else
extern Widget XmtCreateChooser();
extern void XmtRegisterChooser();
extern int XmtChooserGetState();
extern void XmtChooserSetState();
extern XtPointer XmtChooserGetValue();
extern int XmtChooserLookupItemByName();
extern int XmtChooserLookupItemByValue();
extern String XmtChooserLookupItemName();
extern XtPointer XmtChooserLookupItemValue();
extern void XmtChooserSetItemValue();
extern void XmtChooserSetSensitive();
extern Boolean XmtChooserGetSensitivity();
#endif
_XFUNCPROTOEND
    
#endif /* _XmtChooser_h */
