/* 
 * Motif Tools Library, Version 3.1
 * $Id: Chooser.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Chooser.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <Xmt/XmtP.h>
#include <Xmt/ChooserP.h>
#include <Xmt/Symbols.h>
#include <Xmt/Converters.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Layout.h> /* for XmtNlayoutSensitive resource */
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#if XmVersion >= 2000
#include <Xm/ComboBox.h>
#endif

static XtResource resources[] = {
{XmtNchooserType, XmtCChooserType, XmtRXmtChooserType,
     sizeof(XmtChooserType), XtOffsetOf(XmtChooserRec, chooser.type),
     XtRImmediate, (XtPointer)XmtChooserRadioBox},
{XmtNnumItems, XmtCNumItems, XtRInt,
     sizeof(int), XtOffsetOf(XmtChooserRec, chooser.num_items),
     XtRImmediate, (XtPointer) -1},
{XmtNstrings, XmtCStrings, XmtRStringList,
     sizeof(String *), XtOffsetOf(XmtChooserRec, chooser.strings),
     XmtRStringList, NULL},
{XmtNpixmaps, XmtCPixmaps, XmtRPixmapList,
     sizeof(Pixmap *), XtOffsetOf(XmtChooserRec, chooser.pixmaps),
     XmtRPixmapList, NULL},
{XmtNselectPixmaps, XmtCSelectPixmaps, XmtRPixmapList,
     sizeof(Pixmap *), XtOffsetOf(XmtChooserRec, chooser.select_pixmaps),
     XmtRPixmapList, NULL},
{XmtNinsensitivePixmaps, XmtCInsensitivePixmaps, XmtRPixmapList,
     sizeof(Pixmap *), XtOffsetOf(XmtChooserRec, chooser.insensitive_pixmaps),
     XmtRPixmapList, NULL},
{XmtNvalues, XmtCValues, XtRPointer,
     sizeof(XtPointer), XtOffsetOf(XmtChooserRec, chooser.values),
     XtRImmediate, NULL},
{XmtNvalueStrings, XmtCValueStrings, XmtRStringList,
     sizeof(String *), XtOffsetOf(XmtChooserRec, chooser.value_strings),
     XmtRStringList, NULL},
{XmtNvalueType, XmtCValueType, XtRString,
     sizeof(String), XtOffsetOf(XmtChooserRec, chooser.value_type),
     XtRString, NULL},
{XmtNvalueSize, XmtCValueSize, XtRInt,
     sizeof(int), XtOffsetOf(XmtChooserRec, chooser.value_size),
     XtRImmediate, (XtPointer) 0},
{XmtNfontList, XmCFontList, XmRFontList,
     sizeof(XmFontList), XtOffsetOf(XmtChooserRec, chooser.font_list),
     XtRImmediate, NULL},
#if XmVersion >= 2000
{XmtNrenderTable, XmCRenderTable, XmRRenderTable,
     sizeof(XmRenderTable), XtOffsetOf(XmtChooserRec, chooser.render_table),
     XtRImmediate, (XtPointer) NULL},
#endif
{XmtNvisibleItems, XmtCVisibleItems, XtRInt,
     sizeof(int), XtOffsetOf(XmtChooserRec, chooser.visible_items),
     XtRImmediate, (XtPointer) 8},
{XmtNstate, XmtCState, XtRInt,
     sizeof(int), XtOffsetOf(XmtChooserRec, chooser.state),
     XtRImmediate, (XtPointer) 0},
{XmtNsymbolName, XmtCSymbolName, XtRString,
     sizeof(String), XtOffsetOf(XmtChooserRec, chooser.symbol_name),
     XtRImmediate, NULL},
{XmtNlabelType, XmtCLabelType, XmRLabelType,
     sizeof(unsigned char), XtOffsetOf(XmtChooserRec, chooser.label_type),
     XtRImmediate, (XtPointer)XmSTRING},
{XmtNvalueChangedCallback, XtCCallback, XtRCallback,
     sizeof(XtCallbackList), XtOffsetOf(XmtChooserRec, chooser.callback),
     XtRCallback, NULL},
{XmtNitemWidgets, XtCReadOnly, XtRWidgetList,
     sizeof(WidgetList), XtOffsetOf(XmtChooserRec, chooser.item_widgets),
     XtRImmediate, NULL},
/*
 * override defaults for some RowColumn resources
 */
{XmNadjustLast, XmCAdjustLast, XtRBoolean,
     sizeof(Boolean), XtOffsetOf(XmtChooserRec, row_column.adjust_last),
     XtRImmediate, (XtPointer) False},
{XmNpacking, XmCPacking, XmRPacking,
     sizeof(unsigned char), XtOffsetOf(XmtChooserRec, row_column.packing),
     XtRImmediate, (XtPointer) XmPACK_COLUMN},

};

#if NeedFunctionPrototypes
static void ClassInitialize(void);
static void ClassPartInitialize(WidgetClass);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
#else
static void ClassInitialize();
static void ClassPartInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
#endif

#define Superclass (&xmRowColumnClassRec)

externaldef(xmtchooserclassrec) XmtChooserClassRec xmtChooserClassRec = {  {
    /* core_class fields */
    /* superclass         */    (WidgetClass) Superclass,
    /* class_name         */    "XmtChooser",
    /* widget_size        */    sizeof(XmtChooserRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */    ClassPartInitialize,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    XtExposeCompressMaximal,
    /* compress_enterleave*/    FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    XtInheritTranslations,
    /* query_geometry     */    XtInheritQueryGeometry,
    /* display_accelerator*/    XtInheritDisplayAccelerator,
    /* extension          */    NULL
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
    /* extension	  */	NULL
  }, 
  { /* row column class record        */
    /* proc to interface with widgets */   XmInheritMenuProc,
    /* proc to arm&activate menu      */   XmInheritArmAndActivate,
    /* traversal handler              */   XmInheritMenuTraversalProc,
    /* extension                      */   NULL,
  },
  { /* XmtChooser          */
    /* extension	  */    NULL
  }
};

externaldef(xmtchooserwidgetclass) WidgetClass xmtChooserWidgetClass =
    (WidgetClass)&xmtChooserClassRec;


#if NeedFunctionPrototypes
static void SetStateOnWidgets(XmtChooserWidget cw)
#else
static void SetStateOnWidgets(cw)
XmtChooserWidget cw;
#endif
{
    int i, bit;
    
    /*
     * update the subwidgets to reflect the current state.  Don't touch
     * the symbol value or call callbacks.
     */

    if (cw->chooser.num_items == 0) return;
    
    if (cw->chooser.type == XmtChooserRadioList) {
	XmListDeselectAllItems(cw->chooser.list);
	XmListSelectPos(cw->chooser.list, cw->chooser.state+1, False);
    }
#if XmVersion >= 2000
    else if (cw->chooser.type == XmtChooserComboBox) {
        XtVaSetValues(cw->chooser.combo_box,
                      XmNselectedPosition,      cw->chooser.state+1,
                      NULL);
    }
#endif
    else if (cw->chooser.type == XmtChooserCheckList) {
	XmListDeselectAllItems(cw->chooser.list);
	for(i=0, bit = 1; i < cw->chooser.num_items; i++, bit = bit << 1)
	    if (cw->chooser.state & bit)
		XmListSelectPos(cw->chooser.list, i+1, False);
    } 
    else if (cw->chooser.type == XmtChooserOption) {
	/* XXX this may not work before Motif 1.1.4 */
	XtVaSetValues(cw->chooser.option,
		      XmNmenuHistory,
		      cw->chooser.item_widgets[cw->chooser.state],
		      NULL);
    }
    else if ((cw->chooser.type == XmtChooserCheckBox) ||
	     (cw->chooser.type == XmtChooserCheckPalette)) {
	for(i=0, bit = 1; i < cw->chooser.num_items; i++, bit = bit << 1)
	    if (cw->chooser.state & bit)
		XmToggleButtonSetState(cw->chooser.item_widgets[i],
				       True, False);
	    else
		XmToggleButtonSetState(cw->chooser.item_widgets[i],
				       False, False);
    }
    else if ((cw->chooser.type == XmtChooserRadioBox) ||
	     (cw->chooser.type == XmtChooserRadioPalette)) {
	for(i=0; i < cw->chooser.num_items; i++)
	    XmToggleButtonSetState(cw->chooser.item_widgets[i], False, False);
	XmToggleButtonSetState(cw->chooser.item_widgets[cw->chooser.state],
			       True, False);
    }
    else {  /* a button box; it doesn't have state */
	;
    }
}    

/* ARGSUSED */
#if NeedFunctionPrototypes
static void ChooserCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void ChooserCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) tag;
    XmListCallbackStruct *listdata = (XmListCallbackStruct *) data;
#if XmVersion >= 2000
    XmComboBoxCallbackStruct *combodata = (XmComboBoxCallbackStruct *) data;
#endif
    XmtChooserCallbackStruct call_data;
    int i;

    /*
     * Figure out and record the new state of the Chooser.
     * Enforce RadioBox behavior where required.
     * Enforce sensitivity where required (in list widgets).
     * Update the symbol value, if a symbol was specified.
     * Invoke the callback that was registered with this Chooser, if it exists.
     */

    /*
     * if a list, check sensitivity.  If the item that was
     * clicked on is insensitive, restore the old state and return
     */
    if ((cw->chooser.type == XmtChooserRadioList) ||
	(cw->chooser.type == XmtChooserCheckList)) {
	if (cw->chooser.insensitive & (1 << (listdata->item_position-1))) {
	    SetStateOnWidgets(cw);
	    return;
	}
    }
#if XmVersion >= 2000
    else if (cw->chooser.type == XmtChooserComboBox) {
	if (cw->chooser.insensitive & (1 << (combodata->item_position-1))) {
	    SetStateOnWidgets(cw);
	    return;
	}
    }
#endif
    
    if (cw->chooser.type == XmtChooserRadioList) {
	/* list  positions start numbering from 1 */
	call_data.item = call_data.state = cw->chooser.state =
	    listdata->item_position - 1;
    }
#if XmVersion >= 2000
    else if (cw->chooser.type == XmtChooserComboBox) {
	/* list  positions start numbering from 1 */
	call_data.item = call_data.state = cw->chooser.state =
	    combodata->item_position - 1;
    }
#endif
    else if (cw->chooser.type == XmtChooserCheckList) {
	cw->chooser.state = 0;
	for(i=0; i < listdata->selected_item_count; i++)
	    cw->chooser.state |= (1<<(listdata->selected_item_positions[i]-1));
	call_data.state = cw->chooser.state;
	call_data.item = listdata->item_position;
    }
    else {
	/* figure out which button was pressed */
	for(i = 0; i < cw->chooser.num_items; i++)
	    if (cw->chooser.item_widgets[i] == w) break;
	if (i == cw->chooser.num_items) return;  /* just in case */
	call_data.item = i;
	if ((cw->chooser.type == XmtChooserCheckBox) ||
	    (cw->chooser.type == XmtChooserCheckPalette)) {
	    cw->chooser.state ^= (1 << i);
	    call_data.state = cw->chooser.state;
	}
	else if ((cw->chooser.type == XmtChooserRadioBox) ||
	    (cw->chooser.type == XmtChooserRadioPalette)) {
	    /*
	     * if user clicked on an already selected item, reselect it 
	     * otherwise unselect the old selected item.
	     */
	    if (cw->chooser.state == i)
		XmToggleButtonSetState(w, True, False);
	    else
		XmToggleButtonSetState(cw->chooser.item_widgets
				       [cw->chooser.state],
				       False,False);
	    cw->chooser.state = call_data.state = i;
	}
	else if (cw->chooser.type == XmtChooserOption)
	    cw->chooser.state = call_data.state = i;
	else  /* button box */
	    call_data.state = -1;
    }

    /*
     * update the symbol value, if we have one.
     * set a flag that tells our own symbol notify callback that the
     * widgets are already in the correct state and it need not do
     * anything.
     */
    if (cw->chooser.symbol) {
	cw->chooser.ignore_symbol_notify = True;
	XmtSymbolSetValue(cw->chooser.symbol, (XtArgVal)cw->chooser.state);
	cw->chooser.ignore_symbol_notify = False;
    }

    /*
     * figure out the call_data.valuep, if we have values set.
     */
    if (cw->chooser.values)
	call_data.valuep = (XtPointer) &((char *)cw->chooser.values)
	    [call_data.item * cw->chooser.value_size];
    else
	call_data.valuep = NULL;

    /* invoke the callbacks registered for this Chooser */
    XtCallCallbackList((Widget)cw, cw->chooser.callback,(XtPointer)&call_data);
}

#if NeedFunctionPrototypes
static void SetStateOnSymbol(XmtChooserWidget cw)
#else
static void SetStateOnSymbol(cw)
XmtChooserWidget cw;
#endif
{
    /*
     * set the symbol state.  This will cause the notify callbacks to
     * be invoked, which in our case will call SetStateOnWidgets.
     * So if you call this function, don't call SetStateOnWidgets.
     */
    if (cw->chooser.symbol)
	XmtSymbolSetValue(cw->chooser.symbol, (XtArgVal)cw->chooser.state);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void ValueChanged(XmtSymbol s, XtPointer tag, XtArgVal value)
#else
static void ValueChanged(s, tag, value)
XmtSymbol s;
XtPointer tag;
XtArgVal value;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget)tag;

    /*
     * update the widget's internal and  displayed state when the symbol value
     * changes from underneath us.  Note that we do not call the
     * valueChangedCallback; this is only called when the user changes the
     * state by interacting with the widget.  Also, it prevents us from
     * getting into a loop.
     *
     * We don't do anything if the ignore flag is set.  This is the case
     * when the user interacts with the widget to change the state--the widget
     * state and the subwidgets are already correct, the symbol is updated
     * and the notify callbacks are called to notify anyone else who cares
     * about the symbol.
     */

    if (cw->chooser.ignore_symbol_notify) return;
    
    cw->chooser.state = (int) value;
    SetStateOnWidgets(cw);
}


#if NeedFunctionPrototypes
static void BindSymbol(XmtChooserWidget cw)
#else
static void BindSymbol(cw)
XmtChooserWidget cw;
#endif
{
    if (cw->chooser.symbol_name) {
	cw->chooser.symbol = XmtLookupSymbol(cw->chooser.symbol_name);
	if (cw->chooser.symbol) {
	    if (XmtSymbolSize(cw->chooser.symbol)!=sizeof(cw->chooser.state)) {
		XmtWarningMsg("XmtChooser", "symbolSize",
			     "%s: symbol '%s' has wrong size;\n\tshould be int.",
			      XtName((Widget)cw), cw->chooser.symbol_name);
		cw->chooser.symbol = NULL;
	    }
	    else {
		XmtSymbolAddCallback(cw->chooser.symbol,
				     ValueChanged, (XtPointer)cw);
		XmtSymbolGetValue(cw->chooser.symbol,
				  (XtArgVal*) &cw->chooser.state);
	    }
	}
	else {
	    XmtWarningMsg("XmtChooser", "badSymbol",
			  "%s: symbol '%s' is undefined.",
			  XtName((Widget)cw), cw->chooser.symbol_name);
	}
    }
    else
	cw->chooser.symbol = NULL;
}


#if NeedFunctionPrototypes
static void ReleaseSymbol(XmtChooserWidget cw)
#else
static void ReleaseSymbol(cw)
XmtChooserWidget cw;
#endif
{
    if (cw->chooser.symbol) {
	XmtSymbolRemoveCallback(cw->chooser.symbol,
				ValueChanged, (XtPointer)cw);
	cw->chooser.symbol = NULL;
    }
}

#if NeedFunctionPrototypes
static void CheckType(XmtChooserWidget cw)
#else
static void CheckType(cw)
XmtChooserWidget cw;
#endif
{
    /* if this is a check box, then we can't have values */
    if ((cw->chooser.type == XmtChooserCheckBox) ||
	(cw->chooser.type == XmtChooserCheckPalette) ||
	(cw->chooser.type == XmtChooserCheckList)) {
	XmtWarningMsg("XmtChooser", "badType",
		      "%s: XmtNvalues and XmtNvalueStrings resources ignored\n\twhen XmtNchooserType is CheckBox, CheckPalette or CheckList",
		      XtName((Widget)cw));
	cw->chooser.values = NULL;
	cw->chooser.value_strings = NULL;
	return;
    }

    /* also check that value type and size are set */
    if (!cw->chooser.value_type  || !cw->chooser.value_size) {
	XmtWarningMsg("XmtChooser", "badValues",
		      "%s: can't set XmtNvalues or XmtNvalueStrings without\n\talso setting XmtNvalueType and XmtNvalueSize resources",
		      XtName((Widget)cw));
	cw->chooser.values = NULL;
	cw->chooser.value_strings = NULL;
    }
}

#if NeedFunctionPrototypes
static void CopyValues(XmtChooserWidget cw)     
#else
static void CopyValues(cw)
XmtChooserWidget cw;     
#endif
{
    char *values;
    size_t size = cw->chooser.num_items * cw->chooser.value_size;
    
    /* if XmtNvalues is set, be sure to ignore XmtNvalueStrings */
    cw->chooser.value_strings = NULL;

    values = XtMalloc(size);
    memcpy(values, (char *)cw->chooser.values, size);
    cw->chooser.values = (XtPointer) values;
}

#if NeedFunctionPrototypes
static void ConvertValueStrings(XmtChooserWidget cw)
#else
static void ConvertValueStrings(cw)
XmtChooserWidget cw;
#endif
{
    char *values;
    XrmValue from, to;
    Boolean status;
    int i;
    
    values = XtCalloc(cw->chooser.num_items, cw->chooser.value_size);

    /* String values are a special case; no conversion required */
    if (strcmp(cw->chooser.value_type, XtRString) == 0) {
	for(i=0; i < cw->chooser.num_items; i++)
	    ((String *)values)[i] = cw->chooser.value_strings[i];
    }
    else{
	for(i=0; i < cw->chooser.num_items; i++) {
	    from.addr = cw->chooser.value_strings[i];
	    from.size = strlen(cw->chooser.value_strings[i]) + 1;
	    to.addr = &values[i * cw->chooser.value_size];
	    to.size = cw->chooser.value_size;
	    status = XtConvertAndStore((Widget)cw, XtRString, &from,
				       cw->chooser.value_type, &to);
	    if (!status) {
		XmtWarningMsg("XmtChooser", "convert",
			      "%s: conversion failed for item %d of XmtNvalueStrings",
			      XtName((Widget)cw), i);
	    }
	    else if (to.size != cw->chooser.value_size) {
		XmtWarningMsg("XmtChooser", "badSize",
			      "%s: size of converted values not equal to XmtNvalueSize",
			      XtName((Widget)cw));
		XtFree(values);
		values = NULL;
		break;
	    }
	}
    }
    
    cw->chooser.values = values;
    /*
     * we set value_strings to NULL, because we're not making a copy of them,
     * and so we'll notice a change if the programmer specifies the same
     * array with different values in it.  This means that querying this
     * resource doesn't make sense.
     */
    cw->chooser.value_strings = NULL;
}


#if NeedFunctionPrototypes
static void ClassInitialize(void)
#else
static void ClassInitialize()
#endif
{
    static String names[] = {
	"ButtonBox", "CheckBox", "CheckList", "CheckPalette",
#if XmVersion >= 2000
        "ComboBox",
#endif
	"Option", "RadioBox", "RadioList", "RadioPalette"
    };
    static Cardinal values[] = {
	XmtChooserButtonBox, XmtChooserCheckBox, XmtChooserCheckList,
	XmtChooserCheckPalette,
#if XmVersion >= 2000
        XmtChooserComboBox,
#endif
        XmtChooserOption, XmtChooserRadioBox,
	XmtChooserRadioList, XmtChooserRadioPalette
    };
    static String prefixes[] = {"Xmt", "Chooser", NULL};

    XmtRegisterStringListConverter();
    XmtRegisterEnumConverter(XmtRXmtChooserType, names, values,
			     XtNumber(names), prefixes);
}

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass subclass)
#else
static void ClassPartInitialize(subclass)
WidgetClass subclass;
#endif
{
    XmtChooserWidgetClass sub = (XmtChooserWidgetClass)subclass;
    XmtChooserWidgetClass sup =
	(XmtChooserWidgetClass)sub->core_class.superclass;

    /*
     * the XmtChooser widget doesn't have any class fields that need
     * inheritance, but the XmRowColumn does, and that widget does not
     * handle the inheritance itself.  So we do it here.
     */
#define inherit(field) \
    if ((XtProc)sub->field == _XtInherit) sub->field = sup->field

     inherit(row_column_class.menuProcedures);
     inherit(row_column_class.armAndActivate);
     inherit(row_column_class.traversalHandler);

#undef inherit    
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
    XmtChooserWidget nw = (XmtChooserWidget) init;
    String *array;
    Pixmap *p;
    XmString *item_labels = NULL;
    char namebuf[200];
    Arg args[10];
    int i,j;
#if XmVersion >= 2000
    int len;
    short num_columns;
#endif

    /* lists can't have pixmaps */
#if XmVersion < 2000
    if ((nw->chooser.label_type == XmPIXMAP) &&
	((nw->chooser.type == XmtChooserRadioList) ||
	 (nw->chooser.type == XmtChooserCheckList))) {
#else /* Motif 2.0 or later */
    if ((nw->chooser.label_type == XmPIXMAP) &&
	((nw->chooser.type == XmtChooserRadioList) ||
	 (nw->chooser.type == XmtChooserCheckList) ||
         (nw->chooser.type == XmtChooserComboBox))) {
#endif
	nw->chooser.label_type = XmSTRING;
	XmtWarningMsg("XmtChooser", "pixmapType",
		      "%s: XmtNlabelType must be XmSTRING for XmList types.",
		      XtName((Widget)nw));
    }

    /*
     * Palettes have zero spacing.  This is a dynamic default resource,
     * so we can't just override it.  RowCol may do some special processing
     * on it, so we don't just set it directly.
     */
    if ((nw->chooser.type == XmtChooserRadioPalette) ||
	(nw->chooser.type == XmtChooserCheckPalette))
	XtVaSetValues(init, XmNspacing, 0, NULL); 
    
#if XmVersion < 2000
    /* get the default font from an ancestor font list if none set. */
    if (nw->chooser.font_list == NULL)
        nw->chooser.font_list = _XmGetDefaultFontList(init, XmBUTTON_FONTLIST);

    /* make a private copy of the font list */
    nw->chooser.font_list = XmFontListCopy(nw->chooser.font_list);

#else /* Motif 2.0 or later */

    if (!nw->chooser.render_table) {
        if (nw->chooser.font_list) {
            /*
             * There is a fontList, so copy it to the renderTable.
             * NOTE: This code relies on the fact that there is no
             *       disitinction between a fontList and a renderTable
             *       in Motif 2.0.
             */
            nw->chooser.render_table = XmFontListCopy(nw->chooser.font_list);
        }
        else {
           /*
            * Nothing was specified, so look up the default
            * renderTable.
            */
            nw->chooser.render_table = 
                XmeGetDefaultRenderTable(init, XmBUTTON_FONTLIST);

            /* Make a copy of the renderTable. */
            nw->chooser.render_table =
                XmRenderTableCopy(nw->chooser.render_table, NULL, 0);
        }
    }
    else {
        /* Make a copy of the renderTable. */
        nw->chooser.render_table =
            XmRenderTableCopy(nw->chooser.render_table, NULL, 0);
    }
#endif

    /*
     * if we're insensitve, set layoutSensitive, in case we're a child
     * of a layout and have a caption
     */
    if (!nw->core.sensitive || !nw->core.ancestor_sensitive)
	XtVaSetValues(init, XmtNlayoutSensitive, False, NULL);
    
    /*
     * Count the strings or pixmaps.
     * The strings resource may be NULL terminated at this point, or it may
     * not be.  If num_items is -1 (ie. unspecified) then it had better be
     * NULL-terminated.  The resource converter delivers a NULL-terminated
     * array, but most programmers will probably use counted arrays.  It is
     * also possible that strings is NULL-terminated at less than num_items.
     * So figure out num_items.
     */
    if (nw->chooser.label_type == XmSTRING)
	array = nw->chooser.strings;
    else
	array = (String *)nw->chooser.pixmaps;
    
    if (array != NULL) {
	if (nw->chooser.num_items == -1) /* if unspecified look for a NULL */
	    for(i=0; array[i]; i++);
	else                         /* if specified, look for a NULL anyway */
	    for(i=0; i < nw->chooser.num_items && array[i]; i++);
	nw->chooser.num_items = i;
    }
    else /* if no strings are specified */
	nw->chooser.num_items = 0;
	
    /*
     * Now copy the array of strings and the arrays of pixmaps.
     * We copy all arrays, regardless of mode.
     * We copy the array, but not the strings in it.
     * While copying the selected and insensitive pixmaps we count them, too.
     */
    if (nw->chooser.strings != NULL) {
	array = (String *)XtMalloc(nw->chooser.num_items*sizeof(String));
	for(i=0; i < nw->chooser.num_items; i++) {
	    if (nw->chooser.strings[i] == NULL) break;
	    array[i] = nw->chooser.strings[i];
	}
	for(; i < nw->chooser.num_items; i++) array[i] = NULL;
	nw->chooser.strings = array;
    }

    if (nw->chooser.pixmaps != NULL) {
	p = (Pixmap *)XtMalloc(nw->chooser.num_items*sizeof(Pixmap));
	for(i=0; i < nw->chooser.num_items; i++) {
	    if (nw->chooser.pixmaps[i] == None) break;
	    p[i] = nw->chooser.pixmaps[i];
	}
	for(; i < nw->chooser.num_items; i++) p[i] = XmUNSPECIFIED_PIXMAP;
	nw->chooser.pixmaps = p;
    }

    if (nw->chooser.select_pixmaps != NULL) {
	p = (Pixmap *)XtMalloc(nw->chooser.num_items*sizeof(Pixmap));
	for(i=0; i < nw->chooser.num_items; i++) {
	    if (nw->chooser.select_pixmaps[i] == None) break;
	    p[i] = nw->chooser.select_pixmaps[i];
	}
	for(; i < nw->chooser.num_items; i++) p[i] = XmUNSPECIFIED_PIXMAP;
	nw->chooser.select_pixmaps = p;
    }

    if (nw->chooser.insensitive_pixmaps != NULL) {
	p = (Pixmap *)XtMalloc(nw->chooser.num_items*sizeof(Pixmap));
	for(i=0; i < nw->chooser.num_items; i++) {
	    if (nw->chooser.insensitive_pixmaps[i] == None) break;
	    p[i] = nw->chooser.insensitive_pixmaps[i];
	}
	for(; i < nw->chooser.num_items; i++) p[i] = XmUNSPECIFIED_PIXMAP;
	nw->chooser.insensitive_pixmaps = p;
    }

    /*
     * if values or valueStrings is set, make sure that the type is okay,
     * and make sure we have a size and type
     */
    if (nw->chooser.values || nw->chooser.value_strings)
	CheckType(nw);

    /*
     * if values is set, copy the array.
     * otherwise, if values is not set, but valueStrings is,
     * convert the strings, and store in values.  We don't copy
     * valueStrings, because we'll never need to referece it again,
     * but we set it to NULL, so we can detect when it changes.
     */
    if (nw->chooser.values) CopyValues(nw);
    else if (nw->chooser.value_strings) ConvertValueStrings(nw);

    /* create the label XmStrings in string mode */
    if (nw->chooser.label_type == XmSTRING) {
	item_labels = (XmString *)
	    XtMalloc(nw->chooser.num_items*sizeof(XmString));
	for(i = 0; i < nw->chooser.num_items; i++)
	    item_labels[i] = XmtCreateLocalizedXmString((Widget) nw, nw->chooser.strings[i]);
    }
    
    /*
     * Create the widgets.  Option menus are handled separately.
     */
    if (nw->chooser.type == XmtChooserOption) {
	/*
	 * In order to create an Option menu, we have to create the
	 * menu pane first before the cascade button and label. 
	 * The menu shell is automatically created, and
	 * its name is not publicly documented.
	 */
	i = 0;
	nw->chooser.pane = XmCreatePulldownMenu(init, "pane", NULL, 0);
	XtSetArg(args[i], XmNsubMenuId, nw->chooser.pane); i++;
	nw->chooser.option = XmCreateOptionMenu(init, "option", args, i);
#if XmVersion >= 1002
	/*
	 * In Motif 1.2.0, a NULL option label string gives an option menu
	 * with some bogus widget name as the label, so we just unmanage
	 * the label altogether.  We don't do this in 1.1.4, because
	 * with ICS 1.1.4, at least, the option menu doesn't shrink; it
	 * continues to allocate space for the label gadget.
	 */
	XtUnmanageChild(XmOptionLabelGadget(nw->chooser.option));
#endif
	XtManageChild(nw->chooser.option);
#if XmVersion < 2000
	XtVaSetValues(XmOptionButtonGadget(nw->chooser.option),
		      XmNfontList, nw->chooser.font_list,
		      NULL);
#else /* Motif 2.0 or later */
	XtVaSetValues(XmOptionButtonGadget(nw->chooser.option),
		      XmNrenderTable, nw->chooser.render_table,
		      NULL);
#endif
    }
    else if ((nw->chooser.type == XmtChooserRadioList) ||
	     (nw->chooser.type == XmtChooserCheckList)) {
	i = 0;
	XtSetArg(args[i], XmNitems, item_labels); i++;
#if XmVersion < 2000
	XtSetArg(args[i], XmNfontList, nw->chooser.font_list); i++;
#else /* Motif 2.0 or later */
	XtSetArg(args[i], XmNrenderTable, nw->chooser.render_table); i++;
#endif
	XtSetArg(args[i], XmNitemCount, nw->chooser.num_items); i++;
	XtSetArg(args[i], XmNvisibleItemCount, nw->chooser.visible_items); i++;
	XtSetArg(args[i], XmNselectionPolicy,
		 ((nw->chooser.type==XmtChooserRadioList)
		  ? XmBROWSE_SELECT : XmMULTIPLE_SELECT)); i++;
	nw->chooser.list =
	    XmCreateScrolledList(init, "list", args, i);
	if (nw->chooser.type == XmtChooserRadioList)
	    XtAddCallback(nw->chooser.list, XmNbrowseSelectionCallback,
			  ChooserCallback, (XtPointer) nw);
	else
	    XtAddCallback(nw->chooser.list, XmNmultipleSelectionCallback,
			  ChooserCallback, (XtPointer) nw);
	
	    XtManageChild(nw->chooser.list);
    }
#if XmVersion >= 2000
    else if (nw->chooser.type == XmtChooserComboBox) {
	i = 0;
	XtSetArg(args[i], XmNitems, item_labels); i++;
	XtSetArg(args[i], XmNrenderTable, nw->chooser.render_table); i++;
	XtSetArg(args[i], XmNitemCount, nw->chooser.num_items); i++;

        /*
         * ComboBoxes look funny with blank space at the bottom, so
         * if the actual number of items is smaller than the visible
         * items resource, we override the visible item count.
         */
        if (nw->chooser.visible_items > nw->chooser.num_items)
           nw->chooser.visible_items = nw->chooser.num_items;
	XtSetArg(args[i], XmNvisibleItemCount, nw->chooser.visible_items); i++;

        XtSetArg(args[i], XmNcomboBoxType, XmDROP_DOWN_LIST); i++;
        XtSetArg(args[i], XmNmatchBehavior, XmQUICK_NAVIGATE); i++;

        /*
         * ComboBoxes do not automatically size themselves based on
         * their contents.  We therefore change the width manually to
         * match the widest item.
         */
        num_columns = 0;
	for(j=0; j < nw->chooser.num_items; j++)  {
            len = strlen(nw->chooser.strings[j]);
            if (len > num_columns)
               num_columns = len;
        }
        XtSetArg(args[i], XmNcolumns, num_columns); i++;

        nw->chooser.combo_box =
            XmCreateComboBox(init, "combobox", args, i);

        XtAddCallback(nw->chooser.combo_box, XmNselectionCallback,
                      ChooserCallback, (XtPointer) nw);

        /*
         * Get the list widget that was created as part of the ComboBox
         * so we can manipulate it directly.
         */
        nw->chooser.list = XmtNameToWidget(nw->chooser.combo_box, "*List");

        XtManageChild(nw->chooser.combo_box);
    }
#endif

    /* If it is not a List widget, go create the internal buttons */
#if XmVersion < 2000
    if ((nw->chooser.type != XmtChooserRadioList) &&
	(nw->chooser.type != XmtChooserCheckList)) {
#else /* Motif 2.0 or later */
    if ((nw->chooser.type != XmtChooserRadioList) &&
	(nw->chooser.type != XmtChooserCheckList) &&
        (nw->chooser.type != XmtChooserComboBox)) {
#endif
	int ac;
	Widget parent;

	/* For a menu, parent is pane; otherwise self */
	if (nw->chooser.type == XmtChooserOption)
	    parent = nw->chooser.pane;
	else
	    parent = (Widget) nw;

	/* set up the fixed arguments */
	i = 0;
	XtSetArg(args[i], XmNlabelType, nw->chooser.label_type); i++;
#if XmVersion < 2000
	XtSetArg(args[i], XmNfontList, nw->chooser.font_list); i++;
#else /* Motif 2.0 or later */
	XtSetArg(args[i], XmNrenderTable, nw->chooser.render_table); i++;
#endif
	if ((nw->chooser.type != XmtChooserButtonBox) &&
	    (nw->chooser.type != XmtChooserOption)) {
	    XtSetArg(args[i], XmNset, False); i++;
	    if (nw->chooser.type == XmtChooserCheckBox) {
		XtSetArg(args[i], XmNindicatorType, XmN_OF_MANY); i++;
	    }
	    else if (nw->chooser.type == XmtChooserRadioBox) {
#if XmVersion < 2000
		XtSetArg(args[i], XmNindicatorType, XmONE_OF_MANY); i++;
#else /* Motif 2.0 or later */
                Boolean enable_toggle_visual;
                XtVaGetValues(XmGetXmDisplay(XtDisplay((Widget) nw)),
                              XmNenableToggleVisual, &enable_toggle_visual,
                              NULL);
		
                if (!enable_toggle_visual) {
		    XtSetArg(args[i], XmNindicatorType, XmONE_OF_MANY); i++;
		}
                else {
		    XtSetArg(args[i], XmNindicatorType, XmONE_OF_MANY_ROUND);
		    i++;
		}
#endif
	    }
	    else if ((nw->chooser.type == XmtChooserRadioPalette) ||
		     (nw->chooser.type == XmtChooserCheckPalette)) {
		XtSetArg(args[i], XmNindicatorOn, False); i++;
		XtSetArg(args[i], XmNshadowThickness, 2); i++;
		/* XXX shadow thickness should not be hardcoded. */
	    }
	}

	/* remember the number of fixed arguments */
	ac = i;

	/* create an array to hold the widgets we create */
	nw->chooser.item_widgets = (Widget *)
	    XtMalloc(nw->chooser.num_items * sizeof(Widget));

	/* set up the label arg(s) and create the widgets */
	for(j = 0; j < nw->chooser.num_items; j++) {
	    i = ac;
	    sprintf(namebuf, "item%d", j);
	    if (nw->chooser.label_type == XmSTRING) {
		XtSetArg(args[i], XmNlabelString, item_labels[j]);
		i++;
	    }
	    else {
		XtSetArg(args[i], XmNlabelPixmap, nw->chooser.pixmaps[j]); i++;
		if (nw->chooser.select_pixmaps) {
		    XtSetArg(args[i], XmNselectPixmap,
			     nw->chooser.select_pixmaps[j]);
		    i++;
		}
		if (nw->chooser.insensitive_pixmaps) {
		    XtSetArg(args[i], XmNlabelInsensitivePixmap,
			     nw->chooser.insensitive_pixmaps[j]);
		    i++;
		}
	    }
	    if ((nw->chooser.type == XmtChooserOption) ||
	        (nw->chooser.type == XmtChooserButtonBox)) {
		nw->chooser.item_widgets[j] =
		    XmCreatePushButton(parent, namebuf, args,i);
		XtAddCallback(nw->chooser.item_widgets[j], XmNactivateCallback,
			      ChooserCallback, (XtPointer)nw);
	    }
	    else {
		nw->chooser.item_widgets[j] =
		    XmCreateToggleButton(parent, namebuf, args,i);
		XtAddCallback(nw->chooser.item_widgets[j],
			      XmNvalueChangedCallback,
			      ChooserCallback, (XtPointer)nw);
	    }
	}
	XtManageChildren(nw->chooser.item_widgets, nw->chooser.num_items);
    }
    else { /* if it is a list */
	nw->chooser.item_widgets = NULL;
    }

    /* free all the item XmStrings */
    if (nw->chooser.label_type == XmSTRING) {
	for(i = 0; i < nw->chooser.num_items; i++)
	    XmStringFree(item_labels[i]);
	XtFree((char *)item_labels);
    }
	
    /*
     * if there is a symbol, bind the symbol,
     * set up notification, read the initial state,
     * and copy the symbol_name.
     */
    if (nw->chooser.symbol_name) {
	nw->chooser.symbol_name = XtNewString(nw->chooser.symbol_name);
	BindSymbol(nw);
    }
    else nw->chooser.symbol = NULL;

    /* make the widgets correspond to the initial state */
    SetStateOnWidgets(nw);

    /* set initial sensitivity */
    nw->chooser.insensitive = 0;

    /* and do other initialization */
    nw->chooser.ignore_symbol_notify = False;
}
    
#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget)w;
    
#if XmVersion < 2000
    /* free the font list */
    XmFontListFree(cw->chooser.font_list);
#else /* Motif 2.0 or later */
    /* free the renderTable */
    XmRenderTableFree(cw->chooser.render_table);
#endif

    /* free the widget array */
    XtFree((char *)cw->chooser.item_widgets);

    /* free the arrays, but not the strings themselves */
    XtFree((char *)cw->chooser.strings);
    XtFree((char *)cw->chooser.pixmaps);
    XtFree((char *)cw->chooser.select_pixmaps);
    XtFree((char *)cw->chooser.insensitive_pixmaps);
    XtFree((char *)cw->chooser.values);
    
    /* free the symbol name */
    XtFree(cw->chooser.symbol_name);

    /* remove symbol callback and free the symbol, if we have one */    
    ReleaseSymbol(cw);
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
    XmtChooserWidget cw = (XmtChooserWidget) current;
    XmtChooserWidget nw = (XmtChooserWidget) set;
    Arg args[5];

    if (nw->chooser.type != cw->chooser.type) {
	nw->chooser.type = cw->chooser.type;
	XmtWarningMsg("XmtChooser","cantChangeType",
		     "%s: can't change XmtNchooserType resource.",
		     XtName(set));
    }
    
    if ((nw->chooser.strings != cw->chooser.strings) ||
	(nw->chooser.pixmaps != cw->chooser.pixmaps) ||
	(nw->chooser.select_pixmaps != cw->chooser.select_pixmaps) ||
	(nw->chooser.insensitive_pixmaps != cw->chooser.insensitive_pixmaps))
	XmtWarningMsg("XmtChooser", "cantChange",
		      "%s: can't change labels or pixmaps.",
		      XtName((Widget) nw));

    if (nw->chooser.values != cw->chooser.values) {
	XtFree((char *)cw->chooser.values);
	CheckType(nw);
	if (nw->chooser.values) CopyValues(nw);
    }
    else if (nw->chooser.value_strings != cw->chooser.value_strings) {
	XtFree((char*) cw->chooser.values);
	CheckType(nw);
	if (nw->chooser.value_strings) ConvertValueStrings(nw);
    }

    if (nw->chooser.visible_items != cw->chooser.visible_items) {
#if XmVersion < 2000
	if ((nw->chooser.type==XmtChooserRadioList) ||
	    (nw->chooser.type==XmtChooserCheckList)) {
#else /* Motif 2.0 or later */
	if ((nw->chooser.type==XmtChooserRadioList) ||
	    (nw->chooser.type==XmtChooserCheckList) ||
            (nw->chooser.type==XmtChooserComboBox)) {
#endif
	    XtSetArg(args[0], XmNvisibleItemCount, nw->chooser.visible_items);
	    XtSetValues(nw->chooser.list, args, (Cardinal) 1);
	}
    }

    if (nw->chooser.symbol_name != cw->chooser.symbol_name) {
	/*
	 * forget the old symbol, if there is one.
	 * bind the new, if not NULL, register callbacks, set new state.
	 * If there is a new state, update the widgets.
	 */
	XtFree(cw->chooser.symbol_name);
	nw->chooser.symbol_name = XtNewString(nw->chooser.symbol_name);
	ReleaseSymbol(cw);
	BindSymbol(nw);
	if (nw->chooser.state != cw->chooser.state)
	    SetStateOnWidgets(nw);
    }
    else if (nw->chooser.state != cw->chooser.state) {
	/* if state changed, but symbol name didn't */
	if (nw->chooser.symbol) SetStateOnSymbol(nw); /* sets widgets too */
	else SetStateOnWidgets(nw);
    }

    /* 
     * if sensitivity changes, set the layoutSensitive resource, in case
     * we're a child of XmtLayout, and have a caption.
     */
    if ((nw->core.sensitive != cw->core.sensitive) ||
	(nw->core.ancestor_sensitive != cw->core.ancestor_sensitive)) {
	XtVaSetValues(set, XmtNlayoutSensitive,
		      nw->core.sensitive && nw->core.ancestor_sensitive, NULL);
    }

    return False;
}


#if NeedFunctionPrototypes
static void BoundsWarning(XmtChooserWidget cw, int n)
#else
static void BoundsWarning(cw, n)
XmtChooserWidget cw;
int n;
#endif
{
    XmtWarningMsg("XmtChooser", "bounds",
		  "Widget '%s' has %d items.  Item %d requested.",
		  XtName((Widget) cw), cw->chooser.num_items, n);
}


#if NeedFunctionPrototypes
Widget XmtCreateChooser(Widget parent, StringConst name,
			ArgList args, Cardinal n)
#else
Widget XmtCreateChooser(parent, name, args, n)
Widget parent;
StringConst name;
ArgList args;
Cardinal n;
#endif
{
    return XtCreateWidget((String)name,xmtChooserWidgetClass, parent, args, n);
}

#if NeedFunctionPrototypes
int XmtChooserGetState(Widget w)
#else
int XmtChooserGetState(w)
Widget w;
#endif
{
    XmtAssertWidgetClass(w, xmtChooserWidgetClass, "XmtChooserGetState");
    return ((XmtChooserWidget)w)->chooser.state;
}

#if NeedFunctionPrototypes
XtPointer XmtChooserGetValue(Widget w)
#else
XtPointer XmtChooserGetValue(w)
Widget w;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) w;
    XmtAssertWidgetClass(w, xmtChooserWidgetClass, "XmtChooserGetValue");

    if (cw->chooser.values)
	return (XtPointer) &((char *)cw->chooser.values)
	    [cw->chooser.state * cw->chooser.value_size];
    else return NULL;
}

#if NeedFunctionPrototypes
XtPointer XmtChooserLookupItemValue(Widget w, int item)
#else
XtPointer XmtChooserLookupItemValue(w, item)
Widget w;
int item;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) w;
    XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserLookupItemValue");

    if ((item < 0) || (item > cw->chooser.num_items)) {
	BoundsWarning(cw, item);
	return NULL;
    }
    
    if (cw->chooser.values)
	return (XtPointer)&((char *)cw->chooser.values)
	    [item * cw->chooser.value_size];
    else return NULL;
}

#if NeedFunctionPrototypes
void XmtChooserSetItemValue(Widget w, int item, XtPointer valuep)
#else
void XmtChooserSetItemValue(w, item, valuep)
Widget w;
int item;
XtPointer valuep;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) w;
    XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserLookupItemValue");

    if ((item < 0) || (item > cw->chooser.num_items)) {
	BoundsWarning(cw, item);
	return;
    }

    if (!cw->chooser.values)
	cw->chooser.values = (XtPointer) XtCalloc(cw->chooser.num_items,
						  cw->chooser.value_size);

    memmove(&((char *)cw->chooser.values)[item * cw->chooser.value_size],
	    valuep,
	    cw->chooser.value_size);
}


#if NeedFunctionPrototypes
void XmtChooserSetState(Widget w, int state, XmtWideBoolean notify)
#else
void XmtChooserSetState(w, state, notify)
Widget w;
int state;
int notify;
#endif
{
   XmtChooserWidget cw = (XmtChooserWidget)w;
   XmtChooserCallbackStruct call_data;

   XmtAssertWidgetClass(w, xmtChooserWidgetClass, "XmtChooserSetState");

   /* the XmtChooserButtonBox doesn't have a state to set */
   if (cw->chooser.type == XmtChooserButtonBox) return;

   /* if this chooser has radio behavior, check bounds on state */
   if ((cw->chooser.type != XmtChooserCheckBox) &&
       (cw->chooser.type != XmtChooserCheckPalette) &&
       (cw->chooser.type != XmtChooserCheckList) &&
       ((state < 0) || (state > cw->chooser.num_items))) {
       BoundsWarning(cw, state);
       return;
   }

   cw->chooser.state = state;

   if (cw->chooser.symbol) SetStateOnSymbol(cw);  /* indirectly sets widgets */
   else SetStateOnWidgets(cw);

   if (notify) {
       call_data.state = state;
       call_data.item = -1;  /* -1 flags synthetic state changes */
       if (cw->chooser.values &&
#if XmVersion < 2000
	   ((cw->chooser.type == XmtChooserRadioBox) ||
	    (cw->chooser.type == XmtChooserRadioPalette) ||
	    (cw->chooser.type == XmtChooserRadioList) ||
	    (cw->chooser.type == XmtChooserOption)))
#else /* Motif 2.0 or later */
	   ((cw->chooser.type == XmtChooserRadioBox) ||
	    (cw->chooser.type == XmtChooserRadioPalette) ||
	    (cw->chooser.type == XmtChooserRadioList) ||
	    (cw->chooser.type == XmtChooserOption) ||
            (cw->chooser.type == XmtChooserComboBox)))
#endif
	   call_data.valuep = (XtPointer) &((char *)cw->chooser.values)
	       [state * cw->chooser.value_size];
       else
	   call_data.valuep = NULL;
       XtCallCallbackList(w, cw->chooser.callback, (XtPointer)&call_data);
   }
}

#if NeedFunctionPrototypes
int XmtChooserLookupItemByName(Widget w, StringConst name)
#else
int XmtChooserLookupItemByName(w, name)
Widget w;
StringConst name;
#endif
{
   XmtChooserWidget cw = (XmtChooserWidget)w;
   int i;
   
   XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserLookupItemByName");

   if (!name || !cw->chooser.strings) return -1;
   
   for(i = cw->chooser.num_items-1; i >= 0; i--)
       if (cw->chooser.strings[i] && (strcmp(name, cw->chooser.strings[i])==0))
	   break;

   return i;
}

#if NeedFunctionPrototypes
int XmtChooserLookupItemByValue(Widget w, XtPointer valuep)
#else
int XmtChooserLookupItemByValue(w, valuep)
Widget w;
XtPointer valuep;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget)w;
    int i;
    
    XmtAssertWidgetClass(w, xmtChooserWidgetClass,
			 "XmtChooserLookupItemByValue");
    
    if (!cw->chooser.values) return -1;
    
    if (strcmp(cw->chooser.value_type, XtRString) == 0) {
	for(i = cw->chooser.num_items-1; i >= 0; i--)
	    if (strcmp(*(String *)valuep,
		       *(String*)&((char *)cw->chooser.values)[i*cw->chooser.value_size]) == 0)
		break;
    }
    else {
	for(i = cw->chooser.num_items-1; i >= 0; i--)
	    if (memcmp(valuep,
		       &((char*)cw->chooser.values)[i*cw->chooser.value_size],
		       cw->chooser.value_size) == 0) break;
    }
    return i;
}

#if NeedFunctionPrototypes
String XmtChooserLookupItemName(Widget w, int n)
#else
String XmtChooserLookupItemName(w, n)
Widget w;
int n;
#endif
{
   XmtChooserWidget cw = (XmtChooserWidget)w;

   XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserLookupItemName");

   if ((n < 0) || (n > cw->chooser.num_items)) {
       BoundsWarning(cw, n);
       return NULL;
   }

   if (cw->chooser.strings)
       return cw->chooser.strings[n];
   else
       return NULL;
}


#if NeedFunctionPrototypes
void XmtChooserSetSensitive(Widget w, int n, XmtWideBoolean sensitive)
#else
void XmtChooserSetSensitive(w, n, sensitive)
Widget w;
int n;
int sensitive;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget)w;

    XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserSetSensitive");
    
    if ((n < 0) || (n > cw->chooser.num_items)) {
	BoundsWarning(cw, n);
	return;
    }

    if (n > 31) {
	XmtWarningMsg("XmtChooserSetSensitive", "size",
		      "can only set sensitivity on first 32 items.");
	return;
    }

    /*
     * if this is not a list chooser, set the button's sensitivity.
     */
    if (cw->chooser.item_widgets)
	XtSetSensitive(cw->chooser.item_widgets[n], sensitive);

    /*
     * set our internal state
     */
    if (sensitive)
	cw->chooser.insensitive &= ~(1 << n);
    else
	cw->chooser.insensitive |= 1 << n;
}

#if NeedFunctionPrototypes
Boolean XmtChooserGetSensitivity(Widget w, int n)
#else
Boolean XmtChooserGetSensitivity(w, n)
Widget w;
int n;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget)w;

    XmtAssertWidgetClass(w, xmtChooserWidgetClass,"XmtChooserGetSensitivity");
    
    if ((n < 0) || (n > cw->chooser.num_items)) {
	BoundsWarning(cw, n);
	return False;
    }

    if (n > 31) {
	XmtWarningMsg("XmtChooserGetSensitivity", "size",
		      "can only get sensitivity on first 32 items.");
	return False;
    }

    if (cw->chooser.insensitive & (1 << n)) return False;
    else return True;
}

#if NeedFunctionPrototypes
static void setvalue(Widget w, XtPointer address, XrmQuark type, Cardinal size)
#else
static void setvalue(w, address, type, size)
Widget w;
XtPointer address;
XrmQuark type;
Cardinal size;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) w;

    if (cw->chooser.values) {
	if (XrmStringToQuark(cw->chooser.value_type) == type) {
	    int i = XmtChooserLookupItemByValue(w, address);
	    if (i != -1)
		XmtChooserSetState(w, i, False);
	    else
		XmtWarningMsg("XmtChooser", "setvalue0",
			      "Widget '%s':\n\tCan't set value from resource.\n\tSpecified value is not one of the legal values.",
			      XtName(w));
	}
	else
	    XmtWarningMsg("XmtChooser", "setvalue1",
		    "Type mismatch:\n\tcan't set value from a resource of type '%s';\n\tWidget '%s' expects a value of type '%s'",
		    XrmQuarkToString(type), XtName(w), cw->chooser.value_type);
    }
    else {
	int value;
	
	if (size == sizeof(int))
	    value = *(int *)address;
	else if (size == sizeof(short))
	    value = (int) *(short *)address;
	else if (size == sizeof(char))
	    value = (int) *(char *)address;
	else {
	    XmtWarningMsg("XmtChooser", "setvalue2",
			  "Type mismatch: \n\tCan't set value from a resource of non-scalar type '%s'.",
			  XrmQuarkToString(type));
	    return;
	}
	    
        XmtChooserSetState(w, value, False);
    }
}

#if NeedFunctionPrototypes
static void getvalue(Widget w, XtPointer address, XrmQuark type, Cardinal size)
#else
static void getvalue(w, address, type, size)
Widget w;
XtPointer address;
XrmQuark type;
Cardinal size;
#endif
{
    XmtChooserWidget cw = (XmtChooserWidget) w;

    if (cw->chooser.values) {
	if (XrmStringToQuark(cw->chooser.value_type) == type) 
	    memmove(address, XmtChooserGetValue(w), cw->chooser.value_size);
	else
	    XmtWarningMsg("XmtChooser", "getvalue0",
		    "Type mismatch:\n\tcan't set value on a resource of type '%s';\n\tWidget '%s' has values of type '%s'",
		    XrmQuarkToString(type), XtName(w), cw->chooser.value_type);
    }
    else {
	int state = XmtChooserGetState(w);
	
	if (size == sizeof(int))
	    *(int *)address = state;
	else if (size == sizeof(short))  /* note we don't warn on overflow */
	    *(short *)address = state;
	else if (size == sizeof(char))
	    *(char *)address = state;
	else {
	    XmtWarningMsg("XmtChooser", "setvalue2",
			  "Type mismatch: \n\tCan't set value from a resource of non-scalar type '%s'.",
			  XrmQuarkToString(type));
	    return;
	}
    }
}


static XmtWidgetType chooser_widget = {
    "XmtChooser",
    (WidgetClass)&xmtChooserClassRec,
    NULL,
    setvalue,
    getvalue,
};

#if NeedFunctionPrototypes
void XmtRegisterChooser(void)
#else
void XmtRegisterChooser()
#endif
{
    XmtRegisterWidgetTypes(&chooser_widget, 1);
}
