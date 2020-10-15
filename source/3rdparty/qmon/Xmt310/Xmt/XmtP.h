/* 
 * Motif Tools Library, Version 3.1
 * $Id: XmtP.h,v 1.1 2001-07-18 11:06:03 root Exp $
 * 
 * Written by David Flanagan.
 * Copyright (c) 1992-2001 by David Flanagan.
 * All Rights Reserved.  See the file COPYRIGHT for details.
 * This is open source software.  See the file LICENSE for details.
 * There is no warranty for this software.  See NO_WARRANTY for details.
 *
 * $Log: XmtP.h,v $
 * Revision 1.1  2001-07-18 11:06:03  root
 * Initial revision
 *
 * Revision 1.2  2001/06/12 16:25:28  andre
 * *** empty log message ***
 *
 *
 */

#ifndef _XmtP_h
#define _XmtP_h

/*
 * This header includes XmP.h and fixes problems with it
 * and handles 1.1 to 1.2 portability issues
 */

#include <Xmt/Xmt.h>
#include <Xm/XmP.h>

#if XmVersion == 1001
#include <Xm/Traversal.h>
#endif

#if XmVersion >= 1002
#include <Xm/PrimitiveP.h>
#include <Xm/GadgetP.h>
#include <Xm/ManagerP.h>
#endif


#if XmVersion == 1001
/* XmP.h uses NULL instead of None, resulting in a compiler warning */
#ifdef XtIsRealized
#undef XtIsRealized
#endif
#define XtIsRealized(widget) 					\
   (XtIsWidget(widget)  ?					\
      ((Widget)(widget))->core.window != None  :		\
      ((Object)(widget))->object.parent->core.window != None)

#endif  /* XmVersion 1001 */

/*
 * In Motif 1.1.0, and maybe other old 1.1 versions as well,
 * XmMenuProc and other XmRowColumn method types are not defined.
 * If needed, we define them here.
 */
#ifdef MISSING_METHOD_TYPES
#define XmMenuProc XtProc
#define XmArmAndActivate XtProc
#define XmMenuTrav XtProc
#endif

/*
 * Inheritance symbols not defined in Motif 1.1
 */
#if XmVersion == 1001
/* HP Motif does these inheritance symbols a little differently */
#if defined(HP_MOTIF) || defined(HP_NLS)
#ifndef XmInheritArmAndActivate
#define XmInheritArmAndActivate (XtActionProc) _XtInherit
#endif    
#ifndef  XmInheritMenuTraversalProc
#define  XmInheritMenuTraversalProc (XmMenuTraversalProc) _XtInherit
#endif    
#else  /* if not HP/Motif */
#ifndef XmInheritArmAndActivate
#define XmInheritArmAndActivate    (XmArmAndActivate) _XtInherit
#endif    
#ifndef XmInheritMenuTraversalProc
#define XmInheritMenuTraversalProc (XmMenuTrav) _XtInherit
#endif    
#ifndef XmInheritFocusMovedProc
#define XmInheritFocusMovedProc XtInheritFocusMovedProc
#endif    
#endif
#ifndef XmInheritMenuProc
#define XmInheritMenuProc          (XmMenuProc) _XtInherit
#endif    
#ifndef XmInheritBorderHighlight
#define XmInheritBorderHighlight   (XtWidgetProc) _XtInherit
#endif    
#ifndef XmInheritBorderUnhighlight
#define XmInheritBorderUnhighlight (XtWidgetProc) _XtInherit
#endif    
#endif

/*
 * An inheritance symbol omited from 1.2.0
 */
#if XmVersion == 1002
#ifndef XmInheritMenuTraversalProc    
#define XmInheritMenuTraversalProc (XmMenuTraversalProc) _XtInherit
#endif    
#endif

/*
 * Some internal functions' signatures changed between 1.1 and 1.2,
 * so define some wrapper macros that work correctly for either release.
 */
#if XmVersion <= 1001
#define _XmtConfigureObject(g, x, y, w, h, b)\
    _XmConfigureObject((RectObj)g, x, y, w, h, b)
#define _XmtResizeObject(g, w, h, bw) _XmResizeObject((RectObj)g, w, h, bw);
#define _XmtMoveObject(g, x, y) _XmMoveObject((RectObj)g, x, y);
#define _XmtRedisplayGadgets(w, e, r)\
    _XmRedisplayGadgets((CompositeWidget)w, (XExposeEvent *)e, r)
#else
#define _XmtConfigureObject(g,x,y,w,h,b) _XmConfigureObject(g, x, y, w, h, b)
#define _XmtResizeObject(g, w, h, bw) _XmResizeObject(g, w, h, bw);
#define _XmtMoveObject(g, x, y) _XmMoveObject(g, x, y);
#define _XmtRedisplayGadgets(w, e, r) _XmRedisplayGadgets(w, e, r)
#endif

#if XmVersion >= 2000
/*
 * ARH  These functions have been removed from view in Motif 2.0.
 *      Suitable replacements need to be provided, but this will
 *      get us going.
 */
extern void _XmTextMarginsProc(Widget w, XmBaselineMargins *margins_rec) ;

extern void _XmDestroyParentCallback(Widget w, XtPointer client_data,
				     XtPointer call_data) ;
#endif


    
#endif /* _XmtP_h */
