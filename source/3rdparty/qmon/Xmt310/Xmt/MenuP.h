/* 
 * Motif Tools Library, Version 3.1
 * $Id: MenuP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MenuP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtMenuP_h
#define _XmtMenuP_h    

#include <Xm/XmP.h>
#include <Xm/RowColumnP.h>
#include <Xmt/Menu.h>

typedef struct _XmtMenuClassPart {
    XtPointer extension;
} XmtMenuClassPart;

typedef struct _XmtMenuClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmRowColumnClassPart	row_column_class;
    XmtMenuClassPart            menu_class;
} XmtMenuClassRec;

externalref XmtMenuClassRec xmtMenuClassRec;

typedef struct _XmtMenuPart {
    /* resources */
    XmtMenuItem *items;
    int num_items;
    String accelerator_font_tag;
} XmtMenuPart;

typedef struct _XmtMenuRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmRowColumnPart	row_column;
    XmtMenuPart         menu;
} XmtMenuRec;

#endif /* _XmtMenuP_h */
