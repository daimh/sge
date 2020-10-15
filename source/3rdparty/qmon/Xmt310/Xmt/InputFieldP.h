/* 
 * Motif Tools Library, Version 3.1
 * $Id: InputFieldP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: InputFieldP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtInputFieldP_h
#define _XmtInputFieldP_h

#include <Xmt/XmtP.h>
#include <Xm/TextP.h>
#include <Xmt/InputField.h>
#include <Xmt/Symbols.h>

typedef struct _XmtInputFieldClassPart {
    XtPointer extension;
} XmtInputFieldClassPart;

typedef struct _XmtInputFieldClassRec {
    CoreClassPart core_class;
    XmPrimitiveClassPart primitive_class;
    XmTextClassPart text_class;
    XmtInputFieldClassPart input_field_class;
} XmtInputFieldClassRec;

externalref XmtInputFieldClassRec xmtInputFieldClassRec;

typedef struct _XmtInputFieldPart {
    /* resources */
    String input;               /* The string that goes in the text widget */
    String pattern;             /* allowed characters for the string */
    Boolean overstrike;         /* overstrike mode? */
    String buffer_symbol_name;  /* the symbol name for the text value */
    String target_symbol_name;  /* the symbol name for the converted value */
    Boolean auto_insert;        /* auto insert constant chars in pattern? */
    Boolean auto_delete;        /* auto delete constant chars in pattern? */
    Boolean match_all;          /* input must match all of pattern? */
    XtCallbackList input_callback;  /* called on activate or focus out */
    XtCallbackList verify_callback; /* called before stuffing into buffer */
    XtCallbackList error_callback;  /* called when new value is bad */
    Boolean beep_on_error;      /* beep on bad input? */
    Boolean replace_on_error;   /*erase bad input and replace with old value?*/
    Boolean highlight_on_error; /* highlight bad input? */
    String error_string;        /* string to display on bad input */
    Pixel error_foreground;      /* color for highlighting bad input */
    Pixel error_background;      /* color for highlighting bad input */
    /* internal state */
    Boolean input_changed;  /* whether value has changed since last stored */
    String error_value;     /* temp. storage for a bad value */
    short pattern_length;   /* strlen(pattern); */
    short pattern_skip;     /* state variable for HandlePattern() */
    Boolean error_state;    /* state variable for HandleError() & UndoError()*/
    XmtSymbol buffer_symbol;/* bound buffer symbol */
    XmtSymbol target_symbol;/* bound target symbol */
    Boolean ignore_symbol_notify;  /* prevent recursive loop */
    int max_length;         /* used with overstrike mode */
    Pixel foreground, background;  /* save old color values */
} XmtInputFieldPart;

typedef struct _XmtInputFieldRec {
    CorePart core;
    XmPrimitivePart primitive;
    XmTextPart text;
    XmtInputFieldPart input_field;
} XmtInputFieldRec;

#endif /* _XmtInputFieldP_h */
