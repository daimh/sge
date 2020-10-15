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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_hostname.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_prog.h"

#include "sgeobj/sge_all_listsL.h"

#include "comm/commlib.h"
#include "gdi/sge_gdi2.h"

#include "sge.h"

/* #include "basis_types.h" */

#include "msg_krb.h"

#define KRB5_DEPRECATED 1
#include "sge_krb_KRB_L.h"
#include "krb5.h"				/* Kerberos stuff 	*/
#include "com_err.h"

#include "krb_data.h"
#include "krb_lib.h"


#if 1
krb5_error_code KRB5_CALLCONV
krb5_cc_register(krb5_context context, krb5_cc_ops FAR *ops, krb5_boolean override);
#endif

#define send_message krb_send_message

/*

   NOTES

   We may need to tell the difference between a client getting a
   message back that was intended for a previous client and the client
   actually getting back a failed authentication message.  This can be
   a problem because if a client goes down and another comes up, the new
   client may have the same commd address as the old client.  This was handled
   in the existing SGE security code by using a connection ID which
   was assigned by the qmaster.  The client compared the connection ID
   received from the qmaster in the message to his own connection ID.
   If the connection ID is different, he knew this message was for someone
   else.  We could possibly handle this by using a unique identifier
   obtained from the KDC.  Perhaps the session key encrypted in the server's
   key would suffice.

   Do we ever need to reconnect automatically as a client?

   May want to do double (reverse) lookup on hosts during initialization.

*/
#define MIT_KRB 1
#ifdef MIT_KRB
extern krb5_cc_ops krb5_mcc_ops;
#endif

static krb_global_data_t gsd;			/* global security data */

#define SGE_SERVICE "sge"
#define KRB_KEYTAB "keytab"
#define KRB_DEFAULT_IDLE_CLIENT_INTERVAL (15*60)

#define KRB_USE_SAFE_MESSAGES
#ifdef KRB_USE_SAFE_MESSAGES
#  define KRB_ENCRYPT krb5_mk_safe 
#  define KRB_DECRYPT krb5_rd_safe 
#else
#  define KRB_ENCRYPT krb5_mk_priv
#  define KRB_DECRYPT krb5_rd_priv
#endif


static char *ptr2str(void *ptr, char *str);
static void *str2ptr(const char *str);
static int krb_send_auth_failure(char *tocommproc, int toid, char *tohost);
static krb5_error_code krb_get_new_auth_con(krb5_auth_context *auth, krb5_rcache rcache);
static krb5_error_code krb_get_tkt_for_daemon(const char *qmaster_host);
/* static int krb_set_idle_client_interval(int interval);  */
static int krb_delete_client(lListElem *client);

#if 0
static int krb_set_idle_client_interval(int interval) 
{
    gsd.idle_client_interval = interval;
    return 0;
}
#endif

static krb5_error_code krb_get_tkt_for_daemon(const char *qmaster_host) 
{
   krb5_error_code rc;
   char ccname[256];

   DENTER(TOP_LAYER, "krb_get_tkt_for_daemon");

   sprintf(ccname, "FILE:/tmp/krb5cc_%s", gsd.progname);

   krb5_free_cred_contents(gsd.context, &gsd.creds);
   memset(&gsd.creds, 0, sizeof(gsd.creds));

   if ((rc = krb5_cc_resolve(gsd.context, ccname, &gsd.ccdef)))
      goto error;

   if ((rc = krb5_sname_to_principal(gsd.context, qmaster_host, SGE_SERVICE,
				     KRB5_NT_SRV_HST, &gsd.creds.server)))
      goto error;

   if ((rc = krb5_sname_to_principal(gsd.context, gsd.hostname, gsd.progname,
				     KRB5_NT_SRV_HST, &gsd.creds.client)))
      goto error;

   if ((rc = krb5_cc_initialize(gsd.context, gsd.ccdef, gsd.creds.client)))
      goto error;

   /* get SGE ticket using keytab */

   if ((rc = krb5_get_in_tkt_with_keytab(gsd.context, 0, 0, NULL,
					 NULL, gsd.keytab, gsd.ccdef,
					 &gsd.creds, 0)))
      goto error;

   gsd.tgt_acquired = 1;

 error:

   DEXIT;
   return rc;
}


int krb_init(const char *progname) 
{
   int rc;
   char keytab[256];
   struct hostent *he;
   u_long32 prog_number = uti_state_get_mewho();
   

   DENTER(TOP_LAYER, "krb_init");

   if (gsd.initialized) {
      DEXIT;
      return 0;
   }

   /* initialize kerberos context */
   if ((rc = krb5_init_context(&gsd.context))) {
       ERROR((SGE_EVENT, MSG_KRB_KRB5INITCONTEXTFAILEDX_S , error_message(rc)));
       DEXIT;
       exit(1);
   }

   /* register memory-based credentials cache */
   if ((rc = krb5_cc_register(gsd.context, &krb5_mcc_ops, 0))) {
       ERROR((SGE_EVENT, MSG_KRB_KRB5CCREGISTERFAILEDX_S , error_message(rc)));
       DEXIT;
       exit(1);
   }

   strcpy(gsd.progname, progname);

   /* check to see if I am a daemon */
   if(!strcmp(prognames[QMASTER], progname) ||
      !strcmp(prognames[EXECD], progname) ||
      !strcmp(prognames[SCHEDD], progname))
      gsd.daemon = 1;
   else
      gsd.daemon = 0;

   gsd.conn_list = NULL;

#ifdef notdef

   /* Build the SGE kerberos service name. The format of the service
      name is service/host@realm, where service is the name of the
      kerberized service (sge), host is the where the service resides
      (qmaster host), and realm is the kerberos realm of the service
      (qmaster host realm).  If the qmaster host name is not available
      we will build a name based on the local realm (i.e. sge/realm@realm) */

   if ((krbhost = sge_get_master(getenv("SGE_CELL"), 0))) {

      char **realmlist=NULL;

      /* get kerberos realm of master host */
      if ((rc = krb5_get_host_realm(gsd.context, krbhost, &realmlist))) {

	 ERROR((SGE_EVENT, MSG_KRB_COULDNOTGETREALMFORQMASTERHOSTXY_SS ,
		krbhost, error_message(rc)));

      } else {

	 if (realmlist && realmlist[0])
	    krbrealm = sge_strdup(NULL, realmlist[0]);

	 if (realmlist)
	    if ((rc = krb5_free_host_realm(gsd.context, realmlist)))
	       ERROR((SGE_EVENT, MSG_KRB_COULDNOTFREEREALMLISTX_S , 
		     error_message(rc)));
      }

   }


   /* if we don't have a host or realm, get the default realm */

   if (!krbhost || !krbrealm) {
      if ((rc = krb5_get_default_realm(gsd.context, &krbrealm))) {
	 ERROR((SGE_EVENT, MSG_KRB_COULDNOTGETDEFAULTREALMX_S , 
	       error_message(rc)));
      } else {
	 krbhost = krbrealm;
      }
   }

   if (!krbhost || !krbrealm) {
      ERROR((SGE_EVENT, MSG_KRB_COULDNOTDETERMINEHOSTSORREALFORQMASTER ));
      DEXIT;
      exit(1);
   }

   sprintf(gsd.service, "%s/%s@%s", SGE_SERVICE, krbhost, krbrealm);

   if ((rc = krb5_parse_name(gsd.context, gsd.service, &gsd.clientp))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5PARSENAMEFAILEDX_S , error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_KRB5PARSENAMEFAILED );
      DEXIT;
      exit(1);
   }

#endif

   gsd.idle_client_interval = KRB_DEFAULT_IDLE_CLIENT_INTERVAL;

   if (gethostname(gsd.hostname, sizeof(gsd.hostname)) < 0) {
      ERROR((SGE_EVENT, MSG_KRB_GETHOSTNAMEFAILED));
      DEXIT;
      exit(1);
   }

   if ((he = gethostbyname(gsd.hostname)) == NULL) {
      ERROR((SGE_EVENT, MSG_KRB_GETHOSTNAMEFAILED));
      DEXIT;
      exit(1);
   }

   /* get local host address */
   memcpy((struct in_addr *)&gsd.hostaddr, he->h_addr, sizeof(gsd.hostaddr));

   /* if we are a SGE daemon, locate keytab file */

   if (gsd.daemon) {

       /* set defaults for renewing TGTs */

       gsd.tgt_renew_threshold = 60*60; /* renew TGTs if they expire in 1 hr */
       gsd.tgt_renew_interval = 15*60;  /* try to renew TGTs every 15 minutes */

      /* use $SGE_ROOT/keytab */

      sprintf(keytab, "FILE:%s/%s", sge_get_root_dir(0, NULL, 0, 1), KRB_KEYTAB);
      if ((rc = krb5_kt_resolve(gsd.context, keytab, &gsd.keytab))) {

	 ERROR((SGE_EVENT, MSG_KRB_COULDNOTRESOLVEKEYTABX_S, error_message(rc)));
	 DEXIT;
	 exit(1);
      }

      if ((!strcmp(prognames[QMASTER], progname))) {

	 if ((rc = krb5_sname_to_principal(gsd.context, NULL, SGE_SERVICE,
					   KRB5_NT_SRV_HST, &gsd.serverp))) {
	    ERROR((SGE_EVENT, MSG_KRB_KRB5SNAMETOPRINCIPALFAILEDX_S ,
		   error_message(rc)));
	    com_err(gsd.progname, rc, MSG_KRB_KRB5SNAMETOPRINCIPAL );
	    DEXIT;
	    exit(1);
	 }

      } else {

	 if ((rc = krb5_sname_to_principal(gsd.context, gsd.hostname,
					   gsd.progname, KRB5_NT_SRV_HST, &gsd.serverp))) {
	    ERROR((SGE_EVENT, MSG_KRB_KRB5SNAMETOPRINCIPALFAILEDX_S, error_message(rc)));
	    com_err(gsd.progname, rc, MSG_KRB_KRB5SNAMETOPRINCIPAL );
	    DEXIT;
	    exit(1);
	 }

      }

      /* Read private key from keytab, if we can't read $SGE_ROOT/keytab,
	 then try the default keytab */

      if (((rc = krb5_kt_read_service_key(gsd.context, keytab,
                                          gsd.serverp, 0,
                                          ENCTYPE_DES_CBC_CRC,
                                          &gsd.daemon_key))) &&
	  ((rc != KRB5_KT_NOTFOUND && rc != ENOENT) ||
	   (((rc = krb5_kt_default_name(gsd.context, keytab,
				       sizeof(keytab)-1))) ||
            ((rc = krb5_kt_resolve(gsd.context, keytab, &gsd.keytab))) ||
            ((rc = krb5_kt_read_service_key(gsd.context, keytab,
                                          gsd.serverp, 0,
                                          ENCTYPE_DES_CBC_CRC,
                                          &gsd.daemon_key)))))) {

	 ERROR((SGE_EVENT, MSG_KRB_XCOULDNOTGETDAEMONKEYY_SS , gsd.progname, error_message(rc)));
	 exit(1);
      }

   }

   /* do qmaster initialization */
   if (gsd.daemon && (!strcmp(prognames[QMASTER], progname) &&
       prog_number == QMASTER)) {

      /* NOTE: make sure we are REALLY the qmaster */
      gsd.qmaster = 1;

      /* initialize the connection list */
      gsd.conn_list = lCreateList("krb_connections", KRB_Type);

      if (gsd.conn_list == NULL) {
	 ERROR(( SGE_EVENT, MSB_KRB_CONNECTIONLISTCOULDNOTBECREATED ));
         DEXIT;
	 exit(1);
      }

#ifdef notdef
      if ((rc = krb_get_tkt_for_daemon(gsd.hostname))) {
	 ERROR((SGE_EVENT, MSG_KRB_XCOULDNOTGETSGETICKETUSINGKEYTABY_SS ,
		gsd.progname, error_message(rc)));
	 exit(1);
      }
#endif

   } else {
 
      /* initialize client auth_context */
      if ((rc = krb_get_new_auth_con(&gsd.auth_context, NULL))) {
	 ERROR((SGE_EVENT, MSG_KRB_KRBGETNEWAUTHCONFAILUREX_S ,
		error_message(rc)));
	 com_err(gsd.progname, rc, MSG_KRB_KRBGETNEWAUTHCONFAILED );
         DEXIT;
         exit(1);
      }

      if (gsd.daemon) {

         /* initialize the connection list in execd */
         if (!strcmp(prognames[EXECD], progname)) {
            gsd.conn_list = lCreateList("krb_connections", KRB_Type);
            if (gsd.conn_list == NULL) {
               ERROR((SGE_EVENT, MSB_KRB_CONNECTIONLISTCOULDNOTBECREATED));
               DEXIT;
               exit(1);
            }
            gsd.client = lCreateElem(KRB_Type);
         }

         /* get TGT from keytab */

	 /* get service ticket for SGE */

      } else { /* regular client */

	 /* get credentials cache name */

	 if ((rc = krb5_cc_default(gsd.context, &gsd.ccdef))) {
	    ERROR((SGE_EVENT, MSG_KRB_KRB5CCDEFAULTFAILEDX_S ,
		   error_message(rc)));
	    com_err(gsd.progname, rc, MSG_KRB_COULDNOTGETCLIENTCREDENTIALS);
	    DEXIT;
	    exit(1);
	 }

         if ((rc = krb5_cc_get_principal(gsd.context, gsd.ccdef,
                                         &gsd.clientp))) {
            ERROR((SGE_EVENT, MSG_KRB_KRB5CCGETPRINCIPALFAILEDX_S ,
                   error_message(rc)));
	    com_err(gsd.progname, rc, MSG_KRB_COULDNOTGETCLIENTPRINCIPAL );
	    DEXIT;
	    exit(1);
         }

      }

   }

   gsd.initialized = 1;


   DPRINTF(("====================[  KRB SECURITY  ]===========================\n"));

#ifdef notdef
   if (krbrealm) free(krbrealm);
#endif

   DEXIT;
   return 0;
}


static int krb_delete_client(lListElem *client)
{
   krb5_auth_context auth;
   lListElem *tgt_ep;
   krb5_creds ** creds;

   lDechainElem(gsd.conn_list, client);
   auth = (krb5_auth_context)str2ptr(lGetString(client,
                                                KRB_auth_context));
   if (auth) {
      krb5_rcache rcache=NULL;
      if (!krb5_auth_con_getrcache(gsd.context, auth, &rcache))
         if (rcache)
            krb5_auth_con_setrcache(gsd.context, auth, NULL);
      krb5_auth_con_free(gsd.context, auth);
   }

   /* free the TGTs */

   for_each(tgt_ep, lGetList(client, KRB_tgt_list))
      if ((creds = (krb5_creds **)str2ptr(lGetString(tgt_ep, KTGT_tgt))))
         krb5_free_tgt_creds(gsd.context, creds);

   lFreeElem(&client);
   return 0;
}

int krb_check_for_idle_clients(void)
{
   static u_long32 next_time = 0;
   u_long32 now = sge_get_gmt();
   lListElem *client, *next;

   DENTER(TOP_LAYER, "krb_check_for_idle_clients");

   if ((now = sge_get_gmt())<next_time) {
      DEXIT;
      return 0;
   }

   for (client=lFirst(gsd.conn_list); client; client=next) {
      next = lNext(client);

      /* if client connection has timed out, remove client entry */

      if (now > (lGetUlong(client, KRB_timestamp) + KRB_CLIENT_TIMEOUT)) {

         krb_delete_client(client);

      }
   }

   next_time = now + gsd.idle_client_interval;

   DEXIT;
   return 0;
}


static krb5_error_code krb_get_new_auth_con(krb5_auth_context *auth, 
                                            krb5_rcache rcache) 
{
    krb5_error_code rc=0;
    krb5_auth_context new_auth=NULL, old_auth=NULL;
    krb5_int32 flags;

   /* init new auth context */

   if ((rc = krb5_auth_con_init(gsd.context, &new_auth)))
      goto error;

   if ((rc = krb5_auth_con_setrcache(gsd.context, new_auth, rcache)))
      goto error;

#ifndef KRB_DO_REPLAY_STUFF

   /* turn off replay in auth_context */

   if (((rc = krb5_auth_con_getflags(gsd.context, new_auth, &flags))) ||
       ((rc = krb5_auth_con_setflags(gsd.context, new_auth, flags &
           ~KRB5_AUTH_CONTEXT_DO_TIME))))
      goto error;

#endif /* KRB_DO_REPLAY_STUFF */

   /* replace old auth_context with new and free the old */

   old_auth = *auth;
   *auth = new_auth;

   if (old_auth) {
      krb5_rcache old_rcache=NULL;
      if (!krb5_auth_con_getrcache(gsd.context, old_auth, &old_rcache))
         if (old_rcache && rcache && old_rcache == rcache)
            krb5_auth_con_setrcache(gsd.context, old_auth, NULL);
      if ((rc = krb5_auth_con_free(gsd.context, old_auth))) {
	 goto error;
      }
   }

 error:
   return rc;
}


static krb5_error_code krb_get_forwardable_tgt(char *host, 
                                               krb5_auth_context auth,
                                               krb5_creds **in_creds,
                                               krb5_data *tgtbuf) 
{
   krb5_error_code rc = 0;
   char ccname[40];
   krb5_ccache          ccache = NULL;

   DENTER(TOP_LAYER, "krb_get_forwardable_tgt");

   /* create an in-memory credentials cache */

   strcpy(ccname, "MEMORY:xxxx");

   if ((rc = krb5_cc_resolve(gsd.context, ccname, &ccache)))
      goto error;

   if ((rc = krb5_cc_initialize(gsd.context, ccache, (*in_creds)->client)))
      goto error;

   if ((rc = krb5_cc_store_cred(gsd.context, ccache, *in_creds)))
      goto error;

   if ((rc = krb5_fwd_tgt_creds(gsd.context, auth, host,
                                (*in_creds)->client, NULL,
                                ccache, 1,
                                tgtbuf)))
      goto error;

 error:

   if (ccache)
      krb5_cc_destroy(gsd.context, ccache);

   DEXIT;

   return rc;
}
#if 0
int
krb_send_message(int synchron, const char *tocomproc, int toid, 
                 const char *tohost, int tag, char *buffer, 
                 int buflen, u_long32 *mid) 
{
   krb5_error_code rc;
   krb5_timestamp time_now;
   int ret = SEC_SEND_FAILED;
   krb5_auth_context auth;
   lListElem  *client=NULL, *tgt_ep;
   lCondition *where=NULL;
   krb5_data ap_req, inbuf, outbuf, tgtbuf;
   krb5_address addr;
   sge_pack_buffer pb;
   krb5_principal server = NULL;
   int I_am_a_client = 0;
   krb5_creds **tgt_creds;
#ifdef KRB_DO_REPLAY_STUFF
   krb5_address *portlocal_addr;
   krb5_rcache rcache;
   char *cp;
#endif /* KRB_DO_REPLAY_STUFF */
   const char *progname = uti_state_get_sge_formal_prog_name();

   DENTER(TOP_LAYER, "krb_send_message");

   /* prepare packing buffer */
   if ((ret = init_packbuffer(&pb, 4096, 0))) {
	  ERROR((SGE_EVENT, MSG_KRB_INITPACKBUFFERFAILED_S, cull_pack_strerror(ret)));
	  goto error;
   }

   if (!gsd.initialized) {
      ERROR((SGE_EVENT,MSG_KRB_CALLERDIDNOTCALLKRBINIT ));
      goto error;
   }

   if ((rc = krb5_timeofday(gsd.context, &time_now))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5TIMEOFDAYFAILEDX_S,
	     error_message(rc)));
      goto error;
   }

   ap_req.length = 0;
   inbuf.length = 0;
   outbuf.length = 0;
   tgtbuf.length = 0;

   /* if we are acting as a server, look up auth_context for the
      client using the commd tuple */

   if (gsd.qmaster || (gsd.daemon &&
		       strcmp(prognames[QMASTER], tocomproc))) {

      if (toid)
	 where = lWhere("%T(%I==%s && %I==%s && %I==%u)", KRB_Type, KRB_host, 
         tohost, KRB_commproc, tocomproc,
			KRB_id, toid);
      else
	 where = lWhere("%T(%I==%s && %I==%s)", KRB_Type, KRB_host, 
                   tohost, KRB_commproc, tocomproc);
      client = lFindFirst(gsd.conn_list, where);
      if (!client || !where) {
	 ERROR((SGE_EVENT, MSG_KRB_NOCLIENTENTRYFOR_SSI ,
                tohost, tocomproc, toid));
	 goto error;
      }

      /* set activity time stamp in client */
      lSetUlong(client, KRB_timestamp, sge_get_gmt());

      auth = (krb5_auth_context)str2ptr(lGetString(client, KRB_auth_context));

   } else {
      const char *service;

      /* we are a client, we need to get a fresh auth_context */

      I_am_a_client = 1;

      if ((rc = krb_get_new_auth_con(&gsd.auth_context, NULL))) {
	 ERROR((SGE_EVENT, MSG_KRB_KRBGETNEWAUTHCONFAILUREX_S,
		error_message(rc)));
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTGETNEWAUTHCONTEXT);
	 goto error;
      }

      auth = gsd.auth_context;

      /* if we are a SGE daemon sending a message to the qmaster
	 and we don't have a valid SGE ticket, go get one.
	 Not having a valid ticket is due to either 1) we haven't got
	 one yet, 2) the ticket is about to expire, or 3) the qmaster
	 has moved to different host (i.e. the shadow) */
 
      if (gsd.daemon && !strcmp(prognames[QMASTER], tocomproc) &&
	  (!gsd.tgt_acquired || 
          gsd.creds.times.endtime < (time_now + gsd.tgt_renew_threshold) ||
	  (gsd.qmaster_host[0] && sge_hostcmp(tohost, gsd.qmaster_host)))) {

	 strcpy(gsd.qmaster_host, tohost);

	 if ((rc = krb_get_tkt_for_daemon(tohost))) {
	    ERROR((SGE_EVENT, MSG_KRB_XCOULDNOTGETSGETICKETUSINGKEYTABY_SS,
		   gsd.progname, error_message(rc)));
	    exit(1);
	 }
      }

      /* build AP_REQ */

      inbuf.data = (char *) tohost;
      inbuf.length = strlen(tohost);

      /* Use SGE service for msgs to qmaster,
         daemon name for msgs to other daemons */

      if (!strcmp(prognames[QMASTER], tocomproc))
         service = SGE_SERVICE;
      else
         service = tocomproc;
      
      if ((rc = krb5_mk_req(gsd.context, &auth, 0, 
             (char *)service, (char *) tohost,
			    &inbuf, gsd.ccdef, &ap_req))) {
	 ERROR((SGE_EVENT, MSG_KRB_FAILEDCREATINGAP_REQFORWXZY_SSIS,
		tohost, tocomproc, toid, error_message(rc)));
	 if (!gsd.daemon)
	    com_err(gsd.progname, rc, MSG_KRB_COULDNOTCREATEAUTHENTICATIONINFO);
	 goto error;
      }

   }

#ifdef KRB_PORT_STUFF

   addr.addrtype = ADDRTYPE_IPPORT;
   addr.length = sizeof(c_sock.sin_port);
   addr.contents = (krb5_octet *)&c_sock.sin_port;
   if ((rc = krb5_auth_con_setports(gsd.context, auth,
				    &addr, NULL))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5AUTHCONSETADDRSFIALEDFORWXYZ_SSIS,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTSETPORTSINAUTHCONTEXT);
      goto error;
   }

#endif

   addr.addrtype = ADDRTYPE_INET;
   addr.length = sizeof(gsd.hostaddr);
   addr.contents = (krb5_octet *)&gsd.hostaddr;
   if ((rc = krb5_auth_con_setaddrs(gsd.context, auth, &addr, NULL))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5AUTHCONSETADDRSFIALEDFORWXYZ_SSIS,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTSETADDRESSESINAUTHCONTEXT);
      goto error;
   }

#ifdef KRB_DO_REPLAY_STUFF

   if ((rc = krb5_gen_portaddr(gsd.context, &addr,
				 (krb5_pointer) &c_sock.sin_port,
				 &portlocal_addr))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5GENPORTADDRFAILEDFORWXYZ_SSIS,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTGENPORTADDR );
      goto error;
   }

   if ((rc = krb5_gen_replay_name(gsd.context, portlocal_addr,
				      progname ? progname : "sge_client", &cp))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5GENREPLAYNAMEFAILEDFORWXYZ_SSIS ,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTGENREPLAYNAME );
      goto error;
   }

   rcache_name.length = strlen(cp);
   rcache_name.data = cp;

   if ((rc = krb5_get_server_rcache(gsd.context, &rcache_name, &rcache))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5GETSERVERRCACHEFAILEDFORWXYZ_SSIS ,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_COULDNOTGETREPLAYCACHE);
      goto error;
   }

   /* free existing auth_context rcache */
   krb5_auth_con_getrache(gsd.context, auth, &old_rcache);
   if (old_rcache)
      krb5_rc_close(gsd.context, old_rcache);

   /* set auth_context rcache */
   krb5_auth_con_setrcache(gsd.context, auth, rcache);

#endif /* KRB_DO_REPLAY_STUFF */

   /* encrypt message */

   inbuf.data = buffer;
   inbuf.length = buflen;

   if ((rc = KRB_ENCRYPT(gsd.context, auth, &inbuf, &outbuf, NULL))) {
      ERROR((SGE_EVENT, MSG_KRB_FAILEDENCRYPTINGMSGFORWXYZ_SSIS ,
	     tohost, tocomproc, toid, error_message(rc)));
      if (!gsd.daemon)
	 com_err(gsd.progname, rc, MSG_KRB_FAILEDENCRYPTINGMESSAGE );
      goto error;
   }

   /* get forwardable TGT for client */

   if (I_am_a_client && gsd.client_flags & KRB_FORWARD_TGT) {

      if ((rc = krb5_sname_to_principal(gsd.context, NULL, SGE_SERVICE,
                                        KRB5_NT_SRV_HST, &server))) {
         ERROR((SGE_EVENT, MSG_KRB_KRB5SNAMETOPRINCIPALFAILEDX_S,
                error_message(rc)));
         goto error;
      }

      if ((rc = krb5_fwd_tgt_creds(gsd.context, auth, (char*) tohost,
                                   gsd.clientp, server,
                                   gsd.ccdef, 1,
                                   &tgtbuf))) {
         ERROR((SGE_EVENT, MSG_KRB_COULDNOTGETFORWARDABLETGTFORWXYZ_SSIS , tohost, tocomproc, toid, error_message(rc)));
         if (!gsd.daemon)
            com_err(gsd.progname, rc, MSG_KRB_UNABLETOFORWARDTGT );
      }

   } else if (gsd.qmaster && 
              !strcmp(prognames[EXECD], tocomproc) &&
	      gsd.tgt_id && client &&
              ((tgt_ep = lGetElemUlong(lGetList(client, KRB_tgt_list), KTGT_id, gsd.tgt_id))) &&
	      ((tgt_creds = (krb5_creds **)
		    str2ptr(lGetString(tgt_ep, KTGT_tgt))))) {

      /* get forwardable TGT for client for the execution host */

      if ((rc = krb_get_forwardable_tgt((char*) tohost, auth, tgt_creds,
                                        &tgtbuf))) {
         ERROR((SGE_EVENT, MSG_KRB_COULDNOTGETFORWARDABLETGTFORWXYZ_SSIS ,
                tohost, tocomproc, toid, error_message(rc)));
      }

   }

   /* pack AP_REQ, encrypted msg, and TGT together into a single message  */

   packint(&pb, ap_req.length);
   packint(&pb, outbuf.length);
   packint(&pb, tgtbuf.length);
#if 1 /* 
       * TODO EB: following code should be removed for the next version
       *          where it is possible to change the packbuffer format
       */
   /* unused int in pb; was previously used to store if data is compressed */
   packint(&pb, -1);
#endif
   if (ap_req.length)
      packbuf(&pb, ap_req.data, ap_req.length);
   if (outbuf.length)
      packbuf(&pb, outbuf.data, outbuf.length);
   if (tgtbuf.length) {
      packint(&pb, gsd.tgt_id);
      packbuf(&pb, tgtbuf.data, tgtbuf.length);
   }

   ret = send_message(synchron, tocomproc, toid, tohost, tag, pb.head_ptr, pb.bytes_used, mid);
 error:

   if (server)
      krb5_free_principal(gsd.context, server);

   if (where)
      lFreeWhere(&where);

   if (ap_req.length)
      krb5_xfree(ap_req.data);

   if (outbuf.length)
      krb5_xfree(outbuf.data);

   if (tgtbuf.length)
      krb5_xfree(tgtbuf.data);

   clear_packbuffer(&pb);

   DEXIT;
   return ret;
}
#endif

static int 
krb_unpackmsg(sge_pack_buffer *pb, krb5_data *buf1, krb5_data *buf2, 
              krb5_data *buf3, u_long32 *tgt_id);

static int
krb_unpackmsg(sge_pack_buffer *pb, krb5_data *buf1, krb5_data *buf2, 
              krb5_data *buf3, u_long32 *tgt_id) 
{
   int ret;
   u_long32 len1=0, len2=0, len3=0, cpr=0;
   *tgt_id = 0;

   if ((ret=unpackint(pb, &len1))) goto error;
   if ((ret=unpackint(pb, &len2))) goto error;
   if ((ret=unpackint(pb, &len3))) goto error;
#if 1 /* 
       * TODO EB: following code should be removed for the next version
       *          where it is possible to change the packbuffer format
       */
   if ((ret=unpackint(pb, &cpr))) goto error;
#endif
   if ((buf1->length = len1)) {
      if ((ret=unpackbuf(pb, &buf1->data, buf1->length))) goto error;
   } else
      buf1->data = NULL;
   if ((buf2->length = len2)) {
      if ((ret=unpackbuf(pb, &buf2->data, buf2->length))) goto error;
   } else
      buf2->data = NULL;
   if ((buf3->length = len3)) {
      if ((ret=unpackint(pb, tgt_id))) goto error;
      if ((ret=unpackbuf(pb, &buf3->data, buf3->length))) goto error;
   } else
      buf3->data = NULL;
   ret = PACK_SUCCESS;

 error:
   return ret;
}

static int
krb_send_auth_failure(
char *tocommproc,
int toid,
char *tohost 
) {
   int tag = TAG_AUTH_FAILURE;
   char buffer[BUFSIZ];
   int buflen;
   sge_strlcpy(buffer, MSG_KRB_AUTHENTICATIONFAILURE, BUFSIZ);
   buflen = strlen(buffer);
   return send_message(0, tocommproc, toid, tohost, tag, buffer, buflen, NULL);
}

#if 0
int
krb_receive_message(char *fromcommproc, u_short *fromid, char *fromhost, 
                    int *tag, char **buffer, u_long32 *buflen, int synchron) 
{
   krb5_error_code rc;
   int ret = SEC_RECEIVE_FAILED;
   krb5_auth_context auth=NULL;
   krb5_creds ** tgt_creds = NULL;
   lListElem  *client=NULL;
   lCondition *where=NULL;
   krb5_data inbuf, outbuf, ap_req, tgtbuf;
   krb5_address addr;
   sge_pack_buffer pb;
   struct hostent *he;
   struct in_addr hostaddr;
   u_long32 tmplen=0;
   char tmpcommproc[MAXCOMPONENTLEN];
   char tmphost[CL_MAXHOSTLEN];
   u_short tmpid=0;
   int tmptag=0;
   u_long32 tgt_id=0;

   DENTER(TOP_LAYER,"krb_receive_message");

   ap_req.length = 0;
   inbuf.length = 0;
   outbuf.length = 0;
   tgtbuf.length = 0;

   if (!gsd.initialized) {
      ERROR((SGE_EVENT,MSG_KRB_DIDNOTCALLKRBINIT ));
      ret = SEC_RECEIVE_FAILED;
      goto error;
   }

   /* receive message from commd */

   if (fromcommproc)
      strcpy(tmpcommproc, fromcommproc);
   else
      tmpcommproc[0] = 0;
   if (fromid) tmpid = *fromid;
   if (fromhost)
       strcpy(tmphost, fromhost);
   else
       tmphost[0] = 0;
   if (tag) tmptag = *tag;

   ret = receive_message(tmpcommproc, &tmpid, tmphost, &tmptag,
			                buffer, &tmplen, synchron);

   if (buflen) *buflen = tmplen;
   if (fromcommproc) strcpy(fromcommproc, tmpcommproc);
   if (fromhost) strcpy(fromhost, tmphost);
   if (fromid) *fromid = tmpid;
   if (tag) *tag = tmptag;

   if (ret) goto error;

   if (tmptag == TAG_AUTH_FAILURE) {
      if (gsd.qmaster) {
	 ERROR((SGE_EVENT, MSG_KRB_INVALIDTAGAUTHFAILUREMSGRECEIVEDWXYZ_SSI , 
            tmphost, tmpcommproc, (int) tmpid));
	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      } else if (gsd.daemon) {
	 ERROR((SGE_EVENT, MSG_KRB_AUTHENTICATIONTOQMASTERFAILED));
	 gsd.reconnect = 1;
	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      } else {
	 ERROR((SGE_EVENT, MSG_KRB_AUTHENTICATIONTOQMASTERFAILED));
	 com_err(gsd.progname, rc, MSG_KRB_AUTHENTICATIONFAILED );
	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      }
   }

   /* unpack AP_REQ and encrypted message */

   {
      int pack_ret = init_packbuffer_from_buffer(&pb, *buffer, tmplen);
      if(pack_ret != PACK_SUCCESS) {
         ERROR((SGE_EVENT, MSG_KRB_INITPACKBUFFERFAILED_S, cull_pack_strerror(pack_ret)));
         ret = SEC_RECEIVE_FAILED;
         goto error;
      }
   }   

   if (krb_unpackmsg(&pb, &ap_req, &inbuf, &tgtbuf, &tgt_id)) {

      ERROR((SGE_EVENT, MSG_KRB_INVALIDMESSAGEUNPACKFAILURE ));

      if (gsd.qmaster) {

	 if ((rc = krb_send_auth_failure(tmpcommproc, tmpid, tmphost))) {
	    ERROR((SGE_EVENT, MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGEX_S,
		   error_message(rc)));
	 }

      } else {
	 com_err(gsd.progname, rc, MSG_KRB_INVALIDMESSAGEPACKINGERROR );
	 gsd.reconnect = 1;
      }

      ret = SEC_RECEIVE_FAILED;
      goto error;
   }

   /* if we are the qmaster (or a SGE daemon acting as a server) */
   if (gsd.qmaster || (gsd.daemon &&
		       strcmp(prognames[QMASTER], tmpcommproc))) {

      /* make sure we have an AP_REQ */
      if (ap_req.length == 0) {

	 ERROR((SGE_EVENT, MSG_KRB_INVALIDMESSAGENOAP_REQ ));
	 if ((rc = krb_send_auth_failure(tmpcommproc, tmpid, tmphost))) {
	    ERROR((SGE_EVENT, MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGEX_S ,
		   error_message(rc)));
	 }
	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      }

      /* lookup client in the connection list */
      where = lWhere( "%T(%I==%s && %I==%s && %I==%u)", KRB_Type, KRB_host,
                      tmphost, KRB_commproc, tmpcommproc,
                      KRB_id, tmpid);
      client = lFindFirst(gsd.conn_list, where);

      /* Set up daemon replay cache */

      if (gsd.rcache == NULL) {
         if ((rc = krb5_get_server_rcache(gsd.context,         
               krb5_princ_component(gsd.context, gsd.serverp, 0),
               &gsd.rcache))) {
            ERROR((SGE_EVENT, MSG_KRB_XCOULDNOTCREATEREPLAYCACHEY_SS ,
                   gsd.progname, error_message(rc)));
            exit(1);
         }
      }

      /* if client is not in the list, add him */
      if (!client || !where) {

	 client = lCreateElem(KRB_Type);
	 if (!client) {
	    ERROR((SGE_EVENT,  MSG_KRB_FAILEDCREATEOFCLIENT ));
	    ret = SEC_RECEIVE_FAILED;
	    goto error;
	 }
	 lSetHost(client, KRB_host, tmphost);
	 lSetString(client, KRB_commproc, tmpcommproc);
	 lSetUlong(client, KRB_id, tmpid);

	 if ((rc = krb_get_new_auth_con(&auth, gsd.rcache))) {
	    ERROR((SGE_EVENT, MSG_KRB_KRBGETNEWAUTHCONFAILUREX_S ,
		   error_message(rc)));
	    ret = SEC_RECEIVE_FAILED;
	    goto error;
         } else {
	    lSetString(client, KRB_auth_context, ptr2str(auth, NULL));
	 }

	 if ((ret=lAppendElem(gsd.conn_list, client))) {
	    ERROR((SGE_EVENT, MSG_KRB_APPENDELEMFAILUREX_I , ret));
	    ret = SEC_RECEIVE_FAILED;
	    goto error;
	 }

      } else {

	 /* get a fresh auth_context for the client */

	 auth = (krb5_auth_context)str2ptr(lGetString(client,KRB_auth_context));
	 if ((rc = krb_get_new_auth_con(&auth, gsd.rcache))) {
	    ERROR((SGE_EVENT, MSG_KRB_KRBGETNEWAUTHCONFAILUREX_S ,
		   error_message(rc)));
	    ret = SEC_RECEIVE_FAILED;
	    goto error;
	 }
	 lSetString(client, KRB_auth_context, ptr2str(auth, NULL));
      }

      /* set activity time stamp in client */
      lSetUlong(client, KRB_timestamp, sge_get_gmt());

      /* get auth_context from client entry */
      auth = (krb5_auth_context)str2ptr(lGetString(client, KRB_auth_context));

      /* authenticate the client using the AP_REQ */
      if ((rc = krb5_rd_req(gsd.context, &auth, &ap_req, gsd.serverp,
			    gsd.keytab, NULL, NULL))) {
	 ERROR((SGE_EVENT, MSG_KRB_CLIENTWXYFAILEDAUTHENTICATIONZ_SSIS ,
		tmphost, tmpcommproc, (int) tmpid, error_message(rc)));
	 if ((rc = krb_send_auth_failure(tmpcommproc, tmpid, tmphost))) {
	    ERROR((SGE_EVENT, MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGEX_S ,
		   error_message(rc)));
	 }

         /* remove this client from the connection list to force reauthentication */
         krb_delete_client(client);

	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      }

   } else {

      /* make sure we do have no AP_REQ */
      if (ap_req.length) {
	 ERROR((SGE_EVENT, MSG_KRB_INVALIDMESSAGEHASANAP_REQ ));
	 if (!gsd.daemon)
	    com_err(gsd.progname, rc, MSG_KRB_INVALIDMESSAGERECEIVED );
	 ret = SEC_RECEIVE_FAILED;
	 goto error;
      }

      /* use auth context from global data */
      auth = gsd.auth_context;
   }

   /* put client's host address in the client auth_context */

   if ((he = gethostbyname(tmphost)) == NULL) {
      ERROR((SGE_EVENT, MSG_KRB_GETHOSTBYNAMEFAILED));
      ret = SEC_RECEIVE_FAILED;
      goto error;
   }

   memcpy((struct in_addr *)&hostaddr, he->h_addr, sizeof(hostaddr));

   addr.addrtype = ADDRTYPE_INET;
   addr.length = sizeof(hostaddr);
   addr.contents = (krb5_octet *)&hostaddr;
   if ((rc = krb5_auth_con_setaddrs(gsd.context, auth, NULL, &addr))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5AUTHCONSETADDRSFAILEDFORWXYZ_SSIS ,
	     tmphost, tmpcommproc, (int) tmpid, error_message(rc)));
      ret = SEC_RECEIVE_FAILED;
      goto error;
   }

   /* handle forwarded TGT */

   if (tgtbuf.length) {

      if ((rc = krb5_rd_cred(gsd.context, auth, &tgtbuf, &tgt_creds, NULL))) {
	 ERROR((SGE_EVENT, MSG_KRB_UNABLETODECRYPTFORWARDEDTGTFORCLIENTWXYZ_SSIS 
                , tmphost, tmpcommproc, (int)tmpid,
                error_message(rc)));
      } else {

         if (tgt_creds) {

            if ((gsd.qmaster && client) ||
                (gsd.daemon &&
                   !strcmp(prognames[EXECD], gsd.progname))) {
               
               lListElem *tgt_ep;
               krb5_creds ** creds;
               lListElem *tmp_client = (gsd.qmaster && client) ? client : gsd.client;

               /* if this TGT already exists in the client's list, free it */

               if ((tgt_ep = lGetElemUlong(lGetList(tmp_client, KRB_tgt_list), KTGT_id, tgt_id))) {
                  creds = (krb5_creds **)str2ptr(lGetString(tgt_ep, KTGT_tgt));
                  if (creds)
                     krb5_free_tgt_creds(gsd.context, creds);
                  lFreeElem(lDechainElem(lGetList(tmp_client, KRB_tgt_list), tgt_ep));
               }

               /* store TGT in list based on TGT ID, for later retrieval */

               if ((tgt_ep = lAddSubUlong(tmp_client, KTGT_id, tgt_id, KRB_tgt_list, KTGT_Type))) {
                  lSetString(tgt_ep, KTGT_tgt, ptr2str(tgt_creds, NULL));
                  tgt_creds = NULL;
               } else /* this shouldn't happen */
                  ERROR((SGE_EVENT, MSG_KRB_FAILEDADDINGTGTWTOCLIENTSTGTLISTFORXYZ_ISSI, 
                           (int)tgt_id, tmphost, tmpcommproc, (int)tmpid));
               
#if 0
            if (gsd.qmaster && client) {

               krb5_creds ** creds;

               /* for qmaster, save TGT in client entry */

               creds = (krb5_creds **)str2ptr(lGetString(client, KRB_tgt));
               if (creds)
                  krb5_free_tgt_creds(gsd.context, creds);
               
               lSetString(client, KRB_tgt, ptr2str(tgt_creds, NULL));
               tgt_creds = NULL;

            } else if (gsd.daemon &&
                   !strcmp(prognames[EXECD], gsd.progname)) {

                /* for sge_execd, save TGT in gsd structure */

                if (gsd.tgt_creds)
                   krb5_free_tgt_creds(gsd.context, gsd.tgt_creds);
                gsd.tgt_creds = tgt_creds;
                tgt_creds = NULL;
            }

#endif
            } else {

               ERROR((SGE_EVENT, MSG_KRB_ILLOGICALFORWARDABLETGTRECEIVEDFROMXYZ_SSI, 
                     tmphost, tmpcommproc, (int) tmpid));

            }

         }

      }
   }

   /* decrypt message using auth_context */

   if ((rc = KRB_DECRYPT(gsd.context, auth, &inbuf, &outbuf, NULL))) {
      ERROR((SGE_EVENT, MSG_KRB_FAILEDDECRYPTINGMSGFORWXYZ_SSIS ,
	     tmphost, tmpcommproc, (int) tmpid, error_message(rc)));
      if (gsd.qmaster || (gsd.daemon &&
			  strcmp(prognames[QMASTER], tmpcommproc))) {
	 if ((rc = krb_send_auth_failure(tmpcommproc, tmpid, tmphost))) {
	    ERROR((SGE_EVENT, MSG_KRB_FAILEDSENDINGAUTH_FAILUREMESSAGE_S,
		   error_message(rc)));
	 }
      }
      ret = SEC_RECEIVE_FAILED;
      goto error;
   }

   /* fill in caller's buffer and buflen */

   if (buflen) *buflen = outbuf.length;
   if (*buffer) {
      outbuf.length=0;
      free(*buffer);
      *buffer = outbuf.data;
      /* otherwise buffer gets freed again by clear_packbuffer */
      pb.head_ptr = NULL;
   }

 error:

   /* clean up */

   if (where)
      lFreeWhere(&where);

   if (ap_req.length)
      krb5_xfree(ap_req.data);

   if (inbuf.length)
      krb5_xfree(inbuf.data);

   if (outbuf.length)
      krb5_xfree(outbuf.data);

   if (tgtbuf.length)
      krb5_xfree(tgtbuf.data);

   if (tgt_creds)
      krb5_free_tgt_creds(gsd.context, tgt_creds);

#ifdef notdef
   if (ticket)
      krb5_free_ticket(gsd.context, ticket);
#endif

   clear_packbuffer(&pb);

   DEXIT;
   return ret;
}
#endif

int
krb_verify_user(
const char *host,
const char *commproc,
int id,
const char *user 
) {
   krb5_error_code rc;
   int ret=-1;
   lListElem  *client=NULL;
   lCondition *where=NULL;
   krb5_auth_context auth=NULL;
   char clientname[256];
   krb5_authenticator *authenticator=NULL;

   DENTER(TOP_LAYER,"krb_verify_user");

   /* lookup client in the connection list using commd triple */

   where = lWhere( "%T(%I==%s && %I==%s && %I==%u)", KRB_Type, KRB_host, 
                   host, KRB_commproc, commproc,
                   KRB_id, id);
   client = lFindFirst(gsd.conn_list, where);
   if (!client) goto error;

   /* get client name from the client auth_context */

   auth = (krb5_auth_context)str2ptr(lGetString(client, KRB_auth_context));
   if (!auth) goto error;

   if ((rc = krb5_auth_con_getauthenticator(gsd.context, auth,
            &authenticator))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5AUTHCONGETAUTHENTICATORFAILEDFORWXYZ_SSIS,
	     host, commproc, id, error_message(rc)));
      goto error;
   }
   
   {
      char *dat;
      int len;
      dat = krb5_princ_component(gsd.context, authenticator->client, 0)->data;
      len = krb5_princ_component(gsd.context, authenticator->client, 0)->length;
      strncpy(clientname, dat, len);
      clientname[len] = 0;
   }

   /* return comparison of passed-in user to client user */

   if (strcmp(user, clientname)==0 ||
         (strcmp(user, "root")==0 &&
         (strcmp(prognames[EXECD], clientname)==0 ||
         strcmp(prognames[SCHEDD], clientname)==0))) {
      ret = 0;
   }

error:

   if (where)
      lFreeWhere(&where);

#ifdef notdef
   if (clientname)
      free(clientname);
#endif

   if (authenticator)
      krb5_free_authenticator(gsd.context, authenticator);

   DEXIT;
   return ret;
}


/*
 *
 * The ptr2str and str2ptr routines are ugly, but I need some structures
 * in a list and
 *
 *   a) I don't want to write my own list routines, 
 *   b) I don't want to convert the entire structures into the list types
 *   c) I don't have time to add a new BLOB (Bit Level OBject) type
 *      to the cull routines
 *
 * We really need to do (c).  I've looked at what it would take and I
 * think it would not be too difficult to support this, but I don't have
 * time at the moment.
 *
 */
static char* ptr2str(void *ptr, char *str) 
{
   if (str == NULL)
      str = malloc(sizeof(void *)*2+4);
   if (str == NULL || ptr == NULL)
      return NULL;
   sprintf(str, "%p", ptr);
   return str;
}

static void* str2ptr(const char *str) 
{
   void *ptr;
   if (str == NULL)
       return NULL;
   if (sscanf(str, "%p", &ptr) != 1)
       return NULL;
   return ptr;
}


char *
krb_bin2str(
void *data,
int len,
char *str 
) {
   char *src, *dest, *end;
   char *ostr = str;
   int n;

   if (str == NULL)
      str = (char *)malloc(len*2+5);
   if (str == NULL || data == NULL)
      return NULL;

   src = data;
   dest = str;
   end = src+len;

   /* encode length as non-zero values in the first 4 bytes */
   *dest++ = (len / (255*255*255))+1;
   *dest++ = (len / (255*255))+1;
   *dest++ = (len / 255)+1;
   *dest++ = (len % 255)+1;

   /* encode the rest of the binary data as non-zero bytes in the string */
   while (src < end) {
      if (*src == 0) {
          *dest++ = 1;
          for(n=0; n<254 && src<end && *src==0; n++, src++) ;
          *dest++ = n+1;
      } else if (*src == 1) {
          *dest++ = 1;
          *dest++ = 1; /* this is not a typo */
          src++;
      } else {
         *dest++ = *src++;
      }
   }
   *dest = 0;

   if (ostr == NULL)
      str = (char *)sge_realloc(str, dest-str + 1, 1);

   return str;
}


void *
krb_str2bin(
const char *str,
void *data,
int *rlen 
) {
   char *src, *dest;
   unsigned int len=0;
   int i;

   if (str == NULL)
      return NULL;

   /* get length which is encoded as non-zero values in the first 4 bytes */
   for (i=0; i<4; i++)
      len = len * 255 + (str[i]-1);

   if (len == 0)
      return NULL;

   if (data == NULL)
      data = (char *)malloc(len);
   if (data == NULL)
      return NULL;

   src = (char*) str+4;
   dest = data;
   while (*src) {
      if (*src != 1) {
         *dest++ = *src++;
      } else {
         src++;
         if (*src == 1) {
            *dest++ = 1;
         } else {
             memset(dest, 0, (*src)-1);
             dest += (*src)-1;
         }
         src++;
      }
   }

   if (rlen)
      *rlen = len;

   return data;
}


/*
 * krb_encrypt_tgt_creds - this routine encryts the TGT credentials using
 * the qmaster private key
 */

krb5_error_code
krb_encrypt_tgt_creds(
krb5_creds **tgt_creds,
krb5_data *outbuf 
) {
   krb5_error_code      rc = 0;
   krb5_auth_context    auth_context = NULL;
   krb5_int32           flags;
   krb5_data          * scratch = NULL;
   krb5_address         addr;

   DENTER(TOP_LAYER, "krb_encrypt_tgt_creds");

   if ((rc = krb5_auth_con_init(gsd.context, &auth_context)))
      goto error;

   if ((rc = krb5_auth_con_getflags(gsd.context, auth_context, &flags)))
      goto error;

   flags &= ~( KRB5_AUTH_CONTEXT_DO_TIME |
               KRB5_AUTH_CONTEXT_RET_TIME |
               KRB5_AUTH_CONTEXT_DO_SEQUENCE |
               KRB5_AUTH_CONTEXT_RET_SEQUENCE );

   if ((rc = krb5_auth_con_setflags(gsd.context, auth_context, flags)))
      goto error;

   addr.addrtype = ADDRTYPE_INET;
   addr.length = sizeof(gsd.hostaddr);
   addr.contents = (krb5_octet *)&gsd.hostaddr;
   if ((rc = krb5_auth_con_setaddrs(gsd.context, auth_context, &addr, NULL)))
      goto error;

   if ((rc = krb5_auth_con_setuseruserkey(gsd.context, auth_context,
         gsd.daemon_key)))
      goto error;

   if ((rc = krb5_mk_ncred(gsd.context, auth_context, tgt_creds, &scratch,
         NULL)))
      goto error;

   if (rc) {
      if (scratch)
         krb5_free_data(gsd.context, scratch);
   } else {
      *outbuf = *scratch;
      krb5_xfree(scratch);
   }

error:

   if (auth_context)
      krb5_auth_con_free(gsd.context, auth_context);

   DEXIT;
   return rc;
}


/*
 * krb_decrypt_tgt_creds - this routine decryts the TGT credentials using
 * the qmaster's private key
 */

krb5_error_code
krb_decrypt_tgt_creds(
krb5_data *inbuf,
krb5_creds ***tgt_creds 
) {
   krb5_error_code      rc = 0;
   krb5_auth_context    auth_context = NULL;
   krb5_int32           flags;
   krb5_address         addr;

   DENTER(TOP_LAYER, "krb_encrypt_tgt_creds");

   if ((rc = krb5_auth_con_init(gsd.context, &auth_context)))
      goto error;

   if ((rc = krb5_auth_con_getflags(gsd.context, auth_context, &flags)))
      goto error;

   flags &= ~( KRB5_AUTH_CONTEXT_DO_TIME |
               KRB5_AUTH_CONTEXT_RET_TIME |
               KRB5_AUTH_CONTEXT_DO_SEQUENCE |
               KRB5_AUTH_CONTEXT_RET_SEQUENCE );

   if ((rc = krb5_auth_con_setflags(gsd.context, auth_context, flags)))
      goto error;

   if ((rc = krb5_auth_con_setuseruserkey(gsd.context, auth_context,
         gsd.daemon_key )))
      goto error;

   addr.addrtype = ADDRTYPE_INET;
   addr.length = sizeof(gsd.hostaddr);
   addr.contents = (krb5_octet *)&gsd.hostaddr;
   if ((rc = krb5_auth_con_setaddrs(gsd.context, auth_context, NULL, &addr)))
      goto error;

   if ((rc = krb5_rd_cred(gsd.context, auth_context, inbuf, tgt_creds,
         NULL)))
      goto error;

error:

   if (auth_context)
      krb5_auth_con_free(gsd.context, auth_context);

   DEXIT;
   return rc;
}


int
krb_get_client_flags(void)
{
   return gsd.client_flags;
}


int
krb_set_client_flags(
int flags 
) {
   gsd.client_flags = flags;
   return gsd.client_flags;
}


int
krb_clear_client_flags(
int flags 
) {
   gsd.client_flags &= ~flags;
   return gsd.client_flags;
}


/*
 * krb_get_tgt - retrieve a TGT from the client's TGT list
 */

int
krb_get_tgt(
const char *host,
const char *comproc,
int id,
u_long tgt_id,
krb5_creds ***tgt_creds 
) {
   int ret = -1;
   lCondition *where=NULL;
   lListElem  *client=NULL, *tgt_ep;
   krb5_creds **creds=NULL;

   DENTER(TOP_LAYER, "krb_get_tgt");

   if (tgt_creds == NULL || host == NULL || comproc == NULL) {
      ERROR((SGE_EVENT, MSG_KRB_TGTCREDSHOSTORCOMPROCISNULL ));
      goto all_done;
   }

#if 0
   if (host == NULL && comproc == NULL) {

      if (gsd.tgt_creds) {
         *tgt_creds = gsd.tgt_creds;
         ret = 0;
      }

      goto all_done;
   }
#endif

   if (gsd.daemon && !strcmp(prognames[EXECD], gsd.progname))
      client = gsd.client;
   else {

      if (id)
         where = lWhere("%T(%I==%s && %I==%s && %I==%u)", KRB_Type,
                        KRB_host, host, KRB_commproc, comproc,
                        KRB_id, id);
      else
         where = lWhere("%T(%I==%s && %I==%s)", KRB_Type,
                        KRB_host, host, KRB_commproc, comproc);
      client = lFindFirst(gsd.conn_list, where);
      if (!client || !where) {
         ERROR((SGE_EVENT, MSG_KRB_NOCLIENTENTRYFORXYZ_SSI , host, comproc, id));
         goto all_done;
      }
   }

   tgt_ep = lGetElemUlong(lGetList(client, KRB_tgt_list), KTGT_id, tgt_id);
   if (!tgt_ep) {
      ERROR((SGE_EVENT, MSG_KRB_NOTGTFOUNDFORWXYWITHIDZ_SSID , host, comproc, id, tgt_id));
      goto all_done;
   }

   creds = (krb5_creds **)str2ptr(lGetString(tgt_ep, KTGT_tgt));

   if (creds == NULL)
      goto all_done;
   
   *tgt_creds = creds;
   ret = 0;

all_done:

   if (where)
      lFreeWhere(&where);

   DEXIT;
   return ret;
}


/*
 * krb_put_tgt - Store a TGT into the client's TGT list. Storing a
 * NULL TGT will free an existing TGT with a matching tgt_id.
 */

int
krb_put_tgt(
const char *host,
const char *comproc,
int id,
u_long tgt_id,
krb5_creds **tgt_creds 
) {
   int ret = -1;
   lCondition *where=NULL;
   lListElem  *client=NULL, *tgt_ep;
   krb5_creds **creds=NULL;

   DENTER(TOP_LAYER, "krb_put_tgt");

#if 0
   if (host == NULL && comproc == NULL) {

      if (gsd.tgt_creds)
         krb5_free_tgt_creds(gsd.context, gsd.tgt_creds);
      gsd.tgt_creds = tgt_creds;
      ret = 0;

      goto all_done;
   }
#endif

   if (gsd.daemon && !strcmp(prognames[EXECD], gsd.progname))
      client = gsd.client;
   else {
      if (id)
         where = lWhere("%T(%I==%s && %I==%s && %I==%u)", KRB_Type,
                        KRB_host, host, KRB_commproc, comproc,
                        KRB_id, id);
      else
         where = lWhere("%T(%I==%s && %I==%s)", KRB_Type,
                        KRB_host, host, KRB_commproc, comproc);
      client = lFindFirst(gsd.conn_list, where);
      if (!client || !where) {
         ERROR((SGE_EVENT, MSG_KRB_NOTCLIENTENTRYFORXYZUNABLETOSTORETGT_SSI , host, comproc, id));
         goto all_done;
      }
   }

   /* if TGT already exists in the client's list, free it */

   if ((tgt_ep = lGetElemUlong(lGetList(client, KRB_tgt_list), KTGT_id, tgt_id))) {
      creds = (krb5_creds **)str2ptr(lGetString(tgt_ep, KTGT_tgt));
      if (creds)
         krb5_free_tgt_creds(gsd.context, creds);
      lFreeElem(lDechainElem(lGetList(client, KRB_tgt_list), tgt_ep));
   }

   /* store TGT in list based on TGT ID, for later retrieval */

   if (tgt_creds)
      if ((tgt_ep = lAddSubUlong(client, KTGT_id, tgt_id, KRB_tgt_list, KTGT_Type)))
         lSetString(tgt_ep, KTGT_tgt, ptr2str(tgt_creds, NULL));
               
#if 0
   creds = (krb5_creds **)str2ptr(lGetString(client, KRB_tgt));
   if (creds)
      krb5_free_tgt_creds(gsd.context, creds);
   
   lSetString(client, KRB_tgt, ptr2str(tgt_creds, NULL));
#endif

   ret = 0;

all_done:

   if (where)
      lFreeWhere(&where);

   DEXIT;
   return ret;
}


/*
 * krb_store_forwarded_tgt - store a TGT into the job's credentials cache
 */

int
krb_store_forwarded_tgt(
int uid,
int jobid,
krb5_creds **tgt_creds 
) {
   int                  ret = 0;
   krb5_error_code      rc = 0;
   char                 ccname[40];
   krb5_ccache          ccache = NULL;

   DENTER(TOP_LAYER, "krb_store_fowarded_tgt");

   krb_get_ccname(jobid, ccname);

   /* setenv("KRB5CCNAME", ccname, 0); */

   if ((rc = krb5_cc_resolve(gsd.context, ccname, &ccache)))
       goto all_done;

   if ((rc = krb5_cc_initialize(gsd.context, ccache, (*tgt_creds)->client)))
      goto all_done;

   if ((rc = krb5_cc_store_cred(gsd.context, ccache, *tgt_creds)))
       goto all_done;

   if ((rc = chown(ccname+5, uid, -1))) {
       struct stat statbuf;
       if (stat(ccname + 5, & statbuf) == 0) {
           if (statbuf.st_uid == uid)
               rc = 0;
       }
   }

all_done:

   if (ccache)
      krb5_cc_close(gsd.context, ccache);

   if (rc) {
      ERROR((SGE_EVENT, MSG_KRB_FAILEDSTORINGFORWARDEDTGTFORUIDXJOBYZ_IIS ,
             (int)uid, (int)jobid, error_message(rc)));
      ret = -1;
   }

   DEXIT;
   return ret;
}


/*
 * krb_destroy_forwarded_tgt - destroy the job's credentials cache
 */

int
krb_destroy_forwarded_tgt(
int jobid 
) {
   int                  ret = 0;
   krb5_error_code      rc = 0;
   char                 ccname[40];
   krb5_ccache          ccache = NULL;

   DENTER(TOP_LAYER, "krb_destory_fowarded_tgt");

   krb_get_ccname(jobid, ccname);

   if ((rc = krb5_cc_resolve(gsd.context, ccname, &ccache)))
      goto all_done;

   if ((rc = krb5_cc_destroy(gsd.context, ccache)))
      goto all_done;

all_done:

   if (rc && rc != KRB5_FCC_NOFILE) {
      ERROR((SGE_EVENT,MSG_KRB_FAILEDDELETINGTGTFORJOBXY_IS , jobid, error_message(rc)));
      ret = -1;
   }

   DEXIT;
   return ret;
}


/*
 * krb_get_ccname - return the name of the job's credentials cache
 */

char *krb_get_ccname(
int jobid,
char *ccname 
) {
   if (ccname == NULL) {
      ccname = (char *)malloc(40);
      if (ccname == NULL)
         return NULL;
   }
   sprintf(ccname, "FILE:/tmp/krb5cc_sge%d", jobid);
   return ccname;
}


krb5_context
krb_context(void)
{
   return gsd.context;
}

krb_global_data_t
*krb_gsd(void)
{
   return &gsd;
}

void
krb_set_tgt_id(u_long tgt_id)
{
   gsd.tgt_id = tgt_id;
}

