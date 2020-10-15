/* 
 * Motif Tools Library, Version 3.1
 * $Id: DialogP.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: DialogP.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtDialogP_h
#define _XmtDialogP_h

#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/WidgetType.h>

/*
 * We associate one of these structures with each shell widget
 * We build the structure up as we create the children.
 */
typedef struct {
    XtResourceList resources;  /* a compiled resource list */
    WidgetList widgets;        /* one child widget for each resource */
    XmtWidgetType **types;     /* a type for each widget */
    short num_resources;
    XtPointer base;            /* set by XmtDialogDo */
    Boolean return_value;
    Boolean blocking;
} XmtDialogInfo;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtDialogInfo *_XmtDialogLookupDialogInfo(Widget);
#else
extern XmtDialogInfo *_XmtDialogLookupDialogInfo();
#endif
_XFUNCPROTOEND

#endif   
