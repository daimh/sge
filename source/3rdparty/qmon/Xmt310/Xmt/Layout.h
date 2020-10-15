/* 
 * Motif Tools Library, Version 3.1
 * $Id: Layout.h,v 1.1 2001-07-18 11:06:02 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: Layout.h,v $
 * Revision 1.1  2001-07-18 11:06:02  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtLayout_h
#define _XmtLayout_h

#include <Xm/BulletinB.h>

typedef struct _XmtLayoutClassRec*       XmtLayoutWidgetClass;
typedef struct _XmtLayoutRec*            XmtLayoutWidget;

externalref WidgetClass xmtLayoutWidgetClass;

#include <Xmt/LayoutG.h>

typedef enum {
    XmtLayoutFilled=0,
    XmtLayoutFlushLeft=1,
    XmtLayoutFlushTop=1,
    XmtLayoutFlushRight=2,
    XmtLayoutFlushBottom=2,
    XmtLayoutCentered=3
} XmtLayoutJustification;

typedef enum {
    XmtLayoutTop,
    XmtLayoutBottom,
    XmtLayoutLeft,
    XmtLayoutRight
} XmtLayoutEdge;

typedef enum {
    XmtLayoutFrameNone,
    XmtLayoutFrameBox,
    XmtLayoutFrameLeft,
    XmtLayoutFrameRight,
    XmtLayoutFrameTop,
    XmtLayoutFrameBottom
} XmtLayoutFrameType;

typedef enum {
    XmtLayoutFrameShadowIn,
    XmtLayoutFrameShadowOut,
    XmtLayoutFrameEtchedIn,
    XmtLayoutFrameEtchedOut,
    XmtLayoutFrameSingleLine,
    XmtLayoutFrameDoubleLine
} XmtLayoutFrameLineType;

typedef enum {
    XmtLayoutFrameInside,
    XmtLayoutFrameThrough,
    XmtLayoutFrameOutside
} XmtLayoutFramePosition;

/*
 * This enumerated type is used by the XmtLayoutBox gadget to control
 * how it spaces items in a row or column
 */
typedef enum {
    XmtLayoutSpaceNone,
    XmtLayoutSpaceEven,
    XmtLayoutSpaceLREven,
    XmtLayoutSpaceInterval,
    XmtLayoutSpaceLCR,
    XmtLayoutSpaceLTabbed,
    XmtLayoutSpaceCTabbed,  /* thanks to F. Sullivan Segal for this one */
    XmtLayoutSpaceRTabbed
} XmtLayoutSpaceType;

typedef enum {
    XmtLayoutPoints,       /* 1/72 of an inch */
    XmtLayoutInches,       /* 25.4 millimeters */
    XmtLayoutMillimeters,  /* depends on display resolution */
    XmtLayoutEms,          /* width of 'M' in widget font */
    XmtLayoutEns           /* 1/2 of an em */
} XmtLayoutUnitType;

externalref _Xconst char XmtLayoutStrings[];
#ifndef XmtNbackground
#define XmtNbackground ((char*)&XmtLayoutStrings[0])
#endif
#ifndef XmtNbitmap
#define XmtNbitmap ((char*)&XmtLayoutStrings[11])
#endif
#ifndef XmtNbitmask
#define XmtNbitmask ((char*)&XmtLayoutStrings[18])
#endif
#ifndef XmtNdebugLayout
#define XmtNdebugLayout ((char*)&XmtLayoutStrings[26])
#endif
#ifndef XmtNdefaultSpacing
#define XmtNdefaultSpacing ((char*)&XmtLayoutStrings[38])
#endif
#ifndef XmtNequal
#define XmtNequal ((char*)&XmtLayoutStrings[53])
#endif
#ifndef XmtNfont
#define XmtNfont ((char*)&XmtLayoutStrings[59])
#endif
#ifndef XmtNfontList
#define XmtNfontList ((char*)&XmtLayoutStrings[64])
#endif
#ifndef XmtNforeground
#define XmtNforeground ((char*)&XmtLayoutStrings[73])
#endif
#ifndef XmtNitemStretch
#define XmtNitemStretch ((char*)&XmtLayoutStrings[84])
#endif
#ifndef XmtNlabel
#define XmtNlabel ((char*)&XmtLayoutStrings[96])
#endif
#ifndef XmtNlabelString
#define XmtNlabelString ((char*)&XmtLayoutStrings[102])
#endif
#ifndef XmtNlayout
#define XmtNlayout ((char*)&XmtLayoutStrings[114])
#endif
#ifndef XmtNlayoutAfter
#define XmtNlayoutAfter ((char*)&XmtLayoutStrings[121])
#endif
#ifndef XmtNlayoutAllowResize
#define XmtNlayoutAllowResize ((char*)&XmtLayoutStrings[133])
#endif
#ifndef XmtNlayoutBefore
#define XmtNlayoutBefore ((char*)&XmtLayoutStrings[151])
#endif
#ifndef XmtNlayoutCaption
#define XmtNlayoutCaption ((char*)&XmtLayoutStrings[164])
#endif
#ifndef XmtNlayoutCaptionAlignment
#define XmtNlayoutCaptionAlignment ((char*)&XmtLayoutStrings[178])
#endif
#ifndef XmtNlayoutCaptionJustification
#define XmtNlayoutCaptionJustification ((char*)&XmtLayoutStrings[201])
#endif
#ifndef XmtNlayoutCaptionMargin
#define XmtNlayoutCaptionMargin ((char*)&XmtLayoutStrings[228])
#endif
#ifndef XmtNlayoutCaptionPosition
#define XmtNlayoutCaptionPosition ((char*)&XmtLayoutStrings[248])
#endif
#ifndef XmtNlayoutFrameLineType
#define XmtNlayoutFrameLineType ((char*)&XmtLayoutStrings[270])
#endif
#ifndef XmtNlayoutFrameMargin
#define XmtNlayoutFrameMargin ((char*)&XmtLayoutStrings[290])
#endif
#ifndef XmtNlayoutFramePosition
#define XmtNlayoutFramePosition ((char*)&XmtLayoutStrings[308])
#endif
#ifndef XmtNlayoutFrameThickness
#define XmtNlayoutFrameThickness ((char*)&XmtLayoutStrings[328])
#endif
#ifndef XmtNlayoutFrameType
#define XmtNlayoutFrameType ((char*)&XmtLayoutStrings[349])
#endif
#ifndef XmtNlayoutHeight
#define XmtNlayoutHeight ((char*)&XmtLayoutStrings[365])
#endif
#ifndef XmtNlayoutIn
#define XmtNlayoutIn ((char*)&XmtLayoutStrings[378])
#endif
#ifndef XmtNlayoutJustification
#define XmtNlayoutJustification ((char*)&XmtLayoutStrings[387])
#endif
#ifndef XmtNlayoutMarginHeight
#define XmtNlayoutMarginHeight ((char*)&XmtLayoutStrings[407])
#endif
#ifndef XmtNlayoutMarginWidth
#define XmtNlayoutMarginWidth ((char*)&XmtLayoutStrings[426])
#endif
#ifndef XmtNlayoutPosition
#define XmtNlayoutPosition ((char*)&XmtLayoutStrings[444])
#endif
#ifndef XmtNlayoutSensitive
#define XmtNlayoutSensitive ((char*)&XmtLayoutStrings[459])
#endif
#ifndef XmtNlayoutShrinkability
#define XmtNlayoutShrinkability ((char*)&XmtLayoutStrings[475])
#endif
#ifndef XmtNlayoutStretchability
#define XmtNlayoutStretchability ((char*)&XmtLayoutStrings[495])
#endif
#ifndef XmtNlayoutWidth
#define XmtNlayoutWidth ((char*)&XmtLayoutStrings[516])
#endif
#ifndef XmtNorientation
#define XmtNorientation ((char*)&XmtLayoutStrings[528])
#endif
#ifndef XmtNpixmap
#define XmtNpixmap ((char*)&XmtLayoutStrings[540])
#endif
#ifndef XmtNspace
#define XmtNspace ((char*)&XmtLayoutStrings[547])
#endif
#ifndef XmtNspaceStretch
#define XmtNspaceStretch ((char*)&XmtLayoutStrings[553])
#endif
#ifndef XmtNspaceType
#define XmtNspaceType ((char*)&XmtLayoutStrings[566])
#endif
#ifndef XmtCBitmap
#define XmtCBitmap ((char*)&XmtLayoutStrings[576])
#endif
#ifndef XmtCBitmask
#define XmtCBitmask ((char*)&XmtLayoutStrings[583])
#endif
#ifndef XmtCDebugLayout
#define XmtCDebugLayout ((char*)&XmtLayoutStrings[591])
#endif
#ifndef XmtCDefaultSpacing
#define XmtCDefaultSpacing ((char*)&XmtLayoutStrings[603])
#endif
#ifndef XmtCEqual
#define XmtCEqual ((char*)&XmtLayoutStrings[618])
#endif
#ifndef XmtCItemStretch
#define XmtCItemStretch ((char*)&XmtLayoutStrings[624])
#endif
#ifndef XmtCLayout
#define XmtCLayout ((char*)&XmtLayoutStrings[636])
#endif
#ifndef XmtCLayoutAfter
#define XmtCLayoutAfter ((char*)&XmtLayoutStrings[643])
#endif
#ifndef XmtCLayoutAllowResize
#define XmtCLayoutAllowResize ((char*)&XmtLayoutStrings[655])
#endif
#ifndef XmtCLayoutBefore
#define XmtCLayoutBefore ((char*)&XmtLayoutStrings[673])
#endif
#ifndef XmtCLayoutCaption
#define XmtCLayoutCaption ((char*)&XmtLayoutStrings[686])
#endif
#ifndef XmtCLayoutCaptionAlignment
#define XmtCLayoutCaptionAlignment ((char*)&XmtLayoutStrings[700])
#endif
#ifndef XmtCLayoutCaptionJustification
#define XmtCLayoutCaptionJustification ((char*)&XmtLayoutStrings[723])
#endif
#ifndef XmtCLayoutCaptionMargin
#define XmtCLayoutCaptionMargin ((char*)&XmtLayoutStrings[750])
#endif
#ifndef XmtCLayoutCaptionPosition
#define XmtCLayoutCaptionPosition ((char*)&XmtLayoutStrings[770])
#endif
#ifndef XmtCLayoutFrameLineType
#define XmtCLayoutFrameLineType ((char*)&XmtLayoutStrings[792])
#endif
#ifndef XmtCLayoutFrameMargin
#define XmtCLayoutFrameMargin ((char*)&XmtLayoutStrings[812])
#endif
#ifndef XmtCLayoutFramePosition
#define XmtCLayoutFramePosition ((char*)&XmtLayoutStrings[830])
#endif
#ifndef XmtCLayoutFrameThickness
#define XmtCLayoutFrameThickness ((char*)&XmtLayoutStrings[850])
#endif
#ifndef XmtCLayoutFrameType
#define XmtCLayoutFrameType ((char*)&XmtLayoutStrings[871])
#endif
#ifndef XmtCLayoutHeight
#define XmtCLayoutHeight ((char*)&XmtLayoutStrings[887])
#endif
#ifndef XmtCLayoutIn
#define XmtCLayoutIn ((char*)&XmtLayoutStrings[900])
#endif
#ifndef XmtCLayoutJustification
#define XmtCLayoutJustification ((char*)&XmtLayoutStrings[909])
#endif
#ifndef XmtCLayoutMarginHeight
#define XmtCLayoutMarginHeight ((char*)&XmtLayoutStrings[929])
#endif
#ifndef XmtCLayoutMarginWidth
#define XmtCLayoutMarginWidth ((char*)&XmtLayoutStrings[948])
#endif
#ifndef XmtCLayoutPosition
#define XmtCLayoutPosition ((char*)&XmtLayoutStrings[966])
#endif
#ifndef XmtCLayoutSensitive
#define XmtCLayoutSensitive ((char*)&XmtLayoutStrings[981])
#endif
#ifndef XmtCLayoutShrinkability
#define XmtCLayoutShrinkability ((char*)&XmtLayoutStrings[997])
#endif
#ifndef XmtCLayoutStretchability
#define XmtCLayoutStretchability ((char*)&XmtLayoutStrings[1017])
#endif
#ifndef XmtCLayoutWidth
#define XmtCLayoutWidth ((char*)&XmtLayoutStrings[1038])
#endif
#ifndef XmtCPixmap
#define XmtCPixmap ((char*)&XmtLayoutStrings[1050])
#endif
#ifndef XmtCSpace
#define XmtCSpace ((char*)&XmtLayoutStrings[1057])
#endif
#ifndef XmtCSpaceStretch
#define XmtCSpaceStretch ((char*)&XmtLayoutStrings[1063])
#endif
#ifndef XmtCSpaceType
#define XmtCSpaceType ((char*)&XmtLayoutStrings[1076])
#endif
#ifndef XmtRXmtLayoutEdge
#define XmtRXmtLayoutEdge ((char*)&XmtLayoutStrings[1086])
#endif
#ifndef XmtRXmtLayoutFrameLineType
#define XmtRXmtLayoutFrameLineType ((char*)&XmtLayoutStrings[1100])
#endif
#ifndef XmtRXmtLayoutFramePosition
#define XmtRXmtLayoutFramePosition ((char*)&XmtLayoutStrings[1123])
#endif
#ifndef XmtRXmtLayoutFrameType
#define XmtRXmtLayoutFrameType ((char*)&XmtLayoutStrings[1146])
#endif
#ifndef XmtRXmtLayoutJustification
#define XmtRXmtLayoutJustification ((char*)&XmtLayoutStrings[1165])
#endif
#ifndef XmtRXmtLayoutSpaceType
#define XmtRXmtLayoutSpaceType ((char*)&XmtLayoutStrings[1188])
#endif

#if XmVersion >= 2000
#ifndef XmtNrenderTable
#define XmtNrenderTable "renderTable"
#endif
#endif

_XFUNCPROTOBEGIN
#if NeedFunctionPrototypes
extern Widget XmtCreateLayout(Widget, String, ArgList, Cardinal);
extern Widget XmtCreateLayoutDialog(Widget, String, ArgList, Cardinal);
extern void XmtRegisterLayoutParser(void);
extern void XmtRegisterLayoutCreateMethod(void);
extern void XmtLayoutDisableLayout(Widget);
extern void XmtLayoutEnableLayout(Widget);
extern int XmtLayoutConvertSizeToPixels(Widget, double, XmtLayoutUnitType);
#else
extern Widget XmtCreateLayout();
extern Widget XmtCreateLayoutDialog();
extern void XmtRegisterLayoutParser();
extern void XmtRegisterLayoutCreateMethod();
extern void XmtLayoutDisableLayout();
extern void XmtLayoutEnableLayout();
extern int XmtLayoutConvertSizeToPixels();
#endif
_XFUNCPROTOEND    

#endif /* _XmtLayout_h */
