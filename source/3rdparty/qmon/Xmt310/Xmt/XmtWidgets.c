/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmtWidgets.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmtWidgets.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>

#include <Xmt/Chooser.h>
#include <Xmt/Cli.h>
#include <Xmt/HelpBox.h>
#include <Xmt/HelpBrowser.h>
#include <Xmt/InputField.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xmt/Menu.h>
#include <Xmt/MsgLine.h>
#include <Xmt/WorkingBox.h>

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

static XmtWidgetType xmt_widgets[] = {
{"XmtCli", NULL, XmtCreateCli},
{"XmtScrolledCli", NULL, XmtCreateScrolledCli},
{"XmtHelpBox", NULL, XmtCreateHelpBox},
{"XmtHelpDialog", NULL, XmtCreateHelpDialog, NULL, NULL, True},
{"XmtHelpBrowser", NULL, XmtCreateHelpBrowser, NULL, NULL, True},
{"XmtLayout", NULL, XmtCreateLayout},
{"XmtLayoutDialog", NULL, XmtCreateLayoutDialog, NULL, NULL, True},
{"XmtLayoutBox", NULL, XmtCreateLayoutBox},
{"XmtLayoutRow", NULL, XmtCreateLayoutRow},
{"XmtLayoutCol", NULL, XmtCreateLayoutCol},
{"XmtLayoutString", NULL, XmtCreateLayoutString},
{"XmtLayoutPixmap", NULL, XmtCreateLayoutPixmap},
{"XmtLayoutSeparator", NULL, XmtCreateLayoutSeparator},
{"XmtLayoutSpace", NULL, XmtCreateLayoutSpace},
{"XmtMenubar", NULL, XmtCreateMenubar},
{"XmtMenu", NULL, XmtCreateMenubar},
{"XmtMenuPane", NULL, XmtCreateMenuPane, NULL, NULL, True},
{"XmtOptionMenu", NULL, XmtCreateOptionMenu},
{"XmtPopupMenu", NULL, XmtCreatePopupMenu, NULL, NULL, True},
{"XmtMsgLine", NULL, XmtCreateMsgLine},
{"XmtWorkingBox", NULL, XmtCreateWorkingBox},
{"XmtWorkingDialog", NULL, XmtCreateWorkingDialog, NULL, NULL, True},    
{"TopLevelShell", (WidgetClass)&topLevelShellClassRec, NULL, NULL, NULL, True},
{"TransientShell", (WidgetClass)&transientShellClassRec, NULL,NULL,NULL, True},
{"OverrideShell", (WidgetClass)&overrideShellClassRec, NULL, NULL, NULL, True},
{"ApplicationShell", (WidgetClass)&applicationShellClassRec,
     NULL, NULL, NULL, True},
};    

#if NeedFunctionPrototypes
void XmtRegisterXmtWidgets(void)
#else
void XmtRegisterXmtWidgets()
#endif
{
    XmtRegisterWidgetTypes(xmt_widgets, XtNumber(xmt_widgets));
    XmtRegisterChooser();
    XmtRegisterInputField();
}
