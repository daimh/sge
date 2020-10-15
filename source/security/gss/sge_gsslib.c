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
 *   Copyright (C) 2012 Dave Love, University of Liverpool
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

/* This appears to be substantially derived from the example in the MIT
   Kerberos distribution, which has this notice.

 * Copyright 1994 by OpenVision Technologies, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (C) 2004,2005 by the Massachusetts Institute of Technology.
 * All rights reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifdef KRBGSS
#include <krb5.h>
#include <gssapi/gssapi_krb5.h>
#ifndef HEIMDAL
#include <gssapi/gssapi_generic.h>
#endif
#else
#include <gssapi.h>
#endif

#ifdef DCE
#include <dce/sec_login.h>
#include <dce/dce_error.h>
#include <dce/passwd.h>
#include <dce/binding.h>
#include <dce/pgo.h>
#endif

#include "sge_gsslib.h"
#include "msg_gss.h"

/* fixme:  It's not clear what KRB5_EXPORTVAR is about.  It only seems
   to be defined in an MS Windows header in MIT Kerberos.  */
#ifndef KRB5_EXPORTVAR
#define KRB5_EXPORTVAR
#endif
#ifdef KRB5_EXPORTVAR /* this is required for later Kerberos versions */
static void put_creds_in_ccache(char *name, gss_cred_id_t creds);
#endif

#define REQUIRE_FORWARDED_CREDENTIALS
#define DELEGATION
/* #define TURN_OFF_DELEGATION */
#define HACK

static char msgbuf[2048];
static char *msgptr = NULL;
static int verbose = 0;

void gsslib_packint(u_long hostlong, char *buf)
{
   u_long netlong = htonl(hostlong);
   memcpy(buf, (((char *)&netlong)+GSSLIB_INTOFF), GSSLIB_INTSIZE);
}

u_long gsslib_unpackint(char *buf)
{
   u_long netlong;
   memset(&netlong, 0, sizeof(netlong));
   memcpy(((char *)&netlong)+GSSLIB_INTOFF, buf, GSSLIB_INTSIZE);
   return ntohl(netlong);
}

static void gsslib_display_status_1(const char *m, OM_uint32 code, int type)
{
   OM_uint32 maj_stat, min_stat;
   gss_buffer_desc msg;
   GSSAPI_INT msg_ctx;

   memset((void *)&msg, 0, sizeof(msg));
   msg_ctx = 0;
   while (1) {
      maj_stat = gss_display_status(&min_stat, code,
                                    type, GSS_C_NULL_OID,
                                    &msg_ctx, &msg);
      snprintf(msgptr, sizeof msgbuf - strlen(msgptr), MSG_GSS_APIERRORXY_SS,
               m?m:"<>", (char *)msg.value?(char *)msg.value:"<>");
      msgptr += strlen(msgptr);
      snprintf(msgptr, sizeof msgbuf - strlen(msgptr), "\n");
      (void) gss_release_buffer(&min_stat, &msg);
      msgptr += strlen(msgptr);

      if (!msg_ctx)
         break;
   }
}

/*
 * set verbose level
 */

void gsslib_verbose(int verbosity)
{
   verbose = verbosity;
}

void gsslib_display_status(const char *msg, OM_uint32 maj_stat, OM_uint32 min_stat)
{
   msgptr = msgbuf;
   gsslib_display_status_1(msg, maj_stat, GSS_C_GSS_CODE);
   if (min_stat != GSS_S_COMPLETE)
      gsslib_display_status_1(msg, min_stat, GSS_C_MECH_CODE);
}


void gsslib_reset_error(void)
{
   msgptr = msgbuf;
   msgptr[0] = 0;
   return;
}


char *gsslib_print_error(const char *msg)
{
   snprintf(msgptr, sizeof msgbuf - strlen (msgptr), "%s\n", msg);
   msgptr += strlen(msg);
   return msgbuf;
}


char *gsslib_error(void)
{
   return msgbuf;
}


void gsslib_display_ctx_flags(OM_uint32 flags)
{
   if (flags & GSS_C_DELEG_FLAG)
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_DELEG_FLAG);
   if (flags & GSS_C_MUTUAL_FLAG)
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_MUTUAL_FLAG);
   if (flags & GSS_C_REPLAY_FLAG)
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_REPLAY_FLAG);
   if (flags & GSS_C_SEQUENCE_FLAG)
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_SEQUENCE_FLAG);
   if (flags & GSS_C_CONF_FLAG )
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_CONF_FLAG);
   if (flags & GSS_C_INTEG_FLAG )
      gsslib_print_error(MSG_GSS_CONTEXTFLAG_GSS_C_INTEG_FLAG);
   /* fixme:  not handled:  GSS_C_ANON_FLAG GSS_C_PROT_READY_FLAG
      GSS_C_TRANS_FLAG GSS_C_DELEG_POLICY_FLAG */
}


int
gsslib_get_credentials(char *service_name, gss_buffer_desc *cred,
                       gss_cred_id_t client_cred)
{
   gss_OID oid = GSS_C_NULL_OID;
   gss_buffer_desc tok, *token_ptr;
   OM_uint32 maj_stat, min_stat;
   GSSAPI_INT ret_flags;
   gss_name_t target_name;
   gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
   int cc = 0;
#ifdef KRBGSS
   gss_OID name_oid = GSS_C_NT_HOSTBASED_SERVICE;
#else
   gss_OID name_oid = GSS_C_NULL_OID;
#endif

   gsslib_reset_error();

   /*
    * tokenize service name
    */

   tok.value = service_name;
   tok.length = strlen(tok.value)+1;
   maj_stat = gss_import_name(&min_stat, &tok, name_oid, &target_name);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_PARSINGNAME , maj_stat, min_stat);
      return -1;
   }

   if (verbose) {
      gss_buffer_desc service_name;
      gss_OID doid;
      service_name.length = 0;
      maj_stat = gss_display_name(&min_stat, target_name, &service_name, &doid);
      if (maj_stat != GSS_S_COMPLETE) {
         gsslib_display_status(MSG_GSS_DISPLAYSTATUS_DISPLAYINGNAME, maj_stat, min_stat);
         cc = -1;
         goto error;
      }
      fprintf(stderr, "service: \"%.*s\"\n",
             (int) service_name.length, (char *) service_name.value);
      if (service_name.length)
         (void) gss_release_buffer(&min_stat, &service_name);
   }

   token_ptr = GSS_C_NO_BUFFER;

   /*
    * get credentials
    */

   maj_stat = gss_init_sec_context(&min_stat,                
                                   client_cred,      
                                   &gss_context,              
                                   target_name,              
                                   oid,                      
#ifdef DELEGATION
                                   GSS_C_REPLAY_FLAG | GSS_C_DELEG_FLAG,
#else
                                   GSS_C_REPLAY_FLAG,
#endif
                                   0,                        
                                   NULL,       /* no channel bindings */
                                   token_ptr,                
                                   NULL,       /* ignore mech type */
                                   cred,
                                   &ret_flags,
                                   NULL);      /* ignore time_rec */

   if (maj_stat!=GSS_S_COMPLETE && maj_stat!=GSS_S_CONTINUE_NEEDED) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_INITIALIZINGCONTEXT, maj_stat, min_stat);
      cc = -1;
      goto error;
   }

error:

   (void) gss_release_name(&min_stat, &target_name);        
   return cc;                                                
}


/*
 * gsslib_acquire_client_credentials
 * 
 *      Acquire credentials for a particular client. Called by
 *      a client to establish the client's credentials.
 *
 */

int
gsslib_acquire_client_credentials(gss_cred_id_t *client_creds)
{
   gss_name_t client_name = NULL;
   OM_uint32 maj_stat, min_stat;
   int cc=0;
   
   gsslib_reset_error();

   /*
    * get credentials for the service
    */

   maj_stat = gss_acquire_cred(&min_stat, client_name, 0,
                               GSS_C_NULL_OID_SET, GSS_C_INITIATE,
                               client_creds, NULL, NULL);

   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_ACQUIRINGCREDENTIALS , maj_stat, min_stat);
      return -1;
   }

   if (client_name)
      (void) gss_release_name(&min_stat, &client_name);

   return cc;
}


/*
 * gsslib_acquire_server_credentials
 * 
 *      Acquire credentials for a particular service. Called by
 *      a server providing a service to establish the server's
 *      credentials.
 *
 */

int
gsslib_acquire_server_credentials(char *service_name,
                                  gss_cred_id_t *server_creds)
{
   gss_buffer_desc name_buf;
   gss_name_t server_name;
   OM_uint32 maj_stat, min_stat;
   int cc=0;
#ifdef KRBGSS
   gss_OID oid = GSS_C_NT_HOSTBASED_SERVICE;
#else
   gss_OID oid = GSS_C_NULL_OID;
#endif
   
   gsslib_reset_error();

   name_buf.value = service_name;
   name_buf.length = strlen(name_buf.value) + 1;
   maj_stat = gss_import_name(&min_stat, &name_buf, oid, &server_name);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_IMPORTINGNAME, maj_stat, min_stat);
      cc = -1;
      goto error;
   }

#ifdef DCE

   maj_stat =
     gssdce_register_acceptor_identity(&min_stat,
				       server_name,
				       NULL,
				       NULL);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_REGISTERINGIDENTITY , maj_stat, min_stat);
      return -1;
   }

#ifdef DEBUG
   fprintf(stderr, "Registered identity...\n");
   fflush(stderr);
#endif

#endif

   /*
    * get credentials for the service
    */

   maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
                               GSS_C_NULL_OID_SET,
#ifdef DCE
                               GSS_C_BOTH,
#else
			       GSS_C_ACCEPT,
#endif
                               server_creds, NULL, NULL);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_ACQUIRINGCREDENTIALS , maj_stat, min_stat);
      return -1;
   }

 error:

   if (server_name)
      (void) gss_release_name(&min_stat, &server_name);

   return cc;
}


/*
 * gsslib_put_credentials
 * 
 *      Takes the credentials passed in cred and authenticates
 *      the user, and establishes the forwarded credentials.
 *      If username is supplied, then verify the username
 *      matches the client's username.
 */

int
gsslib_put_credentials(gss_cred_id_t server_creds,
                       gss_buffer_desc *cred,
                       char *username)
{
   gss_ctx_id_t context = GSS_C_NO_CONTEXT;
   gss_buffer_desc client_name;
   OM_uint32 maj_stat, min_stat;
   GSSAPI_INT ret_flags;
   gss_buffer_desc send_tok;
   gss_name_t client = NULL;
   gss_OID doid;
   int cc=0;
   gss_cred_id_t delegated_cred = GSS_C_NO_CREDENTIAL;

   gsslib_reset_error();

   send_tok.length = 0;
   client_name.length = 0;

   if (cred->length <= 0) {
      gsslib_print_error(MSG_GSS_PRINTERROR_CREDENTIALBUFFERLENGTHISZERO );
      cc = -1;
      goto error;
   }

   /*
    * establish and forward client credentials
    */

   maj_stat = gss_accept_sec_context(&min_stat,
                                     &context,
                                     server_creds,
                                     cred,
                                     GSS_C_NO_CHANNEL_BINDINGS,
                                     &client,
                                     &doid,
                                     &send_tok,
                                     &ret_flags,
                                     NULL,     /* ignore time_rec */
                                     &delegated_cred);    /* ignore del_cred_handle */

   if (maj_stat!=GSS_S_COMPLETE && maj_stat!=GSS_S_CONTINUE_NEEDED) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_ACCEPTINGCONTEXT, maj_stat, min_stat);
      cc = -1;
      goto error;
   }

   if (send_tok.length != 0) {
      fprintf(stderr, "%s\n", MSG_GSS_ACCEPTSECCONTEXTREQUIRESTOKENTOBESENTBACK );
      /* cc = -1;
      goto error; */
   }

   maj_stat = gss_display_name(&min_stat, client, &client_name, &doid);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_DISPLAYSTATUS_DISPLAYINGNAME, maj_stat, min_stat);
      cc = -1;
      goto error;
   }

#ifdef KRBGSS
#ifdef KRB5_EXPORTVAR /* this is required for later Kerberos versions */

   /* check for delegated credential */
   if (delegated_cred == GSS_C_NO_CREDENTIAL) {
      fprintf(stderr, "WARNING: Credentials were not forwarded\n");
#ifdef REQUIRE_FORWARDED_CREDENTIALS
      cc = 3;
      goto error;
#endif
   }

   if (username && (ret_flags & GSS_C_DELEG_FLAG)) {
      char *principal = malloc(client_name.length + 1);
      strncpy(principal, client_name.value, client_name.length);
      principal[client_name.length] = 0;
      put_creds_in_ccache(principal, delegated_cred);
      free(principal);
   }

#endif
#endif

   /* display the flags */
   if (verbose)
      gsslib_display_ctx_flags(ret_flags);

   if (verbose)
      printf("client: \"%.*s\"\n",
             (int) client_name.length, (char *) client_name.value);

   if (username) {
      gss_buffer_desc tok;
      gss_name_t user_name;
      int str_equal;

      tok.value = username;
      tok.length = strlen(tok.value)+1;
      maj_stat = gss_import_name(&min_stat, &tok, GSS_C_NULL_OID, &user_name);
      if (maj_stat != GSS_S_COMPLETE) {
	 gsslib_display_status(MSG_GSS_DISPLAYSTATUS_PARSINGNAME, maj_stat, min_stat);
         cc = -1;
         goto error;
      }
      maj_stat = gss_compare_name(&min_stat, client, user_name, &str_equal);
      if (maj_stat != GSS_S_COMPLETE) {
	 gsslib_display_status( MSG_GSS_DISPLAYSTATUS_DISPLAYINGNAME, maj_stat, min_stat);
         cc = 6;
         goto error;
      }

#ifdef KRBGSS

      if (!str_equal) {
         krb5_context context;

         maj_stat = krb5_init_context(&context);
         if (maj_stat != GSS_S_COMPLETE) {
            gsslib_display_status(MSG_GSS_DISPLAYSTATUS_GETTINGKRB5CONTEXT,
                                  maj_stat, GSS_S_COMPLETE);
            cc = -1;
            goto error;
         }

         /* see if this user is authorized by the krb5 client */
         if (krb5_kuserok(context, (krb5_principal)client, username))
            str_equal = 1;
      }

      /* Users from Kerberos cross-authenticated realms will not match,
         so we manually compare the user names */
      if (!str_equal) {
         char *s;
         if ((s=strchr((char *)client_name.value, '@')))
            str_equal = !strncmp(username, (char *)client_name.value, 
                                 s-(char *)client_name.value);
      }

#endif
      if (!str_equal) {
         char buf[1024];
         snprintf(buf, sizeof(buf), MSG_GSS_CLIENTNAMEXDOESNOTMATCHUNAMEY_SS,
                  (int)client_name.length, (char *)client_name.value, username);
         gsslib_print_error(buf);
         cc = 5;
         goto error;
      }
   }

#ifdef DCE

   while (delegated_cred) {
      sec_login_handle_t login_context;
      error_status_t st;
      dce_error_string_t err_string;
      int lst;
      sec_login_auth_src_t auth_src=NULL;
      boolean32 reset_passwd=0;
      char errbuf[1024];
      unsigned32 num_groups=0;
      signed32 *groups=NULL;
      unsigned32 flags;

      maj_stat = gssdce_set_cred_context_ownership(&min_stat, delegated_cred, GSSDCE_C_OWNERSHIP_APPLICATION);
      if (maj_stat != GSS_S_COMPLETE) {
	 gsslib_display_status(MSG_GSS_DISPLAYSTATUS_GSSDCESETCREDCONTEXTOWNERSHIP, maj_stat, min_stat);
	 break;
      }

#if 0
      gsslib_print_error(MSG_GSS_PRINTERROR_CREDENTIALDUMP);
      gsslib_print_error(dump_cred(delegated_cred));
#endif

      maj_stat = gssdce_cred_to_login_context(&min_stat, delegated_cred,
					      &login_context);
      if (maj_stat != GSS_S_COMPLETE) {
	 gsslib_display_status(MSG_GSS_DISPLAYSTATUS_GSSDCECREDTOLOGINCONTEXT, maj_stat,
			       min_stat);
	 break;
      }

#ifdef TURN_OFF_DELEGATION
      {
         sec_login_handle_t *new_login_context;

         new_login_context = sec_login_disable_delegation(login_context, &st);
         if (st != error_status_ok) {
            dce_error_inq_text(st, err_string, &lst);
            snprintf(errbuf, sizeof errbuf,
                     MSG_GSS_PRINTERROR_COULDNOTDISABLEDELEGATIONX_S, err_string);
            gsslib_print_error(errbuf);
         } else {
            login_context = *new_login_context;
         }
      }
#endif

      flags = sec_login_get_context_flags(login_context, &st);
      sec_login_set_context_flags(login_context,
				  flags & ~sec_login_credentials_private,
				  &st);


      if (!sec_login_certify_identity(login_context, &st)) {
	 dce_error_inq_text(st, err_string, &lst);
	 snprintf(errbuf, sizeof errbuf,
                  MSG_GSS_PRINTERROR_COULDNOTCERTIFYIDENTITYX_S, err_string);
         gsslib_print_error(errbuf);
	 break;
      }

      sec_login_set_context(login_context, &st);
      if (st != error_status_ok) {
         dce_error_inq_text(st, err_string, &lst);
	 snprintf(errbuf, sizeof errbuf,
                  MSG_GSS_PRINTERROR_COULDNOTSETUPLOGINCONTEXTX_S, err_string);
         gsslib_print_error(errbuf);
	 break;
      }

      {
	 char *cp;
	 cp = getenv("KRB5CCNAME");
	 if (cp) {
            snprintf(errbuf, sizeof errbuf, MSG_GSS_PRINTERROR_NEWKRB5CCNAMEISX_S , cp);
	    gsslib_print_error(errbuf);
	 } else {
	    gsslib_print_error(MSG_GSS_PRINTERROR_KRB5CCNAMENOTFOUND );
	 }
      }

      break;
   }

#endif /* DCE */

 error:

   if (client) {
      maj_stat = gss_release_name(&min_stat, &client);
      if (maj_stat != GSS_S_COMPLETE) {
         gsslib_display_status(MSG_GSS_DISPLAYSTATUS_RELEASINGNAME, maj_stat, min_stat);
         cc = -1;
      }
   }

   if (send_tok.length)
      (void) gss_release_buffer(&min_stat, &send_tok);

   if (client_name.length)
      (void) gss_release_buffer(&min_stat, &client_name);

   return cc;
}


#ifdef DCE

#ifdef HACK

/* typedef unsigned char idl_char; */
/* typedef idl_char sec_rgy_name_t[1025]; */
typedef void login_info_t;
typedef void tgt_request_data_t;
typedef void *krb5_ccache;

/* 4 bytes */
typedef signed32 krb5_int32;
/* typedef unsigned int unsigned32; */

/* 2 bytes */
#if 1
typedef unsigned16 krb5_ui_2;
#else
typedef unsigned short  krb5_ui_2;
#endif

typedef krb5_int32      krb5_error_code;
typedef krb5_error_code krb5_magic;
typedef unsigned char   krb5_octet;
#define FAR

typedef struct _krb5_pa_data {
    krb5_magic magic;
    krb5_ui_2  pa_type;
    int length;
    krb5_octet FAR *contents;
} krb5_pa_data;

typedef struct _krb_info_t {
    krb5_ccache         cache;
    krb5_pa_data        pepper;
    char                **prev_caches;
    tgt_request_data_t  *tgt_data;
} krb_info_t;

typedef enum _context_state {
    none,
    allocated,
    semisetup,
    setup,   
    valid,
    certified
} context_state_t;

/*
typedef enum {sec_login_auth_src_network=0,
    sec_login_auth_src_local=1,
    sec_login_auth_src_overridden=2
} sec_login_auth_src_t;
*/

/* typedef unsigned32 sec_login_flags_t; */

#define sec_login_no_flags (0)
#define sec_login_credentials_private (1)
#define sec_login_external_tgt (2)
#define sec_login_proxy_cred (4)
#define sec_login_machine_princ (8)
#define sec_login_inherit_pag (16)
#define sec_login_master_rgy (32)

typedef struct sec_login_context {
    void                    *magic;
    sec_rgy_name_t          cell;
    sec_rgy_name_t          principal;
    login_info_t            *identity;
    krb_info_t              krb_info;
    context_state_t         state;
    sec_login_auth_src_t    auth_src;
    sec_login_flags_t       flags;
} sec_login_context_t;


sec_login_flags_t
sec_login_get_context_flags(sec_login_handle_t *login_context,
			    error_status_t *status)
{
   if (status)
      *status = error_status_ok;
   return ((sec_login_context_t *)login_context)->flags;
}


void
sec_login_set_context_flags(sec_login_handle_t *login_context,
			    sec_login_flags_t flags,
			    error_status_t *status)
{
   if (status)
      *status = error_status_ok;
   ((sec_login_context_t *)login_context)->flags = flags;
   return;
}


#endif /* HACK */

#endif /* DCE */

#ifdef KRBGSS
#ifdef KRB5_EXPORTVAR /* this is required for later Kerberos versions */

static void
put_creds_in_ccache(char *name, gss_cred_id_t creds)
{
   OM_uint32 maj_stat, min_stat;
   krb5_context context;
   krb5_principal me;
   krb5_ccache ccache;
   krb5_error_code retval;
   char buf[1024];

   maj_stat = krb5_init_context(&context);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_PRINTERROR_GETTINGKRB5CONTEXT, maj_stat,
                            GSS_S_COMPLETE);
      return;
   }

   /* Set up ccache */
   if ((retval = krb5_parse_name(context, name, &me))) {
      snprintf(buf, sizeof buf, MSG_GSS_PRINTERROR_KRB5PARSENAMERETURNEDX_I, retval);
      gsslib_print_error(buf);
      return;
   }

   /* Use default ccache */
   if ((retval = krb5_cc_default(context, &ccache))) {
      snprintf(buf, sizeof buf, MSG_GSS_PRINTERROR_KRB5CCDEFAULTRETURNEDX_I, retval);
      gsslib_print_error(buf);
      return;
   }

   if ((retval = krb5_cc_initialize(context, ccache, me))) {
      snprintf(buf, sizeof buf, MSG_GSS_PRINTERROR_KRB5CCINITIALIZERETURNEDX_I, retval);
      gsslib_print_error(buf);
      return;
   }

   /* Copy GSS creds into ccache */
   maj_stat = gss_krb5_copy_ccache(&min_stat, creds, ccache);
   if (maj_stat != GSS_S_COMPLETE) {
      gsslib_display_status(MSG_GSS_PRINTERROR_COPYINGDELEGATEDCREDSTOCC,
                            maj_stat, min_stat);
      goto cleanup;
   }

   return;

cleanup:
   krb5_cc_destroy(context, ccache);
}
#endif
#endif
