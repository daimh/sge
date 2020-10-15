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
#include <stdio.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/TextF.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_timer.h"
#include "qmon_comm.h"
#include "qmon_widgets.h"
#include "qmon_message.h"
#include "qmon_ckpt.h"
#include "qmon_globals.h"
#include "AskForItems.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ckpt.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi.h"

static Widget qmon_ckpt = 0;
static Widget ckpt_names = 0;
static Widget ckpt_conf_list = 0;
static Widget ckpt_ask_layout = 0;
static Widget ckpt_name_w = 0;
static Widget ckpt_interface_w = 0;
static Widget ckpt_ckpt_command_w = 0;
static Widget ckpt_migr_command_w = 0;
static Widget ckpt_rest_command_w = 0;
static Widget ckpt_clean_command_w = 0;
static Widget ckpt_ckpt_dir_w = 0;
static Widget ckpt_when_w = 0;
static Widget ckpt_signal_w = 0;
static int add_mode = 0;

static String ckpt_interface_types[] = { 
   "USERDEFINED", 
   "HIBERNATOR", 
   "TRANSPARENT", 
   "CPR", 
   "APPLICATION-LEVEL"
};

/*-------------------------------------------------------------------------*/
static void qmonPopdownCkptConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateCkptConfig(Widget parent);
static void qmonCkptFillConf(Widget w, lListElem *ep);
static void qmonSelectCkpt(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonCkptResetAsk(void);
static void qmonCkptSetAsk(lListElem *pep);
static Widget qmonCreateCkptAsk(Widget parent);
static Boolean qmonCkptGetAsk(lListElem *pep);

/*-------------------------------------------------------------------------*/
void qmonPopupCkptConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonPopupCkptConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_ckpt) {
      shell = XmtGetTopLevelShell(w);
      qmon_ckpt = qmonCreateCkptConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownCkptConfig, NULL);
      /*
      ** create ask layout
      */
      ckpt_ask_layout = qmonCreateCkptAsk(qmon_ckpt);

   } 
   XSync(XtDisplay(qmon_ckpt), 0);
   XmUpdateDisplay(qmon_ckpt);

   qmonMirrorMultiAnswer(CKPT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }

   
   qmonTimerAddUpdateProc(CKPT_T, "updateCkptList", updateCkptList);
   qmonStartTimer(CKPT_T);
   updateCkptList();
   XmListSelectPos(ckpt_names, 1, True);

   XtManageChild(qmon_ckpt);
   XRaiseWindow(XtDisplay(XtParent(qmon_ckpt)), XtWindow(XtParent(qmon_ckpt)));

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateCkptList(void)
{
   lList *cl;
   
   DENTER(GUI_LAYER, "updateCkptList");

   cl = qmonMirrorList(SGE_CK_LIST);
   lPSortList(cl, "%I+", CK_name);
   UpdateXmListFromCull(ckpt_names, XmFONTLIST_DEFAULT_TAG, cl, CK_name);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownCkptConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownCkptConfig");

   XtUnmanageChild(qmon_ckpt);
   qmonStopTimer(CKPT_T);
   qmonTimerRmUpdateProc(CKPT_T, "updateCkptList");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptFillConf(
Widget w,
lListElem *ep 
) {
   XmString *items;
   Cardinal itemCount; 
   char buf[BUFSIZ];
   int i;

   DENTER(GUI_LAYER, "qmonCkptFillConf");
   
   if (!ep) {
      /*
      ** clear the ckpt_conf_list
      */
      XtVaSetValues( ckpt_conf_list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
      DEXIT;
      return;
   }
   
   itemCount = 9;
   items = (XmString*) XtMalloc(sizeof(XmString)*itemCount); 

   i = 0;

   /* name */
   sprintf(buf, "%-20.20s %s", "Name", lGetString(ep, CK_name));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* interface */
   sprintf(buf, "%-20.20s %s", "Interface", lGetString(ep, CK_interface));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* ckpt command */
   sprintf(buf, "%-20.20s %s", "Checkpoint command", 
			lGetString(ep, CK_ckpt_command));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* migr command */
   sprintf(buf, "%-20.20s %s", "Migrate command", 
			lGetString(ep, CK_migr_command));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* rest command */
   sprintf(buf, "%-20.20s %s", "Restart command", 
			lGetString(ep, CK_rest_command));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* clean command */
   sprintf(buf, "%-20.20s %s", "Clean command", 
			lGetString(ep, CK_clean_command));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* CKPT directory  */
   sprintf(buf, "%-20.20s %s", "Checkpoint directory", 
               lGetString(ep, CK_ckpt_dir));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* CK_when */
   sprintf(buf, "%-20.20s %s", "Checkpoint When", lGetString(ep, CK_when));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* CK_signal */
   sprintf(buf, "%-20.20s %s", "Checkpoint Signal", lGetString(ep, CK_signal));
   items[i++] = XmStringCreateLtoR(buf, "LIST");


   XtVaSetValues( ckpt_conf_list, 
                  XmNitems, items,
                  XmNitemCount, itemCount,
                  NULL);
   XmStringTableFree(items, itemCount);
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectCkpt(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *ckpt_name;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonSelectCkpt");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &ckpt_name)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ep = ckpt_list_locate(qmonMirrorList(SGE_CK_LIST), ckpt_name);

   XtFree((char*) ckpt_name);

   qmonCkptFillConf(ckpt_conf_list, ep);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreateCkptConfig(
Widget parent 
) {
   Widget ckpt_layout, ckpt_add, ckpt_delete, ckpt_modify, ckpt_done, 
            ckpt_main_link;

   DENTER(GUI_LAYER, "qmonCreateCkptConfig");
   
   ckpt_layout = XmtBuildQueryDialog( parent, "qmon_ckpt",
                           NULL, 0,
                           "ckpt_names", &ckpt_names,
                           "ckpt_conf_list", &ckpt_conf_list,
                           "ckpt_add", &ckpt_add,
                           "ckpt_delete", &ckpt_delete,
                           "ckpt_done", &ckpt_done,
                           "ckpt_modify", &ckpt_modify,
                           "ckpt_main_link", &ckpt_main_link,
                           NULL);

   XtAddCallback(ckpt_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(ckpt_names, XmNbrowseSelectionCallback, 
                     qmonSelectCkpt, NULL);
   XtAddCallback(ckpt_done, XmNactivateCallback, 
                     qmonPopdownCkptConfig, NULL);
   XtAddCallback(ckpt_add, XmNactivateCallback, 
                     qmonCkptAdd, NULL); 
   XtAddCallback(ckpt_modify, XmNactivateCallback, 
                     qmonCkptModify, NULL); 
   XtAddCallback(ckpt_delete, XmNactivateCallback, 
                     qmonCkptDelete, NULL); 

   XtAddEventHandler(XtParent(ckpt_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return ckpt_layout;
}



/*-------------------------------------------------------------------------*/
static Widget qmonCreateCkptAsk(
Widget parent 
) {
   Widget ckpt_ok, ckpt_cancel;

   DENTER(GUI_LAYER, "qmonCreateCkptAsk");
   
   ckpt_ask_layout = XmtBuildQueryDialog( parent, "ckpt_ask_shell",
                           NULL, 0,
                           "ckpt_ok", &ckpt_ok,
                           "ckpt_cancel", &ckpt_cancel,
                           "ckpt_name", &ckpt_name_w,
                           "ckpt_interface", &ckpt_interface_w,
                           "ckpt_ckpt_command", &ckpt_ckpt_command_w,
                           "ckpt_migr_command", &ckpt_migr_command_w,
                           "ckpt_rest_command", &ckpt_rest_command_w,
                           "ckpt_clean_command", &ckpt_clean_command_w,
                           "ckpt_ckpt_dir", &ckpt_ckpt_dir_w,
                           "ckpt_when", &ckpt_when_w,
                           "ckpt_signal", &ckpt_signal_w,
                           NULL);

   XtAddCallback(ckpt_ok, XmNactivateCallback, 
                     qmonCkptOk, NULL);
   XtAddCallback(ckpt_cancel, XmNactivateCallback, 
                     qmonCkptCancel, NULL);
   XtAddEventHandler(XtParent(ckpt_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return ckpt_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptAdd(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonCkptAdd");

   qmonCkptResetAsk();
   XtVaSetValues( ckpt_name_w,
                  XmNeditable, True,
                  NULL);
   add_mode = 1;
   XtManageChild(ckpt_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptModify(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *ckptnames;
   Cardinal ckptnum;
   String ckptstr;
   lListElem *ckp = NULL;

   DENTER(GUI_LAYER, "qmonCkptModify");

   /*
   ** on ockptning the dialog fill in the old values
   */
   XtVaGetValues( ckpt_names,
                  XmNselectedItems, &ckptnames,
                  XmNselectedItemCount, &ckptnum,
                  NULL);
   
   if (ckptnum == 1 && 
         XmStringGetLtoR(ckptnames[0], XmFONTLIST_DEFAULT_TAG, &ckptstr)) {
      XmtInputFieldSetString(ckpt_name_w, ckptstr);
      XtVaSetValues( ckpt_name_w,
                     XmNeditable, False,
                     NULL);
      ckp = ckpt_list_locate(qmonMirrorList(SGE_CK_LIST), ckptstr);
      XtFree((char*)ckptstr);
      if (ckp) {
         add_mode = 0;
         qmonCkptSetAsk(ckp);
         XtManageChild(ckpt_ask_layout);
      }
   }

   XtManageChild(ckpt_ask_layout);

   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonCkptOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *ckptl = NULL;
   lList *alp;
   lEnumeration *what;
   XmString xckpt_name = NULL;
   StringConst ckpt_name = NULL;

   DENTER(GUI_LAYER, "qmonCkptOk");
   /*
   ** get the contents of the dialog fields here,
   ** build the cull list and send gdi request
   ** depending on success of gdi request close the dialog or stay open
   */
   ckptl = lCreateElemList("CKPT_ADD", CK_Type, 1);
   
   if (ckptl) {
      if (qmonCkptGetAsk(lFirst(ckptl))) {
         ckpt_name = (StringConst)lGetString(lFirst(ckptl), CK_name);
         /*
         ** gdi call 
         */
         what = lWhat("%T(ALL)", CK_Type);
         
         if (add_mode) {
            alp = qmonAddList(SGE_CK_LIST, qmonMirrorListRef(SGE_CK_LIST),
                              CK_name, &ckptl, NULL, what);
         }
         else {
            alp = qmonModList(SGE_CK_LIST, qmonMirrorListRef(SGE_CK_LIST),
                              CK_name, &ckptl, NULL, what);
         }

         qmonMessageBox(w, alp, 0);

         if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK) {
            XtUnmanageChild(ckpt_ask_layout);
            updateCkptList();
            /*
            ** select the modified or added Ckpt
            */
            xckpt_name = XmtCreateXmString(ckpt_name);
            XmListSelectItem(ckpt_names, xckpt_name, True);
            XmStringFree(xckpt_name);
         }
         lFreeWhat(&what);
         lFreeList(&alp);
      }
      lFreeList(&ckptl);
   }

   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonCkptCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonCkptCancel");

   XtUnmanageChild(ckpt_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptDelete(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   Cardinal itemCount = 0;
   Boolean status, answer;

   DENTER(GUI_LAYER, "qmonCkptDelete");
    
   lp = XmStringToCull(ckpt_names, CK_Type, CK_name, SELECTED_ITEMS); 

   if (lp) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{ckpt.askdel.Do you really want to delete the\nselected Checkpointing Configuration ?}", 
                     "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                     False, &answer, NULL);
      if (answer) { 
         what = lWhat("%T(ALL)", CK_Type);
         alp = qmonDelList(SGE_CK_LIST, qmonMirrorListRef(SGE_CK_LIST),
                                 CK_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);

         lFreeWhat(&what);
         lFreeList(&alp);

         updateCkptList();
         XtVaGetValues( ckpt_names,
                        XmNitemCount, &itemCount,
                        NULL);
         if (itemCount)
            XmListSelectPos(ckpt_names, 1, True);
         else
            qmonCkptFillConf(ckpt_names, NULL);

      }
      lFreeList(&lp);
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptSetAsk(
lListElem *ckp 
) {
   StringConst ckpt_name = NULL;
   StringConst ckpt_interface = NULL;
   StringConst ckpt_ckpt_command = NULL;
   StringConst ckpt_migr_command = NULL;
   StringConst ckpt_rest_command = NULL;
   StringConst ckpt_clean_command = NULL;
   StringConst ckpt_ckpt_dir = NULL;
   StringConst ckpt_when = NULL;
   StringConst ckpt_signal = NULL;
   int i;
   int state = 0;

   DENTER(GUI_LAYER, "qmonCkptSetAsk");

   if (!ckp) {
      DEXIT;
      return;
   }

   ckpt_name = (StringConst)lGetString(ckp, CK_name);
   if (ckpt_name)
      XmtInputFieldSetString(ckpt_name_w, ckpt_name);

   ckpt_interface = (StringConst)lGetString(ckp, CK_interface);
   if (ckpt_interface) {
      for (i=0; i<XtNumber(ckpt_interface_types); i++) {
         if (!strcasecmp(ckpt_interface, ckpt_interface_types[i])) {
            XmtChooserSetState(ckpt_interface_w, i, False);
            break;
         }
      }
   }
   else
      XmtChooserSetState(ckpt_interface_w, 0, False);

   ckpt_ckpt_command = (StringConst)lGetString(ckp, CK_ckpt_command);
   if (ckpt_ckpt_command)
      XmtInputFieldSetString(ckpt_ckpt_command_w, ckpt_ckpt_command);

   ckpt_migr_command = (StringConst)lGetString(ckp, CK_migr_command);
   if (ckpt_migr_command)
      XmtInputFieldSetString(ckpt_migr_command_w, ckpt_migr_command);

   ckpt_rest_command = (StringConst)lGetString(ckp, CK_rest_command);
   if (ckpt_rest_command)
      XmtInputFieldSetString(ckpt_rest_command_w, ckpt_rest_command);

   ckpt_clean_command = (StringConst)lGetString(ckp, CK_clean_command);
   if (ckpt_clean_command)
      XmtInputFieldSetString(ckpt_clean_command_w, ckpt_clean_command);

   ckpt_ckpt_dir = (StringConst)lGetString(ckp, CK_ckpt_dir);
   if (ckpt_ckpt_dir)
      XmtInputFieldSetString(ckpt_ckpt_dir_w, ckpt_ckpt_dir);

   ckpt_when = (StringConst)lGetString(ckp, CK_when);
   if (ckpt_when && strchr(ckpt_when, 's'))
      state |= 1;
   if (ckpt_when && strchr(ckpt_when, 'm'))
      state |= 2;
   if (ckpt_when && strchr(ckpt_when, 'x'))
      state |= 4;
   if (ckpt_when && strchr(ckpt_when, 'r'))
      state |= 8;

   if (!state)
      state = 1;
   XmtChooserSetState(ckpt_when_w, state, False);
   
   ckpt_signal = (StringConst)lGetString(ckp, CK_signal);
   if (ckpt_signal)
      XmtInputFieldSetString(ckpt_signal_w, ckpt_signal);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonCkptResetAsk(void)
{
   int state = 5; /* ckpt_when set to sx */

   DENTER(GUI_LAYER, "qmonCkptResetAsk");

   XmtInputFieldSetString(ckpt_name_w, "");
   XmtChooserSetState(ckpt_interface_w, 0, False);
   XmtInputFieldSetString(ckpt_ckpt_command_w, "NONE");
   XmtInputFieldSetString(ckpt_migr_command_w, "NONE");
   XmtInputFieldSetString(ckpt_rest_command_w, "NONE");
   XmtInputFieldSetString(ckpt_clean_command_w, "NONE");
   XmtInputFieldSetString(ckpt_ckpt_dir_w, "/tmp");
   XmtChooserSetState(ckpt_when_w, state, False);
   XmtInputFieldSetString(ckpt_signal_w, "NONE");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonCkptGetAsk(
lListElem *ckp 
) {
   String ckpt_name = NULL;
   String ckpt_interface = NULL;
   String ckpt_ckpt_command = NULL;
   String ckpt_migr_command = NULL;
   String ckpt_rest_command = NULL;
   String ckpt_clean_command = NULL;
   String ckpt_ckpt_dir = NULL;
   String ckpt_signal = NULL;
   int state;
   int i;
   char ckpt_when[20];
   static String ckpt_when_strings[] = { 
      "s",  /* shutdown of execd    */
      "m",  /* minimum CPU interval */ 
      "x",  /* suspend of Job       */
      "r"   /* reschedule job       */
   };

   DENTER(GUI_LAYER, "qmonCkptGetAsk");

   if (!ckp) {
      DEXIT;
      return False;
   }

   ckpt_name = XmtInputFieldGetString(ckpt_name_w);
   if (!ckpt_name || ckpt_name[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint name required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_name, qmon_trim(ckpt_name));
  
   state = XmtChooserGetState(ckpt_interface_w);
   ckpt_interface = ckpt_interface_types[state];
   if (!ckpt_interface || ckpt_interface[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint interface required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_interface, ckpt_interface);
  
   ckpt_ckpt_command = XmtInputFieldGetString(ckpt_ckpt_command_w);
   if (!ckpt_ckpt_command || ckpt_ckpt_command[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint ckpt_command required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_ckpt_command, ckpt_ckpt_command);
      
  
   ckpt_migr_command = XmtInputFieldGetString(ckpt_migr_command_w);
   if (!ckpt_migr_command || ckpt_migr_command[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint migr_command required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_migr_command, ckpt_migr_command);
  
   ckpt_rest_command = XmtInputFieldGetString(ckpt_rest_command_w);
   if (!ckpt_rest_command || ckpt_rest_command[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint rest_command required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_rest_command, ckpt_rest_command);
  
   ckpt_clean_command = XmtInputFieldGetString(ckpt_clean_command_w);
   if (!ckpt_clean_command || ckpt_clean_command[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint clean_command required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_clean_command, ckpt_clean_command);
  
   ckpt_ckpt_dir = XmtInputFieldGetString(ckpt_ckpt_dir_w);
   if (!ckpt_ckpt_dir || ckpt_ckpt_dir[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint ckpt_dir required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_ckpt_dir, ckpt_ckpt_dir);
  
   state = XmtChooserGetState(ckpt_when_w);
   strcpy(ckpt_when, "");
   for (i=0; i<XtNumber(ckpt_when_strings); i++) {
      if (state & (1<<i))
         strcat(ckpt_when, ckpt_when_strings[i]);
   }
   if (ckpt_when[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint when required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_when, ckpt_when);
  
   ckpt_signal = XmtInputFieldGetString(ckpt_signal_w);
   if (!ckpt_signal || ckpt_signal[0] == '\0') {
      qmonMessageShow(ckpt_ask_layout, True, "Checkpoint signal required !");
      DEXIT;
      return False;
   }
   lSetString(ckp, CK_signal, ckpt_signal);
  
   DEXIT;
   return True;
}
