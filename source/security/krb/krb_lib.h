#ifndef __KRB_LIB_H
#define __KRB_LIB_H
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

#include <sys/types.h>
#include <krb5.h>

#include "cull/cull_list.h"

#define FAR
#define krb5_xfree(val) free((char FAR *)(val))

#define KRB_CLIENT_TIMEOUT (15*60)

int krb_init(const char *progname);
int krb_send_message(int synchron, const char *tocomproc, int toid, const char *tohost, int tag, char *buffer, int buflen, u_long32 *mid);
int krb_receive_message(char *fromcommproc, u_short *fromid, char *fromhost, int *tag, char **buffer, u_long32 *buflen, int synchron);
int krb_verify_user(const char *host, const char *commproc, int id, const char *user);
int krb_check_for_idle_clients(void);
char *krb_bin2str(void *data, int len, char *str);
void * krb_str2bin(const char *str, void *data, int *rlen);
krb5_error_code krb_encrypt_tgt_creds(krb5_creds **tgt_creds, krb5_data *outbuf);
krb5_error_code krb_decrypt_tgt_creds(krb5_data *inbuf, krb5_creds ***tgt_creds);
int krb_get_client_flags(void);
int krb_set_client_flags(int flags);
int krb_clear_client_flags(int flags);
int krb_put_tgt(const char *host, const char *comproc, int id, u_long tgt_id, krb5_creds **tgt_creds);
int krb_get_tgt(const char *host, const char *comproc, int id, u_long tgt_id, krb5_creds ***tgt_creds);
int krb_store_forwarded_tgt(int uid, int jobid, krb5_creds **tgt_creds);
int krb_destroy_forwarded_tgt(int jobid);
char *krb_get_ccname(int jobid, char *ccname);
krb5_context krb_context(void);
int krb_renew_tgts(lList *joblist);
void krb_set_tgt_id(u_long tgt_id);


#define KRB_FORWARD_TGT 0x0001

#endif /* __KRB_LIB_H */

