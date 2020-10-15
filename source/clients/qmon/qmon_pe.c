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
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Layout.h>
#include <Xmt/Chooser.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/MsgLine.h>
#include <Xmt/InputField.h>

#include "Spinbox.h"

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_appres.h"
#include "qmon_timer.h"
#include "qmon_comm.h"
#include "qmon_widgets.h"
#include "qmon_message.h"
#include "qmon_pe.h"
#include "qmon_globals.h"
#include "AskForItems.h"

#include "uti/sge_dstring.h"

#include "gdi/sge_gdi.h"

#include "comm/commlib.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_pe.h"


static Widget qmon_pe = 0;
static Widget pe_names = 0;
static Widget pe_conf_list = 0;
static Widget pe_ask_layout = 0;
static Widget pe_name_w = 0;
static Widget pe_slots_w = 0;
static Widget pe_acl_w = 0;
static Widget pe_xacl_w = 0;
static Widget pe_start_w = 0;
static Widget pe_stop_w = 0;
static Widget pe_alloc_w = 0;
static Widget pe_urgency_w = 0;
static Widget pe_control_slaves_w = 0;
static Widget pe_job_is_first_task_w = 0;
static Widget pe_accounting_summary_w = 0;
static Widget pe_qsort_args_w = 0;
static int add_mode = 0;

/*-------------------------------------------------------------------------*/
static void qmonPopdownPEConfig(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreatePEConfig(Widget parent);
static void qmonPEFillConf(Widget w, lListElem *ep);
static void qmonSelectPE(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEAdd(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEModify(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEDelete(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonPECancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonPEResetAsk(void);
static void qmonPESetAsk(lListElem *pep);
static Widget qmonCreatePEAsk(Widget parent);
static Boolean qmonPEGetAsk(lListElem *pep);
static void qmonPEAskForUsers(Widget w, XtPointer cld, XtPointer cad);

/*-------------------------------------------------------------------------*/
void qmonPopupPEConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonPopupPEConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_pe) {
      shell = XmtGetTopLevelShell(w);
      qmon_pe = qmonCreatePEConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonPopdownPEConfig, NULL);
      /*
      ** create ask layout
      */
      pe_ask_layout = qmonCreatePEAsk(qmon_pe);

   } 
   XSync(XtDisplay(qmon_pe), 0);
   XmUpdateDisplay(qmon_pe);

   qmonMirrorMultiAnswer(PE_T | USERSET_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }
   
   qmonTimerAddUpdateProc(PE_T, "updatePeList", updatePeList);
   qmonStartTimer(PE_T | USERSET_T);
   updatePeList();
   XmListSelectPos(pe_names, 1, True);

   XtManageChild(qmon_pe);
   XRaiseWindow(XtDisplay(XtParent(qmon_pe)), XtWindow(XtParent(qmon_pe)));

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updatePeList(void)
{
   lList *cl;
   
   DENTER(GUI_LAYER, "updatePeList");

   cl = qmonMirrorList(SGE_PE_LIST);
   lPSortList(cl, "%I+", PE_name);
   UpdateXmListFromCull(pe_names, XmFONTLIST_DEFAULT_TAG, cl, PE_name);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonPopdownPEConfig(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPopdownPEConfig");

   XtUnmanageChild(qmon_pe);
   qmonStopTimer(PE_T | USERSET_T);
   qmonTimerRmUpdateProc(PE_T, "updatePeList");

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEFillConf(
Widget w,
lListElem *ep 
) {
   lList *ul;
   lListElem *uep;
   XmString *items;
   Cardinal itemCount; 
   char buf[BUFSIZ];
   StringConst str;
   int i;
   dstring sb = DSTRING_INIT;

   DENTER(GUI_LAYER, "qmonPEFillConf");
   
   if (!ep) {
      /*
      ** clear the pe_conf_list
      */
      XtVaSetValues( pe_conf_list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
      DEXIT;
      return;
   }

   itemCount = 11;
#ifdef SGE_PQS_API
   itemCount++;
#endif
   items = (XmString*) XtMalloc(sizeof(XmString)*itemCount); 

   i = 0;

   /* pe name */
   sprintf(buf, "%-20.20s %s", "PE Name", lGetString(ep, PE_name));
   items[i++] = XmStringCreateLocalized(buf);

   /* slots */
   sprintf(buf, "%-20.20s %d", "Slots", (int)lGetUlong(ep, PE_slots));
   items[i++] = XmStringCreateLocalized(buf);

   /* users list */
   ul = lGetList(ep, PE_user_list);
   sge_dstring_sprintf(&sb, "%-20.20s", "Users");
   for_each(uep, ul) {
      (void) sge_dstring_append(&sb, " "); 
      (void) sge_dstring_append(&sb, lGetString(uep, US_name));
   }
   if (!lGetNumberOfElem(ul))
      (void) sge_dstring_append(&sb, " NONE"); 
   items[i++] = XmStringCreateLocalized(sb.s);
   
   /* xusers list */
   ul = lGetList(ep, PE_xuser_list);
   sge_dstring_sprintf(&sb, "%-20.20s", "Xusers");
   for_each(uep, ul) {
      (void) sge_dstring_append(&sb, " "); 
      (void) sge_dstring_append(&sb, lGetString(uep, US_name));
   }
   if (!lGetNumberOfElem(ul))
      (void) sge_dstring_append(&sb, " NONE"); 
   items[i++] = XmStringCreateLocalized(sb.s);
   sge_dstring_free(&sb);
   
   /* start_proc_args */
   str = (StringConst)lGetString(ep, PE_start_proc_args);
   sprintf(buf, "%-20.20s %s", "Start Proc Args", str ? str : "NONE" );
   items[i++] = XmStringCreateLocalized(buf);

   /* stop_proc_args */
   str = (StringConst)lGetString(ep, PE_stop_proc_args);
   sprintf(buf, "%-20.20s %s", "Stop Proc Args", str ? str : "NONE" );
   items[i++] = XmStringCreateLocalized(buf);

   /* allocation_rule */
   str = (StringConst)lGetString(ep, PE_allocation_rule);
   sprintf(buf, "%-20.20s %s", "Allocation Rule", str ? str : "NONE" );
   items[i++] = XmStringCreateLocalized(buf);

   /* urgency slots */
   str = (StringConst)lGetString(ep, PE_urgency_slots);
   sprintf(buf, "%-20.20s %s", "Urgency Slots", str ? str : "NONE" );
   items[i++] = XmStringCreateLocalized(buf);

   /* control slaves */
   sprintf(buf, "%-20.20s %s", "Control Slaves", 
            (int)lGetBool(ep, PE_control_slaves) ? "true" : "false");
   items[i++] = XmStringCreateLocalized(buf);

   /* job_is_first_task */
   sprintf(buf, "%-20.20s %s", "Job is first task", 
            (int)lGetBool(ep, PE_job_is_first_task) ? "true" : "false");
   items[i++] = XmStringCreateLocalized(buf);

   /* accounting summary */
   sprintf(buf, "%-20.20s %s", "Accounting Summary", 
            (int)lGetBool(ep, PE_accounting_summary) ? "true" : "false");
   items[i++] = XmStringCreateLocalized(buf);

#ifdef SGE_PQS_API
   /* qsort_args */
   str = (StringConst)lGetString(ep, PE_qsort_args);
   sprintf(buf, "%-20.20s %s", "Queue Sort Args", str ? str : "NONE" );
   items[i++] = XmStringCreateLocalized(buf);
#endif
   
   XtVaSetValues(pe_conf_list, 
                 XmNitems, items,
                 XmNitemCount, itemCount,
                 NULL);
   XmStringTableFree(items, itemCount);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectPE(Widget w, XtPointer cld, XtPointer cad)
{
   XmListCallbackStruct *cbs = (XmListCallbackStruct*) cad;
   char *pename;
   lListElem *ep;
   
   DENTER(GUI_LAYER, "qmonSelectPE");

   if (! XmStringGetLtoR(cbs->item, XmFONTLIST_DEFAULT_TAG, &pename)) {
      fprintf(stderr, "XmStringGetLtoR failed\n");
      DEXIT;
      return;
   }

   ep = pe_list_locate(qmonMirrorList(SGE_PE_LIST), pename);

   XtFree((char*) pename);

   qmonPEFillConf(pe_conf_list, ep);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreatePEConfig(
Widget parent 
) {
   Widget pe_layout, pe_add, pe_delete, pe_modify, pe_done, pe_main_link;

   DENTER(GUI_LAYER, "qmonCreatePEConfig");
   
   pe_layout = XmtBuildQueryDialog( parent, "qmon_pe",
                           NULL, 0,
                           "pe_names", &pe_names,
                           "pe_conf_list", &pe_conf_list,
                           "pe_add", &pe_add,
                           "pe_delete", &pe_delete,
                           "pe_done", &pe_done,
                           "pe_modify", &pe_modify,
                           "pe_main_link", &pe_main_link,
                           NULL);

   XtAddCallback(pe_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(pe_names, XmNbrowseSelectionCallback, 
                     qmonSelectPE, NULL);
   XtAddCallback(pe_done, XmNactivateCallback, 
                     qmonPopdownPEConfig, NULL);
   XtAddCallback(pe_add, XmNactivateCallback, 
                     qmonPEAdd, NULL); 
   XtAddCallback(pe_modify, XmNactivateCallback, 
                     qmonPEModify, NULL); 
   XtAddCallback(pe_delete, XmNactivateCallback, 
                     qmonPEDelete, NULL); 

   XtAddEventHandler(XtParent(pe_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return pe_layout;
}



/*-------------------------------------------------------------------------*/
static Widget qmonCreatePEAsk(
Widget parent 
) {
   Widget pe_ok, pe_cancel, pe_usersPB, pe_xusersPB;

   DENTER(GUI_LAYER, "qmonCreatePEAsk");
   
   pe_ask_layout = XmtBuildQueryDialog(parent, "pe_ask_shell",
                           NULL, 0,
                           "pe_ok", &pe_ok,
                           "pe_cancel", &pe_cancel,
                           "pe_usersPB", &pe_usersPB,
                           "pe_xusersPB", &pe_xusersPB,
                           "pe_name", &pe_name_w,
                           "pe_slots", &pe_slots_w,
                           "pe_users", &pe_acl_w,
                           "pe_xusers", &pe_xacl_w,
                           "pe_start_proc_args", &pe_start_w,
                           "pe_stop_proc_args", &pe_stop_w,
                           "pe_allocation_rule", &pe_alloc_w,
                           "pe_urgency_slots", &pe_urgency_w,
                           "pe_control_slaves", &pe_control_slaves_w,
                           "pe_job_is_first_task", &pe_job_is_first_task_w,
                           "pe_accounting_summary", &pe_accounting_summary_w,
                           "pe_qsort_args", &pe_qsort_args_w,
                           NULL);

#ifndef SGE_PQS_API
   XtUnmanageChild(pe_qsort_args_w);
#endif

   XtAddCallback(pe_ok, XmNactivateCallback, 
                     qmonPEOk, NULL);
   XtAddCallback(pe_cancel, XmNactivateCallback, 
                     qmonPECancel, NULL);
   XtAddCallback(pe_usersPB, XmNactivateCallback, 
                     qmonPEAskForUsers, (XtPointer)pe_acl_w);
   XtAddCallback(pe_xusersPB, XmNactivateCallback, 
                     qmonPEAskForUsers, (XtPointer)pe_xacl_w);

   XtAddEventHandler(XtParent(pe_ask_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);

   DEXIT;
   return pe_ask_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonPEAskForUsers(Widget w, XtPointer cld, XtPointer cad)
{
   lList *ql_out = NULL;
   lList *ql_in = NULL;
   int status;
   Widget list = (Widget) cld;

   DENTER(GUI_LAYER, "qmonPEAskForUsers");
   
   ql_in = qmonMirrorList(SGE_US_LIST);
   ql_out = XmStringToCull(list, US_Type, US_name, ALL_ITEMS);

   status = XmtAskForItems(w, NULL, NULL, "@{Select Access Lists}", ql_in, US_name,
                  "@{@fBAvailable Access Lists}", &ql_out, US_Type, US_name, 
                  "@{@fBChosen Access Lists}", NULL);

   if (status) {
      UpdateXmListFromCull(list, XmFONTLIST_DEFAULT_TAG, ql_out,
                              US_name);
   }
   lFreeList(&ql_out);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEAdd(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPEAdd");

   qmonPEResetAsk();
   XtVaSetValues( pe_name_w,
                  XmNeditable, True,
                  NULL);
   add_mode = 1;
   XtManageChild(pe_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEModify(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *penames;
   Cardinal penum;
   String pestr;
   lListElem *pep = NULL;

   DENTER(GUI_LAYER, "qmonPEModify");

   /*
   ** on opening the dialog fill in the old values
   */
   XtVaGetValues( pe_names,
                  XmNselectedItems, &penames,
                  XmNselectedItemCount, &penum,
                  NULL);
   
   if (penum == 1 && 
         XmStringGetLtoR(penames[0], XmFONTLIST_DEFAULT_TAG, &pestr)) {
      XmtInputFieldSetString(pe_name_w, pestr);
      XtVaSetValues( pe_name_w,
                     XmNeditable, False,
                     NULL);
      pep = pe_list_locate(qmonMirrorList(SGE_PE_LIST), pestr);
      XtFree((char*)pestr);
      if (pep) {
         add_mode = 0;
         qmonPESetAsk(pep);
         XtManageChild(pe_ask_layout);
      }
      XtManageChild(pe_ask_layout);
   }


   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonPEOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *pel = NULL;
   lList *alp;
   lEnumeration *what;
   Boolean status = False;
   XmString xpename = NULL;
   StringConst pename = NULL;

   DENTER(GUI_LAYER, "qmonPEOk");
   /*
   ** get the contents of the dialog fields here,
   ** build the cull list and send gdi request
   ** depending on success of gdi request close the dialog or stay open
   */
   pel = lCreateElemList("PE_ADD", PE_Type, 1);
   
   if (pel) {
      if (qmonPEGetAsk(lFirst(pel))) {

         pename = (StringConst)lGetString(lFirst(pel), PE_name);
         /*
         ** gdi call 
         */
         what = lWhat("%T(ALL)", PE_Type);
         
         if (add_mode) {
            alp = qmonAddList(SGE_PE_LIST, qmonMirrorListRef(SGE_PE_LIST),
                              PE_name, &pel, NULL, what);
         }
         else {
            alp = qmonModList(SGE_PE_LIST, qmonMirrorListRef(SGE_PE_LIST),
                              PE_name, &pel, NULL, what);
         }

         if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK)
            status = True;

         qmonMessageBox(w, alp, 0);

         if (status) {
            XtUnmanageChild(pe_ask_layout);
            updatePeList();
            /*
            ** select the modified or added PE
            */
            xpename = XmtCreateXmString(pename);
            XmListSelectItem(pe_names, xpename, True);
            XmStringFree(xpename);
         }
         lFreeWhat(&what);
         lFreeList(&alp);
      }
      lFreeList(&pel);
   }
   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonPECancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonPECancel");

   XtUnmanageChild(pe_ask_layout);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEDelete(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lEnumeration *what = NULL;
   Cardinal itemCount = 0;
   Boolean status, answer;

   DENTER(GUI_LAYER, "qmonPEDelete");
    
   lp = XmStringToCull(pe_names, PE_Type, PE_name, SELECTED_ITEMS); 

   if (lp) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{pe.askdel.Do you really want to delete the\nselected Parallel Environment\nConfiguration ?}", 
                     "@{Delete}", "@{Cancel}", NULL, XmtNoButton, XmDIALOG_WARNING, 
                     False, &answer, NULL);
      if (answer) { 
         what = lWhat("%T(ALL)", PE_Type);
         alp = qmonDelList(SGE_PE_LIST, qmonMirrorListRef(SGE_PE_LIST),
                                 PE_name, &lp, NULL, what);

         qmonMessageBox(w, alp, 0);

         lFreeWhat(&what);
         lFreeList(&alp);

         updatePeList();
         XtVaGetValues( pe_names,
                        XmNitemCount, &itemCount,
                        NULL);
         if (itemCount)
            XmListSelectPos(pe_names, 1, True);
         else
            qmonPEFillConf(pe_names, NULL);

      }
      lFreeList(&lp);
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPESetAsk(
lListElem *pep 
) {
   StringConst pe_name = NULL;
   u_long32 pe_slots = 0;
   lList *acl = NULL;
   lList *xacl = NULL;
   StringConst start_args = NULL;
   StringConst stop_args = NULL;
   StringConst alloc_rule = NULL;
   StringConst urgency_slots = NULL;
#ifdef SGE_PQS_API
   StringConst qsort_args = NULL;
#endif

   DENTER(GUI_LAYER, "qmonPESetAsk");

   if (!pep) {
      DEXIT;
      return;
   }

   pe_name = (StringConst)lGetString(pep, PE_name);
   if (pe_name)
      XmtInputFieldSetString(pe_name_w, pe_name);

   pe_slots = lGetUlong(pep, PE_slots);
   XmpSpinboxSetValue(pe_slots_w, pe_slots, True);

   acl = lGetList(pep, PE_user_list);
   UpdateXmListFromCull(pe_acl_w, XmFONTLIST_DEFAULT_TAG, acl, US_name);
    
   xacl = lGetList(pep, PE_xuser_list);
   UpdateXmListFromCull(pe_xacl_w, XmFONTLIST_DEFAULT_TAG, xacl, US_name);
      
   start_args = (StringConst)lGetString(pep, PE_start_proc_args);
   if (start_args)
      XmtInputFieldSetString(pe_start_w, start_args);

   stop_args = (StringConst)lGetString(pep, PE_stop_proc_args);
   if (stop_args)
      XmtInputFieldSetString(pe_stop_w, stop_args);

   alloc_rule = (StringConst)lGetString(pep, PE_allocation_rule);
   if (alloc_rule)
      XmtInputFieldSetString(pe_alloc_w, alloc_rule);

   urgency_slots = (StringConst)lGetString(pep, PE_urgency_slots);
   if (urgency_slots)
      XmtInputFieldSetString(pe_urgency_w, urgency_slots);

   XmToggleButtonSetState(pe_control_slaves_w, 
               lGetBool(pep, PE_control_slaves), False);

   XmToggleButtonSetState(pe_job_is_first_task_w, 
               lGetBool(pep, PE_job_is_first_task), False);

   XmToggleButtonSetState(pe_accounting_summary_w, 
               lGetBool(pep, PE_accounting_summary), False);

#ifdef SGE_PQS_API
   qsort_args = (StringConst)lGetString(pep, PE_qsort_args);
   if (qsort_args)
      XmtInputFieldSetString(pe_qsort_args_w, qsort_args);
#endif


   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonPEResetAsk(void)
{

   DENTER(GUI_LAYER, "qmonPEResetAsk");

   XmtInputFieldSetString(pe_name_w, "");

   XmpSpinboxSetValue(pe_slots_w, 0, True);

   /*
   ** the lists have to be converted to XmString
   */
   UpdateXmListFromCull(pe_acl_w, XmFONTLIST_DEFAULT_TAG, NULL, US_name);
    
   UpdateXmListFromCull(pe_xacl_w, XmFONTLIST_DEFAULT_TAG, NULL, US_name);
      
   XmtInputFieldSetString(pe_start_w, "NONE");

   XmtInputFieldSetString(pe_stop_w, "NONE");

   XmtInputFieldSetString(pe_alloc_w, "$pe_slots");

   XmtInputFieldSetString(pe_urgency_w, "min");

   XmToggleButtonSetState(pe_control_slaves_w, 0, False);

   XmToggleButtonSetState(pe_job_is_first_task_w, 0, False);

   XmToggleButtonSetState(pe_accounting_summary_w, 0, False);

#ifdef SGE_PQS_API
   XmtInputFieldSetString(pe_qsort_args_w, "NONE");
#endif

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonPEGetAsk(
lListElem *pep 
) {
   String pe_name = NULL;
   u_long32 pe_slots = 0;
   u_long32 pe_control_slaves = 0;
   u_long32 pe_job_is_first_task = 0;
   u_long32 pe_accounting_summary = 0;
   lList *acl = NULL;
   lList *xacl = NULL;
   String start_args = NULL;
   String stop_args = NULL;
   String alloc_rule = NULL;
   String urgency_slots = NULL;
#ifdef SGE_PQS_API
   String qsort_args = NULL;
#endif

   DENTER(GUI_LAYER, "qmonPEGetAsk");

   if (!pep) {
      DEXIT;
      return False;
   }

   pe_name = XmtInputFieldGetString(pe_name_w);
   if (!pe_name || pe_name[0] == '\0') {
      qmonMessageShow(pe_ask_layout, True, "@{PE name required !}");
      DEXIT;
      return False;
   }
   lSetString(pep, PE_name, qmon_trim(pe_name));
  

   pe_slots = XmpSpinboxGetValue(pe_slots_w);
   lSetUlong(pep, PE_slots, pe_slots);

   /*
   ** XmString entries --> Cull
   */
   acl = XmStringToCull(pe_acl_w, US_Type, US_name, ALL_ITEMS);
   lSetList(pep, PE_user_list, acl);

   xacl = XmStringToCull(pe_xacl_w, US_Type, US_name, ALL_ITEMS);
   lSetList(pep, PE_xuser_list, xacl);
   
   start_args = XmtInputFieldGetString(pe_start_w);
   if (!start_args || !strcasecmp(start_args, "none"))
      lSetString(pep, PE_start_proc_args, NULL);
   else   
      lSetString(pep, PE_start_proc_args, start_args);

   stop_args = XmtInputFieldGetString(pe_stop_w);
   if (!stop_args || !strcasecmp(stop_args, "none"))
      lSetString(pep, PE_stop_proc_args, NULL);
   else   
      lSetString(pep, PE_stop_proc_args, stop_args);

   alloc_rule = XmtInputFieldGetString(pe_alloc_w);
   lSetString(pep, PE_allocation_rule, alloc_rule);

   urgency_slots = XmtInputFieldGetString(pe_urgency_w);
   lSetString(pep, PE_urgency_slots, urgency_slots);

   pe_control_slaves = XmToggleButtonGetState(pe_control_slaves_w); 
   lSetBool(pep, PE_control_slaves, pe_control_slaves);

   pe_job_is_first_task = XmToggleButtonGetState(pe_job_is_first_task_w); 
   lSetBool(pep, PE_job_is_first_task, pe_job_is_first_task);

   pe_accounting_summary = XmToggleButtonGetState(pe_accounting_summary_w); 
   lSetBool(pep, PE_accounting_summary, pe_accounting_summary);

#ifdef SGE_PQS_API
   qsort_args = XmtInputFieldGetString(pe_qsort_args_w);
   if (!qsort_args || !strcasecmp(qsort_args, "none"))
      lSetString(pep, PE_qsort_args, NULL);
   else   
      lSetString(pep, PE_qsort_args, qsort_args);
#endif

   DEXIT;
   return True;
}
