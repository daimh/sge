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
#define QALTER_RUNNING


#include <stdio.h>
#include <stdlib.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/Icon.h>
#include <Xmt/Dialogs.h>
#include <Xmt/Chooser.h>
#include <Xmt/InputField.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_job.h"
#include "qmon_comm.h"
#include "qmon_jobcustom.h"
#include "qmon_appres.h"
#include "qmon_submit.h"
#include "qmon_timer.h"
#include "qmon_globals.h"
#include "qmon_browser.h"
#include "qmon_message.h"
#include "qmon_init.h"
#include "qmon_ticket.h"
#include "qmon_request.h"
#include "qmon_util.h"
#include "qmon_matrix.h"
#include "Matrix.h"
#include "Tab.h"

#include "uti/sge_signal.h"
#include "uti/sge_time.h"
#include "uti/sge_dstring.h"

#include "sched/sge_sched.h"
#include "sched/sge_job_schedd.h"
#include "sched/sge_schedd_text.h"
#include "sched/sgeee.h"
#include "sched/sge_support.h"

#include "sgeobj/sge_range.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_ja_task.h"

#include "gdi/sge_gdi_ctx.h"

#include "basis_types.h"

extern sge_gdi_ctx_class_t *ctx;

enum {
   JOB_DISPLAY_MODE_RUNNING,
   JOB_DISPLAY_MODE_PENDING
};

typedef struct _tAskHoldInfo {
   Boolean blocked;
   XmtButtonType button;
   Widget AskHoldDialog;
   Widget AskHoldFlags;
   Widget AskHoldTasks;
} tAskHoldInfo;
   

static Widget qmon_job = 0;
static Widget job_running_jobs = 0;
static Widget job_pending_jobs = 0;
static Widget current_matrix = 0;
static Widget job_zombie_jobs = 0;
static Widget job_customize = 0;
static Widget job_schedd_info = 0;
static Widget job_delete = 0;
static Widget job_select_all = 0;
static Widget job_reschedule = 0;
static Widget job_qalter = 0;
static Widget job_suspend = 0;
static Widget job_unsuspend = 0;
static Widget job_hold = 0;
static Widget job_error = 0;
static Widget job_priority = 0;
static Widget force_toggle = 0;


/*-------------------------------------------------------------------------*/
static void qmonCreateJobControl(Widget parent);
static void qmonJobToMatrix(Widget w, lListElem *jep, lListElem *jat, lList *jal, int mode, int row);
/* static void qmonSetMatrixLabels(Widget w, lDescr *dp); */
static void qmonJobFolderChange(Widget w, XtPointer cld, XtPointer cad);
static void qmonDeleteJobCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonSelectAllJobCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobPopdown(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobStartUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobStopUpdate(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobHandleEnterLeave(Widget w, XtPointer cld, XEvent *ev, Boolean *ctd);
static void qmonJobShowBrowserInfo(dstring *info, lListElem *jep);
static void qmonJobChangeState(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobPriority(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobHold(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobQalter(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobNoEdit(Widget w, XtPointer cld, XtPointer cad);
static Pixel qmonJobStateToColor(Widget w, lListElem *jep);
static Boolean qmonDeleteJobForMatrix(Widget w, Widget matrix, lList **local);
static lList* qmonJobBuildSelectedList(Widget matrix, lDescr *dp, int nm);
/* static void qmonResizeCB(Widget w, XtPointer cld, XtPointer cad); */
static Boolean AskForHold(Widget w, Cardinal *hold, dstring *dyn_tasks);
static void AskHoldCancelCB(Widget w, XtPointer cld, XtPointer cad);
static void AskHoldOkCB(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobScheddInfo(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobSort(Widget w, XtPointer cld, XtPointer cad);
/*-------------------------------------------------------------------------*/
static int show_info_for_jobs(lList *jid_list, FILE *fp, lList **alpp, dstring *sb);
static int show_info_for_job(FILE *fp, lList **alpp, dstring *sb);

static int field_sort_by;
static int field_sort_direction;
static int jobnum_sort_direction;

/*-------------------------------------------------------------------------*/
/* P U B L I C                                                             */
/*-------------------------------------------------------------------------*/
void qmonJobPopup(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonJobPopup");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_job) {
      qmonMirrorMultiAnswer(JOB_T | CQUEUE_T | EXECHOST_T | CENTRY_T | ZOMBIE_T,
                            &alp);
      if (alp) {
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         /* set normal cursor */
         XmtDisplayDefaultCursor(w);
         DEXIT;
         return;
      }
         
      qmonCreateJobControl(AppShell);
      /*
      ** create the job customize dialog
      */
      qmonCreateJCU(qmon_job, NULL);
      /* 
      ** set the close button callback 
      ** set the icon and iconName
      */
      XmtCreatePixmapIcon(qmon_job, qmonGetIcon("toolbar_job"), None);
      XtVaSetValues(qmon_job, XtNiconName, "qmon:Job Control", NULL);
      XmtAddDeleteCallback(qmon_job, XmDO_NOTHING, qmonJobPopdown,  NULL);
      XtAddEventHandler(qmon_job, StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
      XtAddEventHandler(qmon_job, StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);
   }

   /* updateJobListCB(qmon_job, NULL, NULL); */

   xmui_manage(qmon_job);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonJobReturnMatrix(
Widget *run_m,
Widget *pen_m,
Widget *zombie_m 
) {
   DENTER(GUI_LAYER, "qmonJobReturnMatrix");

   *run_m = job_running_jobs;
   *pen_m = job_pending_jobs;
   *zombie_m = job_zombie_jobs;

   DEXIT;
}

/*-------------------------------------------------------------------------*/
void updateJobListCB(Widget w, XtPointer cld, XtPointer cad)
{
   lList *alp = NULL;

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   qmonMirrorMultiAnswer(JOB_T|CQUEUE_T|EXECHOST_T|ZOMBIE_T|USERSET_T|PROJECT_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      return;
   }

   updateJobList();

   /* set default cursor */
   XmtDiscardButtonEvents(w);
   XmtDisplayDefaultCursor(w);
}


/*-------------------------------------------------------------------------*/
/* P R I V A T E                                                           */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static void qmonJobPopdown(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonJobPopdown");

   xmui_unmanage(qmon_job);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobStartUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonJobStartUpdate");
  
   qmonTimerAddUpdateProc(JOB_T|ZOMBIE_T|CENTRY_T, "updateJobList", updateJobList);
   qmonStartTimer(JOB_T | CQUEUE_T | EXECHOST_T | ZOMBIE_T | CENTRY_T);
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonJobStopUpdate(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonJobStopUpdate");
  
   qmonStopTimer(JOB_T | CQUEUE_T | EXECHOST_T | CENTRY_T | ZOMBIE_T);
   qmonTimerRmUpdateProc(JOB_T|ZOMBIE_T|CENTRY_T, "updateJobList");
   
   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonCreateJobControl(
Widget parent 
) {
   Widget job_submit, job_update, job_done, job_tickets,
          job_main_link, job_pending, job_folder;
   static Widget pw[2];
   static Widget rw[2];
   
   DENTER(GUI_LAYER, "qmonCreateJobControl");

   qmon_job = XmtBuildQueryToplevel( parent, 
                                     "qmon_job",
                                     "job_running_jobs", &job_running_jobs,
                                     "job_pending_jobs", &job_pending_jobs,
                                     "job_zombie_jobs", &job_zombie_jobs,
                                     "job_delete", &job_delete,
                                     "job_select_all", &job_select_all,
                                     "job_error", &job_error,
                                     "job_qalter", &job_qalter,
                                     "job_priority", &job_priority,
                                     "job_update", &job_update,
                                     "job_suspend", &job_suspend,
                                     "job_reschedule", &job_reschedule,
                                     "job_unsuspend", &job_unsuspend,
                                     "job_customize", &job_customize,
                                     "job_submit", &job_submit,
                                     "job_tickets", &job_tickets,
                                     "job_done", &job_done,
                                     "job_hold", &job_hold,
                                     "job_schedd_info", &job_schedd_info,
                                     "job_main_link", &job_main_link,
                                     "job_folder", &job_folder,
                                     "job_pending", &job_pending,
                                     "job_force",  &force_toggle,
                                     NULL);
   pw[0] = job_running_jobs;
   pw[1] = NULL;
   rw[0] = job_pending_jobs;
   rw[1] = NULL;


   XtAddCallback(job_tickets, XmNactivateCallback,
                  qmonPopupTicketOverview, NULL);

   /* start the needed timers and the corresponding update routines */
   XtAddCallback(qmon_job, XmNpopupCallback, 
                     qmonJobStartUpdate, NULL);
   XtAddCallback(qmon_job, XmNpopdownCallback,
                     qmonJobStopUpdate, NULL);
                     
   XtAddCallback(job_folder, XmNvalueChangedCallback, 
                     qmonJobFolderChange, NULL);
   XmTabSetTabWidget(job_folder, job_pending, True);

   XtAddCallback(job_delete, XmNactivateCallback, 
                     qmonDeleteJobCB, NULL);
   XtAddCallback(job_select_all, XmNactivateCallback, 
                     qmonSelectAllJobCB, NULL);
   XtAddCallback(job_update, XmNactivateCallback, 
                     updateJobListCB, NULL);
   XtAddCallback(job_customize, XmNactivateCallback,  
                     qmonPopupJCU, NULL); 
   XtAddCallback(job_submit, XmNactivateCallback, 
                     qmonSubmitPopup, NULL);
   XtAddCallback(job_priority, XmNactivateCallback, 
                     qmonJobPriority, NULL);
   XtAddCallback(job_hold, XmNactivateCallback, 
                     qmonJobHold, NULL);
   XtAddCallback(job_schedd_info, XmNactivateCallback, 
                     qmonJobScheddInfo, NULL);
   XtAddCallback(job_qalter, XmNactivateCallback, 
                     qmonJobQalter, NULL);
   XtAddCallback(job_done, XmNactivateCallback, 
                     qmonJobPopdown, NULL);
   XtAddCallback(job_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
/*    XtAddCallback(job_running_jobs, XmNresizeColumnCallback,  */
/*                      qmonResizeCB, NULL); */
   XtAddCallback(job_running_jobs, XmNenterCellCallback, 
                     qmonJobNoEdit, NULL);
   XtAddCallback(job_pending_jobs, XmNenterCellCallback, 
                     qmonJobNoEdit, NULL);
   XtAddCallback(job_running_jobs, XmNselectCellCallback, 
                     qmonMatrixSelect, (XtPointer) rw);
   XtAddCallback(job_pending_jobs, XmNselectCellCallback, 
                     qmonMatrixSelect, (XtPointer) pw);
   XtAddCallback(job_pending_jobs, XmNlabelActivateCallback, 
                     qmonJobSort, NULL);
   XtAddCallback(job_running_jobs, XmNlabelActivateCallback, 
                     qmonJobSort, NULL);
   XtAddCallback(job_zombie_jobs, XmNlabelActivateCallback, 
                     qmonJobSort, NULL);
   XtAddCallback(job_suspend, XmNactivateCallback, 
                     qmonJobChangeState, (XtPointer)QI_DO_SUSPEND);
   XtAddCallback(job_unsuspend, XmNactivateCallback, 
                     qmonJobChangeState, (XtPointer)QI_DO_UNSUSPEND);
   XtAddCallback(job_error, XmNactivateCallback, 
                     qmonJobChangeState, (XtPointer)QI_DO_CLEARERROR);
   XtAddCallback(job_reschedule, XmNactivateCallback, 
                     qmonJobChangeState, (XtPointer)QI_DO_RESCHEDULE);

   /* Event Handler to display additional job info */
   XtAddEventHandler(job_running_jobs, PointerMotionMask, 
                     False, qmonJobHandleEnterLeave,
                     NULL); 
   XtAddEventHandler(job_pending_jobs, PointerMotionMask, 
                     False, qmonJobHandleEnterLeave,
                     NULL); 

   current_matrix = job_pending_jobs;
   /* initialising sort order to decreasing priority then increasing job number */
   field_sort_by = SGEJ_priority;
   field_sort_direction = SGEJ_sort_decending;
   jobnum_sort_direction = SGEJ_sort_ascending; 

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonJobToMatrix(
Widget w,
lListElem *job,
lListElem *jat,
lList *tasks,
int mode,
int row                                                                       
) {
   int rows, columns;
   int col;
   String *rowa = NULL; 
   Pixel color;
   
   DENTER(GUI_LAYER, "qmonJobToMatrix");

   rows = XbaeMatrixNumRows(w);
   columns = XbaeMatrixNumColumns(w);

   /*
   ** running jobs
   */
   if (mode == JOB_DISPLAY_MODE_RUNNING) {
      rowa = PrintJobField(job, jat, NULL, columns);
      if (row < rows)
         XbaeMatrixDeleteRows(w, row, 1);
      XbaeMatrixAddRows(w, row, rowa, NULL, NULL, 1);
      /*
      ** do show the different job states we show the job id in a different           ** color
      */
      color = qmonJobStateToColor(w, jat);
      XbaeMatrixSetRowColors(w, row, &color, 1);
   }

   /*
   ** pending jobs
   */
   if (mode == JOB_DISPLAY_MODE_PENDING) {
      if (row < rows)
         XbaeMatrixDeleteRows(w, row, 1);
      rowa = PrintJobField(job, jat, tasks, columns);
      XbaeMatrixAddRows(w, row, rowa, NULL, NULL, 1);
      /*
      ** do show the different job states we show the job id in a different           ** color
      */

      if (!jat) {
         lListElem *first_elem = lFirst(tasks);

         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(tasks);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(tasks, NULL);

            jat = job_get_ja_task_template(job, task_id);       
         }
      }
      color = qmonJobStateToColor(w, jat);
      XbaeMatrixSetRowColors(w, row, &color, 1);
   }

   /*
   ** free row array
   */
   if (rowa) {
      for (col=0; col<columns; col++)
         XtFree((char*)rowa[col]);
      XtFree((char*)rowa);
   }
                      
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Pixel qmonJobStateToColor(
Widget w,
lListElem *jat 
) {
   Pixel color;
   static Pixel fg = 0;
   int state;
   int hold;

   DENTER(GUI_LAYER, "qmonJobStateToColor");

   if (!fg)
      XtVaGetValues(w, XmNforeground, &fg, NULL);

   if (!jat) {
      DEXIT;
      return fg;
   }
      

   state = (int)lGetUlong(jat, JAT_state);
   hold = (int)lGetUlong(jat, JAT_hold);

   /*
   ** the order is important
   */

   color = fg;

   if ((state & JSUSPENDED_ON_SUBORDINATE) ||
       (state & JSUSPENDED_ON_SLOTWISE_SUBORDINATE)) {
      color = JobSosPixel;
   }

   if (state & JSUSPENDED)
      color = JobSuspPixel;

   if (state & JDELETED)
      color = JobDelPixel;

   if (hold)
      color = JobHoldPixel;

   if (state & JERROR)
      color = JobErrPixel;

   DEXIT;
   return color;

}

/*-------------------------------------------------------------------------*/
static void qmonJobNoEdit(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   XbaeMatrixEnterCellCallbackStruct *cbs =
         (XbaeMatrixEnterCellCallbackStruct*) cad;

   DENTER(GUI_LAYER, "qmonJobNoEdit");

   cbs->doit = False;
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobSort(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   XbaeMatrixLabelActivateCallbackStruct *cbs = 
            (XbaeMatrixLabelActivateCallbackStruct *) cad;
   /* SGEJ_state is not a typo - PrintStatus uses both JAT_status and JAT_state */
   int column_nm[] = {SGEJ_job_number, SGEJ_priority, SGEJ_job_name, SGEJ_owner, SGEJ_state, SGEJ_master_queue};
   /* 
   ** Mapping the columns to these internal resources:
   ** JB_job_number    (Ulong)
   ** JAT_prio         (Double)
   ** JB_job_name      (String)
   ** JB_owner         (String)
   ** JAT_state        (Ulong)
   ** JAT_master_queue (String)
   */
   DENTER(GUI_LAYER, "qmonJobSort");
 
   DPRINTF(("JobSort = cbs->column = %d\n", cbs->column));
   
   /* not coping beyond 6th column */
   if ( cbs->column > 5) {
      DEXIT;
      return;
   }
   /* 
   ** Here is what we are going to do for sorting:
   ** if the user clicks on a column, we'll sort ascending,
   ** doing a 2nd time toggles to decending; except
   ** we always toggle on the job number the first time
   ** it is selected from a different column, since it is
   ** always a secondary sort key and if it is selected
   ** the user most likely wants a toggle.
   */
      
   if (column_nm[cbs->column] == field_sort_by) {
      /* toggling sort order */
      field_sort_direction = !field_sort_direction;     
   } else {
      field_sort_by = column_nm[cbs->column];
      field_sort_direction = SGEJ_sort_ascending;
      /* switching from another field to job number also toggles */
      if (field_sort_by == SGEJ_job_number) {
         field_sort_direction = !jobnum_sort_direction;
      }
   }

   /* recording the last chosen job number sort direction */
   if (field_sort_by == SGEJ_job_number) {
      jobnum_sort_direction = field_sort_direction;
   }
   updateJobList();
    
   DEXIT;
}

/*----------------------------------------------------------------------------*/
void updateJobList(void)
{
   lList *jl = NULL;
   lList *ql = NULL;
   lListElem *jep = NULL;
   lListElem *qep = NULL;
   lCondition *where_run = NULL;
   lCondition *where_unfinished = NULL;
   lCondition *where_exiting = NULL;
   lEnumeration *what = NULL;
   lCondition *where_no_template = NULL;
   lCondition *where_notexiting = NULL;
   lEnumeration *what_queue = NULL;
   u_long32 jstate;
   StringConst qnm;
   lList *ehl = NULL;
   lList *cl = NULL;
   lList *ol = NULL;
   lList *rl = NULL;
   lList *zl = NULL;
   static Boolean filter_on = False;
   int row = 0, pow = 0, zow = 0;
   int current_rows = 0, desired_rows = 0; /* used at the end to shrink tabs */
         
   DENTER(GUI_LAYER, "updateJobList");
   
   /*
   ** has dialog been created
   */
   if (!qmon_job) {
      DEXIT;
      return;
   }
   
   /*
   ** display only the unfinished jobs
   */
   where_unfinished = lWhere("%T(!(%I->(%I m= %u)))", 
                                 JB_Type, JB_ja_tasks, JAT_status, JFINISHED);
   what = lWhat("%T(ALL)", JB_Type);
   where_no_template = lWhere("%T(%I != %s)", CQ_Type, CQ_name, "template");
   what_queue = lWhat("%T(ALL)", CQ_Type);
   where_notexiting = lWhere("%T(!(%I m= %u))", JAT_Type, JAT_status, JFINISHED);
   where_run = lWhere("%T((%I m= %u || %I m= %u) && (!(%I m= %u)))",
                      JAT_Type, JAT_status, JRUNNING, JAT_status, JTRANSFERING,
                      JAT_state, JEXITING);
   where_exiting = lWhere("%T(!(%I m= %u))", JAT_Type, JAT_status, JFINISHED);
 
   jl = lSelect("jl", qmonMirrorList(SGE_JB_LIST), where_unfinished, what);

   ql = lSelect("ql", qmonMirrorList(SGE_CQ_LIST), where_no_template, 
                  what_queue);
   ehl = qmonMirrorList(SGE_EH_LIST);
   cl = qmonMirrorList(SGE_CE_LIST);
   /* don't free rl, ol they are maintained in qmon_jobcustom.c */
   rl = qmonJobFilterResources();
   ol = qmonJobFilterOwners();


   if (rl || ol) {
      if (!filter_on) {
         setButtonLabel(job_customize, "@{Customize +}");
         filter_on = True;
      }
   }
   else {
      if (filter_on) {
         setButtonLabel(job_customize, "@{Customize}");
         filter_on = False;
      }
   }
   
   /*
   ** match queues to request_list
   */
   if (rl) {
      match_queue(&ql, rl, cl, ehl);
   }   

#if 0      
   {
      lListElem* ep;
      printf("Queues\n");
      for_each(ep, ql) {
         printf("Q: %s\n", lGetString(ep, CQ_name));
      }
   }
   {
      lListElem* ep;
      printf("All Jobs\n");
      for_each(ep, jl) {
         printf("J: %d\n", (int)lGetUlong(ep, JB_job_number));
      }
   }
#endif

   match_job(&jl, ol, ql, cl, ehl, rl); 

#if 0      
   {
      lListElem* ep;
      printf("Matched Jobs\n");
      for_each(ep, jl) {
         printf("J: %d\n", (int)lGetUlong(ep, JB_job_number));
      }
   }
#endif

   /*
   ** we need the ql no longer, free it
   */
   lFreeList(&ql);

   /*
   ** sort the jobs according to priority
   */
   if (lGetNumberOfElem(jl)>0 ) {
      sgeee_sort_jobs_by(&jl, field_sort_by, field_sort_direction, jobnum_sort_direction);
   }
 
   /*
   ** loop over the jobs and the tasks
   */
   XbaeMatrixDisableRedisplay(job_running_jobs);
   XbaeMatrixDisableRedisplay(job_pending_jobs);
   XbaeMatrixDisableRedisplay(job_zombie_jobs);
 
   /*
   ** reset matrices
   */
   XtVaSetValues( job_running_jobs,
                  XmNcells, NULL,
                  NULL);
   XtVaSetValues( job_pending_jobs,
                  XmNcells, NULL,
                  NULL);
   /*
   ** update the job entries
   */
   for_each (jep, jl) {
      lList *rtasks = lCopyList("rtasks", lGetList(jep, JB_ja_tasks));
      lList *etasks = NULL;
      lList *ptasks = NULL;
      lListElem *jap;
      int tow = 0;
      /*
      ** split into running, pending and exiting tasks 
      */
      lSplit(&rtasks, &ptasks, "rtasks", where_run);
      lSplit(&ptasks, &etasks, "etasks", where_exiting);

#if 0 /* EB: DEBUG */
      printf("========> jep\n");
      lWriteElemTo(jep, stdout);
      printf("========> where_run\n"); 
      lWriteWhereTo(where_run, stdout); 
      printf("========> rtasks\n"); 
      lWriteListTo(rtasks, stdout); 
      printf("========> ptasks\n"); 
      lWriteListTo(ptasks, stdout); 
      printf("========> etasks\n");
      lWriteListTo(etasks, stdout);
#endif
      /*
      ** for running tasks we have to set the suspend_on_subordinate flag
      ** if the granted queues are suspended the jat_state field must set
      ** the jsuspended_on_subordinate
      */
      if (lGetNumberOfElem(rtasks) > 0) {
         int tow = 0;
#if 0      
   {
      lListElem* ep;
      printf("Running Tasks\n");
      for_each(ep, rtasks) {
         printf("T: %d (%d)\n", (int)lGetUlong(ep, JAT_task_number), 
               (int)lGetUlong(ep, JAT_suitable));
      }
   }
#endif

         for_each(jap, rtasks) {
            ql = lGetList(jap, JAT_granted_destin_identifier_list);
            if (ql) {
               lList *ehl = qmonMirrorList(SGE_EH_LIST);
               lList *cl = qmonMirrorList(SGE_CE_LIST);
               qnm = lGetString(lFirst(ql), JG_qname);
               qep = cqueue_list_locate_qinstance(qmonMirrorList(SGE_CQ_LIST), qnm);
               if (qep) {
                  lList *st = lGetList(qep, QU_suspend_thresholds);
                  if ( sge_load_alarm(NULL, qep, st, ehl, cl, NULL, false)) {
                     jstate = lGetUlong(jap, JAT_state);
                     jstate &= ~JRUNNING; /* unset bit jrunning */
                     /* set bit jsuspended_on_subordinate */
                     jstate |= JSUSPENDED_ON_SUBORDINATE;
                     lSetUlong(jap, JAT_state, jstate);
                  }
               }
            }

            if (lGetUlong(jap, JAT_suitable)) {
               qmonJobToMatrix(job_running_jobs, jep, jap, NULL,
                                 JOB_DISPLAY_MODE_RUNNING, row + tow);
               tow++;                                                            
            }
         }
         lFreeList(&rtasks);
         row += tow;
      }
      /*
      ** pending jobs
      */
      if (lGetNumberOfElem(ptasks) > 0) {
         if (qmonJobFilterArraysCompressed()) {
            lList *task_group;

            lSplit(&ptasks, NULL, NULL, where_notexiting);
            while (( task_group = ja_task_list_split_group(&ptasks))) {
               qmonJobToMatrix(job_pending_jobs, jep, NULL, task_group,
                              JOB_DISPLAY_MODE_PENDING, pow);
               lFreeList(&task_group);
               pow++;
            }
         } else {
            for_each(jap, ptasks) {
               qmonJobToMatrix(job_pending_jobs, jep, jap, NULL,
                                 JOB_DISPLAY_MODE_RUNNING, pow + tow);
               tow++;
            }
            pow++;
         }
          lFreeList(&ptasks);
      }
      if (lGetList(jep, JB_ja_n_h_ids) || lGetList(jep, JB_ja_u_h_ids) ||
          lGetList(jep, JB_ja_o_h_ids) || lGetList(jep, JB_ja_s_h_ids) ||
          lGetList(jep, JB_ja_a_h_ids)) {
         lList *range_list[16];         /* RN_Type */
         u_long32 hold_state[16];
         int i;

         job_create_hold_id_lists(jep, range_list, hold_state);
         for (i = 0; i <= 15; i++) {
            if (range_list[i] != NULL) {
               lList *task_ids = range_list[i]; 

               if (task_ids != NULL) {
                  tow = 0;
                  if (qmonJobFilterArraysCompressed()) {
                     qmonJobToMatrix(job_pending_jobs, jep, NULL, task_ids,
                                       JOB_DISPLAY_MODE_PENDING, pow + tow);
                     tow++;
                  } else {
                     lListElem *range = NULL;   /* RN_Type */
                     u_long32 task_id;

                     for_each(range, task_ids) {
                        for(task_id = lGetUlong(range, RN_min);
                            task_id <= lGetUlong(range, RN_max);
                            task_id += lGetUlong(range, RN_step)) {     
                           jap = job_get_ja_task_template(jep, task_id);
                           qmonJobToMatrix(job_pending_jobs, jep, jap, 
                                           NULL, JOB_DISPLAY_MODE_RUNNING,
                                           pow + tow);
                           tow++;
                        }
                     }
                  }
                  pow += tow;
               }
            }
         }
         job_destroy_hold_id_lists(range_list);
      }
      lFreeList(&etasks);
   }

   /*
   ** free the where/what
   */
   lFreeWhere(&where_run);
   lFreeWhere(&where_exiting);
   lFreeWhere(&where_notexiting);
   lFreeWhere(&where_no_template);
   lFreeWhere(&where_unfinished);
   lFreeWhat(&what);
   lFreeWhat(&what_queue);
   lFreeList(&jl);

   /*
   ** update the zombie job entries
   */
   zl = qmonMirrorList(SGE_ZOMBIE_LIST);
   sgeee_sort_jobs_by(&zl,  field_sort_by, field_sort_direction, jobnum_sort_direction);

/* lWriteListTo(zl, stdout); */

   XtVaSetValues(job_zombie_jobs, XmNcells, NULL, NULL);

   for_each (jep, zl) {
      lList *task_ids = lGetList(jep, JB_ja_z_ids);
      int tow = 0;

      if (task_ids != NULL) {
         tow = 0;
         if (qmonJobFilterArraysCompressed()) {
            qmonJobToMatrix(job_zombie_jobs, jep, NULL, task_ids,
                              JOB_DISPLAY_MODE_PENDING, zow + tow);
            tow++;
         } else {
            lListElem *range = NULL;   /* RN_Type */
            u_long32 task_id;

            for_each(range, task_ids) {
               for(task_id = lGetUlong(range, RN_min);
                   task_id <= lGetUlong(range, RN_max);
                   task_id += lGetUlong(range, RN_step)) {     
                  lListElem *jap;

                  jap = job_get_ja_task_template(jep, task_id);
                  qmonJobToMatrix(job_zombie_jobs, jep, jap, 
                                  NULL, JOB_DISPLAY_MODE_RUNNING,
                                  zow + tow);
                  tow++;
               }
            }
         }
         zow += tow;
      }
   }

   /* shrinking excessive vertical size of tabs from previous runs of updateJobList() */  
   current_rows = XbaeMatrixNumRows(job_running_jobs);
   desired_rows = MAX(row, 20);
   if (current_rows > desired_rows) {
     XbaeMatrixDeleteRows(job_running_jobs, desired_rows, current_rows - desired_rows);
   }   
   current_rows = XbaeMatrixNumRows(job_pending_jobs);
   desired_rows = MAX(pow, 20);
   if (current_rows > desired_rows) {
     XbaeMatrixDeleteRows(job_pending_jobs, desired_rows, current_rows - desired_rows);
   }
   current_rows = XbaeMatrixNumRows(job_zombie_jobs);
   desired_rows = MAX(zow, 20);
   if (current_rows > desired_rows) {
     XbaeMatrixDeleteRows(job_zombie_jobs, desired_rows, current_rows - desired_rows);
   }

   XbaeMatrixEnableRedisplay(job_running_jobs, True);
   XbaeMatrixEnableRedisplay(job_pending_jobs, True);
   XbaeMatrixEnableRedisplay(job_zombie_jobs, True);
 
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean qmonDeleteJobForMatrix(
Widget w,
Widget matrix,
lList **local 
) {
   int i;
   int rows;
   lList *jl = NULL;
   lList *sl = NULL;
   lListElem *jep = NULL;
   lList *alp = NULL;
   char *str;
   int force;
   static lList *user_list = NULL;

   DENTER(GUI_LAYER, "qmonDeleteJobForMatrix");

/*
   if (!user_list) {
      lAddElemStr(&user_list, ST_name, "*", ST_Type);
   }   
*/   
   force = XmToggleButtonGetState(force_toggle);

   /* 
   ** create delete job list 
   */
   rows = XbaeMatrixNumRows(matrix);
   for (i=0; i<rows; i++) {
      /* is this row selected */ 
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         str = XbaeMatrixGetCell(matrix, i, 0);
         if ( str && *str != '\0' ) { 
            DPRINTF(("JobTask(s) to delete: %s\n", str));
            /* TODO SG: check, if this is correct */
            if (sge_parse_jobtasks(&jl, &jep, str, &alp, false, NULL) == -1) {
               qmonMessageBox(w, alp, 0);
               DEXIT;
               return False;
            }
            lSetUlong(jep, ID_force, force);
            lSetList(jep, ID_user_list, lCopyList("", user_list));
         }
      }
   }

   if (jl) {
      /*
      ** FIXME number of jobs that shall be deleted in one sweep
      */
      int job_max = 100;
      int num_jobs = lGetNumberOfElem(jl);
      int count = 0;

      while (num_jobs) {
         int j=0;
         lListElem *ep, *next;
         count++;
         for (ep = lFirst(jl); ep; ep = next, j++) {
            next=ep->next;
            if (!sl) {
               sl = lCreateList("sl", ID_Type);
            }   
            ep = lDechainElem(jl, ep);
            lAppendElem(sl, ep);
            if (j==job_max) {
               break;
            }   
         }
         num_jobs = lGetNumberOfElem(jl);
         alp = qmonDelList(SGE_JB_LIST, local, 
                           ID_str, &sl, NULL, NULL);
         qmonMessageBox(w, alp, 0);
         lFreeList(&alp);
         lFreeList(&sl);
      }

      updateJobList();
      XbaeMatrixDeselectAll(matrix);

      lFreeList(&jl);
   } 
#if 0   
   else {
      qmonMessageShow(w, True, "@{There are no jobs selected !}");
      DEXIT;
      return False;
   }
#endif

   DEXIT;
   return True;
}

/*-------------------------------------------------------------------------*/
static void qmonSelectAllJobCB(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   
   DENTER(GUI_LAYER, "qmonSelectAllJobCB");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   XbaeMatrixSelectAll(current_matrix);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonDeleteJobCB(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   Boolean status;
   
   DENTER(GUI_LAYER, "qmonDeleteJobCB");

   /*
   ** we have to delete the running jobs independently
   ** they are not removed from the local list only their
   ** state is set to JDELETED, this is marked with a NULL pointer
   ** instead of a valid local list address
   */

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   status = qmonDeleteJobForMatrix(w, job_running_jobs, NULL);
   if (status)
      XbaeMatrixDeselectAll(job_running_jobs);

   status = qmonDeleteJobForMatrix(w, job_pending_jobs,
                           qmonMirrorListRef(SGE_JB_LIST));
   if (status)
      XbaeMatrixDeselectAll(job_pending_jobs);

   updateJobListCB(w, NULL, NULL);

   /* set normal cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobChangeState(
Widget w,
XtPointer cld,
XtPointer cad 
) {
   lList *jl = NULL;
   lList *rl = NULL;
   lList *alp = NULL;
   int force = 0;
   int action = (int)(long)cld;
   Widget force_toggle;
   
   DENTER(GUI_LAYER, "qmonJobChangeState");

   if (action == QI_DO_CLEARERROR) {
      rl = qmonJobBuildSelectedList(job_running_jobs, ST_Type, ST_name);
      
      jl = qmonJobBuildSelectedList(job_pending_jobs, ST_Type, ST_name);

      if (!jl && rl) {
         jl = rl;
      }
      else if (rl) {
         lAddList(jl, &rl);
      }
   }
   else {
      force_toggle = XmtNameToWidget(w, "*job_force");

      force = XmToggleButtonGetState(force_toggle);
      /*
      ** state changes only for running jobs
      */
      jl = qmonJobBuildSelectedList(job_running_jobs, ST_Type, ST_name);
   }

   if (jl) {

      alp = qmonChangeStateList(SGE_JB_LIST, jl, force, action); 

      qmonMessageBox(w, alp, 0);

      updateJobList();
      XbaeMatrixDeselectAll(job_running_jobs);
      XbaeMatrixDeselectAll(job_pending_jobs);

      lFreeList(&jl);
      lFreeList(&alp);
   }
   else {
      qmonMessageShow(w, True, "@{There are no jobs selected !}");
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
/* descriptor must contain JB_job_number at least                          */
/*-------------------------------------------------------------------------*/
static lList* qmonJobBuildSelectedList(matrix, dp, nm)
Widget matrix;
lDescr *dp;
int nm;
{
   int i;
   int rows;
   lList *jl = NULL;
   lListElem *jep = NULL;
   String str;

   DENTER(GUI_LAYER, "qmonJobBuildSelectedList");

   if (nm != JB_job_number && nm != ST_name) {
      DEXIT;
      return NULL;
   }

   rows = XbaeMatrixNumRows(matrix);
      
   for (i=0; i<rows; i++) {
      /* is this row selected */ 
      if (XbaeMatrixIsRowSelected(matrix, i)) {
         str = XbaeMatrixGetCell(matrix, i, 0);
         DPRINTF(("Job to alter: %s\n", str));
         /*
         ** list with describtion job id's and the priority value 
         */
         if ( str && (*str != '\0') ) { 
            switch (nm) { 
               case JB_job_number:
                  {
                     u_long32 start = 0, end = 0, step = 0;
                     lListElem *idp = NULL;
                     lList *ipp = NULL;
                     lList *jat_list = NULL;
                     lList *alp = NULL;
                     /* TODO: SG: check, if this is correct */
                     if (sge_parse_jobtasks(&ipp, &idp, str, &alp, false, NULL) == -1) {
                        lFreeList(&alp);
                        DEXIT;
                        return NULL;
                     }
                     lFreeList(&alp);

                     if (ipp) {
                        for_each(idp, ipp) {
                           lListElem *ip;

                           jep = lAddElemUlong(&jl, JB_job_number, 
                                    atol(lGetString(idp, ID_str)), dp);
                           lSetList(jep, JB_ja_structure, 
                                 lCopyList("", lGetList(idp, ID_ja_structure)));
                           job_get_submit_task_ids(jep, &start, &end, &step); 
                           for_each(ip, lGetList(idp, ID_ja_structure)) {
                              start = lGetUlong(ip, RN_min);
                              end = lGetUlong(ip, RN_max);
                              step = lGetUlong(ip, RN_step);
                              for (; start<=end; start+=step) {
                                 lAddElemUlong(&jat_list, JAT_task_number, 
                                                   start, JAT_Type);
                              }
                           }
                        }
                        if (lGetNumberOfElem(jat_list) == 0) {
                           lAddElemUlong(&jat_list, JAT_task_number, 
                                         1, JAT_Type);
                        }
                        lSetList(jep, JB_ja_tasks, jat_list);
                        lFreeList(&ipp);
                     }
                  }
                  break;
               case ST_name:
                  jep = lAddElemStr(&jl, ST_name, str, dp);
                  lSetList(jep, JB_ja_tasks, NULL);
                  break;
            }
         }
      }
   }

   DEXIT;
   return jl;
}

/*-------------------------------------------------------------------------*/
static void qmonJobPriority(Widget w, XtPointer cld, XtPointer cad)
{
   lList *jl = NULL;
   lList *rl = NULL;
   Boolean status_ask = False;
   int new_priority = 0;

   lDescr prio_descr[] = {
      {JB_job_number, lUlongT | CULL_IS_REDUCED, NULL},
      {JB_ja_tasks, lListT | CULL_IS_REDUCED, NULL},
      {JB_ja_structure, lListT | CULL_IS_REDUCED, NULL},
      /* optional fields */
      {JB_priority, lUlongT | CULL_IS_REDUCED, NULL},
      {NoName, lEndT | CULL_IS_REDUCED, NULL}
   };
   
   DENTER(GUI_LAYER, "qmonJobPriority");
   
   rl = qmonJobBuildSelectedList(job_running_jobs, prio_descr, JB_job_number);
   
   /*
   ** set priority works only for pending jobs in SGE mode
   */
   jl = qmonJobBuildSelectedList(job_pending_jobs, prio_descr, JB_job_number);

   if (!jl && rl) {
      jl = rl;
   } else if (rl) {
      lAddList(jl, &rl);
   }
   
   if (jl) {
      status_ask = XmtAskForInteger(w, NULL, 
                        "@{Enter a new priority (-1023 to 1024) for the selected jobs}", 
                        &new_priority, -1023, 1024, NULL); 
   } else {
      qmonMessageShow(w, True, "@{There are no jobs selected !}");
   }

   if (jl && status_ask) {
      lList *alp = NULL;
      lListElem *jep = NULL;

      for_each(jep, jl) {
         lSetUlong(jep, JB_priority, BASE_PRIORITY + new_priority);
      }
      alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_MOD, &jl, NULL, NULL, false); 
   
      qmonMessageBox(w, alp, 0);

      lFreeList(&alp);

      /*
      ** we use the callback to get the new job list from master
      ** cause the sge_gdi doesn't change the local list
      ** so updateJobList() is not sufficient
      */
      updateJobListCB(w, NULL, NULL);
      XbaeMatrixDeselectAll(job_pending_jobs);
      XbaeMatrixDeselectAll(job_running_jobs);
   }

   lFreeList(&jl);
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobHold(Widget w, XtPointer cld, XtPointer cad)
{
   lList *jl = NULL;
   lList *rl = NULL;
   lListElem *jep = NULL;
   lList *alp = NULL;
   Boolean status_ask = False;
   Cardinal new_hold = 0;
   lListElem *job = NULL;
   lListElem *jatep = NULL;

   lDescr hold_descr[] = {
      {JB_job_number, lUlongT | CULL_IS_REDUCED, NULL},
      {JB_ja_tasks, lListT | CULL_IS_REDUCED, NULL},
      {JB_ja_structure, lListT | CULL_IS_REDUCED, NULL},
      {NoName, lEndT | CULL_IS_REDUCED, NULL}
   };
   
   DENTER(GUI_LAYER, "qmonJobHold");

   rl = qmonJobBuildSelectedList(job_running_jobs, hold_descr, JB_job_number);

   /*
   ** set priority works only for pending jobs
   */
   jl = qmonJobBuildSelectedList(job_pending_jobs, hold_descr, JB_job_number);
   
   if (!jl && rl)
      jl = rl;
   else if (rl)
      lAddList(jl, &rl);
   
   if (jl) {
      dstring dyn_tasks = DSTRING_INIT;
      dstring dyn_oldtasks = DSTRING_INIT;
      lListElem *selected_job;
      lListElem *selected_ja_task;
      u_long32 selected_job_id;
      u_long32 selected_ja_task_id;

      /*
      ** get the first job in list and its first task to preset 
      ** the hold dialog
      */
      selected_job = lFirst(jl);
      selected_job_id = lGetUlong(selected_job, JB_job_number);
      job = job_list_locate(qmonMirrorList(SGE_JB_LIST), selected_job_id);
      selected_ja_task = lFirst(lGetList(selected_job, JB_ja_tasks));
      selected_ja_task_id = lGetUlong(selected_ja_task, JAT_task_number);
      if (job_is_enrolled(job, selected_ja_task_id)) {
         new_hold = lGetUlong(lFirst(lGetList(job, JB_ja_tasks)), JAT_hold);
      } else {
         new_hold = job_get_ja_task_hold_state(job, selected_ja_task_id);
      }

      if (lGetNumberOfElem(jl) == 1 && job_is_array(job)) {
         ja_task_list_print_to_string(lGetList(selected_job, JB_ja_tasks), 
                                     &dyn_tasks);
         sge_dstring_append(&dyn_oldtasks, dyn_tasks.s);
      }

      status_ask = AskForHold(w, &new_hold, &dyn_tasks); 

      if (jl && status_ask) {
         for_each(jep, jl) {
            if (dyn_tasks.s && strcmp(dyn_tasks.s, "") && 
                !strcmp(dyn_tasks.s, dyn_oldtasks.s)) {
               for_each (jatep, lGetList(jep, JB_ja_tasks)) {
                  lSetUlong(jatep, JAT_hold, new_hold | MINUS_H_CMD_SET);
                  DPRINTF(("Hold for" sge_u32 "." sge_u32 " is " sge_u32 "\n", 
                              lGetUlong(jep, JB_job_number), 
                              lGetUlong(jatep, JAT_task_number),
                              new_hold | MINUS_H_CMD_SET));
               }
            } else {
               for_each (jatep, lGetList(jep, JB_ja_tasks)) {
                  lSetUlong(jatep, JAT_hold, new_hold | MINUS_H_CMD_SET);
                  DPRINTF(("Hold for" sge_u32 "." sge_u32 " is " sge_u32 "\n",
                           lGetUlong(jep, JB_job_number),
                           lGetUlong(jatep, JAT_task_number),      
                           new_hold | MINUS_H_CMD_SET));
               }
            }
         }

/* lWriteListTo(jl, stdout); */
         alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_MOD, &jl, NULL, NULL, false); 
         qmonMessageBox(w, alp, 0);

         lFreeList(&jl);
         lFreeList(&alp);

         /*
         ** we use the callback to get the new job list from master
         ** cause the sge_gdi doesn't change the local list
         ** so updateJobList() is not sufficient
         */
         updateJobListCB(w, NULL, NULL);
         XbaeMatrixDeselectAll(job_pending_jobs);
         XbaeMatrixDeselectAll(job_running_jobs);

      }
      sge_dstring_free(&dyn_tasks);
      sge_dstring_free(&dyn_oldtasks);
   }
   else
      qmonMessageShow(w, True, "@{There are no jobs selected !}");


   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobQalter(Widget w, XtPointer cld, XtPointer cad)
{
#ifdef QALTER_RUNNING
   lList *rl = NULL;
#endif
   lList *pl = NULL;
   tSubmitMode data;

   DENTER(GUI_LAYER, "qmonJobQalter");
#ifdef QALTER_RUNNING
   rl = qmonJobBuildSelectedList(job_running_jobs, JB_Type, JB_job_number);
#endif
   /*
   ** qalter works for pending  & running jobs
   */
   pl = qmonJobBuildSelectedList(job_pending_jobs, JB_Type, JB_job_number);

#ifdef QALTER_RUNNING
   if (!pl && rl) {
      pl = rl;
   } else if (rl) {
      lAddList(pl, &rl);
   }
#endif
   /*
   ** call the submit dialog for pending jobs
   */
   if (pl && lGetNumberOfElem(pl) == 1) {
      data.mode = SUBMIT_QALTER_PENDING;
      data.job_id = lGetUlong(lFirst(pl), JB_job_number);
      qmonSubmitPopup(w, (XtPointer)&data, NULL);
      lFreeList(&pl);
      XbaeMatrixDeselectAll(job_pending_jobs);
#ifdef QALTER_RUNNING
      XbaeMatrixDeselectAll(job_running_jobs);
#endif
   } else {
#ifndef QALTER_RUNNING
      qmonMessageShow(w, True, "@{Select one pending job for Qalter !}");
#else
      qmonMessageShow(w, True, "@{Select one job for Qalter !}");
#endif
   }
   
   DEXIT;
}



/*-------------------------------------------------------------------------*/
static void qmonJobHandleEnterLeave(
Widget w,
XtPointer cld,
XEvent *ev,
Boolean *ctd 
) {
   /*
   int root_x, root_y, pos_x, pos_y;
   Window root, child;
   unsigned int keys_buttons;
   */
   char line[BUFSIZ];
   static int prev_row = -1;
   int row, col;
   String str;
   int job_nr;
   lListElem *jep;
   
   DENTER(GUI_LAYER, "qmonJobHandleEnterLeave");
   
   switch (ev->type) {
      case MotionNotify:
         if (qmonBrowserObjectEnabled(BROWSE_JOB)) {
            /* 
            ** XQueryPointer(XtDisplay(w), XtWindow(w), 
            **            &root, &child, &root_x, &root_y,
            **            &pos_x, &pos_y, &keys_buttons);
            */
            if (XbaeMatrixGetEventRowColumn(w, ev, &row, &col)) {
               if ( row != prev_row ) {
                  prev_row = row;
                  /* locate the job */
                  str = XbaeMatrixGetCell(w, row, 0);
                  if (str && *str != '\0') {
                     job_nr = atoi(str);
                     jep = job_list_locate(qmonMirrorList(SGE_JB_LIST), 
                                          (u_long32)job_nr);
                     if (jep) {
                        dstring job_info = DSTRING_INIT;
                        sprintf(line, "+++++++++++++++++++++++++++++++++++++++++++\n");  
                        qmonBrowserShow(line);
                        qmonJobShowBrowserInfo(&job_info, jep);      
                        qmonBrowserShow(sge_dstring_get_string(&job_info));
                        sge_dstring_free(&job_info);
                     }
                  }
               }
            }
         }
         break;
   }
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobShowBrowserInfo(
dstring *info,
lListElem *jep 
) {

   DENTER(GUI_LAYER, "qmonJobShowBrowserInfo");

   sge_dstring_sprintf(info, "%-30.30s"sge_u32"\n", "Job:", lGetUlong(jep, JB_job_number));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "Full job name:", 
                              lGetString(jep, JB_job_name));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "Job Script:", 
                  lGetString(jep, JB_script_file) ? 
                  lGetString(jep, JB_script_file): "-NA-");
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "Owner:", 
                  lGetString(jep, JB_owner));
   sge_dstring_sprintf_append(info, "%-30.30s%d\n", "Priority:", 
                  (int)(lGetUlong(jep, JB_priority) - BASE_PRIORITY));
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "Checkpoint Object:", 
            lGetString(jep, JB_checkpoint_name) ? 
            lGetString(jep, JB_checkpoint_name) : "-NA-");

#ifdef FIXME
   if (lGetList(jep, JB_jid_predecessor_list) || lGetUlong(jep, JB_hold)) {
      snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
               "Status:", "Hold");
   }
   else {
      status = lGetUlong(jep, JB_status);
      switch (status) {
         case JRUNNING:
            snprintf(info + strlen(info), sizeof(info) - strlen(info),
                     WIDTH"%s\n", "Status:", "Running");
            break;
         case JIDLE:
            snprintf(info + strlen(info), sizeof(info) - strlen(info),
                     WIDTH"%s\n", "Status:", "Idle");
            break;
         case JTRANSFERING:
            snprintf(info + strlen(info), sizeof(info) - strlen(info),
                     WIDTH"%s\n", "Status:", "Transiting");
            break;
      }
   }


   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Submission Time:",
            sge_ctime(lGetUlong(jep, JB_submission_time), &ds));

   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s\n",
            "Start Time:",
            lGetUlong(jep, JB_start_time) ?
            sge_ctime(lGetUlong(jep, JB_start_time), &ds) :
            "-none-");
#endif

   if (lGetString(jep, JB_pe)) {
      dstring range_string = DSTRING_INIT;

      range_list_print_to_string(lGetList(jep, JB_pe_range), &range_string, true, false, false);
      sge_dstring_sprintf_append(info, "%-30.30s%s %s\n", "Requested PE:", 
              lGetString(jep, JB_pe), sge_dstring_get_string(&range_string));
      sge_dstring_free(&range_string);
   }

#ifdef FIXME
   if (lGetString(jep, JB_granted_pe)) {
      lListElem *gdil_ep;
      u_long32 pe_slots = 0;
      for_each (gdil_ep, lGetList(jep, JB_granted_destin_identifier_list))
         pe_slots += lGetUlong(gdil_ep, JG_slots);
      snprintf(info + strlen(info), sizeof(info) - strlen(info),
               WIDTH"%s "_u32 "\n", info,
               "Granted PE:", lGetString(jep, JB_pe), pe_slots);
   }
#endif

#ifdef FIXME
   sge_dstring_sprintf_append(info, "%-30.30s%s\n", "Hard Resources:", 

   strcpy(buf, "");
   centry_list_parse_from_string(lGetList(jep, JB_hard_resource_list), buf, sizeof(buf));
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s",
            "Hard Resources:", buf);
   if (*buf=='\0')
      strcat(info, "\n");
               
   strcpy(buf, "");
   centry_list_parse_from_string(lGetList(jep, JB_soft_resource_list), buf, sizeof(buf));
   snprintf(info + strlen(info), sizeof(info) - strlen(info), WIDTH"%s",
            "Soft Resources:", buf);
   if (*buf=='\0')
      strcat(info, "\n");


   
/*    lWriteListTo(lGetList(jep, JB_qs_args), stdout); */
   
   DPRINTF(("info is %d long\n", strlen(info)));
#endif

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static Boolean AskForHold(
Widget w,
Cardinal *hold,
dstring *dyn_tasks 
) {
   static tAskHoldInfo AskHoldInfo;      
   Widget shell = XmtGetShell(w);
   Widget hold_layout, hold_cancel, hold_ok;
   static Widget hold_flags, hold_tasks;

   DENTER(GUI_LAYER, "AskForHold");

   /* 
   ** make sure the shell we pop up over is not a menu shell! 
   */
   while(XtIsOverrideShell(shell)) 
      shell = XmtGetShell(XtParent(shell));
 
   /*
   ** create the hold popup dialog
   */
   if (!AskHoldInfo.AskHoldDialog) {
      AskHoldInfo.AskHoldDialog = XmtBuildQueryDialog( shell, "qmon_hold",
                           NULL, 0,
                           "hold_cancel", &hold_cancel,
                           "hold_ok", &hold_ok,
                           "hold_flags", &hold_flags,
                           "hold_tasks", &hold_tasks,
                           NULL);
      AskHoldInfo.AskHoldFlags = hold_flags;
      AskHoldInfo.AskHoldTasks = hold_tasks;
      XtVaSetValues( AskHoldInfo.AskHoldDialog,
                     XmNdefaultButton, hold_ok,
                     NULL);
      XtAddCallback(hold_ok, XmNactivateCallback, 
                  AskHoldOkCB, (XtPointer)&AskHoldInfo);
      XtAddCallback(hold_cancel, XmNactivateCallback, 
                  AskHoldCancelCB, (XtPointer)&AskHoldInfo);
      
      XmtAddDeleteCallback(XtParent(AskHoldInfo.AskHoldDialog), XmDO_NOTHING,
                      AskHoldCancelCB, (XtPointer)&AskHoldInfo);

   }
   hold_layout = AskHoldInfo.AskHoldDialog;
   hold_flags = AskHoldInfo.AskHoldFlags;
   hold_tasks = AskHoldInfo.AskHoldTasks;
   
   /* 
   ** Tell the dialog who it is transient for 
   */
   XtVaSetValues( XtParent(hold_layout), 
                  XtNtransientFor, shell, 
                  NULL);
   /*
   ** preset with default values
   */
   XmtChooserSetState(hold_flags, *hold, False);
   if (dyn_tasks->s && (dyn_tasks->s)[0] != '\0') {
      XtSetSensitive(hold_tasks, True);
      XmtInputFieldSetString(hold_tasks, dyn_tasks->s);
   }
   else {
      XtSetSensitive(hold_tasks, False);
      XmtInputFieldSetString(hold_tasks, "");
   }   

   /* 
   ** position, set initial focus, and pop up the dialog 
   */
   XmtDialogPosition(XtParent(hold_layout), shell);
   XtManageChild(hold_layout);

   /*
   ** Enter a recursive event loop.
   ** The callback registered on the okay and cancel buttons when
   ** this dialog was created will cause info->button to change
   ** when one of those buttons is pressed.
   */
   AskHoldInfo.blocked = True;
   XmtBlock(shell, &AskHoldInfo.blocked);

   /* pop down the dialog */
   XtUnmanageChild(hold_layout);

   /* make sure what is underneath gets cleaned up
   ** (the calling routine might act on the user's returned
   ** input and not handle events for awhile.)
   */
   XSync(XtDisplay(hold_layout), 0);
   XmUpdateDisplay(hold_layout);

   /*
   ** if the user clicked Ok, return the hold state
   */
   if (AskHoldInfo.button == XmtOkButton) {
      String ts;
      *hold = XmtChooserGetState(hold_flags);
      ts = XmtInputFieldGetString(hold_tasks);
      sge_dstring_free(dyn_tasks);
      sge_dstring_append(dyn_tasks, ts ? ts : ""); 
      DEXIT;
      return True;
   }
   else {
      DEXIT;
      return False;
   }
}

   
static void AskHoldOkCB(Widget w, XtPointer cld, XtPointer cad)
{
    tAskHoldInfo *info = (tAskHoldInfo *)cld;
    info->blocked = False;
    info->button = XmtOkButton;
}

static void AskHoldCancelCB(Widget w, XtPointer cld, XtPointer cad)
{
    tAskHoldInfo *info = (tAskHoldInfo *)cld;
    info->blocked = False;
    info->button = XmtCancelButton;
}

/*-------------------------------------------------------------------------*/
static void qmonJobScheddInfo(Widget w, XtPointer cld, XtPointer cad)
{
   dstring sb = DSTRING_INIT;
   lList *jl = NULL;

   lDescr info_descr[] = {
      {ST_name, lStringT | CULL_IS_REDUCED, NULL},
      {NoName, lEndT | CULL_IS_REDUCED, NULL}
   };
   
   DENTER(GUI_LAYER, "qmonJobScheddInfo");
   
   jl = qmonJobBuildSelectedList(job_pending_jobs, info_descr, ST_name);
   
   qmonBrowserOpen(w, NULL, NULL);
   if (jl ? (show_info_for_jobs(jl, NULL, NULL, &sb) == 0) :  
            (show_info_for_job(NULL, NULL, &sb) == 0) && 
               sge_dstring_get_string(&sb)) {
      qmonBrowserShow(sge_dstring_get_string(&sb));
   }
   else 
      qmonBrowserShow("---------Could not get scheduling info--------\n");

   lFreeList(&jl);
   sge_dstring_free(&sb);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobFolderChange(Widget w, XtPointer cld, XtPointer cad)
{
   XmTabCallbackStruct *cbs = (XmTabCallbackStruct *) cad;

   DENTER(GUI_LAYER, "qmonJobFolderChange");
   
   DPRINTF(("%s\n", XtName(cbs->tab_child)));

   if (!strcmp(XtName(cbs->tab_child), "job_pending")) {
      XtSetSensitive(job_schedd_info, True);
      current_matrix=job_pending_jobs;
   }
   else {
      XtSetSensitive(job_schedd_info, False);
   }

   if (!strcmp(XtName(cbs->tab_child), "job_running")) {
      XtSetSensitive(job_reschedule, True);
      XtSetSensitive(job_suspend, True);
      XtSetSensitive(job_unsuspend, True);
      current_matrix=job_running_jobs;
   }
   else {
      XtSetSensitive(job_reschedule, False);
      XtSetSensitive(job_suspend, False);
      XtSetSensitive(job_unsuspend, False);
   }

   if (!strcmp(XtName(cbs->tab_child), "job_zombie")) {
      XtSetSensitive(force_toggle, False);
      XtSetSensitive(job_suspend, False);
      XtSetSensitive(job_unsuspend, False);
      XtSetSensitive(job_delete, False);
      XtSetSensitive(job_hold, False);
      XtSetSensitive(job_qalter, False);
      XtSetSensitive(job_error, False);
      XtSetSensitive(job_priority, False);
      XtSetSensitive(job_select_all, False);
      current_matrix=job_zombie_jobs;
   }
   else {
      XtSetSensitive(force_toggle, True);
      XtSetSensitive(job_delete, True);
      XtSetSensitive(job_hold, True);
      XtSetSensitive(job_qalter, True);
      XtSetSensitive(job_error, True);
      XtSetSensitive(job_priority, True);
      XtSetSensitive(job_select_all, True);
   }
   updateJobList();

   DEXIT;
}

/*-------------------------------------------------------------------------*/
/*
** qstat_show_job
** displays information about a given job
** to be extended
**
** returns 0 on success, non-zero on failure
*/
static int show_info_for_jobs(
lList *jid_list,
FILE *fp,
lList **alpp,
dstring *sb
) {
   lListElem *j_elem = 0;
   lList* jlp = NULL;
   lList* ilp = NULL;
   lListElem* aep = NULL;
   lCondition *where = NULL, *newcp = NULL;
   lEnumeration* what = NULL;
   lList* alp = NULL;
   bool schedd_info = true;
   bool jobs_exist = true;
   int line_separator=0;
   lListElem* mes;
 
   DENTER(TOP_LAYER, "qstat_show_job");
 
   /* get job scheduling information */
   what = lWhat("%T(ALL)", SME_Type);
   alp = ctx->gdi(ctx, SGE_SME_LIST, SGE_GDI_GET, &ilp, NULL, what, false);
   lFreeWhat(&what);
   for_each(aep, alp) {
      if (lGetUlong(aep, AN_status) != STATUS_OK) {
         if (fp)
            fprintf(fp, "%s", lGetString(aep, AN_text));
         if (alpp) {
            answer_list_add(alpp, lGetString(aep, AN_text),
                  lGetUlong(aep, AN_status), lGetUlong(aep, AN_quality));
         }
         schedd_info = false;
      }
   }
   lFreeList(&alp);
 
   /* build 'where' for all jobs */
   where = NULL;
   for_each(j_elem, jid_list) {
      u_long32 jid = atol(lGetString(j_elem, ST_name));
 
      newcp = lWhere("%T(%I==%u)", JB_Type, JB_job_number, jid);
      if (!where)
         where = newcp;
      else
         where = lOrWhere(where, newcp);
   }                                          
   what = lWhat("%T(ALL)", JB_Type);
   /* get job list */
   alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &jlp, where, what, false);
   lFreeWhere(&where);
   lFreeWhat(&what);
   for_each(aep, alp) {
      if (lGetUlong(aep, AN_status) != STATUS_OK) {
         if (fp)
            fprintf(fp, "%s", lGetString(aep, AN_text));
         if (alpp) {
            answer_list_add(alpp, lGetString(aep, AN_text),
                  lGetUlong(aep, AN_status), lGetUlong(aep, AN_quality));
         }
         jobs_exist = false;
      }
   }
   lFreeList(&alp);
   if(!jobs_exist) {
      DEXIT;
      return 1;
   }
 
   /* print scheduler job information and global scheduler info */
   for_each (j_elem, jlp) {
      u_long32 jid = lGetUlong(j_elem, JB_job_number);
      lListElem *sme = NULL;
      lListElem *jatep = NULL;
 
      if (line_separator) {
         sge_dstring_sprintf_append(sb, "\n");
      } else {
         line_separator = 1;
      }   
/*       cull_show_job(j_elem, 0); */
      if (schedd_info && (sme = lFirst(ilp))) {
         int first_run = 1;
 
         if (sme) {
            /* global schduling info */
            for_each (mes, lGetList(sme, SME_global_message_list)) {
               if (first_run) {
                  sge_dstring_sprintf_append(sb, "%s", "scheduling info:            ");
                  first_run = 0;
               }
               else
                  sge_dstring_sprintf_append(sb, "%s", "                            ");
               sge_dstring_sprintf_append(sb, "%s\n", lGetString(mes, MES_message));
            }
 
            /* job scheduling info */
            where = lWhere("%T(%I->%T(%I==%u))", MES_Type, MES_job_number_list, ULNG_Type, ULNG_value, jid);
            mes = lFindFirst(lGetList(sme, SME_message_list), where);
            if (mes) {
               if (first_run) {
                  sge_dstring_sprintf_append(sb, "%s", "scheduling info:            ");
                  first_run = 0;
               }
               else
                  sge_dstring_sprintf_append(sb, "%s\n", lGetString(mes, MES_message));
            }
            while ((mes = lFindNext(mes, where)))
               sge_dstring_sprintf_append(sb, "                            %s\n",
                                    lGetString(mes, MES_message));
            lFreeWhere(&where);
         }
      }

      /* dump any associated task error info */
      for_each (jatep, lGetList(j_elem, JB_ja_tasks)) {
         lListElem *mesobj = NULL;
         bool first = true;

         if (first) {
            /* start task error info on a new line */
            sge_dstring_sprintf_append(sb, "\n"); 
            first = false;
         }
         for_each(mesobj, lGetList(jatep, JAT_message_list)) {
            const char *message = lGetString(mesobj, QIM_message);
            if (message != NULL) {
               sge_dstring_sprintf_append(sb, "Error for job %u: %s\n",
                                          jid, message);
            }
         }
      }

   }                      
 
   lFreeList(&ilp);
   lFreeList(&jlp);
   DEXIT;
   return 0;
}
 
/* show_info_for_job: show "Why ?" when none is selected
**
** derived from qstat_show_job
*/
static int show_info_for_job(
FILE *fp,
lList **alpp,
dstring *sb
) {
   lList *ilp = NULL, *mlp = NULL;
   lListElem* aep = NULL;
   lEnumeration* what = NULL;
   lList* alp = NULL;
   bool schedd_info = true;
   lListElem* mes;
   int initialized = 0;
   u_long32 last_jid = 0;
   u_long32 last_mid = 0;
   char text[256], ltext[256];
   int ids_per_line = 0;
   int first_run = 1;
   int first_row = 1;
   lListElem *sme;
   lListElem *jid_ulng = NULL;
 
   DENTER(TOP_LAYER, "show_info_for_job");
 
   /* get job scheduling information */
   what = lWhat("%T(ALL)", SME_Type);
   alp = ctx->gdi(ctx, SGE_SME_LIST, SGE_GDI_GET, &ilp, NULL, what, false);
   lFreeWhat(&what);
   for_each(aep, alp) {
      if (lGetUlong(aep, AN_status) != STATUS_OK) {
         if (fp) {
            fprintf(fp, "%s", lGetString(aep, AN_text));
         }
         if (alpp) {
            answer_list_add(alpp, lGetString(aep, AN_text),
                  lGetUlong(aep, AN_status), lGetUlong(aep, AN_quality));
         }
         schedd_info = false;
      }
   }
   lFreeList(&alp);
   if (!schedd_info) {
      DEXIT;
      return 1;
   }
 
   sme = lFirst(ilp);
   if (sme) {
      /* print global schduling info */
      first_run = 1;
      for_each (mes, lGetList(sme, SME_global_message_list)) {
         if (first_run) {
            sge_dstring_sprintf_append(sb, "%s", "scheduling info:            ");
            first_run = 0;
         }
         else
            sge_dstring_sprintf_append(sb, "%s", "                            ");
         sge_dstring_sprintf_append(sb, "%s\n", lGetString(mes, MES_message));
      }                                               
      if (!first_run)
         sge_dstring_sprintf_append(sb, "\n");
 
      first_run = 1;
 
      mlp = lGetList(sme, SME_message_list);
      lPSortList (mlp, "I+", MES_message_number);
 
      text[0]=0;
      for_each(mes, mlp) {
         lPSortList (lGetList(mes, MES_job_number_list), "I+", ULNG_value);
 
         for_each(jid_ulng, lGetList(mes, MES_job_number_list)) {
            u_long32 mid;
            u_long32 jid = 0;
            int skip = 0;
            int header = 0;
 
            mid = lGetUlong(mes, MES_message_number);
            jid = lGetUlong(jid_ulng, ULNG_value);
 
            if (initialized) {
               if (last_mid == mid && last_jid == jid)
                  skip = 1;
               else if (last_mid != mid)
                  header = 1;
            }
            else {
               initialized = 1;
               header = 1;
            }
 
            if (strlen(text) >= MAX_LINE_LEN || ids_per_line >= MAX_IDS_PER_LINE || header) {
               sge_dstring_sprintf_append(sb, "%s", text);
               text[0] = 0;
               ids_per_line = 0;
               first_row = 0;
            }
 
            if (header) {
               if (!first_run) {
                  if (fp)
                     printf("\n\n");
                  else   
                     sge_dstring_sprintf_append(sb, "\n\n");
               } else
                  first_run = 0;
               sge_dstring_sprintf_append(sb, "%s\n", sge_schedd_text(mid+SCHEDD_INFO_OFFSET));
               first_row = 1;
            }
 
            if (!skip) {
               if (ids_per_line == 0)
                  if (first_row)
                     strcat(text, "\t");
                  else
                     strcat(text, ",\n\t");
               else
                  strcat(text, ",\t");
               sprintf(ltext, sge_u32, jid);
               strcat(text, ltext);
               ids_per_line++;
            }
                                                 
            last_jid = jid;
            last_mid = mid;
         }
      }
      if (text[0] != 0)
         sge_dstring_sprintf_append(sb, "%s\n", text);
   }
 
   lFreeList(&ilp);
   DEXIT;
   return 0;
}                             


