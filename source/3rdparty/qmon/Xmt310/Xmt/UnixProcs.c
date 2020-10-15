/* 
 * Motif Tools Library, Version 3.1
 * $Id: UnixProcs.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: UnixProcs.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <Xmt/Xmt.h>
#include <Xmt/Procedures.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif

#ifndef X_NOT_POSIX
#include <unistd.h>
#endif

static XmtProcedureInfo unix_procedures[] = {
{"puts", (XmtProcedure)puts, {XtRString}},
{"system", (XmtProcedure)system, {XtRString}},
{"exit", (XmtProcedure)exit, {XtRInt}},

};
#if NeedFunctionPrototypes
void XmtRegisterUnixProcedures(void)
#else
void XmtRegisterUnixProcedures()
#endif
{
    XmtRegisterProcedures(unix_procedures, XtNumber(unix_procedures));
}
