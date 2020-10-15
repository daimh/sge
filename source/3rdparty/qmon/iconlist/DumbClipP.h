/* $Id: DumbClipP.h,v 1.1 2001-07-18 11:06:03 root Exp $ */
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

#ifndef _DumbClipP_h
#define _DumbClipP_h

#include <Xm/XmP.h>
#include <Xm/BulletinBP.h>
#include "DumbClip.h"

typedef struct _XmDumbClipClassPart
{
	int blammo;
} XmDumbClipClassPart;

typedef struct _XmDumbClipClassRec
{
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	ConstraintClassPart constraint_class;
	XmManagerClassPart manager_class;
	XmBulletinBoardClassPart bulletin_board_class;
	XmDumbClipClassPart dump_clip_class;
} XmDumbClipClassRec, *XmDumbClipWidgetClass;

extern XmDumbClipClassRec xwriMessageBoxClassRec;

typedef struct _XmDumbClipPart
{
	XtCallbackList resizeCallback;
} XmDumbClipPart;

typedef struct _XmDumbClipRec
{
	CorePart core;
	CompositePart	 composite;
	ConstraintPart constraint;
	XmManagerPart manager;
	XmBulletinBoardPart bulletin_board;
	XmDumbClipPart dumb_clip;
} XmDumbClipRec, *XmDumbClipPtr;

#endif /* _DumbClipP_h */
