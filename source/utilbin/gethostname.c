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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basis_types.h"
#include "uti/sge_string.h"
#include "uti/sge_arch.h"
#include "uti/sge_hostname.h"
#include "comm/cl_commlib.h"
#include "gdi/version.h"
#include "msg_utilbin.h"

int usage(void)
{
  fprintf(stderr, "Version: %s\n", GDI_VERSION);
  fprintf(stderr, "%s\n gethostname [-help|-name|-aname|-all]\n\n%s\n", MSG_UTILBIN_USAGE, MSG_COMMAND_USAGE_GETHOSTNAME );
  exit(1);
  return 0;
}

int main(int argc,char *argv[]) {
   struct hostent *he = NULL;
   char* resolved_name = NULL;
   int retval = CL_RETVAL_OK;
   char **tp,**tp2;
   int name_only = 0;
   int sge_aliasing = 0;
   int all_option = 0;
   int system_error = 0;

   if (argc < 1 ) {
      usage();
   } 
   if (argc >= 2) {
      if (!strcmp(argv[1], "-help")) {
         usage();
      }
      if (!strcmp(argv[1], "-name")) {
         if (argc != 2) {
            usage(); 
         }
         name_only = 1;
      }   
      if (!strcmp(argv[1], "-aname")) {
         if (argc != 2) {
            usage(); 
         }
         name_only = 1;
         sge_aliasing = 1;
      }   
      if (!strcmp(argv[1], "-all")) {
         if (argc != 2) {
            usage(); 
         }
         name_only = 0;
         sge_aliasing = 1;
         all_option = 1;
      }
   }
  
   if (name_only == 0 && argc != 1 && all_option == 0) {
      usage();
   }
     
  retval = cl_com_setup_commlib(CL_NO_THREAD ,CL_LOG_OFF, NULL);
  if (retval != CL_RETVAL_OK) {
     fprintf(stderr,"%s\n",cl_get_error_text(retval));
     exit(1);
  }

  if (sge_aliasing ) {
     const char *alias_path = sge_get_alias_path();
     cl_com_set_alias_file(alias_path);
     sge_free(&alias_path);
  }

  retval = cl_com_gethostname(&resolved_name, NULL, &he, &system_error);
  if (retval != CL_RETVAL_OK) {
     char* err_text = cl_com_get_h_error_string(system_error);
     if (err_text == NULL) {
        err_text = strdup(strerror(system_error));
        if (err_text == NULL) {
           err_text = strdup("unexpected error");
        }
     }
     fprintf(stderr,"error resolving local host: %s (%s)\n",cl_get_error_text(retval), err_text);
     sge_free(&err_text); 
     cl_com_cleanup_commlib();
     exit(1);
  }


  if (name_only) {
     if (sge_aliasing) {
        if (resolved_name != NULL) {
           printf("%s\n",resolved_name);
        } else {
           printf("%s\n","unexpected error");
        }
     } else {
        if (he != NULL) {
           printf("%s\n",he->h_name);
        } else {
           printf("%s\n","could not get hostent struct");
        }
     }
  } else {
       if (he != NULL) {
          printf(MSG_SYSTEM_HOSTNAMEIS_S , he->h_name);
	  printf("\n");

	  if (resolved_name != NULL && all_option) {
             printf("SGE name: %s\n",resolved_name);
	  }

          printf("%s", MSG_SYSTEM_ALIASES);
	  for (tp = he->h_aliases; *tp; tp++) {
             printf(" %s", *tp);
          }
          printf("\n");
 
          printf("%s", MSG_SYSTEM_ADDRESSES);
          for (tp2 = he->h_addr_list; *tp2; tp2++) {
             printf(" %s", inet_ntoa(* (struct in_addr *) *tp2));  /* inet_ntoa() is not MT safe */
          }
          printf("\n");
       } else {
         fprintf(stderr,"%s\n","could not get hostent struct");
       }
  }
   sge_free(&resolved_name);
   sge_free_hostent(&he);

   retval = cl_com_cleanup_commlib();
   if (retval != CL_RETVAL_OK) {
      fprintf(stderr,"%s\n",cl_get_error_text(retval));
      exit(1);
   }
   return 0;
}
