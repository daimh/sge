/* 
 * Motif Tools Library, Version 3.1
 * $Id: Dialog.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Dialog.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/DialogP.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Hash.h>
#include <X11/IntrinsicP.h>
#include <X11/CompositeP.h>

static XmtHashTable widget_to_dialog_table = NULL;

#if NeedFunctionPrototypes
static void _XmtDialogStoreDialogInfo(Widget w, XmtDialogInfo *d)
#else
static void _XmtDialogStoreDialogInfo(w, d)
Widget w;
XmtDialogInfo *d;
#endif
{
    if (!widget_to_dialog_table)
	widget_to_dialog_table = XmtHashTableCreate(3);

    XmtHashTableStore(widget_to_dialog_table, (XtPointer)w, (XtPointer)d);
}

#if NeedFunctionPrototypes
static void DestroyDialogInfo(Widget w, XtPointer tag, XtPointer data)
#else
static void DestroyDialogInfo(w, tag, data)
Widget w;
XtPointer tag, data;
#endif
{
    XmtDialogInfo *d;

    if (!widget_to_dialog_table) return;

    if (XmtHashTableLookup(widget_to_dialog_table, (XtPointer) w,
			   (XtPointer *)&d)) {
	XmtHashTableDelete(widget_to_dialog_table, (XtPointer)w);
	XtFree((char *)d->widgets);
	XtFree((char *)d->types);
	XtFree((char *)d);
    }
}

#if NeedFunctionPrototypes
XmtDialogInfo *_XmtDialogLookupDialogInfo(Widget w)
#else
XmtDialogInfo *_XmtDialogLookupDialogInfo(w)
Widget w;
#endif
{
    XmtDialogInfo *d = NULL;
    
    if (!widget_to_dialog_table) return NULL;
    for(;w; w = XtParent(w)) {
	if (XmtHashTableLookup(widget_to_dialog_table,
			       (XtPointer)w, (XtPointer *)&d)) break;
    }
    return d;
}

#if NeedFunctionPrototypes
void XmtDialogSetDialogValues(Widget w, XtPointer base)
#else
void XmtDialogSetDialogValues(w, base)
Widget w;
XtPointer base;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(w);
    int i;
    XtPointer address;

    if (!d) {
	XmtWarningMsg("XmtDialogSetDialogValues", "undef",
		      "no dialog information for widget '%s'.\n\tUse XmtBuildDialog() or call XmtDialogBindResourceList().",
		      XtName(w));
	return;
    }

    for(i=0; i < d->num_resources; i++) {
	/* resource list is compiled, so offset is one's complement */
	address = (XtPointer)((char *)base +
			      (-d->resources[i].resource_offset-1));
	if (d->types[i]) {
	    if (d->types[i]->set_value_proc)
		(*d->types[i]->set_value_proc)(d->widgets[i], address,
				      (XrmQuark)d->resources[i].resource_type,
				      d->resources[i].resource_size);
	    else
		XmtWarningMsg("XmtDialogSetDialogValues", "noproc",
		     "Resource '%s':\n\twidget type '%s' nas no set value procedure",
		     XrmQuarkToString((XrmQuark)d->resources[i].resource_name),
		     d->types[i]->name);
	}
/*
	else
	    XmtWarningMsg("XmtDialogSetDialogValues", "undef",
		   "\n\tno child of dialog '%s' to set resource '%s' on",
		   XtName(w),
	           XrmQuarkToString((XrmQuark)d->resources[i].resource_name));
*/
    }
}

#if NeedFunctionPrototypes
void XmtDialogGetDialogValues(Widget w, XtPointer base)
#else
void XmtDialogGetDialogValues(w, base)
Widget w;
XtPointer base;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(w);
    int i;
    XtPointer address;

    if (!d) {
	XmtWarningMsg("XmtDialogGetDialogValues", "undef",
		      "no dialog information for widget '%s'.\n\tWidget was probably not created with XmtBuildDialog().",
		      XtName(w));
	return;
    }

    for(i=0; i < d->num_resources; i++) {
	/* resource list is compiled, so offset is one's complement */
	address = (XtPointer) ((char *)base +
			       (-d->resources[i].resource_offset-1));
	if (d->types[i]) {
	    if (d->types[i]->get_value_proc)
		(*d->types[i]->get_value_proc)(d->widgets[i], address,
				       (XrmQuark)d->resources[i].resource_type,
				       d->resources[i].resource_size);
	}
/*
	else
	    XmtWarningMsg("XmtDialogGetDialogValues", "undef",
		   "\n\tno child of dialog '%s' to get resource '%s' from",
		   XtName(w),
	           XrmQuarkToString((XrmQuark)d->resources[i].resource_name));
*/		   
    }
}

#if NeedFunctionPrototypes
void XmtDialogGetDefaultValues(Widget w, XtPointer base,
			       ArgList args, Cardinal num_args)
#else
void XmtDialogGetDefaultValues(w, base, args, num_args)
Widget w;
XtPointer base;
ArgList args;
Cardinal num_args;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(w);
    
    if (!d) {
	XmtWarningMsg("XmtDialogGetDefaultValues", "undef",
		      "no dialog information for widget '%s'.\n\tWidget was probably not created with XmtBuildDialog().",
		      XtName(w));
	return;
    }

    XtGetSubresources(w, base,
		      XmtNdefaults, XmtCDefaults,
		      d->resources, d->num_resources,
		      args, num_args);
}


#if NeedFunctionPrototypes
XtPointer XmtDialogGetDataAddress(Widget dialog)
#else
XtPointer XmtDialogGetDataAddress(dialog)
Widget dialog;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(dialog);
    return d->base;
}

#if NeedFunctionPrototypes
void XmtDialogSetReturnValue(Widget dialog, XmtWideBoolean value)
#else
void XmtDialogSetReturnValue(dialog, value)
Widget dialog;
XmtWideBoolean value;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(dialog);
    d->return_value = value;
    d->blocking = False;
}


/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtDialogOkayCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtDialogOkayCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(shell);
    
    if (!d) return;

    XmtDialogGetDialogValues(shell, d->base);
    d->return_value = True;
    d->blocking = False;
    XtUnmanageChild(((CompositeWidget)shell)->composite.children[0]);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtDialogCancelCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtDialogCancelCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(shell);
    
    if (!d) return;
    d->return_value = False;
    d->blocking = False;
    XtUnmanageChild(((CompositeWidget)shell)->composite.children[0]);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtDialogApplyCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtDialogApplyCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(shell);
    
    if (!d) return;
    XmtDialogGetDialogValues(shell, d->base);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtDialogResetCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtDialogResetCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(shell);
    
    if (!d) return;
    XmtDialogSetDialogValues(shell, d->base);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
void XmtDialogDoneCallback(Widget w, XtPointer tag, XtPointer data)
#else
void XmtDialogDoneCallback(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    Widget shell = XmtGetShell(w);
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(shell);
    
    if (!d) return;
    d->return_value = True;
    d->blocking = False;
    XtUnmanageChild(((CompositeWidget)shell)->composite.children[0]);
}

#if NeedFunctionPrototypes
void XmtDialogDo(Widget w, XtPointer base)
#else
void XmtDialogDo(w, base)
Widget w;
XtPointer base;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(w);
	
    if (!d) {
	XmtWarningMsg("XmtDialogDo", "undef",
		      "no dialog information for widget '%s'.\n\tWidget was probably not created with XmtBuildDialog().",
		      XtName(w));
	return;
    }
    if (XtIsManaged(w)) {
	XmtWarningMsg("XmtDialogDo", "managed",
		      "dialog '%s' is already popped up.",
		      XtName(w));
    }

    XmtDialogSetDialogValues(w, base);
    d->base = base;
    XtManageChild(w);
}


#if NeedFunctionPrototypes
Boolean XmtDialogDoSync(Widget w, XtPointer base)
#else
Boolean XmtDialogDoSync(w, base)
Widget w;
XtPointer base;
#endif
{
    XmtDialogInfo *d = _XmtDialogLookupDialogInfo(w);
	
    if (d->blocking) {
	XmtWarningMsg("XmtDialogDoSync", "block",
		      "dialog '%s' already blocking for input.",
		      XtName(w));
	return False;
    }

    XmtDialogDo(w, base);
    
    d->blocking = True;
    XmtBlock(w, &d->blocking);
    return d->return_value;
}

#if NeedFunctionPrototypes
void XmtDialogBindResourceList(Widget w,
			       XtResourceList resources,Cardinal num_resources)
#else
void XmtDialogBindResourceList(w, resources, num_resources)
Widget w;
XtResourceList resources;
Cardinal num_resources;
#endif
{
    XmtDialogInfo *d;
#ifdef X11R5
#if NeedFunctionPrototypes
    extern void _XtCompileResourceList(XtResourceList, Cardinal);
#else
    extern void _XtCompileResourceList();
#endif
#else
#if NeedFunctionPrototypes
    extern void XrmCompileResourceList(XtResourceList, Cardinal);
#else
    extern void XrmCompileResourceList();
#endif    
#endif
    
    /* compile the resource list, if it needs it  */
    if (num_resources && (((int)resources[0].resource_offset) >= 0)) {
#ifdef X11R5	
	_XtCompileResourceList(resources, num_resources);
#else
        XrmCompileResourceList(resources, num_resources);
#endif
    }

    /* create and initialize a dialog info strucutre */
    d = XtNew(XmtDialogInfo);
    d->resources = resources;
    d->num_resources = num_resources;
    d->widgets = (Widget *)XtCalloc(num_resources, sizeof(Widget));
    d->types = (XmtWidgetType **)
	XtCalloc(num_resources, sizeof(XmtWidgetType *));
    d->base = NULL;
    d->blocking = False;
    d->return_value = False;
    
    /* associate the structure with the shell */
    _XmtDialogStoreDialogInfo(w, d);

    /*
     * and register a callback to disassociate and free the structure
     * when the shell is destroyed
     */
    XtAddCallback(w, XtNdestroyCallback, DestroyDialogInfo, NULL);
}    
