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
#include <stdlib.h>

#include "basis_types.h"

#include "comm/lists/cl_errors.h"
#include "comm/lists/msg_commlistslib.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_is_commlib_error()"
int cl_is_commlib_error(int error_id) {
   if ( error_id >= CL_RETVAL_OK && error_id < CL_RETVAL_LAST_ID) {
      return 1;
   } else {
      return 0;
   }
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_get_error_text()"
const char* cl_get_error_text(int error_id) {
   switch(error_id) {
      case CL_RETVAL_OK                        : {
         return MSG_CL_RETVAL_OK;
      }
      case CL_RETVAL_MALLOC                    : {
         return MSG_CL_RETVAL_MALLOC;
      }
      case CL_RETVAL_PARAMS                    : {
         return MSG_CL_RETVAL_PARAMS;
      }
      case CL_RETVAL_UNKNOWN                   : {
         return MSG_CL_RETVAL_UNKNOWN;
      }
      case CL_RETVAL_MUTEX_ERROR               : {
         return MSG_CL_RETVAL_MUTEX_ERROR;
      }
      case CL_RETVAL_MUTEX_CLEANUP_ERROR       : {
         return MSG_CL_RETVAL_MUTEX_CLEANUP_ERROR;
      }
      case CL_RETVAL_MUTEX_LOCK_ERROR          : {
         return MSG_CL_RETVAL_MUTEX_LOCK_ERROR;
      }
      case CL_RETVAL_MUTEX_UNLOCK_ERROR        : {
         return MSG_CL_RETVAL_MUTEX_UNLOCK_ERROR;
      }
      case CL_RETVAL_CONDITION_ERROR           : {
         return MSG_CL_RETVAL_CONDITION_ERROR;
      }
      case CL_RETVAL_CONDITION_CLEANUP_ERROR   : {
         return MSG_CL_RETVAL_CONDITION_CLEANUP_ERROR;
      }
      case CL_RETVAL_CONDITION_WAIT_TIMEOUT    : {
         return MSG_CL_RETVAL_CONDITION_WAIT_TIMEOUT;
      }
      case CL_RETVAL_CONDITION_SIGNAL_ERROR    : {
         return MSG_CL_RETVAL_CONDITION_SIGNAL_ERROR;
      }
      case CL_RETVAL_THREAD_CREATE_ERROR       : {
         return MSG_CL_RETVAL_THREAD_CREATE_ERROR;
      }
      case CL_RETVAL_THREAD_START_TIMEOUT      : {
         return MSG_CL_RETVAL_THREAD_START_TIMEOUT;
      }
      case CL_RETVAL_THREAD_NOT_FOUND          : {
         return MSG_CL_RETVAL_THREAD_NOT_FOUND;
      }
      case CL_RETVAL_THREAD_JOIN_ERROR         : {
         return MSG_CL_RETVAL_THREAD_JOIN_ERROR;
      }
      case CL_RETVAL_THREAD_CANCELSTATE_ERROR  : {
         return MSG_CL_RETVAL_THREAD_CANCELSTATE_ERROR;
      }
      case CL_RETVAL_LOG_NO_LOGLIST            : {
         return MSG_CL_RETVAL_LOG_NO_LOGLIST;
      }
      case CL_RETVAL_CONNECTION_NOT_FOUND      : {
         return MSG_CL_RETVAL_CONNECTION_NOT_FOUND;
      }
      case CL_RETVAL_HANDLE_NOT_FOUND          : {
         return MSG_CL_RETVAL_HANDLE_NOT_FOUND;
      }
      case CL_RETVAL_THREADS_ENABLED       : {
         return MSG_CL_RETVAL_THREADS_ENABLED;
      }
      case CL_RETVAL_NO_MESSAGE                : {
         return MSG_CL_RETVAL_NO_MESSAGE;
      }
      case CL_RETVAL_CREATE_SOCKET             : {
         return MSG_CL_RETVAL_CREATE_SOCKET;
      }
      case CL_RETVAL_CONNECT_ERROR             : {
         return MSG_CL_RETVAL_CONNECT_ERROR;
      }
      case CL_RETVAL_CONNECT_TIMEOUT           : {
         return MSG_CL_RETVAL_CONNECT_TIMEOUT;
      }
      case CL_RETVAL_NOT_OPEN                  : {
         return MSG_CL_RETVAL_NOT_OPEN;
      }
      case CL_RETVAL_SEND_ERROR                : {
         return MSG_CL_RETVAL_SEND_ERROR;
      }
      case CL_RETVAL_BIND_SOCKET               : {
         return MSG_CL_RETVAL_BIND_SOCKET;
      }
      case CL_RETVAL_SELECT_ERROR              : {
         return MSG_CL_RETVAL_SELECT_ERROR;
      }
      case CL_RETVAL_PIPE_ERROR                : {
         return MSG_CL_RETVAL_PIPE_ERROR;
      }
      case CL_RETVAL_GETHOSTNAME_ERROR         : {
         return MSG_CL_RETVAL_GETHOSTNAME_ERROR;
      }
      case CL_RETVAL_IP_NOT_RESOLVED_ERROR:
      case CL_RETVAL_GETHOSTADDR_ERROR         : {
         return MSG_CL_RETVAL_GETHOSTADDR_ERROR;
      }
      case CL_RETVAL_SEND_TIMEOUT              : {
         return MSG_CL_RETVAL_SEND_TIMEOUT;
      }
      case CL_RETVAL_READ_TIMEOUT              : {
         return MSG_CL_RETVAL_READ_TIMEOUT;
      }
      case CL_RETVAL_UNDEFINED_FRAMEWORK       : {
         return MSG_CL_RETVAL_UNDEFINED_FRAMEWORK;
      }
      case CL_RETVAL_NOT_SERVICE_HANDLER       : {
         return MSG_CL_RETVAL_NOT_SERVICE_HANDLER;
      }
      case CL_RETVAL_NO_FRAMEWORK_INIT         : {
         return MSG_CL_RETVAL_NO_FRAMEWORK_INIT;
      }
      case CL_RETVAL_SETSOCKOPT_ERROR          : {
         return MSG_CL_RETVAL_SETSOCKOPT_ERROR;
      }
      case CL_RETVAL_FCNTL_ERROR               : {
         return MSG_CL_RETVAL_FCNTL_ERROR;
      }
      case CL_RETVAL_LISTEN_ERROR              : {
         return MSG_CL_RETVAL_LISTEN_ERROR;
      }
      case CL_RETVAL_NEED_EMPTY_FRAMEWORK      : {
         return MSG_CL_RETVAL_NEED_EMPTY_FRAMEWORK;
      }
      case CL_RETVAL_LOCK_ERROR                : {
         return MSG_CL_RETVAL_LOCK_ERROR;
      }
      case CL_RETVAL_UNLOCK_ERROR              : {
         return MSG_CL_RETVAL_UNLOCK_ERROR;
      }
      case CL_RETVAL_WRONG_FRAMEWORK           : {
         return MSG_CL_RETVAL_WRONG_FRAMEWORK;
      }
      case CL_RETVAL_READ_ERROR                : {
         return MSG_CL_RETVAL_READ_ERROR;
      }
      case CL_RETVAL_MAX_READ_SIZE             : {
         return MSG_CL_RETVAL_MAX_READ_SIZE;
      }
      case CL_RETVAL_CLIENT_WELCOME_ERROR      : {
         return MSG_CL_RETVAL_CLIENT_WELCOME_ERROR;
      }
      case CL_RETVAL_UNKOWN_HOST_ERROR         : {
         return MSG_CL_RETVAL_UNKOWN_HOST_ERROR;
      }
      case CL_RETVAL_LOCAL_HOSTNAME_ERROR      : {
         return MSG_CL_RETVAL_LOCAL_HOSTNAME_ERROR;
      }
      case CL_RETVAL_UNKNOWN_ENDPOINT          : {
         return MSG_CL_RETVAL_UNKNOWN_ENDPOINT;
      }
      case CL_RETVAL_UNCOMPLETE_WRITE          : {
         return MSG_CL_RETVAL_UNCOMPLETE_WRITE;
      }
      case CL_RETVAL_UNCOMPLETE_READ           : {
         return MSG_CL_RETVAL_UNCOMPLETE_READ;
      }
      case CL_RETVAL_LIST_DATA_NOT_EMPTY           : {
         return MSG_CL_RETVAL_LIST_DATA_NOT_EMPTY;
      }
      case CL_RETVAL_LIST_NOT_EMPTY           : {
         return MSG_CL_RETVAL_LIST_NOT_EMPTY;
      }
      case CL_RETVAL_LIST_DATA_IS_NULL: {
         return MSG_CL_RETVAL_LIST_DATA_IS_NULL;
      }
      case CL_RETVAL_THREAD_SETSPECIFIC_ERROR: {
         return MSG_CL_RETVAL_THREAD_SETSPECIFIC_ERROR;
      } 
      case CL_RETVAL_NOT_THREAD_SPECIFIC_INIT: {
         return MSG_CL_RETVAL_NOT_THREAD_SPECIFIC_INIT;
      }
      case CL_RETVAL_ALLREADY_CONNECTED: {
         return MSG_CL_RETVAL_ALLREADY_CONNECTED;
      }
      case CL_RETVAL_STREAM_BUFFER_OVERFLOW: {
         return MSG_CL_RETVAL_STREAM_BUFFER_OVERFLOW;
      }
      case CL_RETVAL_GMSH_ERROR: {
         return MSG_CL_RETVAL_GMSH_ERROR;
      }
      case CL_RETVAL_MESSAGE_ACK_ERROR: {
         return MSG_CL_RETVAL_MESSAGE_ACK_ERROR;
      }
      case CL_RETVAL_MESSAGE_WAIT_FOR_ACK: {
         return MSG_CL_RETVAL_MESSAGE_WAIT_FOR_ACK;
      }
      case CL_RETVAL_ENDPOINT_NOT_UNIQUE: {
         return MSG_CL_RETVAL_ENDPOINT_NOT_UNIQUE;
      }
      case CL_RETVAL_SYNC_RECEIVE_TIMEOUT: {
         return MSG_CL_RETVAL_SYNC_RECEIVE_TIMEOUT;
      }
      case CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR: {
         return MSG_CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR;
      }
      case CL_RETVAL_RESOLVING_SETUP_ERROR: {
         return MSG_CL_RETVAL_RESOLVING_SETUP_ERROR;
      }
      case CL_RETVAL_MESSAGE_IN_BUFFER: {
         return MSG_CL_RETVAL_MESSAGE_IN_BUFFER;
      }
      case CL_RETVAL_CONNECTION_GOING_DOWN: {
         return MSG_CL_RETVAL_CONNECTION_GOING_DOWN;
      }
      case CL_RETVAL_CONNECTION_STATE_ERROR: {
         return MSG_CL_RETVAL_CONNECTION_STATE_ERROR;
      } 
      case CL_RETVAL_SELECT_TIMEOUT: {
         return MSG_CL_RETVAL_SELECT_TIMEOUT;
      }
      case CL_RETVAL_SELECT_INTERRUPT: {
         return MSG_CL_RETVAL_SELECT_INTERRUPT;
      }
      case CL_RETVAL_NO_SELECT_DESCRIPTORS: {
         return MSG_CL_RETVAL_NO_SELECT_DESCRIPTORS;
      }
      case CL_RETVAL_ALIAS_EXISTS: {
         return MSG_CL_RETVAL_ALIAS_EXISTS;
      }
      case CL_RETVAL_NO_ALIAS_FILE: {
         return MSG_CL_RETVAL_NO_ALIAS_FILE;
      }
      case CL_RETVAL_ALIAS_FILE_NOT_FOUND: {
         return MSG_CL_RETVAL_ALIAS_FILE_NOT_FOUND;
      }
      case CL_RETVAL_OPEN_ALIAS_FILE_FAILED: {
         return MSG_CL_RETVAL_OPEN_ALIAS_FILE_FAILED;
      }
      case CL_RETVAL_ALIAS_VERSION_ERROR: {
         return MSG_CL_RETVAL_ALIAS_VERSION_ERROR;
      }
      case CL_RETVAL_SECURITY_ANNOUNCE_FAILED: {
         return MSG_CL_RETVAL_SECURITY_ANNOUNCE_FAILED;
      }
      case CL_RETVAL_SECURITY_SEND_FAILED: {
         return MSG_CL_RETVAL_SECURITY_SEND_FAILED;
      } 
      case CL_RETVAL_SECURITY_RECEIVE_FAILED: {
         return MSG_CL_RETVAL_SECURITY_RECEIVE_FAILED;
      }
      case CL_RETVAL_ACCESS_DENIED: {
         return MSG_CL_RETVAL_ACCESS_DENIED;
      }
      case CL_RETVAL_MAX_CON_COUNT_REACHED: {
         return MSG_CL_RETVAL_MAX_CON_COUNT_REACHED;
      }
      case CL_RETVAL_NO_PORT_ERROR: {
         return MSG_CL_RETVAL_NO_PORT_ERROR;
      }
      case CL_RETVAL_PROTOCOL_ERROR: {
         return MSG_CL_RETVAL_PROTOCOL_ERROR;
      }
      case CL_RETVAL_LOCAL_ENDPOINT_NOT_UNIQUE: {
         return MSG_CL_RETVAL_LOCAL_ENDPOINT_NOT_UNIQUE;
      } 
      case CL_RETVAL_TO_LESS_FILEDESCRIPTORS: {
         return MSG_CL_RETVAL_TO_LESS_FILEDESCRIPTORS;
      }
      case CL_RETVAL_DEBUG_CLIENTS_NOT_ENABLED: {
         return MSG_CL_RETVAL_DEBUG_CLIENTS_NOT_ENABLED;
      }
      case CL_RETVAL_CREATE_RESERVED_PORT_SOCKET: {
         return MSG_CL_RETVAL_CREATE_RESERVED_PORT_SOCKET;
      }
      case CL_RETVAL_NO_RESERVED_PORT_CONNECTION: {
         return MSG_CL_RETVAL_NO_RESERVED_PORT_CONNECTION;
      }
      case CL_RETVAL_NO_LOCAL_HOST_CONNECTION: {
         return MSG_CL_RETVAL_NO_LOCAL_HOST_CONNECTION;
      }
      case CL_RETVAL_SSL_COULD_NOT_SET_METHOD: {
         return MSG_CL_RETVAL_SSL_COULD_NOT_SET_METHOD;
      }
      case CL_RETVAL_SSL_COULD_NOT_CREATE_CONTEXT: {
         return MSG_CL_RETVAL_SSL_COULD_NOT_CREATE_CONTEXT;
      } 
      case CL_RETVAL_SSL_COULD_NOT_SET_CA_CHAIN_FILE: {
         return MSG_CL_RETVAL_SSL_COULD_NOT_SET_CA_CHAIN_FILE;
      }
      case CL_RETVAL_SSL_CANT_SET_CA_KEY_PEM_FILE: {
         return MSG_CL_RETVAL_SSL_CANT_SET_CA_KEY_PEM_FILE;
      }
      case CL_RETVAL_SSL_CANT_READ_CA_LIST: {
         return MSG_CL_RETVAL_SSL_CANT_READ_CA_LIST;
      }
      case CL_RETVAL_SSL_NO_SYMBOL_TABLE: {
         return MSG_CL_RETVAL_SSL_NO_SYMBOL_TABLE;
      }
      case CL_RETVAL_SSL_SYMBOL_TABLE_ALREADY_LOADED: {
         return MSG_CL_RETVAL_SSL_SYMBOL_TABLE_ALREADY_LOADED;
      }
      case CL_RETVAL_SSL_DLOPEN_SSL_LIB_FAILED: {
         return MSG_CL_RETVAL_SSL_DLOPEN_SSL_LIB_FAILED;
      }
      case CL_RETVAL_SSL_CANT_LOAD_ALL_FUNCTIONS: {
         return MSG_CL_RETVAL_SSL_CANT_LOAD_ALL_FUNCTIONS;
      }
      case CL_RETVAL_SSL_SHUTDOWN_ERROR: {
         return MSG_CL_RETVAL_SSL_SHUTDOWN_ERROR;
      }
      case CL_RETVAL_SSL_CANT_CREATE_SSL_OBJECT: {
         return MSG_CL_RETVAL_SSL_CANT_CREATE_SSL_OBJECT;
      }
      case CL_RETVAL_SSL_CANT_CREATE_BIO_SOCKET: {
         return MSG_CL_RETVAL_SSL_CANT_CREATE_BIO_SOCKET;
      }
      case CL_RETVAL_SSL_ACCEPT_HANDSHAKE_TIMEOUT: {
         return MSG_CL_RETVAL_SSL_ACCEPT_HANDSHAKE_TIMEOUT;
      }
      case CL_RETVAL_SSL_ACCEPT_ERROR: {
         return MSG_CL_RETVAL_SSL_ACCEPT_ERROR;
      }
      case CL_RETVAL_SSL_CONNECT_HANDSHAKE_TIMEOUT: {
         return MSG_CL_RETVAL_SSL_CONNECT_HANDSHAKE_TIMEOUT;
      }
      case CL_RETVAL_SSL_CONNECT_ERROR: {
         return MSG_CL_RETVAL_SSL_CONNECT_ERROR;
      }
      case CL_RETVAL_SSL_CERTIFICATE_ERROR: {
         return MSG_CL_RETVAL_SSL_CERTIFICATE_ERROR;
      }
      case CL_RETVAL_SSL_PEER_CERTIFICATE_ERROR: {
         return MSG_CL_RETVAL_SSL_PEER_CERTIFICATE_ERROR;
      }
      case CL_RETVAL_SSL_GET_SSL_ERROR: {
         return MSG_CL_RETVAL_SSL_GET_SSL_ERROR;
      }
      case CL_RETVAL_UNEXPECTED_CHARACTERS: {
         return MSG_CL_RETVAL_UNEXPECTED_CHARACTERS;
      }
      case CL_RETVAL_SSL_NO_SERVICE_PEER_NAME: {
         return MSG_CL_RETVAL_SSL_NO_SERVICE_PEER_NAME;
      }
      case CL_RETVAL_SSL_RAND_SEED_FAILURE: {
         return MSG_CL_RETVAL_SSL_RAND_SEED_FAILURE;
      }
      case CL_RETVAL_SSL_NOT_SUPPORTED: {
         return MSG_CL_RETVAL_SSL_NOT_SUPPORTED;
      }
      case CL_RETVAL_ERROR_SETTING_CIPHER_LIST: {
         return MSG_CL_RETVAL_ERROR_SETTING_CIPHER_LIST;
      }
      case CL_RETVAL_REACHED_FILEDESCRIPTOR_LIMIT: {
         return MSG_CL_RETVAL_REACHED_FILEDESCRIPTOR_LIMIT;
      }
      case CL_RETVAL_HOSTNAME_LENGTH_ERROR: {
         return MSG_CL_RETVAL_HOSTNAME_LENGTH_ERROR;
      }
      case CL_RETVAL_HANDLE_SHUTDOWN_IN_PROGRESS: {
         return MSG_CL_RETVAL_HANDLE_SHUTDOWN_IN_PROGRESS;
      }
      case CL_RETVAL_COMMLIB_SETUP_ALREADY_CALLED: {
         return MSG_CL_RETVAL_COMMLIB_SETUP_ALREADY_CALLED;
      }
      case CL_RETVAL_DO_IGNORE: {
         return MSG_CL_RETVAL_DO_IGNORE;
      }
      case CL_RETVAL_CLOSE_ALIAS_FILE_FAILED: {
         return MSG_CL_RETVAL_CLOSE_ALIAS_FILE_FAILED;
      }
      case CL_RETVAL_SSL_CANT_SET_CERT_PEM_BYTE: {
         return MSG_CL_RETVAL_SSL_CANT_SET_CERT_PEM_BYTE;
      }
      case CL_RETVAL_SSL_SET_CERT_PEM_BYTE_IS_NULL: {
         return MSG_CL_RETVAL_SSL_SET_CERT_PEM_BYTE_IS_NULL;
      }
      case CL_RETVAL_SSL_CANT_SET_KEY_PEM_BYTE: {
         return MSG_CL_RETVAL_SSL_CANT_SET_KEY_PEM_BYTE;
      }
      case CL_RETVAL_UNKNOWN_PARAMETER: {
         return MSG_CL_RETVAL_UNKNOWN_PARAMETER;
      }
   }
   return CL_RETVAL_UNDEFINED_STR;
}

