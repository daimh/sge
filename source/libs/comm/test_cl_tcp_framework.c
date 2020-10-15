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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

#include "comm/lists/cl_lists.h"
#include "comm/cl_tcp_framework.h"
#include "comm/cl_communication.h"
#include "comm/cl_connection_list.h"


void sighandler(int sig);
void *server_thread(void *t_conf);
void *client_thread(void *t_conf);

static int pipe_signal = 0;
static int do_shutdown = 0;
char data[] = "> * * * Welcome to the tcp framework module! * * * <";

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  char help[255];
  struct sigaction sa;
  int nr = 0;

  cl_thread_settings_t* dummy_thread_p;
  cl_raw_list_t* thread_list = NULL;
  cl_raw_list_t* log_list = NULL;
 
  strcpy(help,"");
#if 0
  printf("sizeof CL_SIM_MESSAGE: %d\n", CL_SIM_MESSAGE_SIZE);
  sprintf(help,CL_SIM_MESSAGE,"");
  printf("\nstrlen(CL_SIM_MESSAGE) = %d\n",strlen(help));
  printf(help);
  printf("sizeof CL_SIRM_MESSAGE: %d\n", CL_SIRM_MESSAGE_SIZE);
  sprintf(help,CL_SIRM_MESSAGE,"",(unsigned long)0,(unsigned long)0,(unsigned long)0,(unsigned long)0,(unsigned long)0,(unsigned long)0,(unsigned long)0,"");
  printf("\nstrlen(CL_SIRM_MESSAGE) = %d\n",strlen(help)- 7);
  printf("|%s|\n",help);

  exit(1);
#endif
  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);


  if (argc != 2) {
     printf("arguments are client|server|both\n");
     exit(1);
  }

  printf("starting %s ...\n", argv[1]);
  fflush(stdout);
  sleep(1);

#if 1
  /* setup log list and log level*/
  cl_log_list_setup(&log_list, "main()", 0,CL_LOG_IMMEDIATE , NULL);
  cl_log_list_set_log_level(log_list,CL_LOG_WARNING);
#endif



  printf("debug1\n");
  /* setup thread list */
  cl_thread_list_setup(&thread_list, "thread list");
  printf("debug2\n");

  if (strcmp( argv[1], "client") == 0 || strcmp( argv[1], "both") == 0) {
     printf("debug3\n");

     printf("debug4\n");

     cl_thread_list_create_thread(thread_list, &dummy_thread_p,log_list, "client1", 2, client_thread, NULL, NULL);   
     printf("debug5\n");

  }
  printf("debug6\n");

  if (strcmp( argv[1], "server") == 0 || strcmp( argv[1], "both") == 0) {
     cl_thread_list_create_thread(thread_list, &dummy_thread_p,log_list, "server", 1, server_thread, NULL, NULL); 
  }
#if 0
  cl_thread_list_create_thread(thread_list, log_list, "client2", 3, client_thread );   
  cl_thread_list_create_thread(thread_list, log_list, "client3", 4, client_thread );   
  cl_thread_list_create_thread(thread_list, log_list, "client4", 5, client_thread );   
#endif




  while(do_shutdown == 0) {
     cl_thread_settings_t* thread_p = NULL;
     CL_LOG(CL_LOG_INFO,"still alive ..."); 


     thread_p = cl_thread_list_get_thread_by_id(thread_list, 1);
     if (thread_p) {
        if (thread_p->thread_state == CL_THREAD_EXIT || thread_p->thread_state == CL_THREAD_CANCELED) {
            do_shutdown = 1;
        } 
/*        CL_LOG_STR(CL_LOG_INFO,"thread state of thread 1:", cl_thread_get_state(thread_p)); */
     }

     thread_p = cl_thread_list_get_thread_by_id(thread_list, 2);
     if (thread_p) {
        if (thread_p->thread_state == CL_THREAD_EXIT || thread_p->thread_state == CL_THREAD_CANCELED) {
            do_shutdown = 1;
        } 
/*        CL_LOG_STR(CL_LOG_INFO,"thread state of thread 2:", cl_thread_get_state(thread_p)); */
     }

     if (pipe_signal != 0) {
        CL_LOG(CL_LOG_INFO, "received PIPE signal!");
        pipe_signal = 0;
     }
     sleep(1);
  }
  nr = 1;
  CL_LOG(CL_LOG_INFO, "shutdown ...");
  while(cl_thread_list_get_first_thread(thread_list) != NULL) {
     cl_thread_list_delete_thread_by_id(thread_list, nr);
     nr++;
  }
#if 0
  cl_thread_list_delete_thread(thread_list, 3); 
  cl_thread_list_delete_thread(thread_list, 4); 
  cl_thread_list_delete_thread(thread_list, 5); 
#endif

  cl_thread_list_cleanup(&thread_list);
  cl_log_list_cleanup(&log_list);
  printf("main done\n");
  return 0;
}


/*---------------------------------------------------------------*/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "sighandler()"
void sighandler(
int sig 
) {
   if (sig == SIGPIPE) {
      pipe_signal = 1;
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
#define __CL_FUNCTION__ "server_cleanup_conlist()"
void server_cleanup_conlist(cl_raw_list_t** connection_list) {
   cl_com_handle_t handle;
   CL_LOG(CL_LOG_INFO,"start");
   if (connection_list && *connection_list) {
      cl_connection_list_elem_t* con_elem = NULL;
      cl_raw_list_lock(*connection_list);
      con_elem = cl_connection_list_get_first_elem(*connection_list);
      while(con_elem) {
         cl_com_connection_t* connection = con_elem->connection;
         connection->connection_state = CL_CLOSING;
         connection->connection_sub_state = CL_COM_DO_SHUTDOWN;
         CL_LOG(CL_LOG_INFO,"marking connection to close");
         con_elem = cl_connection_list_get_next_elem(con_elem);
      }
      cl_raw_list_unlock(*connection_list);
      handle.connection_list = *connection_list;
      cl_connection_list_destroy_connections_to_close(&handle);
      cl_connection_list_cleanup(connection_list);
   }
}
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "server_cleanup()"
void server_cleanup(cl_com_connection_t** con) {
   CL_LOG(CL_LOG_INFO,"start");

   if (con && *con) {
      int retval;
      CL_LOG(CL_LOG_INFO,"closing server connection");
      retval = cl_com_close_connection(con);
      CL_LOG_STR(CL_LOG_INFO, "cl_com_close_connection() returned ", cl_get_error_text(retval) );
   }
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "server_thread()"
void *server_thread(void *t_conf) {
   struct timeval now; 
   cl_com_handle_t handle;
   int ret_val, retval,do_exit = 0;
   cl_com_connection_t* con = NULL;
   cl_com_connection_t* new_con = NULL;
   cl_raw_list_t* connection_list = NULL;
   cl_com_endpoint_t* local_host = NULL;
   char* local_hostname = NULL;
   struct in_addr in_addr;

   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 


   /* set thread config data */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      printf("cl_thread_set_thread_config() error\n");
   }


   /* setup thread */
   CL_LOG(CL_LOG_INFO, "starting initialization ...");

   cl_com_gethostname(&local_hostname, &in_addr, NULL, NULL);
   local_host = cl_com_create_endpoint(local_hostname, "server", 1, &in_addr);
   sge_free(&local_hostname);

   retval = cl_com_tcp_setup_connection(&con, 5000, 5000,CL_CM_CT_STREAM, CL_CM_AC_DISABLED, CL_CT_TCP, CL_CM_DF_BIN, CL_TCP_DEFAULT );
   CL_LOG_STR(CL_LOG_INFO, "cl_com_setup_tcp_connection() returned ", cl_get_error_text(retval) );

   retval = cl_com_connection_request_handler_setup(con, local_host);
   CL_LOG_STR(CL_LOG_INFO, "cl_com_connection_request_handler_setup() returned ", cl_get_error_text(retval) );

   if (retval != CL_RETVAL_OK) {
      cl_com_connection_request_handler_cleanup(con);
      cl_com_close_connection(&con);
      do_exit = 1;
   }

   cl_connection_list_setup(&connection_list, "con list" , 1);

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);
   CL_LOG(CL_LOG_INFO, "starting main loop ...");

   /* ok, thread main */

   while (do_exit == 0) {
      int pthread_cleanup_pop_execute = 0; /* workaround for irix compiler warning */
      cl_connection_list_elem_t* con_elem = NULL;
 
      cl_com_connection_t*  connection = NULL;

      pthread_cleanup_push((void (*)(void *)) server_cleanup, (void*) &con );
      pthread_cleanup_push((void (*)(void *)) server_cleanup_conlist, (void*) &connection_list);
      cl_thread_func_testcancel(thread_config);
      pthread_cleanup_pop(pthread_cleanup_pop_execute); /* list cleanup */
      pthread_cleanup_pop(pthread_cleanup_pop_execute); /* server cleanup */

      CL_LOG_INT( CL_LOG_INFO, "--> nr of connections: ", (int)cl_raw_list_get_elem_count(connection_list) );

      new_con = NULL;
      if (con->data_read_flag == CL_COM_DATA_READY) {
         retval = cl_com_connection_request_handler(con, &new_con);
      }
      CL_LOG_STR(CL_LOG_INFO, "cl_com_connection_request_handler() returned ",cl_get_error_text(retval) );

      if (retval == CL_RETVAL_OK && new_con != NULL) {
         CL_LOG(CL_LOG_INFO,"new connection!!!");
         while( (retval=cl_com_connection_complete_request(connection_list, con_elem, CL_DEFINE_GET_CLIENT_CONNECTION_DATA_TIMEOUT, CL_RW_SELECT)) != CL_RETVAL_OK) {
            if (retval != CL_RETVAL_UNCOMPLETE_WRITE && retval != CL_RETVAL_UNCOMPLETE_READ) {
               break;
            }
         }
         if (retval != CL_RETVAL_OK) {
            cl_com_tcp_close_connection(&new_con);
            CL_LOG(CL_LOG_ERROR,"error receiving connection data");
         } else {
            cl_connection_list_append_connection(connection_list, new_con,1);
         }
         
         new_con = NULL;
      }
      
      CL_LOG(CL_LOG_INFO, "checking open connections ...");
      
      retval = cl_com_open_connection_request_handler(CL_CT_TCP,connection_list, con  ,1,0, CL_RW_SELECT);
      CL_LOG_STR( CL_LOG_INFO, "cl_com_open_connection_request_handler() returned ", cl_get_error_text(retval) );

      if (retval == CL_RETVAL_OK) {
         cl_raw_list_lock(connection_list);
         con_elem = cl_connection_list_get_first_elem(connection_list);
         while(con_elem) {
            connection = con_elem->connection;
     
            if (connection->data_read_flag == CL_COM_DATA_READY) {
               CL_LOG( CL_LOG_INFO, "we have data to read");
               gettimeofday(&now,NULL);
               connection->read_buffer_timeout_time = now.tv_sec + 10;
               retval = cl_com_read(connection, connection->data_read_buffer, sizeof(data), NULL); 
               CL_LOG_STR(CL_LOG_INFO, "cl_com_read() returned ", cl_get_error_text(retval) );

               if (retval != CL_RETVAL_OK && retval != CL_RETVAL_UNCOMPLETE_READ) {
                  /* close this connection */
                  con_elem = cl_connection_list_get_next_elem(con_elem);
                  CL_LOG( CL_LOG_INFO, "set connection close flag");
                  connection->connection_state = CL_CLOSING;
                  connection->connection_sub_state = CL_COM_DO_SHUTDOWN;
                  continue;
               }
               CL_LOG_STR( CL_LOG_WARNING, "data is:", (char*)connection->data_read_buffer);
               gettimeofday(&now,NULL);
               connection->write_buffer_timeout_time = now.tv_sec + 10;
               retval = cl_com_write(connection, connection->data_read_buffer, sizeof(data), NULL); 
            } else {
               CL_LOG( CL_LOG_INFO, "no data");
            }
            con_elem = cl_connection_list_get_next_elem(con_elem);
         }
         cl_raw_list_unlock(connection_list);
      }

      CL_LOG_INT(CL_LOG_INFO, "--> nr of connections: ", (int)cl_raw_list_get_elem_count(connection_list) );
      handle.connection_list = connection_list;
      cl_connection_list_destroy_connections_to_close(handle);
      CL_LOG_INT(CL_LOG_INFO, "--> nr of connections: ", (int)cl_raw_list_get_elem_count(connection_list) );

#if 1
      if ((ret_val = cl_thread_wait_for_event(thread_config,2,100000)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               break;
            default:
               CL_LOG_STR( CL_LOG_INFO, ">got error<: ", cl_get_error_text(ret_val));
               do_exit = 1;
         }
      }
#endif
   }

   cl_connection_list_cleanup(&connection_list);
   CL_LOG(CL_LOG_INFO, "exiting ...");

   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "client_cleanup_function()"
void client_cleanup_function(cl_com_connection_t** con) {
   if (con && *con) {
      int retval;
      CL_LOG(CL_LOG_INFO,"closing connection");
      retval = cl_com_close_connection(con);
      CL_LOG_STR(CL_LOG_INFO, "cl_com_close_connection() returned ", cl_get_error_text(retval) );
   }
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "client_thread()"
void *client_thread(void *t_conf) {
   int ret_val, retval,i,do_exit = 0;
   struct timeval now;
   cl_com_connection_t* con = NULL;
   cl_com_endpoint_t* local_host = NULL;
   cl_com_endpoint_t* remote_host = NULL;
   char* local_hostname = NULL;
   struct in_addr local_addr;

   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 

   /* set thread config data */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      printf("cl_thread_set_thread_config() error\n");
   }

   /* setup thread */
   CL_LOG( CL_LOG_INFO, "starting initialization ...");
   cl_com_gethostname(&local_hostname, &local_addr ,NULL, NULL);
   local_host    = cl_com_create_endpoint(local_hostname, thread_config->thread_name, 0, &local_addr);
   remote_host   = cl_com_create_endpoint(local_hostname, "server", 1, &local_addr);
   sge_free(&local_hostname);

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);
   CL_LOG( CL_LOG_INFO,  "starting main loop ...");

   /* ok, thread main */
   while (do_exit == 0) {
      int pthread_cleanup_pop_execute = 0; /* workaround for irix compiler warning */
      pthread_cleanup_push((void (*)(void *)) client_cleanup_function, (void*) &con );
      cl_thread_func_testcancel(thread_config);
      pthread_cleanup_pop(pthread_cleanup_pop_execute);  /* client_thread_cleanup */

      if (con == NULL) {
         cl_com_tcp_setup_connection(&con, 5000, 5000,CL_CM_CT_STREAM, CL_CM_AC_DISABLED, CL_CT_TCP, CL_CM_DF_BIN, CL_TCP_DEFAULT );
         while( (retval = cl_com_open_connection(con, 5, remote_host, local_host)) == CL_RETVAL_UNCOMPLETE_WRITE) {
            sleep(1);
         }
         CL_LOG_STR( CL_LOG_INFO, "cl_com_open_connection() returned ", cl_get_error_text(retval) );
         if (retval != CL_RETVAL_OK) {
            retval = cl_com_close_connection(&con);
            CL_LOG_STR(CL_LOG_INFO, "cl_com_close_connection() returned ", cl_get_error_text(retval) );
         } 

         while( ( retval= cl_com_connection_complete_request(con, CL_DEFINE_GET_CLIENT_CONNECTION_DATA_TIMEOUT, CL_RW_SELECT )) != CL_RETVAL_OK) {
            if (retval != CL_RETVAL_UNCOMPLETE_WRITE && retval != CL_RETVAL_UNCOMPLETE_READ) {
               break;
            }
            sleep(1);
         }
         if (retval != CL_RETVAL_OK) {
            cl_com_tcp_close_connection(&con);
            CL_LOG(CL_LOG_ERROR,"error receiving connection data");
         }
      } 
      if (con != NULL) {
         for(i=0;i<1;i++) {
#if 1
            gettimeofday(&now,NULL);
            con->write_buffer_timeout_time = now.tv_sec + 10;
            retval = cl_com_write(con, (cl_byte_t*)data, sizeof(data), NULL ); 
            CL_LOG_INT( CL_LOG_INFO, "cl_com_write() nr ",i );
            if (retval != CL_RETVAL_OK) {
               CL_LOG_STR(CL_LOG_INFO, "cl_com_write() returned ", cl_get_error_text(retval) );
               retval = cl_com_close_connection(&con);
               CL_LOG_STR(CL_LOG_INFO, "cl_com_close_connection() returned ", cl_get_error_text(retval) );
            }
            

#endif
         }
      }
      
#if 1
      if (con != NULL) {
         gettimeofday(&now,NULL);
         con->read_buffer_timeout_time = now.tv_sec + 5;
         retval = cl_com_read(con, con->data_read_buffer, sizeof(data), NULL); 
         CL_LOG_STR(CL_LOG_INFO, "cl_com_read() returned ", cl_get_error_text(retval) );
         CL_LOG_STR( CL_LOG_WARNING, "data is:", (char*)con->data_read_buffer);
#endif
#if 0  /* enable this to close the connection each time an answer is read */
            retval = cl_com_close_connection(&con);
            CL_LOG_STR(CL_LOG_INFO, "cl_com_close_connection() returned ", cl_get_error_text(retval) );
#endif
#if 1
            
      }
#endif


#if 1
      if ((ret_val = cl_thread_wait_for_event(thread_config,0,1000*100)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               break;
            default:
               CL_LOG_STR( CL_LOG_INFO,  ">got error<: ", cl_get_error_text(ret_val));
               do_exit = 1;
         }
      }
#endif
   }

   CL_LOG( CL_LOG_INFO, "exiting ...");

   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}
