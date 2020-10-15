/* 
 * Motif Tools Library, Version 3.1
 * $Id: AskForString.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AskForString.c,v $
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

#include <stdio.h>
#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>


#if NeedFunctionPrototypes
static void CreateStringDialog(XmtPerScreenInfo *info)
#else
static void CreateStringDialog(info)
XmtPerScreenInfo *info;
#endif
{
    Widget dshell;
    Widget dialog;
    Widget cancel, help;
    Arg args[5];
    int i;

    /* create the dialog shell */
    i = 0;
    XtSetArg(args[i], XmNallowShellResize, True); i++;
    dshell = XmCreateDialogShell(info->topmost_shell,
				 XmtSTRING_DIALOG_SHELL_NAME,
				 args, i);

    /* create the dialog box itself */
    i = 0;
    XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
    XtSetArg(args[i], XmNdialogType, XmDIALOG_PROMPT); i++;
    XtSetArg(args[i], XmNautoUnmanage, False); i++;
    XtSetArg(args[i], XmNdefaultPosition, False); i++;
    dialog = XmCreateSelectionBox(dshell, XmtSTRING_DIALOG_NAME, args, i);

    /* add callbacks on all the dialog buttons */
    cancel = XmSelectionBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
    help = XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
    
    XtAddCallback(cancel, XmNactivateCallback,
		  _XmtCancelCallback, (XtPointer)info);
    XtAddCallback(help, XmNactivateCallback,
		  _XmtHelpCallback, (XtPointer)info);
    
    /* add a callback for f.close */
    XmtAddDeleteCallback(dshell, XmDO_NOTHING,
			 _XmtCancelCallback, (XtPointer)info);

    /* cache the dialog in the DialogInfo structure */
    info->string_dialog = dialog;
}

/* error messages localized in DoStringDialog, used in callback procs below */
static String bad_int_message, bad_double_message;
static String too_small_message, too_big_message;

typedef struct {
    XmtPerScreenInfo *info;
    int int_min, int_max, int_value;
    double double_min, double_max, double_value;
} VerifyCallbackStruct;

/*
 * This callback procedure gets registered on the Ok button and
 * checks that the user's input is a legal integer, and if the
 * integer bounds are not equal to one another, it also checks
 * that the input integer is between those bounds.  If any of
 * these tests fail, it display an error with XmtDisplayError.
 * If all pass, it stores the converted integer into the
 * VerifyCallbackStruct for later use by the calling routine,
 * and sets info->state so that the recursive loop terminates
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
static void IntVerifyCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void IntVerifyCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    VerifyCallbackStruct *data = (VerifyCallbackStruct *) tag;
    Widget text_w;
    String text_value;
    int int_value;
    Boolean check_bounds;

    text_w = XmSelectionBoxGetChild(data->info->string_dialog, XmDIALOG_TEXT);
    text_value = XmTextGetString(text_w);

    check_bounds = (data->int_min < data->int_max);

    if (sscanf(text_value, "%d", &int_value) != 1) /* bad input */
	XmtDisplayError(data->info->string_dialog, XmtBAD_INT_DIALOG,
			bad_int_message);
    else if (check_bounds &&(int_value < data->int_min))
	XmtDisplayError(data->info->string_dialog, XmtTOO_SMALL_DIALOG,
			too_small_message);
    else if (check_bounds  && (int_value > data->int_max))
	XmtDisplayError(data->info->string_dialog, XmtTOO_BIG_DIALOG,
			too_big_message);
    else {
	/* if input is good, store it and exit event loop */
	data->info->blocked = False;
 	data->info->button = XmtOkButton;
	data->int_value = int_value;
    }
    
    XtFree(text_value);
}

/*
 * This callback is like IntVerifyCallback above, but uses doubles
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
static void DoubleVerifyCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void DoubleVerifyCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    VerifyCallbackStruct *data = (VerifyCallbackStruct *) tag;
    Widget text_w;
    String text_value;
    double double_value;
    Boolean check_bounds;

    text_w = XmSelectionBoxGetChild(data->info->string_dialog, XmDIALOG_TEXT);
    text_value = XmTextGetString(text_w);

    check_bounds = (data->double_min < data->double_max);
    
    if (sscanf(text_value, "%lg", &double_value) != 1) /* bad input */
	XmtDisplayError(data->info->string_dialog, XmtBAD_DOUBLE_DIALOG,
			bad_double_message);
    else if (check_bounds &&(double_value < data->double_min))
	XmtDisplayError(data->info->string_dialog, XmtTOO_SMALL_DIALOG,
			too_small_message);
    else if (check_bounds && (double_value > data->double_max))
	XmtDisplayError(data->info->string_dialog, XmtTOO_BIG_DIALOG,
			too_big_message);
    else {
	/* if input is good, store it and exit event loop */
	data->info->blocked = False;
	data->info->button = XmtOkButton;
	data->double_value = double_value;
    }
    
    XtFree(text_value);
}

typedef struct {
    String message;
    String title;
    String help_text;
} StringDialogData;

static XtResource string_resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(StringDialogData, message),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(StringDialogData, title),
     XtRString, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(StringDialogData, help_text),
     XtRString, NULL}
};

/* possible values for the type argument of DoStringDialog */
#define XmtSTRING 1
#define XmtINT 2
#define XmtDOUBLE 3

#if NeedFunctionPrototypes
static Boolean DoStringDialog(Widget w, StringConst dialog_name,
                              StringConst default_prompt,
                              String buffer, int buffer_length,
                              int *int_return, double *double_return,
                              int int_min, int int_max,
                              double double_min, double double_max,
                              StringConst default_help, int type)
#else
static Boolean DoStringDialog(w, dialog_name, default_prompt,
                              buffer, buffer_length, int_return, double_return,
                              int_min, int_max, double_min, double_max,
                              default_help, type)
Widget w;
StringConst dialog_name;
StringConst default_prompt;
String buffer;
int buffer_length;
int *int_return;
double *double_return;
int int_min;
int int_max;
double double_min;
double double_max;
StringConst default_help;
int type;
#endif
{
    Widget shell = XmtGetShell(w);
    Widget text_w, okay_w, help_w;
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    StringDialogData data;
    String dialog_class;
    String default_title;
    String default_text;
    char local_buffer[50];
    XtCallbackProc ok_callback;
    XtPointer ok_client_data;
    VerifyCallbackStruct client_data_struct;
    String value;
    XmString message, title;
    Arg args[10];
    int i;
    static String default_string_title;
    static String default_int_title;
    static String default_double_title;
    
    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize strings, first time through */
    if (!default_string_title) {
	default_string_title = XmtLocalize2(w, XmtSTRING_DIALOG_TITLE_DEFAULT,
					    "XmtAskForString", "title");
	default_int_title = XmtLocalize2(w, XmtINT_DIALOG_TITLE_DEFAULT,
					 "XmtAskForInteger", "title");
	default_double_title = XmtLocalize2(w, XmtDOUBLE_DIALOG_TITLE_DEFAULT,
					    "XmtAskForDouble", "title");
	bad_int_message = XmtLocalize2(w, XmtBAD_INT_MESSAGE,
				       "XmtAskForInteger", "badInput");
	bad_double_message = XmtLocalize2(w, XmtBAD_DOUBLE_MESSAGE,
					  "XmtAskForDouble", "badInput");
	too_small_message = XmtLocalize2(w, XmtTOO_SMALL_MESSAGE,
					 "XmtAskForInteger", "tooSmall");
	too_big_message = XmtLocalize2(w, XmtTOO_BIG_MESSAGE,
				       "XmtAskForInteger", "tooBig");
    }

    /* if there's no cached dialog, create one. */
    if (info->string_dialog == NULL) CreateStringDialog(info);

    /* get handles to the text widget and help button within the dialog */
    text_w = XmSelectionBoxGetChild(info->string_dialog, XmDIALOG_TEXT);
    okay_w = XmSelectionBoxGetChild(info->string_dialog, XmDIALOG_OK_BUTTON);
    help_w = XmSelectionBoxGetChild(info->string_dialog, XmDIALOG_HELP_BUTTON);

    if (default_prompt == NULL) default_prompt = "";
    if (default_help == NULL) default_help= "";

    /* resource lists differ by type of the dialog */
    switch(type) {
    case XmtSTRING:
    default:
	default_title = default_string_title;
	dialog_class = XmtCStringDialog;
	break;
    case XmtINT:
	default_title = default_int_title;
	dialog_class = XmtCIntDialog;
	break;
    case XmtDOUBLE:
	default_title = default_double_title;
	dialog_class = XmtCDoubleDialog;
	break;
    }

    /* if this dialog has a name, look up its resources */
    if (dialog_name != NULL) {
	string_resources[0].default_addr = (XtPointer) default_prompt;
	string_resources[1].default_addr = (XtPointer) default_title;
	string_resources[2].default_addr = (XtPointer) default_help;
	XtGetSubresources(shell, (XtPointer)&data,
			  (String)dialog_name, dialog_class,
			  string_resources, XtNumber(string_resources),
			  NULL, 0);
    }
    else { /* otherwise use arguments as defaults */
	data.message = (String) default_prompt;
	data.title = default_title;
	data.help_text = (String) default_help;
    }

    /* figure out default text for the text widget */
    switch(type) {
    case XmtSTRING:
    default:
	default_text = buffer;
	break;
    case XmtINT:
	sprintf(local_buffer, "%d", *int_return);
	default_text = local_buffer;
	buffer_length = 50;
	break;
    case XmtDOUBLE:
	sprintf(local_buffer, "%g", *double_return);
	default_text = local_buffer;
	buffer_length = 50;
	break;
    }
    
    /* set resources on the dialog box and the text widget within it. */
    message = XmtCreateLocalizedXmString(w, data.message);
    title = XmtCreateLocalizedXmString(w, data.title);
    i = 0;
    XtSetArg(args[i], XmNselectionLabelString, message); i++;
    XtSetArg(args[i], XmNdialogTitle, title); i++;
    XtSetValues(info->string_dialog, args, i);
    XmStringFree(message);
    XmStringFree(title);

    i = 0;
    XtSetArg(args[i], XmNvalue, default_text); i++;
    XtSetArg(args[i], XmNmaxLength, buffer_length -1); i++;
    XtSetValues(text_w, args, i);

    /* if there is help text, make the button sensitive,
     * and put help text where the callback procedure can find it.
     */
    if ((data.help_text != NULL) && (data.help_text[0] != '\0')) {
	XtSetSensitive(help_w, True);
	info->help_text = data.help_text;
    }
    else {
	XtSetSensitive(help_w, False);
	info->help_text = NULL;
    }

    /* set the appropriate callback on the Ok button */
    switch(type) {
    case XmtSTRING:
    default:
	ok_callback = _XmtOkCallback;
	ok_client_data = (XtPointer) info;
	break;
    case XmtINT:
	client_data_struct.info = info;
	client_data_struct.int_min = int_min;
	client_data_struct.int_max = int_max;
	ok_callback = IntVerifyCallback;
	ok_client_data = (XtPointer) &client_data_struct;
	break;
    case XmtDOUBLE:
	client_data_struct.info = info;
	client_data_struct.double_min = double_min;
	client_data_struct.double_max = double_max;
	ok_callback = DoubleVerifyCallback;
	ok_client_data = (XtPointer) &client_data_struct;
	break;
    }
    XtAddCallback(okay_w, XmNactivateCallback, ok_callback, ok_client_data);

    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(info->string_dialog), XtNtransientFor, shell, NULL);

    /* position the dialog, set initial focus, and pop it up */
    XmtDialogPosition(info->string_dialog, shell);
    XmtSetInitialFocus(info->string_dialog, text_w);
    XmTextSetInsertionPosition(text_w, XmTextGetLastPosition(text_w));
    XtManageChild(info->string_dialog);

    /*
     * Enter a recursive event loop.
     * The callback registered on the okay and cancel buttons when
     * this dialog was created will cause info->button to change
     * when one of those buttons is pressed.
     */
    info->blocked = True;
    XmtBlock(info->string_dialog, &info->blocked);
 
    /* pop down the dialog */
    XtUnmanageChild(info->string_dialog);

    /* make sure what is underneath gets cleaned up
     * (the calling routine might act on the user's returned
     * input and not handle events for awhile.)
     */
    XSync(XtDisplay(info->string_dialog), 0);
    XmUpdateDisplay(info->string_dialog);

    /* unregister the okay callback we set on the dialog */
    XtRemoveCallback(okay_w, XmNactivateCallback, ok_callback, ok_client_data);
    
    /* if user clicked Cancel, return False */
    if (info->button == XmtCancelButton)
	return False;
    
    /*
     * otherwise, put the user's input into the buffer or
     * set the int or double return value, as appropriate
     * and return True
     */
    if (type == XmtSTRING) {
	value = XmTextGetString(text_w);
	strncpy(buffer, value, buffer_length);
	buffer[buffer_length-1] = '\0';  /* just in case */
	XtFree(value);
    }
    else if (type == XmtINT)
	*int_return = client_data_struct.int_value;
    else if (type == XmtDOUBLE)
	*double_return = client_data_struct.double_value;

    return True;
}

#if NeedFunctionPrototypes
Boolean XmtAskForString(Widget w, StringConst dialog_name, 
                        StringConst default_prompt,
			String buffer,int buffer_length,
                        StringConst default_help)
#else
Boolean XmtAskForString(w, dialog_name, default_prompt,
			buffer, buffer_length, default_help)
Widget w;
StringConst dialog_name;
StringConst default_prompt;
String buffer;
int buffer_length;
StringConst default_help;
#endif
{
    return DoStringDialog(w, dialog_name, default_prompt,
			  buffer, buffer_length, NULL, NULL,
			  0,0, 0.0, 0.0, default_help, XmtSTRING);
}

#if NeedFunctionPrototypes
Boolean XmtAskForInteger(Widget w, StringConst dialog_name,
			 StringConst default_prompt,
                         int *value, int min, int max,
                         StringConst default_help)
#else
Boolean XmtAskForInteger(w, dialog_name, default_prompt, value,
			 min, max, default_help)
Widget w;
StringConst dialog_name;
StringConst default_prompt;
int *value;
int min;
int max;
StringConst default_help;
#endif
{
    return DoStringDialog(w, dialog_name, default_prompt, NULL, 0,
			  value, NULL, min, max, 0.0, 0.0,
			  default_help, XmtINT);
}

#if NeedFunctionPrototypes
Boolean XmtAskForDouble(Widget w, StringConst dialog_name,
			StringConst default_prompt,
                        double *value, double min, double max,
                        StringConst default_help)
#else
Boolean XmtAskForDouble(w, dialog_name, default_prompt, value,
			min, max, default_help)
Widget w;
StringConst dialog_name;
StringConst default_prompt;
double *value;
double min;
double max;
StringConst default_help;
#endif
{
    return DoStringDialog(w, dialog_name, default_prompt, NULL, 0,
			  NULL, value, 0,0, min, max,
			  default_help, XmtDOUBLE);
}
