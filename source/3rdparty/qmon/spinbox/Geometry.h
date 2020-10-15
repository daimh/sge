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
 * Geometry.h - XmpGeometry Public header
 *
 */

#ifndef _XmpGeometry_h
#define _XmpGeometry_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmpIsGeometry
#define XmpIsGeometry(w) XtIsSubclass(w, xmpGeometryWidgetClass)
#endif

externalref WidgetClass xmpGeometryWidgetClass;

typedef struct _XmpGeometryClassRec *XmpGeometryWidgetClass;
typedef struct _XmpGeometryRec *XmpGeometryWidget;

#ifdef __cplusplus
}
#endif

#endif /* _XmpGeometry_h */

