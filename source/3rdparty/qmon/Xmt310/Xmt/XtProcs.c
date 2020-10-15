/* 
 * Motif Tools Library, Version 3.1
 * $Id: XtProcs.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XtProcs.c,v $
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


static XmtProcedureInfo xt_procedures[] = {
{"XtManageChild", (XmtProcedure) XtManageChild, {XtRWidget}},
{"XtUnmanageChild", (XmtProcedure) XtUnmanageChild, {XtRWidget}},
{"XtSetSensitive", (XmtProcedure) XtSetSensitive, {XtRWidget, XtRBoolean}},
{"XtDestroyWidget", (XmtProcedure) XtDestroyWidget, {XtRWidget}},
{"XtAugmentTranslations", (XmtProcedure)XtAugmentTranslations,
    {XtRWidget, XtRTranslationTable}},
{"XtOverrideTranslations", (XmtProcedure)XtOverrideTranslations,
    {XtRWidget, XtRTranslationTable}},
{"XtPopupExclusive", (XmtProcedure)XtCallbackExclusive,
    {XmtRCallbackWidget, XtRWidget}},
{"XtPopupNonexclusive", (XmtProcedure)XtCallbackNonexclusive,
    {XmtRCallbackWidget, XtRWidget}},
{"XtPopupNone", (XmtProcedure)XtCallbackNone,
    {XmtRCallbackWidget, XtRWidget}},
{"XtPopupSpringLoaded", (XmtProcedure)XtPopupSpringLoaded, {XtRWidget}},
{"XtPopdown", (XmtProcedure)XtPopdown, {XtRWidget}},
{"XtAddCallbacks", (XmtProcedure)XtAddCallbacks,
     {XtRWidget, XtRString, XtRCallback}},
/* can't have XtRemoveCallback() because we'll never get matching client_data*/
{"XtError", (XmtProcedure)XtError, {XtRString}},
{"XtWarning", (XmtProcedure)XtWarning, {XtRString}},
{"XtInstallAccelerators", (XmtProcedure)XtInstallAccelerators,
     {XtRWidget, XtRWidget}},
{"XtInstallAllAccelerators", (XmtProcedure)XtInstallAllAccelerators,
     {XtRWidget, XtRWidget}},
{"XtSetMappedWhenManaged", (XmtProcedure)XtSetMappedWhenManaged,
     {XtRWidget, XtRBoolean}},
{"XtSetKeyboardFocus", (XmtProcedure)XtSetKeyboardFocus,
     {XtRWidget, XtRWidget}},
{"XtRealizeWidget", (XmtProcedure)XtRealizeWidget, {XtRWidget}},
};


#if NeedFunctionPrototypes
void XmtRegisterXtProcedures(void)
#else
void XmtRegisterXtProcedures()
#endif
{
    XmtRegisterProcedures(xt_procedures, XtNumber(xt_procedures));
}
