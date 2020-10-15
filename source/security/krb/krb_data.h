#ifndef __KRB_DATA_H
#define __KRB_DATA_H
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

typedef struct {

  /* client and server info */

   char			progname[256];		/* client/server name */
   char			hostname[256];		/* client/server host */
   int			initialized;		/* initialized flag */
   krb5_context		context;		/* kerberos context */
   int			daemon;			/* am I a SGE deamon? */
   int			qmaster;		/* am I the qmaster? */
   char			service[256];		/* requested service name */
   struct in_addr	hostaddr;		/* local host address */

  /* client-specific info */

   krb5_auth_context    auth_context;		/* local auth_context */
   krb5_principal	clientp;		/* client principal */
   krb5_ccache		ccdef;			/* credentials cache */
   int			reconnect;		/* reconnect flag */
   int                  client_flags;           /* client flags */
   u_long               tgt_id;                 /* TGT identifier */

  /* sge daemon specific info */

   krb5_rcache          rcache;                 /* replay cache */
   char                 qmaster_host[256];      /* qmaster host name */
   krb5_creds		creds;			/* daemon credentials */
   int			tgt_acquired;		/* TGT from keytab yet? */
   krb5_keyblock      * daemon_key;             /* daemon key */
   int                  tgt_renew_threshold;    /* when to renew TGTs */
   int                  tgt_renew_interval;     /* how often to renew TGTs */
   lListElem          * client;                 /* for execd TGT list */

  /* server-specific info */

   int			idle_client_interval;	/* how often to check for
						   idle client connections */
   lList              * conn_list;		/* client connection list */
   krb5_principal	serverp;		/* server principal */
   krb5_keytab          keytab;                 /* keytab file */
   krb5_creds        ** tgt_creds;              /* TGT credentials */

} krb_global_data_t;

krb_global_data_t *krb_gsd(void);

#endif /* __KRB_DATA_H */

