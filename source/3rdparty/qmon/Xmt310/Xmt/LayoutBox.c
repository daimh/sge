/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutBox.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutBox.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/XmtP.h>
#include <Xmt/LayoutGP.h>

#define offset(field) XtOffsetOf(XmtLayoutBoxRec, layout_box.field)
static XtResource resources[] = {
{XmtNorientation, XmCOrientation, XmROrientation,
     sizeof(unsigned char), offset(orientation),
     XtRImmediate, (XtPointer)XmHORIZONTAL},	 
{XmtNbackground, XtCBackground, XtRPixel,
     sizeof(Pixel), offset(background),
     XtRImmediate, (XtPointer) -1},
{XmtNequal, XmtCEqual, XtRBoolean,
     sizeof(Boolean), offset(equal),
     XtRImmediate, (XtPointer)False},
{XmtNspaceType, XmtCSpaceType, XmtRXmtLayoutSpaceType,
     sizeof(unsigned char), offset(space_type),
     XtRImmediate, (XtPointer)XmtLayoutSpaceNone},
{XmtNspace, XmtCSpace, XtRDimension,
     sizeof(Dimension), offset(space),
     XtRImmediate, (XtPointer) 0},
{XmtNspaceStretch, XmtCSpaceStretch, XtRDimension,
     sizeof(Dimension), offset(space_stretch),
     XtRImmediate, (XtPointer) 1},
{XmtNitemStretch, XmtCItemStretch, XtRDimension,
     sizeof(Dimension), offset(item_stretch),
     XtRImmediate, (XtPointer) 1},
};
#undef offset

#if NeedFunctionPrototypes
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void Resize(Widget);
static void Redisplay(Widget, XEvent *, Region);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *,
                                      XtWidgetGeometry *);
#else
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Resize();
static void Redisplay();
static XtGeometryResult QueryGeometry();
#endif

#define superclass (&xmtLayoutGadgetClassRec)

externaldef(xmtlayoutboxclassrec) XmtLayoutBoxClassRec xmtLayoutBoxClassRec = {
{   /* rect_class fields  */
    /* superclass	  */	(WidgetClass)superclass,
    /* class_name	  */	"XmtLayoutBox",
    /* widget_size	  */	sizeof(XmtLayoutBoxRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/	NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,		
    /* rect1		  */	NULL,
    /* rect2		  */	NULL,
    /* rect3	  	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* rect4		  */	FALSE,
    /* rect5		  */	FALSE,
    /* rect6		  */ 	FALSE,
    /* rect7		  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* set_values_hook    */	NULL,			
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* rect9		  */	NULL,
    /* version		  */	XtVersion,
    /* callback_offsets   */    NULL,
    /* rect10	          */    NULL,
    /* query_geometry	  */	QueryGeometry,
    /* rect11		  */	NULL,
    /* extension	    */  NULL
  },
  { /* XmtLayoutGadget field */
    /* change_font        */    NULL
  },
  { /* XmtLayoutBox fields */
    /* extension          */	NULL
  }
};

externaldef(xmtlayoutboxgadgetclass)
WidgetClass xmtLayoutBoxGadgetClass = (WidgetClass) &xmtLayoutBoxClassRec;

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Initialize(Widget request, Widget init,
		       ArgList arglist, Cardinal *num_args)
#else
static void Initialize(request, init, arglist, num_args)
Widget request;
Widget init;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget)init;

    if (lb->layout_box.orientation == XmHORIZONTAL)
	Constraint(init, type) = XmtLayoutRow;
    else
	Constraint(init, type) = XmtLayoutCol;
	
    if (lb->layout_box.background != (Pixel)-1) {
	XGCValues gcv;
	gcv.foreground = lb->layout_box.background;
	lb->layout_box.gc = XtGetGC(init, GCForeground, &gcv);
    }
    else
	lb->layout_box.gc = NULL;
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget)w;
    Widget child;

    if (lb->layout_box.gc) XtReleaseGC(w, lb->layout_box.gc);

    /*
     * we destroy anything within this box.  This seems as reasonable
     * a thing to do as moving those widgets elsewhere.  Except if
     * the entire Layout is being destroyed, in which case we can
     * skip this.
     */
    if (!(XtParent(w))->core.being_destroyed)
	ForEachItem(lb, child) XtDestroyWidget(child);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static Boolean SetValues(Widget current, Widget request, Widget set,
			 ArgList arglist, Cardinal *num_args)
#else
static Boolean SetValues(current, request, set, arglist, num_args)
Widget current;
Widget request;
Widget set;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtLayoutBoxGadget cg = (XmtLayoutBoxGadget) current;
    XmtLayoutBoxGadget sg = (XmtLayoutBoxGadget) set;
    Boolean redisplay = False;
    Boolean relayout = False;

    if (sg->layout_box.background != cg->layout_box.background) {
	XGCValues gcv;
	if (sg->layout_box.gc) XtReleaseGC(set, sg->layout_box.gc);
	if (sg->layout_box.background != -1) {
	    gcv.foreground = sg->layout_box.background;
	    sg->layout_box.gc = XtGetGC(set, GCForeground, &gcv);
	}
	else
	    sg->layout_box.gc = NULL;
	redisplay = True;
    }

    if (sg->layout_box.orientation != cg->layout_box.orientation) {
	if ((sg->layout_box.orientation != XmHORIZONTAL) &&
	    (sg->layout_box.orientation != XmVERTICAL))
	    sg->layout_box.orientation = cg->layout_box.orientation;
	else
	    relayout = True;
    }

    if ((sg->layout_box.space != cg->layout_box.space) ||
	(sg->layout_box.space_type != cg->layout_box.space_type) ||
	(sg->layout_box.space_stretch!=cg->layout_box.space_stretch) ||
        (sg->layout_box.item_stretch !=
	 cg->layout_box.item_stretch) ||
	(sg->layout_box.equal != cg->layout_box.equal))
	relayout = True;

    if (relayout) 
	_XmtLayoutChildren((XmtLayoutWidget)XtParent(set), True);

    return redisplay | relayout;
}

	
/*
 * The story with margins and frames and captions:
 * Any child (including boxes) of an XmtLayout widget can have a
 * frame and/or a caption and/or a margin.
 * The XmtLayout widget will draw the frames and captions appropriately.
 * The size of the frame, caption and margin are not included in any child's
 * core x,y,width, and height fields.  This includes boxes.
 * The children should not include the frame, caption or margin when
 * calculating their preferred size.
 * The frame, caption and margin *are* included when doing stretching 
 * and shrinking calculations.
 * The frame, caption and margin are not included when adjusting the
 * size of children in an "equal" row or column.
 * If the programmer specifies a size for the child, then that size
 * does not include any frame or caption or margin.
 * The frame and caption are inside the margin.
 */

    
#if NeedFunctionPrototypes
static void HorizontalMargins(XmtLayoutWidget lw, Widget child,
			      int *total, int *left)
#else
static void HorizontalMargins(lw, child, total, left)
XmtLayoutWidget lw;
Widget child;
int *total;
int *left;
#endif
{
    XmtLayoutConstraintsPart *cc = LayoutConstraintsRec(child);
    int caption = 0;
    Boolean left_caption = False;
    XmtLayoutFrameType frame_type;
    XmtLayoutFramePosition frame_position;
    int frame = 0;

    *total = 2*cc->margin_width;
    *left = cc->margin_width;
    
    if (cc->caption) {
	if ((cc->caption_position == XmtLayoutLeft) ||
	    (cc->caption_position == XmtLayoutRight)) {
	    caption = cc->caption_width + cc->caption_margin;
	    *total += caption;
	    if (cc->caption_position == XmtLayoutLeft) {
		left_caption = True;
		*left += caption;
	    }
	}
    }

    /* if there is no frame, we're done */
    if ((cc->frame_type == XmtLayoutFrameNone) &&
	!lw->layout.debug_layout)
	return;

    /* get the frame args.  If debug_layout is set, force fixed values */
    if (lw->layout.debug_layout) {
	frame_type = XmtLayoutFrameBox;
	frame_position = XmtLayoutFrameOutside;
	frame = 6;
    }
    else {
	frame_type = cc->frame_type;
	frame_position = cc->frame_position;
	frame = cc->frame_margin + cc->frame_thickness;
    }
    
    if (frame_type == XmtLayoutFrameBox) {
	if (!left_caption || (frame_position!=XmtLayoutFrameThrough))
	    *left += frame;
	else if (frame > caption)
	    *left += frame - caption;

    	*total += frame;
	if (!caption || (frame_position != XmtLayoutFrameThrough))
	    *total += frame;
	else if (frame > caption)
	    *total += frame - caption;
    }
    else if (frame_type == XmtLayoutFrameLeft) {
	if (!left_caption || (frame_position != XmtLayoutFrameThrough)) {
	    *left += frame;
	    *total += frame;
	}
	else if (frame > caption) {
	    *left += frame - caption;
	    *total += frame - caption;
	}
    }
    else if (frame_type == XmtLayoutFrameRight) {
	if (!caption || left_caption ||
	    (frame_position != XmtLayoutFrameThrough))
	    *total += frame;
	else if (frame > caption)
	    *total += frame - caption;
    }
}


#if NeedFunctionPrototypes
static void VerticalMargins(XmtLayoutWidget lw, Widget child,
			    int *total, int *top)
#else
static void VerticalMargins(lw, child, total, top)
XmtLayoutWidget lw;
Widget child;
int *total;
int *top;
#endif
{
    XmtLayoutConstraintsPart *cc = LayoutConstraintsRec(child);
    int caption = 0;
    Boolean top_caption = False;
    XmtLayoutFrameType frame_type;
    XmtLayoutFramePosition frame_position;
    int frame = 0;

    *total = 2*cc->margin_height;
    *top = cc->margin_height;
    
    if (cc->caption) {
	if ((cc->caption_position == XmtLayoutTop) ||
	    (cc->caption_position == XmtLayoutBottom)) {
	    caption = cc->caption_height + cc->caption_margin;
	    *total += caption;
	    if (cc->caption_position == XmtLayoutTop) {
		top_caption = True;
		*top += caption;
	    }
	}
    }
    
    /* if there is no frame, we're done */
    if ((cc->frame_type == XmtLayoutFrameNone) &&
	!lw->layout.debug_layout)
	return;

    /* get the frame args.  If debug_layout is set, force fixed values */
    if (lw->layout.debug_layout) {
	frame_type = XmtLayoutFrameBox;
	frame_position = XmtLayoutFrameOutside;
	frame = 6;
    }
    else {
	frame_type = cc->frame_type;
	frame_position = cc->frame_position;
	frame = cc->frame_margin + cc->frame_thickness;
    }

    if (frame_type == XmtLayoutFrameBox) {
	if (!top_caption || (frame_position != XmtLayoutFrameThrough))
	    *top += frame;
	else if (frame > caption)
	    *top += frame - caption;

    	*total += frame;
	if (!caption || (frame_position != XmtLayoutFrameThrough))
	    *total += frame;
	else if (frame > caption)
	    *total += frame - caption;
    }
    else if (frame_type == XmtLayoutFrameTop) {
	if (!top_caption || (frame_position != XmtLayoutFrameThrough)) {
	    *top += frame;
	    *total += frame;
	}
	else if (frame > caption) {
	    *top += frame - caption;
	    *total += frame - caption;
	}
    }
    else if (frame_type == XmtLayoutFrameBottom) {
	if (!caption || top_caption ||
	    (frame_position != XmtLayoutFrameThrough))
	    *total += frame;
	else if (frame > caption)
	    *total += frame - caption;
    }
}

#if NeedFunctionPrototypes
static void HideHomelessChildren(Widget w)
#else
static void HideHomelessChildren(w)
Widget w;
#endif
{
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget)w;
    Widget child;
    XmtLayoutConstraintsPart *cc;  /* child constraints */

    /*
     * If this box is already positioned outside of the window,
     * then it and its descendants are already hidden.
     */
    if ((w->core.x + w->core.width < 0) &&
	(w->core.y + w->core.height < 0)) return;

    /*
     * Otherwise, we've got to move this box and everything it
     * contains (recursively) out of the window.
     */
    XmtSetRectObj(w);
    XtMoveWidget(w, -w->core.width-20, -w->core.height-20);
    XmtUnsetRectObj(w);

    ForEachItem(lb, child) {
	cc = LayoutConstraintsRec(child);

	/*
	 * If the child is itself a container, then recurse on it,
	 * to hide its contents.
	 */
	if ((cc->type == XmtLayoutRow) || (cc->type == XmtLayoutCol))
	    HideHomelessChildren(child);

	/*
	 * hide the child.  We do this differently depending on whether
	 * the child is an XmtLayoutGadget or not.
	 */
	if (cc->type == XmtLayoutChild)
	    XtMoveWidget(child, -child->core.width-20, -child->core.height-20);
	else {
	    XmtSetRectObj(child);
	    XtMoveWidget(child, -child->core.width-20, -child->core.height-20);
	    XmtUnsetRectObj(child);
	}
    }
}

#if NeedFunctionPrototypes
static void Resize(Widget w)
#else
static void Resize(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(w);
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget)w;
    XmtLayoutConstraintsPart *c = LayoutConstraintsRec(w);
    XmtLayoutConstraintsPart *cc;  /* child constraints */
    Boolean row;
    int extra_space;
    int child_num;
    Widget child;
    double factor;    /* "glue set ratio" */
    Boolean stretch = True;
    int major_start, minor_start;    /* instead of x, y */
    int major_length, minor_length;  /* instead of width, height */
    int cx, cy;  /* child x, y */
    int cw, ch;  /* child w, h */
    int *cmajor0, *cminor0;    /* addresses of child major & minor coord */
    int *cmajorlen, *cminorlen; /* addresses of child dimensions */
    int next;
    int margin_width, margin_height, top_margin, left_margin;
    int *major_margin, *minor_margin;
    int space_stretch;
    int total_space, space, interval = 0;
    
    if (!XtIsManaged(w)) return;

    if (lb->layout_box.orientation == XmHORIZONTAL) row = True;
    else row = False;

    if (row) {
	major_start = w->core.x;
	minor_start = w->core.y;
	major_length = w->core.width;
	minor_length = w->core.height;
	cmajor0 = &cx; cminor0 = &cy;
	cmajorlen = &cw; cminorlen = &ch;
	major_margin = &margin_width;
	minor_margin = &margin_height;
    }
    else {
	major_start = w->core.y;
	minor_start = w->core.x;
	major_length = w->core.height;
	minor_length = w->core.width;
	cmajor0 = &cy; cminor0 = &cx;
	cmajorlen = &ch; cminorlen = &cw;
	major_margin = &margin_height;
	minor_margin = &margin_width;
    }

    if (major_length <= 0) major_length = 1;
    if (minor_length <= 0) minor_length = 1;

    if (row)
	extra_space = major_length - c->pref_w;
    else
	extra_space = major_length - c->pref_h;
    
    /*
     * figure out stretch or shrink factor.
     * The total_stretch figure includes the stretchiness of the spaces,
     * if any.
     */
    if (extra_space > 0) {
	if (lb->layout_box.total_stretch == 0) factor = 0.0;
	else factor = extra_space/(double)lb->layout_box.total_stretch;
	stretch = True;
    }
    else if (extra_space < 0) {
	if (lb->layout_box.total_shrink == 0) factor = 0.0;
	else factor = -extra_space/(double)lb->layout_box.total_shrink;
	stretch = False;
    }
    else factor = 0.0;	

    /* compute the size of the interval for Interval or Tabbed spacing */
    if (lb->layout_box.space_type == XmtLayoutSpaceInterval)
	interval = major_length / ((int)lb->layout_box.num_items+1);
    else if ((lb->layout_box.space_type == XmtLayoutSpaceLTabbed) ||
	     (lb->layout_box.space_type == XmtLayoutSpaceCTabbed) ||
	     (lb->layout_box.space_type == XmtLayoutSpaceRTabbed)){
	if (lb->layout_box.num_items <= 1)
	    interval = major_length;
	else 
	    interval = major_length / ((int)lb->layout_box.num_items);
    }
	
    /* compute the size of the internal spaces, if any */
    if ((lb->layout_box.space_type == XmtLayoutSpaceEven) ||
	(lb->layout_box.space_type == XmtLayoutSpaceLREven)) {
	space = lb->layout_box.space;
	if (stretch) {
	    int num_spaces;
	    if (lb->layout_box.space_type == XmtLayoutSpaceEven)
		num_spaces = lb->layout_box.num_items + 1;
	    else {
		num_spaces = lb->layout_box.num_items - 1;
		if (num_spaces <= 0) num_spaces = 1;
	    }

	    if ((lb->layout_box.item_stretch == 0) ||
		(lb->layout_box.total_stretch == 0)) {
		/* in this case, all the stretchiness goes to the spaces */
		space += extra_space/num_spaces;
	    }
	    else {
		/* otherwise, spaces only get some of the extra space */
		space_stretch = lb->layout_box.total_stretch *
		    (int) lb->layout_box.space_stretch /
			((int) (lb->layout_box.space_stretch +
				lb->layout_box.item_stretch));
		/* rounding works better when we compute it this way */
		total_space = space * num_spaces + space_stretch * factor;
		space = total_space / num_spaces;
	    }
	}
    }
    else space = 0;

    /* compute the major position of the first child */
    *cmajor0 = major_start;
    if (lb->layout_box.space_type == XmtLayoutSpaceEven)
	*cmajor0 += space;

    /* loop through all items in the box */
    child_num = 0;
    ForEachItem(lb, child) {
	cc = LayoutConstraintsRec(child);
	/*
	 * If this is an unmanaged child, we don't have to lay it out.
	 * With the exception, however, that if it is an unmanaged row
	 * or column, we've got to make sure that the contents of that
	 * row or column are hidden.
	 */
	if (!XtIsManaged(child)) {
	    if ((cc->type == XmtLayoutRow) || (cc->type == XmtLayoutCol))
		HideHomelessChildren(child);
	    continue;
	}
	child_num++;
	
	/* Get basic child size and add margins */
	HorizontalMargins(lw, child, &margin_width, &left_margin);
	VerticalMargins(lw, child, &margin_height, &top_margin);
	cw = cc->pref_w + margin_width;
	ch = cc->pref_h + margin_height;

	/* if the box is "Equal", adjust major size of most children */
	if (lb->layout_box.equal) {
	    if ((cc->type != XmtLayoutSpace) &&
		(cc->type != XmtLayoutHSep) &&
		(cc->type != XmtLayoutVSep))
		*cmajorlen = lb->layout_box.equal_size + *major_margin;
	}
	
	/*
	 * stretch or shrink the child's width as needed.
	 * but don't stretch if we've explictly given all stretchiness to
	 * the spaces.  
	 */
	if (factor != 0.0) {
	    if (stretch) {
		if (lb->layout_box.item_stretch != 0)
		    *cmajorlen += (int)(cc->stretchability * factor);
	    }
	    else {
		*cmajorlen -= (int)(cc->shrinkability * factor);
	    }
	}
	
 	/*
	 * And never shrink to less than the size of the margins,
	 * because otherwise the child will have a negative size.
	 */
	if (*cmajorlen <= *major_margin) *cmajorlen = *major_margin+1;

	/* set the minor coordinate depending on justification */
	switch(cc->justification) {
	case XmtLayoutCentered:
	    *cminor0 = minor_start + (minor_length - *cminorlen)/2;
	    break;
	case XmtLayoutFlushLeft:
	    *cminor0 = minor_start;
	    break;
	case XmtLayoutFlushRight:
	    *cminor0 = minor_start + minor_length - *cminorlen;
	    break;
	case XmtLayoutFilled:
	    *cminor0 = minor_start;
	    *cminorlen = minor_length;
	    break;
	}

 	/*
	 * But don't let the minor size be less than the size of the margins,
	 * because otherwise the child will have a negative size.
	 */
	if (*cminorlen <= *minor_margin) *cminorlen = *minor_margin+1;

	/*
	 * for Interval spacing, center the child around the interval.
	 * for Tabbed spacing, position appropriately to the "tab stop".
	 * for LCR spacing, left, center or right justify the child.
	 * for other kinds, remember the starting point of next child.
	 */
	if (lb->layout_box.space_type == XmtLayoutSpaceInterval)
	    *cmajor0 = major_start + interval * child_num - *cmajorlen/2;
	else if (lb->layout_box.space_type == XmtLayoutSpaceLTabbed) 
	    *cmajor0 = major_start + interval * (child_num-1);
	else if (lb->layout_box.space_type == XmtLayoutSpaceCTabbed) 
	    *cmajor0 = major_start + interval * child_num
		- interval/2 - *cmajorlen/2;
	else if (lb->layout_box.space_type == XmtLayoutSpaceRTabbed) 
	    *cmajor0 = major_start + interval * child_num - *cmajorlen;
	else if (lb->layout_box.space_type == XmtLayoutSpaceLCR) {
	    /* first child is already handled */
	    if (child_num == 2)
		*cmajor0 = major_start + (major_length - *cmajorlen)/2;
	    else if (child_num == 3)
		*cmajor0 = major_start + major_length - *cmajorlen;
	}

	/*
	 * figure out where this child ends,
	 * so we know where the next begins (for some space types)
	 */
	next = *cmajor0 + *cmajorlen;
	
	/* indent by the size of the margins, frame and captions */
	cx += left_margin;
	cy += top_margin;
	cw -= margin_width;
	ch -= margin_height;
	
	/* set the child size and location */
	if (XmtIsLayoutGadget(child)) {
	    /*
	     * fake out _XmConfigureObject:
	     * 1) make the XmtLayoutGadget look like a RectObj, briefly.
	     * 2) if this is a box, make sure this is not a no-op
	     *    by setting the child's width to something other than cw.
	     */
	    XmtSetRectObj(child);
	    if (XtClass(child) == xmtLayoutBoxGadgetClass)
		child->core.width = 0;
	    _XmtConfigureObject(child, cx, cy, cw, ch,
				child->core.border_width);
	    XmtUnsetRectObj(child);
	    
	}
	else
	    _XmtConfigureObject(child, cx, cy, cw, ch,
				child->core.border_width);

	/* set starting position for next child for space types that care */
	*cmajor0 = next + space;
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static XtGeometryResult QueryGeometry(Widget w,
				      XtWidgetGeometry *request,
				      XtWidgetGeometry *reply)
#else
static XtGeometryResult QueryGeometry(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
#endif
{
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget) w;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(w);
    XmtLayoutConstraintsPart *cc;
    int total_width, total_height;
    int total_stretch, total_shrink;
    int max_width, max_height;
    int equal_width, equal_height;
    int total_equal_width, total_equal_height;
    int width, height;
    int margin_width, margin_height, top_margin, left_margin;
    int num_items;
    XtWidgetGeometry geometry;
    Widget child;

    total_width = total_height = 0;
    total_stretch = total_shrink = 0;
    equal_width = equal_height = total_equal_width = total_equal_height = 0;
    max_width = max_height = 0;
    num_items = 0;

    /* figure out the size of each child */
    ForEachItem(lb, child) {
	/*
	 * For each managed child:
	 *   figure out the basic size w/o margins.  This may be a hardcoded
	 *     or a preferred size, or it may be the caption size.
	 *   if this is an "equal" box
	 *     compute the maximum marginless size
	 *     compute the total size of non-spaces and non-separators.
	 *   add the margins in
	 *   compute the total and maxmimum sizes with margins
	 *   compute the total stretchability and shrinkability
	 */
	if (!XtIsManaged(child)) continue;
	num_items++;
	cc = LayoutConstraintsRec(child);
	 
	/*
	 * Figure out the basic size of the child without margins.
	 * This is either the preferred size of the child, or the
	 * sizes specified on the layoutWidth and layoutHeight resources.
	 * In either case, this basic size may have to be adjusted if it
	 * is too small for the specified caption.
	 */
	 
	/*
	 * ask the child its preferred size, unless it will be overridden
	 * by resources and the child is not a box we need to recursively
	 * measure.
	 */
	if ((cc->width == 0) || (cc->height == 0) ||
	    (cc->type==XmtLayoutRow) || (cc->type==XmtLayoutCol))
	    XtQueryGeometry(child, NULL, &geometry);

	/* override widget's preferences with resources, if specified */
	if (cc->width) width = cc->width;
	else width = geometry.width;
	if (cc->height) height = cc->height;
	else height = geometry.height;

	/*
	 * if a top or bottom caption is wider than this width, or a
	 * left or right caption is taller than this height, increase
	 * the basic size to match.
	 */
	if (cc->caption) {
	    if ((cc->caption_position == XmtLayoutTop) ||
		(cc->caption_position == XmtLayoutBottom)) {
		if (cc->caption_width > width) width = cc->caption_width;
	    }
	    else { /* caption on the right or left */
		if (cc->caption_height > height) height = cc->caption_height;
	    }
	}

	/* remember this basic preferred or specified size */
	cc->pref_w = width;
	cc->pref_h = height;

	/* handle rows or columns that are "equal" */
	if (lb->layout_box.equal) {
	    /* get maximum marginless sizes */
	    if (width > equal_width) equal_width = width;
	    if (height > equal_height) equal_height = height;

	    /* compute the total size of non-spaces and separators */
	    if ((cc->type != XmtLayoutSpace) &&
		(cc->type != XmtLayoutHSep) &&
		(cc->type != XmtLayoutVSep)) {
		total_equal_width += width;
		total_equal_height += height;
	    }
	}
	    
	/* compute the margins, and add them in */
	HorizontalMargins(lw, child, &margin_width, &left_margin);
	VerticalMargins(lw, child, &margin_height, &top_margin);
	width += margin_width;
	height += margin_height;
	
	/* compute total and maximum sizes */
	total_width += width;
	total_height += height;
	if (max_width < width) max_width = width;
	if (max_height < height) max_height = height;

	/* compute total stretch and shrink */
	total_stretch += cc->stretchability;
	total_shrink += cc->shrinkability;
    }
    
    /*
     * if the row or column has the Equal flag set, go back through
     * the children and recompute the box width or height.
     * Note that spaces and separators aren't affected.
     */
    if (lb->layout_box.equal) {
	if (lb->layout_box.orientation == XmHORIZONTAL)
	    total_width -= total_equal_width;
	else
	    total_height -= total_equal_height;
	
	ForEachItem(lb, child) {
	    if (!XtIsManaged(child)) continue;
	    cc = LayoutConstraintsRec(child);
	    if ((cc->type != XmtLayoutSpace) &&
		(cc->type != XmtLayoutHSep) &&
		(cc->type != XmtLayoutVSep)) {
		if (lb->layout_box.orientation == XmHORIZONTAL)
		    total_width += equal_width;
		else
		    total_height += equal_height;
	    }
	}
    }

    /* check for <= 3 children with LCR spacing */
    if ((lb->layout_box.space_type == XmtLayoutSpaceLCR) && (num_items > 3)) {
	lb->layout_box.space_type = XmtLayoutSpaceLREven;
	XmtWarningMsg("XmtLayout", "lcr",
		      "Widget '%s':\n\tSpace type is LCR, but number of childre is > 3.\n\tUsing LREven instead.",
		      XtName((Widget)lw));
    }

    /* adjust sizes depending on space model */
    if ((lb->layout_box.space_type == XmtLayoutSpaceEven) ||
	(lb->layout_box.space_type == XmtLayoutSpaceInterval)) {
	total_width += lb->layout_box.space * (num_items + 1);
	total_height += lb->layout_box.space * (num_items + 1);
    }
    else if ((lb->layout_box.space_type == XmtLayoutSpaceLREven) ||
	     (lb->layout_box.space_type == XmtLayoutSpaceLCR)) {
	total_width += lb->layout_box.space * (num_items - 1);
	total_height += lb->layout_box.space * (num_items - 1);
    }
    else if ((lb->layout_box.space_type == XmtLayoutSpaceLTabbed) ||
	     (lb->layout_box.space_type == XmtLayoutSpaceCTabbed) ||
	     (lb->layout_box.space_type == XmtLayoutSpaceRTabbed)) {
	total_width += lb->layout_box.space * num_items;
	total_height += lb->layout_box.space * num_items;
    }

    /* adjust the total_stretch to account for the spaces */
    if (lb->layout_box.space_type != XmtLayoutSpaceNone) {
	if (lb->layout_box.item_stretch == 0)
	    total_stretch = INFINITY;
	else
	    total_stretch = total_stretch + total_stretch *
		(int)lb->layout_box.space_stretch /
		    (int)lb->layout_box.item_stretch;
    }
    
    /* record information that we'll need when we lay this box out */
    lb->layout_box.total_stretch = total_stretch;
    lb->layout_box.total_shrink =  total_shrink;
    if (lb->layout_box.orientation == XmHORIZONTAL)
	lb->layout_box.equal_size = equal_width;
    else
	lb->layout_box.equal_size = equal_height;
    lb->layout_box.num_items = num_items;

    /* reply to the geometry query */
    reply->request_mode = CWWidth | CWHeight;
    if (lb->layout_box.orientation == XmHORIZONTAL) {
	reply->width = total_width;
	reply->height = max_height;
    }
    else {
	reply->width = max_width;
	reply->height = total_height;
    }
    return XtGeometryYes;
}

#if NeedFunctionPrototypes
static void Redisplay(Widget w, XEvent *event, Region region)
#else
static void Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
#endif
{
    XmtLayoutBoxGadget lb = (XmtLayoutBoxGadget) w;
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(w);
    Widget child;
    XRectangle rect;
    Boolean filled = False;

    if (lw->core.window == None) return;

    if (lb->layout_box.gc != NULL) {
	XFillRectangle(lw->core.screen->display, lw->core.window,
		       lb->layout_box.gc,
		       lb->rectangle.x, lb->rectangle.y,
		       lb->rectangle.width, lb->rectangle.height);
	if (region) {
	    rect.x = lb->rectangle.x;
	    rect.y = lb->rectangle.y;
	    rect.width = lb->rectangle.width;
	    rect.height = lb->rectangle.height;
	    XUnionRectWithRegion(&rect, region, region);
	}
	filled = True;
    }

    ForEachItem(lb, child) {
        if (XmtIsLayoutGadget(child) && XtIsManaged(child)) {
	    if (!region || filled ||
		XRectInRegion(region, child->core.x, child->core.y,
			      child->core.width, child->core.height))
		_XmtRedisplayGadget(child, event, region);
	}
    }
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutBox(Widget parent, String name,
			  ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutBox(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtLayoutBoxGadgetClass, parent,
			  arglist, num_args);
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutRow(Widget parent, String name,
			  ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutRow(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    Widget w;
    w = XtVaCreateWidget(name, xmtLayoutBoxGadgetClass, parent,
			 XmtNorientation, XmHORIZONTAL, NULL);
    XtSetValues(w, arglist, num_args);
    return w;
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutCol(Widget parent, String name,
			  ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutCol(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    Widget w;
    w = XtVaCreateWidget(name, xmtLayoutBoxGadgetClass, parent,
			 XmtNorientation, XmVERTICAL, NULL);
    XtSetValues(w, arglist, num_args);
    return w;
}
