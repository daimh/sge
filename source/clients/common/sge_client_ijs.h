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
/* Portions of this code are Copyright 2011 Univa Inc. */
/*___INFO__MARK_END__*/

#define COMM_SERVER "qrsh_ijs"
#define COMM_CLIENT "shepherd_ijs"

void set_signal_handlers(void);
void* tty_to_commlib(void *t_conf);
void* commlib_to_tty(void *t_conf);

int start_ijs_server(bool csp_mode,
   const char *username, COMM_HANDLE **phandle, dstring *p_err_msg);

int run_ijs_server(COMM_HANDLE *phandle, const char *remote_host,
   u_long32 job_id, int nostdin, int noshell, int is_rsh, int is_qlogin,
   ternary_t force_pty, ternary_t suspend_remote, int *p_exit_status,
   dstring *p_err_msg);

int stop_ijs_server(COMM_HANDLE **phandle, dstring *p_err_msg);

int force_ijs_server_shutdown(COMM_HANDLE **phandle,
   const char *this_component, dstring *err_msg);
