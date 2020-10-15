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
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

#include "basis_types.h"
#include "msg_utilbin.h"
#include "uti/sge_uidgid.h"


void usage(void)
{
   fprintf(stderr,"%s\n adminrun username command ...\n\n", MSG_UTILBIN_USAGE );
   fprintf(stderr,MSG_COMMAND_RUNCOMMANDASUSERNAME_S, "<username>" );
   fprintf(stderr, "\n");
   exit(1);
}

int main(int argc, char **argv)
{
   struct passwd *pw = NULL;
   int i;

   if (argc < 3)
      usage();

   if(geteuid() != SGE_SUPERUSER_UID) {
      argv+=2;
      execvp(argv[0], argv);
      fprintf(stderr, "execvp errno=%d\n", errno);
      fprintf(stderr, "Arguments, passed to adminrun:\n");
      for (i=0; argv[i] != NULL; i++) {
         fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);
      }
      fprintf(stderr, MSG_COMMAND_EXECUTEFAILED_S , argv[0]);
      fprintf(stderr, "\n");
      return 127;
   }

   i = 10;
   while (i-- && !pw)   
      pw = getpwnam(argv[1]);   
 
   if (!pw || !pw->pw_name) {
      fprintf(stderr, MSG_SYSTEM_RESOLVEUSERFAILED_S , argv[1]);
      fprintf(stderr, "\n");
      return 1;
   }
   
   if (initgroups(pw->pw_name, pw->pw_gid) || setgid(pw->pw_gid)
       || setuid(pw->pw_uid)) {
      fprintf(stderr, "%s\n", MSG_SETUID_FAILED);
      return 1;
   }
 
   argv += 2;
   execvp(argv[0], argv);
   fprintf(stderr, "execvp errno=%d\n", errno);
   fprintf(stderr, "Arguments, passed to adminrun:\n");
   for (i=0; argv[i] != NULL; i++) {
      fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);
   }

   fprintf(stderr, MSG_COMMAND_EXECUTEFAILED_S , argv[0]);
   fprintf(stderr, "\n");
   return 127;
}
