/* 
 * Motif Tools Library, Version 3.1
 * $Id: ChooserP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ChooserP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtChooserP_h
#define _XmtChooserP_h

#include <X11/IntrinsicP.h>
#include <Xm/RowColumnP.h>

#include <Xmt/Chooser.h>
#include <Xmt/Symbols.h>

typedef struct _XmtChooserClassPart {
    XtPointer extension;
} XmtChooserClassPart;

typedef struct _XmtChooserClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    XmRowColumnClassPart row_column_class;
    XmtChooserClassPart chooser_class;
} XmtChooserClassRec;

externalref XmtChooserClassRec xmtChooserClassRec;
    
typedef struct _XmtChooserPart {
    /* resources */
    XmtChooserType type;
    Cardinal num_items;
    String *strings;
    Pixmap *pixmaps;
    Pixmap *select_pixmaps;
    Pixmap *insensitive_pixmaps;
    XtPointer values;
    String *value_strings;
    String value_type;
    Cardinal value_size;
    XmFontList font_list;
#if XmVersion >= 2000
    XmRenderTable render_table;
#endif
    Cardinal visible_items;
    int state;
    String symbol_name;
    unsigned char label_type;
    XtCallbackList callback;
    Widget *item_widgets;
    /* internal state */
    Widget list, pane, option;
#if XmVersion >= 2000
    Widget combo_box;
#endif
    XmtSymbol symbol;
    Boolean ignore_symbol_notify;
    int insensitive;
} XmtChooserPart;

typedef struct _XmtChooserRec {
    CorePart core;
    CompositePart composite;
    ConstraintPart constraint;
    XmManagerPart manager;
    XmRowColumnPart row_column;
    XmtChooserPart chooser;
} XmtChooserRec;

#endif /* _XmtChooserP_h */
