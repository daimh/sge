/* 
 * Motif Tools Library, Version 3.1
 * $Id: WorkingBoxP.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: WorkingBoxP.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtWorkingBoxP_h
#define _XmtWorkingBoxP_h    

#include <Xmt/LayoutP.h>
#include <Xmt/WorkingBox.h>

typedef struct _XmtWorkingBoxClassPart {
    XtPointer extension;
} XmtWorkingBoxClassPart;

typedef struct _XmtWorkingBoxClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart	        constraint_class;
    XmManagerClassPart	        manager_class;
    XmBulletinBoardClassPart    bulletin_class;
    XmtLayoutClassPart          layout_class;
    XmtWorkingBoxClassPart      working_box_class;
} XmtWorkingBoxClassRec;

externalref XmtWorkingBoxClassRec xmtWorkingBoxClassRec;

typedef struct _XmtWorkingBoxPart {
    /* resources */
    String message;
    String scale_label;
    String button_label;
    Pixmap icon;
    Boolean show_scale;
    Boolean show_button;
    int scale_value;
    int scale_min, scale_max;
    /* widgets */
    Widget pixmap;
    Widget scale;
    Widget separator;
    Widget button;
    /* private state */
    Boolean button_pressed;
    Boolean free_icon;
} XmtWorkingBoxPart;

typedef struct _XmtWorkingBoxRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart      constraint;
    XmManagerPart       manager;
    XmBulletinBoardPart	bulletin_board;
    XmtLayoutPart       layout;
    XmtWorkingBoxPart   working_box;
} XmtWorkingBoxRec;

#endif /* _XmtWorkingBoxP_h */
