/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutSpace.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutSpace.c,v $
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

#if NeedFunctionPrototypes
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static XtGeometryResult QueryGeometry(Widget, XtWidgetGeometry *,
                                      XtWidgetGeometry *);
#else
static void Initialize();
static XtGeometryResult QueryGeometry();
#endif

#define superclass (&xmtLayoutGadgetClassRec)

externaldef(xmtlayoutspaceclassrec)
XmtLayoutSpaceClassRec xmtLayoutSpaceClassRec = {
{   /* rect_class fields  */
    /* superclass	  */	(WidgetClass)superclass,
    /* class_name	  */	"XmtLayoutSpace",
    /* widget_size	  */	sizeof(XmtLayoutSpaceRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/	NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,		
    /* rect1		  */	NULL,
    /* rect2		  */	NULL,
    /* rect3	  	  */	0,
    /* resources	  */	NULL,
    /* num_resources	  */	0,
    /* xrm_class	  */	NULLQUARK,
    /* rect4		  */	FALSE,
    /* rect5		  */	FALSE,
    /* rect6		  */ 	FALSE,
    /* rect7		  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	NULL,
    /* expose		  */	NULL,
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
  { /* XmtLayoutSpace fields */
    /* extension          */	NULL
  }
};

externaldef(xmtlayoutspacegadgetclass)
WidgetClass xmtLayoutSpaceGadgetClass = (WidgetClass) &xmtLayoutSpaceClassRec;

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
    Constraint(init, type) = XmtLayoutSpace;
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
    reply->request_mode = CWWidth | CWHeight;
    reply->width = reply->height = 0;
    return XtGeometryYes;
}


#if NeedFunctionPrototypes
Widget XmtCreateLayoutSpace(Widget parent, String name,
			    ArgList arglist, Cardinal num_args)
#else
Widget XmtCreateLayoutSpace(parent, name, arglist, num_args)
Widget parent;
String name;
ArgList arglist;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtLayoutSpaceGadgetClass, parent,
			  arglist, num_args);
}
