
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
#include "uti/sge_uidgid.h"
#define CL_DO_SLOW 0

void sighandler_server(int sig);
static int pipe_signal = 0;
static int hup_signal = 0;
static int do_shutdown = 0;
static int received_qping = 0;

void sighandler_server(
int sig 
) {
   if (sig == SIGPIPE) {
      pipe_signal = 1;
      return;
   }

   if (sig == SIGHUP) {
      hup_signal = 1;
      return;
   }

   /* shutdown all sockets */
   cl_com_ignore_timeouts(true);

   do_shutdown = 1;
}

const char* my_application_tag_name(unsigned long tag) {
   if (tag > 0) {
      return "TAG > 0";
   }
   return "DEFAULT_APPLICATION_TAG";
}

unsigned long my_application_status(char** info_message) {
   if ( info_message != NULL ) {
      (*info_message) = strdup("not specified (state 1)");
   }
#if 0
   received_qping++;
#endif
   if (received_qping > 10) {
      do_shutdown = 1;
   }
   return (unsigned long)1;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_ssl_verify_func()"
static bool my_ssl_verify_func(cl_ssl_verify_mode_t mode, bool service_mode, const char* value) {
   char* user_name = NULL;
   struct passwd *paswd = NULL;
   struct passwd pw_struct;
   char *pw_buffer;
   int pw_buffer_size;

   pw_buffer_size = get_pw_buffer_size();
   pw_buffer = sge_malloc(pw_buffer_size);
   if(getpwuid_r(getuid(), &pw_struct, pw_buffer, pw_buffer_size, &paswd) != 0) {
      CL_LOG(CL_LOG_ERROR, "getpwuid_r failed");
   }
   if (paswd != NULL) {
      user_name = paswd->pw_name;
   }
   if (user_name == NULL) {
      user_name = "unexpected user name";
   }
   if (service_mode == true) {
      CL_LOG(CL_LOG_WARNING,"running in service mode");
      switch(mode) {
         case CL_SSL_PEER_NAME: {
            CL_LOG(CL_LOG_WARNING,"CL_SSL_PEER_NAME");
            if (strcmp(value,"SGE admin user") != 0) {
               CL_LOG(CL_LOG_WARNING,"CL_SSL_PEER_NAME is not \"SGE admin user\"");
               return false;
            }
            break;
         }
         case CL_SSL_USER_NAME: {
            CL_LOG(CL_LOG_WARNING,"CL_SSL_USER_NAME");
            if (strcmp(value,user_name) != 0) {
               CL_LOG_STR(CL_LOG_WARNING,"CL_SSL_USER_NAME is not", user_name);
               return false;
            }
            break;
         }
      }
   } else {
      CL_LOG(CL_LOG_WARNING,"running in client mode");
      switch(mode) {
         case CL_SSL_PEER_NAME: {
            CL_LOG(CL_LOG_WARNING,"CL_SSL_PEER_NAME");
            if (strcmp(value,"SGE admin user") != 0) {
               CL_LOG(CL_LOG_WARNING,"CL_SSL_PEER_NAME is not \"SGE Daemon\"");
               return false;
            }
            break;
         }
         case CL_SSL_USER_NAME: {
            CL_LOG(CL_LOG_WARNING,"CL_SSL_USER_NAME");
            if (strcmp(value,user_name) != 0) {
               CL_LOG_STR(CL_LOG_WARNING,"CL_SSL_USER_NAME is not", user_name);
               return false;
            }
            break;
         }
      }
   }
   return true;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  struct sigaction sa;
  cl_ssl_setup_t ssl_config;
  static int runs = 100;
  int handle_port = 0;
  cl_com_handle_t* handle = NULL; 
  cl_com_message_t* message = NULL;
  cl_com_endpoint_t* sender = NULL;
  int i;
  unsigned long max_connections;
  cl_log_t log_level;
  cl_framework_t framework = CL_CT_TCP;

  memset(&ssl_config, 0, sizeof(ssl_config));
  ssl_config.ssl_method           = CL_SSL_v23;                 /*  v23 method                                  */
  ssl_config.ssl_CA_cert_pem_file = getenv("SSL_CA_CERT_FILE"); /*  CA certificate file                         */
  ssl_config.ssl_CA_key_pem_file  = NULL;                       /*  private certificate file of CA (not used)   */
  ssl_config.ssl_cert_pem_file    = getenv("SSL_CERT_FILE");    /*  certificates file                           */
  ssl_config.ssl_key_pem_file     = getenv("SSL_KEY_FILE");     /*  key file                                    */
  ssl_config.ssl_rand_file        = getenv("SSL_RAND_FILE");    /*  rand file (if RAND_status() not ok)         */
  ssl_config.ssl_crl_file         = getenv("SSL_CRL_FILE");     /*  revocation list file                        */
  ssl_config.ssl_reconnect_file   = NULL;                       /*  file for reconnect data    (not used)       */
  ssl_config.ssl_refresh_time     = 0;                          /*  key alive time for connections (not used)   */
  ssl_config.ssl_password         = NULL;                       /*  password for encrypted keyfiles (not used)  */
  ssl_config.ssl_verify_func      = my_ssl_verify_func;         /*  function callback for peer user/name check  */

  if (getenv("CL_PORT")) {
     handle_port = atoi(getenv("CL_PORT"));
  }

  if (argc < 2) {
      printf("param1=debug_level [param2=framework(TCP/SSL)]\n");
      exit(1);
  }

  prof_mt_init(); 

  if (argv[2]) {
     framework = CL_CT_UNDEFINED;
     if (strcmp(argv[2], "TCP") == 0) {
        framework=CL_CT_TCP;
        printf("using TCP framework\n");
     }
     if (strcmp(argv[2], "SSL") == 0) {
        framework=CL_CT_SSL;
        printf("using SSL framework\n");

        if (ssl_config.ssl_CA_cert_pem_file == NULL ||
            ssl_config.ssl_cert_pem_file    == NULL ||
            ssl_config.ssl_key_pem_file     == NULL ||
            ssl_config.ssl_rand_file        == NULL) {
           printf("please set the following environment variables:\n");
           printf("SSL_CA_CERT_FILE         = CA certificate file\n");
           printf("SSL_CERT_FILE            = certificates file\n");
           printf("SSL_KEY_FILE             = key file\n");
           printf("(optional) SSL_RAND_FILE = rand file (if RAND_status() not ok)\n");
        }
     }
     if (framework == CL_CT_UNDEFINED) {
        printf("unexpected framework type\n");
        exit(1);
     }
  }

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

  switch(atoi(argv[1])) {
     case 0:
        log_level=CL_LOG_OFF;
        break;
     case 1:
        log_level=CL_LOG_ERROR;
        break;
     case 2:
        log_level=CL_LOG_WARNING;
        break;
     case 3:
        log_level=CL_LOG_INFO;
        break;
     case 4:
        log_level=CL_LOG_DEBUG;
        break;
     default:
        log_level=CL_LOG_OFF;
        break;
  }
  cl_com_setup_commlib(CL_RW_THREAD, log_level, NULL);


  cl_com_set_parameter_list_value("parameter1","value1");
  cl_com_set_parameter_list_value("parameter2","value2");
  cl_com_set_parameter_list_value("parameter3","value3");
  cl_com_set_parameter_list_value("parameter4","value4");

  cl_com_set_alias_file("./alias_file");

  cl_com_set_status_func(my_application_status); 
 
  cl_com_set_tag_name_func(my_application_tag_name);

  if (framework == CL_CT_SSL) {
     cl_com_specify_ssl_configuration(&ssl_config);
  }

  handle=cl_com_create_handle(NULL, framework, CL_CM_CT_MESSAGE, true, handle_port, CL_TCP_DEFAULT, "server", 1, 1, 0 );
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

  cl_com_add_allowed_host(handle, handle->local->comp_host);

  cl_com_get_max_connections(handle,&max_connections);
  printf("max open connections is set to %lu\n", max_connections);
  cl_com_set_max_connection_close_mode(handle,CL_ON_MAX_COUNT_CLOSE_AUTOCLOSE_CLIENTS );


  cl_com_append_known_endpoint_from_name(handle->local->comp_host, "server", 1, 5000, CL_CM_AC_ENABLED, false );

  if (getenv("CL_RUNS")) { 
     runs = atoi(getenv("CL_RUNS"));
  }
  while(do_shutdown != 1) {
     int ret_val;

     if (getenv("CL_FORK_SLEEP_CHILD")) {
        int sleep_child = atoi(getenv("CL_FORK_SLEEP_CHILD"));
        int sleep_parent = 1;
        pid_t child_pid = 0;
        if (getenv("CL_FORK_SLEEP_PARENT")) {
           sleep_parent = atoi(getenv("CL_FORK_SLEEP_PARENT"));
        }
        if ((child_pid = fork()) == 0) {
           printf("fork() - child - mypid=(%d)!\n", (int) getpid());
           printf("fork() - child - sleep %d ...\n", (int) sleep_child);
           fflush(stdout);
           sleep(sleep_child);
           printf("fork() - child - sleep %d done\n", (int) sleep_child);
           fflush(stdout);
           exit(0);
        } else {
           printf("fork() - parent - mypid=(%d)!\n", (int) getpid());
           printf("fork() - parent - childpid=(%d)!\n", (int) child_pid);
           printf("fork() - parent - sleep %d ...\n", (int) sleep_parent);
           fflush(stdout);
           sleep(sleep_parent);
           printf("fork() - parent - sleep %d done\n", (int) sleep_parent);
           printf("fork() - parent - killing child pid %d ...\n", (int) child_pid);
           fflush(stdout);
           kill(child_pid, SIGKILL);
           printf("fork() - parent - killing child pid %d done\n", (int) child_pid);
           fflush(stdout);
        }
     } 

     CL_LOG(CL_LOG_INFO,"main()");
     cl_commlib_trigger(handle, 1); 

     if (getenv("CL_RUNS")) { 
        printf("runs: %d\n", runs);
        runs--;
     }

     if (runs<= 0) {
        do_shutdown = 1;
     }
#if 0
     {
        cl_raw_list_t* tmp_endpoint_list = NULL;
        cl_endpoint_list_elem_t* elem = NULL;
   
        cl_commlib_search_endpoint(handle, NULL, NULL, 1, true, &tmp_endpoint_list);
        elem = cl_endpoint_list_get_first_elem(tmp_endpoint_list);
        printf("\nconnected endpoints with id=1:\n");
        printf("==============================\n");
   
        while(elem) {
           printf("%s/%s/%d\n", elem->endpoint->comp_host, elem->endpoint->comp_name, elem->endpoint->comp_id);
           elem = cl_endpoint_list_get_next_elem(tmp_endpoint_list, elem);
        }
        cl_endpoint_list_cleanup(&tmp_endpoint_list);
   
        cl_commlib_search_endpoint(handle, NULL, NULL, 1, false, &tmp_endpoint_list);
        elem = cl_endpoint_list_get_first_elem(tmp_endpoint_list);
        printf("\nconnected and known endpoints with id=1:\n");
        printf("=========================================\n");
   
        while(elem) {
           printf("%s/%s/%d\n", elem->endpoint->comp_host, elem->endpoint->comp_name, elem->endpoint->comp_id);
           elem = cl_endpoint_list_get_next_elem(tmp_endpoint_list, elem);
        }
        cl_endpoint_list_cleanup(&tmp_endpoint_list);
   
        cl_commlib_search_endpoint(handle, NULL, "client", 0, false, &tmp_endpoint_list);
        elem = cl_endpoint_list_get_first_elem(tmp_endpoint_list);
        printf("\nconnected and known endpoints with comp_name=client:\n");
        printf("=====================================================\n");
   
        while(elem) {
           printf("%s/%s/%d\n", elem->endpoint->comp_host, elem->endpoint->comp_name, elem->endpoint->comp_id);
           elem = cl_endpoint_list_get_next_elem(tmp_endpoint_list, elem);
        }
        cl_endpoint_list_cleanup(&tmp_endpoint_list);
     }
#endif

#if 0
     /* TODO: check behaviour for unknown host and for a host which is down */
     cl_commlib_send_message(handle, "down_host", "nocomp", 1, CL_MIH_MAT_ACK, (cl_byte_t*)"blub", 5, NULL, 1, 1 ); /* check wait for ack / ack_types  TODO*/
#endif
 
     ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
     CL_LOG_STR(CL_LOG_INFO,"cl_commlib_receive_message() returned",cl_get_error_text(ret_val));

     if (message != NULL) {
          CL_LOG_STR(CL_LOG_INFO,"received message from",sender->comp_host);

/*        printf("received message from \"%s/%s/%ld\"\n", sender->comp_host, sender->comp_name, sender->comp_id); */



        if (strstr((char*)message->message,"exit") != NULL) {
           printf("received \"exit\" message from host %s, component %s, id %ld\n",
                  sender->comp_host,sender->comp_name,sender->comp_id );
           cl_commlib_close_connection(handle, sender->comp_host,sender->comp_name,sender->comp_id, false );
           
        } else {
           ret_val = cl_commlib_send_message(handle, 
                                sender->comp_host, 
                                sender->comp_name, 
                                sender->comp_id, CL_MIH_MAT_NAK,  
                                &message->message, 
                                message->message_length, 
                                NULL, message->message_id,0, 
                                false,false);
           if (ret_val != CL_RETVAL_OK) {
              CL_LOG_INT(CL_LOG_ERROR,"sent message response for message id", (int)message->message_id);
              CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_send_message() returned:",cl_get_error_text(ret_val));
           } else {
              CL_LOG_INT(CL_LOG_INFO,"sent message response for message id", (int)message->message_id);
           }
           cl_com_application_debug(handle, "message sent (1)");
           cl_com_application_debug(handle, "message sent (2)");
        }

        cl_com_free_message(&message);
        cl_com_free_endpoint(&sender);
        message = NULL;
     } 
#if CL_DO_SLOW
     sleep(1);
#endif
  }

  printf("shutting down server ...\n");
  handle = cl_com_get_handle( "server", 1 );
  if (handle == NULL) {
     printf("could not find handle\n");
     exit(1);
  } else {
     printf("found handle\n");
  }

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
  fflush(stdout);
  return 0;
}
