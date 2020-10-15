/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
/*
 * A lot has been borrowed from the corresponding Ask*.c files
 * from David Flanagans Xmt lib (see 3rdparty/qmon)
 * Adapted to get Sge time input values
 *
 */

#include <stdlib.h>
#include <ctype.h>

#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>

#include <Xm/PushB.h>
#include <Xm/ToggleB.h>

#include "qmon_rmon.h"
#include "Spinbox.h"
#include "AskForTime.h"
#include "uti/sge_parse_num_par.h"


typedef struct _tTimeStruct {
   int days;
   int hours;
   int minutes;
   int seconds;
   int infinity;
} tTimeStruct;

#define TIME_TYPE    0
#define TIME_TYPE_NOINFINITY  1
#define MEMORY_TYPE  2

/*-------------------------------------------------------------------------*/
static String qmonGetMemoryString(Widget memoryw);
static String qmonGetTimeString(Widget memoryw, Boolean show_infinity);
static void qmonGetTime(Widget timew, tTimeStruct *time, Boolean show_infinity);
static void qmonMemSensitivity(Widget w, XtPointer cld, XtPointer cad);
static void qmonSetMemoryString(Widget memoryw, String mem);
static void qmonSetTimeString(Widget timew, String time, Boolean show_infinity);
static void qmonTimeSensitivity(Widget w, XtPointer cld, XtPointer cad);

/*-------------------------------------------------------------------------*/
static void CreateMemoryDialog(
XmtPerScreenInfo *info 
) {
   Widget memory, mem_value, mem_unit, okay, cancel, help,
            layout_box1, layout_box2, mem_box, memory_pix, memoryString;
   XmString ok_str, cancel_str, help_str;
   Arg args[10];
   int i;

   static String mem_units[] = { 
      "@{INFINITY}", 
      "@{Byte}", 
      "@{kByte (1000)}", 
      "@{KByte (1024)}", 
      "@{mByte (1000x1000)}",
      "@{MByte (1024x1024)}",
      "@{gByte (1000x1000x1000)}",
      "@{GByte (1024x1024x1024)}"
   };
   static String mem_values[] = { "INFINITY", "", "k", "K", "m", "M", "g", "G"  };
   
   DENTER(GUI_LAYER, "CreateMemoryDialog");
   /*
    * create the memory dialog.
    */
   i = 0;
   XtSetArg(args[i], XmNallowShellResize, True); i++;
   XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
   XtSetArg(args[i], XmNautoUnmanage, False); i++;
   XtSetArg(args[i], XmNdefaultPosition, False); i++;
   memory = XmtCreateLayoutDialog(info->topmost_shell,
                             XmtMEMORY_DIALOG_NAME, args, i);
                           
   mem_box = XtVaCreateManagedWidget("mem_box",
                           xmtLayoutBoxGadgetClass,
                           memory,
                           XmtNorientation, XmHORIZONTAL,
                           NULL);

   memory_pix = XtVaCreateManagedWidget("Memorypix", 
                           xmtLayoutPixmapGadgetClass, 
                           memory,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, mem_box,
                           NULL);
   memoryString = XtVaCreateManagedWidget("MemMessage", 
                           xmtLayoutStringGadgetClass,
                           memory,
                           XmtNlayoutIn, mem_box,
                           NULL);

   layout_box1 = XtVaCreateManagedWidget("layout_box1",
                           xmtLayoutBoxGadgetClass,
                           memory,
                           XmtNorientation, XmVERTICAL,
                           NULL);

   mem_unit = XtVaCreateManagedWidget("MemoryUnit", 
                                 xmtChooserWidgetClass, 
                                 memory,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmtNchooserType, XmtChooserOption,
                                 XmtNstrings, mem_units,
                                 XmtNvalueStrings, mem_values,
                                 XmtNvalueType, XtRString, 
                                 XmtNvalueSize, sizeof(String),
                                 XmtNnumItems, XtNumber(mem_units),
                                 XmtNlayoutStretchability, 0,
                                 XmtNlayoutShrinkability, 0,
                                 NULL);
   XtAddCallback(mem_unit, XmtNvalueChangedCallback,
                  qmonMemSensitivity, NULL);

   mem_value = XtVaCreateManagedWidget("MemoryValue", 
                                 xmtInputFieldWidgetClass, 
                                 memory,
                                 XmtNlayoutStretchability, 0,
                                 XmtNlayoutShrinkability, 0,
                                 XmtNlayoutAfter, mem_unit,
/*                                  XmtNlayoutJustification, XmtLayoutCentered, */
                                 XmtNlayoutIn, layout_box1,
                                 NULL);
                              

   layout_box2 = XtVaCreateManagedWidget("layout_box2",
                           xmtLayoutBoxGadgetClass,
                           memory,
                           XmtNorientation, XmHORIZONTAL,
                           XmtNequal, True,
                           NULL);

   ok_str = XmtCreateLocalizedXmString(memory, "@{Ok}");
   cancel_str = XmtCreateLocalizedXmString(memory, "@{Cancel}");
   help_str = XmtCreateLocalizedXmString(memory, "@{Help}");
                              
   okay = XtVaCreateManagedWidget("okButton", 
                                 xmPushButtonWidgetClass, 
                                 memory,
                                 XmNlabelString, ok_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmNshowAsDefault, True,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 NULL);

   cancel = XtVaCreateManagedWidget("cancelButton", 
                                 xmPushButtonWidgetClass, 
                                 memory,
                                 XmNlabelString, cancel_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 NULL);

   help = XtVaCreateManagedWidget("helpButton", 
                                 xmPushButtonWidgetClass, 
                                 memory,
                                 XmNlabelString, help_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 NULL);
    XmStringFree(ok_str);
    XmStringFree(cancel_str);
    XmStringFree(help_str);

    /* add callbacks on all the dialog buttons */
    XtAddCallback(okay, XmNactivateCallback, 
                     _XmtOkCallback, (XtPointer)info);
    XtAddCallback(cancel, XmNactivateCallback, 
                     _XmtCancelCallback, (XtPointer)info);
    XtAddCallback(help, XmNactivateCallback, 
                     _XmtHelpCallback, (XtPointer)info);

    /* add a callback for f.close */
    XmtAddDeleteCallback(XtParent(memory), XmDO_NOTHING,
                      _XmtCancelCallback, (XtPointer)info);

    /* cache the dialog in the PerScreenInfo structure */
    /* _AA: see ScreenP.h, don't know if it has to be adapted */
    info->memory_dialog = memory;

    DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonMemSensitivity(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*)cad;
   Widget memory_input;

   DENTER(GUI_LAYER, "qmonMemSensitivity");

   memory_input = XmtNameToWidget(w, "*MemoryValue");

   XmtInputFieldSetString(memory_input, "");
   
   if (cbs->state > 0)
      XtSetSensitive(memory_input, True);
   else
      XtSetSensitive(memory_input, False);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonTimeSensitivity(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   XmtPerScreenInfo *info = (XmtPerScreenInfo*) cld;
   XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*)cad;
   String spin_names[] = {"*Day", "*Hour", "*Minute", "*Second"};
   Widget spins[4];
   Widget time;
   int i;

   DENTER(GUI_LAYER, "qmonTimeSensitivity");

   time = info->time_dialog;

   for (i=0; i<XtNumber(spin_names); i++)
      spins[i] = XmtNameToWidget(time, spin_names[i]);

   if (cbs->set == 0) {
      for (i=0; i<XtNumber(spins); i++)
         XtSetSensitive(spins[i], True);
   } else {
      for (i=0; i<XtNumber(spins); i++)
         XtSetSensitive(spins[i], False);
   }

   DEXIT;
}


static void CreateTimeDialog(
XmtPerScreenInfo *info 
) {
   Widget time, day, hour, minute, second, okay, cancel, help,
            layout_box1, layout_box2, day_box, hour_box, minute_box,
            second_box, hour_pix, day_pix, minute_pix, second_pix, 
            infinity, infinity_box, infinity_pix, timeString;
   XmString ok_str, cancel_str, help_str, inf_str, hour_label,
            day_label, minute_label, second_label;
   Arg args[10];
   int i;

   DENTER(GUI_LAYER, "CreateTimeDialog");

   /*
    * create the time dialog.
    */
   i = 0;
   XtSetArg(args[i], XmNallowShellResize, True); i++;
   XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
   XtSetArg(args[i], XmNautoUnmanage, False); i++;
   XtSetArg(args[i], XmNdefaultPosition, False); i++;
   time = XmtCreateLayoutDialog(info->topmost_shell,
                             XmtTIME_DIALOG_NAME, args, i);
                           
   timeString = XtVaCreateManagedWidget("TimeMessage", 
                           xmtLayoutStringGadgetClass,
                           time,
                           NULL);

   infinity_box = XtVaCreateManagedWidget("infinity_box",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmHORIZONTAL,
                           NULL);
   infinity_pix = XtVaCreateManagedWidget("Infinitypix", 
                           xmtLayoutPixmapGadgetClass, 
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, infinity_box,
                           NULL);
   inf_str = XmtCreateLocalizedXmString(time, "@{Infinity}");
   infinity = XtVaCreateManagedWidget("TimeInfinity", 
                           xmToggleButtonWidgetClass,
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, infinity_box,
                           XmNlabelString, inf_str,
                           NULL);
   XmStringFree(inf_str);
   XtAddCallback(infinity, XmNvalueChangedCallback,
                  qmonTimeSensitivity, (XtPointer)info);
                     

   layout_box1 = XtVaCreateManagedWidget("layout_box1",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmHORIZONTAL,
                           NULL);
   day_box = XtVaCreateManagedWidget("day_box",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           NULL);
   hour_box = XtVaCreateManagedWidget("hour_box",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutAfter, day_box,
                           NULL);
   minute_box = XtVaCreateManagedWidget("minute_box",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutAfter, hour_box,
                           NULL);
   second_box = XtVaCreateManagedWidget("second_box",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutAfter, minute_box,
                           NULL);

   day_label = XmtCreateLocalizedXmString(time, "@{Day}");
   day_pix = XtVaCreateManagedWidget("Daypix", 
                           xmtLayoutPixmapGadgetClass, 
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, day_box,
                           XmtNlayoutCaption, day_label,
                           XmtNlayoutCaptionPosition, XmtLayoutBottom,
                           XmtNlayoutCaptionAlignment, XmALIGNMENT_BEGINNING,
                           XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
                           NULL);
   XmStringFree(day_label);

   day = XtVaCreateManagedWidget("Day", 
                           xmpSpinboxWidgetClass, 
                           time,
                           XmNspinboxType, (XtPointer) XmSPINBOX_NUMBER,
                           XmNminimum, (XtPointer)0,
                           XmNmaximum, (XtPointer)365,
                           XmNcolumns, 4,
                           XmNincrementLarge, 5,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmNbuttonSizeFixed, True,
                           XmtNlayoutIn, day_box,
                           NULL);

   hour_label = XmtCreateLocalizedXmString(time, "@{Hour}");
   hour_pix = XtVaCreateManagedWidget("Hourpix", 
                           xmtLayoutPixmapGadgetClass, 
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, hour_box,
                           XmtNlayoutCaption, hour_label,
                           XmtNlayoutCaptionPosition, XmtLayoutBottom,
                           XmtNlayoutCaptionAlignment, XmALIGNMENT_BEGINNING,
                           XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
                           NULL);
   XmStringFree(hour_label);

   hour = XtVaCreateManagedWidget("Hour", 
                           xmpSpinboxWidgetClass, 
                           time,
                           XmNspinboxType, (XtPointer)XmSPINBOX_NUMBER,
                           XmNminimum, (XtPointer)0,
                           XmNmaximum, (XtPointer)23,
                           XmNcolumns, 2,
                           XmNincrementLarge, 5,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, hour_box,
                           XmNbuttonSizeFixed, True,
                           NULL);

   minute_label = XmtCreateLocalizedXmString(time, "@{Minute}");
   minute_pix = XtVaCreateManagedWidget("Minutepix", 
                           xmtLayoutPixmapGadgetClass, 
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, minute_box,
                           XmtNlayoutCaption, minute_label,
                           XmtNlayoutCaptionPosition, XmtLayoutBottom,
                           XmtNlayoutCaptionAlignment, XmALIGNMENT_BEGINNING,
                           XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
                           NULL);
   XmStringFree(minute_label);

   minute = XtVaCreateManagedWidget("Minute", 
                           xmpSpinboxWidgetClass, 
                           time,
                           XmNspinboxType, (XtPointer)XmSPINBOX_NUMBER,
                           XmNminimum, (XtPointer)0,
                           XmNmaximum, (XtPointer)59,
                           XmNcolumns, 2,
                           XmNincrementLarge, 10,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, minute_box,
                           XmNbuttonSizeFixed, True,
                           NULL);

   second_label = XmtCreateLocalizedXmString(time, "@{Second}");
   second_pix = XtVaCreateManagedWidget("Secondpix", 
                           xmtLayoutPixmapGadgetClass, 
                           time,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, second_box,
                           XmtNlayoutCaption, second_label,
                           XmtNlayoutCaptionPosition, XmtLayoutBottom,
                           XmtNlayoutCaptionAlignment, XmALIGNMENT_BEGINNING,
                           XmtNlayoutCaptionJustification, XmtLayoutFlushLeft,
                           NULL);
   XmStringFree(second_label);

   second = XtVaCreateManagedWidget("Second", 
                           xmpSpinboxWidgetClass, 
                           time,
                           XmNspinboxType, (XtPointer)XmSPINBOX_NUMBER,
                           XmNminimum, (XtPointer)0,
                           XmNmaximum, (XtPointer)59,
                           XmNcolumns, 2,
                           XmNincrementLarge, 10,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           XmtNlayoutIn, second_box,
                           XmNbuttonSizeFixed, True,
                           NULL);
   layout_box2 = XtVaCreateManagedWidget("layout_box2",
                           xmtLayoutBoxGadgetClass,
                           time,
                           XmtNorientation, XmHORIZONTAL,
                           XmtNequal, True,
                           NULL);

   ok_str = XmtCreateLocalizedXmString(time, "@{Ok}");
   cancel_str = XmtCreateLocalizedXmString(time, "@{Cancel}");
   help_str = XmtCreateLocalizedXmString(time, "@{Help}");
                              
   okay = XtVaCreateManagedWidget("okButton", 
                                 xmPushButtonWidgetClass, 
                                 time,
                                 XmNlabelString, ok_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmNshowAsDefault, True,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 NULL);

   cancel = XtVaCreateManagedWidget("cancelButton", 
                                 xmPushButtonWidgetClass, 
                                 time,
                                 XmNlabelString, cancel_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 NULL);

   help = XtVaCreateManagedWidget("helpButton", 
                                 xmPushButtonWidgetClass, 
                                 time,
                                 XmNlabelString, help_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 NULL);
    XmStringFree(ok_str);
    XmStringFree(cancel_str);
    XmStringFree(help_str);

    /* add callbacks on all the dialog buttons */
    XtAddCallback(okay, XmNactivateCallback, 
                     _XmtOkCallback, (XtPointer)info);
    XtAddCallback(cancel, XmNactivateCallback, 
                     _XmtCancelCallback, (XtPointer)info);
    XtAddCallback(help, XmNactivateCallback, 
                     _XmtHelpCallback, (XtPointer)info);

    /* add a callback for f.close */
    XmtAddDeleteCallback(XtParent(time), XmDO_NOTHING,
                      _XmtCancelCallback, (XtPointer)info);

    /* cache the dialog in the PerScreenInfo structure */
    /* _AA: see ScreenP.h, don't know if it has to be adapted */
    info->time_dialog = time;

    DEXIT;
}

typedef struct {
    String message;
    String title;
    String help_text;
} DialogData;

static XtResource resources[] = {
{XmtNtitle, XmtCTitle, XtRString,
     sizeof(String), XtOffsetOf(DialogData, title),
     XtRString, NULL},
{XmtNmessage, XmtCMessage, XtRString,
     sizeof(String), XtOffsetOf(DialogData, message),
     XtRString, NULL},
{XmtNhelpText, XmtCHelpText, XtRString,
     sizeof(String), XtOffsetOf(DialogData, help_text),
     XtRString, NULL}
};


static Boolean GetValue(
Widget w,
StringConst dialog_name,
StringConst prompt_default,
String in_out,
int in_out_len,
StringConst help_text_default,
int type 
) {
    Widget shell = XmtGetShell(w);
    XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
    DialogData data;
    Widget dialog = 0;
    Widget help_w; 
    static String default_time_title;
    static String default_memory_title;
    XmString message, title;
    String return_string = NULL; 
    String dialog_class = NULL;
    Widget messageW;
    Widget memory_unit;
    String default_title = NULL;
    String message_name = NULL;

   DENTER(GUI_LAYER, "GetValue");

    /* make sure the shell we pop up over is not a menu shell! */
    while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize the default title the first time through */
    if (!default_time_title) {
       default_time_title = XmtLocalize2(w, XmtTIME_DIALOG_TITLE_DEFAULT,
                                   "XmtAskForTime", "title");
       default_memory_title = XmtLocalize2(w, XmtMEMORY_DIALOG_TITLE_DEFAULT,
                                   "XmtAskForMemory", "title");
    }

    switch (type) {
      case TIME_TYPE_NOINFINITY:
      case TIME_TYPE:
         /* if there's no cached dialog, create one. */
         if (info->time_dialog == NULL) 
            CreateTimeDialog(info);
         dialog = info->time_dialog;
         dialog_class = XmtCTimeDialog;
         default_title = default_time_title;
         message_name = "*TimeMessage";
         if (type == TIME_TYPE_NOINFINITY) {
            XtSetSensitive(XmtNameToWidget(dialog, "*Day"), True);
            XtSetSensitive(XmtNameToWidget(dialog, "*Hour"), True);
            XtSetSensitive(XmtNameToWidget(dialog, "*Minute"), True);
            XtSetSensitive(XmtNameToWidget(dialog, "*Second"), True);
            XtUnmanageChild(XmtNameToWidget(dialog, "*TimeInfinity"));
            XtUnmanageChild(XmtNameToWidget(dialog, "*Infinitypix"));
            qmonSetTimeString(dialog, in_out, False); 
         } else {
            XtManageChild(XmtNameToWidget(dialog, "*TimeInfinity"));
            XtManageChild(XmtNameToWidget(dialog, "*Infinitypix"));
            qmonSetTimeString(dialog, in_out, True); 
         }
         break;
      case MEMORY_TYPE:
         /* if there's no cached dialog, create one. */
         if (info->memory_dialog == NULL) 
            CreateMemoryDialog(info);
         dialog = info->memory_dialog;
         dialog_class = XmtCMemoryDialog;
         default_title = default_memory_title;
         message_name = "*MemMessage";
         memory_unit = XmtNameToWidget(dialog, "*MemoryUnit");
         qmonSetMemoryString(dialog, in_out);
         break;
    }

    help_w = XmtNameToWidget(dialog, "helpButton");
    messageW = XmtNameToWidget(dialog, message_name);

    /* if this dialog has a name, look up its resources */
    if (dialog_name != NULL) {
       resources[0].default_addr = (XtPointer) prompt_default;
       resources[1].default_addr = (XtPointer) default_title;
       resources[2].default_addr = (XtPointer) help_text_default;
       XtGetSubresources(shell, (XtPointer)&data,
                       (String)dialog_name, dialog_class, 
                       resources, XtNumber(resources),
                       NULL, 0);
    }
    else { /* otherwise use arguments as defaults */
       data.message = (String) prompt_default;
       data.title = default_title;
       data.help_text = (String) help_text_default;
    }

DPRINTF(("%p %s %p message: '%s'\n", dialog, XtName(messageW), messageW, data.message));

    /* create the XmStrings */
    message = XmtCreateLocalizedXmString(shell, data.message);
    title = XmtCreateLocalizedXmString(shell, data.title);

    /* set resources on the dialog */
    XtVaSetValues(dialog,
                XmNdialogTitle, title,
                NULL);
    XtVaSetValues(messageW,
                  XmtNlabelString, message,
                  NULL);

    

    /*
     * if there is help text, make the button sensitive,
     * and put help text where the callback procedure can find it.
     */
    if ((data.help_text != NULL) && (data.help_text[0] != '\0')) {
       XtSetSensitive(help_w, True);
       info->help_text = data.help_text;
    }
    else {
       XtSetSensitive(help_w, False);
       info->help_text = NULL;
    }

    /* destroy the XmStrings  */
    XmStringFree(message);
    XmStringFree(title);
    
    /* Tell the dialog who it is transient for */
    XtVaSetValues(XtParent(dialog), XtNtransientFor, shell, NULL);

    /* position, set initial focus, and pop up the dialog */
    XmtDialogPosition(dialog, shell);
    XtManageChild(dialog);

    /*
     * Enter a recursive event loop.
     * The callback registered on the okay and cancel buttons when
     * this dialog was created will cause info->button to change
     * when one of those buttons is pressed.
     */
    info->blocked = True;
   XmtBlock(shell, &info->blocked);

    /* pop down the dialog */
    XtUnmanageChild(dialog);

    /* make sure what is underneath gets cleaned up
     * (the calling routine might act on the user's returned
     * input and not handle events for awhile.)
     */
    XSync(XtDisplay(dialog), 0);
    XmUpdateDisplay(dialog);

    /*
     * if the user clicked Ok, return the time string
     */
    if (info->button == XmtOkButton) {
       switch (type) {
         case TIME_TYPE_NOINFINITY:
            return_string = qmonGetTimeString(dialog, False);
            break;
         case TIME_TYPE:
            return_string = qmonGetTimeString(dialog, True);
            break;
         case MEMORY_TYPE:
            return_string = qmonGetMemoryString(dialog);
            break;
       }
       strncpy(in_out, return_string, in_out_len-1);  
       in_out[in_out_len-1] = '\0';
    }

    /* if user clicked Cancel, return False, else True */
    if (info->button == XmtCancelButton) {
      DEXIT;
      return False;
    }
    else {
      DEXIT;
      return True;
    }
}


/*-------------------------------------------------------------------------*/
static void qmonGetTime(Widget timew, tTimeStruct *time, Boolean show_infinity)
{
   Widget day, hour, minute, second, infinity=0;
   
   DENTER(GUI_LAYER, "qmonGetTime");

   day = XmtNameToWidget(timew, "^*Day");
   hour = XmtNameToWidget(timew, "^*Hour");
   minute = XmtNameToWidget(timew, "^*Minute");
   second = XmtNameToWidget(timew, "^*Second");
   if (show_infinity)
      infinity = XmtNameToWidget(timew, "^*TimeInfinity");

   time->days = XmpSpinboxGetValue(day);  
   time->hours = XmpSpinboxGetValue(hour);  
   time->minutes = XmpSpinboxGetValue(minute);  
   time->seconds = XmpSpinboxGetValue(second);  
   if (show_infinity)
      time->infinity = XmToggleButtonGetState(infinity); 
   else
      time->infinity = 0;

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static String qmonGetTimeString(Widget timew, Boolean show_infinity)
{
   tTimeStruct time;
   static char buf[BUFSIZ];
   
   DENTER(GUI_LAYER,"qmonGetTimeString");
   qmonGetTime(timew, &time, show_infinity);

   if (!time.infinity)
      sprintf(buf, "%02d:%02d:%02d", time.days*24+time.hours, time.minutes,
               time.seconds);
   else
      sprintf(buf, "INFINITY");

   DEXIT;
   return buf;
}


/*-------------------------------------------------------------------------*/
static void qmonSetTimeString(Widget timew, String timestr, Boolean show_infinity)
{
   Widget day, hour, minute, second, infinity = 0;
   u_long32 time_val = 0;
   int day_val = 0;
   int hour_val = 0;
   int minute_val = 0;
   int second_val = 0;
   
   DENTER(GUI_LAYER, "qmonSetTimeString");

   day = XmtNameToWidget(timew, "*Day");
   hour = XmtNameToWidget(timew, "*Hour");
   minute = XmtNameToWidget(timew, "*Minute");
   second = XmtNameToWidget(timew, "*Second");
   if (show_infinity)
      infinity = XmtNameToWidget(timew, "*TimeInfinity");


   if ( show_infinity && (!strcasecmp(timestr, "infinity") || (timestr && timestr[0]=='\0'))) {
      XmToggleButtonSetState(infinity, 1, True);
      XtSetSensitive(day, False);
      XtSetSensitive(hour, False);
      XtSetSensitive(minute, False);
      XtSetSensitive(second, False);
   }
   else if (parse_ulong_val(NULL, &time_val, TYPE_TIM, timestr, NULL, 0)) {
      day_val = time_val/(24*3600);
      time_val %= 24*3600;
      hour_val = time_val/3600;
      time_val %= 3600;
      minute_val = time_val/60;
      second_val = time_val % 60;
      if (show_infinity)
         XmToggleButtonSetState(infinity, 0, True);
   }

   XmpSpinboxSetValue(day, day_val, True);  
   XmpSpinboxSetValue(hour, hour_val, True);  
   XmpSpinboxSetValue(minute, minute_val, True);  
   XmpSpinboxSetValue(second, second_val, True);  
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static String qmonGetMemoryString(
Widget memoryw 
) {
   static char buf[BUFSIZ];
   String inputstring;
   String unit;
   Widget memory_unit, memory_value;

   DENTER(GUI_LAYER, "qmonGetMemoryString");
   
   memory_unit = XmtNameToWidget(memoryw, "*MemoryUnit"); 
   memory_value = XmtNameToWidget(memoryw, "*MemoryValue");
   
   inputstring = XmtInputFieldGetString(memory_value);
   unit = *(String *) XmtChooserGetValue(memory_unit);

   if (!strcasecmp(unit, "infinity"))
      sprintf(buf, "%s", unit);
   else   
      sprintf(buf, "%s%s", inputstring, unit);

   DEXIT;
   return buf;
}

/*-------------------------------------------------------------------------*/
static void qmonSetMemoryString(
Widget memoryw,
String mem 
) {
   char memstring[128];
   int unit = 0;
   Widget memory_unit, memory_value;
   double mem_val = 0;
   char* dptr = NULL;

   DENTER(GUI_LAYER, "qmonGetMemoryString");
   
   memory_unit = XmtNameToWidget(memoryw, "*MemoryUnit"); 
   memory_value = XmtNameToWidget(memoryw, "*MemoryValue");

   if (!strcasecmp(mem, "infinity") || (mem && mem[0] == '\0')) {
      unit = 0;
   }
   else { 
      mem_val = strtod(mem, &dptr);
      
      while (isspace(*dptr))
         dptr++;

      switch (*dptr) {
         case 'k':
            unit = 2;
            break;
         case 'K':
            unit = 3;
            break;
         case 'm':
            unit = 4;
            break;
         case 'M':
            unit = 5;
            break;
         case '\0':
            unit = 1;
            break;
         default:
            unit = 0;
            break;
      }
   }

   if (!unit || mem_val == 0) {
      unit = 0;
      strcpy(memstring, "");
   }
   else
      sprintf(memstring, "%.2f", mem_val);

   XmtChooserSetState(memory_unit, unit, True);
   XmtInputFieldSetString(memory_value, memstring);

   DEXIT;
}



Boolean XmtAskForTime(Widget w, StringConst dialog_name, 
                      StringConst prompt_default, String in_out, 
                      int in_out_len, StringConst help_text_default, 
                      Boolean show_infinity)
{
   int type = show_infinity ? TIME_TYPE : TIME_TYPE_NOINFINITY;

  
   return GetValue(  w, dialog_name, prompt_default, in_out, in_out_len, 
                  help_text_default, type);
}


Boolean XmtAskForMemory(
Widget w,
StringConst dialog_name,
StringConst prompt_default,
String in_out,
int in_out_len,
StringConst help_text_default 
) {
    return GetValue(  w, dialog_name, prompt_default, in_out, in_out_len, 
                  help_text_default, MEMORY_TYPE);
}
