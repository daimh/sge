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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "msg_utilbin.h"
#include "uti/sge_uidgid.h"

/****** testsuidroot ***************************************
*
*  NAME
*     testsuidroot -- test if suid root flag works
*
*  SYNOPSIS
*     testsuidroot
*
*  FUNCTION
*     Tests, if the set user id flag works for user id root on
*     the filesystem where testsuidroot is installed (might be
*     disabled on NFS mounted filesystems).
*
*     testsuidroot must be owned by root and the suid flag must be set.
*     Example:
*     -rwsr-xr-x   1 root     root        7632 Mar 21 09:23 testsuidroot
*     
*     testsuidroot will perform the following checks:
*        - has the program been started under a uid != 0
*        - is the effective uid 0
*        - can the program bind a privileged socket
*     
*     If a test fails, testsuid exits with an error message and return code
*     != 0.
*
*  INPUTS
*     -q - optional parameter, sets quiet mode, no output will be generated
*
*  RESULT
*     0, if all tests are OK
*     1, if testsuidroot has been started with uid 0
*     2, if effective uid != 0
*     3, if binding a privileged port fails
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*
****************************************************************************
*/

int main(int argc, char *argv[]) {
   int sock;
   int res_port = 1023;
   int quiet = 0;

   if(argc == 2 && strcmp(argv[1], "-q") == 0) {
      quiet = 1;
   }
   if(getuid() == SGE_SUPERUSER_UID) {
      if(!quiet) {
         fprintf(stderr, MSG_SUIDROOT_START_BY_NONROOT, argv[0]);
         fprintf(stderr, "\n");
      }
      return 1;
   }
   if(geteuid() != SGE_SUPERUSER_UID) {
      if(!quiet) {
         fprintf(stderr, MSG_SUIDROOT_EFFECTIVE_USER_ROOT, argv[0]);
         fprintf(stderr, "\n");
      }
      return 2;
   }

   if((sock = rresvport(&res_port)) == -1) {
      if(!quiet) {
         fprintf(stderr, MSG_SUIDROOT_BIND_PRIV_SOCK_FAILED, argv[0]);
         fprintf(stderr, "\n");
      }
      return 3;
   }

   shutdown(sock, 0);
   close(sock);

   return 0;
}
