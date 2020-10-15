/* 
 * Motif Tools Library, Version 3.1
 * $Id: All.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: All.c,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Procedures.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>

#if NeedFunctionPrototypes
void XmtRegisterAll(void)
#else
void XmtRegisterAll()
#endif
{
    XmtRegisterMotifWidgets();
    XmtRegisterXmtWidgets();

    XmtRegisterUnixProcedures();
    XmtRegisterXtProcedures();
    XmtRegisterXmtProcedures();

    /* these are also registered by XmtInitialize() */
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
    XmtRegisterPixelConverter();

    XmtRegisterLayoutParser();
    XmtRegisterLayoutCreateMethod();
}
