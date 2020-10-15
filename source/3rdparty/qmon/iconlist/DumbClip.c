/* $Id: DumbClip.c,v 1.1 2001-07-18 11:06:03 root Exp $ */
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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>

#include <Xm/Xm.h>
#include <Xm/BulletinBP.h>

#include "DumbClipP.h"

#define CORE(a) ((a)->core)
#define WIDTH(a) ((a)->core.width)
#define HEIGHT(a) ((a)->core.height)
#define COMP(a) ((a)->composite)
#define MANAGER(a) ((a)->manager)

#define DC(a) ((a)->guiFileBox)

#define _GOOD_(a) ((a) && XtIsManaged(a))


#if XmVersion <= 1001
#define XmInheritFocusMovedProc XtInheritFocusMovedProc
#define XmInheritTranslations XtInheritTranslations
#endif

#if NeedFunctionPrototypes
static void initialize( XmDumbClipWidget _request, 
                        XmDumbClipWidget _new, 
                        String *_args, 
                        Cardinal *_numArgs);
static XtGeometryResult queryGeometry( XmDumbClipWidget _dcw, 
                                       XtWidgetGeometry *_request, 
                                       XtWidgetGeometry *_pref);
static void resize(Widget _w);
#else
static void initialize(); 
static XtGeometryResult queryGeometry(); 
static void resize();
#endif

#define TheOffset(field) XtOffset(XmDumbClipWidget, dumb_clip.field)

static XtResource resources[] =
{
    {XmNresizeCallback, XmCCallback, XmRCallback, sizeof(XtCallbackList),
        TheOffset(resizeCallback), XtRImmediate, (XtPointer)NULL},
};


XmDumbClipClassRec xmDumbClipClassRec =
{
   {
      (WidgetClass)&xmBulletinBoardClassRec,    /* superclass */
      "XmDumbClip",                             /* class_name */
      (Cardinal)sizeof(XmDumbClipRec),          /* widget size */
      (XtProc)NULL,                             /* class_init */
      (XtWidgetClassProc)NULL,                  /* class_part_init */
      (XtEnum)FALSE,                            /* class_inited */
      (XtInitProc)initialize,                   /* initialize */
      (XtArgsProc)NULL,                         /* init_hook */
      XtInheritRealize,                         /* realize */
      (XtActionList)NULL,                       /* actions */
      (Cardinal)0,                              /* num_actions */
      (XtResourceList)resources,                /* resources */
      (Cardinal)XtNumber(resources),            /* num_resources */
      NULLQUARK,                                /* xrm_class */
      FALSE,                                    /* compress_motion */
      (XtEnum)FALSE,                            /* compress_exposur */
      FALSE,                                    /* compress enterleave */
      FALSE,                                    /* visibility_interest */
      (XtWidgetProc)NULL,                       /* destroy */
      (XtWidgetProc)resize,
      (XtExposeProc)NULL,                       /* XtInheritExpose, */
      (XtSetValuesFunc)NULL,                    /* set_values */
      (XtArgsFunc)NULL,                         /* set_values_hook */
      XtInheritSetValuesAlmost,                 /* set_values_almost */
      (XtArgsProc)NULL,                         /* get_values_hook */
      XtInheritAcceptFocus,                     /* accept_focus */
      XtVersion,                                /* version */
      (XtPointer)NULL,                          /* callback_private */
      XtInheritTranslations,
      (XtGeometryHandler)queryGeometry,         /* query_geometry */
      XtInheritDisplayAccelerator,              /* display_accelerator */
      (XtPointer)NULL,                          /* extension */
   },
   { /* Composite Part */
      (XtGeometryHandler)XtInheritGeometryManager,    /* geometry_manager */
      (XtWidgetProc)XtInheritChangeManaged,           /* change_managed */
      (XtWidgetProc)XtInheritInsertChild,             /* inherit_child */
      (XtWidgetProc)XtInheritDeleteChild,             /* delete_child */
      NULL,                                           /* extension */
   },
   {   /* Constraint Part */
      (XtResourceList)NULL,            /* constraintResources, */
      0,                               /* XtNumber(constraintResources), */
      (Cardinal)0,                     /* sizeof(XmDumbClipConstraintRec), */
      (XtInitProc)NULL,
      (XtWidgetProc)NULL,
      (XtSetValuesFunc)NULL,           /* constraintSetValues, */
      (XtPointer)NULL,
   },
   {   /* XmManager Part */
      XmInheritTranslations,
      NULL,
      0,   
      NULL,
      0,   
      XmInheritParentProcess,
      NULL,
   },
   { /* XmBulletinBoard Part */
      TRUE,
      NULL,                            /* geoMatrixCreate, */
      XmInheritFocusMovedProc,   
      NULL,
   },
   { /* XmDumbClip Part */
      0,
   },
};

WidgetClass xmDumbClipWidgetClass = (WidgetClass)&xmDumbClipClassRec;

#if NeedFunctionPrototypes
static void initialize(XmDumbClipWidget _request, XmDumbClipWidget _new, 
                           String *_args, Cardinal *_numArgs)
#else
static void initialize(_request, _new, _args, _numArgs)
XmDumbClipWidget _request;
XmDumbClipWidget _new;
String *_args;
Cardinal *_numArgs;
#endif
{
   if (WIDTH(_request) == 0)
      WIDTH(_new) = 1;
   if (HEIGHT(_request) == 0)
      HEIGHT(_new) = 1;
}

#if NeedFunctionPrototypes
static void resize(Widget _w)
#else
static void resize(_w)
Widget _w;
#endif
{
   XmDumbClipCallbackStruct dccs;

   if (XtHasCallbacks(_w, XmNresizeCallback))
   {
      dccs.reason = XmCR_RESIZE;
      dccs.event = NULL;
      dccs.width = WIDTH(_w);
      dccs.height = HEIGHT(_w);
      XtCallCallbacks(_w, XmNresizeCallback, (XtPointer)&dccs);
   }
}

#if NeedFunctionPrototypes
static XtGeometryResult queryGeometry(XmDumbClipWidget _dcw, 
                     XtWidgetGeometry *_request, XtWidgetGeometry *_pref)
#else
static XtGeometryResult queryGeometry(_dcw, _request, _pref)
XmDumbClipWidget _dcw;
XtWidgetGeometry *_request;
XtWidgetGeometry *_pref;
#endif
{
   int i;
   for(i=0;i<COMP(_dcw).num_children;i++)
   {
      if (_GOOD_(COMP(_dcw).children[i]))
         break;
   }

   if (i < COMP(_dcw).num_children)
      return XtQueryGeometry(COMP(_dcw).children[i], _request, _pref);
   else
   {
      if (!_request)
      {
         _pref->width = 1;
         _pref->height = 1;
         _pref->border_width = 0;
      }
      return XtGeometryYes;
   }
}

#if NeedFunctionPrototypes
Widget XmCreateDumbClip(Widget _parent, String _name, ArgList _warg, 
                           Cardinal _numWarg)
#else
Widget XmCreateDumbClip(_parent, _name, _warg, _numWarg)
Widget _parent;
String _name;
ArgList _warg;
Cardinal _numWarg;
#endif
{
   return (XtCreateWidget(_name, xmDumbClipWidgetClass, _parent, _warg, _numWarg));
}

