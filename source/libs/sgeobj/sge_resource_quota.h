#ifndef __SGE_RESOURCE_QUOTA_H 
#define __SGE_RESOURCE_QUOTA_H 
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

#include "sgeobj/sge_resource_quota_RQS_L.h"
#include "sgeobj/sge_resource_quota_RQR_L.h"
#include "sgeobj/sge_resource_quota_RQRF_L.h"
#include "sgeobj/sge_resource_quota_RQRL_L.h"
#include "sgeobj/sge_resource_quota_RQL_L.h"

enum {
   FILTER_USERS = 0,
   FILTER_PROJECTS,
   FILTER_PES,
   FILTER_QUEUES,
   FILTER_HOSTS
};

/* values found in RQR_level */
enum {
   RQR_ALL = 0,
   RQR_GLOBAL,
   RQR_CQUEUE,
   RQR_HOST,
   RQR_QUEUEI
};

bool rqs_parse_filter_from_string(lListElem **filter, const char* buffer, lList **alp);
bool rqs_append_filter_to_dstring(const lListElem *filter, dstring *buffer, lList **alp);

lListElem* rqs_set_defaults(lListElem* rqs);

bool rqs_verify_attributes(lListElem *rqs, lList **answer_list, bool in_master);
bool rqs_list_verify_attributes(lList *rqs_list, lList **answer_list, bool in_master);

lListElem *rqs_list_locate(lList *lp, const char *name);
lListElem *rqs_rule_locate(lList *lp, const char *name);

bool rqs_xattr_pre_gdi(lList *this_list, lList **answer_list);

bool rqs_get_rue_string(dstring *name, const lListElem *rule, const char *user, const char *project, const char *host, const char *queue, const char* pe);

int
rqs_debit_rule_usage(lListElem *job, lListElem *rule, dstring *rue_name, int slots, lList *centry_list, const char *obj_name, bool is_master_task);

int
rqs_debit_consumable(lListElem *rqs, lListElem *job, lListElem *granted, const char *pename, lList *centry_list, lList *acl_list, lList *hgrp_list, int slots, bool is_master_task);

lListElem *rqs_get_matching_rule(const lListElem *rqs, const char *user, const char *group, const char *project, const char *pe, const char *host, const char *queue, lList *userset_list, lList* hgroup_list, dstring *rule_name);

bool rqs_is_matching_rule(lListElem *rule, const char *user, const char *group, const char *project, const char *pe, const char *host, const char *queue, lList *master_userset_list, lList *master_hgroup_list);

bool sge_centry_referenced_in_rqs(const lListElem *rqs, const lListElem *centry);

bool rqs_replace_request_verify(lList **answer_list, const lList *request);

bool rqs_filter_match(lListElem *filter, int filter_type, const char *value, lList *master_userset_list, lList *master_hgroup_list, const char *group);

#endif /* __SGE_RESOURCE_QUOTA_H */
