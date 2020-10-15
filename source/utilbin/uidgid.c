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
#include <stdlib.h>
#include <string.h>

#include "basis_types.h"
#if defined(INTERIX)
#  include "wingrid.h"
#endif

/*-------------------------------------------*/
void usage(void)
{
   fprintf(stderr, "usage: uidgid -uid|-gid|-euid|-egid\n");
   exit(1);
} 
 
/*-------------------------------------------*/
int main(int argc, char *argv[])
{
 if (argc != 2)
    usage();
 
 if (!strcmp(argv[1], "-uid"))
    printf(uid_t_fmt"\n", getuid());
 else if (!strcmp(argv[1], "-euid"))
#if defined(INTERIX)
    /*
     * In Interix, return 0 instead of Administrator ID,
     * because the Installation script tests always for 0.
     */
    if(wl_is_user_id_superuser(geteuid())) {
       printf("0\n");
    } else {
       printf(uid_t_fmt"\n", geteuid());
    }
#else
    printf(uid_t_fmt"\n", geteuid());
#endif
 else if (!strcmp(argv[1], "-gid"))
    printf(gid_t_fmt"\n", getgid());
 else if (!strcmp(argv[1], "-egid"))
    printf(gid_t_fmt"\n", getegid());
 else
    usage();
 exit(0);
 return 0;   
}
