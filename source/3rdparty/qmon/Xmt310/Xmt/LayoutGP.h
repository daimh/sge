/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutGP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutGP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLayoutGP_h
#define _XmtLayoutGP_h
    
#include <Xm/XmP.h>
#include <X11/RectObjP.h>
#include <Xmt/LayoutG.h>
#include <Xmt/LayoutP.h>

/*
 * Define an XmtLayoutGadget class as a subclass of Object.
 * Give it class and instance fields identical to RectObj, and
 * also (in a separate structure) a change_font method.
 * We'll be able to cast this to a RectObj or Widget for most uses,
 * but XtIsRectObj will return False, so Motif won't mess with it.
 * We'll still have to fake _XmConfigureObject() out to make it
 * think that it is a rectobj for that one case...
 * Or if _XmConfigureObject() is causing too many redraws, then
 * we can write our own version that doesn't need to be faked out.
 */

/*
 * The following definitions for LayoutGadget are modified from the
 * X11R5 source code mit/lib/Xt/RectObjP.h  See XCOPYRIGHT for copyright info.
 */

/* these fields match CorePart and can not be changed */
typedef struct _XmtLayoutGadgetRectPart {
    Position        x, y;               /* rectangle position               */
    Dimension       width, height;      /* rectangle dimensions             */
    Dimension       border_width;       /* rectangle border width           */
    Boolean         managed;            /* is widget geometry managed?       */
    Boolean         sensitive;          /* is widget sensitive to user events*/
    Boolean         ancestor_sensitive; /* are all ancestors sensitive?      */
}XmtLayoutGadgetRectPart;

typedef struct _XmtLayoutGadgetRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
} XmtLayoutGadgetRec;


/* these fields match CoreClassPart and can not be changed */
typedef struct _XmtLayoutGadgetRectClassPart {
    WidgetClass     superclass;         /* pointer to superclass ClassRec   */
    String          class_name;         /* widget resource class name       */
    Cardinal        widget_size;        /* size in bytes of widget record   */
    XtProc          class_initialize;   /* class initialization proc        */
    XtWidgetClassProc class_part_initialize; /* dynamic initialization      */
    XtEnum          class_inited;       /* has class been initialized?      */
    XtInitProc      initialize;         /* initialize subclass fields       */
    XtArgsProc      initialize_hook;    /* notify that initialize called    */
    XtProc          rect1;		/* NULL                             */
    XtPointer       rect2;              /* NULL                             */
    Cardinal        rect3;              /* NULL                             */
    XtResourceList  resources;          /* resources for subclass fields    */
    Cardinal        num_resources;      /* number of entries in resources   */
    XrmClass        xrm_class;          /* resource class quarkified        */
    Boolean         rect4;              /* NULL                             */
    Boolean         rect5;              /* NULL                             */
    Boolean         rect6;              /* NULL				    */
    Boolean         rect7;              /* NULL                             */
    XtWidgetProc    destroy;            /* free data for subclass pointers  */
    XtWidgetProc    resize;             /* geom manager changed widget size */
    XtExposeProc    expose;             /* rediplay rectangle               */
    XtSetValuesFunc set_values;         /* set subclass resource values     */
    XtArgsFunc      set_values_hook;    /* notify that set_values called    */
    XtAlmostProc    set_values_almost;  /* set values almost for geometry   */
    XtArgsProc      get_values_hook;    /* notify that get_values called    */
    XtProc          rect9;              /* NULL                             */
    XtVersionType   version;            /* version of intrinsics used       */
    XtPointer       callback_private;   /* list of callback offsets         */
    String          rect10;             /* NULL                             */
    XtGeometryHandler query_geometry;   /* return preferred geometry        */
    XtProc          rect11;             /* NULL                             */
    XtPointer       extension;          /* pointer to extension record      */
} XmtLayoutGadgetRectClassPart;

typedef struct _XmtLayoutGadgetClassPart {
    XtWidgetProc    change_font;  /* method called when parent font changes */
} XmtLayoutGadgetClassPart;

typedef struct _XmtLayoutGadgetClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
} XmtLayoutGadgetClassRec;

externalref XmtLayoutGadgetClassRec xmtLayoutGadgetClassRec;

typedef struct {
    XtPointer extension;
} XmtLayoutEmptyClassPart;

typedef struct _XmtLayoutBoxClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
    XmtLayoutEmptyClassPart layout_box_class;
} XmtLayoutBoxClassRec;

externalref XmtLayoutBoxClassRec xmtLayoutBoxClassRec;

typedef struct {
    /* resources */
    unsigned char orientation;
    Pixel background;
    Boolean equal;
    unsigned char space_type;
    Dimension space;
    Dimension space_stretch;
    Dimension item_stretch;
    /* private */
    GC gc;                            /* to draw our background color with */
    int total_stretch, total_shrink;  /* sum over all kids */
    Dimension equal_size;             /* size of elements of an "equal" box */
    Dimension num_items;              /* # of items in the box */
} XmtLayoutBoxPart;

typedef struct _XmtLayoutBoxRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
    XmtLayoutBoxPart layout_box;
} XmtLayoutBoxRec;

typedef struct _XmtLayoutStringClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
    XmtLayoutEmptyClassPart layout_string_class;
} XmtLayoutStringClassRec;

externalref XmtLayoutStringClassRec xmtLayoutStringClassRec;

typedef struct {
    /* resources */
    String label;
    XmString label_string;
    Pixel foreground,  background;
    /* private */
    Dimension width, height;
    GC gc;
    GC background_gc;
} XmtLayoutStringPart;

typedef struct _XmtLayoutStringRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
    XmtLayoutStringPart layout_string;
} XmtLayoutStringRec;

typedef struct _XmtLayoutPixmapClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
    XmtLayoutEmptyClassPart layout_pixmap_class;
} XmtLayoutPixmapClassRec;

externalref XmtLayoutPixmapClassRec xmtLayoutPixmapClassRec;

typedef struct {
    /* resources */
    Pixmap pixmap;
    Pixmap bitmap;
    Pixmap bitmask;
    Pixel foreground, background;
    /* private */
    Dimension width, height, depth;
    GC gc;
} XmtLayoutPixmapPart;

typedef struct _XmtLayoutPixmapRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
    XmtLayoutPixmapPart layout_pixmap;
} XmtLayoutPixmapRec;

typedef struct _XmtLayoutSeparatorClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
    XmtLayoutEmptyClassPart layout_separator_class;
} XmtLayoutSeparatorClassRec;

externalref XmtLayoutSeparatorClassRec xmtLayoutSeparatorClassRec;

typedef struct {
    /* resources */
    unsigned char orientation;
    /* private */
} XmtLayoutSeparatorPart;

typedef struct _XmtLayoutSeparatorRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
    XmtLayoutSeparatorPart layout_separator;
} XmtLayoutSeparatorRec;

typedef struct _XmtLayoutSpaceClassRec {
    XmtLayoutGadgetRectClassPart rect_class;
    XmtLayoutGadgetClassPart layout_gadget_class;
    XmtLayoutEmptyClassPart layout_space_class;
} XmtLayoutSpaceClassRec;

externalref XmtLayoutSpaceClassRec xmtLayoutSpaceClassRec;

typedef struct {
    /* resources */
    /* private */
    Boolean unused;
} XmtLayoutSpacePart;

typedef struct _XmtLayoutSpaceRec {
    ObjectPart object;
    XmtLayoutGadgetRectPart rectangle;
    XmtLayoutSpacePart layout_space;
} XmtLayoutSpaceRec;

#define _XmtRedisplayGadget(w, e, r)\
  if (((XmtLayoutGadgetClass)XtClass(w))->rect_class.expose)\
    ((*((XmtLayoutGadgetClass)XtClass(w))->rect_class.expose)((Widget)w, e, r))

#define _XmtCallChangeFontMethod(w)\
  if (((XmtLayoutGadgetClass)XtClass(w))->layout_gadget_class.change_font)\
    ((*((XmtLayoutGadgetClass)XtClass(w))->layout_gadget_class.change_font)(w))

#endif
