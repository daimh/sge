/* 
 * Motif Tools Library, Version 3.1
 * $Id: Layout.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Layout.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/XmtP.h>
#include <Xmt/LayoutP.h>
#include <Xmt/LayoutGP.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Pixmap.h>
#include <Xm/DialogS.h>
#if (XmVersion >= 1002)
#include <Xm/DrawP.h>  /* declaration of Motif drawing routines */
#endif

#define offset(field) XtOffsetOf(XmtLayoutRec, field)
static XtResource resources[] = {
{XmtNlayout, XmtCLayout, XtRString, sizeof(String),
     offset(layout.layout), XtRString, NULL},
#if XmVersion < 2000
{XmtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     offset(layout.font), XtRString, XtDefaultFont},
#else /* Motif 2.0 or later */
{XmtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     offset(layout.font), XtRImmediate, (XtPointer) NULL},
#endif
{XmtNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
     offset(layout.font_list), XtRImmediate, (XtPointer) NULL},
#if XmVersion >= 2000
{XmtNrenderTable, XmCRenderTable, XmRRenderTable,
     sizeof(XmRenderTable), offset(layout.render_table),
     XtRImmediate, (XtPointer) NULL},
#endif
{XmtNdefaultSpacing, XmtCDefaultSpacing, XtRDimension, sizeof(Dimension),
     offset(layout.default_spacing), XtRImmediate, (XtPointer) 10},
{XmtNorientation, XmCOrientation, XmROrientation,
     sizeof(unsigned char), offset(layout.orientation),
     XtRImmediate, (XtPointer) XmVERTICAL},
{XmtNdebugLayout, XmtCDebugLayout, XtRBoolean,
     sizeof(Boolean), offset(layout.debug_layout),
     XtRImmediate, (XtPointer)False},
/* override some BulletinBoard defaults */
{XmNmarginWidth, XmCMarginWidth, XmRHorizontalDimension,
     sizeof(Dimension),
     XtOffsetOf(struct _XmBulletinBoardRec, bulletin_board.margin_width),
     XmRImmediate, (XtPointer)5},
{XmNmarginHeight, XmCMarginHeight, XmRVerticalDimension,
     sizeof(Dimension),
     XtOffsetOf(struct _XmBulletinBoardRec, bulletin_board.margin_height),
     XmRImmediate, (XtPointer)5},
};
#undef offset

#define offset(field) XtOffsetOf(XmtLayoutConstraintsRec, layout.field)
static XtResource constraint_resources[] = {
{XmtNlayoutWidth, XmtCLayoutWidth, XtRDimension,
     sizeof(Dimension), offset(width),
     XtRImmediate, (XtPointer) 0},
{XmtNlayoutHeight, XmtCLayoutHeight, XtRDimension,
     sizeof(Dimension), offset(height),
     XtRImmediate, (XtPointer) 0},
{XmtNlayoutStretchability, XmtCLayoutStretchability, XtRDimension,
     sizeof(Dimension), offset(stretchability),
     XtRImmediate, (XtPointer) -1},  /* dynamic default */
{XmtNlayoutShrinkability, XmtCLayoutShrinkability, XtRDimension,
     sizeof(Dimension), offset(shrinkability),
     XtRImmediate, (XtPointer) -1},  /* dynamic default */
{XmtNlayoutCaption, XmtCLayoutCaption, XmRXmString,
     sizeof(XmString), offset(caption),
     XtRImmediate, NULL},
{XmtNlayoutIn, XmtCLayoutIn, XtRWidget,
     sizeof(Object), offset(in),
     XtRImmediate, (XtPointer) NULL},
{XmtNlayoutPosition, XmtCLayoutPosition, XtRPosition,
     sizeof(Position), offset(position),
     XtRImmediate, (XtPointer) -1},
{XmtNlayoutAfter, XmtCLayoutAfter, XtRWidget,
     sizeof(Object), offset(after),
     XtRImmediate, (XtPointer) NULL},
{XmtNlayoutBefore, XmtCLayoutBefore, XtRWidget,
     sizeof(Object), offset(before),
     XtRImmediate, (XtPointer) NULL},
{XmtNlayoutAllowResize, XmtCLayoutAllowResize, XtRBoolean,
     sizeof(Boolean), offset(allow_resize),
     XtRImmediate, (XtPointer)True},
{XmtNlayoutSensitive, XmtCLayoutSensitive, XtRBoolean,
     sizeof(Boolean), offset(sensitive),
     XtRImmediate, (XtPointer)True},
{XmtNlayoutJustification, XmtCLayoutJustification, XmtRXmtLayoutJustification,
     sizeof(unsigned char), offset(justification),
     XtRImmediate, (XtPointer)XmtLayoutFilled},
{XmtNlayoutMarginWidth, XmtCLayoutMarginWidth, XtRUnsignedChar,
     sizeof(unsigned char), offset(margin_width),
     XtRImmediate, (XtPointer)255},  /* special unset value */
{XmtNlayoutMarginHeight, XmtCLayoutMarginHeight, XtRUnsignedChar,
     sizeof(unsigned char), offset(margin_height),
     XtRImmediate, (XtPointer)255},  /* special unset value */
{XmtNlayoutFrameType, XmtCLayoutFrameType, XmtRXmtLayoutFrameType,
     sizeof(unsigned char), offset(frame_type),
     XtRImmediate, (XtPointer)XmtLayoutFrameNone},
{XmtNlayoutFrameLineType, XmtCLayoutFrameLineType, XmtRXmtLayoutFrameLineType,
     sizeof(unsigned char), offset(frame_line_type),
     XtRImmediate, (XtPointer)XmtLayoutFrameEtchedIn},
{XmtNlayoutFramePosition, XmtCLayoutFramePosition, XmtRXmtLayoutFramePosition,
     sizeof(unsigned char), offset(frame_position),
     XtRImmediate, (XtPointer)XmtLayoutFrameInside},
{XmtNlayoutFrameMargin, XmtCLayoutFrameMargin, XtRUnsignedChar,
     sizeof(unsigned char), offset(frame_margin),
     XtRImmediate, (XtPointer)2},
{XmtNlayoutFrameThickness, XmtCLayoutFrameThickness, XtRUnsignedChar,
     sizeof(unsigned char), offset(frame_thickness),
     XtRImmediate, (XtPointer)2},
{XmtNlayoutCaptionPosition, XmtCLayoutCaptionPosition, XmtRXmtLayoutEdge,
     sizeof(unsigned char), offset(caption_position),
     XtRImmediate, (XtPointer)XmtLayoutLeft},
{XmtNlayoutCaptionJustification, XmtCLayoutCaptionJustification,
     XmtRXmtLayoutJustification,
     sizeof(unsigned char), offset(caption_justification),
     XtRImmediate, (XtPointer)XmtLayoutCentered},
{XmtNlayoutCaptionAlignment, XmtCLayoutCaptionAlignment, XmRAlignment,
     sizeof(unsigned char), offset(caption_alignment),
     XtRImmediate, (XtPointer)XmALIGNMENT_BEGINNING},
{XmtNlayoutCaptionMargin, XmtCLayoutCaptionMargin, XtRUnsignedChar,
     sizeof(unsigned char), offset(caption_margin),
     XtRImmediate, (XtPointer)2},
};
#undef offset

#if NeedFunctionPrototypes
static void ClassInitialize(void);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void Resize(Widget);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Redisplay(Widget, XEvent *, Region);
static void ChangeManaged(Widget);
static XtGeometryResult GeometryManager(Widget, XtWidgetGeometry *,
                                        XtWidgetGeometry *);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *,
                                      XtWidgetGeometry *);
static void InsertChild(Widget);
static void DeleteChild(Widget);
static void ConstraintInitialize(Widget, Widget, ArgList, Cardinal *);
static void ConstraintDestroy(Widget);
static Boolean ConstraintSetValues(Widget,Widget, Widget, ArgList, Cardinal *);
#else
static void ClassInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Resize();
static void Realize();
static void Redisplay();
static void ChangeManaged();
static XtGeometryResult GeometryManager();
static XtGeometryResult QueryGeometry();
static void InsertChild();
static void DeleteChild();
static void ConstraintInitialize();
static void ConstraintDestroy();
static Boolean ConstraintSetValues();
#endif

#define superclass (&xmBulletinBoardClassRec)

externaldef(xmtlayoutclassrec) XmtLayoutClassRec xmtLayoutClassRec = {
{   /* core_class fields */
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "XmtLayout",
    /* widget_size        */    sizeof(XmtLayoutRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    XtExposeCompressMaximal,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    Resize,
    /* expose             */    Redisplay,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    XtInheritTranslations,
    /* query_geometry     */    QueryGeometry,
    /* display_accelerator*/    XtInheritDisplayAccelerator,
    /* extension          */    NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child       */    InsertChild,
    /* delete_child       */    DeleteChild,
    /* extension          */    NULL
  },
  { /* constraint_class fields */
    /* resource list 	  */    constraint_resources,
    /* num resources	  */    XtNumber(constraint_resources),
    /* constraint size    */    sizeof(XmtLayoutConstraintsRec),
    /* init proc	  */    ConstraintInitialize,
    /* destroy proc       */    ConstraintDestroy,
    /* set values proc    */    ConstraintSetValues,
    /* extension 	  */    NULL
  },
  { /* manager_class	  */
    /* translations 	  */    XtInheritTranslations,
    /* syn_resources	  */	NULL,
    /* num_syn_resources  */	0,
    /* syn_cont_resources */	NULL,
    /* num_syn_cont_resources */0,
    /* parent_process     */    XmInheritParentProcess,
    /* extension	  */	NULL
  },
  { /* Bulletin Board     */
    /* always_install_accelerators */ False,
    /* geo_matrix_create  */	NULL,
    /* focus_moved_proc   */	XmInheritFocusMovedProc,
    /* extension	  */    NULL
  },
  { /* XmtLayout          */
    /* parser             */    NULL,
    /* lookup_type_proc   */    NULL,
    /* create_proc        */    NULL,
    /* extension	  */    NULL
  }
};

externaldef(xmtlayoutwidgetclass)
WidgetClass xmtLayoutWidgetClass = (WidgetClass) &xmtLayoutClassRec;

#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    static String justification_names[] = {
	"Bottom", "Center", "Centered", "Filled", "Left", "Right", "Top"};
    static Cardinal justification_values[] = {
	XmtLayoutFlushBottom, XmtLayoutCentered, XmtLayoutCentered,
	XmtLayoutFilled,XmtLayoutFlushLeft,
	XmtLayoutFlushRight, XmtLayoutFlushTop};
    static String justification_prefixes[] = {"Xmt", "Layout", "Flush", NULL};
    
    static String edge_names[] = {
	"Bottom", "Left", "Right", "Top"};
    static Cardinal edge_values[] = {
        XmtLayoutBottom, XmtLayoutLeft, XmtLayoutRight, XmtLayoutTop
    };
    static String edge_prefixes[] = {"Xmt", "Layout", NULL};

    static String line_type_names[] = {
        "Double", "DoubleLine", "Etched", "EtchedIn", "EtchedOut",
        "Shadow", "ShadowIn", "ShadowOut", "Single", "SingleLine",
    };
    static Cardinal line_type_values[] = {
        XmtLayoutFrameDoubleLine, XmtLayoutFrameDoubleLine,
        XmtLayoutFrameEtchedIn, XmtLayoutFrameEtchedIn,
        XmtLayoutFrameEtchedOut, XmtLayoutFrameShadowIn,
        XmtLayoutFrameShadowIn, XmtLayoutFrameShadowOut,
        XmtLayoutFrameSingleLine, XmtLayoutFrameSingleLine
    };

    static String frame_type_names[] = {
        "Bottom", "Box", "Left", "None", "Right", "Top" };
    static Cardinal frame_type_values[] = {
        XmtLayoutFrameBottom, XmtLayoutFrameBox, XmtLayoutFrameLeft,
        XmtLayoutFrameNone, XmtLayoutFrameRight, XmtLayoutFrameTop
    };
    static String frame_type_prefixes[]  = {"Xmt", "Layout", "Frame", NULL};

    static String position_names[] = { "Inside", "Outside", "Through" };
    static Cardinal position_values[] = {
        XmtLayoutFrameInside, XmtLayoutFrameOutside, XmtLayoutFrameThrough };

    static String space_type_names[] = {
        "CTabbed", "Even", "Interval", "LCR",
	"LREven", "LTabbed", "None", "RTabbed"
    };
    static Cardinal space_type_values[] = {
        XmtLayoutSpaceCTabbed, XmtLayoutSpaceEven,
	XmtLayoutSpaceInterval, XmtLayoutSpaceLCR,
        XmtLayoutSpaceLREven, XmtLayoutSpaceLTabbed,
	XmtLayoutSpaceNone, XmtLayoutSpaceRTabbed
    };
    static String space_type_prefixes[] = {"Xmt", "Layout", "Space", NULL};
    
/* A bitmap we use for stippling captions */
#   define _xmt_gray50_width 32
#   define _xmt_gray50_height 2
    static unsigned char _xmt_gray50_bits[] = {
	0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa};

    XmtRegisterEnumConverter(XmtRXmtLayoutJustification,
			     justification_names, justification_values,
			     XtNumber(justification_names),
			     justification_prefixes);

    XmtRegisterEnumConverter(XmtRXmtLayoutEdge,
			     edge_names, edge_values,
			     XtNumber(edge_names),
			     edge_prefixes);

    XmtRegisterEnumConverter(XmtRXmtLayoutFrameLineType,
			     line_type_names, line_type_values,
			     XtNumber(line_type_names),
			     frame_type_prefixes); /* really frame_type here */

    XmtRegisterEnumConverter(XmtRXmtLayoutFrameType,
			     frame_type_names, frame_type_values,
			     XtNumber(frame_type_names),
			     frame_type_prefixes);

    XmtRegisterEnumConverter(XmtRXmtLayoutFramePosition,
			     position_names, position_values,
			     XtNumber(position_names),
			     frame_type_prefixes); /* really */

    XmtRegisterEnumConverter(XmtRXmtLayoutSpaceType,
			     space_type_names, space_type_values,
			     XtNumber(space_type_names),
			     space_type_prefixes);

    XmtRegisterXbmData("_xmt_gray50", (char *)_xmt_gray50_bits, NULL,
		       _xmt_gray50_width, _xmt_gray50_height, 0, 0);
}

#if XmVersion < 2000
#if NeedFunctionPrototypes
static void FontFromFontList(XmtLayoutWidget lw)
#else
static void FontFromFontList(lw)
XmtLayoutWidget lw;
#endif
{
    XmFontContext context;
    XmStringCharSet charset;

    lw->layout.font_list = XmFontListCopy(lw->layout.font_list);
    (void)XmFontListInitFontContext(&context, lw->layout.font_list);
    (void)XmFontListGetNextFont(context, &charset, &lw->layout.font);
    XmFontListFreeFontContext(context);
    XtFree(charset);
}
#endif

#if NeedFunctionPrototypes
static void GetGCs(XmtLayoutWidget lw)
#else
static void GetGCs(lw)
XmtLayoutWidget lw;
#endif
{
    XGCValues gcv;
    XtGCMask mask;

    /*
     * we set a font, even though we don't use it because XmStringDraw
     * frobs the font field and will cause an X error if it is unset.
     */
    gcv.foreground = lw->manager.foreground;
    gcv.background = lw->core.background_pixel;
    gcv.font = lw->layout.font->fid;
    gcv.line_width = 0;
    gcv.graphics_exposures = False;
    mask = GCForeground | GCBackground | GCFont | GCLineWidth |
	GCGraphicsExposures;
    lw->layout.gc = XtGetGC((Widget)lw, mask, &gcv);

    /*
     * We could remember this bitmap and free it when the Layout is destroyed,
     * but because of caching, this will rarely be worth the trouble.
     */
    gcv.fill_style = FillStippled;
    gcv.stipple = XmtLookupBitmap((Widget)lw, "_xmt_gray50");
    mask |= GCFillStyle | GCStipple;

    lw->layout.grey_gc = XtGetGC((Widget)lw, mask, &gcv);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Initialize(Widget request, Widget init,
		       ArgList args, Cardinal *num_args)
#else
static void Initialize(request, init, args, num_args)
Widget request;
Widget init;
ArgList args;
Cardinal *num_args;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) init;
    Display *dpy = XtDisplay(init);
    int scr = XScreenNumberOfScreen(XtScreen(init));
    Arg arg;

#if XmVersion < 2000
    /*
     * Copy the font list resource, and get its first font.
     * Or, create a font list from the font.
     */
    if (lw->layout.font_list)
	FontFromFontList(lw);
    else
	lw->layout.font_list = XmFontListCreate(lw->layout.font,
						XmSTRING_DEFAULT_CHARSET);
#else /* Motif 2.0 or later */
    if (!lw->layout.render_table) {
        if (!lw->layout.font_list) {
            if (lw->layout.font) {
                Arg args[1];
                XmRendition rendition;
		
                /*
                 * If no renderTable or fontList was specified, but there is
                 * a font, then create a renderTable.
                 */
                XtSetArg(args[0], XmNfont, lw->layout.font);
                rendition = XmRenditionCreate(init, (XmStringTag)"default",
                                              args, 1);
                lw->layout.render_table =
#if (XmVersion >= 2001)
		    XmRenderTableAddRenditions(NULL, &rendition, 1,
					       XmMERGE_REPLACE);
#else
		    XmRenderTableAddRenditions(NULL, &rendition, 1, XmREPLACE);
#endif
            }
            else {
                /*
                 * Nothing was specified, so look up the default
                 * renderTable.
                 */
                lw->layout.render_table =
		    XmeGetDefaultRenderTable(init, XmLABEL_FONTLIST);
		
                /* Make a copy of the renderTable. */
                lw->layout.render_table =
                    XmRenderTableCopy(lw->layout.render_table, NULL, 0);
            }
        }
        else {
            /*
             * There is a fontList, so copy it to the renderTable.
             * NOTE: This code relies on the fact that there is no
             *       disitinction between a fontList and a renderTable
             *       in Motif 2.0.
             */
	    lw->layout.render_table = XmFontListCopy(lw->layout.font_list);
	}
    }
    else {
        /* Make a copy of the renderTable. */
        lw->layout.render_table =
	    XmRenderTableCopy(lw->layout.render_table, NULL, 0);
    }
    
    /*
     * If no font was explicitly given, extract the default font from
     * the renderTable.
     */
    if (!lw->layout.font)
        XmeRenderTableGetDefaultFont(lw->layout.render_table,
                                     &lw->layout.font);
    /*
     * The previous fails with lesstif, try again...
     */
    if (!lw->layout.font)
        _XmFontListGetDefaultFont(lw->layout.font_list,
                                     &lw->layout.font);
#endif
    
    /* figure out resolution independence values */
    lw->layout.pixpermm =(DisplayWidth(dpy,scr)/DisplayWidthMM(dpy,scr) +
			  DisplayHeight(dpy,scr)/DisplayHeightMM(dpy,scr)) / 2;
    lw->layout.pixperem = XTextWidth(lw->layout.font, "M", 1);


    /* get a GC for use by our gadget children */
    GetGCs(lw);

    /* initialize misc. variables */
    lw->layout.needs_layout = False;
    lw->layout.layout_disabled = 0;

    lw->layout.in_parser = False;
    lw->layout.geometry_okay = False;
    lw->layout.widget_info = NULL;

    /*
     * create the toplevel XmtLayoutBox gadget.
     */
    lw->layout.in_parser = True;
    XtSetArg(arg, XmtNorientation, lw->layout.orientation);
    /*
     * got to initialize it to NULL first in order to detect
     * this special case in ConstraintInitialize
     */
    lw->layout.toplevel = NULL;
    lw->layout.toplevel = XmtCreateLayoutBox(init, "topbox", &arg, 1);
    XtManageChild(lw->layout.toplevel);

    /*
     * go parse the layout string, if any.
     * We don't copy this string because we parse it once
     * and never need it again.
     */
    if (lw->layout.layout) {
	if (xmtLayoutClassRec.layout_class.parser == NULL)
	    XmtWarningMsg("XmtLayout", "noParser",
			  "%s: no parser registered for layout string.\n\tSee XmtRegisterLayoutParser().",
			  XtName((Widget)lw));
	else
	    (*xmtLayoutClassRec.layout_class.parser)(lw, lw->layout.layout);
    }
    lw->layout.in_parser = False;

    /*
     * go layout any children that were automatically created by
     * parsing the layout string, but don't issue a resize request for
     * the desired size.  If width and height are not set, we set them
     * from the preferred size.
     */
    _XmtLayoutChildren(lw, True);
    if (lw->core.width == 0)
	lw->core.width = LayoutConstraints(lw->layout.toplevel).pref_w +
	    2*lw->bulletin_board.margin_width;
    
    if (lw->core.height == 0)
	lw->core.height = LayoutConstraints(lw->layout.toplevel).pref_h +
 	    2*lw->bulletin_board.margin_height;
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) w;

#if XmVersion < 2000
    if (lw->layout.font_list) XmFontListFree(lw->layout.font_list);
#else /* Motif 2.0 or later */
    if (lw->layout.render_table) XmRenderTableFree(lw->layout.render_table);
#endif

    XtReleaseGC(w, lw->layout.gc);
    XtReleaseGC(w, lw->layout.grey_gc);
}

#if NeedFunctionPrototypes
static void Realize(Widget w, XtValueMask *vm, XSetWindowAttributes *wa)
#else
static void Realize(w, vm, wa)
Widget w;
XtValueMask *vm;
XSetWindowAttributes *wa;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)w;
    Widget *children;
    Cardinal num;
    int i;
    
    /*
     * Because Motif (1.1, at least) treats all RectObjs as XmGadgets,
     * we've had to make LayoutGadgets a subclass of Object that just
     * happen to have many of the same fields as RectObj.  There are
     * a number of hacks here to support this--see the redefinition of
     * XtIsManaged(), and XmtSetRectObj(), for example, in LayoutP.h
     * 
     * This is another of those hacks--when the Layout is realized with
     * only gadget children (as can happen with the XmtWorkingBox, e.g.)
     * the Intrinsics decide that it has no managed children, and never
     * invoke the ChangeManaged procedure.  We attempt to compensate for
     * that here.
     */
    children = lw->composite.children;
    num = lw->composite.num_children;
    for(i=0; i < num; i++)
	if (XtIsManaged(children[i]) &&
	    (XtIsWidget(children[i]) || !XmtIsLayoutGadget(children[i])))
	    break;
    if (i == num)
	ChangeManaged(w);

    /*
     * And now envelop our superclass's realize method, so we
     * actually get realized.
     */
    (*superclass->core_class.realize)(w, vm, wa);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static Boolean SetValues(Widget current, Widget request, Widget set,
			 ArgList args, Cardinal *num_args)
#else
static Boolean SetValues(current, request, set, args, num_args)
Widget current;
Widget request;
Widget set;
ArgList args;
Cardinal *num_args;
#endif
{
    XmtLayoutWidget cw = (XmtLayoutWidget) current;
    XmtLayoutWidget nw = (XmtLayoutWidget) set;
    Boolean got_new_gc = False;
    Boolean relayout = False;
    Boolean redisplay = False;

#define Changed(field) (nw->layout.field != cw->layout.field)

    /* can't change XmtNlayout */
    if (Changed(layout)) {
	XmtWarningMsg("XmtLayout", "setlayout",
		      "%s: the XmtNlayout resource may not be changed.",
		      XtName(set));
	nw->layout.layout = cw->layout.layout;
    }

    /* can't change XmtNorientation */
    if (Changed(orientation)) {
	XmtWarningMsg("XmtLayout", "setorient",
		      "%s: the XmtNorientation resource may not be changed.",
		      XtName(set));
	nw->layout.orientation = cw->layout.orientation;
    }
    
#if XmVersion < 2000
    /*
     * if fontlist or font changes, free old and figure out the other one.
     */
    if (Changed(font_list)) {
	XmFontListFree(cw->layout.font_list);
	FontFromFontList(nw);
    }
    else if (Changed(font)) {
	XmFontListFree(cw->layout.font_list);
	nw->layout.font_list = XmFontListCreate(nw->layout.font,
						XmSTRING_DEFAULT_CHARSET);
    }
#else /* Motif 2.0 or later */
    if (Changed(render_table) || !nw->layout.font) {
        XmRenderTableFree(cw->layout.render_table);
        if (nw->layout.render_table == NULL) {
            nw->layout.render_table =
               XmeGetDefaultRenderTable(set, XmLABEL_FONTLIST);
        }
        nw->layout.render_table =
           XmRenderTableCopy(nw->layout.render_table, NULL, 0);
	/* unset the font, so it will be recomputed below */
	nw->layout.font = NULL;
    }
    else if (Changed(font_list)) {
        XmRenderTableFree(cw->layout.render_table);
        if (nw->layout.font_list == NULL) {
            nw->layout.font_list =
               XmeGetDefaultRenderTable(set, XmLABEL_FONTLIST);
        }
        nw->layout.render_table = XmFontListCopy(nw->layout.font_list);
	/* unset the font, so it will be recomputed below */
	nw->layout.font = NULL;
    }
    else if (Changed(font)) {
        Arg args[1];
        XmRendition rendition;

        XmRenderTableFree(cw->layout.render_table);

        XtSetArg(args[0], XmNfont, nw->layout.font);
        rendition = XmRenditionCreate(set, (XmStringTag)"default",
                                      args, 1);
        nw->layout.render_table =
#if (XmVersion >= 2001)
           XmRenderTableAddRenditions(NULL, &rendition, 1, XmMERGE_REPLACE);
#else
           XmRenderTableAddRenditions(NULL, &rendition, 1, XmREPLACE);
#endif
    }

    if (!nw->layout.font)
        /* Extract the default font from the renderTable. */
        XmeRenderTableGetDefaultFont(nw->layout.render_table,
                                     &nw->layout.font);
#endif
    
    /*
     * if the font list changed through either of the above cases,
     * get a new GC, change resolution independence values, 
     * invoke the change_font method on all LayoutStringGadget children,
     * and re-measure all caption strings.
     */
#if XmVersion < 2000
      if (Changed(font_list)) {
#else /* Motif 2.0 or later */
    if (Changed(render_table)) {
#endif
	int i;
	Widget child;
	XmtLayoutConstraintsPart *c;
	
	XtReleaseGC(set, cw->layout.gc);
	XtReleaseGC(set, cw->layout.grey_gc);
	GetGCs(nw);
	got_new_gc = True;

	/*
	 * we can compute new resolution independence conversion values,
	 * but we cannot go update all the children that had their size
	 * based on the font...
	 */
	nw->layout.pixperem = XTextWidth(nw->layout.font, "M", 1);

	/*
	 * tell any LayoutGadget children that care about the change.
	 * This is a hardcoded dependency which is a little gross.
	 * It means that we don't have to keep a bunch of copies of
	 * the same fontlist around, however.
	 */
	for(i=0; i < nw->composite.num_children; i++) {
	    child = nw->composite.children[i];
	    c = LayoutConstraintsRec(child);
	    if (!XtIsWidget(child) && !XmIsGadget(child)
		&& XtIsSubclass(child, xmtLayoutGadgetClass))
		_XmtCallChangeFontMethod(child);
	    if (c->caption) {
#if XmVersion < 2000
		XmStringExtent(nw->layout.font_list, c->caption,
			       &c->caption_width, &c->caption_height);
#else /* Motif 2.0 or later */
		XmStringExtent(nw->layout.render_table, c->caption,
			       &c->caption_width, &c->caption_height);
#endif
	    }
	}
	
	relayout = True;
    }

    if (Changed(debug_layout))
	relayout = True;

    /* if colors changed and we haven't already done so, get a new gc */
    if (!got_new_gc &&
	((nw->core.background_pixel != cw->core.background_pixel) ||
	 (nw->manager.foreground != cw->manager.foreground))) {
	XtReleaseGC(set, cw->layout.gc);
	XtReleaseGC(set, cw->layout.grey_gc);
	GetGCs(nw);
	redisplay = True;
    }

    /* if margin size changed, re-layout everything */
    if ((nw->bulletin_board.margin_width != cw->bulletin_board.margin_width) ||
	(nw->bulletin_board.margin_height != cw->bulletin_board.margin_height))
	relayout = True;

    /* if sensitivity changed, redisplay everything to change captions */
    if (nw->core.sensitive != cw->core.sensitive) redisplay = True;

    if (relayout)
	_XmtLayoutChildren(nw, True);
    
    return redisplay | relayout;

#undef Changed
}

#if NeedFunctionPrototypes
void _XmtLayoutChildren(XmtLayoutWidget lw, XmtWideBoolean resize)
#else
void _XmtLayoutChildren(lw, resize)
XmtLayoutWidget lw;
int resize;
#endif
{
    XtWidgetGeometry geometry;

    /* don't do anything if we've been told not to,
     * but remember to later
     */
    if (lw->layout.layout_disabled > 0) {
	lw->layout.needs_layout = True;
	return;
    }

    /*
     * figure out how big we'd like to be.
     * the gadget's query_geometry methods also do other necessary
     * computation, so this must be called before the resize method...
     */
    XtQueryGeometry(lw->layout.toplevel, NULL, &geometry);

    /* XXX
     * Under some circumstances, this query may return a width and height
     * of 0.  If the margins are zero as well, the call to
     * XtMakeResizeRequest() below will cause an X Error.  So we ensure
     * that this never happens.  Note that it is probably a bug that is
     * making the query return a 0,0 size, but it can't hurt to be safe here...
     */
    if (geometry.width == 0) geometry.width = 1;
    if (geometry.height == 0) geometry.height = 1;

    /* remember this preferred size */
    LayoutConstraints(lw->layout.toplevel).pref_w = geometry.width;
    LayoutConstraints(lw->layout.toplevel).pref_h = geometry.height;

    /* add a margin all the way around */
    geometry.width += 2*lw->bulletin_board.margin_width;
    geometry.height += 2*lw->bulletin_board.margin_height;

    /* ask for size plus margins unless we're being called from Resize */
    if (resize &&
	((geometry.width != lw->core.width) ||
	 (geometry.height != lw->core.height))) {
	XtGeometryResult result;
	Dimension compromisew, compromiseh;
	result = XtMakeResizeRequest((Widget)lw,
				     geometry.width, geometry.height,
				     &compromisew, &compromiseh);
	if (result == XtGeometryAlmost)
	    XtMakeResizeRequest((Widget)lw,compromisew,compromiseh,NULL, NULL);
    }

    /*
     * Whether we got a new size or not, configure all the kids
     * to the current size.
     * If the size hasn't changed, this call is a no-op.
     * This means that when we're called from changeManaged, often
     * nothing is called, and the new widget doesn't get configured
     * to appear in the right place.  To work around this, we explictly
     * set the width on the toplevel box to something different.
     */
    lw->layout.toplevel->core.width = 0;
    XtConfigureWidget(lw->layout.toplevel,
		      lw->bulletin_board.margin_width,
		      lw->bulletin_board.margin_height,
		      lw->core.width - 2*lw->bulletin_board.margin_width,
		      lw->core.height - 2*lw->bulletin_board.margin_height,
		      lw->layout.toplevel->core.border_width);

    lw->layout.needs_layout = False;

    /*
     * When the Layout contains a lot of layout gadgets, this relayout
     * process will cause a lot of XClearArea() expose events.  For
     * complicated layouts, the Intrinsics may call the Expose() method
     * before all the events have arrived and been put in the queue.
     * Adding a call to XSync() here seems to help with this problem
     * in some cases.
     */
    XSync(XtDisplay(lw), False);
}


#if NeedFunctionPrototypes
static void DrawFrame(XmtLayoutWidget lw, XmtLayoutFrameType type,
		      XmtLayoutFrameLineType linetype,
		      int thickness, int x, int y, int w, int h)
#else
static void DrawFrame(lw, type, linetype, thickness, x, y, w, h)
XmtLayoutWidget lw;
XmtLayoutFrameType type;
XmtLayoutFrameLineType linetype;
int thickness;
int x;
int y;
int w;
int h;
#endif
{
    Display *dpy = XtDisplay((Widget)lw);
    Window win = XtWindow((Widget)lw);
    Boolean horizontal = False;
    GC topgc = lw->manager.top_shadow_GC;
    GC botgc = lw->manager.bottom_shadow_GC;
    GC tmpgc;  /* for swapping */
    int halfthick = thickness/2;

    if (type == XmtLayoutFrameNone) return;

    if (type == XmtLayoutFrameTop || type == XmtLayoutFrameBottom) 
	horizontal = True;
    
    if (thickness == 1) linetype = XmtLayoutFrameSingleLine;

    if (linetype == XmtLayoutFrameSingleLine) {
	XGCValues values;
	
	if (thickness > 1) {
	    values.line_width = thickness;
	    XChangeGC(dpy, lw->layout.gc, GCLineWidth, &values);
	}

	if (type == XmtLayoutFrameBox)
	    XDrawRectangle(dpy, win, lw->layout.gc,
			   x+halfthick, y+halfthick,
			   w-thickness, h-thickness);
	else if (horizontal)
	    XDrawLine(dpy, win, lw->layout.gc,
		      x, y+halfthick, x+w, y+halfthick);
	else
	    XDrawLine(dpy, win, lw->layout.gc,
		      x+halfthick, y, x+halfthick, y+h);
	    
	if (thickness > 1) {
	    values.line_width = 0;
	    XChangeGC(dpy, lw->layout.gc, GCLineWidth, &values);
	}
    }
    else if (linetype == XmtLayoutFrameDoubleLine)
    {
	if (type == XmtLayoutFrameBox) {
	    XDrawRectangle(dpy, win, lw->layout.gc, x, y, w-1, h-1);
	    XDrawRectangle(dpy, win, lw->layout.gc,
			   x+thickness-1, y+thickness-1,
			   w-2*(thickness-1)-1, h-2*(thickness-1)-1);
	}
	else if (horizontal) {
	    XDrawLine(dpy, win, lw->layout.gc, x, y, x+w, y);
	    XDrawLine(dpy, win, lw->layout.gc,
		      x, y+thickness-1, x+w, y+thickness-1);
	}
	else {
	    XDrawLine(dpy, win, lw->layout.gc, x, y, x, y+h);
	    XDrawLine(dpy, win, lw->layout.gc,
		      x+thickness-1, y, x+thickness-1, y+h);
	}
    }
    else {
	if (type == XmtLayoutFrameBox) {
#if XmVersion >= 1002
	    {
		int xmtype;
		
		switch(linetype) {
		case XmtLayoutFrameShadowIn:
		    xmtype = XmSHADOW_IN;
		    break;
		case XmtLayoutFrameShadowOut:
		    xmtype = XmSHADOW_OUT;
		    break;
		default:
		case XmtLayoutFrameEtchedIn:
		    xmtype = XmSHADOW_ETCHED_IN;
		    break;
		case XmtLayoutFrameEtchedOut:
		    xmtype = XmSHADOW_ETCHED_OUT;
		    break;
		}
#if XmVersion >= 2000
		XmeDrawShadows(dpy, win, topgc, botgc,
			       x, y, w, h, thickness, xmtype);
#else
		_XmDrawShadows(dpy, win, topgc, botgc,
			       x, y, w, h, thickness, xmtype);
#endif
	    }
#else
	    {
		int roundthick = halfthick*2;

		if (linetype == XmtLayoutFrameShadowIn ||
		    linetype == XmtLayoutFrameEtchedIn) {
		    tmpgc = botgc;
		    botgc = topgc;
		    topgc = tmpgc;
		}
		
		switch(linetype) {
		case XmtLayoutFrameShadowOut:
		case XmtLayoutFrameShadowIn:
		    _XmDrawShadow(dpy, win, topgc, botgc, thickness,
				  x, y, w, h);
		    break;
		case XmtLayoutFrameEtchedOut:
		case XmtLayoutFrameEtchedIn:
		    _XmDrawShadow(dpy, win, topgc, botgc, halfthick,
				  x, y, w, h);
		    _XmDrawShadow(dpy, win, botgc, topgc, halfthick,
				  x+halfthick, y+halfthick,
				  w-roundthick, h-roundthick);
		    break;
		default:
		    break;
		}
	    }
#endif
	}
	else {  /* draw an etched line in or out */
	    if (linetype == XmtLayoutFrameShadowIn)
		linetype = XmtLayoutFrameEtchedIn;
	    if (linetype == XmtLayoutFrameShadowOut)
		linetype = XmtLayoutFrameEtchedOut;
	    if (linetype == XmtLayoutFrameEtchedIn) {
		tmpgc = botgc;
		botgc = topgc;
		topgc = tmpgc;
	    }
	    if (horizontal) {
		XFillRectangle(dpy, win, topgc, x, y, w, halfthick);
		XFillRectangle(dpy, win, botgc,
			       x, y+halfthick, w, thickness-halfthick);
	    }
	    else {
		XFillRectangle(dpy, win, topgc, x, y, halfthick, h);
		XFillRectangle(dpy, win, botgc,
			       x+halfthick, y, thickness-halfthick, h);
	    }
	}
    }
}


#if NeedFunctionPrototypes
static void DrawFrameAndCaption(XmtLayoutWidget lw, Widget c, Region region)
#else
static void DrawFrameAndCaption(lw, c, region)
XmtLayoutWidget lw;
Widget c;
Region region;
#endif
{
    XmtLayoutConstraintsPart *cc;
    int x, y, w, h;      /* coordinates of widget */
    int fx = 0, fy = 0, fw = 0, fh = 0;  /* coordinates of outside of frame */
    int cx = 0, cy = 0, cw = 0, ch = 0;  /* coordinates of caption bounding box*/
    XmtLayoutFrameType frame_type;
    XmtLayoutFrameLineType frame_line_type;
    int frame_thickness;
    int frame_margin;
    XmtLayoutFramePosition frame_position;
    int frame_total;
    int caption_total_width = 0, caption_total_height = 0;
    Boolean frame_redrawn;

    /*
     * For the specified child widget:
     * compute the position of the frame and caption.  If the frame intersects
     * the region, redraw it.  If the caption intersects the region, or
     * if the frame intersects the caption and the frame was redrawn, then
     * redraw the caption
     */
    
    cc = LayoutConstraintsRec(c);
    
    /* get the frame args.  If debug_layout is set, force fixed values */
    frame_type = cc->frame_type;
    frame_line_type = cc->frame_line_type;
    frame_margin = cc->frame_margin;
    frame_thickness = cc->frame_thickness;
    frame_position = cc->frame_position;
    if (lw->layout.debug_layout) {
	frame_type = XmtLayoutFrameBox;
	frame_line_type = XmtLayoutFrameSingleLine;
	frame_margin = 5;
	frame_thickness = 1;
	frame_position = XmtLayoutFrameOutside;
    }
    
    frame_total = frame_margin + frame_thickness;
    if (cc->caption) {
	caption_total_width = cc->caption_width + cc->caption_margin;
	caption_total_height = cc->caption_height + cc->caption_margin;
    }

    /* if no frame or caption, don't do anything */
    if (cc->caption == NULL && frame_type == XmtLayoutFrameNone)
	return;
    
    /* widget coordinates */
    x = c->core.x; y = c->core.y; w = c->core.width; h = c->core.height;
    
    /* compute coordinates of outside of frame */
    if (frame_type != XmtLayoutFrameNone) {
	fx = x - frame_total;
	fy = y - frame_total;
	fw = w + 2*frame_total;
	fh = h + 2*frame_total;
	if (cc->caption &&
	    (frame_position == XmtLayoutFrameOutside)) {
	    switch (cc->caption_position) {
	    case XmtLayoutTop:
		fy -= caption_total_height;
		/* note no break statement */
	    case XmtLayoutBottom:
		fh += caption_total_height;
		break;
	    case XmtLayoutLeft:
		fx -= caption_total_width;
		/* note no break statement */
	    case XmtLayoutRight:
		fw += caption_total_width;
		break;
	    }
	}
    }

    /* if the frame is just a line rather than a box, adjust coords */
    /* (x,y) will be top left of line, now, not outside of frame */
    switch(frame_type) {
    case XmtLayoutFrameBottom:
	fy += fh - frame_thickness;
	/* note: no break statement */
    case XmtLayoutFrameTop:
	fx += frame_total;
	fw -= 2*frame_total;
	break;
    case XmtLayoutFrameRight:
	fx += fw - frame_thickness;
	/* note: no break */
    case XmtLayoutFrameLeft:
	fy += frame_total;
	fh -= 2*frame_total;
	break;
    default: break;
    }
    
    /* compute coordinates of caption bounding box */
    if (cc->caption) {
	/* width and height are given */
	cw = cc->caption_width;
	ch = cc->caption_height;
	
	/* compute one dimension based on position */
	switch (cc->caption_position) {
	case XmtLayoutTop:
	    cy = y - (cc->caption_margin + ch);
	    break;
	case XmtLayoutBottom:
	    cy = y + h + cc->caption_margin;
	    break;
	case XmtLayoutLeft:
	    cx = x - (cc->caption_margin + cw);
	    break;
	case XmtLayoutRight:
	    cx = x + w + cc->caption_margin;
	    break;
	}
	
	/* compute the other dimension based on justification */
	if ((cc->caption_position == XmtLayoutTop) ||
	    (cc->caption_position == XmtLayoutBottom)) {
	    switch (cc->caption_justification) {
	    case XmtLayoutFilled:
	    case XmtLayoutFlushLeft:  cx = x; break;
	    case XmtLayoutCentered:   cx = x + (w - cw)/2; break;
	    case XmtLayoutFlushRight: cx = x + w - cw; break;
	    }
	}
	else if ((cc->caption_position == XmtLayoutLeft) ||
		 (cc->caption_position == XmtLayoutRight)) {
	    switch (cc->caption_justification) {
	    case XmtLayoutFilled:
	    case XmtLayoutFlushLeft:  cy = y; break;
	    case XmtLayoutCentered:   cy = y + (h - ch)/2; break;
	    case XmtLayoutFlushRight: cy = y + h - ch; break;
	    }
	}
	
	/* if there is a frame inside the caption, adjust position */
	if ((frame_type != XmtLayoutFrameNone) &&
	    (frame_position == XmtLayoutFrameInside)) {

	    /* adjustments for full frames */
	    if (frame_type == XmtLayoutFrameBox) {
		switch (cc->caption_position) {
		case XmtLayoutTop:     cy -= frame_total; break;
		case XmtLayoutBottom:  cy += frame_total; break;
		case XmtLayoutLeft:    cx -= frame_total; break;
		case XmtLayoutRight:   cx += frame_total; break;
		}
		
		if ((cc->caption_position == XmtLayoutTop) ||
		    (cc->caption_position == XmtLayoutBottom)) {
		    switch (cc->caption_justification) {
		    case XmtLayoutFilled:
		    case XmtLayoutFlushLeft:  cx -= frame_total; break;
		    case XmtLayoutCentered:   break;
		    case XmtLayoutFlushRight: cx += frame_total; break;
		    }
		}
		else if ((cc->caption_position == XmtLayoutLeft) ||
			 (cc->caption_position == XmtLayoutRight)) {
		    switch (cc->caption_justification) {
		    case XmtLayoutFilled:
		    case XmtLayoutFlushLeft:  cy -= frame_total; break;
		    case XmtLayoutCentered:   break;
		    case XmtLayoutFlushRight: cy += frame_total; break;
		    }
		}
	    }
	    else if (frame_type == XmtLayoutFrameLeft) {
		if (cc->caption_position == XmtLayoutLeft)
		    cx -= frame_total;
	    }
	    else if (frame_type == XmtLayoutFrameRight) {
		if (cc->caption_position == XmtLayoutRight)
		    cx += frame_total;
	    }
	    else if (frame_type == XmtLayoutFrameTop) {
		if (cc->caption_position == XmtLayoutTop)
		    cy -= frame_total;
	    }
	    else if (frame_type == XmtLayoutFrameBottom) {
		if (cc->caption_position == XmtLayoutBottom)
		    cy += frame_total;
	    }
	}
    }
    
    /* if there is a frame, and it is in the exposed region, redraw it */
    frame_redrawn = False;
    if ((frame_type != XmtLayoutFrameNone) &&
	(!region || (XRectInRegion(region, fx, fy, fw, fh) != RectangleOut))) {
	frame_redrawn = True;
	DrawFrame(lw, frame_type, frame_line_type, frame_thickness,
		  fx, fy, fw, fh);
    }
    
    /*
     * if there is a caption, and it is in the exposed region,
     * or if it intersects the frame and the frame was redrawn
     * then redraw it.
     */
    if (cc->caption &&
	(!region ||
	 (XRectInRegion(region, cx, cy, cw, ch) != RectangleOut) ||
	 (frame_redrawn && (frame_position == XmtLayoutFrameThrough)))) {
	/* X protocol ignores stippling with DrawImage functions,
	 * so we simulate XmStringDrawImage by clearing first
	 */
	XClearArea(XtDisplay((Widget)lw), XtWindow((Widget)lw),
		   cx, cy, cw, ch, False);
	XmStringDraw(XtDisplay((Widget)lw), XtWindow((Widget)lw),
#if XmVersion < 2000
                     lw->layout.font_list,
#else /* Motif 2.0 or later */
		     lw->layout.render_table,
#endif
                     cc->caption,
		     (lw->core.ancestor_sensitive &&
		      lw->core.sensitive &&
		      cc->sensitive)
		     ?lw->layout.gc
		     :lw->layout.grey_gc,
		     cx, cy, cw,
		     cc->caption_alignment,
		     XmSTRING_DIRECTION_L_TO_R, NULL);
    }
}


#if NeedFunctionPrototypes
static void DrawFramesAndCaptions(XmtLayoutWidget lw, Region region)
#else
static void DrawFramesAndCaptions(lw, region)
XmtLayoutWidget lw;
Region region;
#endif
{
    int i;
    Widget c;

    /*
     * For each child widget:
     * compute the position of the frame and caption.  If the frame intersects
     * the region, redraw it.  If the caption intersects the region, or
     * if the frame intersects the caption and the frame was redrawn, then
     * redraw the caption
     */

    ForEachChild(lw, c)
	if (XtIsManaged(c))
	    DrawFrameAndCaption(lw, c, region);
}

#if NeedFunctionPrototypes
static void Redisplay(Widget widget, XEvent *event, Region region)
#else
static void Redisplay(widget, event, region)
Widget widget;
XEvent *event;
Region region;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) widget;
    
    /* invoke the Redisplay method of the topmost XmtLayoutBox */
    _XmtRedisplayGadget(lw->layout.toplevel, event, region);

    /* redisplay any Xm Gadgets */
    _XmtRedisplayGadgets(widget, event, region);

    /* go draw all the XmtLayout frames and captions */
    DrawFramesAndCaptions(lw, region);
}

#if NeedFunctionPrototypes
static void Resize(Widget w)
#else
static void Resize(w)
Widget w;
#endif
{
    if (XtIsRealized(w)) XClearWindow(XtDisplay(w), XtWindow(w));
    _XmtLayoutChildren((XmtLayoutWidget) w, False);
}

#if NeedFunctionPrototypes
static void ChangeManaged(Widget w)
#else
static void ChangeManaged(w)
Widget w;
#endif
{
    _XmtLayoutChildren((XmtLayoutWidget) w, True);
}




/*
 * This GeometryManager() method is invoked in two distinct cases.
 * 1) When a child widget calls XtMakeGeometryRequest() to change something.
 *    In this case, we deny the request if XmtNlayoutAllowResize if False,
 *    or if the child is trying to change its position, rather than its size.
 *    Otherwise we grant the request.  When we grant this request, we call
 *    _XmtLayoutChildren() to configure the widget (and any of its siblings
 *    that change as a result, and return XtGeometryDone.  The algorithm in
 *    this case is modified from the Xaw Form widget.
 * 2) When the user sets a resource on a child that directly or indirectly
 *    changes a geometry field of the widget.  After a SetValues() request,
 *    the Intrinsics automatically call this method if any geometry fields
 *    have changed.  In this case, the ConstraintSetValues() method will
 *    always be called before this method is.  We have to handle this case
 *    specially because of the constraint resources.  If the XmtNlayoutIn
 *    constraint is set, for example, then the CSV method will call
 *    _XmtLayoutChildren() to rearrange things.  This means that the position
 *    of the child widget will change, and this change will be legal.  If
 *    we didn't have a special case, the change would be rejected.  Also,
 *    since _XmtLayoutChildren() has already been called, we know that we
 *    don't have to call it again here.  We do have to call XtConfigureWidget()
 *    however--when ConstraintSetValues() calls _XmtLayoutChildren(),
 *    the geometry fields of the widget are set at their new values, and so
 *    when _XmtLayoutChildren() tries to configure the widget to those
 *    new values (in the resize() method of LayoutBox.c), the Intrinsics
 *    will think that it doesn't have to do anything.
 *    Note the use of the geometry_okay flag for communication between
 *    the CSV method and this method.
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
					XtWidgetGeometry *reply)
#else
static XtGeometryResult GeometryManager(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(w);
    Dimension width, height;

    /* this is case 2 described above */
    if (lw->layout.geometry_okay) {
	XtConfigureWidget(w,
	      (request->request_mode&CWX)?request->x:w->core.x,
	      (request->request_mode&CWY)?request->y:w->core.y,
	      (request->request_mode&CWWidth)?request->width:w->core.width,
	      (request->request_mode&CWHeight)?request->height:w->core.height,
	      (request->request_mode&CWBorderWidth)?request->border_width
			  :w->core.border_width);
	
	/* make sure we don't do this again without cause */
	lw->layout.geometry_okay = False;

	/*
	 * Tell the Intrinsics we've done the update for it.  Note
	 * that in this case, when the geometry_okay flag is set,
	 * the Layout widget has already been re-laid out.  However,
	 * this particular child may not have had its window
	 * coordinates updated, as explained in a comment in
	 * ConstraintSetValues().  Thus we do have to do the update here.
	 */
	return XtGeometryDone;
    }

    /* Otherwise, we handle case 1 described above */
    if (!LayoutConstraints(w).allow_resize) return XtGeometryNo;

    if (request->request_mode & ~(XtCWQueryOnly | CWWidth | CWHeight))
	return XtGeometryNo;

    if (request->request_mode & CWWidth)
	width = request->width;
    else
	width = w->core.width;

    if (request->request_mode & CWHeight)
	height = request->height;
    else
	height = w->core.height;

    if (width == w->core.width && height == w->core.height)
	return XtGeometryNo;

    if (!(request->request_mode & XtCWQueryOnly)) {
	XtResizeWidget(w, width, height, w->core.border_width);
	_XmtLayoutChildren((XmtLayoutWidget)XtParent(w), True);
	return XtGeometryDone;
    }
    else
	return XtGeometryYes;
}

/* stolen from Xaw Form widget.  Hope it works right */
#if NeedFunctionPrototypes
static XtGeometryResult QueryGeometry(Widget widget, 
				      XtWidgetGeometry *request,
				      XtWidgetGeometry *reply)
#else
static XtGeometryResult QueryGeometry(widget, request, reply)
Widget widget;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
#endif
{
    XmtLayoutWidget w = (XmtLayoutWidget)widget;
    
    reply->width = LayoutConstraints(w->layout.toplevel).pref_w +
	              2 * w->bulletin_board.margin_width;
    reply->height = LayoutConstraints(w->layout.toplevel).pref_h +
	              2 * w->bulletin_board.margin_height;
    reply->request_mode = CWWidth | CWHeight;
    if (((request->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& (request->width == reply->width)
	&& (request->height == reply->height))
	return XtGeometryYes;
    else if ((reply->width == w->core.width) && (reply->height == w->core.height))
	return XtGeometryNo;
    else
	return XtGeometryAlmost;
}

#if NeedFunctionPrototypes
static void InsertChild(Widget w)
#else
static void InsertChild(w)
Widget w;
#endif
{
    /*
     * This is another Motif workaround.
     * The BulletinBoard and Manager class InsertChild procedures ignore
     * non-rectobj children, and never insert them in the children array.
     * The workaround is to call the bulletin board method if this is a
     * RectObj, and call the composite method if this is a LayoutGadget,
     * and do nothing otherwise.  Same for DeleteChild.
     */
    if (XtIsRectObj(w))
	(*xmBulletinBoardClassRec.composite_class.insert_child)(w);
    else if (XmtIsLayoutGadget(w))
	(*compositeClassRec.composite_class.insert_child)(w);
}


#if NeedFunctionPrototypes
static void DeleteChild(Widget w)
#else
static void DeleteChild(w)
Widget w;
#endif
{
    /*
     * this is a hack.  See InsertChild()
     */
    if (XtIsRectObj(w))
	(*xmBulletinBoardClassRec.composite_class.delete_child)(w);
    else if (XmtIsLayoutGadget(w))
	(*compositeClassRec.composite_class.delete_child)(w);
}


#if NeedFunctionPrototypes
static void RepositionContents(Widget container)
#else
static void RepositionContents(container)
Widget container;
#endif
{
    Widget c;                     /* child */
    XmtLayoutConstraintsPart *cc; /* child constraints */
    int i;

    c = LayoutConstraints(container).first_child;
    i = 0;
    while(c) {
	cc = LayoutConstraintsRec(c);
	cc->position = i++;
	c = cc->before;
    }
}

#if NeedFunctionPrototypes
static void PlaceObjectAfter(Widget w, Widget after)
#else
static void PlaceObjectAfter(w, after)
Widget w;
Widget after;
#endif
{
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(w);
    XmtLayoutConstraintsPart *p = LayoutConstraintsRec(after);
    XmtLayoutConstraintsPart *n;
    Widget container = p->in ? p->in :
	((XmtLayoutWidget)XtParent(w))->layout.toplevel;

    c->in = p->in;
    c->after = after;
    c->before = p->before;
    p->before = w;
    if (c->before != NULL) {
	n = LayoutConstraintsRec(c->before);
	n->after = w;
    }
    RepositionContents(container);
}

#if NeedFunctionPrototypes
static void PlaceObjectBefore(Widget w, Widget before)
#else
static void PlaceObjectBefore(w, before)
Widget w;
Widget before;
#endif
{
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(w);
    XmtLayoutConstraintsPart *n = LayoutConstraintsRec(before);
    XmtLayoutConstraintsPart *p;
    Widget container = n->in ? n->in :
	((XmtLayoutWidget)XtParent(w))->layout.toplevel;

    c->in = n->in;
    c->before = before;
    c->after = n->after;
    n->after = w;

    if (c->after == NULL)
	LayoutConstraints(container).first_child = w;
    else {
	p = LayoutConstraintsRec(c->after);
	p->before = w;
    }
    RepositionContents(container);
}

#if NeedFunctionPrototypes
static void PlaceObjectIn(Widget w, Widget in, int pos)
#else
static void PlaceObjectIn(w, in, pos)
Widget w;
Widget in;
int pos;
#endif
{
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(w);
    XmtLayoutConstraintsPart *cc;  /* container */
    Widget n, p;  /* next, previous */
    int j;
    Widget container = in ? in :
	((XmtLayoutWidget)XtParent(w))->layout.toplevel;
    
    /*
     * if in is NULL, we leave the layoutIn constraint NULL as well,
     * instead of using the pointer to the actual internal container.
     * The value NULL is more meaningful to a programmer who might
     * query this resource.
     */
    c->in = in;

    /* now go find the position it belongs at */
    cc = LayoutConstraintsRec(container);
    for(p = NULL, n = cc->first_child, j = 0;
	n && (j != pos);
	p = n, n = LayoutConstraints(n).before, j++);
    c->position = j;  /* convert a -1 position to actual position */
    c->after = p;
    c->before = n;
    if (p)
	LayoutConstraints(p).before = w;
    else
	cc->first_child = w;
    if (n)
	LayoutConstraints(n).after = w;
    RepositionContents(container);
}

/*
 * This procedure is called mostly from the SetValues method, and its
 * two arguments are the "current" and "set" arguments to that method.
 * We need to use the copy widget, because it has its doubly-linked
 * list intact, and we can extract the widget we need.  But we need
 * the actual widget as well, so that we can detect whether that
 * widget actually occurs on the list.
 */
#if NeedFunctionPrototypes
static void UnlinkObject(Widget copy, Widget actual)
#else
static void UnlinkObject(copy, actual)
Widget copy, actual;
#endif
{
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(copy);
    Widget container = c->in ? c->in :
	((XmtLayoutWidget)XtParent(copy))->layout.toplevel;

    /*
     * Note that this procedure may sometimes be called more than once
     * on a particular child.  Thus the test in the else clause below
     * is necessary.  If the child has already been unlinked, then
     * this test will fail, and nothing will be done.
     */
    if (c->before)
	LayoutConstraints(c->before).after = c->after;
    if (c->after)
	LayoutConstraints(c->after).before = c->before;
    else if (LayoutConstraints(container).first_child == actual)
	LayoutConstraints(container).first_child = c->before;

    RepositionContents(container);
    c->before = c->after = c->in = NULL;
}

#if NeedFunctionPrototypes
static Boolean set_parsed_info(XmtLayoutWidget lw, Widget child)
#else
static Boolean set_parsed_info(lw, child)
XmtLayoutWidget lw;
Widget child;
#endif
{
    XmtLayoutInfo *info, *last;
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(child);
    XmtLayoutConstraintsPart *mark;

    /* see if the child's name appears in the list */
    for(last = NULL, info = lw->layout.widget_info;
	info; last = info, info = info->next)
	if (info->name == child->core.xrm_name) break;
    
    /*
     * If no info was found, and if the child
     * is a scrolled window, and its name ends in "SW"
     * see if we have info for the grandchild's name without the SW.
     * This is a special case to make scrolled lists and scrolled texts
     * easier to use in a layout widget.
     */
    if (!info && XmIsScrolledWindow(child)) {
	String name = XtName(child);
	int len = strlen(name);
	XrmQuark grandchild_name;

	if ((name[len-2] == 'S') && (name[len-1] == 'W')) {
	    name = XtNewString(name);
	    name[len-2] = '\0';
	    grandchild_name = XrmStringToQuark(name);
	    XtFree(name);

	    for(last = NULL, info = lw->layout.widget_info;
		info; last = info, info = info->next)
		if (info->name == grandchild_name) break;
	}
    }

    /* if we found some info unlink it, otherwise quit */
    if (info) {
	if (last) last->next = info->next;
	else lw->layout.widget_info = info->next;
    }
    else return False;

    /* override any constraints with info from the layout string */

#define Merge(field)\
    if (info->dummy_constraints.field) c->field = info->constraints.field

    Merge(width);
    Merge(height);
    Merge(stretchability);
    Merge(shrinkability);
    Merge(allow_resize);
    Merge(sensitive);
    Merge(justification);
    Merge(margin_width);
    Merge(margin_height);
    Merge(frame_type);
    Merge(frame_line_type);
    Merge(frame_position);
    Merge(frame_margin);
    Merge(frame_thickness);
    Merge(caption_position);
    Merge(caption_justification);
    Merge(caption_alignment);
    Merge(caption_margin);
    Merge(caption);

#undef Merge
    
    /* this private flag is a special case */
    c->dont_copy_caption = info->constraints.dont_copy_caption;

    /*
     * Set after or before or in to the same value as
     * the marker widget we created in the parser.  We don't need to fully
     * attach this widget, just set a position resource so that
     * ConstraintInitialize can attach it in the right place.
     * Unlink and destroy the marker widget.
     */
    mark = LayoutConstraintsRec(info->constraints.after);
    c->in = mark->in;
    c->position = mark->position;
    /* destroy the marker object, and remove it from the list */
    XtDestroyWidget(info->constraints.after);

    XtFree((char *) info);
    return True;
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void ConstraintInitialize(Widget request, Widget init,
				 ArgList arglist, Cardinal *num_args)
#else
static void ConstraintInitialize(request, init, arglist, num_args)
Widget request;
Widget init;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(init);
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(init);

    /*
     * we don't use constraints for non-rectobjs.  See comment
     * in ConstraintSetValues.
     */
    if (!XtIsWidget(init) &&
	!XmIsGadget(init) &&
	!XmtIsLayoutGadget(init))
	return;

    /*
     * make a note if this is a widget or gadget.
     */
    if (XtIsWidget(init) || XmIsGadget(init))
	c->type = XmtLayoutChild;

    c->dont_copy_caption = False;

    /*
     * go check if there is constraint info about this widget saved away.
     * If so, set it, overriding any resources already set.
     * Don't bother checking if this widget was created by the parser.
     */
    if (!lw->layout.in_parser) set_parsed_info(lw, init);
    
    /*
     * if stretchability and shrinkability are unset,
     * set the default depending on type
     */
    if (c->stretchability == (Dimension) -1) {
	if ((c->type == XmtLayoutChild) ||
	    (c->type == XmtLayoutSpace) ||
	    (c->type == XmtLayoutRow) ||
	    (c->type == XmtLayoutCol))
	    c->stretchability = XmtLAYOUT_DEFAULT_STRETCHABILITY;
	else
	    c->stretchability = 0;
    }
    if (c->shrinkability == (Dimension) -1) {
	if ((c->type == XmtLayoutChild) ||
	    (c->type == XmtLayoutSpace) ||
	    (c->type == XmtLayoutRow) ||
	    (c->type == XmtLayoutCol))
	    c->shrinkability = XmtLAYOUT_DEFAULT_SHRINKABILITY;
	else
	    c->shrinkability = 0;
    }

    /*
     * if margin width or margin height are unset, set default
     * depending on type.
     */
    if (c->margin_width == 255) {
	if ((c->type == XmtLayoutRow) || (c->type == XmtLayoutCol))
	    c->margin_width = 0;
	else
	    c->margin_width = lw->layout.default_spacing/2;
    }
    if (c->margin_height == 255) {
	if ((c->type == XmtLayoutRow) || (c->type == XmtLayoutCol))
	    c->margin_height = 0;
	else
	    c->margin_height = lw->layout.default_spacing/2;
    }
	
    /* if there is a caption, copy it and figure out how big it is */
    if (c->caption) {
	if (!c->dont_copy_caption)
	    c->caption = XmStringCopy(c->caption);
#if XmVersion < 2000
	XmStringExtent(lw->layout.font_list, c->caption,
		       &c->caption_width, &c->caption_height);
#else
	XmStringExtent(lw->layout.render_table, c->caption,
		       &c->caption_width, &c->caption_height);
#endif
    }

    /*
     * Now figure out where this object will be laid out.
     * This can be specified in a number of ways:
     *   after,
     *   before, or
     *   in, optionally combined with position.
     * The fields are examined in the order listed, and the first one
     * specified is used.  Whichever fields are specified, all
     * except position are updated to reflect the actual layout.
     * If none are specified, then the widget is inserted in the toplevel
     * row or column.  position defaults to -1, which inserts
     * at the end of the row or column.
     * Note that if the toplevel box isn't created yet, then we must
     * be creating it now, and we shouldn't place it anywhere.
     */
    if (lw->layout.toplevel != 0) {
	if (c->after)
	    PlaceObjectAfter(init, c->after);
	else if (c->before)
	    PlaceObjectBefore(init, c->before);
	else {
	    PlaceObjectIn(init, c->in, c->position);
	}
    }

    c->first_child = NULL;
}


#if NeedFunctionPrototypes
static void ConstraintDestroy(Widget w)
#else
static void ConstraintDestroy(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(w);
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(w);

    if (!XtIsWidget(w) && !XmIsGadget(w) && !XmtIsLayoutGadget(w))
	return;

    /* if there is a caption, free it */
    if (c->caption)
	XmStringFree(c->caption);

    /*
     * if the whole layout is being destroyed, don't
     * bother messing with the linked lists.  If this is a container,
     * we don't need to mess with the kids because they will be destroyed.
     */
    if (lw->core.being_destroyed)
	return;

    /* unlink the item */
    UnlinkObject(w, w);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static Boolean ConstraintSetValues(Widget current, Widget request, Widget set,
				   ArgList arglist, Cardinal *num_args)
#else
static Boolean ConstraintSetValues(current, request, set, arglist, num_args)
Widget current;
Widget request;
Widget set;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(set);
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(current);
    XmtLayoutConstraintsPart *s = LayoutConstraintsRec(set);
    Boolean relayout = False;
    Boolean redisplay = False;

    /*
     * ** Another gross Motif hack.
     * Motif creates screwy non-rectobj cache objects for every Gadget.
     * These objects have NULL constraint fields but are passed to
     * this function anyway.  They are subclasses of xmExtObjectClass,
     * so we could test for them and rule them out.  But since none of
     * our constraints make sense for any non-rect object, we'll just
     * test that it is a subclass of RectObj.  We'll insert the same test
     * in the other constraint methods for efficiency
     */
	
    if (!XtIsWidget(set) && !XmIsGadget(set) && !XmtIsLayoutGadget(set))
	return False;

#define Changed(field) (s->field != c->field)

    if (Changed(justification) || Changed(margin_width) ||
	Changed(margin_height) || Changed(frame_type) ||
	Changed(frame_position) || Changed(frame_margin) ||
	Changed(caption_position) || Changed(caption_margin) ||
	Changed(width) || Changed(height) ||
	Changed(stretchability) || Changed(shrinkability))
	relayout = True;

    if (Changed(caption_justification)) redisplay = True;

    if (Changed(sensitive) || Changed(frame_line_type) ||
	Changed(caption_alignment))
	if (XtIsRealized((Widget)lw))
	    DrawFrameAndCaption(lw, set, NULL);

    if (Changed(caption)) {
	if (c->caption) XmStringFree(c->caption);
	if (s->caption) {
	    s->caption = XmStringCopy(s->caption);
#if XmVersion < 2000
	    XmStringExtent(lw->layout.font_list, s->caption,
			   &s->caption_width, &s->caption_height);
#else
	    XmStringExtent(lw->layout.render_table, s->caption,
			   &s->caption_width, &s->caption_height);
#endif
	}
	relayout = True;
    }

    if (set != ((XmtLayoutWidget)XtParent(set))->layout.toplevel) {
	if (Changed(after)) {
	    if (!s->after || (XtParent(s->after) != (Widget)lw)) {
		XmtWarningMsg("XmtLayout", "badAfter",
			      "XmtNlayoutAfter constraint of child `%s'\n\tmust be a sibling of the child.",
			      XtName((Widget)set));
		s->after = c->after;
	    }
	    else {
		Widget after = s->after;
		UnlinkObject(current, set);
		PlaceObjectAfter(set, after);
		relayout = True;
	    }
	}
	else if (Changed(before)) {
	    if (!s->before || (XtParent(s->before) != (Widget)lw)) {
		XmtWarningMsg("XmtLayout", "badBefore",
			      "XmtNlayoutBefore constraint of child `%s'\n\tmust be a sibling of the child.",
			      XtName((Widget)set));
		s->before = c->before;
	    }
	    else {
		Widget before = s->before;
		UnlinkObject(current, set);
		PlaceObjectBefore(set, before);
		relayout = True;
	    }
	}
	else if (Changed(in) || Changed(position)) {
	    if (Changed(in) && s->in &&
		((XtParent(s->in) != (Widget)lw) ||
		 (!XtIsSubclass(s->in, xmtLayoutBoxGadgetClass)))) {
		XmtWarningMsg("XmtLayout", "badIn",
			      "XmtNlayoutIn constraint of child `%s'\n\tmust be an XmtLayoutBox sibling of the child, or NULL.",
			      XtName((Widget)set));
		s->in = c->in;
	    }
	    else {
		Widget in = s->in;
		int position = s->position;
		UnlinkObject(current, set);
		PlaceObjectIn(set, in, position);
		relayout = True;
	    }
	}
    }

    /*
     * It doesn't make sense to change the position of children of a layout
     * widget by setting XtNx or XtNy.  So, if these have been changed,
     * warn, and undo the changes.  Note that it is legal to set the
     * width or height of a child--if these fields have changed, the
     * Intrinsics will automatically call the GeometryManager field to
     * make the change, and the GeometryManager will do the relayout,
     * so we don't need to handle this case at all.
     */
    if ((set->core.x != current->core.x) || (set->core.y != current->core.y)) {
	set->core.x = current->core.x;
	set->core.y = current->core.y;
	XmtWarningMsg("XmtLayout", "cantsetpos",
		      "Widget %s: can't set XtNx or XtNy resources of a child of\n\tan XmtLayout widget.  Changes ignored.", XtName((Widget)set));
    }

#undef Changed

    /*
     * If any of these changes require a relayout, do that now.
     *
     * If the changes or the relayout changed any of the widget's
     * geometry fields, then the Intrinsics will invoke the
     * GeometryManager() method to verify that those changes are okay.
     * But we've just called _XmtLayoutChildren() so we know that they're
     * okay.  So we set a flag to tell GeometryManager() to approve
     * the changes, even though they may include position changes
     * which are normally rejected.
     * Notice, however, that when we call _XmtLayoutChildren() here,
     * the core geometry fields are at their new values, and thus, when
     * the relayout procedure ends up calling XtConfigureWidget()
     * (actually XmtConfigureObject, in LayoutBox.c) on this child,
     * the Intrinsics may find that the new geometry matches the current
     * geometry, and thus will not update the actual window fields.
     * Therefore, we've got to be sure that the geometry manager
     * takes this into account.
     * See comment at GeometryManager() for more information.
     */
    if (relayout) {
	_XmtLayoutChildren(lw, True);

	if ((set->core.x != current->core.x) ||
	    (set->core.y != current->core.y) ||
	    (set->core.width != current->core.width) ||
	    (set->core.height != current->core.height))
	    lw->layout.geometry_okay = True;
    }

    /*
     * Returning True from this method will only redraw the child, not
     * the entire Layout widget, as happens when SetValues() returns True.
     * So if either relayout or redisplay is True, then we'll need to force
     * the redraw ourselves.  We do this by using XClearArea() to generate
     * an Expose event.  The relayout process may generate other Expose
     * events if gadgets are moved around, but these events will be combined
     * into a single call to the Layout Redisplay() method.  The reason that
     * it is important to do this is that the Layout provides captions and
     * frames for its children, and these have to be redrawn when the
     * children move or change size.  In particular, if only a widget child
     * is moved, then the relayout call won't generate any expose events,
     * and without the XClearArea() below, the captions and frames would
     * not be redrawn correctly.
     */

    if ((relayout || redisplay) && XtIsRealized((Widget)lw))
	XClearArea(XtDisplay((Widget) lw), XtWindow((Widget) lw),
		   lw->core.x, lw->core.y, lw->core.width, lw->core.height,
		   True);

    return False;
}


#if NeedFunctionPrototypes
void XmtLayoutDisableLayout(Widget w)
#else
void XmtLayoutDisableLayout(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) w;

    XmtAssertWidgetClass(w, xmtLayoutWidgetClass, "XmtLayoutDisable");
    lw->layout.layout_disabled++;
}

#if NeedFunctionPrototypes
void XmtLayoutEnableLayout(Widget w)
#else
void XmtLayoutEnableLayout(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) w;

    XmtAssertWidgetClass(w, xmtLayoutWidgetClass, "XmtEnableLayout");
    if (lw->layout.layout_disabled > 0) lw->layout.layout_disabled--;
    if (lw->layout.needs_layout) _XmtLayoutChildren(lw, True);
}

	
#if NeedFunctionPrototypes
int XmtLayoutConvertSizeToPixels(Widget w, double size,XmtLayoutUnitType units)
#else
int XmtLayoutConvertSizeToPixels(w, size, units)
Widget w;
double size;
XmtLayoutUnitType units;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget) w;

    XmtAssertWidgetClass(w, xmtLayoutWidgetClass,
			 "XmtLayoutConvertSizeToPixels");

    /* convert to pixels */
    /* note intentional missing break statements */
    switch(units) {
    case XmtLayoutPoints:
	size /= 72;                  /* 1 point = 1/72 inch */
    case XmtLayoutInches:
	size *= 25.4;                /* 1 inch = 25.4 mm */
    case XmtLayoutMillimeters:
	size *= lw->layout.pixpermm; /* pixels per mm depends on screen res */
	break;
    case XmtLayoutEns:
	size /= 2;                   /* 1 en = 1/2 em */
    case XmtLayoutEms:
	size *= lw->layout.pixperem; /* pixels per em depends on font size */
	break;
    }

    /* round to the nearest pixel */
    return (int) size;
}

#if NeedFunctionPrototypes
Widget XmtCreateLayout(Widget parent, String name,
		       ArgList args, Cardinal n)
#else
Widget XmtCreateLayout(parent, name, args, n)
Widget parent;
String name;
ArgList args;
Cardinal n;
#endif
{
    return XtCreateWidget(name, xmtLayoutWidgetClass, parent, args, n);
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutDialog(Widget parent, String name,
			     ArgList al, Cardinal ac)
#else
Widget XmtCreateLayoutDialog(parent, name, al, ac)
Widget parent;
String name;
ArgList al;
Cardinal ac;
#endif
{
    Widget shell, w;
    String shell_name;

    shell_name = XtMalloc(strlen(name) + 7);
    (void)sprintf(shell_name, "%s_shell", name);

    shell = XmCreateDialogShell(parent, shell_name, al, ac);
    XtFree(shell_name);
    XtVaSetValues(shell, XmNallowShellResize, True, NULL);

    w = XtCreateWidget(name, xmtLayoutWidgetClass, shell, al, ac);
    XtAddCallback(w, XtNdestroyCallback,
		  (XtCallbackProc)_XmDestroyParentCallback, NULL);
    return w;
}


