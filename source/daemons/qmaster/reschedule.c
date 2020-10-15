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

#include "uti/sge_rmon.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"
#include "uti/sge_prog.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_cqueue.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "basis_types.h"
#include "job_exit.h"
#include "sge_event_master.h"
#include "sge_ckpt_qmaster.h"
#include "sge_host_qmaster.h"
#include "execution_states.h"
#include "mail.h"
#include "symbols.h"
#include "reschedule.h"
#include "configuration_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "sge_give_jobs.h"
#include "msg_qmaster.h"

u_long32 add_time = 0;

static u_long32 reschedule_unknown_timeout(lListElem *hep);
static void update_reschedule_unknown_timeout(lListElem *host);
static lListElem* get_from_reschedule_unknown_list(lListElem *host, 
                                            u_long32 job_number, 
                                            u_long32 task_number);

/****** qmaster/reschedule/reschedule_unknown_event() *************************
*  NAME
*     reschedule_unknown_event() -- event handler to reschedule jobs 
*
*  SYNOPSIS
*     void reschedule_unknown_event(te_event_t anEvent)
*
*  FUNCTION
*     This function initiates the automatic rescheduling for certain
*     jobs running on a specific host. These jobs will be put back into
*     the list of pending jobs.
*     The function is triggered by TYPE_RESCHEDULE_UNKNOWN_EVENT's.
*     TYPE_RESCHEDULE_UNKNOWN_EVENT's occure when the configured 
*     "reschedule_unknown" timout value rundown. The clock tiggering 
*     this event handler will be wind up with a call of 
*     reschedule_unknown_trigger() in following situations: 
*
*        - a execution host went in unknown state due to missing
*          load reports
*           * execd was shut down (qconf -ke) 
*           * execd died (kill or coredump)
*           * network or host problems (machine crashed, cable problem ...)
*        - qmaster startup (all execution hosts are in unknown state) 
*
*  INPUTS
*     sge_timed_event_t* - timed event
*
*  RESULT
*     void - none
*
*  NOTES
*     MT-NOTE: reschedule_unknown_event() is NOT MT safe
*
*******************************************************************************/
void reschedule_unknown_event(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem *qep;            /* QU_Type */
   lList *answer_list = NULL; /* AN_Type */
   lListElem *hep;            /* EH_Type */
   object_description *object_base = object_type_get_object_description();
   lList *master_list = *object_base[SGE_TYPE_CQUEUE].list;
   u_long32 new_timeout = 0;
   u_long32 timeout = te_get_first_numeric_key(anEvent);
   char* hostname = te_get_alphanumeric_key(anEvent);


   DENTER(TOP_LAYER, "reschedule_unknown_event");

   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);
 
   /*
    * is the automatic rescheduling disabled
    */
   if (mconf_get_disable_reschedule()) {
      DTRACE;
      goto Error;
   }         
 
   /*
    * locate the host object which went in unknown-state
    */
   if (!(hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, hostname))) {
      DTRACE;
      goto Error;
   }

   /*
    * Did someone change the timeout value?
    */
   new_timeout = reschedule_unknown_timeout(hep);
   if (new_timeout == 0) {
      INFO((SGE_EVENT, MSG_RU_CANCELED_S, hostname));
      DTRACE;
      goto Error;
   } else if (new_timeout+add_time > timeout) {
      u_long32 when, delta = 0;
      te_event_t ev = NULL;
 
      delta = new_timeout + add_time;    
      when = time(NULL) + (delta - timeout);
      ev = te_new_event((time_t)when, TYPE_RESCHEDULE_UNKNOWN_EVENT, ONE_TIME_EVENT, delta, 0, hostname);
      te_add_event(ev);
      te_free_event(&ev);
      DTRACE;
      goto Error;
   }
 
   /*
    * Check if host is still in unknown state
    */
   for_each(qep, master_list) {
      lListElem *qinstance = NULL;

      qinstance = lGetElemHost(lGetList(qep, CQ_qinstances), QU_qhostname, 
                                         hostname); 
      if (qinstance != NULL) {
         if (!qinstance_state_is_unknown(qinstance)) {
           DTRACE;
           goto Error;
         }
      }
   }

   /*
    * Find all jobs currently running on the host which went into
    * unknown state and append the jobids/taskids into
    * a sublist of the exechost object
    */
   reschedule_jobs(ctx, hep, 0, &answer_list, monitor, false);
   lFreeList(&answer_list);
   
Error:
   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
   sge_free(&hostname);
   DRETURN_VOID;
}
 
/****** qmaster/reschedule/reschedule_jobs() **********************************
*  NAME
*     reschedule_jobs() -- reschedule jobs junning in host/queue 
*
*  SYNOPSIS
*     int reschedule_jobs(lListElem *ep, u_long32 force, lList **answer, bool is_manual) 
*
*  FUNCTION
*     The function is able to reschedule jobs running on a certain host
*     or in a specific queue. Please note that not all jobs will be
*     rescheduled. reschedule_job() containes more information which
*     jobs are applied.
*
*  INPUTS
*     lListElem *ep  - host or queue (EH_Type or QU_Type) 
*     u_long32 force - force the rescheduling of certain jobs (boolean)
*     lList **answer - answer list (AN_Type)
*     bool is_manual - indicator for manual (e.g. 'qmod -rj') or automatic
*                      (e.g. execd goes down) rescheduling.
*
*  RESULT
*     int - 0 on success; 1 if one of the parameters was invalid 
*******************************************************************************/
int reschedule_jobs(sge_gdi_ctx_class_t *ctx, lListElem *ep, u_long32 force, lList **answer, monitoring_t *monitor, bool is_manual) 
{
   lListElem *jep;               /* JB_Type */
   int ret = 1;
 
   DENTER(TOP_LAYER, "reschedule_jobs");
 
   /*
    * if ep is of type EH_Type than we will reschedule all jobs
    * running on that host. if it is of type QU_Type than we will
    * only reschedule the jobs for that queue
    */
   if (object_has_type(ep, EH_Type) ||
       object_has_type(ep, QU_Type)) {
      /*
       * Find all jobs currently running on the host/queue
       * append the jobids/taskids into a sublist of the exechost object
       */
      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         reschedule_job(ctx, jep, NULL, ep, force, answer, monitor, is_manual);
      }      
      ret = 0;
   }
 
   DEXIT;
   return ret;
}
 
/****** qmaster/reschedule/reschedule_job() ***********************************
*  NAME
*     reschedule_job() -- reschedule array tasks or jobs 
*
*  SYNOPSIS
*     int reschedule_job(lListElem *jep, lListElem *jatep, lListElem *ep, 
*                        u_long32 force, lList **answer, bool is_manual) 
*
*  FUNCTION
*     This function is able to reschedule:
*        (a) a job with all its array tasks running anywhere 
*           (jatep == NULL && ep == NULL) 
*        (b) one array task running anywhere 
*           (jatep != NULL && ep == NULL)
*        (c) all tasks of a job running on a certain host/queue
*           (jatep == NULL && ep != NULL)
*        (d) one array task running on a certain host/queue
*           (jatep != NULL && ep != NULL)
*     Additionally to the conditions above jobs/tasks will only be 
*     rescheduled if they fulfill following requirements: 
*        (1) not pending
*        (2) restartable ("rerun" of the queue is "true" or "qsub -r y")
*        (3) not interactive (qsh, qlogin, qrsh)
*        (4) ckpt job and "when" of ckpt-obj containes "r" flag 
*        (5) was not deleted previously (qdel)
*     It is possible to force the rescheduling of jobs/tasks not fulfilling
*     condition (2), (4) and (5) if the force parameter is 1.
*
*  INPUTS
*     lListElem *jep   - job (JB_Type)
*     lListElem *jatep - array task (JAT_Type or NULL) 
*     lListElem *ep    - host or queue (EH_Type or QU_Type or NULL)
*     u_long32 force   - force rescheduling (boolean) 
*     lList **answer   - answer list (AN_Type) 
*     bool is_manual   - indicator for manual (e.g. 'qmod -rj') or automatic
*                        (e.g. execd goes down) rescheduling
*
*  RESULT
*     int - 0 on success
*******************************************************************************/
int reschedule_job(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jatep, lListElem *ep,  
                   u_long32 force, lList **answer, monitoring_t *monitor, bool is_manual) 
{
   lListElem *qep;               /* QU_Type */
   lListElem *hep;               /* EH_Type */
   lListElem *this_jatep;        /* JAT_Type */
   lListElem *next_jatep;        /* JAT_Type */
   char mail_ids[256];
   char mail_type[256];
   u_long32 job_number;
   u_long32 job_now;
   const char *hostname;
   int ret = 0;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "reschedule_job");
 
   job_number = lGetUlong(jep, JB_job_number);

   /*
    * if jatep is NULL then reschedule all tasks of this job
    */
   if (jatep) {
      next_jatep = jatep;
   } else {
      next_jatep = lFirst(lGetList(jep, JB_ja_tasks));
   }

   while ((this_jatep = next_jatep)) {
      lListElem *first_granted_queue;  /* JG_Type */
      lListElem *host;                 /* EH_Type */
      lList *granted_qs;
      u_long32 task_number;
      u_long32 found;

      if (jatep) {
         next_jatep = NULL;
      } else {
         next_jatep = lNext(this_jatep);
      }

      granted_qs = lGetList(this_jatep, JAT_granted_destin_identifier_list);
      first_granted_queue = lFirst(granted_qs);

      /*
       * Jobs which have no granted queue can not be rescheduled (skip pending jobs)
       */
      if (first_granted_queue == NULL) {
         continue;
      }              
      
      task_number = lGetUlong(this_jatep, JAT_task_number);

      if (job_is_array(jep)) {
         snprintf(mail_ids, sizeof(mail_ids), sge_U32CFormat"."sge_U32CFormat,
                  sge_u32c(job_number), sge_u32c(task_number));
         sge_strlcpy(mail_type, MSG_RU_TYPEJOBARRAY, sizeof(mail_type));
      } else {
         snprintf(mail_ids, sizeof(mail_ids), sge_U32CFormat, sge_u32c(job_number));
         sge_strlcpy(mail_type, MSG_RU_TYPEJOB, sizeof(mail_type));
      }

      /*
       * if ep is of type EH_Type than we will reschedule all tasks
       * running on that host. if it is of type QU_Type than we will
       * only reschedule the tasks for that queue. if it is NULL than we will
       * reschedule all tasks of that job
       */
      if (ep && object_has_type(ep, EH_Type)) {
         hep = ep;
         qep = NULL;
         hostname = lGetHost(ep, EH_name);
      } else if (ep && object_has_type(ep, QU_Type)) {
         qep = ep;
         hostname = lGetHost(qep, QU_qhostname);
         hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, hostname);
      } else {
         qep = NULL;
         hep = NULL;
         hostname = NULL;
      }                        

      /*
       * We will skip current job if it is a
       *    - non-PE job and it is not running in queue/host
       *    - PE job where the master task is not running in queue/host (RESCHEDULE_SLAVE == false)
       *    - PE job where none of the tasks is running in queue/host (RESCHEDULE_SLAVE == true)
       */
      {
         const char *the_queuename = (qep != NULL ? lGetString(qep, QU_full_name) : NULL);
         const char *the_hostname = (hep != NULL ? lGetHost(hep, EH_name) : NULL);

         /*
          * If RESCHEDULE_SLAVE is set then 
          *    we have to check each granted queue because PE jobs should be 
          *    rescheduled also if only one PE slave task is running in that queue/host
          * otherwise
          *    PE jobs will only be rescheduled when the master task is effected 
          */
         if (mconf_get_enable_reschedule_slave() == true) {
            lListElem *granted_q;
            bool one_matched = false;

            for_each(granted_q, granted_qs) {
               if ((qep != NULL && strcmp(lGetString(granted_q, JG_qname), the_queuename) == 0) ||
                   (hep != NULL && sge_hostcmp(lGetHost(granted_q, JG_qhostname), the_hostname) == 0)) {
                  one_matched = true; 
                  break;
               }
            }
            if (one_matched == false) {
               continue;
            }
         } else {
            if ((qep != NULL && strcmp(lGetString(first_granted_queue, JG_qname), the_queuename)) ||
                (hep != NULL && sge_hostcmp(lGetHost(first_granted_queue, JG_qhostname), the_hostname))) {
               continue;
            }
         }
      }

      /*
       * Is this job tagged as restartable?
       * If the forced flag is set we will also reschedule this job!
       */
      if (!force && lGetUlong(jep, JB_restart) == 2) {
         if (mconf_get_enable_reschedule_kill()) {
            INFO((SGE_EVENT, MSG_RU_REAPING_NOT_RESTARTABLE_SS, 
                  mail_type, mail_ids));
            sge_commit_job(ctx, jep, this_jatep, NULL, COMMIT_ST_FINISHED_FAILED_EE,
                           COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
            continue;
         } else {
            INFO((SGE_EVENT, MSG_RU_NOT_RESTARTABLE_SS, 
                  mail_type, mail_ids));
            answer_list_add(answer, SGE_EVENT, 
                            STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         }
         continue;
      }

      /*
       * qsh, qlogin, qrsh, qrlogin-jobs won't be rescheduled automatically
       * (immediate jobs (qsub -now y ...) will be rescheduled)
       */
      job_now = lGetUlong(jep, JB_type);
      if (JOB_TYPE_IS_QSH(job_now) || JOB_TYPE_IS_QLOGIN(job_now)
          || JOB_TYPE_IS_QRSH(job_now) || JOB_TYPE_IS_QRLOGIN(job_now)) {
         INFO((SGE_EVENT, MSG_RU_INTERACTIVEJOB_SSS, mail_ids, mail_type, 
            mail_type));
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, 
            ANSWER_QUALITY_WARNING);
         continue;
      }

      /*
       * ckpt-jobs will only be rescheduled when the "when" attribute
       * contains an appropriate flag or when the forced flag is set
       */
      if (!force && lGetString(jep, JB_checkpoint_name)) {
         lListElem *ckpt_ep; /* CK_Type */
    
         ckpt_ep = ckpt_list_locate(*object_base[SGE_TYPE_CKPT].list, 
                                    lGetString(jep, JB_checkpoint_name));
         if (ckpt_ep) {
            u_long32 flags;
    
            flags = lGetUlong(jep, JB_checkpoint_attr);
            if (!flags) {
               flags = sge_parse_checkpoint_attr(
                          lGetString(ckpt_ep, CK_when));
            }
            if (!(flags & CHECKPOINT_AT_AUTO_RES)) {
               INFO((SGE_EVENT, MSG_RU_CKPTNOTVALID_SSS,
                   mail_ids, lGetString(ckpt_ep, CK_name), mail_type));
               answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, 
                  ANSWER_QUALITY_WARNING);
               continue;
            }
         } else {
            INFO((SGE_EVENT, MSG_RU_CKPTEXIST_SS, mail_ids, 
               lGetString(ckpt_ep, CK_name)));
            answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, 
               ANSWER_QUALITY_WARNING);
            continue;
         }
      }               

      /*
       * Jobs which were registered for deletion will
       * not be automaticly rescheduled (exception: forced flag)
       */
      if (!force && (lGetUlong(this_jatep, JAT_state) & JDELETED)) {
         INFO((SGE_EVENT, MSG_RU_INDELETEDSTATE_SS, mail_type, mail_ids));
         answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         continue;
      }

      /*
       * If the user did not use the -r flag during submit
       * we have to check the queue default (exception: forced flag)
       */
      if (!force && lGetUlong(jep, JB_restart) == 0) {
         lListElem *queue; /* QU_Type */

         if (qep && !strcmp(lGetString(first_granted_queue,
            JG_qname), lGetString(qep, QU_full_name))) {
            DTRACE;
            queue = qep;
         } else {
            DTRACE;
            queue = cqueue_list_locate_qinstance(
                           *(object_type_get_master_list(SGE_TYPE_CQUEUE)),
                           lGetString(first_granted_queue, JG_qname));
         }
         if (queue == NULL || !lGetBool(queue, QU_rerun)) {
            if (mconf_get_enable_reschedule_kill()) {
               INFO((SGE_EVENT, MSG_RU_REAPING_NOT_RESTARTABLE_SS, 
                     mail_type, mail_ids));
               sge_commit_job(ctx, jep, this_jatep, NULL, COMMIT_ST_FINISHED_FAILED_EE,
                              COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
               continue;

            } else {
               INFO((SGE_EVENT, MSG_RU_NORERUNQUEUE_SSS, mail_type, mail_ids, 
                  queue?"none":lGetString(queue, QU_full_name)));
               answer_list_add(answer, SGE_EVENT, STATUS_ESEMANTIC, 
                  ANSWER_QUALITY_WARNING);
               continue;
            }
         }
      }

      /*
       * Is this task already contained in the list?
       * Append it if necessary
       */
      if (hep && !sge_hostcmp(lGetHost(first_granted_queue, JG_qhostname),
          lGetHost(hep, EH_name))) {
         host = hep;
      } else {
         host = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, 
                  lGetHost(first_granted_queue, JG_qhostname));
         hostname = lGetHost(first_granted_queue, JG_qhostname);
      }
      if (get_from_reschedule_unknown_list(host, job_number, task_number)) {
         found = 1;
      } else {
         found = 0;
      }

      if (!found) {
         add_to_reschedule_unknown_list(ctx, host, job_number, task_number, 0);
         ret = 0;                
      }

      /*
       * Mails and messages
       */
      if (!found) {
         u_long32 mail_options;
         char mail_action[256];

         mail_options = lGetUlong(jep, JB_mail_options);
         if (force) {
            sge_strlcpy(mail_action, MSG_RU_FORCEDR, sizeof(mail_action));
         } else {
            sge_strlcpy(mail_action, MSG_RU_PUSHEDR, sizeof(mail_action));
         }
         if (VALID(MAIL_AT_ABORT, mail_options)) {
            lList *mail_users;
            char mail_subject[1024];
            char mail_body[1024];

            mail_users = lGetList(jep, JB_mail_list);
            snprintf(mail_subject, sizeof(mail_subject), MSG_RU_MAILSUB_SS,
                     mail_type, mail_ids);
            snprintf(mail_body, sizeof(mail_body), MSG_RU_MAILBODY_SSSS,
               mail_type, mail_ids, hostname, mail_action, mail_type);
            cull_mail(QMASTER, mail_users, mail_subject, mail_body, MSG_RU_MAILTYPE);
         }

         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_RU_MSGFILEINFO, mail_action,
                                mail_type, mail_ids, hostname));

         answer_list_add(answer, SGE_EVENT,
                         STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
      }

      /*
       * Trigger the rescheduling of this task
       */
      if (!found) {
         lListElem *pseudo_jr; /* JR_Type */
         u_long32 state = lGetUlong(this_jatep, JAT_state);

         lSetUlong(this_jatep, JAT_job_restarted, 1);

         if (is_manual) {
            /*
             * Indicate deferred startup processing required.
             */
            SETBIT(JDEFERRED_REQ, state);
         } else {
            /*
             * Paranoia ...
             */
            CLEARBIT(JDEFERRED_REQ, state);
         }
         lSetUlong(this_jatep, JAT_state, state);
            
         pseudo_jr = lCreateElem(JR_Type);
         lSetUlong(pseudo_jr, JR_job_number, job_number);
         lSetUlong(pseudo_jr, JR_ja_task_number, task_number);
         lSetUlong(pseudo_jr, JR_failed, SSTATE_AGAIN);
         lSetString(pseudo_jr, JR_err_str, (char *) MSG_RU_JR_ERRSTR);
         lSetString(pseudo_jr, JR_queue_name,
                    lGetString(first_granted_queue, JG_qname));
         sge_job_exit(ctx, pseudo_jr, jep, this_jatep, monitor);
         lFreeElem(&pseudo_jr);
      }
   }

   DRETURN(ret);
}   

/****** qmaster/reschedule/add_to_reschedule_unknown_list() *******************
*  NAME
*     add_to_reschedule_unknown_list() -- add a job/task 
*
*  SYNOPSIS
*     lListElem* add_to_reschedule_unknown_list(lListElem *host, 
*                                               u_long32 job_number, 
*                                               u_long32 task_number, 
*                                               u_long32 state) 
*
*  FUNCTION
*     This function adds a job/task into the reschedule_unknown list of
*     a host. Jobs contained in this list won't be rescheduled back to
*     that hosts until it is sure that this job/task is not running
*     on that host anymore.
*
*     This mechanism makes it possible to reschedule jobs running on hosts
*     which are in an undefined state (no reports arrive the master).
*
*  INPUTS
*     lListElem *host      - host (EH_Type) where the jok/task was running 
*     u_long32 job_number  - job id 
*     u_long32 task_number - task id 
*     u_long32 state       - state 
*
*  RESULT
*     lListElem* - point to the element added into the reschedule_unknown_list
*                  (RU_Type)
*******************************************************************************/
lListElem* add_to_reschedule_unknown_list(sge_gdi_ctx_class_t *ctx, 
                                          lListElem *host, u_long32 job_number,
                                          u_long32 task_number, u_long32 state)
{
   lListElem* ruep = NULL;
   DENTER(TOP_LAYER, "add_to_reschedule_unknown_list");
 
   if (host) {
      ruep = lAddSubUlong(host, RU_job_number, job_number,
         EH_reschedule_unknown_list, RU_Type);
 
      DPRINTF(("RU: ADDED "sge_u32"."sge_u32" to EH_reschedule_unknown_list "
               "of host "SFN"\n", job_number, task_number, 
               lGetHost(host, EH_name)));
 
      lSetUlong(ruep, RU_task_number, task_number);
      lSetUlong(ruep, RU_state, state);
      {
         lList *answer_list = NULL;
         sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD, 
                         0, 0, lGetHost(host, EH_name), NULL, NULL,
                         host, NULL, NULL, true, true);
         answer_list_output(&answer_list);
      }
   }

   DRETURN(ruep);
}
 
/****** qmaster/reschedule/get_from_reschedule_unknown_list() *****************
*  NAME
*     get_from_reschedule_unknown_list() --  find an entry in a sublist 
*
*  SYNOPSIS
*     lListElem* get_from_reschedule_unknown_list(lListElem *host, 
*                                                 u_long32 job_number, 
*                                                 u_long32 task_number) 
*
*  FUNCTION
*     This function tries to find an entry in the reschedule_unknown_list 
*     of a host.
*      
*
*  INPUTS
*     lListElem *host      - host (EH_Type) 
*     u_long32 job_number  - job id 
*     u_long32 task_number - task id 
*
*  RESULT
*     lListElem* - NULL or valid pointer
*******************************************************************************/
static lListElem* get_from_reschedule_unknown_list(lListElem *host, 
                                            u_long32 job_number, 
                                            u_long32 task_number) 
{
   lListElem *ruep = NULL;
 
   DENTER(TOP_LAYER, "get_from_reschedule_unknown_list");
   for_each(ruep, lGetList(host, EH_reschedule_unknown_list)) {
      if (job_number == lGetUlong(ruep, RU_job_number)
          && task_number == lGetUlong(ruep, RU_task_number)) {
          break;
      }
   }

   DRETURN(ruep);
}               

/****** qmaster/reschedule/delete_from_reschedule_unknown_list() **************
*  NAME
*     delete_from_reschedule_unknown_list() -- delete a sublist entry 
*
*  SYNOPSIS
*     void delete_from_reschedule_unknown_list(lListElem *host) 
*
*  FUNCTION
*     Removes an entry of the reschedule_unknown_list of a host. 
*
*  INPUTS
*     lListElem *host - host (EH_Type) 
*******************************************************************************/
void delete_from_reschedule_unknown_list(sge_gdi_ctx_class_t *ctx, lListElem *host) 
{
   lList *rulp;
   bool changed = false;
   DENTER(TOP_LAYER, "delete_from_reschedule_unknown_list");
 
   rulp = lGetList(host, EH_reschedule_unknown_list);
   if (rulp) {
      lListElem *this, *next;
 
      next = lFirst(rulp);
      while((this = next)) {
         u_long32 state = lGetUlong(this, RU_state);
   
         next = lNext(this);

         if (state == RESCHEDULE_SKIP_JR_REMOVE
             || state == RESCHEDULE_HANDLE_JR_REMOVE) {

            DPRINTF(("RU: REMOVED "sge_u32"."sge_u32" FROM RU LIST\n",
                     lGetUlong(this, RU_job_number), lGetUlong(this, RU_task_number)));
            lRemoveElem(rulp, &this);
            changed = true;
         }
      }
      if (changed) {
         lList *answer_list = NULL;

         sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD, 
                         0, 0, lGetHost(host, EH_name), NULL, NULL,
                         host, NULL, NULL, true, true);
         answer_list_output(&answer_list);
      }
   }
   DRETURN_VOID;
}
 
/****** qmaster/reschedule/update_reschedule_unknown_list() *******************
*  NAME
*     update_reschedule_unknown_list() -- check entries in sublist 
*
*  SYNOPSIS
*     void update_reschedule_unknown_list(lListElem *host) 
*
*  FUNCTION
*     This function checks and changes the state field of the elements 
*     contained in the reschedule_unknown_list of a host. The state field 
*     containes information about the current protocol state between the 
*     master and a execution daemon for a job/task.  
*
*  INPUTS
*     lListElem *host - host (EH_Type)
*******************************************************************************/
void update_reschedule_unknown_list(sge_gdi_ctx_class_t *ctx, lListElem *host) 
{
   lListElem *ruep;
 
   DENTER(TOP_LAYER, "update_reschedule_unknown_list");
   if (host) {
      bool changed = false;
      for_each(ruep, lGetList(host, EH_reschedule_unknown_list)) {
         u_long32 state = lGetUlong(ruep, RU_state);

         if (state == RESCHEDULE_SKIP_JR_SEND_ACK) {
            lSetUlong(ruep, RU_state, RESCHEDULE_SKIP_JR_REMOVE);
            changed = true;
         } else if (state == RESCHEDULE_SKIP_JR) {
            lSetUlong(ruep, RU_state, RESCHEDULE_SKIP_JR_SEND_ACK);
            changed = true;
         }
      }

      if (changed) {   
         lList *answer_list = NULL;
         sge_event_spool(ctx, NULL, 0, sgeE_EXECHOST_MOD, 
                         0, 0, lGetHost(host, EH_name), NULL, NULL,
                         host, NULL, NULL, true, true);
         answer_list_output(&answer_list);
      }
   }
   DRETURN_VOID;
}          

/****** qmaster/reschedule/skip_restarted_job() *******************************
*  NAME
*     skip_restarted_job() -- What should we do with a job report?
*
*  SYNOPSIS
*     u_long32 skip_restarted_job(lListElem *host, lListElem *job_report, 
*                                 u_long32 job_number, u_long32 task_number) 
*
*  FUNCTION
*     This function is used within the master daemon at the place where
*     job reports arrive from the execd's. The function returns an integer 
*     which indicates what to do with a job report and which steps are
*     necessary to interfere in the protocol between master and execd.
*     In following situation it is necessary to interfere in the protocol:
*
*     (1) job A was scheduled to host X
*     (2) All queues of host X went into unknown state because
*         of network problems. 
*     (3) automatic rescheduling mechanism decided to put the job A
*         back into the list of pending jobs
*     (4) job A was scheduled to host Y
*     (5) host X came back and sends reports for job A
*
*     => now we have to ignore all reports from host X
*     => kill the old instance of job A
*     => make sure that no old stuff remains 
*
*  INPUTS
*     lListElem *host       - host (EH_Type) 
*     lListElem *job_report - job report (JR_Type) 
*     u_long32 job_number   - job id 
*     u_long32 task_number  - array task id 
*
*  RESULT
*     u_long32 - what should we do?
*         0 => process the job report 
*        >0 => skip the job report
*              2 -> try to kill the job 
*              3 -> send an ack to execd (job will be removed from filesystem)
*
*******************************************************************************/
u_long32 skip_restarted_job(lListElem *host, lListElem *job_report,
                            u_long32 job_number, u_long32 task_number) 
{
   lListElem *ruep;
   u_long32 ret = 0;
   DENTER(TOP_LAYER, "skip_restarted_job");
 
   for_each(ruep, lGetList(host, EH_reschedule_unknown_list)) {
      if (lGetUlong(ruep, RU_job_number) == job_number
          && lGetUlong(ruep, RU_task_number) == task_number) {
         u_long32 state = lGetUlong(ruep, RU_state);

         if (state == RESCHEDULE_SKIP_JR_REMOVE) {
            lSetUlong(ruep, RU_state, RESCHEDULE_SKIP_JR);
            ret = 2; 
         } else if (state == RESCHEDULE_SKIP_JR_SEND_ACK
                    || state == RESCHEDULE_SKIP_JR) {
            if (lGetUlong(job_report, JR_state) == JEXITING) {
               ret = 3;
            } else {
               ret = 1;
               DPRINTF(("RU: GOT JR\n"));
            }
         }
         break;
      }
   }
 
   DRETURN(ret);
}           

/****** qmaster/reschedule/update_reschedule_unknown_list_for_job() ***********
*  NAME
*     update_reschedule_unknown_list_for_job() -- check and change state 
*
*  SYNOPSIS
*     void update_reschedule_unknown_list_for_job(lListElem *host, 
*                                                 u_long32 job_number, 
*                                                 u_long32 task_number) 
*
*  FUNCTION
*     This function is used to keep the state field up to date which is 
*     contained in the reschedule_unknown_list entries. Only entries added for
*     parallel jobs will be changed.
*      
*
*  INPUTS
*     lListElem *host      - host (EH_Type) 
*     u_long32 job_number  - job id 
*     u_long32 task_number - task id 
*******************************************************************************/
void update_reschedule_unknown_list_for_job(lListElem *host, 
                                            u_long32 job_number,
                                            u_long32 task_number) 
{
   lListElem *ruep;
 
   DENTER(TOP_LAYER, "update_reschedule_unknown_list_for_job");

   if (host) {
      for_each(ruep, lGetList(host, EH_reschedule_unknown_list)) {
         if (lGetUlong(ruep, RU_job_number) == job_number
             && lGetUlong(ruep, RU_task_number) == task_number) {
            if (lGetUlong(ruep, RU_state) == RESCHEDULE_HANDLE_JR_WAIT) {

               lSetUlong(ruep, RU_state, RESCHEDULE_HANDLE_JR_REMOVE);
               DPRINTF(("RU: DECREMENTED PE-TAG OF "sge_u32"."sge_u32"\n",
                  job_number, task_number));
            }
            break;
         }
      }
   }
   DEXIT;
}      

/****** qmaster/reschedule/update_reschedule_unknown_timout_values() **********
*  NAME
*     update_reschedule_unknown_timout_values() -- change cached timeout value 
*
*  SYNOPSIS
*     void update_reschedule_unknown_timout_values(const char *config_name) 
*
*  FUNCTION
*     This functions changes all reschedule unknown values cached within
*     the exec host objects. 'config_name' may either be 'global' or
*     the name of a local configuration. 
*
*  INPUTS
*     const char *config_name - configuration name 
*
*  RESULT
*     void - none
******************************************************************************/
void update_reschedule_unknown_timout_values(const char *config_name) 
{
   lListElem *host = NULL;
   lList *master_exechost_list = *object_type_get_master_list(SGE_TYPE_EXECHOST);

   DENTER(TOP_LAYER, "update_reschedule_unknown_timout_values");

   if (strcmp(SGE_GLOBAL_NAME, config_name) == 0) {
      lListElem *global_exechost_elem   = NULL;
      lListElem *template_exechost_elem = NULL;

      global_exechost_elem   = host_list_locate(master_exechost_list, SGE_GLOBAL_NAME); 
      template_exechost_elem = host_list_locate(master_exechost_list, SGE_TEMPLATE_NAME); 

      for_each(host, master_exechost_list) {
         if ((host != global_exechost_elem) && (host != template_exechost_elem)) {
            update_reschedule_unknown_timeout(host);
         }
      }
   } else if (strcmp(SGE_TEMPLATE_NAME, config_name) != 0 ) {
      host = host_list_locate(master_exechost_list, config_name); 
      update_reschedule_unknown_timeout(host);
   }

   DRETURN_VOID;
}

/****** qmaster/reschedule/update_reschedule_unknown_timeout() ****************
*  NAME
*     update_reschedule_unknown_timeout() -- Cache the timeout value in host 
*
*  SYNOPSIS
*     void update_reschedule_unknown_timeout(lListElem *host) 
*
*  FUNCTION
*     The Function copies the timout value of the global/local configuration 
*     for a certain host within the exec host object.  
*
*  INPUTS
*     lListElem *host - exec host (EH_Type)
*
*  RESULT
*     void - none
******************************************************************************/
static void update_reschedule_unknown_timeout(lListElem *host) 
{
   DENTER(TOP_LAYER, "update_reschedule_unknown_timeout");
   
   if (host != NULL) {
      lListElem *conf_entry = NULL; /* CF_Type */
      const char *hostname = lGetHost(host, EH_name);
      u_long32 timeout = lGetUlong(host, EH_reschedule_unknown);
     
      conf_entry = sge_get_configuration_entry_by_name(hostname,
                                                       "reschedule_unknown");
      if (conf_entry != NULL) {
         const char *value = lGetString(conf_entry, CF_value);
         
         if (parse_ulong_val(NULL, &timeout, TYPE_TIM, value, NULL, 0) == 0) {
            ERROR((SGE_EVENT, MSG_OBJ_RESCHEDULEUNKN_SS, hostname, value));
            timeout = 0;
         }
         
         lFreeElem(&conf_entry);
      } else {
         timeout = 0;
      }
      
      DPRINTF(("%s: reschedule_unknown timeout for host "SFN" is "sge_u32"\n", 
               SGE_FUNC, hostname, timeout));  
         
      lSetUlong(host, EH_reschedule_unknown, timeout); 
   } else {
      DPRINTF(("%s: host == NULL\n", SGE_FUNC));
   }
   
   DRETURN_VOID; 
} 

/****** qmaster/reschedule/reschedule_unknown_timeout() ***********************
*  NAME
*     reschedule_unknown_timeout() -- return the time to wait before resch. 
*
*  SYNOPSIS
*     u_long32 reschedule_unknown_timeout(lListElem *hep) 
*
*  FUNCTION
*     This function returns the time to wait before rescheduling of
*     jobs running in hep will be initiated.  
*
*  INPUTS
*     lListElem *hep - host (EH_Type) 
*
*  RESULT
*     u_long32 - time in seconds
*******************************************************************************/
static u_long32 reschedule_unknown_timeout(lListElem *hep) 
{
   static int not_init = 1;
   u_long32 timeout = 0;
   const char *host = NULL;
 
   DENTER(TOP_LAYER, "reschedule_unknown_timeout");
 
   host = lGetHost(hep, EH_name);
   timeout = lGetUlong(hep, EH_reschedule_unknown);

   /* cache reschedule_unknown parameter in execd host to prevent host name resolving */
   if (mconf_is_new_config() || not_init)
   {
      lListElem *conf_entry = NULL;
      
      if ((conf_entry = sge_get_configuration_entry_by_name(host, "reschedule_unknown")) != NULL)
      {
         if (parse_ulong_val(NULL, &timeout, TYPE_TIM, lGetString(conf_entry, CF_value), NULL, 0) == 0)
         {
            ERROR((SGE_EVENT, MSG_OBJ_RESCHEDULEUNKN_SS, host, lGetString(conf_entry, CF_value)));
            timeout = 0;
         }
         
         lFreeElem(&conf_entry);
      }
   
      DPRINTF(("%s: reschedule_unknown timeout for host %s is "sge_u32"\n", SGE_FUNC, host, timeout));
      
      lSetUlong(hep, EH_reschedule_unknown, timeout);
      
      not_init = 0;
   }
   
   DRETURN(timeout);
}


/****** qmaster/reschedule/reschedule_unknown_trigger() ***********************
*  NAME
*     reschedule_unknown_trigger() -- wind up timer for auto rescheduling 
*
*  SYNOPSIS
*     void reschedule_unknown_trigger(lListElem *hep) 
*
*  FUNCTION
*     This function winds up a timer used to trigger the automatic
*     rescheduling mechanism for a certain host. The initial timout value
*     will be the "reschedule_unknown" value in the global/local configuration
*     plus some time added with the reschedule_add_additional_time() function
*
*  INPUTS
*     lListElem *hep - host EH_Type 
******************************************************************************/
void reschedule_unknown_trigger(lListElem *hep) 
{
   u_long32 timeout;

   DENTER(TOP_LAYER, "reschedule_unknown_trigger"); 

   timeout = reschedule_unknown_timeout(hep);
 
   if (timeout) {
      const char *host = lGetHost(hep, EH_name);
      u_long32 when = time(NULL) + timeout + add_time;
      te_event_t ev = NULL;

      DPRINTF(("RU: Autorescheduling enabled for host "SFN". ("sge_u32 " sec)\n", host, timeout + add_time));
      
      ev = te_new_event((time_t)when, TYPE_RESCHEDULE_UNKNOWN_EVENT, ONE_TIME_EVENT, timeout, 0, host);
      te_add_event(ev);
      te_free_event(&ev);
   }
   DEXIT;
}       

/****** qmaster/reschedule/reschedule_add_additional_time() *******************
*  NAME
*     reschedule_add_additional_time() -- set additional time to wait before r. 
*
*  SYNOPSIS
*     void reschedule_add_additional_time(u_long32 time) 
*
*  FUNCTION
*     This function sets a time value which will be added to the
*     "reschedule_unknown" time. The master will wait this time after
*     a host went into unknown state before it initiates rescheduling of jobs 
*
*  INPUTS
*     u_long32 time - time in seconds
******************************************************************************/
void reschedule_add_additional_time(u_long32 time) 
{
   DENTER(TOP_LAYER, "reschedule_add_additional_time");
   add_time = time;
   DEXIT;
}  

void
remove_from_reschedule_unknown_list(sge_gdi_ctx_class_t *ctx,
                                    lListElem *host, u_long32 job_number,
                                    u_long32 task_number)
{
   DENTER(TOP_LAYER, "remove_from_reschedule_unknown_list");
   if (host) {
      lList *unknown_list = lGetList(host, EH_reschedule_unknown_list);
      lListElem *elem;

      for_each(elem, unknown_list) {
         if (lGetUlong(elem, RU_job_number) == job_number &&
             lGetUlong(elem, RU_task_number) == task_number) {
            break;
         }
      }

      if (elem != NULL) {
         lRemoveElem(unknown_list, &elem);
         sge_event_spool(ctx, NULL, 0, sgeE_EXECHOST_MOD,
                         0, 0, lGetHost(host, EH_name), NULL, NULL,
                         host, NULL, NULL, true, true);
      }
   }

   DRETURN_VOID;
}

void
remove_from_reschedule_unknown_lists(sge_gdi_ctx_class_t *ctx,
                                     u_long32 job_number,
                                     u_long32 task_number)
{
   lListElem *host;

   DENTER(TOP_LAYER, "remove_from_reschedule_unknown_lists");

   for_each(host, *object_type_get_master_list(SGE_TYPE_EXECHOST)) {
      remove_from_reschedule_unknown_list(ctx, host, job_number, task_number);
   }

   DRETURN_VOID;
}
 
