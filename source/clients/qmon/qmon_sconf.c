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
#include <stdlib.h>
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

#include "qmon_proto.h"
#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_util.h"
#include "qmon_comm.h"
#include "qmon_appres.h"
#include "qmon_timer.h"
#include "qmon_widgets.h"
#include "qmon_quarks.h"
#include "qmon_message.h"
#include "qmon_sconf.h"
#include "qmon_load.h"
#include "qmon_matrix.h"
#include "qmon_globals.h"
#include "AskForTime.h"

#include "uti/sge_string.h"

#include "comm/commlib.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_feature.h"

#include "sched/sge_sched.h"

#include "gdi/sge_gdi.h"

/*-------------------------------------------------------------------------*/
static Widget qmonCreateSchedConfig(Widget parent);
static void qmonSchedOk(Widget w, XtPointer cld, XtPointer cad);
static void qmonSchedCancel(Widget w, XtPointer cld, XtPointer cad);
static void qmonSchedTime(Widget w, XtPointer cld, XtPointer cad);
static Boolean qmonSchedSet(lListElem *sep);
static Boolean qmonSchedGet(lListElem *sep);
static void qmonLoadNamesSC(Widget w, XtPointer cld, XtPointer cad);
static void qmonSchedJobInfo(Widget w, XtPointer cld, XtPointer cad);
static void qmonSchedFreeData(void);
/*-------------------------------------------------------------------------*/
   
typedef struct _tSCEntry {
   char *algorithm;
   char *schedule_interval;
   char *sc_params;
   int  maxujobs;
   int  flush_submit_secs;
   int  flush_finish_secs;
   int  max_reservation;
   int  queue_sort_method;
   char *load_adjustment_decay_time;
   char *load_formula;
   lList *job_load_adjustments;
   char *reprioritize_interval;
   char *default_duration;
} tSCEntry;

static XtResource sc_resources[] = {
   { "algorithm", "algorithm", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, algorithm), 
      XtRImmediate, NULL },

   { "schedule_interval", "schedule_interval", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, schedule_interval), 
      XtRImmediate, NULL },

   { "sc_params", "sc_params", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, sc_params), 
      XtRImmediate, NULL },

   { "maxujobs", "maxujobs", XtRInt,
      sizeof(int), XtOffsetOf(tSCEntry, maxujobs),
      XtRImmediate, NULL },
   
   { "flush_submit_secs", "flush_submit_secs", XtRInt,
      sizeof(int), XtOffsetOf(tSCEntry, flush_submit_secs),
      XtRImmediate, NULL },
   
   { "flush_finish_secs", "flush_finish_secs", XtRInt,
      sizeof(int), XtOffsetOf(tSCEntry, flush_finish_secs),
      XtRImmediate, NULL },
   
   { "max_reservation", "max_reservation", XtRInt,
      sizeof(int), XtOffsetOf(tSCEntry, max_reservation),
      XtRImmediate, NULL },
   
   { "queue_sort_method", "queue_sort_method", XtRInt, 
      sizeof(int), XtOffsetOf(tSCEntry, queue_sort_method), 
      XtRImmediate, NULL },

   { "job_load_adjustments", "job_load_adjustments", QmonRCE2_Type, 
      sizeof(lList *), XtOffsetOf(tSCEntry, job_load_adjustments), 
      XtRImmediate, NULL },

   { "load_adjustment_decay_time", "load_adjustment_decay_time", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, load_adjustment_decay_time), 
      XtRImmediate, NULL },

   { "load_formula", "load_formula", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, load_formula), 
      XtRImmediate, NULL },

   { "reprioritize_interval", "reprioritize_interval", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, reprioritize_interval), 
      XtRImmediate, NULL },

   { "default_duration", "default_duration", XtRString, 
      sizeof(String), XtOffsetOf(tSCEntry, default_duration), 
      XtRImmediate, NULL }
};


static tSCEntry data = {NULL, NULL, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL};


static Widget qmon_sconf = 0;
static Widget sconf_algorithm = 0;
static Widget sconf_schedule_interval = 0;
static Widget sconf_sc_params = 0;
static Widget sconf_maxujobs = 0;
static Widget sconf_flush_submit_secs = 0;
static Widget sconf_flush_finish_secs = 0;
static Widget sconf_max_reservation = 0;
static Widget sconf_lad_time = 0;
static Widget sconf_load_formula = 0;
static Widget sconf_load_adjustments = 0;
static Widget sconf_reprioritize_interval = 0;
static Widget sconf_default_duration = 0;
static Widget sconf_queue_sort_method = 0;
static Widget sconf_job_info = 0;
static Widget sconf_job_range = 0;



/*-------------------------------------------------------------------------*/
void qmonPopupSchedConfig(Widget w, XtPointer cld, XtPointer cad)
{
   Widget shell;
   lList *scl = NULL;
   lListElem *sep = NULL;
   lList *alp = NULL;

   DENTER(TOP_LAYER, "qmonPopupSchedConfig");

   /* set busy cursor */
   XmtDisplayBusyCursor(w);

   if (!qmon_sconf) {
      shell = XmtGetTopLevelShell(w);
      qmon_sconf = qmonCreateSchedConfig(shell);
      XmtAddDeleteCallback(shell, XmDO_NOTHING, 
                              qmonSchedCancel, NULL);
   } 
   qmonMirrorMultiAnswer(SC_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      /* set default cursor */
      XmtDisplayDefaultCursor(w);
      DEXIT;
      return;
   }

   scl = qmonMirrorList(SGE_SC_LIST);
   sep = lFirst(scl);
   qmonSchedSet(sep);
   XSync(XtDisplay(qmon_sconf), 0);
   XmUpdateDisplay(qmon_sconf);

   XtManageChild(qmon_sconf);
   XRaiseWindow(XtDisplay(XtParent(qmon_sconf)), 
                  XtWindow(XtParent(qmon_sconf)));

   /* set default cursor */
   XmtDisplayDefaultCursor(w);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static Widget qmonCreateSchedConfig(
Widget parent 
) {
   Widget sconf_layout, sconf_ok, sconf_cancel,
          sconf_main_link, sconf_lad_timePB,
          sconf_schedule_intervalPB, sconf_reprioritize_intervalPB,
          sconf_load_name, sconf_load_value, sconf_load_namePB,
          sconf_load_add, sconf_load_delete, sconf_default_durationPB;

   DENTER(TOP_LAYER, "qmonCreateSchedConfig");
   
   sconf_layout = XmtBuildQueryDialog( parent, "sconf_shell",
                           sc_resources, XtNumber(sc_resources),
                           "sconf_main_link", &sconf_main_link,
                           "sconf_ok", &sconf_ok,
                           "sconf_cancel", &sconf_cancel,
                           "sconf_algorithm", &sconf_algorithm,
                           "sconf_schedule_interval", &sconf_schedule_interval,
                           "sconf_sc_params", &sconf_sc_params,
                           "sconf_maxujobs", &sconf_maxujobs,
                           "sconf_flush_submit_secs", &sconf_flush_submit_secs,
                           "sconf_flush_finish_secs", &sconf_flush_finish_secs,
                           "sconf_max_reservation", &sconf_max_reservation,
                           "sconf_lad_time", &sconf_lad_time,
                           "sconf_load_formula", &sconf_load_formula,
                           "sconf_load_adjustments", &sconf_load_adjustments,
                           "sconf_load_name", &sconf_load_name,
                           "sconf_load_namePB", &sconf_load_namePB,
                           "sconf_load_value", &sconf_load_value,
                           "sconf_load_add", &sconf_load_add,
                           "sconf_load_delete", &sconf_load_delete,
                           "sconf_reprioritize_interval", 
                                 &sconf_reprioritize_interval,
                           "sconf_default_duration", 
                                 &sconf_default_duration,
                           "sconf_queue_sort_method", &sconf_queue_sort_method,
                           "sconf_lad_timePB", &sconf_lad_timePB,
                           "sconf_schedule_intervalPB", 
                                 &sconf_schedule_intervalPB,
                           "sconf_reprioritize_intervalPB",
                                 &sconf_reprioritize_intervalPB,
                           "sconf_default_durationPB",
                                 &sconf_default_durationPB,
                           "sconf_job_info", &sconf_job_info,
                           "sconf_job_range", &sconf_job_range,
                           NULL);

   XtAddCallback(sconf_main_link, XmNactivateCallback, 
                     qmonMainControlRaise, NULL);
   XtAddCallback(sconf_cancel, XmNactivateCallback, 
                     qmonSchedCancel, NULL);
   XtAddCallback(sconf_ok, XmNactivateCallback, 
                     qmonSchedOk, NULL);


   XtAddCallback(sconf_lad_timePB, XmNactivateCallback,
                 qmonSchedTime, (XtPointer)sconf_lad_time); 
   XtAddCallback(sconf_schedule_intervalPB, XmNactivateCallback,
                 qmonSchedTime, (XtPointer)sconf_schedule_interval); 
   XtAddCallback(sconf_reprioritize_intervalPB, XmNactivateCallback,
                 qmonSchedTime, (XtPointer)sconf_reprioritize_interval); 
   XtAddCallback(sconf_default_durationPB, XmNactivateCallback,
                 qmonSchedTime, (XtPointer)sconf_default_duration); 


   
   XtAddCallback(sconf_load_adjustments, XmNlabelActivateCallback,
                  qmonLoadNamesSC, NULL); 

   XtAddCallback(sconf_job_info, XmtNvalueChangedCallback,
                  qmonSchedJobInfo, NULL); 

   XtAddEventHandler(XtParent(sconf_layout), StructureNotifyMask, False, 
                        SetMinShellSize, NULL);
   XtAddEventHandler(XtParent(sconf_layout), StructureNotifyMask, False, 
                        SetMaxShellSize, NULL);
   DEXIT;
   return sconf_layout;
}

/*-------------------------------------------------------------------------*/
static void qmonSchedOk(Widget w, XtPointer cld, XtPointer cad)
{
   lList *scl = NULL;
   lListElem *sep = NULL;
   lEnumeration *what = NULL;
   lList *alp = NULL;
   Boolean status = False;

   DENTER(GUI_LAYER, "qmonSchedOk");

   /*
   ** get the contents of the dialog fields here,
   ** build the cull list and send gdi request
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


   if (qmonSchedGet(sep)) {

      if (rmon_mlgetl(&RMON_DEBUG_ON, GUI_LAYER) & INFOPRINT) {
         printf("___SCHEDULER_CONF________________________\n");
         lWriteListTo(scl, stdout);
         printf("_______________________________________\n");
      }
      /*
      ** gdi call 
      */
      what = lWhat("%T(ALL)", SC_Type);

      alp = qmonModList(SGE_SC_LIST, qmonMirrorListRef(SGE_SC_LIST),
                           0, &scl, NULL, what);

      
      if (lFirst(alp) && lGetUlong(lFirst(alp), AN_status) == STATUS_OK)
         status = True;

      qmonMessageBox(qmon_sconf, alp, 0);

      lFreeWhat(&what);
      lFreeList(&alp);
   }

   if (status)
      XtUnmanageChild(qmon_sconf);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonSchedCancel(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "qmonSchedCancel");

   XtUnmanageChild(qmon_sconf);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonSchedTime(Widget w, XtPointer cld, XtPointer cad)
{
   Widget input_field = (Widget) cld;
   char stringval[256];
   Boolean status = False;
   String current;

   DENTER(GUI_LAYER, "qmonSchedTime");

   current = XmtInputFieldGetString(input_field);
   sge_strlcpy(stringval, current ? current : "", sizeof(stringval));
   status = XmtAskForTime(w, NULL, "@{Enter time}",
               stringval, sizeof(stringval), NULL, False);
   if (stringval[0] == '\0')
      status = False;

   if (status)
      XmtInputFieldSetString(input_field, stringval);

   DEXIT;
}





/*-------------------------------------------------------------------------*/
static Boolean qmonSchedSet(lListElem *sep) {
   static char schedd_job_info[BUFSIZ];

   DENTER(GUI_LAYER, "qmonSchedSet");

   if (!sep) {
      DRETURN(False);
   }

/* printf("------> qmonSchedSet\n"); */
/* lWriteElemTo(sep, stdout);    */

   data.algorithm = sge_strdup(data.algorithm, 
                                 (StringConst)lGetString(sep, SC_algorithm));

   data.schedule_interval = sge_strdup(data.schedule_interval, 
                                 (StringConst)lGetString(sep, SC_schedule_interval));

   data.sc_params = sge_strdup(data.sc_params, 
                                 (StringConst)lGetString(sep, SC_params));


   data.maxujobs = lGetUlong(sep, SC_maxujobs);

   data.flush_submit_secs = lGetUlong(sep, SC_flush_submit_sec);
   data.flush_finish_secs = lGetUlong(sep, SC_flush_finish_sec);

   data.max_reservation = lGetUlong(sep, SC_max_reservation);

   /* this depends on the kind queue_sort_method is represented */
   data.queue_sort_method = lGetUlong(sep, SC_queue_sort_method);

   /*
   ** load adjustments need special treatment
   */
   data.job_load_adjustments = lCopyList("copy", lGetList(sep, SC_job_load_adjustments));
   
   data.load_adjustment_decay_time = sge_strdup(data.load_adjustment_decay_time, 
               (StringConst)lGetString(sep, SC_load_adjustment_decay_time));

   data.load_formula = sge_strdup(data.load_formula, 
                              (StringConst)lGetString(sep, SC_load_formula));

   data.reprioritize_interval = sge_strdup(data.reprioritize_interval, 
                              (StringConst)lGetString(sep, SC_reprioritize_interval));
   data.default_duration = sge_strdup(data.default_duration, 
                              (StringConst)lGetString(sep, SC_default_duration));

/**
printf("->data.algorithm: '%s'\n", data.algorithm ? data.algorithm : "-NA-");
printf("->data.schedule_interval: '%s'\n", data.schedule_interval ? data.schedule_interval : "-NA-");
printf("->data.sc_params: '%s'\n", data.sc_params ? data.sc_params : "-NA-");
printf("->data.maxujobs: '%d'\n", data.maxujobs );
printf("->data.flush_submit_secs: '%d'\n", data.flush_submit_secs );
printf("->data.flush_finish_secs: '%d'\n", data.flush_finish_secs );
printf("->data.max_reservation: '%d'\n", data.max_reservation );
printf("->data.queue_sort_method: '%d'\n", data.queue_sort_method );
printf("->data.load_adjustment_decay_time: '%s'\n", data.load_adjustment_decay_time ? data.load_adjustment_decay_time : "-NA-");
printf("->data.load_formula: '%s'\n", data.load_formula ? data.load_formula : "-NA-");
**/
   
   /*
   ** set the dialog values
   */
   XmtDialogSetDialogValues(qmon_sconf, (XtPointer) &data);

   /*
   ** schedd_job_info needs some extras
   ** attention: order must match order of labels in qmon_sconf.ad
   ** 
   ** Qmon*sconf_general*sconf_job_info.chooserType: ChooserOption
   ** Qmon*sconf_general*sconf_job_info.strings: \
   ** "False", \
   ** "True", \
   ** "Job Range"
   */
   if (lGetString(sep, SC_schedd_job_info))
      sge_strlcpy(schedd_job_info, lGetString(sep, SC_schedd_job_info), BUFSIZ);
   else
      strcpy(schedd_job_info, "false");

   if (!strcasecmp(schedd_job_info, "false")) {
      XmtChooserSetState(sconf_job_info, 0, True);
      XmtInputFieldSetString(sconf_job_range, "");
   }
   else if (!strcasecmp(schedd_job_info, "true")) {
      XmtChooserSetState(sconf_job_info, 1, True);
      XmtInputFieldSetString(sconf_job_range, "");
   }
   else {
      char *sji;
      strtok(schedd_job_info, " \t");
      sji = strtok(NULL, "\n");
      XmtChooserSetState(sconf_job_info, 2, True);
      XmtInputFieldSetString(sconf_job_range, sji);
   }
   qmonSchedFreeData();

   DRETURN(True);
}


/*-------------------------------------------------------------------------*/
static Boolean qmonSchedGet(lListElem *sep) {
   int job_info;
   lList *alp = NULL;
   String str;
   char buf[BUFSIZ];

   DENTER(GUI_LAYER, "qmonSchedGet");

   if (!sep) {
      goto error_exit;
   }

   /*
   ** get entries from dialog
   */
   XmtDialogGetDialogValues(qmon_sconf, &data);

/**
printf("<-data.algorithm: '%s'\n", data.algorithm ? data.algorithm : "-NA-");
printf("<-data.schedule_interval: '%s'\n", data.schedule_interval ? data.schedule_interval : "-NA-");
printf("<-data.sc_params: '%s'\n", data.sc_params ? data.sc_params : "-NA-");
printf("<-data.maxujobs: '%d'\n", data.maxujobs );
printf("<-data.flush_submit_secs: '%d'\n", data.flush_submit_secs );
printf("<-data.flush_finish_secs: '%d'\n", data.flush_finish_secs );
printf("<-data.max_reservation: '%d'\n", data.max_reservation );
printf("<-data.queue_sort_method: '%d'\n", data.queue_sort_method );
printf("<-data.load_adjustment_decay_time: '%s'\n", data.load_adjustment_decay_time ? data.load_adjustment_decay_time : "-NA-");
printf("<-data.load_formula: '%s'\n", data.load_formula ? data.load_formula : "-NA-");
**/

   if (!data.algorithm || data.algorithm[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Algorithm required!}");
      goto error_exit;
   }
   lSetString(sep, SC_algorithm, data.algorithm);
  
   if (!data.schedule_interval || data.schedule_interval[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Schedule Interval required!}");
      goto error_exit;
   }
   lSetString(sep, SC_schedule_interval, data.schedule_interval);
  
   lSetString(sep, SC_params, data.sc_params);

   lSetUlong(sep, SC_maxujobs, (u_long32) data.maxujobs);

   lSetUlong(sep, SC_flush_submit_sec, (u_long32) data.flush_submit_secs);
   lSetUlong(sep, SC_flush_finish_sec, (u_long32) data.flush_finish_secs);

   lSetUlong(sep, SC_max_reservation, (u_long32) data.max_reservation);
  
   lSetUlong(sep, SC_queue_sort_method, (u_long32) data.queue_sort_method);

   /*
   ** load adjustments need special treatment
   */
   lSetList(sep, SC_job_load_adjustments, data.job_load_adjustments);
   data.job_load_adjustments = NULL;

   if (!data.load_adjustment_decay_time || 
         data.load_adjustment_decay_time[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Load Adjustment Decay Time required!}");
      goto error_exit;
   }
   lSetString(sep, SC_load_adjustment_decay_time, 
                  data.load_adjustment_decay_time);
  
   if (!data.load_formula || data.load_formula[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Load Formula required!}");
      goto error_exit;
   }
   lSetString(sep, SC_load_formula, data.load_formula);
  
   if (!data.reprioritize_interval|| 
         data.reprioritize_interval[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Reprioritize Interval required!}");
      goto error_exit;
   }
   lSetString(sep, SC_reprioritize_interval, data.reprioritize_interval);

   if (!data.default_duration|| 
         data.default_duration[0] == '\0') {
      qmonMessageShow(qmon_sconf, True, "@{Default duration required!}");
      goto error_exit;
   }
   lSetString(sep, SC_default_duration, data.default_duration);
   /*
   ** schedd_job_info needs some extras
   ** see comment for schedd_job_info in qmonScheddSet
   */
   job_info = XmtChooserGetState(sconf_job_info);
   switch (job_info) {
      case 0:
         lSetString(sep, SC_schedd_job_info, "false");
         break;
      case 1:
         lSetString(sep, SC_schedd_job_info, "true");
         break;
      case 2:
         str = XmtInputFieldGetString(sconf_job_range);
         {
            lList *range_list = NULL;

            range_list_parse_from_string(&range_list, &alp, str,
                                         1, 0, INF_NOT_ALLOWED);
            lFreeList(&range_list);
         }
         if (alp) {
            qmonMessageShow(sconf_job_range, True, "%s",
                            (StringConst)lGetString(lFirst(alp), AN_text));
            lFreeList(&alp);
            goto error_exit;
         }
         if (str && str[0] != '\0') {
            strcpy(buf, "job_list ");
            strcat(buf, str);
            lSetString(sep, SC_schedd_job_info, buf);
         }
         else {
            qmonMessageShow(qmon_sconf, True, "@{Job Range required!}");
            goto error_exit;
         }
   }

/* printf("------> qmonSchedGet\n"); */
/* lWriteElemTo(sep, stdout);    */

   qmonSchedFreeData();
   DRETURN(True);
error_exit:
   qmonSchedFreeData();
   DRETURN(False);
}


/*-------------------------------------------------------------------------*/
static void qmonLoadNamesSC(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   lList *cl = NULL;
   lList *ehl = NULL;
   lList *entries = NULL;
   lListElem *hep = NULL;
   static lCondition *where = NULL;
   lList *alp = NULL;

   DENTER(GUI_LAYER, "qmonLoadNamesSC");

   qmonMirrorMultiAnswer(CENTRY_T | EXECHOST_T, &alp);
   if (alp) {
      qmonMessageBox(w, alp, 0);
      lFreeList(&alp);
      DEXIT;
      return;
   }
   cl = qmonMirrorList(SGE_CE_LIST);
   ehl = qmonMirrorList(SGE_EH_LIST);

   for_each (hep, ehl) {
      lList *temp_entries = NULL;
      host_complexes2scheduler(&temp_entries, hep, ehl, cl);  
      if (entries == NULL) {
         entries = temp_entries;
      }
      else {
         lAddList(entries, &temp_entries);
      }
   }
  
   lUniqStr(entries, CE_name);
  
   if (!where)
      where = lWhere("%T(%I == %u || %I == %u || %I == %u || %I == %u)", 
                     CE_Type, CE_valtype, TYPE_INT, CE_valtype, TYPE_TIM, 
                     CE_valtype, TYPE_MEM, CE_valtype, TYPE_DOUBLE);
   entries = lSelectDestroy(entries, where);
         
   ShowLoadNames(w, entries);

   /*
   ** free the copied list
   */
   lFreeList(&entries);
}

/*-------------------------------------------------------------------------*/
static void qmonSchedJobInfo(
Widget w,
XtPointer cld,
XtPointer cad  
) {
   XmtChooserCallbackStruct *cbs = (XmtChooserCallbackStruct *) cad;
 
   DENTER(GUI_LAYER, "qmonSchedJobInfo");

   if (cbs->state != 2)
      XtSetSensitive(sconf_job_range, False);
   else
      XtSetSensitive(sconf_job_range, True);
   

   DEXIT;
}

static void qmonSchedFreeData(void)
{
   sge_free(&(data.algorithm));
   sge_free(&(data.schedule_interval));
   sge_free(&(data.sc_params));
   sge_free(&(data.load_adjustment_decay_time));
   sge_free(&(data.load_formula));
   sge_free(&(data.reprioritize_interval));
   sge_free(&(data.default_duration));
}
