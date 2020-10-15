#ifndef __SGE_CSP_PATH_H
#define __SGE_CSP_PATH_H
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

#include "sge_error_class.h"
#include "sge_env.h"
#include "cl_data_types.h"

typedef struct sge_csp_path_class_str sge_csp_path_class_t; 

struct sge_csp_path_class_str {
   void *sge_csp_path_handle;

   void (*dprintf)(sge_csp_path_class_t *thiz);
   const char* (*get_ca_root)(sge_csp_path_class_t *thiz);
   const char* (*get_ca_local_root)(sge_csp_path_class_t *thiz);
   const char* (*get_CA_cert_file)(sge_csp_path_class_t *thiz);
   const char* (*get_CA_key_file)(sge_csp_path_class_t *thiz);
   const char* (*get_cert_file)(sge_csp_path_class_t *thiz);
   const char* (*get_key_file)(sge_csp_path_class_t *thiz);
   const char* (*get_rand_file)(sge_csp_path_class_t *thiz);
   const char* (*get_reconnect_file)(sge_csp_path_class_t *thiz);
   const char* (*get_crl_file)(sge_csp_path_class_t *thiz);
   const char* (*get_password)(sge_csp_path_class_t *thiz);
   int (*get_refresh_time)(sge_csp_path_class_t *thiz);
   cl_ssl_verify_func_t (*get_verify_func)(sge_csp_path_class_t *thiz);

   void (*set_CA_cert_file)(sge_csp_path_class_t *thiz, const char *CA_cert_file);
   void (*set_CA_key_file)(sge_csp_path_class_t *thiz, const char *CA_key_file);
   void (*set_cert_file)(sge_csp_path_class_t *thiz, const char *cert_file);
   void (*set_key_file)(sge_csp_path_class_t *thiz, const char *key_file);
   void (*set_rand_file)(sge_csp_path_class_t *thiz, const char *rand_file);
   void (*set_reconnect_file)(sge_csp_path_class_t *thiz, const char *reconnect_file);
   void (*set_crl_file)(sge_csp_path_class_t *thiz, const char *crl_file);
   void (*set_password)(sge_csp_path_class_t *thiz, const char *password);
   void (*set_refresh_time)(sge_csp_path_class_t *thiz, u_long32 refresh_time);
   void (*set_verify_func)(sge_csp_path_class_t *thiz, cl_ssl_verify_func_t verify_func);
};

sge_csp_path_class_t *sge_csp_path_class_create(sge_env_state_class_t *sge_env, sge_prog_state_class_t *sge_prog, sge_error_class_t *eh);
void sge_csp_path_class_destroy(sge_csp_path_class_t **pst);

#endif /* __SGE_CSP_PATH_H */


