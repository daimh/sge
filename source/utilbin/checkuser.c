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
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "msg_utilbin.h"
#include "basis_types.h"

/*-------------------------------------------*/
void usage(void)
{
   fprintf(stderr, "%s checkuser [-check name]|[-uid name]|[-gid name]\n" , MSG_UTILBIN_USAGE );
   exit(1);
} 
 
/*-------------------------------------------*/
int main(int argc, char *argv[])
{
 int i = 10;
 struct passwd *pw;
 pw = NULL;
 
 if (argc != 3)
    usage();
 
 while (i-- && !pw)
   pw = getpwnam(argv[2]);
   
 if (!pw || !pw->pw_name)
    exit(1);  
   
 if (!strcmp(argv[1], "-check"))
    exit(0);
 else if (!strcmp(argv[1], "-uid"))
    printf(uid_t_fmt"\n", pw->pw_uid);
 else if (!strcmp(argv[1], "-gid"))
    printf(gid_t_fmt"\n", pw->pw_gid);
 else
    usage();
 exit(0);
 return 0;   
}
