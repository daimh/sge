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

#include "comm/lists/cl_lists.h"
#include "comm/lists/cl_thread_list.h"
#include "comm/cl_commlib.h"

#include "uti/sge_profiling.h"

/* some global things */
int test_server_port = 0;
char* test_server_host = NULL;

void sighandler_client(int sig);
static int do_shutdown = 0;

cl_raw_list_t* thread_list = NULL;

void *my_receive_thread(void *t_conf);
void *my_sender_thread(void *t_conf);

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

/* TODO: This should also work */
/*   cl_com_ignore_timeouts(true); */

   do_shutdown = 1;
}

void my_cleanup_func(cl_thread_settings_t* thread_config) {
   cl_com_handle_t* handle = NULL; 
   printf("my thread cleanup called: thread state is: %s\n", cl_thread_get_state(thread_config));
   handle = (cl_com_handle_t*) thread_config->thread_user_data;
   if (handle != NULL) {
      printf("shutting down handle %s/%s/%ld ...\n", handle->local->comp_host, handle->local->comp_name, handle->local->comp_id);
      cl_commlib_shutdown_handle(handle, false); 
   }
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  cl_thread_settings_t* thread_p = NULL;
  cl_thread_settings_t* dummy_thread_p = NULL;
  cl_thread_settings_t* sender_thread = NULL;

  struct sigaction sa;
  cl_com_handle_t* handle = NULL; 
#define TEST_RECEIVER_COUNT 32
  cl_com_endpoint_t* receiver_list[TEST_RECEIVER_COUNT];
  int i;
  int endpoint_index = 0;
  int port = 0;
  struct timeval now;
  struct timeval last_now;
  int next_calc = 0;
  unsigned long message_counter = 0;
  cl_thread_mode_t tmode = CL_NO_THREAD;
  int arg_found = 0;

  prof_mt_init();

  printf("This module test does not work!\n");  
  exit(1);
  if (argc != 2) {
     printf("usage: test_thread_throughput <thread mode>\n");
     printf("<thread mode> = none|rw|pool\n");
     exit(1);
  }

  if (strcmp(argv[1],"none") == 0) {
     tmode = CL_NO_THREAD;
     arg_found = 1;
  } 
  if (strcmp(argv[1],"rw") == 0) {
     tmode = CL_RW_THREAD;
     arg_found = 1;
  } 
#if 0
  if (strcmp(argv[1],"pool") == 0) {
     tmode = CL_THREAD_POOL;
     arg_found = 1;
  }
#endif
  if ( arg_found == 0) {
     printf("usage: test_thread_throughput <thread mode>\n");
     printf("<thread mode> = none|rw|pool\n");
     exit(1);
  }


  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  

  cl_com_setup_commlib(tmode /* CL_NO_THREAD*/ /* CL_RW_THREAD */ /*  CL_THREAD_POOL */,CL_LOG_OFF, NULL);

  if (getenv("CL_PORT") != NULL) {
     port =atoi(getenv("CL_PORT"));
  }  

  handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , true, port , CL_TCP_DEFAULT,"server", 1, 1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }

  /* setup global data before starting any other thread */
  cl_com_get_service_port(handle, &test_server_port);
  printf("application running on port %d\n", test_server_port);
  
  cl_com_gethostname(&test_server_host, NULL, NULL, NULL);
  printf("application running on host %s\n", test_server_host);

  /* setup thread list */
  cl_thread_list_setup(&thread_list,"thread list");

  for (i=1;i<=TEST_RECEIVER_COUNT;i++) {
     printf("starting receiver thread %i\n", i);
     cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "receiver", i, my_receive_thread, my_cleanup_func, NULL, CL_TT_USER1);
  }
  cl_thread_list_create_thread(thread_list, &sender_thread,cl_com_get_log_list(), "sender1", 1, my_sender_thread, my_cleanup_func, NULL, CL_TT_USER1);
  cl_thread_list_create_thread(thread_list, &sender_thread,cl_com_get_log_list(), "sender2", 2, my_sender_thread, my_cleanup_func, NULL, CL_TT_USER1);

  

  /* main thread = application thread */
  while(do_shutdown == 0) {
     cl_com_message_t* message = NULL;
     cl_com_endpoint_t* sender = NULL;
     
     gettimeofday(&now,NULL);
     if (now.tv_sec >= next_calc) {
        double now_time = 0.0;
        double last_time = 0.0;
        double time_range = 0.0;
        double messages_per_second = 0.0;
        now_time  = now.tv_sec * 1000000.0 + now.tv_usec;
        last_time = last_now.tv_sec * 1000000.0 + last_now.tv_usec;

        time_range = (now_time - last_time) / 1000000.0;
        if (time_range > 0.0) {
           messages_per_second = message_counter / time_range;
        }
        message_counter = 0;

        printf("\nmaster thread: %.3f [messages/s]\n", messages_per_second);

        gettimeofday(&last_now,NULL);
        next_calc = now.tv_sec + 5;
     }

     /* synchron message receive */
     cl_commlib_receive_message(handle,NULL, NULL, 0, true, 0, &message, &sender);

     if (message != NULL) {
        int is_route_message = 1;
        cl_com_endpoint_t* client = NULL;

        if (strcmp((char*)message->message, "announce") == 0) {
           printf("server: received announce message from %s/%s/%ld\n", sender->comp_host, sender->comp_name, sender->comp_id);
           receiver_list[endpoint_index] = sender;
           sender = NULL;
           endpoint_index++;
           is_route_message = 0;
        }
        if ((sender!=NULL) &&strcmp((char*)sender->comp_name, "client") == 0) {
           client = sender;
           cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id, 
                                      CL_MIH_MAT_NAK, &message->message, message->message_length,
                                      NULL, message->message_id, 0,
                                      true, false);
           is_route_message = 0;
        }

        if (is_route_message) {

           for (i=0;i< endpoint_index;i++) {
              client = receiver_list[i];
              message_counter++;

              cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id, 
                                      CL_MIH_MAT_NAK, &message->message, message->message_length,
                                      NULL, 0, 0,
                                      true, false);
              cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id, 
                                      CL_MIH_MAT_NAK, &message->message, message->message_length,
                                      NULL, 0, 0,
                                      true, false);
              cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id, 
                                      CL_MIH_MAT_NAK, &message->message, message->message_length,
                                      NULL, 0, 0,
                                      true, false);
              cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id, 
                                      CL_MIH_MAT_NAK, &message->message, message->message_length,
                                      NULL, 0, 0,
                                      true, false);
           }

        }
     }
     cl_com_free_endpoint(&sender);
     cl_com_free_message(&message);
  }

/* TODO: Fix this shutdown problem */
  exit(1);

  cl_thread_list_delete_thread(thread_list, sender_thread);

  for (i=1;i<=TEST_RECEIVER_COUNT;i++) {
     thread_p = cl_thread_list_get_thread_by_id(thread_list,i);
     cl_thread_shutdown(thread_p);
  }

  /* delete all threads */
  while ( (thread_p=cl_thread_list_get_first_thread(thread_list)) != NULL ) {
     cl_thread_list_delete_thread(thread_list, thread_p);
  }

  cl_thread_list_cleanup(&thread_list);

  sleep(2); /* flushing debug clients */
  cl_com_cleanup_commlib();

  printf("main done\n");
  sge_prof_cleanup();
  return 0;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_receive_thread"
void *my_receive_thread(void *t_conf) {
   int do_exit = 0;
   char* message = "announce";
   cl_com_handle_t* handle = NULL; 
   struct timeval now;
   struct timeval last_now;
   int next_calc = 0;
   unsigned long message_counter = 0;


   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 

   /* set thread config data */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      CL_LOG(CL_LOG_ERROR,"thread setup error"); 
      do_exit = 1;
   }

   /* setup thread */
   CL_LOG(CL_LOG_INFO, "starting initialization ...");

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);
   CL_LOG(CL_LOG_INFO, "starting main loop ...");

   handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , false, test_server_port, CL_TCP_DEFAULT,"receiver", thread_config->thread_id, 1,0 );
   if (handle == NULL) {
      printf("receiver: could not get handle\n");
      do_exit = 1;
   }

   cl_com_set_synchron_receive_timeout(handle, 1);

   thread_config->thread_user_data = (void*)handle;

   cl_commlib_send_message(handle, test_server_host, "server", 1, 
                           CL_MIH_MAT_NAK, (cl_byte_t**)&message, strlen(message) + 1,
                           NULL, 0, 0,
                           true, false);


   /* ok, thread main */
   while (do_exit == 0) {
      cl_com_message_t* message = NULL;
      cl_com_endpoint_t* sender = NULL;

      cl_thread_func_testcancel(thread_config);
 
      gettimeofday(&now,NULL);
      if (now.tv_sec >= next_calc) {
         double now_time = 0.0;
         double last_time = 0.0;
         double time_range = 0.0;
         double messages_per_second = 0.0;
         now_time  = now.tv_sec * 1000000.0 + now.tv_usec;
         last_time = last_now.tv_sec * 1000000.0 + last_now.tv_usec;

         time_range = (now_time - last_time) / 1000000.0;
         if (time_range > 0.0) {
            messages_per_second = message_counter / time_range;
         }
         message_counter = 0;

         if (thread_config->thread_id == 1) {
            printf("receiver thread (%d): %.3f [messages/s]\n", thread_config->thread_id, messages_per_second);
         }

         gettimeofday(&last_now,NULL);
         next_calc = now.tv_sec + 5;
      }
     
      cl_commlib_receive_message(handle,
                                 NULL, NULL, 0, 
                                 true,
                                 0,
                                 &message,&sender);
      if (message != NULL) {
         /* printf("receiver: received message: \"%s\"\n", message->message); */ 
         message_counter++;
      }
      cl_com_free_endpoint(&sender);
      cl_com_free_message(&message);
   }

   CL_LOG(CL_LOG_INFO, "exiting ...");
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_sender_thread()"
void *my_sender_thread(void *t_conf) {
   int do_exit = 0;
   cl_com_handle_t* handle = NULL; 
   struct timeval now;
   struct timeval last_now;
   int next_calc = 0;
   unsigned long message_counter = 0;


   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 

   /* set thread config data */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      CL_LOG(CL_LOG_ERROR,"thread setup error"); 
      do_exit = 1;
   }

   /* setup thread */
   CL_LOG(CL_LOG_INFO, "starting initialization ...");

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);



   handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , false, test_server_port, CL_TCP_DEFAULT,"sender", thread_config->thread_id, 1,0 );
   if (handle == NULL) {
      printf("sender: could not get handle\n");
      do_exit = 1;
   }

   cl_com_set_synchron_receive_timeout(handle, 1);

   thread_config->thread_user_data = (void*)handle;

   CL_LOG(CL_LOG_INFO, "starting main loop ...");
   /* ok, thread main */
   while (do_exit == 0) {
      char message[12048];
      int i;


      message[0] = 0;
      cl_thread_func_testcancel(thread_config);

      gettimeofday(&now,NULL);
      if (now.tv_sec >= next_calc) {
         double now_time = 0.0;
         double last_time = 0.0;
         double time_range = 0.0;
         double messages_per_second = 0.0;
         now_time  = now.tv_sec * 1000000.0 + now.tv_usec;
         last_time = last_now.tv_sec * 1000000.0 + last_now.tv_usec;

         time_range = (now_time - last_time) / 1000000.0;
         if (time_range > 0.0) {
            messages_per_second = message_counter / time_range;
         }
         message_counter = 0;

         printf("send thread: %.3f [messages/s]\n", messages_per_second);

         gettimeofday(&last_now,NULL);
         next_calc = now.tv_sec + 5;
      }
      
      for (i=0;i<24;i++) {
         cl_commlib_send_message(handle, test_server_host, "server", 1, 
                              CL_MIH_MAT_ACK, (cl_byte_t**)&message, 12048,
                              NULL, 0, 0,
                              true, true);
         message_counter++;
      }

      cl_thread_wait_for_event(thread_config,2, 0 );
   }
   CL_LOG(CL_LOG_INFO, "exiting ...");
   /* call my cleanup func */
   my_cleanup_func(thread_config);
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


