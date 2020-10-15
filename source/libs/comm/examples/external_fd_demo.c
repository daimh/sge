
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



/* 
 *  global signal flags 
 */
static int do_shutdown   = 0;
char*      server_host   = NULL;
int        handle_port   = 0;
static int ready         = 0;

static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t data_in_mutex = PTHREAD_MUTEX_INITIALIZER;



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
   fprintf(stderr,"usage: client_demo <host> <port>\n\n");
   fprintf(stderr,"   <host>: host of running server_demo\n");
   fprintf(stderr,"   <port>: port number of server_demo\n");
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

/* this is the callback function for std_in */
/* fd          -     file descriptor
   read_ready  -     states if fd is ready for reading
   write_ready -     states if fd is ready for writing
   user_data   -     void pointer which can be used for data transfering (be aware of race conditions)
   err_val     -     states if an error occured while poll/select of the fd
*/
int fd_in_cb(int fd, bool read_ready, bool write_ready, void* user_data, int err_val){
   if (err_val != 0) {
      return CL_RETVAL_UNKNOWN;
   }
   if(read_ready == true){
      char buffer[1024];
      int len = read(fd, buffer, 1024);
      if(strncmp(buffer, "exit\n", 5) == 0){
         do_shutdown = 1;
      }
      /* this causes the deletion of the fd */
      if(strncmp(buffer, "emulate_error\n", 14) == 0){
         do_shutdown = 1;
         return CL_RETVAL_UNKNOWN;
      }
      CL_LOG(CL_LOG_WARNING, "std_in data_ready");
      buffer[len] = '\0';
      pthread_mutex_lock(&data_in_mutex);
      strcat((char*)user_data, buffer); 
      ready = 1;
      pthread_mutex_unlock(&data_in_mutex);
   }
   CL_LOG(CL_LOG_WARNING, "std_in return");
   return CL_RETVAL_OK;
}

/* this is the callback function for std_out */
int fd_out_cb(int fd, bool read_ready, bool write_ready, void* user_data, int err_val){
   if (err_val != 0) {
      return CL_RETVAL_UNKNOWN;
   }
   CL_LOG(CL_LOG_WARNING, "std_out entered");
   if (write_ready == true) {
      char* buffer = NULL;
      CL_LOG(CL_LOG_WARNING, "std_out data_ready");
      buffer = user_data;
      pthread_mutex_lock(&data_mutex);
      if(strcmp((char*)user_data, "error\n") == 0) {
         CL_LOG(CL_LOG_WARNING, "__________App: fd error test______________");
         pthread_mutex_unlock(&data_mutex);
         return CL_RETVAL_UNKNOWN;
      }
      if (write(fd, (char*)user_data, strlen((char*)user_data)) != strlen((char *)user_data)) {
         CL_LOG(CL_LOG_WARNING, "didn't write full buffer");
      }
      if(strcmp((char*)user_data, "SIGTERM") == 0){
         do_shutdown = 1;
      }
      buffer[0] = '\0';
      pthread_mutex_unlock(&data_mutex);
   }
   CL_LOG(CL_LOG_WARNING, "std_out return");
   return CL_RETVAL_OK;
}

/*
 *  main()
 */
extern int main(int argc, char** argv) {
   cl_com_handle_t*    handle        = NULL; 
   cl_com_message_t*   message       = NULL;
   cl_com_endpoint_t*  sender        = NULL;
   cl_ssl_setup_t      ssl_config;
   cl_framework_t      framework     = CL_CT_TCP;
   int                 ret_val       = CL_RETVAL_OK;
   const int           fd_in         = 0;
   const int           fd_out        = 1;
   char                fd_in_data[255];
   char                fd_out_data[255];
   char*               reg           = NULL; 
   int                 error         = CL_RETVAL_OK;   

   reg = "reg";

   fd_in_data[0] = '\0';
   fd_out_data[0] = '\0';
 
   /* check command line argument count */
   if (argc <= 3) {
      usage();
      exit(1);
   }

   /* get service host and port as also the connection mode from command line */
   server_host = argv[1];
   handle_port = atoi(argv[2]);
   if (handle_port <= 0) {
      fprintf(stderr,"need a port number > 0\n");
      usage();
      exit(1);
   }

   if (argv[3]) {
      if (strcmp(argv[3], "SSL") == 0) {
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
   error = cl_com_setup_commlib(CL_RW_THREAD, CL_LOG_OFF, NULL);
 
   if(error != CL_RETVAL_OK){
      fprintf(stderr, "cl_com_setup_commlib failed with: %s\n", cl_get_error_text(error));
   }
   /* setup commlib error function callback */
   cl_com_set_error_func(on_communication_error);
 

   if (framework == CL_CT_SSL) {
      cl_com_specify_ssl_configuration(&ssl_config);
   }

   /* create communication handle */
   handle=cl_com_create_handle(&error,
                               framework,
                               CL_CM_CT_MESSAGE,
                               false,
                               handle_port,
                               CL_TCP_DEFAULT,
                               "client", 0,
                               5, 0 );
   if (handle == NULL) {
      fprintf(stderr, "could not create communication handle with error: %s\n", cl_get_error_text(error));
      cl_com_cleanup_commlib();
      exit(1);
   }
 
   /* print out some info output */
   printf("client running:\n");
   printf("host: \"%s\"\n",  handle->local->comp_host);
   printf("name: \"%s\"\n",  handle->local->comp_name);
   printf("id:   \"%ld\"\n", handle->local->comp_id);
    
   ret_val = cl_com_external_fd_register(handle, fd_in, fd_in_cb, CL_R_SELECT, fd_in_data);
   if (ret_val != CL_RETVAL_OK) {
      printf("could not register stdin-fd: %s\n", cl_get_error_text(ret_val));
      do_shutdown = 1;
   }else{
      printf("stdin-fd was registered\n");
   }

   ret_val = cl_com_external_fd_register(handle, fd_out, fd_out_cb, CL_W_SELECT, fd_out_data);
   if (ret_val != CL_RETVAL_OK) {
      printf("could not register stdout-fd: %s\n", cl_get_error_text(ret_val));
      do_shutdown = 1;
   }else{
      printf("stdout-fd was registered\n");
   }

   /* register at the server */
   ret_val = cl_commlib_send_message(handle,
                                     server_host, "server", 1, 
                                     CL_MIH_MAT_NAK, 
                                     (cl_byte_t**)&reg, strlen(reg)+1,
                                     NULL, 0, 0,
                                     true, false);
                                     
   if (ret_val != CL_RETVAL_OK) {
      do_shutdown = 1;
   }
   /* application main loop */
   while ( do_shutdown == 0 ) {
      CL_LOG(CL_LOG_WARNING, "App: start looping");
      cl_commlib_trigger(handle, 1);
      CL_LOG(CL_LOG_WARNING, "App: trigger was done");
      ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);

      if (message != NULL) {
         CL_LOG(CL_LOG_WARNING, "App: std_out received");
         pthread_mutex_lock(&data_mutex);
         CL_LOG(CL_LOG_WARNING, "App: std_out got mutex");
         strcat(fd_out_data, (const char*)message->message);
         pthread_mutex_unlock(&data_mutex);
         CL_LOG(CL_LOG_WARNING, "App: std_out released mutex");
         cl_com_external_fd_set_write_ready(handle, fd_out);
         CL_LOG(CL_LOG_WARNING, "App: std_out set write ready flag");

         cl_com_free_message(&message);
         cl_com_free_endpoint(&sender);
      }
      pthread_mutex_lock(&data_in_mutex);
      if(ready == 1) {
         cl_byte_t* bp = NULL; 
         CL_LOG(CL_LOG_WARNING, "App: std_in send");
         bp = (cl_byte_t*) fd_in_data;
         cl_commlib_send_message(handle,
                              server_host, "server", 1,
                              CL_MIH_MAT_NAK,
                              &bp, strlen(fd_in_data)+1,
                              NULL, 0, 0,
                              true, false);
         ready = 0;
         fd_in_data[0] = '\0';
      }
      pthread_mutex_unlock(&data_in_mutex);
   }
 
  
   printf("shutting down ...\n");
   cl_com_external_fd_unregister(handle, fd_in);
   cl_com_external_fd_unregister(handle, fd_out);
 
   /* here the application goes down - shutdown communication lib */
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

   /* cleanup commlib */
   cl_com_cleanup_commlib();
   
   return 0;
 }
