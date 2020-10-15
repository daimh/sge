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
#include <string.h>
#include <stdlib.h>

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"
#include "uti/sge_unistd.h"
#include "uti/sge_time.h"
#include "uti/sge_hostname.h"

#include "sgeobj/sge_host.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_answer.h"

#include "sge.h"
#include "sge_job_qmaster.h"
#include "sge_pe_qmaster.h"
#include "sge_give_jobs.h"
#include "execution_states.h"
#include "symbols.h"
#include "sge_reporting_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_qinstance_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_job_enforce_limit.h"
#include "msg_common.h"
#include "msg_qmaster.h"


/************************************************************************
 Master routine for job exit

 We need a rusage struct filled.
 In normal cases this is done by the execd, sending this structure
 to notify master about job finish.

 In case of an error noticed by the master which needs the job to be 
 removed we can fill this structure by hand. We need:

 rusage->job_number
 rusage->qname to clean up the queue (if we didn't find it we nevertheless
               clean up the job

 for functions regarding rusage see sge_rusage.c
 ************************************************************************/
void sge_job_exit(sge_gdi_ctx_class_t *ctx, lListElem *jr, lListElem *jep, lListElem *jatep, monitoring_t *monitor) 
{
   lListElem *queueep = NULL;
   const char *err_str = NULL;
   const char *qname = NULL;  
   const char *hostname = MSG_OBJ_UNKNOWNHOST;
   u_long32 jobid, jataskid;
   lListElem *hep = NULL;
   u_long32 timestamp;
   object_description *object_base = object_type_get_object_description();

   u_long32 failed, general_failure;
   lList *saved_gdil;

   DENTER(TOP_LAYER, "sge_job_exit");

   /* JG: TODO: we'd prefer some more precise timestamp, e.g. from jr */
   timestamp = sge_get_gmt();
                     
   qname = lGetString(jr, JR_queue_name);
   if (qname == NULL) {
      qname = (char *)MSG_OBJ_UNKNOWNQ;
   }

   err_str = lGetString(jr, JR_err_str);
   if (err_str == NULL) {
      err_str = MSG_UNKNOWNREASON;
   }

   jobid = lGetUlong(jr, JR_job_number);
   jataskid = lGetUlong(jr, JR_ja_task_number);
   failed = lGetUlong(jr, JR_failed);
   general_failure = lGetUlong(jr, JR_general_failure);

   cancel_job_resend(jobid, jataskid);

   /* This only has a meaning for Hibernator jobs. The job pid must
    * be saved accross restarts, since jobs get their old pid
    */
   lSetUlong(jatep, JAT_pvm_ckpt_pid, lGetUlong(jr, JR_job_pid));

   DPRINTF(("reaping job "sge_u32"."sge_u32" in queue >%s< job_pid %d\n", 
      jobid, jataskid, qname, (int) lGetUlong(jatep, JAT_pvm_ckpt_pid)));

   queueep = cqueue_list_locate_qinstance(*object_base[SGE_TYPE_CQUEUE].list, qname);
   if (queueep == NULL) {
      ERROR((SGE_EVENT, MSG_JOB_WRITEJFINISH_S, qname));
   }

   sge_job_remove_enforce_limit_trigger(jobid, jataskid);

   /* retrieve hostname for later use */
   if (queueep != NULL) {
      hostname = lGetHost(queueep, QU_qhostname);
   }

   if (failed) {        /* a problem occured */
      WARNING((SGE_EVENT, MSG_JOB_FAILEDONHOST_UUSSSS, sge_u32c(jobid), 
               sge_u32c(jataskid), 
               hostname,
               general_failure ? MSG_GENERAL : "",
               get_sstate_description(failed), err_str));
   } else {
      INFO((SGE_EVENT, MSG_JOB_JFINISH_UUS,  sge_u32c(jobid), sge_u32c(jataskid), hostname));
   }

   /*-------------------------------------------------*/

   /* test if this job is in state JRUNNING or JTRANSFERING */
   if (lGetUlong(jatep, JAT_status) != JRUNNING && 
       lGetUlong(jatep, JAT_status) != JTRANSFERING) {
      ERROR((SGE_EVENT, MSG_JOB_JEXITNOTRUN_UU, sge_u32c(lGetUlong(jep, JB_job_number)), sge_u32c(jataskid)));
      DRETURN_VOID;
   }

   saved_gdil = lCopyList("cpy", lGetList(jatep, JAT_granted_destin_identifier_list));

   /*
    * case 1: job being trashed because 
    *    --> failed starting interactive job
    *    --> job was deleted
    *    --> a failed batch job that explicitely shall not enter error state
    */
   if (((lGetUlong(jatep, JAT_state) & JDELETED) == JDELETED) ||
         (failed && !lGetString(jep, JB_exec_file)) ||
         (failed && general_failure==GFSTATE_JOB && JOB_TYPE_IS_NO_ERROR(lGetUlong(jep, JB_type)))) {
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      /* JG: TODO: we need more information in the log message */
      reporting_create_job_log(NULL, timestamp, JL_DELETED, MSG_EXECD, hostname, jr, jep, jatep, NULL, MSG_LOG_JREMOVED);

      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_FINISHED_FAILED_EE, COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);

      if (lGetUlong(jep, JB_ar) != 0 && (lGetUlong(jatep, JAT_state) & JDELETED) == JDELETED) {
         /* get AR and remove it if no other jobs are debited */
         lList *master_ar_list = *object_base[SGE_TYPE_AR].list;
         lListElem *ar = ar_list_locate(master_ar_list, lGetUlong(jep, JB_ar));

         if (ar != NULL && lGetUlong(ar, AR_state) == AR_DELETED) {
            lListElem *ar_queue;
            u_long32 ar_id = lGetUlong(ar, AR_id);

            for_each(ar_queue, lGetList(ar, AR_reserved_queues)) {
               if (qinstance_slots_used(ar_queue) != 0) {
                  break;
               }
            }
            if (ar_queue == NULL) {
               /* no jobs registered in advance reservation */
               dstring buffer = DSTRING_INIT;

               sge_dstring_sprintf(&buffer, sge_U32CFormat,
                                   sge_u32c(ar_id));

               ar_do_reservation(ar, false);

               reporting_create_ar_log_record(NULL, ar, ARL_DELETED, 
                                              "AR deleted",
                                              timestamp);
               reporting_create_ar_acct_records(NULL, ar, timestamp); 

               lRemoveElem(master_ar_list, &ar);

               sge_event_spool(ctx, NULL, 0, sgeE_AR_DEL, 
                      ar_id, 0, sge_dstring_get_string(&buffer), NULL, NULL,
                      NULL, NULL, NULL, true, true);
               sge_dstring_free(&buffer);
            }
         }
      }
   } 
     /*
      * case 2: set job in error state
      *    --> owner requested wrong 
      *        -e/-o/-S/-cwd
      *    --> user did not exist at the execution machine
      *    --> application controlled job error
      */
   else if ((failed && general_failure==GFSTATE_JOB)) {
      DPRINTF(("set job "sge_u32"."sge_u32" in ERROR state\n", 
               lGetUlong(jep, JB_job_number), jataskid));
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      /* JG: TODO: we need more information in the log message */
      reporting_create_job_log(NULL, timestamp, JL_ERROR, MSG_EXECD, hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_JERRORSET);
      lSetUlong(jatep, JAT_start_time, 0);
      ja_task_message_add(jatep, 1, err_str);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_FAILED_AND_ERROR, COMMIT_DEFAULT, monitor);
   }
      /*
       * case 3: job being rescheduled because it wasnt even started
       *                            or because it was a general error
       */
   else if (((failed && (failed <= SSTATE_BEFORE_JOB)) || 
        (general_failure && general_failure != GFSTATE_QUEUE_NO_RERUN))) {
      /* JG: TODO: we need more information in the log message */
      reporting_create_job_log(NULL, timestamp, JL_RESTART, MSG_EXECD, 
                               hostname, jr, jep, jatep, NULL, 
                               MSG_LOG_JNOSTARTRESCHEDULE);
      ja_task_message_add(jatep, 1, err_str);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_RESCHEDULED, COMMIT_DEFAULT, monitor);
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      lSetUlong(jatep, JAT_start_time, 0);
   }
      /*
       * case 4: job being rescheduled because rerun specified or ckpt job
       */
   else if (((failed == ESSTATE_NO_EXITSTATUS) || failed == ESSTATE_DIED_THRU_SIGNAL) &&
            ((lGetUlong(jep, JB_restart) == 1 || (lGetUlong(jep, JB_checkpoint_attr) & ~NO_CHECKPOINT)) ||
             (!lGetUlong(jep, JB_restart) && (queueep != NULL && lGetBool(queueep, QU_rerun))))) {
      lSetUlong(jatep, JAT_job_restarted, 
                MAX(lGetUlong(jatep, JAT_job_restarted), lGetUlong(jr, JR_ckpt_arena)));
      lSetString(jatep, JAT_osjobid, lGetString(jr, JR_osjobid));
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      /* JG: TODO: we need more information in the log message */
      reporting_create_job_log(NULL, timestamp, JL_RESTART, MSG_EXECD, hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_JRERUNRESCHEDULE);
      lSetUlong(jatep, JAT_start_time, 0);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_RESCHEDULED, COMMIT_DEFAULT, monitor);
   }
      /*
       * case 5: job being rescheduled because it was interrupted and a checkpoint exists
       */
   else if (failed == SSTATE_MIGRATE) {
      DTRACE;
      /* job_restarted == 2 means a checkpoint in the ckpt arena */
      lSetUlong(jatep, JAT_job_restarted, 
                  MAX(lGetUlong(jatep, JAT_job_restarted), 
                      lGetUlong(jr, JR_ckpt_arena)));
      lSetString(jatep, JAT_osjobid, lGetString(jr, JR_osjobid));
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      reporting_create_job_log(NULL, timestamp, JL_MIGRATE, MSG_EXECD, hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_JCKPTRESCHEDULE);
      lSetUlong(jatep, JAT_start_time, 0);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_RESCHEDULED, COMMIT_DEFAULT, monitor);
   }
      /*
       * case 6: job being rescheduled because of exit 99 
       *                            or because of a rerun e.g. triggered by qmod -r <jobid>
       */
   else if (failed == SSTATE_AGAIN) {
      lSetUlong(jatep, JAT_job_restarted, 
                  MAX(lGetUlong(jatep, JAT_job_restarted), 
                      lGetUlong(jr, JR_ckpt_arena)));
      lSetString(jatep, JAT_osjobid, lGetString(jr, JR_osjobid));
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      reporting_create_job_log(NULL, timestamp, JL_RESTART, MSG_EXECD, hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_JNORESRESCHEDULE);
      lSetUlong(jatep, JAT_start_time, 0);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_USER_RESCHEDULED, COMMIT_DEFAULT, monitor);
   }
      /*
       * case 7: job finished 
       */
   else {
      reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
      reporting_create_job_log(NULL, timestamp, JL_FINISHED, MSG_EXECD, hostname, jr, jep, jatep, NULL, MSG_LOG_EXITED);
      sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_FINISHED_FAILED_EE, COMMIT_DEFAULT, monitor);
   }

   if (queueep != NULL) {
      bool found_host = false;
      lList *answer_list = NULL;
      /*
      ** in this case we have to halt all queues on this host
      */
      if (general_failure && general_failure == GFSTATE_HOST) {
         hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, 
                                lGetHost(queueep, QU_qhostname));
         if (hep != NULL) {
            lListElem *cqueue = NULL;
            const char *host = lGetHost(hep, EH_name);
            dstring error = DSTRING_INIT;

            found_host = true;

            for_each(cqueue, *object_base[SGE_TYPE_CQUEUE].list) {
               lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
               lListElem *qinstance = NULL;

               qinstance = lGetElemHost(qinstance_list, QU_qhostname, host);
               if (qinstance != NULL) {

                  sge_qmaster_qinstance_state_set_error(qinstance, true);

                  sge_dstring_sprintf(&error, MSG_LOG_QERRORBYJOBHOST_SUS, lGetString(qinstance, QU_qname), sge_u32c(jobid), host);
                  qinstance_message_add(qinstance, QI_ERROR, sge_dstring_get_string(&error)); 
                  ERROR((SGE_EVENT, SFNMAX, sge_dstring_get_string(&error)));
                  sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD, 
                                  0, 0, lGetString(qinstance, QU_qname), 
                                  lGetHost(qinstance, QU_qhostname), NULL,
                                  qinstance, NULL, NULL, true, true);
               }
            }
            sge_dstring_free(&error);
         }
      }
      /*
      ** to be sure this queue is halted even if the host 
      ** is not found in the next statement
      */
      if (general_failure && general_failure != GFSTATE_JOB && found_host == false) {  
         dstring error = DSTRING_INIT; 

         sge_dstring_sprintf(&error, MSG_LOG_QERRORBYJOBHOST_SUS,
                             lGetString(queueep, QU_qname), sge_u32c(jobid),
                             hostname);
         
         /* general error -> this queue cant run any job */
         sge_qmaster_qinstance_state_set_error(queueep, true);
         qinstance_message_add(queueep, QI_ERROR, sge_dstring_get_string(&error));
         ERROR((SGE_EVENT, SFNMAX, sge_dstring_get_string(&error)));
         sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD, 
                         0, 0, lGetString(queueep, QU_qname), 
                         lGetHost(queueep, QU_qhostname), NULL,
                         queueep, NULL, NULL, true, true);
         sge_dstring_free(&error);
      }

      gdil_del_all_orphaned(ctx, saved_gdil, &answer_list);
      answer_list_output(&answer_list);
   }

   lFreeList(&saved_gdil);
   DRETURN_VOID;
}
