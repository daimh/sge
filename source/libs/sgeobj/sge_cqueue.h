#ifndef __SGE_CQUEUE_H
#define __SGE_CQUEUE_H

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

#include "sgeobj/sge_cqueue_CQ_L.h"

enum {
   SGE_QI_TAG_DEFAULT = 0,

   /*
    * send delete event and remove from spool area
    */
   SGE_QI_TAG_DEL = 1,

   /*
    * send add events and make persistent
    */
   SGE_QI_TAG_ADD = 2,

   /*
    * send mod event and make persistent
    */
   SGE_QI_TAG_MOD = 4,

   /*
    * send mod event but skip spooling (no state value changed!)
    */
   SGE_QI_TAG_MOD_ONLY_CONFIG = 8
};

typedef struct _list_attribute_struct {
   int cqueue_attr;
   int qinstance_attr;
   int href_attr;
   int value_attr;
   int primary_key_attr;
   const char *name;
   bool is_sgeee_attribute;
   bool verify_client;
   bool (*verify_function)(lListElem *attr_elem, lList **answer_list, lListElem *cqueue);
} list_attribute_struct;

extern list_attribute_struct cqueue_attribute_array[];

lEnumeration *
enumeration_create_reduced_cq(bool fetch_all_qi, bool fetch_all_nqi);

bool
cqueue_name_split(const char *name, dstring *cqueue_name, dstring *host_domain,
                  bool *has_hostname, bool *has_domain);

char* cqueue_get_name_from_qinstance(const char *queue_instance);

lListElem *
cqueue_create(lList **answer_list, const char *name);

bool 
cqueue_is_href_referenced(const lListElem *this_elem, 
                          const lListElem *href, bool only_hostlist);

bool
cqueue_is_a_href_referenced(const lListElem *this_elem, 
                            const lList *href_list, bool only_hostlist);

bool
cqueue_list_add_cqueue(lList *this_list, lListElem *queue);

bool
cqueue_set_template_attributes(lListElem *this_elem, lList **answer_list);

lListElem *
cqueue_list_locate(const lList *this_list, const char *name);

lListElem *
cqueue_locate_qinstance(const lListElem *this_elem, const char *hostname);

lListElem *
cqueue_list_locate_qinstance_msg(lList *cqueue_list, const char *full_name, bool raise_error);

bool
cqueue_list_find_all_matching_references(const lList *this_list,
                                         lList **answer_list,
                                         const char *cqueue_pattern,
                                         lList **qref_list);

bool
cqueue_list_find_hgroup_references(const lList *this_list, 
                                   lList **answer_list,
                                   const lListElem *hgroup, 
                                   lList **string_list);

bool
cqueue_xattr_pre_gdi(lList *this_list, lList **answer_list);

bool
cqueue_verify_attributes(lListElem *cqueue, lList **answer_list,
                         lListElem *reduced_elem, bool in_master);

bool
cqueue_is_used_in_subordinate(const char *cqueue_name, const lListElem *cqueue);

void
cqueue_list_set_tag(lList *this_list, u_long32 tag_value, bool tag_qinstances);

lListElem *
cqueue_list_locate_qinstance(lList *cqueue_list, const char *full_name);

bool
cqueue_find_used_href(lListElem *this_elem, lList **answer_list, 
                      lList **href_list);

bool  
cqueue_trash_used_href_setting(lListElem *this_elem, lList **answer_list,
                               const char *hgroup_or_hostname);

bool
cqueue_purge_host(lListElem *this_elem, lList **answer_list, lList *attr_list, const char *hgroup_or_hostname);

bool
cqueue_sick(lListElem *cqueue, lList **answer_list, lList *hgroup_list, dstring *ds);

#define SGE_ATTR_H_FSIZE               "h_fsize"
#define SGE_ATTR_S_FSIZE               "s_fsize"
#define SGE_ATTR_H_CPU                 "h_cpu"
#define SGE_ATTR_S_CPU                 "s_cpu"
#define SGE_ATTR_H_DATA                "h_data"
#define SGE_ATTR_S_DATA                "s_data"
#define SGE_ATTR_H_STACK               "h_stack"
#define SGE_ATTR_S_STACK               "s_stack"
#define SGE_ATTR_H_CORE                "h_core"
#define SGE_ATTR_S_CORE                "s_core"
#define SGE_ATTR_H_RSS                 "h_rss"
#define SGE_ATTR_S_RSS                 "s_rss"
#define SGE_ATTR_H_VMEM                "h_vmem"
#define SGE_ATTR_S_VMEM                "s_vmem"

#define SGE_ATTR_QTYPE                 "qtype"
#define SGE_ATTR_SEQ_NO                "seq_no"
#define SGE_ATTR_LOAD_THRESHOLD        "load_thresholds"
#define SGE_ATTR_SUSPEND_THRESHOLD     "suspend_thresholds"
#define SGE_ATTR_NSUSPEND              "nuspend"
#define SGE_ATTR_SUSPEND_INTERVAL      "suspend_interval"
#define SGE_ATTR_PRIORITY              "priority"
#define SGE_ATTR_MIN_CPU_INTERVAL      "min_cpu_interval"
#define SGE_ATTR_PROCESSORS            "processors"
#define SGE_ATTR_RERUN                 "rerun"
#define SGE_ATTR_TMPDIR                "tmpdir"
#define SGE_ATTR_SHELL                 "shell"
#define SGE_ATTR_SHELL_START_MODE      "shell_start_mode"
#define SGE_ATTR_PROLOG                "prolog"
#define SGE_ATTR_EPILOG                "epilog"
#define SGE_ATTR_STARTER_METHOD        "starter_method"
#define SGE_ATTR_SUSPEND_METHOD        "suspend_method"
#define SGE_ATTR_RESUME_METHOD         "resume_method"
#define SGE_ATTR_TERMINATE_METHOD      "terminate_method"
#define SGE_ATTR_NOTIFY                "notify"
#define SGE_ATTR_OWNER_LIST            "owner_list"
#define SGE_ATTR_CALENDAR              "calendar"
#define SGE_ATTR_INITIAL_STATE         "initial_state"
#define SGE_ATTR_QNAME                 "qname"
#define SGE_ATTR_SUBORDINATE_LIST      "subordinate_list"
#define SGE_ATTR_QUEUE_LIST            "queue_list"
#define SGE_ATTR_HOSTNAME              "hostname"
#define SGE_ATTR_PE_NAME               "pe_name"
#define SGE_ATTR_CKPT_NAME             "ckpt_name"
#define SGE_ATTR_HGRP_NAME             "group_name"
#define SGE_ATTR_RQS_NAME              "name"
#define SGE_ATTR_HOSTLIST              "hostlist"
#define SGE_ATTR_RQSRULES              "resource_quota_rules"
#define SGE_ATTR_PROJECTS              "projects"
#define SGE_ATTR_XPROJECTS             "xprojects"
#define SGE_ATTR_PE_LIST               "pe_list"
#define SGE_ATTR_HOST_LIST             "hostlist"
#define SGE_ATTR_CKPT_LIST             "ckpt_list"
#define SGE_ATTR_USER_NAME             "name"
#define SGE_RQS_NAME                   "resource_quota"
#define SGE_ATTR_PROJECT_NAME          "name"
#define SGE_ATTR_CALENDAR_NAME         "calendar_name"
#define SGE_ATTR_USERSET_NAME          "name"
#define SGE_UNKNOWN_NAME               "unknown"
#define SGE_OBJ_JOB                    "job"
#define SGE_OBJ_AR                     "advance_reservation"
#define SGE_OBJ_RQS                    "resource_quota"
#define SGE_OBJ_CKPT                   "ckpt"
#define SGE_OBJ_CQUEUE                 "queue"
#define SGE_OBJ_HGROUP                 "hostgroup"
#define SGE_OBJ_EXECHOST               "exechost"
#define SGE_OBJ_PE                     "pe"
#define SGE_OBJ_USER                   "user"
#define SGE_OBJ_PROJECT                "project"
#define SGE_OBJ_CALENDAR               "calendar"
#define SGE_OBJ_USERSET                "userset"

#endif /* __SGE_CQUEUE_H */
