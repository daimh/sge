/* 
 * Motif Tools Library, Version 3.1
 * $Id: Cli.c,v 1.4 2003-03-19 17:44:40 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Cli.c,v $
 * Revision 1.4  2003-03-19 17:44:40  andre
 * AA-2003-03-19-0  Enhancem.: - Darwin port
 *
 * Revision 1.3  2003/02/11 15:23:56  andre
 * AA-2003-02-11-0  Bugfix:    qmon crash fixed for pressing Why ? when several
 *                             jobs are selected.
 *                  Changed:   qmon
 *                  Bugtraq:   4816529
 *                  IZ:        #490
 *
 * Revision 1.2  2002/08/22 15:06:10  andre
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
#include <string.h>
#include <ctype.h>
#ifdef DARWIN
#include <stddef.h>
size_t wcslen(const wchar_t *s);
#else
#include <wchar.h>
#endif
#include <Xmt/XmtP.h>
#include <Xmt/CliP.h>
#include <Xmt/Converters.h>
#include <Xmt/Util.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <X11/Xos.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern int atoi();
#endif

#ifndef X_NOT_POSIX
#include <unistd.h>
#else
extern int dup(), dup2(), pipe(), read(), fflush(), close();
extern void perror();
#endif

/* make a BSD vax (and others) speak Posix */
#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif
    
/*
 * The XmtCli widget inherits the XmText widget class's tranlations,
 * and overrides them with the following translations.  This is not
 * particularly clean, but other Motif widgets do it, and it is cleaner
 * than duplicating the Xm Text translations which are probably not
 * particularly stable
 */

static char cli_translations[] = "\
~Ctrl ~Meta ~Alt<Key>osfUp:		previous-command()\n\
~Ctrl ~Meta ~Alt<Key>osfDown:		next-command()\n\
Shift<Key>osfUp:        scroll-backward()\n\
Shift<Key>osfDown:      scroll-forward()\n\
~Ctrl ~Meta ~Alt Shift<Btn1Down>:	save-cursor-pos() extend-start()\n\
~Ctrl ~Meta ~Alt<Btn1Down>:     	save-cursor-pos() grab-focus()\n\
~Ctrl ~Meta ~Alt<Btn1Motion>:		extend-adjust()\n\
~Ctrl ~Meta ~Alt<Btn1Up>:		extend-end() restore-cursor-pos()\n\
~Ctrl ~Meta ~Alt<Btn2Down>:		copy-primary()\n\
~Ctrl ~Meta ~Alt<Btn3Down>:		save-cursor-pos() extend-start()\n\
~Ctrl ~Meta ~Alt<Btn3Motion>:		extend-adjust()\n\
~Ctrl ~Meta ~Alt<Btn3Up>:		extend-end() restore-cursor-pos()\n\
~Ctrl ~Meta ~Alt<Key>Return:		page-or-end-input()\n\
~Ctrl ~Meta ~Alt<Key>osfActivate:       page-or-end-input()\n\
~Ctrl ~Meta ~Alt<Key>space:             page-or-space()\n\
";

static char empty_string[] = "";

#define offset(field) XtOffsetOf(XmtCliRec, field)
static XtResource resources[] = {
    {
	XmtNsaveHistory, XmtCSaveHistory, XtRBoolean,
	sizeof(Boolean), offset(cli.save_history),
	XtRImmediate, (XtPointer)True
    },
    {
	XmtNhistory, XtCReadOnly, XmtRStringList,
	sizeof(String *), offset(cli.history),
	XtRImmediate, NULL
    },
    {
	XmtNhistoryMaxItems, XmtCHistoryMaxItems, XtRShort,
	sizeof(short), offset(cli.history_max_items),
	XtRImmediate, (XtPointer) 50
    }, 
    {
	XmtNhistoryNumItems, XmtCHistoryNumItems, XtRShort,
	sizeof(short), offset(cli.history_num_items),
	XtRImmediate, (XtPointer) 0
    }, 
    {   /* this is a bad name, but compatible with xterm */
	XmtNsaveLines, XmtCSaveLines, XtRShort,
	sizeof(short), offset(cli.save_lines),
	XtRImmediate, (XtPointer)64
    },
    {
	XmtNprompt, XmtCPrompt, XtRString,
	sizeof(String), offset(cli.prompt),
	XtRImmediate, (XtPointer) NULL
    },
    {
	XmtNescapeNewlines, XmtCEscapeNewlines, XtRBoolean,
	sizeof(Boolean), offset(cli.escape_newlines),
	XtRImmediate, (XtPointer) True
    },
    {   
	XmtNpageMode, XmtCPageMode, XtRBoolean,
	sizeof(Boolean), offset(cli.page_mode),
	XtRImmediate, (XtPointer) False
    },
    {
	XmtNpageString, XmtCPageString, XtRString,
	sizeof(String), offset(cli.page_string),
	XtRString, (XtPointer) empty_string
    },
    {
	XmtNpageWidget, XtCReadOnly, XtRWidget,
	sizeof(Widget), offset(cli.page_widget),
	XtRImmediate, NULL
    },
    {  
	XmtNdisplayStdout, XmtCDisplayStdout, XtRBoolean,
	sizeof(Boolean), offset(cli.display_stdout),
	XtRImmediate, (XtPointer) False
    },
    {
	XmtNdisplayStderr, XmtCDisplayStderr, XtRBoolean,
	sizeof(Boolean), offset(cli.display_stderr),
	XtRImmediate, (XtPointer) False
    },
    {
	XmtNfildes, XmtCFildes, XtRInt,
	sizeof(int), offset(cli.fildes),
	XtRImmediate, (XtPointer) -1
    },
    {
	XmtNinputCallback, XtCCallback, XtRCallback,
	sizeof(XtCallbackList), offset(cli.input_callback),
	XtRPointer, (XtPointer) NULL
    },
    {
	XmtNcliTranslations, XmtCCliTranslations, XtRTranslationTable,
	sizeof(XtTranslations), offset(cli.translations),
	XtRTranslationTable, NULL
    },
    /*
     * We've got to override the XmText's default XmNeditMode of XmSINGLE_LINE,
     * because otherwise the XmNrows resource is ignored, and the user
     * will have to set this resource
     */
    {
      XmNeditMode, XmCEditMode, XmREditMode, sizeof(int),
      XtOffsetOf(struct _XmTextRec, text.edit_mode),
      XmRImmediate, (XtPointer) XmMULTI_LINE_EDIT
    },
};

    
#if NeedFunctionPrototypes
static void InitHistory(XmtCliWidget w);
static void DestroyHistory(XmtCliWidget w);
static void ReallocHistory(XmtCliWidget w);
static void AddHistoryItem(XmtCliWidget w, char *s);
static char *NextHistoryItem(XmtCliWidget w);
static char *PrevHistoryItem(XmtCliWidget w);
static void previous_command(Widget, XEvent *, String *, Cardinal *);
static void next_command(Widget, XEvent *, String *, Cardinal *);
static void EndInput(Widget, XEvent *, String *, Cardinal *);
static void SaveCursorPosition(Widget, XEvent *, String *, Cardinal *);
static void RestoreCursorPosition(Widget, XEvent *, String *, Cardinal *);
static void BeginningOfLine(Widget, XEvent *, String *, Cardinal *);
static void ScrollForward(Widget, XEvent *, String *, Cardinal *);
static void ScrollBackward(Widget, XEvent *, String *, Cardinal *);
static void PageOrEndInput(Widget, XEvent *, String *, Cardinal *);
static void PageOrSpace(Widget, XEvent *, String *, Cardinal *);
static void DoInput(XtPointer data, int *fd, XtInputId *id);
static void ShowPageWidget(XmtCliWidget);
static void HidePageWidget(XmtCliWidget);
#else
static void InitHistory();
static void DestroyHistory();
static void ReallocHistory();
static void AddHistoryItem();
static char *NextHistoryItem();
static char *PrevHistoryItem();
static void previous_command();
static void next_command();
static void EndInput();
static void SaveCursorPosition();
static void RestoreCursorPosition();
static void BeginningOfLine();
static void ScrollForward();
static void ScrollBackward();
static void PageOrEndInput();
static void PageOrSpace();
static void DoInput();
static void ShowPageWidget();
static void HidePageWidget();
#endif

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Destroy(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Resize(Widget);
#else
static void ClassPartInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Realize();
static void Resize();
#endif


static XtActionsRec cli_actions[] = {
    {"previous-command", 	previous_command},
    {"next-command", 		next_command},
    {"end-input",               EndInput},
    {"save-cursor-pos",         SaveCursorPosition},
    {"restore-cursor-pos",      RestoreCursorPosition},
    {"beginning-of-line",       BeginningOfLine},
    {"scroll-forward",          ScrollForward},
    {"scroll-backward",         ScrollBackward},
    {"page-or-end-input",       PageOrEndInput},
    {"page-or-space",           PageOrSpace},
};

#if XmVersion > 1001
static XmPrimitiveClassExtRec primClassExtRec = {
    NULL,
    NULLQUARK,
    XmPrimitiveClassExtVersion,
    sizeof(XmPrimitiveClassExtRec),
    XmInheritBaselineProc,                  /* widget_baseline */
    XmInheritDisplayRectProc,               /* widget_display_rect */
    _XmTextMarginsProc  /* this isn't inherited properly */
};
#endif

#define Superclass ((WidgetClass) &xmTextClassRec)

externaldef(xmtcliclassrec) XmtCliClassRec xmtCliClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	Superclass,
    /* class_name	  */	"XmtCli",
    /* widget_size	  */	sizeof(XmtCliRec),
    /* class_initialize   */    NULL,
    /* class_part_initiali*/	ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,
    /* realize		  */	Realize,
    /* actions		  */    cli_actions,
    /* num_actions	  */	XtNumber(cli_actions),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	XtExposeCompressMaximal,
    /* compress_enterleave*/	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */    Destroy,
    /* resize		  */	Resize,
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
   {                            /* cli class field */
      cli_translations,         /* translations */
      NULL,                     /* extension*/
   }
};

externaldef(xmtcliwidgetclass) WidgetClass xmtCliWidgetClass =
    (WidgetClass) &xmtCliClassRec;


#if NeedFunctionPrototypes
static void TrimLines(XmtCliWidget w)
#else
static void TrimLines(w)
XmtCliWidget w;
#endif
{
    char *s;
    XmTextPosition last;
    int pos, lines, insert;
    
    /*
     * This procedure ensures that there are no more than XmtNsaveLines
     * lines of text in the Cli.  The Cli maintains an approximate count of
     * the # of lines it contains, but may not be completely accurate
     * because of deletions of newlines inserted to break long lines.
     * Because the count is not accurate, we start at the end of the
     * text and scan backward counting all the lines we want to keep, rather
     * than counting forward the lines we want to discard.
     * This procedure is sort of expensive and is not intended to be called
     * every time the # of lines exceeds XmtNsaveLines.  Instead, it should
     * be called when teh # of lines exceeds XmtNsaveLines by some
     * larger threshold.
     */
    
    s = XmTextGetString((Widget) w);
    pos = last = XmTextGetLastPosition((Widget) w);
    insert = XmTextGetInsertionPosition((Widget) w);
    lines = 0;
    
    while((pos > 0) && (lines < w->cli.save_lines))
	if (s[pos--] == '\n') lines++;

    if (pos != 0) {
	XmTextDisableRedisplay((Widget)w);
	pos += 2;  /* step back over the newline. */
	/* turn off VerifyModify callback */
	w->cli.permissive = True;
	/*
	 * We could do an XmTextReplace() here, but in Motif 1.1.1, at least,
	 * that function does not update the scrollbar correctly when
	 * lines are deleted from the text widget.
	 */
	XmTextSetString((Widget)w, &s[pos]);
	w->cli.permissive = False;
	w->cli.inputpos -= pos;
	XmTextShowPosition((Widget)w, last-pos);
	XmTextSetInsertionPosition((Widget) w, insert - pos);
	XmTextEnableRedisplay((Widget) w);
    }

    w->cli.last_read_line -= (w->cli.total_lines - lines);
    w->cli.total_lines = lines;
    XtFree(s);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void VerifyCursorMotion(Widget w, XtPointer tag, XtPointer call_data)
#else
static void VerifyCursorMotion(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    Arg a;
    XmtCliWidget cw = (XmtCliWidget) w;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct *)call_data;

    /* special case when called as part of a selection */
    if (cw->cli.permissive) return;
    
    /* for scrolled cli widgets, forces a scroll to end on input */
    XmTextShowPosition(w, XmTextGetInsertionPosition(w));

    if (data->newInsert < cw->cli.inputpos) {
        data->doit = FALSE;
	XtSetArg(a, XmNcursorPosition, cw->cli.inputpos);
	XtSetValues(w, &a, 1);
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
    XmtCliWidget cw = (XmtCliWidget) w;
    XmTextVerifyCallbackStruct *data = (XmTextVerifyCallbackStruct *)call_data;

    /* special case when called from TrimLines */
    if (cw->cli.permissive) {
      return;
    }
    if (data->startPos < cw->cli.inputpos) {
        data->doit = FALSE;
#ifdef BACKSPACEBUG
	if (data->text->length == 0) data->text->ptr = NULL;
#endif	
	/* illegal input, but scroll to bottom */
	XmTextShowPosition(w, XmTextGetInsertionPosition(w)); 
    }
    else {
	/* if insertion will be allowed, count the lines */
	/* Also, ring the bell for Ctrl-G */
	int i;
	int lines, scroll_lines;

	for(i=lines=0; i < data->text->length; i++) {
	    if (data->text->ptr[i] == '\n')
		lines++;
	    if ((data->text->ptr[i] == '\007') && (cw->text.verify_bell))
		XBell(XtDisplay(w), 0);
	}
		
	cw->cli.total_lines += lines;

	/*
	 * if we're not already paging, 
	 * display all this new text, unless that would cause
	 * some of the new text to scroll off the top.  Otherwise
	 * if paging is enabled, start paging.
	 */
	if (!cw->cli.paging) {
	    if (!cw->cli.page_mode ||
		(cw->cli.total_lines-cw->cli.last_read_line<XmtCliNumRows(w)))
		XmTextShowPosition(w, XmTextGetInsertionPosition(w));
	    else {
		scroll_lines = cw->cli.last_read_line - XmtCliTopLine(cw);
		XmTextScroll(w, scroll_lines);
		cw->cli.paging = True;
		cw->cli.last_read_line += XmtCliNumRows(w)-1;
		ShowPageWidget(cw);
	    }
	}
	
	/* XXX
	 * when newlines are pasted, we should call EndInput just as when
	 *   a newline is typed, I think.  But we can't call it from here
	 *   because the widget is in a funny state, so bag it.
	 * On deletions we should check if a newline is being deleted, and
	 *   decrement the # of lines.  But the only way to do this is get
	 *   the whole string, which is too expensive.
	 * Also think about breaking long lines explictly.  This is too hard.
	 */
    }
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void SaveCursorPosition(Widget w, XEvent *e,
			       String *params, Cardinal *num_params)
#else
static void SaveCursorPosition(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;

    /* if we're not already saving something */
    if (cw->cli.saved_cursor_position == -1) {
	cw->cli.saved_cursor_position = XmTextGetInsertionPosition(w);
	/* temporarily turn off verify callbacks */
	cw->cli.permissive = True;
    }
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void RestoreCursorPosition(Widget w, XEvent *e,
				  String *params, Cardinal *num_params)
#else
static void RestoreCursorPosition(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    XmTextPosition start, end;

    /* if there is a saved position */
    if (cw->cli.saved_cursor_position != -1) {
	/* restore verify callbacks */
	cw->cli.permissive = False;
	/*
	 * if the cursor was ever outside of the currently editable
	 * section, restore it to its original position.  Otherwise,
	 * leave it alone.  In either case, forget saved position.
	 */
	if (((XmTextGetSelectionPosition(w, &start, &end))  &&
	     ((start < cw->cli.inputpos) || (end < cw->cli.inputpos))) ||
	    (XmTextGetInsertionPosition(w) < cw->cli.inputpos))
	    XmTextSetInsertionPosition(w, cw->cli.saved_cursor_position);
	cw->cli.saved_cursor_position = -1;
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void ScrollForward(Widget w, XEvent *e,
			  String *params, Cardinal *num_params)
#else
static void ScrollForward(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    int lines;
    
    if (cw->cli.paging) {
	cw->cli.paging = False;
	cw->cli.last_read_line = cw->cli.total_lines;
	HidePageWidget(cw);
    }

    lines = XmtCliNumRows(w)/2;
    if (*num_params) {
	if (isdigit(params[0][0]))
	    lines = atoi(params[0]);
	else if (strncmp(params[0], "page", 4) == 0)
	    lines *= 2;
    }
    XmTextScroll(w, lines);
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
    XmtCliWidget cw = (XmtCliWidget) w;

    XmTextShowPosition(w, cw->cli.inputpos);
    XmTextSetInsertionPosition(w, cw->cli.inputpos);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void ScrollBackward(Widget w, XEvent *e,
			   String *params, Cardinal *num_params)
#else
static void ScrollBackward(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    int lines;

    if (cw->cli.paging) {
	cw->cli.paging = False;
	cw->cli.last_read_line = cw->cli.total_lines;
	HidePageWidget(cw);
    }

    lines = XmtCliNumRows(w)/2;
    if (*num_params) {
	if (isdigit(params[0][0]))
	    lines = atoi(params[0]);
	else if (strncmp(params[0], "page", 4) == 0)
	    lines *= 2;
    }
    XmTextScroll(w, -lines);
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void PageOrEndInput(Widget w, XEvent *e,
			   String *params, Cardinal *num_params)
#else
static void PageOrEndInput(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw;

    /*
     * this action may be called by the cli widget, or by one of
     * its label children
     */
    if (!XmIsText(w)) 
	w = XtParent(w);
    cw = (XmtCliWidget) w;

    if (cw->cli.paging) {
	HidePageWidget(cw);
	XmTextScroll(w, 1);
	cw->cli.last_read_line++;
	if (XmtCliTopLine(cw) + XmtCliNumRows(cw) > cw->cli.total_lines)
	    cw->cli.paging = False;
	else
	    ShowPageWidget(cw);
    }
    else
	EndInput(w, e, params, num_params);
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void PageOrSpace(Widget w, XEvent *e,
			String *params, Cardinal *num_params)
#else
static void PageOrSpace(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw;
    
    if (!XmIsText(w)) 
	w = XtParent(w);
    cw = (XmtCliWidget) w;

    if (cw->cli.paging) {
	int lines = XmtCliNumRows(w);
	lines = (lines>2)?lines-2:1;
	HidePageWidget(cw);
	if (cw->cli.last_read_line + lines > cw->cli.total_lines) {
	    XmTextScroll(w, cw->cli.total_lines - cw->cli.last_read_line);
	    cw->cli.paging = False;
	}
	else {
	    XmTextScroll(w, lines);
	    ShowPageWidget(cw);
	}
	cw->cli.last_read_line += lines;
    }
    else
	XtCallActionProc(w, "self-insert", e, params, *num_params);
}

#if XmVersion == 1001
/*
 * this function simulates the Motif 1.2 function,
 * but is not efficient like that one is.  It is the
 * best we can do in 1.1 using only public functions.
 */
#if NeedFunctionPrototypes
static int XmTextGetSubstring(Widget w, int start, int num_char,
			      int buf_size, char *buffer)
#else
static int XmTextGetSubstring(w, start, num_char, buf_size, buffer)
Widget w;
int start;
int num_char;
int buf_size;
char *buffer;
#endif
{
    String s;

    s = XmTextGetString(w);
    if (num_char >= buf_size) num_char = buf_size-1;
    strncpy(buffer, &s[start], num_char);
    buffer[num_char] = '\0';
    XtFree(s);
    return 1;  /* XmCOPY_SUCCEEDED */
}
#endif


/*ARGSUSED*/
#if NeedFunctionPrototypes
static void EndInput(Widget w, XEvent *e,
		     String *params, Cardinal *num_params)

#else
static void EndInput(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    int len;
    char *buf;
    
    /*
     * if the widget is not editable, then there is no input to end
     * and inputCallback should never be invoked, so just return
     */
    if (!XmTextGetEditable(w)) return;

    /*
     * since the user typed something, that means that he has had time
     * to read all the text that has appeared, and we don't have to worry
     * about paging anything currently on the screen.  So we can set
     * the line field to be total_lines.
     * XXX
     * This would probably be better in VerifyModify(), if we can distinguish
     * user keystrokes from programmatic insertions...
     */
    cw->cli.last_read_line = cw->cli.total_lines;

    len = XmTextGetLastPosition(w) - cw->cli.inputpos;
    buf = XtMalloc(len+1);
    XmTextGetSubstring(w, cw->cli.inputpos, len,
		       len+1, buf);

    /* if last character was \, then this is not the end of the command line */
    if (cw->cli.escape_newlines && buf[len-1] == '\\') {
	XtFree(buf);
	XmTextInsert(w, cw->cli.inputpos+len, "\n");
	cw->cli.has_newlines = True;
	return;
    }

    /* replace escaped newlines with spaces, if necessary. */
    if (cw->cli.has_newlines) {
	char *p, *q;
	for(p = q = buf; *p; p++, q++) {
	    if (*p == '\\' && *(p+1) == '\n') {
		*q = ' ';
		p++;
	    }
	    else *q = *p;
	}
	*q = '\0';
	cw->cli.has_newlines = False;
    }

    cw->cli.inputpos += len;
    XmtCliPuts("\n", w);
    if (cw->cli.save_history) AddHistoryItem(cw, buf);

    /* if blocking for input, end the blocking, save the input string, return.
     * otherwise, invoke the callbacks, and free the string.
     */
    if (cw->cli.blocking == True) { 
	cw->cli.blocking = False;
	cw->cli.input_string = buf;
    }
    else {
	/*
	 * invoke the callbacks.
	 * force any output generated by those callbacks to the text widget.
	 * print a new prompt, if one is defined.
	 * free the input string.
	 */
	XtCallCallbackList(w, cw->cli.input_callback, buf);
	XmtCliFlush(w);
	if (cw->cli.prompt)
	    XmtCliPuts(cw->cli.prompt, w);
	XtFree(buf);
    }
}

#ifndef VMS

static Boolean stdout_grabbed = False;
static Boolean stderr_grabbed = False;

#if NeedFunctionPrototypes
static void CreatePipes(XmtCliWidget w)
#else
static void CreatePipes(w)
XmtCliWidget w;
#endif
{
    int flags;
    /*
     * create a pair of pipes.
     * put the read end in non-blocking mode,
     * and tell Xt to notify us when there is something to read.
     * If requested, stdout and stderr will be redirected to
     * the write end of this pipe pair.
     */
    pipe(w->cli.pipes);
    flags = fcntl(w->cli.pipes[0], F_GETFL, 0);
    fcntl(w->cli.pipes[0], F_SETFL, flags | O_NONBLOCK);
    w->cli.stdID = XtAppAddInput(XtWidgetToApplicationContext((Widget)w),
				 w->cli.pipes[0],
				 (XtPointer) XtInputReadMask,
				 DoInput, (XtPointer) w);
}

#if NeedFunctionPrototypes
static void ClosePipes(XmtCliWidget w)
#else
static void ClosePipes(w)
XmtCliWidget w;
#endif
{ 
    XtRemoveInput(w->cli.stdID);
    if (w->cli.pipes[0] != -1) close(w->cli.pipes[0]);
    if (w->cli.pipes[1] != -1) close(w->cli.pipes[1]);
    w->cli.pipes[0] = w->cli.pipes[1] = -1;
}


#if NeedFunctionPrototypes
static void GrabStdout(XmtCliWidget w)
#else
static void GrabStdout(w)
XmtCliWidget w;
#endif
{
    /*
     * There is only one stdout per process, so only one Cli widget can
     * grab it.  Therefore we need a flag to prevent two widgets from grabbing
     * it.  A real semaphore would be best, but we'll have to make do with
     * something close that still allows a race condition.
     */

    /* test and increment all at once. Hopefully this is an atomic operation.*/
    if (stdout_grabbed++ != False) {
	XtWarning("XmtCli: stdout is already being displayed by another widget.");
	stdout_grabbed = True;
	return;
    }

    /* if we are here, then stdout_grabbed was False, and is now True */

    /*
     * duplicate the stdout fd, to make a copy of it.  (dup)
     * close the original stdout, duplicate the write end of the pipe
     * as the new fd 1, so anyone who writes to stdout is really writing
     * to the pipe we're monitoring (dup2)
     */
    if (w->cli.pipes[1] == -1) CreatePipes(w);
    w->cli.save_stdout = dup(1);
    dup2(w->cli.pipes[1], 1);
}

#if NeedFunctionPrototypes
static void UngrabStdout(XmtCliWidget w)
#else
static void UngrabStdout(w)
XmtCliWidget w;
#endif
{
    dup2(w->cli.save_stdout, 1);
    close(w->cli.save_stdout);
    w->cli.save_stdout = -1;
    /*
     * We've just turned off stdout, if stderr is not in use either,
     * then close the pipes.  Note that we don't just test display_stderr
     * to determine this--it may have been changed by SetValues but not
     * actually processed yet.  Instead, we check save_stderr, to see
     * whether stderr has actually been turned off already.
     */
    if (w->cli.save_stderr == -1) ClosePipes(w);
    stdout_grabbed = False;
}

#if NeedFunctionPrototypes
static void GrabStderr(XmtCliWidget w)
#else
static void GrabStderr(w)
XmtCliWidget w;
#endif
{
    if (stderr_grabbed++ != False) {
	XtWarning("XmtCli: stderr is already being displayed by another widget.");
	stderr_grabbed = True;
	return;
    }

    if (w->cli.pipes[1] == -1) CreatePipes(w);
    w->cli.save_stderr = dup(2);
    dup2(w->cli.pipes[1], 2);
}

#if NeedFunctionPrototypes
static void UngrabStderr(XmtCliWidget w)
#else
static void UngrabStderr(w)
XmtCliWidget w;
#endif
{
    dup2(w->cli.save_stderr, 2);
    close(w->cli.save_stderr);
    w->cli.save_stderr = -1;
    if (w->cli.save_stdout == -1) ClosePipes(w);
    stderr_grabbed = False;
}

#if NeedFunctionPrototypes
static void GrabDescriptor(XmtCliWidget cw)
#else
static void GrabDescriptor(cw)
XmtCliWidget cw;
#endif
{
    int flags;

    /* XXX document that we set this flag */
    flags = fcntl(cw->cli.fildes, F_GETFL);
    fcntl(cw->cli.fildes, F_SETFL, flags | O_NONBLOCK);
    cw->cli.fildesID = XtAppAddInput(XtWidgetToApplicationContext((Widget)cw),
				     cw->cli.fildes,
				     (XtPointer) XtInputReadMask,
				     DoInput, (XtPointer) cw);
}

#if NeedFunctionPrototypes
static void UngrabDescriptor(XmtCliWidget cw)
#else
static void UngrabDescriptor(cw)
XmtCliWidget cw;
#endif
{
    XtRemoveInput(cw->cli.fildesID);
}

#endif /* ifndef VMS */

#if NeedFunctionPrototypes
static void PositionPageWidget(XmtCliWidget cw)
#else
static void PositionPageWidget(cw)
XmtCliWidget cw;
#endif
{
    XtMoveWidget(cw->cli.page_widget,
		 XmtCliMarginWidth(cw),
		 XmtCliMarginHeight(cw) +
		 (XmtCliNumRows(cw) -1) * XmtCliLineHeight(cw));
    XtConfigureWidget(cw->cli.blank_widget,
		      XmtCliMarginWidth(cw) + cw->cli.page_widget->core.width,
		      cw->cli.page_widget->core.y,
		      cw->core.width - 2*XmtCliMarginWidth(cw) -
		        cw->cli.page_widget->core.width,
		      cw->cli.page_widget->core.height,
		      0);
}
    
#if NeedFunctionPrototypes
static void CreatePageWidget(XmtCliWidget cw)
#else
static void CreatePageWidget(cw)
XmtCliWidget cw;
#endif
{
    XmString label = XmtCreateXmString(cw->cli.page_string);
    
    cw->cli.page_widget =
	XtVaCreateWidget("more", xmLabelWidgetClass, (Widget)cw,
			 XmNlabelString, label,
			 XmNfontList, XmtCliFontList(cw),
			 XmNforeground, cw->core.background_pixel,
			 XmNbackground, cw->primitive.foreground,
			 NULL);
    XmStringFree(label);

    /* a blank label */
    label = XmtCreateXmString(" ");
    cw->cli.blank_widget =
	XtVaCreateWidget("blank", xmLabelWidgetClass, (Widget)cw,
			 XmNlabelString, label,
			 NULL);
    XmStringFree(label);

    XtOverrideTranslations(cw->cli.page_widget, (XtTranslations)
			   ((XmtCliWidgetClass)XtClass((Widget)cw))->
			   cli_class.translations);
    XtOverrideTranslations(cw->cli.blank_widget, (XtTranslations)
			   ((XmtCliWidgetClass)XtClass((Widget)cw))->
			   cli_class.translations);
    if (cw->cli.translations) {
	XtOverrideTranslations(cw->cli.page_widget, cw->cli.translations);
	XtOverrideTranslations(cw->cli.blank_widget, cw->cli.translations);
    }
    
    if (XtIsRealized((Widget)cw)) {
	XtRealizeWidget(cw->cli.page_widget);
	XtRealizeWidget(cw->cli.blank_widget);
    }
    PositionPageWidget(cw);
}

#if NeedFunctionPrototypes
static void ShowPageWidget(XmtCliWidget cw)
#else
static void ShowPageWidget(cw)
XmtCliWidget cw;
#endif
{
    XtMapWidget(cw->cli.page_widget);
    XtMapWidget(cw->cli.blank_widget);
}

#if NeedFunctionPrototypes
static void HidePageWidget(XmtCliWidget cw)
#else
static void HidePageWidget(cw)
XmtCliWidget cw;
#endif
{
    XtUnmapWidget(cw->cli.page_widget);
    XtUnmapWidget(cw->cli.blank_widget);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void CancelPaging(Widget w, XtPointer tag, XtPointer data)
#else
static void CancelPaging(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtCliWidget cw = (XmtCliWidget)tag;

    if (cw->cli.paging) {
	cw->cli.paging = False;
	cw->cli.last_read_line = cw->cli.total_lines;
	HidePageWidget(cw);
    }
}

#if NeedFunctionPrototypes
static void ClassPartInitialize(WidgetClass c)
#else
static void ClassPartInitialize(c)
WidgetClass c;
#endif
{
    XmtCliWidgetClass wc = (XmtCliWidgetClass) c;
    XmtCliWidgetClass sc = (XmtCliWidgetClass)c->core_class.superclass;
    
    /* inherit or compile the default cli translations */
    if (wc->cli_class.translations == (String)XtInheritTranslations)
	wc->cli_class.translations = sc->cli_class.translations;
    else
	wc->cli_class.translations = (String)
	    XtParseTranslationTable(wc->cli_class.translations);
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
    XmtCliWidget cw = (XmtCliWidget) init;
    Arg al[10];
    int ac;

    /*
     * The few Cli specific translations overwrite any text widget defaults,
     * and also override any conflicting translations specified by the
     * user with the XtNtranslations resource.
     * This is not good, but it means that we can avoid duplicating
     * the Text widget translations, which are long, and probably not very
     * stable.  We just inherit the default text translations and modify
     * them as needed for the Cli.  The user can specify cli-specific
     * translations to override these default translations with the
     * XmtNcliTranslations resource.
     */
    XtOverrideTranslations(init, (XtTranslations)
			   ((XmtCliWidgetClass)XtClass(init))->
			   cli_class.translations);
    if (cw->cli.translations)
	XtOverrideTranslations(init, cw->cli.translations);
    
    cw->cli.inputpos = 0;
    cw->cli.total_lines = 0;
    cw->cli.last_read_line = 0;
    cw->cli.blocking = False;
    cw->cli.paging = False;
    cw->cli.permissive = False;
    cw->cli.has_newlines = False;
    cw->cli.page_widget = NULL;  /* don't let users set this resource */

    /* override a text widget resource */
    cw->text.auto_show_cursor_position = False;
    
    XtAddCallback(init, XmNmotionVerifyCallback, VerifyCursorMotion, NULL);
    XtAddCallback(init, XmNmodifyVerifyCallback, VerifyModify, NULL);

    InitHistory(cw);

    cw->cli.pipes[0] = cw->cli.pipes[1] = -1;
    cw->cli.save_stdout = cw->cli.save_stderr = -1;

#ifndef VMS
    if (cw->cli.display_stdout == True)
	GrabStdout(cw);

    if (cw->cli.display_stderr == True)
	GrabStderr(cw);

    if (cw->cli.fildes != -1)
	GrabDescriptor(cw);
#else /* VMS */
    if (cw->cli.display_stdout == True)
	XmtWarningMsg("XmtCli", "noStdout",
		      "XmtNdisplayStdout resource is not supported on VMS.");

    if (cw->cli.display_stderr == True)
	XmtWarningMsg("XmtCli", "noStderr",
		      "XmtNdisplayStderr resource is not supported on VMS.");

    if (cw->cli.fildes != -1)
	XmtWarningMsg("XmtCli", "noFildes",
		      "XmtNfildes resource is not supported on VMS.");
#endif /* VMS */	

    ac = 0;
    XtSetArg(al[ac], XmNcursorPositionVisible, True); ac++;
    XtSetValues(init, al, ac);

    cw->cli.saved_cursor_position = -1;  /* no cursor position saved now */

    /*
     * get a localized default value for the page_string
     * resource, if no value is set for that resource.
     */
    if (cw->cli.page_string == empty_string) 
	cw->cli.page_string =
	    XmtLocalizeWidget(init, "-- Press Spacebar for More --",
			      "pageString");

    /* copy page_string */
    if (cw->cli.page_string)
	cw->cli.page_string = XtNewString(cw->cli.page_string);

    /* copy the prompt */
    if (cw->cli.prompt)
	cw->cli.prompt = XtNewString(cw->cli.prompt);
    
    /* send the initial prompt, if any is defined */
    if (cw->cli.prompt)
	XmtCliPuts(cw->cli.prompt, init);

    if (cw->cli.page_mode)
	CreatePageWidget(cw);

    /*
     * if there is a vertical scrollbar, add a callbacks to cancel
     * paging on scroll
     */
    if (XmtCliVBar(cw)) {
	XtAddCallback(XmtCliVBar(cw), XmNvalueChangedCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNincrementCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNdecrementCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNpageIncrementCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNpageDecrementCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNtoTopCallback,
		      CancelPaging, (XtPointer)cw);
	XtAddCallback(XmtCliVBar(cw), XmNtoBottomCallback,
		      CancelPaging, (XtPointer)cw);
    }
}

#if NeedFunctionPrototypes
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;

    /* free up command line history strings */
    DestroyHistory(cw);

#ifndef VMS
    /* undo XtAppAddInput calls */
    if (cw->cli.display_stdout)
	UngrabStdout(cw);

    if (cw->cli.display_stderr) 
	UngrabStderr(cw);

    if (cw->cli.display_stdout || cw->cli.display_stderr)
	ClosePipes(cw);
    
    if (cw->cli.fildes != -1)
	XtRemoveInput(cw->cli.fildesID);
#endif /* not VMS */
    
    if (cw->cli.page_widget) {
	XtDestroyWidget(cw->cli.page_widget);
	XtDestroyWidget(cw->cli.blank_widget);
    }
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
    XmtCliWidget cw = (XmtCliWidget) current;
    XmtCliWidget nw = (XmtCliWidget) set;

#ifndef VMS
    if (nw->cli.display_stdout != cw->cli.display_stdout) {
	if (nw->cli.display_stdout)
	    GrabStdout(nw);
	else
	    UngrabStdout(nw);
    }

    if (nw->cli.display_stderr != cw->cli.display_stderr) {
	if (nw->cli.display_stderr)
	    GrabStderr(nw);
	else
	    UngrabStderr(nw);
    }

    if (nw->cli.fildes != cw->cli.fildes) {
	if (cw->cli.fildes != -1)
	    UngrabDescriptor(cw);
	if (nw->cli.fildes != -1)
	    GrabDescriptor(nw);
    }
#else /* if VMS */
    if (nw->cli.display_stdout != cw->cli.display_stdout)
	XmtWarningMsg("XmtCli", "noStdout",
		      "XmtNdisplayStdout resource is not supported on VMS.");
    if (nw->cli.display_stderr != cw->cli.display_stderr)
	XmtWarningMsg("XmtCli", "noStderr",
		      "XmtNdisplayStderr resource is not supported on VMS.");
    if (nw->cli.fildes != cw->cli.fildes)
	XmtWarningMsg("XmtCli", "noFildes",
		      "XmtNfildes resource is not supported on VMS.");
#endif /* VMS*/

    /* XmtNhistory is a read-only resource */
    if (nw->cli.history != cw->cli.history)
	nw->cli.history = cw->cli.history;
    
    if (nw->cli.history_max_items != cw->cli.history_max_items)
	ReallocHistory(nw);

    if (nw->cli.prompt != cw->cli.prompt) {
	/*
	 * the most likely time for a programmer to change the prompt
	 * is from within an input callback.  the prompt isn't printed
	 * until after those callbacks return, so the right thing
	 * will happen--we don't have to print this new prompt or replace
	 * the old one.
	 */
	XtFree(cw->cli.prompt);
	if (nw->cli.prompt) nw->cli.prompt = XtNewString(nw->cli.prompt);
    }

    /* don't let user's set this resource.  Query only */
    if (nw->cli.page_widget != cw->cli.page_widget)
	nw->cli.page_widget = cw->cli.page_widget;

    if (nw->cli.page_mode != cw->cli.page_mode) {
	if (nw->cli.page_mode && !nw->cli.page_widget)
	    CreatePageWidget(nw);
	nw->cli.last_read_line = nw->cli.total_lines;
    }

    return False;
}

#if NeedFunctionPrototypes
static void Realize(Widget w,
		    XtValueMask *value_mask, XSetWindowAttributes *attributes)
#else
static void Realize(w, value_mask, attributes)
Widget w;
XtValueMask *value_mask;
XSetWindowAttributes *attributes;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;

    /* envelop our superclass realize method */
    /* In Motif 1.2, we do a kludgy workaround of an obscure bug */
#if XmVersion != 1002
    (*Superclass->core_class.realize)(w, value_mask, attributes);
#else
    /* if we don't do this, then in Motif 1.2.0, you can't realize
     * an XmtCli widget with a prompt.  This is an XmText bug.
     */
    {
	Boolean hack;
	hack = cw->text.output->data->hasfocus;
	cw->text.output->data->hasfocus = True;
	(*Superclass->core_class.realize)(w, value_mask, attributes);
	cw->text.output->data->hasfocus = hack;
    }
#endif    
    
    /* explicitly realize our children because we are not composite */
    if (cw->cli.page_widget) {
	XtRealizeWidget(cw->cli.page_widget);
	XtRealizeWidget(cw->cli.blank_widget);
    }
}

#if NeedFunctionPrototypes
static void Resize(Widget w)
#else
static void Resize(w)
Widget w;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;

    /* envelop our superclass method */
    (*Superclass->core_class.resize)(w);

    /* position our child, if it has been created */
    if (cw->cli.page_widget)
	PositionPageWidget(cw);
}

/*
 * append string s to Cli widget w.
 * s must be a writeable string (eg. not a constant string compiled w/ gcc)
 * n must be strlen(s).
 */
#if NeedFunctionPrototypes
static void XmtCliWrite(Widget w, String s, int n)
#else
static void XmtCliWrite(w, s, n)
Widget w;
String s;
int n;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    wchar_t wcs[8*BUFSIZ];
    int mblen = 0;

    mbstowcs(wcs, s, 8*BUFSIZ-1);
#ifndef DARWIN    
    mblen = wcslen(wcs);
#else
    mblen = strlen(s);
#endif    

    /* XXX
     * we may also want to modify this routine to break long lines
     * at the last column, to avoid horizontal scrolling, which is gross.
     */
    XmTextReplaceWcs(w, cw->cli.inputpos, cw->cli.inputpos + mblen, wcs); 

/*     XmTextReplace(w, cw->cli.inputpos, cw->cli.inputpos, s); */
    cw->cli.inputpos += mblen;
    /* XXX  this is kind of a hack; Motif 1.2 seems not to
     * adjust the cursor on insertions, at least sometimes,
     * so we adjust it somewhat here
     */
    if (XmTextGetInsertionPosition(w) < cw->cli.inputpos) {
	cw->cli.permissive = True;
	XmTextSetInsertionPosition(w, cw->cli.inputpos);
	cw->cli.permissive = False;
    }

    if (!cw->cli.paging) {
	XmTextShowPosition(w, XmTextGetLastPosition(w));
	/*
	 * the lines get counted in VerifyModify
	 * if there are 1.5 times the # of lines that we are saving,
	 * get rid of some
	 */
	if (cw->cli.total_lines > cw->cli.save_lines + (cw->cli.save_lines>>1))
	    TrimLines(cw);
    }
}    

#if NeedFunctionPrototypes
void XmtCliClear(Widget w)
#else
void XmtCliClear(w)
Widget w;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;

    XmtAssertWidgetClass(w, xmtCliWidgetClass, "XmtCliClear");
    cw->cli.permissive = True;
    XmTextSetString(w, "");
    cw->cli.inputpos = 0;
    cw->cli.total_lines = 0;
    cw->cli.last_read_line = 0;
    cw->cli.permissive = False;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void DoInput(XtPointer data, int *fd, XtInputId *id)
#else
static void DoInput(data, fd, id)
XtPointer data;
int *fd;
XtInputId *id;
#endif
{
    Widget w = (Widget) data;
    char buf[512];
    int cnt;

    cnt = read(*fd, buf, 511);
    buf[cnt] = '\0';
    XmtCliWrite(w, buf, cnt);
}    

#if NeedFunctionPrototypes
void XmtCliFlush(Widget w)
#else
void XmtCliFlush(w)
Widget w;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    char buf[512];
    int cnt;

    XmtAssertWidgetClass(w, xmtCliWidgetClass, "XmtCliFlush");

    /*
     * Here we read any descriptors we are monitoring until there
     * is nothing left to read, forcing all text to appear in the text
     * widget.  We check the descriptors in a fixed order; what this
     * means is that if there was interleaved output to more than one
     * descriptor, we won't see it in the order it occured.
     */
    if (cw->cli.fildes != -1) {
	while((cnt = read(cw->cli.fildes, buf, 511)) != -1) {
	    buf[cnt] = '\0';
	    XmtCliWrite(w, buf, cnt);
	}
    }

    if (cw->cli.display_stdout || cw->cli.display_stderr) {
	while((cnt = read(cw->cli.pipes[0], buf, 511)) != -1) {
	    buf[cnt] = '\0';
	    XmtCliWrite(w, buf, cnt);
	}
    }
}

#if NeedFunctionPrototypes
void XmtCliPuts(StringConst s, Widget w)
#else
void XmtCliPuts(s, w)
StringConst s;
Widget w;
#endif
{
    int len;
    char *copy;

    XmtAssertWidgetClass(w, xmtCliWidgetClass, "XmtCliPuts");

    /* XmTextReplace doesn't work with constant strings, so make a copy. */
    len = strlen(s);
    copy = XtMalloc(len+1);
    strcpy(copy, s);
    XmtCliWrite(w, copy, len);
    XtFree(copy);
}

#if NeedVarargsPrototypes
void XmtCliPrintf(Widget w, StringConst fmt, ...)
#else
void XmtCliPrintf(w, fmt, va_alist)
Widget w;
String fmt;
va_dcl
#endif
{
    va_list arg_list;
    char buf[1024];

    XmtAssertWidgetClass(w, xmtCliWidgetClass, "XmtCliPrintf");

    Va_start(arg_list, fmt);
    (void) vsprintf(buf, fmt, arg_list);  /* XXX portability to BSD? */
    va_end(arg_list);
    buf[1023]= '\0';
    XmtCliWrite(w, buf, strlen(buf));
}


/* 
 * this is kind of a dangerous routine.
 * But useful for porting old tty type programs.
 */
#if NeedFunctionPrototypes
char *XmtCliGets(char *s, int n, Widget w)
#else
char *XmtCliGets(s, n, w)
char *s;
int n;
Widget w;
#endif
{
    XmtCliWidget cw = (XmtCliWidget) w;
    int len;
    Boolean editable, traversable;
    Widget previous_focus;

    XmtAssertWidgetClass(w, xmtCliWidgetClass, "XmtCliGets");
    if (cw->cli.display_stdout) fflush(stdout);
    if (cw->cli.display_stderr) fflush(stderr);
    
    if (cw->cli.blocking == True) {
	XtWarning("XmtCli widget already waiting for input.\n");
	s[0] = '\0';
	return s;
    }

    traversable = cw->primitive.traversal_on;
    if (!traversable) XtVaSetValues(w, XmNtraversalOn, True, NULL);
    editable = XmTextGetEditable(w);
    if (!editable) XmTextSetEditable(w, True);

    previous_focus = XmGetFocusWidget(w);
    XmProcessTraversal(w, XmTRAVERSE_CURRENT);
    XtAddGrab(w, True, False);

    cw->cli.blocking = True;

    /*
     * loop and handle events until blocking becomes False.
     * This will return after EndInput() has been called.  That action
     * handles command history for us and stores the input string
     * in the input_string field.
     */
    XmtBlock(w, &cw->cli.blocking);
    
    XtRemoveGrab(w);
    XmProcessTraversal(previous_focus, XmTRAVERSE_CURRENT);

    if (!editable) XmTextSetEditable(w, False);
    if (!traversable) XtVaSetValues(w, XmNtraversalOn, False, NULL);

    len = strlen(cw->cli.input_string);
    strncpy(s, cw->cli.input_string, n-1);
    if ((int)strlen(cw->cli.input_string) >= n) {
	XmtWarningMsg("XmtCliGets", "overflow",
		      "input longer than buffer.\n\t%d characters lost.",
		      len-n+1);
	s[n-1] = '\0';
    }
    
    return s;
}

#if NeedFunctionPrototypes
static void new_command(Widget w, char *new)
#else
static void new_command(w, new)
Widget w;
char *new;
#endif
{
    int buflen = XmTextGetLastPosition(w);
    int end = ((XmtCliWidget)w)->cli.inputpos + strlen(new);
    
    XmTextReplace(w, ((XmtCliWidget)w)->cli.inputpos, buflen, new);
    XmTextSetInsertionPosition(w, end);
    XmTextShowPosition(w, end);
}
    

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void next_command(Widget w, XEvent *e,
			 String *params, Cardinal *num_params)
#else
static void next_command(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    char *new;
    
    new = NextHistoryItem((XmtCliWidget)w);
    if (new != NULL) new_command(w,new);
}

/*ARGSUSED*/
#if NeedFunctionPrototypes
static void previous_command(Widget w, XEvent *e,
			     String *params, Cardinal *num_params)
#else
static void previous_command(w, e, params, num_params)
Widget w;
XEvent *e;
String *params;
Cardinal *num_params;
#endif
{
    char *new;

    new = PrevHistoryItem((XmtCliWidget)w);
    if (new != NULL) new_command(w, new);
}

#if NeedFunctionPrototypes
static void InitHistory(XmtCliWidget w)
#else
static void InitHistory(w)
XmtCliWidget w;
#endif
{
    w->cli.history_num_items = 0;
    w->cli.historypos = -1;
    
    if (w->cli.history_max_items == 0)
	w->cli.history = NULL;
    else
	w->cli.history = (String *) XtMalloc(w->cli.history_max_items *
					     sizeof(String));
}

#if NeedFunctionPrototypes
static void DestroyHistory(XmtCliWidget w)
#else
static void DestroyHistory(w)
XmtCliWidget w;
#endif
{
    int i;

    if (w->cli.history == NULL) return;
    
    for(i=0; i < w->cli.history_num_items; i++)
	XtFree(w->cli.history[i]);

    XtFree((char *) w->cli.history);
    w->cli.history = NULL;
}
    
/* this proc is to be called from SetValues() */
#if NeedFunctionPrototypes
static void ReallocHistory(XmtCliWidget w)
#else
static void ReallocHistory(w)
XmtCliWidget w;
#endif
{
    int i;

    /* if we shrunk, free items that no longer fit */
    for(i = w->cli.history_max_items; i < w->cli.history_num_items; i++)
	XtFree(w->cli.history[i]);

    if (w->cli.history_num_items > w->cli.history_max_items)
	w->cli.history_num_items = w->cli.history_max_items;

    if (w->cli.historypos > w->cli.history_max_items)
	w->cli.historypos = w->cli.history_max_items;

    if (w->cli.history_max_items == 0) {
	XtFree((char *)w->cli.history);
	w->cli.history = NULL;
    }
    else
	w->cli.history = (String *) XtRealloc((char *)w->cli.history,
					      w->cli.history_max_items *
					      sizeof(String));
}

#if NeedFunctionPrototypes
static void AddHistoryItem(XmtCliWidget w, char *s)
#else
static void AddHistoryItem(w, s)
XmtCliWidget w;
char *s;
#endif
{
    if (w->cli.history == NULL) return;

    /* don't save empty lines */
    if ((s == NULL) || (s[0] == '\0') || (s[0] == '\n')) return;

    /* if the history is full, free the line we are about to overwrite */
    /* and otherwise, increment the number of full history slots */
    if (w->cli.history_num_items == w->cli.history_max_items)
	XtFree(w->cli.history[w->cli.history_max_items-1]);
    else 
	w->cli.history_num_items++;

    /* move all items one slot into the past.  */
    /* NOTE: overlapping copy operation */
    memmove(&w->cli.history[1], w->cli.history, 
	    (w->cli.history_num_items-1) * sizeof(char *));

    /* now make a copy of the new line */
    w->cli.history[0] = XtMalloc(strlen(s)+1);
    strcpy(w->cli.history[0], s);

    /* and reset the history pointer to the 'present' */
    w->cli.historypos = -1;
}

#if NeedFunctionPrototypes
static char *PrevHistoryItem(XmtCliWidget w)
#else
static char *PrevHistoryItem(w)
XmtCliWidget w;
#endif
{
    if (w->cli.historypos == w->cli.history_num_items-1)
	return NULL;
    else
	w->cli.historypos++;

    return w->cli.history[w->cli.historypos];
}

#if NeedFunctionPrototypes
static char *NextHistoryItem(XmtCliWidget w)
#else
static char *NextHistoryItem(w)
XmtCliWidget w;
#endif
{
    /*
     * if we are already at the beginning, return NULL.
     * if we move back to the beginning , return "".
     * The next-command action treats these cases differently.
     */
    if (w->cli.historypos == -1)
	return NULL;
    else
	w->cli.historypos--;

    if (w->cli.historypos == -1)  /* the present, an empty line */
	return "";
    else
	return w->cli.history[w->cli.historypos];
}

#if NeedFunctionPrototypes
Widget XmtCreateCli(Widget parent, String name, ArgList args, Cardinal n)
#else
Widget XmtCreateCli(parent, name, args, n)
Widget parent;
String name;
ArgList args;
Cardinal n;
#endif
{
    return XtCreateWidget(name, xmtCliWidgetClass, parent, args, n);
}

static Arg scrolled_args[] = {
    {XmNscrollingPolicy,        (XtArgVal) XmAPPLICATION_DEFINED},
    {XmNvisualPolicy,           (XtArgVal) XmVARIABLE},
    {XmNscrollBarDisplayPolicy, (XtArgVal) XmSTATIC},
    {XmNshadowThickness,        (XtArgVal) 0}
};


#if NeedFunctionPrototypes
Widget XmtCreateScrolledCli(Widget parent, String name,
			    ArgList args, Cardinal num_args)
#else
Widget XmtCreateScrolledCli(parent, name, args, num_args)
Widget parent;
String name;
ArgList args;
Cardinal num_args;
#endif
{
    String cname, sname;
    int namelen;
    Widget scrolledw, cli;
    ArgList sargs;

    cname = name?name:"";
    namelen = strlen(cname);
    sname = XtMalloc(namelen + 3);
    strcpy(sname, cname);
    strcpy(&sname[namelen], "SW");

    sargs = XtMergeArgLists(args, num_args,
			    scrolled_args, XtNumber(scrolled_args));
    scrolledw = XtCreateManagedWidget(sname,
				      xmScrolledWindowWidgetClass,
				      parent,
				      sargs, num_args+XtNumber(scrolled_args));
    XtFree(sname);
    XtFree((char *)sargs);
    
    cli =  XtCreateWidget(name, xmtCliWidgetClass, scrolledw,
			  args, num_args);
    XtAddCallback(cli, XmNdestroyCallback,
		  (XtCallbackProc)_XmDestroyParentCallback, NULL);
    return cli;
}
