#ifndef __CL_TCP_FRAMEWORK_H
#define __CL_TCP_FRAMEWORK_H

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

#include <netinet/in.h>
#include "comm/lists/cl_lists.h"
#include "comm/cl_data_types.h"

int cl_com_tcp_get_fd(cl_com_connection_t* connection, int* fd);
int cl_com_tcp_get_service_port(cl_com_connection_t* connection, int* port);
int cl_com_tcp_get_connect_port(cl_com_connection_t* connection, int* port);
int cl_com_tcp_set_connect_port(cl_com_connection_t* connection, int port);
int cl_com_tcp_get_client_socket_in_port(cl_com_connection_t* connection, int* port);


int cl_com_tcp_setup_connection(cl_com_connection_t** connection, 
                                int server_port, 
                                int connect_port, 
                                cl_xml_connection_type_t data_flow_type, 
                                cl_xml_connection_autoclose_t auto_close_mode,
                                cl_framework_t                 framework_type,
                                cl_xml_data_format_t           data_format_type,
                                cl_tcp_connect_t               tcp_connect_mode);

int cl_com_tcp_open_connection(cl_com_connection_t* connection, int timeout);
int cl_com_tcp_close_connection(cl_com_connection_t** connection);

int cl_com_tcp_open_connection_request_handler(cl_com_poll_t*       poll_handle,
                                               cl_com_handle_t*     handle,
                                               cl_raw_list_t*       connection_list,
                                               cl_com_connection_t* service_connection,
                                               int                  timeout_val_sec,
                                               int                  timeout_val_usec,
                                               cl_select_method_t   select_mode );


int cl_com_tcp_connection_request_handler_setup(cl_com_connection_t* connection, bool only_prepare_service);
int cl_com_tcp_connection_request_handler(cl_com_connection_t* connection,cl_com_connection_t** new_connection);
int cl_com_tcp_connection_request_handler_cleanup(cl_com_connection_t* connection); /* CR check */

/* connection establish help functions */
int cl_com_tcp_read_GMSH(cl_com_connection_t* connection, unsigned long *only_one_read);
int cl_com_tcp_write(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long *only_one_write);
int cl_com_tcp_read(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long *only_one_read);





#endif /* __CL_TCP_FRAMEWORK_H */

