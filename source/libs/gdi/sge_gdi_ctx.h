#ifndef __SGE_GDI_CTX_H
#define __SGE_GDI_CTX_H
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

#include "uti/sge_env.h"
#include "uti/sge_prog.h"
#include "uti/setup_path.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_error_class.h"
#include "uti/sge_profiling.h"
#include "uti/sge_uidgid.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi_packet_type.h"
#include "gdi/sge_gdi_ctx_type.h"

#include "sge.h"

int 
sge_gdi2_setup(sge_gdi_ctx_class_t **context, u_long32 progid, u_long32 thread_id, lList **alpp);

int 
sge_setup2(sge_gdi_ctx_class_t **context, u_long32 progid, u_long32 thread_id,
           lList **alpp, bool is_qmaster_intern_client);

sge_gdi_ctx_class_t *
sge_gdi_ctx_class_create(int prog_number, const char *component_name,
                         int thread_number, const char *thread_name,
                         const char* username, const char* groupname, 
                         const char* sge_root, const char* sge_cell, 
                         int sge_qmaster_port, int sge_execd_port,
                         bool from_services, bool is_qmaster_intern_client,
                         lList **alpp);



/*
   create an instance of sge_gdi_ctx_class_t and initialize it
   with the information of the bootstrap file

   @param prog_number  number of the program which uses the context
   @param url          url to the bootstrap file
   @param username     name of the user which uses the context
   @param alpp         answer list reference
   @return the gdi context

   TODO we need a answer list to transport error messages to the caller
*/
sge_gdi_ctx_class_t *
sge_gdi_ctx_class_create_from_bootstrap(int prog_number, const char* component_name,
                                        int thread_number, const char *thread_name,
                                        const char* url, const char* username, lList **alpp);

void sge_gdi_ctx_class_destroy(sge_gdi_ctx_class_t **pst);

sge_gdi_ctx_class_t* sge_gdi_get_thread_local_ctx(void);
void sge_gdi_set_thread_local_ctx(sge_gdi_ctx_class_t* ctx);

void gdi_mt_init(void);

bool sge_daemonize_prepare(sge_gdi_ctx_class_t *context);
bool sge_daemonize_finalize(sge_gdi_ctx_class_t *context);

int sge_daemonize(int *keep_open, unsigned long nr_of_fds, sge_gdi_ctx_class_t *context);

#endif /* __SGE_GDI_CTX_H */


