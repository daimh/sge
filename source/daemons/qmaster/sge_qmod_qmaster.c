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
#include <string.h>
#include <limits.h>
#include <fnmatch.h>

#include "uti/sge_rmon.h"
#include "uti/sge_signal.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_hostname.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_string.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_qref.h"

#include "gdi/sge_security.h"
#include "gdi/sge_gdi2.h"

#include "comm/commlib.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "sge_pe_qmaster.h"
#include "sge_queue_event_master.h"
#include "sge_qmod_qmaster.h"
#include "sge_job_qmaster.h"
#include "sge_give_jobs.h"
#include "symbols.h"
#include "reschedule.h"
#include "sge_qinstance_qmaster.h"
#include "sge_cqueue_qmaster.h"
#include "sge_subordinate_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"

/*-------------------------------------------------------------------------*/
static void signal_slave_jobs_in_queue(sge_gdi_ctx_class_t *ctx, int how,
      lListElem *jep, monitoring_t *monitor);

static void signal_slave_tasks_of_job(sge_gdi_ctx_class_t *ctx, int how,
      lListElem *jep, lListElem *jatep, monitoring_t *monitor);

static int sge_change_queue_state(sge_gdi_ctx_class_t *ctx,
                                  char *user, char *host, lListElem *qep,
                                  u_long32 action, u_long32 force, lList **answer,
                                  monitoring_t *monitor);

static int sge_change_job_state(sge_gdi_ctx_class_t *ctx,
                                char *user, char *host, lListElem *jep, lListElem *jatep,
                                u_long32 task_id, u_long32 action, u_long32 force,
                                lList **answer, monitoring_t *monitor);

static int qmod_queue_weakclean(sge_gdi_ctx_class_t *ctx,
                                lListElem *qep, u_long32 force, lList **answer,
                                char *user, char *host, int isoperator,
                                monitoring_t *monitor);

static int qmod_queue_clean(sge_gdi_ctx_class_t *ctx,
                            lListElem *qep, u_long32 force, lList **answer,
                            char *user, char *host, int isoperator, int isowner,
                            monitoring_t *monitor);

static void qmod_job_suspend(sge_gdi_ctx_class_t *ctx,
                             lListElem *jep, lListElem *jatep, lListElem *queueep,
                             u_long32 force, lList **answer, char *user, char *host,
                             monitoring_t *monitor);

static void qmod_job_unsuspend(sge_gdi_ctx_class_t *ctx,
                               lListElem *jep, lListElem *jatep, lListElem *queueep,
                               u_long32 force, lList **answer, char *user, char *host,
                               monitoring_t *monitor);

static void qmod_job_reschedule(sge_gdi_ctx_class_t *ctx,
                                lListElem *jep, lListElem *jatep, lListElem *queueep,
                                u_long32 force, lList **answer, char *user, char *host,
                                monitoring_t *monitor);

/*-------------------------------------------------------------------------*/

void
sge_gdi_qmod(sge_gdi_ctx_class_t *ctx, sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task,
             monitoring_t *monitor)
{
   lList *alp = NULL;
   lListElem *dep;
   lListElem *jatask = NULL, *rn, *job, *tmp_task;
   bool found;
   u_long32 jobid;
   u_long32 start = 0, end = 0, step = 1;
   int alltasks;
   lList *master_hgroup_list = *(object_type_get_master_list(SGE_TYPE_HGROUP));
   lList *cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));
   dstring cqueue_buffer = DSTRING_INIT;
   dstring hostname_buffer = DSTRING_INIT;
   object_description *object_base = NULL;

   DENTER(TOP_LAYER, "sge_gdi_qmod");


   if (!packet->host || (strlen(packet->user) == 0) || !packet->commproc) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(&(task->answer_list), SGE_EVENT,
                      STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      sge_dstring_free(&cqueue_buffer);
      sge_dstring_free(&hostname_buffer);
      DEXIT;
      return;
   }

   object_base = object_type_get_object_description();
   if (sge_chck_mod_perm_host(&(task->answer_list), task->target, packet->host,
                              packet->commproc, 0, NULL, monitor, object_base)) {
      DEXIT;
      return;
   }

   /*
   ** loop over the ids and change queue or job state and signal them
   ** if necessary
   */
   for_each(dep, task->data_list) {
      lList *tmp_list = NULL;
      lList *qref_list = NULL;
      bool found_something = true;
      u_long32 id_action = lGetUlong(dep, ID_action);

      found = false;

      if ((id_action & JOB_DO_ACTION) == 0) {
         qref_list_add(&qref_list, NULL, lGetString(dep, ID_str));
         qref_list_resolve_hostname(qref_list);
         qref_list_resolve(qref_list, NULL, &tmp_list,
                           &found_something, cqueue_list,
                           master_hgroup_list,
                           true, true);
         if (found_something) {
            lListElem *qref = NULL;

            id_action = (id_action & (~QUEUE_DO_ACTION));

            for_each(qref, tmp_list) {
               const char *full_name = NULL;
               const char *cqueue_name = NULL;
               const char *hostname = NULL;
               lListElem *cqueue = NULL;
               lListElem *qinstance = NULL;
               lList *qinstance_list = NULL;

               full_name = lGetString(qref, QR_name);
               if (!cqueue_name_split(full_name, &cqueue_buffer, &hostname_buffer, NULL,
                                 NULL)) {
                  continue;
               }                  
               cqueue_name = sge_dstring_get_string(&cqueue_buffer);
               hostname = sge_dstring_get_string(&hostname_buffer);
               cqueue = lGetElemStr(cqueue_list, CQ_name, cqueue_name);
               qinstance_list = lGetList(cqueue, CQ_qinstances);
               qinstance = lGetElemHost(qinstance_list, QU_qhostname, hostname);

               sge_change_queue_state(ctx, packet->user, packet->host, qinstance,
                     id_action, lGetUlong(dep, ID_force),
                     &alp, monitor);
               found = true;
            }
         }
         lFreeList(&qref_list);
         lFreeList(&tmp_list);
      }
      if (!found) {
         bool is_jobName_suport = false;
         u_long action = lGetUlong(dep, ID_action);
         if ((action & JOB_DO_ACTION) > 0 &&
             (action & QUEUE_DO_ACTION) == 0) {
            action = (action & (~JOB_DO_ACTION));
            is_jobName_suport = true;
         }

         /*
         ** We found no queue so look for a job. This only makes sense for
         ** suspend, unsuspend and reschedule
         */
         if (sge_strisint(lGetString(dep, ID_str)) &&
               (action == QI_DO_SUSPEND ||
                action == QI_DO_RESCHEDULE ||
                action == QI_DO_CLEARERROR ||
                action == QI_DO_UNSUSPEND)) {
            jobid = strtol(lGetString(dep, ID_str), NULL, 10);

            rn = lFirst(lGetList(dep, ID_ja_structure));
            if (rn) {
               start = lGetUlong(rn, RN_min);
               if (start) {
                  end = lGetUlong(rn, RN_max);
                  step = lGetUlong(rn, RN_step);
                  if (!step)
                     step = 1;
                  alltasks = 0;
               } else {
                  start = 1;
                  end = (u_long32)LONG_MAX;
                  step = 1;
                  alltasks = 1;
               }
               if (start > end)
                  end = start;

            } else {
               alltasks = 1;
            }

            job = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), jobid);
            if (job) {
               jatask = lFirst(lGetList(job, JB_ja_tasks));

               while ((tmp_task = jatask)) {
                  u_long32 task_number;

                  jatask = lNext(tmp_task);
                  task_number = lGetUlong(tmp_task, JAT_task_number);
                  if ((task_number >= start && task_number <= end &&
                     ((task_number-start)%step) == 0) || alltasks) {
                     DPRINTF(("Modify job: "sge_u32"."sge_u32"\n", jobid,
                        task_number));

                     /* this specifies no queue, so lets probe for a job */
                     /* change state of job: */
                     sge_change_job_state(ctx, packet->user, packet->host, job, tmp_task, 0,
                         action, lGetUlong(dep, ID_force), &alp, monitor);
                     found = true;
                  }
               }

               /* create more precise GDI answers also for pending jobs/tasks and jobs/tasks in hold state
                  When the operation is to be applied on the whole job array but no task is enrolled so far
                  (i.e. not found) only one single GDI answer is created. Otherwise one message is created
                  per task */
               if (alltasks && job_is_array(job)) {
                  if (!found) {
                     sge_change_job_state(ctx, packet->user, packet->host, job, NULL, 0,
                         action, lGetUlong(dep, ID_force), &alp, monitor);
                     found = true;
                  }
               } else {
                  lListElem *range;
                  u_long32 min, max, step;
                  u_long32 taskid;

                  /* handle all pending tasks */
                  for_each (range, lGetList(job, JB_ja_n_h_ids)) {
                     range_get_all_ids(range, &min, &max, &step);
                     for (taskid=min; taskid<=max; taskid+= step) {
                        if ((taskid >= start && taskid <= end &&
                           ((taskid-start)%step) == 0) || alltasks) {
                           DPRINTF(("Modify job: "sge_u32"."sge_u32"\n", jobid,
                              taskid));
                           sge_change_job_state(ctx, packet->user, packet->host, job, NULL, taskid,
                               action, lGetUlong(dep, ID_force), &alp, monitor);
                           found = true;
                        }
                     }
                  }

                  /* handle all tasks in user hold */
                  for_each (range, lGetList(job, JB_ja_u_h_ids)) {
                     range_get_all_ids(range, &min, &max, &step);
                     for (taskid=min; taskid<=max; taskid+= step) {
                        if ((taskid >= start && taskid <= end &&
                           ((taskid-start)%step) == 0) || alltasks) {
                           DPRINTF(("Modify job: "sge_u32"."sge_u32"\n", jobid,
                                    taskid));
                           sge_change_job_state(ctx, packet->user, packet->host, job, NULL, taskid,
                                                action, lGetUlong(dep, ID_force), &alp, monitor);
                           found = true;
                        }
                     }
                  }

                  /* handle all tasks in system hold that are not in user hold */
                  for_each (range, lGetList(job, JB_ja_s_h_ids)) {
                     range_get_all_ids(range, &min, &max, &step);
                     for (taskid=min; taskid<=max; taskid+= step) {
                        if (range_list_is_id_within(lGetList(job, JB_ja_u_h_ids), taskid)) {
                           continue;
                        }
                        if ((taskid >= start && taskid <= end &&
                           ((taskid-start)%step) == 0) || alltasks) {
                           DPRINTF(("Modify job: "sge_u32"."sge_u32"\n", jobid,
                                    taskid));
                           sge_change_job_state(ctx, packet->user, packet->host, job, NULL, taskid,
                                                action, lGetUlong(dep, ID_force), &alp, monitor);
                           found = true;
                        }
                     }
                  }

                  /* handle all tasks in operator hold that are not in user hold or system hold */
                  for_each (range, lGetList(job, JB_ja_o_h_ids)) {
                     range_get_all_ids(range, &min, &max, &step);
                     for (taskid=min; taskid<=max; taskid+= step) {
                        if (range_list_is_id_within(lGetList(job, JB_ja_u_h_ids), taskid) ||
                            range_list_is_id_within(lGetList(job, JB_ja_s_h_ids), taskid)) {
                           continue;
                        }
                        if ((taskid >= start && taskid <= end &&
                           ((taskid-start)%step) == 0) || alltasks) {
                           DPRINTF(("Modify job: "sge_u32"."sge_u32"\n", jobid,
                                    taskid));
                           sge_change_job_state(ctx, packet->user, packet->host, job, NULL, taskid,
                                                action, lGetUlong(dep, ID_force), &alp, monitor);
                           found = true;
                        }
                     }
                  }
               }
            }
         }
         /* job name or pattern was submitted */
         else if (is_jobName_suport && (
                  action == QI_DO_SUSPEND ||
                  action == QI_DO_RESCHEDULE ||
                  action == QI_DO_CLEARERROR ||
                  action == QI_DO_UNSUSPEND)) {

            const char *job_name = lGetString(dep, ID_str);
            const lListElem *job;
            lListElem *mod = NULL;
            for_each(job, *(object_type_get_master_list(SGE_TYPE_JOB))) {
               if (!fnmatch(job_name, lGetString(job, JB_job_name), 0)) {
                  char job_id[40];
                  mod = lCopyElem(dep);
                  sprintf(job_id, sge_u32, lGetUlong(job, JB_job_number));
                  lSetString(mod, ID_str, job_id);
                  lAppendElem(task->data_list, mod);
                  found = true;
               }
            }
         }
         else {
            /* job id invalid or action invalid for jobs */

         }
      }

      if (!found) {
         u_long action = lGetUlong(dep, ID_action);
/*
         if ((action & JOB_DO_ACTION)) {
            action = action - JOB_DO_ACTION;
         }
*/
         /*
         ** If the action is QI_DO_UNSUSPEND or QI_DO_SUSPEND,
         ** 'invalid queue or job' will be printed,
         ** otherwise 'invalid queue' will be printed, because these actions
         ** are not suitable for jobs.
         */
         if ((action & QUEUE_DO_ACTION) == 0 && (
             (action & JOB_DO_ACTION) != 0 ||
             (action & QI_DO_SUSPEND) != 0 ||
             (action & QI_DO_UNSUSPEND) != 0||
             (action & QI_DO_CLEAN) != 0))
            ERROR((SGE_EVENT, MSG_QUEUE_INVALIDQORJOB_S, lGetString(dep, ID_str)));
         else
            ERROR((SGE_EVENT, MSG_QUEUE_INVALIDQ_S, lGetString(dep, ID_str)));
         answer_list_add(&alp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      }
   }

   sge_dstring_free(&cqueue_buffer);
   sge_dstring_free(&hostname_buffer);

   task->answer_list = alp;

   DEXIT;
}

static int
sge_change_queue_state(sge_gdi_ctx_class_t *ctx,
                       char *user, char *host, lListElem *qep, u_long32 action,
                       u_long32 force, lList **answer, monitoring_t *monitor)
{
   bool isoperator;
   bool isowner;
   int result = 0;
   const char *ehname = lGetHost(qep, QU_qhostname);

   DENTER(TOP_LAYER, "sge_change_queue_state");

   isowner = qinstance_check_owner(qep, user);
   isoperator = manop_is_operator(user);

   if (!isowner) {
      ERROR((SGE_EVENT, MSG_QUEUE_NOCHANGEQPERMS_SS, user, lGetString(qep, QU_full_name)));
      answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   if (qep == NULL) {
      ERROR((SGE_EVENT, MSG_NULLELEMENTPASSEDTO_S, SGE_FUNC));
      DRETURN(-1);
   }

   switch (action) {
      case QI_DO_CLEARERROR:
      case QI_DO_ENABLE:
      case QI_DO_DISABLE:
      case QI_DO_SUSPEND:
      case QI_DO_UNSUSPEND:
#ifdef __SGE_QINSTANCE_STATE_DEBUG__
      case QI_DO_SETERROR:
      case QI_DO_SETORPHANED:
      case QI_DO_CLEARORPHANED:
      case QI_DO_SETUNKNOWN:
      case QI_DO_CLEARUNKNOWN:
      case QI_DO_SETAMBIGUOUS:
      case QI_DO_CLEARAMBIGUOUS:
#endif
         result = qinstance_change_state_on_command(ctx, qep, answer, action, force ? true : false, user, host, isoperator, isowner, monitor) ? 0 : -1;
         break;
      case QI_DO_CLEAN:
         result = qmod_queue_clean(ctx, qep, force, answer, user, host, isoperator, isowner, monitor);
         break;

      case QI_DO_RESCHEDULE:
         result = qmod_queue_weakclean(ctx, qep, force, answer, user,
                                       host, isoperator, monitor);
         break;
      default:
         INFO((SGE_EVENT, MSG_LOG_QUNKNOWNQMODCMD_U, sge_u32c(action)));
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         break;
   }

   switch (action) {
      case QI_DO_SUSPEND:
         /* When the queue gets suspended, the other queues can possibly unsuspend
          * tasks that where suspended by slotwise subordination.
          * Therefore we call the function with "false" (=unsuspend)
          */
         do_slotwise_x_on_subordinate_check(ctx, qep, false, false, monitor);
         break;
      case QI_DO_UNSUSPEND:
         /* This queue gets unsuspended, possibly tasks in other queues have to
          * be suspended because of slotwise subordination.
          * Therefore we call the function with "true" (=suspend)
          */
         do_slotwise_x_on_subordinate_check(ctx, qep, true, false, monitor);
         break;
      case QI_DO_CLEAN:
      case QI_DO_RESCHEDULE:
         cqueue_list_del_all_orphaned(ctx, *(object_type_get_master_list(SGE_TYPE_CQUEUE)), answer,
               lGetString(qep, QU_qname), ehname);
         break;
      default:
         break;
   }

   DEXIT;
   return result;
}

static int sge_change_job_state(
sge_gdi_ctx_class_t *ctx,
char *user,
char *host,
lListElem *jep,
lListElem *jatep,
u_long32 task_id,
u_long32 action,
u_long32 force,
lList **answer,
monitoring_t *monitor
) {
   lListElem *queueep;
   u_long32 job_id;

   DENTER(TOP_LAYER, "sge_change_job_state");

   job_id = lGetUlong(jep, JB_job_number);

   /* check the modifying users permissions */
   if (strcmp(user, lGetString(jep, JB_owner)) && !manop_is_operator(user)) {
      ERROR((SGE_EVENT, MSG_JOB_NOMODJOBPERMS_SU, user, sge_u32c(job_id)));
      answer_list_add(answer, SGE_EVENT, STATUS_ENOTOWNER, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   if (!jatep) {
      /* unenrolled tasks always are not-running pending/hold */
      if (task_id) {
         WARNING((SGE_EVENT, MSG_QMODJOB_NOTENROLLED_UU, sge_u32c(job_id), sge_u32c(task_id)));
      } else {
         WARNING((SGE_EVENT, MSG_QMODJOB_NOTENROLLED_U, sge_u32c(job_id)));
      }   
      answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
      DEXIT;
      return -1;
   }

   task_id = lGetUlong(jatep, JAT_task_number);

   if (lGetString(jatep, JAT_master_queue)) {
      queueep = cqueue_list_locate_qinstance(
                              *(object_type_get_master_list(SGE_TYPE_CQUEUE)),
                              lGetString(jatep, JAT_master_queue));
   } else {
      queueep = NULL;
   }

   switch (action) {
      case QI_DO_RESCHEDULE:
         qmod_job_reschedule(ctx, jep, jatep, queueep, force, answer, user, host, monitor);
         break;

      case JSUSPENDED:
         qmod_job_suspend(ctx, jep, jatep, queueep, force, answer, user, host, monitor);
         if (queueep != NULL) {
            do_slotwise_x_on_subordinate_check(ctx, queueep, false, false, monitor);
         }
         break;

      case JRUNNING:
         qmod_job_unsuspend(ctx, jep, jatep, queueep, force, answer, user, host, monitor);
         if (queueep != NULL) {
            do_slotwise_x_on_subordinate_check(ctx, queueep, true, false, monitor);
         }   
         break;

      case QI_DO_CLEARERROR:
         if (VALID(JERROR, lGetUlong(jatep, JAT_state))) {
            lSetUlong(jatep, JAT_state, lGetUlong(jatep, JAT_state) & ~JERROR);
            ja_task_message_trash_all_of_type_X(jatep, 1);
/* lWriteElemTo(jatep, stderr); */
            sge_event_spool(ctx,
                            answer, 0, sgeE_JATASK_MOD,
                            job_id, task_id, NULL, NULL, NULL,
                            jep, jatep, NULL, true, true);
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_CLEARERRORTASK_SSUU, user, host, sge_u32c(job_id), sge_u32c(task_id)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_CLEARERRORJOB_SSU, user, host, sge_u32c(job_id)));
            }
         } else {
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_NOERRORSTATETASK_UU, sge_u32c(job_id), sge_u32c(task_id)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_NOERRORSTATEJOB_UU, sge_u32c(job_id)));
            }
         }
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         break;

      default:
         INFO((SGE_EVENT, MSG_LOG_JOBUNKNOWNQMODCMD_U, sge_u32c(action)));
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         break;
   }

   DEXIT;
   return 0;
}

/****
 **** qmod_queue_weakclean (static)
 ****/
static int
qmod_queue_weakclean(sge_gdi_ctx_class_t *ctx, lListElem *qep,
                     u_long32 force, lList **answer, char *user,
                     char *host, int isoperator, monitoring_t *monitor) {
   DENTER(TOP_LAYER, "qmod_queue_weakclean");

   if (!isoperator) {
      ERROR((SGE_EVENT, MSG_QUEUE_NORESCHEDULEQPERMS_SS, user,
         lGetString(qep, QU_full_name)));
      answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   reschedule_jobs(ctx, qep, force, answer, monitor, true);

   DEXIT;
   return 0;
}

/****
 **** qmod_queue_clean (static)
 ****
 **** cleans the specified queue (every job will be deleted)
 **** The user will do this via qconf -cq <qname>
 ****/
static int qmod_queue_clean(
sge_gdi_ctx_class_t *ctx,
lListElem *qep,
u_long32 force,
lList **answer,
char *user,
char *host,
int isoperator,
int isowner,
monitoring_t *monitor
) {
   lListElem *nextjep, *jep;
   const char *qname = NULL;
   DENTER(TOP_LAYER, "qmod_queue_clean");

   qname = lGetString(qep, QU_full_name);

   DPRINTF(("cleaning queue >%s<\n", qname ));

   if (!manop_is_manager(user)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_QUEUE_NOCLEANQPERMS));
      answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   /* using sge_commit_job(j, COMMIT_ST_FINISHED_FAILED) q->job_list
      could get modified so we have to be careful when iterating through the job list */
   nextjep = lFirst(*(object_type_get_master_list(SGE_TYPE_JOB)));
   while ((jep=nextjep)) {
      lListElem *jatep, *nexttep;
      nextjep = lNext(jep);

      nexttep = lFirst(lGetList(jep, JB_ja_tasks));
      while ((jatep=nexttep)) {
         nexttep = lNext(jatep);

         if (lGetSubStr(jatep, JG_qname, qname, JAT_granted_destin_identifier_list) != NULL) {
            /* 3: JOB_FINISH reports aborted */
            sge_commit_job(ctx, jep, jatep, NULL, COMMIT_ST_FINISHED_FAILED_EE, COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
         }
      }
   }
   INFO((SGE_EVENT, MSG_QUEUE_PURGEQ_SSS, user, host, qname ));
   answer_list_add(answer, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);

   DRETURN(0);
}

/****
 **** qmod_job_reschedule (static)
 ****/
static void qmod_job_reschedule(
sge_gdi_ctx_class_t *ctx,
lListElem *jep,
lListElem *jatep,
lListElem *queueep,
u_long32 force,
lList **answer,
char *user,
char *host,
monitoring_t *monitor
) {
   DENTER(TOP_LAYER, "qmod_job_reschedule");

   reschedule_job(ctx, jep, jatep, queueep, force, answer, monitor, true);

   DEXIT;
}
/****
 **** qmod_job_suspend (static)
 ****/
static void qmod_job_suspend(
sge_gdi_ctx_class_t *ctx,
lListElem *jep,
lListElem *jatep,
lListElem *queueep,
u_long32 force,
lList **answer,
char *user,
char *host,
monitoring_t *monitor
) {
   int i;
   u_long32 state = 0;
   u_long32 jataskid = 0;
   u_long32 jobid = 0;
   bool migrate_on_suspend = false;
   u_long32 now;

   DENTER(TOP_LAYER, "qmod_job_suspend");

   now = sge_get_gmt();

   jobid = lGetUlong(jep, JB_job_number);
   jataskid = lGetUlong(jatep, JAT_task_number);

   /* determine whether we actually migrate upon suspend */
   if (lGetUlong(jep, JB_checkpoint_attr) & CHECKPOINT_SUSPEND)
      migrate_on_suspend = true;

   if (VALID(JSUSPENDED, lGetUlong(jatep, JAT_state))) {
      /* this job is already suspended or lives in a suspended queue */
      if (force && queueep) {
         /* here force means to send the suspend signal again
            this can only be done if we know the queue this job
            runs in */
         if (sge_signal_queue(ctx, SGE_SIGSTOP, queueep, jep, jatep, monitor)) {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_NOFORCESUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_NOFORCESUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
         else {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_FORCESUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_FORCESUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
      }
      else {
         if (job_is_array(jep)) {
            WARNING((SGE_EVENT, MSG_JOB_ALREADYSUSPENDED_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
         } else {
            WARNING((SGE_EVENT, MSG_JOB_ALREADYSUSPENDED_SU, user, sge_u32c(jobid)));
         }
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
      }

      /*
      ** may be the queue is suspended, than the job might not be
      */
      state = lGetUlong(jatep, JAT_state);
      CLEARBIT(JRUNNING, state);
      SETBIT(JSUSPENDED, state);
      lSetUlong(jatep, JAT_state, state);
      if (migrate_on_suspend)
         lSetUlong(jatep, JAT_stop_initiate_time, now);

      sge_event_spool(ctx,
                      answer, 0, sgeE_JATASK_MOD,
                      jobid, jataskid, NULL, NULL, NULL,
                      jep, jatep, NULL, true, true);
   }
   else {   /* job wasn't suspended yet */
      if (queueep) {
         if ((i = sge_signal_queue(ctx, SGE_SIGSTOP, queueep, jep, jatep, monitor))) {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_NOSUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_NOSUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
      }
      else
         i = 1;

      if (force) {
         /* set jobs state to suspend in all cases */
         if (!i) {
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_FORCESUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_FORCESUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
         }

         state = lGetUlong(jatep, JAT_state);
         CLEARBIT(JRUNNING, state);
         SETBIT(JSUSPENDED, state);
         lSetUlong(jatep, JAT_state, state);
         if (migrate_on_suspend)
            lSetUlong(jatep, JAT_stop_initiate_time, now);
         sge_event_spool(ctx,
                         answer, 0, sgeE_JATASK_MOD,
                         jobid, jataskid, NULL, NULL, NULL,
                         jep, jatep, NULL, true, true);
      }
      else {
         if (!i) {
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_SUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_SUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);

            state = lGetUlong(jatep, JAT_state);
            CLEARBIT(JRUNNING, state);
            SETBIT(JSUSPENDED, state);
            lSetUlong(jatep, JAT_state, state);
            if (migrate_on_suspend)
               lSetUlong(jatep, JAT_stop_initiate_time, now);
            sge_event_spool(ctx,
                            answer, 0, sgeE_JATASK_MOD,
                            jobid, jataskid, NULL, NULL, NULL,
                            jep, jatep, NULL, true, true);
         }
      }
      reporting_create_job_log(NULL, now, JL_SUSPENDED, user, host, NULL, jep, jatep, NULL, NULL);
   }
   DEXIT;
}

/****
 **** qmod_job_unsuspend (static)
 ****/
static void qmod_job_unsuspend(
sge_gdi_ctx_class_t *ctx,
lListElem *jep,
lListElem *jatep,
lListElem *queueep,
u_long32 force,
lList **answer,
char *user,
char *host,
monitoring_t *monitor
) {
   int i;
   u_long32 state = 0;
   u_long32 jobid, jataskid;
   u_long32 now;

   DENTER(TOP_LAYER, "qmod_job_unsuspend");

   now = sge_get_gmt();

   jobid = lGetUlong(jep, JB_job_number);
   jataskid = lGetUlong(jatep, JAT_task_number);

   /* admin suspend may not override suspend from threshold */
   if (VALID(JSUSPENDED_ON_THRESHOLD, lGetUlong(jatep, JAT_state))) {
      if (VALID(JSUSPENDED, lGetUlong(jatep, JAT_state))) {
         if (job_is_array(jep)) {
            INFO((SGE_EVENT, MSG_JOB_RMADMSUSPENDTASK_SSUU, user, host, sge_u32c(jobid), sge_u32c(jataskid)));
         } else {
            INFO((SGE_EVENT, MSG_JOB_RMADMSUSPENDJOB_SSU, user, host, sge_u32c(jobid)));
         }
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);

         state = lGetUlong(jatep, JAT_state);
         CLEARBIT(JSUSPENDED, state);
         lSetUlong(jatep, JAT_state, state);
         sge_event_spool(ctx,
                         answer, 0, sgeE_JATASK_MOD,
                         jobid, jataskid, NULL, NULL, NULL,
                         jep, jatep, NULL, true, true);
         reporting_create_job_log(NULL, now, JL_UNSUSPENDED, user, host, NULL, jep, jatep, NULL, NULL);
         DEXIT;
         return;
      }
      else {
         /* guess admin tries to remove threshold suspension by qmon -us <jobid> */
         if (job_is_array(jep)) {
            WARNING((SGE_EVENT, MSG_JOB_NOADMSUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
         } else {
            WARNING((SGE_EVENT, MSG_JOB_NOADMSUSPENDJOB_SU, user, sge_u32c(jobid)));
         }
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         DEXIT;
         return;
      }
   }

   if (VALID(JRUNNING, lGetUlong(jatep, JAT_state))) {
      /* this job is already running */
      if (force && queueep) {
         /*
         ** here force means to send the cont signal again
         ** this can only be done if we know the queue this job
         ** runs in
         */
         if (sge_signal_queue(ctx, SGE_SIGCONT, queueep, jep, jatep, monitor)) {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_NOFORCEENABLETASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_NOFORCEENABLEJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
         else {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_FORCEENABLETASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_FORCEENABLEJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
      }
      else {
         if (job_is_array(jep)) {
            WARNING((SGE_EVENT, MSG_JOB_ALREADYUNSUSPENDED_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
         } else {
            WARNING((SGE_EVENT, MSG_JOB_ALREADYUNSUSPENDED_SU, user, sge_u32c(jobid)));
         }
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
      }
      /*
      ** job is already running, so no job information has to be changed
      */
   }
   else {   /* job wasn't suspended till now */
      if (queueep) {
         if ((i = sge_signal_queue(ctx, SGE_SIGCONT, queueep, jep, jatep, monitor))) {
            if (job_is_array(jep)) {
               WARNING((SGE_EVENT, MSG_JOB_NOUNSUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               WARNING((SGE_EVENT, MSG_JOB_NOUNSUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
      }
      else
         i = 1;

      if (force) {
         /* set jobs state to suspend in all cases */
         if (!i) {
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_FORCEUNSUSPTASK_SSUU, user, host, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_FORCEUNSUSPJOB_SSU, user, host, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_ERROR);
         }

         state = lGetUlong(jatep, JAT_state);
         SETBIT(JRUNNING, state);
         CLEARBIT(JSUSPENDED, state);
         lSetUlong(jatep, JAT_state, state);
         sge_event_spool(ctx,
                         answer, 0, sgeE_JATASK_MOD,
                         jobid, jataskid, NULL, NULL, NULL,
                         jep, jatep, NULL, true, true);
      }
      else {
         /* set job state only if communication works */
         if (!i) {
            if (job_is_array(jep)) {
               INFO((SGE_EVENT, MSG_JOB_UNSUSPENDTASK_SUU, user, sge_u32c(jobid), sge_u32c(jataskid)));
            } else {
               INFO((SGE_EVENT, MSG_JOB_UNSUSPENDJOB_SU, user, sge_u32c(jobid)));
            }
            answer_list_add(answer, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_ERROR);

            state = lGetUlong(jatep, JAT_state);
            SETBIT(JRUNNING, state);
            CLEARBIT(JSUSPENDED, state);
            lSetUlong(jatep, JAT_state, state);
            sge_event_spool(ctx,
                            answer, 0, sgeE_JATASK_MOD,
                            jobid, jataskid, NULL, NULL, NULL,
                            jep, jatep, NULL, true, true);
         }
      }
   }
   reporting_create_job_log(NULL, now, JL_UNSUSPENDED, user, host, NULL, jep, jatep, NULL, NULL);
   DEXIT;
}


void rebuild_signal_events()
{
   lListElem *cqueue, *jep, *jatep;

   DENTER(TOP_LAYER, "rebuild_signal_events");

   /* J O B */
   for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB)))
   {
      for_each (jatep, lGetList(jep, JB_ja_tasks))
      {
         time_t when = (time_t)lGetUlong(jatep, JAT_pending_signal_delivery_time);

         if (lGetUlong(jatep, JAT_pending_signal) && (when > 0))
         {
            u_long32 key1 = lGetUlong(jep, JB_job_number);
            u_long32 key2 = lGetUlong(jatep, JAT_task_number);
            te_event_t ev = NULL;

            ev = te_new_event(when, TYPE_SIGNAL_RESEND_EVENT, ONE_TIME_EVENT, key1, key2, NULL);
            te_add_event(ev);
            te_free_event(&ev);
         }
      }
   }

   /* Q U E U E */
   for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE)))
   {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance;

      for_each(qinstance, qinstance_list)
      {
         time_t when = (time_t)lGetUlong(qinstance, QU_pending_signal_delivery_time);

         if (lGetUlong(qinstance, QU_pending_signal) && (when > 0))
         {
            const char* str_key = lGetString(qinstance, QU_full_name);
            te_event_t ev = NULL;

            ev = te_new_event(when, TYPE_SIGNAL_RESEND_EVENT, ONE_TIME_EVENT, 0, 0, str_key);
            te_add_event(ev);
            te_free_event(&ev);
         }
      }
   }

   DEXIT;
   return;
} /* rebuild_signal_events() */

/* this function is called by our timer mechanism for resending signals */
void resend_signal_event(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem *qep, *jep, *jatep;
   u_long32 jobid = te_get_first_numeric_key(anEvent);
   u_long32 jataskid = te_get_second_numeric_key(anEvent);
   const char* queue = te_get_alphanumeric_key(anEvent);

   DENTER(TOP_LAYER, "resend_signal_event");

   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   if (queue == NULL) {
      if (!(jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), jobid)) || !(jatep=job_search_task(jep, NULL, jataskid)))
      {
         ERROR((SGE_EVENT, MSG_EVE_RESENTSIGNALTASK_UU, sge_u32c(jobid), sge_u32c(jataskid)));
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         DEXIT;
         return;
      }

      if ((qep = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), lGetString(jatep, JAT_master_queue)))) {
         sge_signal_queue(ctx, lGetUlong(jatep, JAT_pending_signal), qep, jep, jatep, monitor);
      }
   } else {
      if (!(qep = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), queue))) {
         ERROR((SGE_EVENT, MSG_EVE_RESENTSIGNALQ_S, queue));
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         sge_free(&queue);
         DEXIT;
         return;
      }

      sge_signal_queue(ctx, lGetUlong(qep, QU_pending_signal), qep, NULL, NULL, monitor);
   }

   sge_free(&queue);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
   return;
}

static void sge_propagate_queue_suspension(const char *qnm, int how)
{
   lListElem *jep, *jatep;

   DENTER(TOP_LAYER, "sge_propagate_queue_suspension");

   DPRINTF(("searching for all jobs in queue %s due to %s\n", qnm, sge_sig2str(how)));
   for_each (jep, *object_type_get_master_list(SGE_TYPE_JOB)) {
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         if (lGetElemStr(lGetList(jatep, JAT_granted_destin_identifier_list), JG_qname, qnm)) {
            u_long32 jstate;
            DPRINTF(("found "sge_u32"."sge_u32"\n", lGetUlong(jep, JB_job_number), lGetUlong(jatep, JAT_task_number)));
            jstate = lGetUlong(jatep, JAT_state);
            if (how == SGE_SIGSTOP)
               jstate |= JSUSPENDED_ON_SUBORDINATE;
            else
               jstate &= ~JSUSPENDED_ON_SUBORDINATE;
            lSetUlong(jatep, JAT_state, jstate);
         }
      }
   }

   DRETURN_VOID;
}

/************************************************************************
 This is called by the qmaster to:
 - send a signal to all jobs in a queue (job_number == 0);
 - send a signal to one job
 ************************************************************************/
int sge_signal_queue(
sge_gdi_ctx_class_t *ctx,
int how, /* signal */
lListElem *qep,
lListElem *jep,
lListElem *jatep,
monitoring_t *monitor
) {
   int i;
   u_long32 next_delivery_time = 60;
   u_long32 now;
   sge_pack_buffer pb;
   int sent = 0;

   DENTER(TOP_LAYER, "sge_signal_queue");

   now = sge_get_gmt();

   DEBUG((SGE_EVENT, "queue_signal: %d, queue: %s, job: %d, jatask: %d", how,
            (qep?lGetString(qep, QU_full_name):"none"),
            (int)(jep?lGetUlong(jep,JB_job_number):-1),
            (int)(jatep?lGetUlong(jatep,JAT_task_number):-1)
        ));

   if (!jep && (how == SGE_SIGSTOP || how == SGE_SIGCONT)) {
      sge_propagate_queue_suspension(lGetString(qep, QU_full_name), how);
   }

   /* don't try to signal unheard queues */
   if (!qinstance_state_is_unknown(qep)) {
      const char *hnm, *pnm;

      pnm = prognames[EXECD];
      hnm = lGetHost(qep, QU_qhostname);

      if ((i = init_packbuffer(&pb, 256, 0)) == PACK_SUCCESS) {
         /* identifier for acknowledgement */
         if (jep) {
            /*
             * Due to IZ 1619: pack signal only if
             *    job is a non-parallel job
             *    or all slaves of the parallel job have been acknowledged
             */
            if (!lGetString(jatep, JAT_master_queue) ||
                is_pe_master_task_send(jatep)) {
               /* TAG_SIGJOB */
               packint(&pb, lGetUlong(jep, JB_job_number));
               packint(&pb, lGetUlong(jatep, JAT_task_number));
               packstr(&pb, NULL);
               packint(&pb, how);
            }
         } else {
            /* TAG_SIGQUEUE */
            packint(&pb, 0);
            packint(&pb, 0);
            packstr(&pb, lGetString(qep, QU_full_name));
            packint(&pb, how);
         }

         if (mconf_get_simulate_execds()) {
            i = CL_RETVAL_OK;
            if (jep && how == SGE_SIGKILL)
               trigger_job_resend(sge_get_gmt(), NULL, lGetUlong(jep, JB_job_number), lGetUlong(jatep, JAT_task_number), 1);
         } else {
            if (pb_filled(&pb)) {
               u_long32 dummy = 0;
               i = gdi2_send_message_pb(ctx, 0, pnm, 1, hnm, jep ? TAG_SIGJOB: TAG_SIGQUEUE,
                             &pb, &dummy);
            }
         }

         MONITOR_MESSAGES_OUT(monitor);
         clear_packbuffer(&pb);
      } else {
         i = CL_RETVAL_MALLOC;  /* an error */
      }

      if (i != CL_RETVAL_OK) {
         ERROR((SGE_EVENT, MSG_COM_NOUPDATEQSTATE_IS, how, lGetString(qep, QU_full_name)));
         DRETURN(i);
      }
      sent = 1;
   }

   next_delivery_time += now;

   /* If this is a operation on one job we enter the signal request in the
      job structure. If the operation is not acknowledged in time we can do
      further steps */
   if (jep) {
      te_event_t ev = NULL;

      DPRINTF(("JOB "sge_u32": %s signal %s (retry after "sge_u32" seconds) host: %s\n",
            lGetUlong(jep, JB_job_number), sent?"sent":"queued", sge_sig2str(how), next_delivery_time - now,
            lGetHost(qep, QU_qhostname)));
      te_delete_one_time_event(TYPE_SIGNAL_RESEND_EVENT, lGetUlong(jep, JB_job_number),
         lGetUlong(jatep, JAT_task_number), NULL);

      if (!mconf_get_simulate_execds()) {
         lSetUlong(jatep, JAT_pending_signal, how);
         ev = te_new_event((time_t)next_delivery_time, TYPE_SIGNAL_RESEND_EVENT, ONE_TIME_EVENT,
            lGetUlong(jep, JB_job_number), lGetUlong(jatep, JAT_task_number), NULL);
         te_add_event(ev);
         te_free_event(&ev);
         lSetUlong(jatep, JAT_pending_signal_delivery_time, next_delivery_time);
      }
   } else {
      te_event_t ev = NULL;

      DPRINTF(("QUEUE %s: %s signal %s (retry after "sge_u32" seconds) host %s\n",
            lGetString(qep, QU_full_name), sent?"sent":"queued", sge_sig2str(how), next_delivery_time - now,
            lGetHost(qep, QU_qhostname)));
      te_delete_one_time_event(TYPE_SIGNAL_RESEND_EVENT, 0, 0, lGetString(qep, QU_full_name));

      if (!mconf_get_simulate_execds()) {
         lSetUlong(qep, QU_pending_signal, how);
         ev = te_new_event((time_t)next_delivery_time, TYPE_SIGNAL_RESEND_EVENT, ONE_TIME_EVENT, 0, 0,
            lGetString(qep, QU_full_name));
         te_add_event(ev);
         te_free_event(&ev);
         lSetUlong(qep, QU_pending_signal_delivery_time, next_delivery_time);
      }
   }

   if (!jep) {/* signalling a queue ? - handle slave jobs in this queue */
      signal_slave_jobs_in_queue(ctx, how, qep, monitor);
   }
   else {/* is this the master queue of this job to signal ? - then decide whether slave tasks also
           must get signalled */
      if (!strcmp(lGetString(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)),
            JG_qname), lGetString(qep, QU_full_name))) {
         signal_slave_tasks_of_job(ctx, how, jep, jatep, monitor);
      }
   }

   DEXIT;
   return 0;
} /* sge_signal_queue() */

/* in case we have to signal a queue
   in which slave tasks are running
   we have to notify the master execd
   where the master task of this job is running
*/
static void signal_slave_jobs_in_queue(
sge_gdi_ctx_class_t *ctx,
int how, /* signal */
lListElem *qep,
monitoring_t *monitor
) {
   lList *gdil_lp;
   lListElem *mq, *jep, *jatep;
   const char *qname, *mqname, *pe_name;

   DENTER(TOP_LAYER, "signal_slave_jobs_in_queue");

   qname = lGetString(qep, QU_full_name);
   /* test whether there are parallel jobs
      with a slave slot in this queue
      if so then signal this job */
   for_each (jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
      for_each (jatep, lGetList(jep, JB_ja_tasks)) {

         /* skip sequential and not running jobs */
         if (lGetNumberOfElem( gdil_lp =
               lGetList(jatep, JAT_granted_destin_identifier_list))<=1)
            continue;

         /* signalling of not "slave controlled" parallel jobs will not work
            since they are not known to the apropriate execd - we should
            omit signalling in this case to prevent waste of communication bandwith */
         if (!(pe_name=lGetString(jatep, JAT_granted_pe)) ||
             !pe_list_locate(*object_type_get_master_list(SGE_TYPE_PE), pe_name))
            continue;

         if (lGetElemStr(gdil_lp, JG_qname, qname) != NULL) {

            /* search master queue - needed for signalling of a job */
            if ((mq = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), mqname = lGetString(
                  lFirst(gdil_lp), JG_qname)))) {
               DPRINTF(("found slave job "sge_u32" in queue %s master queue is %s\n",
                  lGetUlong(jep, JB_job_number), qname, mqname));
               sge_signal_queue(ctx, how, mq, jep, jatep, monitor);
            } else {
               ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDMQ_SU, mqname, sge_u32c(lGetUlong(jep, JB_job_number))));
            }
         }
      }
   }

   DRETURN_VOID;
}

static void signal_slave_tasks_of_job(sge_gdi_ctx_class_t *ctx, int how,
        lListElem *jep, lListElem *jatep, monitoring_t *monitor)
{
   lList *gdil_lp;
   lListElem *mq, *pe, *gdil_ep;
   const char *qname, *pe_name;

   DENTER(TOP_LAYER, "signal_slave_tasks_of_job");

   /* do not signal slave tasks in case of checkpointing jobs with
      STOP/CONT when suspending means migration */
   if ((how==SGE_SIGCONT || how==SGE_SIGSTOP) &&
       (lGetUlong(jep, JB_checkpoint_attr) & CHECKPOINT_SUSPEND) != 0) {
      DPRINTF(("omit signaling - checkpoint script does action for whole job\n"));
      return;
   }

   /* forward signal to slave exec hosts
      in case of slave controlled jobs */
   if ( !((lGetNumberOfElem(gdil_lp=lGetList(jatep, JAT_granted_destin_identifier_list)))<=1 ||
         !(pe_name=lGetString(jatep, JAT_granted_pe)) ||
         !(pe=pe_list_locate(*object_type_get_master_list(SGE_TYPE_PE), pe_name)) ||
         !lGetBool(pe, PE_control_slaves)))
      for (gdil_ep=lNext(lFirst(gdil_lp)); gdil_ep; gdil_ep=lNext(gdil_ep))
         if ((mq = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), qname = lGetString(gdil_ep, JG_qname)))) {
            DPRINTF(("found slave job "sge_u32" in queue %s\n",
               lGetUlong(jep, JB_job_number), qname));
            sge_signal_queue(ctx, how, mq, jep, jatep, monitor);
         }

   DEXIT;
   return;
}

