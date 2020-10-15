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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_prog.h"
#include "uti/setup_path.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_hostname.h"
#include "uti/sge_profiling.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_suser.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"

#include "sched/sge_select_queue.h"
#include "sched/sge_resource_quota_schedd.h"
#include "sched/debit.h"

#include "gdi/pack_job_delivery.h"
#include "gdi/sge_security.h"
#include "gdi/sge_gdi2.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "basis_types.h"
#include "sge_subordinate_qmaster.h" 
#include "sge_ckpt_qmaster.h"
#include "sge_job_qmaster.h" 
#include "sge_give_jobs.h"
#include "sge_host_qmaster.h"
#include "sge_event_master.h"
#include "sge_queue_event_master.h"
#include "sge_cqueue_qmaster.h"
#include "reschedule.h"
#include "sge_task_depend.h"
#include "sge_qmaster_timed_event.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_conf.h"
#include "msg_common.h"
#include "msg_qmaster.h"

static void 
sge_clear_granted_resources(sge_gdi_ctx_class_t *ctx,
                            lListElem *jep, lListElem *ja_task, int incslots, 
                            monitoring_t *monitor);

static void 
reduce_queue_limit(const lList* master_centry_list, 
                   lListElem *qep, lListElem *jep, 
                   int nm, char *rlimit_name);

static void 
release_successor_jobs(lListElem *jep);

static void 
release_successor_jobs_ad(lListElem *jep);

static void
release_successor_tasks_ad(lListElem *jep, u_long32 task_id);

static int 
send_slave_jobs(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jatep, 
                lListElem *pe, monitoring_t *monitor); 
                

static int 
send_slave_jobs_wc(sge_gdi_ctx_class_t *ctx, 
                   lListElem *tmpjep, monitoring_t *monitor);

static int 
send_job(sge_gdi_ctx_class_t *ctx, const char *rhost, lListElem *jep, lListElem *jatep, 
         lListElem *pe, lListElem *hep, int master);

static int 
sge_bury_job(bool job_spooling, const char *sge_root, lListElem *jep, u_long32 jid, lListElem *ja_task, int spool_job, int no_events);

static int 
sge_to_zombies(lListElem *jep, lListElem *ja_task);

static void 
sge_job_finish_event(lListElem *jep, lListElem *jatep, lListElem *jr, 
                     int commit_flags, const char *diagnosis);

static int 
setCheckpointObj(lListElem *job); 

static lListElem* 
copyJob(lListElem *job, lListElem *ja_task);

static int queue_field[] = { QU_qhostname,
                             QU_qname,
                             QU_full_name,
                             QU_job_slots,
                             QU_priority,
                             QU_tmpdir,
                             QU_shell_start_mode,
                             QU_shell,
                             QU_tmpdir,
                             QU_prolog,
                             QU_epilog,
                             QU_starter_method,
                             QU_suspend_method,
                             QU_processors,
                             QU_resume_method,
                             QU_terminate_method,
                             QU_min_cpu_interval,
                             QU_notify,
                             QU_consumable_config_list,
                             QU_resource_utilization,
                             QU_state,
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
                             NoName };


/************************************************************************
 Master function to give job to the execd.

 We make asynchron sends and implement a retry mechanism.
 Do everything to make sure the execd is prepared for receiving the job.
 ************************************************************************/
/* pe = is NULL for serial jobs*/
int sge_give_job(sge_gdi_ctx_class_t *ctx,
                 lListElem *jep, lListElem *jatep, lListElem *master_qep, 
                 lListElem *pe, lListElem *hep, monitoring_t *monitor)
{
   const char *rhost;
   int ret = 0; 
   int sent_slaves = 0;
   
   DENTER(TOP_LAYER, "sge_give_job");
   
   rhost = lGetHost(master_qep, QU_qhostname);
   DPRINTF(("execd host: %s\n", rhost));

   switch (send_slave_jobs(ctx, jep, jatep, pe, monitor)) {
      case -1 : ret = -1;
      case  0 : sent_slaves = 1;
         break;
      case  1 : sent_slaves = 0;
         break;
      default :
         DPRINTF(("send_slave_jobs returned an unknown error code\n"));
         ret = -1; 
   }

   if (!sent_slaves) {
      /* wait till all slaves are acked */
      lSetUlong(jatep, JAT_next_pe_task_id, 1);
      ret = send_job(ctx, rhost, jep, jatep, pe, hep, 1);
      MONITOR_MESSAGES_OUT(monitor);
   }

   DRETURN(ret);
}


/****** sge_give_jobs/send_slave_jobs() ****************************************
*  NAME
*     send_slave_jobs() -- send out slave tasks of a pe job
*
*  SYNOPSIS
*     static int send_slave_jobs(lListElem *jep, lListElem 
*     *jatep, lListElem *pe) 
*
*  FUNCTION
*     It prepares the data for the sending out the pe slaves. Once that data
*     is created, it calles the actual send_slave_method.
*
*  INPUTS
*     lListElem *jep     - job structure
*     lListElem *jatep   - ja-taks (template, not the actual one)
*     lListElem *pe      - target pe
*
*  RESULT
*     static int -  1 : no pe job
*                   0 : send slaves
*                  -1 : something went wrong 
*
*  NOTES
*     MT-NOTE: send_slave_jobs() is not MT safe 
*
*  SEE ALSO
*     ???/???
*******************************************************************************/
static int 
send_slave_jobs(sge_gdi_ctx_class_t *ctx, lListElem *jep, lListElem *jatep, lListElem *pe, monitoring_t *monitor)
{
   lListElem *tmpjep, *qep, *tmpjatep;
   lListElem *gdil_ep;
   int ret=0;
   bool is_pe_jobs = false;
   lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);
   lDescr *rdp = NULL;
   lEnumeration *what;

   DENTER(TOP_LAYER, "send_slave_jobs");

   /* do we have pe slave tasks* */
   for_each(gdil_ep, lGetList(jatep, JAT_granted_destin_identifier_list)) { 
      if (lGetUlong(gdil_ep, JG_tag_slave_job)) {
         lSetUlong(jatep, JAT_next_pe_task_id, 1);   
         is_pe_jobs = true;
         break;
      }
   }
   if (!is_pe_jobs) { /* we do not have slaves... nothing to do */
      DRETURN(1);
   }

   /* prepare the data to be send.... */

   /* create a copy of the job */
   if ((tmpjep = copyJob(jep, jatep)) == NULL) {
      DRETURN(-1);
   } 

   /* insert ckpt object if required **now**. it is only
   ** needed in the execd and we have no need to keep it
   ** in qmaster's permanent job list
   */
   if (setCheckpointObj(tmpjep) != 0) {
      lFreeElem(&tmpjep);
      DRETURN(-1);
   }

   /* add all queues referenced in gdil to qlp 
    * (necessary for availability of ALL resource limits and tempdir in queue) 
    * AND
    * copy all JG_processors from all queues to the JG_processors in gdil
    * (so the execd can decide on which processors (-sets) the job will be run).
    */
   what = lIntVector2What(QU_Type, queue_field);
   lReduceDescr(&rdp, QU_Type, what);

   tmpjatep = lFirst(lGetList(tmpjep, JB_ja_tasks));   
   for_each (gdil_ep, lGetList(tmpjatep, JAT_granted_destin_identifier_list)) {
      const char *src_qname = lGetString(gdil_ep, JG_qname);
      lListElem *src_qep = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), src_qname);

      if (src_qep == NULL) {
         DRETURN(-1);
      }  

      /* copy all JG_processors from all queues to gdil (which will be
       * sent to the execd).
       */
      lSetString(gdil_ep, JG_processors, lGetString(src_qep, QU_processors));

      qep = lSelectElemDPack(src_qep, NULL, rdp, what, false, NULL, NULL);

      /* build minimum of job request and queue resource limit */
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_cpu,   "s_cpu");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_cpu,   "h_cpu");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_core,  "s_core");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_core,  "h_core");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_data,  "s_data");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_data,  "h_data");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_stack, "s_stack");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_stack, "h_stack");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_rss,   "s_rss");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_rss,   "h_rss");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_fsize, "s_fsize");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_fsize, "h_fsize");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_vmem,  "s_vmem");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_vmem,  "h_vmem");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_rt,    "s_rt");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_rt,    "h_rt");

      lSetObject(gdil_ep, JG_queue, qep);
   }

   lFreeWhat(&what);
   sge_free(&rdp);

   if (pe) {
      lSetObject(tmpjatep, JAT_pe_object, lCopyElem(pe));
   }

   ret = send_slave_jobs_wc(ctx, tmpjep, monitor);

   lFreeElem(&tmpjep);

   DRETURN(ret);
}

/****** sge_give_jobs/send_slave_jobs_wc() *************************************
*  NAME
*     send_slave_jobs_wc() -- takes the prepared data end sends it out. 
*
*  SYNOPSIS
*     static int send_slave_jobs_wc(lListElem *tmpjep, 
*     lListElem *jatep, lListElem *pe, lList *qlp) 
*
*  FUNCTION
*     This is a helper function of send_slave_jobs. It handles the actual send.
*
*  INPUTS
*     lListElem *tmpjep  - prepared job structure
*     lListElem *jatep   - ja-taks (most likely the templete)
*     lListElem *pe      - target pe
*     lList *qlp         - prepared queue instance list
*
*  RESULT
*     static int -  0 : everything is fine
*                  -1 : an error
*
*  NOTES
*     MT-NOTE: send_slave_jobs_wc() is not MT safe 
*
*******************************************************************************/
static int 
send_slave_jobs_wc(sge_gdi_ctx_class_t *ctx, lListElem *jep, 
                   monitoring_t *monitor)
{
   lList *saved_gdil = NULL;
   lList *gdil;
   lListElem *gdil_ep = NULL;
   lListElem *next_gdil_ep = NULL;
   lListElem *jatep = lFirst(lGetList(jep, JB_ja_tasks));
   int ret = 0;
   int failed = CL_RETVAL_OK;
#ifndef __SGE_NO_USERMAPPING__    
   const char *old_mapped_user = NULL;
#endif   

   object_description *object_base = object_type_get_object_description();
   const char *sge_root = ctx->get_sge_root(ctx);
   bool simulate_execd = mconf_get_simulate_execds();

   DENTER(TOP_LAYER, "send_slave_jobs_wc");

   gdil = saved_gdil = lCreateList("", JG_Type);
   lXchgList(jatep, JAT_granted_destin_identifier_list, &saved_gdil);

   next_gdil_ep = lFirst(saved_gdil);
   while((gdil_ep = next_gdil_ep)) {
      lListElem *hep;
      const char* hostname = NULL;
      unsigned long last_heard_from;

      next_gdil_ep = lNext(gdil_ep);

      if (!lGetUlong(gdil_ep, JG_tag_slave_job)) {
         continue;
      }

      if (!(hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, lGetHost(gdil_ep, JG_qhostname)))) {
         ret = -1;   
         break;
      }
      hostname = lGetHost(gdil_ep, JG_qhostname);

      if (!simulate_execd) {
         /* do ask_commproc() only if we are missing load reports */
         cl_commlib_get_last_message_time(cl_com_get_handle(prognames[QMASTER], 0),
                                          hostname, prognames[EXECD], 1, &last_heard_from);
         if (last_heard_from + mconf_get_max_unheard() <= sge_get_gmt()) {

            ERROR((SGE_EVENT, MSG_COM_CANT_DELIVER_UNHEARD_SSU, prognames[EXECD], hostname, sge_u32c(lGetUlong(jep, JB_job_number))));
            sge_mark_unheard(hep);
            ret = -1;
            break;
         }
      }

      /*
      ** get credential for job
      */
      if (mconf_get_do_credentials()) {
         cache_sec_cred(sge_root, jep, hostname);
      }
  
#ifndef __SGE_NO_USERMAPPING__ 
      /* 
      ** This is for administrator user mapping 
      */
      {
         const char *owner = lGetString(jep, JB_owner);
         const char *mapped_user;

         DPRINTF(("send_job(): Starting job of %s\n", owner));
         cuser_list_map_user(*(cuser_list_get_master_list()), NULL,
                             owner, hostname, &mapped_user);
         if (strcmp(mapped_user, owner)) {
            DPRINTF(("execution mapping: user %s mapped to %s on host %s\n", 
                     owner, mapped_user, hostname));
            lSetString(jep, JB_owner, mapped_user);
         }
      }
#endif

      lDechainElem(saved_gdil, gdil_ep);
      lAppendElem(gdil, gdil_ep);

      /*
      ** security hook
      */
      tgt2cc(jep, hostname);
      {
         u_long32 dummymid = 0;
         sge_pack_buffer send_pb;

         init_packbuffer(&send_pb, 0, 0);

         pack_job_delivery(&send_pb, jep);
         if (!simulate_execd) {
            failed = gdi2_send_message_pb(ctx, 0, prognames[EXECD], 1, hostname, TAG_SLAVE_ALLOW, &send_pb, &dummymid);
         } else {
            failed = CL_RETVAL_OK;
         }
         clear_packbuffer(&send_pb);
      }
      MONITOR_MESSAGES_OUT(monitor);
      /*
      ** security hook
      */
      tgtcclr(jep, hostname); 

      if (failed != CL_RETVAL_OK) { 
         /* we failed sending the job to the execd */
         ERROR((SGE_EVENT, MSG_COM_SENDJOBTOHOST_US, sge_u32c(lGetUlong(jep, JB_job_number)), hostname));
         ERROR((SGE_EVENT, "commlib error: %s", cl_get_error_text(failed)));
         sge_mark_unheard(hep);
         ret = -1;
         break;
      } else {
         DPRINTF(("successfully sent slave job "sge_u32" to host \"%s\"\n", 
                  lGetUlong(jep, JB_job_number), hostname));
      }
      lRemoveElem(gdil, &gdil_ep);
   }
   lFreeList(&saved_gdil);

   DRETURN(ret);
}   

static int 
send_job(sge_gdi_ctx_class_t *ctx, 
         const char *rhost, lListElem *jep, lListElem *jatep,
         lListElem *pe, lListElem *hep, int master) 
{
   int failed;
   u_long32 now;
   sge_pack_buffer pb;
   lListElem *tmpjep, *qep, *tmpjatep = NULL;
   lListElem *gdil_ep;
   unsigned long last_heard_from;
   lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);
   const char* sge_root = ctx->get_sge_root(ctx);
   const char* myprogname = ctx->get_progname(ctx);
   bool job_spooling = ctx->get_job_spooling(ctx);
   bool simulate_execd = mconf_get_simulate_execds();
   lDescr *rdp = NULL;
   lEnumeration *what;

   DENTER(TOP_LAYER, "send_job");

   if (!simulate_execd) {
      cl_commlib_get_last_message_time(cl_com_get_handle(myprogname, 0), (char*)rhost, prognames[EXECD], 1, &last_heard_from);
      now = sge_get_gmt();
      if (last_heard_from + mconf_get_max_unheard() <= now) {

         ERROR((SGE_EVENT, MSG_COM_CANT_DELIVER_UNHEARD_SSU, prognames[EXECD], rhost, sge_u32c(lGetUlong(jep, JB_job_number))));
         sge_mark_unheard(hep);
         DEXIT;
         return -1;
      }
   }
   
   if ((tmpjep = copyJob(jep, jatep)) == NULL) {
      DRETURN(-1);
   } else {
      tmpjatep = lFirst(lGetList(tmpjep, JB_ja_tasks));
   }

   /* load script into job structure for sending to execd */
   /*
   ** if exec_file is not set, then this is an interactive job
   */
   if (master && job_spooling && lGetString(tmpjep, JB_exec_file) && !JOB_TYPE_IS_BINARY(lGetUlong(jep, JB_type))) {
      if (spool_read_script(NULL, lGetUlong(tmpjep, JB_job_number), tmpjep) == false) {
         lFreeElem(&tmpjep);
         DRETURN(-1);
      }
   }

   /* insert ckpt object if required **now**. it is only
   ** needed in the execd and we have no need to keep it
   ** in qmaster's permanent job list
   */
   if (setCheckpointObj(tmpjep) != 0) {
      lFreeElem(&tmpjep);
      DRETURN(-1);
   }
   
   /* add all queues referenced in gdil to qlp 
    * (necessary for availability of ALL resource limits and tempdir in queue) 
    * AND
    * copy all JG_processors from all queues to the JG_processors in gdil
    * (so the execd can decide on which processors (-sets) the job will be run).
    */
   what = lIntVector2What(QU_Type, queue_field);
   lReduceDescr(&rdp, QU_Type, what);

   for_each(gdil_ep, lGetList(tmpjatep, JAT_granted_destin_identifier_list)) {
      const char *src_qname = lGetString(gdil_ep, JG_qname);
      lListElem *src_qep = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), src_qname);

      if (src_qep == NULL) {
         DRETURN(-1);
      }  

      /* copy all JG_processors from all queues to gdil (which will be
       * sent to the execd).
       */
      lSetString(gdil_ep, JG_processors, lGetString(src_qep, QU_processors));

      qep = lSelectElemDPack(src_qep, NULL, rdp, what, false, NULL, NULL);

      /* build minimum of job request and queue resource limit */
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_cpu,   "s_cpu");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_cpu,   "h_cpu");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_core,  "s_core");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_core,  "h_core");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_data,  "s_data");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_data,  "h_data");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_stack, "s_stack");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_stack, "h_stack");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_rss,   "s_rss");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_rss,   "h_rss");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_fsize, "s_fsize");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_fsize, "h_fsize");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_vmem,  "s_vmem");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_vmem,  "h_vmem");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_s_rt,    "s_rt");
      reduce_queue_limit(master_centry_list, qep, tmpjep, QU_h_rt,    "h_rt");

      lSetObject(gdil_ep, JG_queue, qep);
   }

   lFreeWhat(&what);
   sge_free(&rdp);

   if (pe) {
      lSetObject(tmpjatep, JAT_pe_object, lCopyElem(pe));
   }

   /*
   ** get credential for job
   */
   if (mconf_get_do_credentials()) {
      cache_sec_cred(sge_root, tmpjep, rhost);
   }
  
#ifndef __SGE_NO_USERMAPPING__ 
   /* 
   ** This is for administrator user mapping 
   */
   {
      const char *owner = lGetString(tmpjep, JB_owner);
      const char *mapped_user;

      DPRINTF(("send_job(): Starting job of %s\n", owner));
      cuser_list_map_user(*(cuser_list_get_master_list()), NULL,
                          owner, rhost, &mapped_user);
      if (strcmp(mapped_user, owner)) {
         DPRINTF(("execution mapping: user %s mapped to %s on host %s\n", 
                  owner, mapped_user, rhost));
         lSetString(tmpjep, JB_owner, mapped_user);
      }
   }
#endif

   /*
   ** remove some data not used at execd side
   */
   lSetList(tmpjep, JB_ja_template, NULL);
 
   if (init_packbuffer(&pb, 0, 0) != PACK_SUCCESS) {
      lFreeElem(&tmpjep);
      DRETURN(-1);
   }

   pack_job_delivery(&pb, tmpjep);
   lFreeElem(&tmpjep);

   /*
   ** security hook
   */
   tgt2cc(jep, rhost);

   if (simulate_execd) { 
      failed = CL_RETVAL_OK;
   } else {
      u_long32 dummymid = 0;
      failed = gdi2_send_message_pb(ctx, 0, prognames[EXECD], 1, rhost, master?TAG_JOB_EXECUTION:TAG_SLAVE_ALLOW, &pb, &dummymid);
   }

   /*
   ** security hook
   */
   tgtcclr(jep, rhost);

   clear_packbuffer(&pb);

   /* We will get an acknowledge from execd. But we wait in the main loop so 
      that we can handle some requests until the acknowledge arrives.
      This is to make the qmaster non blocking */
      
   if (failed != CL_RETVAL_OK) { 
      /* we failed sending the job to the execd */
      ERROR((SGE_EVENT, MSG_COM_SENDJOBTOHOST_US, sge_u32c(lGetUlong(jep, JB_job_number)), rhost));
      ERROR((SGE_EVENT, "commlib error: %s", cl_get_error_text(failed)));
      sge_mark_unheard(hep);
      DRETURN(-1);
   } else {
      DPRINTF(("successfully sent %sjob "sge_u32" to host \"%s\"\n", 
               master?"":"SLAVE ", 
               lGetUlong(jep, JB_job_number), 
               rhost));
   }

   DRETURN(0);
}

void sge_job_resend_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem* jep, *jatep, *ep, *hep = NULL, *pe, *mqep;
   lList* jatasks;
   const char *qnm, *hnm;
   time_t now;
   u_long32 jobid = te_get_first_numeric_key(anEvent);
   u_long32 jataskid = te_get_second_numeric_key(anEvent);
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_job_resend_event_handler");
   
   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), jobid);
   jatep = job_search_task(jep, NULL, jataskid);
   now = (time_t)sge_get_gmt();

   if (jep == NULL || jatep == NULL) {
      WARNING((SGE_EVENT, MSG_COM_RESENDUNKNOWNJOB_UU, sge_u32c(jobid), sge_u32c(jataskid)));
      SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);     
      DEXIT;
      return;
   }

   jatasks = lGetList(jep, JB_ja_tasks);

   if (mconf_get_simulate_execds()) {
      const lListElem *argv1;
      int runtime = 3;

      if ((argv1 = lFirst(lGetList(jep, JB_job_args))))
         runtime = atoi(lGetString(argv1, ST_name));

      if (lGetUlong(jatep, JAT_status) == JTRANSFERING) {
         sge_commit_job(ctx, jep, jatep, NULL, COMMIT_ST_ARRIVED, COMMIT_DEFAULT, monitor);
         trigger_job_resend(now, hep, jobid, jataskid, runtime);

      } else { /* must be JRUNNING */
         lListElem *ue, *jr = lCreateElem(JR_Type);
         lSetUlong(jr, JR_job_number, jobid);
         lSetUlong(jr, JR_ja_task_number, jataskid);

         if ((ep = lFirst(lGetList(jatep, JAT_granted_destin_identifier_list)))) {
            lSetString(jr, JR_queue_name, lGetString(ep, JG_qname));
            lSetUlong(jr, JR_wait_status, SGE_SET_WEXITSTATUS(SGE_WEXITED_BIT, 0)); /* returned with exit(0) */

            ue = lAddSubStr(jr, UA_name, "submission_time", JR_usage, UA_Type);
            lSetDouble(ue, UA_value, lGetUlong(jep, JB_submission_time)); 

            ue = lAddSubStr(jr, UA_name, "start_time", JR_usage, UA_Type);
            lSetDouble(ue, UA_value, lGetUlong(jatep, JAT_start_time)); 

            ue = lAddSubStr(jr, UA_name, "end_time", JR_usage, UA_Type);
            lSetDouble(ue, UA_value, now); 

            ue = lAddSubStr(jr, UA_name, "ru_wallclock", JR_usage, UA_Type);
            lSetDouble(ue, UA_value, runtime);

            lXchgList(jr, JR_usage, lGetListRef(jatep, JAT_usage_list));
            reporting_create_acct_record(ctx, NULL, jr, jep, jatep, false);
            sge_commit_job(ctx, jep, jatep, jr, COMMIT_ST_FINISHED_FAILED_EE, COMMIT_DEFAULT, monitor);
         }
         lFreeElem(&jr); 
      }

      SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

      DEXIT;
      return;
   }

   /* check whether a slave execd allowance has to be retransmitted */
   if (lGetUlong(jatep, JAT_status) == JTRANSFERING) {

      ep = lFirst(lGetList(jatep, JAT_granted_destin_identifier_list));

      if (!ep || !(qnm=lGetString(ep, JG_qname)) || !(hnm=lGetHost(ep, JG_qhostname)))
      {
         ERROR((SGE_EVENT, MSG_JOB_UNKNOWNGDIL4TJ_UU, sge_u32c(jobid), sge_u32c(jataskid)));
         lDelElemUlong(object_type_get_master_list(SGE_TYPE_JOB), JB_job_number, jobid);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         DEXIT;
         return;
      }

      mqep = cqueue_list_locate_qinstance(*(object_type_get_master_list(SGE_TYPE_CQUEUE)), qnm);

      if (mqep == NULL)
      {
         ERROR((SGE_EVENT, MSG_JOB_NOQUEUE4TJ_SUU, qnm, sge_u32c(jobid), sge_u32c(jataskid)));
         lDelElemUlong(&jatasks, JAT_task_number, jataskid);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         DEXIT;
         return;
      }

      if (!(hnm=lGetHost(mqep, QU_qhostname)) || !(hep = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, hnm)))
      {
         ERROR((SGE_EVENT, MSG_JOB_NOHOST4TJ_SUU, hnm, sge_u32c(jobid), sge_u32c(jataskid)));
         lDelElemUlong(&jatasks, JAT_task_number, jataskid);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         DEXIT;
         return;
      }
      
      if (qinstance_state_is_unknown(mqep))
      {
         trigger_job_resend(now, hep, jobid, jataskid, 0);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
         DEXIT;
         return; /* try later again */
      }

      if (lGetString(jatep, JAT_granted_pe)) {
         if (!(pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, lGetString(jatep, JAT_granted_pe)))) {
            ERROR((SGE_EVENT, MSG_JOB_NOPE4TJ_SUU, lGetString(jep, JB_pe), sge_u32c(jobid), sge_u32c(jataskid)));
            lDelElemUlong(object_type_get_master_list(SGE_TYPE_JOB), JB_job_number, jobid);
            SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
            DEXIT;
            return;
         }
      } else {
         pe = NULL;
      }

      if (lGetUlong(jatep, JAT_start_time)) {
         WARNING((SGE_EVENT, MSG_JOB_DELIVER2Q_UUS, sge_u32c(jobid), 
                  sge_u32c(jataskid), lGetString(jatep, JAT_master_queue)));
      }

      /* send job to execd */
      sge_give_job(ctx, jep, jatep, mqep, pe, hep, monitor);
 
      /* reset timer */
      lSetUlong(jatep, JAT_start_time, now);

      /* initialize resending of job if not acknowledged by execd */
      trigger_job_resend(now, hep, lGetUlong(jep, JB_job_number), 
                         lGetUlong(jatep, JAT_task_number), 0);
   }

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
   return;
} 

void cancel_job_resend(u_long32 jid, u_long32 ja_task_id)
{
   DENTER(TOP_LAYER, "cancel_job_resend");

   DPRINTF(("CANCEL JOB RESEND "sge_u32"/"sge_u32"\n", jid, ja_task_id)); 
   te_delete_one_time_event(TYPE_JOB_RESEND_EVENT, jid, ja_task_id, "job-resend_event");

   DRETURN_VOID;
}

/* 
 * if hep equals to NULL resend is triggered immediatelly 
 */ 
void trigger_job_resend(u_long32 now, lListElem *hep, u_long32 jid, u_long32 ja_task_id, int delta)
{
   u_long32 seconds;
   time_t when = 0;
   te_event_t ev = NULL;

   DENTER(TOP_LAYER, "trigger_job_resend");

   if (mconf_get_simulate_execds()) {
      seconds = delta;
   } else {
      seconds = hep ? MAX(load_report_interval(hep), MAX_JOB_DELIVER_TIME) : 0;
   }
   DPRINTF(("TRIGGER JOB RESEND "sge_u32"/"sge_u32" in "sge_u32" seconds\n", jid, ja_task_id, seconds));

   when = (time_t)(now + seconds);
   ev = te_new_event(when, TYPE_JOB_RESEND_EVENT, ONE_TIME_EVENT, jid, ja_task_id, "job-resend_event");
   te_add_event(ev);
   te_free_event(&ev);

   DEXIT;
   return;
}

/***********************************************************************
 sge_zombie_job_cleanup_handler

 Remove zombie jobs, which have expired (currently, we keep a list of
 conf.zombie_jobs entries)
 ***********************************************************************/
void sge_zombie_job_cleanup_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem *dep;
   lList *master_zombie_list = *(object_type_get_master_list(SGE_TYPE_ZOMBIE));
   const u_long32 zombie_count = mconf_get_zombie_jobs();
  
   DENTER(TOP_LAYER, "sge_zombie_job_cleanup_handler");

   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   while (lGetNumberOfElem(master_zombie_list) > zombie_count) {
      dep = lFirst(master_zombie_list);
      lRemoveElem(master_zombie_list, &dep);
   }

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
}

/****** sge_give_jobs/sge_commit_job() *****************************************
*  NAME
*     sge_commit_job() -- Do job state transitions
*
*  SYNOPSIS
*     void sge_commit_job(lListElem *jep, lListElem *jatep, lListElem *jr, 
*     sge_commit_mode_t mode, sge_commit_flags_t commit_flags) 
*
*  FUNCTION
*     sge_commit_job() implements job state transitons. When a dispatch
*     order arrives from schedd the job is sent asynchonously to execd and 
*     sge_commit_job() is called with mode==COMMIT_ST_SENT. When a job report 
*     arrives from the execd mode is COMMIT_ST_ARRIVED. When the job failed
*     or finished mode is COMMIT_ST_FINISHED_FAILED or 
*     COMMIT_ST_FINISHED_FAILED_EE depending on product mode:
*
*     A job can be removed immediately when it is finished
*     (mode==COMMIT_ST_FINISHED_FAILED). A job may not be deleted
*     (mode==COMMIT_ST_FINISHED_FAILED_EE) before the scheduler has debited 
*     the jobs resource consumption in the corresponding objects (project/user/..). 
*     Only the job script may be deleted at this stage. When an order arrives at 
*     qmaster telling that debitation was done (mode==COMMIT_ST_DEBITED_EE) the 
*     job can be deleted.
*     
*     sge_commit_job() releases resources when a job is no longer running.
*     Also state changes jobs are spooled and finally spooling files are 
*     deleted. Also jobs start time is set to the actual time when job is sent.
*
*  INPUTS
*     lListElem *jep                  - the job 
*     lListElem *jatep                - the array task
*     lListElem *jr                   - the job report (may be NULL)
*     sge_commit_mode_t mode          - the 'mode' - actually the state transition
*     sge_commit_flags_t commit_flags - additional flags for parametrizing 
*
*  SEE ALSO
*     See sge_commit_mode_t typedef for documentation on mode.
*     See sge_commit_flags_t typedef for documentation on commit_flags.
*******************************************************************************/
void sge_commit_job(sge_gdi_ctx_class_t *ctx, 
                    lListElem *jep, lListElem *jatep, lListElem *jr, 
                    sge_commit_mode_t mode, int commit_flags, monitoring_t *monitor)
{
   lListElem *petask, *tmp_ja_task;
   lListElem *global_host_ep;
   lUlong jobid, jataskid;
   int no_unlink = 0;
   int spool_job = !(commit_flags & COMMIT_NO_SPOOLING);
   int no_events = (commit_flags & COMMIT_NO_EVENTS);
   int unenrolled_task = (commit_flags & COMMIT_UNENROLLED_TASK);
   int handle_zombies = (mconf_get_zombie_jobs() > 0);
   u_long32 now = 0;
   const char *session;
   lList *answer_list = NULL;
   object_description *object_base = object_type_get_object_description();
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   lList *master_exechost_list = *object_base[SGE_TYPE_EXECHOST].list;
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_userset_list = *object_base[SGE_TYPE_USERSET].list;
   lList *master_hgroup_list = *object_base[SGE_TYPE_HGROUP].list;
   lList *master_rqs_list = *object_base[SGE_TYPE_RQS].list;
   lList *gdil = lGetList(jatep, JAT_granted_destin_identifier_list);
   lListElem *rqs = NULL;
   lListElem *ep = NULL;

   /* need hostname for job_log */
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   const char *sge_root = ctx->get_sge_root(ctx);
   bool job_spooling = ctx->get_job_spooling(ctx);
   u_long32 task_wallclock = U_LONG32_MAX;
   bool compute_qwallclock = false;
   u_long32 state = 0;

   DENTER(TOP_LAYER, "sge_commit_job");

   jobid = lGetUlong(jep, JB_job_number);
   jataskid = jatep?lGetUlong(jatep, JAT_task_number):0;
   session = lGetString(jep, JB_session);

   now = sge_get_gmt();

   switch (mode) {
   case COMMIT_ST_SENT:
   {
      bool master_task = true;
      lList *master_ar_list = *object_base[SGE_TYPE_AR].list;

      lSetUlong(jatep, JAT_state, JRUNNING);
      lSetUlong(jatep, JAT_status, JTRANSFERING);

      if (!job_get_wallclock_limit(&task_wallclock, jep)) {
         compute_qwallclock = true;
      }

      reporting_create_job_log(NULL, now, JL_SENT, MSG_QMASTER, qualified_hostname, jr, jep, jatep, NULL, MSG_LOG_SENT2EXECD);

      global_host_ep = host_list_locate(master_exechost_list, "global");
      for_each(ep, gdil) {
         lListElem *ar = NULL;
         u_long32 ar_id = lGetUlong(jep, JB_ar);
         const char *queue_name = lGetString(ep, JG_qname);
         lListElem *queue = NULL;
         
         if ((queue = cqueue_list_locate_qinstance(master_cqueue_list, queue_name)) == NULL) {
            ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDQUEUEXREFERENCEDINJOBY_SU,  
                      queue_name, sge_u32c(jobid)));
            master_task = false;
         } else if (ar_id != 0 && (ar = lGetElemUlong(master_ar_list, AR_id, ar_id)) == NULL) {
            ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDARXREFERENCEDINJOBY_UU,
                  sge_u32c(ar_id), sge_u32c(jobid)));
            master_task = false;
         } else {
            const char *queue_hostname = lGetHost(queue, QU_qhostname);
            const char *limit = NULL;
            int tmp_slot = lGetUlong(ep, JG_slots);
            lListElem *host;

            if (compute_qwallclock) {
               limit = lGetString(queue, QU_h_rt);
               if (strcasecmp(limit, "infinity") != 0) {
                  u_long32 clock_val;
                  parse_ulong_val(NULL, &clock_val, TYPE_TIM, limit, NULL, 0);
                  task_wallclock = MIN(task_wallclock, clock_val);
               } else {
                  task_wallclock = MIN(task_wallclock, U_LONG32_MAX);
               }

               limit = lGetString(queue, QU_s_rt);
               if (strcasecmp(limit, "infinity") != 0) {
                  u_long32 clock_val;
                  parse_ulong_val(NULL, &clock_val, TYPE_TIM, limit, NULL, 0);
                  task_wallclock = MIN(task_wallclock, clock_val);
               } else {
                  task_wallclock = MIN(task_wallclock, U_LONG32_MAX);
               }
            }

            /* debit consumable resources */ 
            if (debit_host_consumable(jep, global_host_ep, master_centry_list, tmp_slot, master_task, NULL) > 0) {
               /* this info is not spooled */
               sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                             "global", NULL, NULL, global_host_ep);
               reporting_create_host_consumable_record(&answer_list, global_host_ep, jep, now);
               answer_list_output(&answer_list);
               lListElem_clear_changed_info(global_host_ep);
            }
            host = host_list_locate(master_exechost_list, queue_hostname);
            if (debit_host_consumable(jep, host, master_centry_list, tmp_slot, master_task, NULL) > 0) {
               /* this info is not spooled */
               sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                             queue_hostname, NULL, NULL, host);
               reporting_create_host_consumable_record(&answer_list, host, jep, now);
               answer_list_output(&answer_list);
               lListElem_clear_changed_info(host);
            }
            qinstance_debit_consumable(queue, jep, master_centry_list, tmp_slot, master_task, NULL);
            reporting_create_queue_consumable_record(&answer_list, host, queue, jep, now);
            answer_list_output(&answer_list);
            /* this info is not spooled */
            qinstance_add_event(queue, sgeE_QINSTANCE_MOD);
            lListElem_clear_changed_info(queue);

            if (ar_id == 0) {
               /* debit resource quota set */
               for_each(rqs, master_rqs_list) {
                  if (rqs_debit_consumable(rqs, jep, ep, lGetString(jatep, JAT_granted_pe), master_centry_list, 
                                            master_userset_list, master_hgroup_list, tmp_slot, master_task) > 0) {
                     /* this info is not spooled */
                     sge_add_event(0, sgeE_RQS_MOD, 0, 0, lGetString(rqs, RQS_name), NULL, NULL, rqs);
                     lListElem_clear_changed_info(rqs);
                  }
               }
            } else {
               /* debit in advance reservation */
               lListElem *queue = lGetSubStr(ar, QU_full_name, lGetString(ep, JG_qname), AR_reserved_queues);
               if (qinstance_debit_consumable(queue, jep, master_centry_list, tmp_slot, master_task, NULL) > 0) {
                  dstring buffer = DSTRING_INIT;
                  /* this info is not spooled */
                  sge_dstring_sprintf(&buffer, sge_U32CFormat, ar_id);
                  sge_add_event(0, sgeE_AR_MOD, ar_id, 0, sge_dstring_get_string(&buffer), NULL, NULL, ar);
                  lListElem_clear_changed_info(ar);
                  sge_dstring_free(&buffer);
               }
            }
         }
         master_task = false;
      }

      lSetUlong(jatep, JAT_wallclock_limit, task_wallclock);

      /* 
       * Would be nice if we could use a more accurate start time that could be reported 
       * by execd. However this would constrain time synchronization between qmaster and 
       * execd host .. sigh!
       */
      lSetUlong(jatep, JAT_start_time, now);
      job_enroll(jep, NULL, jataskid);
      sge_event_spool(ctx, &answer_list, now, sgeE_JATASK_MOD, jobid, jataskid, NULL, NULL, session,
                      jep, jatep, NULL, true, true);
      answer_list_output(&answer_list);
      break;
   }
   case COMMIT_ST_ARRIVED:
      lSetUlong(jatep, JAT_status, JRUNNING);
      reporting_create_job_log(NULL, now, JL_DELIVERED, MSG_QMASTER, qualified_hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_DELIVERED);
      job_enroll(jep, NULL, jataskid);
      {
         dstring buffer = DSTRING_INIT;
         /* JG: TODO: why don't we generate an event? */
         spool_write_object(&answer_list, spool_get_default_context(), jatep, 
                            job_get_key(jobid, jataskid, NULL, &buffer), SGE_TYPE_JATASK, job_spooling);
         answer_list_output(&answer_list);
         lListElem_clear_changed_info(jatep);
         sge_dstring_free(&buffer);
      }
      break;

   case COMMIT_ST_RESCHEDULED:
   case COMMIT_ST_USER_RESCHEDULED:
   case COMMIT_ST_FAILED_AND_ERROR:
      WARNING((SGE_EVENT, MSG_JOB_RESCHEDULE_UU, sge_u32c(jobid), sge_u32c(jataskid)));

      reporting_create_job_log(NULL, now, JL_RESTART, MSG_QMASTER, qualified_hostname, jr, jep, jatep, NULL, SGE_EVENT);
      /* JG: TODO: no accounting record created? Or somewhere else? */
      /* add a reschedule unknown list entry to all slave
         hosts where a part of that job ran */
      {
         const char *pe_name = lGetString(jep, JB_pe);      

         if (pe_name) {
            lListElem *pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, pe_name);
            lListElem *granted_queue;

            if (pe && lGetBool(pe, PE_control_slaves)) { 
               bool is_master = true;

               for_each(granted_queue, lGetList(jatep, JAT_granted_destin_identifier_list)) { 
                  if (!is_master) {
                     lListElem *host = host_list_locate(master_exechost_list, lGetHost(granted_queue, JG_qhostname)); 
                     
                     add_to_reschedule_unknown_list(ctx, host, jobid, jataskid, RESCHEDULE_HANDLE_JR_WAIT);

                     DPRINTF(("RU: sge_commit_job: granted_queue %s job "sge_u32"."sge_u32"\n", 
                              lGetString(granted_queue, JG_qname), jobid, jataskid));
                  }
                  is_master = false;
               }
            }
         } else {
            lList *granted_list = lGetList(jatep, JAT_granted_destin_identifier_list);
            lListElem *granted_queue = lFirst(granted_list);
            lListElem *host = host_list_locate(master_exechost_list, lGetHost(granted_queue, JG_qhostname));

            add_to_reschedule_unknown_list(ctx, host, jobid, jataskid, RESCHEDULE_SKIP_JR);
         }
      }

      /*
       * Preserve any potential deferred startup request
       * in JAT_state across re-initialization.
       */
      lSetUlong(jatep, JAT_status, JIDLE);
      state = lGetUlong(jatep, JAT_state);
      state &= JDEFERRED_REQ;
      if (mode == COMMIT_ST_RESCHEDULED || mode == COMMIT_ST_USER_RESCHEDULED) {
         state |= JQUEUED|JWAITING;
      } else {
         state |= JQUEUED|JWAITING|JERROR;
      }
      lSetUlong(jatep, JAT_state, state);

      /*
       * Prepare usage lists for the next run 
       */
      lSetList(jatep, JAT_previous_usage_list, lCopyList("name", lGetList(jatep, JAT_scaled_usage_list)));
      lSetList(jatep, JAT_scaled_usage_list, NULL);
      lSetList(jatep, JAT_reported_usage_list, NULL);

      /* 
       * Submit time will be set to the current timestamp in case a user 
       * reschedules his own jobs. This prevents users from "stealing" cpu
       * resources. As a result of that the rescheduled job will be put 
       * at the end of the pending job list instead of the top.
       *
       * Insert at the end of the pending job list can be disabled by
       * two qmaster params. OLD_RESCHEDULE_BEHAVIOR disables it for non-array jobs
       * whereas OLD_RESCHEDULE_BEHAVIOR_ARRAY_JOB disables it for array job tasks.
       */
      if (mode == COMMIT_ST_USER_RESCHEDULED) {
         bool is_array_job = job_is_array(jep);
   
         if ((is_array_job == false && mconf_get_old_reschedule_behavior() == false) ||
             (is_array_job == true && mconf_get_old_reschedule_behavior_array_job() == false)) {
            lSetUlong(jep, JB_submission_time, now);
            sge_event_spool(ctx, &answer_list, now, sgeE_JOB_MOD, jobid, jataskid, 
                            NULL, NULL, session, jep, jatep, NULL, true, true);
         }
      }

      /* sum up the usage of all pe tasks not yet deleted (e.g. tasks from 
       * exec host in unknown state). Then remove all pe tasks except the 
       * past usage container 
       */
      { 
         lList *pe_task_list = lGetList(jatep, JAT_task_list);

         if (pe_task_list != NULL) {
            lListElem *existing_container = lGetElemStr(pe_task_list, PET_id, PE_TASK_PAST_USAGE_CONTAINER);
            lListElem *container = pe_task_sum_past_usage_all(pe_task_list);
            lListElem *pe_task = NULL;
            lListElem *next = NULL;

            /* the usage container is not spooled */
            if (existing_container == NULL) {
               sge_add_event(now, sgeE_PETASK_ADD, jobid, jataskid, PE_TASK_PAST_USAGE_CONTAINER, 
                             NULL, session, container);
               lListElem_clear_changed_info(container);
            } else {
               sge_add_list_event(now, sgeE_JOB_USAGE, jobid, jataskid, PE_TASK_PAST_USAGE_CONTAINER, 
                                  NULL, session, lGetList(container, PET_scaled_usage));
               lList_clear_changed_info(lGetList(container, PET_scaled_usage));
            }

            next = lFirst(pe_task_list);
            while ((pe_task = next) != NULL) {
               next = lNext(next);
               if (pe_task != container) {
                  lRemoveElem(pe_task_list, &pe_task);
               }
            }
         }
      }   

      sge_clear_granted_resources(ctx, jep, jatep, 1, monitor);
      ja_task_clear_finished_pe_tasks(jatep);
      job_enroll(jep, NULL, jataskid);
      sge_event_spool(ctx, &answer_list, now, sgeE_JATASK_MOD, jobid, jataskid, NULL, NULL, session,
                      jep, jatep, NULL, true, true);
      
      answer_list_output(&answer_list);
      break;

   case COMMIT_ST_FINISHED_FAILED:
      reporting_create_job_log(NULL, now, JL_FINISHED, MSG_QMASTER, qualified_hostname, jr, jep, 
                               jatep, NULL, MSG_LOG_EXITED);
      remove_from_reschedule_unknown_lists(ctx, jobid, jataskid);
      if (handle_zombies) {
         sge_to_zombies(jep, jatep);
      }
      if (!unenrolled_task) { 
         sge_clear_granted_resources(ctx, jep, jatep, 1, monitor);
      }
      sge_job_finish_event(jep, jatep, jr, commit_flags, NULL); 
      sge_bury_job(job_spooling, sge_root, jep, jobid, jatep, spool_job, no_events);
      break;
   case COMMIT_ST_FINISHED_FAILED_EE:
      reporting_create_job_log(NULL, now, JL_FINISHED, MSG_QMASTER, qualified_hostname, 
                               jr, jep, jatep, NULL, MSG_LOG_WAIT4SGEDEL);
      remove_from_reschedule_unknown_lists(ctx, jobid, jataskid);

      lSetUlong(jatep, JAT_status, JFINISHED);

      sge_job_finish_event(jep, jatep, jr, commit_flags, NULL); 

      /* possibly release successor tasks if there are any array dependencies on this task */
      if (jataskid) {
         release_successor_tasks_ad(jep, jataskid);
      }
      if (handle_zombies) {
         sge_to_zombies(jep, jatep);
      }
      sge_clear_granted_resources(ctx, jep, jatep, 1, monitor);
      job_enroll(jep, NULL, jataskid);
      for_each(petask, lGetList(jatep, JAT_task_list)) {
         sge_add_list_event( now, sgeE_JOB_FINAL_USAGE, jobid,
                            jataskid,
                            lGetString(petask, PET_id), 
                            NULL, session,
                            lGetList(petask, PET_scaled_usage));
      }

      sge_add_list_event(now, sgeE_JOB_FINAL_USAGE, jobid, jataskid,
                            NULL, NULL, session, lGetList(jatep, JAT_scaled_usage_list));

      spool_transaction(&answer_list, spool_get_default_context(), STC_begin);

      sge_event_spool(ctx, &answer_list, 0, sgeE_JATASK_MOD, jobid, jataskid, NULL, NULL, 
                      session, jep, jatep, NULL, false, true);

      if (job_get_not_enrolled_ja_tasks(jep)) {
         no_unlink = 1;
      } else {
         /* finished all ja-tasks => remove job script */
         for_each(tmp_ja_task, lGetList(jep, JB_ja_tasks)) {
            if (lGetUlong(tmp_ja_task, JAT_status) != JFINISHED) {
               no_unlink = 1;
               break;
            }
         }
      }

      if (!no_unlink) {
         release_successor_jobs(jep);
         release_successor_jobs_ad(jep);
         if ((lGetString(jep, JB_exec_file) != NULL) && job_spooling && !JOB_TYPE_IS_BINARY(lGetUlong(jep, JB_type))) {
            spool_delete_script(&answer_list, jobid, jep);
         }
      }
      spool_transaction(&answer_list, spool_get_default_context(), STC_commit);
      answer_list_output(&answer_list);
      break;

   case COMMIT_ST_DEBITED_EE: /* triggered by ORT_remove_job */
   case COMMIT_ST_NO_RESOURCES: /* triggered by ORT_remove_immediate_job */
      reporting_create_job_log(NULL, now, JL_DELETED, MSG_SCHEDD, qualified_hostname, jr, jep, jatep, NULL, 
                               (mode==COMMIT_ST_DEBITED_EE) ?  MSG_LOG_DELSGE : MSG_LOG_DELIMMEDIATE);

      if (mode == COMMIT_ST_NO_RESOURCES) {
         sge_job_finish_event(jep, jatep, jr, commit_flags, NULL); 
      }   
      sge_bury_job(job_spooling, sge_root, jep, jobid, jatep, spool_job, no_events);
      break;
   
   case COMMIT_ST_DELIVERY_FAILED: 
      /* The same as case COMMIT_ST_RESCHEDULED except sge_clear_granted_resources() may not increase free slots. */
      WARNING((SGE_EVENT, MSG_JOB_RESCHEDULE_UU, sge_u32c(jobid), sge_u32c(jataskid)));
      reporting_create_job_log(NULL, now, JL_RESTART, MSG_QMASTER, qualified_hostname, jr, jep, jatep, NULL, SGE_EVENT);
      lSetUlong(jatep, JAT_status, JIDLE);
      lSetUlong(jatep, JAT_state, JQUEUED | JWAITING);
      sge_clear_granted_resources(ctx, jep, jatep, 0, monitor);
      job_enroll(jep, NULL, jataskid);
      sge_event_spool(ctx, &answer_list, now, sgeE_JATASK_MOD, jobid, jataskid, 
                      NULL, NULL, session, jep, jatep, NULL, true, false);
      answer_list_output(&answer_list);
      break;
   }

   DRETURN_VOID;
}

   /* 
    * Job finish information required by DRMAA
    * u_long32 jobid
    * u_long32 taskid
    * u_long32 status
    *    SGE_WEXITED   flag
    *    SGE_WSIGNALED flag
    *    SGE_WCOREDUMP flag
    *    SGE_WABORTED  flag (= COMMIT_NEVER_RAN)
    *    exit status  (8 bit)
    *    signal       (8 bit)
    * lList *rusage;
    *
    * Additional job finish information provided by JAPI
    * char *failed_reason
    */
static void sge_job_finish_event(lListElem *jep, lListElem *jatep, lListElem *jr, 
                  int commit_flags, const char *diagnosis)
{
   bool release_jr = false;

   DENTER(TOP_LAYER, "sge_job_finish_event");

   if (!jr) {
      jr = lCreateElem(JR_Type);
      lSetUlong(jr, JR_job_number, lGetUlong(jep, JB_job_number));
      lSetUlong(jr, JR_ja_task_number, lGetUlong(jatep, JAT_task_number));
      lSetUlong(jr, JR_wait_status, SGE_NEVERRAN_BIT);
      release_jr = true;   
   } else {
      /* JR_usage gets cleared in the process of cleaning up a finished job.
       * It's contents get put into JAT_usage_list and eventually added to
       * JAT_usage_list.  In the JAPI, however, it would be ugly to have
       * to go picking through the Master_Job_List to find the accounting data.
       * So instead we pick through it here and stick it back in JR_usage. */

      lXchgList(jr, JR_usage, lGetListRef(jatep, JAT_usage_list));

      if (commit_flags & COMMIT_NEVER_RAN) {
         lSetUlong(jr, JR_wait_status, SGE_NEVERRAN_BIT);
      }   
   } 

   if (diagnosis != NULL) {
      lSetString(jr, JR_err_str, diagnosis);
   } else if (!lGetString(jr, JR_err_str)) {
      if (SGE_GET_NEVERRAN(lGetUlong(jr, JR_wait_status))) {
         lSetString(jr, JR_err_str, "Job never ran");
      } else {
         lSetString(jr, JR_err_str, "Unknown job finish condition");
      }   
   }

   sge_add_event(0, sgeE_JOB_FINISH, lGetUlong(jep, JB_job_number), 
                 lGetUlong(jatep, JAT_task_number), NULL, NULL,
                 lGetString(jep, JB_session), jr);

   if (release_jr) {
      lFreeElem(&jr);
   } else {
      lXchgList(jr, JR_usage, lGetListRef(jatep, JAT_usage_list));
   }

   DRETURN_VOID;
}

static void release_successor_jobs(lListElem *jep)
{
   lListElem *jid, *suc_jep;
   u_long32 job_ident;

   DENTER(TOP_LAYER, "release_successor_jobs");

   for_each(jid, lGetList(jep, JB_jid_successor_list)) {
      suc_jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), lGetUlong(jid, JRE_job_number));
      if (suc_jep) {
         /* if we don't find it by job id we try it with the name */
         job_ident = lGetUlong(jep, JB_job_number);
         if (!lDelSubUlong(suc_jep, JRE_job_number, job_ident, JB_jid_predecessor_list)) {

             DPRINTF(("no reference "sge_u32" and %s to job "sge_u32" in predecessor list of job "sge_u32"\n", 
               job_ident, lGetString(jep, JB_job_name),
               lGetUlong(suc_jep, JB_job_number), lGetUlong(jep, JB_job_number)));
         } else {
            if (lGetList(suc_jep, JB_jid_predecessor_list)) {
               DPRINTF(("removed job "sge_u32"'s dependance from exiting job "sge_u32"\n",
                  lGetUlong(suc_jep, JB_job_number), lGetUlong(jep, JB_job_number)));
            } else 
               DPRINTF(("job "sge_u32"'s job exit triggers start of job "sge_u32"\n",
                  lGetUlong(jep, JB_job_number), lGetUlong(suc_jep, JB_job_number)));
            sge_add_job_event(sgeE_JOB_MOD, suc_jep, NULL);
         }
      }
   }

   DEXIT;
   return;
}

static void release_successor_jobs_ad(lListElem *jep) 
{
   lListElem *jid, *suc_jep;
   u_long32 job_ident;

   DENTER(TOP_LAYER, "release_successor_jobs_ad");

   for_each(jid, lGetList(jep, JB_ja_ad_successor_list)) {
      suc_jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), lGetUlong(jid, JRE_job_number));
      if (suc_jep) {
         int Modified = 0;
         /* if we don't find it by job id we try it with the name */
         job_ident = lGetUlong(jep, JB_job_number);
         if (!lDelSubUlong(suc_jep, JRE_job_number, job_ident, JB_ja_ad_predecessor_list)) {
             DPRINTF(("no reference "sge_u32" and %s to job "sge_u32" in array predecessor list of job "sge_u32"\n", 
               job_ident, lGetString(jep, JB_job_name),
               lGetUlong(suc_jep, JB_job_number), lGetUlong(jep, JB_job_number)));
         } else {
            if (lGetList(suc_jep, JB_ja_ad_predecessor_list)) {
               DPRINTF(("removed job "sge_u32"'s array dependance from exiting job "sge_u32"\n",
                  lGetUlong(suc_jep, JB_job_number), lGetUlong(jep, JB_job_number)));
            } else {
               DPRINTF(("job "sge_u32"'s job exit triggers release of array tasks in job "sge_u32"\n",
                  lGetUlong(jep, JB_job_number), lGetUlong(suc_jep, JB_job_number)));
            }
            Modified = 1;
         }
         /* cascade unlink and flush ops into a single job mod event */
         if (Modified) {
            /* flush task dependency state for empty predecessors list */
            sge_task_depend_flush(suc_jep, NULL);
            sge_add_job_event(sgeE_JOB_MOD, suc_jep, NULL);
         }
      }
   }

   DEXIT;
   return;
}

/*****************************************************************************
 Efficiently release array dependencies based on a completed task of job jep
 *****************************************************************************/
static void release_successor_tasks_ad(lListElem *jep, u_long32 task_id) 
{
   const lListElem *jid;

   DENTER(TOP_LAYER, "release_successor_tasks_ad");

   /* every successor job of this job might have tasks to be released */
   for_each(jid, lGetList(jep, JB_ja_ad_successor_list)) {
      u_long32 job_ident = lGetUlong(jid, JRE_job_number);
      lListElem *suc_range = NULL;
      lListElem *suc_jep = NULL;
      int Modified = 0;
      u_long32 bmin, bmax, sb;

      /* JA: should be doing something if this job cannot be located? */
      suc_jep = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), job_ident);
      if (!suc_jep) continue;

      /* infer reverse dependencies from the completed predecessor task to this successor */
      if (sge_task_depend_get_range(&suc_range, NULL, suc_jep, jep, task_id)) {
         lFreeElem(&suc_range);
         continue;
      }
      
      /* fetch successor job dependency range ids */
      range_get_all_ids(suc_range, &bmin, &bmax, &sb);

      /* recalculate task dependency info for this range of tasks */
      for ( ; bmin <= bmax; bmin += sb) {
         /* returns true if suc_jep was modified */
         if (sge_task_depend_update(suc_jep, NULL, bmin))
            Modified = 1;
      }
      
      /* for immediate scheduler reaction event emitting code is required */
      if (Modified)
         sge_add_job_event(sgeE_JOB_MOD, suc_jep, NULL);

      /* cleanup */
      lFreeElem(&suc_range);
   }

   DEXIT;
   return;
}

/*****************************************************************************
 Clear granted resources for job.
 job->granted_destin_identifier_list->str0 contains the granted queue
 *****************************************************************************/
/****
 **** if incslots is 1, QU_job_slots_used is decreased by the number of
 **** used slots of this job.
 **** if it is 0, QU_job_slots_used is untouched.
 ****/
static void sge_clear_granted_resources(sge_gdi_ctx_class_t *ctx,
                                        lListElem *job, lListElem *ja_task,
                                        int incslots, monitoring_t *monitor) {
   int pe_slots = 0;
   u_long32 job_id = lGetUlong(job, JB_job_number);
   u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
   lList *gdi_list = lGetList(ja_task, JAT_granted_destin_identifier_list);
   lListElem *ep;
   lList *answer_list = NULL;
   u_long32 now;
   object_description *object_base = object_type_get_object_description();
   lList *master_cqueue_list = *object_base[SGE_TYPE_CQUEUE].list;
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;
   lList *master_userset_list = *object_base[SGE_TYPE_USERSET].list;
   lList *master_hgroup_list = *object_base[SGE_TYPE_HGROUP].list;
   lList *master_exechost_list = *object_base[SGE_TYPE_EXECHOST].list;
   lList *master_rqs_list = *object_base[SGE_TYPE_RQS].list;
   lList *master_ar_list = *object_base[SGE_TYPE_AR].list;
   lListElem *rqs = NULL;
   lListElem *global_host_ep = NULL;
   bool master_task = true;
 
   DENTER(TOP_LAYER, "sge_clear_granted_resources");

   if (!job_is_ja_task_defined(job, ja_task_id) ||
       !job_is_enrolled(job, ja_task_id)) { 
      DEXIT;
      return;
   }

   now = sge_get_gmt();

   /* unsuspend queues on subordinate */
   cqueue_list_x_on_subordinate_gdil(ctx, master_cqueue_list, false, gdi_list, monitor);

   global_host_ep = host_list_locate(master_exechost_list, SGE_GLOBAL_NAME);

   /* free granted resources of the queue */
   for_each(ep, gdi_list) {
      u_long32 ar_id = lGetUlong(job, JB_ar);
      const char *queue_name = lGetString(ep, JG_qname);
      lListElem *queue = NULL;
      lListElem *ar = NULL;

      if ((queue = cqueue_list_locate_qinstance(master_cqueue_list, queue_name)) == NULL) {
         ERROR((SGE_EVENT, MSG_CONFIG_CANTFINDQUEUEXREFERENCEDINJOBY_SU,  
                   queue_name, sge_u32c(job_id)));
          master_task = false;
      } else if (ar_id != 0 && (ar = lGetElemUlong(master_ar_list, AR_id, ar_id)) == NULL) {
          ERROR((SGE_EVENT, "can't find advance reservation "sge_U32CFormat" referenced in job "sge_U32CFormat,
                 sge_u32c(ar_id), sge_u32c(job_id)));
          master_task = false;
      } else {
         const char *queue_hostname = lGetHost(queue, QU_qhostname);
         int tmp_slot = lGetUlong(ep, JG_slots);

         /* increase free slots */
         pe_slots += tmp_slot;
         if (incslots) {
            lListElem *host;

            /* undebit consumable resources */ 
            if (debit_host_consumable(job, global_host_ep, master_centry_list, -tmp_slot, master_task, NULL) > 0) {
               /* this info is not spooled */
               sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                             "global", NULL, NULL, global_host_ep);
               reporting_create_host_consumable_record(&answer_list, global_host_ep, job, now);
               answer_list_output(&answer_list);
               lListElem_clear_changed_info(global_host_ep);
            }
            host = host_list_locate(master_exechost_list, queue_hostname);
            if (debit_host_consumable(job, host, master_centry_list, -tmp_slot, master_task, NULL) > 0) {
               /* this info is not spooled */
               sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, 
                             queue_hostname, NULL, NULL, host);
               reporting_create_host_consumable_record(&answer_list, host, job, now);
               answer_list_output(&answer_list);
               lListElem_clear_changed_info(host);
            }
            qinstance_debit_consumable(queue, job, master_centry_list, -tmp_slot, master_task, NULL);
            reporting_create_queue_consumable_record(&answer_list, host, queue, job, now);
            /* this info is not spooled */
            qinstance_add_event(queue, sgeE_QINSTANCE_MOD);
            lListElem_clear_changed_info(queue);

            if (ar_id == 0) {
               /* undebit resource quota set */
               for_each(rqs, master_rqs_list) {
                  DPRINTF(("undebiting rqs %s\n", lGetString(rqs, RQS_name)));
                  if (rqs_debit_consumable(rqs, job, ep, lGetString(ja_task, JAT_granted_pe),
                                            master_centry_list, master_userset_list, master_hgroup_list,
                                            -tmp_slot, master_task) > 0) {
                     /* this info is not spooled */
                     sge_add_event(0, sgeE_RQS_MOD, 0, 0, 
                                   lGetString(rqs, RQS_name), NULL, NULL, rqs);
                     lListElem_clear_changed_info(rqs);
                  }
               }
            } else {
               /* undebit in advance reservation */
               lListElem *queue = lGetSubStr(ar, QU_full_name, lGetString(ep, JG_qname), AR_reserved_queues);
               if (qinstance_debit_consumable(queue, job, master_centry_list, -tmp_slot, master_task, NULL) > 0) {
                  dstring buffer = DSTRING_INIT;
                  /* this info is not spooled */
                  sge_dstring_sprintf(&buffer, sge_U32CFormat, ar_id);
                  sge_add_event(0, sgeE_AR_MOD, ar_id, 0, 
                                sge_dstring_get_string(&buffer), NULL, NULL, ar);
                  lListElem_clear_changed_info(ar);
                  sge_dstring_free(&buffer);
               }
            }
         }
      }
      master_task = false;
   }

   /* free granted resources of the parallel environment */
   if (lGetString(ja_task, JAT_granted_pe)) {
      if (incslots) {
         lListElem *pe = pe_list_locate(*object_base[SGE_TYPE_PE].list, 
                                        lGetString(ja_task, JAT_granted_pe));

         if (!pe) {
            ERROR((SGE_EVENT, MSG_OBJ_UNABLE2FINDPE_S,
                  lGetString(ja_task, JAT_granted_pe)));
         } else {
            pe_debit_slots(pe, -pe_slots, job_id);
            /* this info is not spooled */
            sge_add_event(0, sgeE_PE_MOD, 0, 0, 
                          lGetString(ja_task, JAT_granted_pe), NULL, NULL, pe);
            lListElem_clear_changed_info(pe);
         }
      }
      lSetString(ja_task, JAT_granted_pe, NULL);
   }

   /* forget about old tasks */
   lSetList(ja_task, JAT_task_list, NULL);
         
   /* remove granted_destin_identifier_list */
   lSetList(ja_task, JAT_granted_destin_identifier_list, NULL);
   lSetString(ja_task, JAT_master_queue, NULL);

   DRETURN_VOID;
}

/* what we do is:
      if there is a hard request for this rlimit then we replace
      the queue's value by the job request 

   no need to compare both values - this is schedd's job
*/

static void reduce_queue_limit(const lList* master_centry_list, lListElem *qep, 
                               lListElem *jep, int nm, char *rlimit_name) 
{
   const char *s = NULL;
   lListElem *res = NULL;
   object_description *object_base = object_type_get_object_description();

   DENTER(BASIS_LAYER, "reduce_queue_limit");

   res = lGetElemStr(lGetList(jep, JB_hard_resource_list), CE_name, rlimit_name);
   if ((res != NULL) && (s = lGetString(res, CE_stringval))) {
      DPRINTF(("job reduces queue limit: %s = %s (was %s)\n", rlimit_name, s, lGetString(qep, nm)));
      lSetString(qep, nm, s);
   } else { /* enforce default request if set, but only if the consumable is */
      lListElem *dcep; /*really used to manage resources of this queue, host or globally */
      if ((dcep=centry_list_locate(master_centry_list, rlimit_name))
               && lGetUlong(dcep, CE_consumable))
         if ( lGetSubStr(qep, CE_name, rlimit_name, QU_consumable_config_list) ||
              lGetSubStr(host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, lGetHost(qep, QU_qhostname)),
                     CE_name, rlimit_name, EH_consumable_config_list) ||
              lGetSubStr(host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, SGE_GLOBAL_NAME),
                     CE_name, rlimit_name, EH_consumable_config_list))

            /* managed at queue level, managed at host level or managed at global level */
            lSetString(qep, nm, lGetString(dcep, CE_default));
   }
   
   DRETURN_VOID;
}


/*-------------------------------------------------------------------------*/
/* unlink/rename the job specific files on disk, send event to scheduler   */
/*-------------------------------------------------------------------------*/
static int sge_bury_job(bool job_spooling, const char *sge_root, lListElem *job, u_long32 job_id, lListElem *ja_task, 
                        int spool_job, int no_events) 
{
   u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
   int remove_job = (!ja_task || job_get_ja_tasks(job) == 1);

   DENTER(TOP_LAYER, "sge_bury_job");

   if (job == NULL) {
      job = job_list_locate(*(object_type_get_master_list(SGE_TYPE_JOB)), job_id);
   }
   te_delete_one_time_event(TYPE_SIGNAL_RESEND_EVENT, job_id, ja_task_id, NULL);

   /*
    * Remove the job with the last task
    * or
    * Remove one ja task
    */
   if (remove_job) {
      /* we might have done this before, but to make sure, that we
         did not miss it, we do it again.... */
      release_successor_jobs(job);

      /* and this too, also flushes task dependency cache */
      release_successor_jobs_ad(job);

      /*
       * security hook
       */
      delete_credentials(sge_root, job);

      /* 
       * do not try to remove script file for interactive jobs 
       */
      if (job_spooling) {
         lList *answer_list = NULL;
         dstring buffer = DSTRING_INIT;

         spool_transaction(&answer_list, spool_get_default_context(), STC_begin);

         if ((lGetString(job, JB_exec_file) != NULL)) {
            PROF_START_MEASUREMENT(SGE_PROF_JOBSCRIPT);
            if (!JOB_TYPE_IS_BINARY(lGetUlong(job, JB_type))) {
               spool_delete_script(&answer_list, lGetUlong(job, JB_job_number), job);
            }         
            lSetString(job, JB_exec_file, NULL);
            PROF_STOP_MEASUREMENT(SGE_PROF_JOBSCRIPT);
         }

         spool_delete_object(&answer_list, spool_get_default_context(), 
                             SGE_TYPE_JOB, 
                             job_get_key(job_id, 0, NULL, &buffer),
                             job_spooling);
         answer_list_output(&answer_list);
         sge_dstring_free(&buffer);

         spool_transaction(&answer_list, spool_get_default_context(), STC_commit);
      }
      /*
       * remove the job
       */
      suser_unregister_job(job);
      if (!no_events) {
         sge_add_event( 0, sgeE_JOB_DEL, job_id, ja_task_id, 
                       NULL, NULL, lGetString(job, JB_session), NULL);
      }
      lRemoveElem(*(object_type_get_master_list(SGE_TYPE_JOB)), &job);
   } else {
      int is_enrolled = job_is_enrolled(job, ja_task_id);

      if (!no_events) {
         sge_add_event(0, sgeE_JATASK_DEL, job_id, ja_task_id, 
                       NULL, NULL, lGetString(job, JB_session), NULL);
      }

      /*
       * remove the task
       */
      if (is_enrolled) {
         lList *answer_list = NULL;
         dstring buffer = DSTRING_INIT;
         spool_delete_object(&answer_list, spool_get_default_context(), 
                             SGE_TYPE_JOB, 
                             job_get_key(job_id, ja_task_id, NULL, &buffer),
                             job_spooling);
         answer_list_output(&answer_list);
         sge_dstring_free(&buffer);
         lRemoveElem(lGetList(job, JB_ja_tasks), &ja_task);
      } else {
         job_delete_not_enrolled_ja_task(job, NULL, ja_task_id);
         if (spool_job) {
            lList *answer_list = NULL;
            dstring buffer = DSTRING_INIT;
            spool_write_object(&answer_list, spool_get_default_context(), job, 
                               job_get_key(job_id, ja_task_id, NULL, &buffer), 
                               SGE_TYPE_JOB, job_spooling);
            answer_list_output(&answer_list);
            sge_dstring_free(&buffer);
         }
      }

      /* release the successor tasks. this might have been done before
         but to make sure, we do it again. we need to do this last to
         make sure the task is gone when dependencies are calculated */
      release_successor_tasks_ad(job, ja_task_id);
   }

   DEXIT;
   return True;
}

static int sge_to_zombies(lListElem *job, lListElem *ja_task) 
{
   u_long32 ja_task_id = lGetUlong(ja_task, JAT_task_number);
   u_long32 job_id = lGetUlong(job, JB_job_number);
   int is_defined;

   DENTER(TOP_LAYER, "sge_to_zombies");

   is_defined = job_is_ja_task_defined(job, ja_task_id); 
   if (is_defined) {
      lList **master_zombie_list = object_type_get_master_list(SGE_TYPE_ZOMBIE);

      lListElem *zombie = job_list_locate(*master_zombie_list, job_id);

      /*
       * Create zombie job list if it does not exist
       */
      if (*master_zombie_list == NULL) {
         *master_zombie_list = lCreateList("Master_Zombie_List", JB_Type);
      }

      /*
       * Create zombie job if it does not exist 
       * (don't copy unnecessary sublists)
       */
      if (zombie == NULL) {
         lList *n_h_ids = NULL;     /* RN_Type */ 
         lList *u_h_ids = NULL;     /* RN_Type */ 
         lList *o_h_ids = NULL;     /* RN_Type */ 
         lList *s_h_ids = NULL;     /* RN_Type */ 
         lList *a_h_ids = NULL;     /* RN_Type */ 
         lList *ja_tasks = NULL;    /* JAT_Type */ 

         lXchgList(job, JB_ja_n_h_ids, &n_h_ids);
         lXchgList(job, JB_ja_u_h_ids, &u_h_ids);
         lXchgList(job, JB_ja_o_h_ids, &o_h_ids);
         lXchgList(job, JB_ja_s_h_ids, &s_h_ids);
         lXchgList(job, JB_ja_a_h_ids, &a_h_ids);
         lXchgList(job, JB_ja_tasks, &ja_tasks);
         zombie = lCopyElem(job);
         lXchgList(job, JB_ja_n_h_ids, &n_h_ids);
         lXchgList(job, JB_ja_u_h_ids, &u_h_ids);
         lXchgList(job, JB_ja_o_h_ids, &o_h_ids);
         lXchgList(job, JB_ja_s_h_ids, &s_h_ids);
         lXchgList(job, JB_ja_a_h_ids, &a_h_ids);
         lXchgList(job, JB_ja_tasks, &ja_tasks);
         lAppendElem(*master_zombie_list, zombie);
      }

      /*
       * Add the zombie task id
       */
      if (zombie) {
         job_add_as_zombie(zombie, NULL, ja_task_id); 
      } 

   } else {
      WARNING((SGE_EVENT, "It is impossible to move task "sge_u32" of job "sge_u32
               " to the list of finished jobs\n", ja_task_id, job_id)); 
   }

   DEXIT;
   return True;
}


/****** sge_give_jobs/copyJob() ************************************************
*  NAME
*     copyJob() -- copy the job with only the specified ja task
*
*  SYNOPSIS
*     static lListElem* copyJob(lListElem *job, lListElem *ja_task) 
*
*  FUNCTION
*     copy the job with only the specified ja task
*
*  INPUTS
*     lListElem *job     - job structure
*     lListElem *ja_task - ja-task (template)
*
*  RESULT
*     static lListElem* -  NULL, or new job structure
*
*  NOTES
*     MT-NOTE: copyJob() is MT safe 
*
*******************************************************************************/
static lListElem* 
copyJob(lListElem *job, lListElem *ja_task)
{
   lListElem *job_copy = NULL;
   lListElem *ja_task_copy = NULL;
   lList * tmp_ja_task_list = NULL;
   
   DENTER(TOP_LAYER, "copyJob");

   /* create a copy of the job */
   lXchgList(job, JB_ja_tasks, &tmp_ja_task_list);
   job_copy = lCopyElem(job);
   lXchgList(job, JB_ja_tasks, &tmp_ja_task_list);

   if (job_copy != NULL) { /* not enough memory */
  
      ja_task_copy = lGetElemUlong(lGetList(job, JB_ja_tasks), JAT_task_number,
                                   lGetUlong(ja_task, JAT_task_number));

      tmp_ja_task_list = lCreateList("cp_ja_task_list", lGetElemDescr(ja_task_copy));
      lAppendElem(tmp_ja_task_list, lCopyElem(ja_task_copy));
      lSetList(job_copy, JB_ja_tasks, tmp_ja_task_list);

      if (lGetNumberOfElem(tmp_ja_task_list) == 0) { /* not enough memory */
         lFreeElem(&job_copy); /* this also frees tmp_ja_task_list */
      }
   }

   DRETURN(job_copy);
}


/****** sge_give_jobs/setCheckpointObj() ***************************************
*  NAME
*     setCheckpointObj() -- sets the job checkpointing name
*
*  SYNOPSIS
*     static int setCheckpointObj(lListElem *job) 
*
*  FUNCTION
*     sets the job checkpointing name
*
*  INPUTS
*     lListElem *job - job to modify
*
*  RESULT
*     static int -  0 : everything went find
*                  -1 : something went wrong
*
*  NOTES
*     MT-NOTE: setCheckpointObj() is not MT safe 
*
*******************************************************************************/
static int 
setCheckpointObj(lListElem *job) 
{
   lListElem *ckpt = NULL; 
   lListElem *tmp_ckpt = NULL;
   const char *ckpt_name = NULL;
   int ret = 0; 
   
   DENTER(TOP_LAYER, "setCheckpointObj");

   if ((ckpt_name=lGetString(job, JB_checkpoint_name))) {
      ckpt = ckpt_list_locate(*object_type_get_master_list(SGE_TYPE_CKPT), ckpt_name);
      if (!ckpt) {
         ERROR((SGE_EVENT, MSG_OBJ_UNABLE2FINDCKPT_S, ckpt_name));
         ret =  -1;
      }

      /* get the necessary memory */
      else if (!(tmp_ckpt=lCopyElem(ckpt))) {
         ERROR((SGE_EVENT, MSG_OBJ_UNABLE2CREATECKPT_SU, 
                ckpt_name, sge_u32c(lGetUlong(job, JB_job_number))));
         ret  = -1;
      }
      else {
      /* everything's in place. stuff it in */
         lSetObject(job, JB_checkpoint_object, tmp_ckpt);
      }
   }
   DEXIT;
   return ret;
}

bool gdil_del_all_orphaned(sge_gdi_ctx_class_t *ctx, const lList *gdil_list, lList **alpp)
{
   bool ret = true;
   lListElem *gdil_ep;
   
   dstring cqueue_name = DSTRING_INIT;
   dstring host_name = DSTRING_INIT;
   bool has_hostname, has_domain;
   for_each (gdil_ep, gdil_list) {
      cqueue_name_split(lGetString(gdil_ep, JG_qname), &cqueue_name, &host_name, &has_hostname, &has_domain);
      ret &= cqueue_list_del_all_orphaned(ctx, *(object_type_get_master_list(SGE_TYPE_CQUEUE)), alpp, 
         sge_dstring_get_string(&cqueue_name), sge_dstring_get_string(&host_name));
   }
   sge_dstring_free(&cqueue_name);
   sge_dstring_free(&host_name);

   return ret;
}

