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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_signal.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_event.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_conf.h"

#include "sched/sge_resource_utilization.h"
#include "sched/sge_serf.h"

#include "sge.h"
#include "sge_event_master.h"
#include "sge_persistence_qmaster.h"
#include "sge_queue_event_master.h"
#include "sge_qinstance_qmaster.h"
#include "sge_subordinate_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_calendar_qmaster.h"
#include "msg_qmaster.h"

typedef struct {
   u_long32 transition;
   long state_mask;
   bool (*has_state)(const lListElem *this_elem);
   bool is;
   bool (*set_state)(lListElem *this_elem, bool set); 
   bool set;
   const char *success_msg;
} change_state_t;

static bool
qinstance_change_state_on_calender_(sge_gdi_ctx_class_t *ctx,
                                    lListElem *qi_elem, u_long32 cal_order, 
                                    lList **state_change_list, monitoring_t *monitor);

bool
qinstance_modify_attribute(sge_gdi_ctx_class_t *ctx,
                           lListElem *this_elem, lList **answer_list,
                           const lListElem *cqueue, 
                           int attribute_name, 
                           int cqueue_attibute_name,
                           int sub_host_name, int sub_value_name,
                           int subsub_key, 
                           const char **matching_host_or_group,
                           const char **matching_group,
                           bool *is_ambiguous, 
                           bool *has_changed_conf_attr, 
                           bool *has_changed_state_attr,
                           const bool initial_modify,
                           bool *need_reinitialize,
                           monitoring_t *monitor)
{
#if 0 /* EB: DEBUG: enable debugging for qinstance_modify_attribute() */
#define QINSTANCE_MODIFY_DEBUG
#endif
   bool ret = true;
  
#ifdef QINSTANCE_MODIFY_DEBUG
   DENTER(TOP_LAYER, "qinstance_modify_attribute");
#else 
   DENTER(BASIS_LAYER, "qinstance_modify_attribute");
#endif

   if (this_elem != NULL && cqueue != NULL && 
      attribute_name != NoName && cqueue_attibute_name != NoName) {
      const char *hostname = lGetHost(this_elem, QU_qhostname);
      const lList *attr_list = lGetList(cqueue, cqueue_attibute_name);
      const lDescr *descr = lGetElemDescr(this_elem);
      int pos = lGetPosInDescr(descr, attribute_name);
      int type = lGetPosType(descr, pos);
      bool value_found = true;

      switch (cqueue_attibute_name) {
         case CQ_calendar:
            {
               const char *old_value = lGetString(this_elem, attribute_name);
               const char *new_value;

               str_attr_list_find_value(attr_list, answer_list,
                                        hostname, &new_value, 
                                        matching_host_or_group,
                                        matching_group, is_ambiguous);
               if (old_value == NULL || new_value == NULL ||
                   strcmp(old_value, new_value)) {
                  lList *master_calendar_list = 
                             *(object_type_get_master_list(SGE_TYPE_CALENDAR)); 
                  lListElem *calendar = 
                         calendar_list_locate(master_calendar_list, new_value);
  
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                           lNm2Str(attribute_name),
                           old_value ? old_value : "<null>",
                           new_value ? new_value : "<null>"));
#endif
                  /* check if the modification is possible or if
                   * an existing AR would be violated by the modification
                   */
                  if (!initial_modify && sge_ar_list_conflicts_with_calendar(answer_list, lGetString(this_elem, QU_full_name),
                                                      calendar, *object_type_get_master_list(SGE_TYPE_AR))) {
                     ret = false;
                     break;
                  }

                  if (calendar != NULL) { 
                     qinstance_change_state_on_calendar(ctx, this_elem, calendar, monitor);
                  } else {
                     sge_qmaster_qinstance_state_set_cal_disabled(this_elem, false);
                     sge_qmaster_qinstance_state_set_cal_suspended(this_elem, false);
                     lSetList(this_elem, QU_state_changes, NULL);
                  }
                  lSetString(this_elem, attribute_name, new_value);
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_qtype:
            {
               u_long32 old_value = lGetUlong(this_elem, attribute_name);
               u_long32 new_value;

               qtlist_attr_list_find_value(attr_list, answer_list, 
                                           hostname, &new_value,
                                           matching_host_or_group,
                                           matching_group,
                                           is_ambiguous);
               if (old_value != new_value) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ" from "sge_u32" to "sge_u32"\n",
                           lNm2Str(attribute_name), old_value, new_value));
#endif
                  lSetUlong(this_elem, attribute_name, new_value);
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_s_fsize:
         case CQ_h_fsize:
         case CQ_s_data:
         case CQ_h_data:
         case CQ_s_stack:
         case CQ_h_stack:
         case CQ_s_core:
         case CQ_h_core:
         case CQ_s_rss:
         case CQ_h_rss:
         case CQ_s_vmem:
         case CQ_h_vmem:
            {
               const char *old_value = lGetString(this_elem, attribute_name);
               const char *new_value = NULL;

               mem_attr_list_find_value(attr_list, answer_list, 
                                        hostname, &new_value, 
                                        matching_host_or_group,
                                        matching_group, is_ambiguous);
               if (old_value == NULL || new_value == NULL ||
                   strcmp(old_value, new_value)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                           lNm2Str(attribute_name),
                           old_value ? old_value : "<null>",
                           new_value ? new_value : "<null>")); 
#endif
                  lSetString(this_elem, attribute_name, new_value);
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_s_rt:
         case CQ_h_rt:
         case CQ_s_cpu:
         case CQ_h_cpu:
            {
               const char *old_value = lGetString(this_elem, attribute_name);
               const char *new_value = NULL;

               time_attr_list_find_value(attr_list, answer_list, 
                                         hostname, &new_value, 
                                         matching_host_or_group,
                                         matching_group,
                                         is_ambiguous);

               if (old_value == NULL || new_value == NULL ||
                   strcmp(old_value, new_value)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                           lNm2Str(attribute_name),
                           old_value ? old_value : "<null>",
                           new_value ? new_value : "<null>")); 
#endif
                  lSetString(this_elem, attribute_name, new_value);
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_suspend_interval:
         case CQ_min_cpu_interval:
         case CQ_notify:
            {
               const char *old_value = lGetString(this_elem, attribute_name);
               const char *new_value = NULL;

               inter_attr_list_find_value(attr_list, answer_list, 
                                          hostname, &new_value, 
                                          matching_host_or_group,
                                          matching_group, is_ambiguous);
               if (old_value == NULL || new_value == NULL ||
                   strcmp(old_value, new_value)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                           lNm2Str(attribute_name),
                           old_value ? old_value : "<null>",
                           new_value ? new_value : "<null>"));
#endif
                  if (attribute_name == QU_suspend_interval &&
                      new_value != NULL) {
                     u_long32 interval;

                     parse_ulong_val(NULL, &interval, TYPE_TIM,
                                     new_value, NULL, 0);
                     if (interval == 0) {
                        /*
                         * Suspend Threshold state will be reset later 
                         */
                        lSetUlong(this_elem, QU_gdi_do_later, GDI_DO_LATER); 
                     }
                  }
                       
                  lSetString(this_elem, attribute_name, new_value);
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_ckpt_list:
         case CQ_pe_list:
            {
               lList *old_value = lGetList(this_elem, attribute_name);
               lList *new_value = NULL;

               strlist_attr_list_find_value(attr_list, answer_list,
                                            hostname, &new_value, 
                                            matching_host_or_group,
                                            matching_group, is_ambiguous);
               if (object_list_has_differences(old_value, answer_list, 
                                               new_value, false)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif
                  /*
                   * check if the modification is possible or if 
                   * an existing AR violates that modification
                   */
                  if (cqueue_attibute_name == CQ_ckpt_list &&
                      ar_list_has_reservation_due_to_ckpt(
                           *object_type_get_master_list(SGE_TYPE_AR), answer_list, 
                           lGetString(this_elem, QU_full_name), new_value)) {
                     ret = false;
                     break;
                  } else if (!initial_modify && cqueue_attibute_name == CQ_pe_list &&
                             ar_list_has_reservation_due_to_pe(
                                 *object_type_get_master_list(SGE_TYPE_AR), answer_list,
                                 lGetString(this_elem, QU_full_name), new_value)) {
                     ret = false;
                     break;
                  }

                  lSetList(this_elem, attribute_name, lCopyList("", new_value));
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_owner_list:
         case CQ_acl:
         case CQ_xacl:
            {
               lList *old_value = lGetList(this_elem, attribute_name);
               lList *new_value = NULL;
   
               usrlist_attr_list_find_value(attr_list, answer_list,
                                            hostname, &new_value, 
                                            matching_host_or_group,
                                            matching_group, is_ambiguous);
               if (object_list_has_differences(old_value, answer_list,
                                               new_value, false)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif
                  lSetList(this_elem, attribute_name, lCopyList("", new_value));
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_projects:
         case CQ_xprojects:
            {
               lList *old_value = lGetList(this_elem, attribute_name);
               lList *new_value = NULL;

               prjlist_attr_list_find_value(attr_list, answer_list,
                                            hostname, &new_value, 
                                            matching_host_or_group,
                                            matching_group, is_ambiguous);
               if (object_list_has_differences(old_value, answer_list,
                                               new_value, false)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif
                  lSetList(this_elem, attribute_name, lCopyList("", new_value));
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_job_slots:
            {
               u_long32 old_value = lGetUlong(this_elem, attribute_name);
               u_long32 new_value;

               ulng_attr_list_find_value(attr_list, answer_list, hostname, 
                                         &new_value, 
                                         matching_host_or_group,
                                         matching_group, is_ambiguous);
               if (old_value != new_value) {
                  int slots_reserved = qinstance_slots_reserved(this_elem);
                  DPRINTF(("reserved slots %d\n", slots_reserved));
                  if (!initial_modify && new_value < slots_reserved) {
                     answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                             MSG_QINSTANCE_SLOTSRESERVED_USS, slots_reserved,
                                             lGetString(this_elem, QU_qname), hostname);
                     ret &= false;
                  } else {
#ifdef QINSTANCE_MODIFY_DEBUG
                     DPRINTF(("Changed "SFQ" from "sge_u32" to "sge_u32"\n",
                              lNm2Str(attribute_name),
                              old_value, new_value));
#endif
                     lSetUlong(this_elem, attribute_name, new_value);
                     *has_changed_conf_attr = true;
                     if (need_reinitialize != NULL) {
                        *need_reinitialize = true;
                     }
                  }
               }
            }
            break;
         case CQ_consumable_config_list:
            {
               lList *old_value = lGetList(this_elem, attribute_name);
               lList *new_value = NULL;
               bool created_new_value = false;
               lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);

               celist_attr_list_find_value(attr_list, answer_list,
                                           hostname, &new_value, 
                                           matching_host_or_group,
                                           matching_group, is_ambiguous);

               if (centry_list_fill_request(new_value, answer_list, master_centry_list, 
                                        true, true, false) == 0) {
                  lListElem *slots_ce = NULL;

                  /* implicit slots entry */
                  if (lGetElemStr(new_value, CE_name, "slots") == NULL) {
                     lList *cq_slots_attr = lGetList(cqueue, CQ_job_slots);
                     u_long32 slots_value;
                     dstring buffer = DSTRING_INIT;

                     ulng_attr_list_find_value(cq_slots_attr, answer_list, hostname, 
                                               &slots_value, 
                                               matching_host_or_group,
                                               matching_group, is_ambiguous);
                     sge_dstring_sprintf(&buffer, sge_u32, slots_value);

                     if (new_value == NULL) {
                        created_new_value = true;
                     }
                     slots_ce = lAddElemStr(&new_value, CE_name, "slots", CE_Type);
                     lSetDouble(slots_ce, CE_doubleval, slots_value);
                     lSetString(slots_ce, CE_stringval, sge_dstring_get_string(&buffer));
                     sge_dstring_free(&buffer);
                  }
                  
                  if (object_list_has_differences(old_value, answer_list,
                                                  new_value, false)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                     DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif
                     if (!initial_modify && ar_list_has_reservation_due_to_qinstance_complex_attr(*object_type_get_master_list(SGE_TYPE_AR), answer_list, 
                                                                                this_elem, *object_type_get_master_list(SGE_TYPE_CENTRY))) {
                        ret = false;
                     } else {
                        if (need_reinitialize != NULL) {
                           *need_reinitialize = true;
                        }

                        lSetList(this_elem, attribute_name, lCopyList("", new_value));
                        *has_changed_conf_attr = true;
                     }
                  }
                  lRemoveElem(new_value, &slots_ce);
               } else {
                   ret &= false;
               }
               if (created_new_value) {
                  lFreeList(&new_value);
               }
            }
            break;
         case CQ_load_thresholds:
         case CQ_suspend_thresholds:
            {
               lList *old_value = lGetList(this_elem, attribute_name);
               lList *new_value = NULL;

               celist_attr_list_find_value(attr_list, answer_list,
                                           hostname, &new_value, 
                                           matching_host_or_group,
                                           matching_group, is_ambiguous);
               if (object_list_has_differences(old_value, answer_list,
                                               new_value, false)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif
                  /*
                   * Suspend Threshold state will be reset later 
                   */
                  if (attribute_name == QU_suspend_thresholds) {
                     lSetUlong(this_elem, QU_gdi_do_later, GDI_DO_LATER);
                  }
   
                  lSetList(this_elem, attribute_name, lCopyList("", new_value));
                  *has_changed_conf_attr = true;
               }
            }
            break;
         case CQ_subordinate_list:
            {
               lList *old_value = NULL;          /* SO_Type list */
               lList *new_value = NULL;          /* SO_Type list */

               old_value = lGetList(this_elem, attribute_name);
               solist_attr_list_find_value(attr_list, answer_list,
                                           hostname, &new_value, 
                                           matching_host_or_group,
                                           matching_group, is_ambiguous);
               if (object_list_has_differences(old_value, answer_list, new_value, false)) {
                  lList *master_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
                  lList *unsuspended_so = NULL;  /* SO_Type list */
                  lList *suspended_so = NULL;    /* SO_Type list */
                  lListElem *first_old_elem = NULL;
                  lListElem *first_new_elem = NULL;
                  bool valid_config = false;

                  /*
                   * check slotwise subordinations
                   */
                  /*
                   * Check if there are loops in the slotwise subordination tree.
                   */
                  lList *old_value_copy = NULL;  /* SO_Type list */
                  lList *new_value_copy = NULL;  /* SO_Type list */

                  old_value_copy = lCopyList("copy_old", old_value);
                  new_value_copy = lCopyList("copy_new", new_value);

                  /*
                   * Find all queues that were slotwise subordinated before
                   * and are no longer subordinated now and trigger them.
                   */
                  lDiffListStr(SO_name, &old_value_copy, &new_value_copy);
                  valid_config = check_new_slotwise_subordinate_tree(this_elem,
                                                    new_value_copy, answer_list);

                  lFreeList(&old_value_copy);
                  lFreeList(&new_value_copy);

                  if (valid_config == false) {
                     ret = false;
                     break;
                  }

                  /*
                   * Detect queues that are no longer subordinated to this queue and
                   * trigger recalculation for them.
                   */
                  hostname = lGetHost(this_elem, QU_qhostname);
                  first_old_elem = lFirst(old_value);
                  first_new_elem = lFirst(new_value);

                  if (first_old_elem != NULL && lGetUlong(first_old_elem, SO_slots_sum) > 0 &&
                      first_new_elem != NULL && lGetUlong(first_new_elem, SO_slots_sum) == 0) {
                      /*
                       * If there was slotwise preemption configured before and there is
                       * either no subordination or queue wise subordination configured
                       * now, unsuspend all tasks. Queue wise subordination will be
                       * calculated in the "classic"  section below.
                       */
                      unsuspend_all_tasks_in_slotwise_sub_tree(ctx, this_elem, monitor);
                  } else {
                     /*
                      * If there was slotwise preemption configured and is still slotwise
                      * preemption configured, unsuspend all task in the dechained queues.
                      * New suspension will be calculated in cqueue_success().
                      */

                     /*
                      * Find all queues that were slotwise subordinated before,
                      * are still slotwise subordinated but have different
                      * seq_no now.
                      * TODO: HP: Make sure these two lDiffListUlong() always
                      *           return the results we expect!
                      */
                     if (do_slotwise_subordinate_lists_differ(old_value, new_value) == true) { 
                        /*
                         * unsuspend all tasks in the whole subtree, new suspends will
                         * be calculated in cqueue_success().
                         */
                        unsuspend_all_tasks_in_slotwise_sub_tree(ctx, this_elem, monitor);
                     }
                  }

                  /*
                   * check "classic" queuewise subordinations
                   */
#ifdef QINSTANCE_MODIFY_DEBUG
                  DPRINTF(("Changed "SFQ"\n", lNm2Str(attribute_name)));
#endif

                  /*
                   * Find list of subordinates that are suspended currently 
                   *
                   * This queue can't have any running jobs and thus can't
                   * subordinate anything if the queue was freshly added
                   */
                  if (initial_modify == false) {
                     qinstance_find_suspended_subordinates(this_elem,
                                                           answer_list,
                                                           &unsuspended_so);
                  }

                  /*
                   * Modify sublist
                   */
                  lSetList(this_elem, attribute_name, lCopyList("", new_value));
                  *has_changed_conf_attr = true;

                  if (initial_modify == false) {
                     /*
                      * Find list of subordinates that have to be suspended after
                      * the modification of CQ_subordinate_list-sublist 
                      */
                     qinstance_find_suspended_subordinates(this_elem,
                                                                  answer_list,
                                                                  &suspended_so);

                     /* 
                      * Remove equal entries in both lists 
                      */
                     lDiffListStr(SO_name, &suspended_so, &unsuspended_so);
                     
                     /*
                      * (Un)suspend subordinated queue instances
                      */
                     cqueue_list_x_on_subordinate_so(ctx,
                                                     master_list, answer_list, 
                                                     false, unsuspended_so, monitor);
                     cqueue_list_x_on_subordinate_so(ctx,
                                                     master_list, answer_list, 
                                                     true, suspended_so, monitor);
                  }

                  /*
                   * Cleanup
                   */
                  lFreeList(&suspended_so);
                  lFreeList(&unsuspended_so);
               }
            }
            break;
         default:
            value_found = false;
            break;
      }

      if (!value_found) {
         switch (type) {
            case lStringT:
               {
                  const char *old_value = lGetString(this_elem, attribute_name);
                  const char *new_value = NULL;

                  str_attr_list_find_value(attr_list, answer_list,
                                           hostname, &new_value, 
                                           matching_host_or_group,
                                           matching_group, is_ambiguous);
                  if (old_value == NULL || new_value == NULL ||
                      strcmp(old_value, new_value)) {
#ifdef QINSTANCE_MODIFY_DEBUG
                     DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                              lNm2Str(attribute_name),
                              old_value ? old_value : "<null>",
                              new_value ? new_value : "<null>"));
#endif
                     lSetString(this_elem, attribute_name, new_value);
                     *has_changed_conf_attr = true;
                  }
               }
               break;
            case lUlongT:
               {
                  u_long32 old_value = lGetUlong(this_elem, attribute_name);
                  u_long32 new_value;

                  ulng_attr_list_find_value(attr_list, answer_list, hostname, 
                                            &new_value, 
                                            matching_host_or_group,
                                            matching_group, is_ambiguous);
                  if (old_value != new_value) {
#ifdef QINSTANCE_MODIFY_DEBUG
                     DPRINTF(("Changed "SFQ" from "sge_u32" to "sge_u32"\n",
                              lNm2Str(attribute_name),
                              old_value, new_value));
#endif
                     lSetUlong(this_elem, attribute_name, new_value);
                     *has_changed_conf_attr = true;
                     if (attribute_name == QU_nsuspend &&
                                new_value == 0) {
                        /*
                         * Suspend Threshold state will be reset later 
                         */

                        lSetUlong(this_elem, QU_gdi_do_later, GDI_DO_LATER);
                     }
                  }
               }
               break;
            case lBoolT:
               {
                  bool old_value = lGetBool(this_elem, attribute_name) ? true : false;
                  bool new_value;

                  bool_attr_list_find_value(attr_list, answer_list,
                                            hostname, &new_value, 
                                            matching_host_or_group,
                                            matching_group, is_ambiguous);
                  if (old_value != new_value) {
#ifdef QINSTANCE_MODIFY_DEBUG
                     DPRINTF(("Changed "SFQ" from "SFQ" to "SFQ"\n",
                              lNm2Str(attribute_name),
                              (old_value ? "true" : "false"),
                              (new_value ? "true" : "false")));
#endif
                     lSetBool(this_elem, attribute_name, new_value);
                     *has_changed_conf_attr = true;
                  }
               }
               break;
            default:
               DPRINTF(("unhandled attribute\n"));
               break;
         }
      }
   }
   DRETURN(ret);
}

bool
qinstance_change_state_on_command(sge_gdi_ctx_class_t *ctx,
                                  lListElem *this_elem, lList**answer_list,
                                  u_long32 transition, bool force_transition,
                                  const char *user, const char *host,
                                  bool is_operator, bool is_owner, monitoring_t *monitor)
{
   bool ret = true;
   dstring buffer = DSTRING_INIT;
   const char *qinstance_name = qinstance_get_name(this_elem, &buffer);
   change_state_t transitions[] = {
      { QI_DO_CLEARERROR,     ~QI_ERROR,     qinstance_state_is_error,            true,  sge_qmaster_qinstance_state_set_error,            false, NULL},
      { QI_DO_ENABLE,         ~QI_DISABLED,  qinstance_state_is_manual_disabled,  true,  sge_qmaster_qinstance_state_set_manual_disabled,  false, NULL},
      { QI_DO_DISABLE,        QI_DISABLED,   qinstance_state_is_manual_disabled,  false, sge_qmaster_qinstance_state_set_manual_disabled,  true, NULL},
      { QI_DO_SUSPEND,        QI_SUSPENDED,  qinstance_state_is_manual_suspended, false, sge_qmaster_qinstance_state_set_manual_suspended, true, NULL},
      { QI_DO_UNSUSPEND,      ~QI_SUSPENDED, qinstance_state_is_manual_suspended, true,  sge_qmaster_qinstance_state_set_manual_suspended, false, NULL},
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
      { QI_DO_SETERROR,       QI_ERROR,      qinstance_state_is_error,            false, sge_qmaster_qinstance_state_set_error,            true, NULL},
      { QI_DO_SETORPHANED,    QI_ORPHANED,   qinstance_state_is_orphaned,         false, sge_qmaster_qinstance_state_set_orphaned,         true, NULL},
      { QI_DO_CLEARORPHANED,  ~QI_ORPHANED,  qinstance_state_is_orphaned,         true,  sge_qmaster_qinstance_state_set_orphaned,         false, NULL},
      { QI_DO_SETUNKNOWN,     QI_UNKNOWN,    qinstance_state_is_unknown,          false, sge_qmaster_qinstance_state_set_unknown,          true, NULL},
      { QI_DO_CLEARUNKNOWN,   ~QI_UNKNOWN,   qinstance_state_is_unknown,          true,  sge_qmaster_qinstance_state_set_unknown,          false, NULL},
      { QI_DO_SETAMBIGUOUS,   QI_AMBIGUOUS,  qinstance_state_is_ambiguous,        false, sge_qmaster_qinstance_state_set_ambiguous,        true, NULL},
      { QI_DO_CLEARAMBIGUOUS, ~QI_AMBIGUOUS, qinstance_state_is_ambiguous,        true,  sge_qmaster_qinstance_state_set_ambiguous,        false, NULL},
#endif
      { QI_DO_NOTHING,        0,             NULL,                                true,  NULL,                                 true, NULL}
   };

   DENTER(TOP_LAYER, "qinstance_change_state_on_command");
   if (is_owner || is_operator) {
      int i = 0;

      while (transitions[i].transition != QI_DO_NOTHING) {
         if (transitions[i].transition == transition) {
            break;
         }
         i++;
      }

      /*
       * Verify current state
       */
      if (transitions[i].has_state(this_elem) == transitions[i].is ||
          force_transition) {
         bool did_something = false;

         DTRACE;

         /*
          * Some transitions need extra work
          */
          switch(transition){
            case QI_DO_SUSPEND : 
                  if ((!qinstance_state_is_susp_on_sub(this_elem) &&
                       !qinstance_state_is_cal_suspended(this_elem)) ||
                      force_transition) {
                     sge_signal_queue(ctx, SGE_SIGSTOP, this_elem, NULL, NULL, monitor);
                     did_something = true;
                  }   
               break;
            case QI_DO_UNSUSPEND :    
                  if (!qinstance_state_is_susp_on_sub(this_elem) &&
                      !qinstance_state_is_cal_suspended(this_elem)) {
                     sge_signal_queue(ctx, SGE_SIGCONT, this_elem, NULL, NULL, monitor);
                     did_something = true;
                  }   
               break;
            case QI_DO_CLEARERROR :
                  qinstance_message_trash_all_of_type_X(this_elem, QI_ERROR);
                  did_something = true;
               break;   
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
            case QI_DO_SETERROR :
                 qinstance_message_add(this_elem, QI_ERROR, "this is a debug message\n");
                 did_something = true;
               break;  
#endif                  
            default:   
                  did_something = true;
         }
      
         /*
          * Change state
          */
         if (did_something) {
            transitions[i].set_state(this_elem, transitions[i].set);
         }

         /*
          * Make changes persistent
          */
         if (did_something) {
            qinstance_increase_qversion(this_elem);
            ret &= sge_event_spool(ctx,
                                   answer_list, 0, sgeE_QINSTANCE_MOD,
                                   0, 0, lGetString(this_elem, QU_qname),
                                   lGetHost(this_elem, QU_qhostname), NULL,
                                   this_elem, NULL, NULL, true, true);

            if (ret) {
               if (force_transition) {
                  INFO((SGE_EVENT, MSG_QINSTANCE_FORCEDSTATE_SSSS,
                        user, host, qinstance_name, 
                        qinstance_state_as_string(transitions[i].state_mask)));
               } else {
                  INFO((SGE_EVENT, MSG_QINSTANCE_CHANGEDST_SSSS,
                        user, host, qinstance_name, 
                        qinstance_state_as_string(transitions[i].state_mask)));
               }
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_OK, ANSWER_QUALITY_INFO);
            } else {
               ERROR((SGE_EVENT, MSG_QINSTANCE_STATENOTMOD_S, qinstance_name));
               answer_list_add(answer_list, SGE_EVENT,
                               STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);

               /*
                * Rollback
                */
               if (!force_transition) {
                  transitions[i].set_state(this_elem, transitions[i].set ? false : true);
               }
            }
         }
      } else {
         INFO((SGE_EVENT, MSG_QINSTANCE_HASSTATE_SS, qinstance_name, 
               qinstance_state_as_string(transitions[i].state_mask)));
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
      }
   } else {
      WARNING((SGE_EVENT, MSG_QINSTANCE_STATENOTMODPERM_S, qinstance_name));
      answer_list_add(answer_list, SGE_EVENT,
                      STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
   }
   sge_dstring_free(&buffer);
   DEXIT;
   return ret;
}


/****** sge_qinstance_qmaster/qinstance_change_state_on_calendar() *************
*  NAME
*     qinstance_change_state_on_calendar() --- changes the state of a given qi (wraper)
*
*  SYNOPSIS
*     bool qinstance_change_state_on_calendar(lListElem *this_elem, const 
*     lListElem *calendar) 
*
*  FUNCTION
*     Changes the state of a given qi based on its calendar.
*
*  INPUTS
*     lListElem *this_elem      - quinstance
*     const lListElem *calendar - calendar
*
*  RESULT
*     bool - state got changed or not
*
*  NOTES
*     MT-NOTE: qinstance_change_state_on_calendar() is MT safe 
*
*******************************************************************************/
bool qinstance_change_state_on_calendar(sge_gdi_ctx_class_t *ctx,
             lListElem *this_elem, const lListElem *calendar, monitoring_t *monitor)
{
   bool ret = true;

   DENTER(TOP_LAYER, "qinstance_change_state_on_calendar");

   if (this_elem != NULL && calendar != NULL) {
      lList *state_changes_list = NULL;
      u_long32 state;
      time_t when = 0; 

      state = calender_state_changes(calendar, &state_changes_list, &when, NULL);

      ret = qinstance_change_state_on_calender_(ctx, this_elem, state, &state_changes_list, monitor);

   }
   DRETURN(ret);
}

/****** sge_qinstance_qmaster/qinstance_change_state_on_calendar_all() *********
*  NAME
*     qinstance_change_state_on_calendar_all() -- changes the state of all qis (wraper)
*
*  SYNOPSIS
*     bool qinstance_change_state_on_calendar_all(const char* cal_name, 
*     u_long32 cal_order, const lList *state_change_list) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     const char* cal_name     - calendar name
*     u_long32 cal_order       - calendar state (todo)
*     const lList *state_change_list - state list for the qis
*
*  RESULT
*     bool - true, if it worked
*
*  NOTES
*     MT-NOTE: qinstance_change_state_on_calendar_all() is not MT safe 
*     Directly access the cluster queue list
*
*******************************************************************************/
bool qinstance_change_state_on_calendar_all(sge_gdi_ctx_class_t *ctx,
                                            const char* cal_name, u_long32 cal_order, 
                                            const lList *state_change_list, monitoring_t *monitor)
{
   bool ret = true;
   lListElem *cqueue;

   DENTER(TOP_LAYER, "qinstance_signal_on_calendar_all");

   for_each (cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance = NULL;

      for_each(qinstance, qinstance_list) {
         const char *queue_calendar = lGetString(qinstance, QU_calendar);

         if (queue_calendar != NULL && !strcmp(queue_calendar, cal_name)) {
            lList *copy_state_change_list = lCopyList("state list", state_change_list);
            ret = qinstance_change_state_on_calender_(ctx, qinstance, cal_order, &copy_state_change_list, monitor);
         }
      }
   }

   DEXIT;
   return ret;
}

/****** sge_qinstance_qmaster/qinstance_change_state_on_calender_() ************
*  NAME
*     qinstance_change_state_on_calender_() -- changes qi state based on calendar
*
*  SYNOPSIS
*     static bool qinstance_change_state_on_calender_(lListElem *this_elem, 
*     u_long32 cal_order, lList **state_change_list) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     lListElem *this_elem     - qi
*     u_long32 cal_order       - next state (order)
*     lList **state_change_list - qi state list
*
*  RESULT
*     static bool - true, if it worked
*
*  NOTES
*     MT-NOTE: qinstance_change_state_on_calender_() is MT safe 
*
*******************************************************************************/
static bool qinstance_change_state_on_calender_(sge_gdi_ctx_class_t *ctx,
                                                lListElem *this_elem, u_long32 cal_order, 
                                                lList **state_change_list, monitoring_t *monitor) 
{
   bool ret = true;
   bool old_cal_disabled = qinstance_state_is_cal_disabled(this_elem);
   bool old_cal_suspended = qinstance_state_is_cal_suspended(this_elem);
   bool new_cal_disabled = (cal_order == QI_DO_CAL_DISABLE) ? true : false;
   bool new_cal_suspended = (cal_order == QI_DO_CAL_SUSPEND) ? true : false;

   DENTER(TOP_LAYER, "qinstance_signal_on_calendar_");

   lSetList(this_elem, QU_state_changes, *state_change_list);
   *state_change_list = NULL;

   if (old_cal_disabled != new_cal_disabled) {
      sge_qmaster_qinstance_state_set_cal_disabled(this_elem, new_cal_disabled);
   }
   
   if (old_cal_suspended != new_cal_suspended) {
      const char *name = lGetString(this_elem, QU_full_name);

      sge_qmaster_qinstance_state_set_cal_suspended(this_elem, new_cal_suspended);
      if (new_cal_suspended) {
         if (qinstance_state_is_susp_on_sub(this_elem)) {
            INFO((SGE_EVENT, MSG_QINSTANCE_NOUSSOS_S, name));
         } else if (qinstance_state_is_manual_suspended(this_elem)) {
            INFO((SGE_EVENT, MSG_QINSTANCE_NOUSADM_S, name));
         } else {
            sge_signal_queue(ctx, SGE_SIGSTOP, this_elem, NULL, NULL, monitor);
         }
      } else {
         if (qinstance_state_is_susp_on_sub(this_elem)) {
            INFO((SGE_EVENT, MSG_QINSTANCE_NOSSOS_S, name));
         } else if (qinstance_state_is_manual_suspended(this_elem)) {
            INFO((SGE_EVENT, MSG_QINSTANCE_NOSADM_S, name));
         } else {
            sge_signal_queue(ctx, SGE_SIGCONT, this_elem, NULL, NULL, monitor);
         }
      }
   }

   qinstance_add_event(this_elem, sgeE_QINSTANCE_MOD);

   DRETURN(ret);
}


bool
sge_qmaster_qinstance_state_set_alarm(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_alarm(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_suspend_alarm(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_suspend_alarm(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_manual_disabled(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_manual_disabled(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
      sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                  lGetString(this_elem, QU_full_name), 
                                  QI_DISABLED,
                                  set_state);
   }


   return changed;
}

bool
sge_qmaster_qinstance_state_set_manual_suspended(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_manual_suspended(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
      sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                  lGetString(this_elem, QU_full_name), 
                                  QI_SUSPENDED,
                                  set_state);
   }


   return changed;
}

bool
sge_qmaster_qinstance_state_set_unknown(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_unknown(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
      if (mconf_get_simulate_execds()) {
         sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                        lGetString(this_elem, QU_full_name), 
                                        QI_UNKNOWN,
                                        false);
      } else {
         sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                        lGetString(this_elem, QU_full_name), 
                                        QI_UNKNOWN,
                                        set_state);
      }
   }


   return changed;
}

bool
sge_qmaster_qinstance_state_set_error(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_error(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
      sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                  lGetString(this_elem, QU_full_name), 
                                  QI_ERROR,
                                  set_state);
   }


   return changed;
}

bool
sge_qmaster_qinstance_state_set_susp_on_sub(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_susp_on_sub(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_cal_disabled(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_cal_disabled(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_cal_suspended(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_cal_suspended(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_orphaned(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_orphaned(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
   }

   return changed;
}

bool
sge_qmaster_qinstance_state_set_ambiguous(lListElem *this_elem, bool set_state)
{
   bool changed;
   changed = qinstance_state_set_ambiguous(this_elem, set_state);
   if (changed) {
      reporting_create_queue_record(NULL, this_elem, sge_get_gmt());
      sge_ar_list_set_error_state(*object_type_get_master_list(SGE_TYPE_AR),
                                  lGetString(this_elem, QU_full_name), 
                                  QI_AMBIGUOUS,
                                  set_state);
   }


   return changed;
}

/* ret: did the state change */
bool
sge_qmaster_qinstance_set_initial_state(lListElem *this_elem)
{
   bool ret = false;
   const char *state_string = lGetString(this_elem, QU_initial_state);

#ifdef QINSTANCE_MODIFY_DEBUG
   DENTER(TOP_LAYER, "sge_qmaster_qinstance_set_initial_state");
#else 
   DENTER(BASIS_LAYER, "sge_qmaster_qinstance_set_initial_state");
#endif

   if (state_string != NULL && strcmp(state_string, "default")) {
      bool do_disable = strcmp(state_string, "disabled") == 0 ? true : false;

      if (do_disable != qinstance_state_is_manual_disabled(this_elem)) {
         ret = true;
         sge_qmaster_qinstance_state_set_manual_disabled(this_elem, do_disable);
      }
   }
   DRETURN(ret);
}

/****** daemons/qmaster/qinstance_reinit_consumable_actual_list() ************
*  NAME
*     qinstance_reinit_consumable_actual_list() -- as it says 
*
*  SYNOPSIS
*     static bool 
*     qinstance_reinit_consumable_actual_list(lListElem *this_elem, 
*                                             lList **answer_list) 
*
*  FUNCTION
*     Reinitialize the consumable actual values. 
*
*  INPUTS
*     lListElem *this_elem - QU_Type element 
*     lList **answer_list  - AN_Type element 
*
*  RESULT
*     static bool - error result
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: qinstance_reinit_consumable_actual_list() is MT safe 
*******************************************************************************/
bool
qinstance_reinit_consumable_actual_list(lListElem *this_elem,
                                        lList **answer_list)
{
   bool ret = true;

   DENTER(TOP_LAYER, "qinstance_reinit_consumable_actual_list");

   if (this_elem != NULL) {
      const char *name = lGetString(this_elem, QU_full_name);
      lList *job_list = *(object_type_get_master_list(SGE_TYPE_JOB));
      lList *centry_list = *(object_type_get_master_list(SGE_TYPE_CENTRY));
      lList *ar_list = *(object_type_get_master_list(SGE_TYPE_AR));
      lListElem *ep = NULL;

      lSetList(this_elem, QU_resource_utilization, NULL);
      qinstance_set_conf_slots_used(this_elem);
      qinstance_debit_consumable(this_elem, NULL, centry_list, 0, true, NULL);

      for_each(ep, job_list) {
         lList *ja_task_list = lGetList(ep, JB_ja_tasks);
         lListElem *ja_task = NULL;

         for_each(ja_task, ja_task_list) {
            lList *gdil = lGetList(ja_task, JAT_granted_destin_identifier_list);
            lListElem *gdil_ep = lGetElemStr(gdil, JG_qname, name);

            if (gdil_ep != NULL) {
               bool is_master_task = false;
               int slots = lGetUlong(gdil_ep, JG_slots);

               if (gdil_ep == lFirst(gdil)) {
                  is_master_task = true;
               }

               if (slots > 0) {
                  qinstance_debit_consumable(this_elem, ep, centry_list, slots, is_master_task, NULL);
               }
            }
         }
      }
      for_each(ep, ar_list) {
         lList *gdil = lGetList(ep, AR_granted_slots);
         lListElem *gdil_ep = lGetElemStr(gdil, JG_qname, name);


         if (gdil_ep != NULL) {
            bool is_master_task = false;
            lListElem *dummy_job = lCreateElem(JB_Type);

            if (gdil_ep == lFirst(gdil)) {
               is_master_task = true;
            }
         
            lSetList(dummy_job, JB_hard_resource_list, lCopyList("", lGetList(ep, AR_resource_list)));

            rc_add_job_utilization(dummy_job, 0, SCHEDULING_RECORD_ENTRY_TYPE_RESERVING,
                                   this_elem, centry_list, lGetUlong(gdil_ep, JG_slots),
                                   QU_consumable_config_list, QU_resource_utilization, name,
                                   lGetUlong(ep, AR_start_time), lGetUlong(ep, AR_duration),
                                   QUEUE_TAG, false, is_master_task);
            lFreeElem(&dummy_job);
         }
      }
   }

   DRETURN(ret);
}
