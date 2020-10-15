
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>  /* for inet_makeaddr() */

#include "comm/cl_commlib.h"
#include "comm/cl_host_list.h"

#include "uti/sge_stdlib.h"

#include "uti/sge_stdlib.h"

extern int
main(int argc, char** argv)
{
  int retval = 0;
  int arg = 0;
  struct in_addr addr;
  struct in_addr addr2;
  char* rhost = NULL;
  int loops = -1;


  if (argc < 3) {
      printf("usage: test_cl_hostname_list <DEBUGLEVEL> hostnames\n");
      exit(1);
  }

  printf("commlib setup ...\n");
  retval = cl_com_setup_commlib(CL_NO_THREAD, (cl_log_t)atoi(argv[1]), NULL);
  printf("%s\n\n",cl_get_error_text(retval));

  printf("reslovling host addr 129.157.141.10 ...\n");
  addr = inet_makeaddr(129*256+157,141*256+10);
  printf("ip addr: %s\n",inet_ntoa(addr));  /* inet_ntoa() is not MT save */
  addr2 = inet_makeaddr(192*256+168,11*256+1);
  printf("ip addr2: %s\n",inet_ntoa(addr2));  /* inet_ntoa() is not MT save */



  while ((loops--) != 0) { 
  arg = 2;
  printf("loop\n\n\n");
     while( argv[arg] != NULL) {
        printf("resolving host \"%s\" ...\n", argv[arg]);
        retval = cl_com_cached_gethostbyname(argv[arg],&rhost,NULL,NULL,NULL);
        printf("%s\n",cl_get_error_text(retval));
   
        if (rhost != NULL) {
           printf(" -> host resolved as \"%s\"\n", rhost );
           sge_free(&rhost);
        }
   
        printf("cl_com_gethostbyaddr ... %s\n", inet_ntoa(addr)); /* inet_ntoa() is not MT save */

        retval = cl_com_cached_gethostbyaddr(&addr, &rhost, NULL, NULL);
        printf("%s\n",cl_get_error_text(retval));
   
        if (retval == CL_RETVAL_OK) {
           printf(" -> host name is \"%s\"\n", rhost);
        }
        sge_free(&rhost);
        rhost = NULL;
   
        printf("cl_com_gethostbyaddr ... %s\n",inet_ntoa(addr2) ); /* inet_ntoa() is not MT save */

   
        retval = cl_com_cached_gethostbyaddr(&addr2, &rhost, NULL, NULL);
        printf("%s\n",cl_get_error_text(retval));
   
        if (retval == CL_RETVAL_OK) {
           printf(" -> host name is \"%s\"\n", rhost);
        }
        sge_free(&rhost);
        rhost = NULL;
   
        printf("***********************************************************\n");
        arg++;
     }
     sleep(1); 
  }
  
  
  printf("commlib cleanup ...\n");
  retval = cl_com_cleanup_commlib();
  printf("%s\n\n",cl_get_error_text(retval));


  printf("main done\n");
  return 0;
}
 




