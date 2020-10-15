/* 
 * Motif Tools Library, Version 3.1
 * $Id: AskForBool.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AskForBool.c,v $
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

#include <X11/IntrinsicP.h>
#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>


#if NeedFunctionPrototypes
static Pixmap GetPixmap(Widget w, int type)
#else
static Pixmap GetPixmap(w, type)
Widget w;
int type;
#endif
{
    String name, default_name;
    Pixmap p;
    Pixel fg, bg;

    /*
     * the algorithm (but not the code) of this procedure is from the
     * Motif source code.
     * XXX note that the pixmap names used here are not public, and
     * could change in future releases of Motif.
     * XXX also note that we never call XmDestroyPixmap on pixmaps
     * returned by this function.  The standard  dialog pixmaps that
     * we are using should never be flushed from the cache anyway, so
     * this is okay.
     */

    /* first make sure these names have been registered, by initializing
     * the XmMessageBox widget class
     */
    XtInitializeWidgetClass(xmMessageBoxWidgetClass);
    
    switch(type) {
    case XmDIALOG_ERROR:
	name = "xm_error";
	default_name = "default_xm_error";
	break;
    case XmDIALOG_WARNING:
	name = "xm_warning";
	default_name = "default_xm_warning";
	break;
    case XmDIALOG_INFORMATION:
	name = "xm_information";
	default_name = "default_xm_information";
	break;
    case XmDIALOG_QUESTION:
	name = "xm_question";
	default_name = "default_xm_question";
	break;
    case XmDIALOG_WORKING:
	name = "xm_working";
	default_name = "default_xm_working";
	break;
    default:
	return None;
    }

    XtVaGetValues(w, XmNforeground, &fg, XmNbackground, &bg, NULL);
    p = XmGetPixmapByDepth(XtScreenOfObject(w), name, fg, bg, w->core.depth);
    if (p == XmUNSPECIFIED_PIXMAP)
	p = XmGetPixmapByDepth(XtScreenOfObject(w), default_name, fg, bg,
			       w->core.depth);
    return p;
}

/*
 * This callback is a lot like the _XmtCancelCallback from Dialogs.c,
 * but it checks whether there is a cancel button at all.  This callback
 * is used to respond to the WM_DELETE signal.  XmtDisplay*AndAsk display
 * a dialog without a cancel button, and neither answer should be returned
 * on f.close, so we just ignore f.close in the case that there is no
 * cancel button.
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
static void DeleteCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void DeleteCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    
    if (info->boolean_internals.show_cancel_button)
	_XmtCancelCallback(w, tag, call_data);
}


static Boolean button_converter_registered = False;
static String button_names[] = {
    "Apply", "ApplyButton", "Cancel", "CancelButton",
    "Done", "DoneButton", "Help", "HelpButton",
    "No", "NoButton", "Ok", "OkButton", "Okay", "OkayButton",
    "Reset", "ResetButton", "Stop", "StopButton",
    "Yes", "YesButton",
};
static Cardinal button_values[] = {
    XmtApplyButton, XmtApplyButton, XmtCancelButton, XmtCancelButton,
    XmtDoneButton, XmtDoneButton, XmtHelpButton, XmtHelpButton,
    XmtNoButton, XmtNoButton, XmtOkButton, XmtOkButton,
    XmtOkButton, XmtOkButton, XmtResetButton, XmtResetButton,
    XmtStopButton, XmtStopButton, XmtYesButton, XmtYesButton,
};
static String button_prefixes[] = {"Xmt", "Button", NULL};

#if NeedFunctionPrototypes
static void CreateBooleanDialog(XmtPerScreenInfo *info)
#else
static void CreateBooleanDialog(info)
XmtPerScreenInfo *info;
#endif
{
    Widget dshell;
    Widget dialog;
    Widget msgbox, buttonbox;
    Widget icon, message;
    Widget yes, no, cancel, help;
    XmString help_label;
    Arg args[10];
    int i;

    /* register a converter for XmtButtonType */
    if (!button_converter_registered) {
	button_converter_registered = True;
	XmtRegisterEnumConverter(XmtRXmtButtonType,
				 button_names, button_values,
				 XtNumber(button_names), button_prefixes);
    }

    /* create the dialog shell */
    i = 0;
    XtSetArg(args[i], XmNallowShellResize, True); i++;
    dshell = XmCreateDialogShell(info->topmost_shell,
				 XmtBOOLEAN_DIALOG_SHELL_NAME, args, i);

    /* create the dialog box itself */
    i = 0;
    XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
    XtSetArg(args[i], XmNautoUnmanage, False); i++;
    XtSetArg(args[i], XmNdefaultPosition, False); i++;
    dialog = XmtCreateLayout(dshell, XmtBOOLEAN_DIALOG_NAME, args, i);

    /* create a row for the icon and message */
    i = 0;
    XtManageChild(msgbox = XmtCreateLayoutBox(dialog, "msgbox", args,i));
    
    /* create the icon label, with message as caption */
    i = 0;
    XtSetArg(args[i], XmtNlayoutIn, msgbox); i++;
    XtSetArg(args[i], XmtNlayoutJustification, XmtLayoutCentered); i++;
    XtSetArg(args[i], XmtNlayoutMarginWidth, 10); i++;
    XtSetArg(args[i], XmtNlayoutMarginHeight, 10); i++;
    XtManageChild(icon = XmtCreateLayoutPixmap(dialog, "icon", args, i));

    /* create the message gadget */
    i = 0;
    XtSetArg(args[i], XmtNlayoutIn, msgbox); i++;
    XtSetArg(args[i], XmtNlayoutJustification, XmtLayoutCentered); i++;
    XtSetArg(args[i], XmtNlayoutMarginHeight, 10); i++;
    XtManageChild(message = XmtCreateLayoutString(dialog, "message", args, i));

    /* create a button box */
    i = 0;
    XtSetArg(args[i], XmtNspaceType, XmtLayoutSpaceEven); i++;
    XtSetArg(args[i], XmtNlayoutStretchability, 0); i++;
    XtSetArg(args[i], XmtNlayoutFrameType, XmtLayoutFrameTop); i++;
    XtSetArg(args[i], XmtNlayoutFrameMargin, 8); i++;
    XtManageChild(buttonbox = XmtCreateLayoutBox(dialog, "buttonbox", args,i));
    
    /* create the button children */
    i = 0; 
    XtSetArg(args[i], XmtNlayoutIn, buttonbox); i++;
    XtManageChild(yes = XmCreatePushButton(dialog, "yes", args, i));
    XtManageChild(no = XmCreatePushButton(dialog, "no", args, i));
    XtManageChild(cancel=XmCreatePushButton(dialog, "cancel", args, i));
    help_label = XmtCreateXmString(XmtLocalize2(dialog,
						XmtHELP_BUTTON_LABEL_DEFAULT,
						"XmtAskForBoolean", "help"));
    XtSetArg(args[i], XmNlabelString, help_label); i++;
    XtManageChild(help = XmCreatePushButton(dialog, "help", args, i));
    XmStringFree(help_label);
    
    /* add callbacks on all the dialog buttons */
    XtAddCallback(yes, XmNactivateCallback,
		  _XmtYesCallback, (XtPointer)info);
    XtAddCallback(no, XmNactivateCallback,
		  _XmtNoCallback, (XtPointer)info);
    XtAddCallback(cancel, XmNactivateCallback,
		  _XmtCancelCallback, (XtPointer)info);
    XtAddCallback(help, XmNactivateCallback,
		  _XmtHelpCallback, (XtPointer)info);

    /* add a callback for f.close */
    XmtAddDeleteCallback(dshell, XmDO_NOTHING,
			 DeleteCallback, (XtPointer)info);

    info->boolean_internals.show_cancel_button = True;
    
    /* cache the dialog and its internals in the DialogInfo structure */
    info->boolean_dialog = dialog;
    info->boolean_internals.icon = icon;
    info->boolean_internals.message = message;
    info->boolean_internals.yes = yes;
    info->boolean_internals.no = no;
    info->boolean_internals.cancel = cancel;
    info->boolean_internals.help = help;
}

typedef struct {
    String message;
    String title;
    String yes_label;
    String no_label;
    String cancel_label;
    XmtButtonType default_button;
    unsigned char icon_type;
    String help_text;
} BooleanDialogData;

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, message),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, title),
     XtRString, NULL},
{XmtNyesLabel, XmtCYesLabel, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, yes_label),
     XtRString, NULL},
{XmtNnoLabel, XmtCNoLabel, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, no_label),
     XtRString, NULL},
{XmtNcancelLabel, XmtCCancelLabel, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, cancel_label),
     XtRString, NULL},
{XmtNdefaultButton, XmtCDefaultButton, XmtRXmtButtonType,
     sizeof(XmtButtonType),XtOffsetOf(BooleanDialogData, default_button),
     XtRImmediate, NULL},
{XmtNiconType, XmtCIconType, XmRDialogType,
     sizeof(unsigned char), XtOffsetOf(BooleanDialogData, icon_type),
     XtRImmediate, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(BooleanDialogData, help_text),
     XtRString, NULL}
};


#if NeedFunctionPrototypes
Boolean XmtAskForBoolean(Widget w, StringConst dialog_name,
			 StringConst prompt_default, StringConst yes_default,
			 StringConst no_default, StringConst cancel_default,
                         XmtButtonType default_button_default,
                         int icon_type_default,
                         XmtWideBoolean show_cancel_button,
                         Boolean *value_return, 
                         StringConst help_text_default)
#else
Boolean XmtAskForBoolean(w, dialog_name, prompt_default,
                         yes_default, no_default, cancel_default,
                         default_button_default, icon_type_default,
                         show_cancel_button, value_return, help_text_default)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst yes_default;
StringConst no_default;
StringConst cancel_default;
XmtButtonType default_button_default;
int icon_type_default;
XmtWideBoolean show_cancel_button;
Boolean *value_return;
StringConst help_text_default;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    BooleanDialogData data;
    Widget default_button_widget;
    static String localized_yes,localized_no,localized_cancel,localized_title;
    XmString title, yes_label, no_label, cancel_label;

    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* first time through localize some strings */
    if (!localized_yes) {
	localized_yes = XmtLocalize2(w, XmtBOOLEAN_DIALOG_YES_DEFAULT,
				   "XmtAskForBoolean", "yes");
	localized_no = XmtLocalize2(w, XmtBOOLEAN_DIALOG_NO_DEFAULT,
				  "XmtAskForBoolean", "no");
	localized_cancel = XmtLocalize2(w, XmtBOOLEAN_DIALOG_CANCEL_DEFAULT,
				      "XmtAskForBoolean", "cancel");
	localized_title = XmtLocalize2(w, XmtBOOLEAN_DIALOG_TITLE_DEFAULT,
				     "XmtAskForBoolean", "title");
    }
    
    /* if there's no cached dialog, create one. */
    if (info->boolean_dialog == NULL) CreateBooleanDialog(info);

    /* Set up default defaults if needed */
    if (yes_default == NULL) yes_default = localized_yes;
    if (no_default == NULL) no_default = localized_no;
    if (cancel_default == NULL)
	cancel_default = localized_cancel;
    if ((icon_type_default != XmDIALOG_QUESTION) &&
	(icon_type_default != XmDIALOG_ERROR) &&
	(icon_type_default != XmDIALOG_WARNING) &&
	(icon_type_default != XmDIALOG_INFORMATION) &&
	(icon_type_default != XmDIALOG_WORKING))
	icon_type_default = XmDIALOG_QUESTION;

    /* if this dialog has a name, look up its resources */
    if (dialog_name != NULL) {
	resources[0].default_addr = (XtPointer) prompt_default;
	resources[1].default_addr = (XtPointer) localized_title;
	resources[2].default_addr = (XtPointer) yes_default;
	resources[3].default_addr = (XtPointer) no_default;
	resources[4].default_addr = (XtPointer) cancel_default;
	resources[5].default_addr = (XtPointer) default_button_default;
	resources[6].default_addr = (XtPointer) icon_type_default;
	resources[7].default_addr = (XtPointer) help_text_default;
	XtGetSubresources(shell, (XtPointer) &data,
			  (String)dialog_name, XmtCBooleanDialog,
			  resources, XtNumber(resources),
			  NULL, 0);
    }
    else { /* otherwise use arguments as defaults */
	data.message = (String) prompt_default;
	data.title = localized_title;
	data.yes_label = (String) yes_default;
	data.no_label = (String) no_default;
	data.cancel_label = (String) cancel_default;
	data.default_button = default_button_default;
	data.icon_type = (unsigned char) icon_type_default;
	data.help_text = (String) help_text_default;
    }

    /* create the XmStrings */
    title = XmtCreateLocalizedXmString(w, data.title);
    yes_label = XmtCreateLocalizedXmString(w, data.yes_label);
    no_label = XmtCreateLocalizedXmString(w, data.no_label);
    cancel_label = XmtCreateLocalizedXmString(w, data.cancel_label);

    /* set icon pixmap and message  */
    XtVaSetValues(info->boolean_internals.icon,
		  XmtNpixmap, GetPixmap(info->boolean_dialog, data.icon_type),
		  NULL);
    XtVaSetValues(info->boolean_internals.message,
		  XmtNlabel, data.message,
		  NULL);

    /* set the dialog title */
    XtVaSetValues(info->boolean_dialog, XmNdialogTitle, title, NULL);
    
    /* set button labels */
    XtVaSetValues(info->boolean_internals.yes,
		  XmNlabelString, yes_label, NULL);
    XtVaSetValues(info->boolean_internals.no,
		  XmNlabelString, no_label, NULL);
    XtVaSetValues(info->boolean_internals.cancel,
		  XmNlabelString, cancel_label, NULL);

    /* Figure out the default button widget */
    switch(data.default_button) {
    case XmtYesButton:
	default_button_widget = info->boolean_internals.yes;
	break;
    case XmtNoButton:
	default_button_widget = info->boolean_internals.no;
	break;
    case XmtCancelButton:
    default:
	default_button_widget = info->boolean_internals.cancel;
	break;
    }
    
    /* set the default button */
    XtVaSetValues(default_button_widget, XmNshowAsDefault, True, NULL);
    XtVaSetValues(info->boolean_dialog, XmNdefaultButton,
		  default_button_widget, NULL);

    /* if there is help text, make the button sensitive,
     * and put help text where the callback procedure can find it.
     */
    if ((data.help_text != NULL) && (data.help_text[0] != '\0')) {
	XtSetSensitive(info->boolean_internals.help, True);
	info->help_text = data.help_text;
    }
    else {
	XtSetSensitive(info->boolean_internals.help, False);
	info->help_text = NULL;
    }

    /* Manage or unmanage the Cancel button, as requested */
    if (show_cancel_button != info->boolean_internals.show_cancel_button) {
	if (show_cancel_button)
	    XtManageChild(info->boolean_internals.cancel);
	else
	    XtUnmanageChild(info->boolean_internals.cancel);
	info->boolean_internals.show_cancel_button = show_cancel_button;
    }
    
    /* destroy the XmStrings */
    XmStringFree(title);
    XmStringFree(yes_label);
    XmStringFree(no_label);
    XmStringFree(cancel_label);
    
    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(info->boolean_dialog),
		  XtNtransientFor, shell, NULL);

    /* position and pop up the dialog */
    XmtDialogPosition(info->boolean_dialog, shell);
    XtManageChild(info->boolean_dialog);

    /*
     * Enter a recursive event loop.
     * The callback registered on the okay and cancel buttons when
     * this dialog was created will cause info->button to change
     * when one of those buttons is pressed.
     */
    info->blocked = True;
    XmtBlock(shell, &info->blocked);

    /* pop down the dialog */
    XtUnmanageChild(info->boolean_dialog);

    /* make sure what is underneath gets cleaned up
     * (the calling routine might act on the user's returned
     * input and not handle events for awhile.)
     */
    XSync(XtDisplay(info->boolean_dialog), 0);
    XmUpdateDisplay(info->boolean_dialog);

    /* if user clicked Cancel, return False */
    if (info->button == XmtCancelButton) return False;

    /* else, set the return value and return True */
    if (info->button == XmtYesButton)
	*value_return = True;
    else
	*value_return = False;

    return True;
}


#if NeedFunctionPrototypes
Boolean XmtDisplayErrorAndAsk(Widget w, StringConst dialog_name,
			      StringConst message_default,
                              StringConst yes_default, StringConst no_default,
                              XmtButtonType default_button_default,
			      StringConst help_text_default)
#else
Boolean XmtDisplayErrorAndAsk(w, dialog_name, message_default,
			      yes_default, no_default, default_button_default,
			      help_text_default)
Widget w;
StringConst dialog_name;
StringConst message_default;
StringConst yes_default;
StringConst no_default;
XmtButtonType default_button_default;
StringConst help_text_default;
#endif
{
    Boolean value = False;

    (void) XmtAskForBoolean(w, dialog_name, message_default,
			    yes_default, no_default,
			    NULL, default_button_default, XmDIALOG_ERROR,
			    False, &value, help_text_default);
    return value;
}

#if NeedFunctionPrototypes
Boolean XmtDisplayWarningAndAsk(Widget w, StringConst dialog_name,
				StringConst message_default,
				StringConst yes_default,StringConst no_default,
				XmtButtonType default_button_default,
				StringConst help_text_default)
#else
Boolean XmtDisplayWarningAndAsk(w, dialog_name, message_default,
				yes_default, no_default,
				default_button_default, help_text_default)
Widget w;
StringConst dialog_name;
StringConst message_default;
StringConst yes_default;
StringConst no_default;
XmtButtonType default_button_default;
StringConst help_text_default;
#endif
{
    Boolean value = False;

    (void) XmtAskForBoolean(w, dialog_name, message_default,
			    yes_default, no_default,
			    NULL, default_button_default, XmDIALOG_WARNING,
			    False, &value, help_text_default);
    return value;
}
