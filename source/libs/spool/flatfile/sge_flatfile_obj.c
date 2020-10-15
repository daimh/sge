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
#include <strings.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_resource_utilization_RUE_L.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_attr.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_ckpt.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_cuser.h"
#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_host.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_pe.h"
#include "sgeobj/sge_resource_quota.h"
#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_range.h"
#include "sgeobj/sge_schedd_conf.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_str.h"
#include "sgeobj/sge_subordinate.h"
#include "sgeobj/sge_usage.h"
#include "sgeobj/sge_userprj.h"
#include "sgeobj/sge_userset.h"
#include "sgeobj/sge_advance_reservation.h"
#include "sgeobj/sge_qref.h"
#include "sgeobj/sge_job.h"
#include "sgeobj/sge_mailrec.h"

#include "spool/flatfile/sge_flatfile.h"
#include "spool/flatfile/msg_spoollib_flatfile.h"

#include "msg_common.h"

/* This file defines variables and functions that are used to create field
 * lists to pass to the flatfile spooling framework.  The reason that some
 * field lists are represented as structs and some are represented as
 * functions is that some field lists are always that same, while others
 * change depending on the context from which they're called. */

/* read_func's and write_func's -- signature is defined in
 * sge_spooling_utilities.h:spooling_field. */
static void create_spooling_field (
   spooling_field *field,
   int nm, 
   int width, 
   const char *name, 
   struct spooling_field *sub_fields, 
   const void *clientdata, 
   int (*read_func) (lListElem *ep, int nm, const char *buffer, lList **alp), 
   int (*write_func) (const lListElem *ep, int nm, dstring *buffer, lList **alp)
);
static int read_SC_queue_sort_method(lListElem *ep, int nm,
                                     const char *buffer, lList **alp);
static int write_SC_queue_sort_method(const lListElem *ep, int nm,
                                      dstring *buffer, lList **alp);
static int read_CF_value(lListElem *ep, int nm, const char *buf,
                         lList **alp);
static int read_CQ_ulng_attr_list(lListElem *ep, int nm, const char *buffer,
                                  lList **alp);
static int write_CQ_ulng_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp);
static int read_CQ_mem_attr_list(lListElem *ep, int nm, const char *buffer,
                                 lList **alp);
static int write_CQ_mem_attr_list(const lListElem *ep, int nm,
                                  dstring *buffer, lList **alp);
static int read_CQ_time_attr_list(lListElem *ep, int nm, const char *buffer,
                                  lList **alp);
static int write_CQ_time_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp);
static int read_CQ_prjlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp);
static int write_CQ_prjlist_attr_list(const lListElem *ep, int nm,
                                      dstring *buffer, lList **alp);
static int read_CQ_solist_attr_list(lListElem *ep, int nm, const char *buffer,
                                    lList **alp);
static int write_CQ_solist_attr_list(const lListElem *ep, int nm,
                                     dstring *buffer, lList **alp);
static int read_CQ_usrlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp);
static int write_CQ_usrlist_attr_list(const lListElem *ep, int nm,
                                      dstring *buffer, lList **alp);
static int read_CQ_bool_attr_list(lListElem *ep, int nm, const char *buffer,
                                  lList **alp);
static int write_CQ_bool_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp);
static int read_CQ_strlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp);
static int write_CQ_strlist_attr_list(const lListElem *ep, int nm,
                                      dstring *buffer, lList **alp);
static int read_CQ_qtlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                    lList **alp);
static int write_CQ_qtlist_attr_list(const lListElem *ep, int nm,
                                     dstring *buffer, lList **alp);
static int read_CQ_str_attr_list(lListElem *ep, int nm, const char *buffer,
                                 lList **alp);
static int write_CQ_str_attr_list(const lListElem *ep, int nm,
                                  dstring *buffer, lList **alp);
static int read_CQ_inter_attr_list(lListElem *ep, int nm, const char *buffer,
                                   lList **alp);
static int write_CQ_inter_attr_list(const lListElem *ep, int nm,
                                    dstring *buffer, lList **alp);
static int read_CQ_celist_attr_list(lListElem *ep, int nm, const char *buffer,
                                    lList **alp);
static int write_CQ_celist_attr_list(const lListElem *ep, int nm,
                                     dstring *buffer, lList **alp);
static int read_CQ_hostlist(lListElem *ep, int nm, const char *buffer,
                            lList **alp);
static int write_CQ_hostlist(const lListElem *ep, int nm,
                             dstring *buffer, lList **alp);
static int write_CE_stringval(const lListElem *ep, int nm, dstring *buffer,
                       lList **alp);
static int read_RQR_obj(lListElem *ep, int nm, const char *buffer,
                                    lList **alp);
static int write_RQR_obj(const lListElem *ep, int nm, dstring *buffer,
                       lList **alp);

/* Field lists for context-independent spooling of sub-lists */
static spooling_field AMEM_sub_fields[] = {
   {  AMEM_href,           0, NULL,                NULL, NULL, NULL, NULL},
   {  AMEM_value,          0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ATIME_sub_fields[] = {
   {  ATIME_href,          0, NULL,                NULL, NULL, NULL, NULL},
   {  ATIME_value,         0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field PR_sub_fields[] = {
   {  PR_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field APRJLIST_sub_fields[] = {
   {  APRJLIST_href,       0, NULL,                NULL, NULL, NULL, NULL},
   {  APRJLIST_value,      0, NULL,                PR_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field SO_sub_fields[] = {
   {  SO_name,            11, NULL, NULL, NULL, NULL, NULL},
   {  SO_threshold,       11, NULL, NULL, NULL, NULL, NULL},
   {  SO_slots_sum,       11, NULL, NULL, NULL, NULL, NULL},
   {  SO_seq_no,          11, NULL, NULL, NULL, NULL, NULL},
   {  SO_action,          11, NULL, NULL, NULL, NULL, NULL},
   {  NoName,             11, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field ASOLIST_sub_fields[] = {
   {  ASOLIST_href,        0, NULL,                NULL, NULL, NULL, NULL},
   {  ASOLIST_value,       0, NULL,                SO_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field US_sub_fields[] = {
   {  US_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field AUSRLIST_sub_fields[] = {
   {  AUSRLIST_href,       0, NULL,                NULL, NULL, NULL, NULL},
   {  AUSRLIST_value,      0, NULL,                US_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ABOOL_sub_fields[] = {
   {  ABOOL_href,          0, NULL,                NULL, NULL, NULL, NULL},
   {  ABOOL_value,         0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ST_sub_fields[] = {
   {  ST_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ASTRLIST_sub_fields[] = {
   {  ASTRLIST_href,       0, NULL,                NULL, NULL, NULL, NULL},
   {  ASTRLIST_value,      0, NULL,                ST_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field AQTLIST_sub_fields[] = {
   {  AQTLIST_href,        0, NULL,                NULL, NULL, NULL, NULL},
   {  AQTLIST_value,       0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ASTR_sub_fields[] = {
   {  ASTR_href,           0, NULL,                NULL, NULL, NULL, NULL},
   {  ASTR_value,          0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field AINTER_sub_fields[] = {
   {  AINTER_href,         0, NULL,                NULL, NULL, NULL, NULL},
   {  AINTER_value,        0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field CE_sub_fields[] = {
   {  CE_name,            11, "name",    NULL, NULL, NULL, NULL},
   {  CE_stringval,       11, "value",   NULL, NULL, NULL, write_CE_stringval},
   {  NoName,             11, NULL,      NULL, NULL, NULL, NULL}
};

static spooling_field RUE_sub_fields[] = {
   {  RUE_name,           11, NULL,   NULL, NULL, NULL, NULL},
   {  RUE_utilized_now,   11, NULL,   NULL, NULL, NULL, NULL},
   {  NoName,             11, NULL,   NULL, NULL, NULL, NULL}
};

static spooling_field ACELIST_sub_fields[] = {
   {  ACELIST_href,        0, NULL, NULL, NULL, NULL, NULL},
   {  ACELIST_value,       0, NULL, CE_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field AULNG_sub_fields[] = {
   {  AULNG_href,          0, NULL, NULL, NULL, NULL, NULL},
   {  AULNG_value,         0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field CF_sub_fields[] = {
   {  CF_name,             28, NULL, NULL, NULL, NULL, NULL},
   {  CF_value,            28, NULL, NULL, NULL, read_CF_value, NULL},
   {  NoName,              28, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field STN_sub_fields[] = {
   {  STN_id,              0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field HS_sub_fields[] = {
   {  HS_name,             0, NULL, NULL, NULL, NULL, NULL},
   {  HS_value,            0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field RU_sub_fields[] = {
   {  RU_job_number,         0, NULL, NULL, NULL, NULL, NULL},
   {  RU_task_number,        0, NULL, NULL, NULL, NULL, NULL},
   {  RU_state,              0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,                0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field HL_sub_fields[] = {
   {  HL_name,             0, NULL, NULL, NULL, NULL, NULL},
   {  HL_value,            0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field STU_sub_fields[] = {
   {  STU_name,            0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field UE_sub_fields[] = {
   {  UE_name,             0, NULL, NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL, NULL, NULL, NULL, NULL}
};

static spooling_field UA_sub_fields[] = {
   {  UA_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  UA_value,            0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field UPP_sub_fields[] = {
   {  UPP_name,            0, NULL,                NULL, NULL, NULL, NULL},
   {  UPP_usage,           0, NULL,                UA_sub_fields, NULL, NULL, NULL},
   {  UPP_long_term_usage, 0, NULL,                UA_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field UPU_sub_fields[] = {
   {  UPU_job_number,      0, NULL,                NULL, NULL, NULL, NULL},
   {  UPU_old_usage_list,  0, NULL,                UA_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field HR_sub_fields[] = {
   {  HR_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field RQRL_sub_fields[] = {
   {  RQRL_name,           0, NULL,                NULL, NULL, NULL, NULL},
   {  RQRL_value,          0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field RQR_sub_fields[] = {
   {  RQR_name,            0, "name",              NULL, NULL, NULL, NULL},
   {  RQR_filter_users,    0, "users",             NULL, NULL, read_RQR_obj, write_RQR_obj},
   {  RQR_filter_projects, 0, "projects",          NULL, NULL, read_RQR_obj, write_RQR_obj},
   {  RQR_filter_pes,      0, "pes",               NULL, NULL, read_RQR_obj, write_RQR_obj},
   {  RQR_filter_queues,   0, "queues",            NULL, NULL, read_RQR_obj, write_RQR_obj},
   {  RQR_filter_hosts,    0, "hosts",             NULL, NULL, read_RQR_obj, write_RQR_obj},
   {  RQR_limit,           0, "to",                RQRL_sub_fields,  &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

/* Field lists for context-independent object spooling */
spooling_field CE_fields[] = {
   {  CE_name,            11, "name",          NULL, NULL, NULL, NULL},
   {  CE_shortcut,        11, "shortcut",      NULL, NULL, NULL, NULL},
   {  CE_valtype,         11, "type",          NULL, NULL, NULL, NULL},
   {  CE_relop,           11, "relop",         NULL, NULL, NULL, NULL},
   {  CE_requestable,     11, "requestable",   NULL, NULL, NULL, NULL},
   {  CE_consumable,      11, "consumable",    NULL, NULL, NULL, NULL},
   {  CE_default,         11, "default",       NULL, NULL, NULL, NULL},
   {  CE_urgency_weight,  11, "urgency",       NULL, NULL, NULL, NULL},
   {  NoName,             11, NULL,            NULL, NULL, NULL, NULL}
};

spooling_field CAL_fields[] = {
   {  CAL_name,           16, "calendar_name", NULL, NULL, NULL, NULL},
   {  CAL_year_calendar,  16, "year",          NULL, NULL, NULL, NULL},
   {  CAL_week_calendar,  16, "week",          NULL, NULL, NULL, NULL},
   {  NoName,             16, NULL,            NULL, NULL, NULL, NULL}
};

spooling_field CK_fields[] = {
   {  CK_name,            18, "ckpt_name",        NULL, NULL, NULL, NULL},
   {  CK_interface,       18, "interface",        NULL, NULL, NULL, NULL},
   {  CK_ckpt_command,    18, "ckpt_command",     NULL, NULL, NULL, NULL},
   {  CK_migr_command,    18, "migr_command",     NULL, NULL, NULL, NULL},
   {  CK_rest_command,    18, "restart_command",  NULL, NULL, NULL, NULL},
   {  CK_clean_command,   18, "clean_command",    NULL, NULL, NULL, NULL},
   {  CK_ckpt_dir,        18, "ckpt_dir",         NULL, NULL, NULL, NULL},
   {  CK_signal,          18, "signal",           NULL, NULL, NULL, NULL},
   {  CK_when,            18, "when",             NULL, NULL, NULL, NULL},
   {  NoName,             18, NULL,               NULL, NULL, NULL, NULL}
};

spooling_field HGRP_fields[] = {
   {  HGRP_name,           0, "group_name",        NULL, NULL, NULL, NULL},
   {  HGRP_host_list,      0, "hostlist",          HR_sub_fields, NULL, NULL, NULL},
   {  NoName,              0, NULL,                NULL, NULL, NULL, NULL}
};

spooling_field US_fields[] = {
   {  US_name,    7, "name",    NULL,          NULL, NULL, NULL},
   {  US_type,    7, "type",    NULL,          NULL, NULL, NULL},
   {  US_fshare,  7, "fshare",  US_sub_fields, NULL, NULL, NULL},
   {  US_oticket, 7, "oticket", US_sub_fields, NULL, NULL, NULL},
   {  US_entries, 7, "entries", UE_sub_fields, NULL, NULL, NULL},
   {  NoName,     7, NULL,      NULL,          NULL, NULL, NULL}
};
   
spooling_field SC_fields[] = {
   {  SC_algorithm,                       33, "algorithm",                       NULL,          NULL, NULL,                      NULL},
   {  SC_schedule_interval,               33, "schedule_interval",               NULL,          NULL, NULL,                      NULL},
   {  SC_maxujobs,                        33, "maxujobs",                        NULL,          NULL, NULL,                      NULL},
   {  SC_queue_sort_method,               33, "queue_sort_method",               NULL,          NULL, read_SC_queue_sort_method, write_SC_queue_sort_method},
   {  SC_job_load_adjustments,            33, "job_load_adjustments",            CE_sub_fields, NULL, NULL,                      NULL},
   {  SC_load_adjustment_decay_time,      33, "load_adjustment_decay_time",      CE_sub_fields, NULL, NULL,                      NULL},
   {  SC_load_formula,                    33, "load_formula",                    NULL,          NULL, NULL,                      NULL},
   {  SC_schedd_job_info,                 33, "schedd_job_info",                 NULL,          NULL, NULL,                      NULL},
   {  SC_flush_submit_sec,                33, "flush_submit_sec",                NULL,          NULL, NULL,                      NULL},
   {  SC_flush_finish_sec,                33, "flush_finish_sec",                NULL,          NULL, NULL,                      NULL},
   {  SC_params,                          33, "params",                          NULL,          NULL, NULL,                      NULL},
   {  SC_reprioritize_interval,           33, "reprioritize_interval",           NULL,          NULL, NULL,                      NULL},
   {  SC_halftime,                        33, "halftime",                        NULL,          NULL, NULL,                      NULL},
   {  SC_usage_weight_list,               33, "usage_weight_list",               UA_sub_fields, NULL, NULL,                      NULL},
   {  SC_compensation_factor,             33, "compensation_factor",             NULL,          NULL, NULL,                      NULL},
   {  SC_weight_user,                     33, "weight_user",                     NULL,          NULL, NULL,                      NULL},
   {  SC_weight_project,                  33, "weight_project",                  NULL,          NULL, NULL,                      NULL},
   {  SC_weight_department,               33, "weight_department",               NULL,          NULL, NULL,                      NULL},
   {  SC_weight_job,                      33, "weight_job",                      NULL,          NULL, NULL,                      NULL},
   {  SC_weight_tickets_functional,       33, "weight_tickets_functional",       NULL,          NULL, NULL,                      NULL},
   {  SC_weight_tickets_share,            33, "weight_tickets_share",            NULL,          NULL, NULL,                      NULL},
   {  SC_share_override_tickets,          33, "share_override_tickets",          NULL,          NULL, NULL,                      NULL},
   {  SC_share_functional_shares,         33, "share_functional_shares",         NULL,          NULL, NULL,                      NULL},
   {  SC_max_functional_jobs_to_schedule, 33, "max_functional_jobs_to_schedule", NULL,          NULL, NULL,                      NULL},
   {  SC_report_pjob_tickets,             33, "report_pjob_tickets",             NULL,          NULL, NULL,                      NULL},
   {  SC_max_pending_tasks_per_job,       33, "max_pending_tasks_per_job",       NULL,          NULL, NULL,                      NULL},
   {  SC_halflife_decay_list,             33, "halflife_decay_list",             NULL,          NULL, NULL,                      NULL},
   {  SC_policy_hierarchy,                33, "policy_hierarchy",                NULL,          NULL, NULL,                      NULL},
   {  SC_weight_ticket,                   33, "weight_ticket",                   NULL,          NULL, NULL,                      NULL},
   {  SC_weight_waiting_time,             33, "weight_waiting_time",             NULL,          NULL, NULL,                      NULL},
   {  SC_weight_deadline,                 33, "weight_deadline",                 NULL,          NULL, NULL,                      NULL},
   {  SC_weight_urgency,                  33, "weight_urgency",                  NULL,          NULL, NULL,                      NULL},
   {  SC_weight_priority,                 33, "weight_priority",                 NULL,          NULL, NULL,                      NULL},
   {  SC_max_reservation,                 33, "max_reservation",                 NULL,          NULL, NULL,                      NULL},
   {  SC_default_duration,                33, "default_duration",                NULL,          NULL, NULL,                      NULL},
   {  NoName,                             33, NULL,                              NULL,          NULL, NULL,                      NULL}
};

spooling_field CQ_fields[] = {
   {  CQ_name,                   21, "qname",              NULL,                NULL,                                   NULL,                      NULL},
   {  CQ_hostlist,               21, "hostlist",           HR_sub_fields,       NULL,                                   read_CQ_hostlist,          write_CQ_hostlist},
   {  CQ_seq_no,                 21, "seq_no",             AULNG_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_ulng_attr_list,    write_CQ_ulng_attr_list},
   {  CQ_load_thresholds,        21, "load_thresholds",    ACELIST_sub_fields,  &qconf_sub_name_value_comma_braced_sfi, read_CQ_celist_attr_list,  write_CQ_celist_attr_list},
   {  CQ_suspend_thresholds,     21, "suspend_thresholds", ACELIST_sub_fields,  &qconf_sub_name_value_comma_braced_sfi, read_CQ_celist_attr_list,  write_CQ_celist_attr_list},
   {  CQ_nsuspend,               21, "nsuspend",           AULNG_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_ulng_attr_list,    write_CQ_ulng_attr_list},
   {  CQ_suspend_interval,       21, "suspend_interval",   AINTER_sub_fields,   &qconf_sub_name_value_comma_braced_sfi, read_CQ_inter_attr_list,   write_CQ_inter_attr_list},
   {  CQ_priority,               21, "priority",           ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_min_cpu_interval,       21, "min_cpu_interval",   AINTER_sub_fields,   &qconf_sub_name_value_comma_braced_sfi, read_CQ_inter_attr_list,   write_CQ_inter_attr_list},
   {  CQ_processors,             21, "processors",         ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_qtype,                  21, "qtype",              AQTLIST_sub_fields,  &qconf_sub_name_value_comma_braced_sfi, read_CQ_qtlist_attr_list,  write_CQ_qtlist_attr_list},
   {  CQ_ckpt_list,              21, "ckpt_list",          ASTRLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_strlist_attr_list, write_CQ_strlist_attr_list},
   {  CQ_pe_list,                21, "pe_list",            ASTRLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_strlist_attr_list, write_CQ_strlist_attr_list},
   {  CQ_rerun,                  21, "rerun",              ABOOL_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_bool_attr_list,    write_CQ_bool_attr_list},
   {  CQ_job_slots,              21, "slots",              AULNG_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_ulng_attr_list,    write_CQ_ulng_attr_list},
   {  CQ_tmpdir,                 21, "tmpdir",             ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_shell,                  21, "shell",              ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_prolog,                 21, "prolog",             ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_epilog,                 21, "epilog",             ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_shell_start_mode,       21, "shell_start_mode",   ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_starter_method,         21, "starter_method",     ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_suspend_method,         21, "suspend_method",     ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_resume_method,          21, "resume_method",      ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_terminate_method,       21, "terminate_method",   ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_notify,                 21, "notify",             AINTER_sub_fields,   &qconf_sub_name_value_comma_braced_sfi, read_CQ_inter_attr_list,   write_CQ_inter_attr_list},
   {  CQ_owner_list,             21, "owner_list",         AUSRLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_usrlist_attr_list, write_CQ_usrlist_attr_list},
   {  CQ_acl,                    21, "user_lists",         AUSRLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_usrlist_attr_list, write_CQ_usrlist_attr_list},
   {  CQ_xacl,                   21, "xuser_lists",        AUSRLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_usrlist_attr_list, write_CQ_usrlist_attr_list},
   {  CQ_subordinate_list,       21, "subordinate_list",   ASOLIST_sub_fields,  &qconf_sub_name_value_comma_braced_sfi, read_CQ_solist_attr_list,  write_CQ_solist_attr_list},
   {  CQ_consumable_config_list, 21, "complex_values",     ACELIST_sub_fields,  &qconf_sub_name_value_comma_braced_sfi, read_CQ_celist_attr_list,  write_CQ_celist_attr_list},
   {  CQ_projects,               21, "projects",           APRJLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_prjlist_attr_list, write_CQ_prjlist_attr_list},
   {  CQ_xprojects,              21, "xprojects",          APRJLIST_sub_fields, &qconf_sub_name_value_comma_braced_sfi, read_CQ_prjlist_attr_list, write_CQ_prjlist_attr_list},
   {  CQ_calendar,               21, "calendar",           ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_initial_state,          21, "initial_state",      ASTR_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_str_attr_list,     write_CQ_str_attr_list},
   {  CQ_s_rt,                   21, "s_rt",               ATIME_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_time_attr_list,    write_CQ_time_attr_list},
   {  CQ_h_rt,                   21, "h_rt",               ATIME_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_time_attr_list,    write_CQ_time_attr_list},
   {  CQ_s_cpu,                  21, "s_cpu",              ATIME_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_time_attr_list,    write_CQ_time_attr_list},
   {  CQ_h_cpu,                  21, "h_cpu",              ATIME_sub_fields,    &qconf_sub_name_value_comma_braced_sfi, read_CQ_time_attr_list,    write_CQ_time_attr_list},
   {  CQ_s_fsize,                21, "s_fsize",            AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_fsize,                21, "h_fsize",            AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_s_data,                 21, "s_data",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_data,                 21, "h_data",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_s_stack,                21, "s_stack",            AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_stack,                21, "h_stack",            AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_s_core,                 21, "s_core",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_core,                 21, "h_core",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_s_rss,                  21, "s_rss",              AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_rss,                  21, "h_rss",              AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_s_vmem,                 21, "s_vmem",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  CQ_h_vmem,                 21, "h_vmem",             AMEM_sub_fields,     &qconf_sub_name_value_comma_braced_sfi, read_CQ_mem_attr_list,     write_CQ_mem_attr_list},
   {  NoName,                    21, NULL,                 NULL,                NULL,                                   NULL,                      NULL}
};

#ifndef __SGE_NO_USERMAPPING__
spooling_field CU_fields[] = {
   {  CU_name,           0, "cluster user",         NULL, NULL, NULL, NULL},
   {  CU_ruser_list,     0, "remote user",          ASTR_sub_fields, NULL, NULL, NULL},
   {  CU_ulong32,        0, "ulong32",              AULNG_sub_fields, NULL, NULL, NULL},
   {  CU_bool,           0, "bool",                 ABOOL_sub_fields, NULL, NULL, NULL},
   {  CU_time,           0, "time",                 ATIME_sub_fields, NULL, NULL, NULL},
   {  CU_mem,            0, "mem",                  AMEM_sub_fields, NULL, NULL, NULL},
   {  CU_inter,          0, "inter",                AINTER_sub_fields, NULL, NULL, NULL},
   {  NoName,            0, NULL,                   NULL, NULL, NULL, NULL}
};
#endif

spooling_field SH_fields[] = {
   {  SH_name,           21, "hostname",   NULL, NULL, NULL, NULL},
   {  NoName,            21, NULL,         NULL, NULL, NULL, NULL}
};

spooling_field AH_fields[] = {
   {  AH_name,           21, "hostname",   NULL, NULL, NULL, NULL},
   {  NoName,            21, NULL,         NULL, NULL, NULL, NULL}
};

static spooling_field RN_sub_fields[] = {
   {  RN_min,             0, NULL,                NULL, NULL, NULL, NULL},
   {  RN_max,             0, NULL,                NULL, NULL, NULL, NULL},
   {  RN_step,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,             0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field QR_sub_fields[] = {
   {  QR_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,             0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field JG_sub_fields[] = {
   {  JG_qname,             0, NULL,                NULL, NULL, NULL, NULL},
   {  JG_slots,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,             0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field MR_sub_fields[] = {
   {  MR_user,             0, NULL,                NULL, NULL, NULL, NULL},
   {  MR_host,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,             0, NULL,                NULL, NULL, NULL, NULL}
};

static spooling_field ARA_sub_fields[] = {
   {  ARA_name,             0, NULL,                NULL, NULL, NULL, NULL},
   {  ARA_group,             0, NULL,                NULL, NULL, NULL, NULL},
   {  NoName,             0, NULL,                NULL, NULL, NULL, NULL}
};

spooling_field AR_fields[] = {
   {  AR_id,              20,   "id",                NULL, NULL, NULL, NULL},
   {  AR_name,            20,   "name",              NULL, NULL, NULL, NULL},
   {  AR_account,         20,   "account",           NULL, NULL, NULL, NULL},
   {  AR_owner,           20,   "owner",             NULL, NULL, NULL, NULL},
   {  AR_group,           20,   "group",             NULL, NULL, NULL, NULL},
   {  AR_submission_time, 20,   "submission_time",   NULL, NULL, NULL, NULL},
   {  AR_start_time,      20,   "start_time",        NULL, NULL, NULL, NULL},
   {  AR_end_time,        20,   "end_time",          NULL, NULL, NULL, NULL},
   {  AR_duration,        20,   "duration",          NULL, NULL, NULL, NULL},
   {  AR_verify,          20,   "verify",            NULL, NULL, NULL, NULL},
   {  AR_error_handling,  20,   "error_handling",    NULL, NULL, NULL, NULL},
   {  AR_state,           20,   "state",             NULL, NULL, NULL, NULL},
   {  AR_checkpoint_name, 20,   "checkpoint_name",   NULL, NULL, NULL, NULL},
   {  AR_resource_list,   20,   "resource_list",     CE_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_queue_list,      20,   "queue_list",        QR_sub_fields, NULL, NULL, NULL},
   {  AR_granted_slots,   20,   "granted_slots",     JG_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_mail_options,    20,   "mail_options",      NULL, NULL, NULL, NULL},
   {  AR_mail_list,       20,   "mail_list",         MR_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_pe,              20,   "pe",                NULL, NULL, NULL, NULL},
   {  AR_pe_range,        20,   "pe_range",          RN_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_granted_pe,      20,   "granted_pe",        NULL, NULL, NULL, NULL},
   {  AR_master_queue_list, 20, "master_queue_list", QR_sub_fields, NULL, NULL, NULL},
   {  AR_acl_list,        20,   "acl_list",          ARA_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_xacl_list,       20,   "xacl_list",         ARA_sub_fields, &qconf_sub_name_value_comma_sfi, NULL, NULL},
   {  AR_type,            20,   "type",              NULL, NULL, NULL, NULL},
   {  NoName,             20,   NULL,                NULL, NULL, NULL, NULL}
};

spooling_field PE_fields[] = {
   {  PE_name,            18,   "pe_name",           NULL, NULL, NULL, NULL},
   {  PE_slots,           18,   "slots",             NULL, NULL, NULL, NULL},
   {  PE_user_list,       18,   "user_lists",        US_sub_fields, NULL, NULL, NULL},
   {  PE_xuser_list,      18,   "xuser_lists",       US_sub_fields, NULL, NULL, NULL},
   {  PE_start_proc_args, 18,   "start_proc_args",   NULL, NULL, NULL, NULL},
   {  PE_stop_proc_args,  18,   "stop_proc_args",    NULL, NULL, NULL, NULL},
   {  PE_allocation_rule, 18,   "allocation_rule",   NULL, NULL, NULL, NULL},
   {  PE_control_slaves,  18,   "control_slaves",    NULL, NULL, NULL, NULL},
   {  PE_job_is_first_task, 18,   "job_is_first_task", NULL, NULL, NULL, NULL},
   {  PE_urgency_slots,   18,   "urgency_slots",     NULL, NULL, NULL, NULL},
   {  PE_accounting_summary, 18,   "accounting_summary", NULL, NULL, NULL, NULL},
#ifdef SGE_PQS_API
   {  PE_qsort_args,      18,   "qsort_args",        NULL, NULL, NULL, NULL},
#endif
   {  NoName,             18,   NULL,                NULL, NULL, NULL, NULL}
};

spooling_field RQS_fields[] = {
   {  RQS_name,           12,   "name",              NULL, NULL, NULL, NULL},
   {  RQS_description,    12,   "description",       NULL, NULL, NULL, NULL},
   {  RQS_enabled,        12,   "enabled",           NULL, NULL, NULL, NULL},
   {  RQS_rule,           12,   "limit",             RQR_sub_fields, &qconf_sub_rqs_sfi, NULL, NULL},
   {  NoName,             12,   NULL,                NULL, NULL, NULL, NULL}
};

static void create_spooling_field (
   spooling_field *field,
   int nm, 
   int width, 
   const char *name, 
   struct spooling_field *sub_fields, 
   const void *clientdata, 
   int (*read_func) (lListElem *ep, int nm, const char *buffer, lList **alp), 
   int (*write_func) (const lListElem *ep, int nm, dstring *buffer, lList **alp)
)
{
   if (field != NULL) {
      field->nm = nm;
      field->width = width;
      field->name = name;
      field->sub_fields = sub_fields;
      field->clientdata = clientdata;
      field->read_func = read_func;
      field->write_func = write_func;
   }
}

spooling_field *sge_build_PR_field_list(bool spool)
{
   /* There are 11 possible PR_Type fields. */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field)*11);
   int count = 0;
   
   /* Build the list of fields to read and write */
   create_spooling_field(&fields[count++], PR_name, 0, "name", NULL,
                          NULL, NULL, NULL);
   create_spooling_field(&fields[count++], PR_oticket, 0, "oticket",
                          NULL, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], PR_fshare, 0, "fshare",
                          NULL, NULL, NULL, NULL);
   if (spool) {
      create_spooling_field(&fields[count++], PR_usage, 0, "usage",
                             UA_sub_fields, &qconf_sub_name_value_space_sfi, NULL, NULL);
      create_spooling_field(&fields[count++], PR_usage_time_stamp, 0, "usage_time_stamp",
                             NULL, NULL, NULL, NULL);
      create_spooling_field(&fields[count++], PR_long_term_usage, 0, "long_term_usage",
                             UA_sub_fields, &qconf_sub_name_value_space_sfi, NULL, NULL);
      create_spooling_field(&fields[count++], PR_project, 0, "project",
                             UPP_sub_fields,  &qconf_sub_spool_usage_sfi, NULL, NULL);
   }
   create_spooling_field(&fields[count++], PR_acl, 0, "acl", US_sub_fields,
                          NULL, NULL, NULL);
   create_spooling_field(&fields[count++], PR_xacl, 0, "xacl",
                          US_sub_fields, NULL, NULL, NULL);
   if (spool) {
      create_spooling_field(&fields[count++], PR_debited_job_usage, 0, "debited_job_usage",
                             UPU_sub_fields, &qconf_sub_spool_usage_sfi, NULL, NULL);
   }
   create_spooling_field(&fields[count++], NoName, 0, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

spooling_field *sge_build_UU_field_list(bool spool)
{
   /* There are 11 possible UU_Type fields. */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field)*11);
   int count = 0;
   
   /* Build the list of fields to read and write */
   create_spooling_field(&fields[count++], UU_name, 0, "name", NULL,
                          NULL, NULL, NULL);
   create_spooling_field(&fields[count++], UU_oticket, 0, "oticket",
                          NULL, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], UU_fshare, 0, "fshare",
                          NULL, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], UU_delete_time, 0,
                             "delete_time", NULL, NULL, NULL, NULL);
   if (spool) {
      create_spooling_field(&fields[count++], UU_usage, 0, "usage",
                             UA_sub_fields, &qconf_sub_name_value_space_sfi, NULL, NULL);
      create_spooling_field(&fields[count++], UU_usage_time_stamp, 0, "usage_time_stamp",
                             NULL, NULL, NULL, NULL);
      create_spooling_field(&fields[count++], UU_long_term_usage, 0, "long_term_usage",
                             UA_sub_fields, &qconf_sub_name_value_space_sfi, NULL, NULL);
      create_spooling_field(&fields[count++], UU_project, 0, "project",
                             UPP_sub_fields, &qconf_sub_spool_usage_sfi, NULL, NULL);
   }
   create_spooling_field(&fields[count++], UU_default_project, 0,
                             "default_project", NULL, NULL, NULL, NULL);
   if (spool) {
      create_spooling_field(&fields[count++], UU_debited_job_usage, 0, "debited_job_usage",
                             UPU_sub_fields, &qconf_sub_spool_usage_sfi, NULL, NULL);
   }
   create_spooling_field(&fields[count++], NoName, 0, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

spooling_field *sge_build_STN_field_list(bool spool, bool recurse)
{
   /* There are 7 possible STN_Type fields. */
   spooling_field *fields = (spooling_field *)malloc (sizeof(spooling_field)*7);
   int count = 0;

   if (recurse) {
      create_spooling_field (&fields[count++], STN_id, 0, "id", NULL,
                             NULL, NULL, NULL);
   }
  
   if (spool) {
      create_spooling_field (&fields[count++], STN_version, 0, "version",
                             NULL, NULL, NULL, NULL);
   }

   create_spooling_field (&fields[count++], STN_name, 0, "name", NULL,
                          NULL, NULL, NULL);
   create_spooling_field (&fields[count++], STN_type, 0, "type", NULL,
                          NULL, NULL, NULL);
   create_spooling_field (&fields[count++], STN_shares, 0, "shares",
                          NULL, NULL, NULL, NULL);
   
   if (recurse) {
      create_spooling_field (&fields[count++], STN_children, 0, "childnodes",
                             STN_sub_fields, NULL, NULL, NULL);
   }
   
   create_spooling_field (&fields[count++], NoName, 0, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

/* The spool_flatfile_read_object() function will fail to read the
 * EH_reschedule_unknown_list field from a classic spooling file because
 * classic spooling uses two different field delimiters to represent the
 * field values. */
spooling_field *sge_build_EH_field_list(bool spool, bool to_stdout,
                                        bool history)
{
   /* There are 14 possible EH_Type fields. */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field)*14);
   int count = 0;

   create_spooling_field(&fields[count++], EH_name, 21, "hostname",
                          NULL, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], EH_scaling_list, 21, "load_scaling",
                          HS_sub_fields, &qconf_sub_name_value_comma_sfi, NULL,
                          NULL);
   create_spooling_field(&fields[count++], EH_consumable_config_list, 21,
                          "complex_values", CE_sub_fields,
                          &qconf_sub_name_value_comma_sfi, NULL, NULL);
   
   if (getenv("MORE_INFO")) {
      create_spooling_field(&fields[count++], EH_resource_utilization, 21,
                             "complex_values_actual", RUE_sub_fields,
                             &qconf_sub_name_value_comma_sfi, NULL, NULL);
   }
   
   if (spool || (!spool && to_stdout) || history) {
      create_spooling_field(&fields[count++], EH_load_list, 21, "load_values",
                             HL_sub_fields, &qconf_sub_name_value_comma_sfi,
                             NULL, NULL);
      create_spooling_field(&fields[count++], EH_processors, 21, "processors",
                             NULL, NULL, NULL, NULL);
   }

   if (spool) {
      create_spooling_field(&fields[count++], EH_reschedule_unknown_list, 21,
                             "reschedule_unknown_list", RU_sub_fields,
                             &qconf_sub_name_value_comma_sfi, NULL, NULL);
   }
   
   create_spooling_field(&fields[count++], EH_acl, 21, "user_lists",
                          US_sub_fields, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], EH_xacl, 21, "xuser_lists",
                          US_sub_fields, NULL, NULL, NULL);
   
   create_spooling_field(&fields[count++], EH_prj, 21, "projects",
                          PR_sub_fields, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], EH_xprj, 21, "xprojects",
                          PR_sub_fields, NULL, NULL, NULL);
   create_spooling_field(&fields[count++], EH_usage_scaling_list, 21,
                          "usage_scaling", HS_sub_fields,
                          &qconf_sub_name_value_comma_sfi, NULL, NULL);
   create_spooling_field(&fields[count++], EH_report_variables, 21, 
                          "report_variables", STU_sub_fields, 
                          &qconf_sub_name_value_comma_sfi, NULL, NULL);
   create_spooling_field(&fields[count++], NoName, 21, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

static int read_SC_queue_sort_method(lListElem *ep, int nm,
                                     const char *buffer, lList **alp)
{
   if (!strncasecmp(buffer, "load", 4)) {
      lSetUlong(ep, nm, QSM_LOAD);
   } else if (!strncasecmp(buffer, "seqno", 5)) {
      lSetUlong(ep, nm, QSM_SEQNUM);
   }
   
   return 1;
}

static int write_SC_queue_sort_method(const lListElem *ep, int nm,
                                      dstring *buffer, lList **alp)
{
   if (lGetUlong(ep, nm) == QSM_SEQNUM) {
      sge_dstring_append(buffer, "seqno");
   } else {
      sge_dstring_append(buffer, "load");
   }

   return 1;
}

static int read_CF_value(lListElem *ep, int nm, const char *buf,
                         lList **alp)
{
   const char *name = lGetString(ep, CF_name);
   char *value = NULL;
   char *buffer = strdup(buf);
   struct saved_vars_s *context = NULL;

   DENTER(TOP_LAYER, "read_CF_value");
   
   if (!strcmp(name, "gid_range")) {
      if ((value = sge_strtok_r(buffer, " \t\n", &context))) {
         if (!strcasecmp(value, NONE_STR)) {
            lSetString(ep, CF_value, value);
         } else {
            lList *rlp = NULL;
            
            range_list_parse_from_string(&rlp, alp, value, 
                                         false, false, INF_NOT_ALLOWED);
            if (rlp == NULL) {
               WARNING((SGE_EVENT, MSG_CONFIG_CONF_INCORRECTVALUEFORCONFIGATTRIB_SS, 
                        name, value));
   
               sge_free_saved_vars(context);
               sge_free(&buffer);
               DRETURN(0);
            } else {
               lListElem *rep;

               for_each(rep, rlp) {
                  u_long32 min;

                  min = lGetUlong(rep, RN_min);
                  if (min < GID_RANGE_NOT_ALLOWED_ID) {
                     WARNING((SGE_EVENT, MSG_CONFIG_CONF_GIDRANGELESSTHANNOTALLOWED_I, GID_RANGE_NOT_ALLOWED_ID));
   
                     sge_free_saved_vars(context);
                     sge_free(&buffer);
                     lFreeList(&rlp);
                     DRETURN(0);
                  }                  
               }
               lFreeList(&rlp);
               lSetString(ep, CF_value, value);
            }
         }
      }
   } else if (!strcmp(name, "admin_user")) {
      value = sge_strtok_r(buffer, " \t\n", &context);
      while (value[0] && isspace((int)value[0]))
         value++;
      if (value) {
         lSetString(ep, CF_value, value);
      } else {
         WARNING((SGE_EVENT, MSG_CONFIG_CONF_NOVALUEFORCONFIGATTRIB_S, name));
   
         sge_free_saved_vars(context);
         sge_free(&buffer);
         DRETURN(0);
      }
   } else if (!strcmp(name, "user_lists") || 
      !strcmp(name, "xuser_lists") || 
      !strcmp(name, "projects") || 
      !strcmp(name, "xprojects") || 
      !strcmp(name, "prolog") || 
      !strcmp(name, "epilog") || 
      !strcmp(name, "starter_method") || 
      !strcmp(name, "suspend_method") || 
      !strcmp(name, "resume_method") || 
      !strcmp(name, "terminate_method") || 
      !strcmp(name, "qmaster_params") || 
      !strcmp(name, "execd_params") || 
      !strcmp(name, "reporting_params") || 
      !strcmp(name, "qlogin_command") ||
      !strcmp(name, "rlogin_command") ||
      !strcmp(name, "rsh_command") ||
      !strcmp(name, "jsv_url") ||
      !strcmp(name, "jsv_allowed_mod") ||
      !strcmp(name, "qlogin_daemon") ||
      !strcmp(name, "rlogin_daemon") ||
      !strcmp(name, "rsh_daemon") ||
      !strcmp(name, "additional_jvm_args")) {
      if (!(value = sge_strtok_r(buffer, "\t\n", &context))) {
         /* return line if value is empty */
         WARNING((SGE_EVENT, MSG_CONFIG_CONF_NOVALUEFORCONFIGATTRIB_S, name));
   
         sge_free_saved_vars(context);
         sge_free(&buffer);
         DRETURN(0);
      }
      /* skip leading delimitors */
      while (value[0] && isspace((int)value[0]))
         value++;

      lSetString(ep, CF_value, value);
   } else {
      if (!(value = sge_strtok_r(buffer, " \t\n", &context))) {
         WARNING((SGE_EVENT, MSG_CONFIG_CONF_NOVALUEFORCONFIGATTRIB_S, name));
   
         sge_free_saved_vars(context);
         sge_free(&buffer);
         DRETURN(0);
      }
      if (strcmp(name, "auto_user_oticket") == 0 || 
              strcmp(name, "auto_user_fshare") == 0 ) {
         char *end_ptr = NULL;
         double dbl_value;

         dbl_value = strtod(value, &end_ptr);
         if ( dbl_value < 0) {
            answer_list_add_sprintf(alp, STATUS_EUNKNOWN,
                                 ANSWER_QUALITY_ERROR,
				 MSG_MUST_BE_POSITIVE_VALUE_S,
                                 name);
            DRETURN(0);
         }            
      } 
      lSetString(ep, CF_value, value);

      if (sge_strtok_r(NULL, " \t\n", &context)) {
         /* Allow only one value per line */
         WARNING((SGE_EVENT, MSG_CONFIG_CONF_ONLYSINGLEVALUEFORCONFIGATTRIB_S,
                  name));
   
         sge_free_saved_vars(context);
         sge_free(&buffer);
         DRETURN(0);
      }
   }

   sge_free_saved_vars(context);
   sge_free(&buffer);
   DRETURN(1);
}

spooling_field *sge_build_CONF_field_list(bool spool_config)
{
   /* There are 4 possible CONF_Type fields. */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field)*4);
   int count = 0;
   
   if (spool_config) {
      create_spooling_field(&fields[count++], CONF_name, 28, "conf_name",
                             NULL, NULL, NULL, NULL);
      create_spooling_field(&fields[count++], CONF_version, 28, "conf_version",
                             NULL, NULL, NULL, NULL);
   }
   
   create_spooling_field(&fields[count++], CONF_entries, 28, NULL,
                          CF_sub_fields, &qconf_sub_param_sfi, NULL, NULL);
   create_spooling_field(&fields[count++], NoName, 28, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

spooling_field *sge_build_QU_field_list(bool to_stdout, bool to_file)
{
   /* There are 52 possible QU_Type fields. */
   spooling_field *fields = (spooling_field *)malloc(sizeof(spooling_field)*52);
   int count = 0;
   
   create_spooling_field (&fields[count++], QU_qname, 21, "qname", NULL,
                          NULL, NULL, NULL);
   create_spooling_field (&fields[count++], QU_qhostname, 21, "hostname",
                          NULL, NULL, NULL, NULL);
   
   if (to_stdout) {
      create_spooling_field (&fields[count++], QU_seq_no, 21, "seq_no", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_load_thresholds, 21,
                             "load_thresholds", CE_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_suspend_thresholds, 21,
                             "suspend_thresholds", CE_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_nsuspend, 21, "nsuspend",
                             NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_suspend_interval, 21,
                             "suspend_interval", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_priority, 21, "priority",
                             NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_min_cpu_interval, 21,
                             "min_cpu_interval", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_processors, 21, "processors",
                             NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_qtype, 21, "qtype", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_ckpt_list, 21, "ckpt_list",
                             ST_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_pe_list, 21, "pe_list",
                             ST_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_rerun, 21, "rerun", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_job_slots, 21, "slots", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_tmpdir, 21, "tmpdir", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_shell, 21, "shell", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_prolog, 21, "prolog", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_epilog, 21, "epilog", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_shell_start_mode, 21,
                             "shell_start_mode", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_starter_method, 21,
                             "starter_method", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_suspend_method, 21,
                             "suspend_method", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_resume_method, 21,
                             "resume_method", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_terminate_method, 21,
                             "terminate_method", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_notify, 21, "notify", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_owner_list, 21, "owner_list",
                             US_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_acl, 21, "user_lists",
                             US_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_xacl, 21, "xuser_lists",
                             US_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_subordinate_list, 21,
                             "subordinate_list", SO_sub_fields, NULL, NULL,
                             NULL);
      create_spooling_field (&fields[count++], QU_consumable_config_list, 21,
                             "complex_values", CE_sub_fields, NULL, NULL, NULL);
      
      create_spooling_field (&fields[count++], QU_projects, 21, "projects",
                             PR_sub_fields, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_xprojects, 21, "xprojects",
                             PR_sub_fields, NULL, NULL, NULL);
      
      create_spooling_field (&fields[count++], QU_calendar, 21, "calendar",
                             NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_initial_state, 21,
                             "initial_state", NULL, NULL, NULL, NULL);
#if 0
      create_spooling_field (&fields[count++], QU_fshare, 21, "fshare", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_oticket, 21, "oticket",
                             NULL, NULL, NULL, NULL);
#endif
      create_spooling_field (&fields[count++], QU_s_rt, 21, "s_rt", NULL, NULL,
                             NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_rt, 21, "h_rt", NULL, NULL,
                             NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_cpu, 21, "s_cpu", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_cpu, 21, "h_cpu", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_fsize, 21, "s_fsize", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_fsize, 21, "h_fsize", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_data, 21, "s_data", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_data, 21, "h_data", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_stack, 21, "s_stack", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_stack, 21, "h_stack", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_core, 21, "s_core", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_core, 21, "h_core", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_rss, 21, "s_rss", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_rss, 21, "h_rss", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_s_vmem, 21, "s_vmem", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_h_vmem, 21, "h_vmem", NULL,
                             NULL, NULL, NULL);
   } else if (to_file) {
      /*
       * Spool only non-CQ attributes
       */
      create_spooling_field (&fields[count++], QU_state, 21, "state", NULL,
                             NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_pending_signal, 21,
                             "pending_signal", NULL, NULL, NULL, NULL);
      create_spooling_field (&fields[count++], QU_pending_signal_delivery_time,
                             21, "pending_signal_del", NULL, NULL,
                             NULL, NULL);
      create_spooling_field (&fields[count++], QU_version, 21, "version", NULL,
                             NULL, NULL, NULL);
      /* SG: not supported */
#if 0      
      create_spooling_field (&fields[count++], QU_error_messages, 21,
                             "error_messages", NULL, NULL, NULL, NULL);
#endif      
   }
   create_spooling_field (&fields[count++], NoName, 21, NULL, NULL, NULL, NULL,
                          NULL);
   
   return fields;
}

static int read_CQ_ulng_attr_list(lListElem *ep, int nm, const char *buffer, lList **alp)
{
   lList *lp = NULL;

   if (!ulng_attr_list_parse_from_string(&lp, alp, buffer,
                                         HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }

   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }

   return 0;
}

static int write_CQ_ulng_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   ulng_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_celist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp)
{
   lList *lp = NULL;

   if (!celist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }

   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }

   return 0;
}

static int write_CQ_celist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   celist_attr_list_append_to_dstring(lGetList(ep, nm), buffer);
   
   return 1;
}

static int read_CQ_inter_attr_list(lListElem *ep, int nm, const char *buffer,
                                    lList **alp)
{
   lList *lp = NULL;
   
   if (!inter_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_inter_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   inter_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_str_attr_list(lListElem *ep, int nm, const char *buffer,
                                  lList **alp)
{
   lList *lp = NULL;
   
   if (!str_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_str_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   str_attr_list_append_to_dstring(lGetList(ep, nm), buffer);
   
   return 1;
}

static int read_CQ_qtlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp)
{
   lList *lp = NULL;
   
   if (!qtlist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_qtlist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   qtlist_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_strlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                      lList **alp)
{
   lList *lp = NULL;
   
   if (!strlist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_strlist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   strlist_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_bool_attr_list(lListElem *ep, int nm, const char *buffer,
                                   lList **alp)
{
   lList *lp = NULL;
   
   if (!bool_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_bool_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   bool_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_usrlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                      lList **alp)
{
   lList *lp = NULL;
   
   if (!usrlist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_usrlist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   usrlist_attr_list_append_to_dstring(lGetList (ep, nm), buffer);
   
   return 1;
}

static int read_CQ_solist_attr_list(lListElem *ep, int nm, const char *buffer,
                                     lList **alp)
{
   lList *lp = NULL;
   
   if (!solist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_solist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   lList *lp = lGetList(ep, nm);
   
   solist_attr_list_append_to_dstring(lp, buffer);
   
   return 1;
}

static int read_CQ_prjlist_attr_list(lListElem *ep, int nm, const char *buffer,
                                      lList **alp)
{
   lList *lp = NULL;
   
   if (!prjlist_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_prjlist_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   prjlist_attr_list_append_to_dstring(lGetList(ep, nm), buffer);
   
   return 1;
}

static int read_CQ_time_attr_list(lListElem *ep, int nm, const char *buffer,
                                   lList **alp)
{
   lList *lp = NULL;
   
   if (!time_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_time_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   time_attr_list_append_to_dstring(lGetList(ep, nm), buffer);
   
   return 1;
}

static int read_CQ_mem_attr_list(lListElem *ep, int nm, const char *buffer,
                                  lList **alp)
{
   lList *lp = NULL;
   
   if (!mem_attr_list_parse_from_string(&lp, alp, buffer,
                                          HOSTATTR_ALLOW_AMBIGUITY)) {
      lFreeList(&lp);
      return 0;
   }
   
   if (lp != NULL) {
      lSetList(ep, nm, lp);
      return 1;
   }
   
   return 0;
}

static int write_CQ_mem_attr_list(const lListElem *ep, int nm,
                                   dstring *buffer, lList **alp)
{
   mem_attr_list_append_to_dstring(lGetList(ep, nm), buffer);
   
   return 1;
}

static int read_CQ_hostlist(lListElem *ep, int nm, const char *buffer,
                             lList **alp)
{
   lList *lp = NULL;
   char delims[] = "\t \v\r,"; 

   lString2List(buffer, &lp, HR_Type, HR_name, delims); 

   if (lp != NULL) {
      if (strcasecmp(NONE_STR, lGetHost(lFirst(lp), HR_name)) != 0) {
         lSetList(ep, CQ_hostlist, lp);
      } else {
         lFreeList(&lp);
      }
   }

   return 1;
}

static int write_CQ_hostlist(const lListElem *ep, int nm,
                             dstring *buffer, lList **alp)
{
   lList *lp = lGetList(ep, nm);
   
   if (lp != NULL) {
      href_list_append_to_dstring(lp, buffer);
   } else {
      sge_dstring_append(buffer, NONE_STR);
   }
   
   return 1;
}

static int write_CE_stringval(const lListElem *ep, int nm, dstring *buffer,
                       lList **alp)
{
   const char *s;

   if ((s=lGetString(ep, CE_stringval)) != NULL) {
      sge_dstring_append(buffer, s);
   } else {
      sge_dstring_sprintf_append(buffer, "%f", lGetDouble(ep, CE_doubleval));
   }
   
   return 1;
}

/****** sge_flatfile_obj/read_RQR_obj() ****************************************
*  NAME
*     read_RQR_obj() -- parse a RQR object from string
*
*  SYNOPSIS
*     static int read_RQR_obj(lListElem *ep, int nm, const char *buffer, lList 
*     **alp) 
*
*  FUNCTION
*     Reads in a RQR Element from string
*
*  INPUTS
*     lListElem *ep      - Store for parsed Elem
*     int nm             - nm to be parsed 
*     const char *buffer - String of the elem to be parsed
*     lList **alp        - Answer list
*
*  RESULT
*     static int - 1 on success
*                  0 on error
*
*  NOTES
*     MT-NOTE: read_RQR_obj() is MT safe 
*
*******************************************************************************/
static int read_RQR_obj(lListElem *ep, int nm, const char *buffer,
                             lList **alp) {
   lListElem *filter = NULL;
   int ret = 1;

   DENTER(TOP_LAYER, "read_RQR_obj");

   if ((ret = rqs_parse_filter_from_string(&filter, buffer, alp)) == 1) {
      lSetObject(ep, nm, filter);
   } 

   DRETURN(ret);
}

/****** sge_flatfile_obj/write_RQR_obj() ***************************************
*  NAME
*     write_RQR_obj() -- converts a element to string
*
*  SYNOPSIS
*     static int write_RQR_obj(const lListElem *ep, int nm, dstring *buffer, lList 
*     **alp) 
*
*  FUNCTION
*     Prints out a RQR Element to a string
*
*  INPUTS
*     const lListElem *ep - Elem to be converted
*     int nm              - nm of Elem
*     dstring *buffer     - Element as string
*     lList **alp         - Answer List
*
*  RESULT
*     static int - 1 on success
*                  0 on error
*
*  NOTES
*     MT-NOTE: write_RQR_obj() is MT safe 
*
*******************************************************************************/
static int write_RQR_obj(const lListElem *ep, int nm, dstring *buffer,
                       lList **alp) {
   return rqs_append_filter_to_dstring(lGetObject(ep, nm), buffer, alp);
}
