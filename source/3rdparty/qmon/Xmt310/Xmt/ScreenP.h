/* 
 * Motif Tools Library, Version 3.1
 * $Id: ScreenP.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ScreenP.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtScreenP_h
#define _XmtScreenP_h

#include <Xmt/DialogsP.h>

typedef struct {
    Widget topmost_shell;  /* used as dialog parent */
    XmtDialogCache help_dialog_cache;
    Widget working_dialog;
    Widget string_dialog;     /* dialog for XmtAskForString() */
    Widget boolean_dialog;    /* dialog for XmtAskForBoolean() */
    Widget file_dialog;       /* dialog for by XmtAskForFilename() */
    Widget item_dialog;       /* dialog for XmtAskForItem[Number]() */
/* _AA added XmtAskForTime(), XmtAskForMemory() */
    Widget time_dialog;
    Widget memory_dialog;
    Widget items_dialog;
/******_AA******************/
    struct {                  /* internal widgets of the boolean dialog */
	Widget icon, message;
	Widget yes, no, cancel, help;
	Boolean show_cancel_button;
    } boolean_internals;
    Boolean blocked;          /* flag for recursive event loops */
    XmtButtonType button;     /* which button was pressed */
    StringConst help_text;    /* help message used by _XmtHelpCallback() */
    StringConst file_mode;    /* whether and how to open the FSB file */
    FILE *selected_file;      /* temp. storage for the opened file */
} XmtPerScreenInfo;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtPerScreenInfo *XmtGetPerScreenInfo(Widget);
#else
extern XmtPerScreenInfo *XmtGetPerScreenInfo();
#endif
_XFUNCPROTOEND

#endif /* _XmtScreenP_h */
