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
#include "msg_gss.h"
/* #include "uti/sge_language.h" */
void
usage(char *progname)
{
   char *p;
   p = (NULL == (p = strrchr(progname,'/'))) ? progname : p+1;
   fprintf(stderr, MSG_GSS_ACQUIREX_USAGE_S, p);
   fprintf(stderr, "\n");
   fprintf(stderr, MSG_GSS_ACQUIREX_s_OPT_USAGE , p);
   fprintf(stderr, "\n");
   exit(1);
}


int
main(int argc, char **argv)
{
   char *service_name = NULL;
   gss_cred_id_t server_creds;
   int cc=0;
   int ch;
   extern int optind;
   extern char *optarg;


   while ((ch = getopt(argc, argv, "u:s:")) != EOF) {
      switch (ch) {
         case 's':
            service_name = optarg;
            break;
         default:
            usage(argv[0]);
            break;
      }
   }

   if (argc != optind)
      usage(argv[0]);

   /*
    * get credentials for the specified service
    */

   if (service_name) {

      cc = gsslib_acquire_server_credentials(service_name, &server_creds);

      if (cc) {
         fputs(gsslib_error(), stderr);
         return cc;
      }

   }

   return 0;
}

