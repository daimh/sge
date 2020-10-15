/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutCreate.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutCreate.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/XmtP.h>
#include <Xmt/LayoutP.h>
#include <Xmt/WidgetType.h>

/*
 * These methods are registered indirectly like this so that
 * the XmtLayout widget is not dependent on, and linked with
 * all of WidgetType.c  If the widget is being used with the rest
 * of Xmt, and if the programmer wants this creation feature, he
 * can call this function to arrange for the widget to get it.
 * Note that this needs to be a separate file, or everything will
 * get linked in whether registered or not.
 */
#if NeedFunctionPrototypes
void XmtRegisterLayoutCreateMethod(void)
#else
void XmtRegisterLayoutCreateMethod()
#endif
{
    xmtLayoutClassRec.layout_class.lookup_type_proc =
	(XmtLayoutLookupTypeProc) XmtLookupWidgetType;
    xmtLayoutClassRec.layout_class.create_proc =
	(XmtLayoutCreateProc) XmtCreateWidgetType;
}
