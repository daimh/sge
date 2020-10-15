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

#include <Xmt/Xmt.h>
#include <Xmt/ScreenP.h>
#include <Xmt/DialogsP.h>
#include <Xmt/Converters.h>
#include <Xmt/Layout.h>
#include <Xmt/LayoutG.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>

#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/ToggleBG.h>

#include "qmon_rmon.h"
#include "AskForItems.h"
#include "qmon_util.h"
#include "qmon_init.h"

static Widget ask_items = 0;

/*-------------------------------------------------------------------------*/
static void AddItems(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   XmString *selectedItems = NULL;
   Cardinal selectedItemCount = 0;
   Widget dialog = (Widget) cld;
   Widget avail_list, chosen_list;
   int i;

   DENTER(GUI_LAYER, "AddItems");

   XmtLayoutDisableLayout(ask_items);

   avail_list = XmtNameToWidget(dialog, "*ItemsAvailable");
   chosen_list = XmtNameToWidget(dialog, "*ItemsChosen");

   DPRINTF(("avail_list = %s\n", XtName(avail_list)));
   DPRINTF(("chosen_list = %s\n", XtName(chosen_list)));

   XtVaGetValues( avail_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   for (i=0; i<selectedItemCount; i++) {
      String text = NULL;
      if (! XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text)) {
         DEXIT;
         return;
      }

      XmListAddItemUniqueSorted(chosen_list, text);
      XtFree(text);
   }

   XmtLayoutEnableLayout(ask_items);

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void RmItems(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   XmString *selectedItems = NULL;
   Cardinal selectedItemCount = 0;
   Widget dialog = (Widget) cld;
   Widget chosen_list;

   DENTER(GUI_LAYER, "AddItems");

   XmtLayoutDisableLayout(ask_items);

   chosen_list = XmtNameToWidget(dialog, "*ItemsChosen");

   DPRINTF(("chosen_list = %s\n", XtName(chosen_list)));

   XtVaGetValues( chosen_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   XmListDeleteItems(chosen_list, selectedItems, selectedItemCount);

   XmtLayoutEnableLayout(ask_items);

   DEXIT;
}
   
                  
   
   
   

/*-------------------------------------------------------------------------*/
static void CreateItemsDialog(
XmtPerScreenInfo *info 
) {
   Widget   avail_list, chosen_list, add, remove, space, space2, 
            in_title, out_title, layout_box1, layout_box2, 
            layout_box3, layout_box4, layout_box5, okay, cancel, help;
   XmString ok_str, cancel_str, help_str;
   Arg args[10];
   int i;
   Pixmap addPix, removePix;
   
   
   DENTER(GUI_LAYER, "CreateItemsDialog");

   addPix = qmonGetIcon("rightarrow");
   removePix = qmonGetIcon("leftarrow");

   /*
    * create the items dialog.
    */
   i = 0;
   XtSetArg(args[i], XmNallowShellResize, True); i++;
   XtSetArg(args[i], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); i++;
   XtSetArg(args[i], XmNautoUnmanage, False); i++;
   XtSetArg(args[i], XmNdefaultPosition, False); i++;
   ask_items = XmtCreateLayoutDialog(info->topmost_shell,
                             XmtITEMS_DIALOG_NAME, args, i);
                           
   layout_box1 = XtVaCreateManagedWidget("layout_box1",
                           xmtLayoutBoxGadgetClass,
                           ask_items,
                           XmtNorientation, XmHORIZONTAL,
                           NULL);

   layout_box4 = XtVaCreateManagedWidget("layout_box4",
                           xmtLayoutBoxGadgetClass,
                           ask_items,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           NULL);

   layout_box3 = XtVaCreateManagedWidget("layout_box3",
                           xmtLayoutBoxGadgetClass,
                           ask_items,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           NULL);

   layout_box5 = XtVaCreateManagedWidget("layout_box5",
                           xmtLayoutBoxGadgetClass,
                           ask_items,
                           XmtNorientation, XmVERTICAL,
                           XmtNlayoutIn, layout_box1,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           NULL);

   in_title = XtVaCreateManagedWidget("InTitle",
                           xmtLayoutStringGadgetClass,
                           ask_items,
                           XmtNlayoutIn, layout_box4,
                           NULL);

   i = 0;
   XtSetArg(args[i], XmtNlayoutIn, layout_box4); i++;
   XtSetArg(args[i], XmtNlayoutStretchability, 0); i++;
   XtSetArg(args[i], XmtNlayoutShrinkability, 0); i++;
   XtSetArg(args[i], XmtNlayoutWidth, 300); i++;
   XtSetArg(args[i], XmtNitemStretch, 0); i++;
   XtSetArg(args[i], XmtNlayoutAllowResize, False); i++;
   XtSetArg(args[i], XmNselectionPolicy, XmEXTENDED_SELECT); i++;
/*    XtSetArg(args[i], XmNvisibleItemCount, 15); i++; */
   XtSetArg(args[i], XmtNlayoutHeight, 300); i++;
   avail_list = XmCreateScrolledList(ask_items, "ItemsAvailable", 
                           args, i);
#if 0
   i=0;
   XtSetArg(args[i], XmNscrollBarDisplayPolicy, XmAS_NEEDED); i++;
   XtSetValues(XtParent(avail_list), args, i);
#endif
   XtManageChild(avail_list);
   
   space2 = XtVaCreateManagedWidget("space2", 
                                 xmtLayoutSpaceGadgetClass,
                                 ask_items,
                                 XmtNlayoutIn, layout_box3,
                                 XmtNlayoutWidth, 0,
                                 XmtNlayoutHeight, 20,
                                 XmtNlayoutStretchability, 1,
                                 XmtNlayoutShrinkability, 1,
                                 NULL);
   space = XtVaCreateManagedWidget("space", 
                                 xmtLayoutSpaceGadgetClass,
                                 ask_items,
                                 XmtNlayoutIn, layout_box3,
                                 XmtNlayoutWidth, 0,
                                 XmtNlayoutHeight, 0,
                                 XmtNlayoutStretchability, 10000,
                                 XmtNlayoutShrinkability, 10000,
                                 NULL);
   add = XtVaCreateManagedWidget("addButton", 
                                 xmPushButtonWidgetClass, 
                                 ask_items,
                                 XmNlabelType, XmPIXMAP,
                                 XmNlabelPixmap, addPix,
                                 XmtNlayoutIn, layout_box3,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmtNlayoutShrinkability, 0,
                                 XmtNlayoutStretchability, 0,
                                 NULL);
   XtAddCallback(add, XmNactivateCallback,
                  AddItems, (XtPointer) ask_items);
   remove = XtVaCreateManagedWidget("removeButton", 
                                 xmPushButtonWidgetClass, 
                                 ask_items,
                                 XmNlabelType, XmPIXMAP,
                                 XmNlabelPixmap, removePix,
                                 XmtNlayoutIn, layout_box3,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmtNlayoutShrinkability, 0,
                                 XmtNlayoutStretchability, 0,
                                 NULL);
   XtAddCallback(remove, XmNactivateCallback,
                  RmItems, (XtPointer) ask_items);
   space = XtVaCreateManagedWidget("space", 
                                 xmtLayoutSpaceGadgetClass,
                                 ask_items,
                                 XmtNlayoutIn, layout_box3,
                                 XmtNlayoutWidth, 0,
                                 XmtNlayoutHeight, 0,
                                 XmtNlayoutStretchability, 10000,
                                 XmtNlayoutShrinkability, 10000,
                                 NULL);
   out_title = XtVaCreateManagedWidget("OutTitle",
                           xmtLayoutStringGadgetClass,
                           ask_items,
                           XmtNlayoutIn, layout_box5,
                           NULL);
   i = 0;
   XtSetArg(args[i], XmtNlayoutIn, layout_box5); i++;
   XtSetArg(args[i], XmtNlayoutStretchability, 0); i++;
   XtSetArg(args[i], XmtNlayoutShrinkability, 0); i++;
   XtSetArg(args[i], XmtNlayoutWidth, 300); i++;
   XtSetArg(args[i], XmtNitemStretch, 0); i++;
   XtSetArg(args[i], XmtNlayoutAllowResize, False); i++;
   XtSetArg(args[i], XmNselectionPolicy, XmEXTENDED_SELECT); i++;
/*    XtSetArg(args[i], XmNvisibleItemCount, 15); i++; */
   XtSetArg(args[i], XmtNlayoutHeight, 300); i++;
   chosen_list = XmCreateScrolledList(ask_items, "ItemsChosen", 
                           args, i);
#if 0
   i=0;
   XtSetArg(args[i], XmNscrollBarDisplayPolicy, XmAS_NEEDED); i++;
   XtSetValues(XtParent(chosen_list), args, i);
#endif
   XtManageChild(chosen_list);


   layout_box2 = XtVaCreateManagedWidget("layout_box2",
                           xmtLayoutBoxGadgetClass,
                           ask_items,
                           XmtNorientation, XmHORIZONTAL,
                           XmtNequal, True,
                           XmtNlayoutStretchability, 0,
                           XmtNlayoutShrinkability, 0,
                           NULL);

   ok_str = XmtCreateLocalizedXmString(ask_items, "@{Ok}");
   cancel_str = XmtCreateLocalizedXmString(ask_items, "@{Cancel}");
   help_str = XmtCreateLocalizedXmString(ask_items, "@{Help}");
                              
   okay = XtVaCreateManagedWidget("okButton", 
                                 xmPushButtonWidgetClass, 
                                 ask_items,
                                 XmNlabelString, ok_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmNshowAsDefault, True,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 NULL);

   cancel = XtVaCreateManagedWidget("cancelButton", 
                                 xmPushButtonWidgetClass, 
                                 ask_items,
                                 XmNlabelString, cancel_str,
                                 XmtNlayoutIn, layout_box2,
                                 XmtNlayoutJustification, XmtLayoutCentered,
                                 XmNdefaultButtonShadowThickness, 1, 
                                 NULL);

   help = XtVaCreateManagedWidget("helpButton", 
                                 xmPushButtonWidgetClass, 
                                 ask_items,
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
    XmtAddDeleteCallback(XtParent(ask_items), XmDO_NOTHING,
                      _XmtCancelCallback, (XtPointer)info);

    XtAddEventHandler(XtParent(ask_items), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
    XtAddEventHandler(XtParent(ask_items), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);
    
    /* cache the dialog in the PerScreenInfo structure */
    /* _AA: see ScreenP.h, don't know if it has to be adapted */
    info->items_dialog = ask_items;

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


static Boolean GetItems(
Widget w,
StringConst dialog_name,
StringConst title_default,
StringConst prompt_default,
lList *in_list,
int nm0,
String in_title,
lList **out_list,
lDescr *dp,
int nm1,
String out_title,
StringConst help_text_default 
) {
   Widget shell = XmtGetShell(w);
   XmtPerScreenInfo *info = XmtGetPerScreenInfo(shell);
   DialogData data;
   Widget dialog = 0;
   Widget help_w; 
   static String default_items_title;
   XmString message, title;
   String dialog_class = NULL;
   Widget avail_list, chosen_list, avail_list_title, chosen_list_title;

   DENTER(GUI_LAYER, "GetItems");

   /* make sure the shell we pop up over is not a menu shell! */
   while(XtIsOverrideShell(shell)) shell = XmtGetShell(XtParent(shell));

    /* localize the default title the first time through */
   if (!default_items_title) {
      default_items_title = XmtLocalize2(w, XmtITEMS_DIALOG_TITLE_DEFAULT,
                                   "XmtAskForTime", "title");
   }

   /* if there's no cached dialog, create one. */
   if (info->items_dialog == NULL) 
      CreateItemsDialog(info);
   dialog = info->items_dialog;
   dialog_class = XmtCItemsDialog;

   avail_list = XmtNameToWidget(dialog, "*ItemsAvailable");
   chosen_list = XmtNameToWidget(dialog, "*ItemsChosen");
   avail_list_title = XmtNameToWidget(dialog, "*InTitle");
   chosen_list_title = XmtNameToWidget(dialog, "*OutTitle");

   DPRINTF(("avail_list = %s\n", XtName(avail_list)));
   DPRINTF(("chosen_list = %s\n", XtName(chosen_list)));
   DPRINTF(("avail_list_title = %s\n", XtName(avail_list_title)));
   DPRINTF(("chosen_list_title = %s\n", XtName(chosen_list_title)));

   help_w = XmtNameToWidget(dialog, "helpButton");

   /* if this dialog has a name, look up its resources */
   if (dialog_name != NULL) {
      resources[0].default_addr = (XtPointer) prompt_default;
      resources[1].default_addr = (XtPointer) title_default;
      resources[2].default_addr = (XtPointer) help_text_default;
      XtGetSubresources(shell, (XtPointer)&data,
                       (String)dialog_name, dialog_class, 
                       resources, XtNumber(resources),
                       NULL, 0);
   }
   else { /* otherwise use arguments as defaults */
      data.message = (String) prompt_default;
      data.title = title_default ? (String)title_default : XmtITEMS_DIALOG_TITLE_DEFAULT;
      data.help_text = (String) help_text_default;
   }

    /* create the XmStrings */
    message = XmtCreateLocalizedXmString(shell, data.message);
    title = XmtCreateLocalizedXmString(shell, data.title);

    /* set resources on the dialog */
    XmtLayoutDisableLayout(ask_items);

    XtVaSetValues(dialog,
                XmNdialogTitle, title,
                NULL);
/*     XtVaSetValues(messageW, */
/*                   XmtNlabelString, message, */
/*                   NULL); */
    XtVaSetValues(avail_list_title, 
                  XmtNlabel, in_title ? in_title : "",
                  NULL);
    lPSortList(in_list, "%I+", nm0);
    UpdateXmListFromCull(avail_list, XmFONTLIST_DEFAULT_TAG, in_list, nm0);
    lPSortList(*out_list, "%I+", nm1);
    XtVaSetValues(chosen_list_title, 
                  XmtNlabel, out_title ? out_title : "",
                  NULL);
    UpdateXmListFromCull(chosen_list, XmFONTLIST_DEFAULT_TAG, *out_list, nm1);

    XmtLayoutEnableLayout(ask_items);

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
       /*
       ** the incoming list has to be freed
       ** if *out_list = NULL lFreeList returns
       ** otherwise it's an error anyway
       */
       lFreeList(out_list);
       *out_list = XmStringToCull(chosen_list, dp, nm1, ALL_ITEMS);
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
Boolean XmtAskForItems(
Widget w,
StringConst dialog_name,
StringConst prompt_default,
StringConst title_default,
lList *in_list,
int nm0,
String in_title,
lList **out_list,
lDescr *dp,
int nm1,
String out_title,
StringConst help_text_default 
) {
    return GetItems(w, dialog_name, title_default, prompt_default,  
                  in_list, nm0, in_title,
                  out_list, dp, nm1, out_title, help_text_default);
}
