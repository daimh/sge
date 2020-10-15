/* 
 * Copyright 1994 Alastair Gourlay
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation.
 */

/*
 *
 * GeometryP.h - XmpGeometry Private header
 *
 */

#ifndef _XmpGeometryP_h
#define _XmpGeometryP_h

#if XmVersion > 1001
#include <Xm/ManagerP.h>
#else
#include <Xm/XmP.h>
#endif

#include "Geometry.h"

/**
***  gcc gives a warning when ints are converted to XtPointers
***  and vice versa because they are the wrong size on my AXP.
***  So these macros will convert the integral type to something
***  the size of an XtPointer to solve these warnings.  The type
***  (long) may need to be changed on different hardwares.
**/

#ifdef TARGET_64BIT
typedef long PointerSizedIntegral;
#else
typedef int PointerSizedIntegral;
#endif

#define int2xtp(a) ((XtPointer)((PointerSizedIntegral)(a)))
#define xtp2int(a) ((int)((PointerSizedIntegral)(a)))


#ifdef _NO_PROTO
typedef void (*XmpSizeProc)();
#else
typedef void (*XmpSizeProc)(Widget,Dimension*,Dimension*);
#endif

#define XmpInheritInitializePostHook ((XtInitProc) _XtInherit)
#define XmpInheritSetValuesPostHook ((XtSetValuesFunc) _XtInherit)
#define XmpInheritConstraintInitializePostHook ((XtInitProc) _XtInherit)
#define XmpInheritConstraintSetValuesPostHook ((XtSetValuesFunc) _XtInherit)
#define XmpInheritSize ((XmpSizeProc) _XtInherit)
#define XmpInheritBitGravity ( (long) _XtInherit )

typedef struct
{
	long	bit_gravity;
	XtInitProc		initialize_post_hook;
	XtSetValuesFunc		set_values_post_hook;
	XtInitProc		constraint_initialize_post_hook;
	XtSetValuesFunc		constraint_set_values_post_hook;
	XmpSizeProc		size;
	XtPointer		extension;
} XmpGeometryClassPart;

typedef struct _XmpGeometryClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmpGeometryClassPart	geometry_class;
} XmpGeometryClassRec;

externalref XmpGeometryClassRec xmpGeometryClassRec;

typedef struct
{
	Dimension		pref_width;
	Dimension		pref_height;
	Boolean			compute_width;
	Boolean			compute_height;
	Boolean			reconfigure;
	Boolean			constraint_reconfigure;
	Widget			instigator;
} XmpGeometryPart, *XmpGeometryPartPtr;

typedef struct _XmpGeometryRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmpGeometryPart		geometry;
} XmpGeometryRec;

typedef struct _XmpGeometryConstraintPart
{
	XtPointer		reserved;
} XmpGeometryConstraintPart, *XmpGeometryConstraintPartPtr;

typedef struct _XmpGeometryConstraintRec
{
	XmManagerConstraintPart	manager;
	XmpGeometryConstraintPart geometry;
} XmpGeometryConstraintRec, *XmpGeometryConstraint;



#ifdef _NO_PROTO
extern void XmpPreferredGeometry();
extern void XmpSetGeometry();
extern void XmpWarning();
#else
extern void XmpPreferredGeometry(Widget,Dimension*,Dimension*);
extern void XmpSetGeometry(Widget,Position,Position,Dimension,Dimension,Dimension);
extern void XmpWarning(Widget,String);
#endif
/* Instance field access macros */

#define PrefWidth(w) (w->geometry.pref_width)
#define PrefHeight(w) (w->geometry.pref_height)
#define ComputeWidth(w) (w->geometry.compute_width)
#define ComputeHeight(w) (w->geometry.compute_height)
#define Reconfigure(w) (w->geometry.reconfigure)
#define ConstraintReconfigure(w) (w->geometry.constraint_reconfigure)
#define Instigator(w) (w->geometry.instigator)

#define Children(w) (w->composite.children)
#define NumChildren(w) (w->composite.num_children)

#define X(w) (w->core.x)
#define Y(w) (w->core.y)
#define Width(w) (w->core.width)
#define Height(w) (w->core.height)
#define BorderWidth(w) (w->core.border_width)

/* Constraint field access macros */

#define GeometryConstraint(w) ((XmpGeometryConstraint)(w->core.constraints)

#define GeometryReserved(w) (GeometryConstraint(w)->geometry.reserved)

/* Class method macros  - used by subclass to envelop XmpGeometry methods */

#define XmpGeometryRedisplay(w,ev,rg) \
		(*xmpGeometryWidgetClass->core_class.expose)(w,ev,rg)
#define XmpGeometryChangeManaged(w) \
	(*xmpGeometryWidgetClass->composite_class.change_managed)(w)
#define XmpGeometryRealize(w,m,a) \
		(*xmpGeometryWidgetClass->core_class.realize(w,m,a)

/* Class field access macros */

#define BitGravity(w) (GeometryClass(w)->geometry_class.bit_gravity)

/* XmpGeometry hook method support macros */

#define GeometryClass(w) ((XmpGeometryWidgetClass)XtClass(w))
#define GeometryClassPart(w) (GeometryClass(w)->geometry_class)

#define XmpGeometryInitialize(wc,rw,nw,a,na) \
    if (wc == XtClass(nw)) \
	(*GeometryClassPart(nw).initialize_post_hook)(rw,nw,a,na)

#define XmpGeometrySetValues(wc,ow,rw,nw,a,na) \
    (wc == XtClass(nw) ? \
	(*GeometryClassPart(nw).set_values_post_hook)(ow,rw,nw,a,na) \
	: False)

#define XmpGeometryConstraintInitialize(wc,rw,nw,a,na) \
    if (wc == XtClass(nw)) \
	(*GeometryClassPart(nw).constraint_initialize_post_hook)(rw,nw,a,na)

#define XmpGeometryConstraintSetValues(wc,ow,rw,nw,a,na) \
    (wc == XtClass(nw) ? \
	(*GeometryClassPart(nw).constraint_set_values_post_hook)(ow,rw,nw,a,na) \
	: False)

/* Useful Resize/Size macros */

/* Do the subtraction (a-b), but honor the specified minimum result (d) */
#define XmpSubtract(a,b,d) ((a)>((b)+(d)) ? (a)-(b) : d)

/* the usual Max */
#ifndef Max
#define Max(x, y) (((unsigned)(x) > (unsigned)(y)) ? (x) : (y))
#endif

#endif /* _XmpGeometryP_h */
