/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutSep.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutSep.c,v $
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

#define offset(field) XtOffsetOf(XmtLayoutSeparatorRec, layout_separator.field)
static XtResource resources[] = {
{XmtNorientation, XmCOrientation, XmROrientation,
     sizeof(unsigned char), offset(orientation),
     XtRImmediate, (XtPointer)XmHORIZONTAL},
};
#undef offset

#if NeedFunctionPrototypes
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Redisplay(Widget, XEvent *, Region);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *,
                                      XtWidgetGeometry *);
#else
static void Initialize();
static void Redisplay();
static XtGeometryResult QueryGeometry();
#endif

#define superclass (&xmtLayoutGadgetClassRec)

externaldef(xmtlayoutseparatorclassrec)
XmtLayoutSeparatorClassRec xmtLayoutSeparatorClassRec = {
{   /* rect_class fields  */
    /* superclass	  */	(WidgetClass)superclass,
    /* class_name	  */	"XmtLayoutSeparator",
    /* widget_size	  */	sizeof(XmtLayoutSeparatorRec),
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
    /* destroy		  */	NULL,
    /* resize		  */	NULL,
    /* expose		  */	Redisplay,
    /* set_values	  */	NULL,
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
  { /* XmtLayoutSeparator fields */
    /* extension          */	NULL
  }
};

externaldef(xmtlayoutseparatorgadgetclass)
WidgetClass xmtLayoutSeparatorGadgetClass =
    (WidgetClass) &xmtLayoutSeparatorClassRec;

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
    XmtLayoutSeparatorGadget ls = (XmtLayoutSeparatorGadget) init;
    
    if (ls->layout_separator.orientation == XmHORIZONTAL) {
	Constraint(ls, type) = XmtLayoutHSep;
	if (ls->rectangle.height == 0) ls->rectangle.height = 2;
    }
    else {
	Constraint(ls, type) = XmtLayoutVSep;
	if (ls->rectangle.width == 0) ls->rectangle.width = 2;
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
    XmtLayoutSeparatorGadget ls = (XmtLayoutSeparatorGadget) w;

    reply->request_mode = CWWidth | CWHeight;
    if (ls->layout_separator.orientation == XmHORIZONTAL) {
	reply->width = 0;
	reply->height = ls->rectangle.height;
    }
    else {
	reply->width = ls->rectangle.width;
	reply->height = 0;
    }
    return XtGeometryYes;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Redisplay(Widget widget, XEvent *event, Region region)
#else
static void Redisplay(widget, event, region)
Widget widget;
XEvent *event;
Region region;
#endif
{
    XmtLayoutSeparatorGadget ls = (XmtLayoutSeparatorGadget) widget;
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(widget);
    Display *dpy = lw->core.screen->display;
    Window win = lw->core.window;
    int x = ls->rectangle.x;
    int y = ls->rectangle.y;
    int w = ls->rectangle.width;
    int h = ls->rectangle.height;

    if (lw->core.window == None) return;

    if (ls->layout_separator.orientation == XmHORIZONTAL) {	
	XFillRectangle(dpy, win, lw->manager.bottom_shadow_GC,
		       x, y, w, h/2);
	XFillRectangle(dpy, win, lw->manager.top_shadow_GC,
		       x, y+h/2, w, h-h/2);
    }
    else {
	XFillRectangle(dpy, win, lw->manager.bottom_shadow_GC,
		       x, y, w/2, h);
	XFillRectangle(dpy, win, lw->manager.top_shadow_GC,
		       x+w/2, y, w-w/2, h);
    }
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutSeparator(Widget parent, String name,
			  ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutSeparator(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtLayoutSeparatorGadgetClass, parent,
			  arglist, num_args);
}
