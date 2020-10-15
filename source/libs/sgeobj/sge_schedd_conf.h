#ifndef __SGE_SCHEDCONFL_H
#define __SGE_SCHEDCONFL_H

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

#include "cull/cull.h"

#include "sgeobj/sge_boundaries.h"
#include "sgeobj/sge_schedd_conf_PARA_L.h"
#include "sgeobj/sge_schedd_conf_SC_L.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* 
 * valid values for SC_queue_sort_method 
 */
enum {
   QSM_LOAD = 0,
   QSM_SEQNUM = 1
};

/* defines the last dispatched job */
enum {
    DISPATCH_TYPE_NONE = 0,      /* did not dispatch a job */
    DISPATCH_TYPE_FAST,          /* dispatched a sequential job */
    DISPATCH_TYPE_FAST_SOFT_REQ, /* dispatch a sequential job with soft requests */
    DISPATCH_TYPE_PE,            /* dispatched a pe job*/
    DISPATCH_TYPE_PE_SOFT_REQ    /* dispatched a pe job*/
};

enum schedd_job_info_key {
   SCHEDD_JOB_INFO_FALSE=0,
   SCHEDD_JOB_INFO_TRUE,
   SCHEDD_JOB_INFO_JOB_LIST,
   SCHEDD_JOB_INFO_UNDEF
};

/* defines the algorithm that should be used to compute pe-ranges */
typedef enum {
   SCHEDD_PE_AUTO=-1,      /* automatic, the scheduler will decide */
   SCHEDD_PE_LOW_FIRST=0,  /* least slot first */
   SCHEDD_PE_HIGH_FIRST,   /* highest slot first */
   SCHEDD_PE_BINARY,       /* binary search */
   SCHEDD_PE_ALG_MAX       /* number of algorithms */    /* number of algorithms */
} schedd_pe_algorithm;

typedef enum {
   FIRST_POLICY_VALUE,
   INVALID_POLICY = FIRST_POLICY_VALUE,

   OVERRIDE_POLICY,
   FUNCTIONAL_POLICY,
   SHARE_TREE_POLICY,

   /* TODO: shouldn't LAST_POLICY_VALUE equal SHARE_TREE_POLICY? 
    * POLICY_VALUES = 4, should probably be 3
    */
   LAST_POLICY_VALUE,
   POLICY_VALUES = (LAST_POLICY_VALUE - FIRST_POLICY_VALUE)
} policy_type_t;

typedef struct {
   policy_type_t policy;
   int dependent;
} policy_hierarchy_t;

void sc_mt_init(void);

void sconf_ph_fill_array(policy_hierarchy_t array[]);

void sconf_ph_print_array(policy_hierarchy_t array[]);

void sconf_print_config(void);

lListElem *sconf_create_default(void);

bool sconf_set_config(lList **config, lList **answer_list);

bool sconf_is_valid_load_formula(lList **answer_list,
                                       lList *cmplx_list);

bool
sconf_is_centry_referenced(const lListElem *centry);

bool sconf_validate_config(lList **answer_list, lList *config);

bool sconf_validate_config_(lList **answer_list);

lListElem *sconf_get_config(void);

lList *sconf_get_config_list(void);

bool sconf_is_new_config(void);
void sconf_reset_new_config(void);

bool sconf_is(void);

u_long32 sconf_get_load_adjustment_decay_time(void);

lList *sconf_get_job_load_adjustments(void);

char *sconf_get_load_formula(void);

u_long32 sconf_get_queue_sort_method(void);

u_long32 sconf_get_maxujobs(void);

u_long32 sconf_get_schedule_interval(void);

u_long32 sconf_get_reprioritize_interval(void);

u_long32 sconf_get_weight_tickets_share(void);

lList *sconf_get_schedd_job_info_range(void);

bool sconf_is_id_in_schedd_job_info_range(u_long32 job_number);

lList *sconf_get_usage_weight_list(void);

double sconf_get_weight_user(void);

double sconf_get_weight_department(void);

double sconf_get_weight_project(void);

double sconf_get_weight_job(void);

u_long32 sconf_get_weight_tickets_share(void);

u_long32 sconf_get_weight_tickets_functional(void);

u_long32 sconf_get_halftime(void);

void sconf_set_weight_tickets_override(u_long32 active);

u_long32 sconf_get_weight_tickets_override(void);

double sconf_get_compensation_factor(void);

bool sconf_get_share_override_tickets(void);

bool sconf_get_share_functional_shares(void);

bool sconf_get_report_pjob_tickets(void);

bool sconf_is_job_category_filtering(void);

u_long32 sconf_get_flush_submit_sec(void);

u_long32 sconf_get_flush_finish_sec(void);

u_long32 sconf_get_max_functional_jobs_to_schedule(void);

u_long32 sconf_get_max_pending_tasks_per_job(void);

lList* sconf_get_halflife_decay_list(void);

double sconf_get_weight_ticket(void);
double sconf_get_weight_waiting_time(void);
double sconf_get_weight_deadline(void);
double sconf_get_weight_urgency(void);

u_long32 sconf_get_max_reservations(void);

double sconf_get_weight_priority(void);
bool sconf_get_profiling(void);

u_long32 sconf_get_default_duration(void);

typedef enum {
   QS_STATE_EMPTY,
   QS_STATE_FULL
} qs_state_t;

u_long32 sconf_get_schedd_job_info(void);
void sconf_disable_schedd_job_info(void);
void sconf_enable_schedd_job_info(void);

void sconf_set_qs_state(qs_state_t state);
qs_state_t sconf_get_qs_state(void);

void sconf_set_global_load_correction(bool flag);
bool sconf_get_global_load_correction(void);

bool sconf_get_host_order_changed(void);
void sconf_set_host_order_changed(bool changed);

int  sconf_get_last_dispatch_type(void);
void sconf_set_last_dispatch_type(int changed);

u_long32  sconf_get_duration_offset(void);

bool serf_get_active(void);

schedd_pe_algorithm sconf_best_pe_alg(void);
void sconf_update_pe_alg(int runs, int current, int max);
int  sconf_get_pe_alg_value(schedd_pe_algorithm alg);

void sconf_inc_fast_jobs(void);
int sconf_get_fast_jobs(void);

void sconf_inc_pe_jobs(void);
int sconf_get_pe_jobs(void);

void sconf_set_decay_constant(double decay);
double sconf_get_decay_constant(void);

void sconf_set_mes_schedd_info(bool newval);
bool sconf_get_mes_schedd_info(void);

void schedd_mes_set_logging(int bval);
int schedd_mes_get_logging(void);

lListElem *sconf_get_sme(void);
void sconf_set_sme(lListElem *sme);

lListElem *sconf_get_tmp_sme(void);
void sconf_set_tmp_sme(lListElem *sme);

void sconf_reset_jobs(void);

void sconf_get_weight_ticket_urgency_priority(double *ticket, double *urgency, double *priority);

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_SCHEDCONFL_H */
