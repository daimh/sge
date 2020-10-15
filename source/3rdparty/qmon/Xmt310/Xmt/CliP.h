/* 
 * Motif Tools Library, Version 3.1
 * $Id: CliP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: CliP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtCliP_h
#define _XmtCliP_h    

#include <Xmt/Xmt.h>
#include <Xm/TextP.h>
#include <Xm/TextOutP.h>
#include <Xmt/Cli.h>

typedef struct {
    String translations;
    XtPointer extension;
} XmtCliClassPart;

typedef struct _XmtCliClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmTextClassPart text_class;
    XmtCliClassPart cli_class;
} XmtCliClassRec;

externalref XmtCliClassRec xmtCliClassRec;

typedef struct _XmtCliPart {
    /* resources */
    Boolean save_history;     /* whether to save input in command history */
    char **history;           /* the command history */
    short history_num_items;  /* how many filled slots in the history */
    short history_max_items;  /* how many entries in command history */
    Boolean escape_newlines;  /* whether \ at end of line escapes newline */
    short save_lines;         /* minimum # of lines to save */
    Boolean display_stdout;   /* grab and display output to stdout? */
    Boolean display_stderr;   /* grab and display output to stderr? */
    int fildes;               /* display anything read from this descriptor */
    Boolean page_mode;        /* simulate the more command for long output? */
    String page_string;       /* the string to display for pager */
    XtCallbackList input_callback;  /* called when Return is typed */
    String prompt;            /* string displayed after each input */
    Widget page_widget;       /* an XmLabel widget */
    XtTranslations translations;

    /* private state */
    XmTextPosition inputpos;  /* where editable user input text begins */
    int total_lines;          /* total lines currently in widget */
    int last_read_line;       /* last line displayed, when paging */
    Boolean blocking;         /* flag when blocked in XmtCliGets */
    Boolean paging;           /* flag when paging output */
    int historypos;           /* the current location in the history */
    int pipes[2];             /* pipes for stdin and stdout */
    XtInputId stdID;          /* handle for stdin and stdout input proc */
    XtInputId fildesID;       /* handle for XmtNfildes input proc */
    int save_stdout;          /* a dup'ed copy of stdout */
    int save_stderr;          /* a dup'ed copy of stderr */
    XmTextPosition saved_cursor_position;  /* to restore cursor after drag */
    Boolean permissive;       /* disable motion & modify verify callbacks */
    Widget blank_widget;      /* to blank out the rest of the paging line */
    String input_string;      /* copy of the input string for modal input */
    Boolean has_newlines;     /* whether we need to remove \ \n sequences */
} XmtCliPart;

typedef struct _XmtCliRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmTextPart text;
    XmtCliPart cli;
} XmtCliRec;

/*
 * macros for getting at the internal state of the text widget.
 * These work in both Motif 1.1 and Motif 1.2
 */
#define XmtCliTotalLines(w) (((XmtCliWidget)(w))->text.total_lines)
#define XmtCliTopLine(w) (((XmtCliWidget)(w))->text.top_line)
#define XmtCliVBar(w) (((XmtCliWidget)(w))->text.output->data->vbar)
#define XmtCliNumRows(w)\
    (((XmtCliWidget)(w))->text.output->data->number_lines)
#define XmtCliFontList(w)\
    (((XmtCliWidget)(w))->text.output->data->fontlist)
#define XmtCliLineHeight(w)\
    (((XmtCliWidget)(w))->text.output->data->lineheight)
#define XmtCliMarginHeight(w)\
    (((XmtCliWidget)(w))->text.margin_height)
#define XmtCliMarginWidth(w)\
    (((XmtCliWidget)(w))->text.margin_width)

/*
 * 1.1 to 1.2 portability stuff
 */
#if XmVersion == 1001
#define XmGetFocusWidget(w) _XmGetActiveItem(w)
#define XmTextDisableRedisplay(w) _XmTextDisableRedisplay((XmTextWidget)w,True)
#define XmTextEnableRedisplay(w) _XmTextEnableRedisplay((XmTextWidget)w)
#endif

#endif

    
