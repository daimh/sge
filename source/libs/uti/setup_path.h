#ifndef __SETUP_PATH_H
#define __SETUP_PATH_H
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
#include "cull_list.h"
#include "sge_dstring.h"
#include "sge_env.h"

/* These files and directories will be found in COMMON_DIR
 * They must be accessed with an absolute path. Do not use these defines!
 */
#define ROOT_DIR                  "/usr/SGE"
#define COMMON_DIR                "common"
#define BOOTSTRAP_FILE            "bootstrap"
#define CONF_FILE                 "configuration"
#define SCHED_CONF_FILE           "sched_configuration"
#define ACCT_FILE                 "accounting"
#define REPORTING_FILE            "reporting"
#define LOCAL_CONF_DIR            "local_conf"
#define SHADOW_MASTERS_FILE       "shadow_masters"
#define ALIAS_FILE                "host_aliases"
#define QINSTANCES_DIR            "qinstances"

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

void path_mt_init(void);

#ifndef GDI_OFF

const char *path_state_get_sge_root(void);
const char *path_state_get_cell_root(void);
const char *path_state_get_bootstrap_file(void);
const char *path_state_get_conf_file(void);
const char *path_state_get_sched_conf_file(void);
const char *path_state_get_act_qmaster_file(void);
const char *path_state_get_acct_file(void);
const char *path_state_get_reporting_file(void);
const char *path_state_get_local_conf_dir(void);
const char *path_state_get_shadow_masters_file(void);

void path_state_set_sge_root(const char *path);
void path_state_set_cell_root(const char *path);
void path_state_set_conf_file(const char *path);
void path_state_set_sched_conf_file(const char *path);
void path_state_set_act_qmaster_file(const char *path);
void path_state_set_acct_file(const char *path);
void path_state_set_reporting_file(const char *path);
void path_state_set_local_conf_dir(const char *path);
void path_state_set_shadow_masters_file(const char *path);


bool sge_setup_paths(u_long32 progid, const char *cell, dstring *error_dstring);

#endif




typedef struct sge_path_state_class_str sge_path_state_class_t; 

struct sge_path_state_class_str {
   void *sge_path_state_handle;

   void (*dprintf)(sge_path_state_class_t *thiz);
   const char* (*get_sge_root)(sge_path_state_class_t *thiz);
   const char* (*get_cell_root)(sge_path_state_class_t *thiz);
   const char* (*get_conf_file)(sge_path_state_class_t *thiz);
   const char* (*get_bootstrap_file)(sge_path_state_class_t *thiz);
   const char* (*get_act_qmaster_file)(sge_path_state_class_t *thiz);
   const char* (*get_acct_file)(sge_path_state_class_t *thiz);
   const char* (*get_reporting_file)(sge_path_state_class_t *thiz);
   const char* (*get_local_conf_dir)(sge_path_state_class_t *thiz);
   const char* (*get_shadow_masters_file)(sge_path_state_class_t *thiz);
   const char* (*get_alias_file)(sge_path_state_class_t *thiz);

   void (*set_sge_root)(sge_path_state_class_t *thiz, const char *sge_root);
   void (*set_cell_root)(sge_path_state_class_t *thiz, const char *cell_root);
   void (*set_conf_file)(sge_path_state_class_t *thiz, const char *conf_file);
   void (*set_bootstrap_file)(sge_path_state_class_t *thiz, const char *bootstrap_file);
   void (*set_act_qmaster_file)(sge_path_state_class_t *thiz, const char *act_qmaster_file);
   void (*set_acct_file)(sge_path_state_class_t *thiz, const char *acct_file);
   void (*set_reporting_file)(sge_path_state_class_t *thiz, const char *reporting_file);
   void (*set_local_conf_dir)(sge_path_state_class_t *thiz, const char *local_conf_dir);
   void (*set_shadow_masters_file)(sge_path_state_class_t *thiz, const char *shadow_masters_file);
   void (*set_alias_file)(sge_path_state_class_t *thiz, const char* alias_file);
   void (*set_sched_conf_file)(sge_path_state_class_t *thiz, const char* sched_conf_file);
};

sge_path_state_class_t *sge_path_state_class_create(sge_env_state_class_t *sge_env, sge_error_class_t *eh);
void sge_path_state_class_destroy(sge_path_state_class_t **pst);

#endif /* __SETUP_PATH_H */
