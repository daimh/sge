/* 
 * Motif Tools Library, Version 3.1
 * $Id: Create.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Create.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Include.h>
#include <Xmt/Template.h>
#include <Xmt/DialogP.h>
#include <Xmt/Lexer.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/AppResP.h>
#include <Xm/DialogS.h>
#include <X11/IntrinsicP.h>
#include <X11/CompositeP.h>

#include <unistd.h>


#if NeedFunctionPrototypes
static void ConvertCallbackList(Widget w, StringConst s,
				XtCallbackList *list_return,
				XtCacheRef *ref_return)
#else
static void ConvertCallbackList(w, s, list_return, ref_return)
Widget w;
StringConst s;
XtCallbackList *list_return;
XtCacheRef *ref_return;
#endif
{
    XrmValue from, to;

    if (_XmtCallbackConverter == NULL) {
	XmtWarningMsg("CreateWidget", "noConverter",
		      "no String to XtCallbackList converter registered.\n\tCall XmtRegisterCallbackConverter().");
	return;
    }

    from.addr = (XPointer) s;
    from.size = strlen(s)+1;
    to.addr = (XPointer) list_return;
    to.size = sizeof(XtCallbackList);
    XtCallConverter(XtDisplayOfObject(w), _XmtCallbackConverter, NULL, 0,
		    &from, &to, ref_return);
}

#if NeedFunctionPrototypes
static void FreeCallbackList(Widget w, XtCacheRef ref)
#else
static void FreeCallbackList(w, ref)
Widget w;
XtCacheRef ref;
#endif
{
    XtCacheRef refs[2];

    if (ref == NULL) return;
    refs[0] = ref;
    refs[1] = NULL;
    XtAppReleaseCacheRefs(XtWidgetToApplicationContext(w), refs);
}

#if NeedFunctionPrototypes
static void CallCallbackList(Widget w, XtCallbackList list)
#else
static void CallCallbackList(w, list)
Widget w;
XtCallbackList list;
#endif
{
    for(; list->callback; list++)
	(*list->callback)(w, list->closure, NULL);
}

typedef struct {
    XrmQuark nameq;
    Widget *widgetp;
} WidgetName;

/* forward declaration */
#if NeedFunctionPrototypes
static void CreateChildren(Widget, XmtDialogInfo *, WidgetName *, Cardinal);
static Widget _XmtCreateChild(Widget, StringConst, XmtDialogInfo *,
			      WidgetName *, Cardinal);
#else
static void CreateChildren();
static Widget _XmtCreateChild();
#endif

typedef struct {
    String create;
    String children;
    String managed;
} CallbackStringsType;

static XtResource callback_resources[] = {
{XmtNxmtCreationCallback, XmtCXmtCreationCallback, XtRString,
     sizeof(String), XtOffsetOf(CallbackStringsType, create),
     XtRString, (XtPointer) NULL},
{XmtNxmtChildrenCreationCallback, XmtCXmtChildrenCreationCallback, XtRString,
     sizeof(String), XtOffsetOf(CallbackStringsType, children),
     XtRString, (XtPointer) NULL},
{XmtNxmtManagedCreationCallback, XmtCXmtManagedCreationCallback, XtRString,
     sizeof(String), XtOffsetOf(CallbackStringsType, managed),
     XtRString, (XtPointer) NULL},
};    

static XtResource requires_resource[] = {
{XmtNxmtRequires, XmtCXmtRequires, XtRString,
     sizeof(String), 0, XtRString, NULL}
};

#if NeedFunctionPrototypes
static Widget CreateChild(Widget parent, StringConst name, XmtWidgetType *type,
                          String template, Boolean managed,
                          String *styles, Cardinal num_styles, Boolean load,
                          XmtDialogInfo *dialog_info,
                          WidgetName *names, Cardinal num_names)
#else
static Widget CreateChild(parent, name, type, template, managed,
                          styles, num_styles, load, dialog_info,
                          names, num_names)
Widget parent;
StringConst name;
XmtWidgetType *type;
String template;
Boolean managed;
String *styles;
Cardinal num_styles;
Boolean load;
XmtDialogInfo *dialog_info;
WidgetName *names;
Cardinal num_names;
#endif
{
    Widget w = NULL;
    String requires_string;
    CallbackStringsType callback_strings;
    XrmQuark nameq;
    XtCallbackList create_callback_list = NULL;
    XtCallbackList children_callback_list = NULL;
    XtCallbackList managed_callback_list = NULL;
    XtCacheRef create_callback_ref;
    XtCacheRef children_callback_ref;
    XtCacheRef managed_callback_ref;
    int i;
    
    /* first, load any required files, unless already loaded */
    if (load) {
	XtGetSubresources(parent, (XtPointer)&requires_string,
			  (String)name, (String)name,
			  requires_resource, (Cardinal)1,
			  NULL, (Cardinal) 0);
	if (requires_string) XmtLoadResourceFileList(parent, requires_string);
    }

    /* apply any styles that were specified */
    if (num_styles)
	for(i=0; i < num_styles; i++)
	    XmtTemplateInstantiate(parent, name, styles[i], NULL, 0);

    /* now create the widget */
    if (type)
	w = XmtCreateWidgetType(name, type, parent, NULL, 0);
    else if (template) {
	/*
	 * if the widget is a template type, instantiate the template,
	 * and then start the creation process over to read the xmtType
	 * resource.  Note that we recurse and then return immediately.
	 * If we don't return the callbacks will be called twice and
	 * the children created twice.
	 */
	XmtTemplateInstantiate(parent, name, template, NULL, 0);
	w = _XmtCreateChild(parent, name, dialog_info, names, num_names);
	return w;
    }

    /*
     * if we don't have a widget now, something went wrong.
     * Assume that someone else printed a warning message, and return.
     */
    if (w == NULL) return NULL;

    /*
     * see if the widget matches any in the names list, and if so,
     * store the widget pointer at the given address
     */
    nameq = XrmStringToQuark(name);
    for(i=0; i < num_names; i++) {
	if (nameq == names[i].nameq) {
	    *names[i].widgetp = w;
	    break;
	}
    }

    /*
     * A widget has just been created.  Next we:
     * (0) fetch and convert the creation callback resources
     * (1) invoke the xmtCreationCallback list
     * (2) call CreateChildren to create the widget's children (reg. or popup)
     * (2a) invoke the xmtChildrenCreationCallback list
     * (3) manage it, unless a shell or unmanaged
     * (3a) invoke the xmtManagedCreationCallback list
     * (4) free any creation callbacks.
     */

    /* (0) fetch and convert the creation callback resources */
    XtGetApplicationResources(w, (XtPointer)&callback_strings,
			      callback_resources, XtNumber(callback_resources),
			      NULL, (Cardinal)0);
    if (callback_strings.create)
	ConvertCallbackList(w, callback_strings.create,
			    &create_callback_list, &create_callback_ref);
    if (callback_strings.children)
	ConvertCallbackList(w, callback_strings.children,
			    &children_callback_list, &children_callback_ref);
    if (callback_strings.managed)
	ConvertCallbackList(w, callback_strings.managed,
			    &managed_callback_list, &managed_callback_ref);
    
    /* (1) invoke the xmtCreationCallback list */
    if (create_callback_list) CallCallbackList(w, create_callback_list);

    /* (2) call CreateChildren, to create regular or popup children */
    CreateChildren(w, dialog_info, names, num_names);
    
    /* (2a) invoke the xmtChildrenCreationCallback list */
    if (children_callback_list) CallCallbackList(w,children_callback_list);
    
    /* (3) manage it, unless a shell or unmanaged */
    if (managed && !XtIsShell(w)) {
	XtManageChild(w);
	/* (3a) invoke the xmtManagedCreationCallback list */
	if (managed_callback_list) CallCallbackList(w, managed_callback_list);
    }
   
    /* (4) free any creation callbacks. */
    if (create_callback_list) FreeCallbackList(w, create_callback_ref);
    if (children_callback_list) FreeCallbackList(w, children_callback_ref);
    if (managed_callback_list) FreeCallbackList(w, managed_callback_ref);
    
    return w;
}

static String keywords[] = {
    "managed",
    "unmanaged",
};

#define MANAGED   0
#define UNMANAGED 1

#define MAX_TEMPLATE_ARGS 10

#if NeedFunctionPrototypes
static Boolean ParseType(Widget w, StringConst name, XmtLexer l,
			 XmtWidgetType **type, String *template, 
			 Boolean *managed,
			 String **styles, Cardinal *num_styles)
#else
static Boolean ParseType(w, name, l, type, template, managed,
			 styles, num_styles)
Widget w;
StringConst name;
XmtLexer l;
XmtWidgetType **type;
String *template;
Boolean *managed;
String **styles;
Cardinal *num_styles;
#endif
{
    XmtLexerToken tok;
    int max_styles = 0;
    String style;
    String args[MAX_TEMPLATE_ARGS];
    Cardinal num_args;
    Cardinal expected_args;
    String template_name;
    int i;
    
    /* set defaults and parse any modifiers or styles */
    *managed = True;
    *num_styles = 0;
    *styles = NULL;
    while (1) {
	tok = XmtLexerGetToken(l);
	if (tok == XmtLexerKeyword) {
	    switch(XmtLexerKeyValue(l)) {
	    case MANAGED:
		*managed = True;
		break;
	    case UNMANAGED:
		*managed = False;
		break;
	    }
	    XmtLexerConsumeToken(l);
	}
	else if ((tok == XmtLexerIdent) &&
		 ((style = XmtLookupStyle(w, XmtLexerStrValue(l))) != NULL)) {
	    if (*num_styles == max_styles) {
		max_styles += 4;
		*styles = (String *)XtRealloc((char *)*styles,
					      max_styles * sizeof(String));
	    }
	    template_name = XmtLexerStrValue(l);
	    XmtLexerConsumeToken(l);
	    XmtLexerGetArgList(l, args, MAX_TEMPLATE_ARGS, &num_args);
	    style = XmtTemplateSubstituteArgs(style, args, num_args,
					      &expected_args);
	    (*styles)[*num_styles] = style;
	    *num_styles += 1;
	    if (num_args > expected_args)
		XmtWarningMsg("XmtParseWidgetType", "styleArgs",
			      "widget '%s':\n\tstyle '%s' expects up to %d args; %d passed.",
			      name, template_name, expected_args, num_args);
	    for(i=0; i < num_args; i++) XtFree(args[i]);
	    XtFree(template_name);
	}
	else
	    break;
    }
    
    /* now we expect a widget type or template */
    *type = NULL;
    *template = NULL;
    if (XmtLexerGetToken(l) != XmtLexerIdent) {
	XmtWarningMsg("XmtParseWidgetType", "syntax",
		      "syntax error in xmtChildren or xmtType resource\n\tof widget '%s'.",
		      name);
	return False;
    }
    *type = XmtLookupWidgetType(XmtLexerStrValue(l));
    if (!*type) {
	*template = XmtLookupTemplate(w, XmtLexerStrValue(l));
	if (*template) {
	    template_name = XmtLexerStrValue(l);
	    XmtLexerConsumeToken(l);
	    XmtLexerGetArgList(l, args, MAX_TEMPLATE_ARGS, &num_args);
	    *template = XmtTemplateSubstituteArgs(*template, args, num_args,
						  &expected_args);
	    if (num_args > expected_args)
		XmtWarningMsg("XmtParseWidgetType", "templateArgs",
			      "widget '%s':\n\ttemplate '%s' expects up to %d args; %d passed.",
			      name, template_name, expected_args, num_args);
	    for(i=0; i < num_args; i++) XtFree(args[i]);
	    XtFree(template_name);
	}
    }

    if (!*type && !*template)
	XmtWarningMsg("XmtParseWidgetType", "type",
		      "unknown widget type %s\n\tin xmtChildren or xmtType resource of widget %s.",
		      XmtLexerStrValue(l), name);
    if (!*template) {
	XtFree(XmtLexerStrValue(l));
	XmtLexerConsumeToken(l);
    }

    if (*type || *template) return True;
    else {
	if (*num_styles) {
	    for(i=0; i < *num_styles; i++) XtFree(*styles[i]);
	    XtFree((char *)*styles);
	    *styles = NULL;
	    *num_styles = 0;
	}
	return False;
    }
}    

#if NeedFunctionPrototypes
static void SetDialogInfo(XmtDialogInfo *info, String resource,
			  Widget w, XmtWidgetType *type)
#else
static void SetDialogInfo(info, resource, w, type)
XmtDialogInfo *info;
String resource;
Widget w;
XmtWidgetType *type;
#endif
{
    int i;
    XrmQuark resourceq = XrmStringToQuark(resource);

    if ((type->get_value_proc == NULL) && (type->set_value_proc == NULL)) {
	XmtWarningMsg("XmtCreateChildren", "notype",
		      "Can't associate resource '%s' with child '%s';\n\twidget class '%s' has no get value nor set value procedure.",
		      resource, XtName(w), type->name);
	return;
    }

    for(i = 0; i < info->num_resources; i++)
	if (info->resources[i].resource_name == (String)resourceq) break;

    if (i < info->num_resources) {
	info->widgets[i] = w;
	info->types[i] = type;
    }
    else 
	XmtWarningMsg("XmtCreateChildren", "noname",
		      "Child '%s': resource name '%s' not found.",
		      XtName(w), resource);
}

#if NeedFunctionPrototypes
static void CreateWidgetList(Widget parent, XmtLexer l,
			     XmtDialogInfo *dialog_info,
			     WidgetName *names, Cardinal num_names)
#else
static void CreateWidgetList(parent, l, dialog_info, names, num_names)
Widget parent;
XmtLexer l;
XmtDialogInfo *dialog_info;
WidgetName *names;
Cardinal num_names;
#endif
{
    Boolean managed;
    XmtWidgetType *type;
    String template;
    String *styles;
    Cardinal num_styles;
    XmtLexerToken tok;
    Widget child;
    int i;

    if (ParseType(parent, XtName(parent), l, &type, &template,
		  &managed, &styles, &num_styles) == False)
	goto error;

    /* now read a list of names, and create them */
    /* grammar:  id [= name] (',' id[= name])* ';' */
    while (1) {
	if (XmtLexerGetToken(l) != XmtLexerIdent) goto syntax;
	child = CreateChild(parent, XmtLexerStrValue(l), type, template,
			    managed, styles, num_styles, True, dialog_info,
			    names, num_names);
	XtFree(XmtLexerStrValue(l));
	tok = XmtLexerNextToken(l);
	if (tok == XmtLexerEqual) {
	    tok = XmtLexerNextToken(l);
	    if (tok != XmtLexerIdent) goto syntax;
	    if (!type) {
		XmtWarningMsg("XmtCreateChildren", "resource",
			      "child '%s' cannot have a resource name '%s':\n\tchild's type is a template, not a registered widget type.",
			      XtName(child), XmtLexerStrValue(l));
	    }
	    else if (dialog_info)
		SetDialogInfo(dialog_info, XmtLexerStrValue(l), child, type);
	    else
		XmtWarningMsg("XmtCreateChildren", "resource",
			      "child '%s' cannot have a resource name '%s':\n\tthere is no resource list associated with this widget tree.",
			      XtName(child), XmtLexerStrValue(l));
		
	    XtFree(XmtLexerStrValue(l));
	    tok = XmtLexerNextToken(l);
	}
	if (tok == XmtLexerSemicolon) break;
	else if (tok != XmtLexerComma) goto syntax;
	XmtLexerConsumeToken(l);
    }
    XmtLexerConsumeToken(l); /* skip the semicolon */

    if (template) XtFree(template);
    if (num_styles) {
	for(i=0; i < num_styles; i++) XtFree(styles[i]);
	XtFree((char *)styles);
    }
    
    /* eat the final semicolon, and we're done */
    XmtLexerConsumeToken(l);
    return;
    
 syntax:
    XmtWarningMsg("XmtCreateChildren", "syntax",
		  "syntax error in xmtChildren resource\n\t of widget %s",
		  XtName(parent));
 error: /* read till semicolon */
    while(((tok = XmtLexerNextToken(l)) != XmtLexerSemicolon) &&
	  (tok != XmtLexerEndOfString));
    XmtLexerConsumeToken(l);
}


static XtResource children_resource[] = {
{XmtNxmtChildren, XmtCXmtChildren, XtRString,
     sizeof(String), 0, XtRString, NULL}
};

#if NeedFunctionPrototypes
static void CreateChildren(Widget parent, XmtDialogInfo *dialog_info,
                           WidgetName *names, Cardinal num_names)
#else
static void CreateChildren(parent, dialog_info, names, num_names)
Widget parent;
XmtDialogInfo *dialog_info;
WidgetName *names;
Cardinal num_names;
#endif
{
    String children_string;
    XmtLexer lexer;

    /* fetch the value of the xmtChildren resource */
    XtGetApplicationResources(parent, (XtPointer)&children_string,
			      children_resource, (Cardinal) 1,
			      NULL, (Cardinal)0);

    /* XXX should we issue a warning here? */
    if (children_string == NULL) return;

    /*
     * get ready to parse the string.
     * we copy the string because the process of parsing it can cause
     * new resource files to be loaded, which could cause it to be freed
     * from underneath us.
     */

    children_string = XtNewString(children_string);
    lexer = XmtLexerCreate(keywords, XtNumber(keywords));
    XmtLexerInit(lexer, children_string);
    
    while(XmtLexerGetToken(lexer) != XmtLexerEndOfString)
	CreateWidgetList(parent, lexer, dialog_info, names, num_names);

    XmtLexerDestroy(lexer);
    XtFree(children_string);
}


#if NeedFunctionPrototypes
static void _XmtCreateChildren(Widget parent,
			       WidgetName *names, Cardinal num_names)
#else
static void _XmtCreateChildren(parent, names, num_names)
Widget parent;
WidgetName *names;
Cardinal num_names;
#endif
{
    XmtDialogInfo *dialog_info;
    String requires_string;

    /* see if there is a resource list associated with this parent */
    dialog_info = _XmtDialogLookupDialogInfo(parent);

    /* first, load any required files */
    XtGetApplicationResources(parent, (XtPointer)&requires_string,
			      requires_resource, (Cardinal)1,
			      NULL, (Cardinal) 0);
    if (requires_string) XmtLoadResourceFileList(parent, requires_string);

    /* do the rest */
    CreateChildren(parent, dialog_info, names, num_names);
}

#if NeedFunctionPrototypes
void XmtCreateChildren(Widget parent)
#else
void XmtCreateChildren(parent)
Widget parent;
#endif
{
    _XmtCreateChildren(parent, NULL, 0);
}

#if NeedFunctionPrototypes
static void GetWidgetNames(va_list var,
			   WidgetName **names_return, Cardinal *num_return)
#else
static void GetWidgetNames(var, names_return, num_return)
va_list var;
WidgetName **names_return;
Cardinal *num_return;
#endif
{
    WidgetName *names;
    String name;
    Cardinal num, max;

    num = max = 0;
    names = NULL;
    while((name = va_arg(var, String)) != NULL) {
	if (num == max) {
	    max += 10;
	    names = (WidgetName *)XtRealloc((char *)names,
					    max * sizeof(WidgetName));
	}
	names[num].nameq = XrmStringToQuark(name);
	names[num].widgetp = va_arg(var, Widget *);
	/* initialize *widgetp to NULL in case it is not found */
	*names[num].widgetp = NULL;
	num++;
    }

    *names_return = names;
    *num_return = num;
}

#if NeedVarargsPrototypes
void XmtCreateQueryChildren(Widget parent, ...)
#else
void XmtCreateQueryChildren(parent, va_alist)
Widget parent;
va_dcl
#endif    
{
    va_list var;
    WidgetName *names;
    Cardinal num_names;

    Va_start(var, parent);
    GetWidgetNames(var, &names, &num_names);
    va_end(var);
    _XmtCreateChildren(parent, names, num_names);
    XtFree((char *)names);
}

static XtResource type_resource[] = {
{XmtNxmtType, XmtCXmtType, XtRString,
     sizeof(String), 0, XtRString, NULL}
};


#if NeedFunctionPrototypes
static Widget _XmtCreateChild(Widget parent, StringConst name,
                              XmtDialogInfo *dialog_info,
                              WidgetName *names, Cardinal num_names)
#else
static Widget _XmtCreateChild(parent, name, dialog_info, names, num_names)
Widget parent;
StringConst name;
XmtDialogInfo *dialog_info;
WidgetName *names;
Cardinal num_names;
#endif
{
    String requires_string, type_string;
    XmtLexer lexer;
    XmtWidgetType *type;
    String template;
    Boolean managed;
    String *styles;
    Cardinal num_styles;
    Widget w;
    int i;

    /* first, load any required files XXX ?*/
    XtGetSubresources(parent, (XtPointer)&requires_string,
		      (String)name, (String)name,
		      requires_resource, (Cardinal)1,
		      NULL, (Cardinal) 0);
    if (requires_string) XmtLoadResourceFileList(parent, requires_string);

    /* fetch the value of the xmtType resource */
    XtGetSubresources(parent, (XtPointer)&type_string,
		      (String)name, (String)name,
		      type_resource, (Cardinal) 1,
		      NULL, (Cardinal)0);

    if (type_string == NULL) {
	XmtWarningMsg("XmtCreateChild", "noType",
		      "widget '%s' has no xmtType resource.",
		      name);
	return NULL;
    }

    /* get ready to parse the string */
    lexer = XmtLexerCreate(keywords, XtNumber(keywords));
    XmtLexerInit(lexer, type_string);

    /*
     * if we parse it sucessfully, create the child.
     * Note that CreateChild won't re-load the required files.
     */
    if (ParseType(parent, name, lexer, &type, &template,
		  &managed, &styles, &num_styles)) {
	w = CreateChild(parent, name, type, template,
			managed, styles, num_styles, False,
			dialog_info, names, num_names);
	if (template) XtFree(template);
	if (num_styles) {
	    for(i=0; i < num_styles; i++) XtFree(styles[i]);
	    XtFree((char *)styles);
	}
    }
    else
	w = NULL;

    XmtLexerDestroy(lexer);
    return w;
}

#if NeedFunctionPrototypes
Widget XmtCreateChild(Widget parent, StringConst name)
#else
Widget XmtCreateChild(parent, name)
Widget parent;
StringConst name;
#endif
{
    return _XmtCreateChild(parent, name, NULL, NULL, 0);
}

#if NeedVarargsPrototypes
Widget XmtCreateQueryChild(Widget parent, StringConst name, ...)
#else
Widget XmtCreateQueryChild(parent, name, va_alist)
Widget parent;
StringConst name;
va_dcl
#endif    
{
    va_list var;
    WidgetName *names;
    Cardinal num_names;
    Widget child;

    Va_start(var, name);
    GetWidgetNames(var, &names, &num_names);
    va_end(var);
    child = _XmtCreateChild(parent, name, NULL, names, num_names);
    XtFree((char *)names);
    return child;
}
    

#if NeedFunctionPrototypes
static Widget _XmtBuildDialog(Widget parent, StringConst dialog_name,
			      XtResourceList resources, Cardinal num_resources,
			      WidgetName *names, Cardinal num_names)
#else
static Widget _XmtBuildDialog(parent, dialog_name, resources, num_resources,
			      names, num_names)
Widget parent;
StringConst dialog_name;
XtResourceList resources;
Cardinal num_resources;
WidgetName *names;
Cardinal num_names;
#endif
{
    Widget shell;
    Arg arg;

    /* create the dialog shell */
    /* XXX do I need/want allowShellResize?  What happens without? */
    XtSetArg(arg, XmNallowShellResize, True);
    shell = XmCreateDialogShell(parent, (String)dialog_name, &arg, 1);

    /* associate the resources with the shell */
    XmtDialogBindResourceList(shell, resources, num_resources);
    
    /* create the dialog child and its children */
    _XmtCreateChildren(shell, names, num_names);

    /* return the dialog child, if any, or warn and return NULL */
    if (((CompositeWidget)shell)->composite.num_children)
	return ((CompositeWidget)shell)->composite.children[0];
    else
	XmtWarningMsg("XmtBuildDialog", "nochild",
		      "No xmtChildren resource for shell widget '%s'",
		      dialog_name);
    return NULL;
}

#if NeedFunctionPrototypes
Widget XmtBuildDialog(Widget parent, StringConst dialog_name,
		      XtResourceList resources, Cardinal num_resources)
#else
Widget XmtBuildDialog(parent, dialog_name, resources, num_resources)
Widget parent;
StringConst dialog_name;
XtResourceList resources;
Cardinal num_resources;
#endif
{
    return _XmtBuildDialog(parent, dialog_name,
			   resources, num_resources,
			   NULL, 0);
}


#if NeedVarargsPrototypes
Widget XmtBuildQueryDialog(Widget parent, StringConst dialog_name,
			   XtResourceList resources, Cardinal num_resources,
			   ...)
#else
Widget XmtBuildQueryDialog(parent, dialog_name,
			   resources, num_resources, va_alist)
Widget parent;
StringConst dialog_name;
XtResourceList resources;
Cardinal num_resources;
va_dcl
#endif
{
    va_list var;
    WidgetName *names;
    Cardinal num_names;
    Widget dialog;

    Va_start(var, num_resources);
    GetWidgetNames(var, &names, &num_names);
    va_end(var);
    dialog = _XmtBuildDialog(parent, dialog_name,
			     resources, num_resources,
			     names, num_names);
    XtFree((char *)names);
    return dialog;
}    

#if NeedVarargsPrototypes
Widget XmtBuildQueryToplevel(Widget parent, StringConst name,  ...)
#else
Widget XmtBuildQueryToplevel(parent, name, va_alist)
Widget parent;
StringConst name;
va_dcl
#endif    
{
    va_list var;
    WidgetName *names;
    Cardinal num_names;
    Widget shell;

    shell = XtCreatePopupShell((String)name, topLevelShellWidgetClass,
			       parent, NULL, 0);

    Va_start(var, name);
    GetWidgetNames(var, &names, &num_names);
    va_end(var);
    _XmtCreateChildren(shell, names, num_names);
    XtFree((char *)names);
    if (!((CompositeWidget)shell)->composite.num_children)
	XmtWarningMsg("XmtBuildToplevel", "nochild",
		      "No xmtChildren resource for shell widget '%s'",
		      name);
    return shell;
}

#if NeedFunctionPrototypes
Widget XmtBuildToplevel(Widget parent, StringConst name)
#else
Widget XmtBuildToplevel(parent, name)
Widget parent;
StringConst name;
#endif    
{
    return XmtBuildQueryToplevel(parent, name, NULL);
}

#if NeedVarargsPrototypes
Widget XmtBuildQueryApplication(StringConst appname, StringConst appclass,
				Display *display,
				ArgList args, Cardinal num_args, ...)
#else
Widget XmtBuildQueryApplication(appname, appclass, display, args, num_args,
				va_alist)
StringConst appname, appclass;
Display *display;
ArgList args;
Cardinal num_args;
va_dcl
#endif
{
    va_list var;
    WidgetName *names;
    Cardinal num_names;
    Widget rootshell;

    rootshell = XtAppCreateShell((String)appname, (String)appclass,
				 applicationShellWidgetClass,
				 display, args, num_args);
    Va_start(var, num_args);
    GetWidgetNames(var, &names, &num_names);
    va_end(var);
    _XmtCreateChildren(rootshell, names, num_names);
    XtFree((char *)names);

    if (!((CompositeWidget)rootshell)->composite.num_children)
	XmtWarningMsg("XmtBuildApplication", "nochild",
		      "No xmtChildren resource for root shell widget '%s'",
		      appname);

    XmtInitializeApplicationShell(rootshell, args, num_args);
    return rootshell;
}

#if NeedFunctionPrototypes
Widget XmtBuildApplication(StringConst appname, StringConst appclass,
			   Display *display,
			   ArgList args, Cardinal num_args)
#else
Widget XmtBuildApplication(appname, appclass, display, args, num_args)
StringConst appname, appclass;
Display *display;
ArgList args;
Cardinal num_args;
#endif
{
    return XmtBuildQueryApplication(appname, appclass, display,
				    args, num_args, NULL);
}
