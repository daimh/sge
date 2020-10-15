/* 
 * Motif Tools Library, Version 3.1
 * $Id: WidgetType.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: WidgetType.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtWidgetType_h
#define _XmtWidgetType_h

typedef Widget (*XmtWidgetConstructor)(
#if NeedFunctionPrototypes
				       Widget, String, ArgList, Cardinal
#endif
				       );
typedef void (*XmtSetValueProc)(
#if NeedFunctionPrototypes
				Widget, XtPointer, XrmQuark, Cardinal
#endif
				);
typedef void (*XmtGetValueProc)(
#if NeedFunctionPrototypes
				Widget, XtPointer, XrmQuark, Cardinal
#endif
				);

typedef struct {
    String name;
    WidgetClass wclass;
    XmtWidgetConstructor constructor;
    XmtSetValueProc set_value_proc;
    XmtGetValueProc get_value_proc;
    int popup;
} XmtWidgetType;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern void XmtRegisterWidgetClass(StringConst, WidgetClass);
extern void XmtRegisterWidgetConstructor(StringConst, XmtWidgetConstructor);
extern void XmtRegisterPopupClass(StringConst, WidgetClass);
extern void XmtRegisterPopupConstructor(StringConst, XmtWidgetConstructor);
extern void XmtRegisterWidgetTypes(XmtWidgetType *, Cardinal);
extern XmtWidgetType *XmtLookupWidgetType(StringConst);
extern Widget XmtCreateWidgetType(StringConst, XmtWidgetType*, Widget,
				  ArgList, Cardinal);
extern void XmtRegisterMotifWidgets(void);
extern void XmtRegisterXmtWidgets(void);
extern void XmtRegisterXmText(void);
extern void XmtRegisterXmScrolledText(void);
extern void XmtRegisterXmTextField(void);
extern void XmtRegisterXmToggleButton(void);
extern void XmtRegisterXmScale(void);
#if XmVersion >= 2000
extern void XmtRegisterXmComboBox(void);
#endif
#else
extern void XmtRegisterWidgetClass();
extern void XmtRegisterWidgetConstructor();
extern void XmtRegisterPopupClass();
extern void XmtRegisterPopupConstructor();
extern void XmtRegisterWidgetTypes();
extern XmtWidgetType *XmtLookupWidgetType();
extern Widget XmtCreateWidgetType();
extern void XmtRegisterMotifWidgets();
extern void XmtRegisterXmtWidgets();
extern void XmtRegisterXmText();
extern void XmtRegisterXmScrolledText();
extern void XmtRegisterXmTextField();
extern void XmtRegisterXmToggleButton();
extern void XmtRegisterXmScale();

#if XmVersion >= 2000
extern void XmtRegisterXmComboBox();
#endif

#endif

#if NeedVarargsPrototypes
extern void XmtVaRegisterWidgetClasses(StringConst, WidgetClass, ...);
extern void XmtVaRegisterWidgetConstructors(StringConst,
					    XmtWidgetConstructor,...);
#else
extern void XmtVaRegisterWidgetClasses();
extern void XmtVaRegisterWidgetConstructors();
#endif
_XFUNCPROTOEND

#endif
