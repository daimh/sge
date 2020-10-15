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
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "uti/sge_profiling.h"

#include "comm/lists/cl_lists.h"
#include "comm/cl_commlib.h"

#include "basis_types.h"

/* shutdown when test client can't connect for more than 15 min */
#define SGE_TEST_VIRTUAL_CLIENT_SHUTDOWN_TIMEOUT 15*60

/* counters */
static int snd_messages = 0;
static int events_received = 0;

void sighandler_client(int sig);
static int do_shutdown = 0;

static cl_com_handle_t* handle = NULL; 

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
  struct timeval now;
  time_t last_time = 0;
  time_t shutdown_time = 0;
  int i,first_message_sent = 0;
  int no_output = 0;
  cl_byte_t *reference = NULL;

  prof_mt_init();

  if (argc < 4) {
      printf("syntax: debug_level vmaster_port vmaster_host [no_output]\n");
      exit(1);
  }

  if (argc >= 5) {
     if (strcmp(argv[4],"no_output") == 0) {
        printf("virtual event client: no_output option set\n");
        no_output = 1;
     }
  }


  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  gettimeofday(&now,NULL);
  shutdown_time = now.tv_sec + SGE_TEST_VIRTUAL_CLIENT_SHUTDOWN_TIMEOUT;




  printf("startup commlib ...\n");
  cl_com_setup_commlib(CL_NO_THREAD , (cl_log_t)atoi(argv[1]), NULL);

  printf("setting up handle for connect port %d\n", atoi(argv[2]) );
  handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , false, atoi(argv[2]) , CL_TCP_DEFAULT,"virtual_event_client", 0, 1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }

  printf("local hostname is \"%s\"\n", handle->local->comp_host);
  printf("local component is \"%s\"\n", handle->local->comp_name);
  printf("local component id is \"%ld\"\n", handle->local->comp_id);

  cl_com_get_connect_port(handle, &i);
  printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[3]);

  printf("virtual event client is running ...\n");
  while(do_shutdown == 0) {
     int                retval  = 0;
     cl_com_message_t*  message = NULL;
     cl_com_endpoint_t* sender  = NULL;

     gettimeofday(&now,NULL);
     if (now.tv_sec != last_time && !no_output) {
        printf("virtual event client["pid_t_fmt"] message count[sent |%d|] events[received |%d|]...\n",
           getpid(), snd_messages, events_received);
        last_time = now.tv_sec;
     }

     if (now.tv_sec > shutdown_time ) {
        printf("shutting down test - timeout\n");
        do_shutdown = 1;
     }

     retval = cl_commlib_receive_message(handle, NULL, NULL, 0,      /* handle, comp_host, comp_name , comp_id, */
                                         true, 0,                 /* syncron, response_mid */
                                         &message, &sender );
     if ( retval != CL_RETVAL_OK) {
        if ( retval == CL_RETVAL_CONNECTION_NOT_FOUND ) {

#if 0
            printf("opening connection to %s/%s/%d\n", argv[3], "virtual_master", 1);
#endif


            /* shutdown when virtual qmaster is not running anymore */
            if (events_received > 0) {
               do_shutdown = 1;
               printf("lost connection to  %s/%s/%d\n", argv[3], "virtual_master", 1);
            }
            retval = cl_commlib_open_connection(handle, argv[3], "virtual_master", 1);
            if (retval == CL_RETVAL_OK) {
               first_message_sent = 0; 
            } 
        }
     } else {
#if 0
        printf("received message from %s/%s/%ld: \"%s\" (%ld bytes)\n", sender->comp_host,sender->comp_name,sender->comp_id, message->message, message->message_length);
#endif
        gettimeofday(&now,NULL);
        shutdown_time = now.tv_sec + SGE_TEST_VIRTUAL_CLIENT_SHUTDOWN_TIMEOUT;
        events_received++;
        cl_com_free_message(&message);
        cl_com_free_endpoint(&sender);
     }
#if 0
     printf("status: %s\n",cl_get_error_text(retval));
#endif

     if (  first_message_sent == 0) {
        char data[6] = "event";
        reference = (cl_byte_t *)data;
        first_message_sent = 1;
        retval = cl_commlib_send_message(handle, argv[3], "virtual_master", 1,
                                         CL_MIH_MAT_ACK,
                                         &reference, 6,
                                         NULL, 0, 0 , true, true );
        if (retval == CL_RETVAL_OK) {
           snd_messages++;
        }
#if 0
        printf("sending a event hello message: %s\n",cl_get_error_text(retval) );
#endif
     }
  }

  printf("shutdown commlib ...\n");
  cl_com_cleanup_commlib();

  printf("main done\n");
  return 0;
}

