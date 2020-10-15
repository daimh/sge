/* 
 * Motif Tools Library, Version 3.1
 * $Id: Template.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Template.c,v $
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
#include <X11/IntrinsicP.h>
#include <Xmt/Template.h>

#if NeedFunctionPrototypes
void XmtTemplateInstantiate(Widget parent, StringConst name,
			    StringConst template_string,
			    String *args, Cardinal num_args)
#else
void XmtTemplateInstantiate(parent, name, template_string, args, num_args)
Widget parent;
StringConst name;
StringConst template_string;
String *args;
Cardinal num_args;
#endif
{
    XrmQuark quarks[50];
    XrmBinding bindings[50];
    XrmDatabase db;
    Widget w;
    int depth, i;
    register char *s, *t, *u;
    char *mark;
    String template;
    Cardinal expected_args;

    /* handle the args, if any */
    if (num_args) {
	template = XmtTemplateSubstituteArgs(template_string,
						 args, num_args,
						 &expected_args);
	/* if more args were passed than the template expects, then warn.
	 * note that we don't warn if too few are passed, because templates
	 * can be written that allow omitted args
	 */
	if (expected_args < num_args)
	    XmtWarningMsg("XmtTemplateInstantiate", "toomany",
			  "widget %s:\n\ttemplate or style expects up to %d args; %d passed.",
			  (String)name, expected_args, num_args);
	
    }
    else
	template = XtNewString(template_string);
    
    /* count how many ancestors there are up to but not including the root */
    for(depth = 0, w = parent; XtParent(w); depth++, w = XtParent(w));

    /* now put the names of all those ancestors in the quarks array */
    for(i = depth-1, w = parent; i >= 0; i--, w = XtParent(w)) {
	quarks[i] = w->core.xrm_name;
	bindings[i] = XrmBindLoosely;
    }

    /* now put the name of the child in the quarks array */
    quarks[depth] = XrmStringToQuark(name);
    bindings[depth] = XrmBindLoosely;
    depth++;

    /* get the xrm database we'll be inserting into */
    db = XmtDatabaseOfWidget(w);

    /*
     * Loop through the string, a line at a time.
     * For each line, scan for a colon, and convert everything before
     * the colon into quarks and bindings.  Then skip the colon and whitespace
     * and scan for a newline.  Then we insert the new line into the db
     * with XrmQPutResource.  
     */
    s = template;
    while(*s) {
	/* skip any leading white space on the line */
	while(isspace(*s)) s++;
	
	/* if a comment, scan to end of line and continue */
	if (*s == '!') {
	    while(*s && *s != '\n') s++;
	    if (*s == '\n') s++;
	    continue;
	}
	
	/*
	 * scan 'till colon.
	 * If we find newline or NUL instead, ignore the line and start over.
	 * Replace the colon with NUL, and use XrmStringToBindingQuarkList()
	 * to parse the l.h.s of the resource specification.
	 */
	for(t = s; *t && (*t != ':') && (*t != '\n'); t++);
	if (*t == '\0') {
	    s = t;
	    break;
	}
	if (*t == '\n') {
	    s = t+1;
	    continue;
	}
	*t = '\0';
	XrmStringToBindingQuarkList(s, &bindings[depth], &quarks[depth]);
	s = t+1;

	/*
	 * skip any leading white space in the r.h.s of the resource
	 */
	while(isspace(*s)) s++;

	/*
	 * read 'till newline.  Remember the beginning of the next line
	 * and back up over any trailing white space in this line.  
	 * We'll be inserting this line into the database with
	 * XrmQPutStringResource, which performs no interpretation on the
	 * string.  This differs from regular lines in a resource file which
	 * have \n and other sequences recognized specially.  So we have
	 * to perform this processing ourselves.  We only treat "\n"
	 * specially--by converting it to '\n'.  To fully emulate resource
	 * file processing, we'd have to handle \\n and \nnn as well.
	 * But this will do for the common case.  Use "\\n" in a template
	 * definition to get a newline when the template is instantiated.
	 * For example, to specify a 2-line XmString in a template use:
	 * two\\nlines.
	 */
	for(t = u = s; *t && (*t != '\n'); t++) {
	    if ((*t == '\\') && (*(t+1) == 'n')) {
		t++;
		*u++ = '\n';
	    }
	    else *u++ = *t;
	}
	mark = t;
	if (*mark != '\0') mark++;
	while(isspace(*(u-1))) u--;
	*u = '\0';

	/* place this string at its new location in the db */
	XrmQPutStringResource(&db, bindings, quarks, s);

	/* go to beginning of next line, and take it again from the top */
	s = mark;
    }
	    
    XtFree(template);
}


#if NeedFunctionPrototypes
String XmtTemplateSubstituteArgs(StringConst template,
				 String *args, Cardinal num_args,
				 Cardinal *expected_args)
#else
String XmtTemplateSubstituteArgs(template, args, num_args, expected_args)
StringConst template;
String *args;
Cardinal num_args;
Cardinal *expected_args;
#endif
{
    int argument_lengths[10];
    String new;
    _Xconst char *t;
    register char *s;
    int i, arg;

    /* figure out the length of each of the string arguments */
    for(i=0; i < 10; i++) {
	if (i < num_args) argument_lengths[i] = strlen(args[i]);
	else argument_lengths[i] = 0;
    }

    /*
     * figure out the length of the substituted string.
     */
    i = 0;
    for(t = template; *t; t++) {
	if (*t == '%') {
	    if (isdigit(*(t+1))) {
		i += argument_lengths[*(t+1) - '0'];
		t++;
	    }
	    else if (*(t+1) == '%') {
		i++;
		t++;
	    }
	    else
		i++;
	}
	else
	    i++;
    }

    /*
     * allocate enough space for the new string
     * and do the substitution.
     */
    new = XtMalloc(i+1);
    *expected_args = 0;
    for(t = template, s = new; *t; t++) {
	if (*t == '%') {
	    if (isdigit(*(t+1))) {
		arg = *(t+1) - '0';
		if (arg >= *expected_args) *expected_args = arg+1;
		if (arg < num_args)
		    strcpy(s, args[arg]);
		s += argument_lengths[arg];
		t++;
	    }
	    else if (*(t+1) == '%') {
		*s++ = '%';
		t++;
	    }
	}
	else
	    *s++ = *t;
    }
    *s = '\0';

    return new;
}

static XrmQuark templateq[3] = {NULLQUARK, NULLQUARK, NULLQUARK};
static XrmQuark styleq[3] = {NULLQUARK, NULLQUARK, NULLQUARK};
static XrmBinding templatebindings[3] =
    {XrmBindTightly, XrmBindTightly, XrmBindTightly};

#if NeedFunctionPrototypes
void XmtRegisterTemplate(Widget w, StringConst name, StringConst template)
#else
void XmtRegisterTemplate(w, name, template)
Widget w;
StringConst name;
StringConst template;
#endif
{
    XrmDatabase db = XmtDatabaseOfWidget(w);

    if (templateq[0] == NULLQUARK)
	templateq[0] = XrmPermStringToQuark(XmtNxmtTemplates);

    templateq[1] = XrmStringToQuark(name);

    XrmQPutStringResource(&db, templatebindings, templateq, template);
}

#if NeedFunctionPrototypes
void XmtRegisterStyle(Widget w, StringConst name, StringConst style)
#else
void XmtRegisterStyle(w, name, style)
Widget w;
StringConst name;
StringConst style;
#endif
{
    XrmDatabase db = XmtDatabaseOfWidget(w);

    if (styleq[0] == NULLQUARK)
	styleq[0] = XrmPermStringToQuark(XmtNxmtStyles);

    styleq[1] = XrmStringToQuark(name);

    XrmQPutStringResource(&db, templatebindings, styleq, style);
}

#if NeedFunctionPrototypes
String XmtLookupTemplate(Widget w, StringConst name)
#else
String XmtLookupTemplate(w, name)
Widget w;
StringConst name;
#endif
{
    XrmDatabase db = XmtDatabaseOfWidget(w);
    XrmRepresentation type;
    XrmValue value;

    if (templateq[0] == NULLQUARK)
	templateq[0] = XrmPermStringToQuark(XmtNxmtTemplates);

    templateq[1] = XrmStringToQuark(name);
    if (XrmQGetResource(db, templateq, templateq, &type, &value))
	return value.addr;
    else
	return NULL;
}

#if NeedFunctionPrototypes
String XmtLookupStyle(Widget w, StringConst name)
#else
String XmtLookupStyle(w, name)
Widget w;
StringConst name;
#endif
{
    XrmDatabase db = XmtDatabaseOfWidget(w);
    XrmRepresentation type;
    XrmValue value;

    if (styleq[0] == NULLQUARK)
	styleq[0] = XrmPermStringToQuark(XmtNxmtStyles);

    styleq[1] = XrmStringToQuark(name);
    if (XrmQGetResource(db, styleq, styleq, &type, &value))
	return value.addr;
    else
	return NULL;
}

