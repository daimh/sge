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

#include <Xm/Xm.h>
#include <Xm/List.h>

#include <Xmt/Xmt.h>
#include <Xmt/Help.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/InputField.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_manop.h"
#include "qmon_comm.h"
#include "qmon_globals.h"
#include "qmon_timer.h"
#include "qmon_message.h"
#include "qmon_ticket.h"
#include "qmon_matrix.h"
#include "Tab.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_feature.h"

#include "gdi/sge_gdi.h"

/*-------------------------------------------------------------------------*/
static Widget qmonCreateManopConfig(Widget parent);
static void qmonManopHelp(Widget w, XtPointer cld, XtPointer cad);
static void qmonManopFillList(void);
static void qmonManopAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonManopDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonManopFolderChange(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateUserConfig(Widget parent, XtPointer cld);

static void updateUsersetList(void);
static Widget qmonCreateUsersetConfig(Widget parent, XtPointer cld);
static void qmonUsersetFillConf(Widget w, lListElem *ep);
static void qmonSelectUserset(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetType(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetDelete(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateUsersetAsk(Widget parent);
static void qmonUsersetOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonUsersetUserAdd(Widget w, XtPointer cld, XtPointer cad);
#if 0 /* EB: not used */
static void qmonUsersetName(Widget w, XtPointer cld, XtPointer cad);
#endif

static void qmonUserAskForProject(Widget w, XtPointer cld, XtPointer cad);
static void qmonPopdownManopConfig(Widget w, XtPointer cld, XtPointer cad);
/*-------------------------------------------------------------------------*/
static Widget qmon_manop = NULL;
static Widget manager_list = NULL;
static Widget manager_name = NULL;
static Widget operator_list = NULL;
static Widget operator_name = NULL;
static Widget user_matrix = NULL;
static Widget user_name = NULL;
static Widget manop_modify = NULL;

static int dialog_mode = SGE_UM_LIST;

static Widget manop_folder = 0;
static Widget userset_layout = 0;
static Widget userset_names = 0;
static Widget userset_user_list = 0;
static Widget userset_ask_layout = 0;
static Widget uset_user_list = 0;
static Widget uset_user = 0;
static Widget uset_name = 0;
static Widget uset_type = 0;
static Widget uset_user_remove = 0;
static Widget userset_type = 0;
static int add_mode = 0;
static int userset_type_state = US_ACL;



/*-------------------------------------------------------------------------*/
void qmonPopupManopConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;
   
   DENTER(GUI_LAYER, "qmonPopupManopConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   /*
   ** create the dialog, if it doesn't exist
   */
   if (!qmon_manop) {
      shell = XmtGetTopLevelShell(w);
      qmon_manop = qmonCreateManopConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownManopConfig, NULL);
      XtAddEventHandler(XtParent(qmon_manop), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
      XtAddEventHandler(XtParent(qmon_manop), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);
   } 
   
   qmonMirrorMultiAnswer(MANAGER_T | OPERATOR_T | USERSET_T | USER_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   qmonTimerAddUpdateProc(USERSET_T, "updateUsersetList", updateUsersetList);
   qmonStartTimer(MANAGER_T | OPERATOR_T | USERSET_T | USER_T);
   qmonManopFillList();

   xmui_manage(qmon_manop);

   /*
   ** switch to userset
   */
   if (cld) {
      XmTabSetTabWidget(manop_folder, userset_layout, True);
   }

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownManopConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownManopConfig");

   qmonStopTimer(MANAGER_T | OPERATOR_T | USER_T | USERSET_T);
   qmonTimerRmUpdateProc(USERSET_T, "updateUsersetList");

   xmui_unmanage(qmon_manop);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Widget qmonCreateManopConfig(
Widget parent 
) {
   Widget manop_layout, manop_add, manop_delete, manop_done, manop_help,
          manop_main_link, manop_tickets;

   DENTER(GUI_LAYER, "qmonCreateManopConfig");
   
   manop_layout = XmtBuildQueryDialog( parent, "qmon_manop",
                           NULL, 0,
                           "manop_folder", &manop_folder,
                           "manop_add", &manop_add,
                           "manop_modify", &manop_modify,
                           "manop_tickets", &manop_tickets,
                           "manop_delete", &manop_delete,
                           "manop_done", &manop_done,
                           "manop_help", &manop_help,
                           "manop_main_link", &manop_main_link,
                           "manager_list", &manager_list,
                           "manager_name", &manager_name,
                           "operator_list", &operator_list,
                           "operator_name", &operator_name,
                           NULL);
   
   userset_layout = qmonCreateUsersetConfig(manop_folder, NULL);
   XtManageChild(userset_layout);

   qmonCreateUserConfig(manop_folder, NULL);
   XtAddCallback(manop_tickets, XmNactivateCallback, 
                     qmonPopupTicketOverview, NULL);
                               
   XtAddCallback(manop_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(manop_folder, XmNvalueChangedCallback, 
                     qmonManopFolderChange, NULL);
   XtAddCallback(manop_done, XmNactivateCallback, 
                     qmonPopdownManopConfig, NULL);
   XtAddCallback(manop_add, XmNactivateCallback, 
                     qmonManopAdd, (XtPointer) 1); 
   XtAddCallback(manop_modify, XmNactivateCallback, 
                     qmonManopAdd, NULL); 
   XtAddCallback(manager_name, XmNactivateCallback /* XmtNinputCallback */, 
                     qmonManopAdd, NULL);
   XtAddCallback(operator_name, XmNactivateCallback /* XmtNinputCallback */, 
                     qmonManopAdd, NULL);
   XtAddCallback(manop_delete, XmNactivateCallback, 
                     qmonManopDelete, NULL); 
   XtAddCallback(manop_help, XmNactivateCallback, 
                     qmonManopHelp, NULL);
   
   XtAddEventHandler(XtParent(manop_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return manop_layout;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreateUserConfig(
Widget parent,
XtPointer cld 
) {
   Widget user_layout;

   DENTER(GUI_LAYER, "qmonCreateUserConfig");
   
   user_layout = XmtCreateLayout(parent, "user_layout", NULL, 0);
   XmtCreateQueryChildren( user_layout,
                           "user_name", &user_name,
                           "user_matrix", &user_matrix,
                           NULL);

   XtAddCallback(user_name, XmNactivateCallback, 
                     qmonManopAdd, NULL);
   XtAddCallback(user_matrix, XmNlabelActivateCallback,
                  qmonUserAskForProject, NULL);


   DEXIT;
   return user_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonManopFillList(void)
{
   lList *lp, *cl;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonManopFillList");

   /* manager list */
   lp = qmonMirrorList(SGE_UM_LIST);
   lPSortList(lp, "%I+", UM_name);
   UpdateXmListFromCull(manager_list, XmFONTLIST_DEFAULT_TAG, lp, UM_name);
   
   /* operator list */
   lp = qmonMirrorList(SGE_UO_LIST);
   lPSortList(lp, "%I+", UO_name);
   UpdateXmListFromCull(operator_list, XmFONTLIST_DEFAULT_TAG, lp, UO_name);

   /* userset list */
   updateUsersetList();

   /* user list */
   lp = qmonMirrorList(SGE_UU_LIST);
   lPSortList(lp, "%I+", UU_name);
   /*
   ** set UU_default_project to NONE
   */
   cl = lCopyList("cl", lp);
   for_each (ep, cl) {
      if (ep && !lGetString(ep, UU_default_project)) 
         lSetString(ep, UU_default_project, "NONE");
   }
   qmonSetNxN(user_matrix, cl, 3, UU_name, UU_default_project, UU_delete_time);
   lFreeList(&cl);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateManopList(void)
{
   DENTER(GUI_LAYER, "updateManopList");

   if (qmon_manop && XtIsManaged(qmon_manop))
      qmonManopFillList();

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonManopDelete(Widget w, XtPointer cld, XtPointer cad)
{
   int type = dialog_mode;
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   lDescr *dp;
   int nm;
   int rows, i;
   

   DENTER(GUI_LAYER, "qmonManopDelete");

   switch (type) {
      case SGE_UM_LIST:
         lp = XmStringToCull(manager_list, UM_Type, UM_name, SELECTED_ITEMS);
         dp = UM_Type;
         nm = UM_name;
         break;
      case SGE_UO_LIST:
         lp = XmStringToCull(operator_list, UO_Type, UO_name, SELECTED_ITEMS);
         dp = UO_Type;
         nm = UO_name;
         break;

      case SGE_UU_LIST:
         rows = XbaeMatrixNumRows(user_matrix);
         for (i=0; i<rows; i++) {
            String s = XbaeMatrixGetCell(user_matrix, i, 0);
            if (XbaeMatrixIsRowSelected(user_matrix, i) && s &&
                    strcmp(s, ""))
               lAddElemStr(&lp, UU_name, s, UU_Type); 
         } 
         XbaeMatrixDeselectAll(user_matrix);
         dp = UU_Type;
         nm = UU_name;
         break;

      case SGE_US_LIST:
         qmonUsersetDelete(w, NULL, NULL);
         DEXIT;
         return;
      default:
         DEXIT;
         return;
   }
    
      
   if (lp) {
      what = lWhat("%T(ALL)", dp);
      alp = qmonDelList(type, qmonMirrorListRef(type), nm, &lp, NULL, what);

      qmonMessageBox(w, alp, 0);
   
      updateManopList();

      lFreeList(&alp);
      lFreeList(&lp);
      lFreeWhat(&what);
   } 
   
   DEXIT;
}
         
/*-------------------------------------------------------------------------*/
static void qmonManopAdd(Widget w, XtPointer cld, XtPointer cad)
{
   int type = dialog_mode;
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   String user = NULL;
   Widget username;
   int nm;
   lDescr *dp;

   DENTER(GUI_LAYER, "qmonManopAdd");

   switch (type) {
      case SGE_UM_LIST:
         dp = UM_Type;
         nm = UM_name;
         username = manager_name;
         break;
      case SGE_UO_LIST:
         dp = UO_Type;
         nm = UO_name;
         username = operator_name;
         break;

      case SGE_UU_LIST:
         dp = UU_Type;
         nm = UU_name;
         username = user_name;
         break;

      case SGE_US_LIST:
         if ((long)cld == 1)
            qmonUsersetAdd(w, NULL, NULL);
         else
            qmonUsersetModify(w, NULL, NULL);
         DEXIT;
         return;
      default:
         DEXIT;
         return;
   }

   user = XmtInputFieldGetString(username);
   
   if (user && user[0] != '\0' && user[0] != ' ') { 
      what = lWhat("%T(ALL)", dp);
      if (!(lp = lCreateElemList("AH", dp, 1))) {
         fprintf(stderr, "lCreateElemList failed\n");
         lFreeWhat(&what);
         DEXIT;
         return;
      }
      lSetString(lFirst(lp), nm, user);
/*       if (type == SGE_UU_LIST) */
/*          lSetString(lFirst(lp), UU_default_project, "NONE"); */
      
      alp = qmonAddList(type, qmonMirrorListRef(type), 
                           nm, &lp, NULL, what);
         
      qmonMessageBox(w, alp, 0);

      updateManopList();
         
      XmtInputFieldSetString(username, "");

      lFreeWhat(&what);
      lFreeList(&lp);
      lFreeList(&alp);
   }
   DEXIT;
}
         
/*-------------------------------------------------------------------------*/
static void qmonManopFolderChange(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   XmTabCallbackStruct *cbs = (XmTabCallbackStruct *) cad;

   DENTER(GUI_LAYER, "qmonManopFolderChange");
   
   DPRINTF(("%s\n", XtName(cbs->tab_child)));

   if (!strcmp(XtName(cbs->tab_child), "manager_layout"))
      dialog_mode = SGE_UM_LIST;

   if (!strcmp(XtName(cbs->tab_child), "operator_layout"))
      dialog_mode = SGE_UO_LIST;

   if (!strcmp(XtName(cbs->tab_child), "user_layout"))
      dialog_mode = SGE_UU_LIST;

   if (!strcmp(XtName(cbs->tab_child), "userset_layout"))
      dialog_mode = SGE_US_LIST;

   /*
   ** fetch changed lists and update dialogues
   */
   qmonMirrorMultiAnswer(MANAGER_T | OPERATOR_T | USERSET_T | USER_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   updateManopList();

   /*
   ** set Modify button sensitivity
   */
   if (dialog_mode==SGE_US_LIST) {
      XtSetSensitive(manop_modify, True);
   } else {
      XtSetSensitive(manop_modify, False);
   }   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonManopHelp(Widget w, XtPointer cld, XtPointer cad)
{
   Widget widget = 0;
   
   DENTER(GUI_LAYER, "qmonManopHelp");

   switch(dialog_mode) {
      case SGE_UM_LIST:
         widget = manager_list; 
         break;
      case SGE_UO_LIST:
         widget = operator_list; 
         break;
      case SGE_UU_LIST:
         widget = user_matrix; 
         break;
      case SGE_US_LIST:
         widget = userset_layout; 
         break;
      default:
         DEXIT;
         return;
   }

   XmtHelpDisplayContextHelp(widget);  

   DEXIT;
}



/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static void updateUsersetList(void)
{
   lList *al;
   XmString *selectedItems;
   Cardinal selectedItemCount;
   Cardinal itemCount;
   
   DENTER(GUI_LAYER, "updateUsersetList");

   al = qmonMirrorList(SGE_US_LIST);
   lPSortList(al, "%I+", US_name);
   UpdateXmListFromCull(userset_names, XmFONTLIST_DEFAULT_TAG, al, 
                           US_name);
   XtVaGetValues( userset_names,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  XmNitemCount, &itemCount,
                  NULL);

   if (selectedItemCount)
      XmListSelectItem(userset_names, selectedItems[0], True);
   else if (itemCount)
      XmListSelectPos(userset_names, 1, True);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetFillConf(
Widget w,
lListElem *ep 
) {
   lList *ul;

   DENTER(GUI_LAYER, "qmonUsersetFillConf");
   
   if (!ep) {
      /*
      ** clear the userset_user_list
      */
      XtVaSetValues( userset_user_list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
      DEXIT;
      return;
   }

   userset_type_state = (int)lGetUlong(ep, US_type);
   XmtChooserSetState(userset_type, userset_type_state, False);
   ul = lGetList(ep, US_entries);
   lPSortList(ul, "%I+", UE_name);
   UpdateXmListFromCull(userset_user_list, XmFONTLIST_DEFAULT_TAG, ul, UE_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectUserset(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *usetname;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonSelectUserset");

   if (!XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &usetname)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ep = lGetElemStr(qmonMirrorList(SGE_US_LIST), US_name, usetname);
   if (dialog_mode==SGE_US_LIST) {
      if (usetname && !strcmp(usetname, DEFAULT_DEPARTMENT)) 
         XtSetSensitive(manop_modify, False);
      else
         XtSetSensitive(manop_modify, True);
   }   

   XtFree((char*) usetname);
   
   qmonUsersetFillConf(userset_user_list, ep);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetType(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonUsersetType");

   XmtChooserSetState(w, userset_type_state, False);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Widget qmonCreateUsersetConfig(
Widget parent,
XtPointer cld 
) {
   Widget userset_layout, shell;

   DENTER(TOP_LAYER, "qmonCreateUsersetConfig2");
   userset_layout = XmtCreateLayout( parent, "userset_layout", NULL, 0);
   XmtCreateQueryChildren( userset_layout, 
                           "userset_names", &userset_names,
                           "userset_user_list", &userset_user_list,
                           "userset_type", &userset_type,
                           NULL);


   /*
   ** create ask layout
   */
   shell = XmtGetShell(userset_layout);
   userset_ask_layout = qmonCreateUsersetAsk(shell);

   XtAddCallback(userset_names, XmNbrowseSelectionCallback, 
                     qmonSelectUserset, NULL);
   XtAddCallback(userset_type, XmNvalueChangedCallback,
                  qmonUsersetType, NULL);
                     


   DEXIT;
   return userset_layout;
}

   
/*-------------------------------------------------------------------------*/
static Widget qmonCreateUsersetAsk(
Widget parent 
) {
   Widget uset_ok, uset_cancel;

   DENTER(TOP_LAYER, "qmonCreateUsersetAsk");
   
   userset_ask_layout = XmtBuildQueryDialog( parent, "userset_ask_shell",
                           NULL, 0,
                           "uset_ok", &uset_ok,
                           "uset_cancel", &uset_cancel,
                           "uset_user_list", &uset_user_list,
                           "uset_user_remove", &uset_user_remove,
                           "uset_user", &uset_user,
                           "uset_name", &uset_name,
                           "uset_type", &uset_type,
                           NULL);

   XtAddCallback(uset_ok, XmNactivateCallback,
                  qmonUsersetOk, NULL);
   XtAddCallback(uset_cancel, XmNactivateCallback,
                  qmonUsersetCancel, NULL);
   XtAddCallback(uset_user, XmNinputCallback,
                  qmonUsersetUserAdd, NULL);

#if 0
      XtAddCallback(uset_name, XmNinputCallback,
                  qmonUsersetName, NULL);
#endif   

   XtAddEventHandler(XtParent(userset_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   DEXIT;
   return userset_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetAdd(Widget w, XtPointer cld, XtPointer cad)
{

   DENTER(GUI_LAYER, "qmonUsersetAdd");

   XmtInputFieldSetString(uset_name, "");

   XmtChooserSetState(uset_type, US_ACL, False);
   XtVaSetValues( uset_name,
                  XmNeditable, True,
                  NULL);
   XtVaSetValues( uset_user_list,
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);

   add_mode = 1;

   XtManageChild(userset_ask_layout);
   XmtSetInitialFocus(userset_ask_layout, uset_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetModify(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *usetname;
   Cardinal usetnum;
   String usetstr;
   XmString *usetusers;
   Cardinal usetusernum;

   DENTER(GUI_LAYER, "qmonUsersetModify");
   
   /*
   ** on opening the dialog fill in the old values
   */
   XtVaGetValues( userset_names,
                  XmNselectedItems, &usetname,
                  XmNselectedItemCount, &usetnum,
                  NULL);
   XtVaGetValues( userset_user_list,
                  XmNitems, &usetusers,
                  XmNitemCount, &usetusernum,
                  NULL);

   if (usetnum == 1 && 
         XmStringGetLtoR(usetname[0], XmFONTLIST_DEFAULT_TAG, &usetstr)) {
      XmtInputFieldSetString(uset_name, usetstr);
      XtFree((char *)usetstr);
      XtVaSetValues( uset_name,
                     XmNeditable, False,
                     NULL);
      XtVaSetValues( uset_user_list,
                     XmNitems, usetusers,
                     XmNitemCount, usetusernum,
                     NULL);

      XmtChooserSetState(uset_type, userset_type_state, False);

      add_mode = 0;

      XtManageChild(userset_ask_layout);
      XmtSetInitialFocus(userset_ask_layout, uset_user);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/

#if 0 /* EB: not used */
static void qmonUsersetName(Widget w, XtPointer cld, XtPointer cad)
{
   String name = NULL;

   DENTER(GUI_LAYER, "qmonUsersetName");

   name = XmtInputFieldGetString(uset_name);

   if (name && !strcmp(name, DEFAULT_DEPARTMENT)) {
      XtSetSensitive(uset_user, False);
      XtSetSensitive(uset_user_list, False);
      XtSetSensitive(uset_user_remove, False);
      XmtChooserSetState(uset_type, US_DEPT, False);
      XtSetSensitive(uset_type, False);
   }
   else {
      XtSetSensitive(uset_user, True);
      XtSetSensitive(uset_user_list, True);
      XtSetSensitive(uset_user_remove, True);
      XtSetSensitive(uset_type, True);
   }   

   DEXIT;
}
#endif

/*-------------------------------------------------------------------------*/
static void qmonUsersetUserAdd(Widget w, XtPointer cld, XtPointer cad)
{
   String user = NULL;

   DENTER(GUI_LAYER, "qmonUsersetUserAdd");

   user = XmtInputFieldGetString(uset_user);

   if (user && user[0] != '\0') {
      XmListAddItemUniqueSorted(uset_user_list, user);
      XmtInputFieldSetString(uset_user, "");
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonUsersetOk(Widget w, XtPointer cld, XtPointer cad)
{
   String usetname = NULL;
   XmString xusetname = NULL;
   lList *lp = NULL;
   lList *ul = NULL;
   lList *alp = NULL;
   lListElem *up = NULL;
   lEnumeration *what = NULL;
   Boolean status = False;
   int usettype = 0;

   DENTER(GUI_LAYER, "qmonUsersetOk");

   /*
   ** get the dialog data and
   */
   usetname = XmtInputFieldGetString(uset_name);

   usettype = XmtChooserGetState(uset_type); 

   /*
   ** usetname required, show warning dialog
   */
   if (usetname && usetname[0] != '\0') {
      ul = XmStringToCull(uset_user_list, UE_Type, UE_name, ALL_ITEMS);
      /*
      ** do gdi stuff here
      */ 
      if (add_mode) {
         lAddElemStr(&lp, US_name, usetname, US_Type);
      }
      else {
         up = lCopyElem(lGetElemStr(qmonMirrorList(SGE_US_LIST), 
                           US_name, usetname));
         lp = lCreateList("userset", US_Type);
         lAppendElem(lp, up);
      }

      lSetList(lFirst(lp), US_entries, ul);
      lSetUlong(lFirst(lp), US_type, usettype); 

      what = lWhat("%T(ALL)", US_Type);

      if (add_mode) {
         alp = qmonAddList(SGE_US_LIST, 
                        qmonMirrorListRef(SGE_US_LIST),
                        US_name, &lp, NULL, what);
      }
      else {
         alp = qmonModList(SGE_US_LIST, 
                           qmonMirrorListRef(SGE_US_LIST),
                           US_name, &lp, NULL, what);
      }

      if (!qmonMessageBox(w, alp, 0))
         status = True;

      updateUsersetList();

      xusetname = XmtCreateXmString(usetname);
      XmListSelectItem(userset_names, xusetname, True);
      XmStringFree(xusetname);

      lFreeWhat(&what);
      lFreeList(&lp);
      lFreeList(&alp);
   }  
   else { 
      qmonMessageShow(w, True, "Userset List Name required !");
   }       
   
   if (status) 
      XtUnmanageChild(userset_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonUsersetCancel");

   XtUnmanageChild(userset_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonUsersetDelete(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   Cardinal itemCount = 0;
   lEnumeration *what = NULL;
   Boolean answer;

   DENTER(GUI_LAYER, "qmonUsersetDelete");

   lp = XmStringToCull(userset_names, US_Type, US_name, SELECTED_ITEMS);
      
   if (lp) {
      XmtAskForBoolean(w, "xmtBooleanDialog", 
                  "@{userset.askdel.Do you really want to delete\nthe selected userset lists ?}", 
                  "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                  False, &answer, NULL);
         
      if (answer) { 
         what = lWhat("%T(ALL)", US_Type);
         alp = qmonDelList(SGE_US_LIST, 
                           qmonMirrorListRef(SGE_US_LIST),
                           US_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);
         
         lFreeWhat(&what);
         lFreeList(&lp);
         lFreeList(&alp);

         updateUsersetList();
         XtVaGetValues( userset_names,
                        XmNitemCount, &itemCount,
                        NULL);
         if (itemCount)
            XmListSelectPos(userset_names, 1, True);
         else
            qmonUsersetFillConf(userset_names, NULL);
      }
   }
   DEXIT;

}

/*-------------------------------------------------------------------------*/
static void qmonUserAskForProject(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixLabelActivateCallbackStruct *cbs = 
                            (XbaeMatrixLabelActivateCallbackStruct *) cad;
   Boolean status = False;
   lList *pl = NULL;
   lListElem *cep = NULL;
   int n, i;
   StringConst *strs = NULL;
   static char buf[BUFSIZ];
   lList *lp = NULL;
   lList *alp = NULL;
   lListElem *ep;
   static lEnumeration *what = NULL;
   
   DENTER(GUI_LAYER, "qmonUserAskForProject");

   if (!what) {
      what = lWhat("%T(%I %I)", UU_Type, UU_name, UU_default_project);
   }

   if (cbs->column != 1) {
        DEXIT;
        return;
   }
   
   qmonMirrorMultiAnswer(PROJECT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   pl = qmonMirrorList(SGE_PR_LIST);
   lPSortList(pl, "%I+", PR_name);
   n = lGetNumberOfElem(pl);
   if (n>0) {
      strs = (StringConst*)XtMalloc(sizeof(String)*(n+1)); 
      strs[0] = "NONE";
      for (cep=lFirst(pl), i=0; i<n; cep=lNext(cep), i++) {
        /*
        ** we get only references don't free, the strings
        */
        strs[i+1] = lGetString(cep, PR_name);
      }
    
      strcpy(buf, "");
      /* FIX_CONST_GUI */
      status = XmtAskForItem(w, NULL, "@{Select a project}",
                        "@{Available projects}", (String*)strs, n+1,
                        False, buf, BUFSIZ, NULL); 
      
      if (status) {
         int rows, i;
         rows = XbaeMatrixNumRows(user_matrix);
         for (i=0; i<rows; i++) {
            String s = XbaeMatrixGetCell(user_matrix, i, 0);
            if (XbaeMatrixIsRowSelected(user_matrix, i) && s &&
                    strcmp(s, "")) {
               ep = lAddElemStr(&lp, UU_name, s, UU_Type); 
               lSetString(ep, UU_default_project, buf);
            }
         } 
         XbaeMatrixDeselectAll(user_matrix);

         /*
         ** send to master as modify request
         */
         for_each (ep, lp) {
            if (ep && lGetString(ep, UU_default_project) && 
                !strcasecmp(lGetString(ep, UU_default_project), "NONE")) 
               lSetString(ep, UU_default_project, NULL);
         }
         if (lp) {
             alp = qmonModList(SGE_UU_LIST, 
                               qmonMirrorListRef(SGE_UU_LIST),
                               UU_name, &lp, NULL, what);
             qmonMessageBox(w, alp, 0);
             updateManopList();

             lFreeList(&alp);
             lFreeList(&lp);
         }
      }
      /*
      ** don't free referenced strings, they are in the pl list
      */
      XtFree((char*)strs);
   }
   else {
      qmonMessageShow(w, True, 
            "@{Please configure a project first !}");
   }         
   
   DEXIT;
}
