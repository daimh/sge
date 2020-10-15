
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
 *  Copyright: 2005 by Sun Microsystems, Inc.
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
#include <pwd.h>

#include "cl_commlib.h"
#include "uti/sge_stdlib.h"

/* 
 *  global signal flags 
 */
static int do_shutdown = 0;



/*
 *  signal handler 
 */
void sighandler_function( int sig ) {
   if (sig == SIGPIPE) {
      return;
   }
   if (sig == SIGHUP) {
      return;
   }
   
   /*
    *  set shutdown flag for all other signals
    */ 
   do_shutdown = 1;
}



/*
 *  function for setting up the signal handler 
 */
void setup_signal_handler(void) {
   struct sigaction sa;
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = sighandler_function;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGINT, &sa, NULL);
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGHUP, &sa, NULL);
   sigaction(SIGPIPE, &sa, NULL);
}


static bool my_ssl_verify_func(cl_ssl_verify_mode_t mode, bool service_mode, const char* value) {
   return true;
}


/* 
 *  usage function 
 */
void usage(void) {
   fprintf(stderr,"usage: server_demo <port>\n\n");
   fprintf(stderr,"   <port>: a free port which can be bound\n");
   fprintf(stderr,"   <Connection Mode>: Connection mode of server_demo [TCP|SSL]\n");
}



/*
 *  commlib error callback function
 */
void on_communication_error(const cl_application_error_list_elem_t* commlib_error) {
   if (commlib_error != NULL) {
      /* print any communication error to stderr: */
      fprintf(stderr, "COMMLIB ERROR: %s (%s)\n", 
              cl_get_error_text(commlib_error->cl_error),
              commlib_error->cl_info);
   }
}



/*
 *  commlib debug log function callback 
 */
int on_communication_log(cl_raw_list_t* list_p) {
   cl_log_list_elem_t* elem = NULL;

   /* lock the list */
   cl_raw_list_lock(list_p);

   /* print out the complete log list and delete log message */
   while ( (elem = cl_log_list_get_first_elem(list_p) ) != NULL) {
      if (elem->log_parameter == NULL) {
         printf("COMMLIB LOGGING(%s): %s\n", 
                cl_log_list_convert_type_id(elem->log_type),
                elem->log_message);
      } else {
         printf("COMMLIB LOGGING(%s): %s %s\n", 
                cl_log_list_convert_type_id(elem->log_type),
                elem->log_message,
                elem->log_parameter);
      }
      cl_log_list_del_log(list_p);
   }
   
   /* unlock the list */
   cl_raw_list_unlock(list_p);
   return CL_RETVAL_OK;
}

/*
 *  main()
 */
extern int main(int argc, char** argv) {

   int                handle_port = 0;
   cl_com_handle_t*   handle      = NULL; 
   cl_com_endpoint_t  endpoints[20];
   cl_framework_t framework   = CL_CT_TCP;
   cl_ssl_setup_t     ssl_config;
   int                i           = 0;
   int                i_ends      = 0;
   int                j;

   /* check command line argument count */
   if (argc <= 2) {
      usage();
      exit(1);
   }
 
   /* get server port from command line */
   handle_port = atoi(argv[1]);
   if (handle_port <= 0) {
      fprintf(stderr,"need a port number > 0\n");
      usage();
      exit(1);
   }
 
   if (argv[2]) {
      if (strcmp(argv[2], "SSL") == 0) {
         framework=CL_CT_SSL;
         printf("using SSL framework\n");
      }
   }else{
      printf("using TCP framework\n");
   }
 
   /* set the ssl environment */
   if(framework == CL_CT_SSL){
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
 
   /* setup signalhandling */
   setup_signal_handler();
 
   /* setup commlib */
   cl_com_setup_commlib(CL_RW_THREAD, CL_LOG_OFF, NULL);
 
   /* setup commlib error function callback */
   cl_com_set_error_func(on_communication_error);
 
   if (framework == CL_CT_SSL) {
      cl_com_specify_ssl_configuration(&ssl_config);
   }

   /* create communication handle */
   handle=cl_com_create_handle(NULL, 
                               framework, 
                               CL_CM_CT_MESSAGE,
                               true,
                               handle_port,
                               CL_TCP_DEFAULT,
                               "server", 1,
                               5, 0 );
 
   if (handle == NULL) {
      fprintf(stderr, "could not create communication handle\n");
      cl_com_cleanup_commlib();
      exit(1);
   }
 
   /* print out some info output */
   printf("server running:\n");
   printf("host: \"%s\"\n", handle->local->comp_host);
   cl_com_get_service_port(handle,&i);
   printf("port: %d\n",     i);
   printf("name: \"%s\"\n", handle->local->comp_name);
   printf("id:   %ld\n",    handle->local->comp_id);
     
   /* application main loop */
   while(do_shutdown != 1) {
 
      cl_com_message_t*  message = NULL;
      cl_com_endpoint_t* sender = NULL;
      cl_commlib_trigger(handle, 1);
 
      cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
 
      if (message != NULL) {
         printf("%s@%s: %s\n", sender->comp_host, sender->comp_name, message->message);
         if (strcmp((const char*)message->message, "reg") == 0){
            endpoints[i_ends].comp_host = strdup(sender->comp_host);
            endpoints[i_ends].comp_name = strdup(sender->comp_name);
            endpoints[i_ends].comp_id = sender->comp_id;
            printf("%s@%s was registered\n", endpoints[i_ends].comp_host, endpoints[i_ends].comp_name);
            i_ends++;
         }else{
            for(j = 0; j < i_ends; j++){
               if (strncmp((const char*)message->message, "exit_server", 11) == 0){
                  strcpy((char*)message->message, "SIGTERM");
                  do_shutdown = 1;
               }
               printf("Sending %s to %s@%s:%ld\n", message->message, endpoints[j].comp_host, endpoints[j].comp_name, endpoints[j].comp_id);
               cl_commlib_send_message(handle, 
                                       endpoints[j].comp_host,
                                       endpoints[j].comp_name, 
                                       endpoints[j].comp_id, CL_MIH_MAT_NAK,  
                                       (cl_byte_t**)&message->message, 
                                       strlen((const char*)message->message)+1, 
                                       NULL, 0, 0, 
                                       true,false);
            }
         }
         cl_com_free_message(&message);
         message = NULL;
         cl_com_free_endpoint(&sender);
         sender = NULL;
      }
   }

   printf("shutting down ...\n");

   /* here the application goes down - shutdown communication lib */
   while ( cl_commlib_shutdown_handle(handle, true) == CL_RETVAL_MESSAGE_IN_BUFFER) {
      cl_com_message_t*  message = NULL;
      cl_com_endpoint_t* sender = NULL;
      cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
 
      if (message != NULL) {
         printf("ignoring message from \"%s\"\n", sender->comp_host); 
         cl_com_free_message(&message);
         cl_com_free_endpoint(&sender);
         message = NULL;
         sender = NULL;
      }
   }
   for(j = 0; j < i_ends; j++){
      sge_free(&(endpoints[j].comp_host));
      sge_free(&(endpoints[j].comp_name));
   }
 
   /* cleanup commlib */
   cl_com_cleanup_commlib();

  return 0;
}
