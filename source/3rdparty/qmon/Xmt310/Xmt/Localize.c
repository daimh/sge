/* 
 * Motif Tools Library, Version 3.1
 * $Id: Localize.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Localize.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <Xmt/XmtP.h>
#include <Xmt/LookupP.h>

#if NeedFunctionPrototypes
String _XmtLocalize(Screen *screen, StringConst default_string,
		    StringConst category, StringConst tag)
#else
String _XmtLocalize(screen, default_string, category, tag)
Screen *screen;
StringConst default_string, category, tag;
#endif
{
    char buf[200];
    String name;
    String new;

    if (category && tag) {
	sprintf(buf, "%s.%s", category, tag);
	name = buf;
    }
    else if (category) name = (String) category;
    else name = (String) tag;

    new = _XmtLookupResource(screen, "Mltc", name);
    if (new) return new;
    else return (String) default_string;
}

#if NeedFunctionPrototypes
String XmtLocalize2(Widget w, StringConst default_string,
		   StringConst category, StringConst tag)
#else
String XmtLocalize2(w, default_string, category, tag)
Widget w;
StringConst default_string, category, tag;
#endif
{
    return _XmtLocalize(XtScreenOfObject(w), default_string, category, tag);
}

#if NeedFunctionPrototypes
String XmtLocalize(Widget w, StringConst default_string, StringConst tag)
#else
String XmtLocalize(w, default_string, tag)
Widget w;
StringConst default_string;
StringConst tag;
#endif
{
    return _XmtLocalize(XtScreenOfObject(w), default_string, NULL, tag);
}

#if NeedFunctionPrototypes
String XmtLocalizeWidget(Widget w, StringConst default_string, StringConst tag)
#else
String XmtLocalizeWidget(w, default_string, tag)
Widget w;
StringConst default_string;
StringConst tag;
#endif
{
    return _XmtLocalize(XtScreenOfObject(w), default_string,
			XtClass(w)->core_class.class_name,
			tag);
}

