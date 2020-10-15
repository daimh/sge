/* 
 * Motif Tools Library, Version 3.1
 * $Id: Color.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Color.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtColor_h
#define _XmtColor_h

typedef struct {
    StringConst symbolic_name;
    StringConst color_name;
} XmtColorPair;

typedef struct _XmtColorTableRec *XmtColorTable;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtHSLToRGB(unsigned, unsigned, unsigned,
			unsigned *, unsigned *, unsigned *);
extern void XmtRGBToHSL(unsigned, unsigned, unsigned,
			unsigned *, unsigned *, unsigned *);
extern int XmtAllocColor(Widget, Colormap, Visual *, XmtColorTable,
			 StringConst, Pixel *);
extern int XmtAllocWidgetColor(Widget, StringConst, Pixel *);
extern void XmtFreeColor(Widget, Colormap, Pixel);
extern void XmtFreeWidgetColor(Widget, Pixel);
extern int XmtStoreColor(Widget, Colormap, Visual *, XmtColorTable,
			 StringConst, Pixel);
extern int XmtStoreWidgetColor(Widget, StringConst, Pixel);
extern XmtColorTable XmtCreateColorTable(XmtColorTable);
extern void XmtDestroyColorTable(XmtColorTable);
extern void XmtColorTableSetParent(XmtColorTable, XmtColorTable);
extern XmtColorTable XmtColorTableGetParent(XmtColorTable);
extern void XmtRegisterColor(XmtColorTable, StringConst, StringConst);
extern void XmtRegisterColors(XmtColorTable, XmtColorPair *, Cardinal);
extern void XmtRegisterPixel(XmtColorTable, StringConst, Display *,
			     Colormap, Pixel);
extern void XmtRegisterStandardColors(XmtColorTable, Widget, Pixel, Pixel);
extern String XmtLookupColorName(XmtColorTable, StringConst);
#else
extern void XmtHSLToRGB();
extern void XmtRGBToHSL();
extern int XmtAllocColor();
extern int XmtAllocWidgetColor();
extern void XmtFreeColor();
extern void XmtFreeWidgetColor();
extern int XmtStoreColor();
extern int XmtStoreWidgetColor();
extern XmtColorTable XmtCreateColorTable();
extern void XmtDestroyColorTable();
extern void XmtColorTableSetParent();
extern XmtColorTable XmtColorTableGetParent();
extern void XmtRegisterColor();
extern void XmtRegisterColors();
extern void XmtRegisterPixel();
extern void XmtRegisterStandardColors();
extern String XmtLookupColorName();
#endif

#if NeedVarargsPrototypes
extern void XmtVaRegisterColors(XmtColorTable, ...);
#else
extern void XmtVaRegisterColors();
#endif

_XFUNCPROTOEND


#endif
