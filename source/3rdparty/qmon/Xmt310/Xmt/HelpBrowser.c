/* 
 * Motif Tools Library, Version 3.1
 * $Id: HelpBrowser.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: HelpBrowser.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/XmtP.h>
#include <Xmt/HelpBrowserP.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xmt/Converters.h>
#include <Xmt/Pixmap.h>

#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledWP.h>  /* we need to get at the internal scrollbars */
#include <Xm/DialogS.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern int qsort();
#define strcoll(a,b) strcmp(a,b)
#endif

#ifdef VMS
#define strcoll(a,b) strcmp(a,b)
#endif

static char default_font_list[] = 
"-*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*,\
 -*-helvetica-bold-r-*-*-*-180-*-*-*-*-*-*=BIG,\
 -*-helvetica-bold-r-*-*-*-240-*-*-*-*-*-*=HUGE";
static char default_button_font_list[] = 
    "-*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*";
static char default_section_font_list[] = 
    "-*-helvetica-bold-r-*-*-*-180-*-*-*-*-*-*";
static char default_toc_font_list[] = 
    "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*";
static char default_index_font_list[] = 
    "-*-helvetica-medium-o-*-*-*-120-*-*-*-*-*-*";
static char default_text_font_list[] =
"-*-new century schoolbook-medium-r-*-*-*-140-*-*-*-*-*-*=R,\
 -*-new century schoolbook-bold-r-*-*-*-140-*-*-*-*-*-*=B,\
 -*-new century schoolbook-medium-i-*-*-*-140-*-*-*-*-*-*=I,\
 -*-new century schoolbook-medium-r-*-*-*-180-*-*-*-*-*-*=BIG,\
 -*-new century schoolbook-bold-r-*-*-*-180-*-*-*-*-*-*=HUGE,\
 -*-new century schoolbook-medium-r-*-*-*-120-*-*-*-*-*-*=SMALL,\
 -*-helvetica-medium-r-*-*-*-140-*-*-*-*-*-*=H,\
 -*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*=HB,\
 -*-helvetica-medium-o-*-*-*-140-*-*-*-*-*-*=HI,\
 -*-courier-medium-r-*-*-*-120-*-*-*-*-*-*=CW\
";

#if NeedFunctionPrototypes
static void NextCallback(Widget, XtPointer, XtPointer);
static void PrevCallback(Widget, XtPointer, XtPointer);
static void TocCallback(Widget, XtPointer, XtPointer);
static void IndexCallback(Widget, XtPointer, XtPointer);
static void DoneCallback(Widget, XtPointer, XtPointer);
#else
static void NextCallback();
static void PrevCallback();
static void TocCallback();
static void IndexCallback();
static void DoneCallback();
#endif

#if NeedFunctionPrototypes
static void NextSection(Widget, XEvent *, String *, Cardinal *);
static void PreviousSection(Widget, XEvent *, String *, Cardinal *);
static void ParentSection(Widget, XEvent *, String *, Cardinal *);
static void PageDown(Widget, XEvent *, String *, Cardinal *);
static void PageUp(Widget, XEvent *, String *, Cardinal *);
static void LineDown(Widget, XEvent *, String *, Cardinal *);
static void LineUp(Widget, XEvent *, String *, Cardinal *);
#else
static void NextSection();
static void PreviousSection();
static void ParentSection();
static void PageDown();
static void PageUp();
static void LineDown();
static void LineUp();
#endif

static XtActionsRec actions[] = {
{"next-section", NextSection},
{"previous-section", PreviousSection},
{"parent-section", ParentSection},
{"page-up", PageUp},    
{"page-down", PageDown },    
{"line-up", LineUp},    
{"line-down", LineDown},    
};

static char translations[] = "\
<Key>N:       next-section()\n\
<Key>P:       previous-section()\n\
<Key>U:       parent-section()\n\
<Key>space:                page-down()\n\
<Key>b:                    page-up()\n\
Ctrl<Key>v:                page-down()\n\
Meta<Key>v:                page-up()\n\
~Shift ~Ctrl <Key>osfUp:   line-up()\n\
~Shift ~Ctrl <Key>osfDown: line-down()\n\
~Shift Ctrl <Key>osfUp:	   page-up()\n\
~Shift Ctrl <Key>osfDown:  page-down()\n\
<Key>osfPageUp:	           page-up()\n\
<Key>osfPageDown:          page-down()\n\
";

#if NeedFunctionPrototypes
extern void _XmForegroundColorDefault(Widget, int, XrmValue *);
extern void _XmBackgroundColorDefault(Widget, int, XrmValue *);
#else
extern void _XmForegroundColorDefault();
extern void _XmBackgroundColorDefault();
#endif

static char empty_string[] = "";

#define offset(field) XtOffsetOf(XmtHelpBrowserRec, help_browser.field)

static XtResource resources[] = {
{XmtNtitleLabel, XmtCTitleLabel, XtRString,
     sizeof(String), offset(title_label),
     XtRString, empty_string},
{XmtNtocLabel, XmtCTocLabel, XtRString,
     sizeof(String), offset(toc_label),
     XtRString, empty_string},
{XmtNindexLabel, XmtCIndexLabel, XtRString,
     sizeof(String), offset(index_label),
     XtRString, empty_string},
{XmtNnextLabel, XmtCNextLabel, XtRString,
     sizeof(String), offset(next_label),
     XtRString, empty_string},
{XmtNprevLabel, XmtCPrevLabel, XtRString,
     sizeof(String), offset(prev_label),
     XtRString, empty_string},
{XmtNcrossrefLabel, XmtCCrossrefLabel, XtRString,
     sizeof(String), offset(crossref_label),
     XtRString, empty_string},
{XmtNdoneLabel, XmtCDoneLabel, XtRString,
     sizeof(String), offset(done_label),
     XtRString, empty_string},
{XmtNtopNode, XmtCTopNode, XtRString,
     sizeof(String), offset(top_node),
     XtRString, NULL},
{XmtNcurrentNode, XmtCCurrentNode, XtRString,
     sizeof(String), offset(current_node),
     XtRString, NULL},
{XmtNtitlePixmap, XmtCTitlePixmap, XtRPixmap,
     sizeof(Pixmap), offset(title_pixmap),
     XtRString, XmtHELP_BROWSER_PIXMAP_NAME},
{XmtNtextFontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), offset(text_font_list),
     XtRString, default_text_font_list},
{XmtNsectionFontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), offset(section_font_list),
     XtRString, default_section_font_list},
{XmtNtocFontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), offset(toc_font_list),
     XtRString, default_toc_font_list},
{XmtNindexFontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), offset(index_font_list),
     XtRString, default_index_font_list},
{XmtNtextForeground, XtCForeground, XtRPixel,
     sizeof(Pixel), offset(text_foreground),
     XtRCallProc, (XtPointer)_XmForegroundColorDefault},
{XmtNtextBackground, XtCBackground, XtRPixel,
     sizeof(Pixel), offset(text_background),
     XtRCallProc, (XtPointer)_XmBackgroundColorDefault},
{XmtNtextRows, XmCRows, XtRDimension,
     sizeof(Dimension), offset(text_rows),
     XtRImmediate, (XtPointer) 15},
{XmtNtextColumns, XmCColumns, XtRDimension,
     sizeof(Dimension), offset(text_columns),
     XtRImmediate, (XtPointer) 60},
{XmtNtocRows, XmtCTocRows, XtRDimension,
     sizeof(Dimension), offset(toc_rows),
     XtRImmediate, (XtPointer) 6},
/*
 * override the default of a couple of superclass resources
 */
{XmNautoUnmanage, XmCAutoUnmanage, XtRBoolean,
     sizeof(Boolean),
     XtOffsetOf(XmtHelpBrowserRec, bulletin_board.auto_unmanage),
     XtRImmediate, (XtPointer) False},
{XmNbuttonFontList, XmCButtonFontList, XmRFontList,
     sizeof(XmFontList),
     XtOffsetOf(XmtHelpBrowserRec, bulletin_board.button_font_list),
     XtRString, default_button_font_list},
{XmtNfontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), XtOffsetOf(XmtHelpBrowserRec, layout.font_list),
     XtRString, default_font_list},
};

#if NeedFunctionPrototypes
static void ClassInitialize(void);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void UpCallback(Widget, XtPointer, XtPointer);
static void SeeAlsoCallback(Widget, XtPointer, XtPointer);
#else
static void ClassInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void UpCallback();
static void SeeAlsoCallback();
#endif


#define Superclass (WidgetClass)&xmtLayoutClassRec

externaldef(xmthelpbrowserclassrec)
XmtHelpBrowserClassRec xmtHelpBrowserClassRec = {
  { /* core fields */
    /* superclass		*/	Superclass,
    /* class_name		*/	"XmtHelpBrowser",
    /* widget_size		*/	sizeof(XmtHelpBrowserRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
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
  { /* XmtHelpBrowserClassPart */
    /* extension          */ NULL
  }
};

externaldef(xmthelpbrowserwidgetclass)
WidgetClass xmtHelpBrowserWidgetClass = (WidgetClass)&xmtHelpBrowserClassRec;

#define helpicon_width 48
#define helpicon_height 32
static unsigned char helpicon_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xbf,
   0x17, 0x00, 0x80, 0x00, 0x00, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x00, 0xb8,
   0x17, 0x00, 0x80, 0x00, 0x07, 0xe8, 0x9d, 0xff, 0x9f, 0xc0, 0x1f, 0xb8,
   0x17, 0x00, 0x80, 0xf0, 0x7f, 0xe8, 0x9d, 0xff, 0x9f, 0xf0, 0x78, 0xb8,
   0x17, 0x00, 0x80, 0x38, 0xe0, 0xe8, 0x9d, 0xff, 0x83, 0x38, 0xe0, 0xb9,
   0x17, 0x00, 0x80, 0x1c, 0xc0, 0xe9, 0x1d, 0x00, 0x80, 0x1c, 0xc0, 0xb9,
   0x17, 0xfe, 0x9f, 0x1c, 0xc0, 0xe9, 0x1d, 0x00, 0x80, 0x18, 0xe0, 0xb8,
   0x97, 0xff, 0x9f, 0x00, 0xf0, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x78, 0xb8,
   0x97, 0xff, 0x9f, 0x00, 0x3e, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x0e, 0xb8,
   0x97, 0xff, 0x9f, 0x00, 0x07, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x07, 0xb8,
   0x97, 0x0f, 0x80, 0x00, 0x07, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x07, 0xb8,
   0x17, 0x00, 0x80, 0x00, 0x07, 0xe8, 0x9d, 0xff, 0x9f, 0x00, 0x07, 0xb8,
   0x17, 0x00, 0x80, 0x00, 0x00, 0xe8, 0x9d, 0xff, 0x9f, 0x00, 0x07, 0xb8,
   0x17, 0x00, 0x80, 0x00, 0x07, 0xe8, 0x9d, 0xff, 0x81, 0x00, 0x07, 0xb8,
   0x17, 0x00, 0x80, 0x00, 0x00, 0xe8, 0x1d, 0x00, 0x80, 0x00, 0x00, 0xb8,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    /* register the default pixmap used by this widget */
    XmtRegisterXbmData(XmtHELP_BROWSER_PIXMAP_NAME,
		       (char *)helpicon_bits, NULL,
		       helpicon_width, helpicon_height, 0, 0);
    XmtRegisterPixmapConverter();
}


#if NeedFunctionPrototypes
static void SetSize(XmtHelpBrowserWidget hb, Boolean init)
#else
static void SetSize(hb, init)
XmtHelpBrowserWidget hb;
Boolean init;
#endif
{
    XtWidgetGeometry geometry;
    
    /* get the real preferred size of the text sw, not what the layout
     * thinks the size should be.
     */
    if (!init) XtVaSetValues(hb->help_browser.text_sw,
			     XmtNlayoutWidth, 0,
			     XmtNlayoutHeight, 0,
			     NULL);
    XtQueryGeometry(hb->help_browser.text_sw, NULL, &geometry);

    /* tell the layout to use that size for both scrolled widgets */
    XtVaSetValues(hb->help_browser.text_sw,
		  XmtNlayoutWidth, geometry.width,
		  XmtNlayoutHeight, geometry.height,
		  NULL);
    XtVaSetValues(hb->help_browser.label_sw,
		  XmtNlayoutWidth, geometry.width,
		  XmtNlayoutHeight, geometry.height,
		  NULL);
}


#if NeedFunctionPrototypes
static void CreateWidgets(Widget w)
#else
static void CreateWidgets(w)
Widget w;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) w;
    Widget titlerow, mainrow, buttonbox;
    Widget hbar, vbar, clipw;
    Arg args[10];
    int ac;
    XtTranslations trans;
    XmString label;
    int i;

    /* A row with the title pixmap and the title label in it */
    titlerow = XtVaCreateManagedWidget("titlerow", xmtLayoutBoxGadgetClass, w,
			    XmtNspaceType, XmtLayoutSpaceLREven,
			    XmtNitemStretch, 0,
			    XmtNlayoutFrameType, XmtLayoutFrameBottom,
			    XmtNlayoutFrameLineType, XmtLayoutFrameSingleLine,
			    XmtNlayoutFrameThickness, 4,
			    XmtNlayoutFrameMargin, 2,
			    XmtNlayoutStretchability, 0,
			    XmtNlayoutMarginHeight, 5,
			    NULL);

    hb->help_browser.icon_g =
	XtVaCreateManagedWidget("titleicon", xmtLayoutPixmapGadgetClass, w,
				XmtNpixmap, hb->help_browser.title_pixmap,
				XmtNlayoutIn, titlerow,
				XmtNlayoutMarginHeight, 0,
				XmtNlayoutJustification, XmtLayoutFlushBottom,
				NULL);
    hb->help_browser.title_g =
	XtVaCreateManagedWidget("title", xmtLayoutStringGadgetClass, w,
				XmtNlabel, hb->help_browser.title_label,
				XmtNlayoutIn, titlerow,
				XmtNlayoutMarginHeight, 0,
				XmtNlayoutJustification, XmtLayoutFlushBottom,
				NULL);

    /* followed by a row containing the toc and index */
    mainrow = XtCreateManagedWidget("mainrow", xmtLayoutBoxGadgetClass, w,
				    NULL, 0);
    ac = 0;
    XtSetArg(args[ac], XmNfontList, hb->help_browser.toc_font_list); ac++;
    XtSetArg(args[ac], XmNvisibleItemCount, hb->help_browser.toc_rows); ac++;
    XtSetArg(args[ac], XmtNlayoutIn, mainrow); ac++;
    hb->help_browser.toc_w = XmCreateScrolledList(w, "toc", args, ac);
    label = XmtCreateLocalizedXmString(w, hb->help_browser.toc_label);
    XtVaSetValues(XtParent(hb->help_browser.toc_w),
		  XmtNlayoutCaption, label,
		  XmtNlayoutCaptionPosition, XmtLayoutTop,
		  XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
		  NULL);
    XmStringFree(label);
    XtManageChild(hb->help_browser.toc_w);

    ac = 0;
    XtSetArg(args[ac], XmNfontList, hb->help_browser.index_font_list); ac++;
    XtSetArg(args[ac], XmtNlayoutIn, mainrow); ac++;
    hb->help_browser.index_w = XmCreateScrolledList(w, "index", args, ac);
    label = XmtCreateLocalizedXmString(w, hb->help_browser.index_label);
    XtVaSetValues(XtParent(hb->help_browser.index_w),
		  XmtNlayoutCaption, label,
		  XmtNlayoutCaptionPosition, XmtLayoutTop,
		  XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
		  XmtNlayoutStretchability, 0,
		  NULL);
    XmStringFree(label);
    XtManageChild(hb->help_browser.index_w);

	
    /*
     * The text widget title is an option menu.
     * First, create the menu pane with its menu items.
     * Then we can create the option menu.
     */
    hb->help_browser.text_title_pane=XmCreatePulldownMenu(w,"text_title_pane",
							  NULL, 0);
    for(i=0; i < UPBUTTONS; i++) {
	hb->help_browser.title_up_items[i] =
	    XtVaCreateWidget("", xmPushButtonWidgetClass,
			     hb->help_browser.text_title_pane, 
			     XmNfontList, hb->help_browser.section_font_list,
			     NULL);
	XtAddCallback(hb->help_browser.title_up_items[i], XmNactivateCallback,
		      UpCallback, (XtPointer)hb);
		      
    }
    XtCreateManagedWidget("", xmSeparatorWidgetClass,
			  hb->help_browser.text_title_pane, NULL, 0);
    label = XmtCreateLocalizedXmString(w, hb->help_browser.crossref_label);
    hb->help_browser.see_also_label = 
	XtVaCreateManagedWidget("", xmLabelWidgetClass,
				hb->help_browser.text_title_pane,
				XmNlabelString, label,
				XmNfontList,hb->help_browser.section_font_list,
				XmNalignment, XmALIGNMENT_BEGINNING,
				NULL);
    XmStringFree(label);
    for(i=0; i < XREFBUTTONS; i++) {
	hb->help_browser.title_xref_items[i] =
	    XtVaCreateWidget("", xmPushButtonWidgetClass,
			     hb->help_browser.text_title_pane, 
			     XmNfontList, hb->help_browser.section_font_list,
			     NULL);
	XtAddCallback(hb->help_browser.title_xref_items[i],XmNactivateCallback,
		      SeeAlsoCallback, (XtPointer)hb);
    }

    ac = 0;
    XtSetArg(args[ac], XmNsubMenuId, hb->help_browser.text_title_pane); ac++;
    XtSetArg(args[ac], XmtNlayoutStretchability, 0); ac++;
    XtSetArg(args[ac], XmNspacing, 0); ac++;
    XtSetArg(args[ac], XmNmarginWidth, 0); ac++;
    XtSetArg(args[ac], XmNmarginHeight, 0); ac++;
    hb->help_browser.text_title_w =
	XmCreateOptionMenu(w, "text_title", args, ac);
    XtUnmanageChild(XmOptionLabelGadget(hb->help_browser.text_title_w));
    XtManageChild(hb->help_browser.text_title_w);
    XtVaSetValues(XmOptionButtonGadget(hb->help_browser.text_title_w),
		  XmNfontList, hb->help_browser.section_font_list,
		  XmNalignment, XmALIGNMENT_BEGINNING,
		  NULL);

    /* create a scrolled text, by hand */
    hb->help_browser.text_sw = 
	XtVaCreateManagedWidget("textSW", xmScrolledWindowWidgetClass, w,
				XmNscrollingPolicy, XmAPPLICATION_DEFINED,
				XmNvisualPolicy, XmVARIABLE,
				XmNscrollBarDisplayPolicy, XmSTATIC,
				XmNshadowThickness, 0,
				XmtNlayoutShrinkability, 10,
				NULL);
    hb->help_browser.text_w =
	XtVaCreateManagedWidget("text", xmTextWidgetClass,
				hb->help_browser.text_sw,
				XmNcolumns, hb->help_browser.text_columns,
				XmNrows, hb->help_browser.text_rows,
				XmNfontList, hb->help_browser.text_font_list,
				XmNforeground,hb->help_browser.text_foreground,
				XmNbackground,hb->help_browser.text_background,
				XmNeditMode, XmMULTI_LINE_EDIT,
				XmNeditable, False,
				XmNtraversalOn, False,
				NULL);
    /* fix up the scrollbar background, which is now funny */
    XtVaGetValues(hb->help_browser.text_sw,
		  XmNhorizontalScrollBar, &hbar,
		  XmNverticalScrollBar, &vbar,
		  NULL);
    XtVaSetValues(hbar, XmNbackground, hb->core.background_pixel, NULL);
    XtVaSetValues(vbar, XmNbackground, hb->core.background_pixel, NULL);
    
    hb->help_browser.label_sw =
	XtVaCreateWidget("labelSW", xmScrolledWindowWidgetClass, w,
			 XmNscrollingPolicy, XmAUTOMATIC,
			 XmNscrollBarDisplayPolicy, XmSTATIC,
			 XmtNlayoutShrinkability, 10,
			 NULL);
    /* set the size of label_sw to match text_sw */
    SetSize(hb, True);
    
    /* set the background on the clip window too */
    XtVaGetValues(hb->help_browser.label_sw, XmNclipWindow, &clipw, NULL);
    XtVaSetValues(clipw, XmNbackground, hb->help_browser.text_background,NULL);

    hb->help_browser.label_w =
	XtVaCreateManagedWidget("label", xmLabelWidgetClass,
				hb->help_browser.label_sw,
				XmNfontList, hb->help_browser.text_font_list,
				XmNforeground,hb->help_browser.text_foreground,
				XmNbackground,hb->help_browser.text_background,
				XmNalignment, XmALIGNMENT_BEGINNING,
				NULL);

    /*
     * The main body window is followed by a separator and 5 buttons.
     */

    buttonbox =
	XtVaCreateManagedWidget("buttonbox",xmtLayoutBoxGadgetClass,w,
				XmtNlayoutStretchability, 0,
				XmtNequal, True,
				XmtNlayoutFrameType, XmtLayoutFrameTop,
				XmtNspaceType, XmtLayoutSpaceEven,
				XmtNspaceStretch, 2,
				NULL);

    ac = 0;
    XtSetArg(args[ac], XmNlabelString, NULL); ac++;
    XtSetArg(args[ac], XmtNlayoutIn, buttonbox); ac++;	
    
    args[0].value =
	(XtArgVal)XmtCreateLocalizedXmString(w, hb->help_browser.next_label);
    hb->help_browser.next_w =
	XtCreateManagedWidget("next", xmPushButtonWidgetClass, w, args, ac);
    XmStringFree((XmString)args[0].value);

    args[0].value =
	(XtArgVal)XmtCreateLocalizedXmString(w, hb->help_browser.prev_label);
    hb->help_browser.prev_w =
	XtCreateManagedWidget("prev", xmPushButtonWidgetClass, w, args, ac);
    XmStringFree((XmString)args[0].value);

    args[0].value =
	(XtArgVal)XmtCreateLocalizedXmString(w, hb->help_browser.done_label);
    hb->help_browser.done_w =
	XtCreateWidget("done", xmPushButtonWidgetClass, w, args, ac);
    /* give this button a large margin to set it off from the others */
    XtVaSetValues(hb->help_browser.done_w, XmtNlayoutMarginWidth, 15, NULL);
    XtManageChild(hb->help_browser.done_w);
    XmStringFree((XmString)args[0].value);


    /*
     * install the help browser translations on all the widgets.
     * We override on the buttons, and augment on the scrolled lists.
     * This means that the scrolling translations won't override the
     * list scrolling translations, but the spacebar page-down translation
     * will override the pushbutton spacebar activate translation.
     */
    trans = XtParseTranslationTable(translations);
    XtAugmentTranslations(hb->help_browser.toc_w, trans);
    XtAugmentTranslations(hb->help_browser.index_w, trans); 
    XtAugmentTranslations((Widget)((XmScrolledWindowWidget)
			  hb->help_browser.text_sw)->swindow.vScrollBar,
			  trans);
    XtAugmentTranslations((Widget)((XmScrolledWindowWidget)
			  hb->help_browser.label_sw)->swindow.vScrollBar,
			  trans);
    XtAugmentTranslations((Widget)((XmScrolledWindowWidget)
			  hb->help_browser.text_sw)->swindow.hScrollBar,
			  trans);
    XtAugmentTranslations((Widget)((XmScrolledWindowWidget)
			  hb->help_browser.label_sw)->swindow.hScrollBar,
			  trans);
    XtOverrideTranslations(hb->help_browser.next_w, trans);
    XtOverrideTranslations(hb->help_browser.prev_w, trans);
    XtOverrideTranslations(hb->help_browser.done_w, trans);
    XtOverrideTranslations((Widget)hb, trans);
}

#if NeedFunctionPrototypes
static void bad_node_warning(StringConst name)
#else
static void bad_node_warning(name)
StringConst name;
#endif
{
    XmtWarningMsg("XmtHelpBrowser", "noNode",
		  "undefined help node '%s'",
		  name);
}

#if NeedFunctionPrototypes
static void ScrollListIfNecessary(Widget w, int pos)
#else
static void ScrollListIfNecessary(w, pos)
Widget w;
int pos;
#endif
{
    int top, total, visible;
    int newtop;

    XtVaGetValues(w,
		  XmNitemCount, &total,
		  XmNvisibleItemCount, &visible,
		  XmNtopItemPosition, &top,
		  NULL);

    if (pos == 0) pos = total;
    else if (pos < 0) pos = 1;
    else if (pos > total) pos = total;

    /* if already visible, we're done */
    if ((pos >= top) && (pos <= top + visible - 1)) return;

    newtop = pos - visible/2;
    if (newtop > total - visible + 1)  /* don't scroll past the bottom */
	newtop = total - visible + 1;
    if (newtop < 1) newtop = 1;        /* don't scroll past the top */
    XmListSetPos(w, newtop);
}

#if NeedFunctionPrototypes
static XmtHelpTree *BuildTree(Widget w,
			      XmtHelpTree *parent, StringConst node_name,
			      StringConst node_number, short *toc_len_return,
			      short *index_len_return)
#else
static XmtHelpTree *BuildTree(w, parent, node_name, node_number,
			      toc_len_return, index_len_return)
Widget w;
XmtHelpTree *parent;
StringConst node_name;
StringConst node_number;
short int *toc_len_return;
short int *index_len_return;
#endif
{
    XmtHelpNode *help;
    XmtHelpTree *node;
    char subnumber[100];
    char title[200];
    short nodes, total_nodes;
    short keys, total_keys;
    int i, num;

    /*
     * We assume that the help file or files have already been
     * read and parsed and the nodes stored away, ready to be
     * looked up here.
     */

    help = XmtHelpLookupNode(w, node_name);
    if (!help) {
	*toc_len_return = *index_len_return = 0;
	return NULL;
    }

    node = XtNew(XmtHelpTree);
    node->help = help;
    node->parent = parent;
    node->crossrefs = NULL;
    node->num_crossrefs = 0;
    node->children = (XmtHelpTree **)
	XtMalloc(help->num_subnodes * sizeof(XmtHelpTree *));

    total_nodes = 1;
    total_keys = help->num_keywords;
    for(i=num=0; i < help->num_subnodes; i++) {
	sprintf(subnumber, "%s.%d", node_number, num+1);
	node->children[num] =
	    BuildTree(w, node, help->subnodes[i], subnumber, &nodes, &keys);
	if (node->children[num]) {
	    num++;
	    total_nodes += nodes;
	    total_keys += keys;
	}
	else
	    bad_node_warning(help->subnodes[i]);
    }
    node->num_children = num;

    sprintf(title, "%s  %s", node_number, help->title);
    node->title = XmtCreateXmString(title);

    *toc_len_return = total_nodes;
    *index_len_return = total_keys;
    return node;
}

#if NeedFunctionPrototypes
static void FreeTree(XmtHelpTree *node)
#else
static void FreeTree(node)
XmtHelpTree *node;
#endif
{
    int i;

    if (!node) return;

    for(i=0; i < node->num_children; i++) 
	FreeTree(node->children[i]);

    XtFree((char *)node->children);
    XtFree((char *)node->crossrefs);
    XmStringFree(node->title);
    XtFree((char *)node);
}

#if NeedFunctionPrototypes
static int get_toc(XmtHelpTree *node, XmtHelpTree **nodes, XmString *labels)
#else
static int get_toc(node, nodes, labels)
XmtHelpTree *node;
XmtHelpTree **nodes;
XmString *labels;
#endif
{
    int i, num;
    
    if (!node) return 0;
    
    nodes[0] = node;
    labels[0] = node->title;

    for(num=1,i=0; i < node->num_children; i++) 
	num += get_toc(node->children[i], &nodes[num], &labels[num]);

    return num;
}

#if NeedFunctionPrototypes
static void BuildToc(XmtHelpBrowserWidget hb)
#else
static void BuildToc(hb)
XmtHelpBrowserWidget hb;
#endif
{
    XmString *labels;

    hb->help_browser.toc = (XmtHelpTree **)
	XtMalloc(hb->help_browser.toc_len * sizeof(XmtHelpTree *));
    labels = (XmString *)
	XtMalloc(hb->help_browser.toc_len * sizeof(XmString));
    
    get_toc(hb->help_browser.tree,
	    hb->help_browser.toc,
	    labels);

    XtVaSetValues(hb->help_browser.toc_w,
		  XmNitems, labels,
		  XmNitemCount, hb->help_browser.toc_len,
		  NULL);

    /* free the labels array, but don't free the XmStrings themselves;
     * they belong to the XmtHelpTree structures...
     */
    XtFree((char *)labels);
}

#if NeedFunctionPrototypes
static void FreeToc(XmtHelpBrowserWidget hb)
#else
static void FreeToc(hb)
XmtHelpBrowserWidget hb;
#endif
{
    XtFree((char *)hb->help_browser.toc);
    hb->help_browser.toc = NULL;
    hb->help_browser.toc_len = 0;
}


typedef struct {
    String keyword;
    XmtHelpTree *tree_node;
} Keyword;

#if NeedFunctionPrototypes
static int get_keywords(XmtHelpTree *node, Keyword *keywords)
#else
static int get_keywords(node, keywords)
XmtHelpTree *node;
Keyword *keywords;
#endif
{
    int num;
    int child;

    if (!node) return 0;

    for(num = 0; num < node->help->num_keywords; num++) {
	keywords[num].keyword = node->help->keywords[num];
	keywords[num].tree_node = node;
    }
    
    for(child = 0; child < node->num_children; child++)
	num += get_keywords(node->children[child], &keywords[num]);

    return num;
}

#if NeedFunctionPrototypes
static int compare_keywords(_Xconst void *k1, _Xconst void *k2)
#else
static int compare_keywords(k1, k2)
void *k1, *k2;
#endif
{
    return strcoll(((Keyword *)k1)->keyword,
		   ((Keyword *)k2)->keyword);
}

#if NeedFunctionPrototypes
static void BuildIndex(XmtHelpBrowserWidget hb)
#else
static void BuildIndex(hb)
XmtHelpBrowserWidget hb;
#endif
{
    int i;
    Keyword *keywords;
    XmString *labels;
    
    /* allocate space for the index */
    keywords = (Keyword *)
	XtMalloc(hb->help_browser.index_len * sizeof(Keyword));

    /* recurse through the tree to get the index list */
    get_keywords(hb->help_browser.tree, keywords);

    /* sort them */
    if (hb->help_browser.index_len > 1)
	qsort(keywords,
	      hb->help_browser.index_len,
	      sizeof(Keyword),
	      compare_keywords);

    /*
     * allocate a permanent array to remember the XmtHelpTree pointers
     * in their sorted order.  Also allocate a temporary array to hold
     * XmStrings to stuff into the list widget.
     * Initialize both arrays.
     */
    hb->help_browser.index = (XmtHelpTree **)
	XtMalloc(hb->help_browser.index_len * sizeof(XmtHelpTree *));
    labels = (XmString *)
	XtMalloc(hb->help_browser.index_len * sizeof(XmString));

    for(i=0; i < hb->help_browser.index_len; i++) {
	labels[i] = XmtCreateXmString(keywords[i].keyword);
	hb->help_browser.index[i] = keywords[i].tree_node;
    }

    /* set the XmStrings on the widget, and free them and the array */
    XtVaSetValues(hb->help_browser.index_w,
		  XmNitems, labels,
		  XmNitemCount, hb->help_browser.index_len,
		  NULL);
    for(i=0; i < hb->help_browser.index_len; i++)
	XmStringFree(labels[i]);
    XtFree((char *)labels);
    XtFree((char *)keywords);
}

#if NeedFunctionPrototypes
static void FreeIndex(XmtHelpBrowserWidget hb)
#else
static void FreeIndex(hb)
XmtHelpBrowserWidget hb;
#endif
{
    XtFree((char *)hb->help_browser.index);
    hb->help_browser.index = NULL;
    hb->help_browser.index_len = 0;
}

#if NeedFunctionPrototypes
static int FindNamedNodeInToc(XmtHelpBrowserWidget hb, StringConst name)
#else
static int FindNamedNodeInToc(hb, name)
XmtHelpBrowserWidget hb;
StringConst name;
#endif
{
    XmtHelpNode *help;
    int i;
    
    if (!name) return -1;
    
    help = XmtHelpLookupNode((Widget)hb, name);
    for(i=0; i < hb->help_browser.toc_len; i++)
	if (hb->help_browser.toc[i]->help == help) break;

    if (i < hb->help_browser.toc_len)
	return i;
    else {
	bad_node_warning(name);
	return -1;
    }
}

#if NeedFunctionPrototypes
static void SetupOptionMenu(XmtHelpBrowserWidget hb, XmtHelpTree *node)
#else
static void SetupOptionMenu(hb, node)
XmtHelpBrowserWidget hb;
XmtHelpTree *node;
#endif
{
    int i, j, nodenum;
    XmtHelpTree *parent;
    XmtHelpTree *xref;

    if (!node) return;

    /*
     * display the current node and each of its parents in the
     * top half of the option menu.  Make the current node the
     * visible item in the option menu.
     * XXX We should display xrefs we've followed in this history list too.
     */
    XtVaSetValues(hb->help_browser.title_up_items[UPBUTTONS-1],
		  XmNlabelString, node->title,
		  XmNuserData, node,
		  NULL);
    XtVaSetValues(hb->help_browser.text_title_w,
		  XmNmenuHistory, hb->help_browser.title_up_items[UPBUTTONS-1],
		  NULL);
    /*
     * krs@kampsax.dk reports that setting the menuHistory
     * resource on an option menu isn't sufficient to update
     * the cascade button label with his version of Motif. (HP 1.1.?)
     * I think this was a bug fixed in 1.1.3 or 1.1.4.  The following
     * workaround should be benign, even in fixed versions of Motif.
     * I assume that all Motif 1.2 versions are fixed.
     */
#if XmVersion == 1001
    XtVaSetValues(XmOptionButtonGadget(hb->help_browser.text_title_w),
 		  XmNlabelString, node->title,
		  NULL);
#endif

    for(i = UPBUTTONS-2, parent = node->parent;
	(i >= 0) && parent;
	i--, parent=parent->parent) {
	XtVaSetValues(hb->help_browser.title_up_items[i],
		      XmNlabelString, parent->title,
		      XmNuserData, parent,
		      NULL);
    }
    XtManageChildren(&hb->help_browser.title_up_items[i+1], UPBUTTONS-i-1);
    if (i >= 0) XtUnmanageChildren(hb->help_browser.title_up_items, i+1);

    /*
     * Now setup any xref buttons.
     * The first time we're at this node, we trace the xrefs.
     */
    if (!node->crossrefs && node->help->num_crossrefs) {
	node->crossrefs = (short *)XtMalloc(sizeof(short) *
					    node->help->num_crossrefs);
	for(i=0, j=0; i < node->help->num_crossrefs; i++) {
	    nodenum = FindNamedNodeInToc(hb, node->help->crossrefs[i]);
	    if (nodenum != -1) node->crossrefs[j++] = nodenum;
	}
	node->num_crossrefs = j;
    }

    for(i=0; (i < node->num_crossrefs) && (i < XREFBUTTONS); i++) {
	nodenum = node->crossrefs[i];
	xref = hb->help_browser.toc[nodenum];
	XtVaSetValues(hb->help_browser.title_xref_items[i],
		      XmNlabelString, xref->title,
		      XmNuserData, xref,
		      NULL);
    }
    if(i > 0) {
	XtManageChild(hb->help_browser.see_also_label);
	XtManageChildren(hb->help_browser.title_xref_items, i);
    }
    else
	XtUnmanageChild(hb->help_browser.see_also_label);
    if (i < XREFBUTTONS)
	XtUnmanageChildren(&hb->help_browser.title_xref_items[i],
			   XREFBUTTONS-i);
}

#if NeedFunctionPrototypes
static void SetNodeBody(XmtHelpBrowserWidget hb, String body,
			unsigned char format)
#else
static void SetNodeBody(hb, body, format)
XmtHelpBrowserWidget hb;
String body;
unsigned char format;
#endif
{
    XmString label;
    
    /* change the text displayer widget, if necessary */
    if (hb->help_browser.current_format != format) {
	hb->help_browser.current_format = format;
	if (format == XmtHelpFormatString) {
	    XmtLayoutDisableLayout((Widget)hb);
	    XtUnmanageChild(hb->help_browser.label_sw);
	    XtManageChild(hb->help_browser.text_sw);
	    XmtLayoutEnableLayout((Widget)hb);
	}
	else {
	    XmtLayoutDisableLayout((Widget)hb);
	    XtUnmanageChild(hb->help_browser.text_sw);
	    XtManageChild(hb->help_browser.label_sw);
	    XmtLayoutEnableLayout((Widget)hb);
	}
    }

    /* now set the text in the appropriate widget */
    if (format == XmtHelpFormatString) {
	XmTextSetString(hb->help_browser.text_w, body);
    }
    else {
	label = XmtCreateXmString(body);
	XtVaSetValues(hb->help_browser.label_w,
		      XmNlabelString, label,
		      NULL);
	XmStringFree(label);
    }

}

#if NeedFunctionPrototypes
static void SetNodeNumber(XmtHelpBrowserWidget hb, int node_number)
#else
static void SetNodeNumber(hb, node_number)
XmtHelpBrowserWidget hb;
int node_number;
#endif
{
    String body;
    XmtHelpTree *node;

    if (node_number == -1 || hb->help_browser.toc_len == 0) return;

    node = hb->help_browser.toc[node_number];
    if (hb->help_browser.current_node_number == node_number) return;
	
    /* highlight the node in the toc */
    XmListSelectPos(hb->help_browser.toc_w, node_number+1, False);
    ScrollListIfNecessary(hb->help_browser.toc_w, node_number+1);

    /* set the node title and other buttons in option menu*/
    SetupOptionMenu(hb, node);

    /* get the node body */
    body = XmtHelpNodeGetBody(node->help);

    /* check the node format, set body widgets as necessary */
    SetNodeBody(hb, body, node->help->format);

    /* free the body */
    XtFree(body);

    /* sensitize or desensitize buttons depending on node's pos in the tree */
    if (node_number == 0)  /* no previous or parent node */
	XtSetSensitive(hb->help_browser.prev_w, False);
    if (node_number == hb->help_browser.toc_len-1)  /* no next node */
	XtSetSensitive(hb->help_browser.next_w, False);

    if (hb->help_browser.current_node_number == 0)
	XtSetSensitive(hb->help_browser.prev_w, True);
    if (hb->help_browser.current_node_number==hb->help_browser.toc_len-1)
	XtSetSensitive(hb->help_browser.next_w, True);

    /* set the widget current node */
    hb->help_browser.current_node_number = node_number;
}

#if NeedFunctionPrototypes
static void SetNode(XmtHelpBrowserWidget hb, XmtHelpTree *node)
#else
static void SetNode(hb, node)
XmtHelpBrowserWidget hb;
XmtHelpTree *node;
#endif
{
    int i;
    
    if (!node || hb->help_browser.toc_len == 0) return;

    /* 
     * search for the node in the toc and set it.
     */
    for(i=0; i < hb->help_browser.toc_len; i++)
	if (hb->help_browser.toc[i] == node) break;

    if (i < hb->help_browser.toc_len)
	SetNodeNumber(hb, i);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void UpCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void UpCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;
    XmtHelpTree *node;

    XtVaGetValues(w, XmNuserData, &node, NULL);
    SetNode(hb, node);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void SeeAlsoCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void SeeAlsoCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;
    XmtHelpTree *node;

    XtVaGetValues(w, XmNuserData, &node, NULL);
    SetNode(hb, node);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void Initialize(Widget request, Widget init,
		       ArgList args, Cardinal *num_args)
#else
static void Initialize(request, init, args, num_args)
Widget request, init;
ArgList args;
Cardinal *num_args;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) init;

    /*
     * localize the default strings for any string resources 
     * that have not been set.
     */
    if (hb->help_browser.title_label == empty_string)
	hb->help_browser.title_label =
	    XmtLocalizeWidget(init,"@f[HUGE]Online Help Browser","titleLabel");
    if (hb->help_browser.toc_label == empty_string)
	hb->help_browser.toc_label =
	    XmtLocalizeWidget(init, "@f[BIG]Table of Contents", "tocLabel");
    if (hb->help_browser.index_label == empty_string)
	hb->help_browser.index_label =
	    XmtLocalizeWidget(init, "@f[BIG]Index", "indexLabel");
    if (hb->help_browser.next_label == empty_string)
	hb->help_browser.next_label =
	    XmtLocalizeWidget(init, "Next", "next");
    if (hb->help_browser.prev_label == empty_string)
	hb->help_browser.prev_label =
	    XmtLocalizeWidget(init, "Prev", "prev");
    if (hb->help_browser.crossref_label == empty_string)
	hb->help_browser.crossref_label =
	    XmtLocalizeWidget(init, "See Also:", "crossrefLabel");
    if (hb->help_browser.done_label == empty_string)
	hb->help_browser.done_label =
	    XmtLocalizeWidget(init, "Done", "done");

    /* create all the children widgets */
    CreateWidgets(init);
    hb->help_browser.current_format = XmtHelpFormatString;
    
    /* build a tree of help nodes, and toc and index arrays
     * and set the toc and index arrays in the list widgets
     */
    hb->help_browser.tree = BuildTree((Widget)hb,
				      NULL, hb->help_browser.top_node, "1",
				      &hb->help_browser.toc_len,
				      &hb->help_browser.index_len);
    BuildToc(hb);
    BuildIndex(hb);

    /*
     * highlight the current node in the toc, make it visible.
     * get the node body text and display it.
     * 
     * XXX
     * We can map from node name to node number, but not from
     * number to node name (problem with the hash table), so the
     * currentNode resource is not usefully queryable.
     */
    hb->help_browser.current_node_number = -1;
    if (hb->help_browser.current_node == NULL)
	hb->help_browser.current_node = hb->help_browser.top_node;
    SetNodeNumber(hb, FindNamedNodeInToc(hb, hb->help_browser.current_node));

    /* set callbacks on the buttons */
    XtAddCallback(hb->help_browser.next_w, XmNactivateCallback,
		  NextCallback, (XtPointer)hb);
    XtAddCallback(hb->help_browser.prev_w, XmNactivateCallback,
		  PrevCallback, (XtPointer)hb);
    XtAddCallback(hb->help_browser.done_w, XmNactivateCallback,
		  DoneCallback, (XtPointer)hb);

    /* set callbacks on the list widgets */
    XtAddCallback(hb->help_browser.toc_w, XmNbrowseSelectionCallback,
		  TocCallback, (XtPointer)hb);
    XtAddCallback(hb->help_browser.index_w, XmNbrowseSelectionCallback,
		  IndexCallback, (XtPointer)hb);

    /*
     * I don't want to use the memory to copy the string resources,
     * so I forget them so I can detect when they've changed.
     * This means that they cannot be queried.
     */
    hb->help_browser.title_label = empty_string;
    hb->help_browser.toc_label = empty_string;
    hb->help_browser.index_label = empty_string;
    hb->help_browser.next_label = empty_string;
    hb->help_browser.prev_label = empty_string;
    hb->help_browser.crossref_label = empty_string;
    hb->help_browser.done_label = empty_string;
    
    hb->help_browser.top_node = empty_string;
    hb->help_browser.current_node = empty_string;

}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) w;

    FreeToc(hb);
    FreeIndex(hb);
    FreeTree(hb->help_browser.tree);
}
    
/* ARGSUSED */
#if NeedFunctionPrototypes
static Boolean SetValues(Widget current, Widget request, Widget set,
			 ArgList arglist, Cardinal *num_args)
#else
static Boolean SetValues(current, request, set, arglist, num_args)
Widget current, request, set;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtHelpBrowserWidget cw = (XmtHelpBrowserWidget) current;
    XmtHelpBrowserWidget sw = (XmtHelpBrowserWidget) set;
    XmString label;
    Boolean resize = False;
    
#define Changed(field) (sw->help_browser.field != cw->help_browser.field)

    if (Changed(title_label)) {
	XtVaSetValues(sw->help_browser.title_g, XmtNlabel,
		      sw->help_browser.title_label, NULL);
	sw->help_browser.title_label = empty_string;
    }
    if (Changed(toc_label)) {
	XtVaSetValues(sw->help_browser.toc_title_g, XmtNlabel,
		      sw->help_browser.toc_label, NULL);
	sw->help_browser.toc_label = empty_string;
    }
    if (Changed(index_label)) {
	XtVaSetValues(sw->help_browser.index_title_g, XmtNlabel,
		      sw->help_browser.index_label, NULL);
	sw->help_browser.index_label = empty_string;
    }
    if (Changed(next_label)) {
	label = XmtCreateLocalizedXmString((Widget)sw, sw->help_browser.next_label);
	XtVaSetValues(sw->help_browser.next_w, XmNlabelString, label, NULL);
	XmStringFree(label);
	sw->help_browser.next_label = empty_string;
    }
    if (Changed(prev_label)) {
	label = XmtCreateLocalizedXmString((Widget) sw, sw->help_browser.prev_label);
	XtVaSetValues(sw->help_browser.prev_w, XmNlabelString, label, NULL);
	XmStringFree(label);
	sw->help_browser.prev_label = empty_string;
    }
    if (Changed(done_label)) {
	label = XmtCreateLocalizedXmString((Widget) sw, sw->help_browser.done_label);
	XtVaSetValues(sw->help_browser.done_w, XmNlabelString, label, NULL);
	XmStringFree(label);
	sw->help_browser.done_label = empty_string;
    }

    if (Changed(current_node)) {
	SetNodeNumber(sw,
		      FindNamedNodeInToc(sw, sw->help_browser.current_node));
	sw->help_browser.current_node = empty_string;
    }

    if (Changed(title_pixmap))
	XtVaSetValues(sw->help_browser.icon_g,
		      XmtNpixmap, sw->help_browser.title_pixmap, NULL);

    if (Changed(text_font_list)) {
	XtVaSetValues(sw->help_browser.text_w,
		      XmNfontList, sw->help_browser.text_font_list, NULL);
	XtVaSetValues(sw->help_browser.label_w,
		      XmNfontList, sw->help_browser.text_font_list, NULL);
	resize = True;
    }

    if (Changed(section_font_list))
	XtVaSetValues(sw->help_browser.text_title_w,
		      XmNfontList, sw->help_browser.section_font_list, NULL);

    if (Changed(toc_font_list))
	XtVaSetValues(sw->help_browser.toc_w,
		      XmNfontList, sw->help_browser.toc_font_list, NULL);

    if (Changed(index_font_list))
	XtVaSetValues(sw->help_browser.index_w,
		      XmNfontList, sw->help_browser.index_font_list, NULL);

    if (Changed(text_rows) || Changed(text_columns)) {
	XtVaSetValues(sw->help_browser.text_w,
		      XmNrows, sw->help_browser.text_rows,
		      XmNcolumns, sw->help_browser.text_columns,
		      NULL);
	resize = True;
    }

    if (Changed(toc_rows))
	XtVaSetValues(sw->help_browser.toc_w,
		      XmNvisibleItemCount, sw->help_browser.toc_rows, NULL);

    if (resize) SetSize(sw, False);
    
#undef Changed
    return False;
}
    
/*
 * Internal callback functions
 */

/* ARGSUSED */
#if NeedFunctionPrototypes
static void NextCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void NextCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;

    SetNodeNumber(hb, hb->help_browser.current_node_number+1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void PrevCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void PrevCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;

    SetNodeNumber(hb, hb->help_browser.current_node_number-1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void TocCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void TocCallback(w, tag, call_data)
Widget w;
XtPointer tag, call_data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;
    XmListCallbackStruct *data = (XmListCallbackStruct *)call_data;

    SetNodeNumber(hb, data->item_position-1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void IndexCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void IndexCallback(w, tag, call_data)
Widget w;
XtPointer tag, call_data;
#endif
{
    XmtHelpBrowserWidget hb = (XmtHelpBrowserWidget) tag;
    XmListCallbackStruct *data = (XmListCallbackStruct *)call_data;
    XmtHelpTree *node = hb->help_browser.index[data->item_position-1];

    SetNode(hb, node);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void DoneCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void DoneCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    /*
     * just pop down.  If the programmer wants notification of when the
     * dialog goes down, he can use the Bulletinboard unmapCallback.
     */
    XtUnmanageChild((Widget) tag);
}


/*
 * Action routines
 */

#if NeedFunctionPrototypes
static XmtHelpBrowserWidget GetHelpBrowserAncestor(Widget w)
#else
static XmtHelpBrowserWidget GetHelpBrowserAncestor(w)
Widget w;
#endif
{
    while(w && !XtIsSubclass(w, xmtHelpBrowserWidgetClass)) w = XtParent(w);
    return (XmtHelpBrowserWidget)w;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void NextSection(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void NextSection(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);

    if (hb->help_browser.current_node_number >= hb->help_browser.toc_len-1)
	XBell(XtDisplay(w), 0);
    else
	SetNodeNumber(hb, hb->help_browser.current_node_number+1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void PreviousSection(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void PreviousSection(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);

    if (hb->help_browser.current_node_number <= 0)
	XBell(XtDisplay(w), 0);
    else
	SetNodeNumber(hb, hb->help_browser.current_node_number-1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void ParentSection(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void ParentSection(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);
    int nodenum = hb->help_browser.current_node_number;
    XmtHelpTree *parent = hb->help_browser.toc[nodenum]->parent;

    if (hb->help_browser.current_node_number <= 0)
	XBell(XtDisplay(w), 0);
    else {
	SetNode(hb, parent);
    }
}

static String fake_args[] = {"0"};

/* ARGSUSED */
#if NeedFunctionPrototypes
static void PageDown(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void PageDown(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);
    XmScrolledWindowWidget sw;
    Widget scrollbar;

    if (hb->help_browser.current_format == XmtHelpFormatString)
	sw = (XmScrolledWindowWidget)hb->help_browser.text_sw;
    else
	sw = (XmScrolledWindowWidget)hb->help_browser.label_sw;
    scrollbar = (Widget)sw->swindow.vScrollBar;

    XtCallActionProc(scrollbar, "PageDownOrRight", e, fake_args, 1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void PageUp(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void PageUp(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);
    XmScrolledWindowWidget sw;
    Widget scrollbar;

    if (hb->help_browser.current_format == XmtHelpFormatString)
	sw = (XmScrolledWindowWidget)hb->help_browser.text_sw;
    else
	sw = (XmScrolledWindowWidget)hb->help_browser.label_sw;
    scrollbar = (Widget)sw->swindow.vScrollBar;

    XtCallActionProc(scrollbar, "PageUpOrLeft", e, fake_args, 1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void LineDown(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void LineDown(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);
    XmScrolledWindowWidget sw;
    Widget scrollbar;

    if (hb->help_browser.current_format == XmtHelpFormatString)
	sw = (XmScrolledWindowWidget)hb->help_browser.text_sw;
    else
	sw = (XmScrolledWindowWidget)hb->help_browser.label_sw;
    scrollbar = (Widget)sw->swindow.vScrollBar;

    XtCallActionProc(scrollbar, "IncrementDownOrRight",e,fake_args, 1);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void LineUp(Widget w, XEvent *e, String *args, Cardinal *num)
#else
static void LineUp(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtHelpBrowserWidget hb = GetHelpBrowserAncestor(w);
    XmScrolledWindowWidget sw;
    Widget scrollbar;

    if (hb->help_browser.current_format == XmtHelpFormatString)
	sw = (XmScrolledWindowWidget)hb->help_browser.text_sw;
    else
	sw = (XmScrolledWindowWidget)hb->help_browser.label_sw;
    scrollbar = (Widget)sw->swindow.vScrollBar;

    XtCallActionProc(scrollbar, "IncrementUpOrLeft", e, fake_args, 1);
}


/*
 * Public functions
 */
    
#if NeedFunctionPrototypes
Widget XmtCreateHelpBrowser(Widget parent, String name,
			    ArgList args, Cardinal num_args)
#else
Widget XmtCreateHelpBrowser(parent, name, args, num_args)
Widget parent;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    Widget shell, w;
    String shell_name;

    shell_name = XtMalloc(strlen(name) + 7);
    (void)sprintf(shell_name, "%s_shell", name);

    /* create the shell */
    shell = XmCreateDialogShell(parent, shell_name, NULL, 0);
    XtFree(shell_name);

    /* set some default resources */
    XtVaSetValues(shell,
		  XmNallowShellResize, True,
		  XmNtitle, XmtLocalize2(shell, "Online Help",
					 "XmtHelpBrowser", "dialogTitle"),
		  NULL);

    /* and possibly override them with the supplied args */
    XtSetValues(shell, args, num_args);

    /* create the help browser widget */
    w = XtCreateWidget(name, xmtHelpBrowserWidgetClass, shell, args, num_args);
    XtAddCallback(w, XtNdestroyCallback,
		  (XtCallbackProc) _XmDestroyParentCallback, NULL);
    return w;
}
    
