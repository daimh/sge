/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmtProcs.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmtProcs.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/Xmt.h>
#include <Xmt/Procedures.h>
#include <Xmt/Dialogs.h>
#include <Xmt/Dialog.h>
#include <Xmt/SetValue.h>
#include <Xmt/Util.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Converters.h>
#include <Xmt/Create.h>
#include <Xmt/Help.h>
#include <Xmt/MsgLine.h>
#include <Xmt/Cli.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>
#include <Xmt/Layout.h>

#if NeedFunctionPrototypes
static void AddDeleteCallback(Widget shell, int response,
			      XtCallbackList list)
#else
static void AddDeleteCallback(shell, response, list)
Widget shell;
int response;
XtCallbackList list;
#endif
{
    XmtAddDeleteCallback(shell, response, list[0].callback, list[0].closure);
}


#if NeedFunctionPrototypes
static void AddSaveYourselfCallback(Widget shell, XtCallbackList list)
#else
static void AddSaveYourselfCallback(shell, list)
Widget shell;
XtCallbackList list;
#endif
{
    XmtAddSaveYourselfCallback(shell, list[0].callback, list[0].closure);
}


/* XXX
 * Xmt procedures associated with Xmt widgets could be automatically
 * registered in the widget class initialize procedure.  This is cool
 * because it means that the functions will always be registered if the
 * widget is used, but that there is no generic registration call that
 * will link in the widget even if it is unused.
 * This means that
 * the widgets will have to be shipped around with the procedure registration
 * code.  But this seems pretty reasonable.  We could define a symbol
 * NOT_XMT_LIBRARY that would remove those dependencies.
 */

static XmtProcedureInfo xmt_procedures[] = {
{"XmtDisplayError", (XmtProcedure)XmtDisplayError,
 {XmtRCallbackWidget, XtRString, XtRString}},
{"XmtDisplayWarning", (XmtProcedure)XmtDisplayWarning,
 {XmtRCallbackWidget, XtRString, XtRString}},
{"XmtDisplayInformation", (XmtProcedure)XmtDisplayInformation,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString}},
{"XmtDisplayWarningMsg", (XmtProcedure)XmtDisplayWarningMsg,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString, XtRString}},
{"XmtDisplayErrorMsg", (XmtProcedure)XmtDisplayErrorMsg,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString, XtRString}},
{"XmtDisplayInformationMsg", (XmtProcedure)XmtDisplayInformationMsg,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString, XtRString}},
{"XmtDisplayWarningMsgAndWait", (XmtProcedure)XmtDisplayWarningMsgAndWait,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString, XtRString}},
{"XmtDisplayErrorMsgAndWait", (XmtProcedure)XmtDisplayErrorMsgAndWait,
 {XmtRCallbackWidget, XtRString, XtRString, XtRString, XtRString}},

{"XmtDialogOkayCallback", (XmtProcedure)XmtDialogOkayCallback,
 {XmtRCallbackWidget}},
{"XmtDialogCancelCallback", (XmtProcedure)XmtDialogCancelCallback,
 {XmtRCallbackWidget}},
{"XmtDialogApplyCallback", (XmtProcedure)XmtDialogApplyCallback,
 {XmtRCallbackWidget}},
{"XmtDialogDoneCallback", (XmtProcedure)XmtDialogDoneCallback,
 {XmtRCallbackWidget}},
{"XmtDialogResetCallback", (XmtProcedure)XmtDialogResetCallback,
 {XmtRCallbackWidget}},

{"XmtSetValue", (XmtProcedure)XmtSetValue, {XtRWidget, XtRString, XtRString}},
{"XmtSetTypedValue", (XmtProcedure)XmtSetTypedValue,
   {XtRWidget, XtRString, XtRString, XtRString}},

{"XmtDisplayBusyCursor", (XmtProcedure)XmtDisplayBusyCursor, {XtRWidget}},
{"XmtDisplayDefaultCursor", (XmtProcedure)XmtDisplayDefaultCursor,{XtRWidget}},
{"XmtDisplayCursor", (XmtProcedure)XmtDisplayCursor,{XtRWidget, XtRCursor}},
{"XmtSetInitialFocus",(XmtProcedure)XmtSetInitialFocus, {XtRWidget,XtRWidget}},
{"XmtWaitUntilMapped", (XmtProcedure)XmtWaitUntilMapped, {XtRWidget}},
{"XmtDiscardButtonEvents", (XmtProcedure)XmtDiscardButtonEvents, {XtRWidget}},
{"XmtDiscardKeyPressEvents", (XmtProcedure)XmtDiscardKeyPressEvents,
   {XtRWidget}},

{"XmtIconifyShell", (XmtProcedure)XmtIconifyShell, {XtRWidget}},
{"XmtDeiconifyShell", (XmtProcedure)XmtDeiconifyShell, {XtRWidget}},
{"XmtRaiseShell", (XmtProcedure)XmtRaiseShell, {XtRWidget}},
{"XmtLowerShell", (XmtProcedure)XmtLowerShell, {XtRWidget}},
{"XmtFocusShell", (XmtProcedure)XmtFocusShell, {XtRWidget}},
{"XmtSetFocusToShell", (XmtProcedure)XmtSetFocusToShell, {XtRWidget}},
{"XmtWarpToShell", (XmtProcedure)XmtWarpToShell, {XtRWidget}},
{"XmtMoveShellToPointer", (XmtProcedure)XmtMoveShellToPointer, {XtRWidget}},

{"XmtDialogPosition", (XmtProcedure)XmtDialogPosition, {XtRWidget, XtRWidget}},

{"XmtRegisterImprovedIcons", (XmtProcedure)XmtRegisterImprovedIcons,
 {XtRWidget, XmtRXmtColorTable}},

{"XmtCreateChildren", (XmtProcedure)XmtCreateChildren, {XtRWidget}},
{"XmtCreateChild", (XmtProcedure)XmtCreateChild, {XtRWidget,XtRString}},
{"XmtBuildDialog", (XmtProcedure)XmtBuildDialog,
 {XtRWidget, XtRString, XmtRCallbackUnused, XmtRCallbackUnused}},
{"XmtBuildToplevel", (XmtProcedure)XmtBuildToplevel, {XtRWidget, XtRString}},
{"XmtBuildApplication", (XmtProcedure)XmtBuildApplication,
 {XtRString, XtRString, XmtRCallbackDisplay,
      XmtRCallbackUnused, XmtRCallbackUnused}},

{"XmtAddDeleteCallback", (XmtProcedure)AddDeleteCallback,
 {XtRWidget, XmRDeleteResponse, XtRCallback}},
{"XmtAddSaveYourselfCallback", (XmtProcedure)AddSaveYourselfCallback,
 {XtRWidget, XtRCallback}},

{"XmtHelpDisplayContextHelp", (XmtProcedure)XmtHelpDisplayContextHelp,
 {XtRWidget}},
{"XmtHelpDoContextHelp", (XmtProcedure)XmtHelpDoContextHelp,
 {XmtRCallbackWidget}},
{"XmtHelpContextHelpCallback", (XmtProcedure)XmtHelpContextHelpCallback,
 {XmtRCallbackWidget, XmtRCallbackUnused, XmtRCallbackUnused}},

/* XmtMsgLine widget functions */
{"XmtMsgLineClear", (XmtProcedure)XmtMsgLineClear, {XtRWidget, XtRInt}},
{"XmtMsgLineSet", (XmtProcedure) XmtMsgLineSet, {XtRWidget, XtRString}},
{"XmtMsgLineAppend", (XmtProcedure)XmtMsgLineAppend, {XtRWidget, XtRString}},
{"XmtMsgLinePrintf", (XmtProcedure)XmtMsgLinePrintf,
 {XtRWidget, XtRString, XmtRCallbackUnused}},
{"XmtMsgLinePush", (XmtProcedure)XmtMsgLinePush, {XtRWidget}},
{"XmtMsgLinePop", (XmtProcedure)XmtMsgLinePop, {XtRWidget, XtRInt}},

/* XmtCli widget functions */
{"XmtCliPuts", (XmtProcedure)XmtCliPuts, {XtRString, XtRWidget}},
{"XmtCliPrintf", (XmtProcedure)XmtCliPrintf,
 {XtRWidget, XtRString, XmtRCallbackUnused}},
{"XmtCliClear", (XmtProcedure)XmtCliClear, {XtRWidget}},

/* XmtChooser functions */
{"XmtChooserSetState", (XmtProcedure)XmtChooserSetState,
 {XtRWidget, XtRInt, XtRBoolean}},
{"XmtChooserSetSensitive", (XmtProcedure)XmtChooserSetSensitive,
 {XtRWidget, XtRInt, XtRBoolean}},

/* XmtInputField functions */
{"XmtInputFieldSetString", (XmtProcedure)XmtInputFieldSetString,
 {XtRWidget, XtRString}},

/* Layout functions */
{"XmtLayoutDisableLayout", (XmtProcedure)XmtLayoutDisableLayout, {XtRWidget}},
{"XmtLayoutEnableLayout", (XmtProcedure)XmtLayoutEnableLayout, {XtRWidget}},
};

#if NeedFunctionPrototypes
void XmtRegisterXmtProcedures(void)
#else
void XmtRegisterXmtProcedures()
#endif
{
    XmtRegisterProcedures(xmt_procedures, XtNumber(xmt_procedures));
}
