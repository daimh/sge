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
#include <fnmatch.h>

#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>

#include <Xmt/Xmt.h>
#include <Xmt/Create.h>
#include <Xmt/InputField.h>

#include "qmon_rmon.h"
#include "qmon_cull.h"
#include "qmon_jobcustom.h"
#include "qmon_job.h"
#include "qmon_util.h"
#include "qmon_request.h"
#include "qmon_comm.h"
#include "qmon_widgets.h"
#include "qmon_preferences.h"
#include "qmon_message.h"
#include "Matrix.h" 
#include "IconList.h"

#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/sge_htable.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_usage.h"

#include "sched/sge_sched.h"      

#include "sge.h"
#include "symbols.h"
#include "qstat_printing.h"
#include "sge_cqueue_qstat.h"

/*-------------------------------------------------------------------------*/
/* Prototypes */
static void okCB(Widget w, XtPointer cld, XtPointer cad);
static void cancelCB(Widget w, XtPointer cld, XtPointer cad);
static void addToSelected(Widget w, XtPointer cld, XtPointer cad);
static void rmFromSelected(Widget w, XtPointer cld, XtPointer cad);

static String PrintUlong(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintDoubleAsUlong(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintDouble(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintDoubleOpti(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintPriority(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintString(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintTime(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintStartTime(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintBool(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintGrantedQueue(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintJobTaskId(lListElem *ep, lListElem *jat, lList *eleml, int nm);
/* static String PrintJobId(lListElem *ep, lListElem *jat, */
/*                               lList *eleml,  int nm); */
/* static String PrintTaskId(lListElem *ep, lListElem *jat, */
/*                               lList *eleml,  int nm); */
static String PrintStatus(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintHold(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintMailOptions(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintPathList(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintCPU(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintMEM(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintVMEM(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintMAXVMEM(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintIO(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintMailList(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintMEMValue(lListElem *ep, lListElem *jat, lList *eleml, int
nm, const char *s);

static String PrintPERange(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintJobArgs(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintPredecessors(lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintPredecessorsNr( lListElem *ep, lListElem *jat, lList *eleml, int nm);
static String PrintRestart(lListElem *ep, lListElem *jat, lList *eleml, int nm);                           
static void SetJobLabels(Widget w);
static void qmonWhatSetItems(Widget list, int how);
static void qmonJobFilterSet(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobFilterClear(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobFilterEditResource(Widget w, XtPointer cld, XtPointer cad);
static void qmonJobFilterRemoveResource(Widget w, XtPointer cld, XtPointer cad);

static int is_owner_ok(lListElem *jep, lList *owner_list);
static int is_job_runnable_on_queues(lListElem *jep, lList *queue_list, lList *exec_host_list, lList *complex_list, lList *request_list);
/*-------------------------------------------------------------------------*/
/* selectable items, the names must match the names in sge_jobL.h          */
/* there are fixed columns at the beginning of the natrix                  */
/*-------------------------------------------------------------------------*/

static htable JobColumnPrintHashTable = NULL;
static htable NameMappingHashTable = NULL;

#define FIRST_FIELD     6

static tJobField job_items[] = {
   { 1, JB_job_number, "@{Id}", 12, 20, PrintJobTaskId }, 
/*    { 1, JB_ja_tasks, "@{TaskId(s)}", 8, 20, PrintTaskId },  */
   { 1, JB_priority, "@{Priority}", 8, 20, PrintPriority },
   { 1, JB_job_name, "@{Name}", 10, 50, PrintString },
   { 1, JB_owner, "@{Owner}", 10, 50, PrintString },
   { 1, JAT_status, "@{Status}", 7, 30, PrintStatus },
   { 1, 0, "@{Queue}", 16, 500, PrintGrantedQueue},
   { 0, JB_submission_time, "@{SubmitTime}", 19, 30, PrintTime },
   { 0, JAT_start_time, "@{StartTime}", 19, 30, PrintStartTime },
   { 0, JB_execution_time, "@{ScheduleTime}", 19, 30, PrintTime },
   { 0, JB_account, "@{AccountString}", 15, 50, PrintString },
#if 0 /* JG: removed JB_cell from job object */   
   { 0, JB_cell, "@{Cell}", 10, 30, PrintString },
#endif   
   { 0, JB_cwd, "@{CWD}", 10, 30, PrintString },
   { 0, JB_stderr_path_list, "@{StderrPaths}", 15, 100, PrintPathList },
   { 0, JAT_hold, "@{Hold}", 10, 30, PrintHold },
/*    { 0, JB_reserve, "@{Reserve}", 11, 30, PrintBool }, */
   { 0, JB_merge_stderr, "@{MergeOutput}", 11, 30, PrintBool },
   { 0, JB_mail_options, "@{MailOptions}", 11, 30, PrintMailOptions },
   { 0, JB_mail_list, "@{MailTo}", 15, 30, PrintMailList },
   { 0, JB_notify, "@{Notify}", 6, 30, PrintBool },
   { 0, JB_stdout_path_list, "@{StdoutPaths}", 15, 50, PrintPathList },
   { 0, JB_restart, "@{Restart}", 10, 30, PrintRestart },
   { 0, JB_job_args, "@{JobArgs}", 15, 30, PrintJobArgs},
   { 0, JB_pe, "@{PE}", 10, 30, PrintString },
   { 0, JB_pe_range, "@{PERange}", 15, 30, PrintPERange },
   { 0, JB_jid_request_list, "@{Predecessors Req}", 12, 30, PrintPredecessors },
   { 0, JB_jid_predecessor_list, "@{Predecessors}", 12, 30, PrintPredecessorsNr },
   { 0, JB_ja_ad_request_list, "@{Array Predecessors Req}", 12, 30, PrintPredecessors },
   { 0, JB_ja_ad_predecessor_list, "@{Array Predecessors}", 12, 30, PrintPredecessorsNr },
   { 0, JAT_scaled_usage_list, "@{CPU}", 10, 30, PrintCPU },
   { 0, JAT_scaled_usage_list, "@{MEM}", 10, 30, PrintMEM },
   { 0, JAT_scaled_usage_list, "@{IO}", 10, 30, PrintIO },
   { 0, JAT_scaled_usage_list, "@{VMEM}", 10, 30, PrintVMEM },
   { 0, JAT_scaled_usage_list, "@{MAXVMEM}", 10, 30, PrintMAXVMEM },
/**** EE specific fields *****/
   { 0, JAT_tix, "@{Ticket}", 10, 30, PrintDoubleAsUlong},
   { 0, JAT_ntix, "@{N Ticket}", 10, 30, PrintDouble},
   { 0, JAT_oticket, "@{OTicket}", 10, 30, PrintDoubleAsUlong},
   { 0, JAT_fticket, "@{FTicket}", 10, 30, PrintDoubleAsUlong },
   { 0, JAT_sticket, "@{STicket}", 10, 30, PrintDoubleAsUlong },
   { 0, JAT_share, "@{Share}", 10, 30, PrintDouble },
   { 0, JB_override_tickets, "@{OverrideTickets}", 15, 30, PrintUlong },
   { 0, JB_jobshare, "@{JobShare}", 10, 30, PrintUlong },
   { 0, JB_project, "@{Project}", 10, 30, PrintString },
   { 0, JB_department, "@{Department}", 10, 30, PrintString },
   { 0, JB_deadline, "@{Deadline}", 10, 30, PrintTime },
   { 0, JB_nppri, "@{N Priority}", 10, 30, PrintDouble },
   { 0, JB_reserve, "@{Reservation}", 10, 30, PrintBool },
/**** EE urgency specific fields *****/
   { 0, JB_nurg, "@{N Urgency}", 10, 30, PrintDouble },
   { 0, JB_urg, "@{Urgency}", 10, 30, PrintDoubleOpti },
   { 0, JB_rrcontr, "@{rrcontr}", 10, 30, PrintDoubleOpti },
   { 0, JB_wtcontr, "@{wtcontr}", 10, 30, PrintDoubleOpti },
   { 0, JB_dlcontr, "@{dlcontr}", 10, 30, PrintDoubleOpti },
   { 0, JB_dlcontr, "@{dlcontr}", 10, 30, PrintDoubleOpti },
};

/*
** this list restricts the selected jobs, queues displayed
*/
static lList *jobfilter_resources = NULL;
static lList *jobfilter_owners = NULL;
static lList *jobfilter_fields = NULL;
static Boolean jobfilter_compact = True;


static Widget jcu = 0;
static Widget jobfield = 0;
static Widget jobfilter_ar = 0;
static Widget jobfilter_sr = 0;
static Widget jobfilter_owner = 0;
static Widget jobfilter_arrays_compressed = 0;
static Widget jobfield_available = 0;
static Widget jobfield_selected = 0;

/*-------------------------------------------------------------------------*/
static String PrintUlong(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintUlong");

   if (nm >= JAT_LOWERBOUND && nm <= JAT_UPPERBOUND) {
      int show_value = 0;
 
      if (eleml && !jat) {
         lListElem *first_elem = lFirst(eleml);
 
         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(eleml);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(eleml, NULL);
 
            jat = job_get_ja_task_template(ep, task_id);
            show_value = 1;
         }
      } else if (jat) {
         lList *n_h_ids = lGetList(ep, JB_ja_n_h_ids);
         u_long32 task_id = range_list_get_first_id(n_h_ids, NULL);
 
         /*
          * only show value for the first pending task of a job
          */
         if (task_id == lGetUlong(jat, JAT_task_number)) {
            show_value = 1;
         }
      }
 
      if (show_value) {
         sprintf(buf, "%d", (int)lGetUlong(jat, nm));
      } else {
         sprintf(buf, "NA");
      }                      
   } else {
      sprintf(buf, "%d", (int)lGetUlong(ep, nm));
   }

   str = XtNewString(buf);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintDoubleAsUlong(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintDoubleAsUlong");

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {
      if (nm >= JAT_LOWERBOUND && nm <= JAT_UPPERBOUND) {
         int show_value = 0;

         if (eleml && !jat) {
            lListElem *first_elem = lFirst(eleml);
       
            if (object_has_type(first_elem, JAT_Type)) {
               jat = lFirst(eleml);
            } else if (object_has_type(first_elem, RN_Type)) {
               u_long32 task_id = range_list_get_first_id(eleml, NULL);
       
               jat = job_get_ja_task_template(ep, task_id);
               show_value = 1;
            }
         } else if (jat) {
            u_long32 given_task_id = lGetUlong(jat, JAT_task_number);
            int is_enrolled = job_is_enrolled(ep, given_task_id);

            if (is_enrolled) {
               show_value = 1;
            } else {
               lList *n_h_ids = lGetList(ep, JB_ja_n_h_ids); 
               u_long32 task_id = range_list_get_first_id(n_h_ids, NULL);

               /*
                * only show value for the first pending task of a job
                */
               if (task_id == given_task_id) {
                  show_value = 1;
               }
            }
         }

         if (show_value) {
            sprintf(buf, "%d", (int)lGetDouble(jat, nm));
         } else {
            sprintf(buf, "NA");
         }
      } else {
         sprintf(buf, "%d", (int)lGetDouble(ep, nm));
      }

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintDouble(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintDouble");

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {  
      if (nm >= JAT_LOWERBOUND && nm <= JAT_UPPERBOUND) {
         int show_value = 0;
    
         if (eleml && !jat) {
            lListElem *first_elem = lFirst(eleml);
    
            if (object_has_type(first_elem, JAT_Type)) {
               jat = lFirst(eleml);
            } else if (object_has_type(first_elem, RN_Type)) {
               u_long32 task_id = range_list_get_first_id(eleml, NULL);
    
               jat = job_get_ja_task_template(ep, task_id);
               show_value = 1;
            }
         } else if (jat) {
            u_long32 given_task_id = lGetUlong(jat, JAT_task_number);
            int is_enrolled = job_is_enrolled(ep, given_task_id);
 
            if (is_enrolled) {
               show_value = 1;
            } else {
               lList *n_h_ids = lGetList(ep, JB_ja_n_h_ids);
               u_long32 task_id = range_list_get_first_id(n_h_ids, NULL);
 
               /*
                * only show value for the first pending task of a job
                */
               if (task_id == given_task_id) {
                  show_value = 1;
               }
            }         
         }
    
         if (show_value) {
            sprintf(buf, "%f", lGetDouble(jat, nm));
         } else {
            sprintf(buf, "NA");
         }                                
      }
      else
         sprintf(buf, "%f", lGetDouble(ep, nm));

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintDoubleOpti(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintDoubleOpti");

#define OPTI_PRINT8(value) \
   if (value > 99999999 ) \
      sprintf(buf, "%8.3g ", value); \
   else  \
      sprintf(buf, "%8.0f ", value)

   OPTI_PRINT8(lGetDouble(ep, nm));
   str = XtNewString(buf);

#undef OPTI_PRINT8

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintBool(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;

   DENTER(GUI_LAYER, "PrintBool");

   if (lGetBool(ep, nm))
      str = XtNewString("True");
   else
      str = XtNewString("False");

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintJobArgs(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lList *job_args;
   lListElem *jep = NULL;
   char buf[ 10 * BUFSIZ];

   DENTER(GUI_LAYER, "PrintJobArgs");

   job_args = lGetList(ep, nm);
   
   strcpy(buf, "");
   for_each(jep, job_args) {
      const char *arg = lGetString(jep, ST_name);
      if(arg != NULL) {
         snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %s", arg);
      } else {
         sge_strlcat(buf, " \"\"", sizeof(buf));
      }
   }
   str = XtNewString(buf);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintPredecessors(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lList *pred;
   lListElem *jep = NULL;
   char buf[ 100 * BUFSIZ];

   DENTER(GUI_LAYER, "PrintPredecessors");

   pred = lGetList(ep, nm);
   
   strcpy(buf, "");
   for_each(jep, pred) {
      snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " %s",
               lGetString(jep, JRE_job_name));
   }
   str = XtNewString(buf);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintPredecessorsNr(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lList *pred;
   lListElem *jep = NULL;
   char buf[ 100 * BUFSIZ];

   DENTER(GUI_LAYER, "PrintPredecessors");

   pred = lGetList(ep, nm);
   
   strcpy(buf, "");
   for_each(jep, pred) {
      snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), " "sge_u32,
               lGetUlong(jep, JRE_job_number));
   }
   str = XtNewString(buf);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintPERange(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   dstring range_string = DSTRING_INIT;
   String str;

   DENTER(GUI_LAYER, "PrintPERange");

   range_list_print_to_string(lGetList(ep, nm), &range_string, true, false, false);
   str = XtNewString(sge_dstring_get_string(&range_string));
   sge_dstring_free(&range_string);

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintPathList(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lList *path_list = NULL;

   DENTER(GUI_LAYER, "PrintPathList");

   path_list = lGetList(ep, nm); 

   str = XtNewString(write_pair_list(path_list, PATH_TYPE));

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintCPU(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lListElem *up = NULL;
   char buf[1024];
   u_long32 running;

   DENTER(GUI_LAYER, "PrintCPU");
   
   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {
      if (eleml && !jat) {
         lListElem *first_elem = lFirst(eleml);

         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(eleml);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(eleml, NULL);

            jat = job_get_ja_task_template(ep, task_id);
         }
      }

      running = lGetUlong(jat, JAT_status)==JRUNNING ||
                  lGetUlong(jat, JAT_status)==JTRANSFERING;

      /* scaled cpu usage */
      if (!(up = lGetSubStr(jat, UA_name, USAGE_ATTR_CPU, JAT_scaled_usage_list)))
         sprintf(buf, "%-10.10s ", running?"NA":"");
      else {
         int secs, minutes, hours, days;

         secs = lGetDouble(up, UA_value);

         days    = secs/(60*60*24);
         secs   -= days*(60*60*24);

         hours   = secs/(60*60);
         secs   -= hours*(60*60);

         minutes = secs/60;
         secs   -= minutes*60;

         sprintf(buf, "%d:%2.2d:%2.2d:%2.2d", days, hours, minutes, secs);
      }

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintMEM(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   return PrintMEMValue(ep, jat, eleml, nm, USAGE_ATTR_MEM);
}   

/*-------------------------------------------------------------------------*/
static String PrintVMEM(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   return PrintMEMValue(ep, jat, eleml, nm, USAGE_ATTR_VMEM);
}   

/*-------------------------------------------------------------------------*/
static String PrintMAXVMEM(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   return PrintMEMValue(ep, jat, eleml, nm, USAGE_ATTR_MAXVMEM);
}   

/*-------------------------------------------------------------------------*/
static String PrintMEMValue(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm,
const char *field 
) {
   String str;
   lListElem *up = NULL;
   char buf[1024];
   u_long32 running;

   DENTER(GUI_LAYER, "PrintMEMValue");

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {  
      if (eleml && !jat) {
         lListElem *first_elem = lFirst(eleml);
    
         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(eleml);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(eleml, NULL);
    
            jat = job_get_ja_task_template(ep, task_id);
         }
      }     

      running = lGetUlong(jat, JAT_status)==JRUNNING ||
                  lGetUlong(jat, JAT_status)==JTRANSFERING;

      /* scaled mem usage */
      if (!(up = lGetSubStr(jat, UA_name, field, JAT_scaled_usage_list))) {
         sprintf(buf, "%-7.7s", running?"NA":"");
      } else {
         dstring mem_string = DSTRING_INIT;

         double_print_memory_to_dstring(lGetDouble(up, UA_value), &mem_string);
         sprintf(buf, "%s", sge_dstring_get_string(&mem_string));
         sge_dstring_free(&mem_string);
      }

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintIO(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lListElem *up = NULL;
   char buf[1024];
   u_long32 running;

   DENTER(GUI_LAYER, "PrintIO");

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {  
      if (eleml && !jat) {
         lListElem *first_elem = lFirst(eleml);
    
         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(eleml);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(eleml, NULL);
    
            jat = job_get_ja_task_template(ep, task_id);
         }
      }     

      running = lGetUlong(jat, JAT_status)==JRUNNING ||
                  lGetUlong(jat, JAT_status)==JTRANSFERING;

      /* scaled io usage */
      if (!(up = lGetSubStr(jat, UA_name, USAGE_ATTR_IO,
         JAT_scaled_usage_list)))
         sprintf(buf, "%-7.7s", running?"NA":"");
      else
         sprintf(buf, "%-5.5f", lGetDouble(up, UA_value));

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintMailList(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;
   lList *mail_list = NULL;

   DENTER(GUI_LAYER, "PrintMailList");

   mail_list = lGetList(ep, nm); 

   str = XtNewString(write_pair_list(mail_list, MAIL_TYPE));

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintMailOptions(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;
   u_long32 ca;
   static u_long32 mail_at[] = { MAIL_AT_BEGINNING, MAIL_AT_EXIT,
                                 MAIL_AT_ABORT, MAIL_AT_SUSPENSION};
   static char mailsym[] ="beas";
   int i;

   DENTER(GUI_LAYER, "PrintMailOptions");

   ca = lGetUlong(ep, nm);
   strcpy(buf, "");
   
   if (ca) {
      for (i=0; i<4; i++) {
         if (ca & mail_at[i])
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", mailsym[i]);
      }
   }
   else
      strcpy(buf, "None"); 
   
   str = XtNewString(buf);

   DEXIT;
   return str;
}
   
#if 0
/*-------------------------------------------------------------------------*/
static String PrintCheckpointAttr(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;
   u_long32 ca;
   u_long32 interval;


   DENTER(GUI_LAYER, "PrintCheckpointAttr");

   ca = lGetUlong(ep, JB_checkpoint_attr);
   interval = lGetUlong(ep, JB_checkpoint_interval);

   strcpy(buf, "NOT AVAIL");
   
   str = XtNewString(buf);

   DEXIT;
   return str;
}
#endif
   
/*-------------------------------------------------------------------------*/
static String PrintGrantedQueue(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str = NULL;
   lList *ql = NULL;
   lListElem *qgep = NULL;
   int n, len;
   StringConst queue;

   DENTER(GUI_LAYER, "PrintGrantedQueue");

   /*
   ** display the queue of the first task in the task list
   */

#if 0   
   if (jat) {
      n = 1;
      queue = lGetString(jat, JAT_master_queue);
   }
   else {
      n = 0;
   }
   
   if (n == 0) 
      str = XtNewString("*pending*");
   else
      str = XtNewString(queue);
#endif

   if (eleml && !jat) {
      lListElem *first_elem = lFirst(eleml);
 
      if (object_has_type(first_elem, JAT_Type)) {
         jat = lFirst(eleml);
      } else if (object_has_type(first_elem, RN_Type)) {
         u_long32 task_id = range_list_get_first_id(eleml, NULL);
 
         jat = job_get_ja_task_template(ep, task_id);
      }
   }

   if (job_is_zombie_job(ep)) {
      str = XtNewString("*finished*"); 
   } else {
      if (jat) {
         ql = lGetList(jat, JAT_granted_destin_identifier_list);
         n = lGetNumberOfElem(ql);
      } else {
         n = 0;
      }
      
      if (n == 0 && lGetUlong(jat, JAT_status) == JIDLE) {
         str = XtNewString("*pending*");
      } else {
         str = XtNewString("-");
      }

      if (n >= 1) {
         if (lGetString(ep, JB_pe)) {
            str = XtNewString("P:");
         }
         else
            str = XtNewString("");

         for_each(qgep, ql) {
            queue = lGetString(qgep, JG_qname);
            len = 1 + (str?strlen(str):0) + (queue?strlen(queue):0);
            str = XtRealloc((char*)str, len+1); 
            strcat(str, queue);
            strcat(str, " ");
         }
      }      
   }

   DEXIT;
   return str;
}

#if 0
/*-------------------------------------------------------------------------*/
static String PrintTaskId(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintTaskId");

   /*
   ** prepare task ids, if the job contains only one job array task the job id!    ** is sufficient
   */
   if (jat) {
      sprintf(buf, sge_u32, lGetUlong(jat, JAT_task_number));
   }
   else if (eleml) {
      buf[0] = '\0';
      ja_task_list_print_to_string(eleml, buf);
   }

   str = XtNewString(buf);

   DEXIT;
   return str;
}
 
/*-------------------------------------------------------------------------*/
static String PrintJobId(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintJobId");

   sprintf(buf, sge_u32, lGetUlong(ep, JB_job_number));

   str = XtNewString(buf);

   DEXIT;
   return str;
}
#endif

/*-------------------------------------------------------------------------*/
static String PrintJobTaskId(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   dstring dyn_buf = DSTRING_INIT;
   String str;

   DENTER(GUI_LAYER, "PrintJobTaskId");
   /*
   ** prepare task ids, if the job contains only one job array task the job id!    ** is sufficient
   */
   sge_dstring_sprintf(&dyn_buf, sge_u32, lGetUlong(ep, JB_job_number));
   if (job_is_array(ep)) {
      dstring dyn_buf2 = DSTRING_INIT;
      const char* tmp_string = NULL;

      if (jat) {
         sge_dstring_sprintf(&dyn_buf2, sge_u32, lGetUlong(jat, JAT_task_number)); 
      } else if (eleml) {
         lListElem *first_elem = lFirst(eleml);

         if (object_has_type(first_elem, JAT_Type)) {
            ja_task_list_print_to_string(eleml, &dyn_buf2);
         } else if (object_has_type(first_elem, RN_Type)) {
            range_list_print_to_string(eleml, &dyn_buf2, false, false, false);
         }
      }
      tmp_string = sge_dstring_get_string(&dyn_buf2);
      if (tmp_string) {
         sge_dstring_append(&dyn_buf, ".");
         sge_dstring_append(&dyn_buf, tmp_string);
         sge_dstring_free(&dyn_buf2);
      }
   }

   DPRINTF(("PrintJobTaskId: %s\n", sge_dstring_get_string(&dyn_buf)));

   str = XtNewString(sge_dstring_get_string(&dyn_buf));

   sge_dstring_free(&dyn_buf);

   DEXIT;
   return str;
}
 
/*-------------------------------------------------------------------------*/
static String PrintStatus(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   lList *ql = NULL;
   int n;
   char buf[128] = "";
   String str;
   u_long32 tstate = 0;
   u_long32 tstatus = 0;

   DENTER(GUI_LAYER, "PrintStatus");

   if (eleml && !jat) {
      lListElem *first_elem = lFirst(eleml);
 
      if (object_has_type(first_elem, JAT_Type)) {
         jat = lFirst(eleml);
      } else if (object_has_type(first_elem, RN_Type)) {
         u_long32 task_id = range_list_get_first_id(eleml, NULL);
 
         jat = job_get_ja_task_template(ep, task_id);  
      }
   }

   if (jat) {
      ql = lGetList(jat, JAT_granted_destin_identifier_list);
      n = lGetNumberOfElem(ql);
   }
   else {
      n = 0;
   }

   if (job_is_zombie_job(ep)) {    
      str = XtNewString("NA"); 
   } else {
      /* move status info into state info */
      tstatus = lGetUlong(jat, JAT_status);
      tstate = lGetUlong(jat, JAT_state);

      if (tstatus==JRUNNING) {
         tstate |= JRUNNING;
         tstate &= ~JTRANSFERING;
      } else if (tstatus==JTRANSFERING) {
         tstate |= JTRANSFERING;
         tstate &= ~JRUNNING;
      } else if (tstatus==JFINISHED) {
         tstate |= JEXITING;
         tstate &= ~(JRUNNING|JTRANSFERING);
      }

      /* check suspension of queue */
      if (n>0) {
         if ((tstate & JSUSPENDED_ON_SUBORDINATE) ||
             (tstate & JSUSPENDED_ON_SLOTWISE_SUBORDINATE)) {
            tstate &= ~JRUNNING;
            lSetUlong(jat, JAT_state, tstate);
         }
      }
         
      if (lGetList(ep, JB_jid_predecessor_list) || lGetUlong(jat, JAT_hold)) {
         tstate |= JHELD;
      }

      if (lGetUlong(jat, JAT_job_restarted)) {
         tstate &= ~JWAITING;
         tstate |= JMIGRATING;
      }

      /* write states into string */
      job_get_state_string(buf, tstate);

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintHold(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;
   int value = 0;
   char* hold_string[] = {"u", "o", "s"};
   int i;

   DENTER(GUI_LAYER, "PrintHold");

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {
      if (eleml && !jat) {
         lListElem *first_elem = lFirst(eleml);
    
         if (object_has_type(first_elem, JAT_Type)) {
            jat = lFirst(eleml);
         } else if (object_has_type(first_elem, RN_Type)) {
            u_long32 task_id = range_list_get_first_id(eleml, NULL);
    
            jat = job_get_ja_task_template(ep, task_id);
         }         
      }

      value = (int)lGetUlong(jat, nm);
      strcpy(buf, "");
      for (i=0; i<3; i++) {
         if (value & (1<<i))
            strcat(buf, hold_string[i]);
      }

      str = XtNewString(buf);
   }

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintPriority(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   char buf[BUFSIZ];
   String str;

   DENTER(GUI_LAYER, "PrintPriority");

   if (!jat) {
      lListElem *first_elem = lFirst(eleml);

      if (object_has_type(first_elem, JAT_Type)) {
         jat = lFirst(eleml);
      } else if (object_has_type(first_elem, RN_Type)) {
         u_long32 task_id = range_list_get_first_id(eleml, NULL);

         jat = job_get_ja_task_template(ep, task_id);       
      }
   }
   sprintf(buf, "%7.5f", lGetDouble(jat, JAT_prio));

   str = XtNewString(buf);

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintString(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {

   String str;
   StringConst temp;
   
   DENTER(GUI_LAYER, "PrintString");

   temp = lGetString(ep, nm); 
   
   str = XtNewString(temp?temp:"");

   DEXIT;
   return str;
}
   
/*-------------------------------------------------------------------------*/
static String PrintTime(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {

   String str;
   dstring ds;
   char buffer[128];

   DENTER(GUI_LAYER, "PrintTime");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (lGetUlong(ep, nm))
      str = XtNewString(sge_ctime(lGetUlong(ep, nm), &ds));
   else
      str = XtNewString("");

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintRestart(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {
   String str;

   DENTER(GUI_LAYER, "PrintRestart");

   switch (lGetUlong(ep, nm)) {
      case 1:
         str = XtNewString("True");
         break;
      case 2:
         str = XtNewString("False");
         break;
      default:
         str = XtNewString("Depends on queue");
   }

   DEXIT;
   return str;
}

/*-------------------------------------------------------------------------*/
static String PrintStartTime(
lListElem *ep,
lListElem *jat,
lList *eleml,
int nm 
) {

   String str;
   dstring ds;
   char buffer[128];

   DENTER(GUI_LAYER, "PrintTime");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if (job_is_zombie_job(ep)) {
      str = XtNewString("NA");
   } else {
      if (jat && lGetUlong(jat, nm)) {
         str = XtNewString(sge_ctime(lGetUlong(jat, nm), &ds));
      } else {
         str = XtNewString("");
      }
   }

   DEXIT;
   return str;
}
                                                                              
   
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
String* PrintJobField(
lListElem *ep,
lListElem *jat,
lList *eleml,
int cols 
) {
   int i;
   String* row = NULL;
   int col;
   String entry;

   DENTER(GUI_LAYER, "PrintJobField");

   /*
   ** check col against XtNumber(job_items)
   ** if cols > XtNumber(job_items) we screw the whole thing up
   */

   row = (String*)XtMalloc(sizeof(String)*cols);

   if (ep) {
      for (i=0, col=0; row && i<XtNumber(job_items) && col<cols; i++) {
         if (job_items[i].show) {
            if (job_items[i].printJobField &&
               (entry = job_items[i].printJobField(ep, jat, eleml, job_items[i].nm))) { 
               row[col] = entry;
            }
            else {
               row[col] = XtNewString("");
            }
            col++;
         }
      }
   }
   else {
      for (col=0; row && col<cols; col++) {
         row[col] = XtNewString("");
      }
   }

   DEXIT;
   return row;
}

/*-------------------------------------------------------------------------*/
static void okCB(Widget w, XtPointer cld, XtPointer cad)
{
   int nr_fields = 0;
   int i, j;
   XmString *strlist;
   Widget run_m;
   Widget pen_m;
   Widget zombie_m;
   String owner_str = NULL;
   tJobField *job_item = NULL;
   
   DENTER(GUI_LAYER, "okCB");

   XtVaGetValues( jobfield_selected, 
                  XmNitemCount, &nr_fields, 
                  XmNitems, &strlist,
                  NULL);

   /*
   ** reset show flag
   */ 
   for (j=FIRST_FIELD; j<XtNumber(job_items); j++) {           
      job_items[j].show = 0; 
   }



   for (i=0; i<nr_fields; i++) {
      if (strlist[i]) {
         String text;
         u_long32 temp;
         XmStringGetLtoR(strlist[i], XmFONTLIST_DEFAULT_TAG, &text);
         temp = XrmStringToQuark(text);
         if (sge_htable_lookup(NameMappingHashTable, 
                             &temp,
                             (const void **) &job_item)) {
            job_item->show = 1;
         }
         XtFree(text);
         DPRINTF(("job_item[%d].name: <%s> show = %d\n",
                  i, job_items[i].name, job_items[i].show));
      }
   }        

   qmonJobReturnMatrix(&run_m, &pen_m, &zombie_m);
   SetJobLabels(run_m);
   SetJobLabels(pen_m);
   SetJobLabels(zombie_m);
   
   /*
   ** get the strings to do a wildmatch against
   */
   owner_str = XmtInputFieldGetString(jobfilter_owner);
   lFreeList(&jobfilter_owners);
   lString2List(owner_str, &jobfilter_owners, ST_Type, ST_name, NULL); 

   jobfilter_compact = XmToggleButtonGetState(jobfilter_arrays_compressed);

   updateJobList();

   xmui_unmanage(jcu);

   DEXIT;
}
   
/*-------------------------------------------------------------------------*/
static void cancelCB(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(GUI_LAYER, "cancelCB");
   
   qmonWhatSetItems(jobfield_selected, FILL_SELECTED);
   xmui_unmanage(jcu);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void saveCB(Widget w, XtPointer cld, XtPointer cad)
{
   lList *lp = NULL;
   lList *alp = NULL;
   int j;

   DENTER(GUI_LAYER, "saveCB");
   
   okCB(w, NULL, NULL);

   lSetList(qmonGetPreferences(), PREF_job_filter_resources, 
                     lCopyList("", jobfilter_resources));
   lSetList(qmonGetPreferences(), PREF_job_filter_owners, 
                     lCopyList("", jobfilter_owners));
   lSetBool(qmonGetPreferences(), PREF_job_filter_compact, jobfilter_compact);
   for (j=FIRST_FIELD; j<XtNumber(job_items); j++) {           
      if (job_items[j].show)
         lAddElemStr(&lp, ST_name, job_items[j].name, ST_Type); 
   }
   lSetList(qmonGetPreferences(), PREF_job_filter_fields, lp);

   alp = qmonWritePreferences();

   qmonMessageBox(w, alp, 0);

   lFreeList(&alp);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void addToSelected(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *items;
   Cardinal itemCount;
   int i;

   DENTER(GUI_LAYER, "addToSelected");

   XtVaGetValues( jobfield_available,
                  XmNselectedItems, &items,
                  XmNselectedItemCount, &itemCount,
                  NULL);

   for (i=0; i<itemCount; i++) {
      if (! XmListItemExists(jobfield_selected, items[i]))
         XmListAddItem(jobfield_selected, items[i], 0); 
   }
                     
   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void rmFromSelected(Widget w, XtPointer cld, XtPointer cad)
{
   XmString *items;
   Cardinal itemCount;

   DENTER(GUI_LAYER, "rmFromSelected");

   XtVaGetValues( jobfield_selected,
                  XmNselectedItems, &items,
                  XmNselectedItemCount, &itemCount,
                  NULL);
   
   if (itemCount)
      XmListDeleteItems(jobfield_selected, items, itemCount);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void SetJobLabels(
Widget w 
) {
   int cols;
   int col;
   int i;
   String *labels = NULL;
   short *widths = NULL;
   int *max_lengths = NULL;
   int additional = 0;
   Screen *screen = XtScreen(w);

   DENTER(GUI_LAYER, "SetJobLabels");

   for (i=FIRST_FIELD; i<XtNumber(job_items); i++) {
      if (job_items[i].show)
         additional++;
   }

   XtVaGetValues( w,
                  XmNcolumns, &cols,
                  NULL);

   /*
   ** delete the additional columns
   */
   if (cols > FIRST_FIELD)
      XbaeMatrixDeleteColumns(w, FIRST_FIELD, cols - FIRST_FIELD);

   if (additional > 0) {
      labels = (String*)XtMalloc(sizeof(String)*additional);
      widths = (short*)XtMalloc(sizeof(short)*additional);
      max_lengths = (int*)XtMalloc(sizeof(int)*additional);
      for (i=FIRST_FIELD, col=0; i<XtNumber(job_items) && col<additional; i++) {
         if (job_items[i].show) {
            String str, s, category, tag, defaultstr, free_me = NULL;
            str = job_items[i].name; 
            /* if this string has a tag, localize it first */
            if ((str[0] == '@') && (str[1] == '{')) {
               s = XtNewString(str);
               free_me = s;
               s += 2;
               category = NULL;
               tag = NULL;
               defaultstr = s;
               while(*s) {
                  if (*s == '.' && !category) {
                     *s = '\0';
                     category = defaultstr;
                     defaultstr = s+1;
                  }
                  if (*s == '.' && !tag) {
                     *s = '\0';
                     tag = defaultstr;
                     defaultstr = s+1;
                  }
                  if (*s == '}') {
                     *s = '\0';
                     s++;
                     break;
                  }
                  s++;
               }
               if (!tag)
                  tag = defaultstr;
/*                if (!tag[0]) goto error; */
/*                if (category && !category[0]) goto error; */
               s = _XmtLocalize(screen, defaultstr, category, tag);
/* printf("--> '%s'\n", s);                  */
            }
            else {
               s = (String)str;
            }
    
            labels[col] = XtNewString(s);
            widths[col] = job_items[i].width;
            max_lengths[col] = job_items[i].max_length;
            if (free_me) XtFree(free_me);
            col++;
         } 
      }
      XbaeMatrixAddColumns(w,
                           FIRST_FIELD,
                           NULL,
                           labels,
                           widths,
                           max_lengths,
                           NULL,
                           NULL,
                           NULL,
                           additional);
      for(i=0; i<additional; i++) {
         XtFree(labels[i]);
      }   
      XtFree((char*)labels);
      XtFree((char*)widths);
      XtFree((char*)max_lengths);
   }
                           

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonWhatSetItems(
Widget list,
int how 
) {
/*    XmString *items; */
/*    Cardinal itemCount = 0; */
/*    int i, j; */
   int num_jobs, i;

   DENTER(GUI_LAYER, "qmonWhatSetItems");

   num_jobs = XtNumber(job_items);

#if 0
   if (how == FILL_ALL)
      itemCount = num_jobs - FIRST_FIELD;
   else
      for (i=FIRST_FIELD; i<num_jobs; i++) {
         if (job_items[i].show)
            itemCount++;
      } 

   items = (XmString*)XtMalloc(sizeof(XmString)*itemCount);
   for (i=FIRST_FIELD,j=0; i<num_jobs && j<itemCount; i++) {
      if (how == FILL_ALL) {
         items[j] = XmtCreateLocalizedXmString(list, job_items[i].name); 
         j++;
      }
      else {
         if (job_items[i].show) {
            items[j] = XmtCreateLocalizedXmString(list, job_items[i].name);
            j++;
         }
      }
   }
         
   XtVaSetValues( list, 
                  XmNitems, items,
                  XmNitemCount, itemCount,
                  NULL);
   XmStringTableFree(items, itemCount);

#else   

   XtVaSetValues( list, 
                  XmNitems, NULL,
                  XmNitemCount, 0,
                  NULL);
   for (i=FIRST_FIELD; i<num_jobs; i++) {
      if (how == FILL_ALL) {
         DPRINTF(("XmListAddItemUniqueSorted: '%s'\n", job_items[i].name));
         XmListAddItemUniqueSorted(list, job_items[i].name);
      }
      else {
         if (job_items[i].show) {
            XmListAddItemUniqueSorted(list, job_items[i].name);
         }
      }
   }
         
#endif

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonPopupJCU(Widget w, XtPointer cld, XtPointer cad)
{
   DENTER(TOP_LAYER, "qmonPopupJCU");

   if (!jcu)
      qmonCreateJCU(w, NULL);
   xmui_manage(jcu);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
void qmonCreateJCU(
Widget parent,
XtPointer cld 
) {
   Widget   jcu_ok, jcu_cancel, jcu_save, jcu_folder,
            jobfield_add, jobfield_remove, jobfilter_clear;
   Widget run_m;
   Widget pen_m;
   Widget zombie_m;
   int i;
   char buf[BUFSIZ * 10];
   int first_time = 1;
 
   DENTER(GUI_LAYER, "qmonCreateJCU");

   /*
   ** the resource file for qc_dialog_shell must contain 
   ** qc_dialog as unmanaged XmtLayout child otherwise
   ** it is managed by XmtBuildQueryDialog and managed again
   ** in qmonPopupJCU
   */
   jcu = XmtBuildQueryDialog(parent, "jcu_shell",
                           NULL, 0,
                           "jcu_folder", &jcu_folder,
                           "jobfield_selected", &jobfield_selected,
                           "jobfield_available", &jobfield_available,
                           "jobfield_add", &jobfield_add,
                           "jobfield_remove", &jobfield_remove,
                           "jcu_ok", &jcu_ok,
                           "jcu_cancel", &jcu_cancel,
                           "jcu_save", &jcu_save,
                           "jobfilter_ar", &jobfilter_ar,
                           "jobfilter_sr", &jobfilter_sr,
                           "jobfilter_owner", &jobfilter_owner,
                           "jobfilter_arrays_compressed", 
                                    &jobfilter_arrays_compressed,
                           "jobfilter_clear", &jobfilter_clear,
                           NULL);
   /*
   ** create JobColumnPrintHashTable
   */
   JobColumnPrintHashTable = sge_htable_create(5, dup_func_u_long32, hash_func_u_long32, hash_compare_u_long32);
   for (i=0; i<sizeof(job_items)/sizeof(tJobField); i++) {
      u_long32 temp = XrmStringToQuark(job_items[i].name);
      sge_htable_store(JobColumnPrintHashTable,
                     &temp,
                     (void *)&job_items[i]);
   }

   /*
   ** create NameMappingHashTable
   */
   NameMappingHashTable = sge_htable_create(5, dup_func_u_long32, hash_func_u_long32, hash_compare_u_long32);
   for (i=0; i<sizeof(job_items)/sizeof(tJobField); i++) {
      String text;
      u_long32 temp;
      XmString xstr = XmtCreateLocalizedXmString(jcu, job_items[i].name);
      XmStringGetLtoR(xstr, XmFONTLIST_DEFAULT_TAG, &text);
      temp = XrmStringToQuark(text);
      sge_htable_store(NameMappingHashTable,
                     &temp,
                     (void *)&job_items[i]);
      XmStringFree(xstr);
      XtFree(text);
   }

   /*
   ** reference to preferences
   */
   if (qmonGetPreferences()) {
      lListElem *field;
      lListElem *ep;
      tJobField *job_item = NULL;

      jobfilter_resources = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_job_filter_resources));
      jobfilter_owners = lCopyList("", lGetList(qmonGetPreferences(), 
                                          PREF_job_filter_owners));
      jobfilter_compact = lGetBool(qmonGetPreferences(), PREF_job_filter_compact);
      jobfilter_fields = lCopyList("", lGetList(qmonGetPreferences(),
                                          PREF_job_filter_fields));

/* lWriteListTo(jobfilter_resources, stdout); */
/* lWriteListTo(jobfilter_owners, stdout); */
/* lWriteListTo(jobfilter_fields, stdout); */


      /*
      ** preset the owners
      */
      strcpy(buf, "");
      for_each(ep, jobfilter_owners) {
         if (first_time) {
            first_time = 0;
            strcpy(buf, lGetString(ep, ST_name));
         }
         else
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ",%s",
                     lGetString(ep, ST_name));
      }
      XmtInputFieldSetString(jobfilter_owner, buf);


      /*
      ** set the fields which shall be shown
      */
      for_each(field, jobfilter_fields) {
         u_long32 temp = XrmStringToQuark(lGetString(field, ST_name));
         if (sge_htable_lookup(JobColumnPrintHashTable, 
             &temp,
             (const void **) &job_item)) {
            job_item->show = 1;
         }
      }
   }

   /*
   ** preset the resources 
   */
   qmonJobFilterSet(jcu, NULL, NULL);

   /*
   ** preset the fields
   */
   qmonWhatSetItems(jobfield_available, FILL_ALL); 
   qmonWhatSetItems(jobfield_selected, FILL_SELECTED); 

   /*
   ** preset the column labels
   */
   qmonJobReturnMatrix(&run_m, &pen_m, &zombie_m);
   SetJobLabels(run_m);
   SetJobLabels(pen_m);
   SetJobLabels(zombie_m);
   
                           
   XtAddCallback(jcu_ok, XmNactivateCallback,
                        okCB, NULL);
   XtAddCallback(jcu_cancel, XmNactivateCallback,
                        cancelCB, NULL);
   XtAddCallback(jcu_save, XmNactivateCallback,
                        saveCB, NULL);
   XtAddCallback(jobfield_add, XmNactivateCallback,
                        addToSelected, (XtPointer) jobfield);
   XtAddCallback(jobfield_remove, XmNactivateCallback,
                        rmFromSelected, (XtPointer) jobfield);

   /*
   ** jobfilter callbacks
   */
   XtAddCallback(jobfilter_ar, XmNactivateCallback,
                        qmonJobFilterEditResource, (XtPointer)0);
   XtAddCallback(jobfilter_sr, XmNactivateCallback,
                        qmonJobFilterEditResource, (XtPointer)1);
   XtAddCallback(jobfilter_sr, XmNremoveCallback, 
                     qmonJobFilterRemoveResource, NULL);
   XtAddCallback(jobfilter_clear, XmNactivateCallback,
                        qmonJobFilterClear, NULL);
   XtAddCallback(jcu_folder, XmNvalueChangedCallback, 
                     qmonJobFilterSet, NULL);

   XmToggleButtonSetState(jobfilter_arrays_compressed, jobfilter_compact, False);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonJobFilterClear(Widget w, XtPointer cld, XtPointer cad)
{


   DENTER(GUI_LAYER, "qmonJobFilterClear");

   lFreeList(&jobfilter_resources);
   qmonRequestDraw(jobfilter_sr, jobfilter_resources, 1);

   lFreeList(&jobfilter_owners);
   XmtInputFieldSetString(jobfilter_owner, "");
   
   XmToggleButtonSetState(jobfilter_arrays_compressed, 1, False);

   DEXIT;

}

/*-------------------------------------------------------------------------*/
static void qmonJobFilterSet(Widget w, XtPointer cld, XtPointer cad)
{
   lList *arl = NULL;
   lListElem *ep = NULL;
   lListElem *rp = NULL;

   DENTER(GUI_LAYER, "qmonJobFilterSet");

   arl = qmonGetResources(qmonMirrorList(SGE_CE_LIST), ALL_RESOURCES);

   for_each (ep, jobfilter_resources) {
      rp = lGetElemStr(arl, CE_name, lGetString(ep, CE_name));
      if (!rp)
         rp = lGetElemStr(arl, CE_shortcut, lGetString(ep, CE_name));
      if (rp) {
         lSetString(ep, CE_name, lGetString(rp, CE_name));
         lSetUlong(ep, CE_valtype, lGetUlong(rp, CE_valtype));
      }
   }
      
   /*
   ** do the drawing
   */
   qmonRequestDraw(jobfilter_ar, arl, 0);
   qmonRequestDraw(jobfilter_sr, jobfilter_resources, 1);

   lFreeList(&arl);

   DEXIT;
}


/*-------------------------------------------------------------------------*/
static void qmonJobFilterEditResource(Widget w, XtPointer cld, XtPointer cad)
{
   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;
   long how = (long)cld;
   lList *arl = NULL;
   int type;
   char stringval[CL_MAXHOSTLEN];
   Boolean status = False;
   StringConst name, strval;
   Boolean found = False;
   lListElem *fill_in_request = NULL;
   lListElem *global_fill_in_request = NULL;
   

   DENTER(GUI_LAYER, "qmonJobFilterEditResource");

   arl = qmonGetResources(qmonMirrorList(SGE_CE_LIST), ALL_RESOURCES);


   if (!how) {
      global_fill_in_request = lGetElemStr(arl, CE_name, cbs->element->string[0]);
   }
   for_each (fill_in_request, jobfilter_resources) {
      name = lGetString(fill_in_request, CE_name);
      if (cbs->element->string[0] && name && 
         !strcmp(cbs->element->string[0], name)) {
            found = True;
            break;
      }
   }       
         
   if (!found) {
      fill_in_request = global_fill_in_request; 
   }

   if (!fill_in_request) {
      DEXIT;
      return;
   }

   type = lGetUlong(fill_in_request, CE_valtype);
   strval = lGetString(fill_in_request, CE_stringval);
   sge_strlcpy(stringval, strval, CL_MAXHOSTLEN);

   status = qmonRequestInput(w, type, cbs->element->string[0], 
                              stringval, sizeof(stringval));
   /* 
   ** put the value in the CE_Type elem 
   */
   if (status) {
      lListElem *ep = NULL;
      lSetString(fill_in_request, CE_stringval, stringval);
    
      /* put it in the hard or soft resource list if necessary */
      if (!jobfilter_resources) {
         jobfilter_resources = lCreateList("jobfilter_resources", CE_Type);
      }
      if (!(ep = lGetElemStr(jobfilter_resources, CE_name, cbs->element->string[0]))) {
         lAppendElem(jobfilter_resources, lCopyElem(fill_in_request));
      } else {
         lSetString(ep, CE_stringval, stringval);
      }   
         
      qmonRequestDraw(jobfilter_sr, jobfilter_resources, 1);
   }
   lFreeList(&arl);

   DEXIT;
}

/*-------------------------------------------------------------------------*/
static void qmonJobFilterRemoveResource(Widget w, XtPointer cld, XtPointer cad)
{

   XmIconListCallbackStruct *cbs = (XmIconListCallbackStruct*) cad;
   lListElem *dep = NULL;
   Boolean found = False;
   StringConst name, value;

   DENTER(GUI_LAYER, "qmonJobFilterRemoveResource");

   if (jobfilter_resources) {
      for_each (dep, jobfilter_resources) {
         name = lGetString(dep, CE_name);
         value = lGetString(dep, CE_stringval);
         if (cbs->element->string[0] && name && 
            !strcmp(cbs->element->string[0], name) &&
            cbs->element->string[2] && value &&
            !strcmp(cbs->element->string[2], value) ) {
               found = True;
               break;
         }
      }       
            
      if (found) {
         lRemoveElem(jobfilter_resources, &dep);
         if (lGetNumberOfElem(jobfilter_resources) == 0)
            lFreeList(&jobfilter_resources);
         qmonRequestDraw(jobfilter_sr, jobfilter_resources, 1);
      }
   }
   
   DEXIT;
}

/*-------------------------------------------------------------------------*/
lList* qmonJobFilterResources(void)
{
   return jobfilter_resources;
}

/*-------------------------------------------------------------------------*/
lList* qmonJobFilterOwners(void)
{
   return jobfilter_owners;
}

/*-------------------------------------------------------------------------*/
int qmonJobFilterArraysCompressed(void)
{
   return jobfilter_compact;
}


/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* this function destroys the delivered queue list partially               */
/*-------------------------------------------------------------------------*/
int match_queue(
lList **queue_list,
lList *request_list,
lList *centry_list,
lList *exechost_list 
) {
   lList *filtered_queue_list = NULL;
   static lCondition *tagged_queues = NULL;
   static lEnumeration *all_fields = NULL;
   u_long32 empty_qs = 0; 

   DENTER(GUI_LAYER, "match_queue");

   if (!exechost_list) {
      DPRINTF(("empty exechost_list\n"));
      DEXIT;
      return False;
   } 

   if (!tagged_queues) {
      tagged_queues = lWhere("%T(%I == %u)", CQ_Type, CQ_tag, TAG_SHOW_IT);
      all_fields = lWhat("%T(ALL)", CQ_Type);
   }
   centry_list_init_double(centry_list);

   /*
   ** remove the queues not fulfilling the request_list
   */

   /* all queues are selected */
   cqueue_list_set_tag(*queue_list, TAG_SHOW_IT, true);

   if (select_by_resource_list(request_list, exechost_list, *queue_list, centry_list, empty_qs)<0) {
      DEXIT;
      return False;
   }
   if (!is_cqueue_selected(*queue_list)) {
      lFreeList(queue_list);
      DEXIT;
      return True;
   }

   filtered_queue_list = lSelect("FQL", *queue_list, tagged_queues, all_fields);  
   lFreeList(queue_list);
   *queue_list = filtered_queue_list;

   DEXIT;
   return True;
}
         

/*-------------------------------------------------------------------------*/
/* this function destroys the delivered job list partially               */
/*-------------------------------------------------------------------------*/
int match_job(
lList **job_list,
lList *owner_list,
lList *queue_list, 
lList *complex_list,
lList *exec_host_list,
lList *request_list 
) {
   lListElem *jep;
   lListElem *dep;
   lListElem *jatep;
   int show;

   DENTER(GUI_LAYER, "match_job");

   jep = lFirst(*job_list);
   while (jep) {
      /*
      ** first of all we assume that the job should be displayed
      */
      show = 1;
      
      /* 
      ** all tasks are suitable 
      */
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
            lSetUlong(jatep, JAT_suitable, TAG_SHOW_IT);
      }

      /*
      ** check if job fulfills user_list and set show flag
      */
      if (show && owner_list)
         show = is_owner_ok(jep, owner_list);

      /*
      ** is job runnable on queues fulfilling requests, dito
      */
      if (show && request_list) {
         show = is_job_runnable_on_queues(jep, queue_list, exec_host_list,
                                             complex_list, request_list);
      }

      if (show) {
         jep = lNext(jep);
      }
      else {
         dep = jep;
         jep = lNext(jep);
         lRemoveElem(*job_list, &dep);
      }
   } 

   if (lGetNumberOfElem(*job_list) == 0) {
      lFreeList(job_list);
   }

   DEXIT;
   return True;
}
         
/*-------------------------------------------------------------------------*/
static int is_owner_ok(
lListElem *jep,
lList *owner_list 
) {
   lListElem *op;

   if (!owner_list)
      return True;

   for_each(op, owner_list) {
      if (!fnmatch(lGetString(op, ST_name), lGetString(jep, JB_owner), 0)) {
         return True;
      }
   }
   return False;
}

/*-------------------------------------------------------------------------*/
static int is_job_runnable_on_queues(
lListElem *jep,
lList *queue_list,
lList *exechost_list,
lList *centry_list,
lList *request_list 
) {
   lList *hard_resource_list=NULL;   
   u_long32 empty_qs = 0; 
   Boolean found_task = False;
   lListElem *jatep;

   DENTER(GUI_LAYER, "is_job_runnable_on_queues");

   /*
   ** 
   */
   if (!queue_list) {
      return False;
   }   

   /* all queues are selected */
   cqueue_list_set_tag(queue_list, TAG_SHOW_IT, true);

   hard_resource_list = lGetList(jep, JB_hard_resource_list);
   
#if 0   
   /*
   ** if the job has no requests all queues fit
   */
   if (!hard_resource_list) {
      return True;
   }
#endif   

   /*
   ** see if queues fulfill the request_list of the job
   */
   if (select_by_resource_list(hard_resource_list, exechost_list, queue_list, centry_list, empty_qs)<0) {
      DEXIT;
      return False;
   }
   if (!is_cqueue_selected(queue_list)) {
      DEXIT;
      return False;
   }
   
   /*
   ** show pending jobs in any case
   */
   if(!lGetList(jep, JB_ja_tasks)) {
      found_task = True;
   }   

   if (select_by_resource_list(request_list, exechost_list, queue_list, centry_list, empty_qs)<0) {
      DEXIT;
      return False;
   }
   if (!is_cqueue_selected(queue_list)) {
      DEXIT;
      return False;
   }

   /*
   ** see if job is running on specified queue otherwise don't show
   */
   for_each(jatep, lGetList(jep, JB_ja_tasks)) {
      if (lGetUlong(jatep, JAT_status) == JRUNNING || 
            lGetUlong(jatep, JAT_status) == JTRANSFERING) {
         lListElem *cq;
         for_each(cq, queue_list) {
            lListElem *qep;
            for_each(qep, lGetList(cq, CQ_qinstances)) {
               if (lGetUlong(qep, QU_tag) == 1) {
                  dstring queue_name_buffer = DSTRING_INIT;
                  const char *qnm = qinstance_get_name(qep, &queue_name_buffer);
                  lListElem *gdilep;
                  for_each(gdilep, lGetList(jatep, JAT_granted_destin_identifier_list)) {
                     if (!strcmp(lGetString(gdilep, JG_qname), qnm)) {
                        found_task = True;
                        break;
                     }
                  }
                  sge_dstring_free(&queue_name_buffer);
               }
            }
         }

         if (!found_task) {
            lSetUlong(jatep, JAT_suitable, lGetUlong(jatep, JAT_suitable) & ~TAG_SHOW_IT);
         }   
     }
   }   
   if (!found_task) {
      DEXIT;
      return False;
   }   

   DEXIT;
   return True; 
}
