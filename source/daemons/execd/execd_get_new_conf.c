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

#include <string.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_pe_task.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_ja_task.h"
#include "exec_job.h"

#include "dispatcher.h"
#include "execd_get_new_conf.h"
#include "sge_load_sensor.h"
#include "admin_mail.h"
#include "load_avg.h"
#include "msg_common.h"

#ifdef COMPILE_DC
#  include "ptf.h"
#endif

/*
** DESCRIPTION
**   retrieves new configuration from qmaster, very similar to what is
**   executed on startup. This function is triggered by the execd
**   dispatcher table when the tag TAG_GET_NEW_CONF is received.
*/
int do_get_new_conf(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg)
{
   int ret;
   bool use_qidle = mconf_get_use_qidle();
   u_long32 dummy; /* always 0 */ 
   char* old_spool = NULL;
   char* spool_dir = NULL;
   int old_reprioritization_enabled = mconf_get_reprioritize(); 

   DENTER(TOP_LAYER, "do_get_new_conf");

   unpackint(&(aMsg->buf), &dummy);

   old_spool = mconf_get_execd_spool_dir();  

   ret = gdi2_get_merged_configuration(ctx, &Execd_Config_List);
  
   spool_dir = mconf_get_execd_spool_dir(); 
   if (strcmp(old_spool, spool_dir)) {
      WARNING((SGE_EVENT, MSG_WARN_CHANGENOTEFFECTEDUNTILRESTARTOFEXECHOSTS, "execd_spool_dir"));
   }

#ifdef COMPILE_DC
   if (old_reprioritization_enabled != mconf_get_reprioritize()) {
      /* Here we will make sure that each job which was started
         in SGEEE-Mode (reprioritization) will get its initial
         queue priority if this execd alternates to SGE-Mode */
      lListElem *job, *jatask, *petask;

      errno = 0;
      if (sge_switch2start_user()) {
         CRITICAL((SGE_EVENT, MSG_SWITCH_USER_S, strerror(errno)));
      }

      for_each(job, *(object_type_get_master_list(SGE_TYPE_JOB))) {
         lListElem *master_queue;

         for_each (jatask, lGetList(job, JB_ja_tasks)) {
            int priority;

            master_queue = responsible_queue(jatask, NULL);
            priority = atoi(lGetString(master_queue, QU_priority));

            DPRINTF(("Set priority of job "sge_u32"."sge_u32" running in"
               " queue  %s to %d\n", 
            lGetUlong(job, JB_job_number), 
            lGetUlong(jatask, JAT_task_number),
            lGetString(master_queue, QU_full_name), priority));
            ptf_reinit_queue_priority(lGetUlong(job, JB_job_number),
                                      lGetUlong(jatask, JAT_task_number),
                                      NULL, priority);

            for_each(petask, lGetList(jatask, JAT_task_list)) {
               master_queue = responsible_queue(jatask, petask);
               priority = atoi(lGetString(master_queue, QU_priority));
               DPRINTF(("Set priority of task "sge_u32"."sge_u32"-%s running "
                        "in queue %s to %d\n", 
               lGetUlong(job, JB_job_number), 
               lGetUlong(jatask, JAT_task_number),
               lGetString(petask, PET_id),
               lGetString(master_queue, QU_full_name), priority));
               ptf_reinit_queue_priority(lGetUlong(job, JB_job_number),
                                         lGetUlong(jatask, JAT_task_number),
                                          lGetString(petask, PET_id),
                                          priority);
            }
         }
      }
      sge_switch2admin_user();
   }
#endif

   /*
   ** admin mail block is released on new conf
   */
   adm_mail_reset(BIT_ADM_NEW_CONF);

   sge_ls_qidle(use_qidle);
   DPRINTF(("use_qidle: %d\n", use_qidle));

   sge_free(&old_spool);
   sge_free(&spool_dir);

   DRETURN(ret);
}


