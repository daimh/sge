/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of 
 *  the Sun Industry Standards Source License Version 1.2
 * 
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
#include <stdlib.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

#include <Xmt/Xmt.h>
#include <Xmt/Help.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>
#include <Xmt/Procedures.h>

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_host.h"
#include "qmon_comm.h"
#include "qmon_globals.h"
#include "qmon_timer.h"
#include "qmon_widgets.h"
#include "qmon_quarks.h"
#include "qmon_message.h"
#include "qmon_load.h"
#include "qmon_cplx.h"
#include "qmon_manop.h"
#include "qmon_project.h"
#include "qmon_queue.h"
#include "Matrix.h"
#include "Tab.h"

#include "uti/sge_prog.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_usage.h"

#include "comm/commlib.h"

#include "sched/sge_complex_schedd.h"
#include "sched/load_correction.h"

#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi_ctx.h"

#include "sge.h"
#include "sge_hgroup_qconf.h"

extern sge_gdi_ctx_class_t *ctx;

/*-------------------------------------------------------------------------*/
typedef struct _tHostEntry {
   lList *consumable_config_list;
   lList *scaling_list;
   lList *usage_scaling_list;
   char *name;
   lList *acl;
   lList *xacl;
   lList *prj;
   lList *xprj;
   lList *reporting_variables;
} tHostEntry;

XtResource host_resources[] = {

   { "name", "name", XtRString,
      sizeof(char*), XtOffsetOf(tHostEntry, name),
      XtRImmediate, NULL },
   
   { "scaling_list", "scaling_list", QmonRHS_Type,
      sizeof(lList*), XtOffsetOf(tHostEntry, scaling_list),
      XtRImmediate, NULL },

   { "usage_scaling_list", "usage_scaling_list", QmonRHS_Type,
      sizeof(lList*), XtOffsetOf(tHostEntry, usage_scaling_list),
      XtRImmediate, NULL },

/*---- consumables  ----*/
   { "consumable_config_list", "consumable_config_list", QmonRCE2_Type,
      sizeof(lList*), XtOffsetOf(tHostEntry, consumable_config_list),
      XtRImmediate, NULL },

/*---- access  ----*/
   { "acl", "acl", QmonRUS_Type,
      sizeof(lList *),
      XtOffsetOf(tHostEntry, acl),
      XtRImmediate, NULL },
   
   { "xacl", "xacl", QmonRUS_Type,
      sizeof(lList *),
      XtOffsetOf(tHostEntry, xacl),
      XtRImmediate, NULL },
      
/*---- projects  ----*/
   { "prj", "prj", QmonRPR_Type,
      sizeof(lList *),
      XtOffsetOf(tHostEntry, prj),
      XtRImmediate, NULL },
   
   { "xprj", "xprj", QmonRPR_Type,
      sizeof(lList *),
      XtOffsetOf(tHostEntry, xprj),
      XtRImmediate, NULL },
      
/*---- reporting variables  ----*/
   { "reporting_variables", "reporting_variables", QmonRSTU_Type,
      sizeof(lList *),
      XtOffsetOf(tHostEntry, reporting_variables),
      XtRImmediate, NULL }
   
};


/*-------------------------------------------------------------------------*/
static Widget qmonCreateHostConfig(Widget parent);
static Widget qmonCreateExecHostConfig(Widget parent, XtPointer cld);
static Widget qmonCreateExecHostAsk(Widget parent);
static void qmonHostFillList(void);
static void qmonHostFolderChange(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostHelp(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostDelete(Widget w, XtPointer cld, XtPointer cad);
/*
static Widget qmonCreateAdminHostConfig(Widget parent, XtPointer cld);
static Widget qmonCreateSubmitHostConfig(Widget parent, XtPointer cld);
*/
static void qmonExecHostShutdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostChange(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostSelect(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostCheckName(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostCheckScaling(Widget w, XtPointer cld, XtPointer cad);
static void qmonExecHostSetAsk(StringConst name);
static lList* qmonExecHostGetAsk(void);
static void qmonHostAvailableAcls(void);
static void qmonHostAvailableProjects(void);
static void qmonHostAvailableReportVars(void);

static void qmonExecHostAccessToggle(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostAccessAdd(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostAccessRemove(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostProjectToggle(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostProjectAdd(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostProjectRemove(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostReportVarAdd(Widget w, XtPointer cld, XtPointer cad); 
static void qmonExecHostReportVarRemove(Widget w, XtPointer cld, XtPointer cad); 
static void qmonPopdownHostConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateHostgroupAsk(Widget parent);
static void qmonHostgroupChange(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupCheckName(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupSetAsk(StringConst name);
static void qmonHostgroupSelect(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupAddHost(Widget w, XtPointer cld, XtPointer cad);
static void qmonHostgroupAddHostgroups(Widget w, XtPointer cld, XtPointer cad);
static void updateHostListCB(Widget w, XtPointer cld, XtPointer cad);
static void updateHostList(void);
/*-------------------------------------------------------------------------*/
static Widget qmon_host = NULL;
static Widget adminhost_list = NULL;
static Widget submithost_list = NULL;
static Widget hostgroup_list = NULL;
static Widget hostgroup_memberlist = NULL;
static Widget host_modify = NULL;
static Widget host_shutdown = NULL;

static Widget exechost_list = NULL;
static Widget exechost_access = 0;
static Widget exechost_reporting_variables = 0;
static Widget exechost_load_scaling = 0;
static Widget exechost_consumables = 0;
static Widget exechost_usage_scaling = 0;
static Widget eh_ask_layout = 0;
static Widget hg_ask_layout = 0;
static Widget hg_name_w = 0;
static Widget hg_memberlist_w = 0;
static Widget hg_hglist_w = 0;
static Widget hg_member_w = 0;
static Widget eh_name_w = 0;
static Widget eh_folder = 0;
static Widget access_list = 0;
static Widget access_allow = 0;
static Widget access_deny = 0;
static Widget access_toggle = 0;
static Widget project_list = 0;
static Widget project_allow = 0;
static Widget project_deny = 0;
static Widget project_toggle = 0;
static Widget reporting_variables_list = 0;
static Widget reporting_variables_chosen = 0;

static tHostEntry host_data = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int add_mode = 0;
static int dialog_mode = SGE_AH_LIST;



/*-------------------------------------------------------------------------*/
void qmonPopupHostConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonPopupHostConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_host) {
      shell = XmtGetTopLevelShell(w);
      qmon_host = qmonCreateHostConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownHostConfig, NULL);
      XtAddEventHandler(XtParent(qmon_host), StructureNotifyMask, False,
                                SetMinShellSize, NULL);
   }

   qmonMirrorMultiAnswer(ADMINHOST_T | SUBMITHOST_T | EXECHOST_T | CENTRY_T |
                         USERSET_T | PROJECT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   
   qmonTimerAddUpdateProc(ADMINHOST_T, "updateHostList", updateHostList);
   qmonStartTimer(ADMINHOST_T | SUBMITHOST_T | EXECHOST_T |
                  USERSET_T | PROJECT_T);
   qmonHostFillList();

   xmui_manage(qmon_host);

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownHostConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownHostConfig");

   qmonStopTimer(ADMINHOST_T | SUBMITHOST_T | EXECHOST_T |
                 USERSET_T | PROJECT_T);
   qmonTimerRmUpdateProc(ADMINHOST_T, "updateHostList");
   xmui_unmanage(qmon_host);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void updateHostListCB(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   DENTER(GUI_LAYER, "updateHostListCB");

   /*
   ** fetch changed lists
   */
   qmonMirrorMultiAnswer(ADMINHOST_T | SUBMITHOST_T | EXECHOST_T | HGROUP_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   updateHostList();
}

/*-------------------------------------------------------------------------*/
static void updateHostList(void)
{
   DENTER(GUI_LAYER, "updateHostList");

   if (qmon_host && XtIsManaged(qmon_host))
      qmonHostFillList();

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static Widget qmonCreateHostConfig(
Widget parent 
) {
   Widget host_layout, host_add, host_delete, host_done, host_help,
          host_main_link, host_folder, adminhost_hostname, submithost_hostname,
          exechost_layout;

   DENTER(GUI_LAYER, "qmonCreateHostConfig");
   
   host_layout = XmtBuildQueryDialog( parent, "qmon_host",
                           NULL, 0,
                           "host_folder", &host_folder,
                           "host_add", &host_add,
                           "host_modify", &host_modify,
                           "host_delete", &host_delete,
                           "host_shutdown", &host_shutdown,
                           "host_done", &host_done,
                           "host_help", &host_help,
                           "host_main_link", &host_main_link,
                           "adminhost_list", &adminhost_list,
                           "submithost_list", &submithost_list,
                           "hostgroup_list", &hostgroup_list,
                           "hostgroup_memberlist", &hostgroup_memberlist,
                           "adminhost_hostname", &adminhost_hostname,
                           "submithost_hostname", &submithost_hostname,
                           NULL);
   
   hg_ask_layout = qmonCreateHostgroupAsk(host_folder);
   
   exechost_layout = qmonCreateExecHostConfig(host_folder, NULL);
   XtManageChild(exechost_layout);

                               
   XtAddCallback(host_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(host_folder, XmNvalueChangedCallback, 
                     qmonHostFolderChange, NULL);
   XtAddCallback(host_done, XmNactivateCallback, 
                     qmonPopdownHostConfig, NULL);
   XtAddCallback(host_add, XmNactivateCallback, 
                     qmonHostAdd, (XtPointer) 1); 
   XtAddCallback(host_modify, XmNactivateCallback, 
                     qmonHostAdd, NULL); 
   XtAddCallback(host_delete, XmNactivateCallback, 
                     qmonHostDelete, NULL); 
   XtAddCallback(host_shutdown, XmNactivateCallback, 
                     qmonExecHostShutdown, NULL);
   XtAddCallback(adminhost_hostname, XmtNinputCallback, 
                     qmonHostAdd, NULL);
   XtAddCallback(submithost_hostname, XmtNinputCallback, 
                     qmonHostAdd, NULL);
   XtAddCallback(host_help, XmNactivateCallback, 
                     qmonHostHelp, NULL);
   XtAddCallback(hostgroup_list, XmNbrowseSelectionCallback,
                     qmonHostgroupSelect, NULL);

   XtAddEventHandler(XtParent(host_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return host_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonHostFillList(void)
{
   lList *lp;
   static lCondition *where = NULL;
   static lEnumeration *what = NULL;
   
   DENTER(GUI_LAYER, "qmonHostFillList");


   /* admin host list */
   lp = qmonMirrorList(SGE_AH_LIST);
   lPSortList(lp, "%I+", AH_name);
   UpdateXmListFromCull(adminhost_list, XmFONTLIST_DEFAULT_TAG, lp, AH_name);

   /* submit host list */
   lp = qmonMirrorList(SGE_SH_LIST);
   lPSortList(lp, "%I+", SH_name);
   UpdateXmListFromCull(submithost_list, XmFONTLIST_DEFAULT_TAG, lp, SH_name);

   /* exec host is a special case needs additional configuration */
   if (!where)
      where = lWhere("%T(%I!=%s)", EH_Type, EH_name, SGE_TEMPLATE_NAME);
   if (!what)
      what = lWhat("%T(ALL)", EH_Type);
   
   lp = lSelect("EHL", qmonMirrorList(SGE_EH_LIST), where, what);
   lPSortList(lp, "%I+", EH_name);
   UpdateXmListFromCull(exechost_list, XmFONTLIST_DEFAULT_TAG, lp, EH_name);
   XmListMoveItemToPos(exechost_list, "global", 1);
   lFreeList(&lp);
   XmListSelectPos(exechost_list, 1, True);

   /* host groups */
   lp = qmonMirrorList(SGE_HGRP_LIST);
   lPSortList(lp, "%I+", HGRP_name);
   UpdateXmListFromCull(hostgroup_list, XmFONTLIST_DEFAULT_TAG, lp, HGRP_name);
   XmListSelectPos(hostgroup_list, 1, True);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostAvailableReportVars(void)
{
   lList *lp;
   
   DENTER(GUI_LAYER, "qmonHostAvailableReportVars");

   lp = qmonMirrorList(SGE_CE_LIST);
   lPSortList(lp, "%I+", CE_name);
   UpdateXmListFromCull(reporting_variables_list, XmFONTLIST_DEFAULT_TAG, lp, CE_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostAvailableAcls(void)
{
   lList *lp;
   
   DENTER(GUI_LAYER, "qmonHostAvailableAcls");

   lp = qmonMirrorList(SGE_US_LIST);
   lPSortList(lp, "%I+", US_name);
   UpdateXmListFromCull(access_list, XmFONTLIST_DEFAULT_TAG, lp, US_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostAvailableProjects(void)
{
   lList *lp;
   
   DENTER(GUI_LAYER, "qmonHostAvailableProjects");

   lp = qmonMirrorList(SGE_PR_LIST);
   lPSortList(lp, "%I+", PR_name);
   UpdateXmListFromCull(project_list, XmFONTLIST_DEFAULT_TAG, lp, PR_name);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* Exec Host Stuff                                                         */
/*-------------------------------------------------------------------------*/
static Widget qmonCreateExecHostConfig(
Widget parent,
XtPointer cld 
) {
   Widget exechost_layout, exechost_add, exechost_shutdown, 
          exechost_modify, exechost_delete, exechost_done;

   DENTER(GUI_LAYER, "qmonCreateExecHostConfig");
   
   exechost_layout = XmtCreateLayout(parent, "exechost_layout", NULL, 0);
   XmtDialogBindResourceList(exechost_layout, host_resources, 
                              XtNumber(host_resources));
   XmtCreateQueryChildren( exechost_layout,
                           "exechost_list", &exechost_list,
                           "exechost_load_scaling", &exechost_load_scaling,
                           "exechost_usage_scaling", &exechost_usage_scaling,
                           "exechost_add", &exechost_add,
                           "exechost_modify", &exechost_modify,
                           "exechost_delete", &exechost_delete,
                           "exechost_done", &exechost_done,
                           "exechost_shutdown", &exechost_shutdown,
                           "exechost_consumables", &exechost_consumables,
                           "exechost_access", &exechost_access,
                           "exechost_reporting_variables", 
                                    &exechost_reporting_variables,
                           NULL);

   XtAddCallback(exechost_list, XmNbrowseSelectionCallback,
                     qmonExecHostSelect, NULL);

#if 0
   XtAddCallback(exechost_done, XmNactivateCallback, 
                     qmonPopdownHostConfig, NULL);
   XtAddCallback(exechost_add, XmNactivateCallback, 
                     qmonExecHostChange, (XtPointer) 1);
   XtAddCallback(exechost_modify, XmNactivateCallback, 
                     qmonExecHostChange, (XtPointer) 0);
   XtAddCallback(exechost_delete, XmNactivateCallback, 
                     qmonHostDelete, (XtPointer) SGE_EH_LIST);
   XtAddCallback(exechost_shutdown, XmNactivateCallback, 
                     qmonExecHostShutdown, NULL);
#endif

   /*
   ** create exec host ask dialog for adding/modifying exec hosts
   */
   eh_ask_layout = qmonCreateExecHostAsk(exechost_layout);
  
   DEXIT;
   return exechost_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostSelect(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *ehname;
   lListElem *ehp;
   lListElem *ep;
   lList *lsl;
   lList *usl;
   static char buf[10*BUFSIZ];
   XmTextPosition pos;
   
   DENTER(GUI_LAYER, "qmonExecHostSelect");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &ehname)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ehp = host_list_locate(qmonMirrorList(SGE_EH_LIST), ehname);
   XtFree((char*) ehname);

   if (ehp) {
      /*
      ** fill the load scaling and usage scaling list
      */
      lsl = lGetList(ehp, EH_scaling_list);

      XmTextDisableRedisplay(exechost_load_scaling);
      pos = 0;
      XmTextSetString(exechost_load_scaling, "");
      for_each(ep, lsl) {
         sprintf(buf, "%-15.15s   %3.2f\n", lGetString(ep, HS_name),
                  lGetDouble(ep, HS_value));
         XmTextInsert(exechost_load_scaling, pos, buf);
         pos += strlen(buf);
      }
      XmTextEnableRedisplay(exechost_load_scaling);
    
      /*
      ** fill the consumable list into the textfield
      */
      lsl = lGetList(ehp, EH_consumable_config_list);
      XmTextDisableRedisplay(exechost_consumables);
      pos = 0;
      XmTextSetString(exechost_consumables, "");
      for_each(ep, lsl) {
         sprintf(buf, "%-15.15s   %-15.15s\n", lGetString(ep, CE_name),
                  lGetString(ep, CE_stringval));
         XmTextInsert(exechost_consumables, pos, buf);
         pos += strlen(buf);
      }
      XmTextEnableRedisplay(exechost_consumables);
    
      /*
      ** reporting variables
      */
      lsl = lGetList(ehp, EH_report_variables);
      XmTextDisableRedisplay(exechost_reporting_variables);
      pos = 0;
      XmTextSetString(exechost_reporting_variables, "");
      for_each(ep, lsl) {
         sprintf(buf, "%s\n", lGetString(ep, STU_name));
         XmTextInsert(exechost_reporting_variables, pos, buf);
         pos += strlen(buf);
      }
      XmTextEnableRedisplay(exechost_reporting_variables);

      /*
      ** fill the access list into the textfield
      */
      XmTextDisableRedisplay(exechost_access);
      pos = 0;
      XmTextSetString(exechost_access, "");

      /*
      ** acl
      */
      lsl = lGetList(ehp, EH_acl);
      sprintf(buf, "%-15.15s", "Access");
      for_each(ep, lsl) {
         strcat(buf, " ");
         strcat(buf, lGetString(ep, US_name));
      }
      if (!lGetNumberOfElem(lsl))
         strcat(buf, " NONE");
      strcat(buf, "\n");
      XmTextInsert(exechost_access, pos, buf);
      pos += strlen(buf);

      /*
      ** xacl
      */
      lsl = lGetList(ehp, EH_xacl);
      sprintf(buf, "%-15.15s", "XAccess");
      for_each(ep, lsl) {
         strcat(buf, " ");
         strcat(buf, lGetString(ep, US_name));
      }
      if (!lGetNumberOfElem(lsl))
         strcat(buf, " NONE");
      strcat(buf, "\n");
      XmTextInsert(exechost_access, pos, buf);
      pos += strlen(buf);

      /*
      ** projects
      */
      lsl = lGetList(ehp, EH_prj);
      sprintf(buf, "%-15.15s", "Projects");
      for_each(ep, lsl) {
         strcat(buf, " ");
         strcat(buf, lGetString(ep, PR_name));
      }
      if (!lGetNumberOfElem(lsl))
         strcat(buf, " NONE");
      strcat(buf, "\n");
      XmTextInsert(exechost_access, pos, buf);
      pos += strlen(buf);

      /*
      ** xprojects
      */
      lsl = lGetList(ehp, EH_xprj);
      sprintf(buf, "%-15.15s", "XProjects");
      for_each(ep, lsl) {
         strcat(buf, " ");
         strcat(buf, lGetString(ep, PR_name));
      }
      if (!lGetNumberOfElem(lsl))
         strcat(buf, " NONE");
      strcat(buf, "\n");
      XmTextInsert(exechost_access, pos, buf);
      pos += strlen(buf);

      XmTextEnableRedisplay(exechost_access);
    
      usl = lGetList(ehp, EH_usage_scaling_list);
      XmTextDisableRedisplay(exechost_usage_scaling);
      pos = 0;
      XmTextSetString(exechost_usage_scaling, "");
      for_each(ep, usl) {
         sprintf(buf, "%-15.15s   %3.2f\n", lGetString(ep, HS_name),
                  lGetDouble(ep, HS_value));
         XmTextInsert(exechost_usage_scaling, pos, buf);
         pos += strlen(buf);
      }
      XmTextEnableRedisplay(exechost_usage_scaling);

   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Widget qmonCreateExecHostAsk(
Widget parent 
) {
   Widget eh_ok, eh_cancel, eh_load_scaling, eh_usage_scaling, 
          complexes_ccl;
   Widget access_add, access_remove, access_dialog;
   Widget reporting_variables_add, reporting_variables_remove;
   Widget project_add, project_remove, project_dialog, eh_project;

   DENTER(GUI_LAYER, "qmonCreateExecHostAsk");
   
   eh_ask_layout = XmtBuildQueryDialog( parent, "eh_ask_shell",
                           host_resources, XtNumber(host_resources),
                           "eh_ok", &eh_ok,
                           "eh_folder", &eh_folder,
                           "eh_cancel", &eh_cancel,
                           "eh_name", &eh_name_w,
                           "eh_load_scaling", &eh_load_scaling,
                           "eh_usage_scaling", &eh_usage_scaling,
                           "complexes_ccl", &complexes_ccl,
                           /* access_config */
                           "access_list", &access_list,
                           "access_allow", &access_allow,
                           "access_deny", &access_deny,
                           "access_add", &access_add,
                           "access_remove", &access_remove,
                           "access_toggle", &access_toggle,
                           "access_dialog", &access_dialog,
                           /* project_config */
                           "project_list", &project_list,
                           "project_allow", &project_allow,
                           "project_deny", &project_deny,
                           "project_add", &project_add,
                           "project_remove", &project_remove,
                           "project_toggle", &project_toggle,
                           "project_dialog", &project_dialog,
                           "eh_project", &eh_project,
                           "reporting_variables_list", &reporting_variables_list,
                           "reporting_variables_chosen", &reporting_variables_chosen,
                           "reporting_variables_add", &reporting_variables_add,
                           "reporting_variables_remove", &reporting_variables_remove,
                           NULL);

   XtAddCallback(eh_ok, XmNactivateCallback, 
                     qmonExecHostOk, NULL);
   XtAddCallback(eh_cancel, XmNactivateCallback, 
                     qmonExecHostCancel, NULL);
   XtAddCallback(eh_name_w, XmtNverifyCallback, 
                     qmonExecHostCheckName, NULL);
   XtAddCallback(eh_load_scaling, XmNleaveCellCallback, 
                     qmonExecHostCheckScaling, NULL);
   XtAddCallback(eh_usage_scaling, XmNleaveCellCallback, 
                     qmonExecHostCheckScaling, NULL);

   XtAddCallback(complexes_ccl, XmNselectCellCallback,
                  qmonLoadSelectEntry, NULL);
   XtAddCallback(complexes_ccl, XmNlabelActivateCallback,
                  qmonLoadNames, (XtPointer) 1);

   XtAddCallback(access_toggle, XmtNvalueChangedCallback, 
                     qmonExecHostAccessToggle, NULL);
   XtAddCallback(access_add, XmNactivateCallback, 
                     qmonExecHostAccessAdd, NULL);
   XtAddCallback(access_remove, XmNactivateCallback, 
                     qmonExecHostAccessRemove, NULL);
   XtAddCallback(access_dialog, XmNactivateCallback, 
                     qmonPopupManopConfig, (XtPointer)2);

   XtAddCallback(reporting_variables_add, XmNactivateCallback, 
                     qmonExecHostReportVarAdd, NULL);
   XtAddCallback(reporting_variables_remove, XmNactivateCallback, 
                     qmonExecHostReportVarRemove, NULL);

   /*
   ** Project & Xproject
   */
   XtAddCallback(project_toggle, XmtNvalueChangedCallback, 
                     qmonExecHostProjectToggle, NULL);
   XtAddCallback(project_add, XmNactivateCallback, 
                     qmonExecHostProjectAdd, NULL);
   XtAddCallback(project_remove, XmNactivateCallback, 
                     qmonExecHostProjectRemove, NULL);
   XtAddCallback(project_dialog, XmNactivateCallback, 
                     qmonPopupProjectConfig, NULL);

   XtAddEventHandler(XtParent(eh_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return eh_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *ehl = NULL;
   lList *alp;
   lEnumeration *what;
   Boolean status = False;
   XmString xehname = NULL;
   StringConst ehname = NULL;

   DENTER(GUI_LAYER, "qmonExecHostOk");

   /*
   ** get the list with one new host
   */
   ehl = qmonExecHostGetAsk();

   if (ehl) {
      ehname = lGetHost(lFirst(ehl), EH_name);
      /*
      ** gdi call 
      */
      what = lWhat("%T(ALL)", EH_Type);
      
      if (add_mode) {
         alp = qmonAddList(SGE_EH_LIST, qmonMirrorListRef(SGE_EH_LIST), EH_name, &ehl, NULL, what);
      }
      else {
         alp = qmonModList(SGE_EH_LIST, qmonMirrorListRef(SGE_EH_LIST), EH_name, &ehl, NULL, what);
      }

      if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK)
         status = True;

      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);

      if (status) {
         XtUnmanageChild(eh_ask_layout);
         updateHostList();
         /*
         ** select the modified or added Exec Host
         */
         xehname = XmtCreateXmString(ehname);
         XmListSelectItem(exechost_list, xehname, True);
         XmStringFree(xehname);
      } else {
         qmonExecHostSetAsk(ehname);
      }   

      lFreeWhat(&what);
      lFreeList(&ehl);
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonExecHostCancel");

   XtUnmanageChild(eh_ask_layout);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static lList* qmonExecHostGetAsk(void)
{
   lList *lp;
   static lCondition *where = NULL;
   
   DENTER(GUI_LAYER, "qmonExecHostGetAsk");

   lp = lCreateElemList("EH", EH_Type, 1);

   if (lp) {
      XmtDialogGetDialogValues(eh_ask_layout, &host_data);

      /*
      ** name of exec_host
      */
      lSetHost(lFirst(lp), EH_name, host_data.name);
      XtFree((char*)host_data.name);
      host_data.name = NULL;
      
      /*
      ** the cast to double is absolutely necessary, otherwise
      ** the varargs get expanded in a wrong way
      */
      if (!where) 
         where = lWhere("%T(%I != %g)", HS_Type, HS_value, (double)1.0);
      /*
      ** load scaling
      */
      host_data.scaling_list = lSelectDestroy(host_data.scaling_list,
                                                where);

      lSetList(lFirst(lp), EH_scaling_list, host_data.scaling_list);
      host_data.scaling_list = NULL;

      /*
      ** consumable_config_list 
      */
      lSetList(lFirst(lp), EH_consumable_config_list, 
                     host_data.consumable_config_list);
      host_data.consumable_config_list = NULL;

      /*
      ** (x)acl 
      */
      lSetList(lFirst(lp), EH_acl, host_data.acl);
      host_data.acl = NULL;
      lSetList(lFirst(lp), EH_xacl, host_data.xacl);
      host_data.xacl = NULL;
      /*
      ** reporting variables
      */
      lSetList(lFirst(lp), EH_report_variables, host_data.reporting_variables);
      host_data.reporting_variables = NULL;

      /*
      ** usage scaling
      */
      host_data.usage_scaling_list = 
                  lSelectDestroy(host_data.usage_scaling_list,
                                       where);
   
      lSetList(lFirst(lp), EH_usage_scaling_list, 
                        host_data.usage_scaling_list);
      host_data.usage_scaling_list = NULL;

      /*
      ** (x)project 
      */
      lSetList(lFirst(lp), EH_prj, host_data.prj);
      host_data.prj = NULL;
      lSetList(lFirst(lp), EH_xprj, host_data.xprj);
      host_data.xprj = NULL;
   }
               
   DEXIT;
   return lp;
}


/*-------------------------------------------------------------------------*/
static void qmonExecHostSetAsk(
StringConst name 
) {
   lListElem *ehp = NULL;
   lListElem *ep;
   lListElem *lsep;
   lListElem *usep;
   lList *lsl = NULL;
   lList *ehl = NULL;
   lList *cl = NULL;
   lList *ehsl = NULL;
   lList *ehul = NULL;
   lList *usl = NULL;
   lList *entries = NULL;
   static lCondition *where = NULL;
   
   DENTER(GUI_LAYER, "qmonExecHostSetAsk");

   cl = qmonMirrorList(SGE_CE_LIST);
   ehl = qmonMirrorList(SGE_EH_LIST);

   if (name) {
      /*
      ** get the selected host element
      */
      ehp = host_list_locate(ehl, name);
      if (host_data.name)
         XtFree((char*)host_data.name);
      host_data.name = XtNewString(name);
   } else {
      if (host_data.name)
         XtFree((char*)host_data.name);
      host_data.name = NULL;
   }   
      
   /* get complex entries and filter out scalable entries */
   entries = lCopyList("", cl);
   if (!where)
      where = lWhere("%T(%I == %u || %I == %u || %I == %u || %I == %u)", CE_Type,
                     CE_valtype, TYPE_INT, CE_valtype, TYPE_TIM, 
                     CE_valtype, TYPE_DOUBLE, CE_valtype, TYPE_MEM);
   entries = lSelectDestroy(entries, where);
   lPSortList(entries, "%I+", CE_name);
   for_each (ep, entries) {
      lsep = lAddElemStr(&lsl, HS_name, lGetString(ep, CE_name), HS_Type);
      lSetDouble(lsep, HS_value, 1.0);
   }
   lFreeList(&entries);
      
   if (ehp) {
      ehsl = lGetList(ehp, EH_scaling_list);
      
      for_each (ep, ehsl) {
         lsep = lGetElemStr(lsl, HS_name, lGetString(ep, HS_name));
         lSetDouble(lsep, HS_value, lGetDouble(ep, HS_value));
      }
   }

   /* 
   ** set now fully configured load scaling list 
   */
   lFreeList(&host_data.scaling_list);
   host_data.scaling_list = lsl;

   /*
   ** set the consumable/per slot limit entries
   */
   if (ehp) {
      host_data.consumable_config_list = lGetList(ehp, 
                                             EH_consumable_config_list);
   }
   else {
      host_data.consumable_config_list = NULL;
   }

   /*
   ** set (x)acl
   */
   if (ehp) {
      host_data.acl = lGetList(ehp, EH_acl);
      host_data.xacl = lGetList(ehp, EH_xacl);
   }
   else {
      host_data.acl = NULL;
      host_data.xacl = NULL;
   }

   /*
   ** set reporting variables
   */
   if (ehp) {
      host_data.reporting_variables = lGetList(ehp, EH_report_variables);
   }
   else {
      host_data.reporting_variables = NULL;
   }

   
   
   /*
   ** build the usage scaling list, we have three entries at the moment:
   ** cpu, io, mem
   */
   usl = lCreateElemList("UsageScalingList", HS_Type, 3);
   ep = lFirst(usl);
   lSetString(ep, HS_name, USAGE_ATTR_CPU);
   lSetDouble(ep, HS_value, 1.0);
   ep = lNext(ep);
   lSetString(ep, HS_name, USAGE_ATTR_MEM);
   lSetDouble(ep, HS_value, 1.0);
   ep = lNext(ep);
   lSetString(ep, HS_name, USAGE_ATTR_IO);
   lSetDouble(ep, HS_value, 1.0);

   if (ehp) {
      /*
      ** get the usage scaling list from host
      */
      ehul = lGetList(ehp, EH_usage_scaling_list);
      
      for_each (ep, ehul) {
         usep = lGetElemStr(usl, HS_name, lGetString(ep, HS_name));
         lSetDouble(usep, HS_value, lGetDouble(ep, HS_value));
      }
   }

   /* 
   ** set now fully configured usage scaling list 
   */
   lFreeList(&(host_data.usage_scaling_list));
   host_data.usage_scaling_list = usl;

   /*
   ** set (x)project
   */
   if (ehp) {
      host_data.prj = lGetList(ehp, EH_prj);
      host_data.xprj = lGetList(ehp, EH_xprj);
   } else {
      host_data.prj = NULL;
      host_data.xprj = NULL;
   }

   /*
   ** set the values in the matrices
   */
   XmtDialogSetDialogValues(eh_ask_layout, &host_data);
   
   /*
   ** fill the acl list
   */
   qmonHostAvailableAcls();

   /*
   ** fill the reporting variables
   */
   qmonHostAvailableReportVars();

   /*
   ** fill the project list
   */
   qmonHostAvailableProjects();

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonExecHostShutdown(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   static lCondition *where = NULL;
   const char *default_cell = ctx->get_default_cell(ctx);

   DENTER(GUI_LAYER, "qmonExecHostShutdown");

   if (!where) 
      where = lWhere("%T(%I != %s)", EH_Type, EH_name, SGE_GLOBAL_NAME);

   lp = XmStringToCull(exechost_list, EH_Type, EH_name, SELECTED_ITEMS);
   lp = lSelectDestroy(lp, where); 

   /* if no elements remain, we get an empty list, however gdi_kill()
    * deletes all hosts if list is empty
    */
   if (lp) {
      alp = ctx->kill(ctx, lp, default_cell, 0, EXECD_KILL); 
      qmonMessageBox(w, alp, 1);
      lFreeList(&lp);
      lFreeList(&alp);
   }

   DEXIT;
}
         
/*-------------------------------------------------------------------------*/
static void qmonHostDelete(Widget w, XtPointer cld, XtPointer cad)
{
   /* int type = (int)(long) cld; */ /* 
                                * 64 bit pointers == long , this cast
                                * keeps the compiler's mouth shut
                                */
   int type = dialog_mode;
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   lDescr *dp;
   int field;
   Widget list;

   DENTER(GUI_LAYER, "qmonHostDelete");

   switch (type) {
      case SGE_AH_LIST:
         dp = AH_Type;
         list = adminhost_list;
         field = AH_name;
         break;
      case SGE_SH_LIST:
         dp = SH_Type;
         list = submithost_list;
         field = SH_name;
         break;
      case SGE_EH_LIST:
         dp = EH_Type;
         list = exechost_list;
         field = EH_name;
         break;
      case SGE_HGRP_LIST:
         dp = HGRP_Type;
         list = hostgroup_list;
         field = HGRP_name;
         break;
      default:
         DEXIT;
         return;
   }
    
   lp = XmStringToCull(list, dp, field, SELECTED_ITEMS);
      
   if (lp) {
      what = lWhat("%T(ALL)", dp);
      
      alp = qmonDelList(type, qmonMirrorListRef(type), field, &lp, NULL, what);

      qmonMessageBox(w, alp, 0);

      updateHostList();
      
      lFreeWhat(&what);
      lFreeList(&lp);
      lFreeList(&alp);
   }
   DEXIT;
}
         
/*-------------------------------------------------------------------------*/
static void qmonHostHelp(Widget w, XtPointer cld, XtPointer cad)
{
   Widget widget = 0;
   
   DENTER(GUI_LAYER, "qmonHostHelp");

   switch(dialog_mode) {
      case SGE_AH_LIST:
         widget = adminhost_list; 
         break;
      case SGE_SH_LIST:
         widget = submithost_list; 
         break;
      case SGE_HGRP_LIST:
         widget = hostgroup_list; 
         break;
      case SGE_EH_LIST:
         widget = exechost_list; 
         break;
   }

   XmtHelpDisplayContextHelp(widget);  

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonHostAdd(Widget w, XtPointer cld, XtPointer cad)
{
   int type = dialog_mode;
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   String host = NULL;
   char unique[CL_MAXHOSTLEN];
   lDescr *dp;
   int field, ret;
   Widget list;
   Widget hostname;

   DENTER(GUI_LAYER, "qmonHostAdd");

   switch (type) {
      case SGE_AH_LIST:
         dp = AH_Type;
         list = adminhost_list;
         field = AH_name;
         hostname = XmtNameToWidget(list, "~*adminhost_hostname");
         break;
      case SGE_SH_LIST:
         dp = SH_Type;
         list = submithost_list;
         field = SH_name;
         hostname = XmtNameToWidget(list, "~*submithost_hostname");
         break;
      case SGE_HGRP_LIST:
         qmonHostgroupChange(w, cld, NULL); 
         DEXIT;
         return;
      case SGE_EH_LIST:
         qmonExecHostChange(w, cld, NULL); 
         DEXIT;
         return;
      default:
         DPRINTF(("Not allowed\n"));
         DEXIT;
         return;
   }

   host = XmtInputFieldGetString(hostname);
   

   if (host && host[0] != '\0' && host[0] != ' ') { 
       
      DPRINTF(("host = %s\n", host));

      /* try to resolve hostname */
      ret=sge_resolve_hostname(host, unique, EH_name, sizeof(unique));

      switch ( ret ) {
         case CL_RETVAL_GETHOSTNAME_ERROR:
            qmonMessageShow(w, True, "Can't resolve host '%s'", host);
            break;
         case CL_RETVAL_OK:
            what = lWhat("%T(ALL)", dp);
            
            if (!(lp = lCreateElemList("AH", dp, 1))) {
               fprintf(stderr, "lCreateElemList failed\n");
               lFreeWhat(&what);
               DEXIT;
               return;
            }
            lSetHost(lFirst(lp), field, unique);

            alp = qmonAddList(type, qmonMirrorListRef(type), 
                                 field, &lp, NULL, what);
               
            qmonMessageBox(w, alp, 0);
               
            updateHostList();
            XmListSelectPos(list, 0, True);
            
            lFreeWhat(&what);
            lFreeList(&lp);
            lFreeList(&alp);
            break;
            
         default:
            DPRINTF(("sge_resolve_hostname() failed resolving: %s\n", cl_get_error_text(ret)));
      }
      XmtInputFieldSetString(hostname, "");
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostChange(Widget w, XtPointer cld, XtPointer cad)
{
   long mode = (long) cld;
   Cardinal ehnum;
   XmString *ehnames;
   String ehstr;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonExecHostChange");

   qmonMirrorMultiAnswer(EXECHOST_T | USERSET_T | PROJECT_T | CENTRY_T,
                           &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   if (mode) {
      XtVaSetValues( eh_name_w,
                     XmNeditable, True,
                     NULL);
      XmtInputFieldSetString(eh_name_w, "");
      /*
      ** fill the exec host ask dialog with default values
      */
      qmonExecHostSetAsk(NULL);
      add_mode = mode;

   } else {
      /*
      ** on opening the dialog fill in the old values
      */
      XtVaGetValues( exechost_list,
                     XmNselectedItems, &ehnames,
                     XmNselectedItemCount, &ehnum,
                     NULL);
      
      if (ehnum == 1 && 
            XmStringGetLtoR(ehnames[0], XmFONTLIST_DEFAULT_TAG, &ehstr)) {
         XmtInputFieldSetString(eh_name_w, ehstr);
         XtVaSetValues( eh_name_w,
                        XmNeditable, False,
                        NULL);
         qmonExecHostSetAsk(ehstr);
         XtFree((char*)ehstr);
         add_mode = 0;
      }
   }

   XtManageChild(eh_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostCheckName(Widget w, XtPointer cld, XtPointer cad)
{
   XmtInputFieldCallbackStruct *cbs = (XmtInputFieldCallbackStruct*) cad;
   static char unique[CL_MAXHOSTLEN];
   int ret;

   DENTER(GUI_LAYER, "qmonExecHostCheckName");
   
   if (cbs->input && cbs->input[0] != '\0' && cbs->input[0] != ' ') { 
       
      DPRINTF(("cbs->input = %s\n", cbs->input));

      strcpy(unique, "");

      /* try to resolve hostname */
      ret=sge_resolve_hostname((const char*)cbs->input, unique, EH_name, sizeof(unique));

      switch ( ret ) {
         case CL_RETVAL_GETHOSTNAME_ERROR:
            qmonMessageShow(w, True, "Can't resolve host '%s'", cbs->input);
            cbs->okay = False;
            break;
         case CL_RETVAL_OK:
            cbs->input = unique;
            break;
         default:
            DPRINTF(("sge_resolve_hostname() failed resolving: %s\n", cl_get_error_text(ret)));
            cbs->okay = False;
      }
   }
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonExecHostCheckScaling(Widget w, XtPointer cld, XtPointer cad)
{
   XbaeMatrixLeaveCellCallbackStruct *cbs =
         (XbaeMatrixLeaveCellCallbackStruct*) cad;
   double value;
   Pixel bg;

   DENTER(GUI_LAYER, "qmonExecHostCheckScaling");

   if (cbs->column == 1) {
      value = atof(cbs->value);
      if (value < 0) {
         cbs->doit = False;
      }
      if ( !cbs->doit )
         XbaeMatrixSetCellBackground(w, cbs->row, cbs->column, WarningPixel);
      else {
         if (cbs->row % 2)
            XtVaGetValues(w, XmNoddRowBackground, &bg, NULL);
         else
            XtVaGetValues(w, XmNevenRowBackground, &bg, NULL);
         XbaeMatrixSetCellBackground(w, cbs->row, cbs->column, bg);
      }
   }

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonHostFolderChange(Widget w, XtPointer cld, XtPointer cad)
{
   XmTabCallbackStruct *cbs = (XmTabCallbackStruct *) cad;

   DENTER(GUI_LAYER, "qmonHostFolderChange");
   
   DPRINTF(("%s\n", XtName(cbs->tab_child)));

   if (!strcmp(XtName(cbs->tab_child), "adminhost_layout"))
      dialog_mode = SGE_AH_LIST;

   if (!strcmp(XtName(cbs->tab_child), "submithost_layout"))
      dialog_mode = SGE_SH_LIST;

   if (!strcmp(XtName(cbs->tab_child), "hostgroup_layout"))
      dialog_mode = SGE_HGRP_LIST;

   if (!strcmp(XtName(cbs->tab_child), "exechost_layout"))
      dialog_mode = SGE_EH_LIST;

   
   updateHostListCB(w, NULL, NULL);

   XtSetSensitive(host_modify, (dialog_mode==SGE_EH_LIST ||
                   dialog_mode==SGE_HGRP_LIST) ? True:False);
   XtSetSensitive(host_shutdown, (dialog_mode==SGE_EH_LIST) ? True:False);
      
   DEXIT;
}


/*-------------------------------------------------------------------------*/
/* A C C E S S L I S T     P A G E                                         */
/*-------------------------------------------------------------------------*/
static void qmonExecHostAccessToggle(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;
   
   DENTER(GUI_LAYER, "qmonExecHostAccessToggle");

   if (cbs->state) {
      XtSetSensitive(access_allow, False);
      XtSetSensitive(access_deny, True);
   }
   else {
      XtSetSensitive(access_allow, True);
      XtSetSensitive(access_deny, False);
   }
  
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostAccessAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   Widget list;
   int state;
   String text;
   
   DENTER(GUI_LAYER, "qmonExecHostAccessAdd");

   XtVaGetValues( access_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
/*       XmtLayoutDisableLayout(qc_dialog); */
      state = XmtChooserGetState(access_toggle);

      if (state)
         list = access_deny;
      else
         list = access_allow;
         
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(list, text);
         XtFree(text); 
      }
/*       XmtLayoutEnableLayout(qc_dialog); */
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostAccessRemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   int state;
   Widget list;
   
   DENTER(GUI_LAYER, "qmonExecHostAccessRemove");

   state = XmtChooserGetState(access_toggle);

   if (state)
      list = access_deny;
   else
      list = access_allow;
         
   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* R E P O R T I N G  V A R I A B L E S                                    */
/*-------------------------------------------------------------------------*/
static void qmonExecHostReportVarAdd(Widget w, XtPointer cld, XtPointer cad) 
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   String text;
   
   DENTER(GUI_LAYER, "qmonExecHostReportVarAdd");

   XtVaGetValues( reporting_variables_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(reporting_variables_chosen, text);
         XtFree(text); 
      }
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostReportVarRemove(Widget w, XtPointer cld, XtPointer cad) 
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   
   DENTER(GUI_LAYER, "qmonExecHostReportVarRemove");

   XtVaGetValues( reporting_variables_chosen,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(reporting_variables_chosen, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* P R O J E C T     P A G E                                               */
/*-------------------------------------------------------------------------*/
static void qmonExecHostProjectToggle(Widget w, XtPointer cld, XtPointer cad)
{
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct*) cad;
   
   DENTER(GUI_LAYER, "qmonExecHostProjectToggle");

   if (cbs->state) {
      XtSetSensitive(project_allow, False);
      XtSetSensitive(project_deny, True);
   }
   else {
      XtSetSensitive(project_allow, True);
      XtSetSensitive(project_deny, False);
   }
  
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostProjectAdd(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount, i;
   Widget list;
   int state;
   String text;
   
   DENTER(GUI_LAYER, "qmonExecHostProjectAdd");

   XtVaGetValues( project_list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount) {
/*       XmtLayoutDisableLayout(qc_dialog); */
      state = XmtChooserGetState(project_toggle);

      if (state)
         list = project_deny;
      else
         list = project_allow;
         
      for (i=0; i<selectedItemCount; i++) {
         if (!XmStringGetLtoR(selectedItems[i], XmFONTLIST_DEFAULT_TAG, &text))
            continue;
         XmListAddItemUniqueSorted(list, text);
         XtFree(text); 
      }
/*       XmtLayoutEnableLayout(qc_dialog); */
   }

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonExecHostProjectRemove(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *selectedItems;
   Cardinal selectedItemCount;
   int state;
   Widget list;
   
   DENTER(GUI_LAYER, "qmonExecHostProjectRemove");

   state = XmtChooserGetState(project_toggle);

   if (state)
      list = project_deny;
   else
      list = project_allow;
         
   XtVaGetValues( list,
                  XmNselectedItems, &selectedItems,
                  XmNselectedItemCount, &selectedItemCount,
                  NULL);

   if (selectedItemCount)
      XmListDeleteItems(list, selectedItems, selectedItemCount); 

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Widget qmonCreateHostgroupAsk(
Widget parent 
) {
   Widget hg_ok, hg_cancel, hg_ask_layout, hg_add, hg_add_hg,
          hg_del_member;

   DENTER(GUI_LAYER, "qmonCreateHostgroupAsk");
   
   hg_ask_layout = XmtBuildQueryDialog(parent, "hg_ask_shell",
                           NULL, 0,
                           "hg_ok", &hg_ok,
                           "hg_cancel", &hg_cancel,
                           "hg_name", &hg_name_w,
                           "hg_memberlist", &hg_memberlist_w,
                           "hg_hglist", &hg_hglist_w,
                           "hg_member", &hg_member_w,
                           "hg_add_hg", &hg_add_hg,
                           "hg_add", &hg_add,
                           "hg_del_member", &hg_del_member,
                           NULL);

   XtAddCallback(hg_ok, XmNactivateCallback, 
                     qmonHostgroupOk, NULL);
   XtAddCallback(hg_cancel, XmNactivateCallback, 
                     qmonHostgroupCancel, NULL);
   XtAddCallback(hg_member_w, XmtNverifyCallback,
                      qmonHostgroupCheckName, NULL);
   XtAddCallback(hg_member_w, XmtNinputCallback, 
                      qmonHostgroupAddHost, NULL);
   XtAddCallback(hg_add, XmNactivateCallback, 
                     qmonHostgroupAddHost, NULL);
   XtAddCallback(hg_add_hg, XmNactivateCallback, 
                     qmonHostgroupAddHostgroups, NULL);
   XtAddCallback(hg_del_member, XmNactivateCallback, 
                     DeleteItems, (XtPointer) hg_memberlist_w);

   XtAddEventHandler(XtParent(hg_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return hg_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupChange(Widget w, XtPointer cld, XtPointer cad)
{
   long mode = (long) cld;
   Cardinal hgnum;
   XmString *hgnames;
   String hgstr;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonHostgroupChange");

   qmonMirrorMultiAnswer(HGROUP_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   if (mode) {
      XtVaSetValues( hg_name_w,
                     XmNeditable, True,
                     NULL);
      XmtInputFieldSetString(hg_name_w, "");
      /*
      ** fill the hostgroup ask dialog with default values
      */
      qmonHostgroupSetAsk(NULL);
      add_mode = mode;

   } else {
      /*
      ** on opening the dialog fill in the old values
      */
      XtVaGetValues( hostgroup_list,
                     XmNselectedItems, &hgnames,
                     XmNselectedItemCount, &hgnum,
                     NULL);
      
      if (hgnum == 1 && 
            XmStringGetLtoR(hgnames[0], XmFONTLIST_DEFAULT_TAG, &hgstr)) {
         XmtInputFieldSetString(hg_name_w, hgstr);
         XtVaSetValues( hg_name_w,
                        XmNeditable, False,
                        NULL);
         qmonHostgroupSetAsk(hgstr);
         XtFree((char*)hgstr);
         add_mode = 0;
      }
   }

   XtManageChild(hg_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   StringConst hgname = NULL;
   lListElem *hg_ep = NULL;
   lList *href_list = NULL;

   DENTER(GUI_LAYER, "qmonHostgroupOk");

   /*
   ** get the list with one new host
   */
   hgname = XmtInputFieldGetString(hg_name_w);
   if (!hgname || hgname[0] == '\0') {
      qmonMessageShow(w, True, "@{Empty hostgroup name !}");
      DEXIT;
      return;
   }

   href_list = XmStringToCull(hg_memberlist_w, HR_Type, HR_name, ALL_ITEMS);
   hg_ep = hgroup_create(&alp, hgname, href_list, true);

   if (add_mode) {
      hgroup_add_del_mod_via_gdi(ctx, hg_ep, &alp, SGE_GDI_ADD);
   } else {
      hgroup_add_del_mod_via_gdi(ctx, hg_ep, &alp, SGE_GDI_MOD);
   }   

   if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) != STATUS_OK) {
      qmonMessageBox(w, alp, 0);
   } else {
      XtUnmanageChild(hg_ask_layout);
      updateHostListCB(w, NULL, NULL);
   }

   lFreeList(&alp);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonHostgroupCancel");

   XtUnmanageChild(hg_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupCheckName(Widget w, XtPointer cld, XtPointer cad)
{
   XmtInputFieldCallbackStruct *cbs = (XmtInputFieldCallbackStruct*) cad;
   static char unique[CL_MAXHOSTLEN];
   int ret;

   DENTER(GUI_LAYER, "qmonHostgroupCheckName");
   
   if (cbs->input && cbs->input[0] != '\0' && cbs->input[0] != ' ') { 
       
      DPRINTF(("cbs->input = %s\n", cbs->input));

      strcpy(unique, "");

      /* try to resolve hostname */
      ret=sge_resolve_hostname((const char*)cbs->input, unique, EH_name, sizeof(unique));

      switch ( ret ) {
         case CL_RETVAL_GETHOSTNAME_ERROR:
            qmonMessageShow(w, True, "Can't resolve host '%s'", cbs->input);
            cbs->okay = False;
            break;
         case CL_RETVAL_OK:
            cbs->input = unique;
            break;
         default:
            DPRINTF(("sge_resolve_hostname() failed resolving: %s\n", cl_get_error_text(ret)));
            cbs->okay = False;
      }
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupSetAsk(
StringConst name 
) {
   lListElem *hgp = NULL;
   lList *hgl = NULL;
   
   DENTER(GUI_LAYER, "qmonHostgroupSetAsk");

   hgl = qmonMirrorList(SGE_HGRP_LIST);

   if (name) {
      /*
      ** get the selected hostgroup element
      */
      hgp = hgroup_list_locate(hgl, name);
      if (hgp) {
         /* set name field */
         XmtInputFieldSetString(hg_name_w, name);
         /* set members */
         UpdateXmListFromCull(hg_memberlist_w, XmFONTLIST_DEFAULT_TAG, 
                              lGetList(hgp, HGRP_host_list), HR_name);
      }   
   }
   else {
      UpdateXmListFromCull(hg_memberlist_w, XmFONTLIST_DEFAULT_TAG, 
                              NULL, HR_name);
      XmtInputFieldSetString(hg_name_w, "");
   }   

   /*
   ** FIXME ANDRE
   ** hostgroups that would create a cycle should be suppressed
   */
   UpdateXmListFromCull(hg_hglist_w, XmFONTLIST_DEFAULT_TAG, 
                              hgl, HGRP_name);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupSelect(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *hgname;
   lListElem *hgp;
   
   DENTER(GUI_LAYER, "qmonHostgroupSelect");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &hgname)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   hgp = hgroup_list_locate(qmonMirrorList(SGE_HGRP_LIST), hgname);
   XtFree((char*) hgname);

   if (hgp) {
      UpdateXmListFromCull(hostgroup_memberlist, XmFONTLIST_DEFAULT_TAG, 
                           lGetList(hgp, HGRP_host_list), HR_name);
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupAddHost(Widget w, XtPointer cld, XtPointer cad)
{
   char *host;
   
   DENTER(GUI_LAYER, "qmonHostgroupAddHost");

   host = XmtInputFieldGetString(hg_member_w);

   if (host && host[0] != '\0') {
      XmListAddItemUniqueSorted(hg_memberlist_w, host);
   }

   XmtInputFieldSetString(hg_member_w, "");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonHostgroupAddHostgroups(Widget w, XtPointer cld, XtPointer cad)
{
   int i;
   XmString *items = NULL;
   Cardinal itemCount = 0;

   DENTER(GUI_LAYER, "qmonHostgroupAddHostgroups");

   XtVaGetValues( hg_hglist_w,
               XmNselectedItems, &items,
               XmNselectedItemCount, &itemCount,
               NULL);
   for (i=0; i<itemCount; i++) {
      XmListAddXmStringUniqueSorted(hg_memberlist_w, items[i]); 
   }

   DEXIT;
}
