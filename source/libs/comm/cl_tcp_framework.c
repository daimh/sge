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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>

#include <netinet/tcp.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <poll.h>

#include "comm/cl_tcp_framework.h"
#include "comm/cl_communication.h"
#include "comm/cl_commlib.h"
#include "comm/cl_connection_list.h"
#include "comm/cl_fd_list.h"
#include "comm/msg_commlib.h"

#include "uti/sge_unistd.h"
#include "uti/sge_os.h"

/* connection specific struct (not used from outside) */
typedef struct cl_com_tcp_private_type {
   /* TCP/IP specific */
   int           server_port;         /* used port for server setup */
   int           connect_port;        /* port to connect to */
   int           connect_in_port;     /* port from where client is connected (used for reserved port check) */
   int           sockfd;              /* socket file descriptor */
   int           pre_sockfd;          /* socket which was prepared for later listen call (only_prepare_service == TRUE */
   struct sockaddr_in client_addr;    /* used in connect for storing client addr of connection partner */ 
} cl_com_tcp_private_t;


static cl_com_tcp_private_t* cl_com_tcp_get_private(cl_com_connection_t* connection);
static int cl_com_tcp_free_com_private(cl_com_connection_t* connection);
static int cl_com_tcp_connection_request_handler_setup_finalize(cl_com_connection_t* connection);



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_get_fd()"
int cl_com_tcp_get_fd(cl_com_connection_t* connection, int* fd) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL || fd == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   if ((private=cl_com_tcp_get_private(connection)) != NULL) {
      if (private->sockfd < 0) {
         CL_LOG_INT(CL_LOG_INFO, "return pre_sockfd: ", private->pre_sockfd);
         *fd = private->pre_sockfd;
      } else {
         CL_LOG_INT(CL_LOG_INFO, "return final sockfd: ", private->sockfd);
         *fd = private->sockfd;
      }
      return CL_RETVAL_OK;
   }
   CL_LOG(CL_LOG_ERROR, "cannot get private connection data object!");
   return CL_RETVAL_UNKNOWN;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_get_service_port()"
int cl_com_tcp_get_service_port(cl_com_connection_t* connection, int* port) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL || port == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   if ( (private=cl_com_tcp_get_private(connection)) != NULL) {
      *port = private->server_port;
      return CL_RETVAL_OK;
   }
   return CL_RETVAL_UNKNOWN;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_get_client_socket_in_port()"
int cl_com_tcp_get_client_socket_in_port(cl_com_connection_t* connection, int* port) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL || port == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   if ( (private=cl_com_tcp_get_private(connection)) != NULL) {
      *port = private->connect_in_port;
      return CL_RETVAL_OK;
   }
   return CL_RETVAL_UNKNOWN;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_get_connect_port()"
int cl_com_tcp_get_connect_port(cl_com_connection_t* connection, int* port) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL || port == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   if ( (private=cl_com_tcp_get_private(connection)) != NULL) {
      *port = private->connect_port;
      return CL_RETVAL_OK;
   }
   return CL_RETVAL_UNKNOWN;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_set_connect_port()"
int cl_com_tcp_set_connect_port(cl_com_connection_t* connection, int port) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if ( (private=cl_com_tcp_get_private(connection)) != NULL) {
      private->connect_port = port;
      return CL_RETVAL_OK;
   }
   return CL_RETVAL_UNKNOWN;
}


#if 0
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_dump_tcp_private()"
static void cl_dump_tcp_private(cl_com_connection_t* connection) {
   cl_com_tcp_private_t* private = NULL;
   if (connection == NULL) {
      CL_LOG(CL_LOG_DEBUG, "connection is NULL");
   } else {
      if ( (private=cl_com_tcp_get_private(connection)) != NULL) {
         CL_LOG_INT(CL_LOG_DEBUG,"server port:",private->server_port);
         CL_LOG_INT(CL_LOG_DEBUG,"connect_port:",private->connect_port);
         CL_LOG_INT(CL_LOG_DEBUG,"socked fd:",private->sockfd);
      }
   }
}
#endif



/****** cl_tcp_framework/cl_com_tcp_open_connection() **************************
*  NAME
*     cl_com_tcp_open_connection() -- open a tcp/ip connection
*
*  SYNOPSIS
*     int cl_com_tcp_open_connection(cl_com_connection_t* connection, const 
*     char* comp_host, const char* comp_name, int comp_id, int timeout) 
*
*  FUNCTION
*     This function will create a new socket file descriptor and set the 
*     SO_REUSEADDR socket option and the O_NONBLOCK file descriptor flag. 
*     After that the socket will try to connect the service handler on the
*     given connect_port (set with cl_com_tcp_setup_connection()) on the
*     host specified with comp_host. After a successful connect the 
*     TCP_NODELAY socket option is set. 
*
*  INPUTS
*     cl_com_connection_t* connection - pointer to a connection struct
*     const char* comp_host           - host where a service is available
*     const char* comp_name           - component name of service
*     int comp_id                     - component id of service
*     int timeout                     - timeout for connect
*
*  RESULT
*     int - CL_COMM_XXXX error value or CL_RETVAL_OK for no errors
*
*  SEE ALSO
*     cl_communication/cl_com_open_connection()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_open_connection()"
int cl_com_tcp_open_connection(cl_com_connection_t* connection, int timeout) {
   cl_com_tcp_private_t* private = NULL;
   
   int tmp_error = CL_RETVAL_OK;

   if (connection == NULL || connection->remote == NULL ||
       connection->local == NULL) {
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if ( private->connect_port <= 0 ) {
      CL_LOG(CL_LOG_ERROR, cl_get_error_text(CL_RETVAL_NO_PORT_ERROR));
      return CL_RETVAL_NO_PORT_ERROR; 
   }

   if ( connection->connection_state != CL_OPENING ) {
      CL_LOG(CL_LOG_ERROR,"state is not CL_OPENING - return connect error");
      return CL_RETVAL_CONNECT_ERROR;   
   }

   if (connection->connection_sub_state == CL_COM_OPEN_INIT) {
      int ret;
      int on = 1;
      char* unique_host = NULL;
      struct timeval now;
      int res_port = IPPORT_RESERVED -1;

      CL_LOG(CL_LOG_DEBUG,"connection_sub_state is CL_COM_OPEN_INIT");
      private->sockfd = -1;
      
      switch(connection->tcp_connect_mode) {
         case CL_TCP_DEFAULT: {
            /* create socket */
            if ((private->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
               CL_LOG(CL_LOG_ERROR,"could not create socket");
               private->sockfd = -1;
               cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CREATE_SOCKET, MSG_CL_TCP_FW_SOCKET_ERROR );
               return CL_RETVAL_CREATE_SOCKET;
            }
            break;
         }
         case CL_TCP_RESERVED_PORT: {
            /* create reserved port socket */
            if ((private->sockfd = rresvport(&res_port)) < 0) {
               CL_LOG(CL_LOG_ERROR,"could not create reserved port socket");
               private->sockfd = -1;
               cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CREATE_SOCKET, MSG_CL_TCP_FW_RESERVED_SOCKET_ERROR );
               return CL_RETVAL_CREATE_RESERVED_PORT_SOCKET;
            }
            break;
         }
      }

      if (private->sockfd < 3) {
         CL_LOG_INT(CL_LOG_WARNING, "The file descriptor is < 3. Will dup fd to be >= 3! fd value: ", private->sockfd);
         ret = sge_dup_fd_above_stderr(&(private->sockfd));
         if (ret != 0) {
            CL_LOG_INT(CL_LOG_ERROR, "can't dup socket fd to be >=3, errno = ", ret);
            shutdown(private->sockfd, 2);
            close(private->sockfd);
            private->sockfd = -1;
            cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_DUP_SOCKET_FD_ERROR, MSG_CL_COMMLIB_CANNOT_DUP_SOCKET_FD);
            return CL_RETVAL_DUP_SOCKET_FD_ERROR;
         }
         CL_LOG_INT(CL_LOG_INFO, "fd value after dup: ", private->sockfd);
      }

      /* set local address reuse socket option */
      if (setsockopt(private->sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) != 0) {
         CL_LOG(CL_LOG_ERROR,"could not set SO_REUSEADDR");
         private->sockfd = -1;
         cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_SETSOCKOPT_ERROR, MSG_CL_TCP_FW_SETSOCKOPT_ERROR);
         return CL_RETVAL_SETSOCKOPT_ERROR;
      }
   
      /* this is a non blocking socket */
      if (fcntl(private->sockfd, F_SETFL, O_NONBLOCK) != 0) {
         CL_LOG(CL_LOG_ERROR,"could not set O_NONBLOCK");
         private->sockfd = -1;
         cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_FCNTL_ERROR, MSG_CL_TCP_FW_FCNTL_ERROR);
         return CL_RETVAL_FCNTL_ERROR;
      }

      /* set address  */
      memset((char *) &(private->client_addr), 0, sizeof(struct sockaddr_in));
      private->client_addr.sin_port = htons(private->connect_port);
      private->client_addr.sin_family = AF_INET;
      if ( (tmp_error=cl_com_cached_gethostbyname(connection->remote->comp_host, &unique_host, &(private->client_addr.sin_addr), NULL , NULL)) != CL_RETVAL_OK) {
         char tmp_buffer[256];
   
         shutdown(private->sockfd, 2);
         close(private->sockfd);
         sge_free(&unique_host);
         CL_LOG(CL_LOG_ERROR,"could not get hostname");
         private->sockfd = -1;
         
         if ( connection != NULL && connection->remote != NULL && connection->remote->comp_host != NULL) {
            snprintf(tmp_buffer,256, MSG_CL_TCP_FW_CANT_RESOLVE_HOST_S, connection->remote->comp_host );
         } else {
            snprintf(tmp_buffer,256, "%s", cl_get_error_text(tmp_error));
         }
         cl_commlib_push_application_error(CL_LOG_ERROR, tmp_error, tmp_buffer);
         return tmp_error; 
      } 
      sge_free(&unique_host);

      /* connect */
      gettimeofday(&now,NULL);
      connection->write_buffer_timeout_time = now.tv_sec + timeout;
      connection->connection_sub_state = CL_COM_OPEN_CONNECT;
   }
   
   if (connection->connection_sub_state == CL_COM_OPEN_CONNECT) {
      int my_error;
      int i;

      CL_LOG(CL_LOG_DEBUG,"connection_sub_state is CL_COM_OPEN_CONNECT");

      errno = 0;
      i = connect(private->sockfd, (struct sockaddr *) &(private->client_addr), sizeof(struct sockaddr_in));
      my_error = errno;
      if (i == 0 || my_error == EISCONN) {
         /* we are connected */
         connection->write_buffer_timeout_time = 0;
         connection->connection_sub_state = CL_COM_OPEN_CONNECTED;
      } else {
         switch(my_error) {
            case ECONNREFUSED: {
               /* can't open connection */
               CL_LOG_INT(CL_LOG_ERROR,"connection refused to port ",private->connect_port);
               shutdown(private->sockfd, 2);
               close(private->sockfd);
               private->sockfd = -1;
               cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CONNECT_ERROR, strerror(my_error));
               return CL_RETVAL_CONNECT_ERROR;
            }
            case EADDRNOTAVAIL: {
               /* can't open connection */
               CL_LOG_INT(CL_LOG_ERROR,"address not available for port ",private->connect_port);
               shutdown(private->sockfd, 2);
               close(private->sockfd);
               private->sockfd = -1;
               cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CONNECT_ERROR, strerror(my_error));
               return CL_RETVAL_CONNECT_ERROR;
            }
            case EINPROGRESS:
            case EALREADY: {
               connection->connection_sub_state = CL_COM_OPEN_CONNECT_IN_PROGRESS;
               return CL_RETVAL_UNCOMPLETE_WRITE;
            }
            default: {
               /* we have an connect error */
               CL_LOG_INT(CL_LOG_ERROR,"connect error errno:", my_error);
               shutdown(private->sockfd, 2);
               close(private->sockfd);
               private->sockfd = -1;
               cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CONNECT_ERROR, strerror(my_error));
               return CL_RETVAL_CONNECT_ERROR;
            }
         }
      }
   }

   if (connection->connection_sub_state == CL_COM_OPEN_CONNECT_IN_PROGRESS) {

      struct timeval now;
      int socket_error = 0;
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HPUX)
      int socklen = sizeof(socket_error);
#else
      socklen_t socklen = sizeof(socket_error);
#endif

      CL_LOG(CL_LOG_DEBUG,"connection_sub_state is CL_COM_OPEN_CONNECT_IN_PROGRESS");

#if defined(SOLARIS) && !defined(SOLARIS64)
      getsockopt(private->sockfd, SOL_SOCKET, SO_ERROR, (void*)&socket_error, &socklen);
#else
      getsockopt(private->sockfd, SOL_SOCKET, SO_ERROR, &socket_error, &socklen);
#endif
      if (socket_error == 0 || socket_error == EISCONN) {
         CL_LOG(CL_LOG_INFO,"connected");
         connection->write_buffer_timeout_time = 0;
         connection->connection_sub_state = CL_COM_OPEN_CONNECTED;
         /* we are connected */
      } else {
         if (socket_error != EINPROGRESS && socket_error != EALREADY) {
            CL_LOG_INT(CL_LOG_ERROR,"socket error errno:", socket_error);
            shutdown(private->sockfd, 2);
            close(private->sockfd);
            private->sockfd = -1;
            cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CONNECT_ERROR, strerror(socket_error));
            return CL_RETVAL_CONNECT_ERROR;
         }

         gettimeofday(&now,NULL);
         if (connection->write_buffer_timeout_time <= now.tv_sec || 
             cl_com_get_ignore_timeouts_flag()     == true       ) {

            /* we had an timeout */
            CL_LOG(CL_LOG_ERROR,"connect timeout error");
            connection->write_buffer_timeout_time = 0;
            shutdown(private->sockfd, 2);
            close(private->sockfd);
            private->sockfd = -1;
            cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_CONNECT_TIMEOUT, MSG_CL_TCP_FW_CONNECT_TIMEOUT );
            return CL_RETVAL_CONNECT_TIMEOUT;
         }

         return CL_RETVAL_UNCOMPLETE_WRITE;
      }
   }

   if (connection->connection_sub_state == CL_COM_OPEN_CONNECTED) {
      int on = 1; 

      CL_LOG(CL_LOG_DEBUG,"connection_sub_state is CL_COM_OPEN_CONNECTED");

  
#if defined(SOLARIS) && !defined(SOLARIS64)
      if (setsockopt(private->sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &on, sizeof(int)) != 0) {
         CL_LOG(CL_LOG_ERROR,"could not set TCP_NODELAY");
      }
#else
      if (setsockopt(private->sockfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int))!= 0) {
         CL_LOG(CL_LOG_ERROR,"could not set TCP_NODELAY");
      }
#endif
      return CL_RETVAL_OK;
   }

   return CL_RETVAL_UNKNOWN;
}



/****** cl_communication/cl_com_tcp_setup_connection() *************************
*  NAME
*     cl_com_tcp_setup_connection() -- setup a connection type
*
*  SYNOPSIS
*     int cl_com_tcp_setup_connection(cl_com_connection_t* connection, int 
*     server_port, int connect_port) 
*
*  FUNCTION
*     This function is used to setup the connection type. It will malloc
*     a cl_com_tcp_private_t structure and set the pointer 
*     connection->com_private to this structure.
*
*     When the connection structure is used to provide a service the server_port
*     must be specified. If the connection is used to be a client to a service
*     the connect_port must be specified.
*
*     The memory obtained by the malloc() call for the cl_com_tcp_private_t structure 
*     is released by a call to cl_com_tcp_close_connection()
*
*  INPUTS
*     cl_com_connection_t* connection - empty connection structure
*     int server_port                 - port to provide a tcp service 
*     int connect_port                - port to connect to
*     int data_flow_type              - CL_COM_STREAM or CL_COM_MESSAGE
*
*  RESULT
*     int - CL_COMM_XXXX error value or CL_RETVAL_OK for no errors
*
*  SEE ALSO
*
*     cl_communication/cl_com_close_connection()
*
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_setup_connection()"
int cl_com_tcp_setup_connection(cl_com_connection_t**          connection,
                                int                            server_port,
                                int                            connect_port,
                                cl_xml_connection_type_t       data_flow_type,
                                cl_xml_connection_autoclose_t  auto_close_mode,
                                cl_framework_t                 framework_type,
                                cl_xml_data_format_t           data_format_type,
                                cl_tcp_connect_t               tcp_connect_mode) {
   cl_com_tcp_private_t* com_private = NULL;
   int ret_val;
   if (connection == NULL || *connection != NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (data_flow_type != CL_CM_CT_STREAM && data_flow_type != CL_CM_CT_MESSAGE) {
      return CL_RETVAL_PARAMS;
   }

   /* create new connection */
   if ((ret_val=cl_com_create_connection(connection)) != CL_RETVAL_OK) {
      return ret_val;
   }

   /* check for correct framework specification */
   switch(framework_type) {
      case CL_CT_TCP:
         break;
      case CL_CT_UNDEFINED:
      case CL_CT_SSL: {
         CL_LOG_STR(CL_LOG_ERROR,"unexpected framework:", cl_com_get_framework_type(*connection));
         cl_com_close_connection(connection);
         return CL_RETVAL_WRONG_FRAMEWORK;
      }
   }

   /* create private data structure */
   com_private = (cl_com_tcp_private_t*) malloc(sizeof(cl_com_tcp_private_t));
   if (com_private == NULL) {
      cl_com_close_connection(connection);
      return CL_RETVAL_MALLOC;
   }

   /* set com_private to com_private pointer */
   (*connection)->com_private = com_private;

   /* set modes */
   (*connection)->auto_close_type = auto_close_mode;
   (*connection)->data_flow_type = data_flow_type;
   (*connection)->connection_type = CL_COM_SEND_RECEIVE;
   (*connection)->framework_type = framework_type;
   (*connection)->data_format_type = data_format_type;
   (*connection)->tcp_connect_mode = tcp_connect_mode;

   /* setup tcp private struct */
   com_private->sockfd = -1;
   com_private->pre_sockfd = -1;
   com_private->connect_in_port = 0; 
   com_private->server_port = server_port;
   com_private->connect_port = connect_port;
   return CL_RETVAL_OK;
}


/****** cl_tcp_framework/cl_com_tcp_free_com_private() *************************
*  NAME
*     cl_com_tcp_free_com_private() -- free private struct of a tcp connection
*
*  SYNOPSIS
*     int cl_com_tcp_free_com_private(cl_com_connection_t* connection) 
*
*  FUNCTION
*     This function will free the com_private struct pointer of a tcp connection
*     struct
*
*  INPUTS
*     cl_com_connection_t* connection - pointer to tcp/ip connection
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success
*
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_free_com_private()"
static int cl_com_tcp_free_com_private(cl_com_connection_t* connection) {

   if (connection == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (connection->com_private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   /* free struct cl_com_tcp_private_t */
   sge_free(&(connection->com_private));
   connection->com_private = NULL;
   return CL_RETVAL_OK;
}


/****** cl_tcp_framework/cl_com_tcp_close_connection() *************************
*  NAME
*     cl_com_tcp_close_connection() -- close and shutdown a tcp connection
*
*  SYNOPSIS
*     int cl_com_tcp_close_connection(cl_com_connection_t* connection) 
*
*  FUNCTION
*     This function will shutdown and close the connection (if open) and free
*     the connection->com_private pointer for a tcp connection.
*
*  INPUTS
*     cl_com_connection_t* connection - connection pointer
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success
*
*  SEE ALSO
*     cl_communication/cl_com_close_connection()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_close_connection()"
int cl_com_tcp_close_connection(cl_com_connection_t** connection) {
   cl_com_tcp_private_t* private = NULL;

   if (connection == NULL) {
      return CL_RETVAL_PARAMS;
   }
   if (*connection == NULL) {
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(*connection);

   if (private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if (private->sockfd >= 0) {
      CL_LOG(CL_LOG_INFO,"closing connection");
      /* shutdown socket connection */
      shutdown(private->sockfd, 2);
      close(private->sockfd);
      private->sockfd = -1;
   }
 
   /* free com private structure */
   return cl_com_tcp_free_com_private(*connection);
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_write()"
int cl_com_tcp_write(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long *only_one_write) {
   cl_com_tcp_private_t* private = NULL;
   long data_written = 0;
   int my_errno;

   if (message == NULL) {
      CL_LOG(CL_LOG_ERROR,"no message to write");
      return CL_RETVAL_PARAMS;
   }

   if (only_one_write == NULL) {
      CL_LOG(CL_LOG_ERROR,"only_one_write is NULL");
      return CL_RETVAL_PARAMS;
   }

   if (connection == NULL) {
      CL_LOG(CL_LOG_ERROR,"no connection object");
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if (size == 0) {
      CL_LOG(CL_LOG_ERROR,"data size is zero");
      return CL_RETVAL_PARAMS;
   }

   if (private->sockfd < 0) {
      CL_LOG(CL_LOG_ERROR,"no file descriptor");
      return CL_RETVAL_PARAMS;
   }

   if (size > CL_DEFINE_MAX_MESSAGE_LENGTH) {
      CL_LOG_INT(CL_LOG_ERROR,"data to write is > max message length =", CL_DEFINE_MAX_MESSAGE_LENGTH );
      cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_MAX_READ_SIZE, NULL);
      return CL_RETVAL_MAX_READ_SIZE;
   }

   errno = 0;
   data_written = write(private->sockfd, message, size);   
   my_errno = errno;
   if (data_written < 0) {
      if (my_errno != EWOULDBLOCK && my_errno != EAGAIN && my_errno != EINTR) {
         if (my_errno == EPIPE) {
            CL_LOG_INT(CL_LOG_ERROR,"pipe error errno:", my_errno);
            return CL_RETVAL_PIPE_ERROR;
         }
         CL_LOG_INT(CL_LOG_ERROR,"send error errno:", my_errno);
         return CL_RETVAL_SEND_ERROR;
      } else {
         CL_LOG_INT(CL_LOG_INFO,"send error errno:", my_errno);
         data_written = 0;
      }
   } 

   *only_one_write = data_written;
   if (data_written != size) {
      struct timeval now;

      gettimeofday(&now,NULL);
      if ( now.tv_sec >= connection->write_buffer_timeout_time ) {
         CL_LOG(CL_LOG_ERROR,"send timeout error");
         return CL_RETVAL_SEND_TIMEOUT;
      }
      return CL_RETVAL_UNCOMPLETE_WRITE;
   }
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_read()"
int cl_com_tcp_read(cl_com_connection_t* connection, cl_byte_t* message, unsigned long size, unsigned long* only_one_read) {
   cl_com_tcp_private_t* private = NULL;
   long data_read = 0;
   int my_errno;

   if (message == NULL) {
      CL_LOG(CL_LOG_ERROR,"no message buffer");
      return CL_RETVAL_PARAMS;
   }

   if (only_one_read == NULL) {
      CL_LOG(CL_LOG_ERROR,"only_one_read is NULL");
      return CL_RETVAL_PARAMS;
   }

   if (connection == NULL) {
      CL_LOG(CL_LOG_ERROR,"no connection object");
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if (private->sockfd < 0) {
      CL_LOG(CL_LOG_ERROR,"no file descriptor");
      return CL_RETVAL_PARAMS;
   }


   if (size == 0) {
      CL_LOG(CL_LOG_ERROR,"no data size");
      return CL_RETVAL_PARAMS;
   }

   if (size > CL_DEFINE_MAX_MESSAGE_LENGTH) {
      CL_LOG_INT(CL_LOG_ERROR,"data to read is > max message length =", CL_DEFINE_MAX_MESSAGE_LENGTH );
      cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_MAX_READ_SIZE, NULL);
      return CL_RETVAL_MAX_READ_SIZE;
   }

   errno = 0;
   data_read = read(private->sockfd, message, size);
   my_errno = errno;
   if (data_read <= 0) {
      if (my_errno != EWOULDBLOCK && my_errno != EAGAIN && my_errno != EINTR && my_errno != 0) {
         if (my_errno == EPIPE) {
            CL_LOG_INT(CL_LOG_ERROR,"pipe error (only_one_read != NULL) errno:", my_errno );
            return CL_RETVAL_PIPE_ERROR;
         }
         CL_LOG_INT(CL_LOG_ERROR,"receive error (only_one_read != NULL) errno:", my_errno);
         return CL_RETVAL_READ_ERROR;
      } else {
         if (data_read == 0) {
            /* this should only happen if the connection is down */
            CL_LOG(CL_LOG_WARNING,"client connection disconnected");
            return CL_RETVAL_READ_ERROR;
         }
         CL_LOG_INT(CL_LOG_INFO,"receive error errno:", my_errno);
         data_read = 0;
      }
   }

   *only_one_read = data_read;
   if (data_read != size) {
      struct timeval now;
      gettimeofday(&now,NULL);
      if ( now.tv_sec >= connection->read_buffer_timeout_time ) {
         return CL_RETVAL_READ_TIMEOUT;
      }
      return CL_RETVAL_UNCOMPLETE_READ;
   }
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_read_GMSH()"
int cl_com_tcp_read_GMSH(cl_com_connection_t* connection, unsigned long *only_one_read) {
   int retval = CL_RETVAL_OK;
   unsigned long data_read = 0;
   unsigned long processed_data = 0;

   if (connection == NULL || only_one_read == NULL) {
      CL_LOG(CL_LOG_ERROR,"parameters not initalized");
      return CL_RETVAL_PARAMS;
   }

   /* first read size of gmsh header without data */
   if (connection->data_read_buffer_pos < CL_GMSH_MESSAGE_SIZE) {
      data_read = 0;
      retval = cl_com_tcp_read(connection, 
                               &(connection->data_read_buffer[connection->data_read_buffer_pos]),
                               CL_GMSH_MESSAGE_SIZE - connection->data_read_buffer_pos,
                               &data_read);
      connection->data_read_buffer_pos = connection->data_read_buffer_pos + data_read;
      *only_one_read = data_read;

      if ( retval != CL_RETVAL_OK) {
         CL_LOG_STR(CL_LOG_INFO,"uncomplete read:", cl_get_error_text(retval));
         return retval;
      }
   }

   /* now read complete header */
   while (connection->data_read_buffer[connection->data_read_buffer_pos - 1] != '>' ||
          connection->data_read_buffer[connection->data_read_buffer_pos - 2] != 'h'   ) {

      /* check buffer overflow */
      if (connection->data_read_buffer_pos >= connection->data_buffer_size) {
         CL_LOG(CL_LOG_WARNING,"buffer overflow");
         return CL_RETVAL_STREAM_BUFFER_OVERFLOW;
      }

      data_read = 0;
      retval = cl_com_tcp_read(connection, 
                               &(connection->data_read_buffer[connection->data_read_buffer_pos]),
                               1,
                               &data_read);

      connection->data_read_buffer_pos = connection->data_read_buffer_pos + data_read;
      *only_one_read = data_read;

      if (retval != CL_RETVAL_OK) {
         CL_LOG(CL_LOG_WARNING,"uncomplete read(2):");
         return retval;
      }
   }

   if ( connection->data_read_buffer_pos >= connection->data_buffer_size) {
      CL_LOG(CL_LOG_WARNING,"buffer overflow (2)");
      return CL_RETVAL_STREAM_BUFFER_OVERFLOW;
   }

   connection->data_read_buffer[connection->data_read_buffer_pos] = 0;
   /* header should be now complete */
   if ( strcmp((char*)&(connection->data_read_buffer[connection->data_read_buffer_pos - 7]) ,"</gmsh>") != 0) {
      CL_LOG(CL_LOG_WARNING,"can't find gmsh end tag");
      return CL_RETVAL_GMSH_ERROR;
   }
   
   /* parse header */
   retval = cl_xml_parse_GMSH(connection->data_read_buffer, connection->data_read_buffer_pos, connection->read_gmsh_header, &processed_data);
   connection->data_read_buffer_processed = connection->data_read_buffer_processed + processed_data ;
   if ( connection->read_gmsh_header->dl == 0) {
      CL_LOG(CL_LOG_ERROR,"gmsh header has dl=0 entry");
      return CL_RETVAL_GMSH_ERROR;
   }
   if ( connection->read_gmsh_header->dl > CL_DEFINE_MAX_MESSAGE_LENGTH ) {
      CL_LOG(CL_LOG_ERROR,"gmsh header dl entry is larger than CL_DEFINE_MAX_MESSAGE_LENGTH");
      cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR, NULL);
      return CL_RETVAL_MAX_MESSAGE_LENGTH_ERROR;
   }
   return retval;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_connection_request_handler_setup_finalize()"
static int cl_com_tcp_connection_request_handler_setup_finalize(cl_com_connection_t* connection) {
   cl_com_tcp_private_t* private = NULL;
   int sockfd = 0;

   if (connection == NULL ) {
      CL_LOG(CL_LOG_ERROR, "no connection");
      return CL_RETVAL_PARAMS;
   }
   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      CL_LOG(CL_LOG_ERROR, "framework not initalized");
      return CL_RETVAL_PARAMS; 
   }
 
   sockfd = private->pre_sockfd;

   if (sockfd < 0) {
      CL_LOG(CL_LOG_ERROR, "pre_sockfd not valid");
      return CL_RETVAL_PARAMS;
   }

   /* make socket listening for incoming connects */
   if (listen(sockfd, 5) != 0) {   /* TODO: set listen params */
      shutdown(sockfd, 2);
      close(sockfd);
      CL_LOG(CL_LOG_ERROR,"listen error");
      return CL_RETVAL_LISTEN_ERROR;
   }
   CL_LOG_INT(CL_LOG_INFO,"listening with backlog=", 5);

   /* set server socked file descriptor and mark connection as service handler */
   private->sockfd = sockfd;

   CL_LOG(CL_LOG_INFO,"===============================");
   CL_LOG(CL_LOG_INFO,"TCP server setup done:");
   CL_LOG_INT(CL_LOG_INFO,"server fd:", private->sockfd);
   CL_LOG_STR(CL_LOG_INFO,"host:     ", connection->local->comp_host);
   CL_LOG_STR(CL_LOG_INFO,"component:", connection->local->comp_name);
   CL_LOG_INT(CL_LOG_INFO,"id:       ",(int) (connection->local->comp_id));
   CL_LOG(CL_LOG_INFO,"===============================");
   return CL_RETVAL_OK;
}


/****** cl_tcp_framework/cl_com_tcp_connection_request_handler_setup() *********
*  NAME
*     cl_com_tcp_connection_request_handler_setup() -- bind tcp/ip socket
*
*  SYNOPSIS
*     int cl_com_tcp_connection_request_handler_setup(cl_com_connection_t* 
*     connection) 
*
*  FUNCTION
*     This function creates a new stream socket and sets SO_REUSEADDR socket
*     option. After that the socket is bind to the server_port. A final listen
*     enables connection requests on that socket. 
*
*  INPUTS
*     cl_com_connection_t* connection - pointer to connection
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success 
*
*  SEE ALSO
*     cl_tcp_framework/cl_com_tcp_connection_request_handler()
*     cl_tcp_framework/cl_com_tcp_connection_request_handler_cleanup()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_connection_request_handler_setup()"
int cl_com_tcp_connection_request_handler_setup(cl_com_connection_t* connection, bool only_prepare_service) {
   int sockfd = 0;
   struct sockaddr_in serv_addr;
   cl_com_tcp_private_t* private = NULL;
   int on = 1;
   int ret;

   CL_LOG(CL_LOG_INFO,"setting up TCP request handler ...");
    
   if (connection == NULL ) {
      CL_LOG(CL_LOG_ERROR,"no connection");
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      CL_LOG(CL_LOG_ERROR,"framework not initalized");
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if ( private->server_port < 0 ) {
      CL_LOG(CL_LOG_ERROR,cl_get_error_text(CL_RETVAL_NO_PORT_ERROR));
      return CL_RETVAL_NO_PORT_ERROR;
   }

   /* create socket */
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      CL_LOG(CL_LOG_ERROR,"could not create socket");
      return CL_RETVAL_CREATE_SOCKET;
   }

   if (sockfd < 3) {
      CL_LOG_INT(CL_LOG_WARNING, "The file descriptor is < 3. Will dup fd to be >= 3! fd value: ", sockfd);
      ret = sge_dup_fd_above_stderr(&sockfd);
      if (ret != 0) {
         CL_LOG_INT(CL_LOG_ERROR, "can't dup socket fd to be >=3, errno = ", ret);
         shutdown(sockfd, 2);
         close(sockfd);
         sockfd = -1;
         cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_DUP_SOCKET_FD_ERROR, MSG_CL_COMMLIB_CANNOT_DUP_SOCKET_FD);
         return CL_RETVAL_DUP_SOCKET_FD_ERROR;
      }
      CL_LOG_INT(CL_LOG_INFO, "fd value after dup: ", sockfd);
   }

   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) != 0) {
      CL_LOG(CL_LOG_ERROR,"could not set SO_REUSEADDR");
      return CL_RETVAL_SETSOCKOPT_ERROR;
   }

   /* bind an address to socket */
   /* TODO FEATURE: we can also try to use a specified port range */
   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_port = htons(private->server_port);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      shutdown(sockfd, 2);
      close(sockfd);
      CL_LOG_INT(CL_LOG_ERROR, "could not bind server socket port:", private->server_port);
      return CL_RETVAL_BIND_SOCKET;
   }

   if (private->server_port == 0) {
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HPUX)
      int length;
#else
      socklen_t length;
#endif
      length = sizeof(serv_addr);
      /* find out assigned port number and pass it to caller */
      if (getsockname(sockfd,(struct sockaddr *) &serv_addr, &length ) == -1) {
         shutdown(sockfd, 2);
         close(sockfd);
         CL_LOG_INT(CL_LOG_ERROR, "could not bind random server socket port:", private->server_port);
         return CL_RETVAL_BIND_SOCKET;
      }
      private->server_port = ntohs(serv_addr.sin_port);
      CL_LOG_INT(CL_LOG_INFO,"random server port is:", private->server_port);
   }

   /* if only_prepare_service is enabled we don't want to set the port into
      listen mode now, we have to do it later */
   private->pre_sockfd = sockfd;
   if (only_prepare_service == true) {
      CL_LOG_INT(CL_LOG_INFO,"service socket prepared for listen, using sockfd=", sockfd);
      return CL_RETVAL_OK;
   }
  
   return cl_com_tcp_connection_request_handler_setup_finalize(connection);
}


/****** cl_tcp_framework/cl_com_tcp_connection_request_handler_cleanup() *******
*  NAME
*     cl_com_tcp_connection_request_handler_cleanup() -- shutdown service
*
*  SYNOPSIS
*     int cl_com_tcp_connection_request_handler_cleanup(cl_com_connection_t* 
*     connection) 
*
*  FUNCTION
*     This function will shutdown a service connection, created with the
*     cl_com_tcp_connection_request_handler_setup() function. Free the connection
*     with cl_tcp_close_connection() has to be done by caller.
*
*  INPUTS
*     cl_com_connection_t* connection - Connection to shutdown
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success 
*
*  SEE ALSO
*     cl_tcp_framework/cl_com_tcp_connection_request_handler()
*     cl_tcp_framework/cl_com_tcp_connection_request_handler_setup()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_connection_request_handler_cleanup()"
int cl_com_tcp_connection_request_handler_cleanup(cl_com_connection_t* connection) {

   cl_com_tcp_private_t* private = NULL;

   CL_LOG(CL_LOG_INFO,"cleanup of request handler ...");
   if (connection == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   shutdown(private->sockfd, 2);
   close(private->sockfd);
   private->sockfd = -1;

   return CL_RETVAL_OK;
}





/* caller must free new_connection pointer */

/****** cl_tcp_framework/cl_com_tcp_connection_request_handler() ***************
*  NAME
*     cl_com_tcp_connection_request_handler() -- Check for connection requests
*
*  SYNOPSIS
*     int cl_com_tcp_connection_request_handler(cl_com_connection_t* 
*     connection, cl_com_connection_t** new_connection, int timeout_val_sec, 
*     int timeout_val_usec) 
*
*  FUNCTION
*     This function will do a select call for the service connection file de-
*     scriptor. If the select returns with no error the connection is accepted
*     (via accept()) and a new connection structure ( cl_com_connection_t )
*     is malloced. The new connection will get all default settings from the
*     service connection struct. 
*
*     This function has to fill all struct information for the new connection
*     ( cl_com_connection_t type)
*
*  INPUTS
*     cl_com_connection_t* connection      - pointer to service connection
*     cl_com_connection_t** new_connection - NULL
*     int timeout_val_sec                  - timeout value in sec (for select)
*     int timeout_val_usec                 - timeout value in usec (for select)
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success 
*     an address to a new connection in new_connection parameter
*
*  SEE ALSO
*     cl_tcp_framework/cl_com_tcp_connection_request_handler_setup()
*     cl_tcp_framework/cl_com_tcp_connection_request_handler_cleanup()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_connection_request_handler()"
int cl_com_tcp_connection_request_handler(cl_com_connection_t* connection, cl_com_connection_t** new_connection) {
   cl_com_connection_t* tmp_connection = NULL;
   struct sockaddr_in cli_addr;
   int new_sfd = 0;
   int sso;
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HPUX)
   int fromlen = 0;
#else
   socklen_t fromlen = 0;
#endif
   int retval;
   cl_com_tcp_private_t* private = NULL;
   
   if (connection == NULL || new_connection == NULL) {
      CL_LOG(CL_LOG_ERROR,"no connection or no accept connection");
      return CL_RETVAL_PARAMS;
   }

   if (*new_connection != NULL) {
      CL_LOG(CL_LOG_ERROR,"accept connection is not free");
      return CL_RETVAL_PARAMS;
   }
   
   private = cl_com_tcp_get_private(connection);
   if (private == NULL) {
      CL_LOG(CL_LOG_ERROR,"framework is not initalized");
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   if (connection->service_handler_flag != CL_COM_SERVICE_HANDLER) {
      CL_LOG(CL_LOG_ERROR,"connection is no service handler");
      return CL_RETVAL_NOT_SERVICE_HANDLER;
   }

   /* got new connect */
   fromlen = sizeof(cli_addr);
   memset((char *) &cli_addr, 0, sizeof(cli_addr));
   new_sfd = accept(private->sockfd, (struct sockaddr *) &cli_addr, &fromlen);
   if (new_sfd > -1) {
      char* resolved_host_name = NULL;
      cl_com_tcp_private_t* tmp_private = NULL;

      if (new_sfd < 3) {
         CL_LOG_INT(CL_LOG_WARNING, "The file descriptor is < 3. Will dup fd to be >= 3! fd value: ", new_sfd);
         retval = sge_dup_fd_above_stderr(&new_sfd);
         if (retval != 0) {
            CL_LOG_INT(CL_LOG_ERROR, "can't dup socket fd to be >=3, errno = ", retval);
            shutdown(new_sfd, 2);
            close(new_sfd);
            new_sfd = -1;
            cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_DUP_SOCKET_FD_ERROR, MSG_CL_COMMLIB_CANNOT_DUP_SOCKET_FD);
            return CL_RETVAL_DUP_SOCKET_FD_ERROR;
         }
         CL_LOG_INT(CL_LOG_INFO, "fd value after dup: ", new_sfd);
      }

      cl_com_cached_gethostbyaddr(&(cli_addr.sin_addr), &resolved_host_name, NULL, NULL); 
      if (resolved_host_name != NULL) {
         CL_LOG_STR(CL_LOG_INFO,"new connection from host", resolved_host_name);
      } else {
         CL_LOG(CL_LOG_WARNING,"could not resolve incoming hostname");
      }

      fcntl(new_sfd, F_SETFL, O_NONBLOCK);         /* HP needs O_NONBLOCK, was O_NDELAY */
      sso = 1;
#if defined(SOLARIS) && !defined(SOLARIS64)
      if (setsockopt(new_sfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &sso, sizeof(int)) == -1)
#else
      if (setsockopt(new_sfd, IPPROTO_TCP, TCP_NODELAY, &sso, sizeof(int))== -1)
#endif
      {
         CL_LOG(CL_LOG_ERROR,"could not set TCP_NODELAY");
      }
      /* here we can investigate more information about the client */
      /* ntohs(cli_addr.sin_port) ... */

      tmp_connection = NULL;
      /* setup a tcp connection where autoclose is still undefined */
      if ( (retval=cl_com_tcp_setup_connection(&tmp_connection, 
                                               private->server_port,
                                               private->connect_port,
                                               connection->data_flow_type, 
                                               CL_CM_AC_UNDEFINED,
                                               connection->framework_type,
                                               connection->data_format_type,
                                               connection->tcp_connect_mode)) != CL_RETVAL_OK) {
         cl_com_tcp_close_connection(&tmp_connection); 
         if (resolved_host_name != NULL) {
            sge_free(&resolved_host_name);
         }
         shutdown(new_sfd, 2);
         close(new_sfd);
         return retval;
      }

      /*
       * set resolved hostname of client (NULL is valid because checks are done when
       * connection state is set to connection->connection_state = CL_CONNECTING; and
       * new connection is added to connection list)
       */
      tmp_connection->client_host_name = resolved_host_name;

      /* setup cl_com_tcp_private_t */
      tmp_private = cl_com_tcp_get_private(tmp_connection);
      if (tmp_private != NULL) {
         tmp_private->sockfd = new_sfd;   /* fd from accept() call */
         tmp_private->connect_in_port = ntohs(cli_addr.sin_port);
      }

      *new_connection = tmp_connection;
   }
   return CL_RETVAL_OK;
}



/*
  fill connection struct with client information
  ==============================================

  connection = connection to fill in client information for new connect
  sockfd = new connection socket file descriptor (from accept call)
*/

/****** cl_tcp_framework/cl_com_tcp_get_private() ******************************
*  NAME
*     cl_com_tcp_get_private() -- get cl_com_tcp_private_t struct of a connection
*
*  SYNOPSIS
*     static cl_com_tcp_private_t* cl_com_tcp_get_private(cl_com_connection_t* 
*     connection) 
*
*  FUNCTION
*     This function returns the com_private pointer of the connection and does
*     a type cast.
*
*  INPUTS
*     cl_com_connection_t* connection - tcp connection to get private struct
*
*  RESULT
*     static cl_com_tcp_private_t* - pointer to private tcp data of tcp connection
*
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_get_private()"
static cl_com_tcp_private_t* cl_com_tcp_get_private(cl_com_connection_t* connection) {  /* CR check */
   if (connection != NULL) {
      return connection->com_private;
   }
   return NULL;
}





/****** cl_tcp_framework/cl_com_tcp_open_connection_request_handler() **********
*  NAME
*     cl_com_tcp_open_connection_request_handler() -- ??? 
*
*  SYNOPSIS
*     int cl_com_tcp_open_connection_request_handler(cl_raw_list_t* 
*     connection_list, int timeout_val) 
*
*  FUNCTION
*     First action of this function is do set the data_read_flag of each
*     connection in the list to CL_COM_DATA_NOT_READY.
*
*     After that this function will do a select on all file descriptors in 
*     the list for reading. If the select returns that there is data for a 
*     connection the data_read_flag of the connection 
*     ( struct cl_com_connection_t ) is set.
*
*  INPUTS
*     cl_raw_list_t* connection_list - connection list
*     int timeout_val                - timeout for select
*
*  RESULT
*     int - CL_RETVAL_XXXX error or CL_RETVAL_OK on success 
*
*  SEE ALSO
*     cl_communication/cl_com_open_connection_request_handler()
*******************************************************************************/
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_tcp_open_connection_request_handler()"
int cl_com_tcp_open_connection_request_handler(cl_com_poll_t* poll_handle, cl_com_handle_t* handle, cl_raw_list_t* connection_list, cl_com_connection_t* service_connection, int timeout_val_sec, int timeout_val_usec, cl_select_method_t select_mode )
{
   int select_back;
   cl_connection_list_elem_t* con_elem = NULL;
   cl_com_connection_t*  connection = NULL;
   cl_com_tcp_private_t* con_private = NULL;

   int max_fd = -1;
   int server_fd = -1;
   int retval = CL_RETVAL_UNKNOWN;
   int do_read_select = 0;
   int do_write_select = 0;
   int my_errno = 0;
   int nr_of_descriptors = 0;
   cl_connection_list_data_t* ldata = NULL;
   int socket_error = 0;
   int get_sock_opt_error = 0;

#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HPUX)
   int socklen = sizeof(socket_error);
#else
   socklen_t socklen = sizeof(socket_error);
#endif

   struct pollfd* ufds = NULL;
   cl_com_connection_t** ufds_con = NULL;
   unsigned long ufds_index = 0;
   unsigned long fd_index = 0;
   int fd_offset = 2;
   struct timeval timeout;

   if (poll_handle == NULL) {
      CL_LOG(CL_LOG_ERROR, "poll_handle == NULL");
      return CL_RETVAL_PARAMS;
   }

   if (handle == NULL) {
      CL_LOG(CL_LOG_ERROR,"handle == NULL");
      return CL_RETVAL_PARAMS;
   }

   if (connection_list == NULL) {
      CL_LOG(CL_LOG_ERROR,"no connection list");
      return CL_RETVAL_PARAMS;
   }

   if (select_mode == CL_RW_SELECT || select_mode == CL_R_SELECT) {
      do_read_select = 1;
   }
   if (select_mode == CL_RW_SELECT || select_mode == CL_W_SELECT) {
      do_write_select = 1;
   }

   /* TODO (SH & CR): There seems to be something missing because the need of the low timout. */
   /* TODO Handle no yet write-ready file descriptors, that write thread does not going to sleep */
   /* TODO Remove trigger_write_thread from read thread */
   if (select_mode == CL_W_SELECT) {
      timeout.tv_sec = 0;
      timeout.tv_usec = 5*1000; /* 5 ms */
   } else {
      timeout.tv_sec = timeout_val_sec; 
      timeout.tv_usec = timeout_val_usec;
   }

   /* lock list */
   if ( cl_raw_list_lock(connection_list) != CL_RETVAL_OK) {
      CL_LOG(CL_LOG_ERROR,"could not lock connection list");
      return CL_RETVAL_LOCK_ERROR;
   }

   if ( connection_list->list_data == NULL) {
      cl_raw_list_unlock(connection_list);
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   } else {
      ldata = (cl_connection_list_data_t*) connection_list->list_data;
   }

   /* first check if we have a poll_array of the correct size*/
   fd_offset = fd_offset + cl_raw_list_get_elem_count(handle->file_descriptor_list);
   if (poll_handle->poll_fd_count != handle->max_open_connections + fd_offset) {
      /* max_open_connections might have changed */
      int poll_return = cl_com_malloc_poll_array(poll_handle, handle->max_open_connections + fd_offset);
      if (poll_return != CL_RETVAL_OK) {
         cl_raw_list_unlock(connection_list);
         return poll_return;
      }
   }

   /* check poll_array size */
   if (poll_handle->poll_fd_count < cl_raw_list_get_elem_count(connection_list) + fd_offset) {
      /* This should not happen, but we want to be on the save side */
      int poll_return = cl_com_malloc_poll_array(poll_handle, cl_raw_list_get_elem_count(connection_list) + fd_offset);
      CL_LOG(CL_LOG_WARNING, "max_open_connection count < current connection size - this must NOT happen!");
      if (poll_return != CL_RETVAL_OK) {
         cl_raw_list_unlock(connection_list);
         return poll_return;
      }
   }

   /* init poll_array data */
   ufds = poll_handle->poll_array;
   ufds_con = poll_handle->poll_con;

   /* cleanup first arrays */
   ufds_con[ufds_index] = NULL;
   memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));

   if (service_connection != NULL && do_read_select != 0) {
      cl_com_tcp_private_t* private = NULL;
      int tmp_retval = CL_RETVAL_OK;

      /* this is to come out of select when for new connections */
      if(cl_com_tcp_get_private(service_connection) == NULL ) {
         CL_LOG(CL_LOG_ERROR,"service framework is not initalized");
         cl_raw_list_unlock(connection_list);
         return CL_RETVAL_NO_FRAMEWORK_INIT;
      }
      if( service_connection->service_handler_flag != CL_COM_SERVICE_HANDLER) {
         CL_LOG(CL_LOG_ERROR,"service connection is no service handler");
         cl_raw_list_unlock(connection_list);
         return CL_RETVAL_NOT_SERVICE_HANDLER;
      }
      service_connection->is_read_selected = false;
      private = cl_com_tcp_get_private(service_connection);
      /* check if service is already in listen mode. This might happen
         when only_prepare_service was set to true at 
         cl_com_tcp_connection_request_handler_setup() */
      if (private->sockfd == -1 && private->pre_sockfd != -1 ) {
         /* finalize server socket setup */
         tmp_retval = cl_com_tcp_connection_request_handler_setup_finalize(service_connection);
         if (tmp_retval != CL_RETVAL_OK ) {
            cl_raw_list_unlock(connection_list);
            return tmp_retval;
         } else {
            private->pre_sockfd = -1;
         }
      }
      server_fd = private->sockfd;
      max_fd = MAX(max_fd,server_fd);

      ufds_con[ufds_index] = service_connection;
      ufds[ufds_index].fd = server_fd;
      ufds[ufds_index].events = POLLIN|POLLPRI;
      ufds_index++;
      ufds_con[ufds_index] = NULL;
      memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
      service_connection->is_read_selected = true;
      nr_of_descriptors++;
      service_connection->data_read_flag = CL_COM_DATA_NOT_READY;
   }

   /* reset connection data_read flags */
   con_elem = cl_connection_list_get_first_elem(connection_list);
   while(con_elem) {
      connection = con_elem->connection;

      if ((con_private=cl_com_tcp_get_private(connection)) == NULL) {
         cl_raw_list_unlock(connection_list);
         CL_LOG(CL_LOG_ERROR,"no private data pointer");
         return CL_RETVAL_NO_FRAMEWORK_INIT;
      }
      if (do_read_select != 0) {
         connection->is_read_selected = false;
      }
      if (do_write_select != 0) {
         connection->is_write_selected = false;
      }

      if (con_private->sockfd >= 0) {
         switch(connection->framework_type) {
            case CL_CT_TCP: {
               switch (connection->connection_state) {
                  case CL_CLOSING:
                     if (connection->connection_sub_state != CL_COM_SHUTDOWN_DONE) {
                        if (do_read_select != 0) {
                           ufds[ufds_index].fd = con_private->sockfd;
                           ufds[ufds_index].events = POLLIN|POLLPRI;
                           ufds_con[ufds_index] = connection;
                           connection->is_read_selected = true;
                           max_fd = MAX(max_fd,con_private->sockfd);
                           nr_of_descriptors++;
                           connection->data_read_flag = CL_COM_DATA_NOT_READY;
                        }
                        if (connection->data_write_flag == CL_COM_DATA_READY && do_write_select != 0) {
                           /* this is to come out of select when data is ready to write */
                           ufds[ufds_index].fd = con_private->sockfd;
                           ufds[ufds_index].events |= POLLOUT;
                           ufds_con[ufds_index] = connection;
                           connection->is_write_selected = true;
                           max_fd = MAX(max_fd, con_private->sockfd);
                           connection->fd_ready_for_write = CL_COM_DATA_NOT_READY;
                        }

                        if (ufds[ufds_index].events) {
                           ufds_index++;
                           ufds_con[ufds_index] = NULL;
                           memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
                        }
                     }
                     break;
                  case CL_CONNECTED:
                     if (connection->connection_sub_state != CL_COM_DONE) {
                        if (do_read_select != 0) {
                           ufds[ufds_index].fd = con_private->sockfd;
                           ufds[ufds_index].events = POLLIN|POLLPRI;
                           ufds_con[ufds_index] = connection;
                           connection->is_read_selected = true;
                           max_fd = MAX(max_fd,con_private->sockfd);
                           nr_of_descriptors++;
                           connection->data_read_flag = CL_COM_DATA_NOT_READY;
                        }
                        if (connection->data_write_flag == CL_COM_DATA_READY && do_write_select != 0) {
                           /* this is to come out of select when data is ready to write */
                           ufds[ufds_index].fd = con_private->sockfd;
                           ufds[ufds_index].events |= POLLOUT;
                           ufds_con[ufds_index] = connection;
                           connection->is_write_selected = true;
                           max_fd = MAX(max_fd, con_private->sockfd);
                           connection->fd_ready_for_write = CL_COM_DATA_NOT_READY;
                        }

                        if (ufds[ufds_index].events) {
                           ufds_index++;
                           ufds_con[ufds_index] = NULL;
                           memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
                        }
                     }
                     break;
                  case CL_CONNECTING:
                     if (do_read_select != 0) {
                        ufds[ufds_index].fd = con_private->sockfd;
                        ufds[ufds_index].events = POLLIN|POLLPRI;
                        ufds_con[ufds_index] = connection;
                        connection->is_read_selected = true;
                        max_fd = MAX(max_fd,con_private->sockfd);
                        nr_of_descriptors++;
                        connection->data_read_flag = CL_COM_DATA_NOT_READY;
                     }
                     if (connection->data_write_flag == CL_COM_DATA_READY && do_write_select != 0) {
                        /* this is to come out of select when data is ready to write */
                        ufds[ufds_index].fd = con_private->sockfd;
                        ufds[ufds_index].events |= POLLOUT;
                        ufds_con[ufds_index] = connection;
                        connection->is_write_selected = true;
                        max_fd = MAX(max_fd, con_private->sockfd);
                        connection->fd_ready_for_write = CL_COM_DATA_NOT_READY;
                     }

                     if (ufds[ufds_index].events) {
                        ufds_index++;
                        ufds_con[ufds_index] = NULL;
                        memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
                     }
                     break;
                  case CL_OPENING:
                     CL_LOG_STR(CL_LOG_DEBUG,"connection_sub_state:", cl_com_get_connection_sub_state(connection));
                     switch(connection->connection_sub_state) {
                        case CL_COM_OPEN_INIT:
                        case CL_COM_OPEN_CONNECT: {
                           if (do_read_select != 0) {
                              connection->data_read_flag = CL_COM_DATA_READY;
                           }
                           break;
                        }
                        case CL_COM_OPEN_CONNECTED:
                        case CL_COM_OPEN_CONNECT_IN_PROGRESS: {
                           if (do_read_select != 0) {
                              ufds[ufds_index].fd = con_private->sockfd;
                              ufds[ufds_index].events = POLLIN|POLLPRI;
                              ufds_con[ufds_index] = connection;
                              connection->is_read_selected = true;
                              max_fd = MAX(max_fd,con_private->sockfd);
                              nr_of_descriptors++;
                              connection->data_read_flag = CL_COM_DATA_NOT_READY;
                           }
                           if ( do_write_select != 0) {
                              ufds[ufds_index].fd = con_private->sockfd;
                              ufds[ufds_index].events |= POLLOUT;
                              ufds_con[ufds_index] = connection;
                              connection->is_write_selected = true;
                              max_fd = MAX(max_fd, con_private->sockfd);
                              connection->fd_ready_for_write = CL_COM_DATA_NOT_READY;
                              connection->data_write_flag = CL_COM_DATA_READY;
                           }
                           if (ufds[ufds_index].events) {
                              ufds_index++;
                              ufds_con[ufds_index] = NULL;
                              memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
                           }

                           break;
                        }
                        default:
                           break;
                     }
                     break;
                  case CL_DISCONNECTED:
                  case CL_ACCEPTING:
                     break;
               }
               break;
            }
            case CL_CT_UNDEFINED:
            case CL_CT_SSL: {
               CL_LOG_STR(CL_LOG_WARNING,"ignoring unexpected connection type:",
                          cl_com_get_framework_type(connection));
            }
         }
      }
      con_elem = cl_connection_list_get_next_elem(con_elem);
   }

   /* add the external file descriptors to the FD_SETS */
   if (handle->file_descriptor_list != NULL){
      cl_fd_list_elem_t* elem = NULL;
      cl_raw_list_lock(handle->file_descriptor_list);
      elem = cl_fd_list_get_first_elem(handle->file_descriptor_list);
      /* TODO(SH): Use memsetting like in the sceanrio above */
      while (elem) {
         if(do_read_select == 1 || do_write_select == 1){
            ufds[ufds_index].fd = elem->data->fd;
            if(do_read_select == 1){
               ufds[ufds_index].events |= POLLIN|POLLPRI;
            }
            if(do_write_select == 1){
               if (elem->data->ready_for_writing == true) {
                  ufds[ufds_index].events |= POLLOUT;
               } 
            }
            max_fd = MAX(max_fd, elem->data->fd);
            ufds_index++;
            ufds_con[ufds_index] = NULL;
            memset(&(ufds[ufds_index]), 0, sizeof(struct pollfd));
            nr_of_descriptors++;
         }
         
         elem = cl_fd_list_get_next_elem(elem);
      }
      cl_raw_list_unlock(handle->file_descriptor_list);
   }

   /* we don't have any file descriptor for select(), find out why: */
   if (max_fd == -1) {
      CL_LOG_INT(CL_LOG_INFO,"max fd =", max_fd);
/* TODO: remove CL_W_SELECT and CL_R_SELECT handling and use one handling for 
         CL_W_SELECT, CL_R_SELECT and CL_RW_SELECT ? */
      if ( select_mode == CL_W_SELECT ) {
         /* return immediate for only write select ( only called by write thread) */
         cl_raw_list_unlock(connection_list); 
         CL_LOG(CL_LOG_INFO,"returning, because of no select descriptors (CL_W_SELECT)");
         return CL_RETVAL_NO_SELECT_DESCRIPTORS;
      } else {

         /* (only when not multithreaded): 
          *    don't return immediately when the last call to this function was also
          *    with no possible descriptors! ( which may be caused by a not connectable service )
          *    This must be done to prevent the application to poll endless ( with 100% CPU usage)
          *
          *    we have no file descriptors, but we do a select with standard timeout
          *    because we don't want to overload the cpu by endless trigger() calls 
          *    from application when there is no connection client 
          *    (no descriptors part 1)
          *
          *    we have a handler of the connection list, try to find out if 
          *    this is the first call without guilty file descriptors 
          */
         if ( ldata->select_not_called_count < 3 ) {
            CL_LOG_INT(CL_LOG_INFO, "no usable file descriptor for select() call nr.:", ldata->select_not_called_count);
            ldata->select_not_called_count += 1;
            cl_raw_list_unlock(connection_list); 
            return CL_RETVAL_NO_SELECT_DESCRIPTORS;
         } else {
            CL_LOG(CL_LOG_WARNING, "no usable file descriptors (repeated!) - select() will be used for wait");
            ldata->select_not_called_count = 0;
            CL_LOG(CL_LOG_INFO,"no select descriptors");
            cl_raw_list_unlock(connection_list);
            sge_sleep(timeout.tv_sec, timeout.tv_usec);
            return CL_RETVAL_NO_SELECT_DESCRIPTORS;
         }
      }
   }

   
   /* TODO: Fix this problem (multithread mode):
         -  find a way to wake up select when a new connection was added by another thread
            (perhaps with dummy read file descriptor)
   */
    
   if ((nr_of_descriptors != ldata->last_nr_of_descriptors) &&
       (nr_of_descriptors == 1 && service_connection != NULL && do_read_select != 0)) {
      /* This is to return as fast as possible if this connection has a service and
          a client was disconnected */

      /* a connection is done and no more connections (beside service connection itself) is alive,
         return to application as fast as possible, don't wait for a new connect */
      ldata->last_nr_of_descriptors = nr_of_descriptors;
      cl_raw_list_unlock(connection_list); 
      CL_LOG(CL_LOG_INFO,"last connection closed");
      retval = CL_RETVAL_NO_SELECT_DESCRIPTORS;
   } else {

      ldata->last_nr_of_descriptors = nr_of_descriptors;

      cl_raw_list_unlock(connection_list); 

      errno = 0;
      select_back = poll(ufds, ufds_index, timeout.tv_sec*1000 + timeout.tv_usec/1000);

      my_errno = errno;


      switch(select_back) {
         case -1: {
            /*
             * poll() and select() set errno to EINTR if interrupted
             */
            if (my_errno == EINTR) {
               CL_LOG(CL_LOG_WARNING,"select interrupted (errno=EINTR)");
               retval = CL_RETVAL_SELECT_INTERRUPT;
               break;
            }

            CL_LOG_STR(CL_LOG_ERROR,"select error", strerror(my_errno));
            retval = CL_RETVAL_SELECT_ERROR;
            
            /*
             * 1) select() set errno to EBADF for not valid file descriptors
             * 2) poll() and select() set errno to EINVAL for file descriptors that are
             *    > OPEN_MAX or FD_SETSIZE
             * => In both cases we check the filedescriptors with get_sock_opt()
             */
            if (my_errno == EBADF || my_errno == EINVAL) {
               if (my_errno == EBADF) {
                  CL_LOG(CL_LOG_WARNING, "errno=EBADF, checking file descriptors");
               } else {
                  CL_LOG(CL_LOG_WARNING, "errno=EINVAL, checking file descriptors");
               }
               /* now check all file descriptors and close those which errors */
               cl_raw_list_lock(connection_list); 
               con_elem = cl_connection_list_get_first_elem(connection_list);
               while(con_elem) {
                  connection  = con_elem->connection;
                  con_private = cl_com_tcp_get_private(connection);
                  socket_error = 0;
#if defined(SOLARIS) && !defined(SOLARIS64) 
                  get_sock_opt_error = getsockopt(con_private->sockfd,SOL_SOCKET, SO_ERROR, (void*)&socket_error, &socklen);
#else
                  get_sock_opt_error = getsockopt(con_private->sockfd,SOL_SOCKET, SO_ERROR, &socket_error, &socklen);
#endif
                  if (socket_error != 0 || get_sock_opt_error != 0) {
                     connection->connection_state = CL_CLOSING;
                     connection->connection_sub_state = CL_COM_DO_SHUTDOWN;
                     CL_LOG_STR(CL_LOG_ERROR, "select() or poll() - socket error is: ", strerror(socket_error));
                     cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_SELECT_ERROR, strerror(socket_error));

                     if (connection->remote            != NULL && 
                         connection->remote->comp_host != NULL &&
                         connection->remote->comp_name != NULL ) {
                        char tmp_string[1024];
                        snprintf(tmp_string, 1024, MSG_CL_COMMLIB_CLOSING_SSU,
                                 connection->remote->comp_host,
                                 connection->remote->comp_name,
                                 sge_u32c(connection->remote->comp_id));
                        CL_LOG_STR(CL_LOG_ERROR, "select error:", tmp_string);
                        cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_SELECT_ERROR, tmp_string);
                     }
                  }
                  con_elem = cl_connection_list_get_next_elem(con_elem);
               } /* while */
               cl_raw_list_unlock(connection_list);
               /* look for a broken external file descriptor call its callback function and remove it afterwards */
               if (handle->file_descriptor_list != NULL) {
                  cl_fd_list_elem_t* elem = NULL;
                  cl_raw_list_lock(handle->file_descriptor_list);
                  for (fd_index = 0; fd_index < ufds_index; fd_index++){
                     if (ufds_con[fd_index] != NULL) {
                        continue;
                     }
                     if(ufds[fd_index].revents & (POLLHUP|POLLERR|POLLNVAL)) {
                        elem = cl_fd_list_get_first_elem(handle->file_descriptor_list);
                        while(elem) {
                           if (elem->data->fd == ufds[fd_index].fd) {
                              elem->data->callback(elem->data->fd, 
                                                   elem->data->read_ready, 
                                                   elem->data->write_ready, 
                                                   elem->data->user_data, 
                                                   ufds[fd_index].revents);
                              cl_fd_list_unregister_fd(handle->file_descriptor_list, elem, 0);
                              break;
                           }
                           elem = cl_fd_list_get_next_elem(elem);
                        }
                     }
                  }
                  cl_raw_list_unlock(handle->file_descriptor_list);
               }
               break;
            }
            CL_LOG_INT(CL_LOG_WARNING, "unexpected errno value: ", (int) my_errno);
            break;
         }
         case 0:
            CL_LOG(CL_LOG_INFO,"----->>>>>>>>>>> poll() timeout <<<<<<<<<<<<<<<<<---");
            retval = CL_RETVAL_SELECT_TIMEOUT;
            break;
         default:
         {
            cl_raw_list_lock(connection_list); 
            /* now set the read flags for connections, where data is available */
            for (fd_index = 0; fd_index < ufds_index ; fd_index++) {
               connection = ufds_con[fd_index];
               if (connection != NULL) {
                  if (do_read_select != 0) {
                     if (ufds[fd_index].revents & (POLLIN|POLLPRI)) {
                        connection->data_read_flag = CL_COM_DATA_READY;
                     }
                     connection->is_read_selected = false;
                  }
                  if (do_write_select != 0) {
                     if (ufds[fd_index].revents & POLLOUT) {
                        connection->fd_ready_for_write = CL_COM_DATA_READY;
                     }
                     connection->is_write_selected = false;
                  }

                  /* Do we have poll errors ? */
                  if ((ufds[fd_index].revents & (POLLERR|POLLHUP|POLLNVAL)) && connection != service_connection) {
                     if (ufds[fd_index].revents & POLLNVAL) {
                         CL_LOG_INT(CL_LOG_WARNING, "poll() revents POLLNVAL is set - checking file descriptor: ", (int)ufds[fd_index].fd);
                     }
                     if (ufds[fd_index].revents & POLLERR) {
                         CL_LOG_INT(CL_LOG_WARNING, "poll() revents POLLERR is set - checking file descriptor: ", (int)ufds[fd_index].fd);
                     }
                     if (ufds[fd_index].revents & POLLHUP) {
                         CL_LOG_INT(CL_LOG_WARNING, "poll() revents POLLHUP is set - checking file descriptor: ", (int)ufds[fd_index].fd);
                     }
                     /* check the connection */
                     con_private = cl_com_tcp_get_private(connection);
                     socket_error = 0;
#if defined(SOLARIS) && !defined(SOLARIS64) 
                     get_sock_opt_error = getsockopt(con_private->sockfd,SOL_SOCKET, SO_ERROR, (void*)&socket_error, &socklen);
#else
                     get_sock_opt_error = getsockopt(con_private->sockfd,SOL_SOCKET, SO_ERROR, &socket_error, &socklen);
#endif
                     if (socket_error != 0 || get_sock_opt_error != 0) {
                        connection->connection_state = CL_CLOSING;
                        connection->connection_sub_state = CL_COM_DO_SHUTDOWN;
                        CL_LOG_STR(CL_LOG_ERROR, "socket error: ", strerror(socket_error));
                        cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_SELECT_ERROR, strerror(socket_error));
                        if (connection->remote            != NULL && 
                            connection->remote->comp_host != NULL &&
                            connection->remote->comp_name != NULL ) {
                           char tmp_string[1024];
                           snprintf(tmp_string, 1024, MSG_CL_COMMLIB_CLOSING_SSU,
                                    connection->remote->comp_host,
                                    connection->remote->comp_name,
                                    sge_u32c(connection->remote->comp_id));
                           CL_LOG_STR(CL_LOG_ERROR, "poll() revents error:", tmp_string);
                           cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_SELECT_ERROR, tmp_string);
                        }
                     }
                  }
               }
               /* look for external file descriptors and set its ready flags */
               if(handle->file_descriptor_list != NULL){
                  cl_fd_list_elem_t* elem = NULL;
                  cl_raw_list_lock(handle->file_descriptor_list);
                  elem = cl_fd_list_get_first_elem(handle->file_descriptor_list);
                  while(elem) {
                     if (elem->data->fd == ufds[fd_index].fd) {
                        if(do_read_select == 1) {
                           if(ufds[fd_index].revents & (POLLIN|POLLPRI)){
                              elem->data->read_ready = true;
                           }else{
                              elem->data->read_ready = false;
                           }
                        }
                        if(do_write_select == 1) {
                           if(ufds[fd_index].revents & POLLOUT){
                              elem->data->write_ready = true;
                           }else{
                              elem->data->write_ready = false;
                           }
                        }
                     }
                     elem = cl_fd_list_get_next_elem(elem);
                  }
                  cl_raw_list_unlock(handle->file_descriptor_list);
               }
            }
            cl_raw_list_unlock(connection_list);
            return CL_RETVAL_OK; /* OK - done */
         } /* default */
      } /* switch */

   }
   /* 
    * reset all is_XXXXX_selected flags for the connection
    */
   cl_raw_list_lock(connection_list); 
   for (fd_index = 0; fd_index < ufds_index ; fd_index++) {
      connection = ufds_con[fd_index];
      if (connection != NULL) {
         if (do_read_select != 0) {
            connection->is_read_selected = false;
         }
         if (do_write_select != 0) {
            connection->is_write_selected = false;
         }
      }
   }
   cl_raw_list_unlock(connection_list);
   return retval;
}

