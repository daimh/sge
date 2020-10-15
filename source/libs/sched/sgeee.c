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

/* For fair share, Shannon Davidson referenced J. Kay and P. Lauder,
   `A Fair Share Scheduler', Communications of the ACM, 31(1):44-55
   (1988) <http://www.cs.su.oz.au/~piers/papers/Share/share.html>.
   Maybe the subsequent
   <https://www.it.usyd.edu.au/~piers/papers/Share/share2.html> is
   relevant.  */

#define SGE_INCLUDE_QUEUED_JOBS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <float.h>
#include <math.h>

#include "uti/sge_rmon.h"
#include "uti/sge_profiling.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_prog.h"
#include "uti/sge_language.h"
#include "uti/sge_string.h"

#include "comm/commlib.h"

#include "sgeobj/sge_order.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_pe_task.h"

#include "sge_eejob_FCAT_L.h"
#include "sge_eejob_SGEJ_L.h"
#include "sge.h"
#include "sge_job_schedd.h"
#include "sge_urgency.h"
#include "sgeee.h"
#include "sge_support.h"
#include "sort_hosts.h"
#include "msg_schedd.h"

/* 
 * Following fields are necessary for tasks which are not
 * enrolled in the JB_ja_tasks-list. For these jobs we have no
 * reference to the 'ja_task'-CULL-element. 
 */ 
typedef struct {
   u_long32 job_number;       /* job number */
   u_long32 ja_task_number;   /* ja task id */
   double ja_task_fticket;    /* ftickets for task 'ja_task_id' */ 
   double ja_task_sticket;    /* stickets for task 'ja_task_id' */ 
   double ja_task_oticket;    /* otickets for task 'ja_task_id' */ 
   double ja_task_ticket;     /* tickets for task 'ja_task_id' */ 
   double ja_task_share;      /* share for task 'ja_task_id' */
   u_long32 ja_task_fshare;   /* fshare for task 'ja_task_id' */
} sge_task_ref_t;

/*
 * sge_ref_t - this structure is used in the
 * scheduler to keep references to all the objects
 * associated with a job.  An array is built containing
 * one of these structures for each job/array we
 * are scheduling.
 */

typedef struct {
   lListElem *job;		      /* job reference */
   lListElem *ja_task;        /* task reference */
   lListElem *user;		      /* user reference */
   lListElem *project;		   /* project reference */
   lListElem *dept;		      /* department reference */
   lListElem *node;		      /* node reference */
   int queued;                /* =1 if job is a queued job */
   u_long32  share_tree_type; /* share tree type */
   double user_fshare;        /* job's share of user functional shares */
   double dept_fshare;        /* job's share of department functional shares */
   double project_fshare;     /* job's share of project functional shares */
   double job_fshare;         /* job's share of job functional shares */
   double tickets;            /* job's pending tickets from hierarchical policies */
   sge_task_ref_t *tref;
} sge_ref_t;

/*
 * this allows to build a list from
 * the the ref structure. This is need
 * to build the functional categories
 */
typedef struct sge_ref_list_t{
   sge_ref_t *ref;            /* ref reference */
   struct sge_ref_list_t *next;        /* next list item */
   struct sge_ref_list_t *prev;        /* previous list itme */
} sge_ref_list_t;

static void tix_range_set(double min_tix, double max_tix);
static void tix_range_get(double *min_tix, double *max_tix);

/* EB: debug */
#if 0
#define DEBUG_TASK_REF 
#endif

#ifdef DEBUG_TASK_REF 
static void task_ref_print_table(void);
static void task_ref_print_table_entry(sge_task_ref_t *tref);
#endif

static void sge_clear_ja_task ( lListElem *ja_task );

static sge_task_ref_t *task_ref_get_first(u_long32 job_number, u_long32 ja_task_number);
static sge_task_ref_t *task_ref_get_first_job_entry(void);
static sge_task_ref_t *task_ref_get_next_job_entry(void);
static void task_ref_copy_to_ja_task(sge_task_ref_t *tref, lListElem *ja_task);

static void sge_do_sgeee_priority(lList *job_list, double min_tix, double max_tix,
            bool do_nprio, bool do_nurg);
static void sgeee_priority(lListElem *task, double nsu, double npri,
                           double min_tix, double max_tix);
static void recompute_prio(sge_task_ref_t *tref, lListElem *task, double uc, double npri);


static void calculate_pending_shared_override_tickets(sge_ref_t *job_ref, int num_jobs, int dependent ); 

static double calc_job_tickets(sge_ref_t *ref);

static int sge_calc_tickets (scheduler_all_data_t *lists,
                      lList *running_jobs,
                      lList *finished_jobs,
	          	       lList *queued_jobs,
                      int do_usage, 
                      double *max_tickets);

static lListElem *get_mod_share_tree(lListElem *node, lEnumeration *what, int seqno);
static lList *sge_sort_job_nodes(lListElem *root, lListElem *node,
                      double total_share_tree_tickets, sge_ref_t *job_ref);
static int sge_calc_node_targets(lListElem *root, lListElem *node, scheduler_all_data_t *lists);
static int sge_calc_sharetree_targets(lListElem *root, scheduler_all_data_t *lists,
                           lList *decay_list, u_long curr_time,
                           u_long seqno);
static int sge_init_share_tree_node_fields( lListElem *node, void *ptr );
static int sge_init_share_tree_nodes( lListElem *root );

static double calc_pjob_override_tickets_shared( sge_ref_t *ref); 

static void free_fcategories(lList **fcategories, sge_ref_list_t ** ref_array);
static u_long32 build_functional_categories(sge_ref_t *job_ref, int num_jobs, lList **root, 
                                        sge_ref_list_t ** ref_array, int dependent, 
                                        u_long32 job_tickets, u_long32 user_tickets, u_long32 prj_tickets, u_long32 dp_tickets); 
static void copy_ftickets(sge_ref_list_t *source, sge_ref_list_t *dest);
static void destribute_ftickets(lList *root, int dependent);

static void calc_intern_pending_job_functional_tickets(
                                    lListElem *current,
                                    double sum_of_user_functional_shares,
                                    double sum_of_project_functional_shares,
                                    double sum_of_department_functional_shares,
                                    double sum_of_job_functional_shares,
                                    double total_functional_tickets,
                                    double weight[],
                                    bool share_functional_shares);


static void task_ref_initialize_table(u_long32 number_of_tasks);
static void task_ref_destroy_table(void);
static sge_task_ref_t *task_ref_get_entry(u_long32 index);

static scheduler_all_data_t *all_lists; /* thread local */
static u_long32 sge_scheduling_run = 0; /* thread local */

static sge_task_ref_t *task_ref_table = NULL; /* thread local */
static u_long32 task_ref_entries = 0;   /* thread local */
static u_long32 task_ref_job_pos = 0;   /* thread local */
static double Master_min_tix = 0.0;     /* thread local */
static double Master_max_tix = 0.0;     /* thread local */
static int halflife = 0;                /* stores the last used halflife time to detect changes  thread_local*/
static int last_seqno = 0;              /* stores the last used seqno for the orders  thread_local*/
static u_long32 past = 0;               /* stores the last re-order send time thread local */
static lEnumeration *user_usage_what = NULL; /* thread local */
static lEnumeration *prj_usage_what = NULL; /* thread local */
static lEnumeration *share_tree_what = NULL; /* thread local */


/****** sgeee/tix_range_set() **************************************************
*  NAME
*     tix_range_set() -- Store ticket range.
*
*  SYNOPSIS
*     static void tix_range_set(double min_tix, double max_tix) 
*
*  FUNCTION
*     Stores ticket range in the global variables.
*
*  INPUTS
*     double min_tix - Minimum ticket value.
*     double max_tix - Maximum ticket value.
*
*  NOTES
*     MT-NOTES: tix_range_set() is not MT safe
*******************************************************************************/
static void tix_range_set(double min_tix, double max_tix)
{
   Master_min_tix = min_tix;
   Master_max_tix = max_tix;
}

/****** sgeee/tix_range_get() **************************************************
*  NAME
*     tix_range_get() -- Get stored ticket range.
*
*  SYNOPSIS
*     static void tix_range_get(double *min_tix, double *max_tix) 
*
*  FUNCTION
*     Get stored ticket range from global variables.
*
*  INPUTS
*     double *min_tix - Target for minimum value.
*     double *max_tix - Target for maximum value.
*
*  NOTES
*     MT-NOTES: tix_range_get() is not MT safe
*******************************************************************************/
static void tix_range_get(double *min_tix, double *max_tix)
{
   if (min_tix) 
      *min_tix = Master_min_tix;
   if (max_tix) 
      *max_tix = Master_max_tix;
}

static void task_ref_initialize_table(u_long32 number_of_tasks) 
{
   const size_t size = number_of_tasks * sizeof(sge_task_ref_t);

   if (task_ref_entries != number_of_tasks && number_of_tasks > 0) {
      task_ref_destroy_table();
      task_ref_table = (sge_task_ref_t *) malloc(size);
      task_ref_entries = number_of_tasks;
   }
   if (task_ref_table != NULL) {
      memset(task_ref_table, 0, size);
   }
}

static void task_ref_destroy_table(void) 
{
   if (task_ref_table != NULL) {
      sge_free(&task_ref_table);
      task_ref_entries = 0;
   }
}

static sge_task_ref_t *task_ref_get_entry(u_long32 index) 
{
   sge_task_ref_t *ret = NULL;

   DENTER(BASIS_LAYER, "task_ref_get_entry");
   if (index < task_ref_entries) {
      ret = &task_ref_table[index];
   }
   DEXIT;
   return ret;
}

#ifdef DEBUG_TASK_REF
static void task_ref_print_table(void)
{
   u_long32 i;

   DENTER(BASIS_LAYER, "task_ref_print_table");
   for (i = 0; i < task_ref_entries; i++) {
      sge_task_ref_t *tref = task_ref_get_entry(i);

      if (tref == NULL) {
         break;
      }

      task_ref_print_table_entry(tref);
   }
   DEXIT;
}

static void task_ref_print_table_entry(sge_task_ref_t *tref) 
{
   DENTER(TOP_LAYER, "task_ref_print_table_entry");

   if (tref != NULL) {
      DPRINTF(("    @@@ "
         "job: "sge_u32" "
         "ja_task: "sge_u32" "
         "t: %f "
         "st: %f "
         "s: %f "
         "\n",
         tref->job_number,
         tref->ja_task_number,
         tref->ja_task_ticket,
         tref->ja_task_sticket,
         tref->ja_task_share
      ));       
   }
   DEXIT;
}
#endif

static sge_task_ref_t *task_ref_get_first(u_long32 job_number, 
                                   u_long32 ja_task_number)
{
   sge_task_ref_t *ret = NULL;
   u_long32 i;

   DENTER(TOP_LAYER, "task_ref_get_first");

   for (i = 0; i < task_ref_entries; i++) {
      sge_task_ref_t *tref = task_ref_get_entry(i);

      if (tref != NULL && tref->job_number == job_number &&  
          tref->ja_task_number == ja_task_number) {
         ret = tref;
         break;
      }
   }

   DEXIT;
   return ret;
}

static sge_task_ref_t *task_ref_get_first_job_entry(void)
{
   sge_task_ref_t *ret = NULL;

   DENTER(BASIS_LAYER, "task_ref_get_first_job_entry");
   task_ref_job_pos = 0;
   if (task_ref_job_pos < task_ref_entries) {
      ret = task_ref_get_entry(task_ref_job_pos);
   }
   DEXIT;
   return ret;
}

static sge_task_ref_t *task_ref_get_next_job_entry(void)
{
   sge_task_ref_t *current_entry = task_ref_get_entry(task_ref_job_pos);
   sge_task_ref_t *ret = NULL;
   u_long32 pos = task_ref_job_pos;

   DENTER(BASIS_LAYER, "task_ref_get_next_job_entry"); 
   if (current_entry != NULL) {
      u_long32 current_job_number = current_entry->job_number;
   
      while (++pos < task_ref_entries) {
         sge_task_ref_t *next_entry = task_ref_get_entry(pos);
         
         if (next_entry != NULL && 
             current_job_number != next_entry->job_number) {
            task_ref_job_pos = pos;
            ret = next_entry;
            break; 
         } 
      } 
   }
   DEXIT;
   return ret;        
}

static void task_ref_copy_to_ja_task(sge_task_ref_t *tref, lListElem *ja_task) 
{

   DENTER(BASIS_LAYER, "task_ref_copy_to_ja_task");

   if (ja_task != NULL && tref != NULL) {
      lSetUlong(ja_task, JAT_task_number, tref->ja_task_number);

      lSetDouble(ja_task, JAT_tix,                       tref->ja_task_ticket); 
      lSetDouble(ja_task, JAT_fticket,                   tref->ja_task_fticket); 
      lSetDouble(ja_task, JAT_sticket,                   tref->ja_task_sticket); 
      lSetDouble(ja_task, JAT_oticket,                   tref->ja_task_oticket); 
      lSetDouble(ja_task, JAT_share,                     tref->ja_task_share); 
      lSetUlong(ja_task, JAT_fshare,                     tref->ja_task_fshare); 

   }
   DEXIT;
}


#define SGE_MIN_USAGE 1.0

#define __REF_SET_TYPE(ref, cull_attr, ref_attr, value, function) \
{ \
   if ((ref)->ja_task) { \
      (function)((ref)->ja_task, cull_attr, (value)); \
   } else { \
      (ref_attr) = (value); \
   } \
}
 
#define __REF_GET_TYPE(ref, cull_attr, ref_attr, function) \
   ((ref)->ja_task ? (function)((ref)->ja_task, cull_attr) : (ref_attr))
 
#define __REF_SET_ULONG(ref, cull_attr, ref_attr, value) \
   __REF_SET_TYPE(ref, cull_attr, ref_attr, value, lSetUlong)
 
#define __REF_SET_DOUBLE(ref, cull_attr, ref_attr, value) \
   __REF_SET_TYPE(ref, cull_attr, ref_attr, value, lSetDouble)
 
#define __REF_GET_ULONG(ref, cull_attr, ref_attr) \
   __REF_GET_TYPE(ref, cull_attr, ref_attr, lGetUlong)
 
#define __REF_GET_DOUBLE(ref, cull_attr, ref_attr) \
   __REF_GET_TYPE(ref, cull_attr, ref_attr, lGetDouble)
 
/* u_long32 REF_GET_JA_TASK_NUMBER(sge_ref_t *ref) */
 
#define REF_GET_JA_TASK_NUMBER(ref) \
   __REF_GET_ULONG((ref), JAT_task_number, (ref)->tref->ja_task_number)
 
/* void REF_SET_JA_TASK_NUMBER(sge_ref_t *ref, u_long32 ja_task_number) */
#define REF_SET_JA_TASK_NUMBER(ref, ja_task_id) \
   __REF_SET_ULONG((ref), JAT_task_number, (ref)->tref->ja_task_number, \
                   (ja_task_id))
 
#define REF_GET_FTICKET(ref) \
   __REF_GET_DOUBLE((ref), JAT_fticket, (ref)->tref->ja_task_fticket)
 
#define REF_GET_STICKET(ref) \
   __REF_GET_DOUBLE((ref), JAT_sticket, (ref)->tref->ja_task_sticket)
 
#define REF_GET_OTICKET(ref) \
   __REF_GET_DOUBLE((ref), JAT_oticket, (ref)->tref->ja_task_oticket)

#define REF_GET_TICKET(ref) \
   __REF_GET_DOUBLE((ref), JAT_tix, (ref)->tref->ja_task_ticket)
 
#define REF_GET_SHARE(ref) \
   __REF_GET_DOUBLE((ref), JAT_share, (ref)->tref->ja_task_share)
 
#define REF_GET_FSHARE(ref) \
   __REF_GET_ULONG((ref), JAT_fshare, (ref)->tref->ja_task_fshare)

 
#define REF_SET_FTICKET(ref, ticket) \
   __REF_SET_DOUBLE((ref), JAT_fticket, (ref)->tref->ja_task_fticket, (ticket))
 
#define REF_SET_STICKET(ref, ticket) \
   __REF_SET_DOUBLE((ref), JAT_sticket, (ref)->tref->ja_task_sticket, (ticket))
 
#define REF_SET_OTICKET(ref, ticket) \
   __REF_SET_DOUBLE((ref), JAT_oticket, (ref)->tref->ja_task_oticket, (ticket))

#define REF_SET_TICKET(ref, ticket) \
   __REF_SET_DOUBLE((ref), JAT_tix, (ref)->tref->ja_task_ticket, (ticket))
 
#define REF_SET_SHARE(ref, share) \
   __REF_SET_DOUBLE((ref), JAT_share, (ref)->tref->ja_task_share, (share))

#define REF_SET_FSHARE(ref, fshare) \
   __REF_SET_ULONG((ref), JAT_fshare, (ref)->tref->ja_task_fshare, (fshare))



/****** sgeee/sgeee_resort_pending_jobs() **************************************
*  NAME
*     sgeee_resort_pending_jobs() -- Resort pending jobs after assignment
*
*  SYNOPSIS
*     void sgeee_resort_pending_jobs(lList **job_list, lList *orderlist) 
*
*  FUNCTION
*     Update pending jobs order upon assignement and change ticket amounts
*     in orders previously created. 
*     If we dispatch a job sub-task and the job has more sub-tasks, then 
*     the job is still first in the job list.
*     We need to remove and reinsert the job back into the sorted job
*     list in case another job is higher priority (i.e. has more tickets)
*     Additionally it is neccessary to update the number of pending tickets
*     for the following pending array task. (The next task will get less
*     tickets than the current one)
*
*  INPUTS
*     lList **job_list - The pending job list. The first job in the list was 
*                        assigned right before.
*
*  NOTES
*
*******************************************************************************/
void sgeee_resort_pending_jobs(lList **job_list)
{
   lListElem *next_job = lFirst(*job_list);

   DENTER(TOP_LAYER, "sgeee_resort_pending_jobs");

   if (next_job) {
      u_long32 job_id = lGetUlong(next_job, JB_job_number);
      u_long32 start_time_of_job1 = lGetUlong(next_job, JB_submission_time);
      lListElem *tmp_task = lFirst(lGetList(next_job, JB_ja_tasks));
      lListElem *jep = NULL;
      lListElem *insert_jep = NULL;
      double prio = 0.0;

      if (tmp_task == NULL) {
         double nurg = 0.5;
         double npri = 0.5;

         lList *range_list = lGetList(next_job, JB_ja_n_h_ids);
         u_long32 ja_task_id = range_list_get_first_id(range_list, NULL);
         sge_task_ref_t *tref = task_ref_get_first(job_id, ja_task_id);
         lListElem *ja_task_template = lFirst(
                                       lGetList(next_job, JB_ja_template));

         /* 
          * If no further sge_task_ref_t element was prepared during 
          * ticket computation phase we can safe the effort of both
          * resorting the job list and changing ticket amount kept in 
          * the order list. This is because the best approximation we
          * could use instead is the ticket number of the previous
          * task that was assigned from the very same array task.
          * But as a matter of course this would not change the position
          * within the job list or the ticket amount kept in the order 
          * list. The only way for actually improving the behaviour is to 
          * increase the 'max_pending_tasks_per_job' in sched_conf(5).
          */
         if (tref) {
            bool report_priority = sconf_get_report_pjob_tickets();
            bool do_nurg = (sconf_get_weight_urgency()  != 0 || report_priority) ? true : false;
            bool do_npri = (sconf_get_weight_priority() != 0 || report_priority) ? true : false;

            tmp_task = ja_task_template;

            /*
             * job's normalized urgency and priority did not change,
             * but it's needed for updating prio
             */

            if (do_nurg) {
               nurg = lGetDouble(next_job, JB_nurg);
            }   
            if (do_npri) {
               npri = lGetDouble(next_job, JB_nppri);
            }   

            /*
             * Update pending tickets in template element
             */
            DPRINTF(("task_ref_copy_to_ja_task(tref = "sge_u32", template_task = "sge_u32")\n",
               tref->ja_task_number, lGetUlong(ja_task_template, JAT_task_number)));
            task_ref_copy_to_ja_task(tref, ja_task_template);
            recompute_prio(tref, ja_task_template, nurg, npri);
         }
      }

      /*
       * Re-Insert job at the correct possition
       */
      if (tmp_task) { 
         lDechainElem(*job_list, next_job);
         prio = lGetDouble(tmp_task, JAT_prio);
         for_each(jep, *job_list) {
            u_long32 start_time_of_job2 = lGetUlong(jep, JB_submission_time);
            u_long32 job_id2 = lGetUlong(jep, JB_job_number);
            lListElem *tmp_task2 = lFirst(lGetList(jep, JB_ja_tasks));
            double prio2;

            if (tmp_task2 == NULL) {
               tmp_task2 = lFirst(lGetList(jep, JB_ja_template));
            }
            prio2 = lGetDouble(tmp_task2, JAT_prio);
	    /*
	     * In case of equal priority we select the most senior job
	     * in the system. This guy is marked either by smaller job
	     * ID (the regular case) or earlier submission time (the
	     * wraparound case). Note: we force a one second separation
	     * in time for the latter case to ensure that this assumption
	     * holds.
	     */
            if (prio > prio2 ||
                  (prio == prio2 &&
		  (start_time_of_job1 < start_time_of_job2 || /* JobID rollover */
		  (start_time_of_job1 == start_time_of_job2 && job_id < job_id2)))) { /*Regular case */
               break;
            }
            insert_jep = jep;
         }

         lInsertElem(*job_list, insert_jep, next_job);
      }
   }
   DEXIT;
}


/****** sgeee/recompute_prio() *************************************************
*  NAME
*     recompute_prio() -- Recompute JAT prio based on changed ticket amount
*
*  SYNOPSIS
*     static void recompute_prio(sge_task_ref_t *tref, lListElem *task, double 
*     nurg) 
*
*  FUNCTION
*     Each time when the ticket amount for in a JAT_Type element is changed 
*     the JAT_prio needs to be updated. The new ticket value is normalized
*     and the priorty value is computed.
*
*  INPUTS
*     sge_task_ref_t *tref - The tref element that is related to the ticket change
*     lListElem *task      - The JAT_Type task element.
*     double nurg          - The normalized urgency assumed for the job.
*     double npri          - The normalized POSIX priority assumed for the job.
*
*  NOTES
*******************************************************************************/
static void recompute_prio(sge_task_ref_t *tref, lListElem *task, double nurg, double npri)
{
   double min_tix, max_tix, prio;
   double ntix = 0.5; 
   double weight_ticket = 0.0; 
   double weight_urgency = 0.0;
   double weight_priority = 0.0;

   DENTER(TOP_LAYER, "recompute_prio");

   sconf_get_weight_ticket_urgency_priority(&weight_ticket, &weight_urgency, &weight_priority);

   /* need to know min/max tix values to normalize new ticket value */
   tix_range_get(&min_tix, &max_tix);
   ntix = sge_normalize_value(tref->ja_task_ticket, min_tix, max_tix);
   lSetDouble(task, JAT_ntix, ntix); 

   /* prio changes, but only due to ntix update */
   prio = weight_urgency * nurg + weight_priority * npri + weight_ticket * ntix;
   lSetDouble(task, JAT_prio, prio); 

   DPRINTF(("%f tickets for task "sge_u32": ntix = %f (min/max %f/%f), "
         "prio = %f\n",
         tref->ja_task_ticket, tref->ja_task_number,
         ntix, min_tix, max_tix, 
         prio));

   DEXIT;
   return;
}

/*--------------------------------------------------------------------
 * job_is_active - returns true if job is active
 *--------------------------------------------------------------------*/

static int
job_is_active( lListElem *ja_task )
{
   u_long job_status = lGetUlong(ja_task, JAT_status);
#ifdef SGE_INCLUDE_QUEUED_JOBS
   return (job_status == JIDLE ||
       job_status & (JRUNNING | JMIGRATING | JQUEUED | JTRANSFERING)) &&
       !(lGetUlong(ja_task, JAT_state) 
         & (JSUSPENDED|JSUSPENDED_ON_THRESHOLD));
#else
   return (job_status & (JRUNNING | JMIGRATING | JTRANSFERING)) &&
       !(lGetUlong(ja_task, JAT_state) & 
         (JSUSPENDED|JSUSPENDED_ON_THRESHOLD));
#endif
}

/*--------------------------------------------------------------------
 * locate_department - locate the department object by name
 *--------------------------------------------------------------------*/

static lListElem *
locate_department( lList *dept_list,
                   const char *name )
{
   if (!name)
      return NULL;

   /*-------------------------------------------------------------
    * Look up the department object by name
    *-------------------------------------------------------------*/

   return lGetElemStr(dept_list, US_name, name);
}



/*--------------------------------------------------------------------
 * sge_set_job_refs - set object references in the job entry
 *--------------------------------------------------------------------*/

static void
sge_set_job_refs( lListElem *job,
                  lListElem *ja_task,
                  sge_ref_t *ref,
                  sge_task_ref_t *tref,
                  scheduler_all_data_t *lists,
                  int queued)
{
   lListElem *root;
   int is_enrolled_ja_task = (ja_task != NULL && tref == NULL) ? 1 : 0;

   memset(ref, 0, sizeof(sge_ref_t));

   if (tref != NULL) {
      memset(tref, 0, sizeof(sge_task_ref_t));

      tref->job_number = lGetUlong(job, JB_job_number);
      tref->ja_task_number = lGetUlong(ja_task, JAT_task_number);
      ref->tref = tref;
   }

   /*-------------------------------------------------------------
    * save job reference
    *-------------------------------------------------------------*/

   ref->job = job;

   /*-------------------------------------------------------------
    * save task reference 
    *-------------------------------------------------------------*/

   ref->ja_task = is_enrolled_ja_task ? ja_task : NULL;

   /*-------------------------------------------------------------
    * save job state
    *-------------------------------------------------------------*/

   ref->queued = queued;

   /*-------------------------------------------------------------
    * locate user object and save off reference
    *-------------------------------------------------------------*/

   ref->user = user_list_locate(lists->user_list,
                                   lGetString(job, JB_owner));
   if (ref->user) {
      lSetUlong(ref->user, UU_job_cnt, 0);
      lSetUlong(ref->user, UU_pending_job_cnt, 0);
   }

   /*-------------------------------------------------------------
    * locate project object and save off reference
    *-------------------------------------------------------------*/

   ref->project = prj_list_locate(lists->project_list,
                                      lGetString(job, JB_project));
   if (ref->project) {
      lSetUlong(ref->project, PR_job_cnt, 0);
      lSetUlong(ref->project, PR_pending_job_cnt, 0);
   }

   /*-------------------------------------------------------------
    * locate share tree node and save off reference
    *-------------------------------------------------------------*/

   if ((root = lFirst(lists->share_tree))) {
      lListElem *pnode = NULL;
       
      ref->share_tree_type = lGetUlong(root, STN_type);

      if (ref->user || ref->project) {
         ref->node = search_userprj_node(root,
               ref->user ? lGetString(ref->user, UU_name) : NULL,
               ref->project ? lGetString(ref->project, PR_name) : NULL,
               &pnode);

         /*
          * if the found node is a "default" node, then create a
          * temporary sibling node using the "default" node as a
          * template
          */

         if (ref->user && ref->node && pnode &&
             strcmp(lGetString(ref->node, STN_name), "default") == 0) {
            ref->node = lCopyElem(ref->node);
            lSetString(ref->node, STN_name, lGetString(ref->user, UU_name));
            lSetUlong(ref->node, STN_temp, 1);
            lAppendElem(lGetList(pnode, STN_children), ref->node);
         }
      } else
         ref->node = NULL;
   }

   /*-------------------------------------------------------------
    * locate department object and save off reference in job entry
    *-------------------------------------------------------------*/

   ref->dept = locate_department(lists->dept_list,
                                 lGetString(job, JB_department));
   if (ref->dept) {
      lSetUlong(ref->dept, US_job_cnt, 0);
      lSetUlong(ref->dept, US_pending_job_cnt, 0);
   }
}


/*--------------------------------------------------------------------
 * sge_set_job_cnts - set job counts in the various object entries
 *--------------------------------------------------------------------*/

static void
sge_set_job_cnts( sge_ref_t *ref,
                  int queued )
{
   int prj_job_cnt_nm = queued ? PR_pending_job_cnt : PR_job_cnt;
   int user_job_cnt_nm = queued ? UU_pending_job_cnt : UU_job_cnt;
   int us_job_cnt_nm = queued ? US_pending_job_cnt : US_job_cnt;
   if (ref->user) {
      lAddUlong(ref->user, user_job_cnt_nm, 1);
   }   
   if (ref->project) {
      lAddUlong(ref->project, prj_job_cnt_nm, 1);
   }   
   if (ref->dept) {
      lAddUlong(ref->dept, us_job_cnt_nm, 1);
   }

   return;
}


/*--------------------------------------------------------------------
 * sge_unset_job_cnts - set job counts in the various object entries
 *--------------------------------------------------------------------*/

static void
sge_unset_job_cnts( sge_ref_t *ref,
                    int queued )
{
   int prj_job_cnt_nm = queued ? PR_pending_job_cnt : PR_job_cnt;
   int user_job_cnt_nm = queued ? UU_pending_job_cnt : UU_job_cnt;
   int us_job_cnt_nm = queued ? US_pending_job_cnt : US_job_cnt;
   if (ref->user) {
      lAddUlong(ref->user, user_job_cnt_nm, -1);
   }   
   if (ref->project) {
      lAddUlong(ref->project, prj_job_cnt_nm, -1);
   }   
   if (ref->dept) {
      lAddUlong(ref->dept, us_job_cnt_nm, -1);
   }
   return;
}


/*--------------------------------------------------------------------
 * calculate_m_shares - calculate m_share for share tree node
 *      descendants
 *
 * Calling calculate_m_shares(root_node) will calculate shares for
 * every active node in the share tree.
 *
 * Rather than having to recalculate m_shares on every scheduling
 * interval, we calculate it whenever the scheduler comes up or
 * whenever the share tree itself changes and make adjustments
 * every time a job becomes active or inactive (in adjust_m_shares).
 *--------------------------------------------------------------------*/

static void
calculate_m_shares( lListElem *parent_node )
{
   u_long sum_of_child_shares = 0;
   lListElem *child_node;
   lList *children;
   double parent_m_share;

   DENTER(TOP_LAYER, "calculate_m_shares");

   children = lGetList(parent_node, STN_children);
   if (!children) {
      DEXIT;
      return;
   }

   /*-------------------------------------------------------------
    * Sum child shares
    *-------------------------------------------------------------*/

   for_each(child_node, children) {

      if (lGetUlong(child_node, STN_job_ref_count) > 0)
         sum_of_child_shares += lGetUlong(child_node, STN_shares);
   }

   /*-------------------------------------------------------------
    * Calculate m_shares for each child and descendants
    *-------------------------------------------------------------*/

   parent_m_share = lGetDouble(parent_node, STN_m_share);

   for_each(child_node, children) {
      if (lGetUlong(child_node, STN_job_ref_count) > 0)
         lSetDouble(child_node, STN_m_share,
                    parent_m_share *
                    (sum_of_child_shares ?
                    ((double)(lGetUlong(child_node, STN_shares)) /
                    (double)sum_of_child_shares) : 0));
      else
         lSetDouble(child_node, STN_m_share, 0);

      calculate_m_shares(child_node);
   }

   DEXIT;
   return;
}


/*--------------------------------------------------------------------
 * update_job_ref_count - update job_ref_count for node and descendants
 *--------------------------------------------------------------------*/

u_long
update_job_ref_count( lListElem *node )
{
   int job_count=0;
   lList *children;
   lListElem *child;

   children = lGetList(node, STN_children);
   if (children) {
      for_each(child, children) {
         job_count += update_job_ref_count(child);
      }
      lSetUlong(node, STN_job_ref_count, job_count);
   }

   return lGetUlong(node, STN_job_ref_count);
}

/*--------------------------------------------------------------------
 * update_active_job_ref_count - update active_job_ref_count for node and descendants
 *--------------------------------------------------------------------*/

static u_long
update_active_job_ref_count( lListElem *node )
{
   int active_job_count=0;
   lList *children;
   lListElem *child;

   children = lGetList(node, STN_children);
   if (children) {
      for_each(child, children) {
         active_job_count += update_active_job_ref_count(child);
      }
      lSetUlong(node, STN_active_job_ref_count, active_job_count);
   }

   return lGetUlong(node, STN_active_job_ref_count);
}


/*--------------------------------------------------------------------
 * sge_init_share_tree_node_fields - zero out the share tree node
 * fields that will be set and used during sge_calc_tickets
 *--------------------------------------------------------------------*/

static int
sge_init_share_tree_node_fields( lListElem *node,
                                 void *ptr )
{
   static int sn_m_share_pos = -1;
   static int sn_adjusted_current_proportion_pos,
              sn_last_actual_proportion_pos, sn_sum_priority_pos,
              sn_job_ref_count_pos, sn_active_job_ref_count_pos,
              sn_usage_list_pos, sn_stt_pos, sn_ostt_pos,
              sn_ltt_pos, sn_oltt_pos, sn_shr_pos, sn_ref_pos,
              sn_actual_proportion_pos, sn_queued_pos;


   if (sn_m_share_pos == -1) {
      sn_m_share_pos = lGetPosViaElem(node, STN_m_share, SGE_NO_ABORT);
      sn_adjusted_current_proportion_pos =
            lGetPosViaElem(node, STN_adjusted_current_proportion, SGE_NO_ABORT);
      sn_last_actual_proportion_pos =
            lGetPosViaElem(node, STN_last_actual_proportion, SGE_NO_ABORT);
      sn_job_ref_count_pos = lGetPosViaElem(node, STN_job_ref_count, SGE_NO_ABORT);
      sn_active_job_ref_count_pos = lGetPosViaElem(node, STN_active_job_ref_count, SGE_NO_ABORT);
      sn_usage_list_pos = lGetPosViaElem(node, STN_usage_list, SGE_NO_ABORT);
      sn_sum_priority_pos = lGetPosViaElem(node, STN_sum_priority, SGE_NO_ABORT);
      sn_stt_pos = lGetPosViaElem(node, STN_stt, SGE_NO_ABORT);
      sn_ostt_pos = lGetPosViaElem(node, STN_ostt, SGE_NO_ABORT);
      sn_ltt_pos = lGetPosViaElem(node, STN_ltt, SGE_NO_ABORT);
      sn_oltt_pos = lGetPosViaElem(node, STN_oltt, SGE_NO_ABORT);
      sn_shr_pos = lGetPosViaElem(node, STN_shr, SGE_NO_ABORT);
      sn_ref_pos = lGetPosViaElem(node, STN_ref, SGE_NO_ABORT);
      sn_actual_proportion_pos = lGetPosViaElem(node, STN_actual_proportion, SGE_NO_ABORT);
      sn_queued_pos = lGetPosViaElem(node, STN_queued, SGE_NO_ABORT);
   }

   lSetPosDouble(node, sn_m_share_pos, 0);
   lSetPosDouble(node, sn_last_actual_proportion_pos, 0);
   lSetPosDouble(node, sn_adjusted_current_proportion_pos, 0);
   lSetPosDouble(node, sn_actual_proportion_pos, 0);
   lSetPosUlong(node, sn_job_ref_count_pos, 0);
   lSetPosUlong(node, sn_active_job_ref_count_pos, 0);
   lSetPosList(node, sn_usage_list_pos, NULL);
   lSetPosUlong(node, sn_sum_priority_pos, 0);
   lSetPosDouble(node, sn_stt_pos, 0);
   lSetPosDouble(node, sn_ostt_pos, 0);
   lSetPosDouble(node, sn_ltt_pos, 0);
   lSetPosDouble(node, sn_oltt_pos, 0);
   lSetPosDouble(node, sn_shr_pos, 0);
   lSetPosUlong(node, sn_ref_pos, 0);
   lSetPosUlong(node, sn_queued_pos, 0);
   return 0;
}

/*--------------------------------------------------------------------
 * sge_init_share_tree_nodes - zero out the share tree node fields 
 * that will be set and used during sge_calc_tickets
 *--------------------------------------------------------------------*/

static int
sge_init_share_tree_nodes( lListElem *root )
{
   return sge_for_each_share_tree_node(root,
         sge_init_share_tree_node_fields, NULL);
}


/*--------------------------------------------------------------------
 * get_usage_or_create - return or create usage entry based on name
 *--------------------------------------------------------------------*/

static lListElem *
get_usage_or_create( lList *usage_list,
                   const char *name )
{
   lListElem *usage = lGetElemStr(usage_list, UA_name, name);
   if (!usage) {
      /* Create entry */
      usage = lCreateElem(UA_Type);
      lSetString(usage, UA_name, name);
      lSetDouble(usage, UA_value, 0);
      lAppendElem(usage_list, usage);
   }

   return usage;
}


/*--------------------------------------------------------------------
 * build_usage_list - create a new usage list from an existing list
 *--------------------------------------------------------------------*/

static lList *
build_usage_list( char *name,
                  lList *old_usage_list )
{
   lList *usage_list = NULL;
   lListElem *usage;

   if (old_usage_list) {

      /*-------------------------------------------------------------
       * Copy the old list and zero out the usage values
       *-------------------------------------------------------------*/

      usage_list = lCopyList(name, old_usage_list);
      for_each(usage, usage_list)
         lSetDouble(usage, UA_value, 0);

   } else {

      /*
       * the UA_value fields are implicitly set to 0 at creation
       * time of a new element with lCreateElem or lAddElemStr
       */
        
      lAddElemStr(&usage_list, UA_name, USAGE_ATTR_CPU, UA_Type);
      lAddElemStr(&usage_list, UA_name, USAGE_ATTR_MEM, UA_Type);
      lAddElemStr(&usage_list, UA_name, USAGE_ATTR_IO, UA_Type);
   }

   return usage_list;
}


/*-----------------------------------------------------------------------
 * get_or_build_usage_list - return usage list, create if does not exist
 *-----------------------------------------------------------------------*/
static lList *
get_or_build_usage_list(lListElem *ep, int field, char *name)
{
   lList *usage_list = lGetList(ep, field);
   if (!usage_list) {
      usage_list = build_usage_list(name, NULL);
      lSetList(ep, field, usage_list);
   }

   return usage_list;
}


/*--------------------------------------------------------------------
 * delete_debited_job_usage - deleted debitted job usage for job
 *--------------------------------------------------------------------*/

static void
delete_debited_job_usage( sge_ref_t *ref,
                          u_long seqno )
{
   lListElem *job=ref->job,
             *user=ref->user,
             *project=ref->project;
   lList *upu_list;
   lListElem *upu;
   
   DENTER(TOP_LAYER, "delete_debited_job_usage");

   DPRINTF(("DDJU (1) "sge_u32"\n", lGetUlong(job, JB_job_number)));

   if (user) {
      upu_list = lGetList(user, UU_debited_job_usage);
      DPRINTF(("DDJU (2) "sge_u32"\n", lGetUlong(job, JB_job_number)));
      if (upu_list) {
         
         /* Note: In order to cause the qmaster to delete the
            usage for this job, we zero out UPU_old_usage_list
            for this job in the UU_debited_job_usage list */
         DPRINTF(("DDJU (3) "sge_u32"\n", lGetUlong(job, JB_job_number))); 

         if ((upu = lGetElemUlong(upu_list, UPU_job_number,
                             lGetUlong(job, JB_job_number)))) {
            DPRINTF(("DDJU (4) "sge_u32"\n", lGetUlong(job, JB_job_number)));
            lSetList(upu, UPU_old_usage_list, NULL);
            lSetUlong(user, UU_usage_seqno, seqno);
         }
      }
   }

   if (project) {
      upu_list = lGetList(project, PR_debited_job_usage);
      if (upu_list) {
         
         /* Note: In order to cause the qmaster to delete the
            usage for this job, we zero out UPU_old_usage_list
            for this job in the PR_debited_job_usage list */

         if ((upu = lGetElemUlong(upu_list, UPU_job_number,
                             lGetUlong(job, JB_job_number)))) {
            lSetList(upu, UPU_old_usage_list, NULL);
            lSetUlong(project, PR_usage_seqno, seqno);
         }
      }
   }

   DEXIT;
   return;
}


/*--------------------------------------------------------------------
 * combine_usage - combines a node's associated user/project usage 
 * into a single value and stores it in the node.
 *--------------------------------------------------------------------*/

static void
combine_usage( sge_ref_t *ref )
{
   double usage_value = 0;

   /*-------------------------------------------------------------
    * Get usage from associated user/project object
    *-------------------------------------------------------------*/
   if (ref->node) {
      lList *usage_weight_list = NULL;
      lList *usage_list=NULL;
      lListElem *usage_weight, *usage_elem;
      double sum_of_usage_weights = 0;
      const char *usage_name;

      /*-------------------------------------------------------------
       * Sum usage weighting factors
       *-------------------------------------------------------------*/

      usage_weight_list = sconf_get_usage_weight_list();
      if (usage_weight_list) {
          for_each(usage_weight, usage_weight_list)
             sum_of_usage_weights += lGetDouble(usage_weight, UA_value);
      }

      /*-------------------------------------------------------------
       * Combine user/project usage based on usage weighting factors
       *-------------------------------------------------------------*/

      if (usage_weight_list) {

         if (ref->user) {
            if (ref->project) {
               lList *upp_list = lGetList(ref->user, UU_project);
               lListElem *upp;
               if (upp_list &&
                   ((upp = lGetElemStr(upp_list, UPP_name,
                                       lGetString(ref->project, PR_name)))))
                  usage_list = lGetList(upp, UPP_usage);
            } 
            else {
               usage_list = lGetList(ref->user, UU_usage);
            }
         } 
         else if (ref->project) { /* not sure about this, use it when? */
            usage_list = lGetList(ref->project, PR_usage);
         }

         for_each(usage_elem, usage_list) {
            usage_name = lGetString(usage_elem, UA_name);
            usage_weight = lGetElemStr(usage_weight_list, UA_name,
                                       usage_name);
            if (usage_weight && sum_of_usage_weights>0) {
               usage_value += lGetDouble(usage_elem, UA_value) *
                  (lGetDouble(usage_weight, UA_value) /
                   sum_of_usage_weights);
            }
         }
      }

      /*-------------------------------------------------------------
       * Set combined usage in the node
       *-------------------------------------------------------------*/

      lSetDouble(ref->node, STN_combined_usage, usage_value);
      lFreeList(&usage_weight_list);
   }

   return;
}


/*--------------------------------------------------------------------
 * sum_job_usage - accumulates job usage, returning a usage list
 *--------------------------------------------------------------------*/
static lList* sum_job_usage(lListElem *job)
{
   lListElem *jbtask, *petask;

   /* create empty usage_list */
   lList *usage_list = lCreateList("", UA_Type);

   /* sum tasks */
   for_each(jbtask, lGetList(job, JB_ja_tasks)) {
      usage_list_sum(usage_list,
                     lGetList(jbtask, JAT_scaled_usage_list));

      /* sum sub-tasks */
      /* Usage held in PET_scaled_usage if PE accounting_summary FALSE */
      for_each(petask, lGetList(jbtask, JAT_task_list)) {
         usage_list_sum(usage_list,
                        lGetList(petask, PET_scaled_usage));
      }
   }

   return usage_list;
}


/*
 * copy_or_init_oldusage - retrieves any user/project usage for the job
 */
static lList*
copy_or_init_oldusage(lListElem *userprj,
                      int obj_debited_job_usage,
                      lListElem *job)
{
   lList *old_usage_list = NULL;

   if (userprj) {
      lListElem *upu;
      lList *upu_list = lGetList(userprj, obj_debited_job_usage);
      if (upu_list) {
         if ((upu = lGetElemUlong(upu_list, UPU_job_number,
                                  lGetUlong(job, JB_job_number)))) {
            if ((old_usage_list = lGetList(upu, UPU_old_usage_list))) {
               old_usage_list = lCopyList("", old_usage_list);
            }
         }
      }
   }

   if (!old_usage_list) {
      old_usage_list = build_usage_list("old_usage_list", NULL);
   }

   return old_usage_list;
}


/*----------------------------------------------------------
 * save_oldusage - save job usage in user/project
 *----------------------------------------------------------*/
static void save_oldusage(lListElem *userprj,
                          int obj_debited_job_usage,
                          lListElem *job,
                          lList *usage_list)
{
   if (userprj) {
      lListElem *upu;
      u_long jobnum = lGetUlong(job, JB_job_number);
      lList *upu_list = lGetList(userprj, obj_debited_job_usage);
      if (!upu_list) {
         upu_list = lCreateList("", UPU_Type);
         lSetList(userprj, obj_debited_job_usage, upu_list);
      }
      if ((upu = lGetElemUlong(upu_list, UPU_job_number, jobnum))) {
         lSetList(upu, UPU_old_usage_list,
                  lCopyList(lGetListName(usage_list), usage_list));
      } else {
         upu = lCreateElem(UPU_Type);
         lSetUlong(upu, UPU_job_number, jobnum);
         lSetList(upu, UPU_old_usage_list,
                  lCopyList(lGetListName(usage_list), usage_list));
         lAppendElem(upu_list, upu);
      }
   }
}


/*
 * update_userprj_nodes
 */
static void
update_userprj_usage(lListElem *user,
                     lListElem *project,
                     lList *job_usage_list,
                     lList *old_usage_list)
{
   lList *user_usage_list = NULL;
   lList *project_usage_list = NULL;
   lList *user_long_term_usage_list = NULL;
   lList *project_long_term_usage_list = NULL;

   /*--------------------------------------------------------------------
    * Ensure the lists we need have been initialised.
    * If there is a user & project, usage is kept in the project sub-list
    *---------------------------------------------------------------------*/

   if (user) {
      if (project) {
         lList *upp_list = lGetList(user, UU_project);
         lListElem *upp;
         const char *project_name = lGetString(project, PR_name);

         if (!upp_list) {
            upp_list = lCreateList("", UPP_Type);
            lSetList(user, UU_project, upp_list);
         }
         if (!((upp = lGetElemStr(upp_list, UPP_name, project_name))))
            upp = lAddElemStr(&upp_list, UPP_name, project_name, UPP_Type);

         user_long_term_usage_list =
            get_or_build_usage_list(upp, UPP_long_term_usage, "upp_long_term_usage_list");

         user_usage_list =
            get_or_build_usage_list(upp, UPP_usage, "upp_usage_list");

      } else {
         user_long_term_usage_list =
            get_or_build_usage_list(user, UU_long_term_usage, "user_long_term_usage_list");

         user_usage_list =
            get_or_build_usage_list(user, UU_usage, "user_usage_list");
      }
   }

   if (project) {
      project_long_term_usage_list =
         get_or_build_usage_list(project, PR_long_term_usage, "project_long_term_usage_list");

      project_usage_list =
         get_or_build_usage_list(project, PR_usage, "project_usage_list");
   }

   if (job_usage_list) {
      lListElem *job_usage;

      for_each(job_usage, job_usage_list) {

         lListElem *old_usage=NULL, 
                   *user_usage=NULL, *project_usage=NULL,
                   *user_long_term_usage=NULL,
                   *project_long_term_usage=NULL;
         const char *usage_name = lGetString(job_usage, UA_name);

         /*---------------------------------------------------------
          * Locate the corresponding usage element for the job
          * usage type in the node usage, old job usage, user usage,
          * and project usage.  If it does not exist, create a new
          * corresponding usage element.
          *---------------------------------------------------------*/

         if (old_usage_list) old_usage =
            get_usage_or_create(old_usage_list, usage_name);

         if (user_usage_list) user_usage =
            get_usage_or_create(user_usage_list, usage_name);

         if (user_long_term_usage_list) user_long_term_usage =
            get_usage_or_create(user_long_term_usage_list, usage_name);

         if (project_usage_list) project_usage =
            get_usage_or_create(project_usage_list, usage_name);

         if (project_long_term_usage_list) project_long_term_usage =
            get_usage_or_create(project_long_term_usage_list, usage_name);


         if (job_usage && old_usage) {

            double usage_value;

            usage_value = MAX(lGetDouble(job_usage, UA_value) -
                              lGetDouble(old_usage, UA_value), 0);
                                     
            /*---------------------------------------------------
             * Add usage to decayed user usage
             *---------------------------------------------------*/

            if (user_usage)
                lSetDouble(user_usage, UA_value,
                   lGetDouble(user_usage, UA_value) + usage_value);

            /*---------------------------------------------------
             * Add usage to long term user usage
             *---------------------------------------------------*/

            if (user_long_term_usage)
                lSetDouble(user_long_term_usage, UA_value,
                   lGetDouble(user_long_term_usage, UA_value) + usage_value);

            /*---------------------------------------------------
             * Add usage to decayed project usage
             *---------------------------------------------------*/

            if (project_usage)
                lSetDouble(project_usage, UA_value,
                   lGetDouble(project_usage, UA_value) + usage_value);

            /*---------------------------------------------------
             * Add usage to long term project usage
             *---------------------------------------------------*/

            if (project_long_term_usage)
                lSetDouble(project_long_term_usage, UA_value,
                   lGetDouble(project_long_term_usage, UA_value) + usage_value);
         }
      }
   }
}

/*--------------------------------------------------------------------
 * decay_and_sum_usage - accumulates and decays usage in the correct
 * user and project objects for the specified job
 *
 * running_part is not NULL if function is processing the finished tasks
 *   of a job with other tasks still running. running contains a job
 *   entry containing the running tasks
 *--------------------------------------------------------------------*/

static void
decay_and_sum_usage( sge_ref_t *ref,
                     lList *decay_list,
                     lListElem *running_part,
                     u_long seqno,
                     u_long curr_time )
{
   lList *job_usage_list=NULL,
         *old_usage_list=NULL,
         *run_usage_list=NULL,
         *fin_usage_list=NULL;
   lListElem *job=ref->job,
             *ja_task=ref->ja_task,
             *node=ref->node,
             *user=ref->user,
             *project=ref->project,
             *userprj = NULL;
   int obj_debited_job_usage = PR_debited_job_usage;          

   /*-----------------------------------------
    * Don't process usage if we don't have to
    *-----------------------------------------*/

   /* - no objects to save usage in */
   if (!node && !user && !project) {
      return;
   }

   /* - job has no tasks yet - not sure why job appears in list */
   if (ja_task == NULL) {
      return;
   }

   /* - not first task in job - so have already done this */
   if (ja_task != NULL && ja_task != lFirst(lGetList(job, JB_ja_tasks))) {
      return;
   }


   if (ref->user) {
      userprj = ref->user;
      obj_debited_job_usage = UU_debited_job_usage;
   } else if (ref->project) {
      userprj = ref->project;
      obj_debited_job_usage = PR_debited_job_usage;
   }

   /*-------------------------------------------------------------
    * Decay the usage for the associated user and project
    *-------------------------------------------------------------*/
    
   if (user) {
      decay_userprj_usage(user, true, decay_list, seqno, curr_time);
   }

   if (project) {
      decay_userprj_usage(project, false, decay_list, seqno, curr_time);
   }

   /*-------------------------------------------------------------
    * Calculate job usage
    *
    * Note: Since SGE will update job.usage directly, we 
    * maintain the job usage the last time we collected it from
    * the job.  The difference between the new usage and the old
    * usage is what needs to be added to the user or project node.
    * This old usage is maintained in the user or project node
    * depending on the type of share tree.
    *-------------------------------------------------------------*/

   /*------------------------------
    * Sum usage assocated with job
    *------------------------------*/

   job_usage_list = sum_job_usage(job);

   /* For partly finished array jobs, get entire usage (finished+running) */
   if (running_part) {
      fin_usage_list = lCopyList("", job_usage_list);
      run_usage_list = sum_job_usage(running_part);

      usage_list_sum(job_usage_list, run_usage_list);
   }

   /*--------------------------------------------
    * Read any stored running usage from project
    *--------------------------------------------*/

   old_usage_list = copy_or_init_oldusage(userprj, obj_debited_job_usage, job);

   /*-------------------------------------------------------------
    * Add to user/project node usage for each usage type
    *-------------------------------------------------------------*/

   update_userprj_usage(user, project, job_usage_list, old_usage_list);


   /*-----------------------------------------------------------------------
    * Subtract the finished tasks of a running array job from the job usage
    *-----------------------------------------------------------------------*/
   if (running_part) {
      usage_list_sub(job_usage_list, fin_usage_list);
   }


   /*--------------------------------------------------
    * save current job usage in debited job usage list
    *--------------------------------------------------*/

   if (job_usage_list) {
      save_oldusage(userprj, obj_debited_job_usage, job, job_usage_list);
      lFreeList(&job_usage_list);
   }

   lFreeList(&old_usage_list);
   lFreeList(&run_usage_list);
   lFreeList(&fin_usage_list);
}



/*--------------------------------------------------------------------
 * calc_job_share_tree_tickets_pass1 - performs pass 1 of calculating
 *      the job share tree tickets for the specified job
 *--------------------------------------------------------------------*/

static void
calc_job_share_tree_tickets_pass1(sge_ref_t *ref)
{
   lListElem *job = ref->job;
   lListElem *node = ref->node;

   if (!node) {
      return;
   }

   /*-------------------------------------------------------
    * sum job shares for each job for use in pass 2
    *-------------------------------------------------------*/

   lSetUlong(node, STN_sum_priority,
             lGetUlong(node, STN_sum_priority) +
             lGetUlong(job, JB_jobshare));
}


/*--------------------------------------------------------------------
 * calc_job_share_tree_tickets_pass2 - performs pass 2 of calculating
 *      the job share tree tickets for the specified job
 *--------------------------------------------------------------------*/

static void
calc_job_share_tree_tickets_pass2( sge_ref_t *ref, double total_share_tree_tickets)
{
   double share_tree_tickets;
   lListElem *job = ref->job;
   lListElem *node = ref->node;

   if (!node) {
      return;
   }

   /*-------------------------------------------------------
    * calculate the number of share tree tickets for this job
    *-------------------------------------------------------*/

   share_tree_tickets = lGetDouble(node, STN_adjusted_current_proportion) *
                        total_share_tree_tickets;

   if (lGetUlong(node, STN_sum_priority)) {
      REF_SET_STICKET(ref, 
                (u_long)((double)lGetUlong(job, JB_jobshare) *
                share_tree_tickets /
                lGetUlong(node, STN_sum_priority)));
   } else {
      REF_SET_STICKET(ref,
                share_tree_tickets / lGetUlong(node, STN_job_ref_count));
   }
}

/****** sgeee/copy_ftickets() **************************************************
*  NAME
*     copy_ftickets() -- copy the ftix from one job to an other one 
*
*  SYNOPSIS
*     void copy_ftickets(sge_ref_list_t *source, sge_ref_list_t *dest) 
*
*  FUNCTION
*     Copy the functional tickets and ref fields used for ftix calculation
*     from one job to an other job. 
*
*  INPUTS
*     sge_ref_list_t *source - source job  
*     sge_ref_list_t *dest   - dest job 
*
*  BUGS
*     ??? 
*******************************************************************************/
static void copy_ftickets(sge_ref_list_t *source, sge_ref_list_t *dest){
   if (source == NULL || dest == NULL) {
      return;
   }
   else { 
      sge_ref_t *dest_r = dest->ref;
      sge_ref_t *source_r = source->ref;
 
      dest_r->user_fshare = source_r->user_fshare;
      dest_r->project_fshare = source_r->project_fshare;
      dest_r->dept_fshare = source_r->dept_fshare;
      dest_r->job_fshare = source_r->job_fshare;

      REF_SET_FSHARE(dest_r, lGetUlong(source_r->job, JB_jobshare));
      REF_SET_FTICKET(dest_r, REF_GET_FTICKET(source_r));
   }
}

/****** sgeee/destribute_ftickets() ********************************************
*  NAME
*     destribute_ftickets() -- ensures, that all jobs have ftix asoziated with them. 
*
*  SYNOPSIS
*     void destribute_ftickets(sge_fcategory_t *root, int dependent) 
*
*  FUNCTION
*     After the functional tickets are calculated, only the first job in the fcategory
*     job list has ftix. This function copies the result from the first job to all
*     other jobs in the same list and sums the job ticket count with the ftix. 
*
*  INPUTS
*     sge_fcategory_t *root - fcategory list 
*     int dependent         - does the final ticket count depend on ftix? 
*
*
*
*  NOTES
*     - This function is only needed, because not all functional tickets are calculated
*       and to give a best guess result, all jobs in one category with no ftix get the
*       same amount of ftix. 
*
*******************************************************************************/
static void destribute_ftickets(lList *root, int dependent){   
   lListElem *elem;
   sge_ref_list_t *current = NULL;
   sge_ref_list_t *first = NULL;

   for_each(elem, root) {
      first = lGetRef(elem, FCAT_jobrelated_ticket_first);
/*      if(!first)
         continue;*/
      current = first->next;
      
      while(current != NULL) {
         copy_ftickets(first, current); 
         if(dependent)
            current->ref->tickets += REF_GET_FTICKET(current->ref);        
         current = current->next; 
      }
   }
}

/*
 * job classes are ignored.
 */
/****** sgeee/build_functional_categories() ************************************
*  NAME
*     build_functional_categories() --  sorts the pending jobs into functional categories
*
*  SYNOPSIS
*     void build_functional_categories(sge_ref_t *job_ref, int num_jobs, 
*     sge_fcategory_t **root, int dependent) 
*
*  FUNCTION
*     Generates a list of functional categories. Each category contains a list of jobs
*     which belongs to this category. A functional category is assembled of:
*     - job shares
*     - user shares
*     - department shares
*     - project shares 
*     Alljobs with the same job, user,... shares are put in the same fcategory.
*
*  INPUTS
*     sge_ref_t *job_ref     - array of pointers to the job reference structure 
*     int num_jobs           - number of elements in the job_ref array
*     sge_fcategory_t **root - root pointer to the functional category list 
*     sge_ref_list_t ** ref_array - has to be a pointer to NULL pointer. The memory 
*                                   will be allocated
*                                   in this function and freed with free_fcategories.
*     int dependent          - does the functional tickets depend on prior computed tickets? 
*     u_long32 job_tickets   - job field, which has the tickets ( JB>_jobshare, JB_override_tickets)
*     u_long32 up_tickets    - source for the user/department tickets/shares (UP_fshare, UP_otickets)
*     u_long32 dp_tickets    - source for the department tickets/shares (US_fshare, US_oticket)
*
*  OUTPUT
*     u_long32 - number of jobs in the categories
*
*  NOTES
*     - job classes are ignored.
*
*  IMPROVEMENTS:
*     - the stored values in the functional category structure can be used to speed up the
*       ticket calculation. This will avoid unnecessary CULL accesses in the function
*       calc_job_functional_tickets_pass1
*     - A further improvement can be done by:
*        - limiting the job list length in each category to the max nr of jobs calculated
*        - Sorting the jobs in each functional category by its job category. Each resulting
*          job list can be of max size of open slots. This will result in a correct ftix result
*          for all jobs, which might be scheduled.
*
*  BUGS
*     ??? 
* 
*******************************************************************************/
static u_long32 build_functional_categories(sge_ref_t *job_ref, int num_jobs, lList **fcategories, 
                                        sge_ref_list_t ** ref_array, int dependent, 
                                        u_long32 job_tickets, u_long32 user_tickets, u_long32 project_tickets, u_long32 dp_tickets) {
   lListElem *current;
   u_long32 job_ndx; 
   int ref_array_pos = 0;
   u_long32 job_counter = 0;
   
   DENTER(TOP_LAYER,"build_functional_categories");

   *ref_array = malloc(sizeof(sge_ref_list_t) * num_jobs);
   if (*ref_array == NULL) {
      DEXIT;
      return 0;
   }
   
   memset(*ref_array, 0, sizeof(sge_ref_list_t) * num_jobs);

   for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {
      sge_ref_t *jref = &job_ref[job_ndx];

      if (jref->queued) { /* only work on pending jobs */
         lListElem *user_el;
         lListElem *dept_el;
         lListElem *project_el;
         u_long32 job_shares = 0;  
         u_long32 user_shares = 0;
         u_long32 dept_shares = 0;
         u_long32 project_shares = 0;

         user_el = jref->user;
         dept_el = jref->dept;
         project_el = jref->project;

         if(user_el){
            user_shares = lGetUlong(user_el, user_tickets); 
            if (user_shares == 0) {
               user_el = NULL;
            }
         }
         if(project_el){
            project_shares = lGetUlong(project_el, project_tickets); 
            if (project_shares == 0) {
               project_el = NULL;
            }

         }
         if(dept_el){
            dept_shares = lGetUlong(dept_el, dp_tickets); 
            if (dept_shares == 0) {
               dept_el = NULL;
            }

         }

         job_shares =  lGetUlong(jref->job, job_tickets);
         
         /* jobs in this category will never get tickets, so why compute them...? */
         if ( (job_shares == 0) && 
              (user_shares == 0) && 
              (dept_shares == 0) && 
              (project_shares == 0)) {
           continue;
         }  

         job_counter++;

         /* locate the right category */
         for_each (current, *fcategories) {
            if ((lGetUlong(current, FCAT_job_share) == job_shares) &&
                (lGetRef(current, FCAT_user) == user_el) &&
                (lGetRef(current, FCAT_dept) == dept_el) &&
                (lGetRef(current, FCAT_project) == project_el)) {
               break;
            }
         }

         if (current == NULL) {   /* create new category */
            if (!(current = lAddElemUlong(fcategories, FCAT_job_share, job_shares, FCAT_Type))) {
               free_fcategories(fcategories, ref_array);
               /* maybe an error code */
               DEXIT;
               return 0;
            }
            lSetUlong(current, FCAT_user_share, user_shares);
            lSetRef(current, FCAT_user, user_el);
            lSetUlong(current, FCAT_dept_share, dept_shares);
            lSetRef(current, FCAT_dept, dept_el);
            lSetUlong(current, FCAT_project_share, project_shares);
            lSetRef(current, FCAT_project, project_el);
         }

         {  /* create new job entry */ 
            sge_ref_list_t  *cur_ref_entry = NULL;
            sge_ref_list_t *new_elem = &((*ref_array)[ref_array_pos++]);

            new_elem->ref = jref;

            /* find the location for the new element */
            cur_ref_entry = lGetRef(current, FCAT_jobrelated_ticket_last);

            if (dependent) {
               while (cur_ref_entry != NULL) {
                  sge_ref_t *ref = cur_ref_entry->ref;

                  if (ref->tickets > jref->tickets) {
                     break;
                  }   
                  else if (ref->tickets == jref->tickets){
                     u_long32 ref_time = lGetUlong(ref->job, JB_submission_time);
                     u_long32 jref_time = lGetUlong(jref->job, JB_submission_time);
                     u_long32 ref_jid = lGetUlong(ref->job, JB_job_number);
                     u_long32 jref_jid = lGetUlong(jref->job, JB_job_number);
		     if (ref_time < jref_time ||
                           (ref_time == jref_time && ref_jid < jref_jid)) {
                        break;
                     }
                     else if ((ref_jid == jref_jid)
                           && (REF_GET_JA_TASK_NUMBER(ref) < REF_GET_JA_TASK_NUMBER(jref)))
                        break;
                  }

                  cur_ref_entry = cur_ref_entry->prev;
               }
            }

            /* insert element into the list */ 
            if (cur_ref_entry == NULL) {
               if (lGetRef(current, FCAT_jobrelated_ticket_first) == NULL) {
                  /* first element, no elements in the list */
                  lSetRef(current, FCAT_jobrelated_ticket_first, new_elem);
                  lSetRef(current, FCAT_jobrelated_ticket_last, new_elem);
               } 
               else {
                  /* insert element at the beginning */
                  cur_ref_entry = lGetRef(current, FCAT_jobrelated_ticket_first);
                  cur_ref_entry->prev = new_elem;
                  new_elem->next = cur_ref_entry;
                  lSetRef(current, FCAT_jobrelated_ticket_first, new_elem); 
                  
               }
            }
            else {
               /* insert element somewhere in the list */
               new_elem->prev = cur_ref_entry;
               new_elem->next = cur_ref_entry->next;
               if(cur_ref_entry->next != NULL){
                  cur_ref_entry->next->prev = new_elem;
               }
               else { 
                  /* element is the last in the list */
                  lSetRef(current, FCAT_jobrelated_ticket_last, new_elem);
               }
               cur_ref_entry->next = new_elem; 
            }
         }
      }

   }
   DEXIT;
   return job_counter;
}

/****** sgeee/free_fcategories() ***********************************************
*  NAME
*     free_fcategories() -- frees all fcategories and their job lists. 
*
*  SYNOPSIS
*     void free_fcategories(sge_fcategory_t **fcategories) 
*
*  FUNCTION
*     frees all fcategories and their job lists. 
*
*  INPUTS
*     sge_fcategory_t **fcategories /- pointer to a pointer of the first fcategory 
*     sge_ref_list_t **ref_array - memory for internal structures, allocated with
*     build_functional_categories. Needs to be freed as well.
*
*  NOTES
*     - it does not delete the sge_ref_t structures, which are stored in
*       in the job lists. 
*
*******************************************************************************/
static void free_fcategories(lList **fcategories, sge_ref_list_t **ref_array) {

   lListElem *fcategory;

   for_each(fcategory, *fcategories) {
      lSetRef(fcategory, FCAT_jobrelated_ticket_last, NULL);
      lSetRef(fcategory, FCAT_jobrelated_ticket_first, NULL);      
      lSetRef(fcategory, FCAT_user, NULL);
      lSetRef(fcategory, FCAT_dept, NULL);
      lSetRef(fcategory, FCAT_project, NULL);
   }

   lFreeList(fcategories);
   sge_free(ref_array);

}

/*--------------------------------------------------------------------
 * calc_functional_tickets_pass1 - performs pass 1 of calculating
 *      the functional tickets for the specified job
 *--------------------------------------------------------------------*/

static void calc_job_functional_tickets_pass1( sge_ref_t *ref,
                                   double *sum_of_user_functional_shares,
                                   double *sum_of_project_functional_shares,
                                   double *sum_of_department_functional_shares,
                                   double *sum_of_job_functional_shares,
                                   int shared,
                                   int sum_shares)
{
   double job_cnt;

   /*-------------------------------------------------------------
    * Sum user functional shares
    *-------------------------------------------------------------*/

   if (ref->user) {
      job_cnt = lGetUlong(ref->user, UU_job_cnt);
      ref->user_fshare = shared ?
            (double)lGetUlong(ref->user, UU_fshare) / job_cnt :
            lGetUlong(ref->user, UU_fshare);
      if(sum_shares || job_cnt<=1)
         *sum_of_user_functional_shares += ref->user_fshare;

   }

   /*-------------------------------------------------------------
    * Sum project functional shares
    *-------------------------------------------------------------*/

   if (ref->project) {
      job_cnt = lGetUlong(ref->project, PR_job_cnt);
      ref->project_fshare = shared ?
            (double)lGetUlong(ref->project, PR_fshare) / job_cnt :
            lGetUlong(ref->project, PR_fshare);
      if(sum_shares || job_cnt<=1)
         *sum_of_project_functional_shares += ref->project_fshare;
   }

   /*-------------------------------------------------------------
    * Sum department functional shares
    *-------------------------------------------------------------*/

   if (ref->dept) {
      job_cnt = lGetUlong(ref->dept, US_job_cnt);
      ref->dept_fshare = shared ?
            (double)lGetUlong(ref->dept, US_fshare) / job_cnt :
            lGetUlong(ref->dept, US_fshare);
      if(sum_shares || job_cnt<=1)
         *sum_of_department_functional_shares += ref->dept_fshare;
   }

   /*-------------------------------------------------------------
    * Sum job functional shares
    *-------------------------------------------------------------*/

   REF_SET_FSHARE(ref, lGetUlong(ref->job, JB_jobshare));

   *sum_of_job_functional_shares += REF_GET_FSHARE(ref);
}

enum {
   k_user=0,
   k_department,
   k_project,
   k_job,
   k_last
};


/*--------------------------------------------------------------------
 * get_functional_weighting_parameters - get the weighting parameters for
 *      the functional policy categories from the configuration
 *--------------------------------------------------------------------*/


static void
get_functional_weighting_parameters( double sum_of_user_functional_shares,
                                     double sum_of_project_functional_shares,
                                     double sum_of_department_functional_shares,
                                     double sum_of_job_functional_shares,
                                     double weight[] )
{
   double k_sum;
   memset(weight, 0, sizeof(double)*k_last);

   if (sconf_is()) {
      if (sum_of_user_functional_shares > 0)
         weight[k_user] = sconf_get_weight_user();
      if (sum_of_department_functional_shares > 0)
         weight[k_department] = sconf_get_weight_department();
      if (sum_of_project_functional_shares > 0)
         weight[k_project] = sconf_get_weight_project();
      if (sum_of_job_functional_shares > 0)
         weight[k_job] = sconf_get_weight_job();
      k_sum = weight[k_user] + weight[k_department] + weight[k_project] +
              weight[k_job];
   } else {
      weight[k_user] = 1;
      weight[k_department] = 1;
      weight[k_project] = 1;
      weight[k_job] = 1;
      k_sum = weight[k_user] + weight[k_department] + weight[k_project] +
              weight[k_job];
   }

   if (k_sum>0) {
      weight[k_user] /= k_sum;
      weight[k_department] /= k_sum;
      weight[k_project] /= k_sum;
      weight[k_job] /= k_sum;
   }

   return;
}


/*--------------------------------------------------------------------
 * calc_functional_tickets_pass2 - performs pass 2 of calculating
 *      the functional tickets for the specified job
 *--------------------------------------------------------------------*/

static double
calc_job_functional_tickets_pass2( sge_ref_t *ref,
                                   double sum_of_user_functional_shares,
                                   double sum_of_project_functional_shares,
                                   double sum_of_department_functional_shares,
                                   double sum_of_job_functional_shares,
                                   double total_functional_tickets,
                                   double weight[])
{
   double user_functional_tickets=0,
          project_functional_tickets=0,
          department_functional_tickets=0,
          job_functional_tickets=0,
          total_job_functional_tickets;

   /*-------------------------------------------------------
    * calculate user functional tickets for this job
    *-------------------------------------------------------*/

   if (ref->user && sum_of_user_functional_shares) {
      user_functional_tickets = (ref->user_fshare *
                                total_functional_tickets /
                                sum_of_user_functional_shares);
   }

   /*-------------------------------------------------------
    * calculate project functional tickets for this job
    *-------------------------------------------------------*/

   if (ref->project && sum_of_project_functional_shares)
      project_functional_tickets = (ref->project_fshare *
                                 total_functional_tickets /
                                 sum_of_project_functional_shares);

   /*-------------------------------------------------------
    * calculate department functional tickets for this job
    *-------------------------------------------------------*/

   if (ref->dept && sum_of_department_functional_shares)
      department_functional_tickets = (ref->dept_fshare *
                                 total_functional_tickets /
                                 sum_of_department_functional_shares);

   /*-------------------------------------------------------
    * calculate job functional tickets for this job
    *-------------------------------------------------------*/

   if (sum_of_job_functional_shares)
      job_functional_tickets = ((double)lGetUlong(ref->job, JB_jobshare) *
                                 (double)total_functional_tickets /
                                  sum_of_job_functional_shares);

   /*-------------------------------------------------------
    * calculate functional tickets for this job
    *-------------------------------------------------------*/

   total_job_functional_tickets = weight[k_user] * user_functional_tickets +
             weight[k_department] * department_functional_tickets +
             weight[k_project] * project_functional_tickets +
             weight[k_job] * job_functional_tickets;

   REF_SET_FTICKET(ref, total_job_functional_tickets);

   return job_functional_tickets;
}


/*--------------------------------------------------------------------
 * calc_override_tickets - calculates the number of override tickets for the
 * specified job
 *--------------------------------------------------------------------*/

static double calc_job_override_tickets( sge_ref_t *ref, int shared) {
   double job_override_tickets = 0;
   double otickets, job_cnt;

   DENTER(TOP_LAYER, "calc_job_override_tickets");

   /*-------------------------------------------------------
    * job.override_tickets = user.override_tickets +
    *                        project.override_tickets +
    *                        department.override_tickets +
    *                        job.override_tickets;
    *-------------------------------------------------------*/

   if (shared) {
      if (ref->user) {
         job_cnt = lGetUlong(ref->user, UU_job_cnt);
         if (((otickets = lGetUlong(ref->user, UU_oticket)) &&
             (job_cnt )))
            job_override_tickets += (otickets / job_cnt);
      }

      if (ref->project) {
         job_cnt = lGetUlong(ref->project, PR_job_cnt);
         if (((otickets = lGetUlong(ref->project, PR_oticket)) &&
             (job_cnt )))
            job_override_tickets += (otickets / job_cnt);
      }

      if (ref->dept) {
         job_cnt = lGetUlong(ref->dept, US_job_cnt);
         if (((otickets = lGetUlong(ref->dept, US_oticket)) &&
             (job_cnt )))
            job_override_tickets += (otickets / job_cnt);
      }
   }
   else {
      if (ref->user) {
         job_override_tickets += lGetUlong(ref->user, UU_oticket);
      }

      if (ref->project) {
         job_override_tickets += lGetUlong(ref->project, PR_oticket);
      }

      if (ref->dept) {
         job_override_tickets += lGetUlong(ref->dept, US_oticket);
      }
   }

   job_override_tickets += lGetUlong(ref->job, JB_override_tickets);

   REF_SET_OTICKET(ref, job_override_tickets);

   DEXIT;
   return job_override_tickets;
}

static double calc_pjob_override_tickets_shared( sge_ref_t *ref) {
   double job_override_tickets = 0;
   double otickets, job_cnt;

   DENTER(TOP_LAYER, "calc_job_override_tickets");

   /*-------------------------------------------------------
    * job.override_tickets = user.override_tickets +
    *                        project.override_tickets +
    *                        department.override_tickets +
    *                        job.override_tickets;
    *-------------------------------------------------------*/

   if (ref->user) {
      if ((otickets = lGetUlong(ref->user, UU_oticket)) > 0) { 
         job_cnt = lGetUlong(ref->user, UU_job_cnt) + 1;
         job_override_tickets += (otickets / job_cnt);
      }   
   }

   if (ref->project) {
      if ((otickets = lGetUlong(ref->project, PR_oticket)) > 0) {
         job_cnt = lGetUlong(ref->project, PR_job_cnt) + 1;      
         job_override_tickets += (otickets / job_cnt);
      }   
   }

   if (ref->dept) {
      if ((otickets = lGetUlong(ref->dept, US_oticket)) > 0) {
         job_cnt = lGetUlong(ref->dept, US_job_cnt) + 1;
         job_override_tickets += (otickets / job_cnt);
      }   
   }
 
   job_override_tickets += lGetUlong(ref->job, JB_override_tickets);

   REF_SET_OTICKET(ref, job_override_tickets);

   DEXIT;
   return job_override_tickets;
}

/*--------------------------------------------------------------------
 * calc_job_tickets - calculates the total number of tickets for
 * the specified job
 *--------------------------------------------------------------------*/

static double
calc_job_tickets ( sge_ref_t *ref )
{
   lList *granted;
   lListElem *job = ref->job,
             *ja_task = ref->ja_task;
   double job_tickets;
   lListElem *pe, *granted_el;
   const char *pe_str;

   /*-------------------------------------------------------------
    * Sum up all tickets for job
    *-------------------------------------------------------------*/

   job_tickets = REF_GET_STICKET(ref) + REF_GET_FTICKET(ref) +
                 REF_GET_OTICKET(ref);

   REF_SET_TICKET(ref, job_tickets);

   /* for PE slave-controlled jobs, set tickets for each granted queue */

   if (ja_task && (pe_str=lGetString(ja_task, JAT_granted_pe))
         && (pe=pe_list_locate(all_lists->pe_list, pe_str))
         && lGetBool(pe, PE_control_slaves)
         && (granted=lGetList(ja_task, JAT_granted_destin_identifier_list))) {

      double job_tickets_per_slot, job_otickets_per_slot,
             job_ftickets_per_slot, job_stickets_per_slot, nslots, active_nslots;

      /* Here we get the total number of slots granted on all queues (nslots) and the
         total number of slots granted on all queues with active subtasks or
         on which no subtasks have started yet (active_nslots). */

      nslots = nslots_granted(granted, NULL);
      active_nslots = active_nslots_granted(job, granted, NULL);

      /* If there are some active_nslots, then calculate the number of tickets
         per slot based on the active_nslots */

      if (active_nslots > 0)
         nslots = active_nslots;

      if (nslots > 0) {
         job_stickets_per_slot = REF_GET_STICKET(ref)/nslots;
      } else {
         job_stickets_per_slot = 0;
      }

      for_each(granted_el, granted) {
         double slots;

         /* Only give tickets to queues with active sub-tasks or sub-tasks
            which have not started yet. */
            
         if (active_nslots > 0 && !active_subtasks(job, lGetString(granted_el, JG_qname)))
            slots = 0;
         else
            slots = lGetUlong(granted_el, JG_slots);
         
         if (nslots > 0) {
            job_ftickets_per_slot = (double)(REF_GET_FTICKET(ref))/nslots;
            job_otickets_per_slot = (double)(REF_GET_OTICKET(ref))/nslots;
            job_tickets_per_slot = job_stickets_per_slot + job_ftickets_per_slot + job_otickets_per_slot;
         } else {
            job_ftickets_per_slot = 0;
            job_otickets_per_slot = 0;
            job_tickets_per_slot = 0;
         }

         lSetDouble(granted_el, JG_fticket, job_ftickets_per_slot*slots + lGetDouble(granted_el, JG_jcfticket));
         lSetDouble(granted_el, JG_oticket, job_otickets_per_slot*slots + lGetDouble(granted_el, JG_jcoticket));
         lSetDouble(granted_el, JG_sticket, job_stickets_per_slot*slots);
         lSetDouble(granted_el, JG_ticket, job_tickets_per_slot*slots +
                   lGetDouble(granted_el, JG_jcoticket) + lGetDouble(granted_el, JG_jcfticket));

      }
   }

   return job_tickets;
}


/*--------------------------------------------------------------------
 * sge_clear_job - clear tickets for job
 *--------------------------------------------------------------------*/

void sge_clear_job( lListElem *job, bool is_clear_all) {
   lListElem *ja_task;
   
   if (is_clear_all) {
      lSetDouble(job, JB_nppri,0.0);
      lSetDouble(job, JB_urg,0.0);
      lSetDouble(job, JB_nurg,0.0);
      lSetDouble(job, JB_dlcontr,0.0);
      lSetDouble(job, JB_wtcontr,0.0);
      lSetDouble(job, JB_rrcontr,0.0);
      
      for_each(ja_task, lGetList(job, JB_ja_template)) {
         sge_clear_ja_task(ja_task); 
         lSetUlong(ja_task, JAT_task_number, 1);
      }   
   }

   for_each(ja_task, lGetList(job, JB_ja_tasks))
      sge_clear_ja_task(ja_task);
}

/*--------------------------------------------------------------------
 * sge_clear_ja_task - clear tickets for job task
 *--------------------------------------------------------------------*/

static void
sge_clear_ja_task( lListElem *ja_task )
{
   lListElem *granted_el;
   lSetDouble(ja_task, JAT_prio, 0);
   lSetDouble(ja_task, JAT_ntix, 0);
   lSetDouble(ja_task, JAT_tix, 0);
   lSetDouble(ja_task, JAT_oticket, 0);
   lSetDouble(ja_task, JAT_fticket, 0);
   lSetDouble(ja_task, JAT_sticket, 0);
   lSetDouble(ja_task, JAT_share, 0);
   for_each(granted_el, lGetList(ja_task, JAT_granted_destin_identifier_list)) {
      lSetDouble(granted_el, JG_ticket, 0);
      lSetDouble(granted_el, JG_oticket, 0);
      lSetDouble(granted_el, JG_fticket, 0);
      lSetDouble(granted_el, JG_sticket, 0);
      lSetDouble(granted_el, JG_jcoticket, 0);
      lSetDouble(granted_el, JG_jcfticket, 0);
   }
}

/****** sgeee/calc_intern_pending_job_functional_tickets() *********************
*  NAME
*     calc_intern_pending_job_functional_tickets() -- calc ftix for pending jobs 
*
*  SYNOPSIS
*     void calc_intern_pending_job_functional_tickets(sge_fcategory_t *current, 
*                                    double sum_of_user_functional_shares,
*                                    double sum_of_project_functional_shares,
*                                    double sum_of_department_functional_shares,
*                                    double sum_of_job_functional_shares, 
*                                    double total_functional_tickets,
*                                    double weight[]) 
*
*  FUNCTION
*     This is an optimized and incomplete version of calc_pending_job_functional_tickets.
*     It is good enough to get the order right within the inner loop of the ftix
*     calculation. 
*
*  INPUTS
*     sge_fcategory_t *current                   - current fcategory 
*     double sum_of_user_functional_shares       
*     double sum_of_project_functional_shares     
*     double sum_of_department_functional_shares   
*     double sum_of_job_functional_shares          
*     double total_functional_tickets              
*     double weight[]                            - destribution of the shares to each other  
*
*
*  NOTES
*     be carefull using it 
*
*  BUGS
*     ??? 
*
*******************************************************************************/
static void calc_intern_pending_job_functional_tickets(
                                    lListElem *current,
                                    double sum_of_user_functional_shares,
                                    double sum_of_project_functional_shares,
                                    double sum_of_department_functional_shares,
                                    double sum_of_job_functional_shares,
                                    double total_functional_tickets,
                                    double weight[],
                                    bool share_functional_shares)
{
   sge_ref_list_t *tmp = (lGetRef(current, FCAT_jobrelated_ticket_first)); 
   sge_ref_t *ref = tmp->ref; 

   /*-------------------------------------------------------------
    * Sum user functional shares
    *-------------------------------------------------------------*/

   if (ref->user) {
      ref->user_fshare = lGetUlong(current, FCAT_user_share);
      if(share_functional_shares) {
         ref->user_fshare /= lGetUlong(ref->user, UU_job_cnt) +1 ; 
      }   
      else { 
         sum_of_user_functional_shares += ref->user_fshare;
      }   
   }

   /*-------------------------------------------------------------
    * Sum project functional shares
    *-------------------------------------------------------------*/

   if (ref->project) { 
      ref->project_fshare = lGetUlong(current, FCAT_project_share);
      if(share_functional_shares) {
         ref->project_fshare /= (lGetUlong(ref->project, PR_job_cnt) +1);
      }   
      else {     
         sum_of_project_functional_shares += ref->project_fshare;
      }   
   }


   /*-------------------------------------------------------------
    * Sum department functional shares
    *-------------------------------------------------------------*/

   if (ref->dept) { 
      ref->dept_fshare = lGetUlong(current, FCAT_dept_share);
      if(share_functional_shares) {
            ref->dept_fshare /= lGetUlong(ref->dept, US_job_cnt) +1;
      }      
      else { 
         sum_of_department_functional_shares += ref->dept_fshare;
      }   
   }

   /*-------------------------------------------------------------
    * Sum job functional shares
    *-------------------------------------------------------------*/

   REF_SET_FSHARE(ref, lGetUlong(current, FCAT_job_share));
   sum_of_job_functional_shares += REF_GET_FSHARE(ref);

   calc_job_functional_tickets_pass2(ref,
                                     sum_of_user_functional_shares,
                                     sum_of_project_functional_shares,
                                     sum_of_department_functional_shares,
                                     sum_of_job_functional_shares,
                                     total_functional_tickets,
                                     weight);

   return;
}

static void calc_pending_job_functional_tickets(sge_ref_t *ref,
                                    double *sum_of_user_functional_shares,
                                    double *sum_of_project_functional_shares,
                                    double *sum_of_department_functional_shares,
                                    double *sum_of_job_functional_shares,
                                    double total_functional_tickets,
                                    double weight[],
                                    int shared)
{
   /*
    * If the functional shares of a given object are shared between jobs
    * (schedd_param SHARE_FUNCTIONAL_SHARES=true), then we need to reduce
    * the total number of shares for each category to include the object
    * shares only one time. If the object reference count is greater than
    * one, the object's shares were already included in the total before
    * calc_job_functional_tickets_pass1 was called, so we subtract them
    * out here.
   */
   calc_job_functional_tickets_pass1(ref,
                                     sum_of_user_functional_shares,
                                     sum_of_project_functional_shares,
                                     sum_of_department_functional_shares,
                                     sum_of_job_functional_shares,
                                     shared,
                                     !shared);

   calc_job_functional_tickets_pass2(ref,
                                     *sum_of_user_functional_shares,
                                     *sum_of_project_functional_shares,
                                     *sum_of_department_functional_shares,
                                     *sum_of_job_functional_shares,
                                     total_functional_tickets,
                                     weight);
   return;
}

/*--------------------------------------------------------------------
 * sge_calc_tickets - calculate proportional shares in terms of tickets
 * for all active jobs.
 *
 * queued_jobs should contain all pending jobs to be scheduled
 * running_jobs should contain all currently executing jobs
 * finished_jobs should contain all completed jobs
 *--------------------------------------------------------------------*/

static int
sge_calc_tickets( scheduler_all_data_t *lists,
                  lList *running_jobs,
                  lList *finished_jobs,
                  lList *queued_jobs,
                  int do_usage, 
                  double *max_tickets)
{
   double sum_of_user_functional_shares = 0,
          sum_of_project_functional_shares = 0,
          sum_of_department_functional_shares = 0,
          sum_of_job_functional_shares = 0,
          sum_of_active_tickets = 0,
          sum_of_pending_tickets = 0,
          sum_of_active_override_tickets = 0;

   u_long curr_time;
   int num_jobs, num_queued_jobs, job_ndx;

   u_long32 num_unenrolled_tasks = 0;

   sge_ref_t *job_ref = NULL;

   lListElem *job, *qep, *root = NULL;

   double prof_init=0, prof_pass0=0, prof_pass1=0, prof_pass2=0, prof_calc=0; 

   double total_share_tree_tickets = sconf_get_weight_tickets_share();
   double total_functional_tickets = sconf_get_weight_tickets_functional();
   
   bool share_functional_shares = sconf_get_share_functional_shares();
   u_long32 max_pending_tasks_per_job = sconf_get_max_pending_tasks_per_job();

   lList *decay_list = NULL;

   u_long32 free_qslots = 0;

   DENTER(TOP_LAYER, "sge_calc_tickets");
  
   PROF_START_MEASUREMENT(SGE_PROF_SCHEDLIB4);

   all_lists = lists;

   curr_time = sge_get_gmt();

   sge_scheduling_run++;
   { 
      lList *halflife_decay_list = sconf_get_halflife_decay_list();

      if (halflife_decay_list != NULL) {
         lListElem *ep = NULL;
         lListElem *u = NULL;
         double decay_rate, decay_constant;
         
         for_each(ep, halflife_decay_list) {
            calculate_decay_constant(lGetDouble(ep, UA_value),
                                    &decay_rate, &decay_constant);
            u = lAddElemStr(&decay_list, UA_name, lGetString(ep, UA_name),
                           UA_Type); 
            lSetDouble(u, UA_value, decay_constant);
         }
      } 
      else {
         lListElem *u = NULL;
         double decay_rate, decay_constant;

         calculate_decay_constant(-1, &decay_rate, &decay_constant);
         u = lAddElemStr(&decay_list, UA_name, "finished_jobs", UA_Type); 
         lSetDouble(u, UA_value, decay_constant);
      }
       lFreeList(&halflife_decay_list);
   }

   /*-------------------------------------------------------------
    * Decay usage for all users and projects if halflife changes
    *-------------------------------------------------------------*/

   if (do_usage && sconf_is() && halflife != sconf_get_halftime()) {
      lListElem *userprj;
      int oldhalflife = halflife;
      halflife = sconf_get_halftime(); 
      /* decay up till now based on old half life (unless it's zero),
         all future decay will be based on new halflife */
      if (oldhalflife == 0) {
         calculate_default_decay_constant(halflife);
      }   
      else {
         calculate_default_decay_constant(oldhalflife);
      }   
      for_each(userprj, lists->user_list) {
         decay_userprj_usage(userprj, true, decay_list, sge_scheduling_run, curr_time);
      }
      for_each(userprj, lists->project_list) {
         decay_userprj_usage(userprj, false, decay_list, sge_scheduling_run, curr_time);
      }
   } 
   else {
      calculate_default_decay_constant(sconf_get_halftime());
   }

   /*-------------------------------------------------------------
    * Init job_ref_count in each share tree node to zero
    *-------------------------------------------------------------*/

   if ((lists->share_tree)) {
      if ((root = lFirst(lists->share_tree))) {
         sge_init_share_tree_nodes(root);
         set_share_tree_project_flags(lists->project_list, root);
      }
   }

   for_each(qep, lists->queue_list) {
      free_qslots += MAX(0, ((int)lGetUlong(qep, QU_job_slots)) - qinstance_slots_used(qep));
   }

   /*-----------------------------------------------------------------
    * Create and build job reference array.  The job reference array
    * contains all the references of the jobs to the various objects
    * needed for scheduling. It exists so that the ticket calculation
    * function only has to look up the various objects once.
    *-----------------------------------------------------------------*/
   
   num_jobs = num_queued_jobs = 0;
   if (queued_jobs) {
      for_each(job, queued_jobs) {
         u_long32 max = MIN(max_pending_tasks_per_job, free_qslots+1);
         u_long32 num_unenrolled_tasks_for_job = 0;
         lListElem *ja_task;
         int task_cnt = 0;

         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            if (++task_cnt > max) {
               break;
            }
            if (job_is_active(ja_task)) {
               num_queued_jobs++;
               num_jobs++;
            }
         }
         if (!(task_cnt > max)) {
            int space = max - task_cnt;
            int tasks = job_get_not_enrolled_ja_tasks(job);

            num_unenrolled_tasks_for_job = MIN(space, tasks);
            num_unenrolled_tasks += num_unenrolled_tasks_for_job;
            num_queued_jobs += num_unenrolled_tasks_for_job;
            num_jobs += num_unenrolled_tasks_for_job;
         }        
      }
   }
   if (running_jobs) {
      for_each(job, running_jobs) {
         lListElem *ja_task;
         for_each(ja_task, lGetList(job, JB_ja_tasks))
            if (job_is_active(ja_task))
               num_jobs++;
      }
   }
   if (num_jobs > 0) {
      job_ref = (sge_ref_t *)malloc(num_jobs * sizeof(sge_ref_t));
      memset(job_ref, 0, num_jobs * sizeof(sge_ref_t));
   }
   if (num_unenrolled_tasks > 0) {
      task_ref_initialize_table(num_unenrolled_tasks);
   }

   /*-----------------------------------------------------------------
    * Note: use of the the job_ref_t array assumes that no jobs
    * will be removed from the job lists and the job list order will
    * be maintained during the execution of sge_calc_tickets.
    *-----------------------------------------------------------------*/

   job_ndx = 0;

   if (running_jobs) {
      for_each(job, running_jobs) {
         lListElem *ja_task;

         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            if (!job_is_active(ja_task)) {
               sge_clear_ja_task(ja_task);
            } else {
               sge_set_job_refs(job, ja_task, &job_ref[job_ndx], NULL, lists, 0);
               if (job_ref[job_ndx].node) {
                  lSetUlong(job_ref[job_ndx].node, STN_job_ref_count,
                            lGetUlong(job_ref[job_ndx].node, STN_job_ref_count)+1);
                  lSetUlong(job_ref[job_ndx].node, STN_active_job_ref_count,
                            lGetUlong(job_ref[job_ndx].node, STN_active_job_ref_count)+1);
               }
               job_ndx++;
            }
         }
      }
   }

   if (queued_jobs) {
      u_long32 ja_task_ndx = 0;

      for_each(job, queued_jobs) {
         lListElem *ja_task = NULL;
         lListElem *range = NULL;
         lList *range_list = NULL;  
         u_long32 id;
         int task_cnt = 0;

         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            sge_ref_t *jref = &job_ref[job_ndx];

            if (++task_cnt > MIN(max_pending_tasks_per_job, free_qslots+1) ||
                !job_is_active(ja_task)) {
               sge_clear_ja_task(ja_task);
            } else {
               sge_set_job_refs(job, ja_task, jref, NULL, lists, 1);
               if (jref->node)
                  lSetUlong(jref->node, STN_job_ref_count,
                            lGetUlong(jref->node, STN_job_ref_count)+1);
               job_ndx++;
            }
         }

         range_list = lGetList(job, JB_ja_n_h_ids); 
         for_each(range, range_list) {
            for(id = lGetUlong(range, RN_min);
                id <= lGetUlong(range, RN_max);
                id += lGetUlong(range, RN_step)) {  
               sge_ref_t *jref = &job_ref[job_ndx];
               sge_task_ref_t *tref = task_ref_get_entry(ja_task_ndx);

               if (++task_cnt > MIN(max_pending_tasks_per_job, 
                                    free_qslots+1)) {
                  break;
               } 
               ja_task = job_get_ja_task_template_pending(job, id);
               sge_set_job_refs(job, ja_task, jref, tref, lists, 1); 
               if (jref->node) {
                  lSetUlong(jref->node, STN_job_ref_count,
                            lGetUlong(jref->node, STN_job_ref_count)+1);
               }
               ja_task_ndx++;
               job_ndx++;
            }
         }
      }
   }

   num_jobs = job_ndx;

   /*-------------------------------------------------------------
    * Calculate the m_shares in the share tree.  The m_share
    * is a hierarchichal share value used in the "classic
    * scheduling" mode. It is calculated by subdividing the shares
    * at each active level in the share tree.
    *-------------------------------------------------------------*/
     
   if (root) {
      update_job_ref_count(root);
      update_active_job_ref_count(root);
      lSetDouble(root, STN_m_share, 1.0);
      calculate_m_shares(root);
   }

   /*-----------------------------------------------------------------
    * Handle finished jobs. We add the finished job usage to the
    * user and project objects and then we delete the debited job
    * usage from the associated user/project object.  The debited
    * job usage is a usage list which is maintained in the user/project
    * object to indicate how much of the job's usage has been added
    * to the user/project usage so far.
    * We also add a usage value called finished_jobs to the job usage
    * so it will be summed and decayed in the user and project usage.
    * If the finished_jobs usage value already exists, then we know
    * we already handled this finished job, but the qmaster hasn't
    * been updated yet.
    *-----------------------------------------------------------------*/

   if (do_usage && finished_jobs) {
      for_each(job, finished_jobs) {
         /* Set finished job flag on each task */
         lListElem *ja_task;
         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            if (lGetElemStr(lGetList(ja_task, JAT_scaled_usage_list),
                            UA_name, "finished_jobs") == NULL) {
               lListElem *u;
               if ((u=lAddSubStr(ja_task, UA_name, "finished_jobs",
                                 JAT_scaled_usage_list, UA_Type)))
                  lSetDouble(u, UA_value, 1.0);

               DPRINTF(("DDJU (0) "sge_u32"."sge_u32"\n",
                  lGetUlong(job, JB_job_number),
                  lGetUlong(ja_task, JAT_task_number)));
            }
         }

         /* Find job owners etc. */
         sge_ref_t jref;
         memset(&jref, 0, sizeof(jref));
         sge_set_job_refs(job, lFirst(lGetList(job, JB_ja_tasks)), &jref, NULL, lists, 0);

         /* Spool usage updates (user/project running and final totals) */
         lListElem *running_job;
         if (lGetList(job, JB_ja_structure) &&
             (running_job = lGetElemUlong(running_jobs, JB_job_number, lGetUlong(job, JB_job_number)))) {

            /* Task array with completed and running tasks */
            decay_and_sum_usage(&jref, decay_list, running_job, sge_scheduling_run, curr_time);
         } else {
            /* Non-task array or no other tasks running */
            decay_and_sum_usage(&jref, decay_list, NULL, sge_scheduling_run, curr_time);
            delete_debited_job_usage(&jref, sge_scheduling_run);
         }
      }
   } else {
      DPRINTF(("\n"));
      DPRINTF(("no DDJU: do_usage: %d finished_jobs %d\n",
         do_usage, (finished_jobs!=NULL)));
      DPRINTF(("\n"));
   }     
   
   PROF_STOP_MEASUREMENT(SGE_PROF_SCHEDLIB4);
   prof_init = prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4, false, NULL);
   PROF_START_MEASUREMENT(SGE_PROF_SCHEDLIB4); 

   /*-----------------------------------------------------------------
    * PASS 0
    *-----------------------------------------------------------------*/

   DPRINTF(("=====================[Pass 0]======================\n"));

   for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {

      sge_set_job_cnts(&job_ref[job_ndx], job_ref[job_ndx].queued);

      /*-------------------------------------------------------------
       * Handle usage
       *-------------------------------------------------------------*/

      /* Spool usage updates (user/project running totals) */
      if (do_usage)
         decay_and_sum_usage(&job_ref[job_ndx], decay_list, NULL, sge_scheduling_run, curr_time);

      combine_usage(&job_ref[job_ndx]);
   }

   if (root)
      sge_calc_sharetree_targets(root, lists, decay_list,
                                 curr_time, sge_scheduling_run);

   PROF_STOP_MEASUREMENT(SGE_PROF_SCHEDLIB4);
   prof_pass0 = prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4,false, NULL);
   PROF_START_MEASUREMENT(SGE_PROF_SCHEDLIB4); 

   /*-----------------------------------------------------------------
    * PASS 1
    *-----------------------------------------------------------------*/

   DPRINTF(("=====================[Pass 1]======================\n"));

   for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {
      if (job_ref[job_ndx].queued)
         break;

      if (total_share_tree_tickets > 0)
         calc_job_share_tree_tickets_pass1(&job_ref[job_ndx]);

      if (total_functional_tickets > 0)
         calc_job_functional_tickets_pass1(&job_ref[job_ndx],
                                          &sum_of_user_functional_shares,
                                          &sum_of_project_functional_shares,
                                          &sum_of_department_functional_shares,
                                          &sum_of_job_functional_shares,
                                          share_functional_shares, 1);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SCHEDLIB4);
   prof_pass1= prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4,false, NULL);
   PROF_START_MEASUREMENT(SGE_PROF_SCHEDLIB4); 

   /*-----------------------------------------------------------------
    * PASS 2
    *-----------------------------------------------------------------*/

   DPRINTF(("=====================[Pass 2]======================\n"));
   { 
      double weight[k_last];
      bool share_override_tickets = sconf_get_share_override_tickets();

      if(total_functional_tickets > 0) {
         get_functional_weighting_parameters(sum_of_user_functional_shares,
                                       sum_of_project_functional_shares,
                                       sum_of_department_functional_shares,
                                       sum_of_job_functional_shares,
                                       weight);
      }                                 

      for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {

         /* stop, when teh first pending is found */
         if (job_ref[job_ndx].queued) {
            break;
         }

         job = job_ref[job_ndx].job;

         if (total_share_tree_tickets > 0) {
            calc_job_share_tree_tickets_pass2(&job_ref[job_ndx],
                                           total_share_tree_tickets);
         }                                  

         if (total_functional_tickets > 0) {
            calc_job_functional_tickets_pass2(&job_ref[job_ndx],
                                           sum_of_user_functional_shares,
                                           sum_of_project_functional_shares,
                                           sum_of_department_functional_shares,
                                           sum_of_job_functional_shares,
                                           total_functional_tickets,
                                           weight);
         }                                  

         sum_of_active_override_tickets +=
                  calc_job_override_tickets(&job_ref[job_ndx], share_override_tickets);

         sum_of_active_tickets += calc_job_tickets(&job_ref[job_ndx]);

      }
   }

   /* set scheduler configuration information to go back to GUI */
   if (sconf_is()) {
      sconf_set_weight_tickets_override(sum_of_active_override_tickets);
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SCHEDLIB4);
   prof_pass2 = prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4,false, NULL);
   PROF_START_MEASUREMENT(SGE_PROF_SCHEDLIB4); 

   /*-----------------------------------------------------------------
    * NEW PENDING JOB TICKET CALCULATIONS
    *-----------------------------------------------------------------*/

   if (queued_jobs) {
      lList *sorted_job_node_list;

      policy_hierarchy_t hierarchy[POLICY_VALUES];
      int policy_ndx;

      sconf_ph_fill_array(hierarchy);
      sconf_ph_print_array(hierarchy);  

      for(policy_ndx = 0; 
          policy_ndx < POLICY_VALUES;
          policy_ndx++) {

         switch(hierarchy[policy_ndx].policy) {

         case SHARE_TREE_POLICY:
         DPRINTF(("calc share tree pending tickets: %.3f\n", total_share_tree_tickets));
      /*-----------------------------------------------------------------
       * Calculate pending share tree tickets
       *
       *    The share tree tickets for pending jobs are calculated
       *    by adding each job to the share tree as leaf nodes
       *    under the associated user/project node. The jobs are
       *    then sorted based on their respective short term
       *    entitlements to ensure that the overall goals of the
       *    share tree are met.
       *
       *-----------------------------------------------------------------*/

      if (total_share_tree_tickets > 0 && root) {

         for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {
            sge_ref_t *jref = &job_ref[job_ndx];
            lListElem *node = jref->node;
            if (node) {
               /* add each job to the share tree as a temporary sibling node */
               char tmpstr[64];
               lListElem *child;

               job = jref->job;
               sprintf(tmpstr, sge_u32"."sge_u32, lGetUlong(job, JB_job_number),
                      REF_GET_JA_TASK_NUMBER(jref));
               child = lAddSubStr(node, STN_name, tmpstr, STN_children, STN_Type);
               lSetUlong(child, STN_jobid, lGetUlong(job, JB_job_number));
               lSetUlong(child, STN_taskid, REF_GET_JA_TASK_NUMBER(jref));
               lSetUlong(child, STN_queued, jref->queued);
               lSetUlong(child, STN_temp, 1);
               /* save the job reference, so we can refer to it later to set job fields */
               lSetUlong(child, STN_ref, job_ndx+1);
               if (hierarchy[policy_ndx].dependent) {
                  /* set the sort value based on tickets of higher level policy */
                  lSetDouble(child, STN_tickets, jref->tickets);
                  lSetDouble(child, STN_sort,
                             jref->tickets + (0.01 * (double)lGetUlong(job, JB_jobshare)));
               } else
                  /* set the sort value based on the priority of the job */
                  lSetDouble(child, STN_sort, (double)lGetUlong(job, JB_jobshare));
            }
         }

         if ((sorted_job_node_list = sge_sort_job_nodes(root, root, total_share_tree_tickets, job_ref))) {
            lListElem *job_node;

            /* 
             * set share tree tickets of each pending job 
             * based on the returned sorted node list 
             */
            for_each(job_node, sorted_job_node_list) {
               sge_ref_t *jref = &job_ref[lGetUlong(job_node, STN_ref)-1];
               if (jref->queued) {
                  REF_SET_STICKET(jref, 
                     lGetDouble(job_node, STN_shr) * total_share_tree_tickets);
                  if (hierarchy[policy_ndx].dependent)
                     jref->tickets += REF_GET_STICKET(jref);
               }
            }
            lFreeList(&sorted_job_node_list);
         }
      }

            break;

         case FUNCTIONAL_POLICY:

      /*-----------------------------------------------------------------
       * Calculate pending functional tickets 
       *        
       *    We use a somewhat brute force method where we order the pending
       *    jobs based on which job would get the largest number of 
       *    functional tickets if it were to run next. Because of the
       *    overhead, we limit the number of pending jobs that we will
       *    order based on the configurable schedd parameter called
       *    max_functional_jobs_to_schedule. The default is 100.
       *             
       *    The algorithm works like:
       *    - order all pending jobs by functional categories. A functional
       *      category consits of: user_shares, dept_shares, project_shares,
       *      and job_shares.
       *    - For all max_functional_jobs_to_schedule compute the functional
       *      ticket for the first job of each fcategory. Our currently used
       *      algorithm computes for all jobs under the same fcategory the
       *      same functional tickets. Therefore we have only to compute
       *      the first job of each fcategory. The job list under each category
       *      has to be sorted by time and previous computed tickets.
       *      
       *    - When the job of one functional category is found, increase the
       *      counter for running jobs in the user, dept, proj, and other structures.
       *      Afterwards, compute the real functional tickets.
       *
       *      When this is done, undo all the changes to the different structures 
       *      (running job counters) and free the fcategory data-structure.
       *-----------------------------------------------------------------*/
      DPRINTF(("calc funktional tickets %.3f\n", total_functional_tickets));
      if (total_functional_tickets > 0 && num_queued_jobs > 0) {
         sge_ref_t **sort_list=NULL;
         lList *fcategories = NULL;

         u_long32 i=0;
         
         double weight[k_last];

         sge_ref_list_t *ref_array = NULL;
         double pending_user_fshares = sum_of_user_functional_shares;
         double pending_proj_fshares = sum_of_project_functional_shares;
         double pending_dept_fshares = sum_of_department_functional_shares;
         double pending_job_fshares = sum_of_job_functional_shares;

         u_long32 max =   sconf_get_max_functional_jobs_to_schedule();
         u_long32 pjobs = build_functional_categories(job_ref, num_jobs, &fcategories, 
                                     &ref_array, hierarchy[policy_ndx].dependent, 
                                     JB_jobshare, UU_fshare, PR_fshare, US_fshare);


         max = MIN(max, pjobs); 
                       
         if (max > 0) { 
            sort_list = malloc(max * sizeof(sge_ref_t *));
           
            if(sort_list == NULL){
               /* error message to come */
               
               sge_free(&sort_list);
               sge_free(&job_ref);
               lFreeList(&decay_list);

               if (fcategories != NULL) {
                  free_fcategories(&fcategories, &ref_array);
               }
               
               DEXIT;
               return sge_scheduling_run;
            }

            get_functional_weighting_parameters(1, 1, 1, 1, weight);

            /* Loop through all the jobs calculating the functional tickets and
               find the job with the most functional tickets.  Move it to the
               top of the list and start the process all over again with the
               remaining jobs. */

            for(i=0; i<max; i++) {
               double ftickets, max_ftickets=-1;
               u_long32 jid, save_jid=0, save_tid=0;
	            u_long32 submission_time = 0, save_submission_time = 0;
               lListElem *current = NULL;
               lListElem *max_current = NULL;

               /* loop over all first jobs from the different categories */
               for_each(current, fcategories) { 
                  sge_ref_list_t *tmp = (lGetRef(current, FCAT_jobrelated_ticket_first)); 
                  sge_ref_t *jref = tmp->ref;
                  double user_fshares = pending_user_fshares + 0.001;
                  double proj_fshares = pending_proj_fshares + 0.001;
                  double dept_fshares = pending_dept_fshares + 0.001;
                  double job_fshares = pending_job_fshares + 0.001;

                  /* calc the tickets */

                  calc_intern_pending_job_functional_tickets(current,
                                                      user_fshares,
                                                      proj_fshares,
                                                      dept_fshares,
                                                      job_fshares,
                                                      total_functional_tickets,
                                                      weight,
                                                      share_functional_shares);

                  ftickets = REF_GET_FTICKET(jref);

                  if (hierarchy[policy_ndx].dependent)
                     ftickets += jref->tickets;

                  /* controll, if the current job has the most tickets */
                  if(ftickets >= max_ftickets){
                     jid = lGetUlong(jref->job, JB_job_number);
                     submission_time = lGetUlong(jref->job, JB_submission_time);
                     if (max_current == NULL ||
                           (ftickets > max_ftickets) ||
			   (submission_time < save_submission_time) ||
			   (submission_time == save_submission_time && jid < save_jid) ||
                           (jid == save_jid && REF_GET_JA_TASK_NUMBER(jref) < save_tid)) {
                        max_ftickets = ftickets;
                        save_jid = lGetUlong(jref->job, JB_job_number);
                        save_submission_time = lGetUlong(jref->job, JB_submission_time);
                        save_tid = REF_GET_JA_TASK_NUMBER(jref);
                        sort_list[i] = jref;
                        max_current = current;
                     }
                  }
               }

               /* - set the results from first entry of the winning category to the second 
                  - remove the winning element
                  - if it was the last element, remove the fcategory*/
               if (max_current != NULL) {
                  sge_ref_list_t *tmp =lGetRef(max_current, FCAT_jobrelated_ticket_first);  
                  copy_ftickets (tmp, tmp->next);

                  /* switch to next elem */
                  lSetRef(max_current, FCAT_jobrelated_ticket_first, tmp->next);  
                 
                  /* remove category */
                  if (lGetRef(max_current, FCAT_jobrelated_ticket_first) == NULL) {
                     lSetRef(max_current, FCAT_jobrelated_ticket_last,  NULL);
                     lSetRef(max_current, FCAT_user, NULL);
                     lSetRef(max_current, FCAT_dept, NULL);
                     lSetRef(max_current, FCAT_project, NULL);

                     lRemoveElem(fcategories, &max_current);
                  }

               /* This is the job with the most functional tickets, consider it active */
               sge_set_job_cnts(sort_list[i], 0);

               /* recompute the results afterwards for the winnnig entry */
               calc_pending_job_functional_tickets(sort_list[i],
                                                   &pending_user_fshares,
                                                   &pending_proj_fshares,
                                                   &pending_dept_fshares,
                                                   &pending_job_fshares,
                                                   total_functional_tickets,
                                                   weight,
                                                   share_functional_shares);
               }
               else { /* we reached an end */
                  break;
               }
            }

            /* Reset the pending jobs to inactive */
            {
               int depend = hierarchy[policy_ndx].dependent;
               for(job_ndx=0; job_ndx < max; job_ndx++) {
                  sge_ref_t *jref = sort_list[job_ndx];
                  sge_unset_job_cnts(jref, 0);  

                  /* sum up the tickets, if there are dependencies */             
                  if (depend) {
                     jref->tickets += REF_GET_FTICKET(jref);
                  }   
               }

               destribute_ftickets(fcategories, depend);
            }
         }

         /* free the allocated memory */
         sge_free(&sort_list);
         free_fcategories(&fcategories, &ref_array);

      }

            break;

         case OVERRIDE_POLICY:
               /*-----------------------------------------------------------------
                * Calculate the pending override tickets
                *-----------------------------------------------------------------*/
               {
                  bool share_override_tickets = sconf_get_share_override_tickets();
                  if (share_override_tickets) {
                     calculate_pending_shared_override_tickets(job_ref, num_jobs, hierarchy[policy_ndx].dependent );
                  }
                  else {
                     for(job_ndx=0; job_ndx<num_jobs; job_ndx++) {
                        sge_ref_t *jref = &job_ref[job_ndx];
                        if (jref->queued) {
                           calc_job_override_tickets(jref, share_override_tickets);
                           if (hierarchy[policy_ndx].dependent) {
                              jref->tickets += REF_GET_OTICKET(jref);
                           }   
                        }
                     } /* for */
                  }
               } /* case */

            break;

         default:
            break;
         }
      } /* switch */

      /*-----------------------------------------------------------------
       * Combine the ticket values into the total tickets for the job
       *-----------------------------------------------------------------*/

      for (job_ndx=0; job_ndx<num_jobs; job_ndx++) {
         calc_job_tickets(&job_ref[job_ndx]);
      }   

      /* calculate the ticket % of each job */

      sum_of_active_tickets = 0;
      sum_of_pending_tickets = 0;
      for (job_ndx=0; job_ndx<num_jobs; job_ndx++) {
         sge_ref_t *jref = &job_ref[job_ndx];
         double tickets = REF_GET_TICKET(jref);
         
         if (*max_tickets < tickets) {
            *max_tickets = tickets;
         }
         
         if (jref->queued) {
            sum_of_pending_tickets += tickets;
         } else {
            sum_of_active_tickets += tickets;
         }   
      }

      for (job_ndx=0; job_ndx<num_jobs; job_ndx++) {
         sge_ref_t *jref = &job_ref[job_ndx];
         double ticket_sum = jref->queued ? sum_of_pending_tickets : sum_of_active_tickets;

         if (REF_GET_TICKET(jref) > 0) {
            REF_SET_SHARE(jref, REF_GET_TICKET(jref) / ticket_sum);
         } 
         else {
            REF_SET_SHARE(jref, 0);
         }
      }

   } 
   else {
      for (job_ndx=0; job_ndx<num_jobs; job_ndx++) {
         sge_ref_t *jref = &job_ref[job_ndx];
         double tickets = REF_GET_TICKET(jref);

         if (*max_tickets < tickets) {
            *max_tickets = tickets;
         }

         job = jref->job;

         if (tickets > 0) {
            REF_SET_SHARE(jref, tickets / sum_of_active_tickets);
         } 
         else {
            REF_SET_SHARE(jref, 0);
         }

      }

   }

   /* update share tree node for finished jobs */

   /* NOTE: what purpose does this serve? */

   if (do_usage && finished_jobs) {
      for_each(job, finished_jobs) {
         sge_ref_t jref;
         lListElem *ja_task;

         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            memset(&jref, 0, sizeof(jref));
            sge_set_job_refs(job, ja_task, &jref, NULL, lists, 0);
            if (jref.node) {
               if (sge_scheduling_run != lGetUlong(jref.node, STN_pass2_seqno)) {
                  sge_zero_node_fields(jref.node, NULL);
                  lSetUlong(jref.node, STN_pass2_seqno, sge_scheduling_run);
               }
            }
         }
      }
   }

   /* 
    * copy tickets 
    * 
    * Tickets for unenrolled pending tasks were stored an internal table.
    * Now it is necessary to find the ja_task of a job which got the 
    * most tickets. These ticket numbers will be stored in the template
    * element within the job. 
    */
   if (queued_jobs != NULL) {
      sge_task_ref_t *tref = task_ref_get_first_job_entry();

      while (tref != NULL) {
         lListElem *job = job_list_locate(queued_jobs, tref->job_number); 

         if (job) {
            lListElem *ja_task_template;

            ja_task_template = lFirst(lGetList(job, JB_ja_template));

            task_ref_copy_to_ja_task(tref, ja_task_template);
         } 
         tref = task_ref_get_next_job_entry();
      }
   }

   sge_free(&job_ref);
   lFreeList(&decay_list);

   if (prof_is_active(SGE_PROF_SCHEDLIB4)){
      prof_stop_measurement(SGE_PROF_SCHEDLIB4, NULL);
      prof_calc = prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4, false, NULL);    
   
      PROFILING((SGE_EVENT, "PROF: job ticket calculation: init: %.3f s, pass 0: %.3f s, pass 1: %.3f, pass2: %.3f, calc: %.3f s",
               prof_init, prof_pass0, prof_pass1, prof_pass2, prof_calc));
   }

   DEXIT;
   return sge_scheduling_run;
}


/*---------------------------------------------------
 * sge_sort_job_nodes - return a sorted list of job
 * nodes with the priority set in STN_sort.
 *---------------------------------------------------*/

static lList *
sge_sort_job_nodes(lListElem *root,
                   lListElem *node,
                   double total_share_tree_tickets,
                   sge_ref_t *job_ref)
{
   /* Prune sparsely populated sharetree */
   /* Dispose of uppermost levels without jobs */
   if(root == node){
      int active_nodes = 0;
      lListElem *child;
      lListElem *temp_root = NULL;

      for_each(child, lGetList(node, STN_children)) {
         if (lGetUlong(child, STN_ref)) {
            active_nodes++;
            break;
         } else if (lGetUlong(child, STN_job_ref_count)) {
            temp_root = child;
            active_nodes++; 
         }
         if (active_nodes >1)
               break;
      }

      /*
       * Prune this node if:-
       * - There are no jobs at higher levels, or in sibling nodes
       * - It has no jobs
       * - It has only one child object that contains jobs
       */
      if (active_nodes == 1 && temp_root)
         return sge_sort_job_nodes(temp_root, temp_root, total_share_tree_tickets, job_ref);
   }

   /* get the child job nodes in a single list */
   int job_nodes = 0;
   lList *job_node_list = NULL;

   lListElem *child;
   for_each(child, lGetList(node, STN_children)) {
      if (lGetUlong(child, STN_ref)) {
         /* this is a job node, chain it onto our list */
         if (!job_node_list)
            job_node_list = lCreateList("sorted job node list", STN_Type);
         lAppendElem(job_node_list, lCopyElem(child));
         job_nodes++;
      } else if (lGetUlong(child, STN_job_ref_count)) {
         lList *child_job_node_list;
         /* recursively get all the child job nodes onto our list */
         if ((child_job_node_list = sge_sort_job_nodes(root, child, total_share_tree_tickets, job_ref))) {
            if (job_node_list == NULL)
               job_node_list = child_job_node_list;
            else
               lAddList(job_node_list, &child_job_node_list);
         }
      }
   }

   /* free the temporary job nodes */
   if (job_nodes)
      lSetList(node, STN_children, NULL);

   /* sort the job nodes based on the calculated priority */
   if (root != node || job_nodes) { 
      if (job_node_list && lGetNumberOfElem(job_node_list)>1)
         lPSortList(job_node_list, "%I+ %I- %I+ %I+", STN_queued, STN_sort, STN_jobid, STN_taskid);

      /* calculate a new priority -
         The priority of each job associated with this node is the node's
         short term entitlement (STN_stt) divided by the number of job slots
         which are scheduled ahead of this node times the number of share tree
         tickets. If we are dependent on another higher-level policy, we also add
         the tickets from those policies. */

      /* - get entitlement */
      double node_stt = lGetDouble(node, STN_stt);

      /* - increment job count and dilute entitlement */
      double dilute_factor = 1;

      lListElem *job_node;
      for_each(job_node, job_node_list) {

         lSetDouble(job_node, STN_shr, node_stt / dilute_factor);
         lSetDouble(job_node, STN_sort, lGetDouble(job_node, STN_tickets) +
                    ((node_stt / dilute_factor) * total_share_tree_tickets));

         /* Determine job's contribution to dilution */
         int dilution = 1;
         int job_ndx = lGetUlong(job_node, STN_ref) -1;
         if (job_ndx >= 0) {
            /* Use minimum value of first pe slot range */
            /* (simplistic, but could be worse) */
            lListElem *pe_range = lFirst(lGetList(job_ref[job_ndx].job, JB_pe_range));
            if (pe_range) {
               dilution = MAX(1, lGetUlong(pe_range, RN_min));
            }
         }

         /* Dilute priority for next job */
         dilute_factor += dilution;
      }
   }

   /* return the new list */
   return job_node_list;
}


/*--------------------------------------------------------------------
 * sge_calc_sharetree_targets - calculate the targeted 
 * proportional share for each node in the sharetree.
 *--------------------------------------------------------------------*/

static int
sge_calc_sharetree_targets( lListElem *root,
                            scheduler_all_data_t *lists,
                            lList *decay_list,
                            u_long curr_time,
                            u_long seqno )
{

   DENTER(TOP_LAYER, "sge_calc_sharetree_targets");

   /* decay and store user and project usage into sharetree nodes */

   sge_calc_node_usage(root,
                       lists->user_list,
                       lists->project_list,
                       decay_list,
                       curr_time,
                       NULL,
                       seqno);

   /* Calculate targeted proportions for each node */

   sge_calc_node_targets(root, root, lists);

   DEXIT;
   return 0;
}


/*--------------------------------------------------------------------
 * sge_calc_node_targets - calculate the targeted proportions
 * for the sub-tree rooted at this node
 *--------------------------------------------------------------------*/

static int
sge_calc_node_targets( lListElem *root,
                       lListElem *node,
                       scheduler_all_data_t *lists )
{
   lList *children;
   lListElem *child;
   double sum_shares, sum, compensation_factor;
   /* char *node_name = lGetString(node, STN_name); */

   DENTER(TOP_LAYER, "sge_calc_node_targets");

   /*---------------------------------------------------------------
    * if this is the root node, initialize proportions
    *    ltt - long term targeted % among siblings
    *    oltt - overall long term targeted % among entire share tree
    *    stt - short term targeted % among siblings
    *    ostt - overall short term targeted % among entire share tree
    *---------------------------------------------------------------*/

   if (node == root) {
      lSetDouble(node, STN_stt, 1.0);
      lSetDouble(node, STN_ltt, 1.0);
      lSetDouble(node, STN_ostt, 1.0);
      lSetDouble(node, STN_oltt, 1.0);
   }

   /*---------------------------------------------------------------
    * if this node has no children, there's nothing to do
    *---------------------------------------------------------------*/

   if ((!(children = lGetList(node, STN_children))) ||
       lGetNumberOfElem(children) == 0) {
      DEXIT;
      return 0;
   }

/*
   do for each active sub-node
      sum_shares += subnode.shares
      subnode.shr = subnode.shares * subnode.shares / subnode.usage
      sum += subnode.shr 
   end do
*/

   /*---------------------------------------------------------------
    * calculate the shr value of each child node
    * calculate the short term targeted proportions 
    *---------------------------------------------------------------*/

   sum_shares = 0;
   for_each(child, children) {
      if (lGetUlong(child, STN_job_ref_count)>0) {
         sum_shares += lGetUlong(child, STN_shares);
      }
   }

   sum = 0;
   for_each(child, children) {
      if (lGetUlong(child, STN_job_ref_count)>0) {
         double shares, shr, ltt, oltt;
         shares = lGetUlong(child, STN_shares);
         if (sum_shares > 0) {
            ltt = shares / (double)sum_shares;
         } else {
            ltt = 0;
         }
         oltt = lGetDouble(node, STN_oltt) * ltt;
         lSetDouble(child, STN_ltt, ltt);
         lSetDouble(child, STN_oltt, oltt);
         if (oltt > 0) {
            shr = shares * shares / 
                MAX(lGetDouble(child, STN_combined_usage), SGE_MIN_USAGE*oltt);
         } else {
            shr = 0;
         }
         lSetDouble(child, STN_shr, shr);
         sum += shr;
      }
   }

/*
   do for each active sub-node
      subnode.ltt = subnode.shares / sum_shares;
      subnode.oltt = node.oltt * subnode.ltt;
      subnode.stt = subnode.shr / sum;
      subnode.ostt = node.stt * subnode.stt;
   end do
*/

   /*---------------------------------------------------------------
    * calculate the short term targeted proportions 
    *---------------------------------------------------------------*/

   for_each(child, children) {
      if (lGetUlong(child, STN_job_ref_count)>0) {
         double stt;
  
         if (sum > 0) { 
            stt = lGetDouble(child, STN_shr) / sum;
         } else {
            stt = 0;
         }

         lSetDouble(child, STN_stt, stt);
         lSetDouble(child, STN_ostt, lGetDouble(node, STN_ostt) * stt);
         lSetDouble(child, STN_adjusted_current_proportion, lGetDouble(child, STN_ostt));
      }
   }

/* psydo code:

   if compensation_factor != 0

      sum = 0
      do for each active sub-node
         if subnode.ostt > (compensation_factor * subnode.oltt)
            subnode.shr = subnode.shares * subnode.shares / 
                  (subnode.usage * (subnode.ostt / (compensation_factor * subnode.oltt)))
            sum += subnode.shr
         endif
      enddo

      do for each active sub-node
         subnode.stt = subnode.shr / sum;
         subnode.ostt = node.stt * subnode.stt;
      end do

   endif
*/

   /*---------------------------------------------------------------
    * adjust targeted proportion based on compensation factor
    *---------------------------------------------------------------*/

   compensation_factor = sconf_get_compensation_factor();

   if (compensation_factor != 0) {
      u_long compensation = 0;

      /*---------------------------------------------------------------
       * recalculate the shr value of each child node based on compensation factor
       *---------------------------------------------------------------*/

      sum = 0;
      for_each(child, children) {
         if (lGetUlong(child, STN_job_ref_count)>0) {
            double ostt = lGetDouble(child, STN_ostt);
            double oltt = lGetDouble(child, STN_oltt);
            if (ostt > (compensation_factor * oltt)) {
               double shares = lGetUlong(child, STN_shares);
               double shr = shares * shares /
                     (MAX(lGetDouble(child, STN_combined_usage), SGE_MIN_USAGE*oltt) *
                     (ostt / (compensation_factor * oltt)));
               lSetDouble(child, STN_shr, shr);
               compensation = 1;
            }
            sum += lGetDouble(child, STN_shr);
         }
      }

      /*---------------------------------------------------------------
       * reset short term targeted proportions (if necessary)
       *---------------------------------------------------------------*/

      if (compensation) {
         for_each(child, children) {
            if (lGetUlong(child, STN_job_ref_count)>0) {
               double stt = lGetDouble(child, STN_shr) / sum;
               lSetDouble(child, STN_stt, stt);
               lSetDouble(child, STN_ostt, lGetDouble(node, STN_ostt) * stt);
               lSetDouble(child, STN_adjusted_current_proportion, lGetDouble(child, STN_ostt));
            }
         }
      }

   }

/*
   do for each active sub-node
      call sge_calc_node_targets(subnode, lists);
   end do
*/

   /*---------------------------------------------------------------
    * recursively call self for all active child nodes
    *---------------------------------------------------------------*/

   for_each(child, children) {
      if (lGetUlong(child, STN_job_ref_count)>0) {
         sge_calc_node_targets(root, child, lists);
      }
   }

   DEXIT;
   return 0;
}

/*--------------------------------------------------------------------
 * get_mod_share_tree - return reduced modified share tree
 *--------------------------------------------------------------------*/

static lListElem *
get_mod_share_tree( lListElem *node,
                    lEnumeration *what,
                    int seqno )
{
   lListElem *new_node=NULL;
   lList *children;
   lListElem *child;

   if (((children = lGetList(node, STN_children))) &&
       lGetNumberOfElem(children) > 0) {

      lList *child_list=NULL;

      for_each(child, children) {
         lListElem *child_node;
         child_node = get_mod_share_tree(child, what, seqno);
         if (child_node) {
            if (!child_list)
               child_list = lCreateList("", STN_Type);
            lAppendElem(child_list, child_node);
         }
      }

      if (child_list) {
         new_node = lCopyElem(node);
         lSetList(new_node, STN_children, child_list);
      }
      
   } else {

      if (lGetUlong(node, STN_pass2_seqno) > (u_long32)seqno &&
          lGetUlong(node, STN_temp) == 0) {
         new_node = lCopyElem(node);
      }

   }

   return new_node;
}

/****** sgeee/sge_build_sgeee_orders() *******************************************
*  NAME
*     sge_build_sgeee_orders() -- build orders for updating qmaster

*
*  SYNOPSIS
*     void sge_build_sgeee_orders(sge_Sdescr_t *lists, lList *running_jobs, 
*     lList *queued_jobs, lList *finished_jobs, order_t *orders, int 
*     update_usage_and_configuration, int seqno) 
*
*  FUNCTION
*     Builds generates the orderlist for sending the scheduling decisions
*     to the qmaster. The following orders are generated:
*     - running job tickets
*     - pending job tickets
*     - delete order for finished jobs
*     - update user usage order
*     - update project usage order 
*     - update share tree order
*     - update scheduler configuration order
*     -  orders updating user/project resource usage (ORT_update_project_usage)
*     -  orders updating running tickets needed for dynamic repriorization (ORT_ticket)
*     Most orders are generated by using the sge_create_orders function.
*
*  INPUTS
*     sge_Sdescr_t *lists                 - ??? 
*     lList *running_jobs                 - list of running jobs 
*     lList *queued_jobs                  - list of queued jobs (should be sorted by ticktes)
*     lList *finished_jobs                - list of finished jobs
*     order_t *orders                     - existing order list (new orders will be added to it
*     bool update_usage_and_configuration - if true, the update usage orders are generated
*     int seqno                           - a seqno, changed with each scheduling run
*     bool max_queued_ticket_orders       - if true, pending tickets are submited to the 
*                                           qmaster 
*     bool updated_execd                  - if true, the queue information is send with 
*                                           the running job tickets
*
*  RESULT
*     void
*
*******************************************************************************/
void 
sge_build_sgeee_orders(scheduler_all_data_t *lists, lList *running_jobs, lList *queued_jobs, 
                       lList *finished_jobs, order_t *orders, 
                       bool update_usage_and_configuration, int seqno, bool update_execd)
{
   lCondition *user_where=NULL;
   lCondition *prj_where=NULL;
   lList *up_list = NULL;
   lList *order_list = NULL;
   lListElem *order = NULL;
   lListElem *root = NULL;
   lListElem *job = NULL;
   int norders = 0;
   u_long32 max_pending_tasks_per_job = sconf_get_max_pending_tasks_per_job();

   bool max_queued_ticket_orders = sconf_get_report_pjob_tickets();
   
   DENTER(TOP_LAYER, "sge_build_sgeee_orders");

   if (share_tree_what == NULL) {
      share_tree_what = lWhat("%T(%I %I %I %I %I %I)", STN_Type,
                         STN_version, STN_name, STN_job_ref_count, STN_m_share,
                         STN_last_actual_proportion,
                         STN_adjusted_current_proportion);
   }   

   if (user_usage_what == NULL) {
      user_usage_what = lWhat("%T(%I %I %I %I %I %I %I)", UU_Type,
                              UU_name, UU_usage, UU_usage_time_stamp,
                              UU_long_term_usage, UU_project,
                              UU_debited_job_usage, UU_version);
   }   
   if (prj_usage_what == NULL) {
      prj_usage_what = lWhat("%T(%I %I %I %I %I %I %I)", PR_Type,
                              PR_name, PR_usage, PR_usage_time_stamp,
                              PR_long_term_usage, PR_project,
                              PR_debited_job_usage, PR_version);
   }   

   if (orders->pendingOrderList == NULL) {
      orders->pendingOrderList = lCreateList("orderlist", OR_Type);
   }   

   order_list = orders->pendingOrderList; 
   
   /*-----------------------------------------------------------------
    * build ticket orders for running jobs
    *-----------------------------------------------------------------*/

   if (running_jobs) {
      norders = lGetNumberOfElem(order_list);

      DPRINTF(("   got %d running jobs\n", lGetNumberOfElem(running_jobs)));

      for_each(job, running_jobs) {
         const char *pe_str = NULL;
         lList *granted = NULL;
         lListElem *pe = NULL;
         lListElem *ja_task = NULL;

         for_each(ja_task, lGetList(job, JB_ja_tasks)) {
            if ((pe_str=lGetString(ja_task, JAT_granted_pe))
                  && (pe=pe_list_locate(all_lists->pe_list, pe_str))
                  && lGetBool(pe, PE_control_slaves)) {

               granted=lGetList(ja_task, JAT_granted_destin_identifier_list);
            }  
            else {
               granted=NULL;
            }   

            order_list = sge_create_orders(order_list, ORT_tickets, job, ja_task, granted, update_execd);
         }
      }
      DPRINTF(("   added %d ticket orders for running jobs\n", 
               lGetNumberOfElem(order_list) - norders));
   }


/*
 * check for the number of pending ticket orders should be checked.
 * If the report_pjob_ticktes has changed to false, we need to send 
 * ORT_clear_pri_info to the qmaster to remove the pticket values. 
 * This prevents qstat from reporting wrong pticket values. Its only done
 * once after the config change.
 */
   if ((queued_jobs != NULL) && (max_queued_ticket_orders || sconf_is_new_config())) {
      lListElem *qep = NULL;
      u_long32 free_qslots = 0;
      norders = lGetNumberOfElem(order_list);
      for_each(qep, lists->queue_list) {
         free_qslots += MAX(0, ((int)lGetUlong(qep, QU_job_slots) - qinstance_slots_used(qep)));
      }
      
      for_each(job, queued_jobs) {
         lListElem *ja_task = NULL;
         int tasks = 0;
  
         if (max_queued_ticket_orders) {
         
            /* NOTE: only send pending tickets for the first sub-task */
            /* when the first sub-task gets scheduled, then the other sub-tasks didn't have
               any tickets specified */
            for_each(ja_task, lGetList(job, JB_ja_tasks)) {
               if (++tasks > MIN(max_pending_tasks_per_job, free_qslots+1)) {
                  break;
               }   
               order_list = sge_create_orders(order_list, ORT_ptickets, job, ja_task, NULL, false);
            }

            if (job_get_not_enrolled_ja_tasks(job) > 0) {
               lListElem *task_template = NULL;

               task_template = lFirst(lGetList(job, JB_ja_template));
               if (task_template) {
                  order_list = sge_create_orders(order_list, ORT_ptickets, job, task_template, NULL, false);
               }
            }  
         } else {
            order_list = sge_create_orders(order_list, ORT_clear_pri_info, job, NULL, NULL, false);               
         }
      }
      DPRINTF(("   added %d ticket orders for queued jobs\n", 
               lGetNumberOfElem(order_list) - norders));

   }

   /*-----------------------------------------------------------------
    * build delete job orders for finished jobs
    *-----------------------------------------------------------------*/
   if (finished_jobs) {
      order_list  = create_delete_job_orders(finished_jobs, order_list);
   }

   if (update_usage_and_configuration) {

      /*-----------------------------------------------------------------
       * build update user usage order
       *-----------------------------------------------------------------*/

      /* NOTE: make sure we get all usage entries which have been decayed
         or have accumulated additional usage */

      user_where = lWhere("%T(%I > %u)", UU_Type, UU_usage_seqno, last_seqno);

      if (lists->user_list) {
         norders = lGetNumberOfElem(order_list); 
         if ((up_list = lSelect("", lists->user_list, user_where, user_usage_what))) {
            if (lGetNumberOfElem(up_list)>0) {
               order = lCreateElem(OR_Type);
               lSetUlong(order, OR_type, ORT_update_user_usage);
               lSetList(order, OR_joker, up_list);
               lAppendElem(order_list, order);
            } else {
               lFreeList(&up_list);
            }
         }
         DPRINTF(("   added %d orders for updating usage of user\n",
            lGetNumberOfElem(order_list) - norders));      
      }

      /*-----------------------------------------------------------------
       * build update project usage order
       *-----------------------------------------------------------------*/
      if (lists->project_list) {
         norders = lGetNumberOfElem(order_list); 
         if ((up_list = lSelect("", lists->project_list, prj_where, prj_usage_what))) {
            if (lGetNumberOfElem(up_list)>0) {
               order = lCreateElem(OR_Type);
               lSetUlong(order, OR_type, ORT_update_project_usage);
               lSetList(order, OR_joker, up_list);
               lAppendElem(order_list, order);
            } else {
               lFreeList(&up_list);
            }   
         }
         DPRINTF(("   added %d orders for updating usage of project\n",
            lGetNumberOfElem(order_list) - norders));
      }

      lFreeWhere(&user_where);
      lFreeWhere(&prj_where);

      /*-----------------------------------------------------------------
       * build update share tree order
       *-----------------------------------------------------------------*/

      if (lists->share_tree && ((root = lFirst(lists->share_tree)))) {
         lListElem *node = NULL;
         norders = lGetNumberOfElem(order_list);

         if ((node = get_mod_share_tree(root, share_tree_what, last_seqno))) {
            up_list = lCreateList("", STN_Type);
            lAppendElem(up_list, node);

            order = lCreateElem(OR_Type);
            lSetUlong(order, OR_type, ORT_share_tree);
            lSetList(order, OR_joker, up_list);
            lAppendElem(order_list, order);
         }
         DPRINTF(("   added %d orders for updating share tree\n",
            lGetNumberOfElem(order_list) - norders)); 
      } 

      /*-----------------------------------------------------------------
       * build update scheduler configuration order
       *-----------------------------------------------------------------*/

      if (sconf_is()) {
         lListElem *node = NULL;
         const lDescr schedConfDesc[] = {
                            {SC_weight_tickets_override, lUlongT  | CULL_IS_REDUCED, NULL},
                            {NoName, lEndT  | CULL_IS_REDUCED, NULL}
                           };

         node = lCreateElem(schedConfDesc);
         up_list = lCreateList("sched_conf_update", schedConfDesc);

         lAppendElem(up_list, node);
         
         lSetUlong(node, SC_weight_tickets_override, 
                   sconf_get_weight_tickets_override());

         order = lCreateElem(OR_Type);
         lSetUlong(order, OR_type, ORT_sched_conf);
         lSetList(order, OR_joker, up_list);
         lAppendElem(order_list, order);
         
         DPRINTF(("   added 1 order for scheduler configuration\n"));
         
      }
      last_seqno = seqno;
   }

   DEXIT;
   return;
}

/*--------------------------------------------------------------------
 * sge_do_priority
 *--------------------------------------------------------------------*/

void sge_do_priority(lList *running_jobs, lList *pending_jobs)
{
   lListElem *jep;
   const double min_priority = 0;
   const double max_priority = 2048;
   double priority;

   for_each(jep, running_jobs) {
      priority = (double)lGetUlong(jep, JB_priority);
      lSetDouble(jep, JB_nppri, sge_normalize_value(priority, min_priority, max_priority));
   }

   for_each(jep, pending_jobs) {
      priority = (double)lGetUlong(jep, JB_priority);
      lSetDouble(jep, JB_nppri, sge_normalize_value(priority, min_priority, max_priority));
   }
}

void sge_do_priority_job(lListElem *jep)
{
   const double min_priority = 0;
   const double max_priority = 2048;
   double priority;

   
   priority = (double)lGetUlong(jep, JB_priority);
   lSetDouble(jep, JB_nppri, sge_normalize_value(priority, min_priority, max_priority));
}

/*--------------------------------------------------------------------
 * sge_scheduler
 *--------------------------------------------------------------------*/


/****** sgeee/sgeee_scheduler() ************************************************
*  NAME
*     sgeee_scheduler() -- calc tickets, send orders, and sort job list 
*
*  SYNOPSIS
*     int sgeee_scheduler(sge_Sdescr_t *lists, lList *running_jobs, lList 
*     *finished_jobs, lList *pending_jobs, lList **orderlist) 
*
*  FUNCTION
*     - calculates the running and pending job tickets. 
*     - send the orders to the qmaster about the job tickets
*     - order the pending job list according the the job tickets
*
*    On a "normal" scheduling interval:
*	   - calculate tickets for new and running jobs
*	   - don't decay and sum usage
*	   - don't update qmaster
*
*    On a scheduling interval:
*	   - calculate tickets for new and running jobs
*	   - decay and sum usage
*	   - handle finished jobs
*	   - update qmaster
*
*  INPUTS
*     sge_Sdescr_t *lists  - a ref to all lists in this scheduler 
*     lList *running_jobs  - a list of all running jobs 
*     lList *finished_jobs -  a list of all finished jobs
*     lList *pending_jobs  -  a list of all pending jobs
*     lList **orderlist    -  the order list
*
*  RESULT
*     int - 0 if everthing went fine, -1 if not
*
*******************************************************************************/
int sgeee_scheduler(scheduler_all_data_t *lists,
               lList *running_jobs,
               lList *finished_jobs,
               lList *pending_jobs,
               order_t *orders)
{
   u_long32 now = sge_get_gmt();
   int seqno;
   lListElem *job;
   double min_tix  = 0;
   double max_tix  = -1;

   bool report_priority = sconf_get_report_pjob_tickets();
   bool do_nurg, do_nprio;

   DENTER(TOP_LAYER, "sgeee_scheduler");

   /* skip computation of ntix, nurg and nprio 
      but only if it is irrelevant and not monitored */
   do_nurg  = (sconf_get_weight_urgency() != 0  || report_priority) ? true : false;
   do_nprio = (sconf_get_weight_priority() != 0 || report_priority) ? true : false;

   /* clear fields for queued jobs */
   for_each(job, pending_jobs) {   
      sge_clear_job(job, false);
   }
   for_each(job, running_jobs) {   
      sge_clear_job(job, false);
   }   

   /* calculate per job static urgency values */
   if (do_nurg) {
      PROF_START_MEASUREMENT(SGE_PROF_CUSTOM3);
      sge_do_urgency(now, pending_jobs, running_jobs, lists);

      if (prof_is_active(SGE_PROF_CUSTOM3)) {
         prof_stop_measurement(SGE_PROF_CUSTOM3, NULL);

         PROFILING((SGE_EVENT, "PROF: static urgency took %.3f s",
               prof_get_measurement_wallclock(SGE_PROF_CUSTOM3, false, NULL)));
      }

   }   

   min_tix = 0;
   max_tix = -1;
   
   /* calculate tickets for pending jobs and compute their shares */
   sge_calc_tickets(lists, running_jobs, finished_jobs, pending_jobs, 1, &max_tix);
         
   /* correct the shares for the running jobs */
   seqno = sge_calc_tickets(lists, running_jobs, NULL, NULL, 0, &max_tix);

   if (max_tix == -1) { /* we have no running jobs and the tickets are disabled. */
      max_tix = 0;
   }

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM3);

   /* use min/max tix for normalizing 
      - now to determine initial normalized ticket amount
      - but also later on after tickets recomputation when a job was assigned */
   tix_range_set(min_tix, max_tix);


#ifdef DEBUG_TASK_REF
   task_ref_print_table();
#endif

   /*
    * now combine ticket amount and static urgency scheme into 
    * absolute priority
    */
   DPRINTF(("Normalizing tickets using %f/%f as min_tix/max_tix\n", min_tix, max_tix));
   sge_do_sgeee_priority(running_jobs, min_tix, max_tix, do_nprio, do_nurg); 
   sge_do_sgeee_priority(pending_jobs, min_tix, max_tix, do_nprio, do_nurg); 

   if (prof_is_active(SGE_PROF_CUSTOM3)) {
      prof_stop_measurement(SGE_PROF_CUSTOM3, NULL);

      PROFILING((SGE_EVENT, "PROF: normalizing job tickets took %.3f s",
                 prof_get_measurement_wallclock(SGE_PROF_CUSTOM3, false, NULL)));
   }

   PROF_STOP_MEASUREMENT(SGE_PROF_SCHEDLIB4)
   
   /* somebody might have played with the system clock. */
   if (now < past) {
      past = now;
   }   

   {
      u_long32 reprioritize_interval = sconf_get_reprioritize_interval(); 
      bool update_execd = (reprioritize_interval == 0 || (now >= (past + reprioritize_interval))) ? true : false; 
      if (update_execd){
         past = now;
      } 
      /* we are not calculation pending tickets here, only the running, finished jobs,
         and the sharetree changes are processed. The pending tickets are calculated at the
         end of the scheduler cycle */
      sge_build_sgeee_orders(lists, running_jobs, NULL, finished_jobs, orders, true, seqno, 
                             update_execd);
   }

   if (prof_is_active(SGE_PROF_SCHEDLIB4)) {
      prof_stop_measurement(SGE_PROF_SCHEDLIB4, NULL);

      PROFILING((SGE_EVENT, "PROF: create active job orders: %.3f s",
               prof_get_measurement_wallclock(SGE_PROF_SCHEDLIB4, false, NULL)));
   }  
   
   DEXIT;
   return 0;
}

/****** sgeee/sge_do_sgeee_priority() ******************************************
*  NAME
*     sge_do_sgeee_priority() -- determine GEEE priority for a list of jobs
*
*  SYNOPSIS
*     static void sge_do_sgeee_priority(lList *job_list, double min_tix, double 
*     max_tix) 
*
*  FUNCTION
*     Determines for a list of jobs the GEEE priority. Prior
*     sge_do_sgeee_priority() can be called the normalized urgency value must 
*     already be known for each job. The ticket range passed is used for 
*     normalizing ticket amount.
*
*  INPUTS
*     lList *job_list - The job list
*     double min_tix  - Minumum ticket amount
*     double max_tix  - Maximum ticket amount
*     bool do_nprio   - Needs norm. priority be determined
*     bool do_nurg    - Needs norm. urgency be determined
*
*  NOTES
*     MT-NOTE: sge_do_sgeee_priority() is MT safe
*******************************************************************************/
static void sge_do_sgeee_priority(lList *job_list, double min_tix, double max_tix,
               bool do_nprio, bool do_nurg) 
{
   lListElem *job, *task;
   u_long32 jobid;
   bool enrolled;
   double nsu = 0.5, npri = 0.5;

   for_each (job, job_list) {
      jobid = lGetUlong(job, JB_job_number);

      if (do_nurg)
         nsu = lGetDouble(job, JB_nurg);
      if (do_nprio)
         npri = lGetDouble(job, JB_nppri);
   
      enrolled = false;

      for_each (task, lGetList(job, JB_ja_tasks)) {
         sgeee_priority(task, nsu, npri, min_tix, max_tix);
         enrolled = true;
      }

      if (!enrolled) {
         task = lFirst(lGetList(job, JB_ja_template));
         sgeee_priority(task, nsu, npri, min_tix, max_tix);
      }
   }
}

/****** sgeee/sgeee_priority() *************************************************
*  NAME
*     sgeee_priority() -- Compute final GE priority 
*
*  SYNOPSIS
*     static void sgeee_priority(lListElem *task, double nsu,
*                                double min_tix, double max_tix)
*
*  FUNCTION
*     The GE priority is computed for the task based on the already known
*     ticket amount and already normalized urgency value. The ticket amount
*     is normalized based on the ticket range passed. The weights for
*     ticket and urgency value are applied.
*
*  INPUTS
*     lListElem *task - The task whose priority is computed
*     u_long32 jobid  - The jobs id
*     double nsu      - The normalized urgency value that applies to all 
*                       tasks of the job.
*     double min_tix  - minimum ticket amount 
*     double max_tix  - maximum ticket amount 
*
*  NOTES
*     MT-NOTE: sgeee_priority() is MT safe
*******************************************************************************/
static void sgeee_priority(lListElem *task, double nsu,
                           double npri, double min_tix, double max_tix)
{

   double nta, geee_priority;
   double weight_ticket = 0.0;
   double weight_urgency = 0.0;
   double weight_priority = 0.0; 

   DENTER(TOP_LAYER, "sgeee_priority");
   sconf_get_weight_ticket_urgency_priority(&weight_ticket, &weight_urgency, &weight_priority);

   /* now compute normalized ticket amount (NTA) for each job/task */
   nta = sge_normalize_value(lGetDouble(task, JAT_tix), min_tix, max_tix);
   lSetDouble(task, JAT_ntix, nta);

   /* combine per task NTA with per job normalized static urgency 
      (NSU) and per job normalized POSIX priority (NPRI) into
      SGE priority */
   geee_priority = weight_urgency * nsu + weight_ticket * nta +
                   weight_priority * npri;
/*
   DPRINTF(("SGE priority (" sge_u32 "."sge_u32 ") %f = %f * %f + %f * %f + %f * %f\n",
      jobid, lGetUlong(task, JAT_task_number), geee_priority, 
      weight_urgency, nsu, weight_ticket, nta, weight_priority, npri));
*/
   lSetDouble(task, JAT_ntix, nta);
   lSetDouble(task, JAT_prio, geee_priority);

   DEXIT;
}


/****** sgeee/calculate_pending_shared_override_tickets() **********************
*  NAME
*     calculate_pending_shared_override_tickets() -- calculate shared override tickets
*
*  SYNOPSIS
*     static void calculate_pending_shared_override_tickets(sge_ref_t *job_ref, 
*     int num_jobs, int dependent) 
*
*  FUNCTION
*     We calculate the override tickets for pending jobs, which are shared. The basic
*     algorithm looks like this:
*
*     do for each pending job
*        do for each pending job which isn't yet considered active
*              consider the job active
*              calculate override tickets for that job
*              consider the job not active
*          end do
*          consider the job with the highest priority (taking into account all previous polices + override tickets) as active
*     end do 
*
*     set all pending jobs none active
*
*  Since this algorithm is very expensive, we split all pending jobs into fcategories. The algorithm changes to:
*
*    max_jobs = build fcategories and ignore jobs, which would get 0 override tickets
*
*     do for max_jobs pending job
*        do for each fcategory
*
*           take take first job from category
*           consider the job active
*           calculate override tickets for that job
*           consider the job not active
*           store job with the most override tickets = job_max
*
*        end do
*        set job_max active and remove it from its fcategory.
*        remove job_max fcategory, if job_max was the last job
*     end;
*
*     set all pending jobs none active
*
*
*  That's it. It is very simillar to the functional ticket calculation, except, that we are working with tickts and
*  not with shares. 
*
*  INPUTS
*     sge_ref_t *job_ref - an array of job structures (first running, than pennding)
*     int num_jobs       - number of jobs in the array
*     int dependent      - do other ticket policies depend on this one?
*
*  NOTES
*     MT-NOTE: calculate_pending_shared_override_tickets() is MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static void calculate_pending_shared_override_tickets(sge_ref_t *job_ref, int num_jobs, int dependent) {
         lList *fcategories = NULL;
         sge_ref_list_t *ref_array = NULL;
         sge_ref_t **sort_list=NULL;
         int i;
         u_long32 max;
         u_long32 job_ndx;
       
         DENTER(TOP_LAYER, "calculate_pending_shared_override_tickets");
       
         max = build_functional_categories(job_ref, num_jobs, &fcategories, &ref_array, dependent, 
                                     JB_override_tickets, UU_oticket, PR_oticket, US_oticket);

         if (max > 0) {
            sort_list = malloc(max * sizeof(sge_ref_t *));

            if (sort_list == NULL) {
               /* error message to come */
               
               sge_free(&sort_list);            
               
               if (fcategories != NULL) {
                  free_fcategories(&fcategories, &ref_array);
               }
               DEXIT;
               return;
            }

            for(i=0; i < max; i++) {        
               double max_otickets=-1;
               u_long32 jid, tid, max_jid=0, max_tid=0;            

               lListElem *current = NULL;
               lListElem *max_current = NULL;         
               /* loop over all first jobs from the different categories */
               for_each(current, fcategories){ 
                  sge_ref_list_t *tmp = (lGetRef(current, FCAT_jobrelated_ticket_first)); 
                  sge_ref_t *jref = tmp->ref;    
                  double otickets;

                  otickets = calc_pjob_override_tickets_shared(jref);
                  jid = lGetUlong(jref->job, JB_job_number);
                  tid = REF_GET_JA_TASK_NUMBER(jref);

                  /* get the job with the max otickets */
                  if ((max_otickets < otickets) ||
                      (max_current == NULL)) {
                     max_current = current;
                     max_otickets = otickets;
                     max_jid = jid;
                     max_tid = tid;
                     sort_list[i] = jref;
                  }
                  else if (max_otickets == otickets) {
                     if ((max_jid > jid) ||
                        ((max_jid == jid) && (max_tid > tid))) {
                           max_current = current;
                           max_otickets = otickets;
                           max_jid = jid;
                           max_tid = tid;
                           sort_list[i] = jref;                     
                     }      
                  }         
               }
               
               if (max_current != NULL) {
                  sge_ref_list_t *tmp =lGetRef(max_current, FCAT_jobrelated_ticket_first);  
                  sge_set_job_cnts(tmp->ref, 0); /* set job active */
                  lSetRef(max_current, FCAT_jobrelated_ticket_first, tmp->next);/* next job in that category */  
                  sort_list[i]->tickets += max_otickets;

                  /* remove category */
                  if(lGetRef(max_current, FCAT_jobrelated_ticket_first) == NULL){
                     lSetRef(max_current, FCAT_jobrelated_ticket_last,  NULL);
                     lSetRef(current, FCAT_user, NULL);
                     lSetRef(current, FCAT_dept, NULL);
                     lSetRef(current, FCAT_project, NULL);

                     lRemoveElem(fcategories, &max_current);
                  }            
               }
               else { /* we reached an end. */
                  break;
               }
            }

            /* set pending jobs to be not active */
            for(job_ndx=0; job_ndx < max; job_ndx++) {
               sge_unset_job_cnts(sort_list[job_ndx], 0); 
            }
            sge_free(&sort_list);           
         }
         /* free the allocated memory */

         free_fcategories(&fcategories, &ref_array);

   return;
}


#ifdef MODULE_TEST

int
main(int argc, char **argv)
{
   int job_number;
   sge_Sdescr_t *lists;
   lList *child_list = NULL;
   lList *group_list = NULL;
   lListElem *job, *config, *node, *usage;

   lListElem *ep;

   lists = (sge_Sdescr_t *)malloc(sizeof(sge_Sdescr_t));
   memset(lists, 0, sizeof(sge_Sdescr_t));


#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);   
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */


   lInit(nmv);

   /* build host list */
   lAddElemHost(&(lists->host_list), EH_name, "racerx", EH_Type); 
   lAddElemHost(&(lists->host_list), EH_name, "yosemite_sam", EH_Type); 
   lAddElemHost(&(lists->host_list), EH_name, "fritz", EH_Type); 

   /* build queue list */
   ep = lAddElemStr(&(lists->all_queue_list), QU_full_name, "racerx.q", QU_Type); 
   lSetHost(ep, QU_qhostname, "racerx");

   ep = lAddElemStr(&(lists->all_queue_list), QU_full_name, "yosemite_sam.q", QU_Type); 
   lSetHost(ep, QU_qhostname, "yosemite_sam");

   ep = lAddElemStr(&(lists->all_queue_list), QU_full_name, "fritz.q", QU_Type); 
   lSetHost(ep, QU_qhostname, "fritz");
   
   /* build configuration */

   config = lCreateElem(SC_Type);
   lSetUlong(config, SC_halftime, 60*60);
   lSetList(config, SC_usage_weight_list,
       build_usage_list("usageweightlist", NULL));
   for_each(usage, lGetList(config, SC_usage_weight_list))
      lSetDouble(usage, UA_value, 1.0/3.0);
   lSetDouble(config, SC_compensation_factor, 2);
   lSetDouble(config, SC_weight_user, 0.25);
   lSetDouble(config, SC_weight_project, 0.25);
   lSetDouble(config, SC_weight_job, 0.25);
   lSetDouble(config, SC_weight_department, 0.25);
   lSetUlong(config, SC_weight_tickets_functional, 10000);
   lSetUlong(config, SC_weight_tickets_share, 10000);
   lists->config_list = lCreateList("config_list", SC_Type);
   lAppendElem(lists->config_list, config);

   /* build user list */

   ep = lAddElemStr(&(lists->user_list), UU_name, "davidson", UU_Type); 
   lSetUlong(ep, UU_oticket, 0);
   lSetUlong(ep, UU_fshare, 200);

   ep = lAddElemStr(&(lists->user_list), UU_name, "garrenp", UU_Type); 
   lSetUlong(ep, UU_oticket, 0);
   lSetUlong(ep, UU_fshare, 100);

   ep = lAddElemStr(&(lists->user_list), UU_name, "stair", UU_Type); 
   lSetUlong(ep, UU_oticket, 0);
   lSetUlong(ep, UU_fshare, 100);

   /* build project list */
    
   ep = lAddElemStr(&(lists->project_list), PR_name, "sgeee", PR_Type); 
   lSetUlong(ep, PR_oticket, 0);
   lSetUlong(ep, PR_fshare, 200);

   ep = lAddElemStr(&(lists->project_list), PR_name, "ms", PR_Type); 
   lSetUlong(ep, PR_oticket, 0);
   lSetUlong(ep, PR_fshare, 100);


   /* build department list */

   ep = lAddElemStr(&(lists->dept_list), US_name, "software", US_Type); 
   lSetUlong(ep, US_oticket, 0);
   lSetUlong(ep, US_fshare, 200);

   ep = lAddElemStr(&(lists->dept_list), US_name, "hardware", US_Type); 
   lSetUlong(ep, US_oticket, 0);
   lSetUlong(ep, US_fshare, 100);

   /* build share tree list */

   child_list = lCreateList("childlist", STN_Type);

   group_list = lCreateList("grouplist", STN_Type);

   node = lAddElemStr(&child_list, STN_name, "davidson", STN_Type);
   lSetUlong(node, STN_shares, 100);

   node = lAddElemStr(&child_list, STN_name, "garrenp", STN_Type);
   lSetUlong(node, STN_shares, 100);

   node = lAddElemStr(&group_list, STN_name, "groupA", STN_Type);
   lSetUlong(node, STN_shares, 1000);
   lSetList(node, STN_children, child_list);


   child_list = lCreateList("childlist", STN_Type);

   node = lAddElemStr(&child_list, STN_name, "stair", STN_Type);
   lSetUlong(node, STN_shares, 100);

   node = lAddElemStr(&group_list, STN_name, "groupB", STN_Type);
   lSetUlong(node, STN_shares, 100);
   lSetList(node, STN_children, child_list);

   node = lAddElemStr(&(lists->share_tree), STN_name, "root", STN_Type);
   lSetUlong(node, STN_type, STT_USER);
   lSetList(node, STN_children, group_list);

   /* build job list */

   job_number = 1;


   job = lAddElemUlong(&(lists->job_list), JB_job_number, job_number++, 
                           JB_Type);
   lSetUlong(job, JB_jobshare, 0);
   lSetString(job, JB_owner, "davidson");
   lSetString(job, JB_project, "sgeee");
   lSetString(job, JB_department, "software");
   lSetUlong(job, JB_submission_time, sge_get_gmt() - 60*2);
   lSetUlong(job, JB_deadline, 0);
   lSetHost(job, JB_host, "racerx");
   lSetUlong(job, JB_override_tickets, 0);
   lSetList(job, JB_scaled_usage_list, build_usage_list("jobusagelist", NULL));
   for_each(usage, lGetList(job, JB_scaled_usage_list))
      lSetDouble(usage, UA_value, rand());

   job = lAddElemUlong(&(lists->job_list), JB_job_number, job_number++, 
                           JB_Type);
   lSetUlong(job, JB_jobshare, 1000);
   lSetString(job, JB_owner, "davidson");
   lSetString(job, JB_project, "ms");
   lSetString(job, JB_department, "software");
   lSetUlong(job, JB_submission_time, sge_get_gmt() - 60*2);
   lSetUlong(job, JB_deadline, 0);
   lSetHost(job, JB_host, "racerx");
   lSetUlong(job, JB_override_tickets, 0);
   lSetList(job, JB_scaled_usage_list, build_usage_list("jobusagelist", NULL));
   for_each(usage, lGetList(job, JB_scaled_usage_list))
      lSetDouble(usage, UA_value, rand());

   job = lAddElemUlong(&(lists->job_list), JB_job_number, job_number++, 
                           JB_Type);
   lSetUlong(job, JB_job_number, job_number++);
   lSetUlong(job, JB_jobshare, 0);
   lSetString(job, JB_owner, "stair");
   lSetString(job, JB_project, "sgeee");
   lSetString(job, JB_department, "hardware");
   lSetUlong(job, JB_submission_time, sge_get_gmt() - 60*2);
   lSetUlong(job, JB_deadline, 0);
   lSetHost(job, JB_host, "racerx");
   lSetUlong(job, JB_override_tickets, 0);
   lSetList(job, JB_scaled_usage_list, build_usage_list("jobusagelist", NULL));
   for_each(usage, lGetList(job, JB_scaled_usage_list))
      lSetDouble(usage, UA_value, rand());

   job = lAddElemUlong(&(lists->job_list), JB_job_number, job_number++, 
                           JB_Type);
   lSetUlong(job, JB_jobshare, 0);
   lSetString(job, JB_owner, "garrenp");
   lSetString(job, JB_project, "sgeee");
   lSetString(job, JB_department, "software");
   lSetUlong(job, JB_submission_time, sge_get_gmt() - 60*2);
   lSetUlong(job, JB_deadline, sge_get_gmt() + 60*2);
   lSetHost(job, JB_host, "racerx");
   lSetUlong(job, JB_override_tickets, 0);
   lSetList(job, JB_scaled_usage_list, build_usage_list("jobusagelist", NULL));
   for_each(usage, lGetList(job, JB_scaled_usage_list))
      lSetDouble(usage, UA_value, rand());


   /* call the scheduler */

   sge_setup_lists(lists, lists->job_list, NULL);

   sge_calc_tickets(lists, lists->job_list, NULL, NULL);

   sge_calc_tickets(lists, lists->job_list, NULL, NULL);

/*   lWriteListTo(lists->job_list, stdout);*/

   sge_calc_share_tree_proportions( lists->share_tree,
                                    lists->user_list,
                                    lists->project_list,
                                    lists->config_list);

/*   lWriteListTo(lists->share_tree, stdout);*/

   return 0;
}

#endif


/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/


