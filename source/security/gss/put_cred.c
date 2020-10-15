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
#include <pwd.h>
#include <grp.h>
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
   fprintf(stderr, MSG_GSS_PUTCRED_USAGE, p);
   fprintf(stderr, "\n");
   exit(1);
}


int
main(int argc, char **argv)
{
   char *service_name = NULL;
   gss_cred_id_t server_creds = GSS_C_NO_CREDENTIAL;
   gss_buffer_desc client_cred;
   char *username = NULL;
   int cc=0;
   int ch;
   extern int optind;
   extern char *optarg;
   char *cmd = NULL, *ecmd = NULL;
   int verbose=0;
   char *become_user=NULL;
   char *change_owner=NULL;
   char *ccname = getenv("KRB5CCNAME");
   char *ccbuf;
   int i;
   char lenbuf[GSSLIB_INTSIZE];


   if (ccname) {
      ccbuf = strdup(ccname);
      ccname = ccbuf;
   }

   while ((ch = getopt(argc, argv, "o:b:vu:s:c:e:")) != EOF) {
      switch (ch) {
         case 'b':
            become_user = optarg;
            break;
         case 'o':
            change_owner = optarg;
            break;
         case 'u':
            username = optarg;
            break;
         case 'v':
            verbose = 1;
            break;
         case 's':
            service_name = optarg;
            break;
         case 'c':
            cmd = optarg;
            break;
	 case 'e':
	    ecmd = optarg;
	    break;
         default:
            usage(argv[0]);
            break;
      }
   }

   if (argc != optind)
      usage(argv[0]);

   gsslib_verbose(verbose);

   if (verbose) {
      fprintf(stderr, "%s", MSG_GSS_PUTCRED_ARGUMENTS);
      for (i=0; i<argc; i++)
         fprintf(stderr, "%s ", argv[i]);
      fputc('\n', stderr);
   }

   /*
    * get credentials for the SGE/SGE service
    */

   if (service_name) {

      cc = gsslib_acquire_server_credentials(service_name, &server_creds);

      if (cc) {
         fputs(gsslib_error(), stderr);
         return cc;
      }

   }

   /*
    * read client credentials buffer from stdin
    */

   if (read(0, lenbuf, sizeof(lenbuf)) != sizeof(lenbuf)) {
      fprintf(stderr, "%s\n", MSG_GSS_FAILEDREADINGCREDENTIALLENGTHFROMSTDIN );
      return 3;
   }
   client_cred.length = gsslib_unpackint(lenbuf);
   if (verbose)
      fprintf(stderr, "credentials length = %d\n", (int)client_cred.length);

   if ((client_cred.value = (char *)malloc(client_cred.length)) == 0) {
      fprintf(stderr, MSG_GSS_COULDNOTALLOCATEXBYTESFORCREDENTIALS_I ,
              (int) client_cred.length);
      fprintf(stderr, "\n"); 
      return 3;
   }

   if (read(0, client_cred.value, client_cred.length) != client_cred.length) {
      fprintf(stderr, "%s\n", MSG_GSS_FAILEDREADINGCREDENTIALFROMSTDIN );
      return 3;
   }

   /*
    * establish and forward client credentials
    */

   cc = gsslib_put_credentials(server_creds, &client_cred, username);

   if (cc) {
      fputs(gsslib_error(), stderr);
      return cc;
   } else
      fputs(gsslib_error(), stderr);

   if (become_user || change_owner) {
      struct passwd *pw;
      char *owner;

      owner = change_owner ? change_owner : become_user;

      if (!(pw = getpwnam(owner))) {
         fprintf(stderr, MSG_GSS_COULDNOTGETUSERIDFORXY_SS ,
                 owner, strerror(errno));
         fprintf(stderr, "\n");
         cc = 4;
         goto error;
      }

      /* change ownership of credentials file to user */

      if (pw->pw_uid != geteuid()) {

         char *new_ccname = getenv("KRB5CCNAME");

         if (new_ccname == NULL || strncasecmp(new_ccname, "file:", 5) != 0) {
            fprintf(stderr, MSG_GSS_COULDNOTCHANGEOWNERSHIPOFCREDENTIALSCACHETOXINVALIDKRB5CCNAME_S, owner);
            fprintf(stderr, "\n");
            cc = 4;
            goto error;
         }

         if (chown(&new_ccname[5], pw->pw_uid, pw->pw_gid) < 0) {
            fprintf(stderr, MSG_GSS_COULDNOTCHANGEOWNERSHIPOFXTOYZ_SSS ,
                    &new_ccname[5], owner, strerror(errno));
            fprintf(stderr, "\n");
            cc = 4;
            goto error;
         }

#ifdef DCE

         /*
          * take care of the "extra" DCE credentials files
          */

         {
            char src[PATH_MAX];
            stringT msg;

            errno = 0;
            sprintf(src, "%s.data", &new_ccname[5]);
            if (chown(src, pw->pw_uid, pw->pw_gid) != 0) {
               snprintf(msg, sizeof(msg), MSG_FILE_NOCHOWN_SS, src, "");
               perror(msg);
               goto error;
            }
            sprintf(src, "%s.data.db", &new_ccname[5]);
            if (chown(src, pw->pw_uid, pw->pw_gid) != 0) {
               snprintf(msg, sizeof(msg), MSG_FILE_NOCHOWN_SS, src, "");
               perror(msg);
               goto error;
            }
            sprintf(src, "%s.nc", &new_ccname[5]);
            if (chown(src, pw->pw_uid, pw->pw_gid) != 0) {
               snprintf(msg, sizeof(msg), MSG_FILE_NOCHOWN_SS, src, "");
               perror(msg);
               goto error;
            }
         }

#endif

      }

      if (become_user) {

         if (initgroups(pw->pw_name, pw->pw_gid) || setgid(pw->pw_gid)<0) {
	    cc = 4;
	    perror(MSG_GSS_PERROR_SETGID);
	    goto error;
	 }

	 if (setuid(pw->pw_uid)<0) {
	    cc = 4;
	    perror(MSG_GSS_PERROR_SETUID );
	    goto error;
	 }
      }
   }

#ifdef DCE

   /*
    * Link the user-supplied credentials cache file name to the
    * DCE credentials cache file if they have different file names
    */

   {
      char *dce_ccname = getenv("KRB5CCNAME");
      char src[PATH_MAX], dst[PATH_MAX];

      fprintf(stderr, "dce_ccname=%s\n", dce_ccname);
      fprintf(stderr, "ccname=%s\n", ccname);

      if (cc==0 && ccname && dce_ccname &&
          strcmp(ccname, dce_ccname)) {

         if (strncasecmp(ccname, "file:", 5) == 0 &&
             strncasecmp(dce_ccname, "file:", 5) == 0) {

            if (symlink(&dce_ccname[5], &ccname[5]) < 0) {
               fprintf(stderr, MSG_GSS_COULDNOTLINKXTODCECREDENTIALSCACHEFILEYZ_SSS ,
                       ccname, dce_ccname, strerror(errno));
               fprintf(stderr, "\n");
            }

	    sprintf(src, "%s.data", &dce_ccname[5]);
	    sprintf(dst, "%s.data", &ccname[5]);
	    symlink(src, dst);

	    sprintf(src, "%s.data.db", &dce_ccname[5]);
	    sprintf(dst, "%s.data.db", &ccname[5]);
	    symlink(src, dst);

	    sprintf(src, "%s.nc", &dce_ccname[5]);
	    sprintf(dst, "%s.nc", &ccname[5]);
	    symlink(src, dst);


         } else {
            fprintf(stderr, MSG_GSS_COULDNOTLINKXTODCECREDENTIALSCACHEFILEYINVALIDKRB5CCNAMEENVIRONMENTVARIABLEFORMAT_SS, ccname, dce_ccname);
            fprintf(stderr, "\n");
         }
      }
   }

#endif

   if (verbose)
      fprintf(stderr, "KRB5CCNAME=%s\n", getenv("KRB5CCNAME"));

   if (cmd)
      cc = system(cmd);

   if (ecmd) {
      int eargc = 0;
      char *eargv[256];
      eargv[eargc] = strtok(ecmd, " \t");
      while(eargv[eargc])
	 eargv[++eargc] = strtok(NULL, " \t");
      execv(eargv[0], eargv);
      perror("exec failed");
   }

error:

   return cc;
}

