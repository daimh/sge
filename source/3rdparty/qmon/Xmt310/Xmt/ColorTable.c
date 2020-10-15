/* 
 * Motif Tools Library, Version 3.1
 * $Id: ColorTable.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: ColorTable.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Hash.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/Lexer.h>
#include <Xmt/AppResP.h>
#include <X11/IntrinsicP.h>

/*
 * A color table is basically just a hash table, but it also
 * has a pointer to a "parent" color table that will be searched
 * if a desired color is not found.  This allows selective
 * overriding.  XmtColorTable is typedef'ed to a pointer to this
 * structure in Color.h.
 */
typedef struct _XmtColorTableRec {
    struct _XmtColorTableRec *parent;
    XmtHashTable table;
} XmtColorTableRec;


#if NeedFunctionPrototypes
XmtColorTable XmtCreateColorTable(XmtColorTable parent)
#else
XmtColorTable XmtCreateColorTable(parent)
XmtColorTable parent;
#endif
{
    XmtColorTable ct = XtNew(XmtColorTableRec);

    ct->parent = parent;
    ct->table = XmtHashTableCreate(3);
    return ct;
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void DestroyColorTableEntry(XmtHashTable t,
				   XtPointer key, XtPointer *data)
#else
static void DestroyColorTableEntry(t, key, data)
XmtHashTable t;
XtPointer key, data;
#endif
{
    XtFree(*(String *)data);
}


#if NeedFunctionPrototypes
void XmtDestroyColorTable(XmtColorTable ct)
#else
void XmtDestroyColorTable(ct)
XmtColorTable ct;
#endif
{
    /*
     * Free any memory allocated for the data stored in the
     * hash table, then destroy the hash table itself.
     * Calling this function does not free any pixels that were
     * allocated through this color table.  Nor does it
     * destroy the color table structure itself.  Color table
     * pointers are used in the bitmap and pixmap cache, and
     * so must be persistant unique identifiers.  If we free
     * a colortable, the same address might be reallocated,
     * and throw off the caching scheme.  (This actually happened.)
     * Maybe this could be called a memory leak, but is only 8
     * bytes, and there is nothing we can do about it.
     */
    XmtHashTableForEach(ct->table, DestroyColorTableEntry);
    XmtHashTableDestroy(ct->table);
}

#if NeedFunctionPrototypes
void XmtColorTableSetParent(XmtColorTable table, XmtColorTable parent)
#else
void XmtColorTableSetParent(table, parent)
XmtColorTable table;
XmtColorTable parent;
#endif
{
    table->parent = parent;
}

#if NeedFunctionPrototypes
XmtColorTable XmtColorTableGetParent(XmtColorTable table)
#else
XmtColorTable XmtColorTableGetParent(table)
XmtColorTable table;
#endif
{
    return table->parent;
}

#if NeedFunctionPrototypes
void XmtRegisterColor(XmtColorTable ct, StringConst symbol, StringConst color)
#else
void XmtRegisterColor(ct, symbol, color)
XmtColorTable ct;
StringConst symbol;
StringConst color;
#endif
{
    XrmQuark symbolq = XrmStringToQuark(symbol);
    String oldcolor;

    /*
     * You can safely redefine colors, but note that because of resource
     * caching by Xt, redefined colors won't work unless you use
     * XmtAllocColor() directly.
     */

    if (XmtHashTableLookup(ct->table, (XtPointer)symbolq,
			   (XtPointer *) &oldcolor))
	XtFree(oldcolor);

    XmtHashTableStore(ct->table, (XtPointer)symbolq,
		      (XtPointer)XtNewString(color));
}

#if NeedFunctionPrototypes
void XmtRegisterColors(XmtColorTable ct, XmtColorPair *colors, Cardinal num)
#else
void XmtRegisterColors(ct, colors, num)
XmtColorTable ct;
XmtColorPair *colors;
Cardinal num;
#endif
{
    int i;

    for(i=0; i < num; i++)
	XmtRegisterColor(ct, colors[i].symbolic_name, colors[i].color_name);
}

#if NeedVarargsPrototypes
void XmtVaRegisterColors(XmtColorTable ct, ...)
#else
void XmtVaRegisterColors(ct, va_alist)
XmtColorTable ct;
va_dcl
#endif
{
    va_list var;
    String symbol, color;

    Va_start(var, ct);

    while((symbol = va_arg(var, String)) != NULL) {
	color = va_arg(var, String);
	XmtRegisterColor(ct, symbol, color);
    }

    va_end(var);
}

#if NeedFunctionPrototypes
void XmtRegisterPixel(XmtColorTable ct, StringConst symbol, Display *display,
		      Colormap colormap, Pixel pixel)
#else
void XmtRegisterPixel(ct, symbol, display, colormap, pixel)
XmtColorTable ct;
StringConst symbol;
Display *display;
Colormap colormap;
Pixel pixel;
#endif
{
    XColor color;
    char name[14];
    static char hexchars[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f'};

    color.pixel = pixel;
    XQueryColor(display, colormap, &color);

    name[0] = '#';

    name[1] = hexchars[(int)color.red >> 12];
    name[2] = hexchars[((int)color.red & 0x0F00) >> 8];
    name[3] = hexchars[((int)color.red & 0x00F0) >> 4];
    name[4] = hexchars[(int)color.red & 0x000F];

    name[5] = hexchars[(int)color.green >> 12];
    name[6] = hexchars[((int)color.green & 0x0F00) >> 8];
    name[7] = hexchars[((int)color.green & 0x00F0) >> 4];
    name[8] = hexchars[(int)color.green & 0x000F];

    name[9] = hexchars[(int)color.blue >> 12];
    name[10] = hexchars[((int)color.blue & 0x0F00) >> 8];
    name[11] = hexchars[((int)color.blue & 0x00F0) >> 4];
    name[12] = hexchars[(int)color.blue & 0x000F];

    name[13] = '\0';

    XmtRegisterColor(ct, symbol, name);
}

#if NeedFunctionPrototypes
void XmtRegisterStandardColors(XmtColorTable ct, Widget w,
			       Pixel foreground, Pixel background)
#else
void XmtRegisterStandardColors(ct, w, foreground, background)
XmtColorTable ct;
Widget w;
Pixel foreground;
Pixel background;
#endif
{
    Display *dpy = XtDisplay(w);
    Screen *screen;
    Colormap cmap;
    Pixel dummy;
    Pixel top_shadow, bottom_shadow, select;

    while(!XtIsWidget(w)) w = XtParent(w);
    screen = w->core.screen;
    cmap = w->core.colormap;

    XmtRegisterPixel(ct, "background", dpy, cmap, background);
    XmtRegisterPixel(ct, "foreground", dpy, cmap, foreground);

    if (w->core.depth > 1) {
	XmGetColors(screen, cmap, background,
		    &dummy, &top_shadow, &bottom_shadow, &select);
	XmtRegisterPixel(ct, "top_shadow", dpy, cmap, top_shadow);
	XmtRegisterPixel(ct, "bottom_shadow", dpy, cmap, bottom_shadow);
	XmtRegisterPixel(ct, "select", dpy, cmap, select);
    }
    else {
	XmtRegisterPixel(ct, "top_shadow", dpy, cmap, foreground);
	XmtRegisterPixel(ct, "bottom_shadow", dpy, cmap, foreground);
	XmtRegisterPixel(ct, "select", dpy, cmap, foreground);
    }
}

#if NeedFunctionPrototypes
String XmtLookupColorName(XmtColorTable ct, StringConst symbol)
#else
String XmtLookupColorName(ct, symbol)
XmtColorTable ct;
StringConst symbol;
#endif
{
    XrmQuark symbolq = XrmStringToQuark(symbol);
    String name;

    while(ct) {
	if (XmtHashTableLookup(ct->table, (XtPointer)symbolq,
			       (XtPointer *)&name))
	    return name;
	else
	    ct = ct->parent;
    }
    return NULL;
}


/*
 * An XtConvertArgProc used by a couple of different converters
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
void _XmtFetchColorTable(Widget w, Cardinal *size, XrmValue *value)
#else
void _XmtFetchColorTable(w, size, value)
Widget w;
Cardinal *size;
XrmValue *value;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);

    value->size = sizeof(XmtColorTable);
    value->addr = (XPointer) &app->colortable;
}

