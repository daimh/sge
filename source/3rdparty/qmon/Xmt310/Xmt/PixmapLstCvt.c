/* 
 * Motif Tools Library, Version 3.1
 * $Id: PixmapLstCvt.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: PixmapLstCvt.c,v $
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
#include <Xmt/Pixmap.h>
#include <Xmt/AppResP.h>
#include <Xmt/Color.h>

#define skipblanks(s) while (isspace(*s)) s++

/*
 * A PixmapList is a StringList optionally followed by a colon and
 * a color table.  So we invoke the StringList and ColorTable converters,
 * and then call XmtGetPixmap.
 */

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToPixmapList(Display *dpy,
				     XrmValue *args, Cardinal *num_args,
				     XrmValue *from, XrmValue *to,
				     XtPointer *converter_data)
#else
Boolean XmtConvertStringToPixmapList(dpy, args, num_args,
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
    Widget widget = *(Widget *)args[0].addr;
    XmtAppResources *app = XmtGetApplicationResources(widget);
    char *strlist;
    char *tablestr;
    char *mark;
    Boolean instring;
    Boolean free_string = False;
    Boolean free_table = False;
    Boolean free_string_list = False;
    XrmValue fromval, toval;
    XrmValue color_table_args[2];
    XmtColorTable table, parent_table;
    String *pixmap_names;
    XtCacheRef color_table_cache_ref, string_list_cache_ref;
    XtCacheRef refs[2];
    Boolean status;
    Pixmap *pixmaps;
    int i, num_items;

    table = parent_table = app->colortable;

    /* scan for an unquoted colon */
    for(tablestr = s, instring=False; *tablestr; tablestr++) {
	if (*tablestr == '"') instring = !instring;
	if (!instring && (*tablestr == ':')) break;
    }

    /* if there is one, make a NULL-terminated copy of the leading string list
     */
    if (*tablestr) {
	free_string = True;
	for(mark=tablestr-1; isspace(*mark); mark--);
	strlist = XtMalloc(mark-s+2);
	strncpy(strlist, s, mark-s+1);
	strlist[mark-s+1] = '\0';
    }
    else
	strlist = s;

    /*
     * convert the string list by calling the string list converter
     * directly.  This means we will always link that converter.  No
     * big deal.
     */
    fromval.addr = strlist;
    fromval.size = strlen(strlist) + 1;
    toval.addr = (XPointer) &pixmap_names;
    toval.size = sizeof(String *);
    status = XtCallConverter(dpy, XmtConvertStringToStringList, NULL, 0,
			     &fromval, &toval, &string_list_cache_ref);
    if (!status) goto error;
    else free_string_list = True;
    
    /*
     * convert the color table, if defined, and if a converter is registered.
     */
    if (*tablestr) {
	tablestr++;  /* skip colon */
	for(; isspace(*tablestr); tablestr++);
	if (_XmtColorTableConverter == NULL) {
	    XmtWarningMsg("XmtConvertStringToPixmapList", "noColor",
			  "can't convert string '%s' to XmtColorTable.\n\tNo XmtColorTable converter registered.\n\tSee XmtRegisterColorTableConverter();",
			  tablestr);
	}
	else {
	    Screen *screen = XtScreenOfObject(widget);
	    fromval.addr = tablestr;
	    fromval.size = strlen(tablestr) + 1;
	    toval.addr = (XPointer) &table;
	    toval.size = sizeof(XmtColorTable);
	    color_table_args[0].addr = (XPointer) &parent_table;
	    color_table_args[0].size = sizeof(XmtColorTable);
	    color_table_args[1].addr = (XPointer) &screen;
	    color_table_args[1].size = sizeof(Screen *);
	    status = XtCallConverter(dpy, _XmtColorTableConverter,
				     color_table_args, 2,
				     &fromval, &toval, &color_table_cache_ref);
	    if (!status)
		XmtWarningMsg("XmtConvertStringToPixmapList", "badColor",
			      "continuing with default color table.");
	    else
		free_table = True;
	}
    }

    /*
     * now convert the list of strings to a list of pixmaps
     * First, count the string, and allocate a pixmap array.
     * If a pixmap conversion fails and we get None, we convert it
     * to XmUNSPECIFIED_PIXMAP, because None would be interpreted as
     * terminating the array.  Motif widgets like this value better than
     * None, but if this converter is used with other widget types, the
     * returned array will have to be tested for these values.
     */
    for(i=0; pixmap_names[i]; i++);
    num_items = i;
    pixmaps = (Pixmap *)XtMalloc((num_items+1) * sizeof(Pixmap));
    for(i = 0; i < num_items; i++) {
	pixmaps[i] = XmtGetPixmap(widget, table, pixmap_names[i]);
	if (pixmaps[i] == None) pixmaps[i] = XmUNSPECIFIED_PIXMAP;
    }
    /* NULL-terminate the array */
    pixmaps[num_items] = (Pixmap) NULL;
	
    if (free_string) XtFree(strlist);
    if (free_table) {
	refs[0] = color_table_cache_ref;
	refs[1] = NULL;
	XtAppReleaseCacheRefs(XtWidgetToApplicationContext(widget), refs);
    }
    if (free_string_list) {
	refs[0] = string_list_cache_ref;
	refs[1] = NULL;
	XtAppReleaseCacheRefs(XtWidgetToApplicationContext(widget), refs);
    }

    done(Pixmap *, pixmaps);

 error:
    /*
     * some other converter has called XtDisplayConversionWarning
     * to print the string, so we just have to say that we failed
     */
    XmtWarningMsg("XmtCvtStringToPixmapList", "failed",
		  "Pixmap list conversion failed.");
    if (free_string) XtFree(strlist);
    return False;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void XmtDestroyPixmapList(XtAppContext app, XrmValue *to,
				 XtPointer converter_data,
				 XrmValue *args, Cardinal *num_args)
#else
static void XmtDestroyPixmapList(app, to, converter_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer converter_data;
XrmValue *args;
Cardinal *num_args;
#endif
{
    Widget widget = *(Widget *)args[0].addr;
    Pixmap *pixmaps = *(Pixmap **) to->addr;
    int i;

    for(i=0; pixmaps[i] != (Pixmap)NULL; i++)
	XmtReleasePixmap(widget, pixmaps[i]);
    XtFree((char *)pixmaps);
}

#if NeedFunctionPrototypes
void XmtRegisterPixmapListConverter(void)
#else
void XmtRegisterPixmapListConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	registered = True;
	XmtRegisterStringListConverter();
	XtSetTypeConverter(XtRString, XmtRPixmapList,
			   XmtConvertStringToPixmapList,
			   _XmtWidgetConvertArg, 1,
			   XtCacheNone | XtCacheRefCount,
			   XmtDestroyPixmapList);
    }
}
