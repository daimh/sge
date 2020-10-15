/*
 *
 * SpinboxP.h - XmpSpinbox Private header
 *
 */

#ifndef _XmpSpinboxP_h
#define _XmpSpinboxP_h

#if XmVersion > 1001
#include <Xm/RepType.h>
#else
#include <Xmt/Xmt.h>
#include <Xmt/Converters.h>
#endif

#include "GeometryP.h"
#include "Spinbox.h"

typedef struct
{
#if XmVersion > 1001
        XmRepTypeId		spinbox_type_id;
        XmRepTypeId		spinbox_style_id;
#endif
	XtPointer		extension;
} XmpSpinboxClassPart;

typedef struct _XmpSpinboxClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmpGeometryClassPart	geometry_class;
	XmpSpinboxClassPart	spinbox_class;
} XmpSpinboxClassRec;

externalref XmpSpinboxClassRec xmpSpinboxClassRec;

typedef struct
{
   unsigned char arrow_orientation;
   unsigned char spinbox_style;
   unsigned char spinbox_type;
   short tf_columns;
   short decimal_points;
   Boolean items_are_sorted;
   Boolean use_closest_value;
   Boolean auto_correct;
   Boolean cycle;
   Boolean text_update_constantly;
   int delay_ms;
   int item_count;
   int button_size_ratio;
   Boolean button_size_fixed;
   long val_now;
   long val_old;
   long val_min;
   long val_max;
   long increment;
   long increment_large;
   XtIntervalId interval;
   Widget down_btn;
   Widget up_btn;
   Widget tf;
   XtPointer show_value_data;
   XtPointer get_value_data;
   SpinboxShowValueProc *show_value_proc;
   SpinboxGetValueProc *get_value_proc;
   String* items;
   XtCallbackList ValueChangedCBL;
   XtAppContext context;
} XmpSpinboxPart, *XmpSpinboxPartPtr;

typedef struct _XmpSpinboxRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmpGeometryPart		geometry;
	XmpSpinboxPart		spinbox;
} XmpSpinboxRec;

typedef struct _XmpSpinboxConstraintPart
{
	XtPointer		make_compiler_happy;
} XmpSpinboxConstraintPart, *XmpSpinboxConstraintPartPtr;

typedef struct _XmpSpinboxConstraintRec
{
	XmManagerConstraintPart		manager;
	XmpGeometryConstraintPart	geometry;
	XmpSpinboxConstraintPart	spinbox;
} XmpSpinboxConstraintRec, *XmpSpinboxConstraint;


/* Instance field access macros */

#define SpinboxArrowOrientation(w)  (w->spinbox.arrow_orientation)
#define SpinboxButtonSizeRatio(w)   (w->spinbox.button_size_ratio)
#define SpinboxButtonSizeFixed(w)   (w->spinbox.button_size_fixed)
#define SpinboxContext(w)     (w->spinbox.context)
#define SpinboxCycle(w)       (w->spinbox.cycle)
#define SpinboxDecimalPoints(w)     (w->spinbox.decimal_points)
#define SpinboxDelay(w)       (w->spinbox.delay_ms)
#define SpinboxDownBtn(w)     (w->spinbox.down_btn)
#define SpinboxGetValue(w)    (w->spinbox.get_value_proc)
#define SpinboxGetValueData(w)      (w->spinbox.get_value_data)
#define SpinboxIncrement(w)      (w->spinbox.increment)
#define SpinboxIncLarge(w)    (w->spinbox.increment_large)
#define SpinboxInterval(w)    (w->spinbox.interval)
#define SpinboxItems(w)       (w->spinbox.items)
#define SpinboxItemCount(w)      (w->spinbox.item_count)
#define SpinboxItemsAreSorted(w) (w->spinbox.items_are_sorted)
#define SpinboxMaxValue(w)    (w->spinbox.val_max)
#define SpinboxMinValue(w)    (w->spinbox.val_min)
#define SpinboxOldValue(w)    (w->spinbox.val_old)
#define SpinboxSensitive(w)      (w->core.sensitive)
#define SpinboxShowValue(w)      (w->spinbox.show_value_proc)
#define SpinboxShowValueData(w)     (w->spinbox.show_value_data)
#define SpinboxStyle(w)       (w->spinbox.spinbox_style)
#define SpinboxTextUpdateConstantly(w) (w->spinbox.text_update_constantly)
#define SpinboxTF(w)       (w->spinbox.tf)
#define SpinboxTFColumns(w)      (w->spinbox.tf_columns)
#define SpinboxType(w)        (w->spinbox.spinbox_type)
#define SpinboxUpBtn(w)       (w->spinbox.up_btn)
#define SpinboxUseClosestValue(w)   (w->spinbox.use_closest_value)
#define SpinboxAutoCorrect(w)   (w->spinbox.auto_correct)
#define SpinboxValue(w)       (w->spinbox.val_now)
#define SpinboxValueChangedCBL(w)   (w->spinbox.ValueChangedCBL)

/* Class field access macros */

#define	SpinboxStyleId			(xmpSpinboxClassRec.spinbox_class.spinbox_style_id)
#define	SpinboxTypeId			(xmpSpinboxClassRec.spinbox_class.spinbox_type_id)

#endif /* _XmpSpinboxP_h */
