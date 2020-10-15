#ifndef __SGE_ENV_H
#define __SGE_ENV_H
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

typedef struct sge_env_state_class_str sge_env_state_class_t; 

struct sge_env_state_class_str {
   void *sge_env_state_handle;

   void (*dprintf)(sge_env_state_class_t *thiz);
   const char* (*get_sge_root)(sge_env_state_class_t *thiz);
   const char* (*get_sge_cell)(sge_env_state_class_t *thiz);
   bool (*is_from_services)(sge_env_state_class_t *thiz);
   bool (*is_qmaster_internal)(sge_env_state_class_t *thiz);
   u_long32 (*get_sge_qmaster_port)(sge_env_state_class_t *thiz);
   u_long32 (*get_sge_execd_port)(sge_env_state_class_t *thiz);
   void (*set_sge_root)(sge_env_state_class_t *thiz, const char *sge_root);
   void (*set_sge_cell)(sge_env_state_class_t *thiz, const char *sge_cell);
   void (*set_sge_qmaster_port)(sge_env_state_class_t *thiz, u_long32 sge_qmaster_port);
   void (*set_sge_execd_port)(sge_env_state_class_t *thiz, u_long32 sge_qmaster_port);
};

sge_env_state_class_t *sge_env_state_class_create(const char *sge_root, const char *sge_cell, int sge_qmaster_port, int sge_execd_port, bool from_services, bool qmaster_internal, sge_error_class_t *eh);
void sge_env_state_class_destroy(sge_env_state_class_t **pst);

#endif /* __SGE_ENV_H */


