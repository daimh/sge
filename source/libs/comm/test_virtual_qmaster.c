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
#include "comm/cl_commlib.h"

#include "uti/sge_profiling.h"

#define DATA_SIZE 5000

void sighandler_client(int sig);
static int do_shutdown = 0;

/* counters */

static int rcv_messages = 0;
static int snd_messages = 0;
static int evc_count = 0;
static int events_sent = 0;

static cl_com_handle_t* handle = NULL; 
cl_raw_list_t* thread_list = NULL;

#define MAX_EVENT_CLIENTS 1000
cl_com_endpoint_t* event_client_array[MAX_EVENT_CLIENTS];

void *my_message_thread(void *t_conf);

void do_nothing(void) {
   char help[255];

   sprintf(help,"hallo");
}

void *my_event_thread(void *t_conf);


unsigned long my_application_status(char** info_message) {
   if ( info_message != NULL ) {
      *info_message = strdup("not specified (state 1)");
   }
   return (unsigned long)1;
}

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
   /* shutdown all sockets */
   do_shutdown = 1;
}




#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  cl_thread_settings_t* thread_p = NULL;
  cl_thread_settings_t* dummy_thread_p = NULL;
  struct sigaction sa;
  struct timeval now;
  struct timeval last;
  double usec_last = 0.0;
  int i;
  int time_interval = 0;

  prof_mt_init();

  if (argc != 4) {
      
      printf("syntax: test_virtual_qmaster DEBUG_LEVEL PORT INTERVAL\n");
      exit(1);
  }

  time_interval = atoi(argv[3]);
  
  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  printf("startup commlib ...\n");
  cl_com_setup_commlib(CL_RW_THREAD /* CL_THREAD_POOL */ , (cl_log_t)atoi(argv[1]), NULL);
  cl_com_set_status_func(my_application_status); 

  printf("setting up service on port %d\n", atoi(argv[2]) );
  handle=cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , true, atoi(argv[2]) , CL_TCP_DEFAULT,"virtual_master", 1 , 1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }

  cl_com_get_service_port(handle,&i), 
  printf("server running on host \"%s\", port %d, component name is \"%s\", id is %ld\n", 
         handle->local->comp_host, 
         i, 
         handle->local->comp_name,  
         handle->local->comp_id);

  printf("create application threads ...\n");
  cl_thread_list_setup(&thread_list,"thread list");
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "message_thread_1", 100, my_message_thread, NULL, NULL, CL_TT_USER1);
#if 1
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "message_thread_2", 101, my_message_thread, NULL, NULL, CL_TT_USER1);
#endif
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,cl_com_get_log_list(), "event_thread", 3, my_event_thread, NULL, NULL, CL_TT_USER1);

  gettimeofday(&last,NULL);
  usec_last = (last.tv_sec * 1000000.0) + last.tv_usec;
  
  while(do_shutdown == 0 ) {
     double usec_now  = 0.0;
     double interval  = 0.0;
     double rcv_m_sec = 0.0;
     double snd_m_sec = 0.0;
     double nr_evc_sec = 0.0;
     double snd_ev_sec = 0.0;
     cl_com_handle_statistic_t* statistic_data = NULL;
     int unread_msg = 0;
     int unsend_msg = 0;
     int nr_of_connections = 0;

     gettimeofday(&now,NULL);
     usec_now  = (now.tv_sec  * 1000000.0) + now.tv_usec;
     
     interval = usec_now - usec_last;
     interval /= 1000000.0;

     if (interval > 0.0) {
        rcv_m_sec  = rcv_messages / interval;
        snd_m_sec  = snd_messages / interval;
        nr_evc_sec = evc_count    / interval;
        snd_ev_sec = events_sent  / interval;
     }

     cl_com_get_actual_statistic_data(handle, &statistic_data);
     if (statistic_data != NULL) {
        unread_msg = (int)statistic_data->unread_message_count;
        unsend_msg = (int)statistic_data->unsend_message_count;
        nr_of_connections = (int)statistic_data->nr_of_connections;
        cl_com_free_handle_statistic(&statistic_data);
     }
     printf("|%.5f|[s] received|%d|%.3f|[nr.|1/s] sent|%d|%.3f|[nr.|1/s] event clients|%d|%.3f|[nr.|1/s] events sent|%d|%.3f|[nr.|1/s] rcv_buf|%d|snd_buf|%d| nr_connections|%d|\n", 
            interval,
            rcv_messages, rcv_m_sec, 
            snd_messages, snd_m_sec,
            evc_count,    nr_evc_sec,
            events_sent,  snd_ev_sec,
            unread_msg,
            unsend_msg,
            nr_of_connections);
     fflush(stdout);

     cl_thread_wait_for_event(cl_thread_get_thread_config(),1,0);
     if ( interval >= time_interval ) {
        break;
     }
  }
  printf("shutdown threads ...\n");
  /* delete all threads */
  while ( (thread_p=cl_thread_list_get_first_thread(thread_list)) != NULL ) {
     gettimeofday(&now,NULL);
     printf("shutting down thread %s (%ld)...", thread_p->thread_name, (unsigned long) now.tv_sec);
     fflush(stdout);
     cl_thread_list_delete_thread(thread_list, thread_p);
     gettimeofday(&now,NULL);
     printf(" done (%ld)\n", (unsigned long)now.tv_sec);
  }
  cl_thread_list_cleanup(&thread_list);

  cl_com_ignore_timeouts(true);

  printf("shutdown commlib ...\n");
  cl_com_cleanup_commlib();
  fflush(stdout);
  printf("main done\n");
  return 0;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_message_thread"
void *my_message_thread(void *t_conf) {
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

   printf(" \"%s\" -> running ...\n", thread_config->thread_name );
   rcv_messages = 0;
   snd_messages = 0;
   evc_count    = 0;
   

   /* ok, thread main */
   while (do_exit == 0) {
      int ret_val = 0;
      cl_com_message_t*  message = NULL;
      cl_com_endpoint_t* sender  = NULL;

      cl_thread_func_testcancel(thread_config);

      cl_commlib_trigger(handle, 1);
      ret_val = cl_commlib_receive_message(handle, NULL, NULL, 0,      /* handle, comp_host, comp_name , comp_id, */
                                           false, 0,                       /* syncron, response_mid */
                                           &message, &sender);
      if (ret_val == CL_RETVAL_OK) {
         rcv_messages++;

         if (strcmp((char*)message->message,"event") == 0) {
            /* This is a event client */
            int i,help;
            cl_com_free_message(&message);
            help = 0;
            for (i=0;i<MAX_EVENT_CLIENTS;i++) {
               if ( event_client_array[i] == NULL ) {
                  event_client_array[i] = sender;
                  evc_count++;
                  help=1;
                  break;
               }
            }
            if (help != 1) {
               printf(" \"%s\" -> to much connected event clients\n", thread_config->thread_name);
               cl_com_free_endpoint(&sender);
            } 
         } else {
            /* no event client, just return message to sender */
            ret_val = cl_commlib_send_message(handle, sender->comp_host, sender->comp_name, sender->comp_id,
                                      CL_MIH_MAT_NAK,
                                      &(message->message), message->message_length,
                                      NULL, 0, 0 , false, false);
            if (ret_val == CL_RETVAL_OK) {
               snd_messages++;
            }
            cl_com_free_message(&message);
            cl_com_free_endpoint(&sender);
         }
      }
   }

   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_event_thread()"
void *my_event_thread(void *t_conf) {
   int do_exit = 0;
   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 
   cl_byte_t *reference = NULL;

   /* set thread config data */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      CL_LOG(CL_LOG_ERROR,"thread setup error"); 
      do_exit = 1;
   }

   /* setup thread */

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);
   CL_LOG(CL_LOG_INFO, "starting main loop ...");
   printf(" \"%s\" -> running ...\n", thread_config->thread_name );

   
   /* ok, thread main */
   while (do_exit == 0) {
      int ret_val;
      int i,first,nr;
      static int event_nr = 0;
      

      cl_thread_func_testcancel(thread_config);

      /* this should be 60 events/second per event client*/
      for(nr=0;nr<60;nr++) {
         first = 0;
         for (i=0;i<MAX_EVENT_CLIENTS;i++) {
            cl_com_endpoint_t* client = event_client_array[i];
            if ( client != NULL) {
               char help[DATA_SIZE];
               memset(help, 0, DATA_SIZE);
   
               if (first == 0) {
                  event_nr++;
                  first = 1;
               }
               sprintf(help,"event nr.: %d", event_nr );
#if 0
               printf(" \"%s\" -> sending event to %s/%s/%ld\n", thread_config->thread_name, 
                                                                 client->comp_host, client->comp_name, client->comp_id  );
#endif
               reference = (cl_byte_t *)help;
               ret_val = cl_commlib_send_message(handle, client->comp_host, client->comp_name, client->comp_id,
                                                 CL_MIH_MAT_NAK, &reference, DATA_SIZE,
                                                 NULL, 0, 0 , true, false);
             
               if ( ret_val != CL_RETVAL_OK) {
                  cl_com_free_endpoint(&(event_client_array[i])); /* should be locked at this point */
                  evc_count--;
               } else {
                  events_sent++;
               }
            }
         }
      }

      if ((ret_val = cl_thread_wait_for_event(thread_config,1, 0 )) != CL_RETVAL_OK) {  /* nothing to do sleep 1 sec */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               CL_LOG(CL_LOG_INFO,"condition wait timeout");
               break;
            default:
               CL_LOG_STR( CL_LOG_INFO, ">got error<: ", cl_get_error_text(ret_val));
               do_exit = 1;
         }
      }
   }

   CL_LOG(CL_LOG_INFO, "exiting ...");
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


