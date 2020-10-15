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

#include <pwd.h>
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "basis_types.h"


int    wl_encrypt(const char *uncrypted, char* encrypted, int bufsize);

int    wl_getpwuid_ex_r(uid_t uid, struct passwd *pwd, char *buffer,
                        size_t bufsize, struct passwd **result, uint flags);
bool   wl_handle_ls_results(const char *name, const char *value, 
                            const char *host, char *error_buffer);

uid_t  wl_get_superuser_id();
gid_t  wl_get_superuser_gid();
int    wl_get_superuser_name(char *buf, int bufsize);
bool   wl_is_user_id_superuser(int uid);

int    wl_setuser(int uid, int gid, const char *pass, char *err_str);
bool   wl_use_sgepasswd();
void   wl_set_use_sgepasswd(bool use_it);

char*  wl_strip_hostname(char *user_name);
int    wl_build_fq_local_name(const char *user, char *fq_name);

int    wl_stat(const char *path, struct stat *buf);
int    wl_statvfs(const char *path, struct statvfs *buf);

void   wl_xdrmem_create(XDR *xdrs, const caddr_t addr,
                        const uint_t size, const enum xdr_op op);
void   wl_xdr_destroy(XDR *xdrs);
bool_t wl_xdr_double(XDR *xdrs, double *dp);
uint_t wl_xdr_getpos(const XDR *xdrs);
