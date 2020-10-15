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

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Icon.h>
#include <Xmt/Layout.h>
#include <Xmt/Dialog.h>
#include <Xmt/Dialogs.h>
#include <Xmt/InputField.h>
#include <Xmt/MsgLine.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_globals.h"
#include "qmon_appres.h"
#include "qmon_util.h"
#include "qmon_init.h"
#include "qmon_ticket.h"
#include "qmon_share.h"
#include "qmon_fticket.h"
#include "qmon_comm.h"
#include "qmon_message.h"
#include "qmon_manop.h"
#include "qmon_timer.h"
#include "qmon_quarks.h"

#include "uti/sge_string.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"

#include "gdi/sge_gdi.h"



typedef struct _TOVEntry {
   u_long32 total_share_tree_tickets;
   u_long32 total_functional_tickets;
   u_long32 override_tickets;
   Boolean  share_override_ticket;
   Boolean report_pjob_tickets;
   Boolean  share_functional_ticket;
   int max_pending_tasks_per_job;
   int max_functional_jobs;
   double weight_deadline;
   double weight_waiting_time;
   double weight_urgency;
   double weight_priority;
   double weight_ticket;
   char *policy_hierarchy;
} tTOVEntry;

static tTOVEntry cdata = {0, 0, 0, False, False, False, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, NULL};
static Boolean data_changed = False;

static Widget qmon_tov = 0;
static Widget tov_layout = 0;
static Widget tov_message = 0;

XtResource tov_resources[] = {
   { "total_share_tree_tickets", "total_share_tree_tickets", QmonRUlong32,
      sizeof(u_long32),
      XtOffsetOf(tTOVEntry, total_share_tree_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "label_share_tree_tickets", "label_share_tree_tickets", QmonRUlong32,
      sizeof(u_long32),
      XtOffsetOf(tTOVEntry, total_share_tree_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "total_functional_tickets", "total_functional_tickets", QmonRUlong32,
      sizeof(u_long32),
      XtOffsetOf(tTOVEntry, total_functional_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "label_functional_tickets", "label_functional_tickets", QmonRUlong32,
      sizeof(u_long32),
      XtOffsetOf(tTOVEntry, total_functional_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "override_tickets", "override_tickets", QmonRUlong32,
      sizeof(u_long32),
      XtOffsetOf(tTOVEntry, override_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "max_functional_jobs", "max_functional_jobs", XtRInt,
      sizeof(int),
      XtOffsetOf(tTOVEntry, max_functional_jobs),
      XtRImmediate, (XtPointer) 0 },

   { "max_pending_tasks_per_job", "max_pending_tasks_per_job", XtRInt,
      sizeof(int),
      XtOffsetOf(tTOVEntry, max_pending_tasks_per_job),
      XtRImmediate, (XtPointer) 0 },

   { "report_pjob_tickets", "report_pjob_tickets", XtRBoolean,
      sizeof(Boolean),
      XtOffsetOf(tTOVEntry, report_pjob_tickets),
      XtRImmediate, (XtPointer) 0 },

   { "share_override_ticket", "share_override_ticket", XtRBoolean,
      sizeof(Boolean),
      XtOffsetOf(tTOVEntry, share_override_ticket),
      XtRImmediate, (XtPointer) 0 },

   { "share_functional_ticket", "share_functional_ticket", XtRBoolean,
      sizeof(Boolean),
      XtOffsetOf(tTOVEntry, share_functional_ticket),
      XtRImmediate, (XtPointer) 0 },

   { "weight_deadline", "weight_deadline", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tTOVEntry, weight_deadline),
      XtRImmediate, (XtPointer) 0 },

   { "weight_waiting_time", "weight_waiting_time", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tTOVEntry, weight_waiting_time),
      XtRImmediate, (XtPointer) 0 },

   { "weight_urgency", "weight_urgency", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tTOVEntry, weight_urgency),
      XtRImmediate, (XtPointer) 0 },

   { "weight_priority", "weight_priority", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tTOVEntry, weight_priority),
      XtRImmediate, (XtPointer) 0 },

   { "weight_ticket", "weight_ticket", XmtRDouble,
      sizeof(double),
      XtOffsetOf(tTOVEntry, weight_ticket),
      XtRImmediate, (XtPointer) 0 },

   { "policy_hierarchy", "policy_hierarchy", XtRString,
      sizeof(String),
      XtOffsetOf(tTOVEntry, policy_hierarchy),
      XtRImmediate, (XtPointer) 0 }

};

/*-------------------------------------------------------------------------*/
static void qmonPopdownTicketOverview(Widget w, XtPointer cld, XtPointer cad);
static Widget qmonCreateTicketOverview(Widget parent);
static void qmonTOVInput(Widget w, XtPointer cld, XtPointer cad);
static void qmonTOVUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonTOVApply(Widget w, XtPointer cld, XtPointer cad);
static Boolean qmonTOVEntryReset(tTOVEntry *tov_data);
static Boolean qmonTOVEntryToCull(lListElem *scep, tTOVEntry *tov_data);
static Boolean qmonCulltoTOVEntry(tTOVEntry *tov_data, lListElem *scep);
static int qmonTOVUpdateFill(Widget w, lList **alpp);


static Widget share_tree_tickets_field = 0;
static Widget functional_tickets_field = 0;
/*-------------------------------------------------------------------------*/
/* P U B L I C                                                             */
/*-------------------------------------------------------------------------*/
void qmonPopupTicketOverview(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;
   DENTER(GUI_LAYER, "qmonPopupTicketOverview");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_tov) {
      qmon_tov = qmonCreateTicketOverview(AppShell);
      /* 
      ** set the close button callback 
      ** set the icon and iconName
      */
      XmtCreatePixmapIcon(qmon_tov, qmonGetIcon("toolbar_ticket"), None);
      XtVaSetValues(qmon_tov, XtNiconName, "qmon:Policy Configuration", NULL);
      XmtAddDeleteCallback(qmon_tov, XmDO_NOTHING, 
                              qmonPopdownTicketOverview, NULL);
      XtAddEventHandler(qmon_tov, StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   }

   /*
   ** set the Ticket Overview Data
   ** cdata should always contain the actual data
   */
   if (qmonTOVUpdateFill(qmon_tov, &alp)) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }

   /*
   ** popup and raise the dialog
   */
   xmui_manage(qmon_tov);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static void qmonPopdownTicketOverview(Widget w, XtPointer cld, XtPointer cad)
{
   Boolean status = True;
   Boolean answer = False;

   DENTER(GUI_LAYER, "qmonPopdownTicketOverview");

   qmonTOVInput(w, cld, cad);
   if (data_changed) {
      status = XmtAskForBoolean(w, "xmtBooleanDialog", 
                     "@{ticket.asksave.Do you want to save your changes ?}", 
                     "@{Yes}", "@{No}", "@{Cancel}", XmtNoButton, 
                        /* XmDIALOG_QUESTION */ XmDIALOG_INFORMATION, 
                     False, &answer, NULL);
   }

   if (status) {
      if (answer)
         qmonTOVApply(w, NULL, NULL);
      else
         data_changed = False;
      xmui_unmanage(qmon_tov);    
   }
   
   DEXIT;
}

#if 0
/*-------------------------------------------------------------------------*/
static void qmonTOVStartUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonTOVStartUpdate");
  
   qmonTimerAddUpdateProc(JOB_T, "updateJobList", updateJobList);
   qmonStartTimer(JOB | QUEUE | COMPLEX);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonJobStopUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonJobStopUpdate");
  
   qmonStopTimer(JOB_TIMER | QUEUE_TIMER | COMPLEX_TIMER);
   qmonTimerRmUpdateProc(JOB_T, "updateJobList");
   
   DEXIT;
}

#endif


/*-------------------------------------------------------------------------*/
static Widget qmonCreateTicketOverview(
Widget parent 
) {
   Widget shell, tov_done, tov_edit, tov_users_for_deadline, 
            tov_share_tree_policy, tov_functional_policy, 
            tov_override_policy,
            tov_update, tov_apply, tov_main_link;
   
   DENTER(GUI_LAYER, "qmonCreateTicketOverview");

   /* create shell */
   shell = XmtCreateChild(parent, "qmon_tov");

   /* submit control layout */
   tov_layout = XmtCreateLayout( shell, "tov_layout", 
                                    NULL, 0); 

   /* bind resource list */
   XmtDialogBindResourceList( tov_layout, 
                              tov_resources, XtNumber(tov_resources));

   /* create children */
   XmtCreateQueryChildren( tov_layout, 
                           "tov_edit", &tov_edit,
                           "share_tree_tickets_field", 
                                 &share_tree_tickets_field,
                           "functional_tickets_field", 
                                 &functional_tickets_field,
                           "tov_users_for_deadline", &tov_users_for_deadline,
                           "tov_share_tree_policy", &tov_share_tree_policy,
                           "tov_functional_policy", &tov_functional_policy,
                           "tov_override_policy", &tov_override_policy,
                           "tov_done", &tov_done,
                           "tov_apply", &tov_apply,
                           "tov_update", &tov_update,
                           "tov_main_link", &tov_main_link,
                           "tov_message", &tov_message,
                           NULL);
   XtManageChild(tov_layout);

#if 0
   /* start the needed timers and the corresponding update routines */
   XtAddCallback(shell, XmNpopupCallback, 
                     qmonTOVStartUpdate, NULL);
   XtAddCallback(shell, XmNpopdownCallback,
                     qmonTOVStopUpdate, NULL);
                     
#endif

   XtAddCallback(tov_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(tov_done, XmNactivateCallback, 
                     qmonPopdownTicketOverview, NULL);
   XtAddCallback(tov_update, XmNactivateCallback, 
                     qmonTOVUpdate, NULL);
   XtAddCallback(tov_apply, XmNactivateCallback, 
                     qmonTOVApply, NULL);

   XtAddCallback(tov_users_for_deadline, XmNactivateCallback, 
                     qmonPopupManopConfig, (XtPointer)2);

   XtAddCallback(tov_share_tree_policy, XmNactivateCallback, 
                     qmonShareTreePopup, NULL);
   XtAddCallback(tov_functional_policy, XmNactivateCallback, 
                     qmonFTicketPopup, NULL);
   XtAddCallback(tov_override_policy, XmNactivateCallback, 
                     qmonOTicketPopup, NULL);

   DEXIT;
   return shell;
}


/*-------------------------------------------------------------------------*/
static void qmonTOVInput(Widget w, XtPointer cld, XtPointer cad)
{
   tTOVEntry data;

   DENTER(GUI_LAYER, "qmonTOVInput");

   /*
   ** get the data that have changed and set the corresponding labels
   ** the links are the label_functional_tickets and label_share_tree_tickets
   ** resources which reference the same entries as the 
   ** total_share_tree_tickets total_functional_tickets resources
   */
   XmtDialogGetDialogValues(tov_layout, &data);

   if (data.total_share_tree_tickets != cdata.total_share_tree_tickets) {
      cdata.total_share_tree_tickets = data.total_share_tree_tickets;
      data_changed = True;
   }
   if (data.total_functional_tickets != cdata.total_functional_tickets) {
      cdata.total_functional_tickets = data.total_functional_tickets;
      data_changed = True;
   }
   if (data.share_override_ticket != cdata.share_override_ticket) {
      cdata.share_override_ticket = data.share_override_ticket;
      data_changed = True;
   }
   if (data.share_functional_ticket != cdata.share_functional_ticket) {
      cdata.share_functional_ticket = data.share_functional_ticket;
      data_changed = True;
   }
   if (data.max_pending_tasks_per_job != cdata.max_pending_tasks_per_job) {
      cdata.max_pending_tasks_per_job = data.max_pending_tasks_per_job;
      data_changed = True;
   }
   if (data.max_functional_jobs != cdata.max_functional_jobs) {
      cdata.max_functional_jobs = data.max_functional_jobs;
      data_changed = True;
   }
   if (data.report_pjob_tickets != cdata.report_pjob_tickets) {
      cdata.report_pjob_tickets = data.report_pjob_tickets;
      data_changed = True;
   }
   if (data.weight_deadline != cdata.weight_deadline) {
      cdata.weight_deadline = data.weight_deadline;
      data_changed = True;
   }
   if (data.weight_waiting_time != cdata.weight_waiting_time) {
      cdata.weight_waiting_time = data.weight_waiting_time;
      data_changed = True;
   }
   if (data.weight_urgency != cdata.weight_urgency) {
      cdata.weight_urgency = data.weight_urgency;
      data_changed = True;
   }
   if (data.weight_priority != cdata.weight_priority) {
      cdata.weight_priority = data.weight_priority;
      data_changed = True;
   }
   if (data.weight_ticket != cdata.weight_ticket) {
      cdata.weight_ticket = data.weight_ticket;
      data_changed = True;
   }

   if (strcmp(data.policy_hierarchy, cdata.policy_hierarchy)) {
      cdata.policy_hierarchy = data.policy_hierarchy;
      data.policy_hierarchy = NULL;
      data_changed = True;
   }
   XmtDialogSetDialogValues(tov_layout, &cdata);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonTOVUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonTOVUpdate");
   
   qmonTOVUpdateFill(w, NULL);
      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static int qmonTOVUpdateFill(Widget w, lList **alpp)
{
   lList *scl = NULL;
   lListElem *scep = NULL;
   
   DENTER(GUI_LAYER, "qmonTOVUpdateFill");
   /*
   ** set and get the sge conf list
   */
   if (qmonMirrorMultiAnswer(SC_T, alpp)) {
      DEXIT;
      return -1;
   }
   
   scl = qmonMirrorList(SGE_SC_LIST);
   scep = lFirst(scl);
   qmonCulltoTOVEntry(&cdata, scep);
   
   XmtDialogSetDialogValues(tov_layout, &cdata);
   
   DEXIT;
   return 0;
}   

/*-------------------------------------------------------------------------*/
static void qmonTOVApply(Widget w, XtPointer cld, XtPointer cad)
{
   lList *scl;
   lList *alp = NULL;
   lListElem *sep;

   DENTER(GUI_LAYER, "qmonTOVApply");
   
   /*
   ** set the entries in the cdata element into the cull
   ** element
   */
   qmonMirrorMultiAnswer(SC_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   scl = qmonMirrorList(SGE_SC_LIST);
   sep = lFirst(scl);
   if (sep) {
      lEnumeration *what;
      qmonTOVInput(w, cld, cad);
      qmonTOVEntryToCull(sep, &cdata);
      what = lWhat("%T(ALL)", SC_Type);
      alp = qmonModList(SGE_SC_LIST, qmonMirrorListRef(SGE_SC_LIST),
                  SC_halftime, &scl, NULL, what); 
      lFreeWhat(&what);
      
      qmonMessageBox(w, alp, 0);

      if (alp && lGetUlong(lFirst(alp), AN_status) != STATUS_OK)
         XmtMsgLinePrintf(tov_message, "%s", XmtLocalize(w, "Failure",
                                                         "Failure"));
      else
         XmtMsgLinePrintf(tov_message, "%s", XmtLocalize(w, "Success",
                                                         "Success"));
      XmtMsgLineClear(tov_message, DISPLAY_MESSAGE_DURATION); 

      data_changed = False;

      lFreeList(&alp);
   }
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Boolean qmonTOVEntryReset(tTOVEntry *tov_data)
{
   DENTER(GUI_LAYER, "qmonTOVEntryReset");

   if (!tov_data) {
      DEXIT;
      return False;
   }
   
   tov_data->total_share_tree_tickets = 0;
   tov_data->total_functional_tickets = 0;
   tov_data->override_tickets = 0;
   tov_data->share_override_ticket = False;
   tov_data->report_pjob_tickets = False;
   tov_data->share_functional_ticket = False;
   tov_data->max_pending_tasks_per_job = 0;
   tov_data->max_functional_jobs = 0;
   tov_data->weight_deadline = 0.0;
   tov_data->weight_waiting_time = 0.0;
   tov_data->weight_urgency = 0.0;
   tov_data->weight_priority = 0.0;
   tov_data->weight_ticket = 0.0;
   if (tov_data->policy_hierarchy) {
      sge_free(&(tov_data->policy_hierarchy));
   }
   tov_data->policy_hierarchy = NULL;
   
   DEXIT;
   return True;
}


/*-------------------------------------------------------------------------*/
static Boolean qmonCulltoTOVEntry(
tTOVEntry *tov_data,
lListElem *scep 
) {
   DENTER(GUI_LAYER, "qmonCulltoTOVEntry");

   if (!tov_data || !scep) {
      DEXIT;
      return False;
   }
   
   qmonTOVEntryReset(tov_data);

   tov_data->total_share_tree_tickets = lGetUlong(scep, SC_weight_tickets_share);
   tov_data->total_functional_tickets = lGetUlong(scep, SC_weight_tickets_functional);

   /*
   ** these values are no configuration values but calculated and
   ** made available for convenience
   */
   tov_data->override_tickets = lGetUlong(scep, SC_weight_tickets_override);
   tov_data->share_override_ticket = (Boolean)lGetBool(scep,
                                            SC_share_override_tickets);
   tov_data->report_pjob_tickets = (Boolean)lGetBool(scep,
                                            SC_report_pjob_tickets);
   tov_data->share_functional_ticket = (Boolean)lGetBool(scep,
                                            SC_share_functional_shares);
   tov_data->max_pending_tasks_per_job = (int)lGetUlong(scep,
                                            SC_max_pending_tasks_per_job);
   tov_data->max_functional_jobs = (int)lGetUlong(scep,
                                            SC_max_functional_jobs_to_schedule);
   tov_data->weight_deadline = lGetDouble(scep, SC_weight_deadline);
   tov_data->weight_waiting_time = lGetDouble(scep, SC_weight_waiting_time);
   tov_data->weight_urgency = lGetDouble(scep, SC_weight_urgency);
   tov_data->weight_priority = lGetDouble(scep, SC_weight_priority);
   tov_data->weight_ticket = lGetDouble(scep, SC_weight_ticket);
   tov_data->policy_hierarchy = sge_strdup(tov_data->policy_hierarchy,
                                    lGetString(scep, SC_policy_hierarchy));
   
   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonTOVEntryToCull(
lListElem *scep,
tTOVEntry *tov_data 
) {
   DENTER(GUI_LAYER, "qmonTOVEntryToCull");

   if (!tov_data || !scep) {
      DEXIT;
      return False;
   }


   lSetUlong(scep, SC_weight_tickets_share, tov_data->total_share_tree_tickets);
   lSetUlong(scep, SC_weight_tickets_functional, tov_data->total_functional_tickets);

   lSetBool(scep, SC_share_override_tickets, tov_data->share_override_ticket);
   lSetBool(scep, SC_report_pjob_tickets, tov_data->report_pjob_tickets);
   lSetBool(scep, SC_share_functional_shares, tov_data->share_functional_ticket);
   lSetUlong(scep, SC_max_pending_tasks_per_job, 
               (u_long32)tov_data->max_pending_tasks_per_job);
   lSetUlong(scep, SC_max_functional_jobs_to_schedule, 
               (u_long32)tov_data->max_functional_jobs);
   lSetDouble(scep, SC_weight_deadline, tov_data->weight_deadline);
   lSetDouble(scep, SC_weight_waiting_time, tov_data->weight_waiting_time);
   lSetDouble(scep, SC_weight_urgency, tov_data->weight_urgency);
   lSetDouble(scep, SC_weight_priority, tov_data->weight_priority);
   lSetDouble(scep, SC_weight_ticket, tov_data->weight_ticket);
   
   if (tov_data->policy_hierarchy && tov_data->policy_hierarchy[0] !='\0')
      lSetString(scep, SC_policy_hierarchy, tov_data->policy_hierarchy);
   else   
      lSetString(scep, SC_policy_hierarchy, "OFS");

   DEXIT;
   return True;
}

