/* 
 * Motif Tools Library, Version 3.1
 * $Id: Converters.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Converters.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */
#ifndef _XmtConverters_h
#define _XmtConverters_h

/*
 * Here we define some representation types for some of 
 * the things we have converters for.  
 */
externalref _Xconst char XmtConverterStrings[];
#ifndef XmtRStringList
#define XmtRStringList ((char*)&XmtConverterStrings[0])
#endif
#ifndef XmtRPixmapList
#define XmtRPixmapList ((char*)&XmtConverterStrings[11])
#endif
#ifndef XmtRXmtColorTable
#define XmtRXmtColorTable ((char*)&XmtConverterStrings[22])
#endif
#ifndef XmtRXmtButtonType
#define XmtRXmtButtonType ((char*)&XmtConverterStrings[36])
#endif
#ifndef XmtRBitmask
#define XmtRBitmask ((char*)&XmtConverterStrings[50])
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtRegisterEnumConverter(StringConst, String *, Cardinal *,
				     Cardinal, String *);
extern void XmtRegisterWidgetConverter(void);
extern void XmtRegisterCallbackConverter(void);
extern void XmtRegisterXmStringConverter(void);
extern void XmtRegisterStringListConverter(void);
extern void XmtRegisterMenuItemsConverter(void);
extern void XmtRegisterBitmapConverter(void);
extern void XmtRegisterBitmaskConverter(void);
extern void XmtRegisterPixmapConverter(void);
extern void XmtRegisterPixmapListConverter(void);
extern void XmtRegisterPixelConverter(void);
extern void XmtRegisterColorTableConverter(void);
extern void XmtRegisterXmFontListConverter(void);
#else
extern void XmtRegisterEnumConverter();
extern void XmtRegisterWidgetConverter();
extern void XmtRegisterCallbackConverter();
extern void XmtRegisterXmStringConverter();
extern void XmtRegisterStringListConverter();
extern void XmtRegisterMenuItemsConverter();
extern void XmtRegisterBitmapConverter();
extern void XmtRegisterBitmaskConverter();
extern void XmtRegisterPixmapConverter();
extern void XmtRegisterPixmapListConverter();
extern void XmtRegisterPixelConverter();
extern void XmtRegisterColorTableConverter();
extern void XmtRegisterXmFontListConverter();
#endif

#if NeedFunctionPrototypes
extern Boolean XmtConvertStringToEnum(Display *, XrmValue *, Cardinal *,
				      XrmValue *, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToStringList(Display *, XrmValue *, Cardinal *,
					    XrmValue *, XrmValue*, XtPointer*);
extern Boolean XmtConvertStringToWidget(Display *, XrmValue *, Cardinal *,
					XrmValue *, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToCallback(Display *, XrmValue *, Cardinal *,
					  XrmValue *, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToXmString(Display *, XrmValue *, Cardinal *,
					  XrmValue *, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToXmtMenuItems(Display *, XrmValue *, Cardinal*,
					     XrmValue*, XrmValue*, XtPointer*);
extern Boolean XmtConvertStringToBitmap(Display *, XrmValue *, Cardinal *,
					XrmValue *, XrmValue *, XtPointer *);
extern Boolean XmtConvertStringToBitmask(Display *, XrmValue *, Cardinal *,
					 XrmValue *, XrmValue *, XtPointer *);
extern Boolean XmtConvertStringToPixmap(Display *, XrmValue *, Cardinal *,
					XrmValue *, XrmValue *, XtPointer *);
extern Boolean XmtConvertStringToPixmapList(Display *, XrmValue *, Cardinal *,
					    XrmValue*, XrmValue*, XtPointer *);
extern Boolean XmtConvertStringToPixel(Display *, XrmValue *, Cardinal *,
				       XrmValue *, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToColorTable(Display *, XrmValue *, Cardinal *,
					    XrmValue*, XrmValue *, XtPointer*);
extern Boolean XmtConvertStringToXmFontList(Display *, XrmValue *, Cardinal *,
					    XrmValue*, XrmValue *, XtPointer*);
#else
extern Boolean XmtConvertStringToEnum();
extern Boolean XmtConvertStringToStringList();
extern Boolean XmtConvertStringToWidget();
extern Boolean XmtConvertStringToCallback();
extern Boolean XmtConvertStringToXmString();
extern Boolean XmtConvertStringToXmtMenuItems();
extern Boolean XmtConvertStringToBitmap();
extern Boolean XmtConvertStringToBitmask();
extern Boolean XmtConvertStringToPixmap();
extern Boolean XmtConvertStringToPixmapList();
extern Boolean XmtConvertStringToPixel();
extern Boolean XmtConvertStringToColorTable();
extern Boolean XmtConvertStringToXmFontList();
#endif
_XFUNCPROTOEND

#endif    
