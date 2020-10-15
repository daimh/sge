#ifndef __CL_COMMUNICATION_H
#define __CL_COMMUNICATION_H

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

#include "comm/lists/cl_list_types.h"
#include "comm/cl_data_types.h"
#include "comm/cl_xml_parsing.h"
#include "comm/cl_connection_list.h"

#define CL_DEFINE_READ_TIMEOUT                       30
#define CL_DEFINE_WRITE_TIMEOUT                      30
#define CL_DEFINE_ACK_TIMEOUT                        60
#define CL_DEFINE_MESSAGE_TIMEOUT                    3600 /* default timeout for trashing received but not application fetched messages */
#define CL_DEFINE_GET_CLIENT_CONNECTION_DATA_TIMEOUT 60   /* default timeout for accepting a connection */
#define CL_DEFINE_DELETE_MESSAGES_TIMEOUT_AFTER_CCRM 60   /* default timeout for unread message deletion after connection shutdown */
#define CL_DEFINE_SYNCHRON_RECEIVE_TIMEOUT           60   /* default timeout for synchron send messages */
#define CL_DEFINE_CLIENT_CONNECTION_LIFETIME         600  /* Cut off connection when client is not active for this time */
#define CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT            30   /* timeout for marking duplicate application error messages */


#define CL_DEFINE_DATA_BUFFER_SIZE                   1024 * 4           /* 4 KB buffer for reading/writing messages */
#if 0
/* TODO: enable this when application code is not using u_short with client ids !!! */
#define CL_DEFINE_MAX_MESSAGE_ID                     4294967295UL       /* max unsigned long value for a 32 bit system */
#else
#define CL_DEFINE_MAX_MESSAGE_ID                     65535              /* max unsigned short value */
#endif

#ifdef MAXHOSTNAMELEN
#define CL_MAXHOSTNAMELEN_LENGTH MAXHOSTNAMELEN
#else
#define CL_MAXHOSTNAMELEN_LENGTH 255
#endif


int  cl_com_compare_endpoints(cl_com_endpoint_t* endpoint1, cl_com_endpoint_t* endpoint2);
void cl_com_dump_endpoint(cl_com_endpoint_t* endpoint, const char* text);

int cl_com_endpoint_list_refresh(cl_raw_list_t* endpoint_list);

/* debug client functions */
int cl_com_add_debug_message(cl_com_connection_t* connection, const char* message, cl_com_message_t* ms);

int cl_com_gethostname(char **unique_hostname,struct in_addr *copy_addr,struct hostent **he_copy, int* system_error_value);
int cl_com_host_list_refresh(cl_raw_list_t* host_list);
int cl_com_cached_gethostbyname(const char *hostname, char **unique_hostname, struct in_addr *copy_addr,struct hostent **he_copy, int* system_error_value);
int cl_com_cached_gethostbyaddr( struct in_addr *addr, char **unique_hostname,struct hostent **he_copy,int* system_error_val );
char* cl_com_get_h_error_string(int h_error);
int cl_com_compare_hosts(const char* host1, const char* host2);
int cl_com_set_resolve_method(cl_host_resolve_method_t method, char* local_domain_name);

int cl_com_free_handle_statistic(cl_com_handle_statistic_t** statistic);
int cl_com_free_hostent(cl_com_hostent_t **hostent_p);                    /* CR check */
int cl_com_free_hostspec(cl_com_host_spec_t **hostspec);
int cl_com_print_host_info(cl_com_hostent_t *hostent_p );                 /* CR check */


int cl_com_create_debug_client_setup(cl_debug_client_setup_t** new_setup,
                                     cl_debug_client_t dc_mode,
                                     bool         dc_dump_flag,
                                     int               dc_app_log_level);

int cl_com_free_debug_client_setup(cl_debug_client_setup_t** new_setup);

int cl_com_create_ssl_setup(cl_ssl_setup_t** new_setup,
                            cl_ssl_cert_mode_t  ssl_cert_mode,
                            cl_ssl_method_t  ssl_method,
                            const char*            ssl_CA_cert_pem_file,
                            const char*            ssl_CA_key_pem_file,
                            const char*            ssl_cert_pem_file,
                            const char*            ssl_key_pem_file,
                            const char*            ssl_rand_file,
                            const char*            ssl_reconnect_file,
                            const char*            ssl_crl_file,
                            unsigned long    ssl_refresh_time,
                            const char*            ssl_password,
                            cl_ssl_verify_func_t  ssl_verify_func);

int cl_com_dup_ssl_setup(cl_ssl_setup_t** new_setup, cl_ssl_setup_t* source);
int cl_com_free_ssl_setup(cl_ssl_setup_t** del_setup);

const char* cl_com_get_framework_type(cl_com_connection_t* connection);
const char* cl_com_get_connection_type(cl_com_connection_t* connection);
const char* cl_com_get_service_handler_flag(cl_com_connection_t* connection);
const char* cl_com_get_data_write_flag(cl_com_connection_t* connection);
const char* cl_com_get_data_read_flag(cl_com_connection_t* connection);
const char* cl_com_get_connection_state(cl_com_connection_t* connection);
const char* cl_com_get_connection_sub_state(cl_com_connection_t* connection);
const char* cl_com_get_data_flow_type(cl_com_connection_t* connection);

/* This can be called by an signal handler to trigger abort of communications */
void cl_com_ignore_timeouts(bool flag); 
bool cl_com_get_ignore_timeouts_flag(void);


/* message functions */
int cl_com_setup_message(cl_com_message_t** message, cl_com_connection_t* connection, cl_byte_t* data,unsigned long size, cl_xml_ack_type_t ack_type, unsigned long response_id, unsigned long tag);   /* *message must be zero */
int cl_com_create_message(cl_com_message_t** message);
int cl_com_free_message(cl_com_message_t** message);

int cl_com_create_connection(cl_com_connection_t** connection);
/*
int cl_com_free_connection(cl_com_connection_t** connection);
   use cl_com_close_connection();
*/

/* after this line are the main functions used by lib user */
/* ======================================================= */


int cl_com_connection_complete_accept(cl_com_connection_t* connection,
                                      long timeout);

int cl_com_connection_complete_shutdown(cl_com_connection_t* connection);


int cl_com_open_connection(cl_com_connection_t* connection, 
                                            int timeout, 
                             cl_com_endpoint_t* remote_endpoint, 
                             cl_com_endpoint_t* local_endpoint);    /* CR check */

int cl_com_close_connection(cl_com_connection_t** connection);  /* CR check */

int cl_com_read_GMSH(cl_com_connection_t* connection, unsigned long *only_one_read);
int cl_com_read(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long* only_one_read);
int cl_com_write(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long *only_one_write);



/* This functions need service connection pointer = cl_com_connection_request_handler_setup */
/* ======================================================================================== */

int cl_com_connection_get_connect_port(cl_com_connection_t* connection, int* port);
int cl_com_connection_set_connect_port(cl_com_connection_t* connection, int port);

int cl_com_connection_get_service_port(cl_com_connection_t* connection, int* port);
int cl_com_connection_get_fd(cl_com_connection_t* connection, int* fd);
int cl_com_connection_get_client_socket_in_port(cl_com_connection_t* connection, int* port);


/* setup service */
int cl_com_connection_request_handler_setup(cl_com_connection_t* connection,
                                            cl_com_endpoint_t* local_endpoint );

/* check for new service connection clients */
int cl_com_connection_request_handler(cl_com_connection_t* connection,
                                      cl_com_connection_t** new_connection);
/* cleanup service */
int cl_com_connection_request_handler_cleanup(cl_com_connection_t* connection);
int cl_com_open_connection_request_handler(cl_com_poll_t* poll_handle,
                                           cl_com_handle_t* handle,
                                           int timeout_val_sec,
                                           int timeout_val_usec,
                                           cl_select_method_t select_mode );

int cl_com_free_poll_array(cl_com_poll_t* poll_handle);
int cl_com_malloc_poll_array(cl_com_poll_t* poll_handle, unsigned long nr_of_malloced_connections);

int cl_com_connection_complete_request(cl_raw_list_t* connection_list, cl_connection_list_elem_t* elem, long timeout, cl_select_method_t select_mode );


#endif /* __CL_COMMUNICATION_H */

