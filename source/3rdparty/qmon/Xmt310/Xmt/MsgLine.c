/* 
 * Motif Tools Library, Version 3.1
 * $Id: MsgLine.c,v 1.5 2008-02-13 14:35:05 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MsgLine.c,v $
 * Revision 1.5  2008-02-13 14:35:05  andre
 * AA-2008-02-13-0: Bugfix.:   - jgdi client shall use SGE Daemon keystore for JMX ssl mode
 *                             - jmxeventmon support for SSL, eventmon script
 *                             - memory leaks qmon, cull
 *                  Changed:   jgdi, install scripts
 *                  Review:    RH
 *
 * Revision 1.4  2003/03/19 17:44:40  andre
 * AA-2003-03-19-0  Enhancem.: - Darwin port
 *
 * Revision 1.3  2003/02/11 15:23:56  andre
 * AA-2003-02-11-0  Bugfix:    qmon crash fixed for pressing Why ? when several
 *                             jobs are selected.
 *                  Changed:   qmon
 *                  Bugtraq:   4816529
 *                  IZ:        #490
 *
 * Revision 1.2  2002/08/22 15:06:11  andre
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
 
/*
 * Portions of this file are based upon work done by David Flanagan while
 * at MIT Project Athena.  See the file COPYRIGHT for the MIT copyright
 */

#include <stdio.h>
#include <ctype.h>
#ifdef DARWIN
#include <stddef.h>
size_t wcslen(const wchar_t *s);
#else
#include <wchar.h>
#endif
#include <Xmt/XmtP.h>
#include <Xmt/MsgLineP.h>
    
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern int atoi();
extern double atof();
#endif

/*
 * The XmtMsgLine widget inherits the XmText widget class's tranlations,
 * and overrides them with the following translations.  This is not
 * particularly clean, but other Motif widgets do it, and it is cleaner
 * than duplicating the Xm Text translations which are probably not
 * particularly stable
 */

static char msgline_translations[] = "\
<Key>Return:				end-input()\n\
<Key>osfCancel:				cancel-input()\n\
Ctrl<Key>C:				cancel-input()\n\
Ctrl<Key>G:                             cancel-input()\n\
~Ctrl  Shift ~Meta ~Alt<Btn1Down>:	save-cursor-pos() extend-start()\n\
~Ctrl ~Shift ~Meta ~Alt<Btn1Down>:	save-cursor-pos() grab-focus()\n\
~Ctrl ~Meta ~Alt<Btn1Motion>:		extend-adjust()\n\
~Ctrl ~Meta ~Alt<Btn1Up>:		extend-end() restore-cursor-pos()\n\
~Ctrl ~Meta ~Alt<Btn2Down>:		copy-primary()\n\
~Ctrl ~Meta ~Alt<Btn3Down>:		save-cursor-pos() extend-start()\n\
~Ctrl ~Meta ~Alt<Btn3Motion>:		extend-adjust()\n\
~Ctrl ~Meta ~Alt<Btn3Up>:		extend-end() restore-cursor-pos()\n\
<Key>Tab:                               self-insert()\
";

#define offset(field) XtOffsetOf(XmtMsgLineRec, field)
static XtResource resources[] = { 
    {
	XmtNmsgLineTranslations, XmtCMsgLineTranslations, XtRTranslationTable,
	sizeof(XtTranslations), offset(msgline.translations),
	XtRTranslationTable, NULL
    },
    {
	XmtNallowAsyncInput, XmtCAllowAsyncInput, XtRBoolean,
	sizeof(Boolean), offset(msgline.allow_async_input),
	XtRImmediate, (XtPointer)False
    },
    {
	XmtNinputCallback, XtCCallback, XtRCallback,
	sizeof(XtCallbackList), offset(msgline.input_callback),
	XtRPointer, (XtPointer) NULL
    },
};

#if NeedFunctionPrototypes
static void EndInput(Widget, XEvent *, String *, Cardinal *);
static void CancelInput(Widget, XEvent *, String *, Cardinal *);
static void SaveCursorPosition(Widget, XEvent *, String *, Cardinal *);
static void RestoreCursorPosition(Widget, XEvent *, String *, Cardinal *);
static void BeginningOfLine(Widget, XEvent *, String *, Cardinal *);
#else
static void EndInput();
static void CancelInput();
static void SaveCursorPosition();
static void RestoreCursorPosition();
static void BeginningOfLine();
#endif

static XtActionsRec msgline_actions[] = {
    {"end-input",               EndInput},
    {"cancel-input",            CancelInput},
    {"save-cursor-pos",         SaveCursorPosition},
    {"restore-cursor-pos",      RestoreCursorPosition},
    {"beginning-of-line",       BeginningOfLine},
};

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
#else
static void ClassPartInitialize();
static void Initialize();
static Boolean SetValues();
static void Destroy();
#endif

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

externaldef(xmtmsglineclassrec) XmtMsgLineClassRec xmtMsgLineClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &xmTextClassRec,
    /* class_name	  */	"XmtMsgLine",
    /* widget_size	  */	sizeof(XmtMsgLineRec),
    /* class_initialize   */    NULL,
    /* class_part_initiali*/	ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */    msgline_actions,
    /* num_actions	  */	XtNumber(msgline_actions),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	XtExposeCompressMaximal,
    /* compress_enterleave*/	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */    Destroy,
    /* resize		  */	XtInheritResize,
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_private   */	NULL,
    /* tm_table		  */	XtInheritTranslations,
    /* query_geometry     */    NULL,
    /* display accel	  */	XtInheritDisplayAccelerator,
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
   {                            /* msgline class field */
      msgline_translations,     /* translations */
      NULL,                     /* extension*/
   }
};

externaldef(xmtmsglinewidgetclass)
WidgetClass xmtMsgLineWidgetClass = (WidgetClass) &xmtMsgLineClassRec;

/* ARGSUSED */
#if NeedFunctionPrototypes
static void VerifyCursorMotion(Widget w, XtPointer tag, XtPointer call_data)
#else
static void VerifyCursorMotion(w, tag, call_data)
Widget w;
XtPointer tag, call_data;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct *)call_data;

    /* special case when called as part of a selection */
    if (mw->msgline.permissive) return;
    
    if (data->newInsert < mw->msgline.inputpos) {
        data->doit = FALSE;
	XmTextSetInsertionPosition(w, mw->msgline.inputpos);
    }
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void VerifyModify(Widget w, XtPointer tag, XtPointer call_data)
#else
static void VerifyModify(w, tag, call_data)
Widget w;
XtPointer tag, call_data;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct *)call_data;
    char c;

    /* special case */
    if (mw->msgline.permissive) return;

    if (data->startPos < mw->msgline.inputpos) {
        data->doit = False;
	return;
    }

    /* if it is a deletion, allow it */
    if (data->text->length == 0) return;

    c = data->text->ptr[0];
    
    switch(mw->msgline.input_type) {
    case STRING:
	return;
    case CHAR:
	mw->msgline.blocking = False;
	mw->msgline.input_char = c;
	return;
    case UNSIGNED:
    case INT:
    case DOUBLE:
	if (!((isdigit(c)) ||
	      ((mw->msgline.input_type == INT) && (c == '-')) ||
	      ((mw->msgline.input_type == DOUBLE) &&
	       ((c == '.') || (c == 'E') || (c == 'e') ||
		(c == '-') || (c == '+')))))
	    data->doit = False;
	return;
    }
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void SaveCursorPosition(Widget w, XEvent *e, String *p, Cardinal *np)
#else
static void SaveCursorPosition(w, e, p, np)
Widget w;
XEvent *e;
String *p;
Cardinal *np;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;

    /* if we're not already saving something */
    if (mw->msgline.saved_cursor_position == -1) {
	mw->msgline.saved_cursor_position = XmTextGetInsertionPosition(w);
	/* temporarily turn off verify callbacks */
	mw->msgline.permissive = True;
    }
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void RestoreCursorPosition(Widget w, XEvent *e, String *p, Cardinal *np)
#else
static void RestoreCursorPosition(w, e, p, np)
Widget w;
XEvent *e;
String *p;
Cardinal *np;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    XmTextPosition start, end;

    /* if there is a saved position */
    if (mw->msgline.saved_cursor_position != -1) {
	/* restore verify callbacks */
	mw->msgline.permissive = False;
	/*
	 * if the cursor was ever outside of the currently editable
	 * section, restore it to its original position.  Otherwise,
 	 * leave it alone.  In either case, forget saved position.
	 */
	if (((XmTextGetSelectionPosition(w, &start, &end))  &&
	     ((start < mw->msgline.inputpos) || (end < mw->msgline.inputpos))) ||
	    (XmTextGetInsertionPosition(w) < mw->msgline.inputpos))
	    XmTextSetInsertionPosition(w, mw->msgline.saved_cursor_position);
	mw->msgline.saved_cursor_position = -1;
    }
}


/*ARGSUSED*/
#if NeedFunctionPrototypes
static void EndInput(Widget w, XEvent *e, String *p, Cardinal *np)
#else
static void EndInput(w, e, p, np)
Widget w;
XEvent *e;
String *p;
Cardinal *np;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    char *s, buf[1024];
    int lastpos;
    
    s = XmTextGetString(w);
    strncpy(buf, &s[mw->msgline.inputpos], 1023);
    XtFree(s);

    lastpos = XmTextGetLastPosition(w);

    XmTextSetInsertionPosition(w, lastpos);
    XmTextShowPosition(w, lastpos);
    
    /* if blocking for input, end the blocking and return.
     * otherwise, invoke the callbacks to indicate that input is ready
     */
    if (mw->msgline.blocking == True) { 
	mw->msgline.blocking = False;
    }
    else {
	mw->msgline.inputpos = lastpos;
	XtCallCallbackList(w, mw->msgline.input_callback, buf);
    }

    XFlush(XtDisplay(w));
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void CancelInput(Widget w, XEvent *e, String *p, Cardinal *np)
#else
static void CancelInput(w, e, p, np)
Widget w;
XEvent *e;
String *p;
Cardinal *np;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;

    /*
     * if blocking for input, stop blocking and set canceled flag.
     * if not blocking, don't do anything.
     */
    if (mw->msgline.blocking) {
	mw->msgline.blocking = False;
	mw->msgline.canceled = True;
    }
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void BeginningOfLine(Widget w, XEvent *e, String *p, Cardinal *np)
#else
static void BeginningOfLine(w, e, p, np)
Widget w;
XEvent *e;
String *p;
Cardinal *np;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;

    XmTextShowPosition(w, mw->msgline.inputpos);
    XmTextSetInsertionPosition(w, mw->msgline.inputpos);
}

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass c)
#else
static void ClassPartInitialize(c)
WidgetClass c;
#endif
{
    XmtMsgLineWidgetClass wc = (XmtMsgLineWidgetClass) c;
    XmtMsgLineWidgetClass sc = (XmtMsgLineWidgetClass)c->core_class.superclass;
    
    /* inherit or compile the default msg line translations */
    if (wc->msgline_class.translations == (String)XtInheritTranslations)
	wc->msgline_class.translations = sc->msgline_class.translations;
    else
	wc->msgline_class.translations = (String)
	    XtParseTranslationTable(wc->msgline_class.translations);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void Initialize(Widget request, Widget init,
		       ArgList arglist, Cardinal *num_args)
#else
static void Initialize(request, init, arglist, num_args)
Widget request, init;
ArgList arglist;
Cardinal *num_args;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) init;

    XtOverrideTranslations(init, (XtTranslations)
			   ((XmtMsgLineWidgetClass)XtClass(init))->
			   msgline_class.translations);
    if (mw->msgline.translations)
	XtOverrideTranslations(init, mw->msgline.translations);
    
    mw->msgline.inputpos = 0;
    mw->msgline.blocking = False;
    mw->msgline.permissive = False;
    mw->msgline.timer_id = 0;
    mw->msgline.action_hook_id = NULL;
    mw->msgline.hook_being_deleted = False;
    mw->msgline.delayed_action = NONE;
    mw->msgline.stack = NULL;
    
    XtAddCallback(init, XmNmotionVerifyCallback, VerifyCursorMotion, NULL);
    XtAddCallback(init, XmNmodifyVerifyCallback, VerifyModify, NULL);

    XtVaSetValues(init,
		  XmNcursorPositionVisible, mw->msgline.allow_async_input,
		  XmNeditable, mw->msgline.allow_async_input,
		  XmNtraversalOn, mw->msgline.allow_async_input,
		  NULL);

    mw->msgline.saved_cursor_position = -1;  /* no cursor position saved now */
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
    XmtMsgLineWidget cw = (XmtMsgLineWidget) current;
    XmtMsgLineWidget nw = (XmtMsgLineWidget) set;

    if (nw->msgline.allow_async_input != cw->msgline.allow_async_input) 
	XtVaSetValues(set,
		      XmNcursorPositionVisible, nw->msgline.allow_async_input,
		      XmNeditable, nw->msgline.allow_async_input,
		      XmNtraversalOn, nw->msgline.allow_async_input,
		      NULL);
    return False;
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    Message *m;

    while(mw->msgline.stack) {
	m = mw->msgline.stack;
	mw->msgline.stack = m->next;
	XtFree(m->msg);
	XtFree((char *)m);
    }
    if (mw->msgline.timer_id)
	XtRemoveTimeOut(mw->msgline.timer_id);
    if (mw->msgline.action_hook_id)
	XtRemoveActionHook(mw->msgline.action_hook_id);
}

#if NeedFunctionPrototypes
static void _XmtMsgLineClear(XmtMsgLineWidget);
static void _XmtMsgLinePop(XmtMsgLineWidget);
#else
static void _XmtMsgLineClear();
static void _XmtMsgLinePop();
#endif

/*
 * We can't remove the action hook in handle_delayed_action, because of
 * a bug in Xt (X11R5 & X11R6, at least) in which removing an
 * action hook from an action hook (action_hook() calls handle_delayed)
 * frees an entry in a linked list of action hooks while the
 * Intrinsics are traversing that list of hooks.  The list gets
 * corrupted on some platforms and causes a crash.
 *
 * So we register a zero-length timeout to remove the
 * hook the next time control returns to the event loop.
 * If the action hook is triggered again before the timeout
 * is triggered to remove the hook, we need to prevent it from
 * doing anything, by checking some kind of flag.  We also
 * need to prevent multiple action hooks from being registered.
 * So we'll use the hook_being_deleted flag.  This flag has 4 uses:
 * 1) action_hook() will do nothing if hook_being_deleted is True.
 * 2) register_delayed_action() won't register a new hook if
 *    hook_being_deleted is True; it will just set
 *    hook_being_deleted to False and use the existing hook.
 * 3) remove_action_hook() will only remove the action hook if
 *    hook_being_deleted is True, and it will set the flag to False
 *    before it actually removes the hook (for thread safeness).
 * 4) handle_delayed_action will set the hook_being_deleted when it
 *    wants to turn off the action hook.  At the same time, it will
 *    arrange for remove_action_hook() to be called when it is safe.
 */



#if NeedFunctionPrototypes
static void remove_action_hook(XtPointer client_data, XtIntervalId *id)
#else
static void remove_action_hook(client_data, id)
XtPointer client_data;
XtIntervalId *id;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) client_data;
    XtActionHookId action_id;

    /*
     * if hook_being_deleted is False, then the hook has been
     * re-registered, and it shouldn't be removed now.
     */
    if (mw->msgline.hook_being_deleted) {
	action_id = mw->msgline.action_hook_id;
	mw->msgline.action_hook_id = 0;
	mw->msgline.hook_being_deleted = False;
	XtRemoveActionHook(action_id);
    }
}

#if NeedFunctionPrototypes
static void handle_delayed_action(XmtMsgLineWidget mw)
#else
static void handle_delayed_action(mw)
XmtMsgLineWidget mw;
#endif
{
    /* do the action */
    if (mw->msgline.timer_id ||
	(mw->msgline.action_hook_id && !mw->msgline.hook_being_deleted)) {
	if (mw->msgline.delayed_action == CLEAR)
	    _XmtMsgLineClear(mw);
	else if (mw->msgline.delayed_action == POP)
	    _XmtMsgLinePop(mw);
	mw->msgline.delayed_action = NONE;
    }
    
    /* remove any handlers for the action */
    if (mw->msgline.timer_id) {
	/* sometimes this removal will be superfluous */
	XtRemoveTimeOut(mw->msgline.timer_id);
	mw->msgline.timer_id = 0;
    }
    if (mw->msgline.action_hook_id && !mw->msgline.hook_being_deleted) {
	/*
	 * We can't remove the action hook directly, but we can
	 * set a flag that says it is removed, and remove it later.
	 */
	mw->msgline.hook_being_deleted = True;
	XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)mw),
			0, remove_action_hook,(XtPointer)mw);
    }
}    

/* ARGSUSED */
#if NeedFunctionPrototypes
static void action_hook(Widget w, XtPointer data, String action,
			XEvent *event, String *params, Cardinal *num_params)
#else
static void action_hook(w, data, action, event, params, num_params)
Widget w;
XtPointer data;
String action;
XEvent *event;
String *params;
Cardinal *num_params;
#endif
{
    /*
     * If hook_being_deleted is True, we pretend that this action hook
     * has already been removed.  This is a workaround for an Xt bug.
     * See the long comment above.
     */
    if (((XmtMsgLineWidget)data)->msgline.hook_being_deleted) return;

    /*
     * Key and button press events will clear or pop the message line,
     * but key and button release, enter, leave, focus, etc., will not.
     */
    if ((event->type == KeyPress) || (event->type == ButtonPress))
	handle_delayed_action((XmtMsgLineWidget)data);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void timer_proc(XtPointer data, XtIntervalId *id)
#else
static void timer_proc(data, id)
XtPointer data;
XtIntervalId *id;
#endif
{
    handle_delayed_action((XmtMsgLineWidget)data);
}

#if NeedFunctionPrototypes
static void register_delayed_action(XmtMsgLineWidget mw, int when, int action)
#else
static void register_delayed_action(mw, when, action)
XmtMsgLineWidget mw;
int when;
int action;
#endif
{
    /*
     * if there is a different action pending, do it first
     */
    if (mw->msgline.delayed_action && (mw->msgline.delayed_action != action))
	handle_delayed_action(mw);

    /*
     * If there is an action of the same kind pending, and this
     * is a request for an immediate action, just do the pending one now.
     * If the action is being delayed in the same way a second time,
     * we ignore this request, otherwise we register a handler for it.
     */
    if (mw->msgline.delayed_action == action) { 
	if (when == XmtMsgLineNow) {
	    handle_delayed_action(mw);
	    return;
	}
	else if ((mw->msgline.action_hook_id && (when == XmtMsgLineOnAction))||
		 (mw->msgline.timer_id && (when > 0)))
	    return;
    }
    else if (when != XmtMsgLineNow)
	mw->msgline.delayed_action = action;

    switch(when) {
    case XmtMsgLineNow:
	if (action == CLEAR)
	    _XmtMsgLineClear(mw);
	else if (action == POP)
	    _XmtMsgLinePop(mw);
	break;
    case XmtMsgLineOnAction:
	/*
	 * If an action hook is waiting to be removed, then just reuse it
	 * instead of registering a new one.
	 */
	if (mw->msgline.hook_being_deleted)
	    mw->msgline.hook_being_deleted = False;
	else
	    mw->msgline.action_hook_id =
		XtAppAddActionHook(XtWidgetToApplicationContext((Widget)mw),
				   action_hook, (XtPointer)mw);
	break;
    default:
	mw->msgline.timer_id =
	    XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)mw),
			    when, timer_proc, (XtPointer)mw);
	break;
    }
}


#if NeedFunctionPrototypes
static void _XmtMsgLineClear(XmtMsgLineWidget w)
#else
static void _XmtMsgLineClear(w)
XmtMsgLineWidget w;
#endif
{
    w->msgline.permissive = True;
    XmTextSetString((Widget)w, "");
    XFlush(XtDisplay((Widget) w));
    w->msgline.permissive = False;
    w->msgline.inputpos = 0;
}

#if NeedFunctionPrototypes
void XmtMsgLineClear(Widget w, int when)
#else
void XmtMsgLineClear(w, when)
Widget w;
int when;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineClear");
    register_delayed_action(mw, when, CLEAR);
}

#if NeedFunctionPrototypes
void XmtMsgLineSet(Widget w, StringConst s)
#else
void XmtMsgLineSet(w, s)
Widget w;
StringConst s;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;

    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineSet");
    handle_delayed_action(mw);
    mw->msgline.permissive = True;
    XmTextSetString(w, (String)s);
    mw->msgline.permissive = False;
    mw->msgline.inputpos = strlen(s);
    XmTextSetInsertionPosition(w, mw->msgline.inputpos);
    XmTextShowPosition(w, mw->msgline.inputpos);
    XFlush(XtDisplay(w));
}


#if NeedFunctionPrototypes
void XmtMsgLineAppend(Widget w, StringConst s)
#else
void XmtMsgLineAppend(w,s)
Widget w;
StringConst s;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    wchar_t wcs[8*BUFSIZ];
    int mblen = 0;

    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineAppend");
    handle_delayed_action(mw);
    mbstowcs(wcs, s, 8*BUFSIZ-1);
#ifndef DARWIN    
    mblen = wcslen(wcs);
#else
    mblen = strlen(s);
#endif    
/*     XmTextInsert(w, mw->msgline.inputpos, (String)s); */
    XmTextInsertWcs(w, mw->msgline.inputpos, wcs);
    mw->msgline.inputpos += mblen;
    if (XmTextGetInsertionPosition(w) < mw->msgline.inputpos)
	XmTextSetInsertionPosition(w, mw->msgline.inputpos);
    XmTextShowPosition(w, mw->msgline.inputpos);
    XFlush(XtDisplay(w));
}

#if NeedVarargsPrototypes
void XmtMsgLinePrintf(Widget w, StringConst fmt, ...)
#else
void XmtMsgLinePrintf(w, fmt, va_alist)
Widget w;
String fmt;
va_dcl
#endif
{
    va_list arg_list;
    char buf[1024];

    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLinePrintf");
    Va_start(arg_list, fmt);
    (void) vsprintf(buf, fmt, arg_list);  /* XXX portability to BSD? */
    va_end(arg_list);
    XmtMsgLineAppend(w, buf);
}
    
#if NeedFunctionPrototypes
void XmtMsgLinePush(Widget w)
#else
void XmtMsgLinePush(w)
Widget w;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    Message *m = XtNew(Message);
    
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLinePush");

    handle_delayed_action(mw);
    m->msg = XmTextGetString(w);
    m->inputpos = mw->msgline.inputpos;
    m->cursorpos = XmTextGetInsertionPosition(w);
    m->next = mw->msgline.stack;
    mw->msgline.stack = m;
}

#if NeedFunctionPrototypes
static void _XmtMsgLinePop(XmtMsgLineWidget mw)
#else
static void _XmtMsgLinePop(mw)
XmtMsgLineWidget mw;
#endif
{
    Message *m;
    
    if (mw->msgline.stack) {
	m = mw->msgline.stack;
	mw->msgline.stack = m->next;
	mw->msgline.permissive = True;
	XmTextSetString((Widget)mw, m->msg);
	mw->msgline.permissive = False;
	mw->msgline.inputpos = m->inputpos;
	XmTextSetInsertionPosition((Widget)mw, m->cursorpos);
	XmTextShowPosition((Widget)mw, m->cursorpos);
	XFlush(XtDisplay((Widget)mw));
	XtFree(m->msg);
	XtFree((char *)m);
    }
}

#if NeedFunctionPrototypes
void XmtMsgLinePop(Widget w, int when)
#else
void XmtMsgLinePop(w, when)
Widget w;
int when;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLinePop");
    register_delayed_action(mw, when, POP);
}
    

#if NeedFunctionPrototypes
void XmtMsgLineSetInput(Widget w, StringConst s)
#else
void XmtMsgLineSetInput(w, s)
Widget w;
StringConst s;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    int pos; 

    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineSetInput");
    handle_delayed_action(mw);
    XmTextReplace(w, mw->msgline.inputpos, XmTextGetLastPosition(w),(String)s);
    pos = mw->msgline.inputpos + strlen(s);
    XmTextSetInsertionPosition(w, pos);
    XmTextShowPosition(w, pos);
    XFlush(XtDisplay(w));
}

#if NeedFunctionPrototypes
String XmtMsgLineGetInput(Widget w)
#else
String XmtMsgLineGetInput(w)
Widget w;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    String input;
    int len; 

    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineSetInput");
    handle_delayed_action(mw);

    len = XmTextGetLastPosition(w) - mw->msgline.inputpos;
    input = XtMalloc(len+1);
#if XmVersion == 1001
    {
	String s = XmTextGetString(w);
	strcpy(input, &s[mw->msgline.inputpos]);
	XtFree(s);
    }
#else
    XmTextGetSubstring(w, mw->msgline.inputpos, len, len+1, input);
#endif
    return input;
}


#if NeedFunctionPrototypes
static Boolean BlockForInput(Widget w, String buf, int len,
			     char *char_return, int *int_return,
			     unsigned *unsigned_return, double *double_return,
			     int type)
#else
static Boolean BlockForInput(w, buf, len, char_return, int_return,
			     unsigned_return, double_return, type)
Widget w;
String buf;
int len;
char *char_return;
int *int_return;
unsigned *unsigned_return;
double *double_return;
int type;
#endif
{
    XmtMsgLineWidget mw = (XmtMsgLineWidget) w;
    Widget previous_focus;
    String s;

    handle_delayed_action(mw);
    
    if (mw->msgline.blocking) /* This should never happen if the grab works */
	return False;
    
    mw->msgline.input_type = type;
    mw->msgline.canceled = False;
    mw->msgline.blocking = True;

    if (!mw->msgline.allow_async_input)
	XtVaSetValues(w,
		      XmNcursorPositionVisible, True,
		      XmNeditable, True,
		      XmNtraversalOn, True,
		      NULL);

    previous_focus = XmGetFocusWidget(w);
    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
    XtAddGrab(w, True, False); 

    XmtBlock(w, &mw->msgline.blocking);

    XtRemoveGrab(w);
    XmProcessTraversal(previous_focus, XmTRAVERSE_CURRENT);

    if (!mw->msgline.allow_async_input)
	XtVaSetValues(w,
		      XmNcursorPositionVisible, False,
		      XmNeditable, False,
		      XmNtraversalOn, False,
		      NULL);

    if (mw->msgline.canceled) {
	/* erase canceled input */
	mw->msgline.permissive = True;
	XmTextReplace(w, mw->msgline.inputpos,
		      XmTextGetLastPosition(w), "");
	XFlush(XtDisplay(w));
	mw->msgline.permissive = False;
	return False;
    }

    s = XmtMsgLineGetInput(w);
    mw->msgline.inputpos = XmTextGetLastPosition(w);
    
    switch(type) {
    case STRING:
	strncpy(buf, s, len-1);
	buf[len-1] = '\0';
	break;
    case CHAR:
	*char_return = mw->msgline.input_char;
	break;
    case UNSIGNED:
	*unsigned_return = atoi(s);
    case INT:
	*int_return = atoi(s);
	break;
    case DOUBLE:
	*double_return = atof(s);
	break;
    }

    XtFree(s);
    return True;
}

#if NeedFunctionPrototypes
String XmtMsgLineGetString(Widget w, String buf, int len)
#else
String XmtMsgLineGetString(w, buf, len)
Widget w;
String buf;
int len;
#endif
{
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineGetString");
    
    if (BlockForInput(w, buf, len, NULL, NULL, NULL, NULL, STRING))
	return buf;
    else
	return NULL;
}

#if NeedFunctionPrototypes
int XmtMsgLineGetChar(Widget w)
#else
int XmtMsgLineGetChar(w)
Widget w;
#endif
{
    char c;
    
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineGetChar"); 

    if (BlockForInput(w, NULL, 0, &c, NULL, NULL, NULL, CHAR))
	return c;
    else
	return EOF;
}

#if NeedFunctionPrototypes
Boolean XmtMsgLineGetUnsigned(Widget w, unsigned *u)
#else
Boolean XmtMsgLineGetUnsigned(w, u)
Widget w;
unsigned *u;
#endif
{
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineGetUnsigned"); 
    return BlockForInput(w, NULL, 0, NULL, NULL, u, NULL, UNSIGNED);
}

#if NeedFunctionPrototypes
Boolean XmtMsgLineGetInt(Widget w, int *i)
#else
Boolean XmtMsgLineGetInt(w, i)
Widget w;
int *i;
#endif
{
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineGetInt"); 
    return BlockForInput(w, NULL, 0, NULL, i, NULL, NULL, INT);
}

#if NeedFunctionPrototypes
Boolean XmtMsgLineGetDouble(Widget w, double *d)
#else
Boolean XmtMsgLineGetDouble(w, d)
Widget w;
double *d;
#endif
{
    XmtAssertWidgetClass(w, xmtMsgLineWidgetClass, "XmtMsgLineGetDouble"); 
    return BlockForInput(w, NULL, 0, NULL, NULL, NULL, d, DOUBLE);
}


#if NeedFunctionPrototypes
Widget XmtCreateMsgLine(Widget parent, String name,
			ArgList args, Cardinal num_args)
#else
Widget XmtCreateMsgLine(parent, name, args, num_args)
Widget parent;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    return XtCreateWidget(name, xmtMsgLineWidgetClass, parent, args, num_args);
}
   
