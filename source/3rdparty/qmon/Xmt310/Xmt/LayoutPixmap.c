/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutPixmap.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutPixmap.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/LayoutGP.h>
#include <Xmt/Converters.h> /* for definition of XmtRBitmask */

#define offset(field) XtOffsetOf(XmtLayoutPixmapRec, layout_pixmap.field)
static XtResource resources[] = {
{XmtNpixmap, XmtCPixmap, XtRPixmap,
     sizeof(Pixmap), offset(pixmap),
     XtRImmediate, (XtPointer) None},
{XmtNbitmap, XmtCBitmap, XtRBitmap,
     sizeof(Pixmap), offset(bitmap),
     XtRImmediate, (XtPointer) None},
{XmtNbitmask, XmtCBitmask, XmtRBitmask,
     sizeof(Pixmap), offset(bitmask),
     XtRImmediate, (XtPointer) None},
{XmtNforeground, XtCForeground, XtRPixel,
     sizeof(Pixel), offset(foreground),
     XtRImmediate, (XtPointer) -1},
{XmtNbackground, XtCBackground, XtRPixel,
     sizeof(Pixel), offset(background),
     XtRImmediate, (XtPointer) -1}
};
#undef offset

#if NeedFunctionPrototypes
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void Redisplay(Widget, XEvent *, Region);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *,
                                      XtWidgetGeometry *);
#else
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Redisplay();
static XtGeometryResult QueryGeometry();
#endif

#define superclass (&xmtLayoutGadgetClassRec)

externaldef(xmtlayoutpixmapclassrec)
XmtLayoutPixmapClassRec xmtLayoutPixmapClassRec = {
{   /* rect_class fields  */
    /* superclass	  */	(WidgetClass)superclass,
    /* class_name	  */	"XmtLayoutPixmap",
    /* widget_size	  */	sizeof(XmtLayoutPixmapRec),
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
    /* resize		  */	NULL,
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
  { /* XmtLayoutPixmap fields */
    /* extension          */	NULL
  }
};

externaldef(xmtlayoutpixmapgadgetclass)
WidgetClass xmtLayoutPixmapGadgetClass = (WidgetClass)&xmtLayoutPixmapClassRec;

#if NeedFunctionPrototypes
static void GetGC(XmtLayoutWidget lw, XmtLayoutPixmapGadget lp)
#else
static void GetGC(lw, lp)
XmtLayoutWidget lw;
XmtLayoutPixmapGadget lp;
#endif
{
    XGCValues gcv;
    long flags = GCForeground | GCBackground | GCGraphicsExposures;

    if ((lp->layout_pixmap.foreground == -1)  &&
	(lp->layout_pixmap.background == -1) &&
	(lp->layout_pixmap.bitmask == None)) {
	lp->layout_pixmap.gc = NULL;
	return;
    }
    
    if (lp->layout_pixmap.foreground != -1)
	gcv.foreground = lp->layout_pixmap.foreground;
    else
	gcv.foreground = lw->manager.foreground;
    
    if (lp->layout_pixmap.background != -1)
	gcv.background = lp->layout_pixmap.background;
    else
	gcv.background = lw->core.background_pixel;

    gcv.graphics_exposures = False;
    
    if (lp->layout_pixmap.bitmask != None) {
	gcv.clip_mask = lp->layout_pixmap.bitmask;
	flags |= GCClipMask;
    }
    
    lp->layout_pixmap.gc = XtGetGC((Widget)lp, flags, &gcv);
}

#if NeedFunctionPrototypes
static void MeasurePixmap(XmtLayoutWidget lw, XmtLayoutPixmapGadget lp)
#else
static void MeasurePixmap(lw, lp)
XmtLayoutWidget lw;
XmtLayoutPixmapGadget lp;
#endif
{
    unsigned width, height, depth, border;
    int x, y;
    Window dummywin;
    
    if (lp->layout_pixmap.pixmap || lp->layout_pixmap.bitmap) {
	(void)XGetGeometry(lw->core.screen->display,
			   (lp->layout_pixmap.pixmap
			       ?lp->layout_pixmap.pixmap
			       :lp->layout_pixmap.bitmap),
			   &dummywin, &x, &y, &width, &height, &border,&depth);
	lp->layout_pixmap.width = width;
	lp->layout_pixmap.height = height;
	lp->layout_pixmap.depth = depth;
    }
    else {
	lp->layout_pixmap.width = lp->layout_pixmap.height = 0;
    }
}

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
    XmtLayoutPixmapGadget lp = (XmtLayoutPixmapGadget) init;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(init);

    Constraint(lp, type) = XmtLayoutPixmap;

    MeasurePixmap(lw, lp);
    GetGC(lw, lp);
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtLayoutPixmapGadget lp = (XmtLayoutPixmapGadget) w;

    if (lp->layout_pixmap.gc)
	XtReleaseGC(w, lp->layout_pixmap.gc);
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
    XmtLayoutPixmapGadget cp = (XmtLayoutPixmapGadget) current;
    XmtLayoutPixmapGadget sp = (XmtLayoutPixmapGadget) set;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(set);
    Boolean redisplay = False;
    Boolean relayout = False;

    if ((sp->layout_pixmap.pixmap != cp->layout_pixmap.pixmap) ||
	(sp->layout_pixmap.bitmap != cp->layout_pixmap.bitmap)) {
	MeasurePixmap(lw, sp);
	sp->rectangle.width = sp->layout_pixmap.width;
	sp->rectangle.height = sp->layout_pixmap.height;
	if (!sp->rectangle.width) sp->rectangle.width++;
	if (!sp->rectangle.height) sp->rectangle.height++;
	relayout = True;
    }

    if ((sp->layout_pixmap.foreground != cp->layout_pixmap.foreground) ||
	(sp->layout_pixmap.background != cp->layout_pixmap.background)||
	(sp->layout_pixmap.bitmask    != cp->layout_pixmap.bitmask)) {
	if (sp->layout_pixmap.gc) XtReleaseGC(set, sp->layout_pixmap.gc);
	GetGC(lw, sp);
	redisplay = True;
    }

    if ((redisplay | relayout) && XtIsRealized((Widget)lw))
	XClearArea(XtDisplay((Widget)lw), XtWindow((Widget)lw),
		   cp->rectangle.x, cp->rectangle.y,
		   cp->rectangle.width, cp->rectangle.height,
		   True);
		   
    return False;
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
    XmtLayoutPixmapGadget lp = (XmtLayoutPixmapGadget) w;

    reply->request_mode = CWWidth | CWHeight;
    reply->width = lp->layout_pixmap.width;
    reply->height = lp->layout_pixmap.height;
    return XtGeometryYes;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Redisplay(Widget w, XEvent *event, Region region)
#else
static void Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
#endif
{
    XmtLayoutPixmapGadget lp = (XmtLayoutPixmapGadget) w;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(w);

    if (lw->core.window == None) return;
    if ((lp->layout_pixmap.pixmap == None) && (lp->layout_pixmap.bitmap==None))
	return;

    if (lp->layout_pixmap.bitmask)
	XSetClipOrigin(XtDisplay((Widget)lw), lp->layout_pixmap.gc,
		       lp->rectangle.x, lp->rectangle.y);

    /*
     * pixmap should be multi-plane, and bitmap should be single-plane,
     * but we don't assume that.  We depend on the queried depth.
     */
    if ((lp->layout_pixmap.pixmap == None) ||
	(lp->layout_pixmap.depth != lw->core.depth))
	XCopyPlane(XtDisplay((Widget)lw),
		   (lp->layout_pixmap.pixmap
		      ?lp->layout_pixmap.pixmap
		      :lp->layout_pixmap.bitmap),
		   XtWindow((Widget)lw),
		   (lp->layout_pixmap.gc?lp->layout_pixmap.gc:lw->layout.gc),
		   0, 0, lp->layout_pixmap.width, lp->layout_pixmap.height,
		   lp->rectangle.x, lp->rectangle.y, (unsigned long) 1);
    else
	XCopyArea(XtDisplay((Widget)lw),
		  (lp->layout_pixmap.pixmap
		      ?lp->layout_pixmap.pixmap
		      :lp->layout_pixmap.bitmap),
		  XtWindow((Widget)lw),
		  (lp->layout_pixmap.gc?lp->layout_pixmap.gc:lw->layout.gc),
		  0, 0, lp->layout_pixmap.width, lp->layout_pixmap.height,
		  lp->rectangle.x, lp->rectangle.y);

    if (lp->layout_pixmap.bitmask)
	XSetClipOrigin(XtDisplay((Widget)lw), lp->layout_pixmap.gc, 0, 0);
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutPixmap(Widget parent, String name,
			     ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutPixmap(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtLayoutPixmapGadgetClass, parent,
			  arglist, num_args);
}
