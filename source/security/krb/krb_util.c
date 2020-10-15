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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_prog.h"
#include "uti/sge_spool.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_conf.h"

#include "comm/commlib.h"

#include "spool/classic/read_write_job.h"

#include "basis_types.h"
#include "sge.h"

#include "msg_krb.h"

#include "krb_data.h"
#include "krb_lib.h"

#include "sge_krb_KRB_L.h"
#include "krb5.h"				/* Kerberos stuff 	*/
#include "com_err.h"


/*
 * krb_renew_tgts - Renew TGTs on behalf of the job client. This routine
 * gets the TGT out of the job entry, decrypts it, and checks to see if
 * the TGT needs renewing.  If it does, it renews the TGT and stores the
 * new TGT back into the job entry.  This routine is executed in both the
 * qmaster and the execd.
 */

#define FLAGS2OPTS(flags) (flags & KDC_TKT_COMMON_MASK)

int
krb_renew_tgts(
lList *joblist 
) {
   krb5_error_code rc;
   static u_long32 next_time = 0;
   u_long32 now = sge_get_gmt();
   lListElem *job;
   krb5_context context = krb_context();
   krb5_timestamp time_now;
   krb_global_data_t *gsd = krb_gsd();

   DENTER(TOP_LAYER, "krb_renew_tgts");


   if ((now = sge_get_gmt())<next_time) {
      DEXIT;
      return 0;
   }

   if ((rc = krb5_timeofday(context, &time_now))) {
      ERROR((SGE_EVENT, MSG_KRB_KRB5TIMEOFDAYFAILEDX_S ,
	     error_message(rc)));
      DEXIT;
      return -1;
   }

   /* renew job TGT's */

   for_each(job, joblist) {

      krb5_error_code rc;
      krb5_creds ** tgt_creds = NULL;
      krb5_data tgtbuf;
      const char *tgtstr = NULL;

      tgtbuf.length = 0;

      /* get TGT out of job entry */

      if ((tgtstr = lGetString(job, JB_tgt))) {

         tgtbuf.data = krb_str2bin(tgtstr, NULL, &tgtbuf.length);

         if (tgtbuf.length) {

            /* decrypt the TGT using the daemon key */

            if ((rc = krb_decrypt_tgt_creds(&tgtbuf, &tgt_creds))) {

               ERROR((SGE_EVENT, MSG_KRB_COULDNOTDECRYPTTGTFORJOBXY_DS,
                      sge_u32c(lGetUlong(job, JB_job_number)),
                      error_message(rc)));

            }

	    if (rc == 0 && tgt_creds) {

               krb5_creds *tgt = *tgt_creds;

	       /*
		* If TGT is renewable and TGT expiration time is not past
		* and is within the SGE renewal threshold and the TGT
		* renewal period is not past, then renew the TGT
		*/

	       if (tgt->ticket_flags & KDC_OPT_RENEWABLE &&
		   tgt->times.endtime > time_now &&
		   tgt->times.renew_till > time_now &&
		   tgt->times.endtime < time_now + gsd->tgt_renew_threshold) {

		  krb5_creds *new_creds[2];
		  krb5_creds creds;

		  memset(new_creds, 0, sizeof(new_creds));
		  memset(&creds, 0 ,sizeof(creds));

		  /* renew the TGT */

		  if (((rc = krb5_copy_principal(context, (*tgt_creds)->server,
			&creds.server))) ||
		      ((rc = krb5_copy_principal(context, (*tgt_creds)->client,
			&creds.client))) ||
		      ((rc = krb5_get_cred_via_tkt(context, tgt,
		        FLAGS2OPTS(tgt->ticket_flags)|KDC_OPT_RENEW,
		        tgt->addresses, &creds, &new_creds[0])))) {

		     ERROR((SGE_EVENT, MSG_KRB_COULDNOTRENEWTGTFORJOBXY_DS, 
                  sge_u32c(lGetUlong(job, JB_job_number)),
			         error_message(rc)));

		  }

		  krb5_free_cred_contents(context, &creds);

		  if (rc == 0) {
                     krb5_data outbuf;

		     /* store the new TGT back into the job entry */

		     outbuf.length = 0;

		     if ((rc = krb_encrypt_tgt_creds(new_creds, &outbuf))) {

			ERROR((SGE_EVENT, MSG_KRB_COULDNOTECRYPTTGTFORJOBXY_DS,
                sge_u32c(lGetUlong(job, JB_job_number)),
			       error_message(rc)));

		     } else {

			lSetString(job, JB_tgt,
			      krb_bin2str(outbuf.data, outbuf.length, NULL));
                     }

		     /* if we are called by the execd, also store the
			new TGT in the credentials cache of the user */

                     if (!strcmp(prognames[EXECD], gsd->progname)) {
                         
                        int retries = MAX_NIS_RETRIES;
			struct passwd *pw = NULL;

			while (retries-- && !pw)
			   pw = getpwnam(lGetString(job, JB_owner));

			if (pw) {

			   if ((krb_store_forwarded_tgt(pw->pw_uid,
				    lGetUlong(job, JB_job_number),
				    new_creds))) {

			       ERROR((SGE_EVENT, MSG_KRB_COULDNOTSTORERENEWEDTGTFORXJOBY_SD,
                                      lGetString(job, JB_owner),
				      sge_u32c(lGetUlong(job, JB_job_number))));
                           }

                        } else {

			   ERROR((SGE_EVENT, MSG_KRB_COULDNOTGETUSERIDFORXY_SD , lGetString(job, JB_owner),
				  sge_u32c(lGetUlong(job, JB_job_number))));
                        }
		     }

		     if (outbuf.length)
			krb5_xfree(outbuf.data);

		  }

      if (!mconf_get_simulate_jobs()) {
         job_write_spool_file(job, 0, NULL, SPOOL_DEFAULT);;
      }

		  if (new_creds[0])
		     krb5_free_creds(context, new_creds[0]);

	       }
	    }
         }

         if (tgtbuf.length)
            krb5_xfree(tgtbuf.data);

         if (tgt_creds)
            krb5_free_tgt_creds(context, tgt_creds);

      }

   }

   next_time = now + gsd->tgt_renew_interval;

   DEXIT;
   return 0;
}

