/* 
 * Motif Tools Library, Version 3.1
 * $Id: GetPixmap.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: GetPixmap.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#include <Xmt/Xmt.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Xpm.h>
#include <Xmt/Xbm.h>
#include <Xmt/Color.h>
#include <Xmt/AppResP.h>
#include <Xmt/ConvertersP.h>
#include <Xmt/LookupP.h>
#include <X11/IntrinsicP.h>

#if NeedFunctionPrototypes
static Pixmap GetPixmap(Widget object, StringConst str, XmtColorTable table,
			Boolean get_bitmap)
#else
static Pixmap GetPixmap(object, str, table, get_bitmap)
Widget object;
StringConst str;
XmtColorTable table;
Boolean get_bitmap;
#endif
{
    Widget w, shell;
    Display *display;
    Screen *screen;
    Visual *visual;
    Colormap colormap;
    unsigned int depth;
    XmtAppResources *app;
    XmtImage *xmtimage = NULL;
    struct {
	char *bits;
	int width, height, hot_x, hot_y;
    } xbmdata;
    Pixmap pixmap = None;
    String name = NULL;
    String table_string;
    String filename = NULL;
    String path = NULL;
    Boolean pixmap_file = False;
    Boolean bitmap_file = False;
    XtCacheRef color_table_cache_ref;
    XtCacheRef refs[2];
    Boolean free_color_table = False;
    static int unique_image_number;

    for(w=object; !XtIsWidget(w); w = XtParent(w));
    shell = XmtGetShell(w);

    app = XmtGetApplicationResources(shell);
    screen = XtScreen(w);
    display = DisplayOfScreen(screen);
    visual = XmtGetVisual(shell);
    colormap = w->core.colormap;
    depth = w->core.depth;
    if (!table) table = app->colortable;

    /*
     * see if it is a literal XPM file
     */
    if (!get_bitmap && strncmp(str, "/* XPM */", 9) == 0) {
	xmtimage = XmtParseXpmString(str);
	if (xmtimage) {
	    name = XtMalloc(8);
	    sprintf(name, "_%d_", unique_image_number++);
	    goto found;
	}
	else return None;
    }

    /*
     * see if it is a literal XBM file
     */
    xbmdata.bits = NULL;
    if (strncmp(str, "#define", 7) == 0) {
	if (XmtParseXbmString(str, &xbmdata.bits,
			      &xbmdata.width, &xbmdata.height,
			      &xbmdata.hot_x, &xbmdata.hot_y)) {
	    name = XtMalloc(16);
	    sprintf(name, "_Xmt%d", unique_image_number++);
	    goto found;
	}
	else
	    return None;
    }

    /*
     * Otherwise it is a single name, optionally followed by a color table.
     * If there is a colon, parse the color table, and get a null-terminated
     * copy of the single name.
     * In either case, name is set to the image name.
     * if (name != str) then name must be freed later.
     */
    table_string = strchr(str, ':');
    if (table_string) {
	register int i;

	table_string++; /* points to first char after colon */
	if (!get_bitmap) {
	    if (_XmtColorTableConverter != NULL) {
		XrmValue from, to;
		XrmValue color_table_args[2];
		XmtColorTable parent_table;
		Boolean status;

		parent_table = table;
		from.addr = table_string;
		from.size = strlen(table_string);
		to.addr = (XPointer)&table;
		to.size = sizeof(XmtColorTable);
		color_table_args[0].addr = (XPointer)&parent_table;
		color_table_args[0].size = sizeof(XmtColorTable);
		color_table_args[1].addr = (XPointer)&screen;
		color_table_args[1].size = sizeof(Screen *);
		status = XtCallConverter(display, _XmtColorTableConverter,
					 color_table_args, 2, 
					 &from, &to, &color_table_cache_ref);
		if (!status)
		    XmtWarningMsg("XmtGetPixmap", "badColor",
				  "continuing with default color table.");
		else
		    free_color_table = True;
	    }
	    else {
		XmtWarningMsg("XmtGetPixmap", "noColor",
			      "No XmtColorTable converter registered.\n\tSee XmtRegisterColorTableConverter();");
	    }
	}
	else {
	    XmtWarningMsg("XmtGetBitmap", "table",
		    "color table specification unused in bitmap conversion.");
	}
	    
	table_string -= 2;  /* points to char before colon */
	for(; isspace(*table_string); table_string--);  /* skip over blanks*/
	i = (table_string - str) + 1;
	name = strncpy(XtMalloc(i+1), str, i);
	name[i] = '\0';
    }
    else
	name = (String) str;

    /*
     * see if name is defined in the image cache
     */
    if (get_bitmap)
	pixmap = XmtLookupBitmap(w, name);
    else
	pixmap = XmtLookupPixmap(w, visual, colormap, depth, table, name);
    
    if (pixmap) goto end;
    else {
	/*
	 * If it is not in the image cache, check if it is the name of a
	 * resource under _Pixmaps_ or _Bitmaps_.  We lookup the image
	 * as:
	 *    _Pixmaps_.visual.depth.resolution.language.territory.codeset.name
	 * For backward compatibility with Xmt 1.2, we also do just:
	 *    _Pixmaps_.name
	 * because in 1.2 '.name' was more common than '*name'
	 *
	 * For bitmaps, we use _Bitmaps_, and omit the visual and depth.
	 */
	String value;

	if (!get_bitmap) {
	    value = _XmtLookupResource(screen, "PVDZltc", name);
	    if (value) {
		if ((xmtimage = XmtParseXpmString(value))) goto found;
		else goto end;
	    }
	}

	value = _XmtLookupResource(screen, "BZltc", name);
	if (value) {
	    if (XmtParseXbmString(value, &xbmdata.bits,
				  &xbmdata.width, &xbmdata.height,
				  &xbmdata.hot_x, &xbmdata.hot_y))
		goto found;
	    else goto end;
	}
	
	/* if still not found, look up the old way, for compatibility */
	if (!get_bitmap) {
	    value = _XmtLookupResource(screen, "P", name);
	    if (value) {
		if ((xmtimage = XmtParseXpmString(value))) goto found;
		else goto end;
	    }
	}
	
	value = _XmtLookupResource(screen, "B", name);
	if (value) {
	    if (XmtParseXbmString(value, &xbmdata.bits,
				  &xbmdata.width, &xbmdata.height,
				  &xbmdata.hot_x, &xbmdata.hot_y))
		goto found;
	    else goto end;
	}
    }

    /*
     * if we still haven't found a pixmap, name must be a filename
     */
    if (pixmap == None) {
	/*
	 * handle absolute and relative filenames
	 */
	if ((name[0] == '/') ||
	    ((name[0] == '.') && (name[1] == '/')) ||
	    ((name[0] == '.') && (name[1] == '.') && (name[2] == '/')))
	    filename = name;

 	/*
	 * check relative to an environment variable, if defined
	 * Note that since we don't know the path, we don't know for
	 * sure that it uses the supplied type and suffix, so we don't
	 * know what type of file is being found
	 */
	if (!get_bitmap && !filename && (path = getenv("XPMLANGPATH"))) {
	    filename = XmtFindFile(shell, "pixmaps", name, ".xpm",
				   NULL, path, XmtSearchPathOnly);
	    if (filename) pixmap_file = True;
	}

	if (!filename && (path = getenv("XBMLANGPATH"))) {
	    filename = XmtFindFile(shell, "bitmaps", name, ".xbm",
				   NULL, path, XmtSearchPathOnly);
	    if (filename) bitmap_file = True;
	}

	/* next check the user, app, and system pixmap and bitmap paths */
	if (!filename) {
	    if (!get_bitmap) {
		filename = XmtFindFile(shell, "pixmaps", name, ".xpm",
				       NULL, app->pixmap_file_path,
				       XmtSearchEverywhere);
		if (filename)
		    pixmap_file = True;
	    }
	    if (!filename) {
		filename = XmtFindFile(shell, "bitmaps", name, ".xbm",
				       NULL, app->bitmap_file_path,
				       XmtSearchEverywhere);
		if (filename)
		    bitmap_file = True;
	    }
	}

	/*
	 * if we found a file, read the first few bytes to determine the
	 * type.  Read it as appropriate.  Register the resulting
	 * pixmap in the cache using the filename as the key.
	 */
	if (filename) {
	    FILE *f;
	    char buf[10];

	    if (!pixmap_file && !bitmap_file) {
		if (get_bitmap) bitmap_file = True;
		else if ((f = fopen(filename, "r")) != NULL) {
		    (void)fgets(buf, 10, f);
		    if (strncmp(buf, "/* XPM */", 9) == 0)
			pixmap_file = True;
		    else if (strncmp(buf, "#define", 7) == 0)
			bitmap_file = True;
		    (void)fclose(f);
		}
	    }

	    if (pixmap_file || (!pixmap_file && !bitmap_file)) {
		xmtimage = XmtParseXpmFile(filename);
		if (xmtimage) goto found;
	    }

	    if (bitmap_file || (!pixmap_file && !bitmap_file)) {
		if (XmtParseXbmFile(filename, &xbmdata.bits,
				    &xbmdata.width, &xbmdata.height,
				    &xbmdata.hot_x, &xbmdata.hot_y))
		    goto found;
	    }
	}
    }

 found:
    /*
     * register the xpm or xbm data, and then get a pixmap for it.
     * we jump here when we successfully parse a xpm or xbm file or string.
     */
    if (xmtimage)
	XmtRegisterImage(name, xmtimage);
    else if (xbmdata.bits)
	XmtRegisterXbmData(name, xbmdata.bits, NULL,
			   xbmdata.width, xbmdata.height,
			   xbmdata.hot_x, xbmdata.hot_y);

    if (xmtimage || xbmdata.bits) {
	if (get_bitmap)
	    pixmap = XmtLookupBitmap(w, name);
	else
	    pixmap = XmtLookupPixmap(w, visual, colormap,
				     depth, table, name);
    }
    
 end:
    /*
     * free what we need to and return.
     * we jump here if the pixmap is already cached, or on error.
     */
    if (filename && filename != name) XtFree(filename);
    if (name && name != str) XtFree(name);
    if (free_color_table) {
	refs[0] = color_table_cache_ref;
	refs[1] = NULL;
	XtAppReleaseCacheRefs(XtWidgetToApplicationContext(w), refs);
    }
    return pixmap;
}

#if NeedFunctionPrototypes
Pixmap XmtGetBitmap(Widget object, StringConst str)
#else
Pixmap XmtGetBitmap(object, str)
Widget object;
StringConst str;
#endif
{
    return GetPixmap(object, str, NULL, True);
}

#if NeedFunctionPrototypes
Pixmap XmtGetPixmap(Widget object, XmtColorTable table, StringConst str)
#else
Pixmap XmtGetPixmap(object, table, str)
Widget object;
XmtColorTable table;
StringConst str;
#endif
{
    return GetPixmap(object, str, table, False);
}
