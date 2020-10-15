/* 
 * Motif Tools Library, Version 3.1
 * $Id: Working.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Working.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/WorkingBox.h>
#include <Xmt/DialogsP.h>
#include <Xmt/ScreenP.h>
#include <Xm/DialogS.h>
#include <X11/IntrinsicP.h>

typedef struct {
    String message;
    String scale_label;
    String button_label;
    String title;
} DialogData;

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(DialogData, message),
     XtRString, NULL},
{XmtNscaleLabel, XmtCScaleLabel, XtRString,
     sizeof(String), XtOffsetOf(DialogData, scale_label),
     XtRString, NULL},
{XmtNbuttonLabel, XmtCButtonLabel, XtRString,
     sizeof(String), XtOffsetOf(DialogData, button_label),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(DialogData, title),
     XtRString, NULL},
};


#if NeedFunctionPrototypes
Widget XmtDisplayWorkingDialog(Widget w, StringConst dialog_name,
                               StringConst message_default,
                               StringConst scale_label_default,
                               StringConst button_label_default,
                               int scale_min, int scale_max,
                               XmtWideBoolean show_scale,
                               XmtWideBoolean show_button)
#else
Widget XmtDisplayWorkingDialog(w, dialog_name, message_default,
                               scale_label_default, button_label_default,
                               scale_min, scale_max, show_scale, show_button)
Widget w;
StringConst dialog_name;
StringConst message_default;
StringConst scale_label_default;
StringConst button_label_default;
int scale_min;
int scale_max;
int show_scale;
int show_button;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    DialogData data;
    XmString title;
    static String default_title;
    static String default_scale_label;
    static String default_button_label;
    static String funcname = "XmtDisplayWorkingDialog";
    
    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize some strings the first time we're called. */
    if (!default_title) {
	default_title = XmtLocalize2(w, XmtWORKING_DIALOG_TITLE_DEFAULT,
				     funcname, "title");
	default_scale_label =
	    XmtLocalize2(w, XmtWORKING_DIALOG_SCALE_LABEL_DEFAULT,
			 funcname, "scaleLabel");
	default_button_label =
	    XmtLocalize2(w, XmtWORKING_DIALOG_BUTTON_LABEL_DEFAULT,
			 funcname, "stop");
    }

    if (info->working_dialog == NULL) {
	Arg al[5];
	Cardinal ac;
	Widget dshell, dialog;
	
	ac = 0;
	XtSetArg(al[ac], XmNallowShellResize, True); ac++;
	XtSetArg(al[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;
	dshell = XmCreateDialogShell(info->topmost_shell,
				     XmtWORKING_DIALOG_SHELL_NAME,
				     al, ac);
	ac = 0;
	XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);ac++;
	XtSetArg(al[ac], XmNdefaultPosition, False); ac++;
	dialog = XtCreateWidget(XmtWORKING_DIALOG_NAME,
				xmtWorkingBoxWidgetClass,
				dshell, al, ac);
	
	info->working_dialog = dialog;
    }

    if (message_default == NULL) message_default = "";
    if (scale_label_default == NULL)
	scale_label_default = default_scale_label;
    if (button_label_default == NULL)
	button_label_default = default_button_label;

    if (dialog_name) {
	resources[0].default_addr = (XtPointer) message_default;
	resources[1].default_addr = (XtPointer) scale_label_default;
	resources[2].default_addr = (XtPointer) button_label_default;
	resources[3].default_addr = (XtPointer) default_title;
	XtGetSubresources(shell, (XtPointer)&data,
			  dialog_name, XmtCWorkingDialog,
			  resources, XtNumber(resources), NULL, 0);
    }
    else {
	data.message = (String) message_default;
	data.scale_label = (String) scale_label_default;
	data.button_label = (String) button_label_default;
	data.title = default_title;
    }

    title = XmtCreateLocalizedXmString(w, data.title);
    XtVaSetValues(info->working_dialog,
		  XmtNmessage, data.message,
		  XmtNscaleLabel, data.scale_label,
		  XmtNbuttonLabel, data.button_label,
		  XmNdialogTitle, title,
		  XmtNscaleMin, scale_min,
		  XmtNscaleMax, scale_max,
		  XmtNscaleValue, scale_min,
		  XmtNshowScale, show_scale,
		  XmtNshowButton, show_button,
		  XmNautoUnmanage, True,
		  NULL);
    XmStringFree(title);

    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(info->working_dialog),XtNtransientFor, shell, NULL);
       
    XmtDialogPosition(info->working_dialog, shell);
    XtManageChild(info->working_dialog);
    XmtWaitUntilMapped(info->working_dialog);
    return info->working_dialog;
}

  

#if NeedFunctionPrototypes
void XmtHideWorkingDialog(Widget w)
#else
void XmtHideWorkingDialog(w)
Widget w;
#endif
{
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(w);

    if (!info->working_dialog) return;
    XtUnmanageChild(info->working_dialog);
}
