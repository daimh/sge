
#ifndef __SGE_GSSLIB_H
#define __SGE_GSSLIB_H
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

#define GSSLIB_INTSIZE 4
#define GSSLIB_INTOFF 0 /* everybody else */

void gsslib_packint(u_long hostlong, char *buf);
u_long gsslib_unpackint(char *buf);
void gsslib_display_status(const char *msg, OM_uint32 maj_stat, OM_uint32 min_stat);
void gsslib_reset_error(void);
char *gsslib_print_error(const char *msg);
char *gsslib_error(void);
void gsslib_display_ctx_flags(OM_uint32 flags);
int gsslib_get_credentials(char *service_name, gss_buffer_desc *cred,
                           gss_cred_id_t client_cred);
int gsslib_acquire_client_credentials(gss_cred_id_t *client_creds);
int gsslib_acquire_server_credentials(char *service_name,
                                      gss_cred_id_t *server_creds);
int gsslib_put_credentials(gss_cred_id_t server_creds,
                           gss_buffer_desc *cred,
                           char *username);
void gsslib_verbose(int verbosity);

#ifdef DCE
sec_login_flags_t
sec_login_get_context_flags(sec_login_handle_t *login_context,
			    error_status_t *status);
void
sec_login_set_context_flags(sec_login_handle_t *login_context,
			    sec_login_flags_t flags,
			    error_status_t *status);
#endif

#if 0
#define SERVICE_NAME "sge"
#else
#define SERVICE_NAME "sge"
#endif

#ifdef KRBGSS
#define GSSAPI_INT OM_uint32
#else
#define GSSAPI_INT int
#endif

#endif /* __SGE_GSSLIB_H */

