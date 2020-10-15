/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBox.c,v 1.2 2002-08-22 15:06:10 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBox.c,v $
 * Revision 1.2  2002-08-22 15:06:10  andre
 * AA-2002-08-22-0  I18N:      bunch of fixes for l10n
 *                  Bugtraq:   #4733802, #4733201, #4733089, #4733043,
 *                             #4731976, #4731990, #4731967, #4731958,
 *                             #4731944, #4731935, #4731273, #4729700
 *
 * Revision 1.1.1.1  2001/07/18 11:06:02  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/XmtP.h>
#include <Xmt/HelpBoxP.h>
#include <Xmt/Help.h>
#include <Xmt/AppResP.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrolledWP.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>

/*
 * accelerators for the scrollbar child which will be installed on the
 * push button to allow more- and emacs-like scrolling.
 */
static char accelerators[] = "#override\n\
<Key>space:                PageDownOrRight(0)\n\
<Key>b:                    PageUpOrLeft(0)\n\
Ctrl<Key>v:                PageDownOrRight(0)\n\
Meta<Key>v:                PageUpOrLeft(0)\n\
~Shift ~Ctrl <Key>osfUp:   IncrementUpOrLeft(0)\n\
~Shift ~Ctrl <Key>osfDown: IncrementDownOrRight(0)\n\
~Shift Ctrl <Key>osfUp:	   PageUpOrLeft(0)\n\
~Shift Ctrl <Key>osfDown:  PageDownOrRight(0)\n\
<Key>osfBeginLine:         TopOrBottom()\n\
<Key>osfEndLine:           TopOrBottom()\n\
<Key>osfPageUp:	           PageUpOrLeft(0)\n\
<Key>osfPageDown:          PageDownOrRight(0)";

static XtAccelerators parsed_accelerators = NULL;
    

#if NeedFunctionPrototypes
extern void _XmForegroundColorDefault(Widget, int, XrmValue *);
extern void _XmBackgroundColorDefault(Widget, int, XrmValue *);
#else
extern void _XmForegroundColorDefault();
extern void _XmBackgroundColorDefault();
#endif

#define offset(field) XtOffsetOf(XmtHelpBoxRec, help_box.field)
static XtResource resources[] = {
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), offset(help_text),
     XtRString, NULL},
{XmtNhelpTitle, XmtCHelpTitle, XtRString,
     sizeof(String), offset(help_title),
     XtRString, NULL},
{XmtNhelpNode, XmtCHelpNode, XtRString,
     sizeof(String), offset(help_node),
     XtRString, NULL},
{XmtNhelpPixmap, XmtCHelpPixmap, XtRPixmap,
     sizeof(Pixmap), offset(help_pixmap),
     XtRImmediate, (XtPointer) None},
{XmtNvisibleLines, XmtCVisibleLines, XtRShort,
     sizeof(short), offset(visible_lines),
     XtRImmediate, (XtPointer) 8},
{XmtNhelpFontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), offset(help_font_list),
     XmRFontList, NULL},
/* new name for existing resource */
{XmtNtitleFontList, XmCFontList, XmRFontList,  
     sizeof(XmFontList), XtOffsetOf(XmtHelpBoxRec, layout.font_list),
     XmRFontList, NULL},
{XmtNtitleForeground, XtCForeground, XtRPixel,
     sizeof(Pixel), offset(title_foreground),
     XtRCallProc, (XtPointer)_XmForegroundColorDefault},
{XmtNhelpForeground, XtCForeground, XtRPixel,
     sizeof(Pixel), offset(help_foreground),
     XtRCallProc, (XtPointer)_XmForegroundColorDefault},
{XmtNhelpBackground, XtCBackground, XtRPixel,
     sizeof(Pixel), offset(help_background),
     XtRCallProc, (XtPointer)_XmBackgroundColorDefault},
/* override an existing defaults */
{XmNmarginWidth, XmCMarginWidth, XtRDimension, sizeof(Dimension),
     XtOffsetOf(XmtHelpBoxRec, bulletin_board.margin_width),
     XtRImmediate, (XtPointer)5},
{XmNmarginHeight, XmCMarginHeight, XtRDimension, sizeof(Dimension),
     XtOffsetOf(XmtHelpBoxRec, bulletin_board.margin_height),
     XtRImmediate, (XtPointer)5},
};

#undef offset

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

externaldef(xmthelpboxclassrec) XmtHelpBoxClassRec xmtHelpBoxClassRec = {
  { /* core fields */
    /* superclass		*/	superclass,
    /* class_name		*/	"XmtHelpBox",
    /* widget_size		*/	sizeof(XmtHelpBoxRec),
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
  { /* XmtHelpBoxClassPart */
    /* extension          */ NULL
  }
};

externaldef(xmthelpboxwidgetclass) WidgetClass xmtHelpBoxWidgetClass =
    (WidgetClass)&xmtHelpBoxClassRec;


#if NeedFunctionPrototypes
static void SetScrolledSize(XmtHelpBoxWidget hb)
#else
static void SetScrolledSize(hb)
XmtHelpBoxWidget hb;
#endif
{
    XmString label;
    int lines;
    Dimension width, height;
    XmScrolledWindowWidget s =
	(XmScrolledWindowWidget) hb->help_box.scrolled_widget;


    /* get the label string and size and query # of lines */

#if defined(SOLARIS64)
    /*
    ** I18N bug for label size under Solaris
    */
    XtVaGetValues(hb->help_box.label_widget,
		  XmNlabelString, &label,
		  XmNheight, &height,
		  NULL);
   
      
    XtVaSetValues(hb->help_box.label_widget, 
                  XmNlabelString, label,
                  XmNalignment, XmALIGNMENT_BEGINNING,
                  XmNheight, (int)(1.1 * height), 
                  NULL);
#endif

    XtVaGetValues(hb->help_box.label_widget,
		  XmNlabelString, &label,
		  XmNwidth, &width,
		  XmNheight, &height,
		  NULL);

    if (label) {
	lines = XmStringLineCount(label);
#if XmVersion < 1002
	lines++;  /* Motif computes this wrong in 1.1 */
#endif	
    }
    else lines = 0;
    XmStringFree(label);
    /*
     * if the number of lines in the label widget is > than the requested
     * number of visible lines, then we will have a vertical scrollbar.
     * In this case, we've got to increase the scrolled window width to
     * accomodate the scrollbar.  Note that we'll always make the window
     * wide enough so that we'll never have a horizontal scrollbar.
     */
    if (lines > hb->help_box.visible_lines) {
	width += ((Widget)s->swindow.vScrollBar)->core.width
	    + 2* ((XmPrimitiveWidget)s->swindow.vScrollBar)->
		primitive.highlight_thickness
	    + s->swindow.pad;
    }

    /* adjust the hight to be visible_lines tall */
    height = ((int)(height-4) * hb->help_box.visible_lines) / lines + 4;

    /* add shadow and margin widths to the width and height */
    width += 2*s->manager.shadow_thickness + 2*s->swindow.WidthPad +
	hb->bulletin_board.margin_width;  /* the XmtLayout margin */
    height += 2*s->manager.shadow_thickness + 2*s->swindow.HeightPad +
	hb->bulletin_board.margin_height; /* the XmtLayout margin */

    /* set the size of the scrolled window */
    XtVaSetValues(hb->help_box.scrolled_widget,
		  XmtNlayoutWidth, width,
		  XmtNlayoutHeight, height,
		  XmNwidth, width,
		  XmNheight, height,
		  NULL);
}

#if NeedFunctionPrototypes
static Boolean GetNodeTextAndTitle(XmtHelpBoxWidget hb)
#else
static Boolean GetNodeTextAndTitle(hb)
XmtHelpBoxWidget hb;
#endif
{
    XmtHelpNode *node;
    
    node = XmtHelpLookupNode((Widget)hb, hb->help_box.help_node);
    if (!node) {
	XmtWarningMsg("HelpBox", "badNode",
		      "%s: help node '%s' is undefined.",
		      XtName((Widget)hb), hb->help_box.help_node);
	return False;
    }
    else {
	hb->help_box.help_text = XmtHelpNodeGetBody(node);
	hb->help_box.help_title = XmtHelpNodeGetTitle(node);
	return True;
    }
}


#define question_width 32
#define question_height 32
static unsigned char question_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xfe, 0x07, 0x00,
   0x80, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0x3f, 0x00, 0xe0, 0x0f, 0x7f, 0x00,
   0xe0, 0x03, 0x7c, 0x00, 0xf0, 0x01, 0xf8, 0x00, 0xf0, 0x01, 0xf8, 0x00,
   0xf0, 0x01, 0xf8, 0x00, 0xf0, 0x01, 0xf8, 0x00, 0xf0, 0x01, 0xf8, 0x00,
   0xe0, 0x01, 0xf8, 0x00, 0xc0, 0x01, 0x7c, 0x00, 0x00, 0x00, 0x7c, 0x00,
   0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x80, 0x0f, 0x00,
   0x00, 0xc0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0xe0, 0x03, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xe0, 0x03, 0x00,
   0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    /*
     * register the default help icon
     * We'll probably want different icons for mono and color.
     */
    XmtRegisterXbmData(XmtHELP_BOX_PIXMAP_NAME,
		       (char *)question_bits, NULL,
		       question_width, question_height, 0, 0);
}

static char empty_string[] = "";

/*ARGSUSED*/
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
    XmtHelpBoxWidget hb = (XmtHelpBoxWidget) init;
    Widget row, col;
    XmString label;
    XmScrolledWindowWidget sw;
    Boolean free_body = False;

    /*
     * Note that we don't copy the strings.
     * This is because we simply pass them on to other widgets that
     * do copy them.  We forget the strings once used so we can detect
     * a change, even if the same buffer is used.
     * XXX This means that these resources can't be meaningfully queried!
     *
     * We don't copy the fontlist for the same reason.
     * We do need to obtain a default value for an unspecified fontlist,
     * however, so we can determine the width and height of the help text.
     */
    if (hb->help_box.help_font_list == NULL)
	hb->help_box.help_font_list =
	    _XmGetDefaultFontList(init,XmLABEL_FONTLIST);
    
    /*
     * if a help node is specified, use it to override the help text and title
     * Note that we do copy help_node; this is a resource that it might
     * be useful to query.
     */
    if (hb->help_box.help_node) {
	hb->help_box.help_node = XtNewString(hb->help_box.help_node);
	free_body = GetNodeTextAndTitle(hb);
    }

    /*
     * get a default pixmap if none specified
     */
    if (hb->help_box.help_pixmap == None)
	hb->help_box.help_pixmap =
	    XmtLookupWidgetPixmap(init, XmtHELP_BOX_PIXMAP_NAME);

    /*
     * create the widgets
     */
    row = XtVaCreateManagedWidget("row", xmtLayoutBoxGadgetClass, init,
				  XmtNlayoutFrameType, XmtLayoutFrameBottom,
				  XmtNlayoutFrameMargin, 5,
				  NULL);

    hb->help_box.pixmap_gadget =
	XtVaCreateManagedWidget("icon", xmtLayoutPixmapGadgetClass, init,
				XmtNpixmap, hb->help_box.help_pixmap,
				XmtNlayoutJustification, XmtLayoutCentered,
				XmtNlayoutIn, row,
				NULL);
    
    col = XtVaCreateManagedWidget("col", xmtLayoutBoxGadgetClass, init,
				  XmtNorientation, XmVERTICAL,
				  XmtNlayoutIn, row,
				  NULL);

    hb->help_box.title_gadget =
	XtVaCreateManagedWidget("title", xmtLayoutStringGadgetClass, init,
			      XmtNlabel, hb->help_box.help_title,
			      XmtNforeground, hb->help_box.title_foreground,
			      XmtNlayoutIn, col,
			      XmtNlayoutJustification, XmtLayoutCentered,
			      XmtNlayoutMarginHeight, 0,
			      NULL);
    hb->help_box.scrolled_widget =
	XtVaCreateManagedWidget("scrolled", xmScrolledWindowWidgetClass, init,
				XmNscrollingPolicy, XmAUTOMATIC,
				XmtNlayoutIn, col,
				XmtNlayoutShrinkability, 10,
				XmtNlayoutMarginHeight, 0,
				NULL);
    sw = (XmScrolledWindowWidget)hb->help_box.scrolled_widget;

    label = XmtCreateXmString(hb->help_box.help_text);
    hb->help_box.label_widget =
	XtVaCreateManagedWidget("helptext", xmLabelWidgetClass,
				hb->help_box.scrolled_widget,
				XmNlabelString, label,
				XmNfontList, hb->help_box.help_font_list,
				XmNforeground, hb->help_box.help_foreground,
				XmNbackground, hb->help_box.help_background,
				XmNalignment, XmALIGNMENT_BEGINNING,
				NULL);
    XmStringFree(label);

    /* set background on scrolled window clip window as well */
    XtVaSetValues((Widget)sw->swindow.ClipWindow,
		  XmNbackground, hb->help_box.help_background,
		  NULL);    
    label = XmStringCreateSimple(XmtLocalizeWidget(init, "Okay", "ok"));
    hb->help_box.button_widget =
	XtVaCreateManagedWidget("okay", xmPushButtonWidgetClass, init,
				XmNlabelString, label,
				XmtNlayoutJustification, XmtLayoutFlushRight,
				XmtNlayoutStretchability, 0,
				XmtNlayoutShrinkability, 0,
				NULL);
    XmStringFree(label);

    /*
     * make the push button the default
     */
    XtVaSetValues(init, XmNdefaultButton, hb->help_box.button_widget, NULL);
		     
    /* set accelerators on the vertical scrollbar
     * and install them on the push button
     */
    if (!parsed_accelerators)
	parsed_accelerators = XtParseAcceleratorTable(accelerators);
    XtVaSetValues((Widget)sw->swindow.vScrollBar,
		  XtNaccelerators, parsed_accelerators,
		  NULL);
    XtInstallAccelerators(hb->help_box.button_widget,
			  (Widget)sw->swindow.vScrollBar);

    /*
     * forget the strings, so we'll notice if the user calls
     * XtSetValues with a new string in the same buffer or with NULL.
     * Forget the fontlist for the same reason.
     * We use empty_string for these string values because
     * it is guaranteed to be different from all valid strings or fontlists.
     * NULL would be a natural choice, but that can be considered a
     * valid string.
     * Note that if the help_text came from a help node, we free it first.
     */
    if (free_body) XtFree(hb->help_box.help_text);
    hb->help_box.help_text = hb->help_box.help_title = empty_string;
    hb->help_box.help_font_list = NULL;

    /*
     * set the initial size of the scrolled window
     */
    SetScrolledSize(hb);
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtHelpBoxWidget hb = (XmtHelpBoxWidget) w;

    XtFree(hb->help_box.help_node);
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
    XmtHelpBoxWidget cw = (XmtHelpBoxWidget) current;
    XmtHelpBoxWidget sw = (XmtHelpBoxWidget) set;
    XmString label;
    Boolean free_body = False;
    Boolean resize = False;
    
    /*
     * if the help_node changed, lookup the new node, and use it to
     * set help_text and help_title.
     */
    if (sw->help_box.help_node != cw->help_box.help_node) {
	XtFree(cw->help_box.help_node);
	sw->help_box.help_node = XtNewString(sw->help_box.help_node);
	free_body = GetNodeTextAndTitle(sw);
    }

    /*
     * strings are not copied, but set to a special empty string.
     * We check if they are now different from empty_string.
     */
    if (sw->help_box.help_text != empty_string) {
	label = XmtCreateXmString(sw->help_box.help_text);
	XtVaSetValues(sw->help_box.label_widget,XmNlabelString,label,NULL);
	XmStringFree(label);
	if (free_body) XtFree(sw->help_box.help_text);
	sw->help_box.help_text = empty_string; /* forget it again */
	resize = True;

#if XmVersion > 1001
	/*
	 * and set the scrollbar to the top of the new text...
	 * XXX I don't know how to do this in Motif 1.1
	 */
	XmScrollVisible(sw->help_box.scrolled_widget,
			sw->help_box.label_widget,
			0, 0);
#endif
    }

    if (sw->help_box.help_title != empty_string) { 
	label = XmtCreateXmString(sw->help_box.help_title);
	XtVaSetValues(sw->help_box.title_gadget, XmNlabelString,label,NULL);
	XmStringFree(label);
	sw->help_box.help_title = empty_string;
    }

    if (sw->help_box.help_pixmap != cw->help_box.help_pixmap)
	XtVaSetValues(sw->help_box.pixmap_gadget,
		      XmtNpixmap, sw->help_box.help_pixmap, NULL);
    
    if (sw->help_box.visible_lines != cw->help_box.visible_lines)
	resize = True;

    if (sw->help_box.help_font_list !=  NULL) { 
	XtVaSetValues(sw->help_box.label_widget,
		      XmNfontList, sw->help_box.help_font_list, NULL);
	sw->help_box.help_font_list = NULL; /* forget it again */
	resize = True;
    }

    if (sw->help_box.title_foreground != cw->help_box.title_foreground)
	XtVaSetValues(sw->help_box.title_gadget,
		      XmtNforeground, sw->help_box.title_foreground,
		      NULL);
    

    if (sw->help_box.help_foreground != cw->help_box.help_foreground)
	XtVaSetValues(sw->help_box.label_widget,
		      XmNforeground, sw->help_box.help_foreground,
		      NULL);

    if (sw->help_box.help_background != cw->help_box.help_background)
	XtVaSetValues(sw->help_box.label_widget,
		      XmNbackground, sw->help_box.help_background,
		      NULL);

    if (resize)
	SetScrolledSize(sw);

    return False;
}

#if NeedFunctionPrototypes
Widget XmtCreateHelpBox(Widget parent, String name,
			ArgList al, Cardinal ac)
#else
Widget XmtCreateHelpBox(parent, name, al, ac)
Widget parent;
String name;
ArgList al;
Cardinal ac;
#endif
{
    return XtCreateWidget(name, xmtHelpBoxWidgetClass, parent, al, ac);
}


#if NeedFunctionPrototypes
Widget XmtCreateHelpDialog(Widget parent, String name,
			   ArgList al, Cardinal ac)
#else
Widget XmtCreateHelpDialog(parent, name, al, ac)
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

    w = XtCreateWidget(name, xmtHelpBoxWidgetClass, shell, al, ac);
    XtAddCallback(w, XtNdestroyCallback,
		  (XtCallbackProc)_XmDestroyParentCallback, NULL);
    return w;
}


