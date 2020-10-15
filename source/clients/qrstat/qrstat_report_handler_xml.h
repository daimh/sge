#ifndef __SGE_QRSTAT_REPORT_HANDLER_XML_H
#define __SGE_QRSTAT_REPORT_HANDLER_XML_H
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
      
#ifdef  __cplusplus
extern "C" {
#endif

#include "basis_types.h"
#include "gdi/sge_gdi_ctx.h"
      
qrstat_report_handler_t *
qrstat_create_report_handler_xml(qrstat_env_t *qrstat_env, 
                                 lList **answer_list);

bool
qrstat_destroy_report_handler_xml(qrstat_report_handler_t** handler, 
                                  lList **answer_list);

bool
qrstat_xml_report_start(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_start_ar(qrstat_report_handler_t* handler, qrstat_env_t *qrstat_env, lList **alpp);

bool
qrstat_xml_report_finish_ar(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_ar_node_ulong(qrstat_report_handler_t* handler, lList **alpp,
                                const char *name, u_long32 value);

bool
qrstat_xml_report_ar_node_string(qrstat_report_handler_t* handler, lList **alpp,
                                 const char *name, const char *value);

bool
qrstat_xml_report_ar_node_time(qrstat_report_handler_t* handler, lList **alpp,
                               const char *name, time_t value);

bool
qrstat_xml_report_ar_node_state(qrstat_report_handler_t* handler, lList **alpp,
                                const char *name, u_long32 value);

bool
qrstat_xml_report_ar_node_boolean(qrstat_report_handler_t* handler, lList **alpp,
                                const char *name, bool value);

bool
qrstat_xml_report_start_resource_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_resource_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_resource_list_node(qrstat_report_handler_t* handler, lList **alpp,
                                     const char *name, const char *value);

bool
qrstat_xml_report_start_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_granted_slots_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_granted_slots_list_node(qrstat_report_handler_t* handler, 
                                          lList **alpp,
                                          const char *name, u_long32 value);

bool
qrstat_xml_report_start_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_granted_parallel_environment(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_granted_parallel_environment_node(qrstat_report_handler_t* handler, 
                                                    lList **alpp,
                                                    const char *name, const char *slots_range);

bool
qrstat_xml_report_start_mail_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_mail_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_mail_list_node(qrstat_report_handler_t* handler, 
                                 lList **alpp,
                                 const char *name, const char *hostname);

bool
qrstat_xml_report_start_acl_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_acl_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_acl_list_node(qrstat_report_handler_t* handler, 
                                 lList **alpp, const char *name);

bool
qrstat_xml_report_start_xacl_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_finish_xacl_list(qrstat_report_handler_t* handler, lList **alpp);

bool
qrstat_xml_report_xacl_list_node(qrstat_report_handler_t* handler, 
                                 lList **alpp, const char *name);

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_QRSTAT_REPORT_HANDLER_XML_H */
