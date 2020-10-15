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
#include "qmon_project.h"
#include "qmon_globals.h"
#include "AskForItems.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_userprj.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi.h"


static Widget qmon_project = 0;
static Widget project_names = 0;
static Widget project_conf_list = 0;
static Widget project_ask_layout = 0;
static Widget project_name_w = 0;
static Widget project_acl_w = 0;
static Widget project_xacl_w = 0;
static int add_mode = 0;

/*-------------------------------------------------------------------------*/
static void qmonPopdownProjectConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateProjectConfig(Widget parent);
static void qmonProjectFillConf(Widget w, lListElem *ep);
static void qmonSelectProject(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonProjectResetAsk(void);
static void qmonProjectSetAsk(lListElem *prjp);
static Widget qmonCreateProjectAsk(Widget parent);
static Boolean qmonProjectGetAsk(lListElem *prjp);
static void qmonProjectAskForUsers(Widget w, XtPointer cld, XtPointer cad);

/*-------------------------------------------------------------------------*/
void qmonPopupProjectConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;

   DENTER(GUI_LAYER, "qmonPopupProjectConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_project) {
      shell = XmtGetTopLevelShell(w);
      qmon_project = qmonCreateProjectConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownProjectConfig, NULL);
      /*
      ** create ask layout
      */
      project_ask_layout = qmonCreateProjectAsk(qmon_project);

   } 
   XSync(XtDisplay(qmon_project), 0);
   XmUpdateDisplay(qmon_project);

   qmonTimerAddUpdateProc(PROJECT_T, "updateProjectList", updateProjectList);
   qmonStartTimer(PROJECT_T | USERSET_T);
   updateProjectList();
   XmListSelectPos(project_names, 1, True);

   XtManageChild(qmon_project);
   XRaiseWindow(XtDisplay(XtParent(qmon_project)), 
                  XtWindow(XtParent(qmon_project)));

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateProjectList(void)
{
   lList *cl;
   
   DENTER(GUI_LAYER, "updateProjectList");

   cl = qmonMirrorList(SGE_PR_LIST);
   lPSortList(cl, "%I+", PR_name);
   UpdateXmListFromCull(project_names, XmFONTLIST_DEFAULT_TAG, cl, PR_name);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownProjectConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownProjectConfig");

   XtUnmanageChild(qmon_project);
   qmonStopTimer(PROJECT_T | USERSET_T);
   qmonTimerRmUpdateProc(PROJECT_T, "updateProjectList");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectFillConf(
Widget w,
lListElem *ep 
) {
   lList *ul;
   lListElem *uep;
   XmString *items;
   Cardinal itemCount; 
   char buf[BUFSIZ];
   int i;

   DENTER(GUI_LAYER, "qmonProjectFillConf");
   
   if (!ep) {
      /*
      ** clear the project_conf_list
      */
      XtVaSetValues( project_conf_list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
      DEXIT;
      return;
   }
   
   itemCount = 5;
   items = (XmString*) XtMalloc(sizeof(XmString)*itemCount); 

   i = 0;

   /* Project name */
   sprintf(buf, "%-20.20s %s", "Project Name", lGetString(ep, PR_name));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* Override Tickets */
   sprintf(buf, "%-20.20s %d", "Override Tickets", 
                        (int)lGetUlong(ep, PR_oticket));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* fshare */
   sprintf(buf, "%-20.20s %d", "Functional Share", 
                        (int)lGetUlong(ep, PR_fshare));
   items[i++] = XmStringCreateLtoR(buf, "LIST");

   /* users list */
   ul = lGetList(ep, PR_acl);
   sprintf(buf, "%-20.20s", "Users");
   for_each(uep, ul) {
      strcat(buf, " "); 
      strcat(buf, lGetString(uep, US_name));
   }
   if (!lGetNumberOfElem(ul))
      strcat(buf, " NONE");
   items[i++] = XmStringCreateLtoR(buf, "LIST");
   
   /* xusers list */
   ul = lGetList(ep, PR_xacl);
   sprintf(buf, "%-20.20s", "Xusers");
   for_each(uep, ul) {
      strcat(buf, " "); 
      strcat(buf, lGetString(uep, US_name));
   }
   if (!lGetNumberOfElem(ul))
      strcat(buf, " NONE");
   items[i++] = XmStringCreateLtoR(buf, "LIST");
   
   XtVaSetValues( project_conf_list, 
                  XmNitems, items,
                  XmNitemCount, itemCount,
                  NULL);
   XmStringTableFree(items, itemCount);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectProject(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *prjname;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonSelectProject");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &prjname)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ep = prj_list_locate(qmonMirrorList(SGE_PR_LIST), prjname);

   XtFree((char*) prjname);

   qmonProjectFillConf(project_conf_list, ep);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreateProjectConfig(
Widget parent 
) {
   Widget project_layout, project_add, project_delete, project_modify, 
            project_done, project_main_link;

   DENTER(GUI_LAYER, "qmonCreateProjectConfig");
   
   project_layout = XmtBuildQueryDialog( parent, "qmon_project",
                           NULL, 0,
                           "project_names", &project_names,
                           "project_conf_list", &project_conf_list,
                           "project_add", &project_add,
                           "project_delete", &project_delete,
                           "project_done", &project_done,
                           "project_modify", &project_modify,
                           "project_main_link", &project_main_link,
                           NULL);

   XtAddCallback(project_names, XmNbrowseSelectionCallback, 
                     qmonSelectProject, NULL);
   XtAddCallback(project_done, XmNactivateCallback, 
                     qmonPopdownProjectConfig, NULL);
   XtAddCallback(project_add, XmNactivateCallback, 
                     qmonProjectAdd, NULL); 
   XtAddCallback(project_modify, XmNactivateCallback, 
                     qmonProjectModify, NULL); 
   XtAddCallback(project_delete, XmNactivateCallback, 
                     qmonProjectDelete, NULL); 
   XtAddCallback(project_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL); 

   XtAddEventHandler(XtParent(project_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(project_layout), StructureNotifyMask, False,
                        SetMaxShellSize, NULL);

   DEXIT;
   return project_layout;
}



/*-------------------------------------------------------------------------*/
static Widget qmonCreateProjectAsk(
Widget parent 
) {
   Widget project_ok, project_cancel, project_usersPB, project_xusersPB; 

   DENTER(GUI_LAYER, "qmonCreateProjectAsk");
   
   project_ask_layout = XmtBuildQueryDialog( parent, "project_ask_shell",
                           NULL, 0,
                           "project_ok", &project_ok,
                           "project_cancel", &project_cancel,
                           "project_usersPB", &project_usersPB,
                           "project_xusersPB", &project_xusersPB,
                           "project_name", &project_name_w,
                           "project_users", &project_acl_w, 
                           "project_xusers", &project_xacl_w,
                           NULL);

   XtAddCallback(project_ok, XmNactivateCallback, 
                     qmonProjectOk, NULL);
   XtAddCallback(project_cancel, XmNactivateCallback, 
                     qmonProjectCancel, NULL);
   XtAddCallback(project_usersPB, XmNactivateCallback, 
                     qmonProjectAskForUsers, (XtPointer)project_acl_w);
   XtAddCallback(project_xusersPB, XmNactivateCallback, 
                     qmonProjectAskForUsers, (XtPointer)project_xacl_w);

   XtAddEventHandler(XtParent(project_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return project_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectAskForUsers(Widget w, XtPointer cld, XtPointer cad)
{
   lList *ql_out = NULL;
   lList *ql_in = NULL;
   int status;
   Widget list = (Widget) cld;

   DENTER(GUI_LAYER, "qmonProjectAskForUsers");
   
   ql_in = qmonMirrorList(SGE_US_LIST);
   lPSortList(ql_in, "%I+", US_name);
   ql_out = XmStringToCull(list, US_Type, US_name, ALL_ITEMS);

   status = XmtAskForItems(w, NULL, NULL, "@{Select Access Lists}", 
                  ql_in, US_name,
                  "@{@fBAvailable Access Lists}", &ql_out, US_Type, US_name, 
                  "@{@fBChosen Access Lists}", NULL);

   if (status) {
      lPSortList(ql_out, "%I+", US_name);
      UpdateXmListFromCull(list, XmFONTLIST_DEFAULT_TAG, ql_out,
                              US_name);
   }
   lFreeList(&ql_out);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectAdd(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonProjectAdd");

   qmonProjectResetAsk();
   XtVaSetValues( project_name_w,
                  XmNeditable, True,
                  NULL);
   add_mode = 1;
   XtManageChild(project_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectModify(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *prjnames;
   Cardinal prjnum;
   String prjstr;
   lListElem *prjp = NULL;

   DENTER(GUI_LAYER, "qmonProjectModify");

   /*
   ** on opening the dialog fill in the old values
   */
   XtVaGetValues( project_names,
                  XmNselectedItems, &prjnames,
                  XmNselectedItemCount, &prjnum,
                  NULL);
   
   if (prjnum == 1 && 
         XmStringGetLtoR(prjnames[0], XmFONTLIST_DEFAULT_TAG, &prjstr)) {
      XmtInputFieldSetString(project_name_w, prjstr);
      XtVaSetValues( project_name_w,
                     XmNeditable, False,
                     NULL);
      prjp = prj_list_locate(qmonMirrorList(SGE_PR_LIST), prjstr);
      XtFree((char*)prjstr);
      if (prjp) {
         add_mode = 0;
         qmonProjectSetAsk(prjp);
         XtManageChild(project_ask_layout);
      }
      XtManageChild(project_ask_layout);
   }
   else {
      if (prjnum > 1)
         qmonMessageShow(w, True, "@{Select only one project !}");
      else
         qmonMessageShow(w, True, "@{To modify a project select this project !}");
   }


   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonProjectOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *prjl = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   Boolean status = False;
   XmString xprjname = NULL;
   StringConst prjname = NULL;
   lListElem *new_prep = NULL;
   lListElem *prep = NULL;

   DENTER(GUI_LAYER, "qmonProjectOk");
   /*
   ** get the contents of the dialog fields here,
   ** build the cull list and send gdi request
   ** depending on success of gdi request close the dialog or stay open
   */
   prjl = lCreateList("PR_ADD", PR_Type);
   prep = lCreateElem(PR_Type);
   
   if (prjl) {
      if (qmonProjectGetAsk(prep)) {
         prjname = lGetString(prep, PR_name);
         /*
         ** gdi call 
         */
         what = lWhat("%T(ALL)", PR_Type);
         
         if (add_mode) {
            lAppendElem(prjl, lCopyElem(prep));
            alp = qmonAddList(SGE_PR_LIST, 
                              qmonMirrorListRef(SGE_PR_LIST),
                              PR_name, &prjl, NULL, what);
         }
         else {
            /*
            ** find old element and change this one because we have to keep
            ** the information inside it; due to a weakness of the
            ** current GDI a selective modify is impossible
            */
            qmonMirrorMultiAnswer(PROJECT_T, &alp);
            if (alp) {
               qmonMessageBox(w, alp, 0);
               lFreeList(&alp);
               DEXIT;
               return;
            }
               
            new_prep = lCopyElem(lGetElemStr(qmonMirrorList(SGE_PR_LIST), 
                                    PR_name, prjname));
            lSwapList(new_prep, PR_acl, prep, PR_acl); 
            lSwapList(new_prep, PR_xacl, prep, PR_xacl); 
            lAppendElem(prjl, new_prep);
            alp = qmonModList(SGE_PR_LIST, 
                              qmonMirrorListRef(SGE_PR_LIST),
                              PR_name, &prjl, NULL, what);
         }

         if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK)
            status = True;

         qmonMessageBox(w, alp, 0);

         if (status) {
            XtUnmanageChild(project_ask_layout);
            updateProjectList();
            if (add_mode) {   
               XmListSelectPos(project_names, 0, True);
            }
            else {
               xprjname = XmtCreateXmString(prjname);
               XmListSelectItem(project_names, xprjname, True);
               XmStringFree(xprjname);
            }
         }
         lFreeWhat(&what);
         lFreeList(&prjl);
         lFreeElem(&prep);
         lFreeList(&alp);
      }
   }


   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonProjectCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonProjectCancel");

   XtUnmanageChild(project_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectDelete(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   Cardinal itemCount = 0;
   Boolean status, answer;

   DENTER(GUI_LAYER, "qmonProjectDelete");
    
   lp = XmStringToCull(project_names, PR_Type, PR_name, SELECTED_ITEMS); 

   if (lp) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{project.askdel.Do you really want to delete the\nselected Project Configuration ?}", 
                     "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                     False, &answer, NULL);
      if (answer) { 
         what = lWhat("%T(ALL)", PR_Type);
         alp = qmonDelList(SGE_PR_LIST, 
                           qmonMirrorListRef(SGE_PR_LIST),
                           PR_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);
         lFreeWhat(&what);
         lFreeList(&alp);

         updateProjectList();
         XtVaGetValues( project_names,
                        XmNitemCount, &itemCount,
                        NULL);
         if (itemCount)
            XmListSelectPos(project_names, 1, True);
         else
            qmonProjectFillConf(project_names, NULL);

      }
      lFreeList(&lp);
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectSetAsk(
lListElem *prjp 
) {
   StringConst prj_name = NULL;
   lList *acl = NULL;
   lList *xacl = NULL;

   DENTER(GUI_LAYER, "qmonProjectSetAsk");

   if (!prjp) {
      DEXIT;
      return;
   }

   prj_name = lGetString(prjp, PR_name);
   if (prj_name)
      XmtInputFieldSetString(project_name_w, prj_name);

   /*
   ** the lists have to be converted to XmString
   */
   acl = lGetList(prjp, PR_acl);
   UpdateXmListFromCull(project_acl_w, XmFONTLIST_DEFAULT_TAG, acl, US_name);
    
   xacl = lGetList(prjp, PR_xacl);
   UpdateXmListFromCull(project_xacl_w, XmFONTLIST_DEFAULT_TAG, xacl, US_name);
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonProjectResetAsk(void)
{

   DENTER(GUI_LAYER, "qmonProjectResetAsk");

   XmtInputFieldSetString(project_name_w, "");

   /*
   ** the lists have to be converted to XmString
   */
   UpdateXmListFromCull(project_acl_w, XmFONTLIST_DEFAULT_TAG, NULL, US_name);
    
   UpdateXmListFromCull(project_xacl_w, XmFONTLIST_DEFAULT_TAG, NULL, US_name);
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonProjectGetAsk(
lListElem *prjp 
) {
   String prj_name = NULL;
   lList *acl = NULL;
   lList *xacl = NULL;

   DENTER(GUI_LAYER, "qmonProjectGetAsk");

   if (!prjp) {
      DEXIT;
      return False;
   }

   prj_name = XmtInputFieldGetString(project_name_w);
   if (!prj_name || prj_name[0] == '\0') {
      qmonMessageShow(project_ask_layout, True, "@{Project name required !}");
      DEXIT;
      return False;
   }
   lSetString(prjp, PR_name, prj_name);
  
   /*
   ** XmString entries --> Cull
   */
   acl = XmStringToCull(project_acl_w, US_Type, US_name, ALL_ITEMS);
   lSetList(prjp, PR_acl, acl);

   xacl = XmStringToCull(project_xacl_w, US_Type, US_name, ALL_ITEMS);
   lSetList(prjp, PR_xacl, xacl);
   
   DEXIT;
   return True;
}
