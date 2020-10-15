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

#include "uti/sge_rmon.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_ulong.h"

#include "qrstat_filter.h"
#include "basis_types.h"
#include "sge.h"

void
qrstat_filter_init(qrstat_env_t *qrstat_env)
{
   qrstat_env->ctx = NULL;

   qrstat_env->user_list = NULL;
   qrstat_env->ar_id_list = NULL;
   qrstat_env->is_explain = false;
   qrstat_env->is_xml = false;
   qrstat_env->is_summary = false;
   qrstat_env->header_printed = false;

   qrstat_env->ar_list = NULL;
   qrstat_env->what_AR_Type = NULL;
   qrstat_env->where_AR_Type = NULL;
}

void
qrstat_filter_add_core_attributes(qrstat_env_t *qrstat_env)
{
   lEnumeration *what = NULL;
   const int nm_AR_Type[] = {
      AR_id,
      AR_name,
      AR_owner,
      AR_start_time,
      AR_end_time,
      AR_duration,
      AR_state,
      AR_reserved_queues,
      NoName
   };

   what = lIntVector2What(AR_Type, nm_AR_Type);
   lMergeWhat(&(qrstat_env->what_AR_Type), &what);
}

void
qrstat_filter_add_ar_attributes(qrstat_env_t *qrstat_env)
{
   lEnumeration *what = NULL;
   const int nm_AR_Type[] = {
      AR_account,
      AR_owner,
      AR_group,
      AR_submission_time,
      AR_verify,
      AR_error_handling,
      AR_checkpoint_name,
      AR_resource_list,
      AR_resource_utilization,
      AR_queue_list,
      AR_granted_slots,
      AR_mail_options,
      AR_mail_list,
      AR_pe,
      AR_pe_range,
      AR_master_queue_list,
      AR_acl_list,
      AR_xacl_list,
      AR_type,
      AR_reserved_queues,
      NoName
   };

   what = lIntVector2What(AR_Type, nm_AR_Type);
   lMergeWhat(&(qrstat_env->what_AR_Type), &what);
}

void
qrstat_filter_add_xml_attributes(qrstat_env_t *qrstat_env)
{
   lEnumeration *what = NULL;
   const int nm_AR_Type[] = {
      AR_account,
      AR_owner,
      AR_group,
      AR_submission_time,
      AR_verify,
      AR_error_handling,
      AR_checkpoint_name,
      AR_resource_list,
      AR_resource_utilization,
      AR_queue_list,
      AR_granted_slots,
      AR_mail_options,
      AR_mail_list,
      AR_pe,
      AR_pe_range,
      AR_acl_list,
      AR_xacl_list,
      AR_type,
      NoName
   };

   what = lIntVector2What(AR_Type, nm_AR_Type);
   lMergeWhat(&(qrstat_env->what_AR_Type), &what);
}

void
qrstat_filter_add_explain_attributes(qrstat_env_t *qrstat_env)
{
   lEnumeration *what = NULL;
   const int nm_AR_Type[] = {
      AR_error_handling,
      NoName
   };

   what = lIntVector2What(AR_Type, nm_AR_Type);
   lMergeWhat(&(qrstat_env->what_AR_Type), &what);
}

void
qrstat_filter_add_u_where(qrstat_env_t *qrstat_env)
{
   lCondition *where = NULL;
   lListElem *elem = NULL; /* ST_Type */

   for_each(elem, qrstat_env->user_list) {
      lCondition *tmp_where = NULL;
      const char *name = lGetString(elem, ST_name);

      tmp_where = lWhere("%T(%I p= %s)", AR_Type, AR_owner, name);
      if (tmp_where != NULL) {
         if (where == NULL) {
            where = tmp_where;
         } else {
            where = lOrWhere(where, tmp_where);
         }
      }
   }
   if (where != NULL) {
      if (qrstat_env->where_AR_Type == NULL) {
         qrstat_env->where_AR_Type = where;
      } else {
         qrstat_env->where_AR_Type = lAndWhere(qrstat_env->where_AR_Type, where);
      }
   }
}

void
qrstat_filter_add_ar_where(qrstat_env_t *qrstat_env)
{
   lCondition *where = NULL;
   lListElem *elem = NULL; /* ULNG_Type */

   DENTER(TOP_LAYER, "qrstat_filter_add_ar_where");
   for_each(elem, qrstat_env->ar_id_list) {
      lCondition *tmp_where = NULL;
      u_long32 value = lGetUlong(elem, ULNG_value);

      tmp_where = lWhere("%T(%I == %u)", AR_Type, AR_id, value);
      if (tmp_where != NULL) {
         if (where == NULL) {
            where = tmp_where;
         } else {
            where = lOrWhere(where, tmp_where);
         }
      }
   }
   if (where != NULL) {
      if (qrstat_env->where_AR_Type == NULL) {
         qrstat_env->where_AR_Type = where;
      } else {
         qrstat_env->where_AR_Type = lAndWhere(qrstat_env->where_AR_Type, where);
      }
   }
   DRETURN_VOID;
}

void
qrstat_filter_set_ctx(qrstat_env_t *qrstat_env, sge_gdi_ctx_class_t *ctx) {
   qrstat_env->ctx = ctx;
}

