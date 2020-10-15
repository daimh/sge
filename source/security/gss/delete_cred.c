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
#include <limits.h>
#include <fcntl.h>
#if defined KRBGSS && !defined HEIMDAL
#include <gssapi/gssapi_generic.h>
#else
#include <gssapi.h>
#endif
#include "sge_gsslib.h"
/* #include "sge_language.h" */
#include "msg_gss.h"

int
main(int argc, char **argv)
{
   char *ccname;
   int cc = 0;


   if ((ccname=getenv("KRB5CCNAME"))) {

      if (strncasecmp(ccname, "file:", 5)==0) {

#ifdef KRBGSS

         if (unlink(&ccname[5])<0) {
            cc = 3;
            perror(MSG_GSS_PERROR_UNLINK);
         }

#endif

#ifdef DCE

         /*
          * for DCE, KRB5CCNAME may be a link to the actual
          * credentials cache files. If so, we delete both the
          * links and the files
          */

         {
            struct stat st;
            char rname[PATH_MAX], fname[PATH_MAX];

            if (lstat(&ccname[5], &st)<0) {
               cc = 3;
               perror(MSG_GSS_PERROR_GETTINGFILESTATUS);
               goto error;
            }

            if (S_ISLNK(st.st_mode)) {

               if (readlink(&ccname[5], rname, sizeof(rname)) >= 0) {

                  /* remove the "real" credentials */

                  if (unlink(rname)<0) {
                     perror("unlink");
                     cc = 3;
                  }

                  snprintf(fname, sizeof rname, "%s.data", rname);
                  unlink(fname);

                  snprintf(fname, sizeof rname, "%s.data.db", rname);
                  unlink(fname);

                  snprintf(fname, sizeof rname, "%s.nc", rname);
                  unlink(fname);


               } else {
                  cc = 3;
                  perror(MSG_GSS_PERROR_GETTINGLINK);
               }

               /* delete links or file pointed to by KRB5CCNAME */

               if (unlink(&ccname[5])<0) {
                  perror(MSG_GSS_PERROR_UNLINK);
                  cc = 3;
               }

               snprintf(fname, sizeof fname, "%s.data", &ccname[5]);
               unlink(fname);

               snprintf(fname, sizeof fname, "%s.data.db", &ccname[5]);
               unlink(fname);

               snprintf(fname, sizeof fname, "%s.nc", &ccname[5]);
               unlink(fname);

               snprintf(fname, sizeof fname, "%s.tmp", &ccname[5]);
               unlink(fname);

            }
	 }

#endif /* DCE */

      } else {
         cc = 2;
         fprintf(stderr, "%s\n", MSG_GSS_DELETECREDKRB5CCNAMEENVVARHASINVALIDFORMAT );
      }

   } else {
      cc = 1;
      fprintf(stderr, "%s\n", MSG_GSS_DELETECREDKRB5CCNAMEENVVARNOTSET );
   }

#ifdef DCE
   error:
#endif

   if (cc)
      fprintf(stderr, "%s\n", MSG_GSS_DELETECREDCOULDNOTDELCREDENTIALS);

   return cc;
}

