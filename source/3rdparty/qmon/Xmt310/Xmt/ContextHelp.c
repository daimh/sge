/* 
 * Motif Tools Library, Version 3.1
 * $Id: ContextHelp.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ContextHelp.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Help.h>
#include <Xmt/HelpBox.h>
#include <Xmt/AppResP.h>
#include <Xmt/ScreenP.h>
#include <Xmt/Util.h>
#include <Xm/BulletinB.h>
#include <X11/IntrinsicP.h>
#include <X11/CompositeP.h>  /* to traverse children lists */
#include <X11/ShellP.h>  /* to get application shell class name */
#include <Xmt/XmtP.h>

#if NeedFunctionPrototypes
void LoadHelpFile(Widget w)
#else
void LoadHelpFile(w)
Widget w;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);
    String filename = NULL;

    if (app->help_database) return;

    if (app->context_help_file)
	filename = XmtFindFile(w, "help", app->context_help_file,
			       APPDEFAULTSSUFFIX,
			       NULL, app->help_file_path,
			       XmtSearchAppPath);

    if (filename != NULL) {
	app->help_database = XrmGetFileDatabase(filename);
	XtFree(filename);
    }

    if (app->help_database)
	app->free_help_database = True;
    else {
	app->help_database = XmtDatabaseOfWidget(w);
	app->free_help_database = False;
    }
}

#if NeedFunctionPrototypes
static Boolean IsModal(Widget w)
#else
static Boolean IsModal(w)
Widget w;
#endif
{
    Widget shell, child;
    unsigned char style;

    shell = XmtGetShell(w);
    child = ((CompositeWidget)shell)->composite.children[0];

    if (!XmIsBulletinBoard(child)) return False;
    XtVaGetValues(child, XmNdialogStyle, &style, NULL);
    if ((style == XmDIALOG_MODELESS) ||
	(style == XmDIALOG_WORK_AREA)) return False;
    else return True;
}

#if NeedFunctionPrototypes
static Widget GetHelpDialog(Widget w)
#else
static Widget GetHelpDialog(w)
Widget w;
#endif
{
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(w);
    Widget dialog = NULL;
    XmString title;
    Arg args[2];
    static String title_string;

    /* localize the dialog title, first time called */
    if (!title_string)
	title_string = XmtLocalize2(w, "Help On Context",
				    "XmtHelpDisplayContextHelp",
				    "dialogTitle");

    /* grow the cache, if needed */
    if (info->help_dialog_cache.in_use == info->help_dialog_cache.max)
    {
	info->help_dialog_cache.max += 4;
	info->help_dialog_cache.dialogs = (Widget *)
	    XtRealloc((char *)info->help_dialog_cache.dialogs,
		      info->help_dialog_cache.max * sizeof(Widget));
    }

    /* create a new widget if necessary */
    if (info->help_dialog_cache.in_use == info->help_dialog_cache.num) {
	title = XmStringCreateSimple(title_string);
	XtSetArg(args[0], XmNdialogTitle, title);
	XtSetArg(args[1], XmNdefaultPosition, False);
	dialog = XmtCreateHelpDialog(info->topmost_shell, "xmtHelpDialog",
				     args, 2);
	XmStringFree(title);
	XtAddCallback(XtParent(dialog), XtNpopdownCallback,
		      _XmtDialogPopdownCallback,
		      (XtPointer)&info->help_dialog_cache);
	info->help_dialog_cache.dialogs[info->help_dialog_cache.num++]=dialog;
    }
    else { /* look up an existing one */
	int i;
	for(i=0; i < info->help_dialog_cache.num; i++) {
	    dialog = info->help_dialog_cache.dialogs[i];
	    if (!dialog->core.managed) break;
	}
    }

    info->help_dialog_cache.in_use++;
    return dialog;
}

/*
 * Get any context sensitive help text and title for a widget.
 * The returned strings must be freed with XtFree when no longer needed.
 */
#if NeedFunctionPrototypes
void XmtHelpGetContextHelp(Widget helpw,
			   String *help_string, String *help_title)
#else
void XmtHelpGetContextHelp(helpw, help_string, help_title)
Widget helpw;
String *help_string;
String *help_title;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(helpw);
    XrmQuark names[50];
    XrmQuark classes[50];
    static XrmQuark XmtQxmtHelp = NULLQUARK;
    static XrmQuark XmtQXmtHelp = NULLQUARK;
    static XrmQuark XmtQxmtHelpTitle = NULLQUARK;
    static XrmQuark XmtQXmtHelpTitle = NULLQUARK;
    static XrmQuark XmtQxmtHelpNode = NULLQUARK;
    static XrmQuark XmtQXmtHelpNode = NULLQUARK;
    int count, i;
    Widget w;

    if (!app->help_database) LoadHelpFile(helpw);

    if (XmtQxmtHelp == NULLQUARK) {
	XmtQxmtHelp = XrmPermStringToQuark(XmtNxmtHelp);
	XmtQXmtHelp = XrmPermStringToQuark(XmtCXmtHelp);
	XmtQxmtHelpTitle = XrmPermStringToQuark(XmtNxmtHelpTitle);
	XmtQXmtHelpTitle = XrmPermStringToQuark(XmtCXmtHelpTitle);
	XmtQxmtHelpNode = XrmPermStringToQuark(XmtNxmtHelpNode);
	XmtQXmtHelpNode = XrmPermStringToQuark(XmtCXmtHelpNode);
    }

    for(w=helpw, count = 0; w; count++, w = XtParent(w));
    for(i=count-1, w = helpw; i >= 0; i--, w = XtParent(w)) {
	names[i] = w->core.xrm_name;
	if (i > 0 || !XtIsApplicationShell(w))
	    classes[i] = w->core.widget_class->core_class.xrm_class;
	else
	    classes[i] = ((ApplicationShellWidget)w)->application.xrm_class;
    }

    *help_string = NULL;
    *help_title = NULL;

    while(count > 0 && (!*help_string || !*help_title)) {
	XrmValue value;
	XrmQuark type;
	XmtHelpNode *node;

	names[count+1] = NULLQUARK;
	classes[count+1] = NULLQUARK;

	names[count] = XmtQxmtHelpNode;
	classes[count] = XmtQXmtHelpNode;
	if (XrmQGetResource(app->help_database, names, classes,
			    &type, &value)) {
	    node = XmtHelpLookupNode(helpw, value.addr);
	    if (!node)
		XmtWarningMsg("XmtHelpGetContextHelp", "badNode",
			      "help node '%s' is undefined.",
			      value.addr);
	    else {
		if (!*help_string) *help_string = XmtHelpNodeGetBody(node);
		if (!*help_title)
		    *help_title = XtNewString(XmtHelpNodeGetTitle(node));
	    }
	}
	
	if (!*help_string) {
	    names[count] = XmtQxmtHelp;
	    classes[count] = XmtQXmtHelp;
	    if (XrmQGetResource(app->help_database, names, classes,
				&type, &value))
		*help_string = strcpy(XtMalloc(value.size), value.addr);
	}

	if (!*help_title) {
	    names[count] = XmtQxmtHelpTitle;
	    classes[count] = XmtQXmtHelpTitle;
	    if (XrmQGetResource(app->help_database, names, classes,
				&type, &value)) 
		*help_title = strcpy(XtMalloc(value.size), value.addr);
	}
	count--;
    }
}

#if NeedFunctionPrototypes
void XmtHelpDisplayContextHelp(Widget w)
#else
void XmtHelpDisplayContextHelp(w)
Widget w;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);
    String text, title;
    Widget dialog;
    Widget shell, non_menu_shell;
    static String default_title;
    static String no_help_message;
    static String funcname = "XmtHelpDisplayContextHelp";

    /* Localize the default strings, first time called */
    if (!default_title) {
	default_title = XmtLocalize2(w, "Context Help", funcname, "helpTitle");
	no_help_message =
	    XmtLocalize2(w, "There is no help available there.",
			 funcname, "noHelp");
    }

    /* get the help text and title */
    XmtHelpGetContextHelp(w, &text, &title);

    /* get a dialog shell to display the help in */
    dialog = GetHelpDialog(w);

    /* set dialog help text, title, and icon */
    /* and set modality, depending on the modality of what it is over */
    XtVaSetValues(dialog,
		  XmtNhelpText,
		  (text)?text:no_help_message,
		  XmtNhelpTitle, (title)?title:"Context Help",
		  XmtNhelpPixmap, app->help_pixmap,
		  XmNdialogStyle,(IsModal(w))
		      ? XmDIALOG_PRIMARY_APPLICATION_MODAL
		      : XmDIALOG_MODELESS,
		  NULL);

    /* Set the XtNtransientFor resource */
    non_menu_shell = shell = XmtGetShell(w);
    while(XtIsOverrideShell(non_menu_shell))
	non_menu_shell = XmtGetShell(XtParent(non_menu_shell));
    XtVaSetValues(XtParent(dialog), XtNtransientFor, non_menu_shell, NULL);

    /*
     * position the dialog centered over the widget requesting help,
     * unless that widget is in a menu shell and thus no longer popped up.
     */
    XmtDialogPosition(dialog, (shell==non_menu_shell)?w:non_menu_shell);

    /* pop it up */
    XtManageChild(dialog);

    XtFree(text);
    XtFree(title);
}

#if NeedFunctionPrototypes
void XmtHelpDoContextHelp(Widget w)
#else
void XmtHelpDoContextHelp(w)
Widget w;
#endif
{
    XmtAppResources *app;
    Widget target;

    w = XmtGetApplicationShell(w);
    app = XmtGetApplicationResources(w);
    target = XmTrackingLocate(w, app->help_cursor, False);
    if (target)
	XmtHelpDisplayContextHelp(target);
    else 
	XBell(XtDisplay(w), 0);
}


/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtHelpContextHelpCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtHelpContextHelpCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtHelpDisplayContextHelp(w);
}


#if NeedFunctionPrototypes
void XmtHelpInstallContextHelp(Widget w, XtCallbackProc proc, XtPointer data)
#else
void XmtHelpInstallContextHelp(w, proc, data)
Widget w;
XtCallbackProc proc;
XtPointer data;
#endif
{
    CompositeWidget c;
    int i;

    /* install callback on the specified widget, if appropriate */
    if ((XmIsManager(w) && ((XmManagerWidget)w)->manager.traversal_on) ||
	(XmIsPrimitive(w) && ((XmPrimitiveWidget)w)->primitive.traversal_on) ||
	(XmIsGadget(w) && ((XmGadget)w)->gadget.traversal_on))
	XtAddCallback(w, XmNhelpCallback, proc, data);
 
    /* recurse on all normal children */
    if (XtIsComposite(w)) {
	c = (CompositeWidget) w;
	for(i=0; i < c->composite.num_children; i++)
	    XmtHelpInstallContextHelp(c->composite.children[i], proc, data);
    }

    /* and recurse on any popup children */
    if (XtIsWidget(w))
	for(i=0; i < w->core.num_popups; i++)
	    XmtHelpInstallContextHelp(w->core.popup_list[i], proc, data);
}
