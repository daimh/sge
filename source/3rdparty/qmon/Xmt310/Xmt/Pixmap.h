/* 
 * Motif Tools Library, Version 3.1
 * $Id: Pixmap.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Pixmap.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtPixmap_h
#define _XmtPixmap_h

#include <Xmt/Xpm.h>
#include <Xmt/Color.h>

/*
 * The names of special branches of the resource database
 * that XmtGetBitmap() and XmtGetPixmap() will search for
 * bitmap and pixmap definitions.
 */
#define XmtNxmtPixmaps		"_Pixmaps_"
#define XmtNxmtBitmaps		"_Bitmaps_"

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtRegisterXbmData(StringConst, char *, char *, int, int, int,int);
extern void XmtRegisterImage(StringConst, XmtImage *);
extern Pixmap XmtLookupBitmap(Widget, StringConst);
extern Pixmap XmtLookupPixmap(Widget, Visual *, Colormap, unsigned int,
			      XmtColorTable, StringConst);
extern Pixmap XmtLookupSimplePixmap(Widget, XmtColorTable, StringConst);
extern Pixmap XmtLookupWidgetPixmap(Widget, StringConst);
extern Pixmap XmtLookupBitmask(Widget, StringConst);
extern void XmtReleasePixmap(Widget, Pixmap);
extern Pixmap XmtGetBitmap(Widget, StringConst);
extern Pixmap XmtGetPixmap(Widget, XmtColorTable, StringConst);
extern void XmtRegisterImprovedIcons(Widget, XmtColorTable);
#else
extern void XmtRegisterXbmData();
extern void XmtRegisterImage();
extern Pixmap XmtLookupBitmap();
extern Pixmap XmtLookupPixmap();
extern Pixmap XmtLookupSimplePixmap();
extern Pixmap XmtLookupWidgetPixmap();
extern Pixmap XmtLookupBitmask();
extern void XmtReleasePixmap();
extern Pixmap XmtGetBitmap();
extern Pixmap XmtGetPixmap();
extern void XmtRegisterImprovedIcons();
#endif
_XFUNCPROTOEND

#endif
