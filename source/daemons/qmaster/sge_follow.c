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
#include <string.h>
#include <pthread.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_signal.h"
#include "uti/sge_prog.h"
#include "uti/sge_hostname.h"
#include "uti/sge_mtutil.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_order.h"
#include "sgeobj/sge_order.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_message_SME_L.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_advance_reservation.h"

#include "sched/sge_sched.h"
#include "sched/sgeee.h"
#include "sched/sge_support.h"

#include "gdi/sge_gdi2.h"

#include "sge.h"
#include "sge_userprj_qmaster.h"
#include "sge_qmod_qmaster.h"
#include "sge_subordinate_qmaster.h"
#include "sge_sharetree_qmaster.h"
#include "sge_give_jobs.h"
#include "sge_event_master.h"
#include "sge_queue_event_master.h"
#include "sge_persistence_qmaster.h"
#include "sge_job_qmaster.h"
#include "sge_follow.h"
#include "msg_common.h"
#include "msg_qmaster.h"

typedef enum {
   NOT_DEFINED = 0,
   DO_SPOOL,
   DONOT_SPOOL
} spool_type;

typedef struct {
   pthread_mutex_t last_update_mutex; /* guards the last_update access */
   u_long32 last_update;               /* used to store the last time, when the usage was stored */
   spool_type is_spooling;             /* identifies, if spooling should happen */
   u_long32   now;                     /* stores the time of the last spool computation */
   order_pos_t *cull_order_pos;        /* stores cull positions in the job, ja-task, and order structure */
} sge_follow_t;


static sge_follow_t Follow_Control = {
   PTHREAD_MUTEX_INITIALIZER, 
   0,
   NOT_DEFINED, 
   0, 
   NULL
};

static int ticket_orders_field[] = { OR_job_number,
                                     OR_ja_task_number,
                                     OR_ticket,
                                     NoName };

/****** sge_follow/sge_set_next_spooling_time() ********************************
*  NAME
*     sge_set_next_spooling_time() -- sets the next spooling time
*
*  SYNOPSIS
*     void sge_set_next_spooling_time()
*
*  FUNCTION
*     works on the global sge_follow_t structure. It resets the values and
*     computes the next spooling time.
*
*  NOTES
*     MT-NOTE: sge_set_next_spooling_time() is MT safe
*
*  SEE ALSO
*     sge_follow_order
*******************************************************************************/
void sge_set_next_spooling_time(void)
{
   DENTER(TOP_LAYER, "sge_set_next_spooling_time");

   sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

   if (Follow_Control.is_spooling != NOT_DEFINED) {
      if ((Follow_Control.now + mconf_get_spool_time()) < Follow_Control.last_update) {
         Follow_Control.last_update = Follow_Control.now;
      } else if (Follow_Control.is_spooling == DO_SPOOL) {
         Follow_Control.last_update = Follow_Control.now  + mconf_get_spool_time();
         DPRINTF(("next spooling now:%ld next: %ld time:%d\n\n",
                  (long) Follow_Control.now,(long) Follow_Control.last_update,
                  mconf_get_spool_time()));
      }

      Follow_Control.now = 0;
      Follow_Control.is_spooling = NOT_DEFINED;
   }

   sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

   DEXIT;
}

/**********************************************************************
 Gets an order and executes it.

 Return 0 if everything is fine or

 -1 if the scheduler has sent an inconsistent order list but we think
    the next event delivery will correct this

 -2 if the scheduler has sent an inconsistent order list and we don't think
    the next event delivery will correct this

 -3 if delivery to an execd failed

lList **topp,   ticket orders ptr ptr

 **********************************************************************/
int
sge_follow_order(sge_gdi_ctx_class_t *ctx,
                 lListElem *ep, char *ruser, char *rhost,
                 lList **topp, monitoring_t *monitor, object_description *object_base)
{
   u_long32 job_number, task_number;
   const char *or_pe, *q_name=NULL;
   u_long32 or_type;
   lListElem *jep, *qep, *oep, *hep, *jatp = NULL;
   u_long32 state;
   u_long32 pe_slots = 0, q_slots = 0, q_version;
   lListElem *pe = NULL;

   DENTER(TOP_LAYER, "sge_follow_order");

   or_type=lGetUlong(ep, OR_type);
   or_pe=lGetString(ep, OR_pe);

   DPRINTF(("-----------------------------------------------------------------------\n"));

   switch(or_type) {

   /* -----------------------------------------------------------------------
    * START JOB
    * ----------------------------------------------------------------------- */
   case ORT_start_job: {
      lList *gdil = NULL;
      lListElem *master_qep = NULL;
      lListElem *master_host = NULL;
      lList *exec_host_list = *object_base[SGE_TYPE_EXECHOST].list;

      DPRINTF(("ORDER ORT_start_job\n"));

      job_number = lGetUlong(ep, OR_job_number);
      if (job_number == 0) {
         ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOJOBID));
         DRETURN(-2);
      }

      task_number = lGetUlong(ep, OR_ja_task_number);
      if (task_number == 0) {
         ERROR((SGE_EVENT, MSG_JOB_NOORDERTASK_US, sge_u32c(job_number), "ORT_start_job"));
         DRETURN(-2);
      }

      jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, job_number);
      if(jep == NULL) {
         WARNING((SGE_EVENT, MSG_JOB_FINDJOB_U, sge_u32c(job_number)));
         DRETURN(-1);
      }

      DPRINTF(("ORDER to start Job %ld Task %ld\n", (long) job_number, (long) task_number));

      /* if job got modified in the meantime (qalter), do not start it */
      if (lGetUlong(jep, JB_version) != lGetUlong(ep, OR_job_version)) {
         WARNING((SGE_EVENT, MSG_ORD_OLDVERSION_UUU, sge_u32c(job_number),
               sge_u32c(task_number), sge_u32c(lGetUlong(ep, OR_job_version))));
         DRETURN(-1);
      }

      /* search and enroll task */
      jatp = job_search_task(jep, NULL, task_number);
      if (jatp == NULL) {
         if (range_list_is_id_within(lGetList(jep, JB_ja_n_h_ids), task_number)) {
            jatp = job_create_task(jep, NULL, task_number);
            if (jatp == NULL) {
               WARNING((SGE_EVENT, MSG_JOB_FINDJOBTASK_UU, sge_u32c(task_number),
                        sge_u32c(job_number)));
               DRETURN(-1);
            }

            /* spooling of the JATASK will be done later
             * TODO: we could reduce the number of events sent:
             * Currently we send the ADD event, and a MOD event.
             * This could be reduced to just the ADD event, if we
             * roll back creation of the ja_task in every error situation
             * (delete the ja_task *and* rollback the range information).
             */
            sge_add_event(0, sgeE_JATASK_ADD, job_number, task_number,
                          NULL, NULL, lGetString(jep, JB_session), jatp);
         } else {
            INFO((SGE_EVENT, MSG_JOB_IGNORE_DELETED_TASK_UU,
                  sge_u32c(job_number), sge_u32c(task_number)));
            DRETURN(0);
         }
      }

      /* job is not pending - we got the order twice? */
      if (lGetUlong(jatp, JAT_status) != JIDLE) {
         ERROR((SGE_EVENT, MSG_ORD_TWICE_UU, sge_u32c(job_number), sge_u32c(task_number)));
         DRETURN(-1);
      }

      /* pe job - find the pe object */
      if (or_pe) {
         pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, or_pe);
         if (pe == NULL) {
            ERROR((SGE_EVENT, MSG_OBJ_UNABLE2FINDPE_S, or_pe));
            DRETURN(-2);
         }
         lSetString(jatp, JAT_granted_pe, or_pe);  /* free me on error! */
      }

      /* job is running in an advance reservation - find it */
      if (lGetUlong(jep, JB_ar)) {
         lListElem *ar = ar_list_locate(*object_base[SGE_TYPE_AR].list, lGetUlong(jep, JB_ar));
         if (ar == NULL) {
            ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDARXREFERENCEDINJOBY_UU,
                   sge_u32c(lGetUlong(jep, JB_ar)),
                   sge_u32c(lGetUlong(jep, JB_job_number))));
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-2);
         }
         lSetUlong(jatp, JAT_wallclock_limit, (lGetUlong(ar, AR_end_time) - sge_get_gmt() - sconf_get_duration_offset()));
      }

      /* fill number of tickets into job */
      lSetDouble(jatp, JAT_tix, lGetDouble(ep, OR_ticket));
      lSetDouble(jatp, JAT_ntix, lGetDouble(ep, OR_ntix));
      lSetDouble(jatp, JAT_prio, lGetDouble(ep, OR_prio));

      if ((oep = lFirst(lGetList(ep, OR_queuelist)))) {
         lSetDouble(jatp, JAT_oticket, lGetDouble(oep, OQ_oticket));
         lSetDouble(jatp, JAT_fticket, lGetDouble(oep, OQ_fticket));
         lSetDouble(jatp, JAT_sticket, lGetDouble(oep, OQ_sticket));
      }

      for_each(oep, lGetList(ep, OR_queuelist)) {
         lListElem *gdil_ep;

         q_name    = lGetString(oep, OQ_dest_queue);
         q_version = lGetUlong(oep, OQ_dest_version);
         q_slots   = lGetUlong(oep, OQ_slots);

         /* ----------------------
          *  find and check queue
          */
         if (!q_name) {
            ERROR((SGE_EVENT, SFNMAX, MSG_OBJ_NOQNAME));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-2);
         }

         DPRINTF(("ORDER: start "sge_u32" slots of job \""sge_u32"\" on"
                  " queue \"%s\" v%d with "sge_U32CFormat" initial tickets\n",
               q_slots, job_number, q_name, (int)q_version, sge_u32c((u_long32)lGetDouble(ep, OR_ticket))));

         qep = cqueue_list_locate_qinstance(*object_base[SGE_TYPE_CQUEUE].list, q_name);
         if (qep == NULL) {
            ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDQUEUEXREFERENCEDINJOBY_SU,
                   q_name, sge_u32c(job_number)));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-2);
         }

         /* check queue version */
         if (q_version != lGetUlong(qep, QU_version)) {
            WARNING((SGE_EVENT, MSG_ORD_QVERSION_SUU, q_name,
                  sge_u32c(q_version), sge_u32c(lGetUlong(qep, QU_version))));

            /* try to repair schedd data */
            qinstance_add_event(qep, sgeE_QINSTANCE_MOD);

            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }

         /* the first queue is the master queue */
         if (master_qep == NULL) {
            master_qep = qep;
         }

         DPRINTF(("Queue version: "sge_u32"\n", q_version));

         /* ensure that the jobs owner has access to this queue */
         if (!sge_has_access_(lGetString(jep, JB_owner), lGetString(jep, JB_group),
                                    lGetList(qep, QU_acl), lGetList(qep, QU_xacl),
                                    *object_base[SGE_TYPE_USERSET].list)) {
            ERROR((SGE_EVENT, MSG_JOB_JOBACCESSQ_US, sge_u32c(job_number), q_name));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }

         /* ensure that this queue has enough free slots */
         if (lGetUlong(qep, QU_job_slots) - qinstance_slots_used(qep) < q_slots) {
            ERROR((SGE_EVENT, MSG_JOB_FREESLOTS_USUU, sge_u32c(q_slots), q_name,
                  sge_u32c(job_number), sge_u32c(task_number)));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }

         /* check queue states - queue may not be
          *   - in error state
          *   - calendar suspended
          *   - calendar disabled
          */
         if (qinstance_state_is_error(qep)) {
            WARNING((SGE_EVENT, MSG_JOB_QMARKEDERROR_S, q_name));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }
         if (qinstance_state_is_cal_suspended(qep)) {
            WARNING((SGE_EVENT, MSG_JOB_QSUSPCAL_S, q_name));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }
         if (qinstance_state_is_cal_disabled(qep)) {
            WARNING((SGE_EVENT, MSG_JOB_QDISABLECAL_S, q_name));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-1);
         }

         /* ----------------------
          *  find and check host
          */
         hep = host_list_locate(exec_host_list, lGetHost(qep, QU_qhostname));
         if (hep == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDHOST_S, lGetHost(qep, QU_qhostname)));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(-2);
         } else {
            lListElem *ruep;

            /* host not yet clean after reschedule unknown? */
            for_each(ruep, lGetList(hep, EH_reschedule_unknown_list)) {
               if (job_number == lGetUlong(ruep, RU_job_number)
                   && task_number == lGetUlong(ruep, RU_task_number)) {
                  ERROR((SGE_EVENT, MSG_JOB_UNABLE2STARTJOB_US, sge_u32c(lGetUlong(ruep, RU_job_number)),
                     lGetHost(qep, QU_qhostname)));
                  lFreeList(&gdil);
                  lSetString(jatp, JAT_granted_pe, NULL);
                  DRETURN(-1);
               }
            }
         }

         /* ------------------------------------------------
          *  build up granted_destin_identifier_list (gdil)
          */
         gdil_ep = lAddElemStr(&gdil, JG_qname, q_name, JG_Type); /* free me on error! */
         lSetHost(gdil_ep, JG_qhostname, lGetHost(qep, QU_qhostname));
         lSetUlong(gdil_ep, JG_slots, q_slots);

         /* ------------------------------------------------
          *  tag each gdil entry of slave exec host
          *  in case of sge controlled slaves
          *  this triggers our retry for delivery of slave jobs
          *  and gets untagged when ack has arrived
          */
         if (pe && lGetBool(pe, PE_control_slaves)) {
            lSetDouble(gdil_ep, JG_ticket, lGetDouble(oep, OQ_ticket));
            lSetDouble(gdil_ep, JG_oticket, lGetDouble(oep, OQ_oticket));
            lSetDouble(gdil_ep, JG_fticket, lGetDouble(oep, OQ_fticket));
            lSetDouble(gdil_ep, JG_sticket, lGetDouble(oep, OQ_sticket));

            /* the master host is the host of the master queue */
            if (master_host != NULL && master_host != hep) {
               lListElem *first_at_host;

               /* ensure each host gets tagged only one time
                * we tag the first entry for a host in the existing gdil
                */
               first_at_host = lGetElemHost(gdil, JG_qhostname, lGetHost(hep, EH_name));
               if (!first_at_host) {
                  ERROR((SGE_EVENT, MSG_JOB_HOSTNAMERESOLVE_US,
                         sge_u32c(lGetUlong(jep, JB_job_number)), lGetHost(hep, EH_name)));
               } else {
                  lSetUlong(first_at_host, JG_tag_slave_job, 1);
               }
            } else  {
               master_host = hep;
               DPRINTF(("master host %s\n", lGetHost(master_host, EH_name)));
            }
         } else {
            /* a sequential job only has one host */
            master_host = hep;
         }

         /* in case of a pe job update free_slots on the pe */
         if (pe) {
            pe_slots += q_slots;
         }
      }

      /*
       * check if consumables are still sufficient on the host and on global host
       * - for each individual host with the number of slots on that host
       *   - the first host is master host
       * - for the global host with the total number of slots
       *   - do the check as master host, to make sure global per job consumables are checked
       */
      {
         bool is_master = true;
         bool consumables_ok = true;
         int host_slots = 0;
         int total_slots = 0;
         const char *host_name = NULL;
         lListElem *gdil_ep;
         lListElem *next_gdil_ep = lFirst(gdil);
         lList *centry_list = *object_base[SGE_TYPE_CENTRY].list;

         /* loop over gdil */
         host_name = lGetHost(next_gdil_ep, JG_qhostname);
         while ((gdil_ep = next_gdil_ep) != NULL) {
            /* sum up slots */
            u_long32 slots = lGetUlong(gdil_ep, JG_slots);
            host_slots += slots;
            total_slots += slots;

            /* gdil end or switch to next host: check booking on the current host */
            next_gdil_ep = lNext(gdil_ep);
            if (next_gdil_ep == NULL || strcmp(host_name, lGetHost(next_gdil_ep, JG_qhostname)) != 0) {
               hep = host_list_locate(exec_host_list, host_name);
               debit_host_consumable(jep, hep, centry_list, host_slots, is_master, &consumables_ok);
               if (!consumables_ok) {
                  break;
               }
               /* if there is a next host, it is a slave host */
               is_master = false;

               /* there is a next host: get hostname and reset host slot counter */
               if (next_gdil_ep != NULL) {
                  host_name = lGetHost(next_gdil_ep, JG_qhostname);
                  host_slots = 0;
               }
            }
         }

         /* Per host checks were OK? Then check global host. */
         if (consumables_ok) {
            lListElem *global_hep = host_list_locate(exec_host_list, SGE_GLOBAL_NAME);
            debit_host_consumable(jep, global_hep, centry_list, total_slots, true, &consumables_ok);
         }

         /* Consumable check failed - we cannot start this job! */
         if (!consumables_ok) {
            ERROR((SGE_EVENT, MSG_JOB_RESOURCESNOLONGERAVAILABLE_UU,
                   sge_u32c(job_number), sge_u32c(task_number)));
            lFreeList(&gdil);
            lSetString(jatp, JAT_granted_pe, NULL);
            DRETURN(0);
         }
      }

      /* fill in master_queue */
      lSetString(jatp, JAT_master_queue, lGetString(master_qep, QU_full_name));
      lSetList(jatp, JAT_granted_destin_identifier_list, gdil);

      if (sge_give_job(ctx, jep, jatp, master_qep, pe, master_host, monitor)) {

         /* setting of queues in state unheard is done by sge_give_job() */
         sge_commit_job(ctx, jep, jatp, NULL, COMMIT_ST_DELIVERY_FAILED, COMMIT_DEFAULT, monitor);
         /* This was sge_commit_job(jep, COMMIT_ST_RESCHEDULED). It raised problems if a job
            could not be delivered. The jobslotsfree had been increased even if
            they where not decreased bevore. */

         ERROR((SGE_EVENT, MSG_JOB_JOBDELIVER_UU,
                sge_u32c(lGetUlong(jep, JB_job_number)),
                sge_u32c(lGetUlong(jatp, JAT_task_number))));
         DRETURN(-3);
      }

      /* job is now sent and goes into transfering state */
      /* mode == COMMIT_ST_SENT -> really accept when execd acks */
      sge_commit_job(ctx, jep, jatp, NULL, COMMIT_ST_SENT, COMMIT_DEFAULT, monitor);

      /* now send events and spool the job */
      {
         lList *answer_list = NULL;
         u_long32 now = sge_get_gmt();
         const char *session = lGetString(jep, JB_session);

         /* spool job and ja_task in one transaction, send job mod event */
         sge_event_spool(ctx, &answer_list, now, sgeE_JOB_MOD,
                         job_number, task_number, NULL, NULL, session,
                         jep, jatp, NULL, true, true);
         answer_list_output(&answer_list);
      }

      /* set timeout for job resend */
      trigger_job_resend(sge_get_gmt(), master_host, job_number, task_number, 5);

      if (pe) {
         pe_debit_slots(pe, pe_slots, job_number);
         /* this info is not spooled */
         sge_add_event(0, sgeE_PE_MOD, 0, 0, lGetString(jatp, JAT_granted_pe), NULL, NULL, pe);
         lListElem_clear_changed_info(pe);
      }

      DPRINTF(("successfully handed off job \"" sge_u32 "\" to queue \"%s\"\n",
               lGetUlong(jep, JB_job_number), lGetString(jatp, JAT_master_queue)));

      /* now after successfully (we hope) sent the job to execd
       * suspend all subordinated queues that need suspension
       */
      cqueue_list_x_on_subordinate_gdil(ctx, *object_base[SGE_TYPE_CQUEUE].list,
                                        true, gdil, monitor);
   }
   break;

 /* -----------------------------------------------------------------------
    * SET PRIORITY VALUES TO NULL
    *
    * modifications performed on the job are not spooled
    * ----------------------------------------------------------------------- */
   case ORT_clear_pri_info:

      DPRINTF(("ORDER ORT_ptickets\n"));
      {
         ja_task_pos_t *ja_pos = NULL;
         job_pos_t   *job_pos = NULL;
         lListElem *next_ja_task = NULL;

         job_number = lGetUlong(ep, OR_job_number);
         if (job_number == 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOJOBID));
            DRETURN(-2);
         }

         task_number = lGetUlong(ep, OR_ja_task_number);

         DPRINTF(("ORDER : job("sge_u32")->pri/tickets reset", job_number));

         jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, job_number);
         if (jep == NULL) {
            WARNING((SGE_EVENT, MSG_JOB_UNABLE2FINDJOBORD_U, sge_u32c(job_number)));
            DRETURN(0); /* it's ok - job has exited - forget about him */
         }

         next_ja_task = lFirst(lGetList(jep, JB_ja_tasks));

         /* we have to iterate over the ja-tasks and the template */
         jatp = job_get_ja_task_template_pending(jep, 0);
         if (jatp == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_FINDJOBTASK_UU, sge_u32c(0), sge_u32c(job_number)));
            DRETURN(-2);
         }

         sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         if (Follow_Control.cull_order_pos != NULL) { /* do we have the positions cached? */
            ja_pos =        &(Follow_Control.cull_order_pos->ja_task);
            job_pos =       &(Follow_Control.cull_order_pos->job);

            while (jatp != NULL) {
               lSetPosDouble(jatp, ja_pos->JAT_tix_pos, 0);
               lSetPosDouble(jatp, ja_pos->JAT_oticket_pos, 0);
               lSetPosDouble(jatp, ja_pos->JAT_fticket_pos, 0);
               lSetPosDouble(jatp, ja_pos->JAT_sticket_pos, 0);
               lSetPosDouble(jatp, ja_pos->JAT_share_pos,   0);
               lSetPosDouble(jatp, ja_pos->JAT_prio_pos,    0);
               lSetPosDouble(jatp, ja_pos->JAT_ntix_pos,    0);
               if (task_number != 0) { /* if task_number == 0, we only change the */
                  jatp = next_ja_task; /* pending tickets, otherwise all */
                  next_ja_task = lNext(next_ja_task);
               } else {
                  jatp = NULL;
               }
            }

            lSetPosDouble(jep, job_pos->JB_nppri_pos,   0);
            lSetPosDouble(jep, job_pos->JB_nurg_pos,    0);
            lSetPosDouble(jep, job_pos->JB_urg_pos,     0);
            lSetPosDouble(jep, job_pos->JB_rrcontr_pos, 0);
            lSetPosDouble(jep, job_pos->JB_dlcontr_pos, 0);
            lSetPosDouble(jep, job_pos->JB_wtcontr_pos, 0);
         } else {   /* we do not have the positions cached.... */
            while (jatp != NULL) {
               lSetDouble(jatp, JAT_tix,     0);
               lSetDouble(jatp, JAT_oticket, 0);
               lSetDouble(jatp, JAT_fticket, 0);
               lSetDouble(jatp, JAT_sticket, 0);
               lSetDouble(jatp, JAT_share,   0);
               lSetDouble(jatp, JAT_prio,    0);
               lSetDouble(jatp, JAT_ntix,    0);
               if (task_number != 0) {   /* if task_number == 0, we only change the */
                  jatp = next_ja_task;   /* pending tickets, otherwise all */
                  next_ja_task = lNext(next_ja_task);
               } else {
                  jatp = NULL;
               }
            }

            lSetDouble(jep, JB_nppri,   0);
            lSetDouble(jep, JB_nurg,    0);
            lSetDouble(jep, JB_urg,     0);
            lSetDouble(jep, JB_rrcontr, 0);
            lSetDouble(jep, JB_dlcontr, 0);
            lSetDouble(jep, JB_wtcontr, 0);
         }

         sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);
      }
      break;

   /* -----------------------------------------------------------------------
    * CHANGE TICKETS OF PENDING JOBS
    *
    * Modify the tickets of pending jobs for the sole purpose of being
    * able to display and sort the pending jobs list based on the
    * expected execution order.
    *
    * modifications performed on the job are not spooled
    * ----------------------------------------------------------------------- */
   case ORT_ptickets:
      DPRINTF(("ORDER ORT_ptickets\n"));
      {
         ja_task_pos_t *ja_pos;
         ja_task_pos_t *order_ja_pos;
         job_pos_t   *job_pos;
         job_pos_t   *order_job_pos;
         lListElem *joker;

         job_number = lGetUlong(ep, OR_job_number);
         if (job_number == 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOJOBID));
            DRETURN(-2);
         }

         DPRINTF(("ORDER : job("sge_u32")->ticket = "sge_u32"\n",
            job_number, (u_long32)lGetDouble(ep, OR_ticket)));

         jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, job_number);
         if (jep == NULL) {
            WARNING((SGE_EVENT, MSG_JOB_UNABLE2FINDJOBORD_U, sge_u32c(job_number)));
            DRETURN(0); /* it's ok - job has exited - forget about him */
         }

         task_number = lGetUlong(ep, OR_ja_task_number);
         if (task_number == 0) {
            ERROR((SGE_EVENT, MSG_JOB_NOORDERTASK_US, sge_u32c(job_number), "ORT_ptickets"));
            DRETURN(-2);
         }

         jatp = job_search_task(jep, NULL, task_number);
         if (jatp == NULL) {
            jatp = job_get_ja_task_template_pending(jep, task_number);

            if (jatp == NULL) {
               ERROR((SGE_EVENT, MSG_JOB_FINDJOBTASK_UU, sge_u32c(task_number), sge_u32c(job_number)));
               sge_add_event(0, sgeE_JATASK_DEL, job_number, task_number,
                             NULL, NULL, lGetString(jep, JB_session), NULL);
               DRETURN(-2);
            }
         }

         sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         if (Follow_Control.cull_order_pos == NULL) {
            lListElem *joker_task;

            joker = lFirst(lGetList(ep, OR_joker));
            joker_task = lFirst(lGetList(joker, JB_ja_tasks));

            sge_create_cull_order_pos(&(Follow_Control.cull_order_pos), jep, jatp, joker, joker_task);
         }

         ja_pos =        &(Follow_Control.cull_order_pos->ja_task);
         order_ja_pos =  &(Follow_Control.cull_order_pos->order_ja_task);
         job_pos =       &(Follow_Control.cull_order_pos->job);
         order_job_pos = &(Follow_Control.cull_order_pos->order_job);

         if (lGetPosUlong(jatp, ja_pos->JAT_status_pos) == JFINISHED) {
            WARNING((SGE_EVENT, MSG_JOB_CHANGEPTICKETS_UU,
                     sge_u32c(lGetUlong(jep, JB_job_number)),
                     sge_u32c(lGetUlong(jatp, JAT_task_number))));
            sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);
            DRETURN(0);
         }

         /* modify jobs ticket amount */
         lSetPosDouble(jatp, ja_pos->JAT_tix_pos, lGetDouble(ep, OR_ticket));

         /* check several fields to be updated */
         if ((joker = lFirst(lGetList(ep, OR_joker)))) {
            lListElem *joker_task;

            joker_task = lFirst(lGetList(joker, JB_ja_tasks));

            lSetPosDouble(jatp, ja_pos->JAT_oticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_oticket_pos));
            lSetPosDouble(jatp, ja_pos->JAT_fticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_fticket_pos));
            lSetPosDouble(jatp, ja_pos->JAT_sticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_sticket_pos));
            lSetPosDouble(jatp, ja_pos->JAT_share_pos,   lGetPosDouble(joker_task, order_ja_pos->JAT_share_pos));
            lSetPosDouble(jatp, ja_pos->JAT_prio_pos,    lGetPosDouble(joker_task, order_ja_pos->JAT_prio_pos));
            lSetPosDouble(jatp, ja_pos->JAT_ntix_pos,    lGetPosDouble(joker_task, order_ja_pos->JAT_ntix_pos));

            lSetPosDouble(jep, job_pos->JB_nppri_pos,   lGetPosDouble(joker, order_job_pos->JB_nppri_pos));
            lSetPosDouble(jep, job_pos->JB_nurg_pos,    lGetPosDouble(joker, order_job_pos->JB_nurg_pos));
            lSetPosDouble(jep, job_pos->JB_urg_pos,     lGetPosDouble(joker, order_job_pos->JB_urg_pos));
            lSetPosDouble(jep, job_pos->JB_rrcontr_pos, lGetPosDouble(joker, order_job_pos->JB_rrcontr_pos));
            lSetPosDouble(jep, job_pos->JB_dlcontr_pos, lGetPosDouble(joker, order_job_pos->JB_dlcontr_pos));
            lSetPosDouble(jep, job_pos->JB_wtcontr_pos, lGetPosDouble(joker, order_job_pos->JB_wtcontr_pos));
         }

         sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

#if 0
         DPRINTF(("PRIORITY: "sge_u32"."sge_u32" %f/%f tix/ntix %f npri %f/%f urg/nurg %f prio\n",
            lGetUlong(jep, JB_job_number),
            lGetUlong(jatp, JAT_task_number),
            lGetDouble(jatp, JAT_tix),
            lGetDouble(jatp, JAT_ntix),
            lGetDouble(jep, JB_nppri),
            lGetDouble(jep, JB_urg),
            lGetDouble(jep, JB_nurg),
            lGetDouble(jatp, JAT_prio)));
#endif

      }
      break;


   /* -----------------------------------------------------------------------
    * CHANGE TICKETS OF RUNNING/TRANSFERING JOBS
    *
    * Our aim is to collect all ticket orders of an gdi request
    * and to send ONE packet to each exec host. Here we just
    * check the orders for consistency. They get forwarded to
    * execd having checked all orders.
    *
    * modifications performed on the job are not spooled
    * ----------------------------------------------------------------------- */
   case ORT_tickets:
      DPRINTF(("ORDER ORT_tickets\n"));
      {
         lListElem *joker;

         job_number = lGetUlong(ep, OR_job_number);
         if(job_number == 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOJOBID));
            DRETURN(-2);
         }

         DPRINTF(("ORDER: job("sge_u32")->ticket = "sge_u32"\n",
            job_number, (u_long32)lGetDouble(ep, OR_ticket)));

         jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, job_number);
         if(jep == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDJOBORD_U, sge_u32c(job_number)));
            DRETURN(0); /* it's ok - job has exited - forget about him */
         }
         task_number = lGetUlong(ep, OR_ja_task_number);
         if (task_number == 0) {
            ERROR((SGE_EVENT, MSG_JOB_NOORDERTASK_US, sge_u32c(job_number), "ORT_tickets"));
            DRETURN(-2);
         }
         jatp = job_search_task(jep, NULL, task_number);
         if (jatp == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_FINDJOBTASK_UU,
                  sge_u32c(task_number), sge_u32c(job_number)));
            sge_add_event( 0, sgeE_JATASK_DEL, job_number, task_number,
                          NULL, NULL, lGetString(jep, JB_session), NULL);
            DRETURN(-2);
         }

         /* job must be running */
         if (lGetUlong(jatp, JAT_status) != JRUNNING &&
             lGetUlong(jatp, JAT_status) != JTRANSFERING) {

            /* if the job just finished, ignore the order */
            if (lGetUlong(jatp, JAT_status) != JFINISHED) {
               WARNING((SGE_EVENT, MSG_JOB_CHANGETICKETS_UUU,
                        sge_u32c(lGetUlong(jep, JB_job_number)),
                        sge_u32c(lGetUlong(jatp, JAT_task_number)),
                        sge_u32c(lGetUlong(jatp, JAT_status))
                        ));
               DRETURN(0);
            }
         } else {
            bool distribute_tickets = false;
            /* modify jobs ticket amount and spool job */
            lSetDouble(jatp, JAT_tix, lGetDouble(ep, OR_ticket));
            DPRINTF(("TICKETS: "sge_u32"."sge_u32" "sge_u32" tickets\n",
               lGetUlong(jep, JB_job_number),
               lGetUlong(jatp, JAT_task_number),
               (u_long32)lGetDouble(jatp, JAT_tix)));

            /* check several fields to be updated */
            if ((joker = lFirst(lGetList(ep, OR_joker)))) {
               lListElem *joker_task;
               ja_task_pos_t *ja_pos;
               ja_task_pos_t *order_ja_pos;
               job_pos_t   *job_pos;
               job_pos_t   *order_job_pos;

               joker_task = lFirst(lGetList(joker, JB_ja_tasks));
               distribute_tickets = (lGetPosViaElem(joker_task, JAT_granted_destin_identifier_list, SGE_NO_ABORT) > -1)? true : false;

               sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

               if (Follow_Control.cull_order_pos == NULL) {
                  lListElem *joker_task;

                  joker=lFirst(lGetList(ep, OR_joker));
                  joker_task = lFirst(lGetList(joker, JB_ja_tasks));

                  sge_create_cull_order_pos(&(Follow_Control.cull_order_pos), jep, jatp, joker, joker_task);
               }

               ja_pos =        &(Follow_Control.cull_order_pos->ja_task);
               order_ja_pos =  &(Follow_Control.cull_order_pos->order_ja_task);
               job_pos =       &(Follow_Control.cull_order_pos->job);
               order_job_pos = &(Follow_Control.cull_order_pos->order_job);

               lSetPosDouble(jatp, ja_pos->JAT_oticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_oticket_pos));
               lSetPosDouble(jatp, ja_pos->JAT_fticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_fticket_pos));
               lSetPosDouble(jatp, ja_pos->JAT_sticket_pos, lGetPosDouble(joker_task, order_ja_pos->JAT_sticket_pos));
               lSetPosDouble(jatp, ja_pos->JAT_share_pos,   lGetPosDouble(joker_task, order_ja_pos->JAT_share_pos));
               lSetPosDouble(jatp, ja_pos->JAT_prio_pos,    lGetPosDouble(joker_task, order_ja_pos->JAT_prio_pos));
               lSetPosDouble(jatp, ja_pos->JAT_ntix_pos,    lGetPosDouble(joker_task, order_ja_pos->JAT_ntix_pos));

               lSetPosDouble(jep, job_pos->JB_nppri_pos,   lGetPosDouble(joker, order_job_pos->JB_nppri_pos));
               lSetPosDouble(jep, job_pos->JB_nurg_pos,    lGetPosDouble(joker, order_job_pos->JB_nurg_pos));
               lSetPosDouble(jep, job_pos->JB_urg_pos,     lGetPosDouble(joker, order_job_pos->JB_urg_pos));
               lSetPosDouble(jep, job_pos->JB_rrcontr_pos, lGetPosDouble(joker, order_job_pos->JB_rrcontr_pos));
               lSetPosDouble(jep, job_pos->JB_dlcontr_pos, lGetPosDouble(joker, order_job_pos->JB_dlcontr_pos));
               lSetPosDouble(jep, job_pos->JB_wtcontr_pos, lGetPosDouble(joker, order_job_pos->JB_wtcontr_pos));
               sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);
            }

            /* tickets should only be further distributed in the scheduler reprioritize_interval. Only in
               those intervales does the ticket order structure contain a JAT_granted_destin_identifier_list.
               We use that as an identifier to go on, or not. */
            if (distribute_tickets && topp != NULL) {
               lDescr *rdp = NULL;

               lEnumeration *what = lIntVector2What(OR_Type, ticket_orders_field);
               lReduceDescr(&rdp, OR_Type, what);

               /* If a ticket order has a queuelist, then this is a parallel job
                  with controlled sub-tasks. We generate a ticket order for
                  each host in the queuelist containing the total tickets for
                  all job slots being used on the host */
               if (lGetList(ep, OR_queuelist)) {
                  lList *host_tickets_cache = lCreateList("", UA_Type); /* cashed temporary hash list */
                  /* set granted slot tickets */
                  for_each(oep, lGetList(ep, OR_queuelist)) {
                     lListElem *gdil_ep;
                     lListElem *chost_ep;

                     gdil_ep = lGetSubStr(jatp, JG_qname, lGetString(oep, OQ_dest_queue),
                                          JAT_granted_destin_identifier_list);
                     if (gdil_ep != NULL) {
                        double tickets = lGetDouble(oep, OQ_ticket);
                        const char *hostname = lGetHost(gdil_ep, JG_qhostname);

                        lSetDouble(gdil_ep, JG_ticket, tickets);
                        lSetDouble(gdil_ep, JG_oticket, lGetDouble(oep, OQ_oticket));
                        lSetDouble(gdil_ep, JG_fticket, lGetDouble(oep, OQ_fticket));
                        lSetDouble(gdil_ep, JG_sticket, lGetDouble(oep, OQ_sticket));

                        chost_ep = lGetElemStr(host_tickets_cache, UA_name, hostname);
                        if (chost_ep == NULL) {
                           chost_ep = lAddElemStr(&host_tickets_cache, UA_name, hostname, UA_Type);
                        }
                        lAddDouble(chost_ep, UA_value, tickets);
                     }
                  }

                  /* map cached tickets back to RTIC_Type list */
                  for_each(oep, host_tickets_cache) {
                     const char *hostname = lGetString(oep, UA_name);
                     lListElem *rtic_ep;
                     lList* host_tickets;

                     lListElem *newep = lSelectElemDPack(ep, NULL, rdp, what, false, NULL, NULL);
                     lSetDouble(newep, OR_ticket, lGetDouble(oep, UA_value));

                     rtic_ep = lGetElemHost(*topp, RTIC_host, hostname);
                     if (rtic_ep == NULL) {
                        rtic_ep = lAddElemHost(topp, RTIC_host, hostname, RTIC_Type);
                     }
                     host_tickets = lGetList(rtic_ep, RTIC_tickets);
                     if (host_tickets == NULL) {
                        host_tickets = lCreateList("ticket orders", rdp);
                        lSetList(rtic_ep, RTIC_tickets, host_tickets);
                     }
                     lAppendElem(host_tickets, newep);
                  }
                  lFreeList(&host_tickets_cache);
               } else {
                  lList *gdil = lGetList(jatp, JAT_granted_destin_identifier_list);
                  if (gdil != NULL) {
                     lListElem *newep = lSelectElemDPack(ep, NULL, rdp, what, false, NULL, NULL);
                     lList* host_tickets;
                     const char *hostname = lGetHost(lFirst(gdil), JG_qhostname);
                     lListElem *rtic_ep = lGetElemHost(*topp, RTIC_host, hostname);
                     if (rtic_ep == NULL) {
                        rtic_ep = lAddElemHost(topp, RTIC_host, hostname, RTIC_Type);
                     }
                     host_tickets = lGetList(rtic_ep, RTIC_tickets);
                     if (host_tickets == NULL) {
                        host_tickets = lCreateList("ticket orders", rdp);
                        lSetList(rtic_ep, RTIC_tickets, host_tickets);
                     }
                     lAppendElem(host_tickets, newep);
                  }

               }
               lFreeWhat(&what);
               sge_free(&rdp);
            }
         }
      }
      break;

   /* -----------------------------------------------------------------------
    * REMOVE JOBS THAT ARE WAITING FOR SCHEDD'S PERMISSION TO GET DELETED
    *
    * Using this order schedd can only remove jobs in the
    * "dead but not buried" state
    *
    * ----------------------------------------------------------------------- */
   case ORT_remove_job:
   /* -----------------------------------------------------------------------
    * REMOVE IMMEDIATE JOBS THAT COULD NOT GET SCHEDULED IN THIS PASS
    *
    * Using this order schedd can only remove idle immediate jobs
    * (former ORT_remove_interactive_job)
    * ----------------------------------------------------------------------- */
   case ORT_remove_immediate_job:
      DPRINTF(("ORDER: ORT_remove_immediate_job or ORT_remove_job\n"));

      job_number = lGetUlong(ep, OR_job_number);
      if (job_number == 0) {
         ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOJOBID));
         DRETURN(-2);
      }
      task_number = lGetUlong(ep, OR_ja_task_number);
      if (task_number == 0) {
         ERROR((SGE_EVENT, MSG_JOB_NOORDERTASK_US, sge_u32c(job_number),
            (or_type==ORT_remove_immediate_job)?"ORT_remove_immediate_job":"ORT_remove_job"));
         DRETURN(-2);
      }
      DPRINTF(("ORDER: remove %sjob "sge_u32"."sge_u32"\n",
         or_type==ORT_remove_immediate_job?"immediate ":"" ,
         job_number, task_number));
      jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, job_number);
      if (jep == NULL) {
         if (or_type == ORT_remove_job) {
            ERROR((SGE_EVENT, MSG_JOB_FINDJOB_U, sge_u32c(job_number)));
            /* try to repair schedd data - session is unknown here */
            sge_add_event(0, sgeE_JOB_DEL, job_number, task_number, NULL, NULL, NULL, NULL);
            DRETURN(-1);
         } else {
            /* in case of an immediate parallel job the job could be missing */
            INFO((SGE_EVENT, MSG_JOB_FINDJOB_U, sge_u32c(job_number)));
            DRETURN(0);
         }
      }
      jatp = job_search_task(jep, NULL, task_number);

      /* if ja task doesn't exist yet, create it */
      if (jatp == NULL) {
         lList *answer_list = NULL;

         jatp = job_create_task(jep, NULL, task_number);

         /* new jatask has to be spooled and event sent */
         if (jatp == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_FINDJOBTASK_UU, sge_u32c(task_number),
                   sge_u32c(job_number)));
            DRETURN(-1);
         }

         if (or_type == ORT_remove_job) {
            ERROR((SGE_EVENT, MSG_JOB_ORDERDELINCOMPLETEJOB_UU, sge_u32c(job_number),
                   sge_u32c(task_number)));
            lSetUlong(jatp, JAT_status, JFINISHED);
         }
         sge_event_spool(ctx, &answer_list, 0, sgeE_JATASK_ADD,
                         job_number, task_number, NULL, NULL,
                         lGetString(jep, JB_session),
                         jep, jatp, NULL, true, true);
         answer_list_output(&answer_list);
      }

      if (or_type == ORT_remove_job) {
         if (lGetUlong(jatp, JAT_status) != JFINISHED) {
            ERROR((SGE_EVENT, MSG_JOB_REMOVENOTFINISHED_U, sge_u32c(lGetUlong(jep, JB_job_number))));
            DRETURN(-1);
         }

         /* remove it */
         sge_commit_job(ctx, jep, jatp, NULL, COMMIT_ST_DEBITED_EE, COMMIT_DEFAULT, monitor);
      } else {
         if (!JOB_TYPE_IS_IMMEDIATE(lGetUlong(jep, JB_type))) {
            if (lGetString(jep, JB_script_file)) {
               ERROR((SGE_EVENT, MSG_JOB_REMOVENONINTERACT_U, sge_u32c(lGetUlong(jep, JB_job_number))));
            } else {
               ERROR((SGE_EVENT, MSG_JOB_REMOVENONIMMEDIATE_U,  sge_u32c(lGetUlong(jep, JB_job_number))));
            }
            DRETURN(-1);
         }
         if (lGetUlong(jatp, JAT_status) != JIDLE) {
            ERROR((SGE_EVENT, MSG_JOB_REMOVENOTIDLEIA_U, sge_u32c(lGetUlong(jep, JB_job_number))));
            DRETURN(-1);
         }
         INFO((SGE_EVENT, MSG_JOB_NOFREERESOURCEIA_UU,
               sge_u32c(lGetUlong(jep, JB_job_number)),
               sge_u32c(lGetUlong(jatp, JAT_task_number)),
               lGetString(jep, JB_owner)));

         /* remove it */
         sge_commit_job(ctx, jep, jatp, NULL, COMMIT_ST_NO_RESOURCES, COMMIT_DEFAULT | COMMIT_NEVER_RAN, monitor);
      }
      break;

   /* -----------------------------------------------------------------------
    * REPLACE A PROJECT'S
    *
    * - PR_usage
    * - PR_usage_time_stamp
    * - PR_long_term_usage
    * - PR_debited_job_usage
    *
    * Using this order schedd can debit usage on users/projects
    * both orders are handled identically except target list
    * ----------------------------------------------------------------------- */
   case ORT_update_project_usage:
      DPRINTF(("ORDER: ORT_update_project_usage\n"));
      {
         lListElem *up_order, *up, *ju, *up_ju, *next;
         int pos;
         const char *up_name;
         lList *tlp;
         u_long32 now = 0;
         bool is_spool = false;

         sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         if (Follow_Control.is_spooling == NOT_DEFINED) {

            now = Follow_Control.now = sge_get_gmt();

            DPRINTF((">>next spooling now: %ld next: %ld\n",
                     (long) Follow_Control.now,
                     (long) Follow_Control.last_update));

            if (now >= Follow_Control.last_update) {
               Follow_Control.is_spooling = DO_SPOOL;
               is_spool = true;
            } else {
               Follow_Control.is_spooling = DONOT_SPOOL;
            }
         } else {
            now =  Follow_Control.now;
         }

         sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         DPRINTF(("ORDER: update %d projects\n", lGetNumberOfElem(lGetList(ep, OR_joker))));

         for_each (up_order, lGetList(ep, OR_joker)) {
            if ((pos=lGetPosViaElem(up_order, PR_name, SGE_NO_ABORT))<0 ||
                  !(up_name = lGetString(up_order, PR_name))) {
               continue;
            }

            DPRINTF(("%s %s usage updating with %d jobs\n", MSG_OBJ_PRJ,
               up_name, lGetNumberOfElem(lGetList(up_order, PR_debited_job_usage))));

            if (!(up=prj_list_locate(*object_base[SGE_TYPE_PROJECT].list, up_name))) {
               /* order contains reference to unknown user/prj object */
               continue;
            }

            if ((pos=lGetPosViaElem(up_order, PR_version, SGE_NO_ABORT)) >= 0 &&
                (lGetPosUlong(up_order, pos) != lGetUlong(up, PR_version))) {
               /* order contains update for outdated user/project usage */
               WARNING((SGE_EVENT, MSG_ORD_USRPRJVERSION_SUU, up_name, sge_u32c(lGetPosUlong(up_order, pos)),
                      sge_u32c(lGetUlong(up, PR_version)) ));
               /* Note: Should we apply the debited job usage in this case? */
               continue;
            }

            lAddUlong(up, PR_version, 1);

            if ((pos=lGetPosViaElem(up_order, PR_project, SGE_NO_ABORT)) >= 0) {
               lSwapList(up_order, PR_project, up, PR_project);
            }

            if ((pos=lGetPosViaElem(up_order, PR_usage_time_stamp, SGE_NO_ABORT)) >= 0)
               lSetUlong(up, PR_usage_time_stamp, lGetPosUlong(up_order, pos));

            if ((pos=lGetPosViaElem(up_order, PR_usage, SGE_NO_ABORT))>=0) {
               lSwapList(up_order, PR_usage, up, PR_usage);
            }

            if ((pos=lGetPosViaElem(up_order, PR_long_term_usage, SGE_NO_ABORT)) >= 0) {
               lSwapList(up_order, PR_long_term_usage, up, PR_long_term_usage);
            }

            /* update old usage in up for each job appearing in
               PR_debited_job_usage of 'up_order' */
            next = lFirst(lGetList(up_order, PR_debited_job_usage));
            while ((ju = next)) {
               next = lNext(ju);

               job_number = lGetUlong(ju, UPU_job_number);

               /* seek for existing debited usage of this job */
               if ((up_ju=lGetSubUlong(up, UPU_job_number, job_number, PR_debited_job_usage))) {

                  /* if passed old usage list is NULL, delete existing usage */
                  if (lGetList(ju, UPU_old_usage_list) == NULL) {

                     lRemoveElem(lGetList(up_order, PR_debited_job_usage), &ju);
                     lRemoveElem(lGetList(up, PR_debited_job_usage), &up_ju);

                  } else {

                     /* still exists - replace old usage with new one */
                     DPRINTF(("updating debited usage for job "sge_u32"\n", job_number));
                     lSwapList(ju, UPU_old_usage_list, up_ju, UPU_old_usage_list);
                  }

               } else {
                  /* unchain ju element and chain it into our user/prj object */
                  DPRINTF(("adding debited usage for job "sge_u32"\n", job_number));
                  lDechainElem(lGetList(up_order, PR_debited_job_usage), ju);

                  if (lGetList(ju, UPU_old_usage_list) != NULL) {
                     /* unchain ju element and chain it into our user/prj object */
                     if (!(tlp=lGetList(up, PR_debited_job_usage))) {
                        tlp = lCreateList(up_name, UPU_Type);
                        lSetList(up, PR_debited_job_usage, tlp);
                     }
                     lInsertElem(tlp, NULL, ju);
                  } else {
                     /* do not chain in empty empty usage records */
                     lFreeElem(&ju);
                  }
               }
            }

            /* spool and send event */

            {
               lList *answer_list = NULL;
               sge_event_spool(ctx, &answer_list, now, sgeE_PROJECT_MOD,
                               0, 0, up_name, NULL, NULL,
                               up, NULL, NULL, true, is_spool);
               answer_list_output(&answer_list);
            }
         }
      }
      break;

   /* -----------------------------------------------------------------------
    * REPLACE A USER
    *
    * - UU_usage
    * - UU_usage_time_stamp
    * - UU_long_term_usage
    * - UU_debited_job_usage
    *
    * Using this order schedd can debit usage on users/projects
    * both orders are handled identically except target list
    * ----------------------------------------------------------------------- */
   case ORT_update_user_usage:
      DPRINTF(("ORDER: ORT_update_user_usage\n"));
      {
         lListElem *up_order, *up, *ju, *up_ju, *next;
         int pos;
         const char *up_name;
         lList *tlp;
         u_long32 now = 0;
         bool is_spool = false;

         sge_mutex_lock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         if (Follow_Control.is_spooling == NOT_DEFINED) {

            now = Follow_Control.now = sge_get_gmt();

            DPRINTF((">>next spooling now:%ld next: %ld\n",
                     (long) Follow_Control.now,
                     (long) Follow_Control.last_update));

            if (now >= Follow_Control.last_update) {
               Follow_Control.is_spooling = DO_SPOOL;
               is_spool = true;
            } else {
               Follow_Control.is_spooling = DONOT_SPOOL;
            }
         } else {
            now =  Follow_Control.now;
         }

         sge_mutex_unlock("follow_last_update_mutex", SGE_FUNC, __LINE__, &Follow_Control.last_update_mutex);

         DPRINTF(("ORDER: update %d users\n", lGetNumberOfElem(lGetList(ep, OR_joker))));

         for_each (up_order, lGetList(ep, OR_joker)) {
            if ((pos=lGetPosViaElem(up_order, UU_name, SGE_NO_ABORT))<0 ||
                  !(up_name = lGetString(up_order, UU_name))) {
               continue;
            }

            DPRINTF(("%s %s usage updating with %d jobs\n", MSG_OBJ_USER,
               up_name, lGetNumberOfElem(lGetList(up_order, UU_debited_job_usage))));

            if (!(up=user_list_locate(*object_base[SGE_TYPE_USER].list, up_name))) {
               /* order contains reference to unknown user/prj object */
               continue;
            }

            if ((pos=lGetPosViaElem(up_order, UU_version, SGE_NO_ABORT)) >= 0 &&
                (lGetPosUlong(up_order, pos) != lGetUlong(up, UU_version))) {
               /* order contains update for outdated user/project usage */
               WARNING((SGE_EVENT, MSG_ORD_USRPRJVERSION_SUU, up_name, sge_u32c(lGetPosUlong(up_order, pos)),
                      sge_u32c(lGetUlong(up, UU_version)) ));
               /* Note: Should we apply the debited job usage in this case? */
               continue;
            }

            lAddUlong(up, UU_version, 1);

            if ((pos=lGetPosViaElem(up_order, UU_project, SGE_NO_ABORT))>=0) {
               lSwapList(up_order, UU_project, up, UU_project);
            }

            if ((pos=lGetPosViaElem(up_order, UU_usage_time_stamp, SGE_NO_ABORT))>=0)
               lSetUlong(up, UU_usage_time_stamp, lGetPosUlong(up_order, pos));

            if ((pos=lGetPosViaElem(up_order, UU_usage, SGE_NO_ABORT))>=0) {
               lSwapList(up_order, UU_usage, up, UU_usage);
            }

            if ((pos=lGetPosViaElem(up_order, UU_long_term_usage, SGE_NO_ABORT))>=0) {
               lSwapList(up_order, UU_long_term_usage, up, UU_long_term_usage);
            }

            /* update old usage in up for each job appearing in
               UU_debited_job_usage of 'up_order' */
            next = lFirst(lGetList(up_order, UU_debited_job_usage));
            while ((ju = next)) {
               next = lNext(ju);

               job_number = lGetUlong(ju, UPU_job_number);

               /* seek for existing debited usage of this job */
               if ((up_ju=lGetSubUlong(up, UPU_job_number, job_number,
                                       UU_debited_job_usage))) {

                  /* if passed old usage list is NULL, delete existing usage */
                  if (lGetList(ju, UPU_old_usage_list) == NULL) {

                     lRemoveElem(lGetList(up_order, UU_debited_job_usage), &ju);
                     lRemoveElem(lGetList(up, UU_debited_job_usage), &up_ju);

                  } else {

                     /* still exists - replace old usage with new one */
                     DPRINTF(("updating debited usage for job "sge_u32"\n", job_number));
                     lSwapList(ju, UPU_old_usage_list, up_ju, UPU_old_usage_list);
                  }

               } else {
                  /* unchain ju element and chain it into our user/prj object */
                  DPRINTF(("adding debited usage for job "sge_u32"\n", job_number));
                  lDechainElem(lGetList(up_order, UU_debited_job_usage), ju);

                  if (lGetList(ju, UPU_old_usage_list) != NULL) {
                     /* unchain ju element and chain it into our user/prj object */
                     if (!(tlp=lGetList(up, UU_debited_job_usage))) {
                        tlp = lCreateList(up_name, UPU_Type);
                        lSetList(up, UU_debited_job_usage, tlp);
                     }
                     lInsertElem(tlp, NULL, ju);
                  } else {
                     /* do not chain in empty empty usage records */
                     lFreeElem(&ju);
                  }
               }
            }

            /* spool and send event */

            {
               lList *answer_list = NULL;
               sge_event_spool(ctx, &answer_list, now, sgeE_USER_MOD,
                               0, 0, up_name, NULL, NULL,
                               up, NULL, NULL, true, is_spool);
               answer_list_output(&answer_list);
            }
         }
      }
      break;

   /* -----------------------------------------------------------------------
    * FILL IN SEVERAL SCHEDULING VALUES INTO QMASTERS SHARE TREE
    * TO BE DISPLAYED BY QMON AND OTHER CLIENTS
    * ----------------------------------------------------------------------- */
   case ORT_share_tree:
      DPRINTF(("ORDER: ORT_share_tree\n"));
      sge_init_node_fields(lFirst(*object_base[SGE_TYPE_SHARETREE].list));
      if (update_sharetree(*object_base[SGE_TYPE_SHARETREE].list, lGetList(ep, OR_joker)) != 0) {
         DPRINTF(("ORDER: ORT_share_tree failed\n" ));
         DRETURN(-1);
      }

      break;

   /* -----------------------------------------------------------------------
    * UPDATE FIELDS IN SCHEDULING CONFIGURATION
    * ----------------------------------------------------------------------- */
   case ORT_sched_conf:
      if (sconf_is()) {
         int pos;
         lListElem *joker = lFirst(lGetList(ep, OR_joker));;

         DPRINTF(("ORDER: ORT_sched_conf\n" ));

         if (joker != NULL) {
            if ((pos = lGetPosViaElem(joker, SC_weight_tickets_override, SGE_NO_ABORT)) > -1) {
               sconf_set_weight_tickets_override(lGetPosUlong(joker, pos));
            }
         }
         /* no need to spool sched conf */
      }

      break;

   case ORT_suspend_on_threshold:
      {
         lListElem *queueep;
         u_long32 jobid;

         DPRINTF(("ORDER: ORT_suspend_on_threshold\n"));

         jobid = lGetUlong(ep, OR_job_number);
         task_number = lGetUlong(ep, OR_ja_task_number);

         if (!(jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, jobid))
            || !(jatp = job_search_task(jep, NULL, task_number))
            || !lGetList(jatp, JAT_granted_destin_identifier_list)) {
            /* don't panic - it is probably an exiting job */
            WARNING((SGE_EVENT, MSG_JOB_SUSPOTNOTRUN_UU, sge_u32c(jobid), sge_u32c(task_number)));
         } else {
            const char *qnm = lGetString(lFirst(lGetList(jatp, JAT_granted_destin_identifier_list)), JG_qname);
            queueep = cqueue_list_locate_qinstance(*object_base[SGE_TYPE_CQUEUE].list, qnm);
            if (queueep == NULL) {
               ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDMQ_SU, qnm, sge_u32c(jobid)));
               DRETURN(-1);
            }

            INFO((SGE_EVENT, MSG_JOB_SUSPTQ_UUS, sge_u32c(jobid), sge_u32c(task_number), qnm));

            if (!ISSET(lGetUlong(jatp, JAT_state), JSUSPENDED)) {
               sge_signal_queue(ctx, SGE_SIGSTOP, queueep, jep, jatp, monitor);
               state = lGetUlong(jatp, JAT_state);
               CLEARBIT(JRUNNING, state);
               lSetUlong(jatp, JAT_state, state);
            }
            state = lGetUlong(jatp, JAT_state);
            SETBIT(JSUSPENDED_ON_THRESHOLD, state);
            lSetUlong(jatp, JAT_state, state);

            {
               lList *answer_list = NULL;
               const char *session = lGetString (jep, JB_session);
               sge_event_spool(ctx, &answer_list, 0, sgeE_JATASK_MOD,
                               jobid, task_number, NULL, NULL, session,
                               jep, jatp, NULL, true, true);
               answer_list_output(&answer_list);
            }

            /* update queues time stamp in schedd */
            lSetUlong(queueep, QU_last_suspend_threshold_ckeck, sge_get_gmt());
            qinstance_add_event(queueep, sgeE_QINSTANCE_MOD);
         }
      }
      break;

   case ORT_unsuspend_on_threshold:
      {
         lListElem *queueep;
         u_long32 jobid;

         DPRINTF(("ORDER: ORT_unsuspend_on_threshold\n"));

         jobid = lGetUlong(ep, OR_job_number);
         task_number = lGetUlong(ep, OR_ja_task_number);

         if (!(jep = job_list_locate(*object_base[SGE_TYPE_JOB].list, jobid))
            || !(jatp = job_search_task(jep, NULL,task_number))
            || !lGetList(jatp, JAT_granted_destin_identifier_list)) {
            /* don't panic - it is probably an exiting job */
            WARNING((SGE_EVENT, MSG_JOB_UNSUSPOTNOTRUN_UU, sge_u32c(jobid), sge_u32c(task_number)));
         } else {
            const char *qnm = lGetString(lFirst(lGetList(jatp, JAT_granted_destin_identifier_list)), JG_qname);
            queueep = cqueue_list_locate_qinstance(*object_base[SGE_TYPE_CQUEUE].list, qnm);
            if (queueep == NULL) {
               ERROR((SGE_EVENT, MSG_JOB_UNABLE2FINDMQ_SU, qnm, sge_u32c(jobid)));
               DRETURN(-1);
            }

            INFO((SGE_EVENT, MSG_JOB_UNSUSPOT_UUS, sge_u32c(jobid), sge_u32c(task_number), qnm));

            if (!ISSET(lGetUlong(jatp, JAT_state), JSUSPENDED)) {
               sge_signal_queue(ctx, SGE_SIGCONT, queueep, jep, jatp, monitor);
               state = lGetUlong(jatp, JAT_state);
               SETBIT(JRUNNING, state);
               lSetUlong(jatp, JAT_state, state);
            }
            state = lGetUlong(jatp, JAT_state);
            CLEARBIT(JSUSPENDED_ON_THRESHOLD, state);
            lSetUlong(jatp, JAT_state, state);
            {
               lList *answer_list = NULL;
               const char *session = lGetString (jep, JB_session);
               sge_event_spool(ctx, &answer_list, 0, sgeE_JATASK_MOD,
                               jobid, task_number, NULL, NULL, session,
                               jep, jatp, NULL, true, true);
               answer_list_output(&answer_list);
            }
            /* update queues time stamp in schedd */
            lSetUlong(queueep, QU_last_suspend_threshold_ckeck, sge_get_gmt());
            qinstance_add_event(queueep, sgeE_QINSTANCE_MOD);
         }
      }
      break;

   case ORT_job_schedd_info:
      {
         lList *sub_order_list = lGetList(ep, OR_joker);

         DPRINTF(("ORDER: ORT_job_schedd_info\n"));

         if (sub_order_list != NULL) {
            lListElem *sme  = lFirst(sub_order_list);

            if (sme != NULL) {
               lListElem *first;
               lList **master_job_schedd_info_list = object_base[SGE_TYPE_JOB_SCHEDD_INFO].list;

               DPRINTF(("ORDER: got %d schedd infos\n", lGetNumberOfElem(lGetList(sme, SME_message_list))));

               while ((first = lFirst(*master_job_schedd_info_list))) {
                  lRemoveElem(*master_job_schedd_info_list, &first);
               }
               if (*master_job_schedd_info_list == NULL) {
                  *master_job_schedd_info_list = lCreateList("schedd info", SME_Type);
               }
               lDechainElem(sub_order_list, sme);
               lAppendElem(*master_job_schedd_info_list, sme);

               /* this information is not spooled (but might be usefull in a db) */
               sge_add_event(0, sgeE_JOB_SCHEDD_INFO_MOD, 0, 0, NULL, NULL, NULL, sme);
            }
         }
      }
      break;

   default:
      break;
   }

   DRETURN(STATUS_OK);
}

/*
 * MT-NOTE: distribute_ticket_orders() is NOT MT safe
 */
int distribute_ticket_orders(sge_gdi_ctx_class_t *ctx, lList *ticket_orders, monitoring_t *monitor, object_description *object_base) 
{
   u_long32 now = sge_get_gmt();
   unsigned long last_heard_from = 0;
   int cl_err = CL_RETVAL_OK;
   lListElem *ep;

   DENTER(TOP_LAYER, "distribute_ticket_orders");
   
   for_each(ep, ticket_orders) {
      lList *to_send = lGetList(ep, RTIC_tickets);
      const char *host_name = lGetHost(ep, RTIC_host);
      lListElem *hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, host_name);

      int n = lGetNumberOfElem(to_send);

      if (hep) {
         cl_commlib_get_last_message_time((cl_com_get_handle(prognames[QMASTER], 0)),
                                        (char*)host_name, (char*)prognames[EXECD],1, &last_heard_from);
      }
      if (hep &&  last_heard_from + 10 * mconf_get_load_report_time() > now) {
         sge_pack_buffer pb;

         if (init_packbuffer(&pb, sizeof(u_long32)*3*n, 0)==PACK_SUCCESS) {
            u_long32 dummyid = 0;
            lListElem *ep2;
            for_each (ep2, to_send) {
               packint(&pb, lGetUlong(ep2, OR_job_number));
               packint(&pb, lGetUlong(ep2, OR_ja_task_number));
               packdouble(&pb, lGetDouble(ep2, OR_ticket));
            }
            cl_err = gdi2_send_message_pb(ctx, 0, prognames[EXECD], 1, host_name, 
                                         TAG_CHANGE_TICKET, &pb, &dummyid);
            MONITOR_MESSAGES_OUT(monitor);
            clear_packbuffer(&pb);
            DPRINTF(("%s %d ticket changings to execd@%s\n", 
                     (cl_err==CL_RETVAL_OK)?"failed sending":"sent", n,host_name));
         }
      } else {
         DPRINTF(("     skipped sending of %d ticket changings to "
               "execd@%s because %s\n", n, host_name, 
               !hep?"no such host registered":"suppose host is down"));
      }
   }

   DRETURN(cl_err);
}
