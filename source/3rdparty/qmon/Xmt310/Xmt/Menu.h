/* 
 * Motif Tools Library, Version 3.1
 * $Id: Menu.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Menu.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtMenu_h
#define _XmtMenu_h    

#include <Xm/RowColumn.h>
#include <Xmt/Symbols.h>

typedef enum {
    XmtMenuItemEnd,          /* uses to NULL-terminate the array of items */
    XmtMenuItemPushButton,
    XmtMenuItemToggleButton,
    XmtMenuItemCascadeButton,
    XmtMenuItemSeparator,
    XmtMenuItemDoubleSeparator,
    XmtMenuItemLabel
} XmtMenuItemType;

/* flags for menu items */
#define XmtMenuItemOn      0x10  /* initial state of toggle button is on */
#define XmtMenuItemHelp    0x20  /* cascade button goes to far right of bar */
#define XmtMenuItemTearoff 0x40  /* this button tears off the menu */
#define XmtMenuItemPixmap  0x80  /* label is a pixmap name, not a string */
#define XmtMenuItemCallbackList 0x100 /* callback is a list, not a func */

typedef struct _XmtMenuItem {
    /* programmer initializes some or all of these */
    unsigned type;
    String label;
    char mnemonic;
    String accelerator;
    String accelerator_label;
    XtCallbackProc callback;
    XtPointer client_data;
    struct _XmtMenuItem *submenu;
    String symbol_name;
    String alt_label;
    char alt_mnemonic;
    String name;

    /* private state */
    Widget w;     /* the widget or gadget for this menu item */
    Widget submenu_pane;  /* the XmtMenu widget created with cascade buttons */
    String submenu_name;
    short sensitive;
    XmtSymbol symbol;
    XmString label0, label1;
} XmtMenuItem; 

externalref WidgetClass xmtMenuWidgetClass;
typedef struct _XmtMenuClassRec *XmtMenuWidgetClass;
typedef struct _XmtMenuRec *XmtMenuWidget;

externalref _Xconst char XmtMenuStrings[];
#ifndef XmtNacceleratorFontTag
#define XmtNacceleratorFontTag ((char*)&XmtMenuStrings[0])
#endif
#ifndef XmtNitems
#define XmtNitems ((char*)&XmtMenuStrings[19])
#endif
#ifndef XmtNnumItems
#define XmtNnumItems ((char*)&XmtMenuStrings[25])
#endif
#ifndef XmtCAcceleratorFontTag
#define XmtCAcceleratorFontTag ((char*)&XmtMenuStrings[34])
#endif
#ifndef XmtCItems
#define XmtCItems ((char*)&XmtMenuStrings[53])
#endif
#ifndef XmtCNumItems
#define XmtCNumItems ((char*)&XmtMenuStrings[59])
#endif
#ifndef XmtRXmtMenuItemList
#define XmtRXmtMenuItemList ((char *)&XmtMenuStrings[68])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateMenubar(Widget, String, ArgList, Cardinal);
extern Widget XmtCreatePopupMenu(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateOptionMenu(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateMenuPane(Widget, String, ArgList, Cardinal);
extern XmtMenuItem *XmtMenuGetMenuItem(Widget, StringConst);
extern Widget XmtMenuItemGetSubmenu(XmtMenuItem *);
extern Widget XmtMenuItemGetWidget(XmtMenuItem *);
extern Boolean XmtMenuItemGetState(XmtMenuItem *);
extern void XmtMenuItemSetState(XmtMenuItem *, XmtWideBoolean, XmtWideBoolean);
extern void XmtMenuItemSetSensitivity(XmtMenuItem *, XmtWideBoolean);
extern void XmtMenuInactivateProcedure(Widget, XtCallbackProc);
extern void XmtMenuActivateProcedure(Widget, XtCallbackProc);
extern void XmtMenuPopupHandler(Widget, XtPointer, XEvent *, Boolean *);
#else
extern Widget XmtCreateMenubar();
extern Widget XmtCreatePopupMenu();
extern Widget XmtCreateOptionMenu();
extern Widget XmtCreateMenuPane();
extern XmtMenuItem *XmtMenuGetMenuItem();
extern Widget XmtMenuItemGetSubmenu();
extern Widget XmtMenuItemGetWidget();
extern Boolean XmtMenuItemGetState();
extern void XmtMenuItemSetState();
extern void XmtMenuItemSetSensitivity();
extern void XmtMenuInactivateProcedure();
extern void XmtMenuActivateProcedure();
extern void XmtMenuPopupHandler();
#endif
_XFUNCPROTOEND

#endif /* _XmtMenu_h */
