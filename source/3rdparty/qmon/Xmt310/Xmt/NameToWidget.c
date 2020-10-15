/* 
 * Motif Tools Library, Version 3.1
 * $Id: NameToWidget.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: NameToWidget.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * Portions of this file are derived from the Xt source code.
 * See the file COPYRIGHT for the MIT and Digital copyrights.
 */

#include <Xmt/Xmt.h>
#include <Xmt/Util.h>    
#include <Xmt/AppRes.h>
#include <X11/IntrinsicP.h>

static XrmQuark selfq = NULLQUARK;
static XrmQuark questionq = NULLQUARK;
    
#if NeedFunctionPrototypes
static Widget GetParent(Widget w)
#else
static Widget GetParent(w)
Widget w;
#endif
{
    if (w == NULL) return NULL;
    if (XtParent(w) != NULL) return XtParent(w);
    else return w;
}

#if NeedFunctionPrototypes
static Widget GetNamedAncestor(Widget ref, char *name)
#else
static Widget GetNamedAncestor(ref, name)
Widget ref;
char *name;
#endif
{
    Widget w;
    WidgetClass c;
    XrmQuark q = XrmStringToQuark(name);

    if (ref == NULL) return NULL;

    /* see if the supplied name matches any ancestor's names */
    for(w = XtParent(ref); w != NULL; w = XtParent(w))
	if (w->core.xrm_name == q) return w;

    /*
     * if not, start checking classes.  Check the class name, and
     * all the superclass names of each ancestor widget.
     * This is done with WidgetClass, but should work for objects too.
     */
    for(w = XtParent(ref); w != NULL; w = XtParent(w)) 
	for(c = XtClass(w); c != NULL; c = c->core_class.superclass)
	    if (c->core_class.xrm_class == q) return w;

    /* if no name or class matches, return NULL */
    return NULL;
}

#if NeedFunctionPrototypes
static Widget GetChild(Widget w, XrmName q)
#else
static Widget GetChild(w, q)
Widget w;
XrmName q;
#endif
{
    CompositeWidget parent = (CompositeWidget) w;
    int i;

    /*
     * Check the names of all normal children.
     * Check the names of all popup children.
     * Check the class names of all normal children.
     * Check the class names of all popup children.
     * Note that we don't check superclass names.
     */

    if (w == NULL) return NULL;

    /* normal children names */
    if (XtIsComposite(w)) {
	for(i = 0; i < parent->composite.num_children; i++)
	    if (parent->composite.children[i]->core.xrm_name == q)
		return parent->composite.children[i];
    }

    /* popup children names */
    for(i = 0; i < w->core.num_popups; i++)
	if (w->core.popup_list[i]->core.xrm_name == q)
	    return w->core.popup_list[i];

    /* normal children classes */
    if (XtIsComposite(w)) {
	for(i = 0; i < parent->composite.num_children; i++)
	    if (XtClass(parent->composite.children[i])->core_class.xrm_class
		== q)
		return parent->composite.children[i];
    }

    /* popup children classes */
    for(i = 0; i < w->core.num_popups; i++)
	if (XtClass(w->core.popup_list[i])->core_class.xrm_class == q)
	    return w->core.popup_list[i];

    /* no child matches */
    return NULL;
}


#if NeedFunctionPrototypes
static Widget ParseWidgetModifiers(Widget w, StringConst *name)
#else
static Widget ParseWidgetModifiers(w, name)
Widget w;
StringConst *name;
#endif
{
    if (*name == NULL || **name == '\0')
	return w;  /* no recursion */
    else if (**name == '.') {
	/*
	 * ignore '.' if there are following modifiers, or
	 * return w if no modifiers follow it.
	 */
	*name += 1;
	return ParseWidgetModifiers(w, name);
    }
    else if (**name == '~') {
	*name += 1;
	return XmtGetShell(ParseWidgetModifiers(w, name));
    }
    else if (**name == '^') {
	if (*(*name+1) != '{') {
	    *name += 1;
	    return GetParent(ParseWidgetModifiers(w, name));
	}
    	else {
	    char ancestor[100];
	    int i = 0;
	    *name += 2;
	    while (**name && **name != '}') {
		ancestor[i++] = **name;
		*name += 1;
	    }
	    ancestor[i] = '\0';
	    if (**name == '}') *name += 1;
	    return GetNamedAncestor(ParseWidgetModifiers(w, name),
				    ancestor);
	}
    }
    else /* there are no more modifiers, so don't recurse */
	return w;
}


/*
 * The following 5 procedures are code modified from the internals
 * of the function XtNameToWidget.  This code appears in the file
 * Intrinsic.c in the MIT Xt distribution, with the following RCS info:
 * $XConsortium: Intrinsic.c,v 1.171 91/07/16 18:30:20 converse Exp $
 */
#if NeedFunctionPrototypes
static Widget NameListToWidget(register Widget root,
			       XrmNameList names, XrmBindingList bindings,
			       int num_quarks, int in_depth, int *out_depth,
			       int *found_depth);
#else
static Widget NameListToWidget();
#endif

typedef Widget (*NameMatchProc)();

#if NeedFunctionPrototypes
static Widget MatchExactChildren(XrmNameList names, XrmBindingList bindings,
				 int num_quarks, register WidgetList children,
				 register int num, int in_depth,
				 int *out_depth, int *found_depth)
#else
static Widget MatchExactChildren(names, bindings, num_quarks, children, num,
				 in_depth, out_depth, found_depth)
XrmNameList names;
XrmBindingList bindings;
int num_quarks;
register WidgetList children;
register int num;
int in_depth;
int *out_depth;
int *found_depth;
#endif
{
    register Cardinal   i;
    register XrmName    name = *names;
    Widget w, result = NULL;
    int d, min = 10000;

    /* check the names of children.  '?' always matches -- djf */
    for (i = 0; i < num; i++) {
	if ((name == children[i]->core.xrm_name) ||
	    (name == questionq)) /* djf */ {
	    w = NameListToWidget(children[i], &names[1], &bindings[1],
				 num_quarks-1,
				 in_depth+1, &d, found_depth);
	    if (w != NULL && d < min) {result = w; min = d;}
	}
    }

    *out_depth = min;
    return result;
}

#if NeedFunctionPrototypes
static Widget MatchExactClass(XrmNameList names, XrmBindingList bindings,
			      int num_quarks, register WidgetList children,
			      register int num, int in_depth, int *out_depth,
			      int *found_depth)
#else
static Widget MatchExactClass(names, bindings, num_quarks, children, num,
			      in_depth, out_depth, found_depth)
XrmNameList names;
XrmBindingList bindings;
int num_quarks;
register WidgetList children;
register int num;
int in_depth;
int *out_depth;
int *found_depth;
#endif
{
    register Cardinal   i;
    register XrmName    name = *names;
    Widget w, result = NULL;
    int d, min = 10000;

    /* like the above, but check classes  -- djf */
    for (i = 0; i < num; i++) {
	if (name == XtClass(children[i])->core_class.xrm_class) {
	    w = NameListToWidget(children[i], &names[1], &bindings[1],
				 num_quarks-1,
				 in_depth+1, &d, found_depth);
	    if (w != NULL && d < min) {result = w; min = d;}
	}
    }
	
    *out_depth = min;
    return result;
}


#if NeedFunctionPrototypes
static Widget MatchWildChildren(XrmNameList names, XrmBindingList bindings,
				int num_quarks,	register WidgetList children,
				register int num, int in_depth,
				int *out_depth, int *found_depth)
#else
static Widget MatchWildChildren(names, bindings, num_quarks, children, num,
				in_depth, out_depth, found_depth) 
XrmNameList names;
XrmBindingList bindings;
int num_quarks;
register WidgetList children;
register int num;
int in_depth;
int *out_depth;
int *found_depth;
#endif
{
    register Cardinal   i;
    Widget w, result = NULL;
    int d, min = 10000;

    for (i = 0; i < num; i++) {
	w = NameListToWidget(children[i], names, bindings, num_quarks,
		in_depth+1, &d, found_depth);
	if (w != NULL && d < min) {result = w; min = d;}
    }
    *out_depth = min;
    return result;
}

#if NeedFunctionPrototypes
static Widget SearchChildren(Widget root,
			     XrmNameList names, XrmBindingList bindings,
			     int num_quarks, NameMatchProc matchproc,
			     int in_depth, int *out_depth, int *found_depth)
#else
static Widget SearchChildren(root, names, bindings, num_quarks, matchproc,
			     in_depth, out_depth, found_depth)
Widget root;
XrmNameList names;
XrmBindingList bindings;
int num_quarks;
NameMatchProc matchproc;
int in_depth;
int *out_depth;
int *found_depth;
#endif
{
    Widget w1, w2;
    int d1, d2;

    if (XtIsComposite(root)) {
	w1 = (*matchproc)(names, bindings, num_quarks,
		((CompositeWidget) root)->composite.children,
		((CompositeWidget) root)->composite.num_children,
		in_depth, &d1, found_depth); 
    }
    else {
	d1 = 10000;
	w1 = NULL;
    }
    w2 = (*matchproc)(names, bindings, num_quarks, root->core.popup_list,
	    root->core.num_popups, in_depth, &d2, found_depth);
    *out_depth = (d1 < d2 ? d1 : d2);
    return (d1 < d2 ? w1 : w2);
}

#if NeedFunctionPrototypes
static Widget NameListToWidget(register Widget root,
			       XrmNameList names, XrmBindingList bindings,
			       int num_quarks, int in_depth,
			       int *out_depth, int *found_depth)
#else
static Widget NameListToWidget(root, names, bindings, num_quarks, in_depth,
			       out_depth, found_depth) 
register Widget root;
XrmNameList names;
XrmBindingList bindings;
int num_quarks;
int in_depth;
int *out_depth;
int *found_depth;
#endif
{
    Widget w1, w2;
    int d1, d2;

    if (in_depth >= *found_depth) {
	*out_depth = 10000;
	return NULL;
    }

    if (num_quarks == 0) {
	*out_depth = *found_depth = in_depth;
	return root;
    }

    if (! XtIsWidget(root)) {
	*out_depth = 10000;
	return NULL;
    }

    if (*bindings == XrmBindTightly) {
	w1 =  SearchChildren(root, names, bindings, num_quarks,
			     MatchExactChildren,
			     in_depth, out_depth, found_depth);
	/* if no names match, look for a class name that matches -- djf */
	if (w1 == NULL) {
	    w1 = SearchChildren(root, names, bindings, num_quarks,
				MatchExactClass,
				in_depth, out_depth, found_depth);
	}
	
	if (w1 != NULL)
	    return w1;
    }
    else {  /* XrmBindLoosely */
	w1 = SearchChildren(root, names, bindings, num_quarks,
			    MatchExactChildren,
			    in_depth, &d1, found_depth);
	if (w1 == NULL) { /* if no names match exactly, check classes -- djf */
	    w1 = SearchChildren(root, names, bindings, num_quarks,
				MatchExactClass,
				in_depth, &d1, found_depth);
	}
	w2 = SearchChildren(root, names, bindings, num_quarks,
			    MatchWildChildren,
			    in_depth, &d2, found_depth);
	*out_depth = (d1 < d2 ? d1 : d2);
	return (d1 < d2 ? w1 : w2);
    }
    /* NOTREACHED */
    return NULL;  /* but return something anyway to keep gcc -Wall quiet */
} /* NameListToWidget */


/* modified X Consortium code ends here. */

#if NeedFunctionPrototypes
Widget XmtNameToWidget(Widget ref, StringConst name)
#else
Widget XmtNameToWidget(ref, name)
Widget ref;
StringConst name;
#endif
{
    StringConst newname;
    Boolean modifiers = False;
    XrmQuark quark_array[50];
    XrmBinding binding_array[50];
    XrmQuarkList quarks;
    XrmBindingList bindings;
    int numquarks;
    Widget w;
    int depth = 0;
    int found = 10000;
    
    /* initialize quarks used by lots of functions */
    if (selfq == NULLQUARK) selfq = XrmStringToQuark("self");
    if (questionq == NULLQUARK)	questionq = XrmStringToQuark("?");
    
    newname = name;
    /* if the name begins with a modifier, go parse them */
    if ((name[0] == '.') || (name[0] == '^') || (name[0] == '~')) {
	modifiers = True;
	ref = ParseWidgetModifiers(ref, &newname);
    }

    /*
     * if no name, or nothing following the modifiers,
     * then just return the reference widget.  This is
     * so names like "^^" work.
     */
    if (!newname || !*newname) return ref;

    /* quarkify the name and count the quarks */
    quarks = quark_array;
    bindings = binding_array;
    XrmStringToBindingQuarkList(newname, bindings, quarks);
    for(numquarks = 0; quarks[numquarks] != NULLQUARK; numquarks++);
    
    if (!modifiers) {
	/* if the name begins with '*', it is relative to the root. */
	if (name[0]== '*')
	    ref = XmtGetApplicationShell(ref);
	else {
	    /*
	     * if first quark is 'self', skip it.
	     * if first quark is a sibling of ref, ref = parent
	     * otherwise if first quark doesn't name a child, maybe it
	     * names a root widget.  If it doesn't name any known root,
	     * it is an error.
	     */
	    if (quarks[0] == selfq) {
		quarks++;
		bindings++;
		numquarks--;
	    }
	    else if (GetChild(XtParent(ref), quarks[0]))
		ref = XtParent(ref);
	    else if (!GetChild(ref, quarks[0])) {
		ref = XmtLookupApplicationShell(quarks[0]);
		if (!ref) return NULL;
		quarks++;
		bindings++;
		numquarks--;
	    }
	}
    }

    w =  NameListToWidget(ref, quarks, bindings, numquarks,
			  0, &depth, &found);
    return w;
}

