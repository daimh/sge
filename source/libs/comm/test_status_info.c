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
#include <sys/time.h>

#include "comm/cl_commlib.h"
#include "comm/cl_connection_list.h"


#define CL_DO_SLOW 1
void sighandler_client(int sig);
static int do_shutdown = 0;


/*---------------------------------------------------------------*/
void sighandler_client(
int sig 
) {
/*   thread_signal_receiver = pthread_self(); */
   if (sig == SIGPIPE) {
      return;
   }

   if (sig == SIGHUP) {
      return;
   }
   printf("do_shutdown\n");
   /* shutdown all sockets */
   do_shutdown = 1;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  struct sigaction sa;
  int i;

  cl_com_handle_t* handle = NULL; 

  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  
  if (argc != 6) {
     printf("wrong parameters, param1 = server host, param2 = port number, param3 = comp name, param4 = comp_id, param5=debug_level\n");
     exit(1);
  }
  cl_com_setup_commlib(CL_RW_THREAD, (cl_log_t)atoi(argv[5]), NULL);

  handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , false, atoi(argv[2]) , CL_TCP_DEFAULT,"sim_client", 0, 1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }

  printf("local hostname is \"%s\"\n", handle->local->comp_host);
  printf("local component is \"%s\"\n", handle->local->comp_name);
  printf("local component id is \"%ld\"\n", handle->local->comp_id);
  cl_com_get_connect_port(handle, &i);
  printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[1]);


  while(do_shutdown != 1) {
     int ret;
     cl_com_SIRM_t* status = NULL;
     ret = cl_commlib_get_endpoint_status(handle,argv[1] , argv[3], atoi(argv[4]), &status);
     if (ret != CL_RETVAL_OK) {
        printf("cl_commlib_get_endpoint_status() returned %s\n", cl_get_error_text(ret));
     }
     if (status != NULL) {
        printf("endpoint is up since %ld seconds and has status %ld\n", status->runtime, status->application_status);
        cl_com_free_sirm_message(&status);
        break;
     }
  }
  printf("do_shutdown received\n");
  cl_commlib_shutdown_handle(handle, false);
  cl_com_cleanup_commlib();
  printf("main done\n");
  fflush(stdout);
  return 0;
}





