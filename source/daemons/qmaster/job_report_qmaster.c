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
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/sge_prog.h"
#include "uti/sge_signal.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_ack.h"

#include "spool/sge_spooling.h"

#include "sge_report_execd.h"
#include "execution_states.h"
#include "job_report_qmaster.h"
#include "job_exit.h"
#include "sge_event_master.h"
#include "sge_job_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_pe_qmaster.h"
#include "reschedule.h"
#include "sge.h"
#include "sge_reporting_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "msg_daemons_common.h"
#include "msg_qmaster.h"

/*
 * When the master task of a tightly integrated job exits
 * we wait for the slave execds to report job finish.
 * Do not wait forever, but only n report intervals.
 * Could be made configurable in qmaster_params.
 */
#define MAX_MASTER_TASK_FINISH_BEFORE_EXIT 20

static char *status2str(u_long32 status);

#define is_running(state) (state==JWRITTEN || state==JRUNNING|| state==JWAITING4OSJID)

static char *status2str(
u_long32 status 
) {
   char *s;

   switch (status) {
   case JTRANSFERING:
      s = "JTRANSFERING";
      break;
   case JRUNNING:
      s = "JRUNNING";
      break;
   case JFINISHED:
      s = "JFINISHED";
      break;
   case JIDLE:
      s = "JIDLE";
      break;
   default:
      s = "<unknown>";
      break;
   }

   return s;
}

/*
 * Protocol between qmaster and execd for tightly integrated parallel jobs:
 * ========================================================================
 * 
 * Job delivery: 
 *    - all slave hosts are notified about the job
 *    - once all slave hosts acknowledged having got the job,
 *      the master task is delivered
 *
 * During job runtime
 *    - master host reports master task usage
 *    - slave tasks get started, slave exec hosts report usage
 *      and slave task exit
 *
 * Job finish:
 *    - master tasks reports finish
 *    - all slave hosts are tagged (in ja_task gdil)
 *    - all slave hosts are notified about job finish (ACK_SIGNAL_SLAVE)
 *      triggeres kill of still running tasks and sending of a final report
 *    - master task report is acknowledged with ACK_JOB_REPORT_RESEND,
 *      triggering flushed resend of the master task finish report
 *    - if slave hosts still report the job running, resend the ACK_SIGNAL_SLAVE
 *    - when slave host sends final report, it is untagged (in ja_task gdil)
 *    - when qmaster gets resent master task final report
 *      - and all slaves are done, treat the job as finished
 *      - not all slaves are done, again trigger resend of master task report
 *    - waiting for all slave hosts to finish is terminated when the master task 
 *      report got received more than MAX_MASTER_TASK_FINISH_BEFORE_EXIT times.
 *      In this case (which happens for example when a slave host is down),
 *      the job is treated as finished.
 */

/****** job_report_qmaster/process_job_report() ********************************
*  NAME
*     process_job_report() -- process a job report from execd
*
*  SYNOPSIS
*     void 
*     process_job_report(sge_gdi_ctx_class_t *ctx, lListElem *report,
*                        lListElem *hep, char *rhost, char *commproc,
*                        sge_pack_buffer *pb, monitoring_t *monitor) 
*
*  FUNCTION
*     Process 'report' containing a job report list from 
*     'commproc' at 'rhost'.
*  
*     The 'pb' may get used to collect requests that will be 
*     generated in this process. The caller should reply it
*     to the sender of this job report list if 'pb' remains
*     not empty.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - gdi context
*     lListElem *report        - the job report
*     lListElem *hep           - the host for which we got the job report
*     char *rhost              - name of the remote host having sent the report
*     char *commproc           - the commproc name
*     sge_pack_buffer *pb      - pack buffer for sending answers (ACK)
*     monitoring_t *monitor    - monitor
*
*  NOTES
*     MT-NOTE: process_job_report() is MT safe 
*******************************************************************************/
void process_job_report(sge_gdi_ctx_class_t *ctx, lListElem *report,
                       lListElem *hep, char *rhost, char *commproc,
                       sge_pack_buffer *pb, monitoring_t *monitor)
{
   lList* jrl = lGetList(report, REP_list); /* JR_Type */
   lListElem *jep, *jr, *ep, *jatep = NULL; 
   object_description *object_base = object_type_get_object_description();

   char job_id_buffer[MAX_STRING_SIZE];
   dstring job_id_dstring;
   const char *job_id_string;

   DENTER(TOP_LAYER, "process_job_report");

   sge_dstring_init(&job_id_dstring, job_id_buffer, MAX_STRING_SIZE);

   DPRINTF(("received job report with %d elements:\n", lGetNumberOfElem(jrl)));

   /* 
   ** first process job reports of sub tasks to ensure this we put all these 
   ** job reports to the top of the 'jrl' list this is necessary to ensure 
   ** slave tasks get accounted on a shm machine 
   ** TODO: could be sorted on execd
   */
   {
      static lSortOrder *jr_sort_order = NULL;
      if (!jr_sort_order) {
         DPRINTF(("parsing job report sort order\n"));
         jr_sort_order = lParseSortOrderVarArg(JR_Type, "%I-", 
            JR_pe_task_id_str);
      }
      lSortList(jrl, jr_sort_order);
   }

   /*
   ** now check all job reports found in step 1 are 
   ** removed from job report list
   */
   for_each(jr, jrl) {
      const char *queue_name;
      const char *pe_task_id_str = lGetString(jr, JR_pe_task_id_str);
      u_long32 status = 0;
      lListElem *petask = NULL;
      int fret;
      u_long32 jobid, rstate = 0, jataskid = 0;

      jobid = lGetUlong(jr, JR_job_number);
      jataskid = lGetUlong(jr, JR_ja_task_number);
      rstate = lGetUlong(jr, JR_state);

      jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, jobid);
      if (jep != NULL) {
         jatep = lGetElemUlong(lGetList(jep, JB_ja_tasks), JAT_task_number, jataskid);

         if (jatep != NULL) {
            status = lGetUlong(jatep, JAT_status);

            if (pe_task_id_str != NULL) {
               petask = lGetSubStr(jatep, PET_id, pe_task_id_str, JAT_task_list); 
            }
         }
      }

      /* build a job_id_string once - we need it in many places */
      job_id_string = job_get_id_string(jobid, jataskid, pe_task_id_str, &job_id_dstring);

      /* handle protocol to execd for all jobs which are
         already finished and maybe rescheduled */
      /* RU: */
      fret = skip_restarted_job(hep, jr, jobid, jataskid);
      if (fret > 0) {
         if (fret == 2) {
            pack_ack(pb, ACK_SIGNAL_JOB, jobid, jataskid, NULL);
         } else if (fret == 3) {
            lList *answer_list = NULL;
            u_long32 state = 0;

            pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
            /*
             * Check for deferred startup due to manual rescheduling.
             * Reset JDEFERRED_REQ to allow rescheduled job to run.
             * This is now OK since the initial job is going to die.
             */
            if (jep && jatep) {
               state = lGetUlong(jatep, JAT_state);
               if (state & JDEFERRED_REQ) {
                  CLEARBIT(JDEFERRED_REQ, state);
                  lSetUlong(jatep, JAT_state, state);
                  sge_event_spool(ctx, 
                        &answer_list, 0, sgeE_JATASK_MOD,
                        jobid, jataskid, NULL, NULL, NULL,
                        jep, jatep, NULL, true, true);
               }
            }
         }
         continue;
      }


      if ((queue_name = lGetString(jr, JR_queue_name)) == NULL) {
         queue_name = MSG_OBJ_UNKNOWNQ;
      }
      
      switch (rstate) {
      case JWRITTEN:
      case JRUNNING:   
      case JWAITING4OSJID:
         if (jep && jatep) {
            lList *answer_list = NULL;

            switch (status) {
            case JTRANSFERING:
            case JRUNNING:   
               /* 
                * If a ja_task was deleted while the execd was down, we'll
                * get a "job running" report when the execd starts up again.
                * The ja_task will be deleted by a timer triggered event
                * (TYPE_SIGNAL_RESEND_EVENT), but this can take up to one
                * minute - better send a kill signal immediately.
                */
               if (ISSET(lGetUlong(jatep, JAT_state), JDELETED)) {
                  DPRINTF(("Received report from "sge_u32"."sge_u32
                           " which is already in \"deleted\" state. "
                           "==> send kill signal\n", jobid, jataskid));

                  pack_ack(pb, ACK_SIGNAL_JOB, jobid, jataskid, NULL);
               }

               if (pe_task_id_str == NULL) {
                    
                  /* store unscaled usage directly in job */
                  lXchgList(jr, JR_usage, lGetListRef(jatep, JAT_usage_list));

                  /* update jobs scaled usage list */
                  lSetList(jatep, JAT_scaled_usage_list, 
                      lCopyList("scaled", lGetList(jatep, JAT_usage_list)));
                  scale_usage(lGetList(hep, EH_usage_scaling_list), 
                              lGetList(jatep, JAT_previous_usage_list),
                              lGetList(jatep, JAT_scaled_usage_list));
                 
                  if (status == JTRANSFERING) { /* got async ack for this job */
                     DPRINTF(("--- transfering job "SFN" is running\n", job_id_string));
                     sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_ARRIVED, COMMIT_DEFAULT, monitor); /* implicitly sending usage to schedd */
                     cancel_job_resend(jobid, jataskid);
                  } else {
                     /* need to generate a job event for new usage 
                      * the timestamp should better come from report object
                      */
                     /* jatask usage is not spooled (?) */
                     sge_add_list_event( 0, sgeE_JOB_USAGE, 
                                        jobid, jataskid, NULL, NULL,
                                        lGetString(jep, JB_session),
                                        lGetList(jatep, JAT_scaled_usage_list));
                     lList_clear_changed_info(lGetList(jatep, JAT_scaled_usage_list));
                  }
               } else {
                  /* register running task qmaster will log accounting for all registered tasks */
                  bool new_task = false;

                  /* do we expect a pe task report from this host? */
                  if (ja_task_is_tightly_integrated(jatep) &&
                      lGetElemHost(lGetList(jatep, JAT_granted_destin_identifier_list), JG_qhostname, rhost)) {
                    
                    /* is the task already known (object was created earlier)? */
                    if (petask == NULL) {
                        /* here qmaster hears the first time about this task
                           and thus adds it to the task list of the appropriate job */
                        new_task = true;
                        DPRINTF(("--- task (#%d) "SFN" -> running\n", 
                           lGetNumberOfElem(lGetList(jatep, JAT_task_list)), job_id_string));
                        petask = lAddSubStr(jatep, PET_id, pe_task_id_str, JAT_task_list, PET_Type);
                        lSetUlong(petask, PET_status, JRUNNING);
                        /* JG: TODO: this should be delivered from execd! */
                        lSetUlong(petask, PET_start_time, sge_get_gmt());
                        lSetList(petask, PET_granted_destin_identifier_list, NULL);
                        if ((ep=lAddSubHost(petask, JG_qhostname, rhost, PET_granted_destin_identifier_list, JG_Type))) {
                           lSetString(ep, JG_qname, queue_name);
                        }
                    }

                    /* store unscaled usage directly in sub-task */
                    lXchgList(jr, JR_usage, lGetListRef(petask, PET_usage));

                    /* update task's scaled usage list */
                    lSetList(petask, PET_scaled_usage,
                             lCopyList("scaled", lGetList(petask, PET_usage)));

                    scale_usage(lGetList(hep, EH_usage_scaling_list), 
                                lGetList(petask, PET_previous_usage),
                                lGetList(petask, PET_scaled_usage));

                    /* notify scheduler of task usage event */
                    if (new_task) {
                       sge_event_spool(ctx,
                                       &answer_list, 0, sgeE_PETASK_ADD, 
                                       jobid, jataskid, pe_task_id_str, NULL,
                                       lGetString(jep, JB_session),
                                       jep, jatep, petask, true, true);
                    } else {
                       sge_add_list_event( 0, sgeE_JOB_USAGE, 
                                          jobid, jataskid, pe_task_id_str, 
                                          NULL, lGetString(jep, JB_session),
                                          lGetList(petask, PET_scaled_usage));
                    }
                    answer_list_output(&answer_list);
                  } else if (lGetUlong(jatep, JAT_status) != JFINISHED) {
                     lListElem *jg;
                     const char *shouldbe_queue_name;
                     const char *shouldbe_host_name;

                     if (!(jg = lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)))) {
                        shouldbe_queue_name = MSG_OBJ_NOTRUNNING;
                        shouldbe_host_name = MSG_OBJ_NOTRUNNING;
                     } else {
                        if ((shouldbe_queue_name = lGetString(jg, JG_qname)) == NULL) {
                           shouldbe_queue_name = MSG_OBJ_UNKNOWN;
                        }
                        if ((shouldbe_host_name = lGetHost(jg, JG_qhostname)) == NULL) {
                           shouldbe_host_name = MSG_OBJ_UNKNOWN;
                        }
                     }
                     /* should never happen */
                     ERROR((SGE_EVENT, MSG_JOB_REPORTEXITQ_SUUSSSSS, 
                            rhost, sge_u32c(jobid), sge_u32c(jataskid), 
                            pe_task_id_str?pe_task_id_str:MSG_MASTER, 
                            queue_name, shouldbe_queue_name, 
                            shouldbe_host_name, 
                            status2str(lGetUlong(jatep, JAT_status))));
                  }
               }

               /*
                * once a day write an intermediate usage record to the 
                * reporting file to have correct daily usage reporting with
                * long running jobs
                */
               if (reporting_is_intermediate_acct_required(jep, jatep, petask)) {
                  /* write intermediate usage */
                  reporting_create_acct_record(ctx, NULL, jr, jep, jatep, true);

                  /* this action has changed the ja_task/pe_task - spool */
                  if (pe_task_id_str != NULL) {
                     /* JG: TODO we would need a PETASK_MOD event here!
                      * for spooling only, the ADD event is OK
                      */
                     sge_event_spool(ctx,
                                     &answer_list, 0, sgeE_PETASK_ADD, 
                                     jobid, jataskid, pe_task_id_str, NULL,
                                     lGetString(jep, JB_session),
                                     jep, jatep, petask, false, true);
                  } else {
                     sge_event_spool(ctx,
                                     &answer_list, 0, sgeE_JATASK_MOD, 
                                     jobid, jataskid, NULL, NULL,
                                     lGetString(jep, JB_session),
                                     jep, jatep, NULL, false, true);
                  }
                  answer_list_output(&answer_list);
               }
               break;
            default:
               ERROR((SGE_EVENT, MSG_JOB_REPORTRUNQ_SUUSSU, 
                     rhost, sge_u32c(jobid), sge_u32c(jataskid), 
                     pe_task_id_str?pe_task_id_str:"master", 
                     queue_name, sge_u32c(status)));
               break;
            } 
         } else {
            /* execd reports a running job that is unknown */
            /* signal this job to kill it at execd 
               this can be caused by a qdel -f while 
               execd was unreachable or by deletion of 
               the job in qmasters spool dir + qmaster 
               restart  
               retry is triggered if execd reports
               this job again as running
            */
            ERROR((SGE_EVENT, MSG_JOB_REPORTRUNFALSE_SUUSS, rhost, 
                   sge_u32c(jobid), sge_u32c(jataskid), 
                   pe_task_id_str?pe_task_id_str:MSG_MASTER, queue_name));
            pack_ack(pb, ACK_SIGNAL_JOB, jobid, jataskid, NULL);
         }
         break;
         
      case JSLAVE:
         /*
          * Job report from an execd running slave tasks of a
          * tightly integrated parallel job:
          * - the first report cancels resending of slave job delivery
          * - once all slave jobs have been delivered, the master task is started
          * - during the job runtime, slave job reports are sent in the load report interval
          * - when the master task has finished, a slave job report will be acknowledged
          *   with ACK_SIGNAL_SLAVE
          * - the final slave job report contains a usage list with at least a (dummy) 
          *   exit_status - when we receive such a final slave job report, we trigger 
          *   exit of the job
          */  
         if (jep == NULL || jatep == NULL) {
            DPRINTF(("send cleanup request for slave job "SFN"\n", job_id_string));
            pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
         } else {
            /* must be ack (or later slave job report) for slave job */
            lListElem *first_at_host;

            /* we lookup the first gdil for the reporting host
             * there might be multiple gdil for one host if the pe job spawns multiple queues
             * but only the first one is tagged
             */
            first_at_host = lGetElemHost(lGetList(jatep, JAT_granted_destin_identifier_list), JG_qhostname, rhost);
            if (first_at_host != NULL) {
               /* If the job is being delivered, we first deliver it to the slave hosts,
                * once all slave hosts have been notified, we start the master task.
                * During this time the job is in status JTRANSFERING.
                */
               if (lGetUlong(jatep, JAT_status) == JTRANSFERING) {
                  /* if tag is still 1, this is the ACK for slave notification */
                  if (lGetUlong(first_at_host, JG_tag_slave_job) != 0) {
                     DPRINTF(("slave job "SFN" arrived on host %s\n", job_id_string, rhost));
                     lSetUlong(first_at_host, JG_tag_slave_job, 0);

                     /* should trigger a fast delivery of the job to master execd 
                      * script but only when all other slaves have also arrived
                      */ 
                     if (is_pe_master_task_send(jatep)) {
                        /* triggers direct job delivery to master execd */
                        lSetString(jatep, JAT_master_queue, lGetString(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_qname));

                        DPRINTF(("trigger retry of job delivery to master execd\n"));
                        lSetUlong(jatep, JAT_start_time, 0);
                        cancel_job_resend(jobid, jataskid);
                        trigger_job_resend(sge_get_gmt(), NULL, jobid, jataskid, 0);
                     }
                  }
               } else {
                  /* This is a slave execd report while the job is running.
                   * When the master task has finished, the hosts gdil is tagged ==> we send a ACK_SIGNAL_SLAVE
                   * When the slave report contains JR_usage with exit_status, we are done, untag gdil, pack_job_exit
                   */
                  if (lGetUlong(first_at_host, JG_tag_slave_job) != 0) {
                     if (lGetElemStr(lGetList(jr, JR_usage), UA_name, "exit_status") != NULL) {
                        /* the job is done in this slave exec host */
                        lSetUlong(first_at_host, JG_tag_slave_job, 0);
                        DPRINTF(("slave job "SFN" exited on host %s\n", job_id_string, rhost));
                        pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
                     } else {
                        DPRINTF(("trigger exit of slave job "SFN" on host %s\n", job_id_string, rhost));
                        pack_ack(pb, ACK_SIGNAL_SLAVE, jobid, jataskid, pe_task_id_str);
                     }
                  }
               }
            } else {
               /* clear state with regards to slave controlled container
                * we got a slave report, but GDIL doesn't contain an object for this host!
                * job got rescheduled while the reporting host was down (reschedule unknown)
                */
               lListElem *host;

               host = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, rhost);
               update_reschedule_unknown_list_for_job(host, jobid, jataskid);

               DPRINTF(("RU: CLEANUP FOR SLAVE JOB "SFN" on host "SFN"\n", job_id_string, rhost));

               /* clean up */
               pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
            }
         }
         break;
      case JEXITING:
      {
         int skip_job_exit = 0;

         if (jep == NULL || jatep == NULL || (jep != NULL && status == JFINISHED)) {
            /* must be retry of execds job exit
             * or job was deleted using "qdel -f"
             * while execd was down or ...
             */
            DPRINTF(("exiting job "SFN" does not exist\n", job_id_string));
         } else {
            /* job exited */
            if (pe_task_id_str == NULL) {
               /* store unscaled usage directly in job */
               lXchgList(jr, JR_usage, lGetListRef(jatep, JAT_usage_list));

               /* update jobs scaled usage list */
               lSetList(jatep, JAT_scaled_usage_list,
                  lCopyList("scaled", lGetList(jatep, JAT_usage_list)));
               scale_usage(lGetList(hep, EH_usage_scaling_list),
                           lGetList(jatep, JAT_previous_usage_list),
                           lGetList(jatep, JAT_scaled_usage_list));

               /* additional handling for tightly integrated parallel jobs */
               if (ja_task_is_tightly_integrated(jatep)) {
                  /* when we get the first job finish report from the master task of a
                   * tightly integrated parallel job,
                   * we tag all gdil entries (including the master gdil_ep)
                   * first report is when the master task gdil_ep is not yet tagged!
                   */
                  u_long32 master_tag = lGetUlong(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_tag_slave_job);
                  if (master_tag == 0) {
                     ack_all_slaves(ctx, jobid, jataskid, jatep, ACK_SIGNAL_SLAVE);
                     tag_all_host_gdil(jatep);
                     skip_job_exit = 1;
                     pack_ack(pb, ACK_JOB_REPORT_RESEND, jobid, jataskid, pe_task_id_str);
                  } else {
                     /* 
                      * skip sge_job_exit() and pack_job_exit() in case there
                      * are still running tasks, since execd resends job exit
                      * we do not wait forever - skip waiting after n master task finish reports
                      */
                     if (master_tag < MAX_MASTER_TASK_FINISH_BEFORE_EXIT) {
                        if (!all_slave_jobs_finished(jatep)) {
                           skip_job_exit = 1;
                           master_tag++;
                           lSetUlong(lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)), JG_tag_slave_job, master_tag);
                           pack_ack(pb, ACK_JOB_REPORT_RESEND, jobid, jataskid, pe_task_id_str);
                        }
                     } else {
                        WARNING((SGE_EVENT, MSG_JOB_NOTALLSLAVEEND_S, job_id_string));
                     }
                  }
               }

               switch (status) {
               case JRUNNING:
               case JTRANSFERING:
                  if (!skip_job_exit) {
                     DPRINTF(("--- running job "SFN" is exiting, previous status: "SFN"\n", 
                        job_id_string, (status==JTRANSFERING)?"transfering":"running"));

                     sge_job_exit(ctx, jr, jep, jatep, monitor);
                  } else {
                     u_long32 failed = lGetUlong(jr, JR_failed);

                     if (failed == SSTATE_FAILURE_AFTER_JOB && 
                           !lGetString(jep, JB_checkpoint_name)) {

                        if (!ISSET(lGetUlong(jatep, JAT_state), JDELETED)) {
                           job_mark_job_as_deleted(ctx, jep, jatep);
                           ERROR((SGE_EVENT, MSG_JOB_MASTERTASKFAILED_S, job_id_string));
                        }
                     }
                  }
                  break;
               case JFINISHED:
                  /* must be retry */
                  skip_job_exit = 1;
                  break;
               default:
                  ERROR((SGE_EVENT, MSG_JOB_REPORTEXITJ_UUU,
                        sge_u32c(jobid), sge_u32c(jataskid), sge_u32c(status)));
                  break;
               }
            } else {
               if (ja_task_is_tightly_integrated(jatep) &&
                   lGetElemHost(lGetList(jatep, JAT_granted_destin_identifier_list), JG_qhostname, rhost)) {
                  /* 
                   * here we get usage of tasks that ran on slave/master execd's
                   * we store the pe task id of finished pe tasks in the ja task
                   * to prevent multiple handling of pe task finish in case 
                   * execd resends job report.
                   */

                  if (ja_task_add_finished_pe_task(jatep, pe_task_id_str)) {
                     bool known_pe_task = true; /* did this pe task show up
                                                   earlier (USAGE report) */

                     if (petask == NULL) {
                        known_pe_task = false;
                        petask = lAddSubStr(jatep, PET_id, pe_task_id_str, 
                                            JAT_task_list, PET_Type);
                        lSetUlong(petask, PET_status, JRUNNING);
                     }

                     /* store unscaled usage directly in sub-task */
                     /* lXchgList(jr, JR_usage, lGetListRef(task, JB_usage_list)); */
                     /* copy list because we need to keep usage in jr for sge_log_dusage() */
                     lSetList(petask, PET_usage, lCopyList(NULL, lGetList(jr, JR_usage)));

                     /* update task's scaled usage list */
                     lSetList(petask, PET_scaled_usage,
                              lCopyList("scaled", lGetList(petask, PET_usage)));
                     scale_usage(lGetList(hep, EH_usage_scaling_list), 
                                 lGetList(petask, PET_previous_usage),
                                 lGetList(petask, PET_scaled_usage));


                     if (lGetUlong(petask, PET_status)==JRUNNING ||
                         lGetUlong(petask, PET_status)==JTRANSFERING) {
                        u_long32 failed = lGetUlong(jr, JR_failed);

                        DPRINTF(("--- petask "SFN" -> final usage\n", job_id_string));
                        lSetUlong(petask, PET_status, JFINISHED);

                        reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);

                        /* add tasks (scaled) usage to past usage container */
                        {
                           lListElem *container = lGetSubStr(jatep, PET_id, PE_TASK_PAST_USAGE_CONTAINER, JAT_task_list);
                           if (container == NULL) {
                              lList *answer_list = NULL;
                              container = pe_task_sum_past_usage_list(lGetList(jatep, JAT_task_list), petask);
                              /* usage container will be spooled */
                              sge_event_spool(ctx,
                                            &answer_list, 0, sgeE_PETASK_ADD, 
                                            jobid, jataskid, PE_TASK_PAST_USAGE_CONTAINER, NULL, lGetString(jep, JB_session),  
                                            jep, jatep, container, true, true);
                              answer_list_output(&answer_list);
                           } else {
                              lList *answer_list = NULL;

                              pe_task_sum_past_usage(container, petask);
                              /* create list event for the USAGE_CONTAINER */
                              sge_add_list_event(0, sgeE_JOB_USAGE, 
                                                 jobid, jataskid, 
                                                 PE_TASK_PAST_USAGE_CONTAINER, 
                                                 NULL,
                                                 lGetString(jep, JB_session),
                                                 lGetList(container, PET_scaled_usage));
                              /* usage container will be spooled */
                              /* JG: TODO: it is not really a sgeE_PETASK_ADD,
                               * but a sgeE_PETASK_MOD. We don't have this event
                               * yet. For spooling only, the add event will do
                               */
                              sge_event_spool(ctx,
                                            &answer_list, 0, sgeE_PETASK_ADD, 
                                            jobid, jataskid, PE_TASK_PAST_USAGE_CONTAINER, NULL, lGetString(jep, JB_session),  
                                            jep, jatep, container, false, true);
                              answer_list_output(&answer_list);
                           }
                        }

                        /* remove pe task from job/jatask */
                        if (known_pe_task) {
                           lList *answer_list = NULL;
                           sge_event_spool(ctx,
                                           &answer_list, 0, sgeE_PETASK_DEL, 
                                          jobid, jataskid, pe_task_id_str, 
                                          NULL, NULL, NULL, NULL, NULL, 
                                          true, true);
                           answer_list_output(&answer_list);
                        }
                        lRemoveElem(lGetList(jatep, JAT_task_list), &petask);
                        
                        /* get rid of this job in case a task died from XCPU/XFSZ or 
                           exited with a core dump */
                        if (failed==SSTATE_FAILURE_AFTER_JOB
                              && (ep=lGetElemStr(lGetList(jr, JR_usage), UA_name, "signal"))) {
                           u_long32 sge_signo = (u_long32)lGetDouble(ep, UA_value);

                           switch (sge_signo) {
                           case SGE_SIGXFSZ:
                              INFO((SGE_EVENT, MSG_JOB_FILESIZEEXCEED_SSUU, 
                                   pe_task_id_str, rhost, sge_u32c(jobid), sge_u32c(jataskid)));
                              break;
                           case SGE_SIGXCPU:
                              INFO((SGE_EVENT, MSG_JOB_CPULIMEXCEED_SSUU, 
                                    pe_task_id_str, rhost, sge_u32c(jobid), sge_u32c(jataskid)));
                              break;
                           default: 
                              INFO((SGE_EVENT, MSG_JOB_DIEDTHROUGHSIG_SSUUS, 
                                   pe_task_id_str, rhost, sge_u32c(jobid), sge_u32c(jataskid), sge_sig2str(sge_signo)));
                              break;
                           }   
                        } else if (failed==0) {
                           INFO((SGE_EVENT, MSG_JOB_TASKFINISHED_SSUU, 
                              pe_task_id_str, rhost, sge_u32c(jobid), sge_u32c(jataskid)));
                        } else {
                           INFO((SGE_EVENT, MSG_JOB_TASKFAILED_SSUUU,
                              pe_task_id_str, rhost, sge_u32c(jobid), sge_u32c(jataskid), sge_u32c(failed)));
                        }

                        if (failed == SSTATE_FAILURE_AFTER_JOB && 
                              !lGetString(jep, JB_checkpoint_name)) {
                           if (!ISSET(lGetUlong(jatep, JAT_state), JDELETED)) {
                              job_mark_job_as_deleted(ctx, jep, jatep);
                              ERROR((SGE_EVENT, MSG_JOB_JOBTASKFAILED_S, job_id_string));
                           }
                        }
                     }
                  }
               } else if (status != JFINISHED) {
                  lListElem *jg;
                  const char *shouldbe_queue_name;
                  const char *shouldbe_host_name;

                  if (!(jg = lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)))) {
                     shouldbe_queue_name = MSG_OBJ_NOTRUNNING;
                     shouldbe_host_name = MSG_OBJ_NOTRUNNING;
                  } else {
                     if ((shouldbe_queue_name = lGetString(jg, JG_qname)) == NULL) {
                        shouldbe_queue_name = MSG_OBJ_UNKNOWN;
                     }
                     if ((shouldbe_host_name = lGetHost(jg, JG_qhostname)) == NULL) {
                        shouldbe_host_name = MSG_OBJ_UNKNOWN;
                     }
                  }
                  /* should never happen */
                  ERROR((SGE_EVENT, MSG_JOB_REPORTEXITQ_SUUSSSSS, 
                        rhost, sge_u32c(jobid), sge_u32c(jataskid), 
                        pe_task_id_str?pe_task_id_str:MSG_MASTER, queue_name, 
                        shouldbe_queue_name, shouldbe_host_name, 
                        status2str(lGetUlong(jatep, JAT_status))));
               }
            }
         }
         /* pack ack to enable execd cleaning up */
         if (!skip_job_exit) {
            pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
         }
      }
         break;
      default:
         ERROR((SGE_EVENT, MSG_EXECD_UNKNOWNJ_SUUSUS, 
                rhost, 
                sge_u32c(jobid), 
                sge_u32c(jataskid), 
                pe_task_id_str?pe_task_id_str:MSG_MASTER, 
                sge_u32c(rstate), 
                queue_name));


         pack_ack(pb, ACK_JOB_EXIT, jobid, jataskid, pe_task_id_str);
         break;
      }
   }


   DRETURN_VOID;
}

