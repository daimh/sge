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
 *   Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/sge_spool.h"
#include "uti/sge_log.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_string.h"
#include "uti/sge_time.h"

#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "comm/commlib.h"

#include "sched/sge_complex_schedd.h"
#include "sched/sort_hosts.h"
#include "sched/sge_select_queue.h"
#include "sched/debit.h"

#include "spool/sge_spooling.h"

#include "sge_event_master.h"
#include "sge_c_gdi.h"
#include "sge.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "msg_common.h"
#include "msg_qmaster.h"


/* ------------------------------------------------------------ */

int 
centry_mod(sge_gdi_ctx_class_t *ctx,
           lList **answer_list, lListElem *centry, lListElem *reduced_elem, 
           int add, const char *remote_user, const char *remote_host, 
           gdi_object_t *object, int sub_command, monitoring_t *monitor) 
{
   bool ret = true;
   bool is_slots_attr = false;
   int pos;

   double dval;
   char error_msg[200];
   const char *attrname;
   const char *temp;

   DENTER(TOP_LAYER, "centry_mod");

   /*
    * At least the centry name has to be available (CE_name)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_name, SGE_NO_ABORT);

      if (pos >= 0) {
         const char *name = lGetPosString(reduced_elem, pos);

         DPRINTF(("Got CE_name: "SFQ"\n", name));
         lSetString(centry, CE_name, name);
         if (!strcmp("slots", name)) {
            is_slots_attr = true;
         }
      } 
   }

   /*
    * Shortcut (CE_shortcut)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_shortcut, SGE_NO_ABORT);

      if (pos >= 0) {
         const char *shortcut = lGetPosString(reduced_elem, pos);

         DPRINTF(("Got CE_shortcut: "SFQ"\n", shortcut ? shortcut : "-NA-"));
         lSetString(centry, CE_shortcut, shortcut);
      }
   }
   
   /*
    * Type (CE_valtype)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_valtype, SGE_NO_ABORT);

      if (pos >= 0) {
         u_long32 type = lGetPosUlong(reduced_elem, pos);

         if (is_slots_attr) {
            type = TYPE_INT;
         }
         DPRINTF(("Got CE_valtype: "sge_u32"\n", type));
         lSetUlong(centry, CE_valtype, type);
      }
   }
   
   /*
    * Operator (CE_relop)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_relop, SGE_NO_ABORT);

      if (pos >= 0) {
         u_long32 relop = lGetPosUlong(reduced_elem, pos);

         if (is_slots_attr) {
            relop = CMPLXLE_OP;
         }
         DPRINTF(("Got CE_relop: "sge_u32"\n", relop));
         lSetUlong(centry, CE_relop, relop);
      }
   }

   /*
    * Requestable (CE_request)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_requestable, SGE_NO_ABORT);

      if (pos >= 0) {
         u_long32 request = lGetPosUlong(reduced_elem, pos);

         if (is_slots_attr) {
            request = REQU_YES;
         }
         DPRINTF(("Got CE_requestable: "sge_u32"\n", request));
         lSetUlong(centry, CE_requestable, request);
      }
   }

   /*
    * Consumable (CE_consumable)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_consumable, SGE_NO_ABORT);

      if (pos >= 0) {
         u_long32 consumable = lGetPosUlong(reduced_elem, pos);

         if (is_slots_attr) {
            consumable = CONSUMABLE_YES;
         }
         DPRINTF(("Got CE_consumable: "sge_u32"\n", consumable));
         lSetUlong(centry, CE_consumable, consumable);
      }
   }

   /*
    * Default (CE_default)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_default, SGE_NO_ABORT);

      if (pos >= 0) {
         const char *defaultval = lGetPosString(reduced_elem, pos);

         if (is_slots_attr) {
            defaultval = "1";
         }
         DPRINTF(("Got CE_default: "SFQ"\n", defaultval ? defaultval : "-NA-"));
         lSetString(centry, CE_default, defaultval);
      }
   }

   /*
    * Default (CE_urgency_weight)
    */
   if (ret) {
      pos = lGetPosViaElem(reduced_elem, CE_urgency_weight, SGE_NO_ABORT);

      if (pos >= 0) {
         const char *urgency_weight = lGetPosString(reduced_elem, pos);
         DPRINTF(("Got CE_default: "SFQ"\n", urgency_weight ? urgency_weight : "-NA-"));
 
         /* Check first that the entry is not NULL */
         if (!pos)  {
            ERROR((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
                  lNm2Str(CE_urgency_weight), "urgency_weight"));
            answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
            DEXIT;
            return STATUS_EEXIST;
         }
         /* Check then that the entry is valid   */

         attrname = lGetString(reduced_elem, CE_name);
         temp = lGetString(reduced_elem, CE_urgency_weight);
         if(!parse_ulong_val(&dval, NULL, TYPE_DOUBLE, temp, error_msg, 199)){answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN , ANSWER_QUALITY_ERROR, MSG_INVALID_CENTRY_PARSE_URGENCY_SS, attrname, error_msg);
               ret = false;
         }

         lSetString(centry, CE_urgency_weight, urgency_weight);
      }
   }

   if (ret) {
      ret = centry_elem_validate(centry, NULL, answer_list);
   }

   DEXIT;
   if (ret) {
      return 0;
   } else {
      return STATUS_EUNKNOWN;
   }
}

/* ------------------------------------------------------------ */

int 
centry_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *cep, gdi_object_t *object) 
{
   lList *answer_list = NULL;
   bool dbret;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "centry_spool");

   dbret = spool_write_object(&answer_list, spool_get_default_context(), cep, 
                              lGetString(cep, CE_name), SGE_TYPE_CENTRY,
                              job_spooling);
   answer_list_output(&answer_list);

   if (!dbret) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_PERSISTENCE_WRITE_FAILED_S,
                              lGetString(cep, CE_name));
   } 
   
   DEXIT;
   return dbret ? 0 : 1;
}

/* ------------------------------------------------------------ */

/****** sge_centry_qmaster/centry_success() ************************************
*  NAME
*     centry_success() -- ??? 
*
*  SYNOPSIS
*     int centry_success(lListElem *ep, lListElem *old_ep, gdi_object_t 
*     *object) 
*
*  FUNCTION
*
*
*  INPUTS
*     lListElem *ep        - ??? 
*     lListElem *old_ep    - ??? 
*     gdi_object_t *object - ??? 
*
*  RESULT
*     int - 
*
*  EXAMPLE
*     ??? 
*
*  NOTES
*     MT-NOTE: centry_success() is not MT safe 
*
*  BUGS
*     This function is the cause for huge overhead with processing complex 
*     entry change requests: Each change with complex configuration causes 
*     debitations for ALL resources be re-done with all hosts and queues 
*     based on per job resource requests. There should be a chance to notably 
*     lower resource consumption doing this only for those complexes where 
*     changes actually occurred.
* 
*     There is no need sort centry list each time a change occurs (fixed).
* 
*     Reporting needs to be updated only for the changed complex entry (minor issue).
*
*     No updates have to be done for the ADD operation - the centry cannot be 
*     referenced anywhere at ADD time (fixed).
*
*     Update is only required, if the consumable attribute changed or
*     the centry is a consumable and the default request changed (fixed).
*
*     The whole update mechanism wouldn't be required, if the granted resources
*     would be stored in the job object for running jobs. It is only required,
*     as the granted resources are not stored in the job and therefore a change
*     of the default request would result in a wrong number of resources freed
*     for the consumable at job end.
*     Storing the granted resources in the job object would have further
*     advantages: - qstat -j would display the default requests for consumables
*                 - qstat -j would display granted soft requests
*                 - soft requests on consumables could be enabled
*
*******************************************************************************/
int 
centry_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor) 
{
   bool rebuild_consumables = false;

   DENTER(TOP_LAYER, "centry_success");

   sge_add_event( 0, old_ep?sgeE_CENTRY_MOD:sgeE_CENTRY_ADD, 0, 0, 
                 lGetString(ep, CE_name), NULL, NULL, ep);
   lListElem_clear_changed_info(ep);

   if (old_ep != NULL) {
      /* 
       * If a complex has become a consumable, or
       * is no longer a consumable, or
       * it is a consumable and the default value has changed,
       * queue / host values for these consumables have to be rebuilt.
       */
      u_long32 consumable = lGetUlong(ep, CE_consumable);
      u_long32 old_consumable = lGetUlong(old_ep, CE_consumable);
      if (consumable != old_consumable) {
            rebuild_consumables = true;
      } else if (consumable) {
         const char *default_request = lGetString(ep, CE_default);
         const char *old_default_request = lGetString(old_ep, CE_default);
         if (sge_strnullcmp(default_request, old_default_request) != 0) {
            rebuild_consumables = true;
         }
      }
   }

   if (rebuild_consumables) {
      lAddElemStr(ppList, STU_name, lGetString(ep, CE_name), STU_Type);
   }

   DEXIT;
   return 0;
}

int sge_del_centry(sge_gdi_ctx_class_t *ctx, lListElem *centry, lList **answer_list, 
                   char *remote_user, char *remote_host) 
{
   bool ret = true;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_del_centry");

   if (centry != NULL || remote_user != NULL || remote_host != NULL) {
      const char* name = lGetString(centry, CE_name);

      if (name != NULL) {
         lList *local_answer_list = NULL;
         lList *master_centry_list = *(centry_list_get_master_list());
         lListElem *tmp_centry = centry_list_locate(master_centry_list, name);

         /* check if its a build in value */
         if (get_rsrc(name, true, NULL, NULL, NULL, NULL) == 0 ||
             get_rsrc(name, false, NULL, NULL, NULL, NULL) == 0) {
            answer_list_add_sprintf(answer_list, STATUS_EUNKNOWN , ANSWER_QUALITY_ERROR, 
                                    MSG_INVALID_CENTRY_DEL_S, name);
            ret = false;
         }

         if (ret) {
            if (tmp_centry != NULL) {
               if (!centry_is_referenced(tmp_centry, &local_answer_list, 
                        *object_base[SGE_TYPE_CQUEUE].list,
                        *object_base[SGE_TYPE_EXECHOST].list,
                        *object_base[SGE_TYPE_RQS].list)) {
                  if (sge_event_spool(ctx, answer_list, 0, sgeE_CENTRY_DEL, 
                                      0, 0, name, NULL, NULL,
                                      NULL, NULL, NULL, true, true)) {

                     lRemoveElem(master_centry_list, &tmp_centry);
                     INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDFROMLIST_SSSS, 
                           remote_user, remote_host, name, MSG_OBJ_CPLX));
                     answer_list_add(answer_list, SGE_EVENT, 
                                     STATUS_OK, ANSWER_QUALITY_INFO);
                  } else {
                     ERROR((SGE_EVENT, MSG_CANTSPOOL_SS,
                           "complex entry", name ));
                     answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST,
                                    ANSWER_QUALITY_ERROR);
                     ret = false;
                  }
               } else {
                  lListElem *answer = lFirst(local_answer_list);

                  ERROR((SGE_EVENT, "denied: %s", lGetString(answer, AN_text)));
                  answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR);
                  lFreeList(&local_answer_list);
                  ret = false;
               }
            } else {
               ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, 
                     MSG_OBJ_CPLX, name));
               answer_list_add(answer_list, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
               ret = false;
            }
         }
      } else {
         CRITICAL((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
                   lNm2Str(CE_name), SGE_FUNC));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = false;
      }
   } else {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      ret = false;
   }

   DEXIT;
   if (ret) {
      return STATUS_OK;
   } else {
      return STATUS_EUNKNOWN;
   }
}

static void
sge_change_queue_version_centry(sge_gdi_ctx_class_t *ctx)
{
   lListElem *ep;
   lListElem *cqueue;
   lList *answer_list = NULL;
   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "sge_change_queue_version_centry");

   for_each(cqueue, *object_base[SGE_TYPE_CQUEUE].list) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance = NULL;

      for_each(qinstance, qinstance_list) {
         qinstance_increase_qversion(qinstance);

         sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD,
                         0, 0, lGetString(qinstance, QU_qname),
                         lGetHost(qinstance, QU_qhostname), NULL,
                         qinstance, NULL, NULL, true, false);
      }
   }
   for_each(ep, *object_base[SGE_TYPE_EXECHOST].list) {
      sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD,
                      0, 0, lGetHost(ep, EH_name), NULL, NULL,
                      ep, NULL, NULL, true, false);
   }
   answer_list_output(&answer_list);

   DRETURN_VOID;
}

/****** sge_centry_qmaster/centry_redebit_consumables() ************************
*  NAME
*     centry_redebit_consumables() -- recompute consumable debitation
*
*  SYNOPSIS
*     void 
*     centry_redebit_consumables(const lList *centries) 
*
*  FUNCTION
*     Recomputes the complete consumable debitation for all queues, hosts and
*     jobs.
*
*  INPUTS
*     const lList *centries - list of centries that acually require 
*                             recomputation.
*
*  NOTES
*     MT-NOTE: centry_redebit_consumables() maybe not MT safe (the functions
*     qinstance_debit_consumable and host_debit_consumable have no MT-NOTE).
*
*     TODO: This function could be highly optimized by taking into account the
*     centry list passed as parameter.
*     This would not only increase performance by only recomputing the 
*     debitation for only the changed centries (and spooling only the actually 
*     affected queues instead of all), but also reduce the number of scheduling 
*     decisions trashed due to a changed queue version number.
*******************************************************************************/
void centry_redebit_consumables(sge_gdi_ctx_class_t *ctx, const lList *centries)
{
   lListElem *cqueue = NULL;
   lListElem *hep = NULL;
   lListElem *jep = NULL;
   object_description *object_base = object_type_get_object_description();
   lList *master_centry_list = *object_base[SGE_TYPE_CENTRY].list;

   /* throw away all old actual values lists and rebuild them from scratch */
   for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance = NULL;

      for_each(qinstance, qinstance_list) {
         lSetList(qinstance, QU_resource_utilization, NULL);
         qinstance_debit_consumable(qinstance, NULL, master_centry_list, 0, true, NULL);
      }
   }
   for_each (hep, *object_base[SGE_TYPE_EXECHOST].list) {
      lSetList(hep, EH_resource_utilization, NULL);
      debit_host_consumable(NULL, hep, master_centry_list, 0, true, NULL);
   }

   /* 
    * completely rebuild resource utilization of 
    * all queues and execution hosts
    * change versions of corresponding queues 
    */ 
   for_each (jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
      lListElem* jatep;

      for_each (jatep, lGetList(jep, JB_ja_tasks)) {
         bool master_task = true;
         lListElem *gdil;
         lListElem *qep = NULL;
         int slots = 0;
         for_each (gdil, lGetList(jatep, JAT_granted_destin_identifier_list)) {
            int qslots;

            if (!(qep = cqueue_list_locate_qinstance(
                               *(object_type_get_master_list(SGE_TYPE_CQUEUE)), 
                               lGetString(gdil, JG_qname)))) {
               /* should never happen */
               master_task = false;
               continue;
            }   

            qslots = lGetUlong(gdil, JG_slots);
            debit_host_consumable(jep, host_list_locate(*object_base[SGE_TYPE_EXECHOST].list,
                                  lGetHost(qep, QU_qhostname)), master_centry_list, qslots, master_task, NULL);
            qinstance_debit_consumable(qep, jep, master_centry_list, qslots, master_task, NULL);
            slots += qslots;
            master_task = false;
         }
         debit_host_consumable(jep, host_list_locate(*object_base[SGE_TYPE_EXECHOST].list,
                               "global"), master_centry_list, slots, true, NULL);
      }
   }

   sge_change_queue_version_centry(ctx);

   /* changing complex attributes can change consumables.
    * dump queue and host consumables to reporting file.
    */
   {
      lList *answer_list = NULL;
      u_long32 now = sge_get_gmt();
      
      /* dump all queue consumables */
      for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
         lListElem *qinstance = NULL;

         for_each(qinstance, qinstance_list) {
            const char *hostname = lGetHost(qinstance, QU_qhostname);
            const lListElem *host = lGetElemHost(*object_base[SGE_TYPE_EXECHOST].list, EH_name, hostname);
            reporting_create_queue_consumable_record(&answer_list, host, qinstance, NULL, now);
         }
      }
      answer_list_output(&answer_list);
      /* dump all host consumables */
      for_each (hep, *object_base[SGE_TYPE_EXECHOST].list) {
         reporting_create_host_consumable_record(&answer_list, hep, NULL, now);
      }
      answer_list_output(&answer_list);
   }
}
