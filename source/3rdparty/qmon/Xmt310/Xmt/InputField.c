/* 
 * Motif Tools Library, Version 3.1
 * $Id: InputField.c,v 1.4 2009-03-17 15:18:05 roland Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: InputField.c,v $
 * Revision 1.4  2009-03-17 15:18:05  roland
 * RD-2009-03-17-0: Enhancem.:   - added EXCL relop for boolean consumable
 *                  Bugster:     6731441
 *                  Issue:       2629
 *                  Bugfix:      - libspoolb and libspoolc should not link against
 *                                 jemalloc/mtmalloc
 *                  Bugfix:      - fixed memory leak in shepherd and buildin_starter
 *                  Review:      AA
 *
 * Revision 1.3  2007/05/29 11:52:34  andre
 * AA-2007-05-29-0: Enhancem:  - Advance Reservation qmon changes
 *                             - various qmon bug fixes:
 *                             - split of qrsub sge_parse_qrsub into clients/common/sge_qrsub.c
 *                             - reserved_slots -> resv_slots for consistency
 *                             - jgdi delete job crashes master
 *                  Issue:     1729, 2262, 2263, 2260, 2261, 747,
 *                             2264
 *                  Bugster:   6327539, 6538740, 4818801, 4742097, 6553066, 6291044
 *                             6555953
 *                  Review:    RD
 *
 * Revision 1.2  2001/08/09 12:56:52  andre
 * AA-2001-08-09-0: - IZ 33, fixed duplicate free for scheduler conf dialogue code
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
#include <ctype.h>
#include <string.h>
#include <Xmt/XmtP.h>
#include <Xmt/InputFieldP.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Layout.h> /* for XmtNlayoutSensitive resource */
#include <Xmt/QuarksP.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern int atoi();
extern double atof();
#endif

/*
 * I don't know how MAXINT is getting defined on most platforms, (I
 * don't know where <values.h> is included), but it isn't defined in
 * Linux.  It might be better to include <limits.h> and switch to
 * INT_MAX, since this is the POSIX way to do it.  But then I'd have
 * to take care of non-POSIX platforms as a special case.  So this fix
 * is easier.  Platforms with 64-bit integers that don't define MAXINT
 * will just have to live with this 32-bit definition.  This won't be
 * a problem for its use in this file.
 */
#ifndef MAXINT
#define MAXINT 0x7FFFFFFF
#endif

#define offset(field) XtOffsetOf(XmtInputFieldRec, input_field.field)

static XtResource resources[] = {
{XmtNinput, XmtCInput, XtRString,
     sizeof(String), offset(input),
     XtRString, (XtPointer)NULL},
{XmtNpattern, XmtCPattern, XtRString,
     sizeof(String), offset(pattern),
     XtRString, (XtPointer)NULL},
{XmtNmatchAll, XmtCMatchAll, XtRBoolean,
     sizeof(Boolean), offset(match_all),
     XtRImmediate, (XtPointer)True},
{XmtNoverstrike, XmtCOverstrike, XtRBoolean,
     sizeof(Boolean), offset(overstrike),
     XtRBoolean, (XtPointer) False},
{XmtNbufferSymbolName, XmtCBufferSymbolName, XtRString,
     sizeof(String), offset(buffer_symbol_name),
     XtRString, (XtPointer)NULL},
{XmtNtargetSymbolName, XmtCTargetSymbolName, XtRString,
     sizeof(String), offset(target_symbol_name),
     XtRString, (XtPointer)NULL},
{XmtNautoInsert, XmtCAutoInsert, XtRBoolean,
     sizeof(Boolean), offset(auto_insert),
     XtRImmediate, (XtPointer)True},
{XmtNautoDelete, XmtCAutoDelete, XtRBoolean,
     sizeof(Boolean), offset(auto_delete),
     XtRImmediate, (XtPointer)True},
{XmtNbeepOnError, XmtCBeepOnError, XtRBoolean,
     sizeof(Boolean), offset(beep_on_error),
     XtRImmediate, (XtPointer) True},
{XmtNreplaceOnError, XmtCReplaceOnError, XtRBoolean,
     sizeof(Boolean), offset(replace_on_error),
     XtRImmediate, (XtPointer) True},
{XmtNhighlightOnError, XmtCHighlightOnError, XtRBoolean,
     sizeof(Boolean), offset(highlight_on_error),
     XtRImmediate, (XtPointer) False},
{XmtNerrorString, XmtCErrorString, XtRString,
     sizeof(String), offset(error_string),
     XtRString, (XtPointer) NULL},
{XmtNerrorForeground, XmtCErrorForeground, XtRPixel,
     sizeof(String), offset(error_foreground),
     XtRImmediate, (XtPointer) -1},  /* XXX could be a valid Pixel value */
{XmtNerrorBackground, XmtCErrorBackground, XtRPixel,
     sizeof(String), offset(error_background),
     XtRImmediate, (XtPointer) -1},  /* XXX could be a valid Pixel value */
{XmtNinputCallback, XtCCallback, XtRCallback,
     sizeof(XtCallbackList), offset(input_callback),
     XtRCallback, (XtPointer) NULL},
{XmtNverifyCallback, XtCCallback, XtRCallback,
     sizeof(XtCallbackList), offset(verify_callback),
     XtRCallback, (XtPointer) NULL},
{XmtNerrorCallback, XtCCallback, XtRCallback,
     sizeof(XtCallbackList), offset(error_callback),
     XtRCallback, (XtPointer) NULL},
/*
 * override navigationType here so arrow-key traversal will work
 */
{XmNnavigationType, XmCNavigationType, XmRNavigationType,
     sizeof(unsigned char),
     XtOffsetOf(XmtInputFieldRec, primitive.navigation_type),
     XtRImmediate, (XtPointer) XmNONE},
};
#undef offset

/* widget methods */
#if NeedFunctionPrototypes
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
#else
static void Initialize();
static void Destroy();
static Boolean SetValues();
#endif

/* action procedure */
#if NeedFunctionPrototypes
static void Overstrike(Widget, XEvent *, String *, Cardinal *);
#else
static void Overstrike();
#endif

static XtActionsRec actions[] = {
  {"overstrike", Overstrike},
};

#if XmVersion > 1001
static XmPrimitiveClassExtRec primClassExtRec = {
    NULL,
    NULLQUARK,
    XmPrimitiveClassExtVersion,
    sizeof(XmPrimitiveClassExtRec),
    XmInheritBaselineProc,                  /* widget_baseline */
    XmInheritDisplayRectProc,               /* widget_display_rect */
/*    (XmWidgetMarginsProc) _XtInherit */
    _XmTextMarginsProc  /* this isn't inherited properly */
};
#endif

#define Superclass (&xmTextClassRec)

externaldef(xmtinputfieldclassrec)
XmtInputFieldClassRec xmtInputFieldClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) Superclass,
    /* class_name         */    "XmtInputField",
    /* widget_size        */    sizeof(XmtInputFieldRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    actions,
    /* num_actions        */    XtNumber(actions),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    XtExposeCompressMaximal,
    /* compress_enterleave*/    TRUE,
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
    /* extension	  */	NULL,
  },
   {				/* primitive_class fields 	*/
      XmInheritBorderHighlight, /* Primitive border_highlight   */
      XmInheritBorderUnhighlight,/* Primitive border_unhighlight */
      NULL,	                /* translations                 */
      NULL,         		/* arm_and_activate           	*/
      NULL,  	    	        /* get resources 	        */
      0, 	                /* num get_resources            */
#if XmVersion <= 1001
      NULL,         		/* extension                    */
#else
      (XtPointer)&primClassExtRec,/* extension                    */
#endif
   },
   {				/* text class fields */
      NULL,             	/* extension         */
   },
   { /* XmtInputField      */
     /* extension	   */    NULL
   }
};

externaldef(xmtinputfieldwidgetclass)
WidgetClass xmtInputFieldWidgetClass = (WidgetClass)&xmtInputFieldClassRec;

#if XmVersion != 1002
#define SetString XmTextSetString
#else
#if NeedFunctionPrototypes
static void SetString(Widget w, String s)
#else
static void SetString(w, s)
Widget w;
String s;
#endif
{
    Boolean hack;
    XmtInputFieldWidget iw = (XmtInputFieldWidget) w;
    
    if (s && s[0]) XmTextSetString(w,s);
    else {
	hack = iw->text.auto_show_cursor_position;
	iw->text.auto_show_cursor_position = False;
	XmTextSetString(w,s);
	iw->text.auto_show_cursor_position = hack;
    }
}
#endif


/* ARGSUSED */
#if NeedFunctionPrototypes
static void HandleOverstrike(Widget text, XtPointer tag, XtPointer call_data)
#else
static void HandleOverstrike(text, tag, call_data)
Widget text;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) text;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct*) call_data;
    
    /* if this is not a user keystroke, don't check it */
    if (!data->event) return;

    /* if this is a deletion, do nothing */
    /* Different versions of Motif indicate deletions in different ways,
     * so check several fields */
    if ((data->text->length == 0) ||
	(data->text->ptr == NULL) || (data->text->ptr[0] == '\0')) return;
    
    /*
     * turn an insertion into a replace.
     * HandlePattern() also does this for us, in case it is called
     * after this callback and changes increase the insertion length
     */
    data->endPos = data->startPos+data->text->length;

    /*
     * We can't have overstrike on and the text XmNmaxLength set, because the
     * text won't allow us to type a character when we're at maxLength, because
     * it assumes we'll be inserting it.  So when overstrike is on, we
     * don't set maxLength, and instead check the length here.
     */
    if (data->endPos > iw->input_field.max_length)
	data->doit = False;
}

/* ARGSUSED */
#if NeedFunctionPrototype
static void MoveCursorTimerProc(XtPointer data, XtIntervalId *id)
#else
static void MoveCursorTimerProc(data, id)
XtPointer data;
XtIntervalId *id;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget)data;

    if (iw->input_field.pattern_skip != 0) {
	XmTextSetInsertionPosition((Widget)iw,
				   XmTextGetInsertionPosition((Widget)iw) +
				   iw->input_field.pattern_skip);
	iw->input_field.pattern_skip = 0;
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static int isoctal(int c)
#else
static int isoctal(c)
int c;
#endif
{
    return (strchr("01234567", c)) ? 1:0;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static int ishex(int c)
#else
static int ishex(c)
int c;
#endif
{
    return (strchr("0123456789abcdefABCDEF", c)) ? 1:0;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void HandlePattern(Widget text, XtPointer tag, XtPointer call_data)
#else
static void HandlePattern(text, tag, call_data)
Widget text;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) text;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct*) call_data;
    int i, j, pos, last;
    char *c;

    /* if this is not a user keystroke, don't check it */
    if (!data->event) return;

    last = XmTextGetLastPosition(text);
    
    /* if it is a deletion, only allow it at the end.
     * Also, if autoDelete adjust startPos to point to a non-constant char
     */
    /* Different versions of Motif indicate deletions in different ways,
     * so check several fields */
    if ((data->text->length == 0) || 
	(data->text->ptr == NULL) || (data->text->ptr[0] == '\0')) {
	if (data->endPos != last) {
	    data->doit = False;
#ifdef BACKSPACEBUG
	    data->text->ptr = NULL;
#endif	
	    return;
	}
	if (iw->input_field.auto_delete)
	    for(c = &iw->input_field.pattern[data->startPos];
		data->startPos > 0;
		data->startPos--, c--) {
		if (((*c >= 'a')&&(*c <= 'd')) ||
		    ((*c >= 'A')&&(*c <= 'C')) ||
		    (*c == 'h') ||
		    (*c == 'o'))
		    break;
	    }
	return;
    }
    
    /* otherwise it is an insertion, or if in overstrike mode, a replace.
     * if not in overstrike mode, only allow it at the end.
     * Also, if it would put us past the end of the pattern, reject
     */
    if ((!iw->input_field.overstrike) && (data->startPos != last)) {
	data->doit = False;
	return;
    }
    
    if (data->startPos + data->text->length >
	iw->input_field.pattern_length) {
	data->doit = False;
	return;
    }
    
    iw->input_field.pattern_skip = 0;  /* # of const. chars inserted */
    
    /* test each character of the insertion */
    for(i=0, pos=data->startPos;
	i < data->text->length;
	i++, pos++) {
	c = &data->text->ptr[i];
	switch(iw->input_field.pattern[pos]) {
	case 'A':
	    *c = toupper(*c);
	case 'a':
	    if (!isalpha(*c)) data->doit = False;
	    break;
	case 'B':
	    *c = toupper(*c);
	case 'b':
	    if (!isalpha(*c) && !isdigit(*c)) data->doit = False;
	    break;
	case 'C':
	    *c = toupper(*c);
	case 'c':
	    break;
	case 'd':
	    if (!isdigit(*c)) data->doit = False;
	    break;
        case 'o':
	  if (!isoctal(*c)) data->doit = False;
	  break;
        case 'h':
	  if (!ishex(*c)) data->doit = False;
          break;
	default:
	    /*
	     * it is a constant character.  If *c matches, accept it.
	     * Otherwise, if autoInsert, insert it, otherwise reject.
	     */
	    if (*c == iw->input_field.pattern[pos]) break;
	    if (!iw->input_field.auto_insert) {
		data->doit = False;
		break;
	    }
	    data->text->length++;
	    data->text->ptr = XtRealloc(data->text->ptr,
					data->text->length);
	    for(j=data->text->length-1; j > i; j--)
		data->text->ptr[j] = data->text->ptr[j-1];
	    data->text->ptr[i] = iw->input_field.pattern[pos];
	    iw->input_field.pattern_skip++;
	    break;
	}
	if (data->doit == False) return;
    }
    
    if (iw->input_field.pattern_skip) 
	XtAppAddTimeOut(XtWidgetToApplicationContext(text),
			0, MoveCursorTimerProc, (XtPointer)iw);

    /* if we are overstriking and this is a replace, we've got to
     * increase the deleted characters by the # of inserted constant chars
     * The right thing will happen whether HandleOverstrike is called
     * first or this procedure is called first.
     */
    if (iw->input_field.overstrike)
	data->endPos = data->startPos + data->text->length;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void ValueChanged(Widget text, XtPointer tag, XtPointer data)
#else
static void ValueChanged(text, tag, data)
Widget text;
XtPointer tag;
XtPointer data;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) text;

    /* remember that the value has changed */
    iw->input_field.input_changed = True;

    /* and remove ourselves for efficiency */
    XtRemoveCallback(text, XmNvalueChangedCallback, ValueChanged, NULL);
}

#if NeedFunctionPrototypes
static void RestoreOldValue(XmtInputFieldWidget iw)
#else
static void RestoreOldValue(iw)
XmtInputFieldWidget iw;
#endif
{
    SetString((Widget)iw, iw->input_field.input);
    XmTextSetInsertionPosition((Widget)iw, XmTextGetLastPosition((Widget)iw));
    iw->input_field.input_changed = False;
    XtAddCallback((Widget)iw, XmNvalueChangedCallback, ValueChanged, NULL);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void UndoError(Widget text, XtPointer tag, XtPointer data)
#else
static void UndoError(text, tag, data)
Widget text;
XtPointer tag;
XtPointer data;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) text;

    /*
     * if not handling an error, don't do anything.
     * This should never happen.
     */
    if (!iw->input_field.error_state) return;

    /* remove this callback everywhere it was registered */
    XtRemoveCallback(text, XmNvalueChangedCallback, UndoError, NULL);
    XtRemoveCallback(text, XmNfocusCallback, UndoError, NULL);
    XtRemoveCallback(text, XmNactivateCallback, UndoError, NULL);
		     
    /* if we are currently displaying an error string, change it, either
     * to the last value, or back to the bad value
     */
    if (iw->input_field.error_string) {
	if (iw->input_field.replace_on_error) {
	    RestoreOldValue(iw);
	}
       	else {
	    SetString(text, iw->input_field.error_value);
	    XmTextSetInsertionPosition(text, XmTextGetLastPosition(text));
	    XtFree(iw->input_field.error_value);
	}
    }
    else if (iw->input_field.highlight_on_error) {
	/* if a color specified, and color display, restore old color */
	if (((iw->input_field.error_background != (Pixel)-1) ||
	     (iw->input_field.error_foreground != (Pixel)-1)) &&
	    (iw->core.depth > 1)) {
	    Arg al[2];
	    int ac = 0;
	    if (iw->input_field.error_background != (Pixel)-1) {
		XtSetArg(al[ac],XmNbackground,iw->input_field.background);
		ac++;
	    }
	    if (iw->input_field.error_foreground != (Pixel)-1) {
		XtSetArg(al[ac],XmNforeground,iw->input_field.foreground);
		ac++;
	    }
	    XtSetValues(text, al, ac);
	}
	else {  /* remove the underlining */
	    XmTextSetHighlight(text, 0,XmTextGetLastPosition(text),
			       XmHIGHLIGHT_NORMAL);
	}
	/* if replaceOnError, restore old string */
	if (iw->input_field.replace_on_error)
	    RestoreOldValue(iw);
    }
    
    iw->input_field.error_state = False;
}

#if NeedFunctionPrototypes
static void HandleError(XmtInputFieldWidget iw, String value)
#else
static void HandleError(iw, value)
XmtInputFieldWidget iw;
String value;
#endif
{
    XmtInputFieldCallbackStruct error;

    /*
     * if we're already handling an error, don't do anything.
     */
    if (iw->input_field.error_state) return;

    /*
     * call the error callback to give app programmer the chance to handle
     * the error.  If requested, abort further error handling
     */
    error.input = value;
    error.okay = False;
    XtCallCallbackList((Widget)iw, iw->input_field.error_callback,
		       (XtPointer) &error);
    if (error.okay == True) return;

    /* beep if requested */
    if (iw->input_field.beep_on_error)
	XBell(XtDisplay(iw), 0);

    if (iw->input_field.error_string) {
	/*
	 * We'll display the error string, but first, we've got to save a
	 * copy of the current bad string, unless we're going to replace that
	 * string with the current value resource.
	 * See UndoError() above.
	 */
	if (!iw->input_field.replace_on_error)
	    iw->input_field.error_value =
		XmTextGetString((Widget)iw);
	SetString((Widget)iw, iw->input_field.error_string);
	XmTextSetInsertionPosition((Widget)iw,
				   XmTextGetLastPosition((Widget)iw));
    }
    else if (iw->input_field.highlight_on_error) {
	/* use color if a color is specified, and if a color display */
	if (((iw->input_field.error_background != (Pixel)-1) ||
	     (iw->input_field.error_foreground != (Pixel)-1)) &&
	    (iw->core.depth > 1)) {
	    Arg al[2];
	    int ac = 0;
	    if (iw->input_field.error_background != (Pixel)-1) {
		iw->input_field.background = iw->core.background_pixel;
		XtSetArg(al[ac],XmNbackground,
			 iw->input_field.error_background);
		ac++;
	    }
	    if (iw->input_field.error_foreground != (Pixel)-1) {
		iw->input_field.foreground = iw->primitive.foreground;
		XtSetArg(al[ac],XmNforeground,
			 iw->input_field.error_foreground);
		ac++;
	    }
	    XtSetValues((Widget)iw, al, ac);
	}
	else {  /* just underline the text */
	    XmTextSetHighlight((Widget)iw, 0,XmTextGetLastPosition((Widget)iw),
			       XmHIGHLIGHT_SECONDARY_SELECTED);
	}
    }
    else if (iw->input_field.replace_on_error) {
	/*
	 * if no error string or highlighting, restore old value now.
	 * otherwise, we restore it in UndoError() above.
	 */
	RestoreOldValue(iw);
    }
    
    /*
     * add callbacks to undo the effects of this error handling
     * on focus in or any keypress.  Note that if the error handling is
     * just replace_on_error, there is nothing to undo.
     * Set the error_state flag.
     */
    if (iw->input_field.error_string || iw->input_field.highlight_on_error) {
	XtAddCallback((Widget)iw, XmNvalueChangedCallback, UndoError, NULL);
	XtAddCallback((Widget)iw, XmNfocusCallback, UndoError, NULL);
	XtAddCallback((Widget)iw, XmNactivateCallback, UndoError, NULL);
	iw->input_field.error_state = True;

    }
}

#if NeedFunctionPrototypes
static Boolean SetValueOnSymbols(XmtInputFieldWidget iw, StringConst value,
				 Boolean check)
#else
static Boolean SetValueOnSymbols(iw, value, check)
XmtInputFieldWidget iw;
StringConst value;
Boolean check;
#endif
{
    Boolean status = True;
    
    /*
     * set the buffer and target symbols from the value resource.
     * This will cause the buffer symbol callback to be invoked, so we
     * set a flag which will make that callback a no-op.
     * If check is true, then we don't set the buffer symbol if
     * the target symbol conversion failed.
     */
    if (value == NULL) value = "";
    if (iw->input_field.target_symbol)
	status = XmtSymbolSetTypedValue((Widget) iw,
					iw->input_field.target_symbol,
					XtRString, (XtArgVal) value,
					strlen(value)+1);

    if ((!check || status) && iw->input_field.buffer_symbol) {
	iw->input_field.ignore_symbol_notify = True;
	XmtSymbolSetValue(iw->input_field.buffer_symbol, (XtArgVal)value);
	iw->input_field.ignore_symbol_notify = False;
    }

    return status;
}

#if NeedFunctionPrototypes
static void SetValueFromSymbol(XmtInputFieldWidget iw)
#else
static void SetValueFromSymbol(iw)
XmtInputFieldWidget iw;
#endif
{
    String symbol_value;
    
    /*
     * set the value resource from the buffer_symbol
     */
    if (iw->input_field.buffer_symbol) {
	XmtSymbolGetValue(iw->input_field.buffer_symbol,
			  (XtArgVal *) &symbol_value);
        if (iw->input_field.input) XtFree(iw->input_field.input);
	iw->input_field.input = XtNewString(symbol_value);
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void SymbolValueChanged(XmtSymbol s, XtPointer tag, XtArgVal value)
#else
static void SymbolValueChanged(s, tag, value)
XmtSymbol s;
XtPointer tag;
XtArgVal value;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget)tag;

    /*
     * the notify callback invoked when the value in the buffer symbol
     * changes underneath us.  Update the value resource, the display,
     * and the target symbol.  If this procedure is called as a side
     * effect fo SetValueOnSymbols(), then we do nothing.
     */
    if (iw->input_field.ignore_symbol_notify) return;

    /* set the value resource, and the text widget displayed value */
    SetValueFromSymbol(iw);
    SetString((Widget)iw, iw->input_field.input);

    /* set the target symbol value */
    if (iw->input_field.target_symbol)
	XmtSymbolSetTypedValue((Widget) iw, iw->input_field.target_symbol,
			       XtRString, (XtArgVal) iw->input_field.input,
			       strlen(iw->input_field.input)+1);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void Activate(Widget text, XtPointer tag, XtPointer data)
#else
static void Activate(text, tag, data)
Widget text;
XtPointer tag;
XtPointer data;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) text;
    XmtInputFieldCallbackStruct verify;
    String input;
    int input_length;
    Boolean status;

    /* if we're handling an error, don't activate */
    if (iw->input_field.error_state) return;

    /* if nothing has changed since the last time, don't do anything */
    if (!iw->input_field.input_changed) return;

    /* get the current input */
    input = XmTextGetString(text);
    input_length = strlen(input);
    
    /* check that input matches all of the pattern error if not */
    if (iw->input_field.pattern && iw->input_field.match_all &&
	(input_length != iw->input_field.pattern_length))
	goto error;
    
    /* call the verify callback; error if it fails.
     * The programmer may replace input, but should not free or realloc it.
     * Programmer owns any string used to replace the input.
     */
    verify.input = input;
    verify.okay = True;
    XtCallCallbackList((Widget)iw, iw->input_field.verify_callback,
		       (XtPointer)&verify);
    if (verify.okay == False) goto error;
    if (verify.input != input) {
	XtFree(input);
	input = XtNewString(verify.input);
	input_length = strlen(input);
	SetString(text, input);  
	XmTextSetInsertionPosition(text, XmTextGetLastPosition(text));
    }
    
    /* put the new input into buffer, and target, if specified */
    status = SetValueOnSymbols(iw, input, True);

    if (status == False) goto error;

    /* re-register the value changed callback */
    iw->input_field.input_changed = False;
    XtAddCallback(text, XmNvalueChangedCallback, ValueChanged, NULL);

    /* update the input resource */
    XtFree(iw->input_field.input);
    iw->input_field.input = input;
    
    /* invoke the input field input callback */
    XtCallCallbackList((Widget)iw, iw->input_field.input_callback,
		       input);
    return;

 error:
    HandleError(iw, input);
    XtFree(input);
}

#if NeedFunctionPrototypes
static void BindBufferSymbol(XmtInputFieldWidget iw)
#else
static void BindBufferSymbol(iw)
XmtInputFieldWidget iw;
#endif
{
    if (iw->input_field.buffer_symbol_name) {
	iw->input_field.buffer_symbol =
	    XmtLookupSymbol(iw->input_field.buffer_symbol_name);
	if (iw->input_field.buffer_symbol) {
	    if (strcmp(XmtSymbolType(iw->input_field.buffer_symbol),
		       XmtRBuffer) != 0) {
		XmtWarningMsg("XmtInputField", "symbolType",
			      "%s: buffer symbol '%s' has wrong type;\n\tshould be XmtRBuffer.",
			      XtName((Widget)iw),
			      iw->input_field.buffer_symbol_name);
		iw->input_field.buffer_symbol = NULL;
	    }
	    else {
		/* a defined symbol of the right type */
		XmtSymbolAddCallback(iw->input_field.buffer_symbol,
				     SymbolValueChanged, (XtPointer)iw);
	    }
	}
	else {
	    XmtWarningMsg("XmtInputField", "badBSymbol",
			  "%s: buffer symbol '%s' is undefined.",
			  XtName((Widget)iw),
			  iw->input_field.buffer_symbol_name);
	}
    }
    else
	iw->input_field.buffer_symbol = NULL;
}

#if NeedFunctionPrototypes
static void ReleaseBufferSymbol(XmtInputFieldWidget iw)
#else
static void ReleaseBufferSymbol(iw)
XmtInputFieldWidget iw;
#endif
{
    if (iw->input_field.buffer_symbol) {
	XmtSymbolRemoveCallback(iw->input_field.buffer_symbol,
				SymbolValueChanged, (XtPointer)iw);
	iw->input_field.buffer_symbol = NULL;
    }
}

#if NeedFunctionPrototypes
static void BindTargetSymbol(XmtInputFieldWidget iw)
#else
static void BindTargetSymbol(iw)
XmtInputFieldWidget iw;
#endif
{
    if (iw->input_field.target_symbol_name) {
	iw->input_field.target_symbol =
	    XmtLookupSymbol(iw->input_field.target_symbol_name);
	if (!iw->input_field.target_symbol) 
	    XmtWarningMsg("XmtInputField", "badTSymbol",
			  "%s: target symbol '%s' is undefined.",
			  XtName((Widget)iw),
			  iw->input_field.target_symbol_name);
    }
    else
	iw->input_field.target_symbol = NULL;
}

#if NeedFunctionPrototypes
static void ReleaseTargetSymbol(XmtInputFieldWidget iw)
#else
static void ReleaseTargetSymbol(iw)
XmtInputFieldWidget iw;
#endif
{
    if (iw->input_field.target_symbol)
	iw->input_field.target_symbol = NULL;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Overstrike(Widget w, XEvent * e, String *args, Cardinal *num)
#else
static void Overstrike(w, e, args, num)
Widget w;
XEvent *e;
String *args;
Cardinal *num;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget)w;
    Boolean overstrike;
    
    if (*num == 1) {
	if (args[0][1] == 'n') /* "On" */
	    overstrike = True;
	else
	    overstrike = False;
    }
    else
	overstrike = !iw->input_field.overstrike;
	
    XtVaSetValues(w, XmtNoverstrike, overstrike, NULL);
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
    XmtInputFieldWidget iw = (XmtInputFieldWidget) init;

    /*
     * copy the symbol names, and bind the symbols, if defined
     */
    if (iw->input_field.buffer_symbol_name)
	iw->input_field.buffer_symbol_name =
	    XtNewString(iw->input_field.buffer_symbol_name);
    BindBufferSymbol(iw);
    if (iw->input_field.target_symbol_name)
	iw->input_field.target_symbol_name =
	    XtNewString(iw->input_field.target_symbol_name);
    BindTargetSymbol(iw);
    
    /*
     * figure out the initial value.
     * if value is specified, copy it.
     * also, store it in buffer and target if those are specified.
     * otherwise, buffer contains the initial value, and we make
     * a copy of it for the value resource.  In this case target is not set.
     */
    if (iw->input_field.input) {
	iw->input_field.input = XtNewString(iw->input_field.input);
	(void)SetValueOnSymbols(iw, iw->input_field.input, False);
    }
    else if (iw->input_field.buffer_symbol)
	SetValueFromSymbol(iw);


    /* if there is a pattern, get its length */
    if (iw->input_field.pattern)
	iw->input_field.pattern_length = strlen(iw->input_field.pattern);
    else
	iw->input_field.pattern_length = 0;
    
    /*
     * figure out the maximum length of the input string.  This value can
     * come from 3 sources:
     *   the XmNmaxLength resource
     *   the length of the pattern resource
     *   the size of the buffer.
     */
    if (iw->text.max_length != MAXINT)
	iw->input_field.max_length = iw->text.max_length;
    else if (iw->input_field.pattern)
	iw->input_field.max_length = iw->input_field.pattern_length;
    else if (iw->input_field.buffer_symbol)
	iw->input_field.max_length =
	    XmtSymbolSize(iw->input_field.buffer_symbol);
    else
	iw->input_field.max_length = MAXINT;

    XmTextSetMaxLength(init, iw->input_field.overstrike?MAXINT:
		       iw->input_field.max_length);

    /*
     * if we're insensitve, set layoutSensitive, in case we're a child
     * of a layout and have a caption
     */
    if (!iw->core.sensitive || !iw->core.ancestor_sensitive)
	XtVaSetValues(init, XmtNlayoutSensitive, False, NULL);

    /* set initial value, if any */
    if (iw->input_field.input)
	SetString(init, iw->input_field.input);

    XtAddCallback(init, XmNactivateCallback, Activate, NULL);
    XtAddCallback(init, XmNlosingFocusCallback, Activate, NULL);
    XtAddCallback(init, XmNvalueChangedCallback, ValueChanged, NULL);

    /* if we're in overstrike mode, add the callback */
    if (iw->input_field.overstrike)
	XtAddCallback(init, XmNmodifyVerifyCallback, HandleOverstrike, NULL);
    
    /* if there's a pattern, add the callbacks to handle it */
    if (iw->input_field.pattern) {
	XtAddCallback(init, XmNmodifyVerifyCallback, HandlePattern, NULL);
    }

    /*
     * initialize private fields that need it
     */
    iw->input_field.input_changed = False;
    iw->input_field.pattern_skip = 0;
    iw->input_field.error_state = False;
    iw->input_field.ignore_symbol_notify = False;
}


#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) w;

    XtFree(iw->input_field.input);
    XtFree(iw->input_field.pattern);

    /* release the symbols, if bound, and free the symbol names */
    if (iw->input_field.buffer_symbol_name) {
	ReleaseBufferSymbol(iw);
	XtFree(iw->input_field.buffer_symbol_name);
    }
    if (iw->input_field.target_symbol_name) {
	ReleaseTargetSymbol(iw);
	XtFree(iw->input_field.target_symbol_name);
    }
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
    XmtInputFieldWidget cw = (XmtInputFieldWidget) current;
    XmtInputFieldWidget rw = (XmtInputFieldWidget) request;
    XmtInputFieldWidget sw = (XmtInputFieldWidget) set;

#define Current(field) (cw->input_field.field)    
#define Request(field) (rw->input_field.field)
#define Set(field) (sw->input_field.field)    
#define Changed(field) (rw->input_field.field != cw->input_field.field)

    /*
     * handle changes to the buffer or target symbol names
     */
    if (Changed(buffer_symbol_name)) {
	/* XXX do something with the new value in the symbol? */
	if (Current(buffer_symbol_name)) {
	    XtFree(Current(buffer_symbol_name));
	    ReleaseBufferSymbol(cw);
	}
	if (Set(buffer_symbol_name)) {
	    Set(buffer_symbol_name) = XtNewString(Set(buffer_symbol_name));
	    BindBufferSymbol(sw);
	}
    }
    if (Changed(target_symbol_name)) {
	if (Current(target_symbol_name)) {
	    XtFree(Current(target_symbol_name));
	    ReleaseTargetSymbol(cw);
	}
	if (Set(target_symbol_name)) {
	    Set(target_symbol_name) = XtNewString(Set(target_symbol_name));
	    BindTargetSymbol(sw);
	}
    }

    /*
     * if the value resource changes, copy the new value, set it on the
     * text widget, and copy it to the buffer, and target, if any.  Note
     * that we do not call the verifyCallback, nor do we handle errors.
     * Note that changing value updates the buffer, but changing the buffer
     * does not change the value.
     */
    if (Changed(input)) {
	XtFree(Current(input));
	if (Set(input))
	    Set(input) = XtNewString(Set(input));
	SetString(set, Set(input));
	(void)SetValueOnSymbols(sw, Set(input), False);
    }

    if (Changed(overstrike)) {
	if (Set(overstrike))
	    XtAddCallback(set, XmNmodifyVerifyCallback,
			  HandleOverstrike, NULL);
	else
	    XtRemoveCallback(set, XmNmodifyVerifyCallback,
			     HandleOverstrike, NULL);
    }

    /* if pattern changed, free old, copy new, and adjust callbacks */
    if (Changed(pattern)) {
	/* if the old pattern was NULL, add the callbacks, else free */
	if (Current(pattern) == NULL) {
	    XtAddCallback(set, XmNmodifyVerifyCallback, HandlePattern, NULL);
	}
	else
	    XtFree(Current(pattern));
	
	/* if the new pattern is NULL, remove the callbacks, else copy */
	if (Set(pattern) == NULL) {
	    XtRemoveCallback(set, XmNmodifyVerifyCallback,HandlePattern, NULL);
	}
	else
	    Set(pattern) = XtNewString(Set(pattern));

	/* get the new size of the pattern */
	if (Set(pattern)) 
	    Set(pattern_length) = strlen(Set(pattern));
	else
	    Set(pattern_length) = 0;
    }

    /*
     * update max_length from pattern or buffer size
     */
    if (sw->text.max_length != Current(max_length))
	Set(max_length) = sw->text.max_length;
    else if (Changed(pattern))
	Set(max_length) = Set(pattern_length);
    else if (Changed(buffer_symbol) && Set(buffer_symbol))
	Set(max_length) = XmtSymbolSize(Set(buffer_symbol));
    /*
     * if max_length  or overstrike changed, 
     * update the internal widget
     */
    if (Changed(max_length) || Changed(overstrike))
	XmTextSetMaxLength(set, Set(overstrike)?MAXINT:Set(max_length));
    
    /* 
     * if sensitivity changes, set the layoutSensitive resource, in case
     * we're a child of XmtLayout, and have a caption.
     */
    if ((sw->core.sensitive != cw->core.sensitive) ||
	(sw->core.ancestor_sensitive != cw->core.ancestor_sensitive)) {
	XtVaSetValues(set, XmtNlayoutSensitive,
		      sw->core.sensitive && sw->core.ancestor_sensitive, NULL);
    }

    /* never need to redraw */
    return False;

#undef Current
#undef Request
#undef Set	    
#undef Changed    
}

#if NeedFunctionPrototypes
Widget XmtCreateInputField(Widget parent, StringConst name,
			   ArgList args, Cardinal n)
#else
Widget XmtCreateInputField(parent, name, args, n)
Widget parent;
StringConst name;
ArgList args;
Cardinal n;
#endif
{
    return XtCreateWidget((String)name, xmtInputFieldWidgetClass,
			  parent, args, n);
}

/*
 * returns a string that must not be modified or freed by the caller
 */
#if NeedFunctionPrototypes
String XmtInputFieldGetString(Widget w)
#else
String XmtInputFieldGetString(w)
Widget w;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) w;

    XmtAssertWidgetClass(w, xmtInputFieldWidgetClass,"XmtInputFieldGetString");
    return iw->input_field.input;
}

#if NeedFunctionPrototypes
void XmtInputFieldSetString(Widget w, StringConst s)
#else
void XmtInputFieldSetString(w, s)
Widget w;
StringConst s;
#endif
{
    XmtInputFieldWidget iw = (XmtInputFieldWidget) w;

    XmtAssertWidgetClass(w, xmtInputFieldWidgetClass,"XmtInputFieldSetString");
    if (iw->input_field.input) {
/* _AA fix this                        */    
/* 	XtFree(iw->input_field.input); */
        iw->input_field.input = NULL;
    }    
    if (s)
	iw->input_field.input = XtNewString(s);
    else
	iw->input_field.input = NULL;
    SetString((Widget)iw, (String)s);
    (void) SetValueOnSymbols(iw, s, False);
}
    
    
/* ARGSUSED */
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
    char buf[100];

    if (type == XmtQString)
	XmtInputFieldSetString(w, *(String *)address);
    else if (type == XmtQBuffer)
	XmtInputFieldSetString(w, (char *)address);
    else if ((type == XmtQShort) || (type == XmtQPosition)) {
	(void)sprintf(buf, "%d", *(short *)address);
	XmtInputFieldSetString(w, buf);
    }
    else if (type == XmtQDimension) {
	(void)sprintf(buf, "%u", *(unsigned short *)address);
	XmtInputFieldSetString(w, buf);
    }
    else if (type == XmtQInt) {
	(void)sprintf(buf, "%d", *(int *)address);
	XmtInputFieldSetString(w, buf);
    }
    else if (type == XmtQCardinal) {
	(void)sprintf(buf, "%u", *(unsigned int *)address);
	XmtInputFieldSetString(w, buf);
    }
    else if (type == XmtQFloat) {
	(void)sprintf(buf, "%g", *(float *)address);
	XmtInputFieldSetString(w, buf);
    }
    else if (type == XmtQDouble) {
	(void)sprintf(buf, "%g", *(double *)address);
	XmtInputFieldSetString(w, buf);
    }
    else
	XmtWarningMsg("XmtInputField", "setvalue",
		      "Type mismatch:\n\tCan't set value from resource of type '%s'.  String or Buffer expected.",
		   XrmQuarkToString(type));
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
    String s = XmtInputFieldGetString(w);

    if (type == XmtQString)
	   *(String *)address = s;
    else if (type == XmtQBuffer) {
	int len = strlen(s);

	strncpy(address, s, size-1);
	((char *)address)[size-1] = '\0';
	if (len >= size)
	    XmtWarningMsg("XmtInputField", "trunc",
			  "The input value is %d characters long\n\tand does not fit into a buffer %d characters long.\n\tThe trailing characters have been truncated.",
			  len+1, size);
    }
    /* XXX
     * unsigned values should be handled better here.
     * the XmtInputField should also probably have resources to
     * set that will automatically verify that the value is valid.
     */
    else if ((type == XmtQShort) || (type == XmtQPosition) ||
	     (type == XmtQDimension))
	*(short *)address = atoi(s);
    else if ((type == XmtQInt) || (type == XmtQCardinal))
	*(int *)address = atoi(s);
    else if (type == XmtQFloat)
	*(float *)address = atof(s);
    else if (type == XmtQDouble)
	*(double *)address = atof(s);
    else
	XmtWarningMsg("XmtInputField", "getvalue",
		      "Type mismatch:\n\tCan't set input value on a resource of type '%s'.  String or Buffer expected.",
		      XrmQuarkToString(type));
}


static XmtWidgetType inputfield_widget = {
    "XmtInputField",
    (WidgetClass) &xmtInputFieldClassRec,
    NULL,
    setvalue,
    getvalue,
};

#if NeedFunctionPrototypes
void XmtRegisterInputField(void)
#else
void XmtRegisterInputField()
#endif
{
    _XmtInitQuarks();
    XmtRegisterWidgetTypes(&inputfield_widget, 1);
}

