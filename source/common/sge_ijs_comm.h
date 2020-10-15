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
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/* Portions of this code are Copyright 2011 Univa Inc. */
/*___INFO__MARK_END__*/

#include "uti/sge_dstring.h"

#include "comm/cl_data_types.h"
#include "comm/cl_commlib.h"
#include "comm/cl_connection_list.h"

#define BUFSIZE              64*1024
#define COMM_HANDLE cl_com_handle_t

#define STDIN_DATA_MSG               0
#define STDOUT_DATA_MSG              1
#define STDERR_DATA_MSG              2
#define WINDOW_SIZE_CTRL_MSG         3
#define REGISTER_CTRL_MSG            4
#define UNREGISTER_CTRL_MSG          5
#define UNREGISTER_RESPONSE_CTRL_MSG 6
#define SETTINGS_CTRL_MSG            7
#define SUSPEND_CTRL_MSG             8
#define UNSUSPEND_CTRL_MSG           9
#define STDIN_CLOSE_MSG              10

#define COMM_RETVAL_OK                    0
#define COMM_INVALID_PARAMETER            1
#define COMM_CANT_SETUP_COMMLIB           2
#define COMM_CANT_CLEANUP_COMMLIB         3
#define COMM_CANT_CREATE_HANDLE           4
#define COMM_CANT_SHUTDOWN_HANDLE         5
#define COMM_CANT_OPEN_CONNECTION         6 
#define COMM_CANT_CLOSE_CONNECTION        7
#define COMM_CANT_SETUP_SSL               8
#define COMM_CANT_SET_CONNECTION_PARAM    9
#define COMM_CANT_SET_IGNORE_TIMEOUTS    10
#define COMM_GOT_TIMEOUT                 11
#define COMM_CANT_TRIGGER                12
#define COMM_CANT_SEARCH_ENDPOINT        13
#define COMM_CANT_LOCK_CONNECTION_LIST   14
#define COMM_CANT_UNLOCK_CONNECTION_LIST 15
#define COMM_CANT_RECEIVE_MESSAGE        16
#define COMM_CANT_FREE_MESSAGE           17
#define COMM_CANT_GET_CLIENT_STATUS      18
#define COMM_NO_SELECT_DESCRIPTORS       19
#define COMM_CONNECTION_NOT_FOUND        20
#define COMM_NO_SECURITY_COMPILED_IN     21
#define COMM_SELECT_INTERRUPT            22
#define COMM_ENDPOINT_NOT_UNIQUE         23
#define COMM_ACCESS_DENIED               24
#define COMM_SYNC_RECEIVE_TIMEOUT        25
#define COMM_NO_MESSAGE_AVAILABLE        26

typedef struct recv_message_s {
   unsigned char type;
   char *data;
   struct winsize ws;
   cl_com_message_t *cl_message;
} recv_message_t;


int comm_init_lib(dstring *err_msg);
int comm_cleanup_lib(dstring *err_msg);

int comm_open_connection(bool                 b_server, 
                         bool                 b_secure,
                         const char           *this_component,
                         int                  port, 
                         const char           *other_component,
                         char                 *hostname,
                         const char           *user_name,
                         COMM_HANDLE          **handle, 
                         dstring              *err_msg);

int comm_get_application_error(dstring *err_msg);
int comm_shutdown_connection(COMM_HANDLE *handle,
                             const char *component_name,
                             char *hostname,
                             dstring *err_msg);


int comm_set_connection_param(COMM_HANDLE *handle, int param, int value,
                              dstring *err_msg);
int comm_ignore_timeouts(bool b_ignore, dstring *err_msg);

int comm_wait_for_connection(COMM_HANDLE *handle, const char *component, 
                             int wait_secs, const char **host, dstring *err_msg);
int comm_wait_for_no_connection(COMM_HANDLE *handle, const char *component, 
                                int wait_secs, dstring *err_msg);
int comm_get_connection_count(const COMM_HANDLE *handle, dstring *err_msg);

int comm_trigger(COMM_HANDLE *handle, int synchron, dstring *err_msg);


unsigned long comm_write_message(COMM_HANDLE *handle,
                  const char *unresolved_hostname,
                  const char *component_name,
                  unsigned long component_id,
                  unsigned char *buffer, 
                  unsigned long size, 
                  unsigned char type,
                  dstring *err_msg);

int comm_flush_write_messages(COMM_HANDLE *handle, dstring *err_msg);

int comm_recv_message(COMM_HANDLE *handle, 
                 bool b_synchron, 
                 recv_message_t *recv_mess, 
                 dstring *err_msg);

int comm_free_message(recv_message_t *recv_mess, dstring *err_msg);
int check_client_alive(COMM_HANDLE *handle,
                       const char *component_name,
                       char *hostname,
                       dstring *err_msg);

