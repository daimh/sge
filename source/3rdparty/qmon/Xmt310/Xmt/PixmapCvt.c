/* 
 * Motif Tools Library, Version 3.1
 * $Id: PixmapCvt.c,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: PixmapCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/Pixmap.h>
#include <Xmt/ConvertersP.h>

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToPixmap(Display *dpy,
				 XrmValue *args, Cardinal *num_args,
				 XrmValue *from, XrmValue *to,
				 XtPointer *data)
#else
Boolean XmtConvertStringToPixmap(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *data;
#endif
{
    String str = from->addr;
    Widget w = *(Widget *)args[0].addr;
    Pixmap pixmap;

    pixmap = XmtGetPixmap(w, NULL, str);
    if (!pixmap) {
	XtDisplayStringConversionWarning(dpy, str, XtRPixmap);
	return False;
    }
    done(Pixmap, pixmap)
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void FreeConvertedPixmap(XtAppContext app, XrmValue *to,
				XtPointer closure,
				XrmValue *args, Cardinal *num_args)
#else
static void FreeConvertedPixmap(app, to, closure, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer closure;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XmtReleasePixmap(*(Widget *)args[0].addr, *((Pixmap *) to->addr));
}

static String pixmap_types[] = {
    XtRPixmap,
    XmRPrimForegroundPixmap,
    XmRManForegroundPixmap,
    XmRGadgetPixmap
#ifndef LESSTIF_VERSION
#if XmVersion >= 2000
    ,
    XmRLargeIconPixmap ,
    XmRSmallIconPixmap,
    XmRDynamicPixmap
#endif
#endif
    
};

#if NeedFunctionPrototypes
void XmtRegisterPixmapConverter(void)
#else
void XmtRegisterPixmapConverter()
#endif
{
    static Boolean registered = False;
    int i;
#if NeedFunctionPrototypes
    extern void _XmRegisterPixmapConverters(void);
#else
    extern void _XmRegisterPixmapConverters();
#endif

    if (!registered) {
	/*
	 * The Motif widgets define a different representation type for
	 * different kinds of Pixmaps, depending on the desired foreground
	 * color.  They obviously didn't have multi-plane Pixmaps in mind
	 * like I do.  Most of those rep. types (eg. XmRPrimTopShadowPixmap
	 * are only used by the XmPrimitive and XmManager classes, and only
	 * used for setting shadow pixmaps.  We can leave these alone, and it
	 * means that you can't have a multi-plane shadow pixmap.  No big loss.
	 * I do have to register this new converter for three other types
	 * besides XmRPixmap, however, so that labels, buttons, gadgets, and
	 * the XmMessageBox can have multi-plane pixmaps.
	 *
	 * Note that the first thing we do is force the Motif pixmap converters
	 * to be registered, so that they won't be registered over these
	 * converters when the first Motif widget is created
	 */
	_XmRegisterPixmapConverters();
	for(i=0; i < XtNumber(pixmap_types); i++)
	    XtSetTypeConverter(XtRString, pixmap_types[i],
			       XmtConvertStringToPixmap,
			       _XmtWidgetConvertArg, (Cardinal) 1,
			       XtCacheNone | XtCacheRefCount,
			       FreeConvertedPixmap);
	registered = True;
    }
}

