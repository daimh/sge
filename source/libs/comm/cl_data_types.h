#ifndef __CL_DATA_TYPES_H
#define __CL_DATA_TYPES_H
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

/* implemented communication frameworks 
   for cl_com_connection_t->framework_type flag
*/
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>

#include "basis_types.h"
#include "comm/lists/cl_lists.h"
#include "comm/cl_xml_parsing.h"

typedef enum cl_com_debug_message_tag_type {
   CL_DMT_MESSAGE = 1,  /* default */
   CL_DMT_APP_MESSAGE,
/* ... */
   CL_DMT_MAX_TYPE  /* this must be the last one */
} cl_com_debug_message_tag_t;

/* typedef for tcp port options */
typedef enum cl_tcp_connect_def {
   CL_TCP_DEFAULT = 1,           /* standard tcp/ip options */
   CL_TCP_RESERVED_PORT          /* use reserved port security */
} cl_tcp_connect_t;

/* typedef for global boolean commlib params */
typedef enum cl_global_settings_params_def {
   CL_COMMLIB_DELAYED_LISTEN = 1
} cl_global_settings_params_t;

/* typedef for Connection Type (CT) */
typedef enum cl_framework_def {
   CL_CT_UNDEFINED = 0,
   CL_CT_TCP,        /* tcp/ip framework */
   CL_CT_SSL         /* secure socket layer */
} cl_framework_t;


typedef enum cl_select_method_def {
   CL_RW_SELECT,
   CL_R_SELECT,
   CL_W_SELECT
} cl_select_method_t;

typedef enum cl_max_count_def {
   CL_ON_MAX_COUNT_CLOSE_AUTOCLOSE_CLIENTS = 5,
   CL_ON_MAX_COUNT_DISABLE_ACCEPT,
   CL_ON_MAX_COUNT_OFF
} cl_max_count_t;


typedef enum cl_host_resolve_method_def {
   CL_SHORT = 1,
   CL_LONG  = 2
} cl_host_resolve_method_t;

typedef enum cl_thread_mode_def {
   CL_NO_THREAD,       /* application must call cl_commlib_trigger() in main loop */
   CL_RW_THREAD        /* enable read thread, write thread, trigger thread, service thread   */
} cl_thread_mode_t;





typedef unsigned char cl_byte_t;
#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif

/* connection types for cl_com_connection_t->connection_type flag */
typedef enum cl_connection_type {
   CL_COM_RECEIVE = 1,
   CL_COM_SEND,
   CL_COM_SEND_RECEIVE,
   CL_COM_UNDEFINED
} cl_connection_t;

/* connection types for cl_com_connection_t->data_write_flag and data_read_flag flag */
typedef enum cl_data_ready_flag_type {
   CL_COM_DATA_READY = 1,
   CL_COM_DATA_NOT_READY
} cl_data_ready_flag_t;

/* connection types for cl_com_connection_t->service_handler_flag flag */
typedef enum cl_service_handler_type {
   CL_COM_SERVICE_HANDLER = 1,
   CL_COM_CONNECTION,
   CL_COM_SERVICE_UNDEFINED
} cl_service_handler_t;

/*connection types for cl_com_connection_t->connection_state */
typedef enum cl_connection_state_type {
   CL_DISCONNECTED = 1,
   CL_OPENING,
   CL_ACCEPTING,
   CL_CONNECTING,
   CL_CONNECTED,
   CL_CLOSING
} cl_connection_state_t;

/*connection types for cl_com_connection_t->connection_sub_state */
typedef enum cl_connection_sub_state_type {
   /* when CL_DISCONNECTED */
   CL_COM_SUB_STATE_UNDEFINED = 1,

   /* when CL_COM_OPENING */
   CL_COM_OPEN_INIT, 
   CL_COM_OPEN_CONNECT,
   CL_COM_OPEN_CONNECT_IN_PROGRESS,
   CL_COM_OPEN_CONNECTED,
   CL_COM_OPEN_SSL_CONNECT_INIT,
   CL_COM_OPEN_SSL_CONNECT,

   /* when CL_ACCEPTING */
   CL_COM_ACCEPT_INIT,
   CL_COM_ACCEPT,

   /* when CL_COM_CONNECTING */
   CL_COM_READ_INIT,
   CL_COM_READ_GMSH,
   CL_COM_READ_CM,
   CL_COM_READ_INIT_CRM,
   CL_COM_READ_SEND_CRM,
   CL_COM_SEND_INIT,
   CL_COM_SEND_CM,
   CL_COM_SEND_READ_GMSH,
   CL_COM_SEND_READ_CRM,

   /* when CL_COM_CONNECTED */
   CL_COM_WORK,
   CL_COM_RECEIVED_CCM,
   CL_COM_SENDING_CCM,
   CL_COM_WAIT_FOR_CCRM,
   CL_COM_SENDING_CCRM,
   CL_COM_DONE,

   /* when CL_CLOSING */
   CL_COM_DO_SHUTDOWN,
   CL_COM_SHUTDOWN_DONE

} cl_connection_sub_state_type;




typedef enum cl_message_state_type {
   CL_MS_UNDEFINED = 1,
   CL_MS_INIT_SND,
   CL_MS_SND_GMSH,
   CL_MS_SND_MIH,
   CL_MS_SND,
   CL_MS_INIT_RCV,
   CL_MS_RCV_GMSH,
   CL_MS_RCV_MIH,
   CL_MS_RCV,
   CL_MS_READY,
   CL_MS_PROTOCOL    /* must be higherst name */
}cl_message_state_t;


/* commlib supports the following SSL connection methods */
typedef enum cl_ssl_method_type {
   CL_SSL_v23 = 1
} cl_ssl_method_t;

typedef enum cl_ssl_verify_mode_type {
   CL_SSL_PEER_NAME = 1,
   CL_SSL_USER_NAME = 2
} cl_ssl_verify_mode_t;

typedef enum cl_ssl_cert_mode_type {
   CL_SSL_PEM_FILE = 1,
   CL_SSL_PEM_BYTE = 2
} cl_ssl_cert_mode_t;   

/* callback for verify peer names */
typedef bool (*cl_ssl_verify_func_t)  (cl_ssl_verify_mode_t mode, bool service_mode, const char* value );

typedef enum cl_debug_client_def {
   CL_DEBUG_CLIENT_OFF = 0,
   CL_DEBUG_CLIENT_ALL,
   CL_DEBUG_CLIENT_MSG,
   CL_DEBUG_CLIENT_APP
} cl_debug_client_t;



/*
 *  this structure has the following setup functions:
 *
 *  cl_com_create_debug_client_setup(),
 *  cl_com_free_debug_client_setup() 
 *
 */

typedef struct cl_debug_client_setup_type {
   cl_debug_client_t dc_mode;            /* debug_client_mode */
   bool         dc_dump_flag;       /* flag for sending message data */
   int               dc_app_log_level;   /* application log level */
   cl_raw_list_t*    dc_debug_list;      /* debug list */
} cl_debug_client_setup_t;
/*
 *  this structure has the following setup functions:
 *
 *  cl_com_create_ssl_setup(),
 *  cl_com_dup_ssl_setup() and
 *  cl_com_free_ssl_setup() 
 *
 */
/* TODO: 
 * a) make cl_com_create_ssl_setup() function fail if not unused parameters are missing 
 * b) make TODO parameters check
 */
typedef struct cl_ssl_setup_type {
   cl_ssl_cert_mode_t ssl_cert_mode;           /*  CL_SSL_PEM_FILE or CL_SSL_PEM_BYTE                                          */
                                               /*  if (ssl_cert_mode == CL_SSL_PEM_FILE) {                                     */
                                               /*     ssl_cert_pem_file contains the pem cert file name                        */
                                               /*     ssl_key_pem_file contains the pem key file name                          */
                                               /*  } else {                                                                    */
                                               /*     ssl_cert_pem_file contains the pem cert file name                        */
                                               /*     ssl_key_pem_file contains the pem key file name                          */
                                               /*  }                                                                           */
   cl_ssl_method_t ssl_method;                 /*  used for call to SSL_CTX_new() as parameter       */
   char*           ssl_CA_cert_pem_file;       /*  CA certificate file                           ->ca_cert_file<-              */
   char*           ssl_CA_key_pem_file;        /*  private key file of CA                        ->ca_key_file (not used)<-    */
   char*           ssl_cert_pem_file;          /*  certificate file                              ->cert_file<-                 */
   char*           ssl_key_pem_file;           /*  key file                                      ->key_file<-                  */
   char*           ssl_rand_file;              /*  rand file (if RAND_status() not ok)           ->rand_file<-                 */
   char*           ssl_reconnect_file;         /*  file for reconnect data                       ->reconnect_file (not used)<- */
   char*           ssl_crl_file;               /*  file for revocation list */
   unsigned long   ssl_refresh_time;           /*  key alive time for connections (for services) ->refresh_time (not used)<-   */
   char*           ssl_password;               /*  password for encrypted keyfiles               ->not used<-                  */
   cl_ssl_verify_func_t  ssl_verify_func;       /*  function callback for peer user/name check */
} cl_ssl_setup_t;



typedef struct cl_com_handle_statistic_type {
   struct timeval   last_update;               /* last calculation time */
   unsigned long    new_connections;           /* nr of new connections since last_update */
   unsigned long    access_denied;             /* nr of connections, where access was denied */
   unsigned long    nr_of_connections;         /* nr of open connections */
   unsigned long    bytes_sent ;               /* bytes send since last_update */
   unsigned long    bytes_received;            /* bytes received since last_update */
   unsigned long    real_bytes_sent ;          /* bytes send since last_update */
   unsigned long    real_bytes_received;       /* bytes received since last_update */
   unsigned long    unsend_message_count;      /* nr of messages to send */
   unsigned long    unread_message_count;      /* nr of buffered received messages, waiting for application to pick it up */
   unsigned long    application_status;        /* status of application */
   char*            application_info;          /* application info message */
} cl_com_handle_statistic_t;

typedef struct cl_com_connection_type cl_com_connection_t;

typedef struct cl_com_handle {
   cl_ssl_setup_t*           ssl_setup;         /* used for SSL framework */

   cl_debug_client_setup_t*  debug_client_setup; /* used for debug clients */

   cl_framework_t            framework;        /* framework type CL_CT_TCP, CL_CT_SSL */
   cl_tcp_connect_t          tcp_connect_mode; /* used for reserved port selection for tcp connect */
   cl_xml_connection_type_t  data_flow_type;   /* data_flow type CL_CM_CT_STREAM, CL_CM_CT_MESSAGE  */
   bool                 service_provider; /* if true this component will provide a service for clients (server port) */

   /* connect_port OR service_port is always 0 !!! - CR */
   int connect_port;                /* used port number to connect to other service */
   int service_port;                /* local used service port */

   cl_com_endpoint_t* local;        /* local endpoint id of this handle */
   cl_com_handle_statistic_t* statistic; /* statistic data of handle */

   /* Threads for CL_RW_THREAD */
   cl_thread_condition_t* app_condition;   /* triggered when there are messages to read for application (by read thread) */
   cl_thread_condition_t* read_condition;  /* condition variable for data write */
   cl_thread_condition_t* write_condition; /* condition variable for data read */
   cl_thread_settings_t*  service_thread;  /* pointer to cl_com_handle_service_thread() thread pointer */
   cl_thread_settings_t*  read_thread;   
   cl_thread_settings_t*  write_thread;
   /* Threads for CL_RW_THREAD done */

   
   pthread_mutex_t* messages_ready_mutex;
   unsigned long messages_ready_for_read;

   pthread_mutex_t* connection_list_mutex;
   cl_raw_list_t* connection_list;  /* connections of this handle */
   cl_raw_list_t* allowed_host_list; /* string list with hostnames allowed to connect */
   cl_raw_list_t* file_descriptor_list; /* list with all registered file descriptors */
   unsigned long next_free_client_id;

   cl_raw_list_t* send_message_queue;     /* used as queue for application messages which have to 
                                             be send to a connection ( used in cl_commlib_send_message()) */
   cl_raw_list_t* received_message_queue; /* used as queue for application messages which are ready to be
                                             handled over to the application ( used in cl_commlib_receive_message() 
                                             This message queue must have a "garbage collector", removing 
                                             references to destroyed objects!!!  
                                           */

   unsigned long max_open_connections; /* maximum number of open connections  */
   cl_max_count_t max_con_close_mode;  /*  state of auto close at max connection count */
   cl_xml_connection_autoclose_t auto_close_mode; /* used to enable/disable autoclose of connections opend from this handle to services */
   int max_write_threads;    /* maximum number of send threads */
   int max_read_threads;     /* maximum number of receive threas */
   int select_sec_timeout;
   int select_usec_timeout;
   int connection_timeout;   /* timeout to shutdown connected clients when no messages arive */ 
   int close_connection_timeout; /* timeout for connection to delete unread messages after connection shutdown */
   int read_timeout;
   int write_timeout;
   int open_connection_timeout; 
   int acknowledge_timeout;
   int message_timeout;      /* timeout when to trash message from the received message queue */
   int synchron_receive_timeout;
   int last_heard_from_timeout;      /* do not use, just for compatibility */
   
   /* service specific */
   int do_shutdown;                        /* set when this handle wants to shutdown */
   bool max_connection_count_reached;       /* set when max connection count is reached */
   bool max_connection_count_found_connection_to_close; /* set if we found a connection to close when max_connection_count_reached is set */
   cl_com_connection_t* last_receive_message_connection;  /* this is the last connection from connection list, where cl_comlib_receive_message() was called */
   long shutdown_timeout;                   /* used when shutting down handle */
   cl_com_connection_t* service_handler;    /* service handler of this handle */
   struct timeval start_time;
   struct timeval last_statistic_update_time;      /* used in service thread */
   struct timeval last_message_queue_cleanup_time; /* used in service thread */
} cl_com_handle_t;

typedef struct cl_com_poll {
   struct pollfd*        poll_array;     /* array of pollfd structs */
   cl_com_connection_t** poll_con;       /* array of connection pointers */
   unsigned long         poll_fd_count;  /* nr of malloced pollfd structs and connection pointers */
} cl_com_poll_t;

typedef struct cl_com_hostent {
   struct hostent *he;              /* pointer of type struct hostent (defined in netdb.h) */

#if 0
   /* tried to store gethostbyname_r from unix return values into this struct, but
      now sge_copy_hostent() and sge_gethostbyname() from utilib are used in order 
      to create copies of struct hostent.

      So it is not necessary to save this data anymore */
   char*  he_data_buffer;           /* all struct member pointers point to data in this buffer */
#endif

} cl_com_hostent_t;

/*  the hostent struct should be defined in the following way (system header)
      struct hostent {
         char    *h_name;          canonical name of host 
         char    **h_aliases;      alias list 
         int     h_addrtype;       host address type 
         int     h_length;         length of address 
         char    **h_addr_list;    list of addresses 
     };
*/

typedef struct cl_com_host_spec_type {
   cl_com_hostent_t* hostent;
   struct in_addr*   in_addr;
   char*             unresolved_name;
   char*             resolved_name;
   int               resolve_error;     /* CL_RETVAL_XXX from  cl_com_gethostbyname() call */
   long              last_resolve_time; 
   long              creation_time;

} cl_com_host_spec_t;

/* callback function for external file descriptors.
   The return value has to be CL_RETVAL_OK, otherwise
   the external file descriptor will be removed form the list.

   int fd                   the external file descriptor
   bool read_ready     states if the fd is ready for reading
   bool write_ready    states if the fd is ready for writing
   void* user_data          void pointer to some data of the application
   int err_val              states if an error occured while poll/select of the external fd
*/
typedef int   (*cl_fd_func_t)   (int fd, bool read_ready, bool write_ready, void* user_data, int err_val);
typedef struct cl_com_fd_data_type {
   int                  fd;
   cl_select_method_t   select_mode;         /* select mode of the fd (read, write, read/write) */
   bool            read_ready;          /* is fd ready for read, write, read/write */
   bool            write_ready;         /* is fd ready for read, write, read/write */
   bool            ready_for_writing;   /* the application has data to write */
   cl_fd_func_t         callback;
   void*                user_data;
} cl_com_fd_data_t;




typedef struct cl_com_message_type {
   cl_message_state_t       message_state;
   cl_xml_mih_data_format_t message_df;
   cl_xml_ack_type_t        message_mat;
   int                      message_ack_flag;
   cl_com_SIRM_t*           message_sirm;  /* if NOT NULL this was the response to a SIM */
   unsigned long            message_tag;
   unsigned long            message_id;
   unsigned long            message_response_id;  /* if set, this message is a response for this message_id */
   unsigned long            message_length;
   unsigned long            message_snd_pointer;
   unsigned long            message_rcv_pointer;
   struct timeval           message_receive_time;
   struct timeval           message_remove_time;
   struct timeval           message_send_time;
   struct timeval           message_insert_time;
   cl_byte_t*               message;
} cl_com_message_t;


typedef struct cl_com_con_statistic_type {
   struct timeval   last_update;                 /* last calculation time */
   unsigned long    bytes_sent ;                 /* bytes send since last_update */
   unsigned long    bytes_received;              /* bytes received since last_update */
   unsigned long    real_bytes_sent;
   unsigned long    real_bytes_received;
} cl_com_con_statistic_t;



struct cl_com_connection_type {

   bool             check_endpoint_flag;  /* set when an endpoint should get a SIM to check availability of endpoint */
                                               /* (if CL_TRUE, there is already a sim/sirm check ongoing) */
   unsigned long         check_endpoint_mid;   /* contains the mid of the sim when sent */

   cl_error_func_t       error_func;   /* if not NULL this function is called on errors */
   cl_tag_name_func_t    tag_name_func; /* if not NULL this function is called for debug clients to get tag id name */

   cl_com_endpoint_t*    remote;   /* dst on local host in Connect Message (CM) */
   cl_com_endpoint_t*    local;    /* src on local host in Connect Message (CM) */
   cl_com_endpoint_t*    client_dst; /* dst where client wants to connect to. This info is from Connect Message (CM) */

   unsigned long         last_send_message_id;
   unsigned long         last_received_message_id;

   cl_raw_list_t*        received_message_list;
   cl_raw_list_t*        send_message_list;

   cl_com_handle_t*      handler;           /* this points to the handler of the connection */


   cl_framework_t        framework_type;          /* CL_CT_TCP, ... */
   cl_tcp_connect_t      tcp_connect_mode;        /* used for reserved port socket creation for TCP connect call */ 
   cl_connection_t       connection_type;         /* CL_COM_RECEIVE, CL_COM_SEND or CL_COM_SEND_RECEIVE  */
   cl_service_handler_t  service_handler_flag;    /* CL_COM_SERVICE_HANDLER or CL_COM_CONNECTION or CL_COM_SERVICE_UNDEFINED*/
   cl_data_ready_flag_t  data_write_flag;         /* CL_COM_DATA_READY or CL_COM_DATA_NOT_READY */ 
   cl_data_ready_flag_t  fd_ready_for_write;      /* set by cl_com_open_connection_request_handler() when data_write_flag is CL_COM_DATA_READY 
                                                     and the write is possible (values are CL_COM_DATA_READY or CL_COM_DATA_NOT_READY) */
   cl_data_ready_flag_t  data_read_flag;          /* CL_COM_DATA_READY or CL_COM_DATA_NOT_READY */

   cl_connection_state_t         connection_state;        /* CL_COM_DISCONNECTED,CL_COM_CLOSING ,CL_COM_CONNECTED ,CL_COM_CONNECTING */
   cl_connection_sub_state_type  connection_sub_state;    /* depends on connection_state */

   bool     was_accepted;            /* is set when this is a client connection (from accept() ) */
   bool     was_opened;              /* is set when this connection was opened (with open connection) by connect() */ 
   char*         client_host_name;        /* this is the resolved client host name */
   cl_xml_connection_status_t crm_state;  /* state of connection response message (if server) */
   char*         crm_state_error;         /* error text if crm_state is CL_CRM_CS_DENIED or larger */
   
   /* dataflow */
   cl_xml_connection_type_t      data_flow_type;       /* CL_CM_CT_STREAM or CL_CM_CT_MESSAGE */   
   cl_xml_data_format_t          data_format_type;     /* CL_CM_DF_BIN or CL_CM_DF_XML */
 
   /* data buffer */
   unsigned long  data_buffer_size;             /* connection data buffer size for read/write messages */
   cl_byte_t*     data_read_buffer;             /* connection data buffer for read operations */
   cl_byte_t*     data_write_buffer;            /* connection data buffer for write operations */
   cl_com_GMSH_t* read_gmsh_header;             /* used to store gmsh data length for reading */

   long          read_buffer_timeout_time;     /* timeout for current read */
   long          write_buffer_timeout_time;    /* timeout for current write */

   unsigned long data_write_buffer_pos;        /* actual position in data write buffer */
   unsigned long data_write_buffer_processed;  /* actual position in data write buffer which is processed */
   unsigned long data_write_buffer_to_send;        /* position of last data byte to write */

   unsigned long data_read_buffer_pos;         /* actual position in data read buffer */
   unsigned long data_read_buffer_processed;   /* actual position in data read buffer which is processed */
 
   struct timeval last_transfer_time;           /* time when last message arived/was sent */
   struct timeval connection_close_time;        /* time when connection was closed ( received CCRM ) */
   struct timeval connection_connect_time;      /* time when connection was established (received CM/CRM) */
   long           shutdown_timeout;             /* used for shutdown of connection */
 
   /* connection specific */
   cl_com_con_statistic_t* statistic;
   cl_xml_connection_autoclose_t auto_close_type;       /* CL_CM_AC_ENABLED, CL_CM_AC_DISABLED */  
   bool     is_read_selected;                 /* if set to CL_TRUE this connection is not deleted */
   bool     is_write_selected;                /* if set to CL_TRUE this connection is not deleted */

   void*         com_private;
};



#endif /* __CL_DATA_TYPES_H */
