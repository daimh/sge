/* 
 * Motif Tools Library, Version 3.1
 * $Id: WidgetType.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: WidgetType.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/XmtP.h>
#include <Xmt/WidgetType.h>
#include <Xmt/Hash.h>

static XmtHashTable widget_type_table = NULL;

/* requires a permanently allocated string */
#if NeedFunctionPrototypes
void XmtRegisterWidgetClass(StringConst name, WidgetClass wclass)
#else
void XmtRegisterWidgetClass(name, wclass)
StringConst name;
WidgetClass wclass;
#endif
{
    XmtWidgetType *t = XtNew(XmtWidgetType);
    
    if (widget_type_table == NULL) widget_type_table = XmtHashTableCreate(5);
    
    t->name = (String) name;
    t->wclass = wclass;
    t->constructor = NULL;
    t->set_value_proc = NULL;
    t->get_value_proc = NULL;
    t->popup = False;
    
    XmtHashTableStore(widget_type_table,
		      (XtPointer)XrmPermStringToQuark(name),
		      (XtPointer)t);
}

/* requires a permanently allocated string */
#if NeedFunctionPrototypes
void XmtRegisterWidgetConstructor(StringConst name,
				  XmtWidgetConstructor constructor)
#else
void XmtRegisterWidgetConstructor(name, constructor)
StringConst name;
XmtWidgetConstructor constructor;
#endif
{
    XmtWidgetType *t = XtNew(XmtWidgetType);

    if (widget_type_table == NULL) widget_type_table = XmtHashTableCreate(5);

    t->name = (String) name;
    t->wclass = NULL;
    t->constructor = constructor;
    t->set_value_proc = NULL;
    t->get_value_proc = NULL;
    t->popup = False;

    XmtHashTableStore(widget_type_table,
		      (XtPointer)XrmPermStringToQuark(name),
		      (XtPointer)t);
}

/* requires a permanently allocated string */
#if NeedFunctionPrototypes
void XmtRegisterPopupClass(StringConst name, WidgetClass wclass)
#else
void XmtRegisterPopupClass(name, wclass)
StringConst name;
WidgetClass wclass;
#endif
{
    XmtWidgetType *t = XtNew(XmtWidgetType);
    
    if (widget_type_table == NULL) widget_type_table = XmtHashTableCreate(5);
    
    t->name = (String) name;
    t->wclass = wclass;
    t->constructor = NULL;
    t->set_value_proc = NULL;
    t->get_value_proc = NULL;
    t->popup = True;
    
    XmtHashTableStore(widget_type_table,
		      (XtPointer)XrmPermStringToQuark(name),
		      (XtPointer)t);
}

/* requires a permanently allocated string */
#if NeedFunctionPrototypes
void XmtRegisterPopupConstructor(StringConst name,
				 XmtWidgetConstructor constructor)
#else
void XmtRegisterPopupConstructor(name, constructor)
StringConst name;
XmtWidgetConstructor constructor;
#endif
{
    XmtWidgetType *t = XtNew(XmtWidgetType);

    if (widget_type_table == NULL) widget_type_table = XmtHashTableCreate(5);

    t->name = (String) name;
    t->wclass = NULL;
    t->constructor = constructor;
    t->set_value_proc = NULL;
    t->get_value_proc = NULL;
    t->popup = True;

    XmtHashTableStore(widget_type_table,
		      (XtPointer)XrmPermStringToQuark(name),
		      (XtPointer)t);
}


#if NeedFunctionPrototypes
void XmtRegisterWidgetTypes(XmtWidgetType *types, Cardinal num_types)
#else
void XmtRegisterWidgetTypes(types, num_types)
XmtWidgetType *types;
Cardinal num_types;
#endif
{
    int i;

    if (widget_type_table == NULL) widget_type_table = XmtHashTableCreate(5);

    for(i=0; i < num_types; i++) {
	XmtHashTableStore(widget_type_table,
			  (XtPointer)XrmPermStringToQuark(types[i].name),
			  (XtPointer) &types[i]);
    }
}

#if NeedVarargsPrototypes
void XmtVaRegisterWidgetClasses(StringConst name, WidgetClass wclass, ...)
#else
void XmtVaRegisterWidgetClasses(name, wclass, va_alist)
String name;
WidgetClass wclass;
va_dcl
#endif
{
    va_list var;

    XmtRegisterWidgetClass(name, wclass);
    Va_start(var, wclass);
    while((name = va_arg(var, StringConst)) != NULL) {
	wclass = va_arg(var, WidgetClass);
	XmtRegisterWidgetClass(name, wclass);
    }
    va_end(var);
}

#if NeedVarargsPrototypes
void XmtVaRegisterWidgetConstructors(StringConst name,
				     XmtWidgetConstructor constructor, ...)
#else
void XmtVaRegisterWidgetConstructors(name, constructor, va_alist)
StringConst name;
XmtWidgetConstructor constructor;
va_dcl
#endif
{
    va_list var;

    Va_start(var, constructor);
    while(name != NULL) {
	XmtRegisterWidgetConstructor(name, constructor);
	name = va_arg(var, StringConst);
	if (name) constructor = va_arg(var, XmtWidgetConstructor);
    }
    va_end(var);
}

#if NeedFunctionPrototypes
XmtWidgetType *XmtLookupWidgetType(StringConst name)
#else
XmtWidgetType *XmtLookupWidgetType(name)
StringConst name;
#endif
{
    XmtWidgetType *type;

    if (widget_type_table == NULL) return NULL;
    
    if (XmtHashTableLookup(widget_type_table,
			   (XtPointer) XrmStringToQuark(name),
			   (XtPointer *) &type))
	return type;
    else
	return NULL;
}

#if NeedFunctionPrototypes
Widget XmtCreateWidgetType(StringConst name, XmtWidgetType *type,
			   Widget parent, ArgList args, Cardinal num_args)
#else
Widget XmtCreateWidgetType(name, type, parent, args, num_args)
StringConst name;
XmtWidgetType *type;
Widget parent;
ArgList args;
Cardinal num_args;
#endif
{
    if (!type) return NULL;

    if (!type->popup && !XtIsComposite(parent)) {
	XmtWarningMsg("XmtCreateWidgetType", "badParent",
		      "Can't create %s widget '%s'\n\tas a child of %s widget '%s'.\n\tThe parent is not a Composite widget.",
		      type->name, (String)name,
		      XtClass(parent)->core_class.class_name, XtName(parent));
	return NULL;
    }

    if (type->wclass) {
	if (type->popup)
	    return XtCreatePopupShell(name, type->wclass,parent,args,num_args);
	else
	    return XtCreateWidget(name, type->wclass, parent, args, num_args);
    }
    else if (type->constructor)
	return (*type->constructor)(parent, (String)name, args, num_args);

    return NULL;
}
