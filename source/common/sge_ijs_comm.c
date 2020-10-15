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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#ifndef  TIOCGWINSZ
#include <sys/ioctl.h>  /* 44BSD requires this too */
#endif
#include <time.h>

#include "uti/sge_rmon.h"
#include "uti/sge_arch.h"
#include "uti/config_file.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_time.h"

#include "gdi/sge_security.h"
#include "gdi/msg_gdilib.h"

#include "sgeobj/sge_utility.h"

#include "comm/cl_data_types.h"
#include "comm/cl_commlib.h"
#include "comm/cl_endpoint_list.h"
#include "comm/lists/msg_commlistslib.h"

#include "sge_ijs_comm.h"

extern sig_atomic_t received_signal;

/*
 * TODO: Cleanup / Headers
 * This is just slightly modified copy of the gdi commlib error handling,
 * perhaps it's possible to do minimal changes in the original functions
 * and remove these.
 */
static pthread_mutex_t ijs_general_communication_error_mutex = PTHREAD_MUTEX_INITIALIZER;
/* local static struct to store communication errors. The boolean
 * values com_access_denied and com_endpoint_not_unique will never be
 * restored to false again 
 */
typedef struct sge_gdi_com_error_type {
   int  com_error;                        /* current commlib error */
   bool com_was_error;                    /* set if there was an communication error (but not CL_RETVAL_ACCESS_DENIED or CL_RETVAL_ENDPOINT_NOT_UNIQUE)*/
   int  com_last_error;                   /* last logged commlib error */
   bool com_access_denied;                /* set when commlib reports CL_RETVAL_ACCESS_DENIED */
   int  com_access_denied_counter;        /* counts access denied errors (TODO: workaround for BT: 6350264, IZ: 1893) */
   unsigned long com_access_denied_time; /* timeout for counts access denied errors (TODO: workaround for BT: 6350264, IZ: 1893) */
   bool com_endpoint_not_unique;          /* set when commlib reports CL_RETVAL_ENDPOINT_NOT_UNIQUE */
   int  com_endpoint_not_unique_counter;  /* counts access denied errors (TODO: workaround for BT: 6350264, IZ: 1893) */
   unsigned long com_endpoint_not_unique_time; /* timeout for counts access denied errors (TODO: workaround for BT: 6350264, IZ: 1893) */
} sge_gdi_com_error_t;


static sge_gdi_com_error_t ijs_communication_error = {CL_RETVAL_OK,
                                                      false,
                                                      CL_RETVAL_OK,
                                                      false, 0, 0,
                                                      false, 0, 0};

static bool do_timeout_handling(unsigned long *time, int *counter)
{
   struct timeval  now;
   unsigned long   time_diff = 0;
   bool            ret = false;
   
   gettimeofday(&now, NULL);
   if ((now.tv_sec - *time) > (3 * CL_DEFINE_READ_TIMEOUT)) {
      *time = 0;
      *counter = 0;
   }

   if (*time < now.tv_sec)  {
      if (*time == 0) {
         time_diff = 1;
      } else {
         time_diff = now.tv_sec - *time;
      }
      *counter += time_diff;
      if (*counter > 2*CL_DEFINE_READ_TIMEOUT) {
         ret = true;
      }
      *time = now.tv_sec;
   }
   return ret;
}

static void ijs_general_communication_error(
               const cl_application_error_list_elem_t *commlib_error)
{
   DENTER(TOP_LAYER, "ijs_general_communication_error");

   if (commlib_error == NULL) {
      DEXIT;
      return;
   }

   sge_mutex_lock("ijs_general_communication_error_mutex",
                  SGE_FUNC, __LINE__, &ijs_general_communication_error_mutex);  

   /* save the communication error to react later */
   ijs_communication_error.com_error = commlib_error->cl_error;

   switch (commlib_error->cl_error) {
      case CL_RETVAL_OK:
         break;

      case CL_RETVAL_ACCESS_DENIED:
         if (ijs_communication_error.com_access_denied == false) {
            /* counts access denied errors (TODO: workaround for BT: 6350264, IZ: 1893) */
            /* increment counter only once per second and allow max CL_DEFINE_READ_TIMEOUT + 2 access denied */
            ijs_communication_error.com_access_denied =
               do_timeout_handling(&ijs_communication_error.com_access_denied_time,
                                   &ijs_communication_error.com_access_denied_counter);
         }
         break;

      case CL_RETVAL_ENDPOINT_NOT_UNIQUE: 
         if (ijs_communication_error.com_endpoint_not_unique == false) {
            /* counts endpoint not unique errors (TODO: workaround for BT: 6350264, IZ: 1893) */
            /* increment counter only once per second and allow max CL_DEFINE_READ_TIMEOUT + 2 endpoint not unique */
            DPRINTF(("got endpint not unique"));
            ijs_communication_error.com_endpoint_not_unique = 
               do_timeout_handling(&ijs_communication_error.com_endpoint_not_unique_time,
                                   &ijs_communication_error.com_endpoint_not_unique_counter);
         }
         break;

      default:
         ijs_communication_error.com_was_error = true;
         break;
   }

   /*
    * now log the error if not already reported the 
    * least CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT seconds
    */
   if (commlib_error->cl_already_logged == false && 
      ijs_communication_error.com_last_error != ijs_communication_error.com_error) {

      /*  never log the same messages again and again (commlib
       *  will erase cl_already_logged flag every CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT
       *  seconds (30 seconds), so we have to save the last one!
       */
      ijs_communication_error.com_last_error = ijs_communication_error.com_error;

      switch (commlib_error->cl_err_type) {
         case CL_LOG_ERROR:
            if (commlib_error->cl_info != NULL) {
               ERROR((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_SS,
                      cl_get_error_text(commlib_error->cl_error),
                      commlib_error->cl_info));
            } else {
               ERROR((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_S,
                      cl_get_error_text(commlib_error->cl_error)));
            }
            break;

         case CL_LOG_WARNING:
            if (commlib_error->cl_info != NULL) {
               WARNING((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_SS,
                        cl_get_error_text(commlib_error->cl_error),
                        commlib_error->cl_info));
            } else {
               WARNING((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_S,
                        cl_get_error_text(commlib_error->cl_error)));
            }
            break;

         case CL_LOG_INFO:
            if (commlib_error->cl_info != NULL) {
               INFO((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_SS,
                     cl_get_error_text(commlib_error->cl_error),
                     commlib_error->cl_info));
            } else {
               INFO((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_S,
                     cl_get_error_text(commlib_error->cl_error)));
            }
            break;

         case CL_LOG_DEBUG:
            if (commlib_error->cl_info != NULL) {
               DEBUG((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_SS,
                      cl_get_error_text(commlib_error->cl_error),
                      commlib_error->cl_info));
            } else {
               DEBUG((SGE_EVENT, MSG_GDI_GENERAL_COM_ERROR_S,
                      cl_get_error_text(commlib_error->cl_error)));
            }
            break;

         case CL_LOG_OFF:
            break;
      }
   }
   sge_mutex_unlock("ijs_general_communication_error_mutex", 
                    SGE_FUNC, __LINE__, &ijs_general_communication_error_mutex);  
   DRETURN_VOID;
}

int comm_get_application_error(dstring *err_msg)
{
   int ret = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_get_application_error");
   sge_mutex_lock("ijs_general_communication_error_mutex", 
                    SGE_FUNC, __LINE__, &ijs_general_communication_error_mutex);  

   if (ijs_communication_error.com_endpoint_not_unique == true) {
      sge_dstring_sprintf(err_msg, "%s", MSG_CL_RETVAL_ENDPOINT_NOT_UNIQUE);
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      ret = COMM_ENDPOINT_NOT_UNIQUE;
   }
   if (ijs_communication_error.com_access_denied == true) {
      sge_dstring_sprintf(err_msg, "%s", MSG_CL_RETVAL_ACCESS_DENIED);
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      ret = COMM_ACCESS_DENIED;
   }
   sge_mutex_unlock("ijs_general_communication_error_mutex", 
                    SGE_FUNC, __LINE__, &ijs_general_communication_error_mutex);  
   DRETURN(ret);
}

/* redirects the commlib logging to a file */
/* this is a modified copy of the cl_log_list_flush_list() */
int my_log_list_flush_list(cl_raw_list_t* list_p) {
   cl_log_list_elem_t *elem = NULL;
   FILE               *fp = NULL;
   struct timeval     now;
   int                ret_val;

   if (list_p == NULL) {
      return CL_RETVAL_LOG_NO_LOGLIST;
   }

   if ((ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }

   if ((fp = fopen("cl_log.txt", "a")) == NULL) {
      return CL_RETVAL_NOT_OPEN; 
   }

   while ((elem = cl_log_list_get_first_elem(list_p)) != NULL) {
      gettimeofday(&now,NULL);

      fprintf(fp, "%-76s|", elem->log_module_name);
      if (elem->log_parameter == NULL) {
         fprintf(fp, "%ld.%ld|%20s|%s|%s| %s\n",
                 (long)now.tv_sec,
                 (long)now.tv_usec,
                 elem->log_thread_name,
                 cl_thread_convert_state_id(elem->log_thread_state),
                 cl_log_list_convert_type_id(elem->log_type),
                 elem->log_message);
      } else {
         fprintf(fp, "%ld.%ld|%20s|%s|%s| %s %s\n",
                 (long)now.tv_sec,
                 (long)now.tv_usec,
                 elem->log_thread_name,
                 cl_thread_convert_state_id(elem->log_thread_state),
                 cl_log_list_convert_type_id(elem->log_type),
                 elem->log_message,
                 elem->log_parameter);
      }
      cl_log_list_del_log(list_p);
      fflush(fp);
   }

   fclose(fp);
   return cl_raw_list_unlock(list_p);
}

/****** sge_ijs_comm/comm_init_lib() *******************************************
*  NAME
*     comm_init_lib() -- Initializes the communication library
*
*  SYNOPSIS
*     int comm_init_lib(dstring *err_msg) 
*
*  FUNCTION
*     Initializes the communication library, call it before using any other
*     communication function.
*
*  INPUTS
*     dstring *err_msg - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK: 
*              Communication library was successfully initialized.
*
*           COMM_CANT_SETUP_COMMLIB: 
*              Error initializing the communication library, err_msg contains 
*              the error reason.
*
*  NOTES
*     MT-NOTE: comm_init_lib() is not MT safe 
*
*  SEE ALSO
*    communication/comm_cleanup_lib()
*******************************************************************************/
int comm_init_lib(dstring *err_msg)
{
   int ret, ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_init_lib");

   /*
    * To enable commlib logging to a file (see my_log_list_flush_list()
    * for the file path), exchange this line with the one below.
    * Caution: On some architectures, logging causes problems! 
    */
   /*ret = cl_com_setup_commlib(CL_RW_THREAD, CL_LOG_DEBUG, my_log_list_flush_list);*/
   ret = cl_com_setup_commlib(CL_RW_THREAD, CL_LOG_OFF, NULL); 
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_com_setup_commlib() failed: %s (%d)\n", sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_SETUP_COMMLIB;
   } else {
      const char *alias_path = NULL;

      /* set the alias file */
      alias_path = sge_get_alias_path();
      ret = cl_com_set_alias_file(alias_path);
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_com_set_alias_file() failed: %s (%d)\n", sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_SETUP_COMMLIB;
      }
      sge_free(&alias_path);

      if (ret_val == COMM_RETVAL_OK) {
         cl_host_resolve_method_t resolve_method = CL_SHORT;
         char *help = NULL;
         char *default_domain = NULL;

         /* setup the resolve method */
         if (atoi(get_conf_val("ignore_fqdn")) == 0) {
            resolve_method = CL_LONG;
         }
         if ((help = get_conf_val("default_domain")) != NULL) {
            if (SGE_STRCASECMP(help, NONE_STR) != 0) {
               default_domain = help;
            }
         }

         ret = cl_com_set_resolve_method(resolve_method, default_domain);
         if (ret != CL_RETVAL_OK) {
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
            DPRINTF(("cl_com_set_resolve_method() failed: %s (%d)\n", sge_dstring_get_string(err_msg), ret));
            ret_val = COMM_CANT_SETUP_COMMLIB;
         }
      }
   }

   DRETURN(ret_val);
}

/****** sge_ijs_comm/comm_cleanup_lib() ***************************************
*  NAME
*     comm_cleanup_lib() -- Clean up the communication library
*
*  SYNOPSIS
*     int comm_cleanup_lib(dstring *err_msg) 
*
*  FUNCTION
*     Cleans up the communication library. Call it when done using the library.
*
*  INPUTS
*     dstring *err_msg - Pointer to a dstring that receives a static error
*                        string. If no error happens it get's set to 
*                        "no error happened".
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Communication library was successfully cleaned up.
*
*           COMM_CANT_CLEANUP_COMMLIB:
*              Error cleaning up the communication library, err_msg contains
*              the error reason.
*
*  NOTES
*     MT-NOTE: comm_cleanup_lib() is not MT safe 
*
*  SEE ALSO
*    communication/comm_init_lib()
*******************************************************************************/
int comm_cleanup_lib(dstring *err_msg)
{
   int ret, ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_cleanup_lib");

   ret = cl_com_cleanup_commlib();
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_com_cleanup_commlib() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_CLEANUP_COMMLIB;
   }

   DEXIT;
   return ret_val;
}
 
/****** sge_ijs_comm/comm_open_connection() ***********************************
*  NAME
*     comm_open_connection() -- Connects to or starts a comm server
*
*  SYNOPSIS
*     int comm_open_connection(bool b_server, int port, 
*            const char *component_name, bool b_secure, const char *user_name, 
*            COMM_HANDLE **handle, dstring *err_msg) 
*
*  FUNCTION
*     Either start a comm server or connect to a running comm server.
*
*  INPUTS
*     bool       b_server         - If true, a comm server is started, if false
*                                   a connection to a server is established.
*     bool       b_secure         - If true: Use secured connections
*     const char *this_component  - A unique name for this end of the connection.
*     int        port             - In case of server: Port on which the server
*                                   should listen. If this is 0, a free port is
*                                   selected.
*                                   In case of client: Port on which the server
*                                   listens.
*     const char *other_component - The unique name of the other end of the
*                                   connection.
*     char       *remote_host     - Name of the host to connect to. Ignored if
*                                   b_server == true.
*     const char *user_name       - For secured connections: Name of the user
*                                   whose certificates are to be used.
*                                   Ignored for unsecured connections.
*     COMM_HANDLE **handle        - The address of a COMM_HANDLE pointer
*                                   which must be initialized to NULL.
*     dstring *err_msg            - Pointer to an empty dstring to receive
*                                   error messages.
*
*  OUTPUT
*    COMM_HANDLE **handle - The COMM_HANDLE of the connection.
*    dstring     *err_msg - In case of error: The error reason.
*
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Connection was successfully opened.
* 
*           COMM_INVALID_PARAMETER:
*              The *handle is not NULL.
*
*           COMM_CANT_SETUP_SSL:
*              err_msg contains the reason.
*
*           COMM_CANT_CREATE_HANDLE:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_open_connection() is not MT safe 
*
*  SEE ALSO
*     communication/comm_shutdown_connection()
*******************************************************************************/
int comm_open_connection(bool        b_server, 
                         bool        b_secure,
                         const char  *this_component,
                         int         port, 
                         const char  *other_component,
                         char        *remote_host,
                         const char  *user_name,
                         COMM_HANDLE **handle, 
                         dstring     *err_msg)
{
   int              ret;
   int              old_euid                = SGE_SUPERUSER_UID;
   int              ret_val                 = COMM_RETVAL_OK;
   int              commlib_error           = CL_RETVAL_OK;
   cl_framework_t   communication_framework = CL_CT_TCP;
   cl_tcp_connect_t connect_type            = CL_TCP_DEFAULT;
   cl_xml_connection_type_t connection_type = CL_CM_CT_MESSAGE;

   DENTER(TOP_LAYER, "open_connection");

   /* Check validity of parameters */
   if (*handle != NULL) {
      sge_dstring_sprintf(err_msg, "Invalid parameter: *handle is not NULL");
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      DEXIT;
      return COMM_INVALID_PARAMETER;
   }

   if (b_secure == true) {
#ifdef SECURE
      communication_framework = CL_CT_SSL;

      /*
       * Got to do this with euid = root
       */
      if (getuid() == SGE_SUPERUSER_UID) {
         old_euid = geteuid();
         sge_seteuid(SGE_SUPERUSER_UID);
      }
      ret = sge_ssl_setup_security_path(this_component, user_name);
      /*
       * Switch back to old euid before error handling to do tracing as
       * the SGE admin user.
       */
      if (old_euid != SGE_SUPERUSER_UID) {
         sge_seteuid(old_euid);
      }

      if (ret != 0) {
         DPRINTF(("sge_ssl_setup_security_path() failed!\n"));
         sge_dstring_sprintf(err_msg, "Setting up SSL failed!");
         ret_val = COMM_CANT_SETUP_SSL; 
      }
#else
      /* 
       * If secure communication was requested but we cannot provide it
       * because seclib support was not compiled in, we must not fall back to 
       * insecure mode, instead we must return with a fatal error.
       */
      sge_dstring_sprintf(err_msg, "No security support compiled into this binary!");
      DPRINTF(("%s\n", sge_dstring_get_string(err_msg)));
      return COMM_NO_SECURITY_COMPILED_IN;      
#endif
   } 

   if (ret_val == COMM_RETVAL_OK) {
      /*
       * Define a error handling function for the commlib here -
       * the default error handling function of the commlib prints
       * error messages to stderr!
       */
      ret = cl_com_set_error_func(ijs_general_communication_error);
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "can't set commlib error function: %s",
                             cl_get_error_text(ret));
         DPRINTF(("cl_com_set_error_func() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_SETUP_COMMLIB;
      } else {
         DPRINTF(("trying to create commlib handle\n"));
         if (b_server == false) {
            *handle = cl_com_create_handle(&commlib_error, 
                                          communication_framework, 
                                          connection_type, false, port, 
                                          connect_type, (char*)this_component,
                                          0, 1, 0);
         } else {
            *handle = cl_com_create_handle(&commlib_error, 
                                          communication_framework, 
                                          connection_type, true, port, 
                                          connect_type, (char*)this_component, 
                                          1, 1, 0);
         }

         if (*handle == NULL) {
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(commlib_error));
            DPRINTF(("cl_com_create_handle() failed: %s (%d)\n",
                     sge_dstring_get_string(err_msg), commlib_error));
            ret_val = COMM_CANT_CREATE_HANDLE;
         } else {
            /* Set connection timeout to 'infinite' */
            (*handle)->connection_timeout = 0x0fffffff;
            DPRINTF(("(*handle)->connect_port = %d\n", (*handle)->connect_port));
            DPRINTF(("(*handle)->service_port = %d\n", (*handle)->service_port));

            /* Set synchron receive timeout */
            cl_com_set_synchron_receive_timeout(*handle, 1);
         }
      }
   }

   /*
    * Need to do this as SUPERUSER, because in csp mode we need the permissions
    * to load the job users keys.
    */
   if (b_server == false) {
      if (getuid() == SGE_SUPERUSER_UID) {
         old_euid = geteuid();
         sge_seteuid(SGE_SUPERUSER_UID);
      }
      ret = cl_commlib_open_connection(*handle, remote_host, (char*)other_component, 1);
      if (old_euid != SGE_SUPERUSER_UID) {
         sge_seteuid(old_euid);
      }
      if (ret != CL_RETVAL_OK) {
         ret_val = COMM_CANT_OPEN_CONNECTION;
      }
   }

   if (ret_val == COMM_RETVAL_OK) {
      ret_val = comm_get_application_error(err_msg);
   }

   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_shutdown_connection() *******************************
*  NAME
*     comm_shutdown_connection() -- gracefully shuts down a connection
*
*  SYNOPSIS
*     int comm_shutdown_connection(COMM_HANDLE *handle,  
*                                  const char *component_name, dstring *err_msg) 
*
*  FUNCTION
*     All connections get closed and then the communication handle gets freed.
*
*  INPUTS
*     COMM_HANDLE *handle         - Handle of the connection to be shut down.
*     const char  *component_name - Name of the remote component of the
*                                   connection to be shut down.
*     char        *remote_host    - Name of the server we are connected to.
*                                   Ignored if we are a server.
*     dstring     *err_msg        - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Connection was successfully opened.
* 
*           COMM_CANT_CLOSE_CONNECTION:
*              err_msg contains the reason.
*              
*           COMM_CANT_SHUTDOOWN_HANDLE:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_shutdown_connection() is not MT safe 
*
*  SEE ALSO
*     communication/comm_open_connection()
*******************************************************************************/
int comm_shutdown_connection(COMM_HANDLE *handle, const char *component_name,
                             char *remote_host, dstring *err_msg)
{
   int ret;
   int ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_shutdown_connection");

   /*
    * From here on the user shouldn't get informed of any errors occuring
    * during the shutdown of the connection - just shut down.
    */
   ret = cl_com_set_error_func(NULL);
   ret = cl_commlib_close_connection(handle, remote_host, 
                                     (char*)component_name, 1, false);
   if (ret != CL_RETVAL_OK && ret != CL_RETVAL_UNKNOWN_ENDPOINT) {
      /* shutting down the endpoint returned commlib error */
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_close_connection() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_CLOSE_CONNECTION;
      cl_com_ignore_timeouts(true);
      cl_commlib_shutdown_handle(handle, false);
   } else {
      ret = cl_commlib_shutdown_handle(handle, false);
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_commlib_close_connection() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_SHUTDOWN_HANDLE;
      }
   }
   DEXIT;
   return ret_val;
}  

/****** sge_ijs_comm/comm_set_connection_param() ******************************
*  NAME
*     comm_set_connection_param() -- Set several connection parameters.
*
*  SYNOPSIS
*     int comm_set_connection_param(COMM_HANDLE *handle, int param, 
*                                   int value, dstring *err_msg) 
*
*  FUNCTION
*     Sets several connection parameter. Valid parameters are:
*        HEARD_FROM_TIMEOUT: The time until the communication library will
*                            treat a connection as lost.
*
*  INPUTS
*     COMM_HANDLE *handle  - Handle of the connection.
*     int         param    - ID of the param to set. Currently
*                            HEARD_FROM_TIMEOUT (in seconds) is supported.
*     int         value    - Value to set the param to. 
*     dstring     *err_msg - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Connection was successfully opened.
* 
*           COMM_CANT_SET_CONNECTION_PARAM:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_set_connection_param() is not MT safe 
*******************************************************************************/
int comm_set_connection_param(COMM_HANDLE *handle, int param, int value,
                              dstring *err_msg)
{
   int ret;
   int ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_set_connection_param");
   ret = cl_commlib_set_connection_param(handle, param, value);
   if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_commlib_set_connection_param() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_SET_CONNECTION_PARAM;
   }
   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_ignore_timeouts() ***********************************
*  NAME
*     comm_ignore_timeouts() -- Use timeouts or wait infinitely.
*
*  SYNOPSIS
*     int comm_ignore_timeouts(bool b_ignore) 
*
*  FUNCTION
*     Tells the communication library to either use timeouts or just wait
*     until all work is done.
*
*  INPUTS
*     bool b_ignore    - If true, the comm. library ignores timeouts, 
*                        if false, timeouts are enabled.
*     dstring *err_msg - Gets the error reason in case of error.
*
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Connection was successfully opened.
* 
*           COMM_CANT_SET_IGNORE_TIMEOUTS:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_ignore_timeouts() is not MT safe 
*******************************************************************************/
int comm_ignore_timeouts(bool b_ignore, dstring *err_msg)
{
   int ret     = CL_RETVAL_OK;
   int ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_ignore_timeouts");
   
   cl_com_ignore_timeouts(b_ignore==true ? true : false);
   if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_com_ignore_timeouts() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_SET_IGNORE_TIMEOUTS;
   }
   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_wait_for_connection() *******************************
*  NAME
*     comm_wait_for_connection() -- Waits until at least one client has connected
*
*  SYNOPSIS
*     int comm_wait_for_connection(COMM_HANDLE *handle, const char 
*         *component, int wait_secs, const char **host, dstring *err_msg) 
*
*  FUNCTION
*     On a server, waits until at least one client has connected.
*
*  INPUTS
*     COMM_HANDLE *handle    - Handle of the connection.
*     const char  *component - Wait for a client with this component name.
*     int         wait_secs  - Wait at most wait_secs seconds.
*     const char  **host     - Name of the host from where the client connects.
*     dstring     *err_msg   - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              A client is connected to us.
* 
*           COMM_GOT_TIMEOUT:
*              'wait_seconds' have elapsed.
*
*           COMM_CANT_TRIGGER:
*              err_msg contains the reason.
*
*           COMM_CANT_SEARCH_ENDPOINT:
*              err_msg contains the reason.
*
*           COMM_INVALID_PARAMETER:
*              handle = NULL, err_msg doesn't contain an error reason.
*
*  NOTES
*     MT-NOTE: comm_wait_for_connection() is not MT safe 
*
*  SEE ALSO
*     communication/comm_wait_for_no_connection()
*******************************************************************************/
int comm_wait_for_connection(COMM_HANDLE *handle, 
                             const char *component, 
                             int wait_secs, 
                             const char **host, 
                             dstring *err_msg)
{
   int                     waited_usec = 0;
   int                     ret = 0;
   int                     ret2 = 0;
   int                     ret_val = COMM_RETVAL_OK;
   cl_raw_list_t           *endpoint_list = NULL;
   cl_endpoint_list_elem_t *endpoint;

   DENTER(TOP_LAYER, "wait_for_connection");

   if (handle == NULL) {
      return COMM_INVALID_PARAMETER;
   }
  
   /*
    * In the while loop, do this:
    * Call cl_commlib_trigger(), ignore the return value (it won't return 99)
    * Get the list of endpoints of expected kind
    * If endpointlist is returned and contains 0 elements, sleep for
    * 10 milliseconds and loop again.
    */
   while ((ret2=cl_commlib_trigger(handle, 0)) != 99 
          && (ret = cl_commlib_search_endpoint(handle, NULL,
             (char*)component, 0, true, &endpoint_list)) == CL_RETVAL_OK
          && endpoint_list != NULL
          && endpoint_list->elem_count == 0
          && waited_usec/1000000 < wait_secs) {
      struct timespec req = {0, 10000000}, rem;

      cl_endpoint_list_cleanup(&endpoint_list);
      nanosleep(&req, &rem);
      waited_usec += 10000;
      if (received_signal == SIGINT) {
         break;
      }
   }
   if (waited_usec/1000000 >= wait_secs) {
      sge_dstring_sprintf(err_msg, "Timeout occured while waiting for connection");
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      ret_val = COMM_GOT_TIMEOUT;
   } else if (ret2 != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret2));
      DPRINTF(("cl_commlib_trigger() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret2));
      ret_val = COMM_CANT_TRIGGER;
   } else if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_search_endpoint() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_SEARCH_ENDPOINT;
   }
   if (endpoint_list != NULL) {
      /* A client connected to us, get it's hostname */
      if (endpoint_list->elem_count > 0) {
         endpoint = cl_endpoint_list_get_first_elem(endpoint_list);
         sge_free(host);
         *host = strdup(endpoint->endpoint->comp_host);
         DPRINTF(("A client from host %s has connected\n", *host));
      }
      cl_endpoint_list_cleanup(&endpoint_list);
   }
   if (ret_val == COMM_RETVAL_OK) {
      ret_val = comm_get_application_error(err_msg);
   }
   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_wait_for_no_connection() ****************************
*  NAME
*     comm_wait_for_no_connection() -- Wait until no client is connected any 
*                                      more
*
*  SYNOPSIS
*     int comm_wait_for_no_connection(COMM_HANDLE *handle, const char 
*     *component, int wait_secs, dstring *err_msg) 
*
*  FUNCTION
*     Waits until no client is connected to us any more.
*
*  INPUTS
*     COMM_HANDLE *handle   - Handle of the connection. 
*     const char *component - Filter for clients with this component name
*     int wait_secs         - Wait at most wait_secs seconds.
*     dstring *err_msg      - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              No client is connected to us.
* 
*           COMM_GOT_TIMEOUT:
*              'wait_seconds' have elapsed.
*
*           COMM_CANT_TRIGGER:
*              err_msg contains the reason.
*
*           COMM_CANT_SEARCH_ENDPOINT:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_wait_for_no_connection() is not MT safe 
*
*  SEE ALSO
*     communication/comm_wait_for_connection()
*******************************************************************************/
int comm_wait_for_no_connection(COMM_HANDLE *handle, const char *component, 
                                int wait_secs, dstring *err_msg)
{
   int                     waited_usec = 0;
   int                     ret = 0;
   int                     ret2 = 0;
   int                     ret_val = COMM_RETVAL_OK;
   cl_raw_list_t           *endpoint_list = NULL;
   bool                    do_exit = false;

   DENTER(TOP_LAYER, "comm_wait_for_no_connection");
  
   /*
    * In the while loop, do this:
    * Call cl_commlib_trigger(), ignore the return value (it won't return 99)
    * Get the list of endpoints of expected kind
    * If endpointlist is returned and contains >0 elements, sleep for
    * 10 milliseconds and loop again.
    */

   while (do_exit == false) {
      /* Let commlib update it's lists */
      ret2 = cl_commlib_trigger(handle, 0);
      /* Get list of all endpoints */
      ret  = cl_commlib_search_endpoint(handle, NULL, (char*)component, 0, true, 
                                        &endpoint_list);

      if (ret == CL_RETVAL_OK
          && endpoint_list != NULL
          && endpoint_list->elem_count > 0
          && waited_usec/1000000 < wait_secs) {
         struct timespec req = {0, 10000000}, rem;
         cl_endpoint_list_cleanup(&endpoint_list);
         endpoint_list = NULL;
         nanosleep(&req, &rem);
         waited_usec += 10000;
         if (received_signal == SIGINT) {
            do_exit = true;
            continue;
         }
      } else {
         DPRINTF(("No known endpoint left or timeout -> exit loop\n"));
         do_exit = true;
         continue;
      }
   } 
   
   DPRINTF(("wait_for_no_connection: after while\n"));
   if (waited_usec/1000000 >= wait_secs) {
      sge_dstring_sprintf(err_msg, 
                          "Timeout occured while waiting for no connection");
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      ret_val = COMM_GOT_TIMEOUT;
   }
   if (ret2 != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret2));
      DPRINTF(("cl_commlib_trigger() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret2));
      ret_val = COMM_CANT_TRIGGER;
   }
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_search_endpoint() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_SEARCH_ENDPOINT;
   }
   if (endpoint_list != NULL) {
      DPRINTF(("wait_for_no_connection: cleaning up endpoint list\n"));
      cl_endpoint_list_cleanup(&endpoint_list);
   }
   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_get_connection_count() ******************************
*  NAME
*     comm_get_connection_count() -- Retrieves the current number of connections
*
*  SYNOPSIS
*     int comm_get_connection_count(COMM_HANDLE *handle, dstring *err_msg) 
*
*  FUNCTION
*     Retrieves the current number of connections.
*
*  INPUTS
*     COMM_HANDLE *handle - Handle of the connection. 
*     dstring *err_msg    - Gets the error reason in case of error.
*
*  RESULT
*     int - Number of connections.
*           <0 in case of error:
*           -COMM_CANT_LOCK_CONNECTION_LIST:
*              err_msg contains the reason.
*
*           -COMM_CANT_UNLOCK_CONNECTION_LIST:
*              err_msg contains the reason.
*
*
*  NOTES
*     MT-NOTE: comm_get_connection_count() is not MT safe 
*******************************************************************************/
int comm_get_connection_count(const COMM_HANDLE *handle, dstring *err_msg)
{
   int                        ret;
   int                        ret_val = 1;
   cl_connection_list_elem_t* elem    = NULL;

   DENTER(TOP_LAYER, "comm_get_connection_count");

   ret = cl_raw_list_lock(handle->connection_list);
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_raw_list_lock() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = -COMM_CANT_LOCK_CONNECTION_LIST;
   } else {
      elem = cl_connection_list_get_first_elem(handle->connection_list);
      if (elem == NULL) {
         ret_val = 0;
      }
      ret = cl_raw_list_unlock(handle->connection_list);
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_raw_list_unlock() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = -COMM_CANT_UNLOCK_CONNECTION_LIST;
      }
   }

   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_trigger() *******************************************
*  NAME
*     comm_trigger() -- Trigger communication library
*
*  SYNOPSIS
*     int comm_trigger(COMM_HANDLE *handle, int synchron, dstring *err_msg) 
*
*  FUNCTION
*     Triggers the communication library  to do pending tasks.
*
*  INPUTS
*     COMM_HANDLE *handle  - Handle of the connection. 
*     int         synchron - Set to != 0 to wait until all pending
*                            messages are sent, == 0 to just do one
*                            piece of work and return then.
*     dstring     *err_msg - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              Trigger was successful.
* 
*           COMM_GOT_TIMEOUT:
*              'wait_seconds' have elapsed.
*
*           COMM_CANT_TRIGGER:
*              err_msg contains the reason.
*
*           COMM_CANT_SEARCH_ENDPOINT:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_trigger() is not MT safe 
*******************************************************************************/
int comm_trigger(COMM_HANDLE *handle, int synchron, dstring *err_msg)
{
   int ret;
   int ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_trigger");

   ret = cl_commlib_trigger(handle, synchron);
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_trigger() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_TRIGGER;
   }
   if (ret_val == COMM_RETVAL_OK) {
      ret_val = comm_get_application_error(err_msg);
   }
   DEXIT;
   return ret_val;
}
      
/****** sge_ijs_comm/comm_write_message() *************************************
*  NAME
*     comm_write_message() -- Write a message to the connection
*
*  SYNOPSIS
*     unsigned long comm_write_message(COMM_HANDLE *handle, const char 
*     *unresolved_hostname, const char *component_name, unsigned long component_id, 
*     unsigned char *buffer, unsigned long size, unsigned char type, dstring 
*     *err_msg) 
*
*  FUNCTION
*     Writes a message to the connection.
*
*  INPUTS
*     COMM_HANDLE *handle             - Handle of the connection.
*     const char *unresolved_hostname - Hostname of the destination host.
*     const char *component_name      - Component name of the destination.
*     unsigned long component_id      - Component ID of the destination.
*     unsigned char *buffer           - The message data.
*     unsigned long size              - Message data length.
*     unsigned char type              - Message type.
*     dstring *err_msg                - Gets the error reason in case of error.
*
*  RESULT
*     unsigned long - the number of bytes written.
*                     0 in case of error.
*
*  NOTES
*     MT-NOTE: comm_write_message() is not MT safe 
*
*  SEE ALSO
*     communication/comm_recv_message
*******************************************************************************/
unsigned long comm_write_message(COMM_HANDLE *handle,
                            const char *unresolved_hostname,
                            const char *component_name,
                            unsigned long component_id,
                            unsigned char *buffer, 
                            unsigned long size,
                            unsigned char type,
                            dstring *err_msg)
{
   int           ret;
   cl_byte_t     *sendbuf;
   unsigned long nwritten = 0;

   DENTER(TOP_LAYER, "comm_write_message");

   /* 
    * Copy only 'size' bytes from 'buffer' to a new sendbuf and add
    * one byte for the message type at the beginning of the sendbuf.
    * The commlib will free this buffer when it's content was sent.
    */
   sendbuf = sge_malloc(size+1);
   sendbuf[0] = type;
   memcpy(&sendbuf[1], buffer, size);

   ret = cl_commlib_send_message(handle, 
                           (char*)unresolved_hostname,
                           (char*)component_name,
                           component_id,
                           CL_MIH_MAT_NAK, 
                           &sendbuf,
                           size+1,
                           NULL,
                           0,
                           0,
                           false,  /* don't copy the sendbuf */
                           false); /* don't wait for ack */

   /* TODO: change send_message to ACK, i.e. CL_MIH_MAT_ACK && true in
    *       the last line. This ensures that the connection is open
    *       before the threads are created when this function is used by
    *       parent_loop(). Better solution: Move this to a function
    *       comm_write_first_message() or so.
    *
    *       Problem seems to be: Both threads try to open a connection
    *       to the qrsh client. This problem shoud be handled by the commlib.
    */


   /* sendbuf was freed by the commlib */
   sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));

   if (ret == CL_RETVAL_OK) {
      nwritten = size;
   } else {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_send_message() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
   }
   
   DEXIT;
   return nwritten;
}

/****** sge_ijs_comm/comm_flush_write_messages() ******************************
*  NAME
*     comm_flush_write_messages() -- Flush all messages still in the write list
*                                    of the communication library
*
*  SYNOPSIS
*     int comm_flush_write_messages(COMM_HANDLE *handle, dstring *err_msg)
*
*  FUNCTION
*     Flushes all messages still in the write list of the communication library.
*     comm_write_message() adds a message to the write list and tries to send
*     it immediately. This isn't always possible, so comm_flush_write_messages()
*     makes sure all messages are really written. 
*
*  INPUTS
*     COMM_HANDLE *handle - Handle of the connection.
*     dstring *err_msg    - Contains error message in case of error.
*
*  RESULT
*     int - 0: Ok, all messages were flushed.
*          <0: Retries needed to flush all messages * -1
*          >0: An error occured, error number is a commlib error.
*
*  NOTES
*     MT-NOTE: comm_flush_write_messages() is not MT safe 
*
*  SEE ALSO
*     communication/comm_write_message
*******************************************************************************/
int comm_flush_write_messages(COMM_HANDLE *handle, dstring *err_msg)
{
   unsigned long elems = 0;
   int           ret = 0, retries = 0;

   elems = cl_com_messages_in_send_queue(handle);
   while (elems > 0) {
      /*
       * Don't set the cl_commlib_trigger()-call to be blocking and
       * get rid of the usleep() - it's much slower!
       * The last cl_commlib_trigger()-call will take 1 s.
       */
      ret = cl_commlib_trigger(handle, 0);
      /* 
       * Bail out if trigger fails with an error that indicates that we
       * won't be able to send the messages in the near future.
       */
      if (ret != CL_RETVAL_OK && 
          ret != CL_RETVAL_SELECT_TIMEOUT &&
          ret != CL_RETVAL_SELECT_INTERRUPT) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         retries = ret;  
         break;   
      }
      elems = cl_com_messages_in_send_queue(handle);
      /* 
       * We just tried to send the messages and it wasn't possible to send
       * all messages - give the network some time to recover.
       */
      /* TODO (NEW): make this work correctly by calling check_client_alive */
      if (elems > 0) {
         sge_usleep(10000);
         retries--;
      }
   }
   return retries;
}

/****** sge_ijs_comm/comm_recv_message() **************************************
*  NAME
*     comm_recv_message() -- Receives a message from the connection
*
*  SYNOPSIS
*     int comm_recv_message(COMM_HANDLE *handle, bool b_synchron, 
*     recv_message_t *recv_mess, dstring *err_msg) 
*
*  FUNCTION
*     Receives a message from the connection.
*
*  INPUTS
*     COMM_HANDLE *handle          - Handle of the connection.
*     bool b_synchron              - true: Wait until a complete message was read
*                                    false: Get what's available and return.     
*     recv_message_t *recv_mess    - The message gets filled into this struct.
*                                    The caller has to free buffers.
*     dstring *err_msg             - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              A message was received.
* 
*           COMM_GOT_TIMEOUT:
*              'wait_seconds' have elapsed.
*
*           COMM_CANT_TRIGGER:
*              err_msg contains the reason.
*
*           COMM_CANT_SEARCH_ENDPOINT:
*              err_msg contains the reason.
*
*  NOTES
*     MT-NOTE: comm_recv_message() is not MT safe 
*
*  SEE ALSO
*     communication/comm_send_message, communication/comm_free_message
*******************************************************************************/
int comm_recv_message(COMM_HANDLE *handle, bool b_synchron, 
                      recv_message_t *recv_mess, dstring *err_msg)
{
   int  ret_val = COMM_RETVAL_OK;
   int  ret = 0;
   char sub_type[10];
   cl_com_message_t  *message = NULL;
   cl_com_endpoint_t *sender  = NULL;

   DENTER(TOP_LAYER, "recv_message");

   /* check validity of parameters */
   if (handle == NULL || recv_mess == NULL) {
      if (handle == NULL) {
         sge_dstring_sprintf(err_msg, "Invalid parameter: handle == NULL");
      } else {
         sge_dstring_sprintf(err_msg, "Invalid parameter: recv_mess == NULL");
      }
      DPRINTF(("%s", sge_dstring_get_string(err_msg)));
      DEXIT;
      return COMM_INVALID_PARAMETER;
   }

   ret = cl_commlib_receive_message(handle,
                                    NULL,       /* unresolved_hostname, */
                                    NULL,       /* component_name, */
                                    0,          /* component_id, */
                                    false,
                                    0,
                                    &message,
                                    &sender);
   if (ret != CL_RETVAL_OK) {
      switch (ret) {
         case CL_RETVAL_NO_SELECT_DESCRIPTORS:
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
            DPRINTF(("cl_commlib_receive_message() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
            ret_val = COMM_NO_SELECT_DESCRIPTORS;
         break;
         case CL_RETVAL_CONNECTION_NOT_FOUND:
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
            DPRINTF(("cl_commlib_receive_message() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
            ret_val = COMM_CONNECTION_NOT_FOUND;
         break;
         case CL_RETVAL_SYNC_RECEIVE_TIMEOUT:
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
            DPRINTF(("cl_commlib_receive_message() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
            ret_val = COMM_SYNC_RECEIVE_TIMEOUT;
         break;
         case CL_RETVAL_NO_MESSAGE:
            sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
            DPRINTF(("cl_commlib_receive_message() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
            ret_val = COMM_NO_MESSAGE_AVAILABLE;
         break;
         default:
            sge_dstring_sprintf(err_msg, "can't receive message");
            DPRINTF(("cl_commlib_receive_message() couldn't receive a message\n"));
            ret_val = COMM_CANT_RECEIVE_MESSAGE;
      }
   }

   if(sender != NULL) {
      cl_com_free_endpoint(&sender);
   }
   
   if (message != NULL) {
      recv_mess->cl_message = message;
      if (message->message_length>0) {
         char tmpbuf[100];
         switch (message->message[0]) {
            case STDIN_DATA_MSG:
            case STDOUT_DATA_MSG:
            case STDERR_DATA_MSG:
            case REGISTER_CTRL_MSG:
            case UNREGISTER_CTRL_MSG:
            case UNREGISTER_RESPONSE_CTRL_MSG:
            case SETTINGS_CTRL_MSG:
            case SUSPEND_CTRL_MSG:
            case UNSUSPEND_CTRL_MSG:
            case STDIN_CLOSE_MSG:
               DPRINTF(("length of message: %d\n", (int)message->message_length));
               /* data message */ 
               recv_mess->type = message->message[0];
               recv_mess->data = (char*)&(message->message[1]);

               DPRINTF(("recv_mess->type = %d\n", recv_mess->type));
               memcpy(tmpbuf, recv_mess->data, MIN(99, message->message_length - 1));
               tmpbuf[MIN(99, message->message_length - 1)] = 0;
               DPRINTF(("recv_mess->data = %s\n", tmpbuf));
               break;

            case WINDOW_SIZE_CTRL_MSG:
               memcpy(tmpbuf, message->message, MIN(99, message->message_length));
               tmpbuf[MIN(99, message->message_length)] = 0;
               /* control message */
               recv_mess->type = tmpbuf[0];
               /* scan subtype */
               sscanf((char*)&(tmpbuf[1]), "%9s", sub_type);
               if (strcmp(sub_type, "WS") == 0) {
                  int row, col, xpixel, ypixel;
                  sscanf((char*)&(tmpbuf[4]),
                     "%d%d%d%d",
                     &row, &col, &xpixel, &ypixel);
                  recv_mess->ws.ws_row    = row;
                  recv_mess->ws.ws_col    = col;
                  recv_mess->ws.ws_xpixel = xpixel;
                  recv_mess->ws.ws_ypixel = ypixel;
               }
               break;
         }
      }
   } else {
      cl_commlib_trigger(handle, b_synchron);
   }
   DEXIT;
   return ret_val;
}

/****** sge_ijs_comm/comm_free_message() **************************************
*  NAME
*     comm_free_message() -- free contents of a received message struct
*
*  SYNOPSIS
*     int comm_free_message(recv_message_t *recv_mess, dstring *err_msg) 
*
*  FUNCTION
*     Frees the content of a received message struct.
*
*  INPUTS
*     recv_message_t *recv_mess - The message struct that is to be freed.
*     dstring *err_msg          - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              The message is freed.
* 
*           COMM_CANT_FREE_MESSAGE:
*              err_msg contains the error reason.
*
*  NOTES
*     MT-NOTE: comm_free_message() is not MT safe 
*
*  SEE ALSO
*     communication/comm_recv_message()
*******************************************************************************/
int comm_free_message(recv_message_t *recv_mess, dstring *err_msg)
{
   int ret;
   int ret_val = COMM_RETVAL_OK;

   DENTER(TOP_LAYER, "comm_free_message");

   if (recv_mess != NULL && recv_mess->cl_message != NULL) {
      ret = cl_com_free_message(&(recv_mess->cl_message));
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
         DPRINTF(("cl_com_free_message() failed: %s (%d)\n",
                  sge_dstring_get_string(err_msg), ret));
         ret_val = COMM_CANT_FREE_MESSAGE;
      }
   }
   DEXIT;
   return ret_val;
}


/****** sge_ijs_comm/check_client_alive() *************************************
*  NAME
*     check_client_alive() -- Checks is a know, connected client is still alive
*
*  SYNOPSIS
*     int check_client_alive(COMM_HANDLE *handle, 
*                            const char *component_name, dstring *err_msg) 
*
*  FUNCTION
*     Checks if a known, connected client is still alive.
*
*  INPUTS
*     COMM_HANDLE *handle          - Handle to the connection.
*     const char  *component_name  - Name of the comonent to check.
*     char        *hostname        - Host of the client.
*     dstring     *err_msg         - Gets the error reason in case of error.
*
*  RESULT
*     int - COMM_RETVAL_OK:
*              The client is alive.
* 
*           COMM_CANT_GET_CLIENT_STATUS:
*              err_msg contains the error reason.
*
*  NOTES
*     MT-NOTE: check_client_alive() is not MT safe 
*
*******************************************************************************/
int check_client_alive(COMM_HANDLE *handle,
                       const char *component_name,
                       char *hostname,
                       dstring *err_msg)
{
   int           ret;
   int           ret_val = COMM_RETVAL_OK;
   cl_com_SIRM_t *status = NULL;

   DENTER(TOP_LAYER, "check_client_alive");

   DPRINTF(("handle->connect_port = %d\n", handle->connect_port));
   DPRINTF(("handle->service_port = %d\n", handle->service_port));
   DPRINTF(("client component name = %s\n", component_name));
   DPRINTF(("hostname = %s\n", hostname));

   ret = cl_commlib_get_endpoint_status(handle, hostname, 
                                        (char*)component_name, 1, &status);
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(err_msg, "%s", cl_get_error_text(ret));
      DPRINTF(("cl_commlib_get_endpoint() failed: %s (%d)\n",
               sge_dstring_get_string(err_msg), ret));
      ret_val = COMM_CANT_GET_CLIENT_STATUS;
   }

   cl_com_free_sirm_message(&status);
   DEXIT;
   return ret_val;
}


