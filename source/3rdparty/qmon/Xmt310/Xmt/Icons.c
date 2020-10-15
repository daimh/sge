/* 
 * Motif Tools Library, Version 3.1
 * $Id: Icons.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Icons.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Xpm.h>
#include <Xmt/AppResP.h>
#include <X11/IntrinsicP.h>

/* XPM */
static char *information [] = {
"20 48 6 1",
" 	m None",
"X	s background m white",
":	s foreground m black",
".	s top_shadow m black",
"#	s bottom_shadow m black",
"o	s select m black",
"                    ",
"                    ",
"       ....         ",
"     ..XoXo##       ",
"    ..XoXoXo##      ",
"   ..XoXoXoXo##     ",
"   ..oXoXoXoX##     ",
"   ..XoXoXoXo##     ",
"   ..oXoXoXoX##     ",
"   ..XoXoXoXo##     ",
"   ..oXoXoXoX##     ",
"    ..oXoXoX##      ",
"     ..oXoX##       ",
"      .#####        ",
"                    ",
"                    ",
"  ............      ",
" ..XoXoXoXoXo##     ",
" ..oXoXoXoXoX##     ",
" ..XoXoXoXoXo##     ",
"  ..XoXoXoXoX##     ",
"   ..XoXoXoXo##     ",
"    ..XoXoXoX##     ",
"     ..XoXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"      ..oXoXo##     ",
"      ..XoXoX##     ",
"   .....oXoXo#####  ",
" ..oXoXoXoXoXoXoX## ",
" ..XoXoXoXoXoXoXo## ",
" ..oXoXoXoXoXoXoX## ",
" ..XoXoXoXoXoXoXo## ",
" ..oXoXoXoXoXoXoX## ",
" ..XoXoXoXoXoXoXo## ",
"  .################ ",
"                    "
};

/* XPM */
static char *error [] = {
"48 48 6 1",
" 	m None",
"X	s background m white",
":	s foreground m black",
".	s top_shadow m black",
"#	s bottom_shadow m black",
"o	s select m black",
"               .................                ",
"              ...................               ",
"             ...XoXoXoXoXoXoXoXo##              ",
"            ...XoXoXoXoXoXoXoXoXo##             ",
"           ...XoXoXoXoXoXoXoXoXoXo##            ",
"          ...XoXoXoXoXoXoXoXoXoXoXo##           ",
"         ...XoXoXoXoXoXoXoXoXoXoXoXo##          ",
"        ...XoXoXoX############oXoXoXo##         ",
"       ...XoXoXoX#############.oXoXoXo##        ",
"      ...XoXoXoX###           ..oXoXoXo##       ",
"     ...XoXoXoX###             ..oXoXoXo##      ",
"    ...XoXoXoXo##               ..oXoXoXo##     ",
"   ...XoXoXoXoXo##               ..oXoXoXo##    ",
"  ...XoXoXoXoXoXo##               ..oXoXoXo##   ",
" ...XoXoXoXoXoXoXo##               ..oXoXoXo##  ",
" ..XoXoXoX#XoXoXoXo##               ..oXoXoXo## ",
" ..oXoXoX##..oXoXoXo##               ..oXoXoX## ",
" ..XoXoX##  ..oXoXoXo##               ..oXoXo## ",
" ..oXoXo##   ..oXoXoXo##              ..XoXoX## ",
" ..XoXoX##    ..oXoXoXo##             ..oXoXo## ",
" ..oXoXo##     ..oXoXoXo##            ..XoXoX## ",
" ..XoXoX##      ..oXoXoXo##           ..oXoXo## ",
" ..oXoXo##       ..oXoXoXo##          ..XoXoX## ",
" ..XoXoX##        ..oXoXoXo##         ..oXoXo## ",
" ..oXoXo##         ..oXoXoXo##        ..XoXoX## ",
" ..XoXoX##          ..oXoXoXo##       ..oXoXo## ",
" ..oXoXo##           ..oXoXoXo##      ..XoXoX## ",
" ..XoXoX##            ..oXoXoXo##     ..oXoXo## ",
" ..oXoXo##             ..oXoXoXo##    ..XoXoX## ",
" ..XoXoX##              ..oXoXoXo##   ..oXoXo## ",
" ..oXoXo##               ..oXoXoXo##  ..XoXoX## ",
" ..XoXoXo##               ..oXoXoXo##..XoXoXo## ",
" ..oXoXoXo##               ..oXoXoXoX#XoXoXoX## ",
"  ..oXoXoXo##               ..oXoXoXoXoXoXoX### ",
"   ..oXoXoXo##               ..oXoXoXoXoXoX###  ",
"    ..oXoXoXo##               ..oXoXoXoXoX###   ",
"     ..oXoXoXo##               ..oXoXoXoX###    ",
"      ..oXoXoXo##             ...XoXoXoX###     ",
"       ..oXoXoXo##           ...XoXoXoX###      ",
"        ..oXoXoXo##............XoXoXoX###       ",
"         ..oXoXoXo............XoXoXoX###        ",
"          ..oXoXoXoXoXoXoXoXoXoXoXoX###         ",
"           ..oXoXoXoXoXoXoXoXoXoXoX###          ",
"            ..oXoXoXoXoXoXoXoXoXoX###           ",
"             ..oXoXoXoXoXoXoXoXoX###            ",
"              ..oXoXoXoXoXoXoXoX###             ",
"               ..#################              ",
"                .################               "
};

/* XPM */
static char *warning [] = {
"16 48 6 1",
" 	m None",
"X	s background m white",
":	s foreground m black",
".	s top_shadow m black",
"#	s bottom_shadow m black",
"o	s select m black",
"      ....      ",
"    ..oXoX##    ",
"   ..oXoXoX##   ",
"  ..oXoXoXoX##  ",
"  ..XoXoXoXo##  ",
" ..XoXoXoXoXo## ",
" ..oXoXoXoXoX## ",
"..oXoXoXoXoXoX##",
"..XoXoXoXoXoXo##",
"..oXoXoXoXoXoX##",
"..XoXoXoXoXoXo##",
"..oXoXoXoXoXoX##",
"..XoXoXoXoXoXo##",
"..oXoXoXoXoXoX##",
"..XoXoXoXoXoXo##",
" ..XoXoXoXoXo## ",
" ..oXoXoXoXoX## ",
" ..XoXoXoXoXo## ",
"  ..XoXoXoXo##  ",
"  ..oXoXoXoX##  ",
"  ..XoXoXoXo##  ",
"  ..oXoXoXoX##  ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"    .#######    ",
"                ",
"                ",
"                ",
"      ....      ",
"    ..oXoX##    ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"  ..XoXoXoXo##  ",
"  ..oXoXoXoX##  ",
"  ..XoXoXoXo##  ",
"  ..oXoXoXoX##  ",
"   ..oXoXoX##   ",
"   ..XoXoXo##   ",
"    ..XoXo##    ",
"      ####      "
};


#if NeedFunctionPrototypes
void XmtRegisterImprovedIcons(Widget w, XmtColorTable ctable)
#else
void XmtRegisterImprovedIcons(w, ctable)
Widget w;
XmtColorTable ctable;
#endif
{
    XmtImage *ii, *ei, *wi;
    XImage *ixi, *exi, *wxi;
    Widget shell = XmtGetShell(w);
    Visual *visual = XmtGetVisual(shell);
    Colormap cmap = shell->core.colormap;
    int depth = shell->core.depth;
    XmtAppResources *appres;

    ii = XmtParseXpmData(information);
    ei = XmtParseXpmData(error);
    wi = XmtParseXpmData(warning);

    if (!ctable) {
	appres = XmtGetApplicationResources(shell);
	ctable = appres->colortable;
    }
    
    if (XmtCreateXImageFromXmtImage(shell, visual, cmap, depth, ctable,
				    ii, &ixi, NULL, NULL, NULL))
	XmInstallImage(ixi, "xm_information");

    if (XmtCreateXImageFromXmtImage(shell, visual, cmap, depth, ctable,
				    ei, &exi, NULL, NULL, NULL))
	XmInstallImage(exi, "xm_error");

    if (XmtCreateXImageFromXmtImage(shell, visual, cmap, depth, ctable,
				    wi, &wxi, NULL, NULL, NULL))
	XmInstallImage(wxi, "xm_warning");

    XmtFreeXmtImage(ii);
    XmtFreeXmtImage(ei);
    XmtFreeXmtImage(wi);
}
