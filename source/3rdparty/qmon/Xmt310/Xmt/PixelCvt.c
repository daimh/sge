/* 
 * Motif Tools Library, Version 3.1
 * $Id: PixelCvt.c,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: PixelCvt.c,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#include <Xmt/Xmt.h>
#include <Xmt/AppResP.h>
#include <Xmt/ConvertersP.h>
#include <X11/IntrinsicP.h>

/*
 * A replacement for XtCvtStringToPixel that uses XmtAllocColor()
 */
/* ARGSUSED */
#if NeedFunctionPrototypes
Boolean XmtConvertStringToPixel(Display *dpy,
				XrmValue *args, Cardinal *num_args,
				XrmValue *from, XrmValue *to,
				XtPointer *closure_return)
#else
Boolean XmtConvertStringToPixel(dpy, args, num_args, from, to, closure_return)
Display *dpy;
XrmValue *args;
Cardinal *num_args;
XrmValue *from;
XrmValue *to;
XtPointer *closure_return;
#endif
{
    String	    str = (String)from->addr;
    Widget          rootshell = *((Widget *)args[0].addr);
    Colormap        colormap = *((Colormap *)args[1].addr);
    Visual          *visual = *((Visual **)args[2].addr);
    int  	    status;
    Pixel           pixel;

    status = XmtAllocColor(rootshell, colormap, visual, NULL, str, &pixel);

    if (!status)
	done(Pixel, pixel)
    else {
	XtDisplayStringConversionWarning(dpy, str, XtRPixel);
	if (status == 1)
	    XmtWarningMsg("XmtConvertStringToPixel", "parse",
			  "malformed or unrecognized color name '%s'.",
			  str);
	else
	    XmtWarningMsg("XmtConvertStringToPixel", "alloc",
			  "can't allocate color '%s'; colormap full.",
			  str);
	return False;
    }
}

/* ARGSUSED */
#if NeedFunctionPrototypes
static void FreeConvertedPixel(XtAppContext app, XrmValue *to,
			       XtPointer closure,
			       XrmValue *args, Cardinal *num_args)
#else
static void FreeConvertedPixel(app, to, closure, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer closure;
XrmValue *args;
Cardinal *num_args;
#endif
{
    Widget   rootshell = *((Widget *)args[0].addr);
    Colormap colormap = *((Colormap *)args[1].addr);
    Pixel    pixel = *((Pixel *)to->addr);

    XmtFreeColor(rootshell, colormap, pixel);
}

static XtConvertArgRec pixel_args[] = {
    {XtProcedureArg, (XtPointer)_XmtFetchRootWidget, 0},
    {XtWidgetBaseOffset,
	 (XtPointer)XtOffsetOf(WidgetRec, core.colormap),
	 sizeof(Colormap)},
    {XtProcedureArg, (XtPointer)_XmtFetchVisual, 0},
};

#if NeedFunctionPrototypes
void XmtRegisterPixelConverter(void)
#else
void XmtRegisterPixelConverter()
#endif
{
    static Boolean registered = False;

    if (!registered) {
	XtSetTypeConverter(XtRString, XtRPixel,
			   XmtConvertStringToPixel,
			   pixel_args, XtNumber(pixel_args),
			   XtCacheByDisplay | XtCacheRefCount,
			   FreeConvertedPixel);
	registered = True;
    }
}



