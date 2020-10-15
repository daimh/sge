/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutGadget.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutGadget.c,v $
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

/* $XConsortium: RectObj.c,v 1.14 91/06/11 20:11:45 converse Exp $ */

#if NeedFunctionPrototypes
static void CopyAncestorSensitive(Widget widget, int offset, XrmValue *value);
#else
static void CopyAncestorSensitive();
#endif

static XtResource resources[] = {
    {XtNancestorSensitive, XtCSensitive, XtRBoolean, sizeof(Boolean),
      XtOffsetOf(XmtLayoutGadgetRec,rectangle.ancestor_sensitive),XtRCallProc,
      (XtPointer)CopyAncestorSensitive},
    {XtNx, XtCPosition, XtRPosition, sizeof(Position),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.x),
	 XtRImmediate, (XtPointer)0},
    {XtNy, XtCPosition, XtRPosition, sizeof(Position),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.y),
	 XtRImmediate, (XtPointer)0},
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.width),
	 XtRImmediate, (XtPointer)0},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.height),
	 XtRImmediate, (XtPointer)0},
    {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.border_width),
	 XtRImmediate, (XtPointer)1},
    {XtNsensitive, XtCSensitive, XtRBoolean, sizeof(Boolean),
         XtOffsetOf(XmtLayoutGadgetRec,rectangle.sensitive),
	 XtRImmediate, (XtPointer)True}
    };

#if XmVersion >= 1002
#if NeedFunctionPrototypes
static void ClassInitialize(void);
#else
static void ClassInitialize();
#endif
#endif
#if NeedFunctionPrototypes
static void Initialize(Widget requested_widget, register Widget new_widget,
		       ArgList args, Cardinal *num_args);
static void ClassPartInitialize(register WidgetClass wc);
static void SetValuesAlmost(Widget old, Widget new,
			    XtWidgetGeometry *request,XtWidgetGeometry *reply);
#else
static void Initialize();
static void ClassPartInitialize();
static void SetValuesAlmost();
#endif

#if XmVersion >= 1002
XmBaseClassExtRec baseClassExt = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    NULL,	/* InitializePrehook	*/
    NULL,	/* SetValuesPrehook	*/
    NULL,	/* InitializePosthook	*/
    NULL,	/* SetValuesPosthook	*/
    NULL,	/* secondaryObjectClass	*/
    NULL,	/* secondaryCreate	*/
    NULL,	/* getSecRes data	*/
    { 0 },	/* fastSubclass flags	*/
    NULL,	/* get_values_prehook	*/
    NULL,	/* get_values_posthook	*/
    NULL,       /* classPartInitPrehook */
    NULL,       /* classPartInitPosthook*/
    NULL,       /* ext_resources        */
    NULL,       /* compiled_ext_resources*/
    0,          /* num_ext_resources    */
    FALSE,      /* use_sub_resources    */
    NULL,       /* widgetNavigable      */
    NULL,       /* focusChange          */
};
#endif

externaldef(xmtlayoutgadgetclassrec)
XmtLayoutGadgetClassRec xmtLayoutGadgetClassRec = {
  {
    /* superclass	  */	(WidgetClass)&objectClassRec,
    /* class_name	  */	"XmtLayoutGadget",
    /* widget_size	  */	sizeof(XmtLayoutGadgetRec),
#if XmVersion <= 1001
    /* class_initialize   */    NULL,
#else				
    /* class_initialize   */    ClassInitialize,
#endif
    /* class_part_initialize*/	ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,		
    /* realize		  */	NULL,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	NULL,
    /* expose		  */	NULL,
    /* set_values	  */	NULL,
    /* set_values_hook    */	NULL,			
    /* set_values_almost  */	SetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */	NULL,
#if XmVersion <= 1001
    /* extension	    */  NULL,
#else
    /* extension	    */  (XtPointer)&baseClassExt,
#endif				
  }
};

externaldef(xmtlayoutgadgetclass)
WidgetClass xmtLayoutGadgetClass = (WidgetClass)&xmtLayoutGadgetClassRec;

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void CopyAncestorSensitive(Widget widget, int offset, XrmValue *value)
#else
static void CopyAncestorSensitive(widget, offset, value)
Widget widget;
int offset;
XrmValue *value;
#endif
{
    static Boolean  sensitive;
    Widget parent = widget->core.parent;

    sensitive = (parent->core.ancestor_sensitive & parent->core.sensitive);
    value->addr = (XPointer)(&sensitive);
}

#if XmVersion >= 1002
#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    baseClassExt.record_type = XmQmotif;
}
#endif

#if NeedFunctionPrototypes
static void ClassPartInitialize(register WidgetClass wc)
#else
static void ClassPartInitialize(wc)
register WidgetClass wc;
#endif
{
    register XmtLayoutGadgetClass roc = (XmtLayoutGadgetClass)wc;
    register XmtLayoutGadgetClass super =
	((XmtLayoutGadgetClass)roc->rect_class.superclass);

    if (roc->rect_class.resize == XtInheritResize) {
	roc->rect_class.resize = super->rect_class.resize;
    }

    if (roc->rect_class.expose == XtInheritExpose) {
	roc->rect_class.expose = super->rect_class.expose;
    }

    if (roc->rect_class.set_values_almost == XtInheritSetValuesAlmost) {
       roc->rect_class.set_values_almost = super->rect_class.set_values_almost;
    }

    if (roc->rect_class.query_geometry == XtInheritQueryGeometry) {
	roc->rect_class.query_geometry = super->rect_class.query_geometry;
    }

#if XmVersion >= 1002
    /* always inherit the base class extension.
     * We assume that there aren't any other extension records.
     */
    roc->rect_class.extension = (XtPointer)&baseClassExt;
#endif    
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Initialize(Widget requested_widget, register Widget new_widget,
		       ArgList args, Cardinal *num_args)
#else
static void Initialize(requested_widget, new_widget, args, num_args)
Widget requested_widget;
register Widget new_widget;
ArgList args;
Cardinal *num_args;
#endif
{
    ((XmtLayoutGadget)new_widget)->rectangle.managed = FALSE;
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void SetValuesAlmost(Widget old, Widget new,
			    XtWidgetGeometry *request, XtWidgetGeometry *reply)
#else
static void SetValuesAlmost(old, new, request, reply)
Widget old;
Widget new;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
#endif
{
    *request = *reply;
}
