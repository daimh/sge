/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBoxP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBoxP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHelpBoxP_h
#define _XmtHelpBoxP_h    

#include <Xmt/LayoutP.h>
#include <Xmt/HelpBox.h>

typedef struct _XmtHelpBoxClassPart {
    XtPointer extension;
} XmtHelpBoxClassPart;

typedef struct _XmtHelpBoxClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart	        constraint_class;
    XmManagerClassPart	        manager_class;
    XmBulletinBoardClassPart    bulletin_class;
    XmtLayoutClassPart          layout_class;
    XmtHelpBoxClassPart         help_box_class;
} XmtHelpBoxClassRec;

externalref XmtHelpBoxClassRec xmtHelpBoxClassRec;

typedef struct _XmtHelpBoxPart {
    /* resources */
    String help_text;
    String help_title;
    String help_node;
    Pixmap help_pixmap;
    short visible_lines;
    XmFontList help_font_list;
    Pixel title_foreground;
    Pixel help_foreground;
    Pixel help_background;
    /* child widgets */
    Widget pixmap_gadget;
    Widget title_gadget;
    Widget scrolled_widget;
    Widget label_widget;
    Widget button_widget;
} XmtHelpBoxPart;

typedef struct _XmtHelpBoxRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart	bulletin_board;
    XmtLayoutPart       layout;
    XmtHelpBoxPart      help_box;
} XmtHelpBoxRec;

#endif /* _XmtHelpBoxP_h */
