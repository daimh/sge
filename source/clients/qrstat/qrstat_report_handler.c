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

#include "sgeobj/sge_str.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_mailrec.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_mesobj.h"
#include "sgeobj/cull_parse_util.h"

#include "basis_types.h"
#include "sge.h"
#include "qrstat_report_handler.h"
#include "msg_common.h"

bool
qrstat_print(lList **answer_list, qrstat_report_handler_t *handler, qrstat_env_t *qrstat_env) {
   bool ret = true;

   DENTER(TOP_LAYER, "qrstat_print");

   {
      lListElem *ar = NULL;

      handler->report_start(handler, answer_list);
      if (qrstat_env->is_summary == false &&
          lGetNumberOfElem(qrstat_env->ar_list) == 0) {

          ret = false;

          for_each(ar, qrstat_env->ar_id_list) {
            handler->report_start_unknown_ar(handler, qrstat_env, answer_list);
            handler->report_ar_node_ulong_unknown(handler, qrstat_env, answer_list, "id", lGetUlong(ar, ULNG_value));
            handler->report_finish_unknown_ar(handler, answer_list);
          }
          handler->report_newline(handler, answer_list);
      } else {
         for_each(ar, qrstat_env->ar_list) {

            handler->report_start_ar(handler, qrstat_env, answer_list);
            handler->report_ar_node_ulong(handler, qrstat_env, answer_list, "id", lGetUlong(ar, AR_id));
            handler->report_ar_node_string(handler, answer_list, "name", lGetString(ar, AR_name));
            handler->report_ar_node_string(handler, answer_list, "owner", lGetString(ar, AR_owner));
            handler->report_ar_node_state(handler, answer_list, "state", lGetUlong(ar, AR_state));         
            handler->report_ar_node_time(handler, answer_list, "start_time",
                                         ((time_t)lGetUlong(ar, AR_start_time)));
            handler->report_ar_node_time(handler, answer_list, "end_time",
                                         ((time_t)lGetUlong(ar, AR_end_time)));
            handler->report_ar_node_duration(handler, answer_list, "duration", lGetUlong(ar, AR_duration));

            if (qrstat_env->is_explain || handler->show_summary == false) {
               lListElem *qinstance;
               for_each(qinstance, lGetList(ar, AR_reserved_queues)) {
                  lListElem *qim = NULL;
                  for_each(qim, lGetList(qinstance, QU_message_list)) {
                     const char *message = lGetString(qim, QIM_message);
                     handler->report_ar_node_string(handler, answer_list, "message", message);
                  }
               }
            }
            if (handler->show_summary == false) {
               handler->report_ar_node_time(handler, answer_list, "submission_time",
                                         ((time_t)lGetUlong(ar, AR_submission_time)));
               handler->report_ar_node_string(handler, answer_list, "group", lGetString(ar, AR_group));
               handler->report_ar_node_string(handler, answer_list, "account", lGetString(ar, AR_account));

               if (lGetList(ar, AR_resource_list) != NULL) {
                  lListElem *resource = NULL;

                  handler->report_start_resource_list(handler, answer_list);            
                  for_each(resource, lGetList(ar, AR_resource_list)) {
                     dstring string_value = DSTRING_INIT;

                     if (lGetString(resource, CE_stringval)) {
                        sge_dstring_append(&string_value, lGetString(resource, CE_stringval));
                     } else {
                        sge_dstring_sprintf(&string_value, "%f", lGetDouble(resource, CE_doubleval));
                     }

                     handler->report_resource_list_node(handler, answer_list,
                                                        lGetString(resource, CE_name),
                                                        sge_dstring_get_string(&string_value));
                     sge_dstring_free(&string_value);
                  }
                  handler->report_finish_resource_list(handler, answer_list);
               }
               
               if (lGetUlong(ar, AR_error_handling) != 0) {
                  handler->report_ar_node_boolean(handler, answer_list, "error_handling", true);
               }
               
               if (lGetList(ar, AR_granted_slots) != NULL) {
                  lListElem *resource = NULL;

                  handler->report_start_granted_slots_list(handler, answer_list);
                  for_each(resource, lGetList(ar, AR_granted_slots)) {
                     handler->report_granted_slots_list_node(handler, answer_list,
                                                             lGetString(resource, JG_qname),
                                                             lGetUlong(resource, JG_slots));
                  }
                  handler->report_finish_granted_slots_list(handler, answer_list);
               }
               if (lGetString(ar, AR_pe) != NULL) {
                  dstring pe_range_string = DSTRING_INIT;

                  range_list_print_to_string(lGetList(ar, AR_pe_range), &pe_range_string, true, false, false);
                  handler->report_start_granted_parallel_environment(handler, answer_list);
                  handler->report_granted_parallel_environment_node(handler, answer_list,
                                                                    lGetString(ar, AR_pe),
                                                                    sge_dstring_get_string(&pe_range_string));
                  handler->report_finish_granted_parallel_environment(handler, answer_list);
                  sge_dstring_free(&pe_range_string);
               }
               if (lGetList(ar, AR_master_queue_list) != NULL) {
                  char tmp_buffer[MAX_STRING_SIZE];
                  int fields[] = {QR_name, 0 };
                  const char *delis[] = {" ", ",", ""};
                  uni_print_list(NULL, tmp_buffer, sizeof(tmp_buffer), lGetList(ar, AR_master_queue_list), 
                                 fields, delis, FLG_NO_DELIS_STRINGS);
                  handler->report_ar_node_string(handler, answer_list, "master hard queue_list", tmp_buffer);
               }

               if (lGetString(ar, AR_checkpoint_name) != NULL) {
                  handler->report_ar_node_string(handler, answer_list, "checkpoint_name",
                                                 lGetString(ar, AR_checkpoint_name));
               }

               if (lGetUlong(ar, AR_mail_options) != 0) {
                  dstring mailopt = DSTRING_INIT;

                  sge_dstring_append_mailopt(&mailopt, lGetUlong(ar, AR_mail_options));
                  handler->report_ar_node_string(handler, answer_list, "mail_options",
                                                 sge_dstring_get_string(&mailopt));

                  sge_dstring_free(&mailopt);
               }

               if (lGetList(ar, AR_mail_list) != NULL) {
                  lListElem *mail = NULL;

                  handler->report_start_mail_list(handler, answer_list);
                  for_each(mail, lGetList(ar, AR_mail_list)) {
                     const char *host=NULL;
                     host=lGetHost(mail, MR_host);
                     handler->report_mail_list_node(handler, answer_list,
                                                    lGetString(mail, MR_user),
                                                    host?host:"NONE");
                  }
                  handler->report_finish_mail_list(handler, answer_list);
               }
               if (lGetList(ar, AR_acl_list) != NULL) {
                  lListElem *acl = NULL;

                  handler->report_start_acl_list(handler, answer_list);
                  for_each(acl, lGetList(ar, AR_acl_list)) {
                     handler->report_acl_list_node(handler, answer_list,
                                                    lGetString(acl, ARA_name));
                  }
                  handler->report_finish_acl_list(handler, answer_list);
               }
               if (lGetList(ar, AR_xacl_list) != NULL) {
                  lListElem *xacl = NULL;

                  handler->report_start_xacl_list(handler, answer_list);
                  for_each(xacl, lGetList(ar, AR_xacl_list)) {
                     handler->report_xacl_list_node(handler, answer_list,
                                                    lGetString(xacl, ARA_name));
                  }
                  handler->report_finish_xacl_list(handler, answer_list);
               }
            }
            handler->report_finish_ar(handler, answer_list);
         }
      }

      handler->report_finish(handler, answer_list);
   }
   DRETURN(ret);
}

