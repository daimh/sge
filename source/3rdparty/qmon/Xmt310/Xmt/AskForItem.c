/* 
 * Motif Tools Library, Version 3.1
 * $Id: AskForItem.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AskForItem.c,v $
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

#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xmt/Converters.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/List.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
static void NoMatchCallback(Widget w, XtPointer tag, XtPointer data)
#else
static void NoMatchCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtDisplayError(w, XmtNO_MATCH_DIALOG,
		    XmtLocalize2(w, XmtNO_MATCH_MESSAGE,
				 "XmtAskForItem", "noMatch"));
}

#if NeedFunctionPrototypes
static void CreateItemDialog(XmtPerScreenInfo *info)
#else
static void CreateItemDialog(info)
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
				 XmtITEM_DIALOG_SHELL_NAME, args, i);

    /*
     * create the dialog box itself.
     * Use WORK_AREA type so we don't get an apply button.
     */
    i = 0;
    XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
    XtSetArg(args[i], XmNdialogType, XmDIALOG_WORK_AREA); i++;
    XtSetArg(args[i], XmNautoUnmanage, False); i++;
    XtSetArg(args[i], XmNdefaultPosition, False); i++;
    dialog = XmCreateSelectionBox(dshell, XmtITEM_DIALOG_NAME, args, i);

    /* add callbacks on all the dialog buttons */
    XtAddCallback(dialog, XmNokCallback, _XmtOkCallback, (XtPointer)info);
    XtAddCallback(dialog, XmNcancelCallback,
		  _XmtCancelCallback, (XtPointer)info);
    XtAddCallback(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON),
		  XmNactivateCallback, _XmtHelpCallback, (XtPointer)info);

    /* add a callback for f.close */
    XmtAddDeleteCallback(dshell, XmDO_NOTHING,
			 _XmtCancelCallback, (XtPointer)info);

    /*
     * If XmNmustMatch is True, the selection box does its own checking.
     * We only need to add a callback to handle the no match case.
     */
    XtAddCallback(dialog, XmNnoMatchCallback, NoMatchCallback,(XtPointer)info);
    
    /* cache the dialog in the PerScreenInfo structure */
    info->item_dialog = dialog;
}

typedef struct {
    String message;
    String title;
    String list_title;
    String *items;
    String help_text;
    int visible_items;
} ItemDialogData;

static XtResource resources[] = {
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(ItemDialogData, message),
     XtRString, NULL},
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(ItemDialogData, title),
     XtRString, NULL},
{XmtNlistTitle, XmtCListTitle, XtRString,
     sizeof(String), XtOffsetOf(ItemDialogData, list_title),
     XtRString, NULL},
{XmtNitems, XmtCItems, XmtRStringList,
     sizeof(String *), XtOffsetOf(ItemDialogData, items),
     XtRImmediate, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(ItemDialogData, help_text),
     XtRString, NULL},
{XmtNvisibleItems, XmtCVisibleItems, XtRInt,
     sizeof(int), XtOffsetOf(ItemDialogData, visible_items),
     XtRImmediate, (XtPointer)8}
};


#if NeedFunctionPrototypes
static Boolean GetItem(Widget w, StringConst dialog_name,
                StringConst prompt_default,
		StringConst list_title_default, String *items_default, 
		int num_items_default, Boolean must_match,
		String buffer_return, int buffer_len,
		int *int_return, StringConst help_text_default,
		Boolean return_number)
#else
static Boolean GetItem(w, dialog_name, prompt_default, list_title_default,
                       items_default, num_items_default, must_match,
                       buffer_return, buffer_len, int_return,
                       help_text_default, return_number)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst list_title_default;
String *items_default;
int num_items_default;
Boolean must_match;
String buffer_return;
int buffer_len;
int *int_return;
StringConst help_text_default;
Boolean return_number;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    ItemDialogData data;
    Widget help_w;
    int num_items;
    static String localized_title;
    XmString message, title, list_title;
    XmString *item_labels;
    XmString default_item_label;
    XmString selected_item_label;
    String selected_item;
    Widget text_widget;
    int i;
    static Boolean registered;

    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize the default title the first time through */
    if (!localized_title)
	localized_title = XmtLocalize2(w, XmtITEM_DIALOG_TITLE_DEFAULT,
				       "XmtAskForItem", "title");

    /* if there's no cached dialog, create one. */
    if (info->item_dialog == NULL) CreateItemDialog(info);

    /* if this dialog has a name, look up its resources */
    if (dialog_name != NULL) {
	if (!registered) {
	    XmtRegisterStringListConverter();
	    registered = True;
	}
	resources[0].default_addr = (XtPointer) prompt_default;
	resources[1].default_addr = (XtPointer) localized_title;
	resources[2].default_addr = (XtPointer) list_title_default;
	resources[3].default_addr = (XtPointer) items_default;
	resources[4].default_addr = (XtPointer) help_text_default;
	resources[5].default_addr = (XtPointer) 8;
	XtGetSubresources(shell, (XtPointer)&data,
			  (String)dialog_name, XmtCItemDialog,
			  resources, XtNumber(resources),
			  NULL, 0);
    }
    else { /* otherwise use arguments as defaults */
	data.message = (String) prompt_default;
	data.title = localized_title;
	data.list_title = (String)list_title_default;
	data.items = items_default;
	data.help_text = (String) help_text_default;
	data.visible_items = 8;  /* no argument for this one */
    }

    /* create the XmStrings */
    message = XmtCreateLocalizedXmString(w, data.message);
    title = XmtCreateLocalizedXmString(w, data.title);
    list_title = XmtCreateLocalizedXmString(w, data.list_title);

    /*
     * If we got an items string from the resource db,
     * it is NULL-terminated, and we need to count the number
     * of items.  Otherwise we can just use num_items_default.
     */
    if (data.items != items_default) {
	for(num_items=0; data.items[num_items]; num_items++);
    }
    else {
	num_items = num_items_default;
    }

    /* convert the list items to XmStrings */
    item_labels = (XmString *) XtMalloc(num_items * sizeof(XmString));
    for(i = 0; i < num_items; i++)
	item_labels[i] = XmtCreateLocalizedXmString(w, data.items[i]);

    if (return_number)
	default_item_label = item_labels[*int_return];
    else if (buffer_return && *buffer_return)
	default_item_label = XmtCreateLocalizedXmString(w, buffer_return);
    else
	default_item_label = item_labels[0];

    /* set resources on the dialog */
    XtVaSetValues(info->item_dialog,
		  XmNselectionLabelString, message,
		  XmNdialogTitle, title,
		  XmNlistLabelString, list_title,
		  XmNlistItems, item_labels,
		  XmNlistItemCount, num_items,
		  XmNlistVisibleItemCount,
		    (num_items < data.visible_items)
		       ?((num_items>0)?num_items:1):data.visible_items,
		  XmNmustMatch, must_match,
		  XmNtextString, default_item_label,
		  NULL);

    /*
     * Be sure that the item selected in the list widget is the
     * same as the item displayed in the text widget.  This does
     * not happen just by setting XmNtextString, and since we reuse
     * this dialog, this is important to reset each time.
     */
    XmListDeselectAllItems(XmSelectionBoxGetChild(info->item_dialog,
						  XmDIALOG_LIST));
    XmListSelectItem(XmSelectionBoxGetChild(info->item_dialog, XmDIALOG_LIST),
		     default_item_label, False);


#if 0  /* this is a great idea, but it doesn't work */

    /* if there are no items in the list, unmanage list and title */
    list_w = XmSelectionBoxGetChild(info->item_dialog, XmDIALOG_LIST);
    list_label_w = XmSelectionBoxGetChild(info->item_dialog,
					  XmDIALOG_LIST_LABEL);
    if (num_items == 0) {
	XtUnmanageChild(list_w);
	XtUnmanageChild(list_label_w);
    }
    else {
	XtManageChild(list_w);
	XtManageChild(list_label_w);
    }
#endif
    
    /*
     * if there is help text, make the button sensitive,
     * and put help text where the callback procedure can find it.
     */
    help_w = XmSelectionBoxGetChild(info->item_dialog, XmDIALOG_HELP_BUTTON);
    if ((data.help_text != NULL) && (data.help_text[0] != '\0')) {
	XtSetSensitive(help_w, True);
	info->help_text = data.help_text;
    }
    else {
	XtSetSensitive(help_w, False);
	info->help_text = NULL;
    }

    /* destroy the XmStrings  */
    XmStringFree(message);
    XmStringFree(title);
    XmStringFree(list_title);
    for(i = 0; i < num_items; i++)
	XmStringFree(item_labels[i]);
    XtFree((char *)item_labels);
    if (!return_number && buffer_return && *buffer_return)
	XmStringFree(default_item_label);
    
    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(info->item_dialog), XtNtransientFor, shell, NULL);

    /* position, set initial focus, and pop up the dialog */
    XmtDialogPosition(info->item_dialog, shell);
    text_widget = XmSelectionBoxGetChild(info->item_dialog, XmDIALOG_TEXT);
    XmtSetInitialFocus(info->item_dialog, text_widget);
    XmTextSetInsertionPosition(text_widget,XmTextGetLastPosition(text_widget));
    XtManageChild(info->item_dialog);

    /*
     * Enter a recursive event loop.
     * The callback registered on the okay and cancel buttons when
     * this dialog was created will cause info->button to change
     * when one of those buttons is pressed.
     */
    info->blocked = True;
    XmtBlock(shell, &info->blocked);

    /* pop down the dialog */
    XtUnmanageChild(info->item_dialog);

    /* make sure what is underneath gets cleaned up
     * (the calling routine might act on the user's returned
     * input and not handle events for awhile.)
     */
    XSync(XtDisplay(info->item_dialog), 0);
    XmUpdateDisplay(info->item_dialog);

    /*
     * if the user clicked Ok, figure out the selected string
     * and set the return values.
     */
    if (info->button == XmtOkButton) {
	XtVaGetValues(info->item_dialog,
		      XmNtextString, &selected_item_label, NULL);
	XmStringGetLtoR(selected_item_label, XmSTRING_DEFAULT_CHARSET,
			&selected_item);
	XmStringFree(selected_item_label);

	/* put the string in the buffer */
	if (!return_number) {
	    strncpy(buffer_return, selected_item, buffer_len-1);
	    buffer_return[buffer_len-1] = '\0';
	}
	else {  /* or figure out the item number */
	    for(i = 0; i < num_items-1; i++)
		if (strcmp(selected_item, data.items[i]) == 0) break;
	    *int_return = i;
	}
	XtFree(selected_item);
    }

    /* if user clicked Cancel, return False, else True */
    if (info->button == XmtCancelButton) return False;
    else return True;
}



#if NeedFunctionPrototypes
Boolean XmtAskForItem(Widget w, StringConst dialog_name,
		      StringConst prompt_default,
                      StringConst list_title_default,
		      String *items_default, 
                      int num_items_default, XmtWideBoolean must_match,
                      String buffer_return, int buffer_len,
                      StringConst help_text_default)
#else
Boolean XmtAskForItem(w, dialog_name, prompt_default, list_title_default,
		      items_default, num_items_default, must_match,
		      buffer_return, buffer_len, help_text_default)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst list_title_default;
String *items_default;
int num_items_default;
int must_match;
String buffer_return;
int buffer_len;
StringConst help_text_default;
#endif
{
    return GetItem(w, dialog_name, prompt_default, list_title_default,
		   items_default, num_items_default, must_match,
		   buffer_return, buffer_len, NULL, help_text_default, False);
}


#if NeedFunctionPrototypes
Boolean XmtAskForItemNumber(Widget w, StringConst dialog_name,
			    StringConst prompt_default,
                            StringConst list_title_default,
			    String *items_default,
                            int num_items_default, int *value_return,
                            StringConst help_text_default)
#else
Boolean XmtAskForItemNumber(w, dialog_name, prompt_default, list_title_default,
			    items_default, num_items_default, value_return,
			    help_text_default)
Widget w;
StringConst dialog_name;
StringConst prompt_default;
StringConst list_title_default;
String *items_default;
int num_items_default;
int *value_return;
StringConst help_text_default;
#endif
{
    return GetItem(w, dialog_name, prompt_default, list_title_default,
		   items_default, num_items_default, True, NULL, 0,
		   value_return, help_text_default, True);
}
