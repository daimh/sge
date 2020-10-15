/* $Id: IconList.c,v 1.4 2006-11-30 13:29:21 joga Exp $ */
/*
 * Copyright 1996 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.    John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose. It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

/*
** IconList widget 
*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <Xm/XmP.h>
#include <Xm/ScrolledW.h>
#include <Xm/Frame.h>
#include <Xm/ScrollBar.h>

#if XmVersion > 1001
/* for Motif1.1 the needed stuff is hopefully in XmP.h */
#include <Xm/DrawP.h>
#include <Xm/PrimitiveP.h>
#else
#include <Xm/Traversal.h>
#endif

#ifdef LesstifVersion
#undef NeedWidePrototypes
#endif

#ifndef SOLARIS
#if XmVersion >= 2000
extern void _XmResizeObject(
                        Widget g,
#ifdef NeedWidePrototypes
                        int width,
                        int height,
                        int border_width) ;
#else
                        Dimension width,
                        Dimension height,
                        Dimension border_width) ;
#endif /* NeedWidePrototypes */
extern void _XmMoveObject(
                        Widget g,
#ifdef NeedWidePrototypes
                        int x,
                        int y) ;
#else
                        Position x,
                        Position y) ;
#endif /* NeedWidePrototypes */
extern void _XmDrawHighlight(
                        Display *display,
                        Drawable d,
                        GC gc,
#ifdef NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick,
#endif /* NeedWidePrototypes */
                        int line_style) ;

#endif
#endif

#include "DumbClip.h"
#include "IconListP.h"

#if XmVersion < 1002
#define XmInheritSecObjectCreate   ((XtInitProc) _XtInherit)
#define XmInheritBorderHighlight   ((XtWidgetProc) _XtInherit)
#define XmInheritBorderUnhighlight   ((XtWidgetProc) _XtInherit)

extern void _XmSelectColorDefault(Widget widget, int offset, XrmValue *value);

#endif

#if XmVersion >= 2000
extern void _XmSelectColorDefault(Widget widget, int offset, XrmValue *value);
#endif


#if NeedFunctionPrototypes

static void classInitialize(void);
static void initialize(Widget request, Widget new, ArgList _args, 
                        Cardinal *_numArgs);
static void expose(Widget ilw, XEvent *_event, Region _region);
static void resize(Widget ilw);
static void getGCsAndFonts(XmIconListWidget _ilw);

#if XmVersion > 1001
static XmNavigability widgetNavigable(Widget _w);
static void focusChange(Widget _w, XmFocusChange _fc);
#endif

static void destroy(XmIconListWidget _ilw);
static void drawElementBackground(XmIconListWidget _ilw, int _nelement, GC _gc);
static void drawIconListElement(XmIconListWidget _ilw, int _row, GC _backgroundGC, GC _highlightGC);
static void drawIconList(XmIconListWidget _ilw, XRectangle *_xrect);

static int yToElementRow(XmIconListWidget _ilw, Position _y);

static void arm(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);
static void selectItem(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);
static void activate(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);
static void armAndActivate(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);

static void up(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);
static void down(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs);

static void buttonMotionEH(Widget _w, XtPointer _data, XEvent *_event, Boolean *_cont);

/* static int getFirstVisibleElement(XmIconListWidget _ilw); */
static Boolean rowFullyVisible(XmIconListWidget _ilw, int _row, Boolean *_aboveTop);
static void moveFocusElement(XmIconListWidget _ilw, int _whence, XEvent *_event);
static void setFocusElement(XmIconListWidget _ilw, int _row, Boolean _toTop);
static void scrollToRow(XmIconListWidget _ilw, int _row, Boolean _jump, Boolean _toTop);
static void syncScrollBar(XmIconListWidget _ilw);
static void highlightElement(XmIconListWidget _ilw, int _row, GC _gc);
static void callCallbacks(XmIconListWidget _ilw, int _reason, XEvent *_event);
static XtGeometryResult queryGeometry(Widget ilw, XtWidgetGeometry *_request, 
                                       XtWidgetGeometry *_pref);

#else

static void classInitialize();
static void initialize();
static void expose();
static void resize();
static void getGCsAndFonts();

#if XmVersion > 1001
static XmNavigability widgetNavigable();
static void focusChange();
#endif

static void destroy();
static void drawElementBackground();
static void drawIconListElement();
static void drawIconList();

static int yToElementRow();

static void arm();
static void selectItem();
static void activate();
static void armAndActivate();

static void up();
static void down();

static void buttonMotionEH();

/* static int getFirstVisibleElement(); */
static Boolean rowFullyVisible();
static void moveFocusElement();
static void setFocusElement();
static void scrollToRow();
static void syncScrollBar();
static void highlightElement();
static void callCallbacks();
static XtGeometryResult queryGeometry();

#endif




#define SPACE 5

#define CORE(a) ((a)->core)
#define PRIM(a) ((a)->primitive)
#define ICON_LIST(a) ((a)->iconList)

#define WIDTH(a) ((a)->core.width)
#define HEIGHT(a) ((a)->core.height)
#define BORDER(a) ((a)->core.border_width)
#define BWIDTH(a) ((a)->core.width + 2 * (a)->core.border_width)
#define BHEIGHT(a) ((a)->core.height + 2 * (a)->core.border_width)

#if XmVersion > 1001
#define MOVE(a,b,c) _XmMoveObject((Widget)(a), (b), (c))
#define RESIZE(a,b,c,d) _XmResizeObject((Widget)(a), (b), (c), (d))
#else
#define MOVE(a,b,c) _XmMoveObject((RectObj)(a), (b), (c))
#define RESIZE(a,b,c,d) _XmResizeObject((RectObj)(a), (b), (c), (d))
#endif

#define HIGHLIGHT_GC(a) ((a)->primitive.highlight_GC)

#define BG(a) ((a)->core.background_pixel)
#define DEPTH(a) ((a)->core.depth)

#define COPY_GC(a) ((a)->iconList.copyGC)
#define THE_GC(a) ((a)->iconList.gc)
#define ERASE_GC(a) ((a)->iconList.eraseGC)
#define IGC(a) ((a)->iconList.insensitiveGC)
#define SELECT_GC(a) ((a)->iconList.selectGC)
#define SELECT(a) ((a)->iconList.selectColor)
#define FG(a) ((a)->primitive.foreground)
#define FL(a) ((a)->iconList.fontList)
#define THE_FONT(a) ((a)->iconList.font)
#define ELEMENTS(a) ((a)->iconList.elements)
#define FOCUS_EL(a) ((a)->iconList.focusElement)
#define NUM_ELEMENTS(a) ((a)->iconList.numberOfElements)
#define FULL_EL_HEIGHT(a) ((a)->iconList.fullElementHeight)
#define HT(a) ((a)->iconList.highlightThickness)
#define VIS_COUNT(a) ((a)->iconList.visibleItemCount)
#define MARG_W(a)  ((a)->iconList.marginWidth)
#define MARG_H(a) ((a)->iconList.marginHeight)
#define TEXT_HEIGHT(a) ((a)->iconList.textHeight)
#define TEXT_OFFSET(a) ((a)->iconList.textOffset)
#define ICON_HEIGHT(a) ((a)->iconList.iconHeight)
#define ICON_WIDTH(a) ((a)->iconList.iconWidth)
#define USING_BITMAPS(a) ((a)->iconList.usingBitmaps)
#define LAST_CLICK_TIME(a) ((a)->iconList.lastClickTime)
#define LAST_CLICK_ROW(a) ((a)->iconList.lastClickRow)
#define CLICK_COUNT(a) ((a)->iconList.clickCount)
#define CLIP(a) ((a)->iconList.clipWidget)
#define VBAR(a) ((a)->iconList.verticalScrollBarWidget)
#define ST(a) Prim_ShadowThickness(a)
#define START_ROW(a) ((a)->iconList.startRow)
#define END_ROW(a) ((a)->iconList.endRow)
#define SW(a) ((a)->iconList.scrolledWindowWidget)
#define HAS_FOCUS(a) ((a)->iconList.hasFocus)
#define CALL_CALLBACKS(a) ((a)->iconList.callCallbacks)

#define _MAX_(a,b) ((a) > (b) ? (a) : (b))

#define MODE(a,b) ((a)->request_mode & (b))

#define drawShadow(wid,tgc,bgc,x,y,w,h) \
   _XmDrawShadows(XtDisplay(wid), XtWindow (wid), \
         tgc, bgc, \
         x, y, w, h, \
         ST(wid), XmSHADOW_OUT);




#if XmVersion >= 1002

static char iconListTranslations[] = 
"<Btn1Down>,<Btn1Up>:Activate()\n\
<Btn1Down>:Arm()\n\
<Btn3Down>:SelectItem()\n\
~s ~m ~a <Key>Return:ArmAndActivate()\n\
~s ~m ~a <Key>space:ArmAndActivate() \n\
<Key>osfActivate:ArmAndActivate()\n\
<Key>osfSelect:ArmAndActivate()\n\
<Key>osfHelp: PrimitiveHelp()\n\
c <Key>osfUp:IconListTraverseUp(PAGE)\n\
c <Key>osfDown: IconListTraverseDown(PAGE)\n\
<Key>osfUp:IconListTraverseUp()\n\
<Key>osfDown:IconListTraverseDown()\n\
<Key>osfPageUp: IconListTraverseUp(PAGE)\n\
<Key>osfPageDown: IconListTraverseDown(PAGE)\n\
<EnterWindow>:PrimitiveEnter()\n\
<LeaveWindow>:PrimitiveLeave()\n\
<FocusOut>:PrimitiveFocusOut()\n\
<FocusIn>:PrimitiveFocusIn()\n\
s ~m ~a <Key>Tab:PrimitivePrevTabGroup()\n\
~m ~a <Key>Tab:PrimitiveNextTabGroup()\n\
<Unmap>: PrimitiveUnmap()";

#else

static char iconListTranslations[] = 
"<Btn1Down>,<Btn1Up>:Activate()\n\
<Btn1Down>:Arm()\n\
<Btn3Down>:SelectItem()\n\
~s ~m ~a <Key>Return:ArmAndActivate()\n\
~s ~m ~a <Key>space:ArmAndActivate() \n\
<Key>osfActivate:ArmAndActivate()\n\
<Key>osfSelect:ArmAndActivate()\n\
<Key>osfHelp: PrimitiveHelp()\n\
c <Key>osfUp:IconListTraverseUp(PAGE)\n\
c <Key>osfDown: IconListTraverseDown(PAGE)\n\
<Key>osfUp:IconListTraverseUp()\n\
<Key>osfDown:IconListTraverseDown()\n\
<Key>osfPageUp: IconListTraverseUp(PAGE)\n\
<Key>osfPageDown: IconListTraverseDown(PAGE)\n\
<EnterWindow>:PrimitiveEnter()\n\
<LeaveWindow>:PrimitiveLeave()\n\
<FocusOut>:PrimitiveFocusOut()\n\
<FocusIn>:PrimitiveFocusIn()\n\
s ~m ~a <Key>Tab:PrimitivePrevTabGroup()\n\
~m ~a <Key>Tab:PrimitiveNextTabGroup()\n\
<Unmap>: PrimitiveUnmap()";

#endif

static XtActionsRec actions[] =
{
   {"Arm", (XtActionProc)arm},
   {"SelectItem", (XtActionProc)selectItem},
   {"Activate", (XtActionProc)activate},
   {"ArmAndActivate", (XtActionProc)armAndActivate},
   {"IconListTraverseUp", (XtActionProc)up},
   {"IconListTraverseDown", (XtActionProc)down},
};

#define TheOffset(field) XtOffset(XmIconListWidget, iconList.field)
static XtResource resources[] =
{
   {XmNuserData, XmCUserData, XtRPointer, sizeof(XtPointer),
      TheOffset(userData), XtRImmediate, (XtPointer)0},
   {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      TheOffset(fontList), XmRImmediate, (XtPointer)NULL},
   {XmNhighlightThickness, XmCHighlightThickness, XmRHorizontalDimension, 
      sizeof(Dimension), TheOffset(highlightThickness), 
      XmRImmediate, (XtPointer) 2},
   {XmNmarginHeight, XmCMarginHeight, XmRVerticalDimension, sizeof(Dimension),
      TheOffset(marginHeight), XmRImmediate, (XtPointer)2},
   {XmNmarginWidth, XmCMarginWidth, XmRHorizontalDimension, sizeof(Dimension),
      TheOffset(marginWidth), XmRImmediate, (XtPointer)2},
   {XmNhighlightOnEnter, XmCHighlightOnEnter, XtRBoolean, sizeof(Boolean),
      TheOffset(highlightOnEnter), XtRImmediate, (XtPointer)TRUE},
   {XmNactivateCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      TheOffset(activateCallback), XtRImmediate, (XtPointer)NULL},
   {XmNvalueChangedCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      TheOffset(valueChangedCallback), XtRImmediate, (XtPointer)NULL},
   {XmNremoveCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
      TheOffset(removeCallback), XtRImmediate, (XtPointer)NULL},
   {XmNiconWidth, XmCIconDimension, XmRDimension, sizeof(Dimension),
      TheOffset(iconWidth), XtRImmediate, (XtPointer)16},
   {XmNiconHeight, XmCIconDimension, XmRDimension, sizeof(Dimension),
      TheOffset(iconHeight), XtRImmediate, (XtPointer)16},
   {XmNvisibleItemCount, XmCVisibleItemCount, XtRInt, sizeof(int),
      TheOffset(visibleItemCount), XtRImmediate, (XtPointer)10},
   {XmNusingBitmaps, XmCUsingBitmaps, XtRBoolean, sizeof(Boolean),
      TheOffset(usingBitmaps), XtRImmediate, (XtPointer)FALSE},
   {XmNclipWidget, XmCClipWidget, XtRWidget, sizeof(Widget),
      TheOffset(clipWidget), XtRImmediate, (XtPointer)NULL},
   {XmNhorizontalScrollBar, XmCHorizontalScrollBar, XmRWidget, sizeof(Widget),
      TheOffset(horizontalScrollBarWidget), XtRImmediate, (XtPointer)NULL},
   {XmNverticalScrollBar, XmCVerticalScrollBar, XmRWidget, sizeof(Widget),
      TheOffset(verticalScrollBarWidget), XtRImmediate, (XtPointer)NULL},
   {XmNselectColor, XmCSelectColor, XtRPixel, sizeof(Widget),
      TheOffset(selectColor), XmRCallProc, (XtPointer)_XmSelectColorDefault},
};
#undef TheOffset

static XmBaseClassExtRec baseClassExtRec = {
   NULL,
   NULLQUARK,
   XmBaseClassExtVersion,
   sizeof(XmBaseClassExtRec),
   NULL,                         /* InitializePrehook   */
   NULL,                         /* SetValuesPrehook */
   NULL,                         /* InitializePosthook    */
   NULL,                         /* SetValuesPosthook   */
   XmInheritClass,               /* secondaryObjectClass */
   XmInheritSecObjectCreate,     /* secondaryCreate   */
   XmInheritGetSecResData,       /* getSecRes data    */
   { 0 },                        /* fastSubclass flags    */
   XmInheritGetValuesPrehook,    /* getValuesPrehook */
   XmInheritGetValuesPosthook,   /* getValuesPosthook   */
   NULL,                         /* classPartInitPrehook */
   NULL,                         /* classPartInitPosthook*/
   NULL,                         /* ext_resources      */
   NULL,                         /* compiled_ext_resources*/
   0,                            /* num_ext_resources   */
   FALSE,                        /* use_sub_resources   */
#if XmVersion > 1001
   widgetNavigable,              /* widgetNavigable      */
   focusChange                   /* focusChange         */
#endif
};

XmIconListClassRec xmIconListClassRec = {
   {
      (WidgetClass)&xmPrimitiveClassRec,  /* superclass */
      (String)"XmIconList",               /* classname */
      (Cardinal)sizeof(XmIconListRec),    /* widget size */
      classInitialize,                    /* class init */
      (XtWidgetClassProc)NULL,            /* class part init */
      FALSE,                              /* class inited */
      (XtInitProc)initialize,             /* initialize */
      (XtArgsProc)NULL,                   /* initialize hook */
      (XtRealizeProc)XtInheritRealize,    /* realize */
      actions,                            /* actions */
      XtNumber(actions),                  /* number of actions */
      resources,                          /* resources */
      XtNumber(resources),                /* number of resources */
      NULLQUARK,                          /* xrm class */
      TRUE,                               /* compress motions */
      XtExposeCompressMultiple |
         XtExposeGraphicsExposeMerged,    /* compress exposures */
      TRUE,                               /* compress enter/leave */
      TRUE,                               /* visibility interest */
      (XtWidgetProc)destroy,              /* destroy */
      (XtWidgetProc)resize,               /* resize */
      (XtExposeProc)expose,               /* expose */
      NULL,                               /* set values */
      NULL,                               /* set values hook */
      XtInheritSetValuesAlmost,           /* set values almost */
      NULL,                               /* get values hook */
      (XtAcceptFocusProc)NULL,            /* accept focus */
      XtVersion,                          /* version */
      NULL,                               /* callback private */
      iconListTranslations,               /* translations */
      (XtGeometryHandler)queryGeometry,   /* query geometry */
      NULL,                               /* display accelerator */
      (XtPointer)&baseClassExtRec,        /* extension */
   },
   {   /* XmPrimitive Part */
      XmInheritBorderHighlight,
      XmInheritBorderUnhighlight,
      NULL,                               /* XtInheritTranslations */
      NULL,
      NULL,
      0,
      (XtPointer)NULL,    /* extension */
   },
   { /* XmIconList Part */
      0,
   },
};

WidgetClass xmIconListWidgetClass = (WidgetClass)&xmIconListClassRec;

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void classInitialize(void)
#else
static void classInitialize()
#endif
{
   baseClassExtRec.record_type = XmQmotif;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void initialize(Widget request, Widget new, 
                        ArgList _args, Cardinal *_numArgs)
#else
static void initialize(request, new, _args, _numArgs)
Widget request;
Widget new;
ArgList _args;
Cardinal *_numArgs;
#endif
{
   XmIconListWidget _request = (XmIconListWidget) request;
   XmIconListWidget _new = (XmIconListWidget) new;

   getGCsAndFonts(_new);

   /* XmAddTabGroup((Widget)_new); */

   CLIP(_new) = NULL;
   VBAR(_new) = NULL;
   SW(_new) = NULL;
   HAS_FOCUS(_new) = FALSE;

   FOCUS_EL(_new) = -1;
   START_ROW(_new) = 0;
   END_ROW(_new) = 0;

   CALL_CALLBACKS(_new) = TRUE;

   NUM_ELEMENTS(_new) = 0;
   ELEMENTS(_new) = NULL;
   LAST_CLICK_TIME(_new) = XtLastTimestampProcessed(XtDisplay(_request));
   CLICK_COUNT(_new) = 0;
   LAST_CLICK_ROW(_new) = -1;

   TEXT_HEIGHT(_new) = THE_FONT(_new)->ascent + THE_FONT(_new)->descent;

   /*
   ** Full spaced out element height.
   */

   FULL_EL_HEIGHT(_new) = _MAX_(ICON_HEIGHT(_new),  TEXT_HEIGHT(_new)) + 
                              2 * HT(_new);

#if 0
   printf("+++++++ ICON_HEIGHT = %d\n", ICON_HEIGHT(_new));
   printf("+++++++ TEXT_HEIGHT = %d\n", TEXT_HEIGHT(_new));
   printf("+++++++ FULL_EL_HEIGHT = %d\n", FULL_EL_HEIGHT(_new));
#endif

   TEXT_OFFSET(_new) = (FULL_EL_HEIGHT(_new) - TEXT_HEIGHT(_new))/2;

   HEIGHT(_new) = FULL_EL_HEIGHT(_new) * VIS_COUNT(_new);

   WIDTH(_new) =  ICON_WIDTH(_new) + SPACE + 
                  (20 * THE_FONT(_new)->max_bounds.width) + 
                  MARG_W(_new) + 2 * HT(_new);


   XtAddEventHandler((Widget)_new, ButtonMotionMask, FALSE, 
                     (XtEventHandler)buttonMotionEH, (XtPointer)_new);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void getGCsAndFonts(XmIconListWidget _ilw)
#else
static void getGCsAndFonts(_ilw)
XmIconListWidget _ilw;
#endif
{
   XGCValues      values;
   XtGCMask      valueMask;
   XFontStruct    *fs = (XFontStruct *) NULL;

   if (FL(_ilw) == NULL)
      FL(_ilw) = _XmGetDefaultFontList( (Widget) _ilw, XmLABEL_FONTLIST);

   FL(_ilw) = XmFontListCopy(FL(_ilw));

   valueMask = GCForeground | GCBackground;

   _XmFontListGetDefaultFont(FL(_ilw), &fs);
   values.foreground = FG(_ilw);
   values.background = BG(_ilw);
   values.graphics_exposures = False;




   COPY_GC(_ilw) = XtAllocateGC((Widget)_ilw, _ilw->core.depth,
         valueMask, &values, ~0L, 0L);

#if 0
   COPY_GC(_ilw) = XCreateGC(XtDisplay((Widget)_ilw), XtWindow((Widget)_ilw),
                     valueMask, &values);
#endif

   valueMask |= GCFont | GCGraphicsExposures;

   if (fs == (XFontStruct *)NULL)
      valueMask &= ~GCFont;
   else
      values.font = fs->fid;



   THE_GC(_ilw) = XtAllocateGC((Widget)_ilw, _ilw->core.depth, valueMask, 
                              &values, GCClipMask | GCClipYOrigin, 0L);
#if 0
   THE_GC(_ilw) = XCreateGC(XtDisplay((Widget)_ilw), XtWindow((Widget)_ilw),
                     valueMask, &values);
#endif

   values.function = GXcopy;

   values.foreground = BG(_ilw);
   values.subwindow_mode = IncludeInferiors;
   ERASE_GC(_ilw) = XtGetGC((Widget)_ilw, valueMask, &values);

#if 0
   values.foreground = FG(_ilw);

   valueMask |= GCFillStyle | GCTile;
   values.fill_style = FillTiled;
   values.subwindow_mode = ClipByChildren;
   values.tile = XmGetPixmapByDepth (XtScreen((Widget)(_ilw)), "50_foreground",
         FG(_ilw), BG(_ilw), DEPTH(_ilw));

   IGC(_ilw) = XtGetGC((Widget) _ilw, valueMask, &values);
#endif

   if (fs == (XFontStruct *)NULL)
   {
      XGetGCValues(XtDisplay(_ilw), THE_GC(_ilw), GCFont, &values);
      fs = XQueryFont(XtDisplay(_ilw), values.font);
   }

   if (SELECT(_ilw) == FG(_ilw))
      SELECT(_ilw) = BG(_ilw);

   valueMask = GCForeground;
   values.foreground = SELECT(_ilw);

   SELECT_GC(_ilw) = XtGetGC((Widget)_ilw, valueMask, &values);

   THE_FONT(_ilw) = fs;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void callCallbacks(XmIconListWidget _ilw, int _reason, XEvent *_event)
#else
static void callCallbacks(_ilw, _reason, _event)
XmIconListWidget _ilw;
int _reason;
XEvent *_event;
#endif
{
   XmIconListCallbackStruct ilcs;

   if (!CALL_CALLBACKS(_ilw) || (FOCUS_EL(_ilw) == -1) || !HAS_FOCUS(_ilw))
      return;

   ilcs.row = FOCUS_EL(_ilw);
   ilcs.element = ELEMENTS(_ilw)+FOCUS_EL(_ilw);   
   ilcs.reason = _reason;
   ilcs.event = _event;

   switch(_reason) {
      case XmCR_VALUE_CHANGED:
         if (XtHasCallbacks((Widget)_ilw, XmNvalueChangedCallback))
            XtCallCallbacks((Widget)_ilw, XmNvalueChangedCallback, &ilcs);
         break;

      case XmCR_ACTIVATE:
         if (XtHasCallbacks((Widget)_ilw, XmNactivateCallback))
            XtCallCallbacks((Widget)_ilw, XmNactivateCallback, &ilcs);
         break;
      case XmCR_REMOVE:
         if (XtHasCallbacks((Widget)_ilw, XmNremoveCallback))
            XtCallCallbacks((Widget)_ilw, XmNremoveCallback, &ilcs);
   }
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static XtGeometryResult queryGeometry(Widget ilw, XtWidgetGeometry *_request, 
                                       XtWidgetGeometry *_pref)
#else
static XtGeometryResult queryGeometry(ilw, _request, _pref)
Widget ilw;
XtWidgetGeometry *_request;
XtWidgetGeometry *_pref;
#endif
{
   XmIconListWidget _ilw = (XmIconListWidget) ilw;
   
   if (CLIP(_ilw))
      _pref->height = HEIGHT(CLIP(_ilw));
   else
      _pref->height = HEIGHT(_ilw);

   _pref->border_width = BORDER(_ilw);
   _pref->width = WIDTH(_ilw);

   return XtGeometryYes;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void expose(Widget ilw, XEvent *_event, Region _region)
#else
static void expose(ilw, _event, _region)
Widget ilw;
XEvent *_event;
Region _region;
#endif
{
   XmIconListWidget _ilw = (XmIconListWidget) ilw;
   XRectangle xrect;

   XClipBox(_region, &xrect);
   if (NUM_ELEMENTS(_ilw) <= 0)
      return;

   drawIconList(_ilw, &xrect);
   /* drawElementBackground(_ilw, FOCUS_EL(_ilw), SELECT_GC(_ilw));  */
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void resize(Widget ilw)
#else
static void resize(ilw)
Widget ilw;
#endif
{
#if 0
   XmIconListWidget _ilw = (XmIconListWidget) ilw;
   XRectangle xrect;

   if (!XtIsRealized((Widget)_ilw))
      return;

   xrect.x = 0;
   xrect.y = 0;
   xrect.width = WIDTH(_ilw);
   xrect.height = HEIGHT(_ilw);

   if (NUM_ELEMENTS(_ilw) <= 0)
      return;
#endif
}


#if 0
static int getFirstVisibleElement(XmIconListWidget _ilw)
{
   int offset;

   if (CORE(_ilw).y < 0)
      offset = (-CORE(_ilw).y)/FULL_EL_HEIGHT(_ilw);
   else
      offset = 0;

   return offset;
}
#endif


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static Boolean rowFullyVisible(XmIconListWidget _ilw, int _row, 
                                 Boolean *_aboveTop)
#else
static Boolean rowFullyVisible(_ilw, _row, _aboveTop)
XmIconListWidget _ilw;
int _row;
Boolean *_aboveTop;
#endif
{
   Boolean visible;

   visible = FALSE;
   if (_row < START_ROW(_ilw))
      *_aboveTop = TRUE;
   else
   if (_row > END_ROW(_ilw))
      *_aboveTop = FALSE;
   else
      visible = TRUE;

   return visible;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void syncScrollBar(XmIconListWidget _ilw)
#else
static void syncScrollBar(_ilw)
XmIconListWidget _ilw;
#endif
{
   Arg warg[7];
   int diff, n, size;

   if (VBAR(_ilw))
   {
      if (NUM_ELEMENTS(_ilw))
      {
         diff = END_ROW(_ilw) - START_ROW(_ilw) + 1;
         size = NUM_ELEMENTS(_ilw);
      }
      else
      {
         diff = 1;
         size = 1;
      }

      n = 0;
      XtSetArg(warg[n], XmNminimum, 0); n++;
      XtSetArg(warg[n], XmNmaximum, size); n++;
      XtSetArg(warg[n], XmNincrement, 1); n++;
      if (NUM_ELEMENTS(_ilw))
      {
         if (FOCUS_EL(_ilw) == -1)
            XtSetArg(warg[n], XmNvalue, 0);
         else
            XtSetArg(warg[n], XmNvalue, FOCUS_EL(_ilw)); 
         n++;
      }
      else
      {
         XtSetArg(warg[n], XmNvalue, 0); n++;
      }
      if (NUM_ELEMENTS(_ilw) > diff)
      {
         
         XtSetArg(warg[n], XmNsliderSize, diff); n++;
      }
      else
      {
         XtSetArg(warg[n], XmNsliderSize, size); n++;
      }
      XtSetValues(VBAR(_ilw), warg, n);
   }
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void syncToClip(XmIconListWidget _ilw)
#else
static void syncToClip(_ilw)
XmIconListWidget _ilw;
#endif
{
   int offset;

   if (CLIP(_ilw) == NULL)
      return;

   offset = FULL_EL_HEIGHT(_ilw) - 1;

   START_ROW(_ilw) = (-CORE(_ilw).y)/FULL_EL_HEIGHT(_ilw);
   END_ROW(_ilw) = START_ROW(_ilw) + 
                     HEIGHT(CLIP(_ilw))/FULL_EL_HEIGHT(_ilw) - 1;
   if (END_ROW(_ilw) >= NUM_ELEMENTS(_ilw))
      END_ROW(_ilw) = NUM_ELEMENTS(_ilw)-1;

#if 0
   if (VBAR(_ilw))
   {
      if ( (START_ROW(_ilw) == 0) && (END_ROW(_ilw) == (NUM_ELEMENTS(_ilw)-1)))
         XtUnmanageChild(VBAR(_ilw));
      else
         XtManageChild(VBAR(_ilw));
   }
#endif


}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void scrollBarSetValue(XmIconListWidget _ilw, Widget _sb, 
                                 int _value, Boolean _erase)
#else
static void scrollBarSetValue(_ilw, _sb, _value, _erase)
XmIconListWidget _ilw;
Widget _sb;
int _value;
Boolean _erase;
#endif
{
   Position x, y;
   Boolean aboveTop;
   Boolean visible, nextVisible;
   int nextStart, nextEnd, offset;

   if (_sb != VBAR(_ilw))
      return;

   y = -(_value * FULL_EL_HEIGHT(_ilw));
   x = CORE(_ilw).x;

   if (CLIP(_ilw))
   {
      visible = rowFullyVisible(_ilw, FOCUS_EL(_ilw), &aboveTop);

      offset = FULL_EL_HEIGHT(_ilw) - 1;

      nextStart = y/FULL_EL_HEIGHT(_ilw);

      if (nextStart == START_ROW(_ilw))
         return;

      nextEnd = nextStart + HEIGHT(CLIP(_ilw))/FULL_EL_HEIGHT(_ilw) - 1;
      if (nextEnd >= NUM_ELEMENTS(_ilw))
         nextEnd = NUM_ELEMENTS(_ilw)-1;

      nextVisible = ( (FOCUS_EL(_ilw) >= START_ROW(_ilw)) && 
                        (FOCUS_EL(_ilw) <= END_ROW(_ilw)) );

      if (_erase && !visible && !nextVisible)
         highlightElement(_ilw, FOCUS_EL(_ilw), ERASE_GC(_ilw));
   }

   MOVE(_ilw, x, y);
   syncToClip(_ilw);

/*
   if (nextVisible)
      highlightElement(_ilw, FOCUS_EL(_ilw), HIGHLIGHT_GC(_ilw));
*/

/*    callCallbacks(_ilw, XmCR_VALUE_CHANGED); */

/*
   if (CLIP(_ilw) && !(visible && nextVisible))
      highlightElement(_ilw, FOCUS_EL(_ilw), HIGHLIGHT_GC(_ilw));
*/
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void clipCB(Widget _clip, XtPointer _data, XtPointer _call)
#else
static void clipCB(_clip, _data, _call)
Widget _clip;
XtPointer _data;
XtPointer _call;
#endif
{
   XmIconListWidget ilw;
   Dimension height;
   XmDumbClipCallbackStruct *dccs = (XmDumbClipCallbackStruct *)_call;

   ilw = (XmIconListWidget)_data;

   if (dccs->reason == XmCR_RESIZE)
   {
      height = _MAX_(HEIGHT(CLIP(ilw)), NUM_ELEMENTS(ilw)*FULL_EL_HEIGHT(ilw));
      height -= 2 * BORDER(ilw);
      RESIZE(ilw, WIDTH(CLIP(ilw))-2*BORDER(ilw), height, BORDER(ilw));
      syncToClip(ilw);
   }
   syncScrollBar(ilw);
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void scrollBarCB(Widget _sb, XtPointer _data, XtPointer _call)
#else
static void scrollBarCB(_sb, _data, _call)
Widget _sb;
XtPointer _data;
XtPointer _call;
#endif
{
   XmScrollBarCallbackStruct *sbcs = (XmScrollBarCallbackStruct *)_call;
   XmIconListWidget ilw = (XmIconListWidget)_data;

   scrollBarSetValue(ilw, _sb, sbcs->value, sbcs->reason == XmCR_DRAG);
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void scrollToRow(XmIconListWidget _ilw, int _row, 
                           Boolean _jump, Boolean _toTop)
#else
static void scrollToRow(_ilw, _row, _jump, _toTop)
XmIconListWidget _ilw;
int _row;
Boolean _jump;
Boolean _toTop;
#endif
{
   int rowsPerClip;
   int value, ss, inc, pi;
   int newRow;
   Boolean fromTop;

   if (CLIP(_ilw) == NULL)
      return;

   if (!_toTop && rowFullyVisible(_ilw, _row, &fromTop))
      return;

   rowsPerClip = HEIGHT(CLIP(_ilw))/FULL_EL_HEIGHT(_ilw);

   if (_toTop)
   {
      if (_row >= rowsPerClip)
      {
         if (((NUM_ELEMENTS(_ilw) - _row) - 1) < rowsPerClip)
            newRow = NUM_ELEMENTS(_ilw) - rowsPerClip;
         else
            newRow = _row;
      }
      else
      {
         if (NUM_ELEMENTS(_ilw) > rowsPerClip)
            newRow = _row; /* 0; */
         else
            newRow = 0;
      }
   }
   else
   if (fromTop)
      newRow = _row;
   else
   {
      newRow = _row - (rowsPerClip-1);
      if (newRow < 0)
         newRow = 0;
   }

   XmScrollBarGetValues(VBAR(_ilw), &value, &ss, &inc, &pi);
   XmScrollBarSetValues(VBAR(_ilw), newRow, ss, inc, pi, TRUE);

   /* scrollBarSetValue(_ilw, VBAR(_ilw), newRow, FALSE); */
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static int yToElementRow(XmIconListWidget _ilw, Position _y)
#else
static int yToElementRow(_ilw, _y)
XmIconListWidget _ilw;
Position _y;
#endif
{
   int retElement;

   retElement = _y/FULL_EL_HEIGHT(_ilw);

   if (retElement > (NUM_ELEMENTS(_ilw)-1))
      return -1;
   else
      return retElement;
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void activate(Widget _w, XEvent *_event, 
                        String *_args, Cardinal *_numArgs)
#else
static void activate(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   int thisRow;
   XmIconListWidget ilw;
   int multiTime;
   Boolean activate;

   ilw = (XmIconListWidget)_w;
   if (!XmIsIconList(_w))
      return;

   if (ELEMENTS(ilw) == NULL)
      return;

   thisRow = yToElementRow(ilw, _event->xbutton.y);
   if ( (thisRow < 0) || (thisRow > NUM_ELEMENTS(ilw)))
   {
      LAST_CLICK_ROW(ilw) = -1;
      CLICK_COUNT(ilw) = 0;
      return;
   }

   activate = FALSE;
   if (_event->type == ButtonRelease)
   {
      if (CLICK_COUNT(ilw) == 0)
      {
         LAST_CLICK_ROW(ilw) = thisRow;
         CLICK_COUNT(ilw)++;
      }
      else
      if (thisRow == LAST_CLICK_ROW(ilw))
      {
         multiTime = XtGetMultiClickTime(XtDisplay(_w));
         if ( (_event->xbutton.time - LAST_CLICK_TIME(ilw)) <= multiTime)
         {
            CLICK_COUNT(ilw) = 0;
            activate = TRUE;
         }
         else
            CLICK_COUNT(ilw) = 1;
      }
      LAST_CLICK_TIME(ilw) = _event->xbutton.time;
      LAST_CLICK_ROW(ilw) = thisRow;
   }
   else
      activate = TRUE;

   if (!activate)
      return;

   scrollToRow(ilw, FOCUS_EL(ilw), FALSE, FALSE);

   callCallbacks(ilw, XmCR_ACTIVATE, _event);
   /* printf("Activate\n"); */
   
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void arm(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs)
#else
static void arm(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   XmIconListWidget ilw = (XmIconListWidget)_w;
   int thisElement;

   if (!XmIsIconList(ilw) || (ELEMENTS(ilw) == NULL))
      return;

   XmProcessTraversal(_w, XmTRAVERSE_CURRENT);

   if (_event->type == ButtonPress)
   {
      thisElement = yToElementRow(ilw, _event->xbutton.y);
      setFocusElement(ilw, thisElement, FALSE);
   }
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void selectItem(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs)
#else
static void selectItem(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   XmIconListWidget ilw = (XmIconListWidget)_w;
   int thisElement;

   if (!XmIsIconList(ilw) || (ELEMENTS(ilw) == NULL))
      return;

   XmProcessTraversal(_w, XmTRAVERSE_CURRENT);

   if (_event->type == ButtonPress)
   {
      thisElement = yToElementRow(ilw, _event->xbutton.y);
      setFocusElement(ilw, thisElement, False);
      drawIconListElement(ilw, FOCUS_EL(ilw), SELECT_GC(ilw), 
                  HAS_FOCUS(ilw) ? HIGHLIGHT_GC(ilw) : ERASE_GC(ilw));
   }
   callCallbacks(ilw, XmCR_REMOVE, NULL);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void setFocusElement(XmIconListWidget _ilw, int _row, Boolean _toTop)
#else
static void setFocusElement(_ilw, _row, _toTop)
XmIconListWidget _ilw;
int _row;
Boolean _toTop;
#endif
{
   Boolean above;
   if ( _row == -1)
      return;

   if ( (_row != FOCUS_EL(_ilw)) || _toTop || 
               !rowFullyVisible(_ilw, _row, &above)) {
         if (FOCUS_EL(_ilw) != -1)
            drawIconListElement(_ilw, FOCUS_EL(_ilw), ERASE_GC(_ilw), 
                                       ERASE_GC(_ilw));
         FOCUS_EL(_ilw) =_row;
         scrollToRow(_ilw, _row, FALSE, _toTop);
         drawIconListElement(_ilw, FOCUS_EL(_ilw), SELECT_GC(_ilw), 
                  HAS_FOCUS(_ilw) ? HIGHLIGHT_GC(_ilw) : ERASE_GC(_ilw));

   }
   callCallbacks(_ilw, XmCR_VALUE_CHANGED, NULL);
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void armAndActivate(Widget _w, XEvent *_event, 
                           String *_args, Cardinal *_numArgs)
#else
static void armAndActivate(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   arm(_w, _event, _args, _numArgs);
   activate(_w, _event, _args, _numArgs);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void moveFocusElement(XmIconListWidget _ilw, int _whence, XEvent *_event)
#else
static void moveFocusElement(_ilw, _whence, _event)
XmIconListWidget _ilw;
int _whence;
XEvent *_event;
#endif
{
   int next;

   if (_whence == 0)
      return;

   if (FOCUS_EL(_ilw) != -1)
      next = _whence + FOCUS_EL(_ilw);
   else
      next = _whence;

   if (next < 0)
      next = 0;
   else
   if (next >= NUM_ELEMENTS(_ilw))
      next = NUM_ELEMENTS(_ilw)-1;

   if (FOCUS_EL(_ilw) == next)
      return;

   if (FOCUS_EL(_ilw) != -1)
      drawIconListElement(_ilw, FOCUS_EL(_ilw), ERASE_GC(_ilw), ERASE_GC(_ilw));

   FOCUS_EL(_ilw) = next;
   scrollToRow(_ilw, FOCUS_EL(_ilw), TRUE, FALSE);
   drawIconListElement(_ilw, FOCUS_EL(_ilw), SELECT_GC(_ilw), 
                           HIGHLIGHT_GC(_ilw));

   callCallbacks(_ilw, XmCR_VALUE_CHANGED, _event);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void up(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs)
#else
static void up(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   XmIconListWidget ilw;
   int move;

   ilw = (XmIconListWidget)_w;

   if (!XmIsIconList(ilw) || (ELEMENTS(ilw) == NULL))
      return;

   if (FOCUS_EL(ilw) == 0)
      return;

   move = -1;
   if ((*_numArgs) == 1)
   {
      if (strcmp(_args[0], "PAGE") == 0)
         move = -VIS_COUNT(ilw);
   }

   if ((Widget)ilw != _w)
   {
      Time ltime = XtLastTimestampProcessed(XtDisplay(ilw));
   /*    XmProcessTraversal((Widget)ilw, XmTRAVERSE_PREV_TAB_GROUP); */
      XmProcessTraversal((Widget)ilw, XmTRAVERSE_CURRENT);
      XtCallAcceptFocus((Widget)ilw, &ltime);
   }

   moveFocusElement(ilw, move, _event);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void down(Widget _w, XEvent *_event, String *_args, Cardinal *_numArgs)
#else
static void down(_w, _event, _args, _numArgs)
Widget _w;
XEvent *_event;
String *_args;
Cardinal *_numArgs;
#endif
{
   XmIconListWidget ilw;
   int move;

   ilw = (XmIconListWidget)_w;

   if (!XmIsIconList(ilw) || (ELEMENTS(ilw) == NULL))
      return;

   if ( (FOCUS_EL(ilw)+1) == NUM_ELEMENTS(ilw))
      return;

   move = 1;
   if ((*_numArgs) == 1)
   {
      if (strcmp(_args[0], "PAGE") == 0)
         move = VIS_COUNT(ilw);
   }

   if ((Widget)ilw != _w)
   {
      XmProcessTraversal((Widget)ilw, XmTRAVERSE_CURRENT);
      XmProcessTraversal((Widget)ilw, XmTRAVERSE_CURRENT);
      /* XtSetKeyboardFocus((Widget)ilw, (Widget)ilw); */
   }

   moveFocusElement(ilw, move, _event);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void buttonMotionEH(Widget _w, XtPointer _data, 
                           XEvent *_event, Boolean *_cont)
#else
static void buttonMotionEH(_w, _data, _event, _cont)
Widget _w;
XtPointer _data;
XEvent *_event;
Boolean *_cont;
#endif
{
   int row;
   XmIconListWidget ilw = (XmIconListWidget)_w;

   if (!XmIsIconList(ilw) || (ELEMENTS(ilw) == NULL))
      return;

   row = yToElementRow(ilw, _event->xmotion.y);
   if ((row < 0) || (row == FOCUS_EL(ilw)))
      return;

   setFocusElement(ilw, row, FALSE);

}

#if XmVersion > 1001

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static XmNavigability widgetNavigable(Widget _w)
#else
static XmNavigability widgetNavigable(_w)
Widget _w;
#endif
{
   XmIconListWidget ilw = (XmIconListWidget)_w;

   if( XtIsSensitive(_w) && PRIM(ilw).traversal_on)
      return XmTAB_NAVIGABLE;

   return XmNOT_NAVIGABLE;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void focusChange(Widget _w, XmFocusChange _fc)
#else
static void focusChange(_w, _fc)
Widget _w;
XmFocusChange _fc;
#endif
{  
   XmIconListWidget ilw = (XmIconListWidget)_w;

   /* Enter/Leave is called only in pointer mode,
    * Focus in/out only called in explicit mode.
    */
   switch(   _fc   )
   {
      case XmENTER:
         if(!ICON_LIST(ilw).highlightOnEnter)
            break;
      /*    Drop through. */
      case XmFOCUS_IN:
         HAS_FOCUS(ilw) = TRUE;
         if ( (FOCUS_EL(ilw) == -1) && NUM_ELEMENTS(ilw))
            FOCUS_EL(ilw) = 0;
         else
         if (NUM_ELEMENTS(ilw) == 0)
            FOCUS_EL(ilw) = -1;
         drawIconListElement(ilw, FOCUS_EL(ilw), SELECT_GC(ilw), 
                                 HIGHLIGHT_GC(ilw));
         break;
      case XmLEAVE:
         if (!ICON_LIST(ilw).highlightOnEnter)
            break;
         /* Drop through. */
      case XmFOCUS_OUT:
         HAS_FOCUS(ilw) = FALSE;
         drawIconListElement(ilw, FOCUS_EL(ilw), SELECT_GC(ilw), 
                                 ERASE_GC(ilw));
         break;
   }
   return;
}

#endif


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void destroy(XmIconListWidget _ilw)
#else
static void destroy(_ilw)
XmIconListWidget _ilw;
#endif
{
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void drawElementBackground(XmIconListWidget _ilw, int _nelement, GC _gc)
#else
static void drawElementBackground(_ilw, _nelement, _gc)
XmIconListWidget _ilw;
int _nelement;
GC _gc;
#endif
{
   Position y;
   Position x;

   if (_nelement < START_ROW(_ilw))
      return;

   y = _nelement * FULL_EL_HEIGHT(_ilw) + HT(_ilw);
   x = HT(_ilw);

   XFillRectangle(XtDisplay(_ilw), XtWindow(_ilw), _gc,
            x, y, WIDTH(_ilw)-2*HT(_ilw), FULL_EL_HEIGHT(_ilw)-2*HT(_ilw));
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void highlightElement(XmIconListWidget _ilw, int _row, GC _gc)
#else
static void highlightElement(_ilw, _row, _gc)
XmIconListWidget _ilw;
int _row;
GC _gc;
#endif
{
   Position y;
   Position x;
   Boolean aboveTop;

#if XmVersion < 1002
#define _XmDrawHighlight(wid, win, gc, x, y, w, h, ht, ls)
#endif
/*    _XmDrawBorder((Widget)wid, gc, x, y, w, h, ht) */

   if (!XtIsRealized((Widget)_ilw))
      return;

   y = _row * FULL_EL_HEIGHT(_ilw);
   x = 0;

   if ( (_row != FOCUS_EL(_ilw)) || 
               (rowFullyVisible(_ilw, FOCUS_EL(_ilw), &aboveTop)))
       _XmDrawHighlight( XtDisplay(_ilw), XtWindow(_ilw), _gc,
               x, y, WIDTH(_ilw), FULL_EL_HEIGHT(_ilw), HT(_ilw),
               LineSolid);

   else if (_row != -1)
       _XmDrawHighlight( XtDisplay(_ilw), XtWindow(_ilw), _gc,
               x, START_ROW(_ilw)*FULL_EL_HEIGHT(_ilw), 
               WIDTH(_ilw), ((END_ROW(_ilw)-START_ROW(_ilw))+1)*
                  FULL_EL_HEIGHT(_ilw), HT(_ilw),
               LineSolid);
   else
       _XmDrawHighlight( XtDisplay(_ilw), XtWindow(_ilw), _gc,
               0, 0, WIDTH(CLIP(_ilw))-2*BORDER(_ilw),
               HEIGHT(CLIP(_ilw))-2*BORDER(_ilw), HT(_ilw),
               LineSolid);

#if XmVersion < 1002
#undef _XmDrawHighlight
#endif

}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void drawText(XmIconListWidget _ilw, GC _gc, Position _x, Position _y, 
                         char *_string, int _len)
#else
static void drawText(_ilw, _gc, _x, _y, _string, _len)
XmIconListWidget _ilw;
GC _gc;
Position _x;
Position _y;
char *_string;
int _len;
#endif
{
   XDrawString(XtDisplay(_ilw), XtWindow(_ilw), _gc, 
               _x, _y + THE_FONT(_ilw)->ascent, (char *)_string, _len);
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void drawIconListElement(XmIconListWidget _ilw, int _row, 
                                 GC _backgroundGC, GC _highlightGC)
#else
static void drawIconListElement(_ilw, _row, _backgroundGC, _highlightGC)
XmIconListWidget _ilw;
int _row;
GC _backgroundGC;
GC _highlightGC;
#endif
{
   Position startY, xpos, yOffset;
   IconListElement *thisElement;
   XGCValues gcValues;
   int i;

   if (!XtIsRealized((Widget)_ilw))
      return;

   if (_row == -1) {
      highlightElement(_ilw, _row, _highlightGC);
      return;
   }

   thisElement = ELEMENTS(_ilw)+_row;

   startY = _row*FULL_EL_HEIGHT(_ilw);
   yOffset = (FULL_EL_HEIGHT(_ilw) - thisElement->iconPixmap.height)/2;
   xpos = HT(_ilw);

/*
printf("FULL_EL_HEIGHT/iconPixmap.height/yOffset/startY  %d/%d/%d/%d\n", 
         FULL_EL_HEIGHT(_ilw), thisElement->iconPixmap.height, 
         yOffset, startY); 
*/


   drawElementBackground(_ilw, _row, _backgroundGC);
   highlightElement(_ilw, _row, _highlightGC);

   if (thisElement->iconPixmap.isBitmap)
      gcValues.clip_mask = thisElement->iconPixmap.pixmap;
   else
      gcValues.clip_mask = thisElement->iconPixmap.mask;

   gcValues.clip_y_origin = startY + yOffset; 
   gcValues.clip_x_origin = xpos;

   XChangeGC(XtDisplay(_ilw), COPY_GC(_ilw), 
      GCClipMask | GCClipXOrigin | GCClipYOrigin, &gcValues);

   if (!thisElement->iconPixmap.isBitmap)
      XCopyArea(XtDisplay(_ilw), thisElement->iconPixmap.pixmap, 
         XtWindow(_ilw), COPY_GC(_ilw), 0, 0, thisElement->iconPixmap.width, 
         thisElement->iconPixmap.height, xpos, startY+yOffset);
   else
      XCopyPlane(XtDisplay(_ilw), thisElement->iconPixmap.pixmap, 
         XtWindow(_ilw), COPY_GC(_ilw), 0, 0, thisElement->iconPixmap.width, 
         thisElement->iconPixmap.height, xpos, startY+yOffset, 1);


   xpos += MARG_W(_ilw) + thisElement->iconPixmap.width + SPACE;
   startY += TEXT_OFFSET(_ilw);
   
   for (i=0; i<thisElement->numStrings; i++) {
      if (thisElement->string[i]) {
         drawText(_ilw, THE_GC(_ilw), xpos, startY, thisElement->string[i], 
               strlen(thisElement->string[i]));
         xpos += SPACE + XTextWidth(THE_FONT(_ilw), thisElement->string[i], 
                           strlen(thisElement->string[i]));
      } 
   }
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
static void drawIconList(XmIconListWidget _ilw, XRectangle *_xrect)
#else
static void drawIconList(_ilw, _xrect)
XmIconListWidget _ilw;
XRectangle *_xrect;
#endif
{
   int beginElement, endElement;
   int i;

   if (!XtIsRealized((Widget)_ilw))
      return;

   beginElement = _xrect->y/FULL_EL_HEIGHT(_ilw);
   endElement = beginElement + 
                (_xrect->height+(FULL_EL_HEIGHT(_ilw)-1))/FULL_EL_HEIGHT(_ilw);

   if (beginElement >= NUM_ELEMENTS(_ilw)) {
      beginElement = -1;
      endElement = -2;
   }
   else if (endElement > (NUM_ELEMENTS(_ilw)-1))
      endElement = NUM_ELEMENTS(_ilw) - 1;

   for(i=beginElement;i<=endElement;i++) {
      /* printf("drawing %d\n", i); */
      if ( (i == FOCUS_EL(_ilw))) /*  && HAS_FOCUS(_ilw)) */
         drawIconListElement(_ilw, i, SELECT_GC(_ilw), 
                  HAS_FOCUS(_ilw) ? HIGHLIGHT_GC(_ilw) : ERASE_GC(_ilw));
      else
         drawIconListElement(_ilw, i, ERASE_GC(_ilw), ERASE_GC(_ilw));
   }

   if (CLIP(_ilw) && HAS_FOCUS(_ilw))
      highlightElement(_ilw, FOCUS_EL(_ilw), HIGHLIGHT_GC(_ilw));
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
void XmIconListSetItems(Widget _iconList, IconListElement *_elements, 
                           int _count)
#else
void XmIconListSetItems(_iconList, _elements, _count)
Widget _iconList;
IconListElement *_elements;
int _count;
#endif
{
   XmIconListWidget ilw;

   ilw = (XmIconListWidget)_iconList;

   ELEMENTS(ilw) = _elements;
   NUM_ELEMENTS(ilw) = _count;
   FOCUS_EL(ilw) = -1;

   if (XtIsRealized((Widget)ilw))
      XClearArea(XtDisplay(ilw), XtWindow(ilw), 0, 0, 
                     WIDTH(ilw), HEIGHT(ilw), TRUE);

   if (CLIP(ilw)) {
      RESIZE(ilw, WIDTH(CLIP(ilw)), _MAX_(FULL_EL_HEIGHT(ilw) * _count, 
                  HEIGHT(CLIP(ilw))), 0);
      MOVE(ilw, 0, 0);
      syncToClip(ilw);
   }

   /* purify_watch(&HEIGHT(ilw), "w"); */

   syncScrollBar(ilw);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
void XmIconListGetItems(Widget _iconList, IconListElement **_elements, 
                           int *_count)
#else
void XmIconListGetItems(_iconList, _elements, _count)
Widget _iconList;
IconListElement **_elements;
int *_count;
#endif
{
   XmIconListWidget ilw;

   ilw = (XmIconListWidget)_iconList;

   if (ilw) {
      *_elements = ELEMENTS(ilw);
      *_count = NUM_ELEMENTS(ilw);
   }
}


/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
Widget XmCreateScrolledIconList(Widget _parent, char *_name, 
                                 ArgList _args, Cardinal _numArgs)
#else
Widget XmCreateScrolledIconList(_parent, _name, _args, _numArgs)
Widget _parent;
char *_name;
ArgList _args;
Cardinal _numArgs;
#endif
{
   Widget sw, bb, clip;
   Widget vBar;
   XmIconListWidget ilw;
   Arg warg[7];
   Cardinal n;

   n = 0;
   XtSetArg(warg[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
   sw = XtCreateWidget( _name, 
                        xmScrolledWindowWidgetClass, 
                        _parent, 
                        warg, n);

   n = 0;
   XtSetArg(warg[n], XmNorientation, XmVERTICAL); n++;
   XtSetArg(warg[n], XmNminimum, 0); n++;
   XtSetArg(warg[n], XmNmaximum, 100); n++;
   vBar = XtCreateManagedWidget( "_vscroll", 
                                 xmScrollBarWidgetClass, 
                                 sw, 
                                 warg, n);

   n = 0;
   XtSetArg(warg[n], XmNshadowType, XmSHADOW_IN); n++;
   bb = XtCreateWidget( "_border", 
                        xmFrameWidgetClass, 
                        sw, 
                        warg, n);

   n = 0;
   XtSetArg(warg[n], XmNshadowThickness, 0); n++;
   XtSetArg(warg[n], XmNmarginWidth, 0); n++;
   XtSetArg(warg[n], XmNmarginHeight, 0); n++;
#if XmVersion > 1001
   XtSetArg(warg[n], XmNchildType, XmFRAME_WORKAREA_CHILD); n++;
#endif
   clip = XtCreateWidget( "_clip", 
                                 xmDumbClipWidgetClass, 
                                 bb, 
                                 warg, n);

   ilw = (XmIconListWidget) XtCreateManagedWidget(  "_iconList",
                                 xmIconListWidgetClass,
                                 clip,
                                 _args, _numArgs);

   n = 0;
   XtSetArg(warg[n], XtNborderWidth, 0); n++;
   XtSetArg(warg[n], XtNbackground, CORE(ilw).background_pixel); n++;
   XtSetArg(warg[n], XmNx, 0); n++;
   XtSetArg(warg[n], XmNy, 0); n++;
   XtSetArg(warg[n], XmNwidth, WIDTH(ilw)+2*BORDER(ilw)); n++;
   XtSetArg(warg[n], XmNheight, (FULL_EL_HEIGHT(ilw) * VIS_COUNT(ilw))+
                  2*BORDER(ilw)); n++;
   XtSetValues(clip, warg, n);
   XtManageChild(clip);


   n = 0;
   XtSetArg(warg[n], XmNbackground, CORE(ilw).background_pixel); n++;
   XtSetArg(warg[n], XmNshadowThickness, ST(ilw)); n++;
   XtSetValues(bb, warg, n);

   XtManageChild(bb);

   XmScrolledWindowSetAreas(sw, NULL, vBar, (Widget)bb);

   VBAR(ilw) = vBar;
   CLIP(ilw) = clip;
   SW(ilw) = sw;

   XtAddCallback(clip, XmNresizeCallback, 
                  (XtCallbackProc)clipCB, (XtPointer)ilw);

   XtAddCallback(VBAR(ilw), XmNvalueChangedCallback, 
                  (XtCallbackProc)scrollBarCB, (XtPointer)ilw);
   XtAddCallback(VBAR(ilw), XmNdragCallback, 
                  (XtCallbackProc)scrollBarCB, (XtPointer)ilw);

   XtManageChild(sw);

   return (Widget)ilw;
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
Widget XmIconListGetScrolledWindow(Widget _w)
#else
Widget XmIconListGetScrolledWindow(_w)
Widget _w;
#endif
{
   XmIconListWidget ilw;

   ilw = (XmIconListWidget)_w;
   return SW(ilw);
}

/*-------------------------------------------------------------------------*/
#if NeedFunctionPrototypes
void XmIconListScrollToRow(Widget _w, int _row, Boolean _toTop, 
                           Boolean _makeFocus, Boolean _callCallbacks)
#else
void XmIconListScrollToRow(_w, _row, _toTop, _makeFocus, _callCallbacks)
Widget _w;
int _row;
Boolean _toTop;
Boolean _makeFocus;
Boolean _callCallbacks;
#endif
{
   XmIconListWidget ilw;
   Boolean above;

   ilw = (XmIconListWidget)_w;

   if ( (_row < 0) || (_row >= NUM_ELEMENTS(ilw)))
      return;

   if (_makeFocus && HAS_FOCUS(ilw) && FOCUS_EL(ilw) != -1 && 
      !rowFullyVisible(ilw, FOCUS_EL(ilw), &above))
      highlightElement(ilw, FOCUS_EL(ilw), ERASE_GC(ilw));

   CALL_CALLBACKS(ilw) = _callCallbacks;

   if (_makeFocus)
      setFocusElement(ilw, _row, _toTop);

   CALL_CALLBACKS(ilw) = TRUE;

   /*   
   if (_makeFocus && HAS_FOCUS(ilw) && (FOCUS_EL(ilw) != -1))
      highlightElement(ilw, FOCUS_EL(ilw), HIGHLIGHT_GC(ilw));
   */
}
