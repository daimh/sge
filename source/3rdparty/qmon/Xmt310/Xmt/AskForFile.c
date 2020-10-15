/* 
 * Motif Tools Library, Version 3.1
 * $Id: AskForFile.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AskForFile.c,v $
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
#include <errno.h>
#ifdef VMS
#include <perror.h>
#endif
#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/List.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
static void OpenFileCallback(Widget w, XtPointer tag, XtPointer call_data)
#else
static void OpenFileCallback(w, tag, call_data)
Widget w;
XtPointer tag;
XtPointer call_data;
#endif
{
    XmtPerScreenInfo *info = (XmtPerScreenInfo *)tag;
    XmFileSelectionBoxCallbackStruct *data =
	(XmFileSelectionBoxCallbackStruct *) call_data;
    String filename;
    FILE *f;
    
    /* figure out the selected filename */
    XmStringGetLtoR(data->value, XmSTRING_DEFAULT_CHARSET, &filename);
    if (!filename || *filename == '\0') {
	XBell(XtDisplay(w), -50);
	return;
    }

    if (info->file_mode != NULL) {
	/* attempt to open the file */
	f = fopen(filename, info->file_mode);
	
	/* if we couldn't open the file, display error dialog */
	if (f == NULL)
	    XmtDisplayError(w, XmtCANT_OPEN_DIALOG, strerror(errno));
	else {
	    info->selected_file = f;
	    info->blocked = False;
	}
    }
    else 
	info->blocked = False;

    info->button = XmtOkButton;

    XtFree(filename);
}


#if NeedFunctionPrototypes
static void CreateFileDialog(XmtPerScreenInfo *info)
#else
static void CreateFileDialog(info)
XmtPerScreenInfo *info;
#endif
{
    Widget dshell;
    Widget dialog;
    Arg args[5];
    int i;

    /* create the dialog shell */
    i = 0;
    XtSetArg(args[i], XmNallowShellResize, True); i++;
    dshell = XmCreateDialogShell(info->topmost_shell,
				 XmtFILE_DIALOG_SHELL_NAME, args, i);

    /* create the dialog box itself */
    i = 0;
    XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
    XtSetArg(args[i], XmNautoUnmanage, False); i++;
    XtSetArg(args[i], XmNdefaultPosition, False); i++;
    dialog = XmCreateFileSelectionBox(dshell, XmtFILE_DIALOG_NAME, args, i);

    /* add callbacks on all the dialog buttons */
    XtAddCallback(dialog, XmNokCallback, OpenFileCallback, (XtPointer)info);
    XtAddCallback(dialog, XmNcancelCallback,
		  _XmtCancelCallback, (XtPointer)info);
    
    XtAddCallback(XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON),
		  XmNactivateCallback, _XmtHelpCallback, (XtPointer)info);
    
    /* add a callback for f.close */
    XmtAddDeleteCallback(dshell, XmDO_NOTHING,
			 _XmtCancelCallback, (XtPointer)info);

    /* cache the dialog in the PerScreenInfo structure */
    info->file_dialog = dialog;
}

typedef struct {
    String message;
    String title;
    String directory;
    String pattern;
    String help_text;
} FileDialogData;

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(FileDialogData, message),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(FileDialogData, title),
     XtRString, NULL},
{XmtNdirectory, XmtCDirectory, XtRString,
     sizeof(String), XtOffsetOf(FileDialogData, directory),
     XtRString, NULL},
{XmtNpattern, XmtCPattern, XtRString,
     sizeof(String), XtOffsetOf(FileDialogData, pattern),
     XtRString, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(FileDialogData, help_text),
     XtRString, NULL}
};


#if NeedFunctionPrototypes
static Boolean GetFile(Widget w, StringConst dialog_name,
                       StringConst prompt_default,
                       StringConst directory_default,
                       StringConst pattern_default,
                       FILE **value_return, StringConst file_mode,
                       String filename_buffer, int filename_buffer_len,
                       String directory_buffer, int directory_buffer_len,
                       String pattern_buffer, int pattern_buffer_len,
                       StringConst help_text_default, Boolean open_file)
#else
static Boolean GetFile(w, dialog_name, prompt_default, directory_default,
                       pattern_default, value_return, file_mode,
                       filename_buffer, filename_buffer_len,
                       directory_buffer, directory_buffer_len,
                       pattern_buffer, pattern_buffer_len,
                       help_text_default, open_file)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst directory_default;
StringConst pattern_default;
FILE **value_return;
StringConst file_mode;
String filename_buffer;
int filename_buffer_len;
String directory_buffer;
int directory_buffer_len;
String pattern_buffer;
int pattern_buffer_len;
StringConst help_text_default;
Boolean open_file;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    FileDialogData data;
    Widget help_w;
    Boolean lookup_directory, lookup_pattern;
    static String localized_prompt, localized_title;
    XmString message, title, file, directory, pattern;
    String file_string, dir_string, pattern_string;
    Widget text_widget;

    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize some strings the first time we're called */
    if (!localized_prompt) {
	localized_prompt = XmtLocalize2(w, XmtFILE_DIALOG_PROMPT_DEFAULT,
					"XmtAskForFile", "prompt");
	localized_title = XmtLocalize2(w, XmtFILE_DIALOG_TITLE_DEFAULT,
				       "XmtAskForFile", "title");
    }
    

    /*
     * If called by XmtAskForFile, we need the ok callback to open the
     * file, so store file_mode in the info structure.  Otherwise set
     * it to NULL to indicate not to open the file.
     */
    if ((file_mode == NULL) || (file_mode[0] == '\0')) file_mode = "r";
    if (open_file) info->file_mode = file_mode;
    else info->file_mode = NULL;

    /* if there's no cached dialog, create one. */
    if (info->file_dialog == NULL) CreateFileDialog(info);

    /* Set up default defaults if needed */
    if (prompt_default == NULL) prompt_default = localized_prompt;

    /* determine where the directory and pattern are coming from */
    lookup_directory =
	((directory_buffer == NULL) || (directory_buffer[0] == '\0'));
    lookup_pattern = ((pattern_buffer == NULL)||(pattern_buffer[0] == '\0'));
    
    /* if this dialog has a name, look up its resources */
    if (dialog_name != NULL) {
	resources[0].default_addr = (XtPointer) prompt_default;
	resources[1].default_addr = (XtPointer) localized_title;
	resources[2].default_addr = (XtPointer) directory_default;
	resources[3].default_addr = (XtPointer) pattern_default;
	resources[4].default_addr = (XtPointer) help_text_default;
	XtGetSubresources(shell, (XtPointer)&data,
			  (String)dialog_name, XmtCFileDialog,
			  resources, XtNumber(resources),
			  NULL, 0);
    }
    else { /* otherwise use arguments as defaults */
	data.message = (String) prompt_default;
	data.title = localized_title;
	data.directory = (String) directory_default;
	data.pattern = (String) pattern_default;
	data.help_text = (String) help_text_default;
    }

    /* if the directory and pattern are from buffers, use them to override */
    if (!lookup_directory) data.directory = directory_buffer;
    if (!lookup_pattern) data.pattern = pattern_buffer;
	
    /* create XmStrings */
    message = XmtCreateLocalizedXmString(w, data.message);
    title = XmtCreateLocalizedXmString(w, data.title);
    directory = XmtCreateLocalizedXmString(w, data.directory);
    pattern = XmtCreateLocalizedXmString(w, data.pattern);

    /*
     * Set XmNdirectory to NULL, so that the next call changes it
     * and forces the XmFileSelectionBox to re-selected the current dir.
     */
    XtVaSetValues(info->file_dialog, XmNdirectory, NULL, NULL);

    /* set resources on the dialog */
    XtVaSetValues(info->file_dialog,
		  XmNselectionLabelString, message,
		  XmNdialogTitle, title,
		  XmNdirectory, directory,
		  XmNpattern, pattern,
		  NULL);

    /*
     * Clear all selected items in the file list, in case one
     * is left over from the last invocation of this dialog
     */
    XmListDeselectAllItems(XmFileSelectionBoxGetChild(info->file_dialog,
						      XmDIALOG_LIST));

    /*
     * If there is a default file, set it in the text widget,
     * and also select it in the file list widget
     */
    if ((filename_buffer != NULL) && (filename_buffer[0] != '\0')) {
	XmString label = XmtCreateXmString(filename_buffer);
	XtVaSetValues(info->file_dialog, XmNdirSpec, label, NULL);
	XmListSelectItem(XmFileSelectionBoxGetChild(info->file_dialog,
						    XmDIALOG_LIST),
			 label, False);
	XmStringFree(label);
    }

    /*
     * Clear any selected items in the directory list, in case
     * something is left over from the last invocation of the
     * dialog.  This means nothing will be selected in that list
     * by default.  But this is the most reasonable thing.
     * Selecting the curent directory /foo/bar/. is pretty
     * meaningless, and having anything else selected would be
     * misleading.  So nothing selected makes the most sense.
     */
    XmListDeselectAllItems(XmFileSelectionBoxGetChild(info->file_dialog,
						      XmDIALOG_DIR_LIST));
    
    /*
     * if there is help text, make the button sensitive,
     * and put help text where the callback procedure can find it.
     */
    help_w=XmFileSelectionBoxGetChild(info->file_dialog,XmDIALOG_HELP_BUTTON);
    if ((data.help_text != NULL) && (data.help_text[0] != '\0')) {
	XtSetSensitive(help_w, True);
	info->help_text = data.help_text;
    }
    else {
	XtSetSensitive(help_w, False);
	info->help_text = NULL;
    }

    /* destroy the XmStrings */
    XmStringFree(message);
    XmStringFree(title);
    XmStringFree(directory);
    XmStringFree(pattern);

    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(info->file_dialog), XtNtransientFor, shell, NULL);

    /* position and pop up the dialog */
    XmtDialogPosition(info->file_dialog, shell);
    text_widget = XmFileSelectionBoxGetChild(info->file_dialog, XmDIALOG_TEXT);
    XmtSetInitialFocus(info->file_dialog, text_widget);
    XmTextSetInsertionPosition(text_widget,XmTextGetLastPosition(text_widget));
    XtManageChild(info->file_dialog);

    /*
     * Enter a recursive event loop.
     * The callback registered on the okay and cancel buttons when
     * this dialog was created will cause info->button to change
     * when one of those buttons is pressed.
     */
    info->blocked = True;
    XmtBlock(shell, &info->blocked);

    /* pop down the dialog */
    XtUnmanageChild(info->file_dialog);

    /* make sure what is underneath gets cleaned up
     * (the calling routine might act on the user's returned
     * input and not handle events for awhile.)
     */
    XSync(XtDisplay(info->file_dialog), 0);
    XmUpdateDisplay(info->file_dialog);

    /* if user clicked Cancel, return False */
    if (info->button == XmtCancelButton) return False;

    /* else, set the return values and return True */
    XtVaGetValues(info->file_dialog,
		  XmNdirSpec, &file,
		  XmNdirectory, &directory,
		  XmNpattern, &pattern, NULL);
    
    XmStringGetLtoR(file, XmSTRING_DEFAULT_CHARSET, &file_string);
    strncpy(filename_buffer, file_string, filename_buffer_len-1);
    filename_buffer[filename_buffer_len-1] = '\0';
    XtFree(file_string);

    if ((directory_buffer != NULL) && (directory_buffer_len != 0)) {
	XmStringGetLtoR(directory, XmSTRING_DEFAULT_CHARSET, &dir_string);
	strncpy(directory_buffer, dir_string, directory_buffer_len-1);
	directory_buffer[directory_buffer_len-1] = '\0';
	XtFree(dir_string);
    }

    if ((pattern_buffer != NULL) && (pattern_buffer_len != 0)) {
	XmStringGetLtoR(pattern, XmSTRING_DEFAULT_CHARSET, &pattern_string);
	strncpy(pattern_buffer, pattern_string, pattern_buffer_len-1);
	pattern_buffer[pattern_buffer_len-1] = '\0';
	XtFree(pattern_string);
    }

    XmStringFree(file);
    XmStringFree(directory);
    XmStringFree(pattern);

    if (open_file && (value_return != NULL))
	*value_return = info->selected_file;

    return True;
}



#if NeedFunctionPrototypes
Boolean XmtAskForFilename(Widget w, StringConst dialog_name,
			  StringConst prompt_default,
                          StringConst directory_default,
			  StringConst pattern_default,
                          String filename_buffer, int filename_buffer_len,
                          String directory_buffer, int directory_buffer_len,
                          String pattern_buffer, int pattern_buffer_len,
                          StringConst help_text_default)
#else
Boolean XmtAskForFilename(w, dialog_name, prompt_default, directory_default,
			  pattern_default,
			  filename_buffer, filename_buffer_len,
			  directory_buffer, directory_buffer_len,
			  pattern_buffer, pattern_buffer_len,
			  help_text_default)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst directory_default;
StringConst pattern_default;
String filename_buffer;
int filename_buffer_len;
String directory_buffer;
int directory_buffer_len;
String pattern_buffer;
int pattern_buffer_len;
StringConst help_text_default;
#endif
{
    return GetFile(w, dialog_name,
		   prompt_default, directory_default, pattern_default,
		   NULL, NULL,
		   filename_buffer, filename_buffer_len,
		   directory_buffer, directory_buffer_len,
		   pattern_buffer, pattern_buffer_len,
		   help_text_default, False);
}


#if NeedFunctionPrototypes
Boolean XmtAskForFile(Widget w, StringConst dialog_name,
		      StringConst prompt_default,
                      StringConst directory_default,
		      StringConst pattern_default,
                      FILE **value_return, StringConst file_mode,
                      String filename_buffer, int filename_buffer_len,
                      String directory_buffer, int directory_buffer_len,
                      String pattern_buffer, int pattern_buffer_len,
                      StringConst help_text_default)
#else
Boolean XmtAskForFile(w, dialog_name, prompt_default, directory_default,
		      pattern_default, value_return, file_mode,
		      filename_buffer, filename_buffer_len,
		      directory_buffer, directory_buffer_len,
		      pattern_buffer, pattern_buffer_len, help_text_default)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst directory_default;
StringConst pattern_default;
FILE **value_return;
StringConst file_mode;
String filename_buffer;
int filename_buffer_len;
String directory_buffer;
int directory_buffer_len;
String pattern_buffer;
int pattern_buffer_len;
StringConst help_text_default;
#endif
{
    return GetFile(w, dialog_name,
		   prompt_default, directory_default, pattern_default,
		   value_return, file_mode,
		   filename_buffer, filename_buffer_len,
		   directory_buffer, directory_buffer_len,
		   pattern_buffer, pattern_buffer_len,
		   help_text_default, True);
}
