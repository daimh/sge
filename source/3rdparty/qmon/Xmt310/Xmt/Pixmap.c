/* 
 * Motif Tools Library, Version 3.1
 * $Id: Pixmap.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Pixmap.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Pixmap.h>
#include <Xmt/Xpm.h>
#include <Xmt/Hash.h>
#include <Xmt/Color.h>
#include <Xmt/AppResP.h>
#include <X11/IntrinsicP.h>

static XmtHashTable image_cache = NULL;
static XmtHashTable pixmap_to_image_table = NULL;

typedef enum {
    PixmapImage, PixmapMask
} PixmapType;

typedef struct _PixmapRec {
    PixmapType type;
    Pixmap pixmap;
    Screen *screen;
    Visual *visual;
    Colormap colormap;
    unsigned int depth;
    XmtColorTable table;
    short refcount;
    Pixel *allocated_pixels;    /* free these pixels when done w/ pixmap */
    short num_allocated_pixels; 
    struct _PixmapRec *next;
    struct _ImageCacheEntry *listhead;
} PixmapRec, *PixmapList;

typedef enum {
    ImageTypeXbm, ImageTypeXpm
} ImageType;    

typedef struct {
    char *imagedata;
    char *maskdata;
    unsigned short width, height;
    XPoint hotspot;
} XbmImage;

typedef struct _ImageCacheEntry {
    ImageType type;
    union {
	XbmImage *xbm;
	XmtImage *xpm;
    } data;
    Boolean orphan;
    PixmapList list;
} ImageCacheEntry;

#if NeedFunctionPrototypes
static void FreeImageCacheEntry(ImageCacheEntry *ie)
#else
static void FreeImageCacheEntry(ie)
ImageCacheEntry *ie;
#endif
{
    if (ie->type == ImageTypeXbm) {
	XtFree((char *)ie->data.xbm);
    }
    XtFree((char *)ie);
}

#if NeedFunctionPrototypes
static void RegisterImage(StringConst name, ImageType type, XtPointer data)
#else
static void RegisterImage(name, type, data)
StringConst name;
ImageType type;
XtPointer data;
#endif
{
    XrmQuark q = XrmStringToQuark(name);
    ImageCacheEntry *ie;
    
    if (image_cache == NULL) {
	image_cache = XmtHashTableCreate(3);
	pixmap_to_image_table = XmtHashTableCreate(3);
    }

    /*
     * if there is already an image by that name, free the old image
     * if possible, or make it an orphan so it gets cleaned up when
     * all of its outstanding pixmaps have been released.
     */
    if (XmtHashTableLookup(image_cache, (XtPointer)q, (XtPointer *)&ie)) {
	if (ie->list == NULL) FreeImageCacheEntry(ie);
	else ie->orphan = True;
    }

    ie = XtNew(ImageCacheEntry);
    ie->type = type;
    if (type == ImageTypeXbm) ie->data.xbm = (XbmImage *) data;
    else ie->data.xpm = (XmtImage *)data;
    ie->orphan = False;
    ie->list = NULL;
    XmtHashTableStore(image_cache, (XtPointer)q, (XtPointer)ie);
}


#if NeedFunctionPrototypes
void XmtRegisterXbmData(StringConst name, char *imagedata, char *maskdata,
			int width, int height, int hotspot_x, int hotspot_y)
#else
void XmtRegisterXbmData(name, imagedata, maskdata, width, height,
			hotspot_x, hotspot_y)
StringConst name;
char *imagedata;
char *maskdata;
int width;
int height;
int hotspot_x;
int hotspot_y;
#endif
{
    XbmImage *data;

    data = XtNew(XbmImage);
    data->imagedata = imagedata;
    data->maskdata = maskdata;
    data->width = width;
    data->height = height;
    data->hotspot.x = hotspot_x;
    data->hotspot.y = hotspot_y;
    
    RegisterImage(name, ImageTypeXbm, (XtPointer)data);
}
    
#if NeedFunctionPrototypes
void XmtRegisterImage(StringConst name, XmtImage *data)
#else
void XmtRegisterImage(name, data)
StringConst name;
XmtImage *data;
#endif
{
    RegisterImage(name, ImageTypeXpm, (XtPointer)data);
}


#if NeedFunctionPrototypes
Pixmap XmtLookupBitmap(Widget widget, StringConst name)
#else
Pixmap XmtLookupBitmap(widget, name)
Widget widget;
StringConst name;
#endif
{
    Screen *screen = XtScreenOfObject(widget);
    ImageCacheEntry *ie;
    PixmapRec *p;

    if (image_cache == NULL) return None;

    /* if nothing by that name, return None */
    if (!XmtHashTableLookup(image_cache, (XtPointer)XrmStringToQuark(name),
			    (XtPointer *)&ie))
	return None;

    /* if it is not bitmap data, return None*/
    if (ie->type != ImageTypeXbm) return None;

    /* if there is no image data, return None */
    if (ie->data.xbm->imagedata == NULL) return None;

    /* go see if a 1-bit image pixmap has  been created for this screen */
    for (p=ie->list; p; p = p->next) {
	if ((p->type == PixmapImage) &&
	    (p->screen == screen) &&
	    (p->depth == 1)) {
	    p->refcount++;
	    return p->pixmap;
	}
    }

    /* otherwise create it and add to the list */
    p = XtNew(PixmapRec);
    p->type = PixmapImage;
    p->pixmap = XCreateBitmapFromData(DisplayOfScreen(screen),
				      RootWindowOfScreen(screen),
				      ie->data.xbm->imagedata,
				      ie->data.xbm->width,
				      ie->data.xbm->height);
    p->screen = screen;
    p->visual = NULL;
    p->colormap = None;
    p->depth = 1;
    p->table = NULL;
    p->refcount = 1;
    p->allocated_pixels = NULL;
    p->num_allocated_pixels = 0;
    p->listhead = ie;
    p->next = ie->list;
    ie->list = p;

    /*
     * map the bitmap back to its list entry, so that we
     * can decrement its reference count when done with it.
     */
    XmtHashTableStore(pixmap_to_image_table,
		      (XtPointer) p->pixmap, (XtPointer)p);

    return p->pixmap;
}


#if NeedFunctionPrototypes
Pixmap XmtLookupPixmap(Widget widget, Visual *visual, Colormap colormap,
		       unsigned int depth, XmtColorTable table,
		       StringConst name)
#else
Pixmap XmtLookupPixmap(widget, visual, colormap, depth, table, name)
Widget widget;
Visual *visual;
Colormap colormap;
unsigned int depth;
XmtColorTable table;
StringConst name;
#endif
{
    Screen *screen = XtScreenOfObject(widget);
    ImageCacheEntry *ie;
    PixmapRec *p;
    Boolean status;
    Pixmap pixmap;
    Pixel fg, bg;
    Pixel *pixels;
    int num_pixels;

    if (image_cache == NULL) return None;

    /* if nothing by that name, return None */
    if (!XmtHashTableLookup(image_cache, (XtPointer)XrmStringToQuark(name),
			    (XtPointer *)&ie))
	return None;

    /* if xbm format with no image data, return None */
    if (ie->type == ImageTypeXbm && ie->data.xbm->imagedata == NULL)
	return None;

    /* go see if an appropriate Pixmap is already created */
    for (p=ie->list; p; p = p->next) {
	if ((p->type == PixmapImage) &&
	    (p->screen == screen) &&
	    (p->visual == visual) &&
	    (p->colormap == colormap) &&
	    (p->depth == depth) &&
	    (p->table == table)) {
	    p->refcount++;
	    return p->pixmap;
	}
    }

    /* otherwise go create it */
    if (ie->type == ImageTypeXpm) {
	status = XmtCreatePixmapFromXmtImage(widget,
					     RootWindowOfScreen(screen),
					     visual, colormap, depth, table,
					     ie->data.xpm,
					     &pixmap, NULL,
					     &pixels,
					     &num_pixels);
	if (!status) 
	    return None;
    }
    else {
	if (XmtAllocColor(widget, colormap, visual, table,
			  "+$foreground", &fg) ||
	    XmtAllocColor(widget, colormap, visual, table,
			  "-$background", &bg)) {
	    XmtWarningMsg("XmtLookupPixmap", "color",
			  "bitmap '%s'\n\tforeground and/or background colors are not defined in colortable,\n\tor are invalid colors, or the colormap is full.",
			  name);
	    fg = (Pixel)1;
	    bg = (Pixel)0;
	}
	pixmap = XCreatePixmapFromBitmapData(DisplayOfScreen(screen),
					     RootWindowOfScreen(screen),
					     ie->data.xbm->imagedata,
					     ie->data.xbm->width,
					     ie->data.xbm->height,
					     fg, bg, depth);
	pixels = NULL;
	num_pixels = 0;
    }
    
    p = XtNew(PixmapRec);
    p->type = PixmapImage;
    p->pixmap = pixmap;
    p->screen = screen;
    p->visual = visual;
    p->colormap = colormap;
    p->depth = depth;
    p->table = table;
    p->refcount = 1;
    p->allocated_pixels = pixels;
    p->num_allocated_pixels = num_pixels;
    p->listhead = ie;
    p->next = ie->list;
    ie->list = p;

    /*
     * map the pixmap back to its list entry, so that we
     * can decrement its reference count when done with it.
     */
    XmtHashTableStore(pixmap_to_image_table,
		      (XtPointer) p->pixmap, (XtPointer)p);

    return p->pixmap;
}


#if NeedFunctionPrototypes
Pixmap XmtLookupBitmask(Widget widget, StringConst name)
#else
Pixmap XmtLookupBitmask(widget, name)
Widget widget;
StringConst name;
#endif
{
    Screen *screen = XtScreenOfObject(widget);
    ImageCacheEntry *ie;
    PixmapRec *p;
    Boolean status;
    Pixmap pixmap;

    if (image_cache == NULL) return None;

    /* if nothing by that name, return None */
    if (!XmtHashTableLookup(image_cache, (XtPointer)XrmStringToQuark(name),
			    (XtPointer *)&ie))
	return None;

    /* if xbm format with no mask data, return None */
    if (ie->type == ImageTypeXbm && ie->data.xbm->maskdata == NULL)
	return None;

    /* go see if an appropriate Pixmap is already created */
    for (p=ie->list; p; p = p->next) {
	if ((p->type == PixmapMask) &&
	    (p->screen == screen)) {
	    p->refcount++;
	    return p->pixmap;
	}
    }

    /* otherwise go create one */
    if (ie->type == ImageTypeXbm) {
	pixmap = XCreateBitmapFromData(DisplayOfScreen(screen),
				       RootWindowOfScreen(screen),
				       ie->data.xbm->maskdata,
				       ie->data.xbm->width,
				       ie->data.xbm->height);
    }
    else {
	status = XmtCreatePixmapFromXmtImage(widget,
					     RootWindowOfScreen(screen),
					     NULL, None, 1, NULL,
					     ie->data.xpm, NULL, &pixmap,
					     NULL, NULL);
	if (!status || pixmap == None) return None;
    }

    p = XtNew(PixmapRec);
    p->type = PixmapMask;
    p->pixmap = pixmap;
    p->screen = screen;
    p->visual = NULL;
    p->colormap = None;
    p->depth = 1;
    p->table = NULL;
    p->refcount = 1;
    p->allocated_pixels = NULL;
    p->num_allocated_pixels = 0;
    p->listhead = ie;
    p->next = ie->list;
    ie->list = p;

    /*
     * map the pixmap back to its list entry, so that we
     * can decrement its reference count when done with it.
     */
    XmtHashTableStore(pixmap_to_image_table,
		      (XtPointer) p->pixmap, (XtPointer)p);

    return p->pixmap;
}


#if NeedFunctionPrototypes
Pixmap XmtLookupSimplePixmap(Widget widget, XmtColorTable table,
			     StringConst name)
#else
Pixmap XmtLookupSimplePixmap(widget, table, name)
Widget widget;
XmtColorTable table;
StringConst name;
#endif
{
    while(!XtIsWidget(widget)) widget = XtParent(widget);
    return XmtLookupPixmap(widget,
			   XmtGetVisual(widget),
			   widget->core.colormap,
			   widget->core.depth,
			   table, name);
}


#if NeedFunctionPrototypes
Pixmap XmtLookupWidgetPixmap(Widget w, StringConst name)
#else
Pixmap XmtLookupWidgetPixmap(w, name)
Widget w;
StringConst name;
#endif
{
    XmtAppResources *app;

    while(!XtIsWidget(w)) w = XtParent(w);
    app = XmtGetApplicationResources(w);
    return XmtLookupPixmap(w,
			   XmtGetVisual(w),
			   w->core.colormap,
			   w->core.depth,
			   app->colortable, name);
}


#if NeedFunctionPrototypes
void XmtReleasePixmap(Widget widget, Pixmap pixmap)
#else
void XmtReleasePixmap(widget, pixmap)
Widget widget;
Pixmap pixmap;
#endif
{
    PixmapRec *p, *q;
    ImageCacheEntry *ie;
    
    if (pixmap_to_image_table == NULL) return;

    if (!XmtHashTableLookup(pixmap_to_image_table, (XtPointer)pixmap,
			    (XtPointer *)&p))
	return;

    /* decrement the refcount */
    p->refcount--;

    /*
     * if the refcount is 0, remove the hash table entry, 
     * free the pixmap, unlink the record,
     * free any allocated pixels, and free the record.
     */
    if (p->refcount == 0) {
	XmtHashTableDelete(pixmap_to_image_table, (XtPointer)pixmap);
	XFreePixmap(DisplayOfScreen(p->screen), p->pixmap);
	ie = p->listhead;
	if (p == ie->list) ie->list = p->next;
	else {
	    for(q = ie->list; q->next != p; q = q->next);
	    q->next = p->next;
	}
	if (p->num_allocated_pixels) {
	    int i;
	    for(i=0; i < p->num_allocated_pixels; i++)
		XmtFreeColor(widget, p->colormap, p->allocated_pixels[i]);
	    XtFree((char *)p->allocated_pixels);
	}
	XtFree((char *) p);

	/*
	 * if that was the last pixmap for this cache entry, and this
	 * cache entry has been orphaned, (ie. not bound to a symbolic
	 * name in the hash table anymore) free the entry.
	 */
	if (ie->orphan && ie->list == NULL)
	    FreeImageCacheEntry(ie);
    }
	
}

