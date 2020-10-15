/*
 * Copyright(C) Q. Frank Xia (qx@math.columbia.edu), 1994. 
 *
 *                       All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Q. Frank Xia not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * 
 * This software is provided as-is and without any warranty of any kind.
 *
 * $Id: CellP.h,v 1.1 2001-07-18 11:05:59 root Exp $
 */

/*
 * CellP.h - Private definitions for Cell widget
 *
 * 7-20-1994:
 *        This file is created.
 */

#ifndef _XQ_CELL_H_
#define _XQ_CELL_H_


#if XmVersion <= 1001
#include <Xm/XmP.h>
#else
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/TextFP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XqCellClassPart{
  void *extension;
} XqCellClassPart;

typedef struct _XqCellClassRec {
  CoreClassPart core_class;
  XmPrimitiveClassPart primitive_class;
  XmTextFieldClassPart text_class;
  XqCellClassPart cell_class;
} XqCellClassRec;

externalref XqCellClassRec xqCellClassRec;

typedef struct _XqCellPart {
  void *extension;
} XqCellPart;

typedef struct _XqCellRec {
  CorePart core;
  XmPrimitivePart primitive;
  XmTextFieldPart text;
  XqCellPart cell;
} XqCellRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XQ_CELL_H_ */




