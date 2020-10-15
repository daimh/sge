/* 
 * Motif Tools Library, Version 3.1
 * $Id: Initialize.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Initialize.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */


/*
 * Some of the code in this file is derived from the Xt sources:
 * Xt/Initialize.c and Xt/Display.c
 */

#include <Xmt/Xmt.h>
#include <Xmt/AppRes.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>

#if NeedFunctionPrototypes
Widget XmtInitialize(XtAppContext *app_context_return,
		     String application_class,
		     XrmOptionDescList options, Cardinal num_options,
		     int *argc_in_out, String *argv_in_out,
		     String *fallback_resources,
		     ArgList args, Cardinal num_args)
#else
Widget XmtInitialize(app_context_return, application_class,
		     options, num_options, argc_in_out, argv_in_out,
		     fallback_resources, args, num_args)
XtAppContext *app_context_return;
String application_class;
XrmOptionDescList options;
Cardinal num_options;
int *argc_in_out;
String *argv_in_out;
String *fallback_resources;
ArgList args;
Cardinal num_args;
#endif
{
    Widget root;

    /*
     * Initialize Xt and create the root shell widget.
     */
    root = XtAppInitialize(app_context_return,
			   application_class,
			   options, num_options,
			   argc_in_out, argv_in_out,
			   fallback_resources,
			   args, num_args);

    /*
     * parse Xmt-specific command-line options
     */
    XmtParseCommandLine(root, argc_in_out, argv_in_out);

    /*
     * register some type converters.
     * We do this now so we can convert
     * Xmt application resources read below.
     */
    XmtRegisterPixelConverter();
    XmtRegisterBitmapConverter();
    XmtRegisterBitmaskConverter();
    XmtRegisterPixmapConverter();
    XmtRegisterColorTableConverter();
    XmtRegisterWidgetConverter();
    XmtRegisterCallbackConverter();
    XmtRegisterXmStringConverter(); 
    XmtRegisterXmFontListConverter();
    XmtRegisterStringListConverter();
    XmtRegisterMenuItemsConverter();
    XmtRegisterPixmapListConverter();

    /*
     * These aren't strictly type converters, but
     * the idea is the same.
     */
    XmtRegisterLayoutParser();
    XmtRegisterLayoutCreateMethod();

    /*
     * Now register the shell, and look up the app-resources
     * (and do app-resource-related initialization.)
     */
    XmtInitializeApplicationShell(root, args, num_args);

    return(root);
}
