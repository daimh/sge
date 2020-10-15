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
#if defined KRBGSS && !defined HEIMDAL
#include <gssapi/gssapi_generic.h>
#else
#include <gssapi.h>
#endif
#include "sge_gsslib.h"
/* #include "sge_language.h" */
#include "msg_gss.h"

void
usage(char *progname)
{
   char *p;
   p = (NULL == (p = strrchr(progname,'/'))) ? progname : p+1;
   fprintf(stderr, MSG_GSS_GETCRED_USAGE, p);
   fprintf(stderr, "\n");
   exit(1);
}


int
main(int argc, char **argv)
{
   int cc = 0;
   char *service;
   gss_buffer_desc cred;
   gss_cred_id_t client_creds = GSS_C_NO_CREDENTIAL; 
   int verbose = 0;
   char lenbuf[GSSLIB_INTSIZE];
   int ch;

   while ((ch = getopt(argc, argv, "v")) != EOF) {
      switch (ch) {
         case 'v':
            verbose = 1;
            break;
         default:
            usage(argv[0]);
            break;
      }
   }

   if (argc > optind)
      service = argv[optind];
   else
      service = SERVICE_NAME;

   cred.length = 0;

   if (verbose)
      gsslib_verbose(1);

#if 0

   cc = gsslib_acquire_client_credentials(&client_creds);

   if (cc) {
      fputs(gsslib_error(), stderr);
      return cc;
   }

#endif

   /*
    * get caller's credentials
    */

   cc = gsslib_get_credentials(service, &cred, client_creds);

   if (cc != 0) {
      fputs(gsslib_error(), stderr);
      return 2;
   }

   /*
    * write credentials to stdout
    */

   if (cred.length) {
      if (verbose) {
         fprintf(stderr, MSG_GSS_WRITINGXBYTESTOSTDOUT_I, (int) cred.length);
         fprintf(stderr, "\n");
      }
      gsslib_packint(cred.length, lenbuf);
      if ((GSSLIB_INTSIZE != write(1, lenbuf, GSSLIB_INTSIZE)) ||
	  (cred.length != write (1, cred.value, cred.length))) {
	 perror (MSG_GSS_WRITEFAILED);
	 cc = 1;
      }
   } else {
      fprintf(stderr, "%s\n", MSG_GSS_GETCREDNOCREDENTIALSFOUND );
      cc = 1;
   }

   return cc;
}

