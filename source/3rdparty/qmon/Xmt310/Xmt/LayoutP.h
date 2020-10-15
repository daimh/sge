/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLayoutP_h
#define _XmtLayoutP_h
    
#include <Xm/BulletinBP.h>
#include <Xmt/Layout.h>
#include <Xmt/Lexer.h>

typedef enum {
    XmtLayoutRow,  
    XmtLayoutCol,  
    XmtLayoutChild,
    XmtLayoutSpace,
    XmtLayoutString,
    XmtLayoutPixmap,
    XmtLayoutVSep,
    XmtLayoutHSep
} XmtLayoutType;

typedef void (*XmtLayoutParserProc)(
#if NeedFunctionPrototypes
    XmtLayoutWidget, String
#endif
);

typedef XtPointer (*XmtLayoutLookupTypeProc)(
#if NeedFunctionPrototypes
    StringConst
#endif	
);

typedef Widget (*XmtLayoutCreateProc)(
#if NeedFunctionPrototypes				      
    StringConst, XtPointer, Widget, ArgList, Cardinal
#endif
);

/*
 * The Layout class part structure contains three procedure pointers.
 * These are optional methods; the fields should be statically
 * initialzed to NULL, and can be dynamically set with special
 * registration procedures.  The reason that they are not statically
 * initialzed is to avoid linking in code that is not always used.
 */
typedef struct {
    XmtLayoutParserProc parser; 
    XmtLayoutLookupTypeProc lookup_type_proc;
    XmtLayoutCreateProc create_proc;
    XtPointer extension;
} XmtLayoutClassPart;

typedef struct _XmtLayoutClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    XmBulletinBoardClassPart bulletin_class;
    XmtLayoutClassPart layout_class;
} XmtLayoutClassRec;

externalref XmtLayoutClassRec xmtLayoutClassRec;

typedef struct {
    /* resources */
    String layout;
    XFontStruct *font;
    XmFontList font_list;
#if XmVersion >= 2000
    XmRenderTable render_table;
#endif
    Dimension default_spacing;
    unsigned char orientation;
    Boolean debug_layout;
    /* private state */
    GC gc, grey_gc;
    double pixpermm;  /* pixels per mm; for resolution independence */
    double pixperem;  /* pixels per em; for font size independence */
    Boolean needs_layout;
    short layout_disabled;
    Widget toplevel;
    struct _XmtLayoutInfo *widget_info;
    XmtLexer lexer;
    Boolean in_parser;
    Boolean geometry_okay;
} XmtLayoutPart;

typedef struct _XmtLayoutRec {
    CorePart core;
    CompositePart composite;
    ConstraintPart constraint;
    XmManagerPart manager;
    XmBulletinBoardPart	bulletin_board;
    XmtLayoutPart layout;
} XmtLayoutRec;

typedef struct {
    /* resources */
    Dimension width;
    Dimension height;
    Dimension stretchability;
    Dimension shrinkability;
    Boolean allow_resize;
    Boolean sensitive;
    unsigned char justification;
    unsigned char margin_width;
    unsigned char margin_height;
    unsigned char frame_type;
    unsigned char frame_line_type;
    unsigned char frame_position;
    unsigned char frame_margin;
    unsigned char frame_thickness;
    unsigned char caption_position;
    unsigned char caption_justification;
    unsigned char caption_alignment;
    unsigned char caption_margin;
    XmString caption;
    Widget in;         /* the row or column we're in */
    Position position; /* and the position we're at */
    Widget after;      /* or the object we're after  in the layout*/
    Widget before;     /* or the object we're before in the layout */
    /* private */
    Widget first_child;       /* the first item in a row or col */
    Dimension pref_w, pref_h;
    Dimension caption_width, caption_height;
    unsigned char type;
    Boolean dont_copy_caption;
} XmtLayoutConstraintsPart;

typedef struct {
    XmManagerConstraintPart  manager;
    XmtLayoutConstraintsPart layout;
} XmtLayoutConstraintsRec, *XmtLayoutConstraints;

/*
 * This structure is used to store information about children widgets
 * that is parsed from the layout string before those widgets are
 * created.  The flags field indicates which fields in the constraints
 * record are valid.
 */
typedef struct _XmtLayoutInfo {
    XrmQuark name;
    XmtLayoutConstraintsPart constraints;
    XmtLayoutConstraintsPart dummy_constraints;
    struct _XmtLayoutInfo *next;
} XmtLayoutInfo;

#define Constraint(obj, field) \
    (((XmtLayoutConstraints)(((Object)obj)->object.constraints))->layout.field)

#define LayoutConstraintsRec(obj) \
    ((XmtLayoutConstraintsPart *)\
    &(((XmtLayoutConstraints)(((Object)obj)->object.constraints))->layout))

#define LayoutConstraints(obj) \
    (((XmtLayoutConstraints)(((Object)obj)->object.constraints))->layout)

#define ForEachChild(w, child)\
    for(i=0;\
	(i < w->composite.num_children) && (child=lw->composite.children[i]);\
	i++)

#define ForEachItem(b,i)\
    for(i = LayoutConstraints(b).first_child;\
	i;\
	i = LayoutConstraints(i).before)

/* a very big number */
/* but not so big that we're likely to overflow an unsigned short */
#define INFINITY 5000

#define XmtLAYOUT_DEFAULT_STRETCHABILITY 10
#define XmtLAYOUT_DEFAULT_SHRINKABILITY 10

/* a very gross hack */
#define XmtIsLayoutGadget(w)\
    XtIsSubclass((w), xmtLayoutGadgetClass)
#define XmtUnsetRectObj(w)\
    ((w)->core.widget_class->core_class.class_inited &= ~0x2)
#define XmtSetRectObj(w)\
    ((w)->core.widget_class->core_class.class_inited |= 0x2)

#undef XtIsManaged  /* XmP.h defines this to check for RectObj */
#define XtIsManaged(w)\
    ((XtIsRectObj(w) || XmtIsLayoutGadget(w)) && (w)->core.managed)


_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void _XmtLayoutChildren(XmtLayoutWidget, XmtWideBoolean);
#else
extern void _XmtLayoutChildren();
#endif
_XFUNCPROTOEND

#endif /* _XmtLayoutP_h */
