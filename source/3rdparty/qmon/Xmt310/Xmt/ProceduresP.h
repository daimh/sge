/* 
 * Motif Tools Library, Version 3.1
 * $Id: ProceduresP.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ProceduresP.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtProceduresP_h
#define _XmtProceduresP_h

#include <Xmt/Procedures.h>

/* some quarks for special procedure argument types */
extern XrmQuark _XmtQCallbackWidget;
extern XrmQuark _XmtQCallbackData;
extern XrmQuark _XmtQCallbackAppContext;
extern XrmQuark _XmtQCallbackWindow;
extern XrmQuark _XmtQCallbackDisplay;
extern XrmQuark _XmtQCallbackUnused;

#endif
