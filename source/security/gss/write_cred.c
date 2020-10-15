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
#include <sys/param.h>
#include <fcntl.h>
#include <pwd.h>
#if defined KRBGSS && !defined HEIMDAL
#include <gssapi/gssapi_generic.h>
#else
#include <gssapi.h>
#endif
#include "sge_gsslib.h"
#include "msg_gss.h"
/* #include "sge_language.h" */

void
usage(char *progname)
{
   char *p;
   p = (NULL == (p = strrchr(progname,'/'))) ? progname : p+1;
   fprintf(stderr, MSG_GSS_WRITECRED_USAGE_S , p);
   fprintf(stderr, "\n");
   exit(1);
}


int
main(int argc, char **argv)
{
   gss_buffer_desc client_cred;
   int cc=0;
   int fd=1;
   char lenbuf[GSSLIB_INTSIZE];


   if (argc > 1) {
      fd = SGE_OPEN3(argv[1], O_WRONLY|O_CREAT, 0700);
      if (fd < 0) {
	 fprintf(stderr, MSG_GSS_COULDNOTOPENXY_SS , argv[1], strerror(errno));
    fprintf(stderr, "\n");
	 exit(1);
      }
   }
     

   /*
    * read client credentials buffer from stdin
    */

   fprintf(stderr, "%s\n", MSG_GSS_READINGCREDENTIALLENGTH);

   if (read(0, lenbuf, sizeof(lenbuf)) != sizeof(lenbuf)) {
      fprintf(stderr, "%s\n", MSG_GSS_FAILEDREADINGCREDENTIALLENGTHFROMSTDIN);
      return 3;
   }
   client_cred.length = gsslib_unpackint(lenbuf);             

   fprintf(stderr, MSG_GSS_READLENGTHOFX_I, (int)client_cred.length);
   fprintf(stderr, "\n");

   if ((client_cred.value = (char *)malloc(client_cred.length)) == 0) {
      fprintf(stderr, MSG_GSS_COULDNOTALLOCATEXBYTESFORCREDENTIALS_I,
              (int)client_cred.length);
      fprintf(stderr, "\n");
      return 3;
   }

   if (read(0, client_cred.value, client_cred.length) != client_cred.length) {
      fprintf(stderr, "%s\n", MSG_GSS_FAILEDREADINGCREDENTIALLENGTHFROMSTDIN);
      return 3;
   }

   fprintf(stderr, MSG_GSS_READXBYTES_I, (int)client_cred.length);
   fprintf(stderr, "\n");

   /*
    * write credentials to stdout
    */                                                        
                                                              
   if (client_cred.length) {                                         
      gsslib_packint(client_cred.length, lenbuf);
      write(fd, lenbuf, sizeof(lenbuf));                      
      write(fd, client_cred.value, client_cred.length);                      
      fprintf(stderr, MSG_GSS_WROTEXBYTES_I , (int)client_cred.length);
      fprintf(stderr, "\n");
   } else {                                                   
      fprintf(stderr, MSG_GSS_WRITECREDNOCREDENTIALSFOUND);      
      fprintf(stderr, "\n");
      cc = 1;                                                 
   }                                                          
  
   return cc;
}

