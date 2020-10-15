/* 
 * Motif Tools Library, Version 3.1
 * $Id: Symbols.c,v 1.2 2006-10-02 14:24:58 andre Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Symbols.c,v $
 * Revision 1.2  2006-10-02 14:24:58  andre
 * AA-2006-10-02-0: Enhancem:  o jgdi qlimit
 *                             o sge_error_verror()
 *                             o japi segv
 *                             o copyright comments
 *                  Review:    RH
 *
 * Revision 1.1.1.1  2001/07/18 11:06:03  root
 * Initial checkin.
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Symbols.h>
#include <Xmt/Hash.h>
#include <Xmt/QuarksP.h>
#include <X11/IntrinsicP.h>

#if NeedFunctionPrototypes
        void _XtCopyToArg(XtPointer, XtArgVal *, Cardinal);
        void _XtCopyFromArg(XtArgVal, XtPointer, Cardinal);
#else
        void _XtCopyToArg();
        void _XtCopyFromArg();
#endif

/*
 * values for the XmtSymbolRec mode field.
 */
#define XmtSymbolABSOLUTE   1
#define XmtSymbolIMMEDIATE  2
#define XmtSymbolRESOURCE   3

static XmtHashTable symbol_table = NULL;


#if NeedFunctionPrototypes
static void GetResourceTypeAndSize(Widget w, String resource_name,
				   XrmQuark *type, Cardinal *size)
#else
static void GetResourceTypeAndSize(w, resource_name, type, size)
Widget w;
String resource_name;
XrmQuark *type;
Cardinal *size;
#endif
{
    int i;
    Boolean found = False;
    WidgetClass c = XtClass(w);
    XtResourceList *list = (XtResourceList *) c->core_class.resources;
    XrmQuark name = XrmStringToQuark(resource_name);
    
    /* check normal resources first */
    /* Note that we assume knowledge of the compiled resource format */
    for(i=0; i < c->core_class.num_resources; i++) {
	if ((list[i] != NULL) &&
	    ((XrmName)list[i]->resource_name == name)) {
	    found = True;
	    break;
	}		
    }
    
    /* check constraint resources if parent is constraint */
    if (!found && XtParent(w) && XtIsConstraint(XtParent(w))) {
	ConstraintWidgetClass c =
	    (ConstraintWidgetClass) XtClass(XtParent(w));
	list = (XtResourceList *) c->constraint_class.resources;
	for(i = 0; i < c->constraint_class.num_resources; i++) {
	    if ((list[i] != NULL) &&
		((XrmName)list[i]->resource_name == name)) {
		found = True;
		break;
	    }
	}
    }

    if (found) {
	*type = (XrmQuark)list[i]->resource_type;
	*size = list[i]->resource_size;
    }
}

/*
 * s must be in static memory, or at least in memory that will never
 * be freed.  s->name must be the same.  For statically declared arrays
 * of symbols this is the usual case.  For dynamically created symbols,
 * the programmer must make a copy of the name in allocated memory.
 */
#if NeedFunctionPrototypes
void XmtRegisterSymbol(XmtSymbol s)
#else
void XmtRegisterSymbol(s)
XmtSymbol s;
#endif
{
    XrmName name;

    if (symbol_table == NULL)
	symbol_table = XmtHashTableCreate(3);

    if (s->addr == 0) {  /* symbol is an immediate value */
	/*
	 * if size > sizeof(XtPointer), an immediate value won't
	 * actually fit, so issue error msg. and exit.
	 */
	if (s->size > sizeof(s->addr))
	    XmtErrorMsg("XmtSymbol", "size",
			"Symbol '%s' has size too big for immediate mode.",
			s->name);
	s->mode = XmtSymbolIMMEDIATE;
	s->type = (String)XrmStringToQuark(s->type);
    }
    else if (s->resource_name) {  /* symbol is a widget resource */
	GetResourceTypeAndSize((Widget)s->addr, s->resource_name,
			       (XrmQuark *) &s->type, &s->size);
	s->mode = XmtSymbolRESOURCE;
    }
    else {
	s->mode = XmtSymbolABSOLUTE;
	s->type = (String)XrmStringToQuark(s->type);
    }

    name = XrmPermStringToQuark(s->name);
    XmtHashTableStore(symbol_table, (XtPointer)name, (XtPointer)s);
}


/*
 * The symbol records and the string within must be permanently allocated
 */
#if NeedFunctionPrototypes
void XmtRegisterSymbols(XmtSymbolRec *syms, Cardinal num_syms)
#else
void XmtRegisterSymbols(syms, num_syms)
XmtSymbolRec *syms;
Cardinal num_syms;
#endif
{
    int i;
    for(i=0; i < num_syms; i++)	XmtRegisterSymbol(&syms[i]);
}

/*
 * All strings must be permanently allocated, and may not change
 */
#if NeedVarargsPrototypes
void XmtVaRegisterSymbols(StringConst name, ...)
#else
void XmtVaRegisterSymbols(name, va_alist)
StringConst name;
va_dcl
#endif
{
    va_list var;
    XmtSymbol sym;

    Va_start(var, name);
    
    while(name) {
	sym = (XmtSymbol) XtCalloc(1, sizeof(XmtSymbolRec));
	sym->name = (String) name;
	sym->type = va_arg(var, String);
	sym->size = va_arg(var, int);
	sym->addr = va_arg(var, XtPointer);
	XmtRegisterSymbol(sym);
	name = va_arg(var, String);
    }

    va_end(var);
}

#if NeedFunctionPrototypes
void XmtRegisterResourceList(XtPointer base,
			     XtResourceList resources, Cardinal num_resources)
#else
void XmtRegisterResourceList(base, resources, num_resources)
XtPointer base;
XtResourceList resources;
Cardinal num_resources;
#endif
{
    int i;
    XmtSymbolRec *syms = (XmtSymbolRec *)XtCalloc(num_resources,
						  sizeof(XmtSymbolRec));

    /*
     * This procedure knows about the Intrinsics compiled resource format.
     * This is a little dangerous.
     */
    if (((int) resources[0].resource_offset) >= 0) { /* uncompiled */
	for(i=0; i < num_resources; i++) {
	    syms[i].name = resources[i].resource_name;
	    syms[i].type = resources[i].resource_type;
	    syms[i].size = resources[i].resource_size;
	    syms[i].addr = (XtPointer) 
		(resources[i].resource_offset + (char *)base);
	}
    }
    else {  /* compiled */
#define tostring(s) XrmQuarkToString((XrmQuark)s)	
	for(i=0; i < num_resources; i++) {
	    syms[i].name = tostring(resources[i].resource_name);
	    syms[i].type = tostring(resources[i].resource_type);
	    syms[i].size = resources[i].resource_size;
	    syms[i].addr = (XtPointer)
		(-resources[i].resource_offset - 1 + (char *)base);
	}
#undef tostring
    }

    XmtRegisterSymbols(syms, num_resources);
}

#if NeedFunctionPrototypes
void XmtRegisterWidgetResource(StringConst name, Widget w, StringConst res)
#else
void XmtRegisterWidgetResource(name, w, res)
StringConst name;
Widget w;
StringConst res;
#endif
{
    XmtSymbol s = (XmtSymbol) XtCalloc(1, sizeof(XmtSymbolRec));

    s->name = (String) name;
    s->addr = (XtPointer) w;
    s->resource_name = (String) res;
    XmtRegisterSymbol(s);
}


#if NeedFunctionPrototypes
XmtSymbol XmtLookupSymbol(StringConst name)
#else
XmtSymbol XmtLookupSymbol(name)
StringConst name;
#endif
{
    XmtSymbol s;
    XrmQuark q = XrmStringToQuark(name);

    if (!symbol_table) return NULL;
    if (XmtHashTableLookup(symbol_table,(XtPointer)q, (XtPointer *)&s))
	return s;
    else
	return NULL;
}

#if NeedFunctionPrototypes
void XmtSymbolSetValue(XmtSymbol s, XtArgVal value)
#else
void XmtSymbolSetValue(s, value)
XmtSymbol s;
XtArgVal value;
#endif
{
    XmtSymbolCallbackRec *p;
    int index;

    /*
     * Special case for XmtRBuffer type symbols.
     * we copy the NULL-terminated string, and warn if it is too
     * large for the buffer.
     * XXX
     * we should do sanity checking on buffer symbols.  They can only
     * have ABSOLUTE addressing mode.
     */
    if (s->type == (String) XmtQBuffer) {
	int len = strlen((char *)value);
	if (len < s->size)
	    strcpy((char *)s->addr, (char *) value);
	else {
	    strncpy((char *)s->addr, (char *)value, s->size-1);
	    ((char *)s->addr)[s->size] = '\0';
	    XmtWarningMsg("XmtSymbolSetValue", "overflow",
		       "Symbol '%s': buffer overflow.\n\t%d characters lost.",
			  s->name, len - s->size +1);
	}
    }
    else {  /* anything other than buffer symbols */
	/*
	 * if this is an enumerated symbol, then the new value is the
	 * index into the supplied array of values, not the value itself.
	 */
	if (s->num_values > 0) {
	    index = (unsigned int) value;
	    if ((index >= s->num_values) || (index < 0)) {
		XmtWarningMsg("XmtSymbolSetValue", "badValue",
			      "symbol '%s' has no enumerated value for %d.",
			      s->name, index);
		return;
	    }
	    s->index = (unsigned short)index;
	    value = (XtArgVal)s->values[index];
	}
	
	/* XXX
	 * machine dependencies?  Does this work on a Sparc?
	 */
	switch(s->mode) {
	case XmtSymbolABSOLUTE:
	    _XtCopyFromArg(value, s->addr, s->size);
	    break;
	case XmtSymbolIMMEDIATE:
	    _XtCopyFromArg(value, &s->addr, s->size);
	    break;
	case XmtSymbolRESOURCE:
	    XtVaSetValues((Widget)s->addr, s->resource_name, value, NULL);
	    break;
	}
    }

    /* 
     * call notify procs.
     */
    for(p = s->callbacks; p; p = p->next)
	(*p->proc)(s, p->closure, value);
}

#if NeedFunctionPrototypes
Boolean XmtSymbolSetTypedValue(Widget w, XmtSymbol s, String type,
			       XtArgVal value, Cardinal size)
#else
Boolean XmtSymbolSetTypedValue(w, s, type, value, size)
Widget w;
XmtSymbol s;
String type;
XtArgVal value;
Cardinal size;
#endif
{
    XrmQuark typeq = XrmStringToQuark(type);
    XrmValue from, to;
    XtArgVal converted_value;
    Boolean status;

    /*
     * if the types match, just call XmtSymbolSetValue().
     * if the symbol is a buffer, and the value a string, that is a match.
     */
    if ((typeq == (XrmQuark) s->type) ||
	((typeq == XmtQString) && (s->type == (String) XmtQBuffer))) {
	XmtSymbolSetValue(s, value);
	return True;
    }

    /*
     * otherwise attempt to convert the value.
     */
    from.size = size;
    if ((size > sizeof(XtArgVal)) ||
	(typeq == XmtQString) || (typeq == XmtQBuffer))
	from.addr = (XPointer) value;
    else
	from.addr = (XPointer) &value;
    /* let the converter return a value of the right type. */
    to.addr = NULL;  
    to.size = s->size;
    status = XtConvertAndStore(w, type, &from,
			       XrmQuarkToString((XrmQuark)s->type), &to);

    if (status) {
	converted_value = 0;  /* make func below store directly into us */
	_XtCopyToArg(to.addr, &converted_value, to.size);
	XmtSymbolSetValue(s, converted_value);
    }
    else {
	XmtWarningMsg("XmtSymbolSetTypedValue", "convert",
		      "Couldn't set symbol '%s'.", s->name);
    }

    return status;
}

#if NeedFunctionPrototypes
void XmtSymbolGetValue(XmtSymbol s, XtArgVal *valuep)
#else
void XmtSymbolGetValue(s, valuep)
XmtSymbol s;
XtArgVal *valuep;
#endif
{

    /*
     * Special case for XmtRBuffer type symbols.
     * Just return the address.
     */
    if (s->type == (String) XmtQBuffer) {
	*(char **)valuep = s->addr;
	return;
    }
    
    switch(s->mode) {
    case XmtSymbolABSOLUTE:
	_XtCopyToArg(s->addr, (XtArgVal *) &valuep, s->size);
	break;
    case XmtSymbolIMMEDIATE:
	_XtCopyToArg(&s->addr, (XtArgVal *) &valuep, s->size);
	break;
    case XmtSymbolRESOURCE:
	XtVaGetValues((Widget)s->addr, s->resource_name, valuep, NULL);
	break;
    }
}

#if NeedFunctionPrototypes
int XmtSymbolGetScalarValue(XmtSymbol s)
#else
int XmtSymbolGetScalarValue(s)
XmtSymbol s;
#endif
{
    if (s->num_values > 0) return (int)s->index;
    else 
	XmtWarningMsg("XmtSymbolGetScalarValue", "badSymbol",
		      "symbol '%s' is not an enumerated value.",
		      s->name);
    return -1;
}
	    

#if NeedFunctionPrototypes
void XmtSymbolAddCallback(XmtSymbol s, XmtSymbolCallbackProc proc,
			  XtPointer closure)
#else
void XmtSymbolAddCallback(s, proc, closure)
XmtSymbol s;
XmtSymbolCallbackProc proc;
XtPointer closure;
#endif
{
    XmtSymbolCallbackRec *cb = XtNew(XmtSymbolCallbackRec);
    XmtSymbolCallbackRec *p;

    cb->proc = proc;
    cb->closure = closure;
    cb->next = NULL;
    if (s->callbacks == NULL) s->callbacks = cb;
    else {
	for(p = s->callbacks; p->next; p = p->next);
	p->next = cb;
    }
}

#if NeedFunctionPrototypes
void XmtSymbolRemoveCallback(XmtSymbol s, XmtSymbolCallbackProc proc,
			     XtPointer closure)
#else
void XmtSymbolRemoveCallback(s, proc, closure)
XmtSymbol s;
XmtSymbolCallbackProc proc;
XtPointer closure;
#endif
{
    XmtSymbolCallbackRec *p, *q;

    for(q=NULL, p = s->callbacks; p; q=p, p = p->next)
	if ((p->proc == proc) && (p->closure == closure)) break;

    if (p) {
	if (!q) s->callbacks = p->next;
	else q->next = p->next;
	XtFree((char *)p);
    }
}

#if NeedFunctionPrototypes
String XmtSymbolName(XmtSymbol s)
#else
String XmtSymbolName(s)
XmtSymbol s;
#endif
{
    return s->name;
}

#if NeedFunctionPrototypes
int XmtSymbolSize(XmtSymbol s)
#else
int XmtSymbolSize(s)
XmtSymbol s;
#endif
{
    return s->size;
}
    
#if NeedFunctionPrototypes
XrmQuark XmtSymbolTypeQuark(XmtSymbol s)
#else
XrmQuark XmtSymbolTypeQuark(s)
XmtSymbol s;
#endif
{
    return (XrmQuark) s->type;
}

#if NeedFunctionPrototypes
String XmtSymbolType(XmtSymbol s)
#else
String XmtSymbolType(s)
XmtSymbol s;
#endif
{
    return XrmQuarkToString((XrmQuark)s->type);
}

#if NeedFunctionPrototypes
Boolean XmtSymbolIsEnumerated(XmtSymbol s)
#else
Boolean XmtSymbolIsEnumerated(s)
XmtSymbol s;
#endif
{
    return (s->num_values > 0);
}
