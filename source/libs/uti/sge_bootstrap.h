#ifndef __SGE_BOOTSTRAP_H
#define __SGE_BOOTSTRAP_H
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

#include "basis_types.h"
#include "sge_dstring.h"
#include "setup_path.h"

void bootstrap_mt_init(void);

#ifndef GDI_OFF

const char *bootstrap_get_admin_user(void);
const char *bootstrap_get_default_domain(void);
bool        bootstrap_get_ignore_fqdn(void);
const char *bootstrap_get_spooling_method(void);
const char *bootstrap_get_spooling_lib(void);
const char *bootstrap_get_spooling_params(void);
const char *bootstrap_get_binary_path(void);
const char *bootstrap_get_qmaster_spool_dir(void);
const char *bootstrap_get_security_mode(void);
bool        bootstrap_get_job_spooling(void);
int         bootstrap_get_listener_thread_count(void);
int         bootstrap_get_worker_thread_count(void);
int         bootstrap_get_scheduler_thread_count(void);
int         bootstrap_get_jvm_thread_count(void);

void bootstrap_set_admin_user(const char *value);
void bootstrap_set_default_domain(const char *value);
void bootstrap_set_ignore_fqdn(bool value);
void bootstrap_set_spooling_method(const char *value);
void bootstrap_set_spooling_lib(const char *value);
void bootstrap_set_spooling_params(const char *value);
void bootstrap_set_binary_path(const char *value);
void bootstrap_set_qmaster_spool_dir(const char *value);
void bootstrap_set_security_mode(const char *value);
void bootstrap_set_job_spooling(bool value);
void bootstrap_set_listener_thread_count(int value);
void bootstrap_set_worker_thread_count(int value);
void bootstrap_set_scheduler_thread_count(int value);
void bootstrap_set_jvm_thread_count(int value);

#endif

bool sge_bootstrap(const char *bootstrap_file, dstring *error_dstring);

typedef struct sge_bootstrap_state_class_str sge_bootstrap_state_class_t; 

struct sge_bootstrap_state_class_str {
   void *sge_bootstrap_state_handle;

   void (*dprintf)(sge_bootstrap_state_class_t *thiz);
   
   const char* (*get_admin_user)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_default_domain)(sge_bootstrap_state_class_t *thiz);
   bool (*get_ignore_fqdn)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_spooling_method)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_spooling_lib)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_spooling_params)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_binary_path)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_qmaster_spool_dir)(sge_bootstrap_state_class_t *thiz);
   const char* (*get_security_mode)(sge_bootstrap_state_class_t *thiz);
   bool (*get_job_spooling)(sge_bootstrap_state_class_t *thiz);
   int (*get_listener_thread_count)(sge_bootstrap_state_class_t *thiz);
   int (*get_worker_thread_count)(sge_bootstrap_state_class_t *thiz);
   int (*get_scheduler_thread_count)(sge_bootstrap_state_class_t *thiz);
   int (*get_jvm_thread_count)(sge_bootstrap_state_class_t *thiz);

   void (*set_admin_user)(sge_bootstrap_state_class_t *thiz, const char *admin_user);
   void (*set_default_domain)(sge_bootstrap_state_class_t *thiz, const char *default_domain);
   void (*set_ignore_fqdn)(sge_bootstrap_state_class_t *thiz, bool ignore_fqdn);
   void (*set_spooling_method)(sge_bootstrap_state_class_t *thiz, const char *spooling_method);
   void (*set_spooling_lib)(sge_bootstrap_state_class_t *thiz, const char *spooling_lib);
   void (*set_spooling_params)(sge_bootstrap_state_class_t *thiz, const char *spooling_params);
   void (*set_binary_path)(sge_bootstrap_state_class_t *thiz, const char *binary_path);
   void (*set_qmaster_spool_dir)(sge_bootstrap_state_class_t *thiz, const char *qmaster_spool_dir);
   void (*set_security_mode)(sge_bootstrap_state_class_t *thiz, const char *security_mode);
   void (*set_job_spooling)(sge_bootstrap_state_class_t *thiz, bool job_spooling);
   void (*set_listener_thread_count)(sge_bootstrap_state_class_t *thiz, int thread_count);
   void (*set_worker_thread_count)(sge_bootstrap_state_class_t *thiz, int thread_count);
   void (*set_scheduler_thread_count)(sge_bootstrap_state_class_t *thiz, int thread_count);
   void (*set_jvm_thread_count)(sge_bootstrap_state_class_t *thiz, int thread_count);
};

sge_bootstrap_state_class_t *sge_bootstrap_state_class_create(sge_path_state_class_t *sge_paths, sge_error_class_t *eh);
void sge_bootstrap_state_class_destroy(sge_bootstrap_state_class_t **pst);

void sge_bootstrap_state_set_thread_local(sge_bootstrap_state_class_t* ctx);


#endif /* __SGE_BOOTSTRAP_H */
