/*
 *
 * Spinbox.h - XmpSpinbox Public header
 *
 */

#ifndef _XmpSpinbox_h
#define _XmpSpinbox_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmpIsSpinbox
#define XmpIsSpinbox(w) XtIsSubclass(w, xmpSpinboxWidgetClass)
#endif

/**
***   RESOURCES
**/
#define XmNdouble "double"
#define XmRDouble "Double"

#define XmNlong      "long"
#define XmRLong      "Long"

#define XmNspinboxDelay "spinboxDelay"
#define XmCSpinboxDelay "SpinboxDelay"

#define XmNspinboxCycle "spinboxCycle"
#define XmCSpinboxCycle "SpinboxCycle"

#define XmNincrementLarge "incrementLarge"
#define XmCIncrementLarge "IncrementLarge"

#define XmNupdateText "updateText"
#define XmCUpdateText "UpdateText"

#ifndef XmNarrowOrientation
#define XmNarrowOrientation "arrowOrientation"
#endif
#ifndef XmCArrowOrientation
#define XmCArrowOrientation "ArrowOrientation"
#endif
#ifndef XmRArrowOrientation
#define XmRArrowOrientation "ArrowOrientation"
#endif

#define XmNspinboxType "spinboxType"
#define XmCSpinboxType "SpinboxType"
#define XmRSpinboxType "SpinboxType"
enum {
   XmSPINBOX_NUMBER,
   XmSPINBOX_CLOCK_HM,
   XmSPINBOX_CLOCK_HMS,
   XmSPINBOX_CLOCK_DHMS,
   XmSPINBOX_BEACONCODE,
   XmSPINBOX_DOLLARS,
   XmSPINBOX_STRINGS,
   XmSPINBOX_USER_DEFINED 
};

#define XmNspinboxStyle "spinboxStyle"
#define XmCSpinboxStyle "SpinboxStyle"
#define XmRSpinboxStyle "SpinboxStyle"
enum {
   XmSPINBOX_STACKED,
   XmSPINBOX_STACKED_LEFT,
   XmSPINBOX_STACKED_RIGHT, 
   XmSPINBOX_LEFT,
   XmSPINBOX_RIGHT,
   XmSPINBOX_SEPARATE 
};

#define SPINBOX_BUTTON_SIZE_RATIO_DECIMAL_PLACES 2
#define XmNbuttonSizeRatio "buttonSizeRatio"
#define XmCButtonSizeRatio "ButtonSizeRatio"

#define XmNbuttonSizeFixed "buttonSizeFixed"
#define XmCButtonSizeFixed "ButtonSizeFixed"

#define XmNshowValueProc   "showValueProc"
#define XmCShowValueProc   "ShowValueProc"
#define XmRShowValueProc   "ShowValueProc"

#define XmNshowValueData   "showValueData"
#define XmCShowValueData   "ShowValueData"

#define XmNgetValueData	   "getValueData"
#define XmCGetValueData	   "GetValueData"

#define XmNgetValueProc	   "getValueProc"
#define XmCGetValueProc	   "GetValueProc"
#define XmRGetValueProc	   "GetValueProc"

#define XmNitemsAreSorted  "itemsAreSorted"
#define XmCItemsAreSorted  "ItemsAreSorted"

#define XmNspinboxUseClosestValue	"spinboxUseClosestValue"
#define XmCSpinboxUseClosestValue	"SpinboxUseClosestValue"

#define XmNspinboxAutoCorrect	"spinboxAutoCorrect"
#define XmCSpinboxAutoCorrect	"SpinboxAutoCorrect"

externalref WidgetClass xmpSpinboxWidgetClass;

typedef struct _XmpSpinboxClassRec *XmpSpinboxWidgetClass;
typedef struct _XmpSpinboxRec *XmpSpinboxWidget;

typedef struct {
   const char *str;
   size_t str_len;
   int reason;
   long value;
} XmpSpinboxCallbackStruct;


typedef void (SpinboxShowValueProc) (
   Widget spinbox,
   XtPointer client_data,
   long num,
   char *buffer,
   size_t buflen_max );

typedef Boolean (SpinboxGetValueProc) (
   Widget spinbox,
   XtPointer client_data,
   const char *buffer,
   size_t buflen,
   long *num );



/**
***  Convenience functions
**/

extern Widget XmpCreateSpinbox (
   Widget parent,
   char *name,
   Arg *args,
   Cardinal arg_qty );

extern void XmpSpinboxSetValue (
   Widget spinbox,
   long value,
   Boolean notify);

extern long XmpSpinboxGetValue (
   Widget spinbox );



#ifdef __cplusplus
}
#endif

#endif /* _XmpSpinbox_h */
