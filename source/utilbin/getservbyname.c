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
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>

#include "msg_utilbin.h"
#include "basis_types.h"

#include "gdi/version.h"

#if defined(INTERIX)
#include <arpa/inet.h>
#endif

void usage(void)
{
   fprintf(stderr, "Version: %s\n", GDI_VERSION);
   fprintf(stderr, "%s\n getservbyname [-help|-number] service | -check port_number\n\n%s\n",MSG_UTILBIN_USAGE, MSG_COMMAND_USAGE_GETSERVBYNAME );
   /*fprintf(stderr, "       get number of a tcp service\n"); */
   exit(1);
}   

/*-------------------------------------------------------*/
int main(int argc, char *argv[])
{
 int retry = 5;
 int number_only = 0;
 int check_port = 0;
 struct servent *se = NULL; 


 if (argc < 2 || argc > 3)
    usage();
    
 if (!strcmp(argv[1], "-number"))
    number_only = 1;

 if (!strcmp(argv[1], "-check"))
    check_port = 1;
    
 if (!strcmp(argv[1], "-help"))
    usage();
 
 if (check_port) {
    if (!argv[1+check_port]) usage();
    while (retry-- && !((se = getservbyport(htons(atoi(argv[1+check_port])), "tcp"))))
       ;
    if (!se) {
       fprintf(stderr, MSG_SYSTEM_PORTNOTINUSE_S, argv[1+check_port]);
       fprintf(stderr, "\n");
       exit(1);
    } 
    else {
       printf("%s\n", se->s_name);
       exit(0);
    }
 }
 else {
    if (!argv[1+number_only]) usage();
    while (retry-- && !((se = getservbyname(argv[1+number_only], "tcp"))))
       ;
    if (!se) {
       fprintf(stderr, MSG_SYSTEM_SERVICENOTFOUND_S , argv[1+number_only]);
       fprintf(stderr, "\n");
       exit(1);
    }

    if (number_only) { 
       printf("%d\n", ntohs(se->s_port));
       exit (0);
    }

    printf("%s %d\n", argv[1], ntohs(se->s_port));
    exit(0);

 }
 return 0;
}
