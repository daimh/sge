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

#include "uti/sge_profiling.h"

#define CL_DO_SLOW 1
void sighandler_client(int sig);
static int do_shutdown = 0;


/*---------------------------------------------------------------*/
void sighandler_client(
int sig 
) {
/*   thread_signal_receiver = pthread_self(); */
   if (sig == SIGPIPE || sig == SIGHUP) {
      return;
   }

   /* shutdown all sockets */
   do_shutdown = 1;
   cl_com_ignore_timeouts(true);
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  struct sigaction sa;
  static int runs = 100;


  cl_com_handle_t* handle = NULL; 
  cl_com_message_t* message = NULL;
  cl_com_endpoint_t* sender = NULL;

  int bytes_received = 0;
  unsigned long total_bytes_sent = 0;
  unsigned long total_bytes_received = 0;
  unsigned long total_connections = 0;
  unsigned long total_connection_sum = 0;
#if 1
  char* welcome_text = "Welcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\nWelcome to the tcp framework module!Welcome to the tcp framework module!Welcome to the tcp framework module!\n";
#else
   char* welcome_text = "This message is from thread 1";
#endif

  struct timeval now;
  long start_time;
  long appl_start_time;
  long end_time;
  int retval = CL_RETVAL_PARAMS;
  int welcome_text_size;
  int close_connection = 0;
  unsigned long last_mid = 0;
  int i;
  cl_framework_t framework = CL_CT_TCP;
  cl_xml_connection_autoclose_t autoclose;

  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_client;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

  
  if (argc < 7) {
     printf("wrong parameters, param1 = server host, param2 = port number, param3 = client id, param4=debug_level, param5=sleep time, param6=do_close, [param7=framework(TCP/SSL)]\n");
     exit(1);
  }
  prof_mt_init();
  cl_com_setup_commlib(CL_NO_THREAD, (cl_log_t)atoi(argv[4]), NULL);
  if (atoi(argv[6]) != 0) {
     close_connection = 1;
  }
  if (argv[7]) {
     framework = CL_CT_UNDEFINED;
     if (strcmp(argv[7], "TCP") == 0) {
        framework=CL_CT_TCP;
        printf("using TCP framework\n");
     }
     if (strcmp(argv[7], "SSL") == 0) {
        framework=CL_CT_SSL;
        printf("using SSL framework\n");
     }
     if (framework == CL_CT_UNDEFINED) {
        printf("unexpected framework type\n");
        exit(1);
     }
  }

  cl_com_set_alias_file("./alias_file");
  if ( framework == CL_CT_SSL) {
     cl_ssl_setup_t ssl_config;
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
     ssl_config.ssl_verify_func      = NULL;                       /*  function callback for peer user/name check  */
   
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
     cl_com_specify_ssl_configuration(&ssl_config);
  }
  CL_LOG_STR(CL_LOG_INFO,"connection to server on host", argv[1]);
  CL_LOG_INT(CL_LOG_INFO,"using port",atoi(argv[2])); 
  
  CL_LOG_STR(CL_LOG_INFO,"component is","client");
  CL_LOG_INT(CL_LOG_INFO,"id ist",atoi(argv[3]));
#define SELECT_TIMEOUT 1
#if 0
#define CREATE_SERVICE
#endif

#ifdef CREATE_SERVICE
  cl_com_append_known_endpoint_from_name(argv[1], "server", 1,atoi(argv[2]),CL_CM_AC_DISABLED , 1);
  handle=cl_com_create_handle(NULL,framework,CL_CM_CT_MESSAGE , true, 0 , CL_TCP_DEFAULT, "client", atoi(argv[3]),SELECT_TIMEOUT,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  } else {
     int my_port;
     cl_com_get_service_port(handle,&my_port);
     printf("I'm reachable at port %d!\n", my_port);
  }
#else
  handle=cl_com_create_handle(NULL,framework,CL_CM_CT_MESSAGE , false, atoi(argv[2]) , CL_TCP_DEFAULT,"client", atoi(argv[3]),SELECT_TIMEOUT,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     exit(1);
  }
#endif

  cl_com_set_auto_close_mode(handle,CL_CM_AC_ENABLED );
  cl_com_get_auto_close_mode(handle,&autoclose);
  if (autoclose != CL_CM_AC_ENABLED ) {
     printf("could not enable autoclose\n");
     exit(1);
  }  

  printf("local hostname is \"%s\"\n", handle->local->comp_host);
  printf("local component is \"%s\"\n", handle->local->comp_name);
  printf("local component id is \"%ld\"\n", handle->local->comp_id);

#ifdef CREATE_SERVICE
  cl_com_get_known_endpoint_port_from_name(argv[1], "server", 1, &i);
  printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[1]);
#else
  cl_com_get_connect_port(handle, &i);
  printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[1]);
#endif


  gettimeofday(&now,NULL);
  start_time =    now.tv_sec;
  appl_start_time = start_time;
  welcome_text_size = strlen(welcome_text) + 1;

  if (getenv("CL_RUNS")) { 
     runs = atoi(getenv("CL_RUNS"));
  } else {
     runs = -1;  /* disable runs shutdown */
  }
  while(do_shutdown != 1) {
     unsigned long mid = 0;
     int my_sent_error = 0;

     CL_LOG(CL_LOG_INFO,"main loop");
     if (runs > 0) {
        runs--;
     }

     /* printf("sending to \"%s\" ...\n", argv[1]);  */

/*     CL_LOG(CL_LOG_ERROR,"sending ack message ..."); */
     
     my_sent_error = cl_commlib_send_message(handle, argv[1], "server", 1, CL_MIH_MAT_ACK, (cl_byte_t**)&welcome_text , welcome_text_size, &mid ,0,0, true, false);
     if ( retval == CL_RETVAL_CONNECTION_NOT_FOUND ) {
        CL_LOG(CL_LOG_ERROR,"after new connection");
     }

     if (last_mid >= mid || mid == 1) {
        total_connections++;
        total_connection_sum++;
     }
     last_mid = mid;

#if 1
     if (my_sent_error != CL_RETVAL_OK) {
        printf("cl_commlib_send_message() returned %s\n", cl_get_error_text(my_sent_error)); 
#ifdef CREATE_SERVICE
        cl_com_get_known_endpoint_port_from_name(argv[1], "server", 1, &i);
        printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[1]);
#else
        cl_com_get_connect_port(handle, &i);
        printf("connecting to port \"%d\" on host \"%s\"\n", i, argv[1]);
#endif

        /* exit(1); */
#if CL_DO_SLOW
        sleep(atoi(argv[5]));
#endif
        cl_commlib_trigger(handle,1);
        continue;
     }
#endif
#if 1
     retval = CL_RETVAL_PARAMS;
     while (retval != CL_RETVAL_OK ) {


        while ( (retval=cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender)) == CL_RETVAL_OK) {
          if (message != NULL) {
             cl_com_free_endpoint(&sender);
             cl_com_free_message(&message);
          } else {
             break;
          }
        } 

        if ( retval == CL_RETVAL_CONNECTION_NOT_FOUND ) {
           CL_LOG(CL_LOG_ERROR,"connection not found (1)");
           break;
        }

        retval = cl_commlib_check_for_ack(handle, argv[1], "server", 1, mid, true );
        if (retval != CL_RETVAL_MESSAGE_WAIT_FOR_ACK && retval != CL_RETVAL_OK) {
           printf("retval of cl_commlib_check_for_ack(%ld) is %s\n",mid,cl_get_error_text(retval)); 
           /* exit(1);  */
           break;
        }
        if (retval == CL_RETVAL_OK) {
           CL_LOG_INT(CL_LOG_INFO,"received ack for message mid", (int)mid); 
        } else {
           cl_commlib_trigger(handle, 1);
        }

        if ( retval == CL_RETVAL_CONNECTION_NOT_FOUND ) {
           CL_LOG(CL_LOG_ERROR,"connection not found (2)");
           break;
        }



/*        printf("retval of cl_commlib_check_for_ack(%ld) is %s\n",mid,cl_get_error_text(retval));  */
     }
     if (retval == CL_RETVAL_CONNECTION_NOT_FOUND) {
         cl_commlib_trigger(handle, 1);
         continue; 
     }
#endif


     total_bytes_sent  = total_bytes_sent + welcome_text_size;
     CL_LOG_INT(CL_LOG_INFO,"bytes sent:", welcome_text_size);

     bytes_received = 0;
     while (bytes_received != welcome_text_size ) {

        cl_commlib_trigger(handle, 1); 


        CL_LOG_INT(CL_LOG_INFO,"waiting for mid .... ", (int)mid); 
        retval = cl_commlib_receive_message(handle,NULL, NULL, 0, false, mid, &message, &sender);


        CL_LOG_STR(CL_LOG_INFO,"waiting for bytes ...", cl_get_error_text(retval));
        if (retval == CL_RETVAL_CONNECTION_NOT_FOUND) {
#if CL_DO_SLOW
/*           CL_LOG(CL_LOG_ERROR,"connection not found"); */

           if (atoi(argv[5]) > 0) {
              printf("sleeping...\n");
              sleep(atoi(argv[5]));
           }
#endif
           break;
        }
        if (message != NULL) {

        /*   printf("received message from \"%s\"\n", sender->comp_host); */
           CL_LOG_INT(CL_LOG_INFO,"bytes received:", (int)message->message_length);
           if (strcmp((char*)message->message, welcome_text) != 0) {
              printf("------------------------> message transfer error\n");
           }
           total_bytes_received = total_bytes_received + message->message_length;
           bytes_received = bytes_received + (int)message->message_length;
           cl_com_free_endpoint(&sender);
           cl_com_free_message(&message);
        }
        
        while ( (retval = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender)) == CL_RETVAL_OK) {
          if (message != NULL) {
             cl_com_free_endpoint(&sender);
             cl_com_free_message(&message);
          } else {
             break;
          }
        } 
      
        if (retval == CL_RETVAL_CONNECTION_NOT_FOUND) {
           CL_LOG(CL_LOG_ERROR,"connection not found (3)");
           break;

        }


#if CL_DO_SLOW
        sleep(atoi(argv[5]));
#endif
        if (do_shutdown == 1) {
           break;
        }
    }

  
#if CL_DO_SLOW
     sleep(atoi(argv[5]));
#endif
     gettimeofday(&now,NULL);
     end_time =    now.tv_sec;
     if (end_time - start_time >= 2 ) {
        cl_com_connection_t* con = NULL;
        cl_connection_list_elem_t* elem = NULL;
/*        printf("Kbit/s sent: %.2f   ", ((total_bytes_sent * 8.0)/1024.0) /  (double)(end_time - start_time));
        printf("Kbit/s read: %.2f   ", ((total_bytes_received * 8.0)/1024.0) /  (double)(end_time - start_time)); */
        printf("KBit/s     : %.2f   ", (((total_bytes_received + total_bytes_sent) * 8.0)/1024.0) /  (double)(end_time - start_time));
        printf("connections/s: %.2f ", (double) total_connections / (double)(end_time - start_time));
        printf("avg. connections/s: %.2f", (double) total_connection_sum / (double)(end_time - appl_start_time));
        cl_raw_list_lock(handle->connection_list);
        elem = cl_connection_list_get_first_elem(handle->connection_list);
        if (elem != NULL) {
           con = elem->connection;
           if (elem->connection->local) {
              printf("[for comp host, comp name, comp id: \"%s\", \"%s\", \"%ld\"]    \n",con->local->comp_host, con->local->comp_name , con->local->comp_id);
           }
        } else {
           printf("     \n");
        }
        cl_raw_list_unlock(handle->connection_list);


        start_time =    now.tv_sec;
        total_bytes_sent = 0;
        total_bytes_received = 0;
        total_connections = 0;
        fflush(stdout);
     }
     if (close_connection != 0) {

        while (cl_commlib_shutdown_handle(handle, true) == CL_RETVAL_MESSAGE_IN_BUFFER) {
           printf("got message\n");
           message = NULL;
           cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
           if (message != NULL) {
              cl_com_free_endpoint(&sender);
              cl_com_free_message(&message);
           } else {
              printf("error shutdown handle");
              exit(-1);
              break;
           }
        }  
#ifdef CREATE_SERVICE
        handle=cl_com_create_handle(NULL,framework,CL_CM_CT_MESSAGE , true, 0 , CL_TCP_DEFAULT, "client", atoi(argv[3]),SELECT_TIMEOUT,0 );
        if (handle == NULL) {
           printf("could not get handle\n");
           exit(-1);
        } else {
           int my_port;
           cl_com_get_service_port(handle,&my_port);
           printf("I'm reachable at port %d!\n", my_port);
        }
#else
        handle=cl_com_create_handle(NULL,framework,CL_CM_CT_MESSAGE , false, atoi(argv[2]) , CL_TCP_DEFAULT, "client", atoi(argv[3]), SELECT_TIMEOUT,0 );
        if (handle == NULL) {
           printf("could not get handle\n");
           exit(-1);
        }
#endif
        cl_com_set_auto_close_mode(handle,CL_CM_AC_ENABLED );
        cl_com_get_auto_close_mode(handle,&autoclose);
        if (autoclose != CL_CM_AC_ENABLED ) {
           printf("could not enable autoclose\n");
           exit(1);
        }  
     }
     if (runs == 0) {
        do_shutdown = 1;
     }
  }
  printf("do_shutdown received\n");
  fflush(stdout);
  while (cl_commlib_shutdown_handle(handle, true) == CL_RETVAL_MESSAGE_IN_BUFFER) {
     printf("got message\n");
     message = NULL;

     cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
     if (message != NULL) {
        cl_com_free_endpoint(&sender);
        cl_com_free_message(&message);
     } else {
        break;
     }
  } 
  printf("cleanup commlib ...\n");
  cl_com_cleanup_commlib();

  printf("main done\n");
  fflush(stdout);
  return 0;
}





