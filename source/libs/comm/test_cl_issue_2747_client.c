
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
#include <pwd.h>

#include "comm/cl_commlib.h"
#include "comm/lists/cl_log_list.h"
#include "comm/cl_endpoint_list.h"
#include "uti/sge_profiling.h"

void sighandler_server(int sig);
static int pipe_signal = 0;
static int hup_signal = 0;



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "sighandler_server()"
void sighandler_server(int sig) {
   if (sig == SIGPIPE) {
      pipe_signal = 1;
      return;
   }
   if (sig == SIGHUP) {
      hup_signal = 1;
      return;
   }
   cl_com_ignore_timeouts(true);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  struct sigaction sa;
  int exit_state = 0;
  int ret_val = CL_RETVAL_OK;
  cl_thread_mode_t thread_mode = CL_RW_THREAD;
  int handle_port = 0;
  cl_com_handle_t* handle = NULL; 
  cl_com_message_t* message = NULL;
  cl_com_endpoint_t* sender = NULL;
  char* server_host = NULL;
  int server_port = -1;
  int i;
  char message_text[] = "blub";
  cl_byte_t* message_text_pointer = (cl_byte_t*)message_text;


  if (getenv("CL_PORT")) {
     handle_port = atoi(getenv("CL_PORT"));
  }

  if (argc != 3) {
      printf("Usage: test_cl_issue_2747_client SERVER PORT\n");
      printf("  SERVER: host where test_cl_issue_2747_server is running\n");
      printf("  PORT: port where test_cl_issue_2747_server is running\n\n");
      printf("Use CL_PORT env to configure fixed server port.\n");
      printf("Use CL_THREADS env with value \"true\" or \"false\" to enable/disable commlib threads\n");
      printf("Use SGE_COMMLIB_DEBUG env to configure debug level.\n");
      exit(1);
  }

  server_port = atoi(argv[2]);
  server_host = argv[1];

  prof_mt_init(); 

  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_server;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);


  printf("commlib setup ...\n");
  /* this is for compiler warning on irix65 */
  if (pipe_signal) {
     printf("pipe\n");
  }
  if (hup_signal) {
     printf("hup\n");
  };

  if (getenv("CL_THREADS") != NULL) {
     if (strcasecmp(getenv("CL_THREADS"),"false") == 0) {
        thread_mode = CL_NO_THREAD;
     }
  }

  if (thread_mode == CL_NO_THREAD) {
     printf("INFO: commlib threads are disabled\n");
  } else {
     printf("INFO: commlib threads are enabled\n");
  }


  cl_com_setup_commlib(thread_mode, CL_LOG_OFF, NULL);


  handle=cl_com_create_handle(NULL, CL_CT_TCP, CL_CM_CT_MESSAGE, true, handle_port, CL_TCP_DEFAULT, "client", 1, 1, 0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     cl_com_cleanup_commlib();
     exit(-1);
  }

  cl_com_get_service_port(handle,&i), 
  printf("server running on host \"%s\", port %d, component name is \"%s\", id is %ld\n", 
         handle->local->comp_host, 
         i, 
         handle->local->comp_name,  
         handle->local->comp_id);

  {
     char* resolved_server_host = NULL;
     cl_com_cached_gethostbyname(server_host, &resolved_server_host, NULL, NULL, NULL);
     printf("server will connect to server on host \"%s\" bound to port \"%d\"\n", resolved_server_host , server_port);
     sge_free(&resolved_server_host);
  }


  cl_com_append_known_endpoint_from_name(server_host, "server", 1, server_port, CL_CM_AC_ENABLED, true); 

  /* send 100 messages to server. Server will respond them all */
  printf("sending 10 messages to server ...\n");
  for (i=1; i<=10; i++) {
     ret_val = cl_commlib_send_message(handle,
                                       server_host, "server", 1,
                                       CL_MIH_MAT_NAK,  
                                       &message_text_pointer, 5, 
                                       NULL, 0, 0, 
                                       true, false);
     if (ret_val != CL_RETVAL_OK) {
        exit_state = 1;
     }
     printf("cl_commlib_send_message (nr: %d): %s\n", i, cl_get_error_text(ret_val));
  }

  printf("receiving one message from server ...\n");
  /* we receive first syncron message response from server */
  ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, true, 1, &message, &sender);
  if (message != NULL) {
     printf("received message from \"%s/%s/%ld\" (%s)\n", sender->comp_host, sender->comp_name, sender->comp_id,  cl_get_error_text(ret_val));
     cl_com_free_message(&message);
     cl_com_free_endpoint(&sender);
  } else {
     printf("cl_commlib_receive_message: %s\n", cl_get_error_text(ret_val));
     exit_state = 2;
  }

  printf("triggering commlib ...\n");
  cl_commlib_trigger(handle, 1);

  printf("sending another 90 messages to server ...\n");
  for (i=1; i<=90; i++) {
     ret_val = cl_commlib_send_message(handle,
                                       server_host, "server", 1,
                                       CL_MIH_MAT_NAK,  
                                       &message_text_pointer, 5, 
                                       NULL, 0, 0, 
                                       true, false);
     if (ret_val != CL_RETVAL_OK) {
        exit_state = 1;
     }
     printf("cl_commlib_send_message (nr: %d): %s\n", i, cl_get_error_text(ret_val));
  }

  printf("commlib cleanup ...\n");
  cl_com_cleanup_commlib();

  if (exit_state != 0) {
     printf("test FAILED!\n");
  } else {
     printf("test OK!\n");
  }
  fflush(stdout);
  return exit_state;
}
