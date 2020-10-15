#ifndef __SGE_JOB_H 
#define __SGE_JOB_H 
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

#include "uti/sge_htable.h"
#include "uti/sge_dstring.h"

#include "sgeobj/sge_job_JB_L.h"
#include "sgeobj/sge_job_JG_L.h"
#include "sgeobj/sge_job_PN_L.h"
#include "sgeobj/sge_job_AT_L.h"
#include "sgeobj/sge_job_ref_JRE_L.h"

/* Job states moved in from def.h */
#define JIDLE                                0x00000000
/* #define JENABLED                             0x00000008 */
#define JHELD                                0x00000010
#define JMIGRATING                           0x00000020
#define JQUEUED                              0x00000040
#define JRUNNING                             0x00000080
#define JSUSPENDED                           0x00000100
#define JTRANSFERING                         0x00000200
#define JDELETED                             0x00000400
#define JWAITING                             0x00000800
#define JEXITING                             0x00001000
#define JWRITTEN                             0x00002000
/* used in execd - job waits for getting its ASH/JOBID */
#define JWAITING4OSJID                       0x00004000
/* used in execd - shepherd reports job exit but there are still processes */
#define JERROR                               0x00008000

/*
 * JSUSPEND_ON_THRESHOLD and JFINISHED have the same value, but
 * JSUSPEND_ON_THRESHOLD is only set in the JAT_state filed,
 * where JFINISHED is set in the JAT_status and PET_status fields.
 */
#define JSUSPENDED_ON_THRESHOLD              0x00010000
/*
 * SGEEE: qmaster delays job removal until scheduler longer needs this
 * finished job
 */
#define JFINISHED                            0x00010000
/* used in execd to prevent slave jobs from getting started */
#define JSLAVE                               0x00020000
#define JDEFERRED_REQ                        0x00100000

/* 
   GDI request syntax for JB_hold 

   Example:
 
   qalter -h {u|s|o|n}

  POSIX (overwriting):
  u: SET|USER 
  o: SET|OPERATOR
  s: SET|SYSTEM 
  n: SUB|USER|SYSTEM|OPERATOR
 
  SGE (adding):
  +u: ADD|USER
  +o: ADD|OPERATOR
  +s: ADD|SYSTEM
 
  SGE (removing):
  -u: SUB|USER
  -o: SUB|OPERATOR
  -s: SUB|SYSTEM
   
*/
enum {
   /* need place for tree bits */
   MINUS_H_CMD_ADD = (0<<4), /* adds targetted flags */
   MINUS_H_CMD_SUB = (1<<4), /* remove targetted flags */
   MINUS_H_CMD_SET = (2<<4)  /* overwrites using targetted flags */
}; 

enum {
   MINUS_H_TGT_USER     = 1, /* remove needs at least job owner */
   MINUS_H_TGT_OPERATOR = 2, /* remove needs at least operator  */
   MINUS_H_TGT_SYSTEM   = 4, /* remove needs at least manager   */
   MINUS_H_TGT_JA_AD    = 8, /* removed automatically */
   MINUS_H_TGT_ALL      = 15,
   MINUS_H_TGT_NONE     = 31
};

/* values for JB_verify_suitable_queues */
#define OPTION_VERIFY_STR "nwevp"
enum {
   SKIP_VERIFY = 0,     /* -w n no expendable verifications will be done */
   WARNING_VERIFY,      /* -w w qmaster will warn about these jobs - but submit will succeed */
   ERROR_VERIFY,        /* -w e qmaster will make expendable verifications to reject 
                            jobs that are not schedulable (default) */
   JUST_VERIFY,         /* -w v just verify at qmaster but do not submit */
   POKE_VERIFY          /* -w p do verification with all resource utilizations in place (poke) */
};

/************    scheduling constants   *****************************************/
/* priorities are in the range from -1023 to 1024 */
/* to put them in into u_long we need to add 1024 */
#define BASE_PRIORITY  1024

/* int -> u_long */
#define PRI_ITOU(x) ((x)+BASE_PRIORITY)
/* u_long -> int */
#define PRI_UTOI(x) ((x)-BASE_PRIORITY)

#define PRIORITY_OFFSET 8
#define NEWCOMER_FLAG     0x1000000

/* forced negative sign bit  */
#define MAX_JOBS_EXCEEDED 0x8000000
#define ALREADY_SCANNED   0x4000000
#define PRIORITY_MASK     0xffff00
#define SUBPRIORITY_MASK  0x0000ff
#define JOBS_SCANNED_PER_PASS 10

/* 
   used in qstat:

   JSUSPENDED_ON_SUBORDINATE means that the job is
   suspended because its queue is suspended

*/
#define JSUSPENDED_ON_SUBORDINATE            0x00002000
#define JSUSPENDED_ON_SLOTWISE_SUBORDINATE   0x00004000

/* reserved names for JB_context */
#define CONTEXT_IOR "IOR"
#define CONTEXT_PARENT "PARENT"

/****** sgeobj/job/jb_now *****************************************************
*  NAME
*     jb_now -- macros to handle flag JB_type 
*
*  SYNOPSIS
*
*     JOB_TYPE_IMMEDIATE
*     JOB_TYPE_QSH
*     JOB_TYPE_QLOGIN
*     JOB_TYPE_QRSH
*     JOB_TYPE_QRLOGIN
*        
*     JOB_TYPE_NO_ERROR
*        When a job of this type fails and the error condition usually
*        would result in the job error state the job is finished. Thus
*        no qmod -c "*" is supported.
*******************************************************************************/

#define JOB_TYPE_IMMEDIATE  0x01UL
#define JOB_TYPE_QSH        0x02UL
#define JOB_TYPE_QLOGIN     0x04UL
#define JOB_TYPE_QRSH       0x08UL
#define JOB_TYPE_QRLOGIN    0x10UL
#define JOB_TYPE_NO_ERROR   0x20UL

/* submitted via "qsub -b y" or "qrsh [-b y]" */ 
#define JOB_TYPE_BINARY     0x40UL

/* array job (qsub -t ...) */
#define JOB_TYPE_ARRAY      0x80UL
/* Do a raw exec (qsub -noshell) */
#define JOB_TYPE_NO_SHELL   0x100UL

#define JOB_TYPE_QXXX_MASK \
   (JOB_TYPE_QSH | JOB_TYPE_QLOGIN | JOB_TYPE_QRSH | JOB_TYPE_QRLOGIN | JOB_TYPE_NO_ERROR)

#define JOB_TYPE_STR_IMMEDIATE  "IMMEDIATE"
#define JOB_TYPE_STR_QSH        "INTERACTIVE"
#define JOB_TYPE_STR_QLOGIN     "QLOGIN"
#define JOB_TYPE_STR_QRSH       "QRSH"
#define JOB_TYPE_STR_QRLOGIN    "QRLOGIN"
#define JOB_TYPE_STR_NO_ERROR   "NO_ERROR"

#define JOB_TYPE_CLEAR_IMMEDIATE(jb_now) \
   jb_now = jb_now & ~JOB_TYPE_IMMEDIATE 

#define JOB_TYPE_SET_IMMEDIATE(jb_now) \
   jb_now =  jb_now | JOB_TYPE_IMMEDIATE

#define JOB_TYPE_SET_QSH(jb_now) \
   jb_now = (jb_now & (~JOB_TYPE_QXXX_MASK)) | JOB_TYPE_QSH

#define JOB_TYPE_SET_QLOGIN(jb_now) \
   jb_now = (jb_now & (~JOB_TYPE_QXXX_MASK)) | JOB_TYPE_QLOGIN

#define JOB_TYPE_SET_QRSH(jb_now) \
   jb_now = (jb_now & ~JOB_TYPE_QXXX_MASK) | JOB_TYPE_QRSH

#define JOB_TYPE_SET_QRLOGIN(jb_now) \
   jb_now = (jb_now & ~JOB_TYPE_QXXX_MASK) | JOB_TYPE_QRLOGIN

#define JOB_TYPE_SET_BINARY(jb_now) \
   jb_now = jb_now | JOB_TYPE_BINARY

#define JOB_TYPE_CLEAR_BINARY(jb_now) \
   jb_now = jb_now & ~JOB_TYPE_BINARY

#define JOB_TYPE_SET_ARRAY(jb_now) \
   jb_now = jb_now | JOB_TYPE_ARRAY

#define JOB_TYPE_CLEAR_NO_ERROR(jb_now) \
   jb_now = jb_now & ~JOB_TYPE_NO_ERROR

#define JOB_TYPE_SET_NO_ERROR(jb_now) \
   jb_now =  jb_now | JOB_TYPE_NO_ERROR

#define JOB_TYPE_SET_NO_SHELL(jb_now) \
   jb_now =  jb_now | JOB_TYPE_NO_SHELL

#define JOB_TYPE_CLEAR_NO_SHELL(jb_now) \
   jb_now =  jb_now & ~JOB_TYPE_NO_SHELL

#define JOB_TYPE_UNSET_BINARY(jb_now) \
   jb_now = jb_now & ~JOB_TYPE_BINARY

#define JOB_TYPE_UNSET_NO_SHELL(jb_now) \
   jb_now =  jb_now & ~JOB_TYPE_NO_SHELL

#define JOB_TYPE_IS_IMMEDIATE(jb_now)      (jb_now & JOB_TYPE_IMMEDIATE)
#define JOB_TYPE_IS_QSH(jb_now)            (jb_now & JOB_TYPE_QSH)
#define JOB_TYPE_IS_QLOGIN(jb_now)         (jb_now & JOB_TYPE_QLOGIN)
#define JOB_TYPE_IS_QRSH(jb_now)           (jb_now & JOB_TYPE_QRSH)
#define JOB_TYPE_IS_QRLOGIN(jb_now)        (jb_now & JOB_TYPE_QRLOGIN)
#define JOB_TYPE_IS_BINARY(jb_now)         (jb_now & JOB_TYPE_BINARY)
#define JOB_TYPE_IS_ARRAY(jb_now)          (jb_now & JOB_TYPE_ARRAY)
#define JOB_TYPE_IS_NO_ERROR(jb_now)       (jb_now & JOB_TYPE_NO_ERROR)
#define JOB_TYPE_IS_NO_SHELL(jb_now)       (jb_now & JOB_TYPE_NO_SHELL)


bool job_is_enrolled(const lListElem *job, 
                     u_long32 ja_task_number);

u_long32 job_get_ja_tasks(const lListElem *job);

u_long32 job_get_not_enrolled_ja_tasks(const lListElem *job);

u_long32 job_get_enrolled_ja_tasks(const lListElem *job);

u_long32 job_get_submit_ja_tasks(const lListElem *job);
 
lListElem *job_enroll(lListElem *job, lList **answer_list, 
                      u_long32 task_number);  

void job_delete_not_enrolled_ja_task(lListElem *job, lList **answer_list,
                                     u_long32 ja_task_number);

int job_count_pending_tasks(lListElem *job, bool count_all);

bool job_has_soft_requests(lListElem *job);

bool job_is_ja_task_defined(const lListElem *job, u_long32 ja_task_number);

void job_set_hold_state(lListElem *job, 
                        lList **answer_list, u_long32 ja_task_id,
                        u_long32 new_hold_state);

u_long32 job_get_hold_state(lListElem *job, u_long32 ja_task_id);

/* int job_add_job(lList **job_list, char *name, lListElem *job, int check,
                 int hash, htable* Job_Hash_Table); */

void job_list_print(lList *job_list);

lListElem *job_get_ja_task_template(const lListElem *job, u_long32 ja_task_id); 

lListElem *job_get_ja_task_template_hold(const lListElem *job,
                                         u_long32 ja_task_id, 
                                         u_long32 hold_state);

lListElem *job_get_ja_task_template_pending(const lListElem *job,
                                            u_long32 ja_task_id);

lListElem *job_search_task(const lListElem *job, lList **answer_list, u_long32 ja_task_id);
lListElem *job_create_task(lListElem *job, lList **answer_list, u_long32 ja_task_id);

void job_add_as_zombie(lListElem *zombie, lList **answer_list, 
                       u_long32 ja_task_id);

int job_list_add_job(lList **job_list, const char *name, lListElem *job, 
                     int check); 

u_long32 job_get_ja_task_hold_state(const lListElem *job, u_long32 ja_task_id);

void job_destroy_hold_id_lists(lList *id_list[16]);

void job_create_hold_id_lists(const lListElem *job, lList *id_list[16],
                              u_long32 hold_state[16]);

bool job_is_zombie_job(const lListElem *job); 

const char *job_get_shell_start_mode(const lListElem *queue,
                                     const char *conf_shell_start_mode);

bool job_is_array(const lListElem *job); 

bool job_is_parallel(const lListElem *job);

bool job_is_tight_parallel(const lListElem *job, const lList *pe_list);

bool job_might_be_tight_parallel(const lListElem *job, const lList *pe_list);

void job_get_submit_task_ids(const lListElem *job, u_long32 *start, 
                             u_long32 *end, u_long32 *step); 

int job_set_submit_task_ids(lListElem *job, u_long32 start, u_long32 end,
                            u_long32 step);

u_long32 job_get_smallest_unenrolled_task_id(const lListElem *job);

u_long32 job_get_smallest_enrolled_task_id(const lListElem *job);

u_long32 job_get_biggest_unenrolled_task_id(const lListElem *job);

u_long32 job_get_biggest_enrolled_task_id(const lListElem *job);

int job_list_register_new_job(const lList *job_list, u_long32 max_jobs,
                              int force_registration);   

void jatask_list_print_to_string(const lList *task_list, dstring *range_string);

lList* ja_task_list_split_group(lList **task_list);

int job_initialize_id_lists(lListElem *job, lList **answer_list);

void job_initialize_env(lListElem *job, 
                        lList **answer_list,
                        const lList* path_alias_list,
                        const char *unqualified_hostname,
                        const char *qualified_hostname);

const char* job_get_env_string(const lListElem *job, const char *variable);

void job_set_env_string(lListElem *job, const char *variable, 
                        const char *value);

void job_check_correct_id_sublists(lListElem *job, lList **answer_list);

const char *job_get_id_string(u_long32 job_id, u_long32 ja_task_id, 
                              const char *pe_task_id, dstring *buffer);

const char *job_get_job_key(u_long32 job_id, dstring *buffer);

const char *job_get_key(u_long32 job_id, u_long32 ja_task_id, 
                        const char *pe_task_id, dstring *buffer);

const char *jobscript_get_key(lListElem *jep, dstring *buffer);

char *jobscript_parse_key(char *key,const char **exec_file);

bool job_parse_key(char *key, u_long32 *job_id, u_long32 *ja_task_id,
                   char **pe_task_id, bool *only_job);

bool job_is_pe_referenced(const lListElem *job, const lListElem *pe);

bool job_is_ckpt_referenced(const lListElem *job, const lListElem *ckpt);

void job_get_state_string(char *str, u_long32 op);

lListElem *job_list_locate(lList *job_list, u_long32 job_id);

void job_add_parent_id_to_context(lListElem *job);

int job_check_qsh_display(const lListElem *job, 
                          lList **answer_list, 
                          bool output_warning);

int job_check_owner(const char *user_name, u_long32 job_id, lList *master_job_list);

int job_resolve_host_for_path_list(const lListElem *job, lList **answer_list, int name);

lListElem *
job_get_request(const lListElem *this_elem, const char *centry_name);

bool
job_get_contribution(const lListElem *this_elem, lList **answer_list,
                     const char *name, double *value,
                     const lListElem *implicit_centry);

/* unparse functions */
bool sge_unparse_string_option_dstring(dstring *category_str, const lListElem *job_elem, 
                               int nm, char *option);

bool sge_unparse_ulong_option_dstring(dstring *category_str, const lListElem *job_elem, 
                               int nm, char *option);
                               
bool sge_unparse_pe_dstring(dstring *category_str, const lListElem *job_elem, int pe_pos, int range_pos,
                            const char *option); 

bool sge_unparse_resource_list_dstring(dstring *category_str, lListElem *job_elem, 
                                       int nm, const char *option);

bool sge_unparse_queue_list_dstring(dstring *category_str, lListElem *job_elem, 
                                    int nm, const char *option);   

bool sge_unparse_acl_dstring(dstring *category_str, const char *owner, const char *group, 
                             const lList *acl_list, const char *option);

bool job_verify(const lListElem *job, lList **answer_list, bool do_cull_verify);
bool job_verify_submitted_job(const lListElem *job, lList **answer_list);

bool job_get_wallclock_limit(u_long32 *limit, const lListElem *jep);

bool
job_is_binary(const lListElem *job);

bool
job_set_binary(lListElem *job, bool is_binary);

bool
job_is_no_shell(const lListElem *job);

bool
job_set_no_shell(lListElem *job, bool is_no_shell);

bool
job_set_owner_and_group(lListElem *job, u_long32 uid, u_long32 gid,
                        const char *user, const char *group);

bool  
job_get_ckpt_attr(int op, dstring *string);

bool
job_get_verify_attr(u_long32 op, dstring *string);

void 
set_context(lList *jbctx, lListElem *job);

bool 
job_parse_validation_level(int *level, const char *input, int prog_number, lList **answer_list);

bool
job_is_requesting_consumable(lListElem *jep, const char *resource_name);

bool
job_init_binding_elem(lListElem *jep);

int
job_list_sort(lList *this_list);

#endif /* __SGE_JOB_H */    
