/* 
 * Motif Tools Library, Version 3.1
 * $Id: WorkingBox.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: WorkingBox.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/XmtP.h>
#include <Xmt/WorkingBoxP.h>
#include <Xmt/Pixmap.h>

#include <Xmt/LayoutG.h>
#include <Xm/Scale.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>

static char empty_string[] = "";

#define offset(field) XtOffsetOf(XmtWorkingBoxRec, working_box.field)

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), offset(message),
     XtRString, NULL},
{XmtNscaleLabel, XmtCScaleLabel, XtRString,
     sizeof(String), offset(scale_label),
     XtRString, empty_string},
{XmtNbuttonLabel, XmtCButtonLabel, XtRString,
     sizeof(String), offset(button_label),
     XtRString, empty_string},
{XmtNicon, XmtCIcon, XtRPixmap,
     sizeof(Pixmap), offset(icon),
     XtRImmediate, None},
{XmtNshowScale, XmtCShowScale, XtRBoolean,
     sizeof(Boolean), offset(show_scale),
     XtRImmediate, (XtPointer) True},
{XmtNshowButton, XmtCShowButton, XtRBoolean,
     sizeof(Boolean), offset(show_button),
     XtRImmediate, (XtPointer) True},
{XmtNscaleValue, XmtCScaleValue, XtRInt,
     sizeof(int), offset(scale_value),
     XtRImmediate, (XtPointer) 0},
{XmtNscaleMin, XmtCScaleMin, XtRInt,
     sizeof(int), offset(scale_min),
     XtRImmediate, (XtPointer) 0},
{XmtNscaleMax, XmtCScaleMax, XtRInt,
     sizeof(int), offset(scale_max),
     XtRImmediate, (XtPointer) 100},
{XmtNscaleWidget, XtCReadOnly, XtRWidget,
     sizeof(Widget), offset(scale),
     XtRWidget, NULL},
{XmtNbuttonWidget, XtCReadOnly, XtRWidget,
     sizeof(Widget), offset(button),
     XtRWidget, NULL},
};


#if NeedFunctionPrototypes
static void ClassInitialize(void);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
#else
static void ClassInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
#endif

#define superclass (WidgetClass)&xmtLayoutClassRec

externaldef(xmtworkingboxclassrec)
XmtWorkingBoxClassRec xmtWorkingBoxClassRec = {
  { /* core fields */
    /* superclass		*/	superclass,
    /* class_name		*/	"XmtWorkingBox",
    /* widget_size		*/	sizeof(XmtWorkingBoxRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	XtExposeCompressMaximal,
    /* compress_enterleave	*/	FALSE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	XtInheritResize,
    /* expose			*/	XtInheritExpose,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	XtInheritAcceptFocus,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child       */    XtInheritInsertChild,
    /* delete_child       */    XtInheritDeleteChild,
    /* extension          */    NULL
  },
  { /* constraint_class fields */
    /* resource list 	  */    NULL,
    /* num resources	  */    0,
    /* constraint size    */    sizeof(XmtLayoutConstraintsRec),
    /* init proc	  */    NULL,
    /* destroy proc       */    NULL,
    /* set values proc    */    NULL,
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
    /* extension	  */    NULL
  },
  { /* XmtWorkingBoxClassPart */
    /* extension          */ NULL
  }
};

externaldef(xmtworkingoboxwidgetclass)
WidgetClass xmtWorkingBoxWidgetClass = (WidgetClass)&xmtWorkingBoxClassRec;

/* XPM */
static char *clock_image [] = {
"48 48 6 1",
"  	m None",
"X	s background m white",
":	s foreground m black",
".	s top_shadow m black",
"#	s bottom_shadow m black",
"o	s select m black",
"                       .                        ",
"                 .............                  ",
"              .........X.........               ",
"            .....XXXXXXXXXXXXX#####             ",
"           ...XXXXXXX####XXXXXXXX###            ",
"         ...XXXXXXXXX#oo.XXXXXXXXXX###          ",
"        ...XXXXXXXXXX#oo.XXXXXXXXXXX###         ",
"       ...XXX####XXXX#...XXXXXX####XX####       ",
"      ...XXXX#oo.XXXXXXXXXXXXXX#oo.XXXX##       ",
"     ...XXXXX#oo.XXXXXXXXXXXXXX#oo.XXXX###      ",
"     ..XXXXXX#...XXXXX..XXXXXXX#...XXXXX##      ",
"    ..XXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXX##     ",
"   ..XXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXX##    ",
"   ..XX####XXXXXXXXXX.::#XXXXXXXXXXX####XX##    ",
"  ..XXX#oo.XXXXXXXXXX.::#XXXXXXXXXXX#oo.XXX##   ",
"  ..XXX#oo.XXXXXXXXXX.::#XXXXXXXXXXX#oo.XXX##   ",
"  ..XXX#...XXXXXXXXXX.::#XXXXXXXXXXX#...XXX##   ",
" ..XXXXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXX##  ",
" ..X####XXXXXXXXXXXXX.::#.......XXXXXXX####X##  ",
"..XX#oo.XXXXXXXXXXXXX.::::::::::#XXXXXX#oo.XX## ",
" ..X#oo.XXXXXXXXXXXXX:::::::::::#XXXXXX#oo.X##  ",
" ..X#...XXXXXXXXXXX.::##########XXXXXXX#...X##  ",
" ..XXXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXXXXXXX##  ",
" ..XXXXXXXXXXXX.::#XXXXXXXXXXXXXXXXXXXXXXXXX##  ",
"  ..XXX####XXX.::#XXXXXXXXXXXXXXXXXX####XXX##   ",
"  ..XXX#oo.XX.::#XXXXXXXXXXXXXXXXXXX#oo.XXX##   ",
"  ..XXX#oo.X.::#XXXXXXXXXXXXXXXXXXXX#oo.XXX##   ",
"   ..XX#...XX:#XXXXXXXXXXXXXXXXXXXXX#...XX##    ",
"   ..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX##    ",
"    ..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX##     ",
"     ..XXXXXX####XXXXXXXXXXXXX####XXXXXX##      ",
"     ...XXXXX#oo.XXXXXXXXXXXXX#oo.XXXXX###      ",
"      ..XXXXX#oo.XXXXXXXXXXXXX#oo.XXXX###       ",
"       ...XXX#...XXXXX####XXXX#...XXX###        ",
"       ....XXXXXXXXXXX#oo.XXXXXXXXXX###         ",
"         ...XXXXXXXXXX#oo.XXXXXXXXX###          ",
"           ...XXXXXXXX#...XXXXXXX###            ",
"            .....XXXXXXXXXXXXX#####             ",
"              ...######X#########               ",
"                 .############                  ",
"                       #                        ",
"                                                "};


/* ARGSUSED */
#if NeedFunctionPrototypes
static void ButtonCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void ButtonCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtWorkingBoxWidget wd = (XmtWorkingBoxWidget)tag;

    wd->working_box.button_pressed = True;
}

#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    XmtImage *image;

    image = XmtParseXpmData(clock_image);
    XmtRegisterImage("_xmt_clock_image", image);
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
    XmtWorkingBoxWidget wd = (XmtWorkingBoxWidget) init;
    Widget pixmap, scale, separator, button;
    XmString label;
    
    /*
     * get the clock pixmap.
     */
    if (wd->working_box.icon == None) {
	wd->working_box.icon =
	    XmtLookupWidgetPixmap(init, "_xmt_clock_image");
	wd->working_box.free_icon = True;
    }
    else wd->working_box.free_icon = False;

    /*
     * localize the default scale and button labels, if these
     * resources have not been set.
     */
    if (wd->working_box.scale_label == empty_string)
	wd->working_box.scale_label =
	    XmtLocalizeWidget(init, "% Complete", "scaleLabel");
    if (wd->working_box.button_label == empty_string)
	wd->working_box.button_label =
	    XmtLocalizeWidget(init, "Stop", "stop");

    /*
     * create the widgets
     */
    label = XmtCreateLocalizedXmString((Widget)wd, wd->working_box.message);
    pixmap = XtVaCreateManagedWidget("icon", xmtLayoutPixmapGadgetClass, init,
			XmtNpixmap, wd->working_box.icon,
			XmtNlayoutCaption, label,
			XmtNlayoutCaptionPosition, XmtLayoutRight,
			XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
			XmtNlayoutCaptionMargin, 15,
			XmtNlayoutJustification, XmtLayoutFlushLeft,
			XmtNlayoutStretchability, 0,
			NULL);
    XmStringFree(label);

    label = XmtCreateLocalizedXmString((Widget)wd, wd->working_box.scale_label);
    scale = XtVaCreateWidget("scale", xmScaleWidgetClass, init,
			 XmNorientation, XmHORIZONTAL,
			 XmNshowValue, True,
			 XmNvalue, wd->working_box.scale_value,
			 XmNminimum, wd->working_box.scale_min,
			 XmNmaximum, wd->working_box.scale_max,
			 XmNhighlightThickness, 0,
			 XmtNlayoutJustification, XmtLayoutFilled,
			 XmtNlayoutCaption, label,
			 XmtNlayoutCaptionJustification, XmtLayoutFlushBottom,
			 XmtNlayoutStretchability, 0,
			 NULL);
    XtSetSensitive(scale, False);
    XmStringFree(label);

    separator = XtVaCreateWidget("separator",
				 xmtLayoutSeparatorGadgetClass, init,
				 NULL);

    label = XmtCreateLocalizedXmString((Widget)wd, wd->working_box.button_label);
    button = XtVaCreateWidget("button", xmPushButtonWidgetClass, init,
			      XmNlabelString, label,
			      XmtNlayoutJustification, XmtLayoutFlushRight,
			      XmtNlayoutStretchability, 0,
			      NULL);
    XmStringFree(label);
    XtAddCallback(button, XmNactivateCallback, ButtonCallback,(XtPointer)wd);

    if (wd->working_box.show_scale) 
	XtManageChild(scale);
    
    if (wd->working_box.show_button) {
	XtManageChild(separator);
	XtManageChild(button);
    }

    wd->working_box.pixmap = pixmap;
    wd->working_box.scale = scale;
    wd->working_box.separator = separator;
    wd->working_box.button = button;
    
    /*
     * We don't make a private copy of the string resources because
     * it is pretty wasteful.  Instead, we set the strings to a private
     * empty string so that we can detect any changes to the string.
     * Note that this means the app programmer cannot meaningfully query
     * the value of these resources.
     */
    wd->working_box.message = empty_string;
    wd->working_box.scale_label = empty_string;
    wd->working_box.button_label = empty_string;

    /* When we start out, the "Cancel" button has not been pressed. */
    wd->working_box.button_pressed = False; 
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtWorkingBoxWidget wd = (XmtWorkingBoxWidget) w;

    if (wd->working_box.free_icon) XmtReleasePixmap(w,wd->working_box.icon);
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
    XmtWorkingBoxWidget cw = (XmtWorkingBoxWidget) current;
    XmtWorkingBoxWidget sw = (XmtWorkingBoxWidget) set;
    XmString label;
    Arg args[5];
    Cardinal i = 0;

    /*
     * strings are not copied, but set to a special empty string.
     * We check if they are now different from empty_string.
     */
    if (sw->working_box.message != empty_string) {
	if (sw->working_box.message)
	    label = XmtCreateLocalizedXmString((Widget)sw, sw->working_box.message);
	else
	    label = NULL;
	XtVaSetValues(sw->working_box.pixmap, XmtNlayoutCaption, label, NULL);
	if (label) XmStringFree(label);
	sw->working_box.message = empty_string;
    }

    if (sw->working_box.scale_label != empty_string) {
	if (sw->working_box.scale_label)
	    label = XmtCreateLocalizedXmString((Widget)sw, sw->working_box.scale_label);
	else
	    label = NULL;
	XtVaSetValues(sw->working_box.scale, XmtNlayoutCaption, label, NULL);
	if (label) XmStringFree(label);
	sw->working_box.scale_label = empty_string;
    }

    if (sw->working_box.button_label != empty_string) {
	if (sw->working_box.button_label) {
	    label = XmtCreateLocalizedXmString((Widget)sw, sw->working_box.button_label);
	    XtVaSetValues(sw->working_box.button, XmNlabelString, label, NULL);
	    XmStringFree(label);
	}
	sw->working_box.button_label = empty_string;
    }

    if (sw->working_box.icon != cw->working_box.icon) {
	XtVaSetValues(sw->working_box.pixmap,
		      XmtNpixmap, sw->working_box.icon,
		      NULL);
	if (cw->working_box.free_icon)
	    XmtReleasePixmap(current, cw->working_box.icon);
	sw->working_box.free_icon = False;
    }

    if (sw->working_box.show_scale != cw->working_box.show_scale) {
	if (sw->working_box.show_scale) XtManageChild(sw->working_box.scale);
	else  XtUnmanageChild(sw->working_box.scale);
    }
	
    if (sw->working_box.show_button != cw->working_box.show_button) {
	if (sw->working_box.show_button) {
	    XtManageChild(sw->working_box.separator);
	    XtManageChild(sw->working_box.button);
	}
	else {
	    XtUnmanageChild(sw->working_box.separator);
	    XtUnmanageChild(sw->working_box.button);
	}
    }

    if (sw->working_box.scale_value != cw->working_box.scale_value) {
	XtSetArg(args[i], XmNvalue, sw->working_box.scale_value);
	i++;
    }

    if (sw->working_box.scale_min != cw->working_box.scale_min) {
	XtSetArg(args[i], XmNminimum, sw->working_box.scale_min);
	i++;
    }

    if (sw->working_box.scale_max != cw->working_box.scale_max) {
	XtSetArg(args[i], XmNmaximum, sw->working_box.scale_max);
	i++;
    }

    if (i != 0) XtSetValues(sw->working_box.scale, args, i);

    return False;
}



#if NeedFunctionPrototypes
Widget XmtCreateWorkingBox(Widget parent, String name,
			   ArgList al, Cardinal ac)
#else
Widget XmtCreateWorkingBox(parent, name, al, ac)
Widget parent;
String name;
ArgList al;
Cardinal ac;
#endif
{
    return XtCreateWidget(name, xmtWorkingBoxWidgetClass, parent, al, ac);
}

#if NeedFunctionPrototypes
Widget XmtCreateWorkingDialog(Widget parent, String name,
			      ArgList al, Cardinal ac)
#else
Widget XmtCreateWorkingDialog(parent, name, al, ac)
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

    w = XtCreateWidget(name, xmtWorkingBoxWidgetClass, shell, al, ac);
    XtAddCallback(w, XtNdestroyCallback,
		  (XtCallbackProc) _XmDestroyParentCallback, NULL);
    return w;
}

#if NeedFunctionPrototypes
void XmtWorkingBoxSetScaleValue(Widget w, int value)
#else
void XmtWorkingBoxSetScaleValue(w, value)
Widget w;
int value;
#endif
{
    XmtWorkingBoxWidget wd = (XmtWorkingBoxWidget) w;

    XmtAssertWidgetClass(w,xmtWorkingBoxWidgetClass,
			 "XmtWorkingBoxSetScaleValue");
    if (wd->working_box.scale_value != value) {
	wd->working_box.scale_value = value;
	XmScaleSetValue(wd->working_box.scale, value);
	XFlush(XtDisplay(w));
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static Bool check_event(Display *dpy, XEvent *event, XPointer tag)
#else
static Bool check_event(dpy, event, tag)
Display *dpy;
XEvent *event;
XPointer tag;
#endif
{
    Window *windows = (Window *)tag;

    if ((event->xany.window == windows[0]) ||
	(event->xany.window == windows[1]) ||
	(event->xany.window == windows[2]))
	return True;
    else
	return False;
}

/*
 * This function is loosely based on CheckForInterrupt()
 * from page 683 of Dan Heller's _Motif Programming Manual_
 */
#if NeedFunctionPrototypes
Boolean XmtWorkingBoxHandleEvents(Widget w)
#else
Boolean XmtWorkingBoxHandleEvents(w)
Widget w;
#endif
{
    XmtWorkingBoxWidget wd = (XmtWorkingBoxWidget) w;
    Display *dpy = XtDisplay(w);
    Window windows[3];
    XEvent event;

    XmtAssertWidgetClass(w, xmtWorkingBoxWidgetClass,
			 "XmtWorkingBoxHandleEvents");

    if (!XtIsRealized(w)) return False;
    
    /*
     * Flush any X output, and wait for any resulting events to be queued.
     */
    XSync(dpy, False);
    
    /*
     * Get and dispatch all events in the dialog.  If the
     * Cancel button is activated, the callback will be called, and
     * button_pressed will be set to True.  Or, if the main event loop
     * is still active, button_pressed may have already been set to True
     * before this procedure was even called.
     */
    windows[0] = XtWindow(w);
    windows[1] = XtWindow(wd->working_box.button);
    windows[2] = XtWindow(wd->working_box.scale);
    while(XPending(dpy) &&
	  XCheckIfEvent(dpy, &event, check_event, (XPointer) windows)) {
	XtDispatchEvent(&event);
    }

    /* handle any other expose events on the interface */
    XmUpdateDisplay(w);

    /* and discard any other keypress or mouse events
     * Note that we don't discard KeyRelease events
     */
    XmtDiscardButtonEvents(w);
    XmtDiscardKeyPressEvents(w);

    /* return the value of button_pressed, but reset that field first. */
    if  (wd->working_box.button_pressed) {
	wd->working_box.button_pressed = False;
	return True;
    }
    else
	return False;
}
