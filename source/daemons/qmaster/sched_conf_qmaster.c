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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/setup_path.h"

#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_conf.h"

#include "sched/sge_sched.h"

#include "spool/sge_spooling.h"

#include "sched_conf_qmaster.h"
#include "sge_event_master.h"
#include "configuration_qmaster.h"
#include "sge_persistence_qmaster.h"
#include "msg_qmaster.h"
#include "msg_common.h"



static void check_reprioritize_interval(sge_gdi_ctx_class_t *ctx, lList **alpp, char *ruser, char *rhost);


int sge_read_sched_configuration(sge_gdi_ctx_class_t *ctx, lListElem *aSpoolContext, lList **anAnswer)
{
   lList *sched_conf = NULL;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "sge_read_sched_configuration");

   spool_read_list(anAnswer, aSpoolContext, &sched_conf, SGE_TYPE_SCHEDD_CONF);

   if (lGetNumberOfElem(sched_conf) == 0)
   {
      lListElem *ep = sconf_create_default();

      if (sched_conf == NULL) {
         sched_conf = lCreateList("schedd_config_list", SC_Type);
      }
   
      lAppendElem(sched_conf, ep);
      spool_write_object(anAnswer, spool_get_default_context(), ep, "schedd_conf", SGE_TYPE_SCHEDD_CONF, job_spooling);
   }
   
   if (!sconf_set_config(&sched_conf, anAnswer))
   {
      lFreeList(&sched_conf);
      DEXIT;
      return -1;
   } 

   check_reprioritize_interval(ctx, anAnswer, "local" , "local");

   DEXIT;
   return 0;
}
	

/************************************************************
  sge_mod_sched_configuration - Master code

  Modify scheduler configuration. We have only one entry in
  Master_Sched_Config_List. So we replace it with the new one.
 ************************************************************/
int sge_mod_sched_configuration(
sge_gdi_ctx_class_t *ctx,
lListElem *confp,
lList **alpp,
char *ruser,
char *rhost 
) {
   lList *temp_conf_list = NULL;
   
   DENTER(TOP_LAYER, "sge_mod_sched_configuration");

   if ( !confp || !ruser || !rhost ) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }
   temp_conf_list = lCreateList("sched config", SC_Type);

   lSetUlong(confp, SC_weight_tickets_override, 
             sconf_get_weight_tickets_override());

   confp = lCopyElem(confp);
   lAppendElem(temp_conf_list, confp);

   /* just check and log */
   if (!sconf_set_config(&temp_conf_list, alpp)) {
      lFreeList(&temp_conf_list);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   if (!sge_event_spool(ctx,
                        alpp, 0, sgeE_SCHED_CONF, 
                        0, 0, "schedd_conf", NULL, NULL,
                        confp, NULL, NULL, true, true)) {
      answer_list_add(alpp, MSG_SCHEDCONF_CANTCREATESCHEDULERCONFIGURATION, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return -1;
   }

   check_reprioritize_interval(ctx, alpp, ruser, rhost);

   INFO((SGE_EVENT, MSG_SGETEXT_MODIFIEDINLIST_SSSS, ruser, rhost, "scheduler", "scheduler configuration"));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);

   DEXIT;
   return STATUS_OK;
} /* sge_mod_sched_configuration */


static void check_reprioritize_interval(sge_gdi_ctx_class_t *ctx, lList **alpp, char *ruser, char *rhost)
{
   DENTER(TOP_LAYER, "check_reprioritize_interval");

   if (((sconf_get_reprioritize_interval() == 0) && (mconf_get_reprioritize())) ||
       ((sconf_get_reprioritize_interval() != 0) && (!mconf_get_reprioritize()))) {
      bool flag       = (sconf_get_reprioritize_interval() != 0) ? true : false;
      lListElem *conf = sge_get_configuration_for_host(SGE_GLOBAL_NAME);

      sge_set_conf_reprioritize(conf, flag);

      sge_mod_configuration(ctx, conf, alpp, ruser, rhost);

      lFreeElem(&conf);
   }

   DEXIT;
   return;
} /* check_reprioritize_interval */

