/* 
 * Motif Tools Library, Version 3.1
 * $Id: Menu.c,v 1.2 2008-02-13 14:35:05 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Menu.c,v $
 * Revision 1.2  2008-02-13 14:35:05  andre
 * AA-2008-02-13-0: Bugfix.:   - jgdi client shall use SGE Daemon keystore for JMX ssl mode
 *                             - jmxeventmon support for SSL, eventmon script
 *                             - memory leaks qmon, cull
 *                  Changed:   jgdi, install scripts
 *                  Review:    RH
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
#include <Xm/XmP.h>
#include <Xm/RowColumnP.h>
#include <Xm/MenuShellP.h>
#include <Xmt/MenuP.h>
#include <Xmt/Util.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Procedures.h>
#include <Xmt/Lexer.h>
#include <Xm/Separator.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>

#if XmVersion >= 2000
#include <Xm/MenuT.h>
#endif
#if XmVersion >= 2001
#include <Xm/TraitP.h>
#endif

static XtResource resources[] = {
{XmtNitems, XmtCItems, XmtRXmtMenuItemList,
     sizeof(XmtMenuItem *), XtOffsetOf(XmtMenuRec, menu.items),
     XmtRXmtMenuItemList, NULL},
{XmtNnumItems, XmtCNumItems, XtRInt,
     sizeof(Cardinal), XtOffsetOf(XmtMenuRec, menu.num_items),
     XtRImmediate, (XtPointer) -1},
{XmtNacceleratorFontTag, XmtCAcceleratorFontTag, XtRString,
     sizeof(String), XtOffsetOf(XmtMenuRec, menu.accelerator_font_tag),
     XtRImmediate, NULL},
/* override RowCol default */
{XmNrowColumnType, XmCRowColumnType, XmRRowColumnType,
     sizeof(unsigned char), XtOffsetOf(XmtMenuRec, row_column.type),
     XtRImmediate, (XtPointer) XmMENU_BAR}
};

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
#else
static void ClassPartInitialize();
static void Initialize();
static void Destroy();
#endif

#if XmVersion >= 1002

#if NeedFunctionPrototype
static void ClassInitialize(void);
#else
static void ClassInitialize();
#endif

/*
 * we purposely inherit the initialize prehook here, but not the
 * the initialize posthook.  The posthook scheme is buggy and does
 * not allow recursive creations of submenus.  So instead, we undo
 * what the prehook did explicitly in our Initialize() method.  It is
 * okay that this is not done strictly at the "leaf class", because the
 * prehook really only affects translations which are handled early on,
 * in core class or somewhere.  See the comments in Initialize() for
 * more info.
 */
static XmBaseClassExtRec       baseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    XmInheritInitializePrehook,               /* initialize prehook   */
    XmInheritSetValuesPrehook,                /* set_values prehook   */
    NULL,                                     /* initialize posthook  */
    XmInheritSetValuesPosthook,               /* set_values posthook  */
    XmInheritClass,                           /* secondary class      */
    XmInheritSecObjectCreate,                 /* creation proc        */
    XmInheritGetSecResData,                   /* getSecResData        */
    {0},                                      /* fast subclass        */
    XmInheritGetValuesPrehook,                /* get_values prehook   */
    XmInheritGetValuesPosthook,               /* get_values posthook  */
    NULL,                                     /* classPartInitPrehook */
    NULL,                                     /* classPartInitPosthook*/
    NULL,                                     /* ext_resources        */
    NULL,                                     /* compiled_ext_resources*/
    0,                                        /* num_ext_resources    */
    FALSE,                                    /* use_sub_resources    */
    XmInheritWidgetNavigable,                 /* widgetNavigable      */
    XmInheritFocusChange,                     /* focusChange          */
};

static XmManagerClassExtRec managerClassExtRec = {
    NULL,
    NULLQUARK,
    XmManagerClassExtVersion,
    sizeof(XmManagerClassExtRec),
    XmInheritTraversalChildrenProc,
};
#endif

externaldef(xmtmenuclassrec) XmtMenuClassRec xmtMenuClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &xmRowColumnClassRec,
    /* class_name		*/	"XmtMenu",
    /* widget_size		*/	sizeof(XmtMenuRec),
#if XmVersion >= 1002
    /* class_initialize		*/	ClassInitialize,
#else					
    /* class_initialize		*/	NULL,
#endif
    /* class_part_initialize	*/	ClassPartInitialize,
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
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	XtInheritAcceptFocus,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
#if XmVersion >= 1002
    /* extension		*/	(XtPointer)&baseClassExtRec,
#else
    /* extension		*/	NULL,
#endif
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
    /* constraint size    */    sizeof(XmRowColumnConstraintRec),
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
#if XmVersion >= 1002
    /* extension	  */	(XtPointer)&managerClassExtRec
#else				
    /* extension	  */	NULL
#endif	
  },
  { /* row column class record        */
    /* proc to interface with widgets */   XmInheritMenuProc,
    /* proc to arm&activate menu      */   XmInheritArmAndActivate,
    /* traversal handler              */   XmInheritMenuTraversalProc,
    /* extension                      */   NULL,
  },
  { /* XmtMenuClassPart */
    /* extension          */ NULL
  }
};

externaldef(xmtmenuwidgetclass)
WidgetClass xmtMenuWidgetClass = (WidgetClass)&xmtMenuClassRec;

#if XmVersion == 2000
/* Traits Declarations */
extern XmMenuSystemTraitRec _XmRC_menuSystemRecord;
#endif


/*
 * a macro for extracting the type bits from the type field of a menu item
 */
#define GetType(item) ((item)->type & 0x07)


#if XmVersion >= 1002

#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    baseClassExtRec.record_type = XmQmotif;

#if XmVersion == 2000
    /* Trait records */
    XmeTraitSet((XtPointer) xmtMenuWidgetClass, XmQTmenuSystem,
                (XtPointer) &_XmRC_menuSystemRecord);
#endif
#if XmVersion >= 2001
    /* Trait records */
    XmeTraitSet((XtPointer) xmtMenuWidgetClass, XmQTmenuSystem,
                XmeTraitGet ((XtPointer) xmRowColumnWidgetClass,
			     XmQTmenuSystem));
#endif
}

#endif

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass subclass)
#else
static void ClassPartInitialize(subclass)
WidgetClass subclass;
#endif
{
    XmtMenuWidgetClass sub = (XmtMenuWidgetClass)subclass;
    XmtMenuWidgetClass super = (XmtMenuWidgetClass) sub->core_class.superclass;

    /*
     * the XmtMenu widget doesn't have any class fields that need
     * inheritance, but the XmRowColumn does, and that widget does not
     * handle the inheritance itself.  So we do it here.
     */
#define inherit(field) \
    if ((XtProc)sub->field == _XtInherit) sub->field = super->field

     inherit(row_column_class.menuProcedures);
     inherit(row_column_class.armAndActivate);
     inherit(row_column_class.traversalHandler);

#undef inherit    
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void UpdateWidget(XmtSymbol s, XtPointer tag, XtArgVal value)
#else
static void UpdateWidget(s, tag, value)
XmtSymbol s;
XtPointer tag;
XtArgVal value;
#endif
{
    XmtMenuItem *item = (XmtMenuItem *)tag;

    /*
     * When the user changes the widget, the callback below will change
     * the symbol, and this callback will get called, and will go change
     * the widget again.  But it changes the widget to the same value,
     * and with notify False, it does not call the callbacks again, so
     * it is okay.  
     */
    XmToggleButtonSetState(item->w, value, False);
    if (value)
	item->type |= XmtMenuItemOn;
    else
	item->type &= ~XmtMenuItemOn;
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void ToggleCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void ToggleCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtMenuItem *item = (XmtMenuItem *)tag;
    XmToggleButtonCallbackStruct *tbcs = (XmToggleButtonCallbackStruct *)data;
    
    /* remember the current state */
    if (tbcs->set)
	item->type |= XmtMenuItemOn;
    else
	item->type &= ~XmtMenuItemOn;

    /*
     * if the toggle has a symbol, update the symbol.
     */
    if (item->symbol) {
	XmtSymbolSetValue(item->symbol, (XtArgVal)tbcs->set);
    }

    /*
     * if there is an alternate string (not a pixmap), add a callback
     * to switch strings  when the state changes.
     */
    if (item->alt_label && !(item->type & XmtMenuItemPixmap)) 
	XtVaSetValues(item->w,
		     XmNlabelString, (tbcs->set)?item->label1:item->label0,
		     XmNmnemonic,(tbcs->set)?item->alt_mnemonic:item->mnemonic,
		     NULL);
}

#if NeedFunctionPrototypes
static void CreateMenuItems(Widget w, XmtMenuItem *items, Cardinal num_items)
#else
static void CreateMenuItems(w, items, num_items)
Widget w;
XmtMenuItem *items;
Cardinal num_items;
#endif
{
    XmtMenuWidget mw = (XmtMenuWidget) w;
    XmtMenuItem *item;
    Arg args[10], submenu_args[10];
    int n, m;
    XmString accel_label = NULL;
    char namebuf[20], submenu_buf[20];
    char *name, *submenu_name;
    int numlabel, numpush, numtoggle, numsep, numsub;
    int i;
    static XrmQuark QBoolean = NULLQUARK;

    if (QBoolean == NULLQUARK) QBoolean = XrmPermStringToQuark(XtRBoolean);
    
    numlabel = numpush = numtoggle = numsep = numsub = 0;

    for(i=0; i < num_items; i++) {
	n = 0;
	item = &items[i];

	/* XXX
	 * We should do some error checking here.  A cascade button
	 * shouldn't have accelerators, for example, and a non-cascade
	 * button shouldn't have the help flag set, and there are probably
	 * more.
	 */

	switch(GetType(item)) {
	case XmtMenuItemLabel:
	    sprintf(namebuf, "label%d", numlabel);  numlabel++; break;
	case XmtMenuItemPushButton:
	    sprintf(namebuf, "button%d", numpush); numpush++; break;
	case XmtMenuItemToggleButton:
	    sprintf(namebuf, "toggle%d", numtoggle); numtoggle++; break;
	case XmtMenuItemCascadeButton:
	    sprintf(namebuf, "cascade%d", numsub); numsub++; break;
	case XmtMenuItemSeparator:
	case XmtMenuItemDoubleSeparator:
	    sprintf(namebuf, "separator%d", numsep); numsep++; break;
	}
	
	if (item->name) name = item->name;
	else name = namebuf;

	/* if there is a symbol name, bind it and get the initial state */
	if ((item->symbol_name) && (GetType(item) == XmtMenuItemToggleButton)){
	    Boolean state;
	    /*  XXX
	     * we should register a callback to free these symbols if
	     * the toggle buttons are destroyed.
	     */
	    item->symbol = XmtLookupSymbol(item->symbol_name);
	    if (!item->symbol ||
		(XmtSymbolTypeQuark(item->symbol) != QBoolean)) {
		XmtWarningMsg("XmtMenu", "symbolType",
			      "item '%s': symbol '%s' undefined or not of type Boolean",
			      item->label, item->symbol_name);
		item->symbol_name = NULL;
		item->symbol = NULL;
	    }
	    else {
		XmtSymbolGetValue(item->symbol, (XtArgVal *)&state);
		if (state) item->type |= XmtMenuItemOn;
		else item->type &= ~XmtMenuItemOn;
	    }
	}

	/*
	 * if there is an alternate label, and this is a toggle button,
	 * create the alternate XmString or pixmap
	 */
	if (item->alt_label && (GetType(item) == XmtMenuItemToggleButton)) {
	    if (item->type & XmtMenuItemPixmap) {
		XrmValue from, to;

		from.addr = item->alt_label;
		from.size = strlen(item->label) + 1;
		to.addr =(XPointer) &item->label1;
		to.size = sizeof(Pixmap);
		item->label1 = (XmString)XmUNSPECIFIED_PIXMAP;
		XtConvertAndStore(w, XtRString, &from, XtRPixmap, &to);
	    }
	    else item->label1 = XmtCreateLocalizedXmString(w, item->alt_label);
	}

	/* if there is a normal label, create the XmString or pixmap */
	if (item->label) {
	    if (item->type & XmtMenuItemPixmap) {
		XrmValue from, to;

		from.addr = item->label;
		from.size = strlen(item->label) + 1;
		to.addr =(XPointer) &item->label0;
		to.size = sizeof(Pixmap);
		item->label0 = (XmString)XmUNSPECIFIED_PIXMAP;
		XtConvertAndStore(w, XtRString, &from, XtRPixmap, &to);
	    }
	    else item->label0 = XmtCreateLocalizedXmString(w, item->label);
	}

	/*
	 * if this is a toggle button w/ an alternate string and mnemonic,
	 * set whichever is appopriate for the initial state.  Otherwise,
	 * just set the normal one.
	 * Also, for toggle buttons with two labels, turn the indicator off.
	 */
	if (item->alt_label && (GetType(item) == XmtMenuItemToggleButton)) {
	    XtSetArg(args[n], XmNindicatorOn, False); n++;
	    if (item->type & XmtMenuItemPixmap) {
		XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
		XtSetArg(args[n], XmNlabelPixmap, item->label0); n++;
		XtSetArg(args[n], XmNselectPixmap, item->label1); n++;
	    }
	    else {
		XtSetArg(args[n], XmNlabelString,
			 (item->type&XmtMenuItemOn)?item->label1:item->label0);
		n++;
		XtSetArg(args[n], XmNmnemonic, (item->type&XmtMenuItemOn)
			 ?item->alt_mnemonic
			 :item->mnemonic);
		n++;
	    }
	}
	else {
	    if (item->type & XmtMenuItemPixmap) {
		XtSetArg(args[n], XmNlabelType, XmPIXMAP); n++;
		XtSetArg(args[n], XmNlabelPixmap, item->label0); n++;
	    }
	    else {
		XtSetArg(args[n], XmNlabelString, item->label0); n++;
		XtSetArg(args[n], XmNmnemonic, item->mnemonic); n++;
	    }
	}

	if (item->accelerator_label) {
	    if (mw->menu.accelerator_font_tag)
		accel_label = XmStringCreate(item->accelerator_label,
					     mw->menu.accelerator_font_tag);
	    else
		accel_label = XmStringCreateSimple(item->accelerator_label);
	    XtSetArg(args[n], XmNacceleratorText, accel_label); n++;
	}

	if (item->accelerator) {
	    XtSetArg(args[n], XmNaccelerator, item->accelerator); n++;
	}

	switch (GetType(item)) {
	case XmtMenuItemLabel:
	    item->w = XmCreateLabel(w, name, args, n);
	    break;
	case XmtMenuItemDoubleSeparator:
	    XtSetArg(args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
	case XmtMenuItemSeparator:
	    item->w = XmCreateSeparator(w, name, args, n);
	    break;
	case XmtMenuItemPushButton:
	    item->w = XmCreatePushButton(w, name, args, n);
	    if (item->callback) {
		if (item->type & XmtMenuItemCallbackList)
		    XtAddCallbacks(item->w, XmNactivateCallback,
				   (XtCallbackList)item->callback);
		else
		    XtAddCallback(item->w, XmNactivateCallback,
				  item->callback, item->client_data);
	    }
	    break;
	case XmtMenuItemToggleButton:
	    XtSetArg(args[n], XmNset, (item->type&XmtMenuItemOn)?True:False);
	    n++;
	    item->w = XmCreateToggleButton(w, name, args, n);

	    /*
	     * Add a callback to track the button state and do other
	     * housekeeping.
	     */
	    XtAddCallback(item->w, XmNvalueChangedCallback,
			  ToggleCallback, (XtPointer)item);

	    /* Add any programmer specified callbacks */
	    if (item->callback) {
		if (item->type & XmtMenuItemCallbackList)
		    XtAddCallbacks(item->w, XmNvalueChangedCallback,
				   (XtCallbackList)item->callback);
		else
		    XtAddCallback(item->w, XmNvalueChangedCallback,
				  item->callback, item->client_data);
	    }

	    /*
	     * if the item has a symbol, add a callback to be notifed
	     * when the symbol value changes under us.  Note that
	     * ToggleCallback handles updating the symbol when the
	     * toggle changes
	     */
	    if (item->symbol) {
		XmtSymbolAddCallback(item->symbol,
				     UpdateWidget, (XtPointer) item);
	    }
	    break;
	case XmtMenuItemCascadeButton:
	    m = 0;
	    if (item->submenu) {
		XtSetArg(submenu_args[m], XmtNitems, item->submenu); m++;
		sprintf(submenu_buf, "item%d_pane", i);
		submenu_name = submenu_buf;
	    }
	    else if (item->submenu_name) {
		submenu_name = item->submenu_name;
	    }
	    else {
		submenu_name = NULL;
		XmtWarningMsg("XmtMenu", "noSubmenu",
			      "no submenu specified for menu item %s",
			      item->label);
	    }

	    if (submenu_name) {
		XtSetArg(submenu_args[m],XmNrowColumnType,XmMENU_PULLDOWN);m++;
		if (item->type & XmtMenuItemTearoff) {
#if XmVersion >= 1002
		    XtSetArg(submenu_args[m],
			     XmNtearOffModel, XmTEAR_OFF_ENABLED);
		    m++;
#else
		    XmtWarningMsg("XmtMenu", "tearoff",
		  "item '%s': tearoff menus not supported prior to Motif 1.2",
				  item->label);
#endif		    
		}
		item->submenu_pane = XmtCreateMenuPane(w, submenu_name,
						       submenu_args, m);
		XtSetArg(args[n], XmNsubMenuId, item->submenu_pane); n++;
		if (item->submenu == NULL)
		    item->submenu =
			((XmtMenuWidget)item->submenu_pane)->menu.items;
	    }
	    item->w = XmCreateCascadeButton(w, name, args, n);
	    break;
	default:  /* an error */
	    break;
	}

	XtManageChild(item->w);

	if ((GetType(item) == XmtMenuItemCascadeButton) &&
	    (item->type & XmtMenuItemHelp))
	    XtVaSetValues(w, XmNmenuHelpWidget, item->w, NULL);

	/* free the XmStrings */
	if (item->label
	    && !(item->alt_label && (GetType(item) == XmtMenuItemToggleButton))
	    && !(item->type & XmtMenuItemPixmap))
	    XmStringFree(item->label0);
	if (item->accelerator_label)
	    XmStringFree(accel_label);
    }
}

    
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
    XmtMenuWidget w = (XmtMenuWidget)init;
    int i;

#if XmVersion >= 1002
    /*
     * In Motif 1.2, the first thing we do is restore our core_class
     * translations field, which was munged by the initialize prehook
     * method we inherited (in the base class extension record) from
     * the XmRowColumn.  This restore would normally be done by a
     * corresponding posthook method, but we purposely did not inherit
     * that posthook, because if we have a posthook, then we cannot
     * recursively create submenus (as we need to do) -- the post hook
     * does not get restored soon enough, and the recursive creations
     * are done without ever calling this initialize procedure.
     */
#if NeedFunctionPrototypes
    extern void _XmRestoreCoreClassTranslations(Widget);
#else    
    extern void _XmRestoreCoreClassTranslations();
#endif
    _XmRestoreCoreClassTranslations(init);
#endif    
    
    /*
     * Copy the XmtNacceleratorFontTag string.  The only reason we have
     * to do this is so that programmers can query that resource.
     */
    if (w->menu.accelerator_font_tag)
	w->menu.accelerator_font_tag =
	    XtNewString(w->menu.accelerator_font_tag);

    if (w->menu.items != NULL) {
    /*
     * The items resource may be NULL terminated at this point, or it may
     * not be.  If num_items is -1 (ie. unspecified) then it had better be
     * NULL-terminated.  The resource converter delivers a NULL-terminated
     * array, but most programmers will probably use counted arrays.  It is
     * also possible that items is NULL-terminated at less than num_items.
     * So figure out num_items.  Note that NULL-terminated in this case
     * means that the type field is XmtMenuItemEnd.
     */
	if (w->menu.num_items == -1) {
	    /* if unspecified look for a NULL */
	    for(i=0; GetType(&w->menu.items[i]) != XmtMenuItemEnd; i++);
	    w->menu.num_items = i;
	}
	else {
	    /* if specified, look for a NULL anyway */
	    for(i=0;
		i<w->menu.num_items &&
		  GetType(&w->menu.items[i])!=XmtMenuItemEnd;
		i++);
	    w->menu.num_items = i;
	}
    }
    else {/* if no items are specified */
	w->menu.num_items = 0;
	XmtWarningMsg("XmtMenu", "noItems",
		      "no items specified for widget %s",
		      XtName(init));
    }
    
    CreateMenuItems(init, w->menu.items, w->menu.num_items);
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtMenuWidget mw = (XmtMenuWidget)w;

    XtFree(mw->menu.accelerator_font_tag);
}

#if NeedFunctionPrototypes
Widget XmtCreateMenubar(Widget w, String name,
			ArgList args, Cardinal num_args)
#else
Widget XmtCreateMenubar(w, name, args, num_args)
Widget w;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtMenuWidgetClass, w, args, num_args);
}

#if NeedFunctionPrototypes
static Widget CreateMenuPane(Widget w, String name,
		      ArgList args, Cardinal num_args, int type)
#else
static Widget CreateMenuPane(w, name, args, num_args, type)
Widget w;
String name;
ArgList args;
Cardinal num_args;
int type;
#endif
{
    ArgList pane_args, shell_args;
    Cardinal pane_num_args, shell_num_args;
    Arg new_args[10];
    String shell_name;
    Widget parent, popup, shell = NULL;
    Widget menu;
    int i, n;
    
    /*
     * if this menu pane is a submenu of a pulldown or a popup menu, then
     * the programmer probably passed the rowcol as the parent widget.  What
     * we really want is the menu shell that is the parent of that rowcol.
     * This is kludgy, but it is what the Motif XmCreatePulldownMenu() does.
     */
    if (XtParent(w) && XmIsMenuShell(XtParent(w)))
	parent = XtParent(w);
    else
	parent = w;
    
    /*
     * Now we need a menu shell.  A menubar or menupane with multiple
     * submenus need only have a single menu shell child, because only
     * one submenu can be popped up at a time.  Therefore, before we
     * go create a menu shell, we go see if one already exists as a popup
     * child of our parent.  We only make this check if the supplied parent
     * widget is a menubar or a menupane. Again, this is what Motif does.
     */
    if (XmIsRowColumn(w) &&
	((((XmRowColumnWidget)w)->row_column.type == XmMENU_BAR) ||
	 (((XmRowColumnWidget)w)->row_column.type == XmMENU_PULLDOWN) ||
	 (((XmRowColumnWidget)w)->row_column.type == XmMENU_POPUP))) {
	/* XXX
	 * Motif does the above test, which effectively means that popup
	 * menus never share shells.  I don't know if it is really needed.
	 */
	for (i = 0; i < parent->core.num_popups; i++) {
	    popup = parent->core.popup_list[i];
	    if (XmIsMenuShell(popup) && !popup->core.being_destroyed &&
		((XmMenuShellWidget)popup)->menu_shell.private_shell) {
		shell = popup;
		break;
	    }
	}
    }

    /*
     * if we can't share a shell, create one, and mark it as sharable.
     * We use the same shell naming convention as Motif does.
     */
    if (!shell) {
	/* put together a new arg list */
	n = 0;
	XtSetArg(new_args[n], XmNwidth, 5); n++;
	XtSetArg(new_args[n], XmNheight, 5); n++;
	XtSetArg(new_args[n], XmNallowShellResize, True); n++;
	XtSetArg(new_args[n], XmNoverrideRedirect, True); n++;
	shell_args = XtMergeArgLists(args, num_args, new_args, n);
	shell_num_args = num_args + n;
		     
	/* put together the new name */
	shell_name = XtMalloc(strlen(name) + 7);
	sprintf(shell_name, "popup_%s", name);

	/* create the shell */
	shell = XmCreateMenuShell(parent, shell_name,
				  shell_args, shell_num_args);

	/* free the name and arglist */
	XtFree(shell_name);
	XtFree((char *)shell_args);

	/* mark shell to allow sharing */
	((XmMenuShellWidget)shell)->menu_shell.private_shell = True;
    }

    /*
     * The default type of an XmtMenu widget is MENU_BAR.  We need to
     * explictly specify that this one is a pulldown or popup type.
     */
    n = 0;
    XtSetArg(new_args[n], XmNrowColumnType, type); n++;
    pane_args = XtMergeArgLists(args, num_args, new_args, n);
    pane_num_args = num_args + n;

    /* finally, create the menu */
    menu = XtCreateWidget(name, xmtMenuWidgetClass, shell,
			  pane_args, pane_num_args);

    /* and free the allocated arglist */
    XtFree((char *) pane_args);

    return menu;
}

#if NeedFunctionPrototypes
Widget XmtCreateMenuPane(Widget w, String name,
			 ArgList args, Cardinal num_args)
#else
Widget XmtCreateMenuPane(w, name, args, num_args)
Widget w;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    return CreateMenuPane(w, name, args, num_args, XmMENU_PULLDOWN);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtMenuPopupHandler(Widget w, XtPointer data, XEvent *event,Boolean *cont)
#else
void XmtMenuPopupHandler(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
#endif
{
    XmtMenuWidget menu = (XmtMenuWidget) data;
    XButtonPressedEvent *e = (XButtonPressedEvent *)event;
    int state;

    /* mask buttons out of modifiers/buttons state */
    state = e->state & (ShiftMask | ControlMask | LockMask | Mod1Mask |
			Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);

    if ((e->button == RC_PostButton(menu)) &&
	((state == RC_PostModifiers(menu)) ||
	 (RC_PostModifiers(menu) == AnyModifier))) {
	XmMenuPosition((Widget)menu, e);
	XtManageChild((Widget)menu);
    }
}

#if NeedFunctionPrototypes
Widget XmtCreatePopupMenu(Widget w, String name,
			  ArgList args, Cardinal num_args)
#else
Widget XmtCreatePopupMenu(w, name, args, num_args)
Widget w;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    Widget menu;

    menu = CreateMenuPane(w, name, args, num_args, XmMENU_POPUP);
    XtAddEventHandler(w, ButtonPressMask,
		      False, XmtMenuPopupHandler, (XtPointer)menu);
    return menu;
}

#if NeedFunctionPrototypes
Widget XmtCreateOptionMenu(Widget w, String name,
			   ArgList args, Cardinal num_args)
#else
Widget XmtCreateOptionMenu(w, name, args, num_args)
Widget w;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    Widget pane;
    Widget option;
    String pane_name;

    /* XXX
     * This is sort of a bad convenience routine.  It returns an XmRowColumn,
     * rather than an XmMenu widget.  It passes the same arglist to both
     * creation routines, and because they are similar widgets, there could
     * be arguments that are only intended for one.
     */

    pane_name = XtMalloc(strlen(name) + 6);
    sprintf(pane_name, "%s_pane", name);
    pane = XmtCreateMenuPane(w, pane_name, args, num_args);
    XtFree(pane_name);
    
    option = XmCreateOptionMenu(w, (String)name, args, num_args);
    XtVaSetValues(option, XmNsubMenuId, pane, NULL);
    return option;
}


#if NeedFunctionPrototypes
Widget XmtMenuItemGetSubmenu(XmtMenuItem *item)
#else
Widget XmtMenuItemGetSubmenu(item)
XmtMenuItem *item;
#endif
{
    if (GetType(item) != XmtMenuItemCascadeButton) {
	XmtWarningMsg("XmtMenuItemGetSubmenu", "typeMismatch",
		      "specified item is not a cascade button.");
	return NULL;
    }
    else
	return item->submenu_pane;
}

#if NeedFunctionPrototypes
Widget XmtMenuItemGetWidget(XmtMenuItem *item)
#else
Widget XmtMenuItemGetWidget(item)
XmtMenuItem *item;
#endif
{
    return item->w;
}

#if NeedFunctionPrototypes
Boolean XmtMenuItemGetState(XmtMenuItem *item)
#else
Boolean XmtMenuItemGetState(item)
XmtMenuItem *item;
#endif
{
    if (GetType(item) != XmtMenuItemToggleButton) {
	XmtWarningMsg("XmtMenuItemGetState", "typeMismatch",
		      "specified item is not a toggle button.");
	return False;
    }
    else return ((item->type & XmtMenuItemOn) != 0);
}

#if NeedFunctionPrototypes
void XmtMenuItemSetState(XmtMenuItem *item, XmtWideBoolean state,
			 XmtWideBoolean notify)
#else
void XmtMenuItemSetState(item, state, notify)
XmtMenuItem *item;
int state;
int notify;
#endif
{
    if (GetType(item) != XmtMenuItemToggleButton) {
	XmtWarningMsg("XmtMenuItemSetState", "typeMismatch",
		      "specified item is not a toggle button.");
    }
    else {
	if (item->w)
	    XmToggleButtonSetState(item->w, state, notify);
	if (state) item->type |= XmtMenuItemOn;
	else item->type &= ~XmtMenuItemOn;
    }
}

#if NeedFunctionPrototypes
void XmtMenuItemSetSensitivity(XmtMenuItem *item, XmtWideBoolean sensitive)
#else
void XmtMenuItemSetSensitivity(item, sensitive)
XmtMenuItem *item;
int sensitive;
#endif
{
    if (sensitive) {
	if (item->sensitive == 0)  /* if already sensitive */
	    return;
	item->sensitive--;
	if ((item->sensitive == 0) && (item->w))  /* if newly sensitive */
	    XtSetSensitive(item->w, True);
    }
    else {
	if ((item->sensitive == 0) && item->w)  /* if currently sensitive */
	    XtSetSensitive(item->w, False);
	item->sensitive++;
    }
}

#if NeedFunctionPrototypes
static XmtMenuItem *GetMenuItem(XmtMenuWidget mw, XrmName quark)
#else
static XmtMenuItem *GetMenuItem(mw, quark)
XmtMenuWidget mw;
XrmName quark;
#endif
{
    int i;
    XmtMenuItem *item, *subitem;

    /*
     * check each of the items in the menu.
     * Note that we use the widget name rather than item->name, because
     * comparing quarks is faster.
     */
    for(i=0; i < mw->menu.num_items; i++) {
	item = &mw->menu.items[i];
	if (item->w && (item->w->core.xrm_name == quark))
	    return item;
    }

    /* if no item matched, recursively check any submenus */
    for(i=0; i < mw->menu.num_items; i++) {
	item = &mw->menu.items[i];
	if (GetType(item) == XmtMenuItemCascadeButton) {
	    subitem = GetMenuItem((XmtMenuWidget)item->submenu_pane, quark);
	    if (subitem) return subitem;
	}
    }

    /* if no subitem matched, give up */
    return NULL;
}


#if NeedFunctionPrototypes
XmtMenuItem *XmtMenuGetMenuItem(Widget w, StringConst name)
#else
XmtMenuItem *XmtMenuGetMenuItem(w, name)
Widget w;
StringConst name;
#endif
{
    XmtMenuWidget mw = (XmtMenuWidget) w;
    XmtMenuItem *item;
    
    XmtAssertWidgetClass(w, xmtMenuWidgetClass, "XmtMenuGetMenuItem");
    item = GetMenuItem(mw, XrmStringToQuark(name));
    return item;
}

#if NeedFunctionPrototypes
static void CheckSensitivity(Widget w, XtCallbackProc proc, Boolean sensitive)
#else
static void CheckSensitivity(w, proc, sensitive)
Widget w;
XtCallbackProc proc;
Boolean sensitive;
#endif
{
    XmtMenuWidget mw = (XmtMenuWidget) w;
    XmtMenuItem *item;
    int i;

    if (!mw) return;
    
    for(i=0; i < mw->menu.num_items; i++) {
	item = &mw->menu.items[i];
	if (GetType(item) == XmtMenuItemCascadeButton)
	    CheckSensitivity(item->submenu_pane, proc, sensitive);
	else {
	    if (((item->type & XmtMenuItemCallbackList) &&
		 (XmtCallbackCheckList((XtCallbackList) item->callback,
				       (XmtProcedure) proc))) ||
		(!(item->type & XmtMenuItemCallbackList) &&
		 (item->callback == proc)))
		XmtMenuItemSetSensitivity(item, sensitive);
	}
    }
}

#if NeedFunctionPrototypes
void XmtMenuInactivateProcedure(Widget w, XtCallbackProc proc)
#else
void XmtMenuInactivateProcedure(w, proc)
Widget w;
XtCallbackProc proc;
#endif
{
    XmtAssertWidgetClass(w, xmtMenuWidgetClass, "XmtMenuInactivateProcedure");
    CheckSensitivity(w, proc, False);
}

#if NeedFunctionPrototypes
void XmtMenuActivateProcedure(Widget w, XtCallbackProc proc)
#else
void XmtMenuActivateProcedure(w, proc)
Widget w;
XtCallbackProc proc;
#endif
{
    XmtAssertWidgetClass(w, xmtMenuWidgetClass, "XmtMenuActivateProcedure");
    CheckSensitivity(w, proc, True);
}

