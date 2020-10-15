/* 
 * Motif Tools Library, Version 3.1
 * $Id: BitmapCvt.c,v 1.1 2001-07-18 11:06:01 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: BitmapCvt.c,v $
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
#include <Xmt/Pixmap.h>
#include <Xmt/ConvertersP.h>


/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToBitmap(Display *dpy,
				 XrmValue *args, Cardinal *num_args,
				 XrmValue *from, XrmValue *to,
				 XtPointer *data)
#else
Boolean XmtConvertStringToBitmap(dpy, args, num_args, from, to, data)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *data;
#endif
{
    String str = from->addr;
    Pixmap bitmap;

    bitmap = XmtGetBitmap(*(Widget *)args[0].addr, str);
    if (!bitmap) {
	XtDisplayStringConversionWarning(dpy, str, XtRBitmap);
	return False;
    }
    done(Pixmap, bitmap)
}

/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToBitmask(Display *dpy,
				 XrmValue *args, Cardinal *num_args,
				 XrmValue *from, XrmValue *to,
				 XtPointer *data)
#else
Boolean XmtConvertStringToBitmask(dpy, args, num_args, from, to, data)
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
    Pixmap bitmask;

    /*
     * First, go see if there is a bitmask registered in the cache by
     * this name.  This will be the case if you've previously called
     * XmtRegisterXbmData().  More commonly, it will be the case if
     * the Pixmap converter has previously been invoked to read in an
     * XPM file.  Note that this call does not go out looking for an XPM
     * file, it just extracts a mask from a previously found file.  This
     * means that we've got to be sure that pixmap resoruces are queried
     * before bitmask resources are.  We should be able to do this just
     * by arranging them correctly.
     *
     * If there is no bitmask already by this name, then assume that
     * the string names a bitmap file, and go find that file for use as
     * a mask.
     */
    bitmask = XmtLookupBitmask(w, str);

    if (bitmask == None)
	bitmask = XmtGetBitmap(w, str);
    
    if (bitmask == None) {
	XtDisplayStringConversionWarning(dpy, str, XmtRBitmask);
	return False;
    }
    done(Pixmap, bitmask)
}


/* ARGSUSED */
#if NeedFunctionPrototypes
static void FreeConvertedBitmap(XtAppContext app, XrmValue *to,
				XtPointer closure,
				XrmValue *args, Cardinal *num_args)
#else
static void FreeConvertedBitmap(app, to, closure, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer closure;
XrmValue *args;
Cardinal *num_args;
#endif
{
    XmtReleasePixmap(*(Widget *)args[0].addr, *((Pixmap *) to->addr));
}


#if NeedFunctionPrototypes
void XmtRegisterBitmaskConverter(void)
#else
void XmtRegisterBitmaskConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	XtSetTypeConverter(XtRString, XmtRBitmask,
			   XmtConvertStringToBitmask,
			   _XmtWidgetConvertArg, (Cardinal) 1,
			   XtCacheNone | XtCacheRefCount,
			   FreeConvertedBitmap);
	registered = True;
    }
}

#if NeedFunctionPrototypes
void XmtRegisterBitmapConverter(void)
#else
void XmtRegisterBitmapConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	XtSetTypeConverter(XtRString, XtRBitmap,
			   XmtConvertStringToBitmap,
			   _XmtWidgetConvertArg, (Cardinal) 1,
			   XtCacheNone | XtCacheRefCount,
			   FreeConvertedBitmap);
	registered = True;
    }
}
