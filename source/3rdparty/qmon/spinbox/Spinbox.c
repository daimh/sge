/**
***
***  XmpSpinbox widget class -- an extensible data selection widget.
***  Copyright (C) 1995 Charles S. Kerr (cskerr@delenn.jccbi.gov)
***  Comments, and criticisms, will all be greatly appreciated.
***
***  Use of this library has certain conditions that are easy to
***  fulfill.  For more information, read the file LICENSE.html that
***  came with this widget.
***
***  This widget is distributed in the hope that it will be useful,
***  but WITHOUT ANY WARRANTY; without even the implied warranty of
***  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
***  file LICENSE.html for more details.
***
***  Advance warning: set your term to 132 cols. :)
***
**/

static const char *version =
   "XmpSpinbox Version 1.3  96/01/18  Author Charles S. Kerr";

/**
***  Header Files
**/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( SUN4 ) && !defined( SVR4 )
#include <sys/limits.h>
extern long strtol();
#else
#include <limits.h>
#endif

#ifdef SPRINTF_BROKEN
#include <stdarg.h>
#endif

#include <Xm/TextF.h>
#include <Xm/ArrowB.h>

#include "SpinboxP.h"

/**
***  XmTextFieldPeek* will allow us to avoid having to call
***  XmTextFieldGetString (which involves an unnecessary call to
***  XtMalloc and XtFree) and strlen().  These are the only
***  two places where we use the info from the TextFP header.
***
***  DO NOT USE XmTextFieldPeek* if you are working with
***  wide character sets (usually for I18N).
**/

#if defined(XMTF_PEEK)
#include <Xm/TextFP.h>
#define XmTextFieldPeekString(w) ((const char*)TextF_Value(w))
#define XmTextFieldPeekLength(w) ((int)(((XmTextFieldWidget)(w))->text.string_length))
#endif


/**
***  My compiler complains when I try to convert ints to
***  pointers, implicitly or explicitly, because I'm running
***  on an AXP where sizeof(int)==32 and sizeof(XtPointer)==64.
***  Creating an intermediate step that converts the int to
***  whatever the integral of the same size as a pointer
***  satisfies the compiler.
**/

#define int2xtp(a) ((XtPointer)((PointerSizedIntegral)(a)))
#define xtp2int(a) ((int)((PointerSizedIntegral)(a)))

/**
***  When user presses arrowbutton down, the value will click
***  up/down XmNincrement.  If the arrowbutton is held down, after 
***  INITIAL_DELAY milliseconds the values will start scrolling
***  at the rate of once per SECOND_DELAY milliseconds,
***  speeding up until we are scrolling at the rate of
***  once per MINIMUM_DELAY seconds.  Every time we speed up,
***  we cut DELAY_INCREMENT ms off of our time.
**/

#define INITIAL_DELAY 300
#define SECOND_DELAY 100
#define MINIMUM_DELAY 5
#define DELAY_DECREMENT 3

/**
***  Convenience macros, mainly used when determining
***  the layout of the textfield and arrowbuttons.
**/

#define min2(a,b) ((a)<(b)?(a):(b))
#define max2(a,b) ((a)>(b)?(a):(b))
#define min3(a,b,c) (min2(min2((a),(b)),(c)))
#define max3(a,b,c) (max2(max2((a),(b)),(c)))
#ifndef XtWidth
#define XtWidth(w)	((w)->core.width)
#endif
#ifndef XtHeight
#define XtHeight(w)	((w)->core.height)
#endif

/**
***  the *_show_value functions for the various XmNspinboxTypes.
**/

static SpinboxShowValueProc
   alpha_show_value,
   bc_show_value,
   clock_hm_show_value,
   clock_hms_show_value,
   clock_dhms_show_value,
   decimal_show_value,
   dollar_show_value,
   long_show_value;

/**
***  the *_get_value functions for the various XmNspinboxTypes.
**/

static SpinboxGetValueProc
   alpha_get_value,
   bc_get_value,
   clock_hm_get_value,
   clock_hms_get_value,
   clock_dhms_get_value,
   decimal_get_value,
   dollar_get_value,
   long_get_value;

/**
***  callbacks to keep the values spinning when the arrowbuttons
***  are pressed and held down.
**/

static void up_activate_cb		( Widget, XtPointer, XtPointer );
static void up_arm_cb			( Widget, XtPointer, XtPointer );
static void down_activate_cb		( Widget, XtPointer, XtPointer );
static void down_arm_cb			( Widget, XtPointer, XtPointer );
static void ab_disarm_cb		( Widget, XtPointer, XtPointer );

/**
***  Actions
**/

static void Action_Decrement_Small	( Widget, XEvent*, String*, Cardinal* );
static void Action_Decrement_Large	( Widget, XEvent*, String*, Cardinal* );
static void Action_Increment_Small	( Widget, XEvent*, String*, Cardinal* );
static void Action_Increment_Large	( Widget, XEvent*, String*, Cardinal* );

/**
***  Miscellaneous Prototypes
**/

static Boolean CvtStringToLong (
   Display *dpy,
   XrmValuePtr args,
   Cardinal *num_args,
   XrmValuePtr from,
   XrmValuePtr to,
   XtPointer *data );

#ifdef SPRINTF_BROKEN
static int int_sprintf			( char *buffer, const char *fmt, ... );
#else
#define int_sprintf sprintf
#endif

static Boolean did_user_specify		( const char *res, const ArgList, Cardinal );
static void set_tf_cb			( XmpSpinboxWidget cw, Boolean is_on );
static void get_text_field		( XmpSpinboxWidget );
static void set_text_field		( XmpSpinboxWidget );
static Boolean validate_new_inc_large	( XmpSpinboxWidget, long value );
static Boolean validate_new_minimum	( XmpSpinboxWidget, long value );
static Boolean validate_new_maximum	( XmpSpinboxWidget, long value );
static Boolean validate_new_delay	( XmpSpinboxWidget, int value );
static Boolean validate_new_type	( XmpSpinboxWidget, int value );
static Boolean validate_new_style	( XmpSpinboxWidget, int value );
static Boolean validate_new_value	( XmpSpinboxWidget, long *value );
static void set_spinbox_value		( XmpSpinboxWidget cw, int value, Boolean notify );
static void set_getshow_procs		( XmpSpinboxWidget cw );
static void set_default_min		( XmpSpinboxWidget cw );
static void set_default_max		( XmpSpinboxWidget cw );
static void set_default_inc_large	( XmpSpinboxWidget cw );
static void set_default_button_ratio	( XmpSpinboxWidget cw );
static void set_arrow_orientation	( XmpSpinboxWidget cw );
static void set_sensitive		( XmpSpinboxWidget cw, Boolean sensitive );
static Boolean validate_new_button_size_ratio	( XmpSpinboxWidget, int ratio );

/**
***  Manditory Widget Functions
**/

static	void	ClassInitialize		( void );
static	void	Initialize		( Widget, Widget, ArgList, Cardinal* );
static	Boolean	SetValues		( Widget, Widget, Widget, ArgList, Cardinal* );
static	void	Size			( Widget, Dimension*, Dimension* );
static	void	Resize			( Widget );

/**
***
***  Translation Table
***
**/

static XtTranslations Translations;

static char translations[] =
   "<Key>osfUp:		IncrementSmall()	\n\
    <Key>osfDown:	DecrementSmall()	\n\
    <Key>osfPageUp:	IncrementLarge()	\n\
    <Key>osfPageDown:	DecrementLarge()";

/**
***
***  Action Table
***
**/

static XtActionsRec actions[] = {
   { "DecrementSmall", Action_Decrement_Small },
   { "DecrementLarge", Action_Decrement_Large },
   { "IncrementSmall", Action_Increment_Small },
   { "IncrementLarge", Action_Increment_Large },
   { NULL, NULL }
};

/**
***
***  Resources List
***
**/
static XtResource resources[] = 
{
    {
       XmNarrowOrientation, XmCArrowOrientation,
       XmROrientation, sizeof (unsigned char),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.arrow_orientation),
       XtRImmediate, int2xtp(XmVERTICAL),
    },
    {
       XmNcolumns, XmCColumns,
       XmRShort, sizeof(short),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.tf_columns),
       XtRImmediate, int2xtp(8)
    },
    {
       XmNbuttonSizeRatio, XmCButtonSizeRatio,
       XmRInt, sizeof(int),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.button_size_ratio),
       XtRImmediate, int2xtp(5)
    },
    {
       XmNbuttonSizeFixed, XmCButtonSizeFixed,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.button_size_fixed),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNdecimalPoints, XmCDecimalPoints,
       XmRShort, sizeof (short),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.decimal_points),
       XtRImmediate, int2xtp(0),
    },
    {
       XmNgetValueData, XmCGetValueData,
       XtRPointer, sizeof(XtPointer),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.get_value_data),
       XtRImmediate, (XtPointer)0
    },
    {
       XmNgetValueProc, XmCGetValueProc,
       XmRGetValueProc, sizeof(SpinboxGetValueProc*),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.get_value_proc),
       XtRImmediate, (XtPointer)long_get_value
    },
    {
       XmNincrement, XmCIncrement,
       XmRLong, sizeof(long),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.increment),
       XtRImmediate, int2xtp(1)
    },
    {
       XmNincrementLarge, XmCIncrementLarge,
       XmRLong, sizeof(long),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.increment_large),
       XtRImmediate, int2xtp(5)
    },
    {
       XmNitemCount, XmCItemCount,
       XtRInt, sizeof(int),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.item_count),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNitems, XmCItems,
       XtRStringArray, sizeof(String*),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.items),
       XtRImmediate, (XtPointer)0
    },
    {
       XmNitemsAreSorted, XmCItemsAreSorted,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.items_are_sorted),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNmaximum, XmCMaximum,
       XmRLong, sizeof(long),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.val_max),
       XtRImmediate, int2xtp(LONG_MAX-1) /* before int2xtp(0) */
    },
    {
       XmNminimum, XmCMinimum,
       XmRLong, sizeof(long),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.val_min),
       XtRImmediate, int2xtp(LONG_MIN+1) /* before int2xtp(0) */
    },
    {
       XmNshowValueData, XmCShowValueData,
       XtRPointer, sizeof(XtPointer),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.show_value_data),
       XtRImmediate, (XtPointer)0
    },
    {
       XmNshowValueProc, XmCShowValueProc,
       XmRShowValueProc, sizeof(SpinboxShowValueProc*),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.show_value_proc),
       XtRImmediate, (XtPointer)long_show_value
    },
    {
       XmNspinboxCycle, XmCSpinboxCycle,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.cycle),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNspinboxStyle, XmCSpinboxStyle,
       XmRSpinboxStyle, sizeof (unsigned char),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.spinbox_style),
       XtRImmediate, int2xtp(XmSPINBOX_STACKED_RIGHT),
    },
    {
       XmNspinboxType, XmCSpinboxType,
       XmRSpinboxType, sizeof (unsigned char),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.spinbox_type),
       XtRImmediate, int2xtp(XmSPINBOX_NUMBER),
    },
    {
       XmNspinboxUseClosestValue, XmCSpinboxUseClosestValue,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.use_closest_value),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNspinboxAutoCorrect, XmCSpinboxAutoCorrect,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.auto_correct),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNupdateText, XmCUpdateText,
       XmRBoolean, sizeof(Boolean),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.text_update_constantly),
       XtRImmediate, int2xtp(False)
    },
    {
       XmNvalue, XmCValue,
       XmRLong, sizeof(long),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.val_now),
       XtRImmediate, int2xtp(0)
    },
    {
       XmNvalueChangedCallback, XmCCallback,
       XmRCallback, sizeof (XtCallbackList),
       XtOffsetOf(struct _XmpSpinboxRec, spinbox.ValueChangedCBL),
       XtRCallback, (XtPointer)0,
    }
};


/**
***
***  The Widget CLASS RECORD
***
**/

externaldef(xmspinboxclassrec) XmpSpinboxClassRec xmpSpinboxClassRec = 
{
  {                                           /* core_class            */
    (WidgetClass)&xmpGeometryClassRec,        /* superclass            */
    "XmpSpinbox",                             /* class_name            */
    sizeof(XmpSpinboxRec),                    /* widget_size           */
    ClassInitialize,                          /* class_initialize      */
    NULL,                                     /* class_part_initialize */
    FALSE,                                    /* class_inited          */
    Initialize,                               /* initialize            */
    NULL,                                     /* initialize_hook       */
    XtInheritRealize,                         /* realize               */
    NULL,                                     /* actions               */
    0,                                        /* num_actions           */
    resources,                                /* resources             */
    XtNumber(resources),                      /* num_resources         */
    NULLQUARK,                                /* xrm_class             */
    TRUE,                                     /* compress_motion       */
    XtExposeCompressMaximal,                  /* compress_exposure     */
    TRUE,                                     /* compress_enterleave   */
    FALSE,                                    /* visible_interest      */
    NULL,                                     /* destroy               */
    Resize,                                   /* resize                */
    XtInheritExpose,                          /* expose                */
    SetValues,                                /* set_values            */
    NULL,                                     /* set_values_hook       */
    XtInheritSetValuesAlmost,                 /* set_values_almost     */
    NULL,                                     /* get_values_hook       */
    NULL,                                     /* accept_focus          */
    XtVersion,                                /* version               */
    NULL,                                     /* callback_private      */
    XtInheritTranslations,                    /* tm_table              */
    XtInheritQueryGeometry,                   /* query_geometry        */
    NULL,                                     /* display_accelerator   */
    NULL                                      /* extension             */
  },

  {                                           /* composite_class       */
    XtInheritGeometryManager,                 /* geometry_manager      */
    XtInheritChangeManaged,                   /* change_managed        */
    XtInheritInsertChild,                     /* insert_child          */
    XtInheritDeleteChild,                     /* delete_child          */
    NULL                                      /* extension             */
  },

  {                                           /* constraint_class      */
    NULL,                                     /* resources             */   
    0, 			                      /* num_resources         */   
    sizeof(XmpSpinboxConstraintRec),          /* constraint_size       */
    NULL,                                     /* constraint_initialize */
    NULL,                                     /* destroy               */   
    NULL,                                     /* set_values            */   
    NULL                                      /* extension             */
  },

  {                                           /* manager class         */
    XtInheritTranslations,                    /* translations          */
    NULL,                                     /* syn_resources         */
    0,                                        /* num_syn_resources     */
    NULL,                                     /* syn_constraint_resources */
    0,                                        /* num_syn_constraint_resources */
    XmInheritParentProcess,                   /* parent_process        */
    NULL                                      /* extension             */    
  },

  {                                           /* geometry class        */
    XmpInheritBitGravity,                     /* bit_gravity           */
    XmpInheritInitializePostHook,             /* initialize_post_hook  */
    XmpInheritSetValuesPostHook,              /* set_values_post_hook  */
    XmpInheritConstraintInitializePostHook,   /* constraint_initialize_post_hook*/
    XmpInheritConstraintSetValuesPostHook,    /* constraint_set_values_post_hook*/
    Size,                                     /* size                  */
    NULL                                      /* extension             */    
  },
  
  {                                           /* spinbox class         */
#if XmVersion > 1001
    0,					                         /* spinbox_type_id       */
    0,					                         /* spinbox_style_id      */
#endif
    NULL                                      /* extension             */    
  }        
};
externaldef(xmpspinboxwidgetclass) WidgetClass xmpSpinboxWidgetClass =
			                 (WidgetClass) &xmpSpinboxClassRec;


static String SpinboxTypeNames[] = {
   "spinbox_number",
   "spinbox_clock_hm",
   "spinbox_clock_hms",
   "spinbox_clock_dhms",
   "spinbox_beaconcode",
   "spinbox_dollars",
   "spinbox_strings",
   "spinbox_userdefined"
};

static String SpinboxStyleNames[] = {
   "spinbox_stacked",
   "spinbox_stacked_left",
   "spinbox_stacked_right",
   "spinbox_left",
   "spinbox_right",
   "spinbox_separate"
};

#if XmVersion <= 1001
static String SpinboxPrefixes[] = {"Xm", NULL};

static int SpinboxTypeValues[] = {
   XmSPINBOX_NUMBER,
   XmSPINBOX_CLOCK_HM,
   XmSPINBOX_CLOCK_HMS,
   XmSPINBOX_CLOCK_DHMS,
   XmSPINBOX_BEACONCODE,
   XmSPINBOX_DOLLARS,
   XmSPINBOX_STRINGS,
   XmSPINBOX_USER_DEFINED 
};   

static int SpinboxStyleValues[] = {
   XmSPINBOX_STACKED,
   XmSPINBOX_STACKED_LEFT,
   XmSPINBOX_STACKED_RIGHT, 
   XmSPINBOX_LEFT,
   XmSPINBOX_RIGHT,
   XmSPINBOX_SEPARATE 
};

#endif

   
/*
**  And finally, after 500+ lines, the program begins. . .
*/

/************************************************************************
 *
 *  ClassInitialize
 *
 ************************************************************************/

static void
ClassInitialize(void)
{


    /**
    ***  Make the compiler happy, it wants "version" to be used
    ***  if it's defined.
    **/
    version = version;

#if XmVersion > 1001
    SpinboxStyleId = XmRepTypeRegister ( XmRSpinboxStyle,
       SpinboxStyleNames, NULL,
       XtNumber(SpinboxStyleNames));

    SpinboxTypeId = XmRepTypeRegister ( XmRSpinboxType,
       SpinboxTypeNames, NULL,
       XtNumber(SpinboxTypeNames));
#else
    XmtRegisterEnumConverter(XmRSpinboxStyle, SpinboxStyleNames,
                              SpinboxStyleValues, XtNumber(SpinboxStyleNames),
                              SpinboxPrefixes);
    
    XmtRegisterEnumConverter(XmRSpinboxType, SpinboxTypeNames,
                              SpinboxTypeValues, XtNumber(SpinboxTypeNames),
                              SpinboxPrefixes);
#endif
   
   XtSetTypeConverter (
     XmRString, XmRLong,
     CvtStringToLong, NULL, 0,
     XtCacheNone, NULL );

   Translations = XtParseTranslationTable ( translations );

}

static Boolean did_user_specify (
   const char *resource_name,
   const ArgList args,
   Cardinal qty )
{
   Cardinal i;

   for ( i=0; i<qty; i++ ) {
      if ( !strcmp ( args[i].name, resource_name ) ) {
         return True;
      }
   }
   return False;
}

/************************************************************************
 *
 *  Initialize
 *
 ************************************************************************/

static void Initialize (
   Widget request_w,
   Widget new_w,
   ArgList args,
   Cardinal* num_args )
{
   Widget w;
   char tmpstr[256];
   XmpSpinboxWidget cw = (XmpSpinboxWidget)new_w;


   SpinboxOldValue(cw) = LONG_MAX;
   
   SpinboxContext(cw) = XtWidgetToApplicationContext(new_w);
   XtAppAddActions ( SpinboxContext(cw), actions, XtNumber(actions) );

   validate_new_type ( cw, SpinboxType(cw) );
   set_getshow_procs ( cw );

   /**
   ***  did_user_specify() is better for SetValues than for Initialize
   ***  because it doesn't take reosurce file settings into account.
   ***  However, we can still use it here if we're a little more careful.
   **/

   if (!did_user_specify(XmNminimum,args,*num_args) && 
         SpinboxMinValue(cw)==LONG_MIN+1) 
      set_default_min(cw);
   if (!did_user_specify(XmNmaximum,args,*num_args) && 
         SpinboxMaxValue(cw)==LONG_MAX-1) 
      set_default_max(cw);
   if (!did_user_specify(XmNincrementLarge,args,*num_args) &&
               SpinboxIncLarge(cw)==0) 
      set_default_inc_large(cw);
   if (!did_user_specify(XmNbuttonSizeRatio,args,*num_args)) /* ccc */
      set_default_button_ratio(cw);

   /**
   ***  Make sure the values are all hunky-dory
   **/

   validate_new_minimum(cw, SpinboxMinValue(cw));
   validate_new_maximum(cw, SpinboxMaxValue(cw));
   validate_new_inc_large(cw, SpinboxIncLarge(cw));
   validate_new_style(cw, SpinboxStyle(cw));

   /**
   ***  TextField
   **/

   (*SpinboxShowValue(cw))((Widget)cw, SpinboxShowValueData(cw), 
                              SpinboxValue(cw), tmpstr, sizeof(tmpstr));
   SpinboxTF(cw) = XtVaCreateManagedWidget("tf", xmTextFieldWidgetClass, new_w,
      XmNvalue, tmpstr,
      XmNcolumns, SpinboxTFColumns(cw),
      NULL );
   set_tf_cb ( cw, True );
   XtOverrideTranslations ( SpinboxTF(cw), Translations );

   /**
   ***  Up ArrowButton
   **/

   SpinboxUpBtn(cw) = w = XtVaCreateManagedWidget ( "up_ab", 
                           xmArrowButtonWidgetClass, new_w,
                           NULL );
   XtAddCallback ( w, XmNactivateCallback,   up_activate_cb, (XtPointer)cw );
   XtAddCallback ( w, XmNarmCallback,        up_arm_cb,      (XtPointer)cw );
   XtAddCallback ( w, XmNdisarmCallback,     ab_disarm_cb,   (XtPointer)cw );

   /**
   ***  Down ArrowButton
   **/

   SpinboxDownBtn(cw) = w = XtVaCreateManagedWidget ( "down_ab", 
                              xmArrowButtonWidgetClass, new_w,
                              NULL );
   XtAddCallback ( w, XmNactivateCallback, down_activate_cb, (XtPointer)cw );
   XtAddCallback ( w, XmNarmCallback,      down_arm_cb,      (XtPointer)cw );
   XtAddCallback ( w, XmNdisarmCallback,   ab_disarm_cb,   (XtPointer)cw );

   set_arrow_orientation ( cw );
   if ( SpinboxSensitive(cw) )
      set_sensitive ( cw, True );


   XmpGeometryInitialize ( xmpSpinboxWidgetClass, request_w, new_w, args, num_args );
}




/************************************************************************
 *
 *  Set Values
 *
 ************************************************************************/

static Boolean SetValues (
   Widget old_w,
   Widget request_w,
   Widget new_w,
   ArgList args,
   Cardinal* num_args )
{
   XmpSpinboxWidget old = (XmpSpinboxWidget)old_w;
   XmpSpinboxWidget new = (XmpSpinboxWidget)new_w;
   Boolean needs_set_text_field = False;
   Boolean reconfigure = False;

   
   /**
   ***  See if type has changed..
   **/

   if ( SpinboxType(new) != SpinboxType(old) )
   {
      if ( !validate_new_type ( new, SpinboxType(new) ) )
         SpinboxType(new) = SpinboxType(old);
      else {
         set_getshow_procs ( new );
         if ( !did_user_specify( XmNminimum, args, *num_args ) ) 
            set_default_min ( new );
         if ( !did_user_specify( XmNmaximum, args, *num_args ) ) 
            set_default_max ( new );
         if ( !did_user_specify( XmNincrementLarge, args, *num_args ) ) 
            set_default_inc_large ( new );
         validate_new_minimum ( new, SpinboxMinValue(new) );
         validate_new_maximum ( new, SpinboxMaxValue(new) );
         validate_new_inc_large ( new, SpinboxIncLarge(new) );
         needs_set_text_field = True;
      }
   }

   /**
   ***  See size ratio has changed...
   **/

   if ( !SpinboxButtonSizeFixed(new) && 
         SpinboxButtonSizeRatio(new) != SpinboxButtonSizeRatio(old) )
   {
      if ( !validate_new_button_size_ratio ( new, SpinboxButtonSizeRatio(new) ) )
         SpinboxButtonSizeRatio(new) = SpinboxButtonSizeRatio(old);
      else
         reconfigure = True;
   }

   /**
   ***  See style has changed...
   **/

   if ( SpinboxStyle(new) != SpinboxStyle(old) )
   {
      if ( !validate_new_style ( new, SpinboxStyle(new) ) )
         SpinboxStyle(new) = SpinboxStyle(old);
      else {
         if ( !did_user_specify( XmNbuttonSizeRatio, args, *num_args ) ) 
            set_default_button_ratio( new );
         needs_set_text_field = True;
         reconfigure = True;
      }
   }

   /**
   ***  See delay has changed...
   **/

   if ( SpinboxDelay(new) != SpinboxDelay(old) )
      if ( !validate_new_delay ( new, SpinboxDelay(new) ) )
         SpinboxDelay(new) = SpinboxDelay(old);

   /**
   ***  See if sensitive has changed...
   **/

   if ( SpinboxSensitive(new) != SpinboxSensitive(old) )
      set_sensitive ( new, SpinboxSensitive(new) );

   /**
   ***  See if min value has changed...
   **/

   if ( SpinboxMinValue(new) != SpinboxMinValue(old) )
   {
      if ( !validate_new_minimum ( new, SpinboxMinValue(new) ) )
      {
         SpinboxMinValue(new) = SpinboxMinValue(old);
         if ( !did_user_specify ( XmNincrementLarge, args, *num_args ) )
            set_default_inc_large ( new );
         validate_new_inc_large ( new, SpinboxIncLarge(new) );
      }
   }

   /**
   ***  See if maximum has changed...
   **/

   if ( SpinboxMaxValue(new) != SpinboxMaxValue(old) )
   {
      if ( !validate_new_maximum ( new, SpinboxMaxValue(new) ) )
      {
         SpinboxMaxValue(new) = SpinboxMaxValue(old);
         if ( !did_user_specify ( XmNincrementLarge, args, *num_args ) )
            set_default_inc_large ( new );
         validate_new_inc_large ( new, SpinboxIncLarge(new) );
      }
   }

   /**
   ***  See if value has changed...
   **/

   if ( SpinboxValue(new) != SpinboxValue(old) )
   {
      /* XmpSpinboxSetValue wants to do the work itself, 
         so take a step backwards */
      long value = SpinboxValue(new);
      SpinboxValue(new) = SpinboxValue(old);
      XmpSpinboxSetValue ( (Widget)new, value, True );
   }

   /**
   ***  See if columns has changed... (todo: send these to tf)
   **/

   if ( SpinboxTFColumns(new) != SpinboxTFColumns(old) ) {
      SpinboxTFColumns(new) = SpinboxTFColumns(old);
      XtVaSetValues ( SpinboxTF(new), XmNcolumns, SpinboxTFColumns(new), NULL );
   }

   /**
   ***  See if decimal places has changed...
   **/

   if ( SpinboxDecimalPoints(new) != SpinboxDecimalPoints(old) ) {
      if ( !did_user_specify( XmNincrementLarge, args, *num_args ) ) 
         set_default_inc_large ( new );
      validate_new_inc_large ( new, SpinboxIncLarge(new) );
      set_getshow_procs ( new ); /* to set the get/display procs 
                                    if type is SPINBOX_NUMBER */
      needs_set_text_field = True;
   }

   /**
   ***  See if arrow orientation has changed...
   **/

   if ( SpinboxArrowOrientation(new) != SpinboxArrowOrientation(old) )
   {
      set_arrow_orientation ( new );
   }

   Reconfigure(new) = reconfigure;
   reconfigure |= XmpGeometrySetValues ( xmpSpinboxWidgetClass,
      old_w, request_w, new_w,
      args, num_args );

   if ( needs_set_text_field )
      set_text_field ( new );

   /* just in case XmpGeometrySetValues is
      constrained by a higher parent.. */
   if ( reconfigure )
      Resize ( new_w );

   return ( reconfigure );
}

/************************************************************************
 *
 *  Size
 *
 ************************************************************************/

static void Size (
   Widget w,
   Dimension* spinboxWidth,
   Dimension* spinboxHeight )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   const double ratio = SpinboxButtonSizeRatio(cw);
   Dimension width=0, height=0;
   Dimension ubw=0, ubh=0;
   Dimension dbw=0, dbh=0;
   Dimension tfw=0, tfh=0;


   XmpPreferredGeometry ( SpinboxUpBtn(cw), &ubw, &ubh );
   XmpPreferredGeometry ( SpinboxDownBtn(cw), &dbw, &dbh );
   XmpPreferredGeometry ( SpinboxTF(cw), &tfw, &tfh );

   switch ( SpinboxStyle(cw) )
   {

      case XmSPINBOX_STACKED :
         width = tfw;
         height = (int)(tfh * 11.0 / 5.0);
         break;

      case XmSPINBOX_STACKED_LEFT :
      case XmSPINBOX_STACKED_RIGHT :
         height = tfh;
         if (SpinboxButtonSizeFixed(cw))
            width = tfw + ubw;
         else
            width = (int)(tfw*(1 + ratio)/ratio);
         break;

      case XmSPINBOX_LEFT :
      case XmSPINBOX_RIGHT :
      case XmSPINBOX_SEPARATE :
      default :
         height = tfh;
         if (SpinboxButtonSizeFixed(cw))
            width = tfw + ubw;
         else
            width = (int)(tfw * (2 + ratio)/ratio);
         break;
   }

   *spinboxWidth = max2 ( 1, width );
   *spinboxHeight = max2 ( 1, height );

}



/************************************************************************
 *
 *  Resize 
 *
 ************************************************************************/

static void Resize (
   Widget w )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   const double ratio = SpinboxButtonSizeRatio(cw);
   const Dimension width = XtWidth ( cw );
   const Dimension height = XtHeight ( cw );
   Dimension ubx=0, uby=0, ubw=0, ubh=0;
   Dimension dbx=0, dby=0, dbw=0, dbh=0;
   Dimension tfx=0, tfy=0, tfw=0, tfh=0;

   XmpPreferredGeometry ( SpinboxUpBtn(cw), &ubw, &ubh );
   XmpPreferredGeometry ( SpinboxDownBtn(cw), &dbw, &dbh );
   XmpPreferredGeometry ( SpinboxTF(cw), &tfw, &tfh );

   switch ( SpinboxStyle(cw) )
   {
      case XmSPINBOX_STACKED :
         ubw = dbw = tfw = width;
         tfh = (int)(height * 5.0/11.0);
         ubh = dbh = (int)(height * 3.0/11.0);

         ubx = uby = tfx = dbx = 0;
         tfy = ubh;
         dby = tfy + tfh;
         break;

      case XmSPINBOX_STACKED_LEFT :
         tfh = height;
         ubh = dbh = height / 2;
         if (SpinboxButtonSizeFixed(cw)) {
            tfw = width - ubw; 
         }
         else {
            tfw = (int)(ratio*width)/(ratio + 1);
            ubw = dbw = width - tfw;
         }

         uby = tfy = ubx = dbx = 0;
         tfx = ubw;
         dby = ubh;
         break;

      case XmSPINBOX_STACKED_RIGHT :
         tfh = height;
         ubh = dbh = height / 2;
         if (SpinboxButtonSizeFixed(cw)) {
            tfw = width - ubw; 
         }
         else {
            tfw = (int)(ratio*width)/(ratio + 1);
            ubw = dbw = width - tfw;
         }

         tfx = tfy = uby = 0;
         ubx = dbx = tfw;
         dby = ubh;
         break;

      case XmSPINBOX_LEFT :
         tfh = ubh = dbh = height;
         ubw = dbw = (int)(width / (2 + ratio));
         tfw = width - ubw - dbw;

         uby = dby = tfy = tfx = 0;
         ubx = dbw;
         tfx = ubx + ubw;
         break;

      case XmSPINBOX_RIGHT :
         tfh = ubh = dbh = height;
         ubw = dbw = (int)(width / (2 + ratio));
         tfw = width - ubw - dbw;

         tfx = tfy = dby = uby = 0;
         dbx = tfw;
         ubx = dbx + dbw;
         break;

      case XmSPINBOX_SEPARATE :
         tfh = ubh = dbh = height;
         ubw = dbw = (int)(width / (2 + ratio));
         tfw = width - ubw - dbw;

         tfy = dby = uby = dbx = 0;
         tfx = dbw;
         ubx = tfx + tfw;
         break;
   }

   XmpSetGeometry ( SpinboxUpBtn(cw), ubx, uby, max2(ubw,1), max2(ubh,1), 0 );
   XmpSetGeometry ( SpinboxDownBtn(cw), dbx, dby, max2(dbw,1), max2(dbh,1), 0 );
   XmpSetGeometry ( SpinboxTF(cw), tfx, tfy, max2(tfw,1), max2(tfh,1), 0 );
}

/************************************************************************
 *
 *  XmpCreateSpinbox
 *
 ************************************************************************/

Widget
XmpCreateSpinbox (
   Widget parent,
   char* name,
   Arg* args,
   Cardinal num_args)
{
    return(XtCreateWidget(name, xmpSpinboxWidgetClass, parent, args, num_args));
}


/**
***   Default get/set textfield functions
**/

static void set_text_field (
   XmpSpinboxWidget cw )
{
   char buffer[128];
   

   (*SpinboxShowValue(cw))(
      (Widget)cw,
      SpinboxShowValueData(cw),
      SpinboxValue(cw),
      buffer, sizeof(buffer));

   XmTextFieldSetString ( SpinboxTF(cw), buffer );

}

static void get_text_field (
   XmpSpinboxWidget cw )
{
   Boolean result=0;
   long val=0;
   

#if defined(XMTF_PEEK)
   result = (*SpinboxGetValue(cw))(
      (Widget)cw, SpinboxGetValueData(cw),
      XmTextFieldPeekString(SpinboxTF(cw)),
      XmTextFieldPeekLength(SpinboxTF(cw)),
      &val );
#else
   char *pchar = XmTextFieldGetString(SpinboxTF(cw));
   result = (*SpinboxGetValue(cw))(
      (Widget)cw, SpinboxGetValueData(cw),
      pchar, strlen(pchar),
      &val );
   XtFree ( pchar );
#endif


   /**
   ***  If error, reset text field.
   **/

   if ( !result )
      set_text_field ( cw );
   else if ( validate_new_value ( cw, &val ) )
      XmpSpinboxSetValue ( (Widget)cw, val, True );

}


/***
****
****  TextField / ArrowButton Callbacks
****
***/

static void tf_valchange_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   get_text_field ( (XmpSpinboxWidget)client );
   set_text_field ( (XmpSpinboxWidget)client );
}

static void set_tf_cb (
   XmpSpinboxWidget cw,
   Boolean is_on )
{
   Widget tf = SpinboxTF(cw);
   
   /* we don't want error checking when arrowbuttons are on,
      so this func can turn tf error checking on/off */
   if ( is_on ) {
      if ( SpinboxTextUpdateConstantly(cw) ) {
         XtAddCallback ( tf, XmNvalueChangedCallback, 
                           tf_valchange_cb, (XtPointer)cw );
      } else {
         XtAddCallback ( tf, XmNactivateCallback, 
                           tf_valchange_cb, (XtPointer)cw );
         XtAddCallback ( tf, XmNlosingFocusCallback, 
                           tf_valchange_cb, (XtPointer)cw );
         XtAddCallback ( tf, XmNlosePrimaryCallback, 
                           tf_valchange_cb, (XtPointer)cw );
      }
   } else {
      if ( SpinboxTextUpdateConstantly(cw) ) {
         XtRemoveCallback ( tf, XmNvalueChangedCallback, 
                              tf_valchange_cb, (XtPointer)cw );
      } else {
         XtRemoveCallback ( tf, XmNactivateCallback, 
                              tf_valchange_cb, (XtPointer)cw );
         XtRemoveCallback ( tf, XmNlosingFocusCallback, 
                              tf_valchange_cb, (XtPointer)cw);
         XtRemoveCallback ( tf, XmNlosePrimaryCallback, 
                              tf_valchange_cb, (XtPointer)cw);
      }
   }
}


static void up_activate_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   XmpSpinboxSetValue((Widget)cw, SpinboxValue(cw)+SpinboxIncrement(cw), True);
}

static void up_armloop_cb (
   XtPointer client,
   XtIntervalId *pId )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   up_activate_cb ( NULL, client, NULL );
   if ( SpinboxDelay(cw)==INITIAL_DELAY ) SpinboxDelay(cw) = SECOND_DELAY;
   else if ( SpinboxDelay(cw)-DELAY_DECREMENT >= MINIMUM_DELAY )
      SpinboxDelay(cw) -= DELAY_DECREMENT;
   SpinboxInterval(cw) = XtAppAddTimeOut( SpinboxContext(cw), 
                              SpinboxDelay(cw), up_armloop_cb, client );
}
static void up_arm_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   SpinboxDelay(cw) = INITIAL_DELAY;
   set_tf_cb ( cw, False );
   SpinboxInterval(cw) = XtAppAddTimeOut( SpinboxContext(cw), 
                           SpinboxDelay(cw), up_armloop_cb, client );
}
static void ab_disarm_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   XtRemoveTimeOut ( SpinboxInterval(cw) );
   set_tf_cb ( cw, True );
}


static void down_activate_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   XmpSpinboxSetValue((Widget)cw, SpinboxValue(cw)-SpinboxIncrement(cw), True);
}
static void down_armloop_cb (
   XtPointer client,
   XtIntervalId *pId )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   down_activate_cb ( NULL, client, NULL );
   if ( SpinboxDelay(cw)==INITIAL_DELAY ) SpinboxDelay(cw) = SECOND_DELAY;
   else if ( SpinboxDelay(cw)-DELAY_DECREMENT >= MINIMUM_DELAY )
      SpinboxDelay(cw) -= DELAY_DECREMENT;
   SpinboxInterval(cw) = XtAppAddTimeOut( SpinboxContext(cw), 
                           SpinboxDelay(cw), down_armloop_cb, client );
}
static void down_arm_cb (
   Widget w,
   XtPointer client,
   XtPointer call )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)client;
   SpinboxDelay(cw) = INITIAL_DELAY;
   set_tf_cb ( cw, False );
   SpinboxInterval(cw) = XtAppAddTimeOut ( SpinboxContext(cw), 
                           SpinboxDelay(cw), down_armloop_cb, client );
}

/**
***
***   SET routines...
***
**/

static void set_getshow_procs (
   XmpSpinboxWidget cw )
{
   switch ( SpinboxType(cw) ) {

      case XmSPINBOX_CLOCK_HM :
         SpinboxShowValue(cw) = clock_hm_show_value;
         SpinboxGetValue(cw) = clock_hm_get_value;
         break;

      case XmSPINBOX_CLOCK_HMS :
         SpinboxShowValue(cw) = clock_hms_show_value;
         SpinboxGetValue(cw) = clock_hms_get_value;
         break;

      case XmSPINBOX_CLOCK_DHMS :
         SpinboxShowValue(cw) = clock_dhms_show_value;
         SpinboxGetValue(cw) = clock_dhms_get_value;
         break;

      case XmSPINBOX_BEACONCODE :
         SpinboxShowValue(cw) = bc_show_value;
         SpinboxGetValue(cw) = bc_get_value;
         break;

      case XmSPINBOX_DOLLARS :
         SpinboxShowValue(cw) = dollar_show_value;
         SpinboxGetValue(cw) = dollar_get_value;
         break;

      case XmSPINBOX_STRINGS :
         SpinboxShowValue(cw) = alpha_show_value;
         SpinboxGetValue(cw) = alpha_get_value;
         break;

      case XmSPINBOX_USER_DEFINED :
         break;

      case XmSPINBOX_NUMBER :
         if ( SpinboxDecimalPoints(cw) ) {
            SpinboxShowValue(cw) = decimal_show_value;
            SpinboxGetValue(cw) = decimal_get_value;
         } else {
            SpinboxShowValue(cw) = long_show_value;
            SpinboxGetValue(cw) = long_get_value;
         }
         break;

      default :
         assert ( 0 );
         break;
   }
}

static void set_default_inc_large (
    XmpSpinboxWidget cw  )
{
   switch ( SpinboxType(cw) ) {
      case XmSPINBOX_CLOCK_HM :
      case XmSPINBOX_CLOCK_HMS :
      case XmSPINBOX_CLOCK_DHMS :
         SpinboxIncLarge(cw) = 60;
         break;
      case XmSPINBOX_DOLLARS :
         SpinboxIncLarge(cw) = 100;
         break;
      case XmSPINBOX_NUMBER :
         if ( SpinboxDecimalPoints(cw) ) {
            int i;
            for ( i=0, SpinboxIncLarge(cw)=1;
                  i<SpinboxDecimalPoints(cw);
                  i++ )
               SpinboxIncLarge(cw) *= 10;
         } else {
            SpinboxIncLarge(cw) = (int)((SpinboxMaxValue(cw)-SpinboxMinValue(cw))/10.0);
         }
         break;
      case XmSPINBOX_STRINGS :
      case XmSPINBOX_BEACONCODE :
         SpinboxIncLarge(cw) = (int)((SpinboxMaxValue(cw)-SpinboxMinValue(cw))/10.0);
         break;
      case XmSPINBOX_USER_DEFINED :
         break;
      default :
         assert ( 0 );
         break;
   }
}
static void set_default_min ( XmpSpinboxWidget cw )
{
   switch ( SpinboxType(cw) ) {
      case XmSPINBOX_CLOCK_HM : SpinboxMinValue(cw) = 0; break;
      case XmSPINBOX_CLOCK_HMS : SpinboxMinValue(cw) = 0; break;
      case XmSPINBOX_CLOCK_DHMS : SpinboxMinValue(cw) = 0; break;
      case XmSPINBOX_BEACONCODE : SpinboxMinValue(cw) = 0; break;
      case XmSPINBOX_DOLLARS : SpinboxMinValue(cw) = LONG_MIN+1; break;
      case XmSPINBOX_STRINGS : SpinboxMinValue(cw) = 0; break;
      case XmSPINBOX_USER_DEFINED : break;
      case XmSPINBOX_NUMBER : SpinboxMinValue(cw) = LONG_MIN+1; break;
      default : assert ( 0 ); break;
   }
}

static void set_default_button_ratio (
   XmpSpinboxWidget cw )
{
   switch ( SpinboxStyle(cw) ) {
      case XmSPINBOX_STACKED_LEFT:
      case XmSPINBOX_STACKED_RIGHT:
      case XmSPINBOX_LEFT:
      case XmSPINBOX_RIGHT:
      case XmSPINBOX_STACKED:
      case XmSPINBOX_SEPARATE: SpinboxButtonSizeRatio(cw) = 5; break;
      default : assert ( 0 ); break;
   }
}

static void set_default_max ( XmpSpinboxWidget cw )
{
   switch ( SpinboxType(cw) ) {
      case XmSPINBOX_CLOCK_HM : SpinboxMaxValue(cw) = 24*60-1; break;
      case XmSPINBOX_CLOCK_HMS : SpinboxMaxValue(cw) = 24*60*60-1; break;
      case XmSPINBOX_CLOCK_DHMS : SpinboxMaxValue(cw) = 365*24*60*60-1; break;
      case XmSPINBOX_BEACONCODE : SpinboxMaxValue(cw) = 010000-1; break;
      case XmSPINBOX_DOLLARS : SpinboxMaxValue(cw) = LONG_MAX-1; break;
      case XmSPINBOX_STRINGS : SpinboxMaxValue(cw) = SpinboxItemCount(cw)-1; break;
      case XmSPINBOX_USER_DEFINED : break;
      case XmSPINBOX_NUMBER : SpinboxMaxValue(cw) = LONG_MAX-1; break;
      default : assert ( 0 ); break;
   }
}

static void set_sensitive (
   XmpSpinboxWidget cw,
   Boolean sensitive )
{
   XtVaSetValues ( SpinboxUpBtn(cw),    XmNsensitive, sensitive, NULL );
   XtVaSetValues ( SpinboxDownBtn(cw),  XmNsensitive, sensitive, NULL );
   XtVaSetValues ( SpinboxTF(cw),       XmNsensitive, sensitive, NULL );
}

static void set_arrow_orientation ( XmpSpinboxWidget nw )
{
   XtVaSetValues ( SpinboxUpBtn(nw),
      XmNarrowDirection, SpinboxArrowOrientation(nw)==XmVERTICAL ? XmARROW_UP : XmARROW_RIGHT,
      NULL );
   XtVaSetValues ( SpinboxDownBtn(nw),
      XmNarrowDirection, SpinboxArrowOrientation(nw)==XmVERTICAL ? XmARROW_DOWN : XmARROW_LEFT,
      NULL );
}


/**
***  Validation Routines...
**/
static Boolean validate_new_button_size_ratio (
   XmpSpinboxWidget cw,
   int ratio )
{
   return ( ratio > 0 );
}

static Boolean validate_new_inc_large (
   XmpSpinboxWidget cw,
   long value )
{
   unsigned range = SpinboxMaxValue(cw) - SpinboxMinValue(cw);

   if ( (unsigned)value > range ) {
      char tmpstr[512];
      (void) sprintf ( tmpstr,
         "Requested Large Increment (%ld) is larger than the range of the spinbox [%ld...%ld]!",
         value, SpinboxMinValue(cw), SpinboxMaxValue(cw) );
      XmpWarning ( (Widget)cw, tmpstr );
      return False;
   }
   if ( value < 0 ) {
      XmpWarning ( (Widget)cw, "\nRequested spinbox large increment must be greater than zero!" );
      return False;
   }
   return True;
}

static Boolean validate_new_type (
   XmpSpinboxWidget cw,
   int value )
{
   if ( value != XmSPINBOX_NUMBER &&
        value != XmSPINBOX_CLOCK_HM && 
        value != XmSPINBOX_CLOCK_HMS && 
        value != XmSPINBOX_CLOCK_DHMS && 
        value != XmSPINBOX_BEACONCODE &&
        value != XmSPINBOX_DOLLARS &&
        value != XmSPINBOX_STRINGS &&
        value != XmSPINBOX_USER_DEFINED ) {
      XmpWarning ( (Widget)cw, "\nRequested spinbox type not recognized!" );
      return False;
   }

   return True;
}
static Boolean validate_new_style (
    XmpSpinboxWidget cw,
    int value )
{
   if ( value != XmSPINBOX_STACKED &&
        value != XmSPINBOX_STACKED_LEFT &&
        value != XmSPINBOX_STACKED_RIGHT &&
        value != XmSPINBOX_LEFT &&
        value != XmSPINBOX_RIGHT &&
        value != XmSPINBOX_SEPARATE ) {
      XmpWarning ( (Widget)cw, "\nRequested spinbox style not recognized!" );
      return False;
   }

   return True;
}

static Boolean validate_new_minimum (
   XmpSpinboxWidget cw,
   long value )
{
   if (value > SpinboxValue(cw) ) {
      if (SpinboxAutoCorrect(cw)) {
         SpinboxValue(cw) = value;
      }
      else {
         XmpWarning ( (Widget)cw, "\nRequested minimum is greater than current Spinbox value!" );
         return False;
      }
   }

   return True;
}

static Boolean validate_new_maximum (
   XmpSpinboxWidget cw,
   long value )
{
   if ( value < SpinboxValue(cw) ) {
      if (SpinboxAutoCorrect(cw)) 
         SpinboxValue(cw) = value;
      else {
         XmpWarning ( (Widget)cw, "\nRequested maximum is less than current spinbox value!" );
         return False;
      }
   }

   return True;
}

static Boolean validate_new_delay (
   XmpSpinboxWidget cw,
   int value )
{
   if ( value<1 ) {
      XmpWarning ( (Widget)cw, "\nNew delay is smaller than minimum spinbox delay!" );
      return False;
   }

   return True;
}

static Boolean validate_new_value (
   XmpSpinboxWidget cw,
   long *value )
{
   if ( SpinboxCycle(cw) )
      return True;

   if ( *value<SpinboxMinValue(cw) )
   {
      if ( SpinboxUseClosestValue(cw) )
      {
         *value = SpinboxMinValue(cw);
         return ( True );
      }
      return False;
   }

   if ( *value>SpinboxMaxValue(cw) )
   {
      if ( SpinboxUseClosestValue(cw) )
      {
         *value = SpinboxMaxValue(cw);
         return ( True );
      }
      return False;
   }

   return True;
}


/**
***  SET SPINBOX && CALL VALUE CHANGED CALLBACKS
**/

static void set_spinbox_value (
   XmpSpinboxWidget cw,
   int value,
   Boolean notify )
{
   if ( value>SpinboxMaxValue(cw) && SpinboxCycle(cw) ) value = SpinboxMinValue(cw);
   if ( value<SpinboxMinValue(cw) && SpinboxCycle(cw) ) value = SpinboxMaxValue(cw);

   SpinboxValue(cw) = value;

   if ( !notify )
      return;

   if ( ( value != SpinboxOldValue(cw) ) &&
        ( XtHasCallbacks((Widget)cw, XmNvalueChangedCallback ) == XtCallbackHasSome )
)
   {
#if defined(XMTF_PEEK)
      XmpSpinboxCallbackStruct data;
      data.reason = XmCR_VALUE_CHANGED;
      data.value = value;
      data.str = XmTextFieldPeekString(SpinboxTF(cw));
      data.str_len = XmTextFieldPeekLength(SpinboxTF(cw));
      XtCallCallbacks ((Widget)cw, XmNvalueChangedCallback, (XtPointer)&data );
#else
      char *pchar = XmTextFieldGetString(SpinboxTF(cw));
      XmpSpinboxCallbackStruct data;
      data.reason = XmCR_VALUE_CHANGED;
      data.value = value;
      data.str = pchar;
      data.str_len = strlen(pchar);
      XtCallCallbacks ((Widget)cw, XmNvalueChangedCallback, (XtPointer)&data );
      XtFree ( pchar );
#endif
   }
}

void XmpSpinboxSetValue (
   Widget w,
   long value,
   Boolean notify )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;

   if ( SpinboxValue(cw) == value )
      return;
   if ( validate_new_value ( cw, &value ) )
   {
      set_spinbox_value ( cw, value, notify );
      set_text_field ( cw );
      SpinboxOldValue(cw) = SpinboxValue(cw);
   }
}

long XmpSpinboxGetValue ( Widget w )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   return ( SpinboxValue(cw) );
}

/***
****
****    ACTIONS
****
***/

static void Action_Increment_Small (
   Widget w,
   XEvent *event,
   String *strings,
   Cardinal *string_qty )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)XtParent(w);
   XmpSpinboxSetValue((Widget)cw, SpinboxValue(cw)+SpinboxIncrement(cw), True);
}
static void Action_Increment_Large (
   Widget w,
   XEvent *event,
   String *strings,
   Cardinal *string_qty )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)XtParent(w);
   XmpSpinboxSetValue ((Widget)cw, SpinboxValue(cw)+SpinboxIncLarge(cw), True);
}
static void Action_Decrement_Small (
   Widget w,
   XEvent *event,
   String *strings,
   Cardinal *string_qty )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)XtParent(w);
   XmpSpinboxSetValue ((Widget)cw, SpinboxValue(cw)-SpinboxIncrement(cw), True);
}
static void Action_Decrement_Large (
   Widget w,
   XEvent *event,
   String *strings,
   Cardinal *string_qty )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)XtParent(w);
   XmpSpinboxSetValue ((Widget)cw, SpinboxValue(cw)-SpinboxIncLarge(cw), True);
}

/***
****
****	SHOW_VALUE and GET_VALUE functions
****
***/

static void decimal_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   int i, left, right, magnitude;
   char *pchar = buffer;

   for ( i=0,magnitude=1; i<SpinboxDecimalPoints(cw); i++ ) magnitude *= 10;

   /* show the - if number is negative */
   if ( value<0 ) {
      *pchar++ = '-';
      value = -value;
   }

   /* print lhs of the decimal point */
   left = value / magnitude;
   pchar += int_sprintf ( pchar, "%d", left );

   right = value % magnitude;
   if ( !right ) return;
   if ( right<0 ) right = -right;

   pchar += int_sprintf ( pchar, ".%0*d", SpinboxDecimalPoints(cw), right );

   while ( *(--pchar)=='0' ) *pchar = '\0'; /* strip trailing 0s */
}

static Boolean decimal_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   long left=0, right=0;
   int i, magnitude;
   char *pchar, *pchar2;

   for ( i=0,magnitude=1; i<SpinboxDecimalPoints(cw); i++ )
      magnitude *= 10;

   errno = 0;
   left = strtol ( buffer, &pchar, 10 );
   if ( errno ) return ( False );
   left *= magnitude;

   if ( *pchar == '.' )
   {
      int len;

      errno = 0;
      right = strtol ( ++pchar, &pchar2, 10 );
      if ( errno ) return ( False );

      if ( pchar!=pchar2 )
      {
         if ( *pchar2 ) return False;
         len = SpinboxDecimalPoints(cw) - (pchar2-pchar);
         while ( len > 0 ) { right *= 10; len--; }
         while ( len < 0 ) { right /= 10; len++; }
      }
   }

   *value = left + right;
   return True;
}


static void dollar_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   register char *src, *tgt;
   int i, len, commas;
   char tmpbuf[32];
   Boolean neg;

   if (( neg = value < 0 ))
      value = -value;

   src = tmpbuf;
   tgt = buffer;
   len = int_sprintf ( src, "%01d", (int)value/100 );

   if ( neg )
      *tgt++ = '(';

   *tgt++ = '$';

   commas = (len-1)/3;
   i = len - commas*3;
   while ( i-- )
      *tgt++ = *src++;
   while ( commas-- ) {
      *tgt++ = ',';
      *tgt++ = *src++;
      *tgt++ = *src++;
      *tgt++ = *src++;
   }

   if (( i = value % 100 )) /* cents */
      tgt += int_sprintf ( tgt, ".%02d", i );

   if ( neg )
      *tgt++ = ')';

   *tgt = '\0';
}

static Boolean dollar_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   register char ch;
   Boolean dot = False;
   Boolean neg = False;
   long val = 0;

   while ( buflen-- ) {
      ch = *buffer++;
      if ( isdigit ( ch ) ) val = (val*10) + (ch-'0');
      else if ( ch==',' || ch=='$' ) continue;
      else if ( ch=='.' ) { buflen=2; dot=True; }
      else if ( ch=='(' || ch=='-' || ch==')' ) neg = True;
      else return False;
   }

   if ( !dot ) val *= 100;
   if ( neg ) val = -val;
   *value = val;
   return True;
}

static void long_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   (void) sprintf ( buffer, "%ld", value );
}

static Boolean long_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
#if !defined(LINUX) && !defined(SOLARIS) && !defined(DARWIN) && !defined(FREEBSD) && !defined(NETBSD) && \
   !defined(AIX51) && !defined(AIX43) && !defined(ALPHA) && !defined(HP1164) && !defined(HPUX) && !defined(IRIX) 
   int errno = 0; 
#endif
   *value = strtol ( buffer, NULL, 10 );
   return ( !errno );
}

static void clock_dhms_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   int sec, min, hr, day;
   
   sec = value % 60;
   value /= 60;
   min = value % 60;
   value /= 60;
   hr = value % 24;
   day = value/24;
   
   (void) sprintf ( buffer, "%01d:%02d:%02d:%02d", day, hr, min, sec );
}

static void clock_hms_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   int sec = value % 60;
   value /= 60;
   (void) sprintf ( buffer, "%01d:%02d:%02d", (int)value/60, (int)value%60, sec );
}

static void clock_hm_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   (void) sprintf ( buffer, "%01d:%02d", (int)value/60, (int)value%60 );
}

static Boolean clock_dhms_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   long day=0, hr=0, min=0, sec=0;
   char *pchar;

   /* get day */
   errno=0;
   day = strtol ( buffer, &pchar, 10);
   if ( errno ) return(False);

   /* get hour */
   errno = 0;
   hr = strtol ( buffer, &pchar, 10 );
   if ( errno ) return ( False );

   /* get minute */
   if ( *pchar ) {
      pchar++;
      min = strtol ( pchar, &pchar, 10 );
   }
   if ( errno ) return ( False );

   /* get seconds */
   if ( *pchar ) {
      pchar++;
      sec = strtol ( pchar, &pchar, 10 );
   }
   if ( errno ) return ( False );

   *value = day*24 + hr*3600 + min*60 + sec;
   return True;

}

static Boolean clock_hms_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   long hr=0, min=0, sec=0;
   char *pchar;

   /* get hour */
   errno = 0;
   hr = strtol ( buffer, &pchar, 10 );
   if ( errno ) return ( False );

   /* get minute */
   if ( *pchar ) {
      pchar++;
      min = strtol ( pchar, &pchar, 10 );
   }
   if ( errno ) return ( False );

   /* get seconds */
   if ( *pchar ) {
      pchar++;
      sec = strtol ( pchar, &pchar, 10 );
   }
   if ( errno ) return ( False );

   *value = hr*3600 + min*60 + sec;
   return True;
}

static Boolean clock_hm_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   long hr, min=0;
   char *pchar;

   /* get hour */
   errno = 0;
   hr = strtol ( buffer, &pchar, 10 );
   if ( errno ) return ( False );

   /* get minute */
   if ( *pchar ) {
      pchar++;
      min = strtol ( pchar, &pchar, 10 );
   }
   if ( errno ) return ( False );

   /* set value */
   *value = hr*60 + min;
   return True;
}

static void bc_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   (void) sprintf ( buffer, "%04o", (int)value );
}


static Boolean bc_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   char *pchar;
   long val;

   /* get number */
   errno = 0;
   val = strtol ( buffer, &pchar, 8 );
   if ( errno ) return ( False );

   /* we only want digits, nothing else. */
   if ( *pchar ) return False;

   *value = val;
   return True;
}

static void alpha_show_value (
   Widget w,
   XtPointer client,
   long value,
   char *buffer,
   size_t maxlen )
{
   XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   assert ( value >= 0 );
   assert ( value < SpinboxItemCount(cw) );
   (void) strcpy ( buffer, ((char**)SpinboxItems(cw))[value] );
}

static int alpha_cmp (
   const void *a,
   const void *b )
{
   const String sa = *(const String *)a;
   const String sb = *(const String *)b;
   assert ( sa && (isprint(*sa)||!*sa) );
   assert ( sb && (isprint(*sb)||!*sb) );
   return ( strcmp ( sa, sb ) );
}

static String* alpha_search (
   const char *findme,
   const String *base,
   int qty,
   Boolean are_sorted )
{
   int i;

   if ( are_sorted )
      return ( (String*) bsearch( &findme, (const void*) base, qty, sizeof(String), alpha_cmp ) );

   for ( i=0; i<qty; i++ )
      if ( !strcmp ( findme, base[i] ) )
         break;
   if ( i==qty )
      return (String*)0;

   return ((String*)(base + i));
}

static Boolean alpha_get_value (
   Widget w,
   XtPointer client,
   const char *buffer,
   size_t buflen,
   long *value )
{
   const XmpSpinboxWidget cw = (XmpSpinboxWidget)w;
   const String *pStr;
   const String *base = SpinboxItems(cw);

   assert ( buffer );
   assert ( isprint(*buffer) || !*buffer );

   pStr = alpha_search ( buffer, base, SpinboxItemCount(cw), SpinboxItemsAreSorted(cw)
 );
   if ( !pStr ) return False;

   *value = (long)(pStr - base);
   return True;
}

#ifdef SPRINTF_BROKEN
int int_sprintf ( char *buffer, const char *fmt, ... ) {
   va_list val;
   int len;
   va_start ( val, fmt );
   (void) vsprintf ( buffer, fmt, val );
   len = strlen ( buffer );
   va_end ( val );
   return ( len );
}
#endif


/**
***  String-to-long resource converter
**/

static Boolean CvtStringToLong (
   Display *dpy,
   XrmValuePtr args,
   Cardinal *num_args,
   XrmValuePtr from,
   XrmValuePtr to,
   XtPointer *data )
{
   assert ( !*num_args );
   assert ( from->addr );
   assert ( to->addr );
   assert ( to->size == sizeof(long) );

   errno = 0;
   *(long*)to->addr = strtol ( (char*)from->addr, NULL, 10 );
   return ( !errno );
}

