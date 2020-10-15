/* 
 * Motif Tools Library, Version 3.1
 * $Id: Xpm.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Xpm.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

/*
 * This file is derived in part from the XPM 3.0 distribution by
 * Arnaud Le Hors.  See the file COPYRIGHT for the Groupe Bull copyright.
 */

#ifndef _XmtXpm_h
#define _XmtXpm_h

#include <Xmt/Color.h>

/*
 * the visual types we recognize colors for.
 */
typedef enum {
    Mono=0, Gray4=1, Gray=2, Color=3
} XmtXpmVisualClass;

#define NVISUALS 4

typedef struct {
    String symbolic_name;    /* a symbolic name for this color, quarkified */
    String default_colors[NVISUALS];  /* a color string for each Visual type */
} XmtXpmColor, *XmtXpmColorTable;

/*
 * structure containing data related to an Xpm pixmap.
 * Each pixel of the image is stored as a byte in the data field.
 * The values in this char array are indexes into the color table.
 * We use chars to conserve space, which means that images cannot
 * contain more than 256 colors.  Since this package is intended for
 * use for UI icons and the like, this is not an unreasonable
 * restriction.
 */
typedef struct {
    unsigned short width;
    unsigned short  height;
    XPoint hotspot;
    Boolean has_hotspot;
    unsigned short ncolors;
    XmtXpmColorTable color_table;
    unsigned char *data;
} XmtImage;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern XmtImage *XmtParseXpmFile(StringConst);
extern XmtImage *XmtParseXpmData(String *);
extern XmtImage *XmtParseXpmString(StringConst);
extern void XmtFreeXmtImage(XmtImage *);
extern Boolean XmtCreateXImageFromXmtImage(Widget, Visual *, Colormap,
				    unsigned int, XmtColorTable, XmtImage *,
				    XImage **, XImage **, Pixel **, int *);
extern Boolean XmtCreatePixmapFromXmtImage(Widget, Drawable, Visual *,
				    Colormap, unsigned int, XmtColorTable,
				    XmtImage *, Pixmap *, Pixmap *,
				    Pixel **, int *);
#else
extern XmtImage *XmtParseXpmFile();
extern XmtImage *XmtParseXpmData();
extern XmtImage *XmtParseXpmString();
extern void XmtFreeXmtImage();
extern Boolean XmtCreateXImageFromXmtImage();
extern Boolean XmtCreatePixmapFromXmtImage();
#endif
_XFUNCPROTOEND

#endif  /* _XmtXpm_h */
