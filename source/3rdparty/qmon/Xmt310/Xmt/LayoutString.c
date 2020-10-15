/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutString.c,v 1.2 2001-10-29 16:27:45 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutString.c,v $
 * Revision 1.2  2001-10-29 16:27:45  andre
 * AA-2001-10-29-0: Fix for weired characters in helper dialogs (PE,Project Configi, Calendar, Cluster Config,
 *                  Complex Config)
 *                  Queue Control Reschedule functionality added (corresponds qmod -r <qname>)
 *                  Cluster config Reschedule Unknown bug fixed
 *                  Job Custom dialogue core dump fix
 *
 * Revision 1.1.1.1  2001/07/18 11:06:02  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/LayoutGP.h>
#include <Xmt/Converters.h>

/* an empty string value unique to us. */
static char empty_string[] = "";

#define offset(field) XtOffsetOf(XmtLayoutStringRec, layout_string.field)
static XtResource resources[] = {
{XmtNlabel, XtCLabel, XtRString,
     sizeof(String), offset(label),
     XtRString, (XtPointer) empty_string},
{XmtNlabelString, XmCLabelString, XmRXmString,
     sizeof(XmString), offset(label_string),
     XmRXmString, (XtPointer)NULL},
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
static void ChangeFont(Widget);
#else
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Redisplay();
static XtGeometryResult QueryGeometry();
static void ChangeFont();
#endif

#define superclass (&xmtLayoutGadgetClassRec)

externaldef(xmtlayoutstringclasrec)
XmtLayoutStringClassRec xmtLayoutStringClassRec = {
{   /* rect_class fields  */
    /* superclass	  */	(WidgetClass)superclass,
    /* class_name	  */	"XmtLayoutString",
    /* widget_size	  */	sizeof(XmtLayoutStringRec),
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
    /* change_font        */    ChangeFont,
  },
  { /* XmtLayoutString fields */
    /* extension          */	NULL
  }
};

externaldef(xmtlayoutstringgadgetclass)
WidgetClass xmtLayoutStringGadgetClass = (WidgetClass)&xmtLayoutStringClassRec;

#define MARGINWIDTH 2
#define MARGINHEIGHT 2


#if NeedFunctionPrototypes
static void GetGC(XmtLayoutWidget lw, XmtLayoutStringGadget ls)
#else
static void GetGC(lw, ls)
XmtLayoutWidget lw;
XmtLayoutStringGadget ls;
#endif
{
    XGCValues gcv;

    if ((ls->layout_string.foreground == -1) ||
	(ls->layout_string.foreground == lw->manager.foreground)) 
	ls->layout_string.gc = None;
    else {
	gcv.foreground = ls->layout_string.foreground;
	/* a background for XmStringDrawImage (& Motif2.0 renditions) */ 
	if (ls->layout_string.background != -1)
	    gcv.background = ls->layout_string.background;
	else
	    gcv.background = lw->core.background_pixel;
	/* we need to specify some font so XmDrawString() doesn't barf */
	gcv.font = lw->layout.font->fid;

	ls->layout_string.gc = XtGetGC((Widget)ls,
				       GCForeground | GCBackground | GCFont,
				       &gcv);
    }
}

#if NeedFunctionPrototypes
static void GetBackgroundGC(XmtLayoutWidget lw, XmtLayoutStringGadget ls)
#else
static void GetBackgroundGC(lw, ls)
XmtLayoutWidget lw;
XmtLayoutStringGadget ls;
#endif
{
    XGCValues gcv;

    if ((ls->layout_string.background == -1) ||
	(ls->layout_string.background == lw->core.background_pixel))
	ls->layout_string.background_gc = NULL;
    else {
	gcv.foreground = ls->layout_string.background;
	ls->layout_string.background_gc =
	    XtGetGC((Widget)ls, GCForeground, &gcv);
    }
}

#if NeedFunctionPrototypes
static void MeasureString(XmtLayoutWidget lw, XmtLayoutStringGadget ls)
#else
static void MeasureString(lw, ls)
XmtLayoutWidget lw;
XmtLayoutStringGadget ls;
#endif
{
    if (ls->layout_string.label_string)
#if XmVersion < 2000
	XmStringExtent(lw->layout.font_list, ls->layout_string.label_string,
		       &ls->layout_string.width, &ls->layout_string.height);
#else /* Motif 2.0 or later */
	XmStringExtent(lw->layout.render_table, ls->layout_string.label_string,
		       &ls->layout_string.width, &ls->layout_string.height);
#endif
    else 
	ls->layout_string.width = ls->layout_string.height = 0;

    ls->layout_string.width += 2*MARGINWIDTH;
    ls->layout_string.height += 2*MARGINHEIGHT;
}    

#if NeedFunctionPrototypes
static void ChangeFont(Widget w)
#else
static void ChangeFont(w)
Widget w;
#endif
{
    XmtLayoutWidget lw = (XmtLayoutWidget)XtParent(w);
    XmtLayoutStringGadget ls = (XmtLayoutStringGadget)w;

    MeasureString(lw, ls);
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
    XmtLayoutStringGadget ls = (XmtLayoutStringGadget) init;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(init);

    Constraint(ls, type) = XmtLayoutString;

    /* copy the XmString, if specified */
    if (ls->layout_string.label_string)
	ls->layout_string.label_string =
	    XmStringCopy(ls->layout_string.label_string);
    else if (ls->layout_string.label) {
	/* we don't copy the string; just convert it to an XmString */
	/* and then set it to "" because we don't own it. */
	ls->layout_string.label_string =
	    XmtCreateLocalizedXmString((Widget) lw, ls->layout_string.label);
	ls->layout_string.label = empty_string;
    }

    MeasureString(lw, ls);
    GetGC(lw, ls);
    GetBackgroundGC(lw, ls);
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtLayoutStringGadget ls = (XmtLayoutStringGadget) w;

    if (ls->layout_string.label_string)
	XmStringFree(ls->layout_string.label_string);

    if (ls->layout_string.gc)
	XtReleaseGC(w, ls->layout_string.gc);
    if (ls->layout_string.background_gc)
	XtReleaseGC(w, ls->layout_string.background_gc);
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
    XmtLayoutStringGadget cs = (XmtLayoutStringGadget) current;
    XmtLayoutStringGadget ss = (XmtLayoutStringGadget) set;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(set);
    Boolean redisplay = False;
    Boolean relayout = False;
    Dimension cs_width, cs_height;

    if (ss->layout_string.label_string  != cs->layout_string.label_string) {
	if (cs->layout_string.label_string)
	    XmStringFree(cs->layout_string.label_string);
	if (ss->layout_string.label_string)
	    ss->layout_string.label_string =
		XmStringCopy(ss->layout_string.label_string);
        cs_width  = cs->layout_string.width;
        cs_height = cs->layout_string.height;
	MeasureString(lw, ss);
        if (cs_width  != ss->layout_string.width ||
                cs_height != ss->layout_string.height)
            relayout = True;
        else  
            redisplay = True;
    }
    else if (ss->layout_string.label != empty_string) {
	if (cs->layout_string.label_string)
	    XmStringFree(cs->layout_string.label_string);
	ss->layout_string.label_string =
	    XmtCreateLocalizedXmString((Widget) lw, ss->layout_string.label);
	ss->layout_string.label = empty_string;
        cs_width  = cs->layout_string.width;
        cs_height = cs->layout_string.height;
	MeasureString(lw, ss);
        if (cs_width  != ss->layout_string.width ||
                cs_height != ss->layout_string.height)
            relayout = True;
        else
            redisplay = True;
    }

    if (ss->layout_string.foreground != cs->layout_string.foreground) {
	if (ss->layout_string.gc) XtReleaseGC(set, ss->layout_string.gc);
	GetGC(lw, ss);
	redisplay = True;
    }

    if (ss->layout_string.background != cs->layout_string.background) {
	if (ss->layout_string.background_gc)
	    XtReleaseGC(set, ss->layout_string.gc);
	GetBackgroundGC(lw, ss);
	redisplay = True;
    }

    /*
     * If the string has changed, then everything will need to be updated.
     * Instead of calling _XmtLayoutChildren() directly, though, we just
     * set our core.width and core.height fields to their new values,
     * The new size will be noticed in Layout.c:ConstraintSetValues(),
     * and _XmtLayoutChildren() will be invoked there.  This way, if the
     * string changes, but the size stays the same, no change will be made.
     */
    if (relayout) {
/*	_XmtLayoutChildren(lw, True); */
	ss->rectangle.width = ss->layout_string.width;
	ss->rectangle.height = ss->layout_string.height;
    }

    /*
     * Returning True from this procedure doesn't do any good, presumably
     * because the Intrinsics think that we're an Object, not a RectObj
     * (This is one of the gross Motif hacks, described in Layout.c)
     * So we generate an Expose event ourselves.
     */
    if (redisplay && XtIsRealized((Widget)lw)) {
         Redisplay((Widget)ss, (XEvent *)NULL, (Region)NULL);
    }
    else if ( relayout && XtIsRealized((Widget)lw)) {
        XClearArea(XtDisplay((Widget)lw), XtWindow((Widget)lw),
                   cs->rectangle.x, cs->rectangle.y,
                   cs->rectangle.width, cs->rectangle.height,
                   True);
    }
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
    XmtLayoutStringGadget ls = (XmtLayoutStringGadget) w;

    reply->request_mode = CWWidth | CWHeight;
    reply->width = ls->layout_string.width;
    reply->height = ls->layout_string.height;
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
    XmtLayoutStringGadget ls = (XmtLayoutStringGadget) w;
    XmtLayoutWidget lw = (XmtLayoutWidget) XtParent(w);

    if (lw->core.window == None) return;
    
    if (ls->layout_string.background_gc)
	XFillRectangle(XtDisplay((Widget)lw), XtWindow((Widget)lw),
		       ls->layout_string.background_gc,
		       ls->rectangle.x, ls->rectangle.y,
		       ls->rectangle.width, ls->rectangle.height);

    XmStringDrawImage(lw->core.screen->display, lw->core.window,
#if XmVersion < 2000
		 lw->layout.font_list,
#else /* Motif 2.0 or later */
                 lw->layout.render_table,
#endif
                 ls->layout_string.label_string,
		 (ls->layout_string.gc?ls->layout_string.gc:lw->layout.gc),
		 ls->rectangle.x + MARGINWIDTH,
		 ls->rectangle.y + MARGINHEIGHT,
		 ls->rectangle.width - 2*MARGINWIDTH,
		 XmALIGNMENT_BEGINNING, XmSTRING_DIRECTION_L_TO_R, NULL);
}

#if NeedFunctionPrototypes
Widget XmtCreateLayoutString(Widget parent, String name,
			     ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutString(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtLayoutStringGadgetClass, parent,
			  arglist, num_args);
}
