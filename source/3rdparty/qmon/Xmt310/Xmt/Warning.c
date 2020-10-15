/* 
 * Motif Tools Library, Version 3.1
 * $Id: Warning.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Warning.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>

#if NeedFunctionPrototypes
static void PrintMsg(String name, String type, String default_msg,
		     Boolean error, va_list var)
#else
static void PrintMsg(name, type, default_msg, error, var)
String name, type, default_msg;
Boolean error;
va_list var;
#endif
{
    String *params;
    Cardinal num_params;
    register char *c;
    register int i;
    int len;
    String msg;

    /*
     * The number of arguments should equal the number of '%'
     * characters in the default string that are not followed
     * by another % character.
     */
    for(num_params = 0, c = default_msg; *c != '\0'; c++)
	if ((*c == '%') && (*(c+1) != '\0') && (*(c+1) != '%'))
	    num_params++;

    /* add one for the procedure name */
    num_params++;
    
    /*
     * allocate space for the params,
     * and initialize the array from the name and arglist
     */
    params = (String *)XtMalloc(num_params * sizeof(String));
    params[0] = name;
    for(i=1; i < num_params; i++)
	params[i] = va_arg(var, String);

    /*
     * prepend "%s: " to the default message.
     */
    len = strlen(default_msg) + 5;
    msg = XtMalloc(len);
    (void) strcpy(msg, "%s: ");
    (void) strcpy(&msg[4], default_msg);

    if (error)
	XtErrorMsg(name, type, "XmtError", msg, params, &num_params);
    else
	XtWarningMsg(name, type, "XmtWarning", msg, params, &num_params);

    XtFree((char *)params);
    XtFree(msg);
}

#if NeedVarargsPrototypes
void XmtErrorMsg(String name, String type, String default_msg, ...)
#else
void XmtErrorMsg(name, type, default_msg, va_alist)
String name, type, default_msg;
va_dcl
#endif
{
    va_list var;

    Va_start(var, default_msg);
    PrintMsg(name, type, default_msg, True, var);
    va_end(var);
}

#if NeedVarargsPrototypes
void XmtWarningMsg(String name, String type, String default_msg, ...)
#else
void XmtWarningMsg(name, type, default_msg, va_alist)
String name, type, default_msg;
va_dcl
#endif
{
    va_list var;

    Va_start(var, default_msg);
    PrintMsg(name, type, default_msg, False, var);
    va_end(var);
}
