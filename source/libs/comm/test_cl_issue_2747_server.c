
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
static int do_shutdown = 0;


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
   do_shutdown = 1;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  struct sigaction sa;
  static int runs = 100;
  int message_counter = 0;
  struct timeval now;
  time_t shutdown_time = 0;
  int timeout_error = 0;
  cl_thread_mode_t thread_mode = CL_RW_THREAD;

  int handle_port = 0;
  cl_com_handle_t* handle = NULL; 
  cl_com_message_t* message = NULL;
  cl_com_endpoint_t* sender = NULL;
  int i;

  if (getenv("CL_PORT")) {
     handle_port = atoi(getenv("CL_PORT"));
  }

  if (argc != 1) {
      printf("Usage: test_cl_issue_2747_server - no parameters supported!\n\n");
      printf("Use CL_PORT env to configure fixed server port.\n");
      printf("Use CL_THREADS env with value \"true\" or \"false\" to enable/disable commlib threads\n");
      printf("Use SGE_COMMLIB_DEBUG env to configure debug level.\n");
      printf("Use CL_RUNS to limit main loop runs.\n");
      exit(1);
  }

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
     if (strcasecmp(getenv("CL_THREADS"), "false") == 0) {
        thread_mode = CL_NO_THREAD;
     }
  }

  if (thread_mode == CL_NO_THREAD) {
     printf("INFO: commlib threads are disabled\n");
  } else {
     printf("INFO: commlib threads are enabled\n");
  }

  cl_com_setup_commlib(thread_mode, CL_LOG_OFF, NULL);


  handle=cl_com_create_handle(NULL, CL_CT_TCP, CL_CM_CT_MESSAGE, true, handle_port, CL_TCP_DEFAULT, "server", 1, 1, 0 );
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

  if (getenv("CL_RUNS") != NULL) { 
     runs = atoi(getenv("CL_RUNS"));
  }

  gettimeofday(&now,NULL);
  shutdown_time = now.tv_sec + 90;
  while (do_shutdown != 1) {
     int ret_val;
     CL_LOG(CL_LOG_INFO,"main()");
     cl_commlib_trigger(handle, 1); 

     gettimeofday(&now,NULL);
     if (now.tv_sec >= shutdown_time) {
        do_shutdown = 1;
        timeout_error = 1;
     }

     if (message_counter > 10) {
        int test_client_connected = 0;
        cl_connection_list_elem_t* elem = NULL;
        pthread_mutex_lock(handle->connection_list_mutex);
        cl_raw_list_lock(handle->connection_list);
        elem = cl_connection_list_get_first_elem(handle->connection_list);
        while(elem) {
           if (elem->connection->remote != NULL && 
               elem->connection->remote->comp_name != NULL ) {
              if (strcmp(elem->connection->remote->comp_name, "client") == 0) {
                 test_client_connected = 1;
                 break;
              }
           }
           elem = cl_connection_list_get_next_elem(elem);
        }
        cl_raw_list_unlock(handle->connection_list);
        pthread_mutex_unlock(handle->connection_list_mutex);
        if (test_client_connected == 0) {
           do_shutdown = 1;
        }
     }

     if (runs<= 0) {
        do_shutdown = 1;
     }

     ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
     CL_LOG_STR(CL_LOG_INFO,"cl_commlib_receive_message() returned",cl_get_error_text(ret_val));
     if (message != NULL) {
        message_counter++;
        if (getenv("CL_RUNS")) { 
           printf("runs: %d\n", runs);
           runs--;
        }

        printf("received message from \"%s/%s/%ld\"\n", sender->comp_host, sender->comp_name, sender->comp_id);
        ret_val = cl_commlib_send_message(handle,
                                          sender->comp_host, sender->comp_name, sender->comp_id,
                                          CL_MIH_MAT_NAK,  
                                          &message->message, message->message_length, 
                                          NULL, message->message_id,0, 
                                          false, false);
        if (ret_val != CL_RETVAL_OK) {
           CL_LOG_INT(CL_LOG_ERROR,"sent message response for message id", (int)message->message_id);
           CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_send_message() returned:",cl_get_error_text(ret_val));
        } else {
           CL_LOG_INT(CL_LOG_INFO,"sent message response for message id", (int)message->message_id);
        }
        cl_com_free_message(&message);
        cl_com_free_endpoint(&sender);
        message = NULL;
     } 
  }

  /* add a flush time for the qping client */
  gettimeofday(&now,NULL);
  shutdown_time = now.tv_sec + 5;
  while ( now.tv_sec < shutdown_time ) {
     cl_commlib_trigger(handle, 1); 
     gettimeofday(&now,NULL);
  }

  printf("shutting down server ...\n");
  while ( cl_commlib_shutdown_handle(handle, true) == CL_RETVAL_MESSAGE_IN_BUFFER) {
     message = NULL;
     cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);

     if (message != NULL) {
        printf("ignoring message from \"%s\"\n", sender->comp_host); 
        cl_com_free_message(&message);
        cl_com_free_endpoint(&sender);
        message = NULL;
     }
  }

  printf("commlib cleanup ...\n");
  cl_com_cleanup_commlib();
  fflush(stdout);

  printf("main done\n");
  printf("messages received in main loop: %d\n", message_counter);
  fflush(stdout);
  if (message_counter != 100) {
     printf("error: message counter is not 100\n");
     return 1;
  }
  if (timeout_error != 0) {
     printf("error: timeout error for shutdown\n");
     return 2;
  }
  return 0;
}
