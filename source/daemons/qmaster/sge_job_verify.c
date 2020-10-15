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
 *  Copyright: 2008 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <errno.h>

#include "sge.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_monitor.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_stdio.h"
#include "uti/sge_time.h"
#include "uti/sge_binding_hlp.h"
#include "uti/sge_mtutil.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"
#include "gdi/sge_security.h"

#include "spool/sge_spooling.h"

#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_suser.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_binding.h"
#include "sgeobj/sge_conf.h"

#include "sched/valid_queue_user.h"

#include "sge_userprj_qmaster.h"
#include "sge_userset_qmaster.h"
#include "sge_job_qmaster.h"
#include "symbols.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "msg_daemons_common.h"

static bool check_binding_param_consistency(lListElem* binding_elem);

int
sge_job_verify_adjust(sge_gdi_ctx_class_t *ctx, lListElem *jep, lList **alpp, 
                      lList **lpp, char *ruser, char *rhost, uid_t uid, gid_t gid, char *group, 
                      sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task,
                      monitoring_t *monitor)
{
   object_description *object_base = object_type_get_object_description();
   int ret = STATUS_OK;

   DENTER(TOP_LAYER, "sge_job_verify_adjust");

   if (jep == NULL || ruser == NULL || rhost == NULL ) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      ret = STATUS_EUNKNOWN;
   }

   /* check min_uid */
   if (ret == STATUS_OK) {
      if (uid < mconf_get_min_uid()) {
         ERROR((SGE_EVENT, MSG_JOB_UID2LOW_II, (int)uid, (int)mconf_get_min_uid()));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   /* check min_gid */
   if (ret == STATUS_OK) {
      if (gid < mconf_get_min_gid()) {
         ERROR((SGE_EVENT, MSG_JOB_GID2LOW_II, (int)gid, (int)mconf_get_min_gid()));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   /* 
    * adjust user and group    
    *
    * we cannot rely on the information we got from the client
    * therefore we fill in the data we got from communication
    * library.
    */
   if (ret == STATUS_OK) {
      if (!job_set_owner_and_group(jep, uid, gid, ruser, group)) {
         ret = STATUS_EUNKNOWN;
      }
   }

   /* check for non-parallel job that define a master queue */
   if (ret == STATUS_OK) {
      if (lGetList(jep, JB_master_hard_queue_list) != NULL && lGetString(jep, JB_pe) == NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_JOB_MQNONPE));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   /* check for qsh without DISPLAY set */
   if (ret == STATUS_OK) {
      if (JOB_TYPE_IS_QSH(lGetUlong(jep, JB_type))) {
         ret = job_check_qsh_display(jep, alpp, false);
      }   
   }

   /*
    * verify and adjust array job ids:
    *    JB_ja_structure, JB_ja_n_h_ids, JB_ja_u_h_ids, 
    *    JB_ja_s_h_ids, JB_ja_o_h_ids, JB_ja_a_h_ids, JB_ja_z_ids
    */
   if (ret == STATUS_OK) {
      job_check_correct_id_sublists(jep, alpp);
      if (answer_list_has_error(alpp)) {
         ret = STATUS_EUNKNOWN;
      }
   }  

   /*
    * resolve host names contained in path names
    */ 
   if (ret == STATUS_OK) {
      int s1, s2, s3, s4;
      
      s1 = job_resolve_host_for_path_list(jep, alpp, JB_stdout_path_list);
      s2 = job_resolve_host_for_path_list(jep, alpp, JB_stdin_path_list);
      s3 = job_resolve_host_for_path_list(jep, alpp,JB_shell_list);
      s4 = job_resolve_host_for_path_list(jep, alpp, JB_stderr_path_list);
      if (s1 != STATUS_OK || s2 != STATUS_OK || s3 != STATUS_OK || s4 != STATUS_OK) {
         ret = STATUS_EUNKNOWN;
      }
   }

   /* take care that non-binary jobs have a script */
   if (ret == STATUS_OK) {
      if ((!JOB_TYPE_IS_BINARY(lGetUlong(jep, JB_type)) &&
          !lGetString(jep, JB_script_ptr) && lGetString(jep, JB_script_file))) {
         ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NOSCRIPT));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = STATUS_EUNKNOWN;
      }
   }

   /* set the jobs submittion time */
   if (ret == STATUS_OK) {
      lSetUlong(jep, JB_submission_time, sge_get_gmt());
   }

   /* initialize the task template element and other sublists */
   if (ret == STATUS_OK) {
      lSetList(jep, JB_ja_tasks, NULL);
      lSetList(jep, JB_jid_successor_list, NULL);
      lSetList(jep, JB_ja_ad_successor_list, NULL);
      if (lGetList(jep, JB_ja_template) == NULL) {
         lAddSubUlong(jep, JAT_task_number, 0, JB_ja_template, JAT_Type);
      }
   } 

   if (ret == STATUS_OK) {
      lListElem *binding_elem = lFirst(lGetList(jep, JB_binding));
               
      if (binding_elem == NULL) {
         bool lret = job_init_binding_elem(jep);

         if (lret == false) {
            ret = STATUS_EUNKNOWN;
         }
      } else {
         /* verify if binding parameters are consistent (bugster 6903956) */ 
         if (check_binding_param_consistency(binding_elem) == false) {
            /* TODO add to answer list */
            answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
            ret = STATUS_EUNKNOWN;
         } 
      }
   }

   /* verify or set the account string */
   if (ret == STATUS_OK) {
      if (!lGetString(jep, JB_account)) {
         lSetString(jep, JB_account, DEFAULT_ACCOUNT);
      } else {
         if (verify_str_key(alpp, lGetString(jep, JB_account), MAX_VERIFY_STRING,
                            "account string", QSUB_TABLE) != STATUS_OK) { 
            ret = STATUS_EUNKNOWN;
         }
      }
   } 

   /* verify the job name */
   if (ret == STATUS_OK) {
      if (object_verify_name(jep, alpp, JB_job_name, SGE_OBJ_JOB)) {
         ret = STATUS_EUNKNOWN;
      }     
   }

   /* is the max. size of array jobs exceeded? */
   if (ret == STATUS_OK) {
      u_long32 max_aj_tasks = mconf_get_max_aj_tasks();

      if (max_aj_tasks > 0) {
         lList *range_list = lGetList(jep, JB_ja_structure);
         u_long32 submit_size = range_list_get_number_of_ids(range_list);
      
         if (submit_size > max_aj_tasks) {
            ERROR((SGE_EVENT, MSG_JOB_MORETASKSTHAN_U, sge_u32c(max_aj_tasks)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = STATUS_EUNKNOWN;
         }
      }
   }

   /* 
    * JB_context contains a raw context list, which needs to be transformed into
    * a real context. For that, we have to take out the raw context and add it back
    * again, processed. 
    */
   if (ret == STATUS_OK) {
      lList* temp = NULL;

      lXchgList(jep, JB_context, &temp);
      set_context(temp, jep);
      lFreeList(&temp);
   } 

   /*
    * Following block should only be executed once, when the job has no job id.
    *
    * At first  we try to find a job id which is not yet used. AFTER that we need
    * to set the submission time. Only this makes wure that forced separation 
    * in time is effective in case of job ID rollover.
    */
   /* ORDER IS IMPORTANT */
   if (lGetUlong(jep, JB_job_number) == 0) {
      u_long32 jid;

      do {
         jid = sge_get_job_number(ctx, monitor);
      } while (job_list_locate(*object_base[SGE_TYPE_JOB].list, jid));      
      lSetUlong(jep, JB_job_number, jid);
      lSetUlong(jep, JB_submission_time, sge_get_gmt());
   }

   /*      
    * with interactive jobs, JB_exec_file is not set
    */      
   if (lGetString(jep, JB_script_file)) {
      dstring string = DSTRING_INIT;
      sge_dstring_sprintf(&string, "%s/%d", EXEC_DIR, (int)lGetUlong(jep, JB_job_number));
      lSetString(jep, JB_exec_file, sge_dstring_get_string(&string));
      sge_dstring_free(&string);
   }

   /* check max_jobs */
   if (job_list_register_new_job(*object_base[SGE_TYPE_JOB].list, mconf_get_max_jobs(), 0)) {
      INFO((SGE_EVENT, MSG_JOB_ALLOWEDJOBSPERCLUSTER, sge_u32c(mconf_get_max_jobs())));
      answer_list_add(alpp, SGE_EVENT, STATUS_NOTOK_DOAGAIN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_NOTOK_DOAGAIN);      
   }

   if (lGetUlong(jep, JB_verify_suitable_queues) != JUST_VERIFY &&
       lGetUlong(jep, JB_verify_suitable_queues) != POKE_VERIFY) {
      if (suser_check_new_job(jep, mconf_get_max_u_jobs()) != 0) { 
         INFO((SGE_EVENT, MSG_JOB_ALLOWEDJOBSPERUSER_UU, sge_u32c(mconf_get_max_u_jobs()),
                                                         sge_u32c(suser_job_count(jep))));
         answer_list_add(alpp, SGE_EVENT, STATUS_NOTOK_DOAGAIN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_NOTOK_DOAGAIN);
      }
   }

   {
      lList *user_lists = mconf_get_user_lists();
      lList *xuser_lists = mconf_get_xuser_lists();

      if (!sge_has_access_(ruser, lGetString(jep, JB_group), /* read */
            user_lists, xuser_lists, *object_base[SGE_TYPE_USERSET].list)) {
         ERROR((SGE_EVENT, MSG_JOB_NOPERMS_SS, ruser, rhost));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         lFreeList(&user_lists);
         lFreeList(&xuser_lists);
         DRETURN(STATUS_EUNKNOWN);
      }
      lFreeList(&user_lists);
      lFreeList(&xuser_lists);
   }

   /* 
    * fill name and shortcut for all requests
    * fill numeric values for all bool, time, memory and int type requests
    * use the master_CEntry_list for all fills
    * JB_hard/soft_resource_list points to a CE_Type list
    */
   {
      lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;

      if (centry_list_fill_request(lGetList(jep, JB_hard_resource_list),
                                   alpp, master_centry_list, false, true,
                                   false)) {
         DRETURN(STATUS_EUNKNOWN);
      }
      if (compress_ressources(alpp, lGetList(jep, JB_hard_resource_list), SGE_OBJ_JOB)) {
         DRETURN(STATUS_EUNKNOWN);
      }

      if (centry_list_fill_request(lGetList(jep, JB_soft_resource_list),
                                   alpp, master_centry_list, false, true,
                                   false)) {
         DRETURN(STATUS_EUNKNOWN);
      }
      if (compress_ressources(alpp, lGetList(jep, JB_soft_resource_list), SGE_OBJ_JOB)) {
         DRETURN(STATUS_EUNKNOWN);
      }
      if (deny_soft_consumables(alpp, lGetList(jep, JB_soft_resource_list), master_centry_list)) {
         DRETURN(STATUS_EUNKNOWN);
      }
      if (!centry_list_is_correct(lGetList(jep, JB_hard_resource_list), alpp)) {
         DRETURN(STATUS_EUNKNOWN);
      }
      if (!centry_list_is_correct(lGetList(jep, JB_soft_resource_list), alpp)) {
         DRETURN(STATUS_EUNKNOWN);
      }
   }

   if (!qref_list_is_valid(lGetList(jep, JB_hard_queue_list), alpp)) {
      DRETURN(STATUS_EUNKNOWN);
   }
   if (!qref_list_is_valid(lGetList(jep, JB_soft_queue_list), alpp)) {
      DRETURN(STATUS_EUNKNOWN);
   }
   if (!qref_list_is_valid(lGetList(jep, JB_master_hard_queue_list), alpp)) {
      DRETURN(STATUS_EUNKNOWN);
   }

   /* 
    * here we test (if requested) the parallel environment exists.
    * if not the job is refused
    */
   {
      const char *pe_name = NULL;
      lList *pe_range = NULL;

      pe_name = lGetString(jep, JB_pe);
      if (pe_name) {
         const lListElem *pep;

         pep = pe_list_find_matching(*object_base[SGE_TYPE_PE].list, pe_name);
         if (!pep) {
            ERROR((SGE_EVENT, MSG_JOB_PEUNKNOWN_S, pe_name));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(STATUS_EUNKNOWN);
         }
         /* check pe_range */
         pe_range = lGetList(jep, JB_pe_range);
         if (object_verify_pe_range(alpp, pe_name, pe_range, SGE_OBJ_JOB)!=STATUS_OK) {
            DRETURN(STATUS_EUNKNOWN);
         }

#ifdef SGE_PQS_API
#if 0
         /* verify PE qsort_args */
         if ((qsort_args=lGetString(pep, PE_qsort_argv)) != NULL) {
            sge_assignment_t a = SGE_ASSIGNMENT_INIT;
            int ret;

            a.job = jep;
            a.job_id = 
            a.ja_task_id =
            a.slots = 
            ret = sge_call_pe_qsort(&a, qsort_args, 1, err_str);
            if (!ret) {
               answer_list_add(alpp, err_str, STATUS_EUNKNOWN,
                               ANSWER_QUALITY_ERROR);
               DRETURN(STATUS_EUNKNOWN);
            }
         }
#endif
#endif
      }
   }

   {
      u_long32 ckpt_attr = lGetUlong(jep, JB_checkpoint_attr);
      u_long32 ckpt_inter = lGetUlong(jep, JB_checkpoint_interval);
      const char *ckpt_name = lGetString(jep, JB_checkpoint_name);
      lListElem *ckpt_ep;
      int ckpt_err = 0;

      /* request for non existing ckpt object will be refused */
      if ((ckpt_name != NULL)) {
         if (!(ckpt_ep = ckpt_list_locate(*object_base[SGE_TYPE_CKPT].list, ckpt_name)))
            ckpt_err = 1;
         else if (!ckpt_attr) {
            ckpt_attr = sge_parse_checkpoint_attr(lGetString(ckpt_ep, CK_when));
            lSetUlong(jep, JB_checkpoint_attr, ckpt_attr);
         }
      }

      if (!ckpt_err) {
         if ((ckpt_attr & NO_CHECKPOINT) && (ckpt_attr & ~NO_CHECKPOINT)) {
            ckpt_err = 2;
         }
         else if (ckpt_name && (ckpt_attr & NO_CHECKPOINT)) {
            ckpt_err = 3;
         }
         else if ((!ckpt_name && (ckpt_attr & ~NO_CHECKPOINT))) {
            ckpt_err = 4;
         }
         else if (!ckpt_name && ckpt_inter) {
            ckpt_err = 5;
         }
      }

      if (ckpt_err) {
         switch (ckpt_err) {
         case 1:
            ERROR((SGE_EVENT, MSG_JOB_CKPTUNKNOWN_S, ckpt_name));
          break;
         case 2:
         case 3:
            ERROR((SGE_EVENT, "%s", MSG_JOB_CKPTMINUSC));
            break;
         case 4:
         case 5:
            ERROR((SGE_EVENT, "%s", MSG_JOB_CKPTDENIED));
          break;
         default:
            ERROR((SGE_EVENT, "%s", MSG_JOB_CKPTDENIED));
            break;
         }
         answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_ESEMANTIC);
      }
   }

   /* first check user permissions */
   {
      lListElem *cqueue = NULL;
      int has_permissions = 0;

      for_each (cqueue, *object_base[SGE_TYPE_CQUEUE].list) {
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
         lListElem *qinstance = NULL;
         lList *master_userset_list = *object_base[SGE_TYPE_USERSET].list;

         for_each(qinstance, qinstance_list) {
            if (sge_has_access(ruser, lGetString(jep, JB_group),
                  qinstance, master_userset_list)) {
               DPRINTF(("job has access to queue "SFQ"\n", lGetString(qinstance, QU_qname)));
               has_permissions = 1;
               break;
            }
         }
         if (has_permissions == 1) {
            break;
         }
      }
      if (has_permissions == 0) {
         SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_JOB_NOTINANYQ_S, ruser));
         answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      }
   }

   /* if enforce_user flag is "auto", add or update the user */
   {
      char* enforce_user = mconf_get_enforce_user();

      if (enforce_user && !strcasecmp(enforce_user, "auto")) {
         int status = sge_add_auto_user(ctx, ruser, alpp, monitor);

         if (status != STATUS_OK) {
            sge_free(&enforce_user);
            DRETURN(status);
         }
      }

      /* ensure user exists if enforce_user flag is set */
      if (enforce_user && !strcasecmp(enforce_user, "true") &&
               !user_list_locate(*object_base[SGE_TYPE_USER].list, ruser)) {
         ERROR((SGE_EVENT, MSG_JOB_USRUNKNOWN_S, ruser));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         sge_free(&enforce_user);
         DRETURN(STATUS_EUNKNOWN);
      }
      sge_free(&enforce_user);
   }

   /* set default project */
   if (!lGetString(jep, JB_project) && ruser && *object_base[SGE_TYPE_USER].list) {
      lListElem *uep = NULL;
      if ((uep = user_list_locate(*object_base[SGE_TYPE_USER].list, ruser))) {
         lSetString(jep, JB_project, lGetString(uep, UU_default_project));
      }
   }

   /* project */
   {
      int ret = job_verify_project(jep, alpp, ruser, group);
      if (ret != STATUS_OK) {
         DRETURN(ret);
      }
   }

   /* try to dispatch a department to the job */
   if (set_department(alpp, jep, *object_base[SGE_TYPE_USERSET].list) != 1) {
      /* alpp gets filled by set_department */
      DRETURN(STATUS_EUNKNOWN);
   }

   /* 
    * If it is a deadline job the user has to be a deadline user
    */
   if (lGetUlong(jep, JB_deadline)) {
      if (!userset_is_deadline_user(*object_base[SGE_TYPE_USERSET].list, ruser)) {
         ERROR((SGE_EVENT, MSG_JOB_NODEADLINEUSER_S, ruser));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_EUNKNOWN);
      }
   }

   /* Verify existence of ar, if ar exists */
   {
      u_long32 ar_id = lGetUlong(jep, JB_ar);

      if (ar_id != 0) {
         lListElem *ar;
         u_long32 ar_start_time, ar_end_time, job_execution_time, job_duration, now_time; 

         DPRINTF(("job -ar "sge_u32"\n", sge_u32c(ar_id)));

         ar=ar_list_locate(*object_base[SGE_TYPE_AR].list, ar_id);
         if (ar == NULL) {
            ERROR((SGE_EVENT, MSG_JOB_NOAREXISTS_U, sge_u32c(ar_id)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(STATUS_EEXIST);
         } else if ((lGetUlong(ar, AR_state) == AR_DELETED) ||
                    (lGetUlong(ar, AR_state) == AR_EXITED)) {
            ERROR((SGE_EVENT, MSG_JOB_ARNOLONGERAVAILABE_U, sge_u32c(ar_id)));
            answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(STATUS_EEXIST);
         }
         /* fill the job and ar values */         
         ar_start_time = lGetUlong(ar, AR_start_time);
         ar_end_time = lGetUlong(ar, AR_end_time);
         now_time = sge_get_gmt();
         job_execution_time = lGetUlong(jep, JB_execution_time);

         /* execution before now is set to at least now */
         if (job_execution_time < now_time) {
            job_execution_time = now_time;
         }

         /* to be sure the execution time is NOT before AR start time */
         if (job_execution_time < ar_start_time) {
            job_execution_time = ar_start_time;
         }

         /* hard_resources h_rt limit */
         if (job_get_wallclock_limit(&job_duration, jep) == true) {
            DPRINTF(("job -ar "sge_u32", ar_start_time "sge_u32", ar_end_time "sge_u32
                     ", job_execution_time "sge_u32", job duration "sge_u32" \n",
                     sge_u32c(ar_id),sge_u32c( ar_start_time),sge_u32c(ar_end_time),
                     sge_u32c(job_execution_time),sge_u32c(job_duration)));

            /* fit the timeframe */
            if (job_duration > (ar_end_time - ar_start_time)) {
               ERROR((SGE_EVENT, MSG_JOB_HRTLIMITTOOLONG_U, sge_u32c(ar_id)));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               DRETURN(STATUS_DENIED);
            }
            if ((job_execution_time + job_duration) > ar_end_time) {
               ERROR((SGE_EVENT, MSG_JOB_HRTLIMITOVEREND_U, sge_u32c(ar_id)));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               DRETURN(STATUS_DENIED);
            }
         }
      }
   }

   /* verify schedulability */
   {
      int ret = verify_suitable_queues(alpp, jep, NULL, false);
      if (lGetUlong(jep, JB_verify_suitable_queues) == JUST_VERIFY  ||
          lGetUlong(jep, JB_verify_suitable_queues) == POKE_VERIFY || ret != 0) {
         DRETURN(ret);
      }
   }

   /*
    * only operators and managers are allowed to submit
    * jobs with higher priority than 0 (=BASE_PRIORITY)
    */
   if (lGetUlong(jep, JB_priority) > BASE_PRIORITY && !manop_is_operator(ruser)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_JOB_NONADMINPRIO));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(STATUS_EUNKNOWN);
   }

   /* checks on -hold_jid */
   if (job_verify_predecessors(jep, alpp)) {
      DRETURN(STATUS_EUNKNOWN);
   }

   /* checks on -hold_jid_ad */
   if (job_verify_predecessors_ad(jep, alpp)) {
      DRETURN(STATUS_EUNKNOWN);
   }

   /*
   ** security hook
   **
   ** Execute command to store the client's DCE or Kerberos credentials.
   ** This also creates a forwardable credential for the user.
   */
   if (mconf_get_do_credentials()) {
      const char *sge_root = ctx->get_sge_root(ctx);

      if (store_sec_cred(sge_root, packet, jep, mconf_get_do_authentication(), alpp, ctx) != 0) {
         DRETURN(STATUS_EUNKNOWN);
      }
   }

   job_suc_pre(jep);

   job_suc_pre_ad(jep);

   DRETURN(ret);
}

             
/****** sge_job_verify/check_binding_param_consistency() ***********************
*  NAME
*     check_binding_param_consistency() -- Semantic check of JSV binding parameter change.
*
*  SYNOPSIS
*     static bool check_binding_param_consistency(lListElem* binding_elem) 
*
*  FUNCTION
*     Checks the JSV parameter change of the binding parater for semantical 
*     correctness. In case there is some missconfiguration found in the 
*     binding CULL sublist an error is thrown and the job is rejected. 
* 
*     This function is introduced in order to fix bugster 6903956
*
*  INPUTS
*     lListElem* binding_elem - CULL sublist for core binding 
*
*  RESULT
*     static bool - true if semantic is correct - false if not
*
*  NOTES
*     MT-NOTE: check_binding_param_consistency() is MT safe 
*
*******************************************************************************/
static bool check_binding_param_consistency(lListElem* binding_elem)
{
   const char* strategy;
   
   DENTER(TOP_LAYER, "check_binding_param_consistency");

   if (binding_elem == NULL) {
      DRETURN(false);
   }

   if ((strategy = lGetString(binding_elem, BN_strategy)) == NULL) {
      /* no binding strategy set */
      ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_strategy", "NULL"));
      DRETURN(false);
   }

   /* check according strategy linear */
   if ((strcmp(strategy, "linear") == 0)
         || (strcmp(strategy, "linear_automatic") == 0)) {
      
      /* amount must be > 0 */
      if (lGetUlong(binding_elem, BN_parameter_n) == 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter", "n"));
         DRETURN(false);
      }

      /* socket and core values are implicitly 0 */
      
      /* check if step_size > 0 */
      if (lGetUlong(binding_elem, BN_parameter_striding_step_size) > 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_striding_step_size", "> 0"));
         DRETURN(false);
      }

      /* check if explicit value is set */
      if (lGetString(binding_elem, BN_parameter_explicit) != NULL) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_explicit", "!= NULL"));
         DRETURN(false);
      }

      /* in case of linear_automatic the core and socket number 
         must not be > 0 */
      if (strcmp(strategy, "linear_automatic") == 0) {
         if (lGetUlong(binding_elem, BN_parameter_socket_offset) > 0) {
            ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_socket_offset", "> 0"));
            DRETURN(false);
         }
         if (lGetUlong(binding_elem, BN_parameter_core_offset) > 0) {
            ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_core_offset", "> 0"));
            DRETURN(false);
         }
      }
   }
   
   /* check according strategy striding */
   if ((strcmp(strategy, "striding") == 0)
         || (strcmp(strategy, "striding_automatic") == 0)) {

      /* the number of cores requested must be > 0 */
      if (lGetUlong(binding_elem, BN_parameter_n) == 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_n", "0"));
         DRETURN(false);
      }
      
      /* socket and core values are implicitly 0 */

      /* check explicit socket core list */
      if (lGetString(binding_elem, BN_parameter_explicit) != NULL) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_explicit", "!= NULL"));
         DRETURN(false);
      }

   }

   /* check according strategy explicit */
   if (strcmp(strategy, "explicit") == 0) {
      const char* expl;
      int amount;

      /* the explicit parameter must be set */
      if (lGetString(binding_elem, BN_parameter_explicit) == NULL) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_explicit", "== NULL"));
         DRETURN(false);
      }
      
      /* amount makes no sense */
      if (lGetUlong(binding_elem, BN_parameter_n) > 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_n", "> 0"));
         DRETURN(false);
      }

      /* step size makes no sense */
      if (lGetUlong(binding_elem, BN_parameter_striding_step_size) > 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_striding_step_size", "> 0"));
         DRETURN(false);
      }
     
      /* check if socket offset was set */
      if (lGetUlong(binding_elem, BN_parameter_socket_offset) > 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_socket_offset", "> 0"));
         DRETURN(false);
      }

      /* check if core offset was set */
      if (lGetUlong(binding_elem, BN_parameter_core_offset) > 0) {
         ERROR((SGE_EVENT, MSG_JSV_BINDING_REJECTED_SS, "BN_parameter_core_offset", "> 0"));
         DRETURN(false);
      }

      expl = lGetString(binding_elem, BN_parameter_explicit);
      amount = get_explicit_number(expl, false);

      if (check_explicit_binding_string(expl, amount, false) == false) {
         DRETURN(false);
      }

   }

   return true;
}

