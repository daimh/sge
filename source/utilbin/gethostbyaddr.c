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

#include "comm/lists/cl_util.h"
#include "comm/cl_commlib.h"

#include "uti/sge_language.h"
#include "uti/sge_hostname.h"
#include "uti/sge_arch.h"

#include "gdi/version.h"

#include "msg_utilbin.h"

void usage(void)
{
  fprintf(stderr, "Version: %s\n", GDI_VERSION);
  fprintf(stderr, "%s gethostbyaddr [-help|-name|-aname|-all]x.x.x.x\n", MSG_UTILBIN_USAGE );
  exit(1);
}

static int error_reported = 0;

static void gethostbyaddr_communication_error(const cl_application_error_list_elem_t* commlib_error)
{
   if (commlib_error != NULL) {
      switch (commlib_error->cl_err_type) {
         case CL_LOG_ERROR: {
            if (commlib_error->cl_info != NULL) {
               fprintf(stderr, "%s: %s\n", cl_get_error_text(commlib_error->cl_error), commlib_error->cl_info);
            } else {
               fprintf(stderr, "%s\n", cl_get_error_text(commlib_error->cl_error));
            }
            error_reported = 1;
            break;
         }
         default: {
            break;
         }
      }
   }
}


int main(int argc, char *argv[])
{
  struct hostent *he = NULL;
  char* resolved_name = NULL;
  int retval = CL_RETVAL_OK;
  char* ip_string = NULL;
  char** tp = NULL;
  char** tp2 = NULL;
  int name_only = 0;
  int sge_aliasing = 0;
  int all_option = 0;
  int system_error = 0;
  struct in_addr addr;

  if (argc < 2) {
    usage();
  }

  ip_string = argv[1];
  if (!strcmp(argv[1], "-help")) {
     usage();
  }
  if (!strcmp(argv[1], "-name")) {
     if (argc != 3) {
        usage(); 
     }
     name_only = 1;
     ip_string = argv[2];
  }   
  if (!strcmp(argv[1], "-aname")) {
     if (argc != 3) {
        usage(); 
     }
     name_only = 1;
     sge_aliasing = 1;
     ip_string = argv[2];
  }   
  if (!strcmp(argv[1], "-all")) {
     if (argc != 3) {
        usage(); 
     }
     name_only = 0;
     sge_aliasing = 1;
     all_option = 1;
     ip_string = argv[2];
  }

  
  retval = cl_com_setup_commlib(CL_NO_THREAD ,CL_LOG_OFF, NULL);
  if (retval != CL_RETVAL_OK) {
     fprintf(stderr,"%s\n",cl_get_error_text(retval));
     exit(1);
  }

  if (sge_aliasing ) {
     cl_com_set_alias_file(sge_get_alias_path());
  }

  retval = cl_com_set_error_func(gethostbyaddr_communication_error);
  if (retval != CL_RETVAL_OK) {
     fprintf(stderr,"%s\n",cl_get_error_text(retval));
     exit(1);
  }
  
  addr.s_addr = inet_addr(ip_string);

  retval = cl_com_cached_gethostbyaddr(&addr, &resolved_name, &he, &system_error);
  if (retval != CL_RETVAL_OK) {
     cl_com_cleanup_commlib();
     if (error_reported == 0) {
        char* err_text = cl_com_get_h_error_string(system_error);

        if (err_text == NULL) {
           err_text = strdup(strerror(system_error));
           if (err_text == NULL) {
              err_text = strdup("unexpected error");
           }
        }
        if ( ip_string == NULL) {
            ip_string = "NULL";
        }

        fprintf(stderr,"error resolving ip "SFQ": %s (%s)\n",ip_string, cl_get_error_text(retval), err_text); 

        sge_free(&err_text); 
     }
     exit(1);
  }

  if (he == NULL) {
     fprintf(stderr,"%s\n","could not get hostent struct");
  }

  if (name_only) {
     if (sge_aliasing) {
        if (resolved_name != NULL) {
           printf("%s\n",resolved_name);
        } else {
           fprintf(stderr,"%s\n","unexpected error");
           exit(1);
        }
     } else {
        if (he != NULL) {
           printf("%s\n",he->h_name);
        } else {
           fprintf(stderr,"%s\n","could not get hostent struct");
           exit(1);
        }
     }
  } else {
     if (he != NULL) {
        printf(MSG_SYSTEM_HOSTNAMEIS_S,he->h_name);
        printf("\n");
     }
   
     if (resolved_name != NULL && all_option) {
        printf("SGE name: %s\n",resolved_name);
        sge_free(&resolved_name);
     } 
   
     if (he != NULL) {
        printf("%s", MSG_SYSTEM_ALIASES);
        for (tp = he->h_aliases; *tp; tp++)
          printf(" %s", *tp);
        printf("\n");
     
        printf("%s", MSG_SYSTEM_ADDRESSES);
        for (tp2 = he->h_addr_list; *tp2; tp2++)
           printf(" %s", inet_ntoa(* (struct in_addr *) *tp2));  /* inet_ntoa() is not MT safe */
        printf("\n");    
     }
  }
  sge_free_hostent(&he);
  if (resolved_name != NULL) {
     sge_free(&resolved_name);
  }
  retval = cl_com_cleanup_commlib();
  if (retval != CL_RETVAL_OK) {
     fprintf(stderr,"%s\n",cl_get_error_text(retval));
     exit(1);
  }
  return 0;
}
