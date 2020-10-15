/* 
 * Motif Tools Library, Version 3.1
 * $Id: StringLstCvt.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: StringLstCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <ctype.h>
#include <Xmt/Xmt.h>
#include <Xmt/ConvertersP.h>

#define skipblanks(s) while (isspace(*s)) s++

/*
 * Convert a string to a NULL-terminated array of strings.
 * Individual strings must be surrounded by double quotes, and
 * may be optionally comma-separated.  Newlines and whitespace
 * outside of double quotes are ignored.
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToStringList(Display *dpy,
				     XrmValue *args, Cardinal *num_args,
				     XrmValue *from, XrmValue *to,
				     XtPointer *converter_data)
#else
Boolean XmtConvertStringToStringList(dpy, args, num_args,
				     from, to, converter_data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *converter_data;
#endif
{
    char *s = (char *)from->addr;
    char *mark;
    int num_items = 0;
    int max_items = 6;
    String *items = (String *) XtMalloc(max_items * sizeof(String));

    skipblanks(s);
    for(;;) {
	if (*s == '\0') break;
	
	if (*s != '"') {
	    XtDisplayStringConversionWarning(dpy, (char *)from->addr,
					     XmtRStringList);
	    XmtWarningMsg("XmtConvertStringToStringList", "expected",
			  "open quote expected.");
	    XtFree((char *)items);
	    return False;
	}
	
	mark = ++s;
	while (*s && *s != '"') s++;
	
	if (*s == '\0') {
	    XtDisplayStringConversionWarning(dpy, (char *)from->addr,
					     XmtRStringList);
	    XmtWarningMsg("XmtConvertStringToStringList", "expected",
			  "close quote expected.");
	    XtFree((char *)items);
	    return False;
	}

	if (num_items == max_items) {
	    max_items *= 2;
	    items = (String *) XtRealloc((char *)items,
					 max_items * sizeof(String));
	}

	items[num_items] = (String) XtMalloc((s - mark) + 1);
	strncpy(items[num_items], mark, (s-mark));
	items[num_items][s-mark] = '\0';
	num_items++;

	s++;
	skipblanks(s);
	if (*s == ',') {
	    s++;
	    skipblanks(s);
	}
    }

    /* NULL-terminate the array */
    items = (String *) XtRealloc((char *)items, (num_items+1)*sizeof(String));
    items[num_items] = NULL;

    /* do the right thing: see ConvertersP.h */
    done(String *, items);
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void XmtDestroyStringList(XtAppContext app, XrmValue *to,
				 XtPointer converter_data,
				 XrmValue *args, Cardinal *num_args)
#else
static void XmtDestroyStringList(app, to, converter_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer converter_data;
XrmValue *args;
Cardinal *num_args;
#endif
{
    String *items = *(String **) to->addr;
    int i;

    for(i=0; items[i] != NULL; i++)
	XtFree(items[i]);
    XtFree((char *)items);
}

#if NeedFunctionPrototypes
void XmtRegisterStringListConverter(void)
#else
void XmtRegisterStringListConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	registered = True;
	XtSetTypeConverter(XtRString, XmtRStringList,
			   XmtConvertStringToStringList, NULL, 0,
			   XtCacheAll | XtCacheRefCount,
			   XmtDestroyStringList);
    }
}
