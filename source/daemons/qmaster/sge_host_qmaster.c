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
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_prog.h"
#include "uti/sge_time.h"
#include "uti/sge_log.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_hostname.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_lock.h"

#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_id.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_manop.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "comm/commlib.h"

#include "sched/sge_job_schedd.h"
#include "sched/sge_resource_utilization.h"
#include "sched/sge_serf.h"
#include "sched/debit.h"
#include "sched/valid_queue_user.h"

#include "spool/sge_spooling.h"

#include "sge.h"
#include "symbols.h"
#include "msg_common.h"
#include "msg_qmaster.h"
#include "sge_host_qmaster.h"
#include "sge_event_master.h"
#include "configuration_qmaster.h"
#include "sge_c_gdi.h"
#include "mail.h"
#include "configuration_qmaster.h"
#include "sge_cqueue_qmaster.h"
#include "sge_userset_qmaster.h"
#include "sge_userprj_qmaster.h"
#include "reschedule.h"
#include "sge_qinstance_qmaster.h"
#include "sge_utility_qmaster.h"
#include "qmaster_to_execd.h"
#include "sge_persistence_qmaster.h"
#include "sge_reporting_qmaster.h"
#include "sge_advance_reservation_qmaster.h"
#include "sge_job_enforce_limit.h"

static void exec_host_change_queue_version(sge_gdi_ctx_class_t *ctx, const char *exechost_name);
static void master_kill_execds(sge_gdi_ctx_class_t *ctx, sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task);
static void host_trash_nonstatic_load_values(lListElem *host);
static void notify(sge_gdi_ctx_class_t *ctx, lListElem *lel, sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task, int kill_jobs, int force); 

static int verify_scaling_list(lList **alpp, lListElem *host); 
static void host_update_categories(const lListElem *new_hep, const lListElem *old_hep);
static int attr_mod_threshold(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *ep, lListElem *new_ep,
                              int sub_command, char *attr_name, char *object_name);

#define SGE_ATTR_USAGE_SCALING "usage_scaling"

void 
host_initalitze_timer(void)
{
   object_description *object_base = NULL;

   DENTER(TOP_LAYER, "host_initalitze_timer");

   object_base = object_type_get_object_description();

   /* initiate timer for all hosts because they start in 'unknown' state */
   if (*object_base[SGE_TYPE_EXECHOST].list) {
      lListElem *host               = NULL;
      lListElem *global_host_elem   = NULL;
      lListElem *template_host_elem = NULL;

      /* get "global" element pointer */
      global_host_elem   = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, SGE_GLOBAL_NAME);

      /* get "template" element pointer */
      template_host_elem = host_list_locate(*object_base[SGE_TYPE_EXECHOST].list, SGE_TEMPLATE_NAME);
      
      for_each(host, *object_base[SGE_TYPE_EXECHOST].list) {
         if ((host != global_host_elem) && (host != template_host_elem)) {
            reschedule_add_additional_time(load_report_interval(host));
            reschedule_unknown_trigger(host);
            reschedule_add_additional_time(0);
         }
      }  
   }
   DRETURN_VOID;
}


/****** qmaster/host/host_trash_nonstatic_load_values() ***********************
*  NAME
*     host_trash_nonstatic_load_values() -- Trash old load values 
*
*  SYNOPSIS
*     static void host_trash_nonstatic_load_values(lListElem *host) 
*
*  FUNCTION
*     Trash old load values in "host" element 
*
*  INPUTS
*     lListElem *host - EH_Type element 
*
*  RESULT
*     void - None
*******************************************************************************/
static void host_trash_nonstatic_load_values(lListElem *host) 
{
   lList *load_attr_list;
   lListElem *next_load_attr;
   lListElem *load_attr;

   load_attr_list = lGetList(host, EH_load_list);
   next_load_attr = lFirst(load_attr_list);
   while ((load_attr = next_load_attr)) {
      next_load_attr = lNext(load_attr);
      if (!lGetBool(load_attr, HL_static)) {
         lRemoveElem(load_attr_list, &load_attr);
      }
   }
}

/* ------------------------------------------ 

   adds the host to host_list with type 

   -1 error 
   0 ok
   1 host does exist

 */
int sge_add_host_of_type(sge_gdi_ctx_class_t *ctx, const char *hostname,
                         u_long32 target, monitoring_t *monitor)
{
   int ret;
   int dataType;
   int pos;
   lListElem *ep;
   gdi_object_t *object;
   lList *ppList = NULL;
   const char *username = ctx->get_username(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "sge_add_host_of_type");

   if (hostname == NULL) {
      DRETURN(-1);
   }
  
   object = get_gdi_object(target);
   
   /* prepare template */
   ep = lCreateElem(object->type);
   pos = lGetPosInDescr(object->type,object->key_nm);
   dataType = lGetPosType(object->type , pos);
   switch (dataType) {
      case lStringT:
         lSetString(ep, object->key_nm, hostname);
         break;
      case lHostT:
         lSetHost(ep, object->key_nm, hostname);
         break;
      default:
         DPRINTF(("sge_add_host_of_type: unexpected datatype\n"));
   }
   ret = sge_gdi_add_mod_generic(ctx, NULL, ep, 1, object, username, 
      qualified_hostname, 0, &ppList, monitor);
   lFreeElem(&ep);
   lFreeList(&ppList);

   DEXIT;
   return (ret == STATUS_OK) ? 0 : -1;
}

bool
host_list_add_missing_href(sge_gdi_ctx_class_t *ctx,
                           lList *this_list, 
                           lList **answer_list, 
                           const lList *href_list, 
                           monitoring_t *monitor)
{
   bool ret = true;
   lListElem *href = NULL;

   DENTER(TOP_LAYER, "host_list_add_missing_href");
   for_each(href, href_list) {
      const char *hostname = lGetHost(href, HR_name);
      lListElem *host = host_list_locate(this_list, hostname);

      if (host == NULL) {
         ret &= (sge_add_host_of_type(ctx, hostname, SGE_EH_LIST, monitor) == 0);
      }
   }
   DRETURN(ret);
}

/* ------------------------------------------------------------

   sge_del_host - deletes a host from the host_list

   if the invoking process is the qmaster the host list is
   spooled to disk

*/
int sge_del_host(sge_gdi_ctx_class_t *ctx, lListElem *hep, lList **alpp,
                 char *ruser, char *rhost, u_long32 target,
                 const lList* master_hGroup_List)
{
   int pos;
   lListElem *ep;
   const char *host;
   char unique[CL_MAXHOSTLEN];
   lList **host_list = NULL;
   int nm = 0;
   char *name = NULL;
   int ret;
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);

   DENTER(TOP_LAYER, "sge_del_host");

   if ( !hep || !ruser || !rhost ) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   switch ( target ) {
   case SGE_EH_LIST:
      host_list = object_type_get_master_list(SGE_TYPE_EXECHOST);
      nm = EH_name;
      name = "execution host";
      break;
   case SGE_AH_LIST:
      host_list = object_type_get_master_list(SGE_TYPE_ADMINHOST);
      nm = AH_name;
      name = "administrative host";
      break;
   case SGE_SH_LIST:
      host_list = object_type_get_master_list(SGE_TYPE_SUBMITHOST);
      nm = SH_name;
      name = "submit host";
      break;
   default:
     DEXIT;
     return STATUS_EUNKNOWN;
   }
   /* ep is no host element, if ep has no nm */
   if ((pos = lGetPosViaElem(hep, nm, SGE_NO_ABORT)) < 0) {
      ERROR((SGE_EVENT, MSG_SGETEXT_MISSINGCULLFIELD_SS,
            lNm2Str(nm), SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   host = lGetPosHost(hep, pos);
   if (!host) {
      ERROR((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }

   ret = sge_resolve_hostname(host, unique, EH_name, sizeof(unique));
   if (ret  != CL_RETVAL_OK) {
      /* 
       * Due to CR 6319231, IZ 1760 this is allowed 
       */
      ;
   }

   /* check if host is in host list */
   if ((ep=host_list_locate(*host_list, unique))==NULL) {
      /* may be host was not the unique hostname.
         Get the unique hostname and try to find it again. */
      if (getuniquehostname(host, unique, 0)!=CL_RETVAL_OK)
      {
         ERROR((SGE_EVENT, MSG_SGETEXT_CANTRESOLVEHOST_S, host));
         answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DEXIT;
         return STATUS_EUNKNOWN;
      }
      /* again check if host is in host list. This time use the unique
         hostname */
      if ((ep=host_list_locate(*host_list, unique))==NULL) {
         ERROR((SGE_EVENT, MSG_SGETEXT_DOESNOTEXIST_SS, name, host));
         answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
         DEXIT;
         return STATUS_EEXIST;
      }
   }

   /* 
      check if someone tries to delete 
      the qmaster host from admin host list
   */
   if (target==SGE_AH_LIST && 
         !sge_hostcmp(unique, qualified_hostname)) {
      ERROR((SGE_EVENT, MSG_SGETEXT_CANTDELADMINQMASTER_S, 
          qualified_hostname));
      answer_list_add(alpp, SGE_EVENT, STATUS_EEXIST, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EEXIST;
   }

   if (target == SGE_EH_LIST && 
       host_is_referenced(hep, alpp, 
                          *(object_type_get_master_list(SGE_TYPE_CQUEUE)),
                          master_hGroup_List)) {
      answer_list_log(alpp, false, true);                    
      DEXIT;
      return STATUS_ESEMANTIC;
   }

   if (target==SGE_EH_LIST && !strcasecmp(unique, "global")) {
      ERROR((SGE_EVENT, SFNMAX, MSG_OBJ_DELGLOBALHOST));
      answer_list_add(alpp, SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_ESEMANTIC;
   }

   /* remove host file and send event */
   switch(target) {
      case SGE_AH_LIST:
         {
            lList *answer_list = NULL;
            sge_event_spool(ctx, &answer_list, 0, sgeE_ADMINHOST_DEL, 
                            0, 0, lGetHost(ep, nm), NULL, NULL,
                            NULL, NULL, NULL, true, true);
            answer_list_output(&answer_list);
         }
         break;
      case SGE_EH_LIST:
         {
            lList *answer_list = NULL;
            sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_DEL, 
                            0, 0, lGetHost(ep, nm), NULL, NULL,
                            NULL, NULL, NULL, true, true);
            answer_list_output(&answer_list);
         }
	 host_update_categories(NULL, ep);

         break;
      case SGE_SH_LIST:
         {
            lList *answer_list = NULL;
            sge_event_spool(ctx, &answer_list, 0, sgeE_SUBMITHOST_DEL, 
                            0, 0, lGetHost(ep, nm), NULL, NULL,
                            NULL, NULL, NULL, true, true);
            answer_list_output(&answer_list);
         }
         break;
   }

   /* delete found host element */
   lRemoveElem(*host_list, &ep);

   INFO((SGE_EVENT, MSG_SGETEXT_REMOVEDFROMLIST_SSSS, 
         ruser, rhost, unique, name));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
   DEXIT;
   return STATUS_OK;
}

/* ------------------------------------------------------------ */

int host_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *new_host, lListElem *ep, int add,
             const char *ruser, const char *rhost, gdi_object_t *object, int sub_command,
             monitoring_t *monitor)
{
   const char *host;
   int nm;
   int pos;
   int dataType;
   bool changed = false;
   bool update_qversion = false;

   object_description *object_base = object_type_get_object_description();

   DENTER(TOP_LAYER, "host_mod");

   nm = object->key_nm;

   /* ---- [EAS]H_name */
   if (add) {
      if (attr_mod_str(alpp, ep, new_host, nm, object->object_name)) {
         goto ERROR;
      }
   }
   pos = lGetPosViaElem(new_host, nm, SGE_NO_ABORT);
   dataType = lGetPosType(lGetElemDescr(new_host),pos);
   if (dataType == lHostT) {
      host = lGetHost(new_host, nm);
   } else {
      host = lGetString(new_host, nm);
   }
   if (nm == EH_name) {
      bool acl_changed = false;

      /* ---- EH_scaling_list */
      if (lGetPosViaElem(ep, EH_scaling_list, SGE_NO_ABORT)>=0) {
         attr_mod_sub_list(alpp, new_host, EH_scaling_list, HS_name, ep,
                           sub_command, SGE_ATTR_LOAD_SCALING, SGE_OBJ_EXECHOST, 0, &changed);
         if (verify_scaling_list(alpp, new_host)!=STATUS_OK) {
            goto ERROR;
         }
         if (changed) {
            update_qversion = true;
         }
      }

      /* ---- EH_consumable_config_list */
      if (attr_mod_threshold(ctx, alpp, ep, new_host,
                             sub_command, SGE_ATTR_COMPLEX_VALUES,
                             SGE_OBJ_EXECHOST)) {
         goto ERROR;
      }

      /* ---- EH_acl */
      if (lGetPosViaElem(ep, EH_acl, SGE_NO_ABORT)>=0) {
         DPRINTF(("got new EH_acl\n"));
         /* check acl list */
         if (userset_list_validate_acl_list(lGetList(ep, EH_acl), alpp) != STATUS_OK) {
            goto ERROR;
         }
         attr_mod_sub_list(alpp, new_host, EH_acl, US_name, ep,
                           sub_command, SGE_ATTR_USER_LISTS, SGE_OBJ_EXECHOST, 0, &changed);
         if (changed) {
            acl_changed = true;
            update_qversion = true;
         }
      }

      /* ---- EH_xacl */
      if (lGetPosViaElem(ep, EH_xacl, SGE_NO_ABORT)>=0) {
         DPRINTF(("got new EH_xacl\n"));
         /* check xacl list */
         if (userset_list_validate_acl_list(lGetList(ep, EH_xacl), alpp) != STATUS_OK) {
            goto ERROR;
         }
         attr_mod_sub_list(alpp, new_host, EH_xacl, US_name, ep,
                           sub_command, SGE_ATTR_XUSER_LISTS,
                           SGE_OBJ_EXECHOST, 0, &changed);
         if (changed) {
            acl_changed = true;
            update_qversion = true;
         }
      }


      /* ---- EH_prj */
      if (lGetPosViaElem(ep, EH_prj, SGE_NO_ABORT)>=0) {
         DPRINTF(("got new EH_prj\n"));
         /* check prj list */
         if (verify_project_list(alpp, lGetList(ep, EH_prj),
                  *object_base[SGE_TYPE_PROJECT].list, "projects",
                  object->object_name, host)!=STATUS_OK) {
            goto ERROR;
         }
         attr_mod_sub_list(alpp, new_host, EH_prj, PR_name, ep,
                           sub_command, SGE_ATTR_PROJECTS,
                           SGE_OBJ_EXECHOST, 0, &changed);
         if (changed) {
            update_qversion = true;
         }
      }

      /* ---- EH_xprj */
      if (lGetPosViaElem(ep, EH_xprj, SGE_NO_ABORT)>=0) {
         DPRINTF(("got new EH_xprj\n"));
         /* check xprj list */
         if (verify_project_list(alpp, lGetList(ep, EH_xprj),
                  *object_base[SGE_TYPE_PROJECT].list, "xprojects",
                  object->object_name, host)!=STATUS_OK) {
            goto ERROR;
         }
         attr_mod_sub_list(alpp, new_host, EH_xprj, PR_name, ep,
                           sub_command, SGE_ATTR_XPROJECTS,
                           SGE_OBJ_EXECHOST, 0, &changed);
         if (changed) {
            update_qversion = true;
         }
      }

      /* ---- EH_usage_scaling_list */
      if (lGetPosViaElem(ep, EH_usage_scaling_list, SGE_NO_ABORT)>=0) {
         attr_mod_sub_list(alpp, new_host, EH_usage_scaling_list, HS_name, ep,
         sub_command, SGE_ATTR_USAGE_SCALING, SGE_OBJ_EXECHOST, 0, &changed);
         if (changed) {
            update_qversion = true;
         }
      }

      if (lGetPosViaElem(ep, EH_report_variables, SGE_NO_ABORT)>=0) {
         const lListElem *var;

         attr_mod_sub_list(alpp, new_host, EH_report_variables, STU_name, ep,
                           sub_command, "report_variables",
                           SGE_OBJ_EXECHOST, 0, NULL);

         /* check if all report_variables are valid complex variables */
         for_each(var, lGetList(ep, EH_report_variables)) {
            const char *name = lGetString(var, STU_name);
            if (centry_list_locate(*object_base[SGE_TYPE_CENTRY].list, name) == NULL) {
               ERROR((SGE_EVENT, MSG_SGETEXT_UNKNOWN_RESOURCE_S, name));
               answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               goto ERROR;
            }
         }
      }

      if (acl_changed == true) {
         lListElem *ar;
         lList *master_userset_list = *(object_type_get_master_list(SGE_TYPE_USERSET));

         for_each(ar, *(object_type_get_master_list(SGE_TYPE_AR))) {
            if (lGetElemHost(lGetList(ar, AR_granted_slots), JG_qhostname, host)) {
               if (!sge_ar_have_users_access(NULL, ar, host, lGetList(ep, EH_acl),
                                             lGetList(ep, EH_xacl),
                                             master_userset_list)) {
                  ERROR((SGE_EVENT, MSG_PARSE_MOD3_REJECTED_DUE_TO_AR_SU,
                         SGE_ATTR_USER_LISTS, sge_u32c(lGetUlong(ar, AR_id))));
                  answer_list_add(alpp, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
                  goto ERROR;
               }
            }
         }
      }
   }

   if (update_qversion) {
      exec_host_change_queue_version(ctx, host);
   }

   DRETURN(0);
ERROR:
   DRETURN(STATUS_EUNKNOWN);
}

int host_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *ep, gdi_object_t *object)
{
   int pos;
   int dataType;
   const char *key;
   sge_object_type host_type = SGE_TYPE_ADMINHOST;

   int ret = 0;
   lList *answer_list = NULL;
   bool job_spooling = ctx->get_job_spooling(ctx);

   DENTER(TOP_LAYER, "host_spool");

   pos = lGetPosViaElem(ep, object->key_nm, SGE_NO_ABORT );
   dataType = lGetPosType(lGetElemDescr(ep),pos);
   if (dataType == lHostT ) { 
      key = lGetHost(ep, object->key_nm);
   } else {
      key = lGetString(ep, object->key_nm);
   }
     
   switch (object->key_nm) {
      case AH_name:
         host_type = SGE_TYPE_ADMINHOST;
         break;
      case EH_name:
         host_type = SGE_TYPE_EXECHOST;
         break;
      case SH_name:
         host_type = SGE_TYPE_SUBMITHOST;
         break;
   }
     
   if (!spool_write_object(alpp, spool_get_default_context(), ep, key, host_type, job_spooling)) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, 
                              ANSWER_QUALITY_ERROR, 
                              MSG_PERSISTENCE_WRITE_FAILED_S,
                              key);
      ret = 1;
   }
   answer_list_output(&answer_list);

   DRETURN(ret);
}

int host_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep, gdi_object_t *object, lList **ppList, monitoring_t *monitor) 
{
   DENTER(TOP_LAYER, "host_success");

   switch(object->key_nm) {
      case EH_name:
      {
         const char *host = lGetHost(ep, EH_name);
         int global_host = !strcmp(SGE_GLOBAL_NAME, host);

         if (global_host) {
            host_list_merge(*object_type_get_master_list(SGE_TYPE_EXECHOST));
         } else {
            const lListElem *global_ep = NULL;

            global_ep = lGetElemHost(*object_type_get_master_list(SGE_TYPE_EXECHOST), EH_name,
                                     SGE_GLOBAL_NAME);
            host_merge(ep, global_ep);
         }

         host_update_categories(ep, old_ep);
         sge_add_event( 0, old_ep?sgeE_EXECHOST_MOD:sgeE_EXECHOST_ADD,
                       0, 0, host, NULL, NULL, ep);
         lListElem_clear_changed_info(ep);
      }
      break;

      case AH_name:
         sge_add_event( 0, old_ep?sgeE_ADMINHOST_MOD:sgeE_ADMINHOST_ADD, 
                       0, 0, lGetHost(ep, AH_name), NULL, NULL, ep);
         lListElem_clear_changed_info(ep);
      break;

      case SH_name:
         sge_add_event( 0, old_ep?sgeE_SUBMITHOST_MOD:sgeE_SUBMITHOST_ADD, 
                       0, 0, lGetHost(ep, SH_name), NULL, NULL, ep);
         lListElem_clear_changed_info(ep);
      break;
   }

   DRETURN(0);
}

/* ------------------------------------------------------------ */

void sge_mark_unheard(lListElem *hep) {
   const char *host;

   DENTER(TOP_LAYER, "sge_mark_unheard");

   host = lGetHost(hep, EH_name);

   if (cl_com_remove_known_endpoint_from_name(host, prognames[EXECD], 1) == CL_RETVAL_OK) {
      DEBUG((SGE_EVENT, "set %s/%s/%d to unheard\n", host, prognames[EXECD], 1));
   }

   if (lGetUlong(hep, EH_lt_heard_from) != 0) {
      host_trash_nonstatic_load_values(hep);
      cqueue_list_set_unknown_state(
            *(object_type_get_master_list(SGE_TYPE_CQUEUE)),
            host, true, true);

      lSetUlong(hep, EH_lt_heard_from, 0);

      /* add a trigger to enforce limits when they are exceeded */
      sge_host_add_enforce_limit_trigger(host);

      /* hedeby depends on this event */
      sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, host, NULL, NULL, hep);
   }

   DRETURN_VOID;
}

/* ----------------------------------------

   updates global and host specific load values
   using the load report list lp
*/
void sge_update_load_values(sge_gdi_ctx_class_t *ctx, const char *rhost, lList *lp)
{
   u_long32 now;
   lListElem *ep, **hepp = NULL;
   lListElem *lep;
   lListElem *global_ep = NULL;
   lListElem *host_ep = NULL;
   bool statics_changed = false;
   lList *answer_list = NULL;

   DENTER(TOP_LAYER, "sge_update_load_values");

   /* JG: TODO: this time should better come with the report.
    *           it is the time when the reported values were valid.
    */
   now = sge_get_gmt();

   host_ep = lGetElemHost(*object_type_get_master_list(SGE_TYPE_EXECHOST), EH_name, rhost);
   if (host_ep == NULL) {
      /* report from unknown host arrived, ignore it */
      DRETURN_VOID;
   }

   /* 
    * if rhost is unknown set him to known
    */
   if (lGetUlong(host_ep, EH_lt_heard_from) == 0) {
      cqueue_list_set_unknown_state(*(object_type_get_master_list(SGE_TYPE_CQUEUE)),
                                    rhost, true, false);

      /* remove a trigger to enforce limits when they are exceeded */
      sge_host_remove_enforce_limit_trigger(rhost);

      lSetUlong(host_ep, EH_lt_heard_from, sge_get_gmt());
   }

   host_ep = NULL;
   /* loop over all received load values */
   for_each(ep, lp) {
      /* get name, value and other info */
      const char *name = lGetString(ep, LR_name);
      const char *value = lGetString(ep, LR_value);
      const char *host  = lGetHost(ep, LR_host);
      u_long32 global = lGetUlong(ep, LR_global);
      u_long32 is_static = lGetUlong(ep, LR_static);

      /* erroneous load report */
      if (!name || !value || !host) {
         continue;
      }   

      /* handle global or exec host? */
      if (global) {
         hepp = &global_ep;
      } else {
         hepp = &host_ep;
      }

      /* update load value list of reported host */
      if (*hepp == NULL || sge_hostcmp(host, lGetHost(*hepp, EH_name)) != 0) {
         if (*hepp != NULL) {
            /* we have a host change, send events for the previous one */
            sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD, 
                            0, 0, lGetHost(*hepp, EH_name), NULL, NULL,
                            host_ep, NULL, NULL, true, statics_changed);
            reporting_create_host_record(&answer_list, *hepp, now);
            statics_changed = false;
         }

         /* get the new host */
         *hepp = lGetElemHost(*object_type_get_master_list(SGE_TYPE_EXECHOST), EH_name, host);
         if (*hepp == NULL) {
            INFO((SGE_EVENT, MSG_CANT_ASSOCIATE_LOAD_SS, rhost, host));
            continue;
         }
      } 

      if (is_static == 2) {
         /* remove old load value */
         lep = lGetSubStr(*hepp, HL_name, name, EH_load_list);  

         lRemoveElem(lGetList(*hepp, EH_load_list), &lep);
      } else {
         /* add a new load value */
         lep = lGetSubStr(*hepp, HL_name, name, EH_load_list);  
         if (lep == NULL) {
            lep = lAddSubStr(*hepp, HL_name, name, EH_load_list, HL_Type);
            DPRINTF(("%s: adding load value: "SFQ" = "SFQ"\n", host, name, value));
            if (is_static == 1) {
               statics_changed = true;
            }
         } else {
            /* replace an existing load value */
            if (is_static == 1) {
               if (sge_strnullcmp(value, lGetString(lep, HL_value)) != 0) {
                  statics_changed = true;
               }
            }
         }

         /* copy value */
         lSetString(lep, HL_value, value); 
         lSetUlong(lep, HL_last_update, now);
         lSetBool(lep, HL_static, is_static);
      }
   }

   /*
   ** if static load values (eg arch) have changed
   ** then spool
   */
   if (hepp != NULL && *hepp != NULL) {
      sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD, 
                      0, 0, lGetHost(*hepp, EH_name), NULL, NULL,
                      *hepp, NULL, NULL, true, statics_changed);

      reporting_create_host_record(&answer_list, *hepp, now);
   }

   if (global_ep) {
      sge_event_spool(ctx, &answer_list, 0, sgeE_EXECHOST_MOD, 
                      0, 0, SGE_GLOBAL_NAME, NULL, NULL,
                      global_ep, NULL, NULL, true, false);
      reporting_create_host_record(&answer_list, global_ep, now);
   }
   answer_list_output(&answer_list);

   DRETURN_VOID;
}

/* ----------------------------------------

   trash old load values 
   
*/
void sge_load_value_cleanup_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor)
{
   lListElem *hep; 
   const char *host;
   u_long32 timeout; 
   lListElem *global_host_elem   = NULL;
   lListElem *template_host_elem = NULL;
   u_long32 now = sge_get_gmt();
   lList *master_exechost_list = *object_type_get_master_list(SGE_TYPE_EXECHOST);
   u_long32 max_unheard = mconf_get_max_unheard();
   bool simulate_execds = mconf_get_simulate_execds();

   DENTER(TOP_LAYER, "sge_load_value_cleanup_handler");

   MONITOR_WAIT_TIME(SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE), monitor);

   /* get "global" element pointer */
   global_host_elem   = host_list_locate(master_exechost_list, SGE_GLOBAL_NAME);    
   /* get "template" element pointer */
   template_host_elem = host_list_locate(master_exechost_list, SGE_TEMPLATE_NAME); 
   /* take each host including the "global" host */
   for_each(hep, master_exechost_list) {
      unsigned long last_heard;
      if (hep == template_host_elem || hep == global_host_elem) {
         continue;
      }   

      host = lGetHost(hep, EH_name);

      /* do not trash load values of simulated hosts */
      if (simulate_execds) {
         const lListElem *load_report_host = lGetSubStr(hep, CE_name, "load_report_host", EH_consumable_config_list);
         if (load_report_host != NULL) {
            const char *real_host = lGetString(load_report_host, CE_stringval);
            if (real_host != NULL && sge_hostcmp(real_host, host) != 0) {
               DPRINTF(("skip trashing load values for host %s simulated by %s\n", host, real_host));
               continue;
            }
         }
      }

      if (lGetUlong(hep, EH_lt_heard_from) == 0) {
         /* host is already unknown, nothing to trash */
         continue;
      }

      cl_commlib_get_last_message_time((cl_com_get_handle(prognames[QMASTER],0)),
                                     (char*)host, (char*)prognames[EXECD], 1, &last_heard);
      

      timeout = MAX(load_report_interval(hep)*3, max_unheard); 
      if (now <= last_heard + timeout) {
         continue;
         /* host is known, nothing to trash */
      }

      lSetUlong(hep, EH_lt_heard_from, 0);

      /* take each load value */
      host_trash_nonstatic_load_values(hep);

      /* set all queues residing at this host in unknown state */
      cqueue_list_set_unknown_state(
            *(object_type_get_master_list(SGE_TYPE_CQUEUE)),
            host, true, true);

      /* add a trigger to enforce limits when they are exceeded */
      sge_host_add_enforce_limit_trigger(host);

      /* hedeby depends on this event */
      sge_add_event(0, sgeE_EXECHOST_MOD, 0, 0, host, NULL, NULL, hep);

      /* initiate timer for this host because they turn into 'unknown' state */
      reschedule_unknown_trigger(hep); 
   }

   mconf_set_new_config(false);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DRETURN_VOID;
}

u_long32 load_report_interval(lListElem *hep)
{
   u_long32 timeout; 
   const char *host;

   DENTER(TOP_LAYER, "load_report_interval");

   host = lGetHost(hep, EH_name);

   /* cache load report interval in exec host to prevent host name resolving each epoch */
   timeout = lGetUlong(hep, EH_load_report_interval);
   if (mconf_is_new_config() || timeout == 0) {
      lListElem *conf_entry = NULL;
      
      if ((conf_entry = sge_get_configuration_entry_by_name(host, "load_report_time")) != NULL) {
         if (parse_ulong_val(NULL, &timeout, TYPE_TIM, lGetString(conf_entry, CF_value), NULL, 0) == 0) {
            ERROR((SGE_EVENT, MSG_OBJ_LOADREPORTIVAL_SS, host, lGetString(conf_entry, CF_value)));
            timeout = 120;
         }
         
         lFreeElem(&conf_entry);
      }
   
      DPRINTF(("%s: load value timeout for host %s is "sge_u32"\n", SGE_FUNC, host, timeout));
      
      lSetUlong(hep, EH_load_report_interval, timeout);
   }
    
   DRETURN(timeout);
}

static void exec_host_change_queue_version(sge_gdi_ctx_class_t *ctx, const char *exechost_name)
{
   lListElem *cqueue = NULL;
   bool change_all = (strcasecmp(exechost_name, SGE_GLOBAL_NAME) == 0) ? true : false;

   DENTER(TOP_LAYER, "exec_host_change_queue_version");

   for_each(cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
      lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
      lListElem *qinstance = NULL;
      lListElem *next_qinstance = NULL;
      const void *iterator = NULL;

      if (change_all) {
         next_qinstance = lFirst(qinstance_list);
      } else {
         next_qinstance = lGetElemHostFirst(qinstance_list, QU_qhostname,
                                            exechost_name, &iterator);
      }
      while ((qinstance = next_qinstance)) {
         const char *name = NULL;
         lList *answer_list = NULL;

         if (change_all) {
            next_qinstance = lNext(qinstance);
            name = SGE_GLOBAL_NAME;
         } else {
            next_qinstance = lGetElemHostNext(qinstance_list, QU_qhostname,
                                              exechost_name, &iterator);
            name = exechost_name;
         }
         DPRINTF((SFQ" has changed. Increasing qversion of"SFQ"\n",
                  name, lGetString(qinstance, QU_full_name)));
         qinstance_increase_qversion(qinstance);
         sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD,
                         0, 0, lGetString(qinstance, QU_qname),
                         lGetHost(qinstance, QU_qhostname), NULL,
                         qinstance, NULL, NULL, true, false);
         answer_list_output(&answer_list);
      }
   }

   DRETURN_VOID;
}

/****
 **** sge_gdi_kill_exechost
 ****
 **** prepares the killing of an exechost (or all).
 **** Actutally only the permission is checked here
 **** and master_kill_execds is called to do the work.
 ****/
void sge_gdi_kill_exechost(sge_gdi_ctx_class_t *ctx, 
                           sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task)
{

   DENTER(GDI_LAYER, "sge_gdi_kill_exechost");

   if (!manop_is_manager(packet->user)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_OBJ_SHUTDOWNPERMS));
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ENOMGR, 
                      ANSWER_QUALITY_ERROR);
      DEXIT;
      return;
   }

   master_kill_execds(ctx, packet, task);
   DEXIT;
}

/******************************************************************
   We have to tell execd(s) to terminate. 

   request->lp is a lList of Type ID_Type.
      If the first ID_str is NULL, we have to kill all execd's.
      Otherwise, every ID_str describes an execd to kill.
      If ID_force is 1, we have to kill jobs, too.
      If ID_force is 0, we don't kill jobs.
 *******************************************************************/
static void master_kill_execds(
sge_gdi_ctx_class_t *ctx, sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task) 
{
   int kill_jobs;
   lListElem *lel, *rep;
   char host[CL_MAXHOSTLEN];
   const char *hostname;

   DENTER(TOP_LAYER, "master_kill_execds");

   if (lGetString(lFirst(task->data_list), ID_str) == NULL) {
      /* this means, we have to kill every execd. */

      kill_jobs = lGetUlong(lFirst(task->data_list), ID_force)?1:0;
      /* walk over exechost list and send every exechosts execd a
         notification */
      for_each(lel, *object_type_get_master_list(SGE_TYPE_EXECHOST)) {  
         hostname = lGetHost(lel, EH_name);
         if (strcmp(hostname, SGE_TEMPLATE_NAME) && strcmp(hostname, SGE_GLOBAL_NAME)) {
            notify(ctx, lel, packet, task, kill_jobs, 0); 

            /* RU: */
            /* initiate timer for this host which turns into unknown state */
            reschedule_unknown_trigger(lel);
         }
      }
      if (lGetNumberOfElem(task->answer_list) == 0) {
         /* no exechosts have been killed */
         DPRINTF(("%s", MSG_SGETEXT_NOEXECHOSTS));
         INFO((SGE_EVENT, SFNMAX, MSG_SGETEXT_NOEXECHOSTS));
         answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
      }
   } else {
      /* only specified exechosts should be killed. */
      
      /* walk over list with execd's to kill */
      for_each(rep, task->data_list) {
         if ((getuniquehostname(lGetString(rep, ID_str), host, 0)) != CL_RETVAL_OK)
         {
            WARNING((SGE_EVENT, MSG_SGETEXT_CANTRESOLVEHOST_S, lGetString(rep, ID_str)));
            answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
         } else {
            if ((lel = host_list_locate(*object_type_get_master_list(SGE_TYPE_EXECHOST), host))) {
               kill_jobs = lGetUlong(rep, ID_force)?1:0;
               /*
               ** if a host name is given, then a kill is forced
               ** this means that even if the host is unheard we try
               ** to kill it
               */
               notify(ctx, lel, packet, task, kill_jobs, 1);
               /* RU: */
               /* initiate timer for this host which turns into unknown state */ 
               reschedule_unknown_trigger(lel);
            } else {
               WARNING((SGE_EVENT, MSG_SGETEXT_ISNOEXECHOST_S, host));
               answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ESEMANTIC, ANSWER_QUALITY_WARNING);
            }
         }
      }
   }
   DEXIT;
}


/********************************************************************
 Notify execd on a host to shutdown
 ********************************************************************/
static void 
notify(sge_gdi_ctx_class_t *ctx, lListElem *lel, sge_gdi_packet_class_t *packet, 
       sge_gdi_task_class_t *task, int kill_jobs, int force) 
{
   const char *hostname;
   u_long execd_alive;
   const char *action_str;
   u_long32 state;
   lListElem *jep;
   lList *mail_users, *gdil;
   int mail_options;
   unsigned long last_heard_from;
   bool job_spooling = ctx->get_job_spooling(ctx);
   int result;

   DENTER(TOP_LAYER, "notify");

   action_str = kill_jobs ? MSG_NOTIFY_SHUTDOWNANDKILL:MSG_NOTIFY_SHUTDOWN;

   hostname = lGetHost(lel, EH_name);

   cl_commlib_get_last_message_time((cl_com_get_handle(prognames[QMASTER], 0)),
                                        (char*)hostname, (char*)prognames[EXECD],1, &last_heard_from);
   execd_alive = last_heard_from;

   if (!force && !execd_alive) {
      WARNING((SGE_EVENT, MSG_OBJ_NOEXECDONHOST_S, hostname));
      answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_ESEMANTIC, 
                     ANSWER_QUALITY_WARNING);
   } else {
      result = host_notify_about_kill(ctx, lel, kill_jobs);
      if (result != 0) {
         INFO((SGE_EVENT, MSG_COM_NONOTIFICATION_SSS, action_str, 
               (execd_alive ? "" : MSG_OBJ_UNKNOWN), hostname));
         answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_EDENIED2HOST, ANSWER_QUALITY_ERROR);         
      } else {
         INFO((SGE_EVENT, MSG_COM_NOTIFICATION_SSS, action_str, 
               (execd_alive ? "" : MSG_OBJ_UNKNOWN), hostname));
         answer_list_add(&(task->answer_list), SGE_EVENT, STATUS_OK, ANSWER_QUALITY_INFO);
      }
      DPRINTF(("%s", SGE_EVENT));
   }

   if(kill_jobs) {
      char sge_mail_subj[1024];
      char sge_mail_body[1024];

      /* mark killed jobs as deleted */
      for_each(jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {   
         lListElem *jatep;
         mail_users = NULL;
         mail_options = 0;

         for_each(jatep, lGetList(jep, JB_ja_tasks)) {
            gdil = lGetList(jatep, JAT_granted_destin_identifier_list);
            if(gdil) {
               if(!(sge_hostcmp(lGetHost(lFirst(gdil), JG_qhostname), hostname))) {
                  /*   send mail to users if requested                  */
                  if (mail_users == NULL) {
                     mail_users = lGetList(jep, JB_mail_list);
                  }
                  if (mail_options == 0) {
                     mail_options = lGetUlong(jep, JB_mail_options);
                  }
                  
                  if (VALID(MAIL_AT_ABORT, mail_options) && !(lGetUlong(jatep, JAT_state) & JDELETED)) {
                     lUlong jep_JB_job_number;
                     const char* jep_JB_job_name;

                     jep_JB_job_number = lGetUlong(jep, JB_job_number);
                     jep_JB_job_name   = lGetString(jep, JB_job_name);

                     snprintf(sge_mail_subj, sizeof(sge_mail_subj),
                              MSG_MAIL_JOBKILLEDSUBJ_US,
                              sge_u32c(jep_JB_job_number), jep_JB_job_name);
                     snprintf(sge_mail_body, sizeof(sge_mail_body),
                              MSG_MAIL_JOBKILLEDBODY_USS,
                              sge_u32c(jep_JB_job_number), jep_JB_job_name,
                              hostname);
                     cull_mail(QMASTER, mail_users, sge_mail_subj, sge_mail_body, "job abortion");
                  }
    
                  /* this job has the killed exechost as master host */
                  state = lGetUlong(jatep, JAT_state);
                  SETBIT(JDELETED, state);
                  lSetUlong(jatep, JAT_state, state);
                  /* spool job */
                  {
                     lList *answer_list = NULL;
                     dstring buffer = DSTRING_INIT;
                     spool_write_object(&answer_list, 
                                        spool_get_default_context(), jep,
                                        job_get_key(lGetUlong(jep, JB_job_number), 
                                            lGetUlong(jatep, JAT_task_number), 
                                            NULL, &buffer), 
                                        SGE_TYPE_JOB,
                                        job_spooling);
                     lListElem_clear_changed_info(jatep);
                     /* JG: TODO: don't we have to send an event? */
                     answer_list_output(&answer_list);
                     sge_dstring_free(&buffer);
                  }
               }
            }
         }
      }
   }

   sge_mark_unheard(lel);

   DEXIT;
   return;
}


/****
 **** sge_execd_startedup
 ****
 **** gdi call for old request starting_up.
 ****/
int 
sge_execd_startedup(sge_gdi_ctx_class_t *ctx, lListElem *host, lList **alpp,
                    char *ruser, char *rhost, u_long32 target,
                    monitoring_t *monitor, bool is_restart) {
   lListElem *hep, *cqueue;
   dstring ds;
   char buffer[256];

   DENTER(TOP_LAYER, "sge_execd_startedup");

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   if( !host || !ruser || !rhost) {
      CRITICAL((SGE_EVENT, MSG_SGETEXT_NULLPTRPASSED_S, SGE_FUNC));
      answer_list_add(alpp, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return STATUS_EUNKNOWN;
   }
   
   hep = host_list_locate(*object_type_get_master_list(SGE_TYPE_EXECHOST), rhost);
   if (!hep) {
      if (sge_add_host_of_type(ctx, rhost, SGE_EH_LIST, monitor) < 0) {
         ERROR((SGE_EVENT, MSG_OBJ_INVALIDHOST_S, rhost));
         answer_list_add(alpp, SGE_EVENT, STATUS_DENIED, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_DENIED);
      } 

      hep = host_list_locate(*object_type_get_master_list(SGE_TYPE_EXECHOST), rhost);
      if (!hep) {
         ERROR((SGE_EVENT, MSG_OBJ_NOADDHOST_S, rhost));
         answer_list_add(alpp, SGE_EVENT, STATUS_DENIED, ANSWER_QUALITY_ERROR);
         DRETURN(STATUS_DENIED);
      }
   }

   lSetUlong(hep, EH_featureset_id, lGetUlong(host, EH_featureset_id));
   lSetUlong(hep, EH_report_seqno, 0);

   /*
    * reinit state of all qinstances at this host according to initial_state
    */
   if (!is_restart) {
      for_each (cqueue, *(object_type_get_master_list(SGE_TYPE_CQUEUE))) {
         lList *qinstance_list = lGetList(cqueue, CQ_qinstances);
         lListElem *qinstance = NULL;

         qinstance = lGetElemHost(qinstance_list, QU_qhostname, rhost);
         if (qinstance != NULL) {
            if (sge_qmaster_qinstance_set_initial_state(qinstance)) {
               lList *answer_list = NULL;

               qinstance_increase_qversion(qinstance);
               sge_event_spool(ctx, &answer_list, 0, sgeE_QINSTANCE_MOD, 
                               0, 0, lGetString(qinstance, QU_qname), 
                               rhost, NULL,
                               qinstance, NULL, NULL, true, true);
               answer_list_output(&answer_list); 
            }
         }
      }
   }
   
   DPRINTF(("=====>STARTING_UP: %s %s on >%s< is starting up\n", 
            feature_get_product_name(FS_SHORT_VERSION, &ds), "execd", rhost));

   INFO((SGE_EVENT, MSG_LOG_REGISTER_SS, "execd", rhost));
   answer_list_add(alpp, SGE_EVENT, STATUS_OK, ANSWER_QUALITY_ERROR);

   DRETURN(STATUS_OK);
}


static int verify_scaling_list(lList **answer_list, lListElem *host) 
{
   bool ret = true;
   lListElem *hs_elem = NULL;
   lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);

   DENTER(TOP_LAYER, "verify_scaling_list");
   for_each (hs_elem, lGetList(host, EH_scaling_list)) {
      const char *name = lGetString(hs_elem, HS_name);
      lListElem *centry = centry_list_locate(master_centry_list, name);
   
      if (centry == NULL) {
         ERROR((SGE_EVENT, MSG_OBJ_NOSCALING4HOST_SS, name, lGetHost(host, EH_name)));
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         ret = false;
         break;
      }
   }
   DEXIT;
   return ret ? STATUS_OK : STATUS_EUNKNOWN;
}

/****** sge_host_qmaster/host_diff_sublist() ***********************************
*  NAME
*     host_diff_sublist() -- Diff exechost sublists
*
*  SYNOPSIS
*     static void host_diff_sublist(const lListElem *new, const lListElem *old,
*     int snm1, int snm2, int key_nm, const lDescr *dp, lList **new_sublist,
*     lList **old_sublist)
*
*  FUNCTION
*     Makes a diff userset/project sublists of an exec host.
*
*  INPUTS
*     const lListElem *new - New exec host (EH_Type)
*     const lListElem *old - Pld exec host (EH_Type)
*     int snm1             - First exec host sublist field
*     int snm2             - Second exec host sublist field
*     int key_nm           - Field with key in sublist
*     const lDescr *dp     - Type for outgoing sublist arguments
*     lList **new_sublist  - List of new references
*     lList **old_sublist  - List of old references
*
*  NOTES
*     MT-NOTE: host_diff_sublist() is MT safe
*******************************************************************************/
static void host_diff_sublist(const lListElem *new, const lListElem *old,
      int snm1, int snm2, int key_nm, const lDescr *dp,
      lList **new_sublist, lList **old_sublist)
{
   const lListElem *ep;
   const char *p;

   /* collect 'old' entries in 'old_sublist' */
   if (old && old_sublist) {
      for_each (ep, lGetList(old, snm1)) {
         p = lGetString(ep, key_nm);
         if (!lGetElemStr(*old_sublist, key_nm, p))
            lAddElemStr(old_sublist, key_nm, p, dp);
      }
      for_each (ep, lGetList(old, snm2)) {
         p = lGetString(ep, key_nm);
         if (!lGetElemStr(*old_sublist, key_nm, p))
            lAddElemStr(old_sublist, key_nm, p, dp);
      }
   }

   /* collect 'new' entries in 'new_sublist' */
   if (new && new_sublist) {
      for_each (ep, lGetList(new, snm1)) {
         p = lGetString(ep, key_nm);
         if (!lGetElemStr(*new_sublist, key_nm, p))
            lAddElemStr(new_sublist, key_nm, p, dp);
      }
      for_each (ep, lGetList(new, snm2)) {
         p = lGetString(ep, key_nm);
         if (!lGetElemStr(*new_sublist, key_nm, p))
            lAddElemStr(new_sublist, key_nm, p, dp);
      }
   }

   return;
}


/****** sge_host_qmaster/host_diff_projects() **********************************
*  NAME
*     host_diff_projects() -- Diff old/new exec host projects
*
*  SYNOPSIS
*     void host_diff_projects(const lListElem *new, const lListElem *old, lList
*     **new_prj, lList **old_prj)
*
*  FUNCTION
*     A diff new/old is made regarding exec host projects/xprojects.
*     Project references are returned in new_prj/old_prj.
*
*  INPUTS
*     const lListElem *new - New exec host (EH_Type)
*     const lListElem *old - Old exec host (EH_Type)
*     lList **new_prj      - New project references (US_Type)
*     lList **old_prj      - Old project references (US_Type)
*
*  NOTES
*     MT-NOTE: host_diff_projects() is not MT safe
*******************************************************************************/
void host_diff_projects(const lListElem *new,
         const lListElem *old, lList **new_prj, lList **old_prj)
{
   host_diff_sublist(new, old, EH_prj, EH_xprj,
         PR_name, PR_Type, new_prj, old_prj);
   lDiffListStr(PR_name, new_prj, old_prj);
}

/****** sge_host_qmaster/host_diff_usersets() **********************************
*  NAME
*     host_diff_usersets() -- Diff old/new exec host usersets
*
*  SYNOPSIS
*     void host_diff_usersets(const lListElem *new, const lListElem *old, lList
*     **new_acl, lList **old_acl)
*
*  FUNCTION
*     A diff new/old is made regarding exec host acl/xacl.
*     Userset references are returned in new_acl/old_acl.
*
*  INPUTS
*     const lListElem *new - New exec host (EH_Type)
*     const lListElem *old - Old exec host (EH_Type)
*     lList **new_acl      - New userset references (US_Type)
*     lList **old_acl      - Old userset references (US_Type)
*
*  NOTES
*     MT-NOTE: host_diff_usersets() is not MT safe
*******************************************************************************/
void host_diff_usersets(const lListElem *new,
      const lListElem *old, lList **new_acl, lList **old_acl)
{
   host_diff_sublist(new, old, EH_acl, EH_xacl,
         US_name, US_Type, new_acl, old_acl);
   lDiffListStr(US_name, new_acl, old_acl);
}



/****** sge_host_qmaster/host_update_categories() ******************************
*  NAME
*     host_update_categories() --  Update categories wrts userset/project
*
*  SYNOPSIS
*     static void host_update_categories(const lListElem *new_hep, const
*     lListElem *old_hep)
*
*  FUNCTION
*     The userset/project information wrts categories is updated based
*     on new/old exec host configuration and events are sent upon
*     changes.
*
*
*  INPUTS
*     const lListElem *new_hep - New exec host (EH_Type)
*     const lListElem *old_hep - Old exec host (EH_Type)
*
*  NOTES
*     MT-NOTE: host_update_categories() is not MT safe
*******************************************************************************/
static void host_update_categories(const lListElem *new_hep, const lListElem *old_hep)
{
   lList *old = NULL, *new = NULL;

   host_diff_projects(new_hep, old_hep, &new, &old);
   project_update_categories(new, old);
   lFreeList(&old);
   lFreeList(&new);

   host_diff_usersets(new_hep, old_hep, &new, &old);
   userset_update_categories(new, old);
   lFreeList(&old);
   lFreeList(&new);
}

/****** sge_utility_qmaster/attr_mod_threshold() *******************************
*  NAME
*     attr_mod_threshold() -- modify the threshold configuration sublist 
*
*  SYNOPSIS
*     int attr_mod_threshold(lList **alpp, lListElem *ep, lListElem *new_ep, 
*     int sub_command, char *attr_name, char 
*     *object_name) 
*
*  FUNCTION
*   Validation tries to find each element of the ep element in the threshold identified by nm.
*   Elements which already exist here are copied into sublist of new_ep.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx  - gdi context
*     lList **alpp              - The answer list 
*     lListElem *ep            - The source object element 
*     lListElem *new_ep         - The target object element 
*     int sub_command           - The add, modify, remove command 
*     const char *attr_name     - The attribute name 
*     const char *object_name   - The target object name
*
*  RESULT
*     int - 0 if success
*
*  NOTES
*     MT-NOTE: attr_mod_threshold() is MT safe 
*
*******************************************************************************/
static int attr_mod_threshold(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *ep, lListElem *new_ep,
                              int sub_command, char *attr_name, char *object_name) {

   DENTER(TOP_LAYER, "attr_mod_threshold");

   /* ---- attribute EH_consumable_config_list */
   if (lGetPosViaElem(ep, EH_consumable_config_list, SGE_NO_ABORT) >= 0) {
      lListElem *tmp_elem = NULL;

      DPRINTF(("got new %s\n", attr_name));

      /* check if corresponding complex attributes exist */
      if (ensure_attrib_available(alpp, ep, EH_consumable_config_list)) {
         DRETURN(STATUS_EUNKNOWN);
      }

      /* work on a temporary element to allow rollback on error */
      tmp_elem = lCopyElem(new_ep);

      /* the attr_mod_sub_list return boolean and there is stored in the int value, attention true=1 */
      if (!attr_mod_sub_list(alpp, tmp_elem, EH_consumable_config_list, CE_name, ep,
                              sub_command, attr_name, object_name, 0, NULL)) {
         lFreeElem(&tmp_elem);
         DRETURN(STATUS_EUNKNOWN);
      }

      /* the centry_list_fill_request returns 0 if success */
      if (centry_list_fill_request(lGetList(tmp_elem, EH_consumable_config_list), alpp,
                                     *centry_list_get_master_list(), true, false, false)) {
         lFreeElem(&tmp_elem);
         DRETURN(STATUS_EUNKNOWN);
      }
      {
         lListElem *jep = NULL;
         lListElem *ar_ep = NULL;
         const char *host = lGetHost(tmp_elem, EH_name);
         int global_host = !strcmp(SGE_GLOBAL_NAME, host);
         lList *master_centry_list = *object_type_get_master_list(SGE_TYPE_CENTRY);

         lSetList(tmp_elem, EH_resource_utilization, NULL);
         debit_host_consumable(NULL, tmp_elem, master_centry_list, 0, true, NULL);
         for_each (jep, *(object_type_get_master_list(SGE_TYPE_JOB))) {
            lListElem *jatep = NULL;

            for_each (jatep, lGetList(jep, JB_ja_tasks)) {
               lList *gdil = lGetList(jatep, JAT_granted_destin_identifier_list);
               int slots;
               bool is_master_task = false;
               const void *iterator = NULL;

               if (global_host || (lFirst(gdil) == lGetElemHostFirst(gdil, JG_qhostname, host, &iterator))) {
                  is_master_task = true;
               }

               slots = nslots_granted(lGetList(jatep, JAT_granted_destin_identifier_list),
                  global_host?NULL:host);
            
               if (slots > 0) {
                  debit_host_consumable(jep, tmp_elem, master_centry_list, slots, is_master_task, NULL);
               }
            }
         }

         for_each(ar_ep, *object_type_get_master_list(SGE_TYPE_AR)) {
            lList *gdil = lGetList(ar_ep, AR_granted_slots);
            lListElem *gdil_ep = lGetElemHost(gdil, JG_qhostname, host);
            bool is_master_task = false;

            if (gdil_ep == lFirst(gdil)) {
               is_master_task = true;
            }

            if (gdil_ep != NULL) {
               lListElem *dummy_job = lCreateElem(JB_Type);

               lSetList(dummy_job, JB_hard_resource_list, lCopyList("", lGetList(ar_ep, AR_resource_list)));

               rc_add_job_utilization(dummy_job, 0, SCHEDULING_RECORD_ENTRY_TYPE_RESERVING,
                                      tmp_elem, master_centry_list, lGetUlong(gdil_ep, JG_slots),
                                      EH_consumable_config_list, EH_resource_utilization, host,
                                      lGetUlong(ar_ep, AR_start_time), lGetUlong(ar_ep, AR_duration),
                                      HOST_TAG, false, is_master_task);
               lFreeElem(&dummy_job);
            }
         }
      }

      if (ar_list_has_reservation_due_to_host_complex_attr(*object_type_get_master_list(SGE_TYPE_AR), alpp,
                                                           tmp_elem, *object_type_get_master_list(SGE_TYPE_CENTRY))) {
         lFreeElem(&tmp_elem);
         DRETURN(STATUS_EUNKNOWN);
      }

      /* copy back the consumable config and resource utilization lists to new exec host object */
      {
         lList *t = NULL;
         lXchgList(tmp_elem, EH_consumable_config_list, &t);
         lXchgList(new_ep, EH_consumable_config_list, &t);
         lXchgList(tmp_elem, EH_consumable_config_list, &t);

         t = NULL;
         lXchgList(tmp_elem, EH_resource_utilization, &t);
         lXchgList(new_ep, EH_resource_utilization, &t);
         lXchgList(tmp_elem, EH_resource_utilization, &t);
      }

      lFreeElem(&tmp_elem);
   }

   DRETURN(0);
}





