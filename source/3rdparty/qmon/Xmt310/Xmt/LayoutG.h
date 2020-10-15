/* 
 * Motif Tools Library, Version 3.1
 * $Id: LayoutG.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: LayoutG.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLayoutG_h
#define _XmtLayoutG_h

typedef struct _XmtLayoutGadgetClassRec*    XmtLayoutGadgetClass;
typedef struct _XmtLayoutGadgetRec*         XmtLayoutGadget;
externalref WidgetClass xmtLayoutGadgetClass;

typedef struct _XmtLayoutBoxClassRec*    XmtLayoutBoxGadgetClass;
typedef struct _XmtLayoutBoxRec*         XmtLayoutBoxGadget;
externalref WidgetClass xmtLayoutBoxGadgetClass;

typedef struct _XmtLayoutStringClassRec* XmtLayoutStringGadgetClass;
typedef struct _XmtLayoutStringRec*      XmtLayoutStringGadget;
externalref WidgetClass xmtLayoutStringGadgetClass;

typedef struct _XmtLayoutPixmapClassRec* XmtLayoutPixmapGadgetClass;
typedef struct _XmtLayoutPixmapRec*      XmtLayoutPixmapGadget;
externalref WidgetClass xmtLayoutPixmapGadgetClass;

typedef struct _XmtLayoutSeparatorClassRec* XmtLayoutSeparatorGadgetClass;
typedef struct _XmtLayoutSeparatorRec*      XmtLayoutSeparatorGadget;
externalref WidgetClass xmtLayoutSeparatorGadgetClass;

typedef struct _XmtLayoutSpaceClassRec* XmtLayoutSpaceGadgetClass;
typedef struct _XmtLayoutSpaceRec*      XmtLayoutSpaceGadget;
externalref WidgetClass xmtLayoutSpaceGadgetClass;

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateLayoutBox(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutRow(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutCol(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutString(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutPixmap(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutSeparator(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutSpace(Widget, String, ArgList, Cardinal);
#else
extern Widget XmtCreateLayoutBox();
extern Widget XmtCreateLayoutRow();
extern Widget XmtCreateLayoutCol();
extern Widget XmtCreateLayoutString();
extern Widget XmtCreateLayoutPixmap();
extern Widget XmtCreateLayoutSeparator();
extern Widget XmtCreateLayoutSpace();
#endif
_XFUNCPROTOEND    

#endif
