#ifndef __SGE_QSTAT_H
#define __SGE_QSTAT_H
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

#ifdef  __cplusplus
extern "C" {
#endif

#include "gdi/sge_gdi_ctx.h"

typedef struct qstat_env_str qstat_env_t;

struct qstat_env_str {
   sge_gdi_ctx_class_t *ctx;

   /*  Input parameters */
   lList *resource_list;         /* -l resource_request           */ 
   lList *qresource_list;        /* -F qresource_request          */
   lList* queueref_list;         /* -q queue_list                 */
   lList* peref_list;            /* -pe pe_list                   */
   lList* user_list;             /* -u user_list - selects jobs   */
   lList* queue_user_list;       /* -U user_list - selects queues */
   u_long32 full_listing;        /* -ext      */
   u_long32 qselect_mode;        /* called as qselect */
   u_long32 group_opt;           /* -g        */
   u_long32 queue_state;         /* -qs       */
   u_long32 explain_bits;        /* -explain  */
   u_long32 job_info;            /* -j        */
   u_long32 is_binding_format;   /* -cb       */
   
   /* Needed lists */
   lList* queue_list;
   lList* centry_list;
   lList* exechost_list;
   lList* schedd_config;
   lList* pe_list;
   lList* ckpt_list;
   lList* acl_list;
   lList* zombie_list;
   lList* job_list;
   lList* hgrp_list;
   lList* project_list;
   
   bool need_queues;
   
   int (*shut_me_down)(void);
   u_long32 global_showjobs;
   u_long32 global_showqueues;
   
   /* 
   ** number of slots to be printed in slots column when 0 i
   ** is passed the number of requested slots printed
   */
   int slots_per_line;
   
   /* length of the longest queue name */   
   int longest_queue_length;
   
   
   lEnumeration *what_JB_Type;
   lEnumeration *what_JAT_Type_template;
   lEnumeration *what_JAT_Type_list;
   
};


typedef struct qselect_handler_str qselect_handler_t;

struct qselect_handler_str {
   void *ctx;
   
   int (*report_started)(qselect_handler_t *thiz, lList** alpp);
   int (*report_finished)(qselect_handler_t *thiz, lList** alpp);
   int (*report_queue)(qselect_handler_t *thiz, const char* qname, lList** alpp);
   int (*destroy)(qselect_handler_t *thiz, lList** alpp);
};

int qselect(qstat_env_t* qstat_env, qselect_handler_t *handler, lList **alpp);

/* ------------- Cluster Queue Summary -------------------------------------- */
/* qstat -g c                                                                 */

typedef struct cqueue_summary_str cqueue_summary_t;

struct cqueue_summary_str {
   double load;
   bool   is_load_available;
   u_long32 used;
   u_long32 resv;
   u_long32 total;
   u_long32 temp_disabled; 
   u_long32 available; 
   u_long32 manual_intervention;
   u_long32 suspend_manual; 
   u_long32 suspend_threshold; 
   u_long32 suspend_on_subordinate;
   u_long32 suspend_calendar; 
   u_long32 unknown, load_alarm;
   u_long32 disabled_manual; 
   u_long32 disabled_calendar; 
   u_long32 ambiguous;
   u_long32 orphaned, error;
};

typedef struct cqueue_summary_handler_str cqueue_summary_handler_t;

struct cqueue_summary_handler_str {
   void *ctx;
   qstat_env_t *qstat_env;
   
   int (*report_started)(cqueue_summary_handler_t *thiz, lList **alpp);
   int (*report_finished)(cqueue_summary_handler_t *thiz, lList **alpp);
   
   int (*report_cqueue)(cqueue_summary_handler_t *thiz, const char* cqname, cqueue_summary_t *summary, lList **alpp);
   
   int (*destroy)(cqueue_summary_handler_t *thiz);
};

int qstat_cqueue_summary(qstat_env_t* qstat_env, cqueue_summary_handler_t *handler, lList **alpp);

/* ---------------- QStat queue/job handling ---------------------------------*/

typedef struct queue_summary_str queue_summary_t;

struct queue_summary_str {
   
   const char* queue_type;
   
   u_long32    used_slots;
   u_long32    resv_slots;
   u_long32    total_slots;
   
   const char* arch;
   const char* state;
   
   const char* load_avg_str;
   bool has_load_value;
   bool has_load_value_from_object;
   double load_avg;
   
};

typedef struct job_summary_str job_summary_t;

struct job_summary_str {
   bool print_jobid;
   u_long32 priority;
   double nurg;
   double urg;
   double nppri;
   double nprior;
   double ntckts;
   double rrcontr;
   double wtcontr;
   double dlcontr;
   const char* name;
   const char* user;
   const char* project;
   const char* department;
   char state[8];
   time_t submit_time;
   time_t start_time;
   time_t stop_initiate_time;
   time_t deadline;
   
   bool   has_cpu_usage;
   u_long32 cpu_usage;
   bool   has_mem_usage;
   double mem_usage;
   bool   has_io_usage;
   double io_usage;
   
   bool   is_zombie;
   u_long override_tickets;
   bool   is_queue_assigned;
   u_long tickets;
   u_long otickets;
   u_long ftickets;
   u_long stickets;

   double share;
   const char* queue;
   const char* master;
   u_long32 slots;
   bool is_array;
   bool is_running;
   const char* task_id;
   
};

typedef enum {
   JOB_ADDITIONAL_INFO_ERROR = 0,
   CHECKPOINT_ENV  = 1,
   MASTER_QUEUE    = 2,
   FULL_JOB_NAME   = 3
} job_additional_info_t;

typedef struct task_summary_str task_summary_t;

struct task_summary_str {
   const char* task_id;
   const char* state;
   bool has_cpu_usage;
   double cpu_usage;
   bool has_mem_usage;
   double mem_usage;
   bool has_io_usage;
   double io_usage;
   bool is_running;
   bool has_exit_status;
   u_long32 exit_status;
};

typedef struct job_handler_str job_handler_t;

struct job_handler_str {

  void *ctx;
  qstat_env_t *qstat_env;
  
  int(*report_job)(job_handler_t* handler, u_long32 jid, job_summary_t *summary, lList **alpp);
  
  int (*report_sub_tasks_started)(job_handler_t* handler, lList **alpp);
  int (*report_sub_task)(job_handler_t* handler, task_summary_t *summary, lList **alpp);
  int (*report_sub_tasks_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_additional_info)(job_handler_t *handler, job_additional_info_t name, const char* value, lList **alpp);
  
  int (*report_requested_pe)(job_handler_t *handler, const char* pe_name, const char* pe_range, lList **alpp);
  int (*report_granted_pe)(job_handler_t *handler, const char* pe_name, int pe_slots, lList **alpp);
  
  int (*report_request)(job_handler_t* handler, const char* name, const char* value, lList **alpp);
  
  int (*report_hard_resources_started)(job_handler_t* handler, lList **alpp);
  int (*report_hard_resource)(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
  int (*report_hard_resources_finished)(job_handler_t* handler, lList **alpp);

  int (*report_soft_resources_started)(job_handler_t* handler, lList **alpp);
  /* RH TODO: the soft resource/request has no contribution => remove the parameter uc */
  int (*report_soft_resource)(job_handler_t *handler, const char* name, const char* value, double uc, lList **alpp);
  int (*report_soft_resources_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_hard_requested_queues_started)(job_handler_t *handler, lList **alpp);
  int (*report_hard_requested_queue)(job_handler_t *handler, const char* name, lList **alpp);
  int (*report_hard_requested_queues_finished)(job_handler_t *handler, lList **alpp);
  
  int (*report_soft_requested_queues_started)(job_handler_t *handler, lList **alpp);
  int (*report_soft_requested_queue)(job_handler_t *handler, const char* name, lList **alpp);
  int (*report_soft_requested_queues_finished)(job_handler_t *handler, lList **alpp);
  
  int (*report_master_hard_requested_queues_started)(job_handler_t* handler, lList **alpp);
  int (*report_master_hard_requested_queue)(job_handler_t* handler, const char* name, lList **alpp);
  int (*report_master_hard_requested_queues_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_predecessors_requested_started)(job_handler_t* handler, lList **alpp);
  int (*report_predecessor_requested)(job_handler_t* handler, const char* name, lList **alpp);
  int (*report_predecessors_requested_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_predecessors_started)(job_handler_t* handler, lList **alpp);
  int (*report_predecessor)(job_handler_t* handler, u_long32 jid, lList **alpp);
  int (*report_predecessors_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_ad_predecessors_requested_started)(job_handler_t* handler, lList **alpp);
  int (*report_ad_predecessor_requested)(job_handler_t* handler, const char* name, lList **alpp);
  int (*report_ad_predecessors_requested_finished)(job_handler_t* handler, lList **alpp);
  
  int (*report_ad_predecessors_started)(job_handler_t* handler, lList **alpp);
  int (*report_ad_predecessor)(job_handler_t* handler, u_long32 jid, lList **alpp);
  int (*report_ad_predecessors_finished)(job_handler_t* handler, lList **alpp);

  int (*report_binding_started)(job_handler_t* handler, lList **alpp);
  int (*report_binding)(job_handler_t *handler, const char *binding, lList **alpp);
  int (*report_binding_finished)(job_handler_t* handler, lList **alpp);

  int (*report_job_finished)(job_handler_t* handler, u_long32 jid, lList **alpp);
};

typedef struct qstat_handler_str qstat_handler_t;

struct qstat_handler_str {
  void *ctx;
  qstat_env_t *qstat_env;
  
  int (*report_started)(qstat_handler_t* handler, lList** alpp);
  int (*report_finished)(qstat_handler_t* hanlder, lList** alpp);
  
  int (*report_queue_started)(qstat_handler_t *handler, const char* qname, lList **alpp);
  int (*report_queue_summary)(qstat_handler_t *handler, const char* qname,  queue_summary_t *summary, lList **alpp);
  int (*report_queue_load_alarm)(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
  int (*report_queue_suspend_alarm)(qstat_handler_t* handler, const char* qname, const char* reason, lList **alpp);
  int (*report_queue_message)(qstat_handler_t* handler, const char* qname, const char *message, lList **alpp);
  
  int (*report_queue_resource)(qstat_handler_t* handler, const char* dom, const char* name, const char* value, lList **alpp);
  
  job_handler_t job_handler;
  
  int (*report_queue_jobs_started)(qstat_handler_t *handler, const char* qname, lList **alpp);
  int (*report_queue_jobs_finished)(qstat_handler_t *handler, const char* qname, lList **alpp);

  int (*report_queue_finished)(qstat_handler_t* handler, const char* qname, lList **alpp);

  int (*report_pending_jobs_started)(qstat_handler_t *handler, lList **alpp);
  int (*report_pending_jobs_finished)(qstat_handler_t *handler, lList **alpp);
  int (*report_finished_jobs_started)(qstat_handler_t *handler, lList **alpp);
  int (*report_finished_jobs_finished)(qstat_handler_t *handler, lList **alpp);
  int (*report_error_jobs_started)(qstat_handler_t *handler, lList **alpp);
  int (*report_error_jobs_finished)(qstat_handler_t *handler, lList **alpp);
  int (*report_zombie_jobs_started)(qstat_handler_t *handler, lList **alpp);
  int (*report_zombie_jobs_finished)(qstat_handler_t *handler, lList **alpp);
  
  
  int (*destroy)(qstat_handler_t* handler);
};



int qstat_no_group(qstat_env_t* qstat_env, qstat_handler_t* handler, lList **alpp);


void qstat_env_destroy(qstat_env_t *qstat_env);

const char* sge_get_dominant_stringval(lListElem *rep, u_long32 *dominant_p, dstring *resource_string_p);

int filter_queues(lList **filtered_queue_list,
                  lList *queue_list, 
                  lList *centry_list,
                  lList *hgrp_list,
                  lList *exechost_list,
                  lList *acl_list,
                  lList *prj_list,
                  lList *pe_list,
                  lList *resource_list, 
                  lList *queueref_list, 
                  lList *peref_list, 
                  lList *queue_user_list,
                  u_long32 queue_states,
                  lList **alpp);
                  
lCondition *qstat_get_JB_Type_selection(lList *user_list, u_long32 show);
lEnumeration *qstat_get_JB_Type_filter(qstat_env_t* qstat_env);
void qstat_filter_add_core_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_ext_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_pri_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_urg_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_l_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_q_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_pe_attributes(qstat_env_t* qstat_env);
void qstat_filter_add_r_attributes(qstat_env_t *qstat_env);
void qstat_filter_add_xml_attributes(qstat_env_t *qstat_env);
void qstat_filter_add_t_attributes(qstat_env_t *qstat_env);
void qstat_filter_add_U_attributes(qstat_env_t *qstat_env);

int build_job_state_filter(qstat_env_t *qstat_env, const char* job_state, lList **alpp);

#ifdef  __cplusplus
}
#endif

#endif

