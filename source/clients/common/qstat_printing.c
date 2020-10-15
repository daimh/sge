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
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_time.h"
#include "uti/sge_prog.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_usage.h"

#include "sched/sge_sched.h"
#include "sched/sge_urgency.h"

#include "sge.h"
#include "show_job.h"
#include "qstat_printing.h"
#include "sig_handlers.h"
#include "msg_clients_common.h"

static int sge_print_job(lListElem *job, lListElem *jatep, lListElem *qep, int print_jobid, 
                         char *master, dstring *task_str, u_long32 full_listing, int
                         slots, int slot, lList *ehl, lList *cl, const lList *pe_list, 
                         char *intend, u_long32 group_opt, int slots_per_line, 
                         int queue_name_length,
                         qhost_report_handler_t *report_handler,
                         lList **alpp);

static int sge_print_subtask(lListElem *job, lListElem *ja_task, lListElem *task, int print_hdr, int indent);


static char hashes[] = "##############################################################################################################";

static int sge_print_subtask(
lListElem *job,
lListElem *ja_task,
lListElem *pe_task,  /* NULL, if master task shall be printed */
int print_hdr,
int indent 
) {
   char task_state_string[8];
   u_long32 tstate, tstatus;
   int task_running;
   const char *str;
   lListElem *ep;
   lList *usage_list;
   lList *scaled_usage_list;

   DENTER(TOP_LAYER, "sge_print_subtask");

   /* is sub-task logically running */
   if(pe_task == NULL) {
      tstatus = lGetUlong(ja_task, JAT_status);
      usage_list = lGetList(ja_task, JAT_usage_list);
      scaled_usage_list = lGetList(ja_task, JAT_scaled_usage_list);
   } else {
      tstatus = lGetUlong(pe_task, PET_status);
      usage_list = lGetList(pe_task, PET_usage);
      scaled_usage_list = lGetList(pe_task, PET_scaled_usage);
   }

   task_running = (tstatus==JRUNNING || tstatus==JTRANSFERING);

   if (print_hdr) {
      printf(QSTAT_INDENT "Sub-tasks:           %-12.12s %5.5s %s %-4.4s %-6.6s\n", 
             "task-ID",
             "state",
             USAGE_ATTR_CPU "        " USAGE_ATTR_MEM "     " USAGE_ATTR_IO "     ",
             "stat",
             "failed");
   }

   if(pe_task == NULL) {
      str = "";
   } else {
      str = lGetString(pe_task, PET_id);
   }
   printf("   %s%-12s ", indent?QSTAT_INDENT2:"", str);

   /* move status info into state info */
   tstate = lGetUlong(ja_task, JAT_state);
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

   if (lGetList(job, JB_jid_predecessor_list) || lGetUlong(ja_task, JAT_hold)) {
      tstate |= JHELD;
   }

   if (lGetUlong(ja_task, JAT_job_restarted)) {
      tstate &= ~JWAITING;
      tstate |= JMIGRATING;
   }

   /* write states into string */ 
   job_get_state_string(task_state_string, tstate);
   printf("%-5.5s ", task_state_string); 

   {
      lListElem *up;

      /* scaled cpu usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_CPU))) {
         printf("%-10.10s ", task_running?"NA":""); 
      } else {
         dstring resource_string = DSTRING_INIT;

         double_print_time_to_dstring(lGetDouble(up, UA_value), 
                                      &resource_string);
         printf("%s ", sge_dstring_get_string(&resource_string));
         sge_dstring_free(&resource_string);
      }

      /* scaled mem usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_MEM))) 
         printf("%-7.7s ", task_running?"NA":""); 
      else
         printf("%-5.5f ", lGetDouble(up, UA_value)); 
  
      /* scaled io usage */
      if (!(up = lGetElemStr(scaled_usage_list, UA_name, USAGE_ATTR_IO))) 
         printf("%-7.7s ", task_running?"NA":""); 
      else
         printf("%-5.5f ", lGetDouble(up, UA_value)); 
   }

   if (tstatus==JFINISHED) {
      ep=lGetElemStr(usage_list, UA_name, "exit_status");

      printf("%-4d", ep ? (int)lGetDouble(ep, UA_value) : 0);
   }

   putchar('\n');

   DRETURN(0);
}

/*-------------------------------------------------------------------------*/
/* print jobs per queue                                                    */
/*-------------------------------------------------------------------------*/
/* actually just called by qhost */
int sge_print_jobs_queue(
lListElem *qep,
lList *job_list,
const lList *pe_list,
lList *user_list,
lList *ehl,
lList *centry_list,
int print_jobs_of_queue,
u_long32 full_listing,
char *indent,
u_long32 group_opt, 
int queue_name_length,
qhost_report_handler_t *report_handler, 
lList **alpp
) {
   lListElem *jlep;
   lListElem *jatep;
   lListElem *gdilep;
   u_long32 job_tag;
   u_long32 jid = 0, old_jid;
   u_long32 jataskid = 0, old_jataskid;
   const char *qnm;
   dstring dyn_task_str = DSTRING_INIT;

   DENTER(TOP_LAYER, "sge_print_jobs_queue");

   qnm = lGetString(qep, QU_full_name);

   for_each(jlep, job_list) {
      int master, i;

      for_each(jatep, lGetList(jlep, JB_ja_tasks)) {
         u_long32 jstate = lGetUlong(jatep, JAT_state);

         if (shut_me_down) {
            DRETURN(1);
         }

         if (ISSET(jstate, JSUSPENDED_ON_SUBORDINATE) ||
             ISSET(jstate, JSUSPENDED_ON_SLOTWISE_SUBORDINATE)) {
            lSetUlong(jatep, JAT_state, jstate & ~JRUNNING);
         }

         gdilep = lGetElemStr(lGetList(jatep, JAT_granted_destin_identifier_list), JG_qname, qnm);
         if (gdilep != NULL) {
            int slot_adjust = 0;
            int lines_to_print;
            int slots_per_line, slots_in_queue = lGetUlong(gdilep, JG_slots); 

            job_tag = lGetUlong(jatep, JAT_suitable);
            job_tag |= TAG_FOUND_IT;
            lSetUlong(jatep, JAT_suitable, job_tag);

            master = !strcmp(qnm, 
                  lGetString(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_qname));

            if (master) {
               const char *pe_name;
               lListElem *pe;
               if (((pe_name=lGetString(jatep, JAT_granted_pe))) &&
                   ((pe=pe_list_locate(pe_list, pe_name))) &&
                   !lGetBool(pe, PE_job_is_first_task))

                   slot_adjust = 1;
            }

            /* job distribution view ? */
            if (!(group_opt & GROUP_NO_PETASK_GROUPS)) {
               /* no - condensed ouput format */
               if (!master && !(full_listing & QSTAT_DISPLAY_FULL)) {
                  /* skip all slave outputs except in full display mode */
                  continue;
               }

               /* print only on line per job for this queue */
               lines_to_print = 1;

               /* always only show the number of job slots represented by the line */
               if ((full_listing & QSTAT_DISPLAY_FULL))
                  slots_per_line = slots_in_queue;
               else
                  slots_per_line = sge_granted_slots(lGetList(jatep, JAT_granted_destin_identifier_list));
            } else {
               /* yes */
               lines_to_print = (int)slots_in_queue+slot_adjust;
               slots_per_line = 1;
            }

            for (i=0; i<lines_to_print ;i++) {
               int already_printed = 0;

               if (!lGetNumberOfElem(user_list) || 
                  (lGetNumberOfElem(user_list) && (lGetUlong(jatep, JAT_suitable)&TAG_SELECT_IT))) {
                  if (print_jobs_of_queue && (job_tag & TAG_SHOW_IT)) {
                     int different, print_jobid;

                     old_jid = jid;
                     jid = lGetUlong(jlep, JB_job_number);
                     old_jataskid = jataskid;
                     jataskid = lGetUlong(jatep, JAT_task_number);
                     sge_dstring_sprintf(&dyn_task_str, sge_u32, jataskid);
                     different = (jid != old_jid) || (jataskid != old_jataskid);

                     if (different) 
                        print_jobid = 1;
                     else {
                        if (!(full_listing & QSTAT_DISPLAY_RUNNING))
                           print_jobid = master && (i==0);
                        else 
                           print_jobid = 0;
                     }

                     if (!already_printed && (full_listing & QSTAT_DISPLAY_RUNNING) &&
                           (lGetUlong(jatep, JAT_state) & JRUNNING)) {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);   
                        already_printed = 1;
                     }
                     if (!already_printed && (full_listing & QSTAT_DISPLAY_SUSPENDED) &&
                         ((lGetUlong(jatep, JAT_state)&JSUSPENDED) ||
                         (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_THRESHOLD) ||
                         (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_SUBORDINATE) ||
                         (lGetUlong(jatep, JAT_state)&JSUSPENDED_ON_SLOTWISE_SUBORDINATE))) {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);   
                        already_printed = 1;
                     }

                     if (!already_printed && (full_listing & QSTAT_DISPLAY_USERHOLD) &&
                         (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_USER)) {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);
                        already_printed = 1;
                     }

                     if (!already_printed && (full_listing & QSTAT_DISPLAY_OPERATORHOLD) &&
                         (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_OPERATOR))  {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);
                        already_printed = 1;
                     }
                         
                     if (!already_printed && (full_listing & QSTAT_DISPLAY_SYSTEMHOLD) &&
                         (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_SYSTEM)) {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);
                        already_printed = 1;
                     }

                     if (!already_printed && (full_listing & QSTAT_DISPLAY_JOBARRAYHOLD) &&
                         (lGetUlong(jatep, JAT_hold)&MINUS_H_TGT_JA_AD)) {
                        sge_print_job(jlep, jatep, qep, print_jobid,
                           (master && different && (i==0))?"MASTER":"SLAVE", &dyn_task_str, full_listing,
                           slots_in_queue+slot_adjust, i, ehl, centry_list, pe_list, indent, 
                           group_opt, slots_per_line, queue_name_length, report_handler, alpp);
                        already_printed = 1;
                     }
                  }
               }
            }
         }
      }
   }
   sge_dstring_free(&dyn_task_str);

   DRETURN(0);
}

void sge_printf_header(u_long32 full_listing, u_long32 sge_ext)
{
   static int first_pending = 1;
   static int first_zombie = 1;

   if ((full_listing & QSTAT_DISPLAY_PENDING) && 
       (full_listing & QSTAT_DISPLAY_FULL)) {
      if (first_pending) {
         first_pending = 0;
         printf("\n############################################################################%s\n",
            sge_ext?hashes:"");
         printf("%s\n", MSG_QSTAT_PRT_PEDINGJOBS);
         printf("############################################################################%s\n",
            sge_ext?hashes:"");
      }
   } 
   if ((full_listing & QSTAT_DISPLAY_ZOMBIES) &&
       (full_listing & QSTAT_DISPLAY_FULL)) {
      if (first_zombie) {
         first_zombie = 0;
         printf("\n############################################################################%s\n", sge_ext?hashes:"");
         printf("%s\n", MSG_QSTAT_PRT_FINISHEDJOBS);
         printf("############################################################################%s\n", sge_ext?hashes:""); 
      }
   }
}

#define OPTI_PRINT8(value) \
   if (value > 99999999 ) \
      printf("%8.3g ", value); \
   else  \
      printf("%8.0f ", value)

/* regular output */
static char jhul1[] = "---------------------------------------------------------------------------------------------";
/* -g t */
static char jhul2[] = "-";
/* -ext */
static char jhul3[] = "-------------------------------------------------------------------------------";
/* -t */
static char jhul4[] = "-----------------------------------------------------";
/* -urg */
static char jhul5[] = "----------------------------------------------------------------";
/* -pri */
static char jhul6[] = "-----------------------------------";

static int sge_print_job(
lListElem *job,
lListElem *jatep,
lListElem *qep,
int print_jobid,
char *master,
dstring *dyn_task_str,
u_long32 full_listing,
int slots,
int slot,
lList *exechost_list,
lList *centry_list,
const lList *pe_list,
char *indent,
u_long32 group_opt,
int slots_per_line,  /* number of slots to be printed in slots column 
                       when 0 is passed the number of requested slots printed */
int queue_name_length,
qhost_report_handler_t *report_handler,
lList **alpp
) {
   char state_string[8];
   static int first_time = 1;
   u_long32 jstate;
   int sge_urg, sge_pri, sge_ext, sge_time;
   lList *ql = NULL;
   lListElem *qrep, *gdil_ep=NULL;
   int running;
   const char *queue_name = NULL;
   const char *cqname = NULL;
   int tsk_ext;
   u_long tickets,otickets,stickets,ftickets;
   int is_zombie_job;
   dstring ds;
   char buffer[128];
   dstring queue_name_buffer = DSTRING_INIT;
   char jobid[128];
   int ret = QHOST_SUCCESS; 

   DENTER(TOP_LAYER, "sge_print_job");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   is_zombie_job = job_is_zombie_job(job);

   if (qep != NULL) {
      queue_name = qinstance_get_name(qep, &queue_name_buffer);
      cqname = lGetString(qep, QU_qname);
   }

   sge_ext = ((full_listing & QSTAT_DISPLAY_EXTENDED) == QSTAT_DISPLAY_EXTENDED);
   tsk_ext = (full_listing & QSTAT_DISPLAY_TASKS);
   sge_urg = (full_listing & QSTAT_DISPLAY_URGENCY);
   sge_pri = (full_listing & QSTAT_DISPLAY_PRIORITY);
   sge_time = !sge_ext;
   sge_time = sge_time | tsk_ext | sge_urg | sge_pri;


   if (!report_handler) {
      if (first_time) {
         first_time = 0;
         if (!(full_listing & QSTAT_DISPLAY_FULL)) {
            int line_length = queue_name_length-10+1;
            char * seperator = sge_malloc(line_length);
            const char *part1 = "%s%-7.7s %s %s%s%s%s%s %-10.10s %-12.12s %s%-5.5s %s%s%s%s%s%s%s%s%s%-";
            const char *part3 = ".";
            const char *part5 = "s %s %s%s%s%s%s%s";
            char *part6 = sge_malloc(strlen(part1) + strlen(part3) + strlen(part5) + 20);
            {
               int i;
               for(i=0; i<line_length; i++){
                  seperator[i] = '-';
               }
            }
            seperator[line_length-1] = '\0';
            sprintf(part6, "%s%d%s%d%s", part1, queue_name_length, part3, queue_name_length, part5);
         
            printf(part6,
                     indent,
                     "job-ID",
                     "prior ",
                  (sge_pri||sge_urg)?" nurg   ":"",
                  sge_pri?" npprior":"",
                  (sge_pri||sge_ext)?" ntckts ":"",
                  sge_urg?" urg      rrcontr  wtcontr  dlcontr ":"",
                  sge_pri?"  ppri":"",
                     "name",
                     "user",
                  sge_ext?"project          department ":"",
                     "state",
                  sge_time?"submit/start at     ":"",
                  sge_urg ? " deadline           " : "",
                  sge_ext ? USAGE_ATTR_CPU "        " USAGE_ATTR_MEM "     " USAGE_ATTR_IO "      " : "",
                  sge_ext?"tckts ":"",
                  sge_ext?"ovrts ":"",
                  sge_ext?"otckt ":"",
                  sge_ext?"ftckt ":"",
                  sge_ext?"stckt ":"",
                  sge_ext?"share ":"",
                     "queue",
                  (group_opt & GROUP_NO_PETASK_GROUPS)?"master":"slots",
                     "ja-task-ID ", 
                  tsk_ext?"task-ID ":"",
                  tsk_ext?"state ":"",
                  tsk_ext?USAGE_ATTR_CPU "        " USAGE_ATTR_MEM "     " USAGE_ATTR_IO "      " : "",
                  tsk_ext?"stat ":"",
                  tsk_ext?"failed ":"" );

            printf("\n%s%s%s%s%s%s%s%s\n", indent, 
                  jhul1, 
                  seperator,
                  (group_opt & GROUP_NO_PETASK_GROUPS)?jhul2:"",
                  sge_ext ? jhul3 : "", 
                  tsk_ext ? jhul4 : "",
                  sge_urg ? jhul5 : "",
                  sge_pri ? jhul6 : "");
                  
            sge_free(&part6);
            sge_free(&seperator);               
         }
      }

      printf("%s", indent);

      /* job number / ja task id */
      if (print_jobid) {
         printf("%7d ", (int)lGetUlong(job, JB_job_number)); 
      } else {
         printf("        "); 
      }   
   } else {
      snprintf(jobid, sizeof(jobid)-1, "%d", (int)lGetUlong(job, JB_job_number));
      ret = report_handler->report_job_begin(report_handler, cqname, jobid, alpp);
      if (ret != QHOST_SUCCESS ) {
         DRETURN(ret);
      }
   }


   /* per job priority information */
   {
      if (report_handler) {
         ret = report_handler->report_job_double_value(report_handler, cqname, jobid, "priority", lGetDouble(jatep, JAT_prio), alpp);
         if (ret != QHOST_SUCCESS) {
            DRETURN(ret);
         }
      } else {
         if (print_jobid) {
            printf("%7.5f ", lGetDouble(jatep, JAT_prio)); /* nprio 0.0 - 1.0 */
         } else {
            printf("        ");
         }
      }

      if (sge_pri || sge_urg) {
         if (print_jobid) {
            printf("%7.5f ", lGetDouble(job, JB_nurg)); /* nurg 0.0 - 1.0 */
         } else {
            printf("        ");
         }
      }

      if (sge_pri) {
         if (print_jobid)
            printf("%7.5f ", lGetDouble(job, JB_nppri)); /* nppri 0.0 - 1.0 */
         else
            printf("        ");
      }

      if (sge_pri || sge_ext) {
         if (print_jobid)
            printf("%7.5f ", lGetDouble(jatep, JAT_ntix)); /* ntix 0.0 - 1.0 */
         else
            printf("        ");
      }

      if (sge_urg) {
         if (print_jobid) {
            OPTI_PRINT8(lGetDouble(job, JB_urg));
            OPTI_PRINT8(lGetDouble(job, JB_rrcontr));
            OPTI_PRINT8(lGetDouble(job, JB_wtcontr));
            OPTI_PRINT8(lGetDouble(job, JB_dlcontr));
         } else {
            printf("         "
                   "         "
                   "         "
                   "         ");
         }
      } 

      if (sge_pri) {
         if (print_jobid) {
            printf("%5d ", ((int)lGetUlong(job, JB_priority))-BASE_PRIORITY); 
         } else {
            printf("         "
                   "         ");
         }
      }

   } 

   if (report_handler) {
      ret = report_handler->report_job_string_value(report_handler, cqname, jobid, "qinstance_name", queue_name, alpp);
      if (ret != QHOST_SUCCESS ) {
         DRETURN(ret);
      }
      ret = report_handler->report_job_string_value(report_handler, cqname, jobid, "job_name", lGetString(job, JB_job_name), alpp);
      if (ret != QHOST_SUCCESS ) {
         DRETURN(ret);
      }
      ret = report_handler->report_job_string_value(report_handler, cqname, jobid, "job_owner", lGetString(job, JB_owner), alpp);
      if (ret != QHOST_SUCCESS ) {
         DRETURN(ret);
      }
   } else {
      if (print_jobid) {
         /* job name */
         printf("%-10.10s ", lGetString(job, JB_job_name)); 

         /* job owner */
         printf("%-12.12s ", lGetString(job, JB_owner)); 
      } else {
         printf("           "); 
         printf("             "); 
      }
   }

   if (sge_ext) {
      const char *s;

      if (print_jobid) {
         /* job project */
         printf("%-16.16s ", (s=lGetString(job, JB_project))?s:"NA"); 
         /* job department */
         printf("%-10.10s ", (s=lGetString(job, JB_department))?s:"NA"); 
      } else {
         printf("                 "); 
         printf("           "); 
      }
   }

   /* move status info into state info */
   jstate = lGetUlong(jatep, JAT_state);
   if (lGetUlong(jatep, JAT_status)==JTRANSFERING) {
      jstate |= JTRANSFERING;
      jstate &= ~JRUNNING;
   }

   if (lGetList(job, JB_jid_predecessor_list) || lGetUlong(jatep, JAT_hold)) {
      jstate |= JHELD;
   }

   if (lGetUlong(jatep, JAT_job_restarted)) {
      jstate &= ~JWAITING;
      jstate |= JMIGRATING;
   }

   if (report_handler) {
      job_get_state_string(state_string, jstate);
      report_handler->report_job_string_value(report_handler, cqname, jobid, "job_state", state_string, alpp);
   } else {
      if (print_jobid) {
         /* write states into string */ 
         job_get_state_string(state_string, jstate);
         printf("%-5.5s ", state_string); 
      } else {
         printf("      "); 
      }
   }

   if (sge_time) {
      if (report_handler) {
         if (!lGetUlong(jatep, JAT_start_time)) {
            report_handler->report_job_ulong_value(report_handler, cqname, jobid, "submit_time", lGetUlong(job, JB_submission_time), alpp);
         } else {
            report_handler->report_job_ulong_value(report_handler, cqname, jobid, "start_time", lGetUlong(jatep, JAT_start_time), alpp);
         }
      } else {
         if (print_jobid) {
            /* start/submit time */
            if (!lGetUlong(jatep, JAT_start_time) ) {
               printf("%s ", sge_ctime((time_t)lGetUlong(job, JB_submission_time), &ds));
            }   
            else {
   #if 0
               /* AH: intermediate change to monitor JAT_stop_initiate_time 
                * must be removed before 6.0 if really needed a better possiblity 
                * for monitoring must be found (TODO)
                */
               if (getenv("JAT_stop_initiate_time") && (lGetUlong(jatep, JAT_state) & JDELETED))
                  printf("%s!", sge_ctime(lGetUlong(jatep, JAT_stop_initiate_time), &ds));
               else
   #endif
                  printf("%s ", sge_ctime((time_t)lGetUlong(jatep, JAT_start_time), &ds));
            }
         } else {
            printf("                    "); 
         }
      }
   }

   /* is job logically running */
   running = lGetUlong(jatep, JAT_status)==JRUNNING || 
      lGetUlong(jatep, JAT_status)==JTRANSFERING;

   /* deadline time */
   if (sge_urg) {
      if (print_jobid) { 
         if (!lGetUlong(job, JB_deadline) )
            printf("                    ");
         else
            printf("%s ", sge_ctime((time_t)lGetUlong(job, JB_deadline), &ds));
      } else {
         printf("                    "); 
      }
   }

   if (sge_ext) {
      lListElem *up, *pe, *task;
      lList *job_usage_list;
      const char *pe_name;
      
      if (!master || !strcmp(master, "MASTER"))
         job_usage_list = lCopyList(NULL, lGetList(jatep, JAT_scaled_usage_list));
      else
         job_usage_list = lCreateList("", UA_Type);

      /* sum pe-task usage based on queue slots */
      if (job_usage_list) {
         int subtask_ndx=1;
         for_each(task, lGetList(jatep, JAT_task_list)) {
            lListElem *dst, *src, *ep;
            const char *qname;

            if (!slots ||
                (queue_name && 
                 ((ep=lFirst(lGetList(task, PET_granted_destin_identifier_list)))) &&
                 ((qname=lGetString(ep, JG_qname))) &&
                 !strcmp(qname, queue_name) && ((subtask_ndx++%slots)==slot))) {
               for_each(src, lGetList(task, PET_scaled_usage)) {
                  if ((dst=lGetElemStr(job_usage_list, UA_name, lGetString(src, UA_name))))
                     lSetDouble(dst, UA_value, lGetDouble(dst, UA_value) + lGetDouble(src, UA_value));
                  else
                     lAppendElem(job_usage_list, lCopyElem(src));
               }
            }
         }
      }


      /* scaled cpu usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_CPU))) 
         printf("%-10.10s ", running?"NA":""); 
      else {
         int secs, minutes, hours, days;

         secs = lGetDouble(up, UA_value);

         days    = secs/(60*60*24);
         secs   -= days*(60*60*24);

         hours   = secs/(60*60);
         secs   -= hours*(60*60);

         minutes = secs/60;
         secs   -= minutes*60;
      
         printf("%d:%2.2d:%2.2d:%2.2d ", days, hours, minutes, secs); 
      } 
      /* scaled mem usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_MEM))) 
         printf("%-7.7s ", running?"NA":""); 
      else
         printf("%-5.5f ", lGetDouble(up, UA_value)); 
  
      /* scaled io usage */
      if (!(up = lGetElemStr(job_usage_list, UA_name, USAGE_ATTR_IO))) 
         printf("%-7.7s ", running?"NA":""); 
      else
         printf("%-5.5f ", lGetDouble(up, UA_value)); 

      lFreeList(&job_usage_list);

      /* get tickets for job/slot */
      /* braces needed to suppress compiler warnings */
      if ((pe_name=lGetString(jatep, JAT_granted_pe)) &&
           (pe=pe_list_locate(pe_list, pe_name)) &&
           lGetBool(pe, PE_control_slaves)
         && slots && (gdil_ep=lGetSubStr(jatep, JG_qname, queue_name,
               JAT_granted_destin_identifier_list))) {
         if (slot == 0) {
            tickets = (u_long)lGetDouble(gdil_ep, JG_ticket);
            otickets = (u_long)lGetDouble(gdil_ep, JG_oticket);
            ftickets = (u_long)lGetDouble(gdil_ep, JG_fticket);
            stickets = (u_long)lGetDouble(gdil_ep, JG_sticket);
         }
         else {
            if (slots) {
               tickets = (u_long)(lGetDouble(gdil_ep, JG_ticket) / slots);
               otickets = (u_long)(lGetDouble(gdil_ep, JG_oticket) / slots);
               ftickets = (u_long)(lGetDouble(gdil_ep, JG_fticket) / slots);
               stickets = (u_long)(lGetDouble(gdil_ep, JG_sticket) / slots);
            } 
            else {
               tickets = otickets = ftickets = stickets = 0;
            }
         }
      }
      else {
         tickets = (u_long)lGetDouble(jatep, JAT_tix);
         otickets = (u_long)lGetDouble(jatep, JAT_oticket);
         ftickets = (u_long)lGetDouble(jatep, JAT_fticket);
         stickets = (u_long)lGetDouble(jatep, JAT_sticket);
      }


      /* report jobs dynamic scheduling attributes */
      /* only scheduled have these attribute */
      /* Pending jobs can also have tickets */
      if (is_zombie_job) {
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
         printf("   NA ");
      } else {
         if (sge_ext || lGetList(jatep, JAT_granted_destin_identifier_list)) {
            printf("%5d ", (int)tickets),
            printf("%5d ", (int)lGetUlong(job, JB_override_tickets)); 
            printf("%5d ", (int)otickets);
            printf("%5d ", (int)ftickets);
            printf("%5d ", (int)stickets);
            printf("%-5.2f ", lGetDouble(jatep, JAT_share)); 
         } else {
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
            printf("      "); 
         }
      }
   }

   /* if not full listing we need the queue's name in each line */
   if (!(full_listing & QSTAT_DISPLAY_FULL)) {
      if (report_handler) {
         report_handler->report_job_string_value(report_handler, cqname, jobid, "queue_name", queue_name, alpp);
      } else {
         char temp[20];
         sprintf(temp,"%%-%d.%ds ", queue_name_length, queue_name_length);
         printf(temp, queue_name?queue_name:"");
      }
   }

   if ((group_opt & GROUP_NO_PETASK_GROUPS)) {
      /* MASTER/SLAVE information needed only to show parallel job distribution */
      if (report_handler) {
         report_handler->report_job_string_value(report_handler, cqname, jobid, "pe_master", master, alpp);
      } else {
         if (master) {
            printf("%-7.6s", master);
         } else {
            printf("       ");
         }
      }   
   } else {
      /* job slots requested/granted */
      if (!slots_per_line) {
         slots_per_line = sge_job_slot_request(job, pe_list);
      }   
      printf("%5d ", slots_per_line);
   }

   if (report_handler) {
      const char*taskid = sge_dstring_get_string(dyn_task_str);
      if (job_is_array(job)) {
         ret = report_handler->report_job_string_value(report_handler, cqname, jobid, "taskid", taskid, alpp);
         if (ret != QHOST_SUCCESS ) {
            DRETURN(ret);
         }
      }
   } else {
      if (sge_dstring_get_string(dyn_task_str) && job_is_array(job)) {
         printf("%s", sge_dstring_get_string(dyn_task_str)); 
      } else {
         printf("       ");
      }
   }
   
   if (tsk_ext) {
      lList *task_list = lGetList(jatep, JAT_task_list);
      lListElem *task, *ep;
      const char *qname;
      int indent=0;
      int subtask_ndx=1;
      int num_spaces = sizeof(jhul1)-1 + (sge_ext?sizeof(jhul2)-1:0) - 
            ((full_listing & QSTAT_DISPLAY_FULL)?11:0);

      /* print master sub-task belonging to this queue */
      if (!slot && task_list && queue_name &&
          ((ep=lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)))) &&
          ((qname=lGetString(ep, JG_qname))) &&
          !strcmp(qname, queue_name)) {
         if (indent++) {
            printf("%*s", num_spaces, " ");
         }   
         sge_print_subtask(job, jatep, NULL, 0, 0);
         /* subtask_ndx++; */
      }
         
      /* print sub-tasks belonging to this queue */
      for_each(task, task_list) {
         if (!slots || (queue_name && 
              ((ep=lFirst(lGetList(task, PET_granted_destin_identifier_list)))) &&
              ((qname=lGetString(ep, JG_qname))) &&
              !strcmp(qname, queue_name) && ((subtask_ndx++%slots)==slot))) {
            if (indent++) {
               printf("%*s", num_spaces, " ");
            }   
            sge_print_subtask(job, jatep, task, 0, 0);
         }
      }

      if (!indent) {
         putchar('\n');
      }

   } else {
      if (!report_handler) {
         /* print a new line */
         putchar('\n');
      } else {
         ret = report_handler->report_job_finished(report_handler, cqname, jobid, alpp);
         if (ret != QHOST_SUCCESS ) {
            DRETURN(ret);
         }
      }   
   }

   /* print additional job info if requested */
   if (print_jobid && (full_listing & QSTAT_DISPLAY_RESOURCES)) {
         printf(QSTAT_INDENT "Full jobname:     %s\n", lGetString(job, JB_job_name));

         if(queue_name) {
            printf(QSTAT_INDENT "Master queue:     %s\n", queue_name);
         }

         if (lGetString(job, JB_pe)) {
            dstring range_string = DSTRING_INIT;

            range_list_print_to_string(lGetList(job, JB_pe_range), 
                                       &range_string, true, false, false);
            printf(QSTAT_INDENT "Requested PE:     %s %s\n", 
                   lGetString(job, JB_pe), sge_dstring_get_string(&range_string)); 
            sge_dstring_free(&range_string);
         }
         if (lGetString(jatep, JAT_granted_pe)) {
            lListElem *gdil_ep;
            u_long32 pe_slots = 0;
            for_each (gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list))
               pe_slots += lGetUlong(gdil_ep, JG_slots);
            printf(QSTAT_INDENT "Granted PE:       %s "sge_u32"\n", 
               lGetString(jatep, JAT_granted_pe), pe_slots); 
         }
         if (lGetString(job, JB_checkpoint_name)) 
            printf(QSTAT_INDENT "Checkpoint Env.:  %s\n", 
               lGetString(job, JB_checkpoint_name)); 

         sge_show_ce_type_list_line_by_line(QSTAT_INDENT "Hard Resources:   ",
               QSTAT_INDENT2, lGetList(job, JB_hard_resource_list), true, centry_list,
               sge_job_slot_request(job, pe_list)); 

         /* display default requests if necessary */
         {
            lList *attributes = NULL;
            lListElem *ce;
            const char *name;
            lListElem *hep;

            queue_complexes2scheduler(&attributes, qep, exechost_list, centry_list);
            for_each (ce, attributes) {
               double dval;

               name = lGetString(ce, CE_name);
               if (!lGetUlong(ce, CE_consumable) || !strcmp(name, "slots") || 
                   job_get_request(job, name))
                  continue;

               parse_ulong_val(&dval, NULL, lGetUlong(ce, CE_valtype), lGetString(ce, CE_default), NULL, 0); 
               if (dval == 0.0)
                  continue;

               /* For pending jobs (no queue/no exec host) we may print default request only
                  if the consumable is specified in the global host. For running we print it
                  if the resource is managed at this node/queue */
               if ((qep && lGetSubStr(qep, CE_name, name, QU_consumable_config_list)) ||
                   (qep && (hep=host_list_locate(exechost_list, lGetHost(qep, QU_qhostname))) &&
                    lGetSubStr(hep, CE_name, name, EH_consumable_config_list)) ||
                     ((hep=host_list_locate(exechost_list, SGE_GLOBAL_NAME)) &&
                         lGetSubStr(hep, CE_name, name, EH_consumable_config_list)))

               printf("%s%s=%s (default)\n", QSTAT_INDENT, name, lGetString(ce, CE_default));      
            }
            lFreeList(&attributes);
         }

         sge_show_ce_type_list_line_by_line(QSTAT_INDENT "Soft Resources:   ",
               QSTAT_INDENT2, lGetList(job, JB_soft_resource_list), false, NULL, 0); 

         ql = lGetList(job, JB_hard_queue_list);
         if (ql) {
            printf(QSTAT_INDENT "Hard requested queues: ");
            for_each(qrep, ql) {
               printf("%s", lGetString(qrep, QR_name));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }

         ql = lGetList(job, JB_soft_queue_list);
         if (ql) {
            printf(QSTAT_INDENT "Soft requested queues: ");
            for_each(qrep, ql) {
               printf("%s", lGetString(qrep, QR_name));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
         ql = lGetList(job, JB_master_hard_queue_list);
         if (ql){
            printf(QSTAT_INDENT "Master task hard requested queues: ");
            for_each(qrep, ql) {
               printf("%s", lGetString(qrep, QR_name));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
         ql = lGetList(job, JB_jid_request_list );
         if (ql) {
            printf(QSTAT_INDENT "Predecessor Jobs (request): ");
            for_each(qrep, ql) {
               printf("%s", lGetString(qrep, JRE_job_name));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
         ql = lGetList(job, JB_jid_predecessor_list);
         if (ql) {
            printf(QSTAT_INDENT "Predecessor Jobs: ");
            for_each(qrep, ql) {
               printf(sge_u32, lGetUlong(qrep, JRE_job_number));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
         ql = lGetList(job, JB_ja_ad_request_list );
         if (ql) {
            printf(QSTAT_INDENT "Predecessor Array Jobs (request): ");
            for_each(qrep, ql) {
               printf("%s", lGetString(qrep, JRE_job_name));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
         ql = lGetList(job, JB_ja_ad_predecessor_list);
         if (ql) {
            printf(QSTAT_INDENT "Predecessor Array Jobs: ");
            for_each(qrep, ql) {
               printf(sge_u32, lGetUlong(qrep, JRE_job_number));
               printf("%s", lNext(qrep)?", ":"\n");
            }
         }
   }

#undef QSTAT_INDENT
#undef QSTAT_INDENT2

   sge_dstring_free(&queue_name_buffer);

   DRETURN(1);
}
