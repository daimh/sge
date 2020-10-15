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

/* fixme: doesn't build -- should it be fixed and kept? */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_dstring.h"

#include "comm/commlib.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_order.h"
#include "sgeobj/sge_event.h"

#include "mir/sge_mirror.h"

#include "evc/sge_event_client.h"

#include "sig_handlers.h"
#include "sge_ssi.h"
#include "sge_orders.h"
#include "sge_job_schedd.h"

#include "msg_clients_common.h"

/* examples for retrieving information from SGE's master lists */
static void print_load_value(lListElem *host, const char *name, const char *format);
static void get_cluster_info(void);
static void get_workload_info(void);
static void get_policy_info(void);

/* implementation of a simple job scheduler */
static sge_callback_result remove_finished_job(sge_evc_class_t *evc, 
                                object_description *object_base, 
                                sge_object_type type, 
                                sge_event_action action, 
                                lListElem *event, 
                                void *clientdata);
/* static int queue_get_free_slots(lListElem *queue); */
/* static void allocate_queue_slots(lList **allocated_queues, lListElem *queue, u_long32 *procs); */
static void simple_scheduler(sge_evc_class_t *evc);
/* static void delete_some_jobs(sge_gdi_ctx_class_t *ctx); */


static sge_callback_result remove_finished_job(sge_evc_class_t *evc, object_description *object_base, 
                                sge_object_type type, sge_event_action action, 
                                lListElem *event, void *clientdata)
{
   DENTER(TOP_LAYER, "remove_finished_job");

   /* if we get a final usage event for a ja_task,
    * we have to send a job delete order to qmaster
    */
   if(lGetUlong(event, ET_type) == sgeE_JOB_FINAL_USAGE) {
      if(lGetString(event, ET_strkey) == NULL) {
         lList *order_list = NULL;
         lListElem *job, *ja_task;
         u_long32 job_id, ja_task_id;
         dstring id_dstring = DSTRING_INIT;

         job_id = lGetUlong(event, ET_intkey);
         ja_task_id = lGetUlong(event, ET_intkey2);

         DPRINTF(("got final usage for job %s\n", 
                  job_get_id_string(job_id, ja_task_id, NULL, &id_dstring)));
         sge_dstring_free(&id_dstring);

         job = job_list_locate(*object_type_get_master_list(SGE_TYPE_JOB), job_id);
         ja_task = job_search_task(job, NULL, ja_task_id);

         order_list = sge_create_orders(order_list, ORT_remove_job, job, ja_task, NULL, false);
         sge_send_orders2master(evc_context, &order_list);
      }
   }
   
   DEXIT;
   return true;
}

static void print_load_value(lListElem *host, const char *name, const char *format)
{
   const char *value = host_get_load_value(host, name);
   if(value == NULL) {
      value = "-";
   }

   printf(format, value);
}

/* example how to retrieve information about execution hosts
 * to get values for oslevel, disk_total and disk_free
 * configure the loadsensor util/resources/loadsensors/maui.sh
 */
static void get_cluster_info()
{
   lListElem *host;

   printf("\n%-20s %-10s %-10s %8s %12s %12s %12s %12s %12s %12s %12s\n", 
          "host", "arch", "oslevel", "num_proc", "np_load_avg", 
          "mem_total", "mem_free", "swap_total", "swap_free", "disk_total", "disk_free");

   /* loop over all exec hosts and output information */
   for_each(host, *object_type_get_master_list(SGE_TYPE_EXECHOST)) {
      const char *name = lGetHost(host, EH_name);
      printf("%-20s", name);

      print_load_value(host, "arch", " %-10s");

      /* configure a host complex attribute "oslevel" and retrieve it by a load sensor */
      print_load_value(host, "oslevel", " %-10s");
      print_load_value(host, "num_proc", " %8s");
      print_load_value(host, "np_load_avg", " %12s");
      print_load_value(host, "mem_total", " %12s");
      print_load_value(host, "mem_free", " %12s");
      print_load_value(host, "swap_total", " %12s");
      print_load_value(host, "swap_free", " %12s");

      /* configure host complex attributes "disk_*" and retrieve them by a load sensor */
      print_load_value(host, "disk_total", " %12s");
      print_load_value(host, "disk_free", " %12s");
      printf("\n");
   }
   printf("\n");
}

static void get_workload_info()
{
   lListElem *job;
   const char *hformat = "%10s %-10s %-10s %-10s %7s %7s %15s";
   const char *dformat = "%10s %-10s %-10s %-10s %7d %7.0f %15s";
   dstring id_dstring = DSTRING_INIT;

   printf(hformat, "job id", "user", "group", "pe", "procs", "wclock", "start_time");
   printf("\n");

   /* loop over all jobs and output information */
   for_each(job, *object_type_get_master_list(SGE_TYPE_JOB)) {
      u_long32 job_id, procs;
      const char *user, *group, *pe;
      lListElem *ja_task, *range;
      time_t start_time;
      double wclock = 0.0;

      /* general info */
      job_id = lGetUlong(job, JB_job_number);
      user   = lGetString(job, JB_owner);
      group  = lGetString(job, JB_group);
#if 0      
      /* TODO: this doesnt work anymore */
      ep     = queue_or_job_get_states(job, "h_rt");
      if(ep != NULL) {
         wclock = lGetDouble(ep, CE_doubleval);
      }
#endif      

      /* parallel processing.
       * SGE supports (multiple) ranges, build maximum
       */
      pe = lGetString(job, JB_pe);
      procs = 1;
      for_each(range, lGetList(job, JB_pe_range)) {
         procs = MAX(procs, lGetUlong(range, RN_max));
      }
      
      /* output running tasks.
       * do not show tasks that have status FINISHED but are still in the list
       */
      for_each(ja_task, lGetList(job, JB_ja_tasks)) {
         u_long32 ja_task_id;

         ja_task_id = lGetUlong(ja_task, JAT_task_number);
         start_time = lGetUlong(ja_task, JAT_start_time);
         if(lGetUlong(ja_task, JAT_status) != JFINISHED) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }

      /* for not yet started tasks, set start_time to the requested start time */
      start_time = lGetUlong(job, JB_execution_time);

      /* output tasks without hold
       * these are the tasks to be scheduled
       */
      for_each(range, lGetList(job, JB_ja_n_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }
      /* output pending tasks with user hold */
      for_each(range, lGetList(job, JB_ja_u_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }
      /* output pending tasks with system hold */
      for_each(range, lGetList(job, JB_ja_s_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }
      /* output pending tasks with operator hold */
      for_each(range, lGetList(job, JB_ja_o_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }
      /* output pending tasks with array hold */
      for_each(range, lGetList(job, JB_ja_a_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            printf(dformat, job_get_id_string(job_id, ja_task_id, NULL, &id_dstring), 
                   user, group, 
                   (pe == NULL ? "-" : pe), procs, wclock,
                   start_time > 0 ? ctime(&start_time) : "-\n");
         }
      }
   }

   printf("\n");
   sge_dstring_free(&id_dstring);
}

static void get_policy_info()
{

   printf("%-15s %7s %-15s %s\n", "pe", "procs", "rule", "hosts");

#if 0
   lListElem *pe;

   /* output information for all parallel environments */
   for_each(pe, *object_type_get_master_list(SGE_TYPE_PE)) {
      const char *name, *allocation_rule;
      int procs; 
      lListElem *queue_ref;
      dstring hosts = DSTRING_INIT;
      const char *host_string;
      lList *host_list = NULL;
      lListElem *host;

      /* general information */
      name = lGetString(pe, PE_name);
      procs = lGetUlong(pe, PE_slots);
      allocation_rule = lGetString(pe, PE_allocation_rule);

      /* TODO: PE <-> Queue relation is stored in Queue object */
      /* build a hostslist.
       * SGE pe's have a queuelist which may contain the keyword "all".
       * get the hostnames from the queues.
       */
      for_each(queue_ref, lGetList(pe, PE_queue_list)) {
         lListElem *queue;

         const char *qname = lGetString(queue_ref, QR_name);
         
         if(strcmp(qname, "all") == 0) {  
            /* TODO: CQ_Type not QU_Type */
            for_each(queue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
               const char *host_name = lGetHost(queue, QU_qhostname);
               if(lGetElemStr(host_list, STR, host_name) == NULL) {
                  lAddElemStr(&host_list, STR, host_name, ST_Type);
               }
            }
         } else {
            const char *host_name = lGetHost(lGetElemStr(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), QU_full_name, qname), QU_qhostname);
            if(lGetElemStr(host_list, STR, host_name) == NULL) {  
               lAddElemStr(&host_list, STR, host_name, ST_Type);
            }
         }
      }
      /* build a string containing all hosts */
      for_each(host, host_list) {
         sge_dstring_append(&hosts, lGetString(host, STR));
         sge_dstring_append(&hosts, " ");
      }

      /* output info */
      host_string = sge_dstring_get_string(&hosts);
      printf("%-15s %7d %-15s %s\n", name, procs, allocation_rule, host_string == NULL ? "-" : host_string);

      /* free allocated memory */
      sge_dstring_free(&hosts);
      lFreeList(&host_list);
   }
#endif

}

static bool find_pending_ja_task(lListElem **job, lListElem **ja_task) {
#if 0
   lListElem *sjob;

   /* find a pending job */
   for_each(sjob, *object_type_get_master_list(SGE_TYPE_JOB)) {
      lListElem *range;

      /* find non enrolled, pending ja_task_id */
      for_each(range, lGetList(sjob, JB_ja_n_h_ids)) {
         u_long32 ja_task_id, range_min, range_max, range_step;
         
         range_get_all_ids(range, &range_min, &range_max, &range_step);
         
         for(ja_task_id = range_min; ja_task_id <= range_max; ja_task_id += range_step) {
            if(lGetElemUlong(lGetList(sjob, JB_ja_tasks), JAT_task_number, ja_task_id) == NULL) {
               *job = sjob;
               *ja_task = job_get_ja_task_template_pending(sjob, ja_task_id); 
               object_delete_range_id(sjob, NULL, JB_ja_n_h_ids, ja_task_id);
               return true;
            }
         }
      }   
   }
#endif   

   /* no pending job found */
   return false;
}

#if 0
static int queue_get_free_slots(lListElem *queue)
{
   int slots = 0;
   const char *name;
   lListElem *job;

   name  = lGetString(queue, QU_full_name);
   slots = lGetUlong(queue, QU_job_slots);

   for_each(job, object_type_get_master_list_mt(SGE_TYPE_JOB)) {
      lListElem *ja_task;

      for_each(ja_task, lGetList(job, JB_ja_tasks)) {
         lListElem *granted_queue;

         for_each(granted_queue, lGetList(ja_task, JAT_granted_destin_identifier_list)) {
            if(strcmp(name, lGetString(granted_queue, JG_qname)) == 0) {
               slots -= lGetUlong(granted_queue, JG_slots);
            }
         }
      }
   }

   return slots;
}

static void allocate_queue_slots(lList **allocated_queues, lListElem *queue, u_long32 *procs)
{
   int queue_free_slots;

   queue_free_slots = queue_get_free_slots(queue);
   if(queue_free_slots > 0) {
      int slots;
      lListElem *granted_queue;
      const char *queue_name;

      queue_name = lGetString(queue, QU_full_name);
      DPRINTF(("found %d slots in queue %s\n", queue_free_slots, queue_name));

      slots = MIN(*procs, queue_free_slots);
      *procs -= slots;
      DPRINTF(("allocating %d slots in queue %s, still %d slots to allocate\n", 
               slots, queue_name, *procs));

      granted_queue = lAddElemStr(allocated_queues, JG_qname, lGetString(queue, QU_full_name), 
                                  JG_Type);
      lSetHost(granted_queue, JG_qhostname, lGetHost(queue, QU_qhostname));
      lSetUlong(granted_queue, JG_slots, slots);
   }
}
#endif

static void simple_scheduler(sge_evc_class_t *evc)
{
   lListElem *job, *ja_task;
   const char *pe_name;
   lListElem *pe = NULL;
   u_long32 procs = 1;
   lList *allocated_queues = NULL; /* JG_Type */
  
   DENTER(TOP_LAYER, "simple_scheduler");

   /* find a pending job */
   if(!find_pending_ja_task(&job, &ja_task)) {
      return;
   }

   /* parallel processing.
    * SGE supports (multiple) ranges, build maximum
    */
   pe_name = lGetString(job, JB_pe);
   if(pe_name != NULL) {
      lListElem *range;
      pe = lGetElemStr(*object_type_get_master_list(SGE_TYPE_PE), PE_name, pe_name);
      for_each(range, lGetList(job, JB_pe_range)) {
         procs = MAX(procs, lGetUlong(range, RN_max));
      }
   }
   
   DPRINTF(("jobs requests %d slots in parallel environment %s\n",
            procs,
            pe_name != NULL ? pe_name : "-"));

#if 0 /* TODO: PE <-> Queue relation is stored in Queue object */
   /* allocate free slots
    * if no parallel environment is given or the pe contains the "all" keyword 
    * in the queue list, consider all queues.
    * If a pe is given and it contains a queue list, consider only these queues.
    */
   if(pe == NULL || 
      strcmp("all", lGetString(lFirst(lGetList(pe, PE_queue_list)), QR_name)) == 0) {
      /* find suited queue(s) */
      for_each(queue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
         allocate_queue_slots(&allocated_queues, queue, &procs);
         if(procs <= 0) {
            break;
         }
      }
   } else {
      lListElem *queue_ref;
      for_each(queue_ref, lGetList(pe, PE_queue_list)) {
         lListElem *queue = lGetElemStr(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), QU_full_name, lGetString(queue_ref, QR_name));
         if(queue != NULL) {
            allocate_queue_slots(&allocated_queues, queue, &procs);
            if(procs <= 0) {
               break;
            }
         }
      }
   }
#endif

   /* if all requested slots could be granted, procs should be 0 */
   if(procs > 0) {
      DPRINTF(("job could not be scheduled\n"));
      lFreeList(&allocated_queues);
      DEXIT;
      return;
   }

   /* schedule job:
    * create a task_map and call sge_ssi_job_start
    */
   if(allocated_queues != NULL) {
      int num_allocated_queues = lGetNumberOfElem(allocated_queues);
      if(num_allocated_queues > 0) {
         task_map *map;
         char id[100];
         lListElem *queue;
         int i = 0;
    
         map = (task_map *)malloc((num_allocated_queues + 1) * sizeof(task_map));
    
         for_each(queue, allocated_queues) {
            map[i].procs = lGetUlong(queue, JG_slots);
            map[i].host_name = lGetHost(queue, JG_qhostname);
            i++;
         }   

         map[num_allocated_queues].procs = 0;
         map[num_allocated_queues].host_name = NULL;

         sprintf(id, sge_U32CFormat"."sge_U32CFormat, sge_u32c(lGetUlong(job, JB_job_number)), sge_u32c(lGetUlong(ja_task, JAT_task_number)));

         sge_ssi_job_start(evc_context, id, pe_name, map);

         sge_free(&map);
      }   
   }

   DEXIT;
}

#if 0
static void delete_some_jobs(sge_evc_class_t *evc)
{
   /* delete jobs running longer than 2 minutes
    * to test the sge_ssi_job_cancel function 
    */
   lListElem *job; 
   u_long32 now = sge_get_gmt();

   for_each(job, *object_type_get_master_list(SGE_TYPE_JOB)) {
      lListElem *ja_task;
      for_each(ja_task, lGetList(job, JB_ja_tasks)) {
         if((lGetUlong(ja_task, JAT_start_time) + 120) < now) {
            char id[100];
            sprintf(id, sge_U32CFormat"."sge_U32CFormat, 
                    sge_u32c(lGetUlong(job, JB_job_number)), sge_u32c(lGetUlong(ja_task, JAT_task_number)));
            sge_ssi_job_cancel(evc_context, id, false);
         }
      }
   }
}
#endif

static void register_scheduler(sge_evc_class_t *evc)
{
   DENTER(TOP_LAYER, "register_scheduler");

   /* initialize mirroring interface */
   sge_mirror_initialize(evc, EV_ID_SCHEDD, "simple_scheduler", true);
   sge_mirror_subscribe(evc, SGE_TYPE_ALL, NULL, NULL, NULL, NULL, NULL); 

   /* we have to explicitly remove finished jobs from qmaster
    */
   sge_mirror_subscribe(evc, SGE_TYPE_JOB, remove_finished_job, NULL, NULL, NULL, NULL);

   DEXIT;
}

int main(int argc, char *argv[])
{
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_evc_class_t *evc = NULL;

   DENTER_MAIN(TOP_LAYER, "simple_scheduler");

   /* setup signal handlers */
   sge_setup_sig_handlers(QSCHED);

   if (sge_gdi2_setup(&ctx, SCHEDD, &alp) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT(&ctx, 1);
   }

   if (ctx && (ctx->reresolve_qualified_hostname(ctx) != CL_RETVAL_OK)) {
      SGE_EXIT(&ctx, 1);
   }   

   evc = sge_evc_class_create(ctx, EV_ID_SCHEDD, &alp, false); 

   if (evc == NULL) {
      answer_list_output(&alp);
      SGE_EXIT(&ctx, 1);
   }

   register_scheduler(evc);

   /* do processing until shutdown is requested */
   while(!shut_me_down) {
      /* get data */
      sge_mirror_process_events(evc, event_client);

      if(!shut_me_down) {
         /* output some info */
         get_cluster_info();
         get_policy_info();
         get_workload_info();

         /* schedule jobs -> test sge_ssi_job_start(evc) */
         simple_scheduler(evc);
/*          sleep(60); */
         
         /* test sge_ssi_job_cancel(evc) */
/*          delete_some_jobs(evc); */
      }
   }
   
   DEXIT;
   return EXIT_SUCCESS;
}
