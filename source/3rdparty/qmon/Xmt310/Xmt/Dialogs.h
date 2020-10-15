/* 
 * Motif Tools Library, Version 3.1
 * $Id: Dialogs.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Dialogs.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtDialogs_h
#define _XmtDialogs_h    

#include <stdio.h>

typedef enum {
    XmtCancelButton,
    XmtOkButton,
    XmtHelpButton,
    XmtApplyButton,
    XmtResetButton,
    XmtDoneButton,
    XmtStopButton,
    XmtYesButton,	
    XmtNoButton
} XmtButtonType;

/*
 * These are the names of some of the dialogs we create
 */
#define XmtMESSAGE_DIALOG_NAME "xmtMessageDialog"
#define XmtMESSAGE_DIALOG_SHELL_NAME "xmtMessageDialogShell"
#define XmtWORKING_DIALOG_NAME "xmtWorkingDialog"
#define XmtWORKING_DIALOG_SHELL_NAME "xmtWorkingDialogShell"
#define XmtSTRING_DIALOG_NAME "xmtStringDialog"
#define XmtSTRING_DIALOG_SHELL_NAME "xmtStringDialogShell"
#define XmtBOOLEAN_DIALOG_NAME "xmtBooleanDialog"
#define XmtBOOLEAN_DIALOG_SHELL_NAME "xmtBooleanDialogShell"
#define XmtFILE_DIALOG_NAME "xmtFileDialog"
#define XmtFILE_DIALOG_SHELL_NAME "xmtFileDialogShell"
#define XmtITEM_DIALOG_NAME "xmtItemDialog"
#define XmtITEM_DIALOG_SHELL_NAME "xmtItemDialogShell"

/*
 * And here are some other strings used by the dialogs
 */
#define XmtMESSAGE_DIALOG_TITLE_DEFAULT "Message"
#define XmtERROR_DIALOG_TITLE_DEFAULT "Error"
#define XmtWARNING_DIALOG_TITLE_DEFAULT "Warning"
#define XmtINFO_DIALOG_TITLE_DEFAULT "Help"
#define XmtBOOLEAN_DIALOG_YES_DEFAULT "Yes"
#define XmtBOOLEAN_DIALOG_NO_DEFAULT "No"
#define XmtBOOLEAN_DIALOG_CANCEL_DEFAULT "Cancel"
#define XmtHELP_BUTTON_LABEL_DEFAULT "Help"
#define XmtFILE_DIALOG_PROMPT_DEFAULT "Selection"  /* The Motif default */
#define XmtSTRING_DIALOG_TITLE_DEFAULT "Enter a String"
#define XmtINT_DIALOG_TITLE_DEFAULT "Enter an Integer"
#define XmtDOUBLE_DIALOG_TITLE_DEFAULT "Enter a Number"
#define XmtBOOLEAN_DIALOG_TITLE_DEFAULT "Question"
#define XmtFILE_DIALOG_TITLE_DEFAULT "Select a File"
#define XmtITEM_DIALOG_TITLE_DEFAULT "Select an Item"
#define XmtWORKING_DIALOG_TITLE_DEFAULT "Working"
#define XmtWORKING_DIALOG_SCALE_LABEL_DEFAULT "% Complete:"
#define XmtWORKING_DIALOG_BUTTON_LABEL_DEFAULT "Stop"

/*
 * Class names under which we look up resources for instantiations
 * of cached dialogs.
 */
#define XmtCMessageDialog "XmtMessageDialog"
#define XmtCWarningDialog "XmtWarningDialog"
#define XmtCErrorDialog   "XmtErrorDialog"
#define XmtCInformationDialog "XmtInformationDialog"
#define XmtCWorkingDialog "XmtWorkingDialog"
#define XmtCStringDialog "XmtStringDialog"
#define XmtCBooleanDialog "XmtBooleanDialog"
#define XmtCIntDialog "XmtIntDialog"
#define XmtCDoubleDialog "XmtDoubleDialog"
#define XmtCFileDialog "XmtFileDialog"
#define XmtCItemDialog "XmtItemDialog"

/*
 * Names and classes for the resources we look up for
 * each instantiation of a cached dialog.  Different
 * dialog types look up different resources.
 */
#ifndef XmtNmessage
#define XmtNmessage "message"
#endif
#ifndef XmtCMessage
#define XmtCMessage "Message"
#endif
#ifndef XmtNtitle
#define XmtNtitle "title"
#endif
#ifndef XmtCTitle
#define XmtCTitle "Title"
#endif
#ifndef XmtNyesLabel
#define XmtNyesLabel "yesLabel"
#endif
#ifndef XmtCYesLabel
#define XmtCYesLabel "YesLabel"
#endif
#ifndef XmtNnoLabel
#define XmtNnoLabel "noLabel"
#endif
#ifndef XmtCNoLabel
#define XmtCNoLabel "NoLabel"
#endif
#ifndef XmtNcancelLabel
#define XmtNcancelLabel "cancelLabel"
#endif
#ifndef XmtCCancelLabel
#define XmtCCancelLabel "CancelLabel"
#endif
#ifndef XmtNhelpText
#define XmtNhelpText "helpText"
#endif
#ifndef XmtCHelpText
#define XmtCHelpText "HelpText"
#endif
#ifndef XmtNicon
#define XmtNicon "icon"
#endif
#ifndef XmtCIcon
#define XmtCIcon "Icon"
#endif
#ifndef XmtNdefaultButton
#define XmtNdefaultButton "defaultButton"
#endif
#ifndef XmtCDefaultButton
#define XmtCDefaultButton "DefaultButton"
#endif
#ifndef XmtNiconType
#define XmtNiconType "iconType"
#endif
#ifndef XmtCIconType
#define XmtCIconType "IconType"
#endif
#ifndef XmtNmin
#define XmtNmin "min"
#endif
#ifndef XmtCMin
#define XmtCMin "Min"
#endif
#ifndef XmtNmax
#define XmtNmax "max"
#endif
#ifndef XmtCMax
#define XmtCMax "Max"
#endif
#ifndef XmtNdirectory
#define XmtNdirectory "directory"
#endif
#ifndef XmtCDirectory
#define XmtCDirectory "Directory"
#endif
#ifndef XmtNpattern
#define XmtNpattern "pattern"
#endif
#ifndef XmtCPattern
#define XmtCPattern "Pattern"
#endif
#ifndef XmtNlistTitle
#define XmtNlistTitle "listTitle"
#endif
#ifndef XmtCListTitle
#define XmtCListTitle "ListTitle"
#endif
#ifndef XmtNitems
#define XmtNitems "items"
#endif
#ifndef XmtCItems
#define XmtCItems "Items"
#endif
#ifndef XmtNvisibleItems
#define XmtNvisibleItems "visibleItems"
#endif
#ifndef XmtCVisibleItems
#define XmtCVisibleItems "VisibleItems"
#endif

/* names of dialogs used by other Xmt dialog routines */
#define XmtBAD_INT_DIALOG     "xmtBadIntDialog"
#define XmtBAD_DOUBLE_DIALOG  "xmtBadDoubleDialog"
#define XmtTOO_SMALL_DIALOG   "xmtTooSmallDialog"
#define XmtTOO_BIG_DIALOG     "xmtTooBigDialog"
#define XmtCANT_OPEN_DIALOG   "xmtCantOpenDialog"
#define XmtNO_MATCH_DIALOG    "xmtNoMatchDialog"    

/* error messages displayed in dialogs by Xmt routines */
#define XmtBAD_INT_MESSAGE     "Please enter an integer."
#define XmtBAD_DOUBLE_MESSAGE  "Please enter a number."
#define XmtTOO_SMALL_MESSAGE   "Please enter a larger number."
#define XmtTOO_BIG_MESSAGE     "Please enter a smaller number."
#define XmtNO_MATCH_MESSAGE    "Please select an item that appears\n\
on the list."

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtDisplayWarning(Widget, StringConst, StringConst);
extern void XmtDisplayError(Widget, StringConst, StringConst);
extern void XmtDisplayInformation(Widget,StringConst,StringConst, StringConst);
extern void XmtDisplayMessageVaList(Widget, StringConst, StringConst,
				    StringConst, StringConst, StringConst,
				    Pixmap, int, int, va_list);
extern void XmtDisplayMessageAndWaitVaList(Widget, StringConst, StringConst,
				           StringConst, StringConst,
					   StringConst, Pixmap,
					   int, int, va_list);
extern Widget XmtDisplayWorkingDialog(Widget, StringConst,
			       StringConst, StringConst, StringConst, int, int,
			       XmtWideBoolean, XmtWideBoolean);
extern void XmtHideWorkingDialog(Widget);
extern Boolean XmtAskForString(Widget, StringConst, StringConst, String, int,
			       StringConst);
extern Boolean XmtAskForInteger(Widget, StringConst, StringConst,
				int *, int, int, StringConst);
extern Boolean XmtAskForDouble(Widget, StringConst, StringConst, double *,
			       double, double, StringConst);
extern Boolean XmtAskForBoolean(Widget, StringConst, StringConst,
				StringConst, StringConst, StringConst,
				XmtButtonType, int, XmtWideBoolean, Boolean *,
				StringConst);
extern Boolean XmtDisplayErrorAndAsk(Widget, StringConst,
				     StringConst, StringConst, StringConst,
				     XmtButtonType, StringConst);
extern Boolean XmtDisplayWarningAndAsk(Widget, StringConst,
				       StringConst, StringConst, StringConst,
				       XmtButtonType, StringConst);
extern Boolean XmtAskForFilename(Widget, StringConst,
				 StringConst, StringConst, StringConst,
				 String, int, String, int, String, int,
				 StringConst);
extern Boolean XmtAskForFile(Widget, StringConst,
			     StringConst, StringConst, StringConst,
			     FILE **, StringConst,
			     String, int, String, int, String, int,
			     StringConst);
extern Boolean XmtAskForItem(Widget, StringConst, StringConst, StringConst,
			     String *, int, XmtWideBoolean,
			     String, int, StringConst);
extern Boolean XmtAskForItemNumber(Widget, StringConst,
				   StringConst, StringConst,
				   String *, int, int *, StringConst);
extern void XmtDialogPosition(Widget, Widget);
#else
extern void XmtDisplayWarning();
extern void XmtDisplayError();
extern void XmtDisplayInformation();
extern void XmtDisplayMessageVaList();
extern void XmtDisplayMessageAndWaitVaList();
extern Widget XmtDisplayWorkingDialog();
extern void XmtHideWorkingDialog();
extern Boolean XmtAskForString();
extern Boolean XmtAskForInteger();
extern Boolean XmtAskForDouble();
extern Boolean XmtAskForBoolean();
extern Boolean XmtDisplayErrorAndAsk();
extern Boolean XmtDisplayWarningAndAsk();
extern Boolean XmtAskForFilename();
extern Boolean XmtAskForFile();
extern Boolean XmtAskForItem();
extern Boolean XmtAskForItemNumber();
extern void XmtDialogPosition();
#endif

#if NeedVarargsPrototypes
extern void XmtDisplayMessage(Widget, StringConst, StringConst,
			      StringConst, StringConst, StringConst, Pixmap,
			      int, int, ...)
                              gcc_printf_func(4,10);
extern void XmtDisplayWarningMsg(Widget, StringConst, StringConst,
				 StringConst, StringConst, ...)
                                 gcc_printf_func(3,6);
extern void XmtDisplayErrorMsg(Widget, StringConst, StringConst,
			       StringConst, StringConst, ...)
                               gcc_printf_func(3,6);
extern void XmtDisplayInformationMsg(Widget, StringConst, StringConst,
				     StringConst, StringConst, ...)
                                     gcc_printf_func(3,6);
extern void XmtDisplayMessageAndWait(Widget, StringConst, StringConst,
				     StringConst, StringConst, StringConst,
				     Pixmap, int, int, ...)
                                     gcc_printf_func(4,10);
extern void XmtDisplayWarningMsgAndWait(Widget, StringConst, StringConst,
					StringConst, StringConst, ...)
                                        gcc_printf_func(3,6);
extern void XmtDisplayErrorMsgAndWait(Widget, StringConst, StringConst,
				      StringConst, StringConst, ...)
                                      gcc_printf_func(3,6);
#else
extern void XmtDisplayMessage();
extern void XmtDisplayWarningMsg();
extern void XmtDisplayErrorMsg();
extern void XmtDisplayInformationMsg();
extern void XmtDisplayMessageAndWait();
extern void XmtDisplayWarningMsgAndWait();
extern void XmtDisplayErrorMsgAndWait();
#endif
_XFUNCPROTOEND

#endif  /* _XmtDialogs_h */
