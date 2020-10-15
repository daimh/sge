#ifndef __SGE_CONF_H
#define __SGE_CONF_H
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

#include "sgeobj/sge_conf_CONF_L.h"
#include "sgeobj/sge_conf_CF_L.h"

/* The scheduler configuration changes this configuration element only. It is
   not spooled and is not shown in qconf -mconf */
#define REPRIORITIZE "reprioritize"

#define GID_RANGE_NOT_ALLOWED_ID 100
#define RLIMIT_UNDEFINED -9999

typedef int (*tDaemonizeFunc)(void *ctx);

/* This list is *ONLY* used by the execd and should be moved eventually */
extern lList *Execd_Config_List;

int merge_configuration(lList **answer_list, u_long32 progid, const char *cell_root, lListElem *global, lListElem *local, lList **lpp);
void sge_show_conf(void);
void conf_update_thread_profiling(const char *thread_name);

char* mconf_get_execd_spool_dir(void);
char* mconf_get_mailer(void);
char* mconf_get_xterm(void);
char* mconf_get_load_sensor(void);
char* mconf_get_prolog(void);
char* mconf_get_epilog(void);
char* mconf_get_shell_start_mode(void);
char* mconf_get_login_shells(void);
u_long32 mconf_get_min_uid(void);
u_long32 mconf_get_min_gid(void);
u_long32 mconf_get_load_report_time(void);
u_long32 mconf_get_max_unheard(void);
u_long32 mconf_get_loglevel(void);
char* mconf_get_enforce_project(void);
char* mconf_get_enforce_user(void);
char* mconf_get_administrator_mail(void);
lList* mconf_get_user_lists(void);
lList* mconf_get_xuser_lists(void);
lList* mconf_get_projects(void);
lList* mconf_get_xprojects(void);
char* mconf_get_set_token_cmd(void);
char* mconf_get_pag_cmd(void);
u_long32 mconf_get_token_extend_time(void);
char* mconf_get_shepherd_cmd(void);
char* mconf_get_qmaster_params(void);
char* mconf_get_execd_params(void);
char* mconf_get_reporting_params(void);
char* mconf_get_gid_range(void);
u_long32 mconf_get_zombie_jobs(void);
char* mconf_get_qlogin_daemon(void);
char* mconf_get_qlogin_command(void);
char* mconf_get_rsh_daemon(void);
char* mconf_get_rsh_command(void);
char* mconf_get_jsv_url(void);
char* mconf_get_jsv_allowed_mod(void);
char* mconf_get_rlogin_daemon(void);
char* mconf_get_rlogin_command(void);
u_long32 mconf_get_reschedule_unknown(void);
u_long32 mconf_get_max_aj_instances(void);
u_long32 mconf_get_max_aj_tasks(void);
u_long32 mconf_get_max_u_jobs(void);
u_long32 mconf_get_max_jobs(void);
u_long32 mconf_get_max_advance_reservations(void);
u_long32 mconf_get_reprioritize(void);
u_long32 mconf_get_auto_user_fshare(void);
u_long32 mconf_get_auto_user_oticket(void);
char* mconf_get_auto_user_default_project(void);
u_long32 mconf_get_auto_user_delete_time(void);
char* mconf_get_delegated_file_staging(void);
void mconf_set_new_config(bool new_config);
bool mconf_is_new_config(void);
bool mconf_get_old_reschedule_behavior(void);
bool mconf_get_old_reschedule_behavior_array_job(void);

/* params */
bool mconf_is_monitor_message(void);
bool mconf_get_use_qidle(void);
bool mconf_get_forbid_reschedule(void);
bool mconf_get_forbid_apperror(void);
bool mconf_get_do_credentials(void);   
bool mconf_get_do_authentication(void);  
bool mconf_get_acct_reserved_usage(void);
bool mconf_get_sharetree_reserved_usage(void);
bool mconf_get_keep_active(void);
bool mconf_get_enable_windomacc(void);
bool mconf_get_enable_binding(void);
bool mconf_get_simulate_execds(void);
bool mconf_get_simulate_jobs(void);
long mconf_get_ptf_max_priority(void);
long mconf_get_ptf_min_priority(void);
bool mconf_get_use_qsub_gid(void);
int mconf_get_notify_susp_type(void);      
char* mconf_get_notify_susp(void);       
int mconf_get_notify_kill_type(void);      
char* mconf_get_notify_kill(void);
bool mconf_get_disable_reschedule(void);  
int mconf_get_scheduler_timeout(void);
int mconf_get_max_dynamic_event_clients(void);
void mconf_set_max_dynamic_event_clients(int value);
bool mconf_get_set_lib_path(void);
bool mconf_get_inherit_env(void);
int mconf_get_spool_time(void);
u_long32 mconf_get_monitor_time(void);
bool mconf_get_do_accounting(void);
bool mconf_get_do_reporting(void);
bool mconf_get_do_joblog(void);
int mconf_get_reporting_flush_time(void);
int mconf_get_accounting_flush_time(void);
int mconf_get_sharelog_time(void);
int mconf_get_log_consumables(void);
bool mconf_get_enable_forced_qdel(void);
bool mconf_get_enable_forced_qdel_if_unknown(void);
bool mconf_get_enable_enforce_master_limit(void);
bool mconf_get_enable_test_sleep_after_request(void);
int mconf_get_max_job_deletion_time(void);
bool mconf_get_enable_addgrp_kill(void);
u_long32 mconf_get_pdc_interval(void);
bool mconf_get_enable_reschedule_kill(void);
bool mconf_get_enable_reschedule_slave(void);
void mconf_get_h_descriptors(char **pret);
void mconf_get_s_descriptors(char **pret);
void mconf_get_h_maxproc(char **pret);
void mconf_get_s_maxproc(char **pret);
void mconf_get_h_memorylocked(char **pret);
void mconf_get_s_memorylocked(char **pret);
void mconf_get_h_locks(char **pret);
void mconf_get_s_locks(char **pret);
int mconf_get_jsv_timeout(void);
int mconf_get_jsv_threshold(void);
bool mconf_get_print_malloc_info(void);
bool mconf_get_ignore_ngroups_max_limit(void);
bool mconf_get_use_cgroups(void);
bool mconf_get_use_smaps(void);
bool mconf_get_demand_ls(void);

#endif /* __SGE_CONF_H */
