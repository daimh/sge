/* 
 * Motif Tools Library, Version 3.1
 * $Id: MotifWidgets.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MotifWidgets.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WidgetType.h>

#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>

/*
 * ARH	These include files cover widgets that were added in Motif-2.0.
 */
#if XmVersion >= 2000
#include <Xm/Container.h>
#include <Xm/IconG.h>
#include <Xm/Notebook.h>
#include <Xm/SpinB.h>
#include <Xm/ComboBox.h>
#endif

static XmtWidgetType motif_widgets[] = {
{"XmArrowButton", NULL, XmCreateArrowButton},
{"XmBulletinBoard", NULL, XmCreateBulletinBoard},
{"XmBulletinBoardDialog", NULL, XmCreateBulletinBoardDialog, NULL, NULL, True},
{"XmCascadeButton", NULL, XmCreateCascadeButton},
{"XmCommand", NULL, XmCreateCommand},

#if XmVersion >= 2000
{"XmContainer", NULL, XmCreateContainer, NULL, NULL, True},
#endif

{"XmDialogShell", NULL, XmCreateDialogShell, NULL, NULL, True},
{"XmDrawingArea", NULL, XmCreateDrawingArea},
{"XmDrawnButton", NULL, XmCreateDrawnButton},
{"XmErrorDialog", NULL, XmCreateErrorDialog, NULL, NULL, True},
{"XmFileSelectionBox", NULL, XmCreateFileSelectionBox},
{"XmFileSelectionDialog", NULL, XmCreateFileSelectionDialog, NULL, NULL, True},
{"XmForm", NULL, XmCreateForm},
{"XmFormDialog", NULL, XmCreateFormDialog, NULL, NULL, True},
{"XmFrame", NULL, XmCreateFrame},

#if XmVersion >= 2000
{"XmIconGadget", NULL, XmCreateIconGadget, NULL, NULL, True},
#endif

{"XmInformationDialog", NULL, XmCreateInformationDialog, NULL, NULL, True},
{"XmLabel", NULL, XmCreateLabel},
/* Motif 1.2.0 has the prototypes wrong for these next two and ScrolledList */ 
{"XmList", NULL, (XmtWidgetConstructor) XmCreateList}, 
{"XmMainWindow", NULL, (XmtWidgetConstructor) XmCreateMainWindow},
{"XmMenuBar", NULL, XmCreateMenuBar},
{"XmMenuShell", NULL, XmCreateMenuShell, NULL, NULL, True},
{"XmMessageBox", NULL, XmCreateMessageBox},
{"XmMessageDialog", NULL, XmCreateMessageDialog, NULL, NULL, True},

#if XmVersion >= 2000
{"XmNotebook", NULL, XmCreateNotebook, NULL, NULL, True},
#endif

{"XmOptionMenu", NULL, XmCreateOptionMenu},
{"XmPanedWindow", NULL, XmCreatePanedWindow},
{"XmPopupMenu", NULL, XmCreatePopupMenu, NULL, NULL, True},
{"XmPromptDialog", NULL, XmCreatePromptDialog, NULL, NULL, True},
{"XmPulldownMenu", NULL, XmCreatePulldownMenu, NULL, NULL, True},
{"XmPushButton", NULL, XmCreatePushButton},
{"XmQuestionDialog", NULL, XmCreateQuestionDialog, NULL, NULL, True},
{"XmRadioBox", NULL, XmCreateRadioBox},
{"XmRowColumn", NULL, XmCreateRowColumn},
{"XmScrollBar", NULL, XmCreateScrollBar},
{"XmScrolledList", NULL, (XmtWidgetConstructor) XmCreateScrolledList},
{"XmScrolledWindow", NULL, XmCreateScrolledWindow},
{"XmSelectionBox", NULL, XmCreateSelectionBox},
{"XmSelectionDialog", NULL, XmCreateSelectionDialog, NULL, NULL, True},
{"XmSeparator", NULL, XmCreateSeparator},
{"XmSimpleCheckBox", NULL, XmCreateSimpleCheckBox},
{"XmSimpleMenuBar", NULL, XmCreateSimpleMenuBar},
{"XmSimpleOptionMenu", NULL, XmCreateSimpleOptionMenu},
{"XmSimplePopupMenu", NULL, XmCreateSimplePopupMenu, NULL, NULL, True},
{"XmSimplePulldownMenu", NULL, XmCreateSimplePulldownMenu, NULL, NULL, True},
{"XmSimpleRadioBox", NULL, XmCreateSimpleRadioBox},

#if XmVersion >= 2000
{"XmSpinBox", NULL, XmCreateSpinBox, NULL, NULL, True},
#endif

{"XmWarningDialog", NULL, XmCreateWarningDialog, NULL, NULL, True},
{"XmWorkArea", NULL, XmCreateWorkArea},
{"XmWorkingDialog", NULL, XmCreateWorkingDialog, NULL, NULL, True},
};


#if NeedFunctionPrototypes
void XmtRegisterMotifWidgets(void)
#else
void XmtRegisterMotifWidgets()
#endif
{
    XmtRegisterWidgetTypes(motif_widgets, XtNumber(motif_widgets));
    XmtRegisterXmText();
    XmtRegisterXmScrolledText();
    XmtRegisterXmTextField();
    XmtRegisterXmToggleButton();
    XmtRegisterXmScale();
#if XmVersion >= 2000
    XmtRegisterXmComboBox();
#endif
#if XmVersion == 2000
    XmtRegisterXmCSText();
    XmtRegisterXmScrolledCSText();
#endif
}
