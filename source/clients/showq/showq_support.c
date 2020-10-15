/* ___INFO__MARK_BEGIN__ */
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
 *   Copyright: 2009 by Texas Advanced Computing Center
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/* ___INFO__MARK_END__ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#define __SGE_GDI_LIBRARY_HOME_OBJECT_FILE__

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"

#include "cull/cull.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/str2nm_converter.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_binding.h"

#include "sched/sgeee.h"

#include "showq_support.h"

static void get_core_binding_string(lListElem *job,const int task_number, dstring* corebinding);

struct sort_def{
   int sort_key[20];
   int count;
   char format_string[256];
};

/*
 * This is the tacc display job type, used to display and sort jobs/tasks
 */

enum {
   TACCDJ_priority = 0,
   TACCDJ_jobid,
   TACCDJ_jobname,
   TACCDJ_username,
   TACCDJ_state,
   TACCDJ_core,
   TACCDJ_host,
   TACCDJ_queue,
   TACCDJ_wclimit,
   TACCDJ_queuetime,
   TACCDJ_remaining,
   TACCDJ_starttime,
   TACCDJ_corebinding
};

LISTDEF(TACCDJ_Type)
   SGE_DOUBLE(TACCDJ_priority, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_jobid, CULL_DEFAULT)
   SGE_STRING(TACCDJ_jobname, CULL_DEFAULT)
   SGE_STRING(TACCDJ_username, CULL_DEFAULT)
   SGE_STRING(TACCDJ_state, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_core, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_host, CULL_DEFAULT)
   SGE_STRING(TACCDJ_queue, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_wclimit, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_queuetime, CULL_DEFAULT)
   SGE_LONG(TACCDJ_remaining, CULL_DEFAULT)
   SGE_ULONG(TACCDJ_starttime, CULL_DEFAULT)
   SGE_STRING(TACCDJ_corebinding, CULL_DEFAULT)
LISTEND

NAMEDEF(TACCDJN)
   NAME("priority")
   NAME("jobid")
   NAME("jobname")
   NAME("username")
   NAME("state")
   NAME("core")
   NAME("host")
   NAME("queue")
   NAME("wclimit")
   NAME("queuetime")
   NAME("remaining")
   NAME("starttime")
   NAME("corebinding")
NAMEEND

/* *INDENT-ON* */

#define TACCDJS sizeof(TACCDJN) / sizeof(char*)

lNameSpace taccdj_nmv[] =
{
   { 1, TACCDJS, TACCDJN },
   { 0, 0, NULL }
};

struct sort_def active_default_sort = {
   {TACCDJ_starttime}, 1, {"%I+"}
};

struct sort_def waiting_default_sort = {
   {TACCDJ_priority}, 1, {"%I-"}
};

int sort_dj_list(lList *djobs, lList *field_list, bool sort_waiting)
{
   /*
    * feed djobs into lPsortList, using field_list and direction_list to
    * build lPsortList inputs
    */


   /* procedure for getting a field index from a string: */

   char            format_string[256] = "";        /* the formats for the sort
                                                 * call */
   int             sort_key[20];/* the keys for the sort call */
   int             ind = 0;
   const char     *field_str;
   char            dir_str[256];
   int             ret;
   int             i;
   struct sort_def *default_sort_def = NULL;
   lListElem      *field_elem;

   if (sort_waiting) {
      default_sort_def = &waiting_default_sort;
   } else {
      default_sort_def = &active_default_sort;
   }


   /* pull out sort keys */

   if (lGetNumberOfElem(field_list) != 0) {

      for_each(field_elem, field_list) {
         field_str = lGetString(field_elem, ST_name);
         /* pull out +/- if it is there */
         if ((strncmp(field_str, "+", 1) == 0)) {
            strcpy(dir_str, "%I+ ");
            field_str++;
         } else if ((strncmp(field_str, "_", 1) == 0)) {
            strcpy(dir_str, "%I- ");
            field_str++;
         } else {
            strcpy(dir_str, "%I- ");
         }
         strcat(format_string, dir_str);
         if ((ret = lStr2NmGenerator(field_str, taccdj_nmv)) != NoName) {
            sort_key[ind] = ret;
            ind++;
         } else {
            fprintf(stderr, "Unknown sort key!\n");
            return 1;
         }
      }
      /* may need to trim last space from format string here */
   } else {
      /* defaults - for no specification of sort keys by user */
      strcpy(format_string, default_sort_def->format_string);
      ind = default_sort_def->count;
      for (i = 0; i < ind; i++) {
         sort_key[i] = default_sort_def->sort_key[i];
      }
   }
   /* OK, now sort the job list */
   /* I'll process as many as 5 sort keys */

   switch (ind) {
   case 0:
      return 0;                        /* no sorting */
   case 1:
      return lPSortList(djobs, format_string, sort_key[0]);
   case 2:
      return lPSortList(djobs, format_string, sort_key[0], sort_key[1]);
   case 3:
      return lPSortList(djobs, format_string, sort_key[0], sort_key[1], sort_key[2]);
   case 4:
      return lPSortList(djobs, format_string, sort_key[0], sort_key[1], sort_key[2], sort_key[3]);
   case 5:
      return lPSortList(djobs, format_string, sort_key[0], sort_key[1], sort_key[2], sort_key[3], sort_key[4]);
   default:
      fprintf(stderr, "Sort key count exceeded! \n");
      return 1;
   }

}



int extract_dj_lists(lList *job_list, lList **active_jobs, lList **waiting_jobs,
                     lList **dep_waiting_jobs, lList **unsched_jobs)
{

   /*
    * extract active jobs from job_list (JB_Type) and return a list of type
    * TACCDJ
    */
   lListElem      *job = NULL, *nxt_job = NULL;
   u_long32        job_state;
   lListElem      *tmp_dj_job = NULL;
   const char     *qname = NULL;
   lList          *req_queue_list = NULL;
   u_long32        ultime;
   u_long32        time_limit = 0;
   int             remaining_time;
   u_long32        pe_range_max = 0;
   u_long32        unf_task_range_min = 0;
   u_long32        unf_task_range_max = 0;
   u_long32        unf_task_range_step = 0;
   time_t          killtime;
   time_t          current_time;
   int             jdep;
   u_long32        unf_task_start, unf_task_end, unf_task_step;
   lListElem      *range_spec;
   lListElem      *pe_range = NULL;        /* RN_Type */
   /*
    * lList *active_jobs; lList *waiting_jobs; lList *dep_waiting_jobs; lList
    * *unsched_jobs;
    */

   DENTER(TOP_LAYER, "extract_dj_lists");

   if (job_list == NULL) {
      return 1;
   }
   /*-----------------------------------------------------------------
    * Create tmp list
    *-----------------------------------------------------------------*/
   *active_jobs = lCreateList("active job list", TACCDJ_Type);
   *waiting_jobs = lCreateList("waiting job list", TACCDJ_Type);
   *dep_waiting_jobs = lCreateList("dep waiting job list", TACCDJ_Type);
   *unsched_jobs = lCreateList("unsched job list", TACCDJ_Type);
   if (*active_jobs == NULL) {
      printf("NULL active_jobs\n");
   }

   nxt_job = lFirst(job_list);
   while ((job = nxt_job)) {
      int             unf_tasks = 0;
      lListElem      *tmp_task;        /* JAT_Type */
      /*
       * alright, scan this gnarly bugger of a job for running/waiting DJ
       * data
       */
      int             running_slots = 0;
      int             job_tag = 0;        /* initialize job to not found as
                                         * active/xfering */

      /* printf("processing a job\n"); */
      nxt_job = lNext(nxt_job);
      /* tmp_dj_job = lCreateElem(TACCDJ_Type); */

      /* if any tasks are running, they'll be in JB_ja_tasks */
      if (lGetPosViaElem(job, JB_ja_tasks, SGE_NO_ABORT) >= 0) {
         lListElem      *jatep, *granted_destination;

         for_each(jatep, lGetList(job, JB_ja_tasks)) {

            job_state = lGetUlong(jatep, JAT_status);
            if ((job_state == JRUNNING) | (job_state == JTRANSFERING)) {
               /* printf("this one is running\n"); */

               tmp_dj_job = lCreateElem(TACCDJ_Type);
               if (tmp_dj_job == NULL) {
                  printf("NULL tmp_dj_job\n");
               }
               running_slots = 0;
               /*
                * if this thing is running, dig down in and add up slot
                * counts for all hosts
                */
               /*
                * alternatively, we could add up the pe values for every
                * running task, but since pe processor counts are expressed
                * as ranges, let's cut through the poo by drilling straight
                * down to granted slots
                */
               for_each(granted_destination, lGetList(jatep, JAT_granted_destin_identifier_list)) {
                  running_slots = running_slots + lGetUlong(granted_destination, JG_slots);
               }

               lSetUlong(tmp_dj_job, TACCDJ_jobid, lGetUlong(job, JB_job_number));
               lSetString(tmp_dj_job, TACCDJ_jobname, lGetString(job, JB_job_name));
               lSetString(tmp_dj_job, TACCDJ_username, lGetString(job, JB_owner));
               if (job_state == JRUNNING) {
                  lSetString(tmp_dj_job, TACCDJ_state, "Running");
               } else {
                  lSetString(tmp_dj_job, TACCDJ_state, "Xfering");
               }
               lSetUlong(tmp_dj_job, TACCDJ_core, running_slots);
               lSetUlong(tmp_dj_job, TACCDJ_host, running_slots / 16);

               req_queue_list = lGetList(job, JB_hard_queue_list);
               if (req_queue_list != NULL) {
                  qname = lGetString(lFirst(req_queue_list), QR_name);
                  lSetString(tmp_dj_job, TACCDJ_queue, qname);
               }
               /* times for an active job */

               /* first, get the start time */
               ultime = lGetUlong(jatep, JAT_start_time);
               lSetUlong(tmp_dj_job, TACCDJ_starttime, ultime);

               /* get submission time on job */
               time_limit = lGetUlong(job, JB_submission_time);
               lSetUlong(tmp_dj_job, TACCDJ_queuetime, time_limit);

               /* get time limit on job (possibly zero) */
               time_limit = lGetUlong(jatep, JAT_wallclock_limit);
               lSetUlong(tmp_dj_job, TACCDJ_wclimit, time_limit);

               /*
                * if a time limit is set, compute remaining time - otherwise
                * just report the start time
                */
               remaining_time = 0;
               if (time_limit) {
                  killtime = (time_t) (ultime + time_limit);
                  /* get current time */
                  current_time = time(NULL);
                  remaining_time = (int) killtime - (int) current_time;
               }
               lSetLong(tmp_dj_job, TACCDJ_remaining, remaining_time);


               /* mark this job as having had a job task displayed as active */
               job_tag = 1;

               /* set priority */
               tmp_task = lFirst(lGetList(job, JB_ja_tasks));
               /*
                * If there is no enrolled task than take the template element
                */
               if (tmp_task == NULL) {
                  tmp_task = lFirst(lGetList(job, JB_ja_template));
               }
               lSetDouble(tmp_dj_job, TACCDJ_priority,
                          lGetDouble(tmp_task, JAT_prio));

               /* core binding */           
               {
                  dstring cb = DSTRING_INIT;
                  get_core_binding_string(job, (int)lGetUlong(jatep, JAT_task_number), &cb);

                  if (sge_dstring_get_string(&cb) != NULL) {
                     lSetString(tmp_dj_job, TACCDJ_corebinding, 
                                 sge_dstring_get_string(&cb));
                  } else {
                     lSetString(tmp_dj_job, TACCDJ_corebinding, "-");
                  }

                  sge_dstring_free(&cb);
               }
               /* append this job to the active job list */
               /* printf("appending to active jobs\n"); */
               /*
                * printf("active job data:
                * %s\n",lGetString(tmp_dj_job,TACCDJ_queue));
                */
               lAppendElem(*active_jobs, tmp_dj_job);
            }
         }
      }
      /* process this job for waiting tasks */
      if (lGetPosViaElem(job, JB_ja_n_h_ids, SGE_NO_ABORT) >= 0) {
         for_each(range_spec, lGetList(job, JB_ja_n_h_ids)) {
            unf_task_range_min = lGetUlong(range_spec, RN_min);
            unf_task_range_max = lGetUlong(range_spec, RN_max);
            unf_task_range_step = lGetUlong(range_spec, RN_step);
            /* compute how many things this is */
            unf_tasks = unf_tasks + (unf_task_range_max - unf_task_range_min) / unf_task_range_step + 1;
         }
      }
      if (lGetString(job, JB_pe) != NULL) {
         /*
          * not sure why there would be more than one of these JB_pe things
          * (presumably this is due to the specification of a complex
          * pe_range) - I'm taking the first one
          */
         pe_range = lFirst(lGetList(job, JB_pe_range));
         /* lDumpList(stdout,lGetList(job,JB_pe_range),0); */
         pe_range_max = lGetUlong(pe_range, RN_max);
         /* printf("%-6d", pe_range_max); */
      } else {
         /* no pe - every task is just a task */
         pe_range_max = 1;
         /* printf("%-6d", pe_range_max); */
      }

      if (lGetList(job, JB_jid_predecessor_list) != NULL) {
         jdep = 1;
      } else {
         jdep = 0;
      }

      /*
       * if this job has unfinished tasks, assemble the appropriate waiting
       * job info
       */

      if ((unf_tasks > 0)) {

         /* printf("this job is waiting\n"); */
         tmp_dj_job = lCreateElem(TACCDJ_Type);

         /* grab an appropriate task */
         tmp_task = lFirst(lGetList(job, JB_ja_tasks));
         /*
          * If there is no enrolled task than take the template element
          */
         if (tmp_task == NULL) {
            tmp_task = lFirst(lGetList(job, JB_ja_template));
         }
         lSetUlong(tmp_dj_job, TACCDJ_jobid, lGetUlong(job, JB_job_number));
         lSetString(tmp_dj_job, TACCDJ_jobname, lGetString(job, JB_job_name));
         lSetString(tmp_dj_job, TACCDJ_username, lGetString(job, JB_owner));
         if (jdep == 0) {
            lSetString(tmp_dj_job, TACCDJ_state, "Waiting");
         } else {
            lSetString(tmp_dj_job, TACCDJ_state, "DepWait");
         }

         lSetUlong(tmp_dj_job, TACCDJ_core, pe_range_max * unf_tasks);
         lSetUlong(tmp_dj_job, TACCDJ_host, pe_range_max * unf_tasks / 16);

         req_queue_list = lGetList(job, JB_hard_queue_list);
         if (req_queue_list != NULL) {
            qname = lGetString(lFirst(req_queue_list), QR_name);
            lSetString(tmp_dj_job, TACCDJ_queue, qname);
         }
         /* times for a waiting job */
         /* get time limit on job (possibly zero) */
         /*
          * time_limit = lGetUlong(tmp_task,JAT_wallclock_limit);
          */
         job_get_wallclock_limit(&time_limit, job);
         lSetUlong(tmp_dj_job, TACCDJ_wclimit, time_limit);
         time_limit = lGetUlong(job, JB_submission_time);
         lSetUlong(tmp_dj_job, TACCDJ_queuetime, time_limit);

         lSetDouble(tmp_dj_job, TACCDJ_priority,
                    lGetDouble(tmp_task, JAT_prio));

         /* core binding */
         {
            dstring cb = DSTRING_INIT;
            get_core_binding_string(job, -1, &cb);
            
            lSetString(tmp_dj_job, TACCDJ_corebinding, 
                           sge_dstring_get_string(&cb));
            
            sge_dstring_free(&cb);
         }

         job_tag = 1;                /* make sure this job is marked as accounted
                                 * for */

         if (jdep == 0) {
            lAppendElem(*waiting_jobs, tmp_dj_job);
         } else {
            lAppendElem(*dep_waiting_jobs, tmp_dj_job);
         }

      }
      /* if this job still has job_tag=0, it is unscheduled */
      if (job_tag == 0) {

         if (lGetPosViaElem(job, JB_ja_structure, SGE_NO_ABORT) >= 0) {
            job_get_submit_task_ids(job, &unf_task_start, &unf_task_end, &unf_task_step);
            unf_tasks = (unf_task_end - unf_task_start) / unf_task_step + 1;
         } else {
            /* If that didn't work, let's assume there is 1 unfinished task */
            unf_tasks = 1;
         }

         /*
          * Now grab the job pe to determine the number of slots to multiply
          * unfinished tasks by:
          */
         if (lGetString(job, JB_pe) != NULL) {
            /*
             * not sure why there would be more than one of these JB_pe
             * things (presumably this is due to the specification of a
             * complex pe_range) - I'm taking the first one
             */
            pe_range = lFirst(lGetList(job, JB_pe_range));
            /* lDumpList(stdout,lGetList(job,JB_pe_range),0); */
            pe_range_max = lGetUlong(pe_range, RN_max);
            /* printf("%-6d", pe_range_max); */
         } else {
            /* no pe - every task is just a task */
            pe_range_max = 1;
            /* printf("%-6d", pe_range_max); */
         }

         tmp_dj_job = lCreateElem(TACCDJ_Type);
         tmp_task = lFirst(lGetList(job, JB_ja_tasks));
         /*
          * If there is no enrolled task than take the template element
          */
         if (tmp_task == NULL) {
            tmp_task = lFirst(lGetList(job, JB_ja_template));
         }
         lSetUlong(tmp_dj_job, TACCDJ_jobid, lGetUlong(job, JB_job_number));
         lSetString(tmp_dj_job, TACCDJ_jobname, lGetString(job, JB_job_name));
         lSetString(tmp_dj_job, TACCDJ_username, lGetString(job, JB_owner));
         lSetString(tmp_dj_job, TACCDJ_state, "Unsched");
         lSetUlong(tmp_dj_job, TACCDJ_core, pe_range_max * unf_tasks);
         lSetUlong(tmp_dj_job, TACCDJ_host, pe_range_max * unf_tasks / 16);
         req_queue_list = lGetList(job, JB_hard_queue_list);
         if (req_queue_list != NULL) {
            qname = lGetString(lFirst(req_queue_list), QR_name);
            lSetString(tmp_dj_job, TACCDJ_queue, qname);
         }
         /*
          * time_limit = lGetUlong(tmp_task,JAT_wallclock_limit);
          */
         job_get_wallclock_limit(&time_limit, job);
         lSetUlong(tmp_dj_job, TACCDJ_wclimit, time_limit);
         time_limit = lGetUlong(job, JB_submission_time);
         lSetUlong(tmp_dj_job, TACCDJ_queuetime, time_limit);

         /* set priority */
         lSetDouble(tmp_dj_job, TACCDJ_priority,
                    lGetDouble(tmp_task, JAT_prio));

         /* core binding */
         {
            dstring cb = DSTRING_INIT;
            get_core_binding_string(job, -1, &cb);
            
            lSetString(tmp_dj_job, TACCDJ_corebinding, 
                          sge_dstring_get_string(&cb));
            
            sge_dstring_free(&cb);
         }

         lAppendElem(*unsched_jobs, tmp_dj_job);

      }
      /* lDechainElem(*job_list, job); */
   }
   /* printf("done in extract_job_lists\n"); */
   /* pass all of the pointers back out */
   /*
    * active_jobs_out=active_jobs; waiting_jobs_out=waiting_jobs;
    * dep_waiting_jobs_out=dep_waiting_jobs; unsched_jobs_out=unsched_jobs;
    */
   return 0;
}

int sum_slots(lList *dj_list)
{
   /* sums core field from TACCDJ list input */
   int             sum = 0;
   lListElem      *entry;

   for_each(entry, dj_list) {
      sum += (int)lGetUlong(entry, TACCDJ_core);
   }

   return sum;

}

void show_active_jobs(lList *joblist, int flags, const bool binding)
{
   time_t ultime;
   char truncated_jobname[256];
   char trunc_string[256];

   int             remaining_time = 0;
   u_long32        time_limit = 0;
   /* u_long32 job_tag; */

   /* const char *qname =NULL; */
   /* lListElem *rqueue; */
   /* lList* req_queue_list = NULL; */
   lListElem      *job;

   for_each(job, joblist) {

      if (lGetUlong(job, TACCDJ_jobid)) {
         printf("%-10d", (int) lGetUlong(job, TACCDJ_jobid));
      } else {
         printf("%-10s", "");
      }

      if (lGetString(job, TACCDJ_jobname)) {
         strncpy(truncated_jobname, lGetString(job, TACCDJ_jobname), 10);
         truncated_jobname[10] = '\0';
         printf("%-11s", truncated_jobname);
      } else {
         printf("-11%s", "");
      }

      if (lGetString(job, TACCDJ_username)) {
         printf("%-14s", lGetString(job, TACCDJ_username));
      } else {
         printf("%-14s", "");
      }

      printf("%-8s", lGetString(job, TACCDJ_state));

      /* processor count for all running tasks */
      printf("%-6"sge_U32CLetter, sge_u32c(lGetUlong(job, TACCDJ_core)));

      if (flags) {
         /* -l mode: print hosts and queue */
         const char *queue = lGetString(job, TACCDJ_queue);
         printf("%-6"sge_U32CLetter, sge_u32c(lGetUlong(job, TACCDJ_host)));
         if (queue != NULL) {
            strncpy(trunc_string, queue, 13);
            trunc_string[13] = '\0';
            printf("%-13s", trunc_string);
         } else {
            printf("%-13s", "");
         }
      }
      /* first, get the start time */
      ultime = (time_t)lGetUlong(job, TACCDJ_starttime);

      /* get time limit on job (possibly zero) */
      time_limit = lGetUlong(job, TACCDJ_wclimit);

      /*
       * if a time limit is set, compute remaining time - otherwise just
       * report the start time
       */

      if (time_limit) {
         dstring buffer = DSTRING_INIT;

         remaining_time = lGetLong(job, TACCDJ_remaining);

         /* this is crap - something is always + */
         if (remaining_time < 0) {
            time_limit = -remaining_time;
            double_print_time_to_dstring(time_limit, &buffer);
            printf("-%-10s", sge_dstring_get_string(&buffer));
         } else {
            double_print_time_to_dstring(remaining_time, &buffer);

            if (sge_dstring_strlen(&buffer) <= 9) {
               printf(" %-10s", sge_dstring_get_string(&buffer));
            } else {
               printf(" %-10s", "");
            }
         }
         sge_dstring_free(&buffer);
      } else {
         printf(" %-10s", "");
      }

      if ((ultime = (time_t)lGetUlong(job, TACCDJ_starttime))) {
         const char *time_string = ctime((time_t *)&ultime);
         if (time_string != NULL) {
            char *truncated_time = strdup(time_string);
            truncated_time[20] = '\0';
            printf("%20s", truncated_time);
            sge_free(&truncated_time);
         } else {
            printf("%-20s", "");
         }
      } else {
         printf("%20s", "");
      }
   
      /* core binding */ 
      if (binding) {
         if ((lGetString(job, TACCDJ_corebinding) != NULL)) {
            printf("%s", lGetString(job, TACCDJ_corebinding));
         }
      }   
      
      printf("\n");
   }
}

void show_waiting_jobs(lList *joblist, int flags)
{
   time_t ultime;
   char truncated_jobname[256];
   u_long32 time_limit = 0;
   char trunc_string[256];

   lListElem *job;

   for_each(job, joblist) {

      /*
       * display waiting tasks of this job in a summary fashion: If any tasks
       * are explicitly waiting around, they'll be (specified) in
       * JB_ja_n_h_ids (where else!). I'm assuming that multiple elements of
       * JB_ja_n_h_ids correspond to failed tasks or such that sge has set
       * these jobs to rerun ?? In any case, multiple elements with multiple
       * ranges may exist
       */

      if (lGetUlong(job, TACCDJ_jobid)) {
         printf("%-10d", (int) lGetUlong(job, TACCDJ_jobid));
      } else {
         printf("%-10s", "");
      }

      if (lGetString(job, TACCDJ_jobname) != NULL) {
         strncpy(truncated_jobname, lGetString(job, TACCDJ_jobname), 10);
         truncated_jobname[10] = '\0';
         printf("%-11s", truncated_jobname);
      } else {
         printf("-11%s", "");
      }

      if (lGetString(job, TACCDJ_username)) {
         printf("%-14s", lGetString(job, TACCDJ_username));
      } else {
         printf("%-14s", "");
      }

      printf("%-8s", lGetString(job, TACCDJ_state));

      /* processor count for all running tasks */
      printf("%-6"sge_U32CLetter, sge_u32c(lGetUlong(job, TACCDJ_core)));

      if (flags) {
         /* -l mode: print hosts and queue */
         const char *queue = lGetString(job, TACCDJ_queue);
         printf("%-6"sge_U32CLetter, sge_u32c(lGetUlong(job, TACCDJ_host)));
         if (queue != NULL) {
            strncpy(trunc_string, queue, 13);
            trunc_string[13] = '\0';
            printf("%-13s", trunc_string);
         } else {
            printf("%-13s", "");
         }
      }
      /* WCLIMIT and QUEUTIME for a waiting job */

      /* get time limit on job (possibly zero) */
      time_limit = lGetUlong(job, TACCDJ_wclimit);
      if (time_limit == U_LONG32_MAX) {
         printf(" %-10s", "infinity");
      } else if (time_limit > 0) {
         dstring buffer = DSTRING_INIT;
         /* convert to HHMMSS */
         double_print_time_to_dstring(time_limit, &buffer);
         printf(" %-10s", sge_dstring_get_string(&buffer));
         /* printf("%-11d",w_limit); */
         sge_dstring_free(&buffer);
      } else {
         printf(" %-10s", "");
      }

      if ((ultime = (time_t)lGetUlong(job, TACCDJ_queuetime))) {
         const char *time_string = ctime((time_t *)&ultime);
         if (time_string != NULL) {
            char *truncated_time = strdup(time_string);
            truncated_time[20] = '\0';
            printf("%20s\n", truncated_time);
            sge_free(&truncated_time);
         } else {
            printf("%-20s\n", "");
         }
      } else {
         printf("%-20s\n", "");
      }
   }
}

static void get_core_binding_string(lListElem *job, const int task_number, dstring* corebinding)
{
   lListElem *jatep;
   bool binding_of_first_task_found = false;

   if (corebinding == NULL || job == NULL) {
      return;
   } 

   for_each (jatep, lGetList(job, JB_ja_tasks)) {
      /* int first_task = 1; */
      lListElem *usage_elem;
      const char *binding_inuse = NULL; 
      const char *binding_topo = NULL;

      if (task_number > 0) {
         if ((int)lGetUlong(jatep, JAT_task_number) != task_number) {
            continue;
         }
      }

      if (lGetUlong(jatep, JAT_status) != JRUNNING && lGetUlong(jatep, JAT_status) != JTRANSFERING) {
         continue;
      }

      for_each (usage_elem, lGetList(jatep, JAT_scaled_usage_list)) {
         const char *binding_name = "binding_inuse";
         const char *usage_name = lGetString(usage_elem, UA_name);

         if (strncmp(usage_name, binding_name, strlen(binding_name)) == 0) {
            binding_inuse = strstr(usage_name, "!"); 
            if (binding_inuse != NULL) {
               binding_inuse++;
            }
            break;
         }
      }

      if (binding_inuse != NULL && strcmp(binding_inuse, "NULL") != 0) {  
         binding_topo = binding_get_topology_for_job(binding_inuse);

         if (binding_topo != NULL) {
            sge_dstring_append(corebinding, binding_topo);
            binding_of_first_task_found = true;
            break;
         }
      }
      
   }
   
   if (binding_of_first_task_found == false) {
      sge_dstring_append(corebinding, "-");
   }

}

