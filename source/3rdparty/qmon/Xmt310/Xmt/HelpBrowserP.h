/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBrowserP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBrowserP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtHelpBrowserP_h
#define _XmtHelpBrowserP_h

#include <Xmt/HelpBrowser.h>
#include <Xmt/Help.h>
#include <Xmt/LayoutP.h>

#define UPBUTTONS 6
#define XREFBUTTONS 6

typedef struct _XmtHelpTree {
    XmtHelpNode *help;
    struct _XmtHelpTree **children;
    short num_children;
    short *crossrefs;  /* index of crossrefs in the toc */
    short num_crossrefs;
    struct _XmtHelpTree *parent;
    XmString title;
} XmtHelpTree;

typedef struct _XmtHelpBrowserClassPart {
    XtPointer extension;
} XmtHelpBrowserClassPart;

typedef struct _XmtHelpBrowserClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart	        constraint_class;
    XmManagerClassPart	        manager_class;
    XmBulletinBoardClassPart    bulletin_class;
    XmtLayoutClassPart          layout_class;
    XmtHelpBrowserClassPart     help_browser_class;
} XmtHelpBrowserClassRec;

externalref XmtHelpBrowserClassRec xmtHelpBrowserClassRec;

typedef struct _XmtHelpBrowserPart {
    /* resources */
    String title_label;
    String toc_label;
    String index_label;
    String crossref_label;
    String next_label;
    String prev_label;
    String done_label;

    String top_node;
    String current_node;
    
    Pixmap title_pixmap;

    XmFontList text_font_list;    /* for the help text itself */
    XmFontList section_font_list; /* for section titles */
    XmFontList toc_font_list;     /* for titles in the toc & xref widgets */
    XmFontList index_font_list;   /* for keywords in the index widget */

    Pixel text_foreground;
    Pixel text_background;

    Dimension text_rows;
    Dimension text_columns;
    Dimension toc_rows;

    /* child widgets */
    Widget icon_g, title_g;
    Widget toc_w, index_w;
    Widget text_sw, label_sw;
    Widget text_w, label_w;
    Widget next_w, prev_w, done_w;
    Widget text_title_w;    /* an option menu */
    Widget text_title_pane; /* a menu pane containing these items */
    Widget title_up_items[UPBUTTONS];
    Widget see_also_label;
    Widget title_xref_items[XREFBUTTONS];
    Widget toc_title_g, index_title_g, xref_title_g;

    /* internal state */
    XmtHelpTree *tree;
    XmtHelpTree **toc;
    short toc_len;
    XmtHelpTree **index;
    short index_len;
    short current_node_number;
    unsigned char current_format;
} XmtHelpBrowserPart;

typedef struct _XmtHelpBrowserRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart	bulletin_board;
    XmtLayoutPart       layout;
    XmtHelpBrowserPart  help_browser;
} XmtHelpBrowserRec;

#endif
