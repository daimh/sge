/* 
 * Motif Tools Library, Version 3.1
 * $Id: AppRes.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: AppRes.c,v $
 * Revision 1.1  2001-07-18 11:06:01  root
 * Initial revision
 *
 * Revision 1.1  2001/06/12 15:00:21  andre
 * AA-2001-06-12-0: replaced Xmt212 by Xmt310
 *                  (http://sourceforge.net/projects/motiftools) with
 *                  our xmt212 patches applied
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/AppResP.h>
#include <Xmt/Hash.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Color.h>
#include <Xmt/Xpm.h>
#include <Xmt/Converters.h> /* for def of XmtRXmtColorTable */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>  /* to get application shell class name */

#if NeedFunctionPrototypes
extern void _XmForegroundColorDefault(Widget, int, XrmValue *);
extern void _XmBackgroundColorDefault(Widget, int, XrmValue *);
#else
extern void _XmForegroundColorDefault();
extern void _XmBackgroundColorDefault();
#endif

#ifndef XMTDEFAULTCONFIGDIR
#ifndef VMS
#define XMTDEFAULTCONFIGDIR "/usr/lib/X11"
#else
#define XMTDEFAULTCONFIGDIR "decw$system_defaults:"
#endif
#endif

#ifndef XMTFILESEARCHPATH
#ifndef VMS
#define XMTFILESEARCHPATH "\
%R/%L/%a/%N%C%S:\
%R/%L/%a/%N%S:\
%R/%l/%a/%N%C%S:\
%R/%l/%a/%N%S:\
%R/%a/%N%C%S:\
%R/%a/%N%S:\
%R/%N%C%S:\
%R/%N%S\
"
#else /* VMS */
#define XMTFILESEARCHPATH "\
%R/%a/%N%S:\
%R/%a/%N:\
%R/%N%S:\
%R/%N:\
xmt_root/%a/%N%S:\
xmt_root/%a/%N:\
%a_root/%T/%N%S:\
%a_root/%T/%N:\
%a_root/%N%S:\
%a_root/%N:\
%a_%T/%N%S:\
%a_%T/%N:\
"
#endif /* VMS */
#endif

static char default_path[] = XMTFILESEARCHPATH;

#ifdef DECWINDOWS
static void def_cursor_proc(
#if NeedFunctionPrototypes
Widget w, int offset, XrmValue *val
#endif
);
#endif /* DECWINDOWS */


#define offset(field) XtOffsetOf(XmtAppResources, field)
static XtResource xmt_app_resource_list1[] = {
{XmtNhelpFile, XmtCHelpFile, XtRString,
     sizeof(String), offset(help_file),
     XtRString, NULL},
{XmtNcontextHelpFile, XmtCContextHelpFile, XtRString,
     sizeof(String), offset(context_help_file),
     XtRString, NULL},
{XmtNconfigDir, XmtCConfigDir, XtRString,
     sizeof(String), offset(config_dir),
     XtRString, XMTDEFAULTCONFIGDIR},
{XmtNconfigPath, XmtCConfigPath, XtRString,
     sizeof(String), offset(config_path),
     XtRString, default_path},
{XmtNuserConfigPath, XmtCConfigPath, XtRString,
     sizeof(String), offset(user_config_path),
     XtRString, default_path},
{XmtNresourceFilePath, XmtCResourceFilePath, XtRString,
     sizeof(String), offset(resource_file_path),
     XtRString, NULL},
{XmtNpixmapFilePath, XmtCPixmapFilePath, XtRString,
     sizeof(String), offset(pixmap_file_path),
     XtRString, NULL},
{XmtNbitmapFilePath, XmtCBitmapFilePath, XtRString,
     sizeof(String), offset(bitmap_file_path),
     XtRString, NULL},
{XmtNhelpFilePath, XmtCHelpFilePath, XtRString,
     sizeof(String), offset(help_file_path),
     XtRString, NULL},
{XmtNcolorTable, XmtCColorTable, XmtRXmtColorTable,
     sizeof(XmtColorTable), offset(colortable),
     XmtRXmtColorTable, NULL},
{XmtNdefaultColorTable, XmtCDefaultColorTable, XmtRXmtColorTable,
     sizeof(XmtColorTable), offset(default_colortable),
     XmtRXmtColorTable, NULL},
};

/*
 * Converting these resources may require the color table
 * resource defined in the above list.
 */
static XtResource xmt_app_resource_list2[] = {
{XtNforeground, XtCForeground, XtRPixel,
     sizeof(Pixel), offset(foreground),
     XtRCallProc, (XtPointer)_XmForegroundColorDefault},
{XtNbackground, XtCBackground, XtRPixel,
     sizeof(Pixel), offset(background),
     XtRCallProc, (XtPointer)_XmBackgroundColorDefault},
{XmtNreverseVideo, XmtCReverseVideo, XtRBoolean,
     sizeof(Boolean), offset(reverse_video),
     XtRImmediate, False},
};

/*
 * Converting these resources may require the colors
 * defined in the above resource list, and the file
 * paths and config directories defined in the first
 * resource list.
 */
static XtResource xmt_app_resource_list3[] = {
{XmtNcursor, XmtCCursor, XtRCursor,
     sizeof(Cursor), offset(cursor),
     XtRImmediate, (XtPointer)None},
{XmtNcursorForeground, XmtCCursorForeground, XtRPixel,
     sizeof(Pixel), offset(cursor_foreground),
     XtRString, (XtPointer)"#000"},
{XmtNcursorBackground, XmtCCursorBackground, XtRPixel,
     sizeof(Pixel), offset(cursor_background),
     XtRString, (XtPointer)"#ffffffffffff"},
{XmtNcontextHelpPixmap, XmtCContextHelpPixmap, XtRPixmap,
     sizeof(Pixmap), offset(help_pixmap),
     XtRImmediate, (XtPointer) None},
{XmtNhelpCursor, XmtCHelpCursor, XtRCursor,
     sizeof(Cursor), offset(help_cursor),
     XtRString, "question_arrow"},
{XmtNhelpCursorForeground, XmtCCursorForeground, XtRPixel,
     sizeof(Pixel), offset(help_cursor_foreground),
     XtRString, (XtPointer)"#000"},
{XmtNhelpCursorBackground, XmtCCursorBackground, XtRPixel,
     sizeof(Pixel), offset(help_cursor_background),
     XtRString, (XtPointer)"#ffffffffffff"},
#ifndef DECWINDOWS
{XmtNbusyCursor, XmtCBusyCursor, XtRCursor,
     sizeof(Cursor), offset(busy_cursor),
     XtRString, "watch"},
#else /* DECWINDOWS */
/* use the dec windows watch, not the X11 watch */
{XmtNbusyCursor, XmtCBusyCursor, XtRCursor,
     sizeof(Cursor), offset(busy_cursor),
     XtRCallProc, def_cursor_proc},
#endif /* DECWINDOWS */
{XmtNbusyCursorForeground, XmtCCursorForeground, XtRPixel,
     sizeof(Pixel), offset(busy_cursor_foreground),
     XtRString, (XtPointer)"#000"},
{XmtNbusyCursorBackground, XmtCCursorBackground, XtRPixel,
     sizeof(Pixel), offset(busy_cursor_background),
     XtRString, (XtPointer)"#ffffffffffff"},
};
#undef offset

static XrmOptionDescRec xmt_options_list[] = {
{"-helpFile", ".helpFile", XrmoptionSepArg, NULL},
{"-contextHelpFile", ".contextHelpFile", XrmoptionSepArg, NULL},
{"-helpCursor", ".helpCursor", XrmoptionSepArg, NULL},
{"-busyCursor", ".busyCursor", XrmoptionSepArg, NULL},
{"-contexthelpPixmap", ".contextHelpPixmap", XrmoptionSepArg, NULL},
{"-configDir", ".configDir", XrmoptionSepArg, NULL},
{"-configPath", ".configPath", XrmoptionSepArg, NULL},
{"-userConfigPath", ".userConfigPath", XrmoptionSepArg, NULL},
{"-resourceFilePath", ".resourceFilePath", XrmoptionSepArg, NULL},
{"-pixmapFilePath", ".pixmapFilePath", XrmoptionSepArg, NULL},
{"-bitmapFilePath", ".bitmapFilePath", XrmoptionSepArg, NULL},
{"-helpFilePath", ".helpFilePath", XrmoptionSepArg, NULL},
{"-colorTable", ".colorTable", XrmoptionSepArg, NULL},
{"-cursor", ".cursor", XrmoptionSepArg, NULL},
{"-CursorForeground", ".CursorForeground", XrmoptionSepArg, NULL},
{"-CursorBackground", ".CursorBackground", XrmoptionSepArg, NULL},
{"-cursorForeground", ".cursorForeground", XrmoptionSepArg, NULL},
{"-cursorBackground", ".cursorBackground", XrmoptionSepArg, NULL},
{"-helpCursorForeground", ".helpCursorForeground", XrmoptionSepArg, NULL},
{"-helpCursorBackground", ".helpCursorBackground", XrmoptionSepArg, NULL},
{"-busyCursorForeground", ".busyCursorForeground", XrmoptionSepArg, NULL},
{"-busyCursorBackground", ".busyCursorBackground", XrmoptionSepArg, NULL},
{"-fontFamily", ".fontFamily", XrmoptionSepArg, NULL},
{"-palette", ".palette", XrmoptionSepArg, NULL},
};

/* XPM */
static char *context_help_image[] = {
"32 48 6 1",
"  	m None",
"X	s background m white",
":	s foreground m black",
".	s top_shadow m black",
"#	s bottom_shadow m black",
"o	s select m black",
"                                ",
"          ..oooooo##            ",
"       ..oooooooooooo##         ",
"     ..oooooooooooooooo##       ",
"    ..oooooooooooooooooo##      ",
"   ..oooooooooooooooooooo##     ",
"  ..oooooooooooooooooooooo##    ",
"  ..oooooooo##   ..oooooooo##   ",
" ..ooooooo##       ..ooooooo##  ",
" ..ooooo##           ..ooooo##  ",
"..ooooo##            ..oooooo## ",
"..ooooo##             ..ooooo## ",
"..ooooo##             ..ooooo## ",
"..ooooo##             ..ooooo## ",
"..ooooo##             ..ooooo## ",
" ..ooooo##            ..ooooo## ",
" ..oooooo##           ..ooooo## ",
" ..oooooo##          ..ooooo##  ",
"  ..oooooo##        ..oooooo##  ",
"   ..ooo###        ..oooooo##   ",
"   ..oo##         ..oooooo##    ",
"    .##         ..ooooooo##     ",
"             ..ooooooooo##      ",
"          ..ooooooooooo##       ",
"          ..oooooooooo##        ",
"          ..ooooooooo##         ",
"          ..o####.oo##          ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"          ..o#   .o##           ",
"   .........o#   .o##......#    ",
"   ..oooooooo#   .oooooooo##    ",
"   ..oooo#####   .####oooo##    ",
"    ..ooo#           .ooo##     ",
"     ..ooo#         .ooo##      ",
"      ..ooo#       .ooo##       ",
"       ..ooo#     .ooo##        ",
"        ..ooo#   .ooo##         ",
"         ..ooo# .ooo##          ",
"          ..ooo#ooo##           ",
"           ..ooooo##            ",
"            ..ooo##             ",
"             ..o##              ",
"              ..#               "};



static XmtHashTable app_resource_table = NULL;
static XmtHashTable root_name_table = NULL;

/* ARGSUSED */
#if NeedFunctionPrototypes
static void Destroy(Widget w, XtPointer tag, XtPointer data)
#else
static void Destroy(w, tag, data)
Widget w;
XtPointer tag;
XtPointer data;
#endif
{
    XmtAppResources *app_resources = (XmtAppResources *)tag;

    /* remove hash table entries */
    XmtHashTableDelete(app_resource_table, (XtPointer) w);
    XmtHashTableDelete(root_name_table, (XtPointer)w->core.xrm_name);

    /* release the help_pixmap if it was explicitly allocated */
    if (app_resources->free_help_pixmap)
	XmtReleasePixmap(w, app_resources->help_pixmap);

    /* destroy the help database if it was explicitly created */
    if (app_resources->help_database && app_resources->free_help_database)
	XrmDestroyDatabase(app_resources->help_database);

    /* Free copied strings */
    XtFree(app_resources->help_file);
    XtFree(app_resources->context_help_file);
    XtFree(app_resources->config_dir);
    XtFree(app_resources->config_path);
    XtFree(app_resources->user_config_path);
    XtFree(app_resources->resource_file_path);
    XtFree(app_resources->pixmap_file_path);
    XtFree(app_resources->bitmap_file_path);
    XtFree(app_resources->help_file_path);

    /* free the structure itself */
    XtFree((char *)app_resources);
}

#if NeedFunctionPrototypes
void XmtParseCommandLine(Widget w, int *argc, char **argv)
#else
void XmtParseCommandLine(w, argc, argv)
Widget w;
int *argc;
char **argv;
#endif
{
    String name, class;
    XrmDatabase db;

    XtGetApplicationNameAndClass(XtDisplay(w), &name, &class);
    db = XmtDatabaseOfWidget(w);
    XrmParseCommand(&db, xmt_options_list, XtNumber(xmt_options_list),
		    name, argc, argv);
}


#if NeedFunctionPrototypes
void XmtInitializeApplicationShell(Widget w, ArgList args, Cardinal num_args)
#else
void XmtInitializeApplicationShell(w, args, num_args)
Widget w;
ArgList args;
Cardinal num_args;
#endif
{
    XmtAppResources *app_resources;
    XrmDatabase db = XmtDatabaseOfWidget(w);
    static XmtHashTable per_database_table = NULL;
    XmtColorTable standard_colortable;
    XColor colors[2];

    /* one-time initialization of the hash tables */
    if (app_resource_table == NULL) app_resource_table = XmtHashTableCreate(2);
    if (root_name_table == NULL) root_name_table = XmtHashTableCreate(2);
    if (per_database_table == NULL) per_database_table = XmtHashTableCreate(2);
    
    /* find the root shell widget */
    while(XtParent(w)) w = XtParent(w);

    /* store it by name */
    XmtHashTableStore(root_name_table,
		      (XtPointer) w->core.xrm_name,
		      (XtPointer) w);

    /* create the struct, initializing everything to 0, False, NULL */
    app_resources = (XmtAppResources *) XtCalloc(1, sizeof(XmtAppResources));

    /*
     * store the struct in the hash table.
     * The reason we store the struct now, rather than once we've
     * initialized it is that when we go to get the app resources,
     * we might invoke the ColorTable converter which calls
     * XmtGetApplicationResources.  If we haven't registered this struct
     * yet, that function calls this one and we recurse.  The Pixel and
     * Pixmap converters can cause this recursion too.
     */
    XmtHashTableStore(app_resource_table,
		      (XtPointer)w, (XtPointer)app_resources);

    /* register a callback to remove the struct when the shell is destroyed */
    XtAddCallback(w, XtNdestroyCallback, Destroy, (XtPointer)app_resources);

    /*
     * We've got to look up the application resources in several separate
     * batches, because the converters for some of them depend on values
     * for the others already being in place.
     *
     * First, look up the simple resources with no dependencies.  These
     * are strings, booleans, etc.  And the colortable.
     */
    /* look the resources up in the db, overriding with args */
    XtGetApplicationResources(w, (XtPointer) app_resources,
			      xmt_app_resource_list1,
			      XtNumber(xmt_app_resource_list1),
			      args, num_args);

    /*
     * make copies of the string resources--these are not guaranteed to
     * remain constant as the application overloads the db with other
     * resource files.
     */
#define Copy(field)\
    if (app_resources->field) \
	 app_resources->field = XtNewString(app_resources->field)
    Copy(help_file);
    Copy(context_help_file);
    Copy(config_dir);
    Copy(config_path);
    Copy(user_config_path);
    Copy(resource_file_path);
    Copy(pixmap_file_path);
    Copy(bitmap_file_path);
    Copy(help_file_path);
#undef Copy       

    /*
     * Now, look up the second batch, which include the foreground
     * and background colors.  These might make use of the colortable
     * just parsed above.  With these foreground and background colors,
     * we can augment the color table with some other standard colors.
     */
    XtGetApplicationResources(w, (XtPointer) app_resources,
			      xmt_app_resource_list2,
			      XtNumber(xmt_app_resource_list2),
			      args, num_args);


    /* if reverse video, exchange foreground & background */
    if (app_resources->reverse_video) {
	Pixel dummy = app_resources->foreground;
	app_resources->foreground = app_resources->background;
	app_resources->background = dummy;
    }

    /*
     * figure out the HSL values for the background and foreground
     * colors, for use with delta HSL specifications.
     */
    colors[0].pixel = app_resources->background;
    colors[1].pixel = app_resources->foreground;
    XQueryColors(XtDisplay(w), w->core.colormap, colors, 2);
    XmtRGBToHSL(colors[0].red, colors[0].green, colors[0].blue,
		&app_resources->background_hue,
		&app_resources->background_saturation,
		&app_resources->background_lightness);
    XmtRGBToHSL(colors[1].red, colors[1].green, colors[1].blue,
		&app_resources->foreground_hue,
		&app_resources->foreground_saturation,
		&app_resources->foreground_lightness);

    /*
     * An app can have three color tables in a chain:
     * 1) a "standard" colortable, with XmtRegisterStandardColors()
     *    called on it.  This will be the default table, if no color table
     *    resources are specified.
     * 2) a "default" color table, which the application should set its
     *    colors in.  The defaultColorTable resource
     * 3) a "user" color table, which the user can set with the colorTable
     *    resource or the -colorTable argument.
     */
    
    /* set up the "standard" color table */
    standard_colortable = XmtCreateColorTable(NULL);
    XmtRegisterStandardColors(standard_colortable, w,
			      app_resources->foreground,
			      app_resources->background);

    /* set up the chain of color tables */
    if (app_resources->colortable) {
	if (app_resources->default_colortable) {
	    XmtColorTableSetParent(app_resources->colortable,
				   app_resources->default_colortable);
	    XmtColorTableSetParent(app_resources->default_colortable,
				   standard_colortable);
	}
	else {
	    XmtColorTableSetParent(app_resources->colortable,
				   standard_colortable);
	}
    }
    else {
	if (app_resources->default_colortable) {
	    XmtColorTableSetParent(app_resources->default_colortable,
				   standard_colortable);
	    app_resources->colortable = app_resources->default_colortable;
	}
	else
	{
	    app_resources->colortable = standard_colortable;
	}
    }

    /*
     * Now that the colortable is set up, with standard colors
     * registered in it, we can go and look up any other Pixel,
     * Pixmap, Bitmap, and Cursor resources.  Note that the Pixmap
     * and Bitmap converters also rely on the config path and
     * config dir resources looked up in the first batch.
     */
    XtGetApplicationResources(w, (XtPointer) app_resources,
			      xmt_app_resource_list3,
			      XtNumber(xmt_app_resource_list3),
			      args, num_args);

    /*
     * If there is a cursor, set its colors.
     * Note that we don't set the cursor anywhere--you've got
     * to do that yourself after you realize each of your shell widgets.
     */
    if (app_resources->cursor) {
	XColor colors[2];
	colors[0].pixel = app_resources->cursor_foreground;
	colors[1].pixel = app_resources->cursor_background;
	XQueryColors(XtDisplay(w), w->core.colormap, colors, 2);
	XRecolorCursor(XtDisplay(w), app_resources->cursor,
		       &colors[0], &colors[1]);
    }

    /*
     * And set colors for the other cursors, too.
     */
    if (app_resources->busy_cursor) {
	XColor colors[2];
	colors[0].pixel = app_resources->busy_cursor_foreground;
	colors[1].pixel = app_resources->busy_cursor_background;
	XQueryColors(XtDisplay(w), w->core.colormap, colors, 2);
	XRecolorCursor(XtDisplay(w), app_resources->busy_cursor,
		       &colors[0], &colors[1]);
    }
    if (app_resources->help_cursor) {
	XColor colors[2];
	colors[0].pixel = app_resources->help_cursor_foreground;
	colors[1].pixel = app_resources->help_cursor_background;
	XQueryColors(XtDisplay(w), w->core.colormap, colors, 2);
	XRecolorCursor(XtDisplay(w), app_resources->help_cursor,
		       &colors[0], &colors[1]);
    }
    

    /* if no help_pixmap, get the default one */
    if (app_resources->help_pixmap == None) {
	static Boolean help_image_registered = False;
	XmtImage *help_image;

	if (!help_image_registered) {
	    help_image = XmtParseXpmData(context_help_image);
	    XmtRegisterImage("_xmt_context_help_image", help_image);
	    help_image_registered = True;
	}
	/* can't call XmtLookupWidgetPixmap here because it would cause
	 * an infinite recursion with this function.
	 */
	app_resources->help_pixmap =
	    XmtLookupPixmap(w, XmtGetVisual(w),
			    w->core.colormap, w->core.depth,
			    app_resources->colortable, 
			    "_xmt_context_help_image");
	app_resources->free_help_pixmap = True;
    }
    else app_resources->free_help_pixmap = False;


    /* set the help_database to NULL.  Will read it when needed. */
    app_resources->help_database = NULL;
    app_resources->free_help_database = False;

    /* get the application shell name and class */
    app_resources->application_name = w->core.name;
    if (XtIsApplicationShell(w))
	app_resources->application_class =
	    ((ApplicationShellWidget)w)->application.class;
    else
	app_resources->application_class =
	    w->core.widget_class->core_class.class_name;

    /* create a hash table to store per-appshell, per-screen info */
    app_resources->screen_table = XmtHashTableCreate(2);

    /* lookup or create the loaded_files_table hash table.
     * There is one of these per resource db (per screen or display)
     * Not all app shells have a unique one.
     */
    if (!XmtHashTableLookup(per_database_table, (XtPointer) db,
			    (XtPointer *)&app_resources->loaded_files_table)) {
	app_resources->loaded_files_table = XmtHashTableCreate(2);
	XmtHashTableStore(per_database_table,
			  (XtPointer) db,
			  (XtPointer) app_resources->loaded_files_table);
    }
}


#if NeedFunctionPrototypes
XmtAppResources *XmtGetApplicationResources(Widget w)
#else
XmtAppResources *XmtGetApplicationResources(w)
Widget w;
#endif
{
    XmtAppResources *app_resources;
    if (!w) return NULL;
    
    /* find the root shell widget */
    while(XtParent(w)) w = XtParent(w);

    /*
     * look up the resources for this widget in the hash table.
     * if they're not there, initialize the shell and look again.
     */
    if (!app_resource_table ||
	!XmtHashTableLookup(app_resource_table,
			    (XtPointer)w, (XtPointer *)&app_resources)) {
	XmtInitializeApplicationShell(w, NULL, (Cardinal) 0);
	(void)XmtHashTableLookup(app_resource_table,
				 (XtPointer)w, (XtPointer *)&app_resources);
    }

    return app_resources;
}

#if NeedFunctionPrototypes
void XmtSetApplicationValues(Widget w, ArgList args, Cardinal num_args)
#else
void XmtSetApplicationValues(w, args, num_args)
Widget w;
ArgList args;
Cardinal num_args;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);

    XtSetSubvalues((XtPointer)app,
		   xmt_app_resource_list1, XtNumber(xmt_app_resource_list1),
		   args, num_args);
    XtSetSubvalues((XtPointer)app,
		   xmt_app_resource_list2, XtNumber(xmt_app_resource_list2),
		   args, num_args);
    XtSetSubvalues((XtPointer)app,
		   xmt_app_resource_list3, XtNumber(xmt_app_resource_list3),
		   args, num_args);
}


#if NeedFunctionPrototypes
void XmtGetApplicationValues(Widget w, ArgList args, Cardinal num_args)
#else
void XmtGetApplicationValues(w, args, num_args)
Widget w;
ArgList args;
Cardinal num_args;
#endif
{
    XmtAppResources *app = XmtGetApplicationResources(w);

    XtGetSubvalues((XtPointer)app,
		   xmt_app_resource_list1, XtNumber(xmt_app_resource_list1),
		   args, num_args);
    XtGetSubvalues((XtPointer)app,
		   xmt_app_resource_list2, XtNumber(xmt_app_resource_list2),
		   args, num_args);
    XtGetSubvalues((XtPointer)app,
		   xmt_app_resource_list3, XtNumber(xmt_app_resource_list3),
		   args, num_args);
}


#if NeedFunctionPrototypes
Widget XmtLookupApplicationShell(XrmQuark name)
#else
Widget XmtLookupApplicationShell(name)
XrmQuark name;
#endif
{
    Widget shell;
    
    if (XmtHashTableLookup(root_name_table,
			   (XtPointer) name,
			   (XtPointer *) &shell))
	return shell;
    else
	return NULL;
}

#ifdef DECWINDOWS
/*
 * Procedure to create a DECwindows watch cursor. This procedure is designed as
 * a resource default procedure, and is registered as the resource default
 * procedure for the busyCursor application resource. It will create a DEC
 * watch cursor and return it so that the intrinsics will install it as the
 * value of the busyCursor resource. The DECwindows watch cursor is
 * significantly different from the MIT watch cursor, and DECwindows users
 * might very well not recognize what the MIT cursor signifies.
 */

#if NeedFunctionPrototypes
static void def_cursor_proc(Widget w, int offset, XrmValue *val)
#else
static void def_cursor_proc(w, offset, val)
Widget w;
int offset;
XrmValue *val;
#endif
{

    static Cursor watch = 0;

    if(!watch)watch = DXmCreateCursor(w, 4);
    val->size = sizeof(watch);
    val->addr = &watch;
}
#endif /* DECWINDOWS */
