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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uti/sge_rmon.h"
#include "uti/sge_profiling.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_prog.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/parse.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance_state.h"

#include "comm/commlib.h"

#include "sched/msg_schedd.h"
#include "sched/sgeee.h"

#include "evc/sge_event_client.h"

#include "sge.h"
#include "sge_sched_job_category.h"
#include "sge_sched_prepare_data.h"
#include "sge_sched_process_events.h"

static const int cqueue_field_ids[] = {
   CQ_name,
   CQ_hostlist,
   CQ_qinstances,
   CQ_consumable_config_list,
   CQ_projects,
   CQ_xprojects,
   CQ_acl,
   CQ_xacl,
   CQ_qtype,
   CQ_pe_list,
   CQ_nsuspend,
   CQ_job_slots,
   CQ_calendar,
   CQ_h_core,
   CQ_h_cpu,
   CQ_h_data,
   CQ_h_fsize,
   CQ_h_rss,
   CQ_h_rt,
   CQ_h_stack,
   CQ_h_vmem,
   CQ_min_cpu_interval,
   CQ_rerun,
   CQ_s_core,
   CQ_s_cpu,
   CQ_s_data,
   CQ_s_fsize,
   CQ_s_rt,
   CQ_s_rss,
   CQ_s_stack,
   CQ_s_vmem,
   NoName
};

static const int qinstance_field_ids[] = {
   QU_full_name,
   QU_qhostname,
   QU_tag,
   QU_qname,
   QU_acl,
   QU_xacl,
   QU_projects,
   QU_xprojects,
   QU_resource_utilization,
   QU_job_slots,
   QU_load_thresholds,
   QU_suspend_thresholds,
   QU_host_seq_no,
   QU_seq_no,
   QU_state,
   QU_tagged4schedule,
   QU_nsuspend,
   QU_suspend_interval,
   QU_consumable_config_list,
   QU_available_at,
   QU_soft_violation,
   QU_version,
   QU_subordinate_list,

   QU_qtype,
   QU_calendar,
   QU_s_rt,
   QU_h_rt,
   QU_s_cpu,
   QU_h_cpu,
   QU_s_fsize,
   QU_h_fsize,
   QU_s_data,
   QU_h_data,
   QU_s_stack,
   QU_h_stack,
   QU_s_core,
   QU_h_core,
   QU_s_rss,
   QU_h_rss,
   QU_s_vmem,
   QU_h_vmem,
   QU_min_cpu_interval,

   QU_suspended_on_subordinate,
   QU_last_suspend_threshold_ckeck,
   QU_pe_list,
   QU_ckpt_list,

   QU_state_changes,

   NoName
};

static const int job_nm[] = {
   JB_job_number,
   JB_category,
   JB_hard_queue_list,
   JB_owner,
   JB_hard_resource_list,
   JB_group,
   JB_ja_n_h_ids,
   JB_soft_resource_list,
   JB_ja_template,
   JB_soft_queue_list,
   JB_type,
   JB_ja_u_h_ids,
   JB_ja_s_h_ids,
   JB_ja_o_h_ids,
   JB_ja_a_h_ids,
   JB_pe,
   JB_project,
   JB_department,
   JB_execution_time,
   JB_override_tickets,
   JB_jid_predecessor_list,
   JB_deadline,
   JB_submission_time,
   JB_checkpoint_name,
   JB_version,
   JB_priority,
   JB_host,
   JB_ja_structure,
   JB_jobshare,
   JB_master_hard_queue_list,
   JB_pe_range,
   JB_nppri,
   JB_urg,
   JB_nurg,
   JB_dlcontr,
   JB_wtcontr,
   JB_rrcontr,
   JB_soft_wallclock_gmt,
   JB_hard_wallclock_gmt,
   JB_reserve,
   JB_ja_tasks,
   JB_ar,
   JB_ja_task_concurrency,
   NoName
};

static const int jat_nm[] = {
   JAT_task_number,
   JAT_tix,
   JAT_state,
   JAT_fshare,
   JAT_status,
   JAT_granted_pe,
   JAT_scaled_usage_list,
   JAT_task_list,
   JAT_start_time,
   JAT_hold,
   JAT_granted_destin_identifier_list,
   JAT_master_queue,
   JAT_oticket,
   JAT_fticket,
   JAT_sticket,
   JAT_share,
   JAT_prio,
   JAT_ntix,
   JAT_wallclock_limit,
   NoName
};

static const int pet_nm[] = {
   PET_id,
   PET_status,
   PET_granted_destin_identifier_list,
   PET_usage,
   PET_scaled_usage,
   PET_previous_usage,
   NoName
};

static const int eh_nm[] = {
   EH_name,
   EH_scaling_list,
   EH_consumable_config_list,
   EH_usage_scaling_list,
   EH_load_list,
   EH_acl,
   EH_xacl,
   EH_prj,
   EH_xprj,
   EH_sort_value,
   EH_load_correction_factor,
   EH_seq_no,
   EH_resource_utilization,
   EH_reschedule_unknown_list,
   NoName
};

static const int pe_nm[] = {
   PE_name,
   PE_slots,               
   PE_user_list,          
   PE_xuser_list,         
   PE_allocation_rule,    
   PE_control_slaves,
   PE_resource_utilization,
   PE_urgency_slots,
#ifdef SGE_PQS_API
   PE_qsort_args,
#endif
   NoName
};


void 
ensure_valid_what_and_where(sge_where_what_t *where_what) 
{
   lDescr *tmp_what_descr = NULL;

   DENTER(GDI_LAYER, "ensure_valid_what_and_where");

   /* prepare temp data used to create new lists with partial descriptor */
   if (where_what->what_queue2 == NULL || where_what->where_queue2 == NULL ||
       where_what->what_queue == NULL || where_what->where_queue == NULL) {
      int n = 0;
      int index = 0;
      lEnumeration *tmp_what_queue = NULL;

      tmp_what_queue = lIntVector2What(QU_Type, qinstance_field_ids);
      n = lCountWhat(tmp_what_queue, QU_Type);
      tmp_what_descr = sge_malloc(sizeof(lDescr) * (n + 1));
      lPartialDescr(tmp_what_queue, QU_Type, tmp_what_descr, &index);

      lFreeWhat(&tmp_what_queue); 
   }

   /* acl */
   if (where_what->where_acl == NULL) {
      where_what->where_acl = lWhere("%T(%I m= %u)", US_Type, US_type, US_ACL);
   }
   if (where_what->what_acldept == NULL) {
      where_what->what_acldept = lWhat("%T(ALL)", US_Type);
   }

   /* cqueues */
   if (where_what->what_cqueue == NULL) {
      where_what->what_cqueue = lIntVector2What(CQ_Type, cqueue_field_ids);
   }

   /* departments */
   if (where_what->where_dept == NULL) {
      where_what->where_dept = lWhere("%T(%I m= %u)", US_Type, US_type, US_DEPT);
   }

   /* host */
   if (where_what->where_host == NULL) {
      where_what->where_host = lWhere("%T(!(%Ic=%s))", EH_Type, EH_name, SGE_TEMPLATE_NAME);
   }
   if (where_what->what_host == NULL) {
      where_what->what_host = lIntVector2What(EH_Type, eh_nm);
   }

   /* job */
   if (where_what->what_job == NULL) {
      where_what->what_job = lIntVector2What(JB_Type, job_nm);
   }

   /* jat */
   if (where_what->what_jat == NULL) {
      where_what->what_jat = lIntVector2What(JAT_Type, jat_nm);
   }

   /* pet */
   if (where_what->what_pet == NULL) {
      where_what->what_pet = lIntVector2What(PET_Type, pet_nm);
   }

   /* pe */
   if (where_what->what_pe == NULL) {
      where_what->what_pe = lIntVector2What(PE_Type, pe_nm);
   }

   /* qinstances */
   if (where_what->where_queue == NULL) {
      where_what->where_queue = lWhere("%T("
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u))",
            tmp_what_descr,
            QU_state, QI_SUSPENDED,        /* only not suspended queues      */
            QU_state, QI_SUSPENDED_ON_SUBORDINATE,
            QU_state, QI_CAL_SUSPENDED,
            QU_state, QI_ERROR,            /* no queues in error state       */
            QU_state, QI_UNKNOWN,
            QU_state, QI_AMBIGUOUS
            );         /* only known queues              */
   }
   if (where_what->what_queue == NULL) {
      where_what->what_queue  = lIntVector2What(QU_Type, qinstance_field_ids);
   }

   /* qinstances */
   if (where_what->where_queue2 == NULL) {
      where_what->where_queue2 = lWhere("%T("
            "  (%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u) &&"
            " !(%I m= %u))",
            tmp_what_descr,
            QU_state, QI_CAL_SUSPENDED,
            QU_state, QI_CAL_DISABLED,
            QU_state, QI_SUSPENDED,        /* only not suspended queues      */
            QU_state, QI_SUSPENDED_ON_SUBORDINATE,
            QU_state, QI_ERROR,            /* no queues in error state       */
            QU_state, QI_UNKNOWN,
            QU_state, QI_DISABLED,
            QU_state, QI_AMBIGUOUS
            );         /* only known queues              */
   }
   if (where_what->what_queue2 == NULL) {
      where_what->what_queue2 = lWhat("%T(ALL)", tmp_what_descr);
   }
   if (where_what->where_all_queue == NULL) {
      where_what->where_all_queue = lWhere("%T(%I!=%s)", QU_Type,
            QU_qname, SGE_TEMPLATE_NAME); 
   }

   if (tmp_what_descr == NULL ||
       where_what->where_acl == NULL || where_what->what_acldept == NULL || 
       where_what->what_cqueue == NULL || where_what->where_dept == NULL || 
       where_what->where_host == NULL || where_what->what_host == NULL ||
       where_what->what_job == NULL || where_what->what_jat == NULL ||
       where_what->what_pet == NULL || where_what->where_queue == NULL ||
       where_what->what_queue == NULL || where_what->where_queue2 == NULL ||
       where_what->what_queue2 == NULL || where_what->where_all_queue == NULL ||
       where_what->what_pe == NULL) {
      CRITICAL((SGE_EVENT, SFNMAX, MSG_SCHEDD_UNABLE_TO_SETUP_FILTER));
   }
   /* cleanup tmp data */
   if (tmp_what_descr != NULL) {
      cull_hash_free_descr(tmp_what_descr);
      sge_free(&tmp_what_descr);
   }
   DRETURN_VOID;
}

sge_callback_result
sge_process_schedd_conf_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                     sge_event_action action, lListElem *event, void *clientdata)
{
   lListElem *new = NULL;

   DENTER(GDI_LAYER, "sge_process_schedd_conf_event_before");

   DPRINTF(("callback processing schedd config event\n"));

   new = lFirst(lGetList(event, ET_new_version));

   if (new == NULL) {
      ERROR((SGE_EVENT, "> > > > > no scheduler configuration available < < < < <"));
      DEXIT;
      return SGE_EMA_FAILURE;
   }
   /* check for valid load formula */
   {
      lListElem *old = sconf_get_config();
      const char *new_load_formula = lGetString(new, SC_load_formula);
      lList *alpp = NULL;
      lList *master_centry_list = *sge_master_list(object_base, SGE_TYPE_CENTRY);

      if (master_centry_list != NULL &&
          !validate_load_formula(new_load_formula, &alpp, master_centry_list, SGE_ATTR_LOAD_FORMULA)) {

         ERROR((SGE_EVENT,MSG_INVALID_LOAD_FORMULA, new_load_formula ));
         answer_list_output(&alpp);
         if (old) {
            lSetString(new, SC_load_formula, lGetString(old, SC_load_formula) );
         } else {
            lSetString(new, SC_load_formula, "none");
         }
      } else {
         int n = strlen(new_load_formula);

         if (n > 0) {
            char *copy = NULL;

            copy = malloc(n + 1);
            if (copy != NULL) {
               strcpy(copy, new_load_formula);

               sge_strip_blanks(copy);
               lSetString(new, SC_load_formula, copy);
            }

            sge_free(&copy);
         }
      }
      lFreeElem(&old);
   }

   DEXIT;
   return SGE_EMA_OK;
}

sge_callback_result
sge_process_schedd_conf_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                       sge_event_action action, lListElem *event, void *clientdata){
   sconf_print_config();

   if (sconf_is_job_category_filtering()) {
     set_rebuild_categories(true);
   }

   return SGE_EMA_OK;
}

sge_callback_result
sge_process_project_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                    sge_event_action action, lListElem *event, void *clientdata)
{
   const lListElem *new, *old;
   const char *p;

   DENTER(GDI_LAYER, "sge_process_project_event_before");

   if (action != SGE_EMA_ADD &&
       action != SGE_EMA_MOD &&
       action != SGE_EMA_DEL) {
      DEXIT;
      return SGE_EMA_OK;
   }

   p = lGetString(event, ET_strkey);
   new = lFirst(lGetList(event, ET_new_version));
   old = prj_list_locate(*sge_master_list(object_base, SGE_TYPE_PROJECT), p);

   switch (action) {
   case SGE_EMA_ADD:
      if (new != NULL && lGetBool(new, PR_consider_with_categories) == true) {
         set_rebuild_categories(true);
         DPRINTF(("callback before project event: rebuild categories due to SGE_EMA_ADD(%s)\n", p));
      }
      break;
   case SGE_EMA_MOD:
      if (new != NULL && old != NULL && lGetBool(new, PR_consider_with_categories) != lGetBool(old, PR_consider_with_categories)) {
         set_rebuild_categories(true);
         DPRINTF(("callback before project event: rebuild categories due to SGE_EMA_MOD(%s)\n", p));
      }
      break;
   case SGE_EMA_DEL:
      if (old != NULL && lGetBool(old, PR_consider_with_categories) == true) {
         set_rebuild_categories(true);
         DPRINTF(("callback before project event: rebuild categories due to SGE_EMA_DEL(%s)\n", p));
      }
      break;
   default:
      break;
   }

   DEXIT;
   return SGE_EMA_OK;
}

sge_callback_result
sge_process_schedd_monitor_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                 sge_event_action action, lListElem *event, void *clientdata)
{
   DENTER(GDI_LAYER, "sge_process_schedd_monitor_event");
   DPRINTF(("monitoring next scheduler run\n"));
   evc->monitor_next_run = true;
   DRETURN(SGE_EMA_OK);
}

sge_callback_result
sge_process_global_config_event(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                sge_event_action action, lListElem *event, void *clientdata)
{
   DENTER(GDI_LAYER, "sge_process_global_config_event");
   DPRINTF(("notification about new global configuration\n"));
   st_set_flag_new_global_conf(true);
   DEXIT;
   return SGE_EMA_OK;
}

sge_callback_result
sge_process_job_event_before(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                             sge_event_action action, lListElem *event, void *clientdata)
{
   u_long32 job_id = 0;
   lListElem *job = NULL;

   DENTER(GDI_LAYER, "sge_process_job_event_before");
   DPRINTF(("callback processing job event before default rule\n"));

   if (action == SGE_EMA_DEL || action == SGE_EMA_MOD) {
      job_id = lGetUlong(event, ET_intkey);
      job = job_list_locate(*object_type_get_master_list(SGE_TYPE_JOB), job_id);
      if (job == NULL) {
         dstring id_dstring = DSTRING_INIT;
         ERROR((SGE_EVENT, MSG_CANTFINDJOBINMASTERLIST_S,
                job_get_id_string(job_id, 0, NULL, &id_dstring)));
         sge_dstring_free(&id_dstring);
         DRETURN(SGE_EMA_FAILURE);
      }
   } else {
      DRETURN(SGE_EMA_OK);
   }

   switch (action) {
      case SGE_EMA_DEL:
            /* delete job category if necessary */
            sge_delete_job_category(job);
         break;

      case SGE_EMA_MOD:
         switch (lGetUlong(event, ET_type)) {
            case sgeE_JOB_MOD:
               sge_delete_job_category(job);
            break;

            default:
            break;
         }
         break;

      default:
         break;
   }

   DRETURN(SGE_EMA_OK);
}

sge_callback_result
sge_process_job_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                            sge_event_action action, lListElem *event, void *clientdata)
{
   u_long32 job_id = 0;
   lListElem *job  = NULL;

   DENTER(TOP_LAYER, "sge_process_job_event_after");
   DPRINTF(("callback processing job event after default rule\n"));

   if (action == SGE_EMA_ADD || action == SGE_EMA_MOD) {
      job_id = lGetUlong(event, ET_intkey);
      job = job_list_locate(*object_type_get_master_list(SGE_TYPE_JOB), job_id);
      if (job == NULL) {
         dstring id_dstring = DSTRING_INIT;
         ERROR((SGE_EVENT, MSG_CANTFINDJOBINMASTERLIST_S,
                job_get_id_string(job_id, 0, NULL, &id_dstring)));
         sge_dstring_free(&id_dstring);
         DEXIT;
         return SGE_EMA_FAILURE;
      }
      sge_do_priority_job(job); /* job got added or modified, recompute the priorities */
   }

   switch (action) {
      case SGE_EMA_LIST:
         set_rebuild_categories(true);
         sge_do_priority(*object_type_get_master_list(SGE_TYPE_JOB), NULL); /* recompute the priorities */
         break;

      case SGE_EMA_ADD:
         {
            u_long32 start, end, step;

            /* add job category */
            sge_add_job_category(job,
                                 *object_type_get_master_list(SGE_TYPE_USERSET),
                                 *object_type_get_master_list(SGE_TYPE_PROJECT),
                                 *object_type_get_master_list(SGE_TYPE_RQS));

            job_get_submit_task_ids(job, &start, &end, &step);

            if (job_is_array(job)) {
               DPRINTF(("Added job-array "sge_u32"."sge_u32"-"sge_u32":"sge_u32"\n",
                        job_id, start, end, step));
            } else {
               DPRINTF(("Added job "sge_u32"\n", job_id));
            }
         }
         break;


      case SGE_EMA_MOD:
         switch (lGetUlong(event, ET_type)) {
            case sgeE_JOB_MOD:
               /*
               ** after changing the job, read category reference 
               ** for changed job
               */

               sge_add_job_category(job,
                                    *object_type_get_master_list(SGE_TYPE_USERSET),
                                    *object_type_get_master_list(SGE_TYPE_PROJECT),
                                    *object_type_get_master_list(SGE_TYPE_RQS));
               break;

            case sgeE_JOB_FINAL_USAGE:
               {
                  const char *pe_task_id;

                  pe_task_id = lGetString(event, ET_strkey);

                  /* ignore FINAL_USAGE for a pe task here */
                  if (pe_task_id == NULL) {
                     u_long32 ja_task_id;
                     lListElem *ja_task;

                     ja_task_id = lGetUlong(event, ET_intkey2);
                     ja_task = job_search_task(job, NULL, ja_task_id);

                     if (ja_task == NULL) {
                        ERROR((SGE_EVENT, MSG_CANTFINDTASKINJOB_UU,
                               sge_u32c(ja_task_id), sge_u32c(job_id)));
                        DEXIT;
                        return SGE_EMA_FAILURE;
                     }

                     lSetUlong(ja_task, JAT_status, JFINISHED);
                  }
               }
               break;

            case sgeE_JOB_MOD_SCHED_PRIORITY:
               break;

            default:
               break;
         }
         break;

      default:
         break;
   }

   DEXIT;
   return SGE_EMA_OK;
}

/* If the last ja task of a job is deleted, 
 * remove the job category.
 * Do we really need it?
 * Isn't a job delete event sent after the last array task exited?
 */
sge_callback_result
sge_process_ja_task_event_after(sge_evc_class_t *evc, object_description *object_base, sge_object_type type,
                                sge_event_action action, lListElem *event, void *clientdata)
{
   DENTER(GDI_LAYER, "sge_process_ja_task_event_after");

   if (action == SGE_EMA_DEL) {
      lListElem *job;
      u_long32 job_id;
      DPRINTF(("callback processing ja_task event after default rule SGE_EMA_DEL\n"));

      job_id = lGetUlong(event, ET_intkey);
      job = job_list_locate(*sge_master_list(object_base, SGE_TYPE_JOB), job_id);
      if (job == NULL) {
         dstring id_dstring = DSTRING_INIT;
         ERROR((SGE_EVENT, MSG_CANTFINDJOBINMASTERLIST_S,
                job_get_id_string(job_id, 0, NULL, &id_dstring)));
         sge_dstring_free(&id_dstring);
         DEXIT;
         return SGE_EMA_FAILURE;
      }
   } else {
      DPRINTF(("callback processing ja_task event after default rule\n"));
   }

   DEXIT;
   return SGE_EMA_OK;
}

/****** sge_process_events/sge_process_userset_event_before() ******************
*  NAME
*     sge_process_userset_event_before() -- ???
*
*  SYNOPSIS
*     bool sge_process_userset_event_before(sge_object_type type,
*     sge_event_action action, lListElem *event, void *clientdata)
*
*  FUNCTION
*     Determine whether categories need to be rebuilt. Rebuilding
*     categories is necessary, if a userset (a) gets used first
*     time as ACL or (b) is no longer used as ACL. Also categories
*     must be rebuild if entries change with a userset is used as ACL.
*
*  NOTES
*     MT-NOTE: sge_process_userset_event_before() is not MT safe
*******************************************************************************/
sge_callback_result 
sge_process_userset_event_before(sge_evc_class_t *evc, object_description *object_base,
                                 sge_object_type type, sge_event_action action, lListElem *event, void *clientdata)
{
   const lListElem *new, *old;
   const char *u;

   DENTER(GDI_LAYER, "sge_process_userset_event_before");

   if (action != SGE_EMA_ADD &&
       action != SGE_EMA_MOD &&
       action != SGE_EMA_DEL) {
      DEXIT;
      return SGE_EMA_OK;
   }

   u = lGetString(event, ET_strkey);
   new = lFirst(lGetList(event, ET_new_version));
   old = userset_list_locate(*sge_master_list(object_base, SGE_TYPE_USERSET), u);

   switch (action) {
   case SGE_EMA_ADD:
      if (lGetBool(new, US_consider_with_categories) == true) {
         set_rebuild_categories(true);
         DPRINTF(("callback before userset event: rebuild categories due to SGE_EMA_ADD(%s)\n", u));
      }
      break;
   case SGE_EMA_MOD:
      /* need to redo categories if certain changes occur:
         --> it gets used or was used as ACL with queue_conf(5)/host_conf(5)/sge_pe(5)
         --> it is in use as ACL with queue_conf(5)/host_conf(5)/sge_pe(5)
             and a change with users/groups occured */

      if ((lGetBool(new, US_consider_with_categories) != lGetBool(old, US_consider_with_categories))
          || ( lGetBool(old, US_consider_with_categories) == true &&
            object_list_has_differences(lGetList(old, US_entries), NULL, lGetList(new, US_entries), false))) {
         set_rebuild_categories(true);
         DPRINTF(("callback before userset event: rebuild categories due to SGE_EMA_MOD(%s)\n", u));
      }

      break;
   case SGE_EMA_DEL:
      if (lGetBool(old, US_consider_with_categories) == true) {
         set_rebuild_categories(true);
         DPRINTF(("callback before userset event: rebuild categories due to SGE_EMA_DEL(%s)\n", u));
      }
      break;
   default:
      break;
   }

   DEXIT;
   return SGE_EMA_OK;
}

