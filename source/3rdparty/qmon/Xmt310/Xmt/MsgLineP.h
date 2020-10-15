/* 
 * Motif Tools Library, Version 3.1
 * $Id: MsgLineP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MsgLineP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtMsgLineP_h
#define _XmtMsgLineP_h    

#include <Xmt/Xmt.h>
#include <Xm/TextP.h>
#include <Xm/TextOutP.h>
#include <Xmt/MsgLine.h>

typedef struct {
    String translations;
    XtPointer extension;
} XmtMsgLineClassPart;

typedef struct _XmtMsgLineClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmTextClassPart text_class;
    XmtMsgLineClassPart msgline_class;
} XmtMsgLineClassRec;

externalref XmtMsgLineClassRec xmtMsgLineClassRec;

/* types of input we can ask for */
enum { STRING, CHAR, UNSIGNED, INT, DOUBLE };

/* types of actions we can delay */
enum { NONE, CLEAR, POP };

typedef struct _Message {
    String msg;
    short inputpos;
    short cursorpos;
    struct _Message *next;
} Message;

typedef struct _XmtMsgLinePart {
    /* resources */
    XtTranslations translations;
    Boolean allow_async_input;
    XtCallbackList input_callback;  /* called when Return is typed */

    /* private state */
    short inputpos;           /* where editable user input text begins */
    Boolean blocking;         /* flag when blocked in XmtMsgLineGetString */
    unsigned char input_type; /* STRING, CHAR, INT, or DOUBLE */
    char input_char;          /* the character we blocked for */
    short saved_cursor_position;  /* to restore cursor after drag */
    Boolean permissive;       /* disable motion & modify verify callbacks */
    Boolean canceled;         /* flag set by cancel-input action */
    XtIntervalId timer_id;    /* timer to clear msgline later */
    XtActionHookId action_hook_id; /* hook to clear msgline on activity */
    unsigned char delayed_action;  /* NONE, CLEAR, or POP */
    Message *stack;            /* a stack of messages */
    Boolean hook_being_deleted; /* workaround for an Xt bug. */
} XmtMsgLinePart;

typedef struct _XmtMsgLineRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmTextPart text;
    XmtMsgLinePart msgline;
} XmtMsgLineRec;

/*
 * 1.1 to 1.2 portability stuff
 */
#if XmVersion == 1001
#define XmGetFocusWidget(w) _XmGetActiveItem(w)
#endif

#endif

    
