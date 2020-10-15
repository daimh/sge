
/************************************************************************* 
 * Version 1.0  on  15-May-1997
 * (c) 1997 Pralay Dakua (pkanti@hotmail.com)
 *     
 * This is a free software and permission to use, modify, distribute,
 * selling and using for commercial purpose is hereby granted provided
 * that THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE
 * INCLUDED IN ALL COPIES AND THEIR SUPPORTING DOCUMENTATIONS.
 *
 * There is no warranty for this software. In no event Pralay Dakua
 * will be liable for merchantability and fitness of the software and 
 * damages due to this software.
 *
 * Author:
 * Pralay Dakua (pkanti@hotmail.com)
 *
 **************************************************************************/

#ifndef __TABP_H__
#define __TABP_H__

#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include "Tab.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmTabClassPart {
	XtPointer extension;
} XmTabClassPart;

typedef struct _XmTabClassRec {
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	ConstraintClassPart constraint_class;
	XmManagerClassPart manager_class;
	XmTabClassPart tab_class;
} XmTabClassRec;

extern XmTabClassRec xmTabClassRec;

typedef struct _XmTabPart {
	XtCallbackList value_changed_callback;
	Widget active_tab;
	XmFontList tab_font_list;
	Boolean resize_children;
   Boolean layout_disabled;
   Cardinal tabs_per_row;
	Dimension cut_size;
	Dimension raise;
	Dimension tab_height;
   Cardinal tab_rows;
	Dimension tab_total_width;
	Dimension tab_total_height;
	Dimension margin_width;
	Dimension margin_height;
	GC normal_gc;
   GC highlight_gc;
} XmTabPart;

typedef struct _XmTabRec {
	CorePart core;
	CompositePart composite;
	ConstraintPart constraint;
	XmManagerPart manager;
	XmTabPart tab;
} XmTabRec;

typedef struct _XmTabConstraintPart {
	XmString tab_label;
	XRectangle tab_rect;
} XmTabConstraintPart;

typedef struct _XmTabConstraintRec {
	XmManagerConstraintPart manager;
	XmTabConstraintPart tab;
} XmTabConstraintRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /**  __TABP_H__  **/


