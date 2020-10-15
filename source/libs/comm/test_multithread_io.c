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

#include "uti/sge_language.h"

#include "comm/lists/cl_lists.h"
#include "comm/cl_commlib.h"

void sighandler_client(int sig);
static int do_shutdown = 0;

static cl_com_handle_t* handle = NULL; 


cl_raw_list_t* thread_list = NULL;

void *my_multi_thread(void *t_conf);
void *my_multi_read_thread(void *t_conf);

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
extern int main(void)
{
  cl_thread_settings_t* thread_p = NULL;
  cl_thread_settings_t* dummy_thread_p = NULL;
  struct sigaction sa;

  
  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  /* This module test is defect */
  printf("This test does not work!\n");
  exit(1);

  cl_com_setup_commlib(CL_RW_THREAD ,CL_LOG_WARNING, NULL);
  handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , false, 5000 , CL_TCP_DEFAULT,"client", 0, 1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }

  /* setup thread list */
  cl_thread_list_setup(&thread_list,"thread list");

  /* setup first thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "1st thread", 1, my_multi_thread, NULL, NULL, CL_TT_USER1);

#if 1
  /* setup second thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "2nd thread", 2, my_multi_thread, NULL, NULL, CL_TT_USER1);  

  /* setup third thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "3nd thread", 3, my_multi_read_thread, NULL, NULL, CL_TT_USER1); 
#endif

  while(do_shutdown == 0) {
     cl_com_message_t* message = NULL;
     cl_com_endpoint_t* sender = NULL;
     int retval;

     /* synchron message receive */
     retval = cl_commlib_receive_message(handle,NULL, NULL, 0, true, 0, &message, &sender);
     if (retval != CL_RETVAL_OK) {
        CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_receive_message:",cl_get_error_text(retval));
        retval = cl_commlib_open_connection(handle, "es-ergb01-01", "server", 1);
        if (retval != CL_RETVAL_OK) {
           CL_LOG_STR(CL_LOG_ERROR, "cl_commlib_open_connection:",cl_get_error_text(retval));
        }

     } else {
        if (message != NULL) {
           CL_LOG_STR(CL_LOG_INFO,"received message:",(char*)message->message );
           printf("received message from \"%s\": \"%s\"\n", sender->comp_host,message->message);
        }
        cl_com_free_endpoint(&sender);
        cl_com_free_message(&message);
     }
  }

  /* delete all threads */
  while ( (thread_p=cl_thread_list_get_first_thread(thread_list)) != NULL ) {
     int id = thread_p->thread_id;
     printf("-----------> delete thread %d ...\n",id );

     cl_thread_list_delete_thread(thread_list, thread_p);
     printf("<----------- delete thread %d done\n",id );

  }

  cl_thread_list_cleanup(&thread_list);
  while (cl_commlib_shutdown_handle(handle, true) == CL_RETVAL_MESSAGE_IN_BUFFER) {
     cl_com_message_t* message = NULL;
     cl_com_endpoint_t* sender = NULL;

     printf("got message\n");

     cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
     if (message != NULL) {
        cl_com_free_endpoint(&sender);
        cl_com_free_message(&message);
     }
  } 
  cl_com_cleanup_commlib();

  printf("main done\n");
  return 0;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_multi_thread"
void *my_multi_thread(void *t_conf) {
   int do_exit = 0;
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

   /* ok, thread main */
   while (do_exit == 0) {
      char message[255];
      unsigned long message_length = 0;
      unsigned long mid = 0;
      int ret_val = 0;
      CL_LOG(CL_LOG_INFO,"test cancel ...");
      cl_thread_func_testcancel(thread_config);

      CL_LOG_INT(CL_LOG_INFO,"thread running:", thread_config->thread_id);

      sprintf(message,"This message is from %s", thread_config->thread_name);
      message_length = strlen(message) + 1;
      ret_val = cl_commlib_send_message(handle, "es-ergb01-01", "server", 1, CL_MIH_MAT_ACK, (cl_byte_t**)&message ,message_length , &mid , 0, 0, true, true);
      if (ret_val != CL_RETVAL_OK) {
         CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_send_message() returned", cl_get_error_text(ret_val));
      } 
      CL_LOG_INT(CL_LOG_INFO,"message has mid", (int)mid);
   }

   CL_LOG(CL_LOG_INFO, "exiting ...");
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_multi_read_thread()"
void *my_multi_read_thread(void *t_conf) {
   int do_exit = 0;
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

   /* ok, thread main */
   while (do_exit == 0) {
      cl_com_message_t* message = NULL;
      cl_com_endpoint_t* sender = NULL;
      int retval;

      CL_LOG(CL_LOG_INFO,"test cancel ...");
      cl_thread_func_testcancel(thread_config);

      CL_LOG_INT(CL_LOG_INFO,"thread running:", thread_config->thread_id);


     /* synchron message receive */
/* TODO For an application which more than one thread for cl_commlib_receive_message() calls it is
        a better solution to exactly trigger the thread which want's to receive a message.

        The implemented thread handling will always do a broadcast when a new message arives. The application
        should only use one thread for cl_commlib_receive_message() calls for a better performance. 
*/
     retval = cl_commlib_receive_message(handle,NULL, NULL, 0, true, 0, &message, &sender);
     if (retval != CL_RETVAL_OK) {
        CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_receive_message:",cl_get_error_text(retval));
        retval = cl_commlib_open_connection(handle, "es-ergb01-01", "server", 1);
        if (retval != CL_RETVAL_OK) {
           CL_LOG_STR(CL_LOG_ERROR, "cl_commlib_open_connection:",cl_get_error_text(retval));
        }

     } else {
        if (message != NULL) {
           CL_LOG_STR(CL_LOG_INFO,"received message:",(char*)message->message );
/*
           printf("received message from \"%s\": \"%s\"\n", sender->comp_host,message->message);
*/
        }
        cl_com_free_endpoint(&sender);
        cl_com_free_message(&message);
     }

#if 0
      CL_LOG(CL_LOG_INFO,"wait for event ...");
      if ((ret_val = cl_thread_wait_for_event(thread_config,0,thread_config->thread_id*1000*1000 )) != CL_RETVAL_OK) {  /* nothing to do sleep 1 sec */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               CL_LOG(CL_LOG_INFO,"condition wait timeout");
               break;
            default:
               CL_LOG_STR( CL_LOG_INFO, ">got error<: ", cl_get_error_text(ret_val));
               do_exit = 1;
         }
      }
#endif
   }

   CL_LOG(CL_LOG_INFO, "exiting ...");
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


