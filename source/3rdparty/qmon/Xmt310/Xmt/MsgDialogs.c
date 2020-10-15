/* 
 * Motif Tools Library, Version 3.1
 * $Id: MsgDialogs.c,v 1.2 2007-05-10 11:01:12 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: MsgDialogs.c,v $
 * Revision 1.2  2007-05-10 11:01:12  andre
 * AA-2007-05-10-0: Bugfix:    qmon crashes when displaying about dialog
 *                  Bugster:   6555744
 *                  Issue:     2243
 *                  Review:    RD
 *
 * Revision 1.1.1.1  2001/07/18 11:06:02  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/DialogsP.h>
#include <Xmt/ScreenP.h>
#include <Xmt/Hash.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <X11/IntrinsicP.h>


/*
 * Message dialogs are cached per-shell.  This is unlike the Askfor
 * dialogs which are more expensive, are always full_modal, and are cached
 * per screen.  Given a shell, we look up its cache record in the
 * shell_to_cache_table hash table, and get a cached dialog from an array
 * of available dialogs.  (There may be more than one because info dialogs
 * are modeless.  We also maintain an array of help text strings for each
 * dialog that is popped up.  This string is freed when the dialog is popped
 * down.  The popped_up variable is used for the synchronous (AndWait) versions
 * of the message dialogs.  There only needs to be one variable, because
 * synchronous dialogs are always modal, locking out all other input to the
 * shell.
 */

static XmtHashTable shell_to_cache_table;

typedef struct {
    Widget *dialogs;
    String *help_strings;
    short num, max, in_use;
    Boolean popped_up;
} MsgDialogCache;


#if NeedFunctionPrototypes
static void Popdown(Widget dialog, MsgDialogCache *cache)
#else
static void Popdown(dialog, cache)
Widget dialog;
MsgDialogCache *cache;
#endif
{
    int i;

    /* first, actually pop down the dialog */
    XtUnmanageChild(dialog);

    /* figure out what item in the cache we are. */
    for(i=0; i < cache->num; i++) if (cache->dialogs[i] == dialog) break;

    /* free any help text we have */
    if (cache->help_strings[i]) {
	XtFree(cache->help_strings[i]);
	cache->help_strings[i] = NULL;
    }

    /*
     * if there is already a free dialog, destroy this one
     * and remove it from the cache.  Otherwise, just note that
     * we now have one free.
     */
    if (cache->in_use < cache->num) {
	XtDestroyWidget(dialog);
	/* now compress the remaining array elements */
	for(; i < cache->num-1; i++) cache->dialogs[i] = cache->dialogs[i+1];
	cache->num--;
    }
    
    cache->in_use--;
    cache->popped_up = False;  /* stops blocking in ...AndWait functions */
}

#if NeedFunctionPrototypes
static void OkCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void OkCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    MsgDialogCache *cache = (MsgDialogCache *)tag;
    Widget dialog = XtParent(w);

    Popdown(dialog, cache);
}

#if NeedFunctionPrototypes
static void HelpCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void HelpCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    MsgDialogCache *cache = (MsgDialogCache *)tag;
    Widget dialog = XtParent(w);
    XmString msg, help, blank, s1, s2;
    int i;

    /*
     * find the dialog in the cache; this locates the help string.
     */
    for(i=0; i < cache->num-1; i++)
	if (cache->dialogs[i] == dialog) break;

    XtVaGetValues(dialog, XmNmessageString, &msg, NULL);
    blank = XmStringSeparatorCreate();
    help = XmtCreateLocalizedXmString(w, cache->help_strings[i]);
    s1 = XmStringConcat(msg, blank);
    s2 = XmStringConcat(s1, help);
    XtVaSetValues(dialog, XmNmessageString, s2, NULL);
    XmStringFree(msg);
    XmStringFree(blank);
    XmStringFree(help);
    XmStringFree(s1);
    XmStringFree(s2);
    XtSetSensitive(w, False);
}

#if NeedFunctionPrototypes
static void CloseCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void CloseCallback(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    MsgDialogCache *cache = (MsgDialogCache *)tag;
    Widget dialog = ((CompositeWidget)w)->composite.children[0];

    Popdown(dialog, cache);
}


#if NeedFunctionPrototypes
static Widget CreateMessageDialog(Widget shell, MsgDialogCache *cache)
#else
static Widget CreateMessageDialog(shell, cache)
Widget shell;
MsgDialogCache *cache;
#endif
{
    Widget dshell;
    Widget dialog;
    Widget ok, cancel, help;
    Arg args[5];
    int i;

    /*
     * create the dialog shell
     */
    i = 0; 
    XtSetArg(args[i], XmNallowShellResize, True); i++;
    XtSetArg(args[i], XmNdeleteResponse, XmDO_NOTHING); i++;
    dshell = XmCreateDialogShell(shell, XmtMESSAGE_DIALOG_SHELL_NAME,
				 args, i);

    /*
     * Create the message box.
     * Note that each shell and each message box we create has the
     * same name.  This is legal, but means that you cannot specify
     * different resources for different message dialogs from a
     * resource file.  This is not a problem because these widgets are
     * cached and the programmer cannot know in advance which on will
     * be used for what.
     */
    i = 0;
    XtSetArg(args[i], XmNautoUnmanage, False); i++;
    XtSetArg(args[i], XmNdefaultPosition, False); i++;
    dialog = XmCreateMessageBox(dshell, XmtMESSAGE_DIALOG_NAME, args, i);

    /*
     * Register callbacks on the Ok and Help buttons.
     * Get rid of the Cancel button; it is unused.
     */
    ok = XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON);
    cancel = XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON);
    help = XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
    XtAddCallback(ok, XmNactivateCallback, OkCallback, cache);
    XtAddCallback(help, XmNactivateCallback, HelpCallback, cache);
    XtUnmanageChild(cancel);

    /*
     * register a callback to handle f.close on the dialog
     */
    XmtAddDeleteCallback(dshell, XmDO_NOTHING, CloseCallback,(XtPointer)cache);

    return dialog;
}

#if NeedFunctionPrototypes
static void DeleteCacheRecord(Widget w, XtPointer tag, XtPointer data)
#else
static void DeleteCacheRecord(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    MsgDialogCache *cache = (MsgDialogCache *)tag;
    int i;

    XtFree((char *)cache->dialogs);  /* the dialogs will be auto. destroyed */
    for(i=0; i < cache->num; i++) XtFree(cache->help_strings[i]);
    XtFree((char *)cache->help_strings);
    XtFree((char *)cache);
    XmtHashTableDelete(shell_to_cache_table, (XtPointer)w);
}

#if NeedFunctionPrototypes
static Widget GetMessageDialog(Widget shell, StringConst help_text,
			       MsgDialogCache **cache_return)
#else
static Widget GetMessageDialog(shell, help_text, cache_return)
Widget shell;
String help_text;
MsgDialogCache **cache_return;
#endif
{
    MsgDialogCache *cache;
    Widget dialog = 0, help_button;
    String help;
    
    /* the first time, create the hash table */
    if (!shell_to_cache_table)
	shell_to_cache_table = XmtHashTableCreate(3);

    /* go get or create the cache record for this shell */
    if (!XmtHashTableLookup(shell_to_cache_table, (XtPointer)shell,
			    (XtPointer *)&cache)) {
	cache = (MsgDialogCache *) XtCalloc(1, sizeof(MsgDialogCache));
	XmtHashTableStore(shell_to_cache_table, (XtPointer)shell,
			  (XtPointer)cache);
	/* set a callback on the shell to remove the cache record */
	XtAddCallback(shell, XtNdestroyCallback, DeleteCacheRecord, cache);
    }

    /* grow the arrays in the cache, if needed */
    if (cache->in_use == cache->max) {
	cache->max += 4;
	cache->dialogs = (Widget *)
	    XtRealloc((char *)cache->dialogs, cache->max * sizeof(Widget));
	cache->help_strings = (String *)
	    XtRealloc((char *)cache->help_strings, cache->max*sizeof(String));
    }

    /* make a copy of the help text */
    help = XtNewString(help_text);

    /* create a new widget if necessary */
    if (cache->in_use == cache->num) {
	dialog = CreateMessageDialog(shell, cache);
	cache->dialogs[cache->num] = dialog;
	cache->help_strings[cache->num] = help;
	cache->num++;
    }
    /* or find one in the cache array */
    else {
	int i;
	for(i=0; i < cache->num; i++) {
	    if (!XtIsManaged(cache->dialogs[i])) {
		dialog = cache->dialogs[i];
		cache->help_strings[i] = help;
		break;
	    }
	}
    }

    /*
     * manage or unmanage the Help button, depending on whether
     * there is any help for this dialog
     */
    help_button = XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
    if (help_text) {
	XtManageChild(help_button);
	XtSetSensitive(help_button, True);
    }
    else
	XtUnmanageChild(help_button);

    cache->in_use++;
    *cache_return = cache;
    return dialog;
}



typedef struct {
    StringConst message;
    StringConst title;
    StringConst help;
    Pixmap icon;
} MessageDialogData;

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(MessageDialogData, message),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(MessageDialogData, title),
     XtRString, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(MessageDialogData, help),
     XtRString, NULL},
{XmtNicon, XmtCIcon, XtRPixmap,
     sizeof(Pixmap), XtOffsetOf(MessageDialogData, icon),
     XtRPixmap, (XtPointer) None},
};

#if NeedFunctionPrototypes
Boolean *_XmtDisplayMessage(Widget w,
                            StringConst dialog_name, StringConst dialog_class,
                            StringConst msg_default, va_list *msg_args,
                            StringConst title_default,
                            StringConst help_default, Pixmap icon_default,
                            int type, int style)
#else
Boolean *_XmtDisplayMessage(w, dialog_name, dialog_class,
                            msg_default, msg_args,
                            title_default, help_default, icon_default,
                            type, style)
Widget w;
StringConst dialog_name, dialog_class;
StringConst msg_default;
va_list *msg_args;
StringConst title_default, help_default;
Pixmap icon_default;
int type;
int style;
#endif
{
    Widget shell;
    Widget dialog;
    MessageDialogData data;
    XmString msg, title;
    Arg args[5];
    char buffer[20000];
    MsgDialogCache *cache;
    int i;
    static String message_title_default;
    static String info_title_default;
    static String error_title_default;
    static String warning_title_default;

    /*
     * go figure out which shell the dialog is cached for.
     * make sure it is not a menu shell.
     */
    shell = XmtGetShell(w);
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize title defaults, first time only */
    if (!message_title_default) {
	message_title_default = XmtLocalize2(w,XmtMESSAGE_DIALOG_TITLE_DEFAULT,
					     "XmtDisplayMessage",
					     "messageTitle");
	info_title_default = XmtLocalize2(w, XmtINFO_DIALOG_TITLE_DEFAULT,
					  "XmtDisplayMessage", "helpTitle");
	error_title_default = XmtLocalize2(w, XmtERROR_DIALOG_TITLE_DEFAULT,
					   "XmtDisplayMessage", "errorTitle");
	warning_title_default = XmtLocalize2(w,XmtWARNING_DIALOG_TITLE_DEFAULT,
					     "XmtDisplayMessage",
					     "warningTitle");
    }

    /* If NULL arguments were supplied, set up defaults */
    if (msg_default == NULL) msg_default = "";
    if (title_default == NULL) {
	switch(type) {
	case XmDIALOG_MESSAGE:
	    title_default = message_title_default; break;
	case XmDIALOG_INFORMATION:
	    title_default = info_title_default; break;
	case XmDIALOG_ERROR:
	    title_default = error_title_default; break;
	case XmDIALOG_WARNING:
	    title_default = warning_title_default; break;
	}
    }

    if ((dialog_class == NULL)&& (dialog_name != NULL)) {
	switch(type) {
	case XmDIALOG_MESSAGE:
	    dialog_class = XmtCMessageDialog;
	    break;
	case XmDIALOG_INFORMATION:
	    dialog_class = XmtCInformationDialog;
	    break;
   	case XmDIALOG_ERROR:
	    dialog_class = XmtCErrorDialog;
	    break;
	case XmDIALOG_WARNING:
	    dialog_class = XmtCWarningDialog;
	    break;
	}
    }

    /*
     * If this message has a name, look up its resources.
     * Note that we look up resources under whatever shell is closest
     * to the widget that requested this dialog.
     */
    if (dialog_name != NULL) {
	resources[0].default_addr = (XtPointer) msg_default;
	resources[1].default_addr = (XtPointer) title_default;
	resources[2].default_addr = (XtPointer) help_default;
	resources[3].default_addr = (XtPointer) icon_default;
	XtGetSubresources(shell, (XtPointer)&data,
			  dialog_name, dialog_class,
			  resources, XtNumber(resources), NULL, 0);
    }
    else { /* otherwise set them ourselves */
	data.message = msg_default;
	data.title = title_default;
	data.help = help_default;
	data.icon = icon_default;
    }

    /* find a dialog to reuse or create a new one */
    dialog = GetMessageDialog(shell, data.help, &cache);

    /*
     * if there were printf args, sprintf the message.
     * If there was a default message, and it was overridden,
     * check whether the arguments of the new message match
     * so we don't get a core dump
     */
    if (msg_args) {
	if (msg_default && (msg_default != data.message)) {
	    if (!XmtCheckPrintfFormat(msg_default, data.message)) {
		XmtWarningMsg("XmtDisplayMessage", "badfmt",
			      "message specified for dialog '%s' has bad format.\n\tUsing default message.",
			      dialog_name);
		data.message = msg_default;
	    }
	}
	vsnprintf(buffer, sizeof(buffer), data.message, *msg_args);
	data.message = buffer;
    }
    
    /* create XmStrings */
    msg = XmtCreateLocalizedXmString(w, data.message);
    title = XmtCreateLocalizedXmString(w, data.title);
    
    /*
     * reset the type of the dialog so that the next call will change it.
     * This is reqired to reset a custom icon back to the default icon
     * when the type doesn't change
     */
    XtSetArg(args[0], XmNdialogType, XmDIALOG_MESSAGE);
    XtSetValues(dialog, args, 1);

    /* set the message and title and dialog type */
    i = 0;
    XtSetArg(args[i], XmNdialogTitle, title); i++;
    XtSetArg(args[i], XmNmessageString, msg); i++;
    XtSetArg(args[i], XmNdialogType, type); i++;
    XtSetArg(args[i], XmNdialogStyle, style); i++;
    if (data.icon || type == XmDIALOG_MESSAGE) {
	XtSetArg(args[i], XmNsymbolPixmap,
		 data.icon?data.icon:XmUNSPECIFIED_PIXMAP);
	i++;
    }
    XtSetValues(dialog, args, i);

    /* free the XmStrings */
    XmStringFree(msg);
    XmStringFree(title);
    
    /* tell the dialog who it is transient for */
    XtVaSetValues(XtParent(dialog), XtNtransientFor, shell, NULL);

    /* Now position the dialog over the shell and pop it up */
    XmtDialogPosition(dialog, shell);
    cache->popped_up = True;
    XtManageChild(dialog);
    return &cache->popped_up;
}

#if NeedVarargsPrototypes
void XmtDisplayMessage(Widget w, StringConst name, StringConst class,
		       StringConst message, StringConst title,
		       StringConst help, Pixmap icon,
		       int modality, int type, ...)
#else
void XmtDisplayMessage(w, name, class, message, title, help, icon,
		       modality, type, va_alist)
Widget w;
String name, class, message, title, help;
Pixmap icon;
int modality, type;
va_dcl
#endif
{
    va_list args;
    
    Va_start(args, type);
    _XmtDisplayMessage(w, name, class, message, &args, title, help, icon,
		       type, modality);
    va_end(args);
}

#if NeedFunctionPrototypes
void XmtDisplayMessageVaList(Widget w, StringConst name, StringConst class,
			     StringConst message, StringConst title,
			     StringConst help, Pixmap icon,
			     int modality, int type, va_list args)
#else
void XmtDisplayMessageVaList(w, name, class, message, title, help, icon,
			     modality, type, args)
Widget w;
String name, class, message, title, help;
Pixmap icon;
int modality, type;
va_list args;
#endif
{
    _XmtDisplayMessage(w, name, class, message, &args, title, help, icon,
		       type, modality);
}


#if NeedVarargsPrototypes
void XmtDisplayMessageAndWait(Widget w, StringConst name, StringConst class,
			      StringConst message, StringConst title,
			      StringConst help, Pixmap icon,
			      int modality, int type, ...)
#else
void XmtDisplayMessageAndWait(w, name, class, message, title, help, icon,
			      modality, type, va_alist)
Widget w;
String name, class, message, title, help;
Pixmap icon;
int modality, type;
va_dcl
#endif
{
    va_list args;
    Boolean *block;
    
    if (modality == XmDIALOG_MODELESS) {
	XmtWarningMsg("XmtDisplayMessageAndWait", "modality",
		      "Can't use a modeless dialog with a blocking function.");
	modality = XmDIALOG_PRIMARY_APPLICATION_MODAL;
    }

    Va_start(args, type);
    block = _XmtDisplayMessage(w, name, class, message, &args,
			       title, help, icon, type, modality);
    va_end(args);
    XmtBlock(w, block);
}


#if NeedFunctionPrototypes
void XmtDisplayMessageAndWaitVaList(Widget w, StringConst name, 
				    StringConst class,
				    StringConst message, StringConst title,
				    StringConst help, Pixmap icon,
				    int modality, int type, va_list args)
#else
void XmtDisplayMessageAndWaitVaList(w, name, class, message, title, help, icon,
				    modality, type, args)
Widget w;
String name, class, message, title, help;
Pixmap icon;
int modality, type;
va_list args;
#endif
{
    Boolean *block;
    
    if (modality == XmDIALOG_MODELESS) {
	XmtWarningMsg("XmtDisplayMessageAndWaitVaList", "modality",
		      "Can't use a modeless dialog with a blocking function.");
	modality = XmDIALOG_PRIMARY_APPLICATION_MODAL;
    }
    
    block = _XmtDisplayMessage(w, name, class, message, &args,
			       title, help, icon, type, modality);
    XmtBlock(w, block);
}


#if NeedFunctionPrototypes
void XmtDisplayInformation(Widget w, StringConst dialog_name,
			   StringConst msg_default, StringConst title_default)
#else
void XmtDisplayInformation(w, dialog_name, msg_default, title_default)
Widget w;
StringConst dialog_name;
StringConst msg_default;
StringConst title_default;
#endif
{
    (void)_XmtDisplayMessage(w, dialog_name, XmtCInformationDialog,
			     msg_default, NULL,
			     title_default, NULL, None, 
		             XmDIALOG_INFORMATION, XmDIALOG_MODELESS);
}

#if NeedFunctionPrototypes
void XmtDisplayWarning(Widget w, StringConst dialog_name,
		       StringConst msg_default)
#else
void XmtDisplayWarning(w, dialog_name, msg_default)
Widget w;
StringConst dialog_name;
StringConst msg_default;
#endif
{
    (void)_XmtDisplayMessage(w, dialog_name, XmtCWarningDialog,
			     msg_default, NULL, NULL, NULL, 
			     None, XmDIALOG_WARNING, 
			     XmDIALOG_PRIMARY_APPLICATION_MODAL);
}

#if NeedFunctionPrototypes
void XmtDisplayError(Widget w, StringConst dialog_name,
		     StringConst msg_default)
#else
void XmtDisplayError(w, dialog_name, msg_default)
Widget w;
StringConst dialog_name;
StringConst msg_default;
#endif
{
      (void)_XmtDisplayMessage(w, dialog_name, XmtCErrorDialog,
			       msg_default, NULL, NULL, NULL, 
			       None, XmDIALOG_ERROR,
			       XmDIALOG_PRIMARY_APPLICATION_MODAL);
}


#if NeedVarargsPrototypes
void XmtDisplayWarningMsg(Widget w, StringConst name,
			  StringConst msg_default,
			  StringConst title_default,
			  StringConst help_default,
			  ...)
#else
void XmtDisplayWarningMsg(w, name, msg_default, title_default, help_default,
			  va_alist)
Widget w;
String name, title_default, help_default, msg_default;
va_dcl
#endif
{
    va_list args;

    Va_start(args, help_default);
    (void)_XmtDisplayMessage(w, name, XmtCWarningDialog,
			     msg_default, &args,
			     title_default, help_default, None, 
			     XmDIALOG_WARNING,
			     XmDIALOG_PRIMARY_APPLICATION_MODAL);
    va_end(args);
}

#if NeedVarargsPrototypes
void XmtDisplayErrorMsg(Widget w, StringConst name,
			StringConst msg_default,
			StringConst title_default,
			StringConst help_default,
			...)
#else
void XmtDisplayErrorMsg(w, name, msg_default, title_default, help_default,
			va_alist)
Widget w;
String name, title_default, help_default, msg_default;
va_dcl
#endif
{
    va_list args;

    Va_start(args, help_default);
    (void)_XmtDisplayMessage(w, name, XmtCErrorDialog,
			     msg_default, &args,
			     title_default, help_default, None,
			     XmDIALOG_ERROR,
			     XmDIALOG_PRIMARY_APPLICATION_MODAL);
    va_end(args);
}

#if NeedVarargsPrototypes
void XmtDisplayInformationMsg(Widget w, StringConst name,
			      StringConst msg_default,
			      StringConst title_default,
			      StringConst help_default,
			      ...)
#else
void XmtDisplayInformationMsg(w, name,
			      msg_default, title_default, help_default,
			      va_alist)
Widget w;
String name, title_default, help_default, msg_default;
va_dcl
#endif
{
    va_list args;

    Va_start(args, help_default);
    (void)_XmtDisplayMessage(w, name, XmtCInformationDialog,
			     msg_default, &args,
			     title_default, help_default, None,
			     XmDIALOG_INFORMATION, XmDIALOG_MODELESS);
    va_end(args);
}

#if NeedVarargsPrototypes
void XmtDisplayWarningMsgAndWait(Widget w, StringConst name,
				 StringConst msg_default,
				 StringConst title_default,
				 StringConst help_default,
				 ...) 
#else
void XmtDisplayWarningMsgAndWait(w, name,
				 msg_default, title_default, help_default,
				 va_alist)
Widget w;
String name, title_default, help_default, msg_default;
va_dcl
#endif
{
    va_list args;
    Boolean *block;

    Va_start(args, help_default);
    block = _XmtDisplayMessage(w, name, XmtCWarningDialog,
			       msg_default, &args,
			       title_default, help_default, None,
			       XmDIALOG_WARNING,
			       XmDIALOG_PRIMARY_APPLICATION_MODAL);
    va_end(args);
    XmtBlock(w, block);
}

#if NeedVarargsPrototypes
void XmtDisplayErrorMsgAndWait(Widget w, StringConst name,
			       StringConst msg_default,
			       StringConst title_default,
			       StringConst help_default,
			       ...)
#else
void XmtDisplayErrorMsgAndWait(w, name,
			       msg_default, title_default, help_default,
			       va_alist)
Widget w;
String name, title_default, help_default, msg_default;
va_dcl
#endif
{
    va_list args;
    Boolean *block;

    Va_start(args, help_default);
    block = _XmtDisplayMessage(w, name, XmtCErrorDialog,
			       msg_default, &args,
			       title_default, help_default, None,
			       XmDIALOG_ERROR,
			       XmDIALOG_PRIMARY_APPLICATION_MODAL);
    va_end(args);
    XmtBlock(w, block);
}

