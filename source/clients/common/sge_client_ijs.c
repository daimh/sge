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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#if defined(DARWIN)
#  include <termios.h>
#  include <sys/ttycom.h>
#  include <sys/ioctl.h>
#elif defined(HP11) || defined(HP1164)
#  include <termios.h>
#elif defined(INTERIX) || defined(__CYGWIN__)
#  include <termios.h>
#  include <sys/ioctl.h>
#elif defined(FREEBSD) || defined(NETBSD)
#  include <termios.h>
#  include <sys/ioctl.h>
#else
#  include <termio.h>
#endif

#include <sys/timeb.h>

#include "uti/sge_rmon.h"
#include "uti/sge_io.h"
#include "uti/sge_pty.h"

#include "sgeobj/sge_utility.h"

#include "sge_ijs_comm.h"
#include "sge_ijs_threads.h"
#include "sge_client_ijs.h"
#include "ijs/sge_ijs_lib.h"

/* module variables */
static char *g_hostname  = NULL;
static int  g_exit_status = 0; /* set by worker thread, read by main thread */
static int  g_nostdin     = 0; /* set by main thread, read by worker thread */
static int  g_noshell     = 0; /* set by main thread, read by worker thread */
static int  g_is_rsh      = 0; /* set by main thread, read by worker thread */
static unsigned int g_pid = 0; /* set by main thread, read by worker thread */
static int  g_raw_mode_state = 0; /* set by main thread, read by worker thread */
static int  g_suspend_remote = 0; /* set by main thread, read by worker thread */
static COMM_HANDLE *g_comm_handle = NULL;

/*
 * static volatile sig_atomic_t received_window_change_signal = 1;
 * Flag to indicate that we have received a window change signal which has
 * not yet been processed.  This will cause a message indicating the new
 * window size to be sent to the server a little later.  This is volatile
 * because this is updated in a signal handler.
 */
static volatile sig_atomic_t received_window_change_signal = 1;
static volatile sig_atomic_t received_broken_pipe_signal = 0;
static volatile sig_atomic_t quit_pending; /* Set non-zero to quit the loop. */

volatile sig_atomic_t received_signal = 0;

/****** window_change_handler **************************************************
*  NAME
*     window_change_handler() -- handler for the window changed signal
*
*  SYNOPSIS
*     static void window_change_handler(int sig)
*
*  FUNCTION
*     Signal handler for the window change signal (SIGWINCH).  This just sets a
*     flag indicating that the window has changed.
*
*  INPUTS
*     int sig - number of the received signal
*
*  RESULT
*     void - no result
*
*  NOTES
*    MT-NOTE: window_change_handler() is not MT safe, because it uses
*             received_window_change_signal
*
*  SEE ALSO
*******************************************************************************/
static void window_change_handler(int sig)
{
   /* Do not use DPRINTF in a signal handler! */
   received_window_change_signal = 1;
   signal(SIGWINCH, window_change_handler);
}

/****** broken_pipe_handler ****************************************************
*  NAME
*     broken_pipe_handler() -- handler for the broken pipe signal
*
*  SYNOPSIS
*     static void broken_pipe_handler(int sig)
*
*  FUNCTION
*     Handler for the SIGPIPE signal.
*
*  INPUTS
*     int sig - number of the received signal
*
*  RESULT
*     void - no result
*
*  NOTES
*
*  SEE ALSO
*******************************************************************************/
static void broken_pipe_handler(int sig)
{
   received_broken_pipe_signal = 1;
   signal(SIGPIPE, broken_pipe_handler);
}

/****** signal_handler() *******************************************************
*  NAME
*     signal_handler() -- handler for quit signals
*
*  SYNOPSIS
*     static void signal_handler(int sig)
*
*  FUNCTION
*     Handler for all signals that quit the program. These signals are trapped 
*     in order to restore the terminal modes.
*
*  INPUTS
*     int sig - number of the received signal
*
*  RESULT
*     void - no result
*
*  NOTES
*
*  SEE ALSO
*******************************************************************************/
void signal_handler(int sig)
{
   received_signal = sig;
   quit_pending = 1;
}

/****** set_signal_handlers() **************************************************
*  NAME
*     set_signal_handlers() -- set all signal handlers
*
*  SYNOPSIS
*     static void set_signal_handlers(void)
*
*  FUNCTION
*     Sets all signal handlers. Doesn't overwrite SIG_IGN and therefore
*     matches the behaviour of rsh.
*
*  RESULT
*     void - no result
*
*  NOTES
*
*  SEE ALSO
*******************************************************************************/
void set_signal_handlers(void)
{
  struct sigaction old_handler, new_handler;

   /* Is SIGHUP necessary? 
    * Yes: termio(7I) says:
    * "When a modem disconnect is detected, a SIGHUP signal is sent
    *  to the terminal's controlling process.
    *  Unless other arrangements have  been  made,  these  signals
    *  cause  the  process  to  terminate. If  SIGHUP is ignored or
    *  caught, any subsequent  read  returns  with  an  end-of-file
    *  indication until the terminal is closed."
    */
   sigaction(SIGHUP, NULL, &old_handler);
   if (old_handler.sa_handler != SIG_IGN) {
      new_handler.sa_handler = signal_handler;
      sigaddset(&new_handler.sa_mask, SIGHUP);
      new_handler.sa_flags = SA_RESTART;
      sigaction(SIGHUP, &new_handler, NULL);
   }

   sigaction(SIGINT, NULL, &old_handler);
   if (old_handler.sa_handler != SIG_IGN) {
      new_handler.sa_handler = signal_handler;
      sigaddset(&new_handler.sa_mask, SIGINT);
      new_handler.sa_flags = SA_RESTART;
      sigaction(SIGINT, &new_handler, NULL);
   }

   sigaction(SIGCONT, NULL, &old_handler);
   if (old_handler.sa_handler != SIG_IGN) {
      new_handler.sa_handler = signal_handler;
      sigaddset(&new_handler.sa_mask, SIGCONT);
      new_handler.sa_flags = SA_RESTART;
      sigaction(SIGCONT, &new_handler, NULL);
   }

   sigaction(SIGQUIT, NULL, &old_handler);
   if (old_handler.sa_handler != SIG_IGN) {
      new_handler.sa_handler = signal_handler;
      sigaddset(&new_handler.sa_mask, SIGQUIT);
      new_handler.sa_flags = SA_RESTART;
      sigaction(SIGQUIT, &new_handler, NULL);
   }

   sigaction(SIGTERM, NULL, &old_handler);
   if (old_handler.sa_handler != SIG_IGN) {
      new_handler.sa_handler = signal_handler;
      sigaddset(&new_handler.sa_mask, SIGTERM);
      new_handler.sa_flags = SA_RESTART;
      sigaction(SIGTERM, &new_handler, NULL);
   }

   new_handler.sa_handler = window_change_handler;
   sigaddset(&new_handler.sa_mask, SIGWINCH);
   new_handler.sa_flags = SA_RESTART;
   sigaction(SIGWINCH, &new_handler, NULL);

   new_handler.sa_handler = broken_pipe_handler;
   sigaddset(&new_handler.sa_mask, SIGPIPE);
   new_handler.sa_flags = SA_RESTART;
   sigaction(SIGPIPE, &new_handler, NULL);
}

/****** client_check_window_change() *******************************************
*  NAME
*     client_check_window_change() -- check if window size was change and
*                                     submit changes to pty
*
*  SYNOPSIS
*     static void client_check_window_change(COMM_HANDLE *handle)
*
*  FUNCTION
*     Checks if the window size of the terminal window was changed.
*     If the size was changed, submits the new window size to the
*     pty.
*     The actual change is detected by a signal (on Unix), this function
*     just checks the according flag.
*
*  INPUTS
*     COMM_HANDLE *handle - pointer to the commlib handle
*
*  RESULT
*     void - no result
*
*  NOTES
*     MT-NOTE: client_check_window_change() is MT-safe (see comment in code)
*
*  SEE ALSO
*     window_change_handler()
*******************************************************************************/
static void client_check_window_change(COMM_HANDLE *handle)
{
   struct winsize ws;
   char           buf[200];
   dstring        err_msg = DSTRING_INIT;

   DENTER(TOP_LAYER, "client_check_window_change");

   if (received_window_change_signal) {
      /*
       * here we can have a race condition between the two working threads,
       * but it doesn't matter - in the worst case, the new window size gets 
       * submitted two times.
       */
      received_window_change_signal = 0;
      if (ioctl(fileno(stdin), TIOCGWINSZ, &ws) >= 0) {
         DPRINTF(("sendig WINDOW_SIZE_CTRL_MSG with new window size: "
                  "%d, %d, %d, %d to shepherd\n",
                  ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel));

         sprintf(buf, "WS %d %d %d %d",
                 ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel);
         comm_write_message(handle, g_hostname, COMM_CLIENT, 1,
                            (unsigned char*)buf, strlen(buf), 
                            WINDOW_SIZE_CTRL_MSG, &err_msg);
      } else {
         DPRINTF(("client_check_windows_change: ioctl() failed! "
            "sending dummy WINDOW_SIZE_CTRL_MSG to fullfill protocol.\n"));
         sprintf(buf, "WS 60 80 480 640");
         comm_write_message(handle, g_hostname, COMM_CLIENT, 1,
            (unsigned char*)buf, strlen(buf), WINDOW_SIZE_CTRL_MSG, &err_msg);
      }
   }
   sge_dstring_free(&err_msg);
   DEXIT;
}

/****** tty_to_commlib() *******************************************************
*  NAME
*     tty_to_commlib() -- tty_to_commlib thread entry point and main loop
*
*  SYNOPSIS
*     void* tty_to_commlib(void *t_conf)
*
*  FUNCTION
*     Entry point and main loop of the tty_to_commlib thread.
*     Reads data from the tty and writes it to the commlib.
*
*  INPUTS
*     void *t_conf - pointer to cl_thread_settings_t struct of the thread
*
*  RESULT
*     void* - always NULL
*
*  NOTES
*     MT-NOTE: tty_to_commlib is MT-safe ?
*
*  SEE ALSO
*******************************************************************************/
void* tty_to_commlib(void *t_conf)
{
   char                 *pbuf;
   fd_set               read_fds;
   struct timeval       timeout;
   dstring              err_msg = DSTRING_INIT;
   dstring              dbuf = DSTRING_INIT;
   int                  do_exit = 0;
   int                  ret, nread = 0;

   DENTER(TOP_LAYER, "tty_to_commlib");
   thread_func_startup(t_conf);
   
   /* 
    * allocate working buffer
    */
   pbuf = (char*)malloc(BUFSIZE);
   if (pbuf == NULL) {
      DPRINTF(("tty_to_commlib can't allocate working buffer: %s (%d)\n",
         strerror(errno), errno));
      do_exit = 1;
   }

   while (do_exit == 0) {
      FD_ZERO(&read_fds);
      if (g_nostdin == 0) {
         /* wait for input on tty */
         FD_SET(STDIN_FILENO, &read_fds);
      } 
      timeout.tv_sec  = 1;
      timeout.tv_usec = 0;

      if (received_signal == SIGCONT) {
				received_signal = 0;
        if (continue_handler (g_comm_handle, g_hostname) == 1) {
          do_exit = 1;
          continue;
        }
        if (g_raw_mode_state == 1) {
          /* restore raw-mode after SIGCONT */
          if (terminal_enter_raw_mode () != 0) {
						 DPRINTF(("tty_to_commlib: couldn't enter raw mode for pty\n"));
             do_exit = 1;
             continue;
            }
        }
			}
      
      DPRINTF(("tty_to_commlib: Waiting in select() for data\n"));
      ret = select(STDIN_FILENO+1, &read_fds, NULL, NULL, &timeout);

      thread_testcancel(t_conf);
      client_check_window_change(g_comm_handle);

      if (received_signal == SIGHUP ||
          received_signal == SIGINT ||
          received_signal == SIGQUIT ||
          received_signal == SIGTERM) {
         /* If we receive one of these signals, we must terminate */
         do_exit = 1;
         continue;
      }

      if (ret > 0) {
         if (g_nostdin == 1) {
            /* We should never get here if STDIN is closed */
            DPRINTF(("tty_to_commlib: STDIN ready to read while it should be closed!!!\n"));
         }
         DPRINTF(("tty_to_commlib: trying to read() from stdin\n"));
         nread = read(STDIN_FILENO, pbuf, BUFSIZE-1);
         pbuf[nread] = '\0';
         sge_dstring_append (&dbuf, pbuf);
         DPRINTF(("tty_to_commlib: nread = %d\n", nread));

         if (nread < 0 && (errno == EINTR || errno == EAGAIN)) {
            DPRINTF(("tty_to_commlib: EINTR or EAGAIN\n"));
            /* do nothing */
         } else if (nread <= 0) {
            do_exit = 1;
         } else {
            DPRINTF(("tty_to_commlib: writing to commlib: %d bytes\n", nread));
            if (suspend_handler(g_comm_handle, g_hostname, g_is_rsh, g_suspend_remote, g_pid, &dbuf) == 1) {
                if (comm_write_message(g_comm_handle, g_hostname, 
                    COMM_CLIENT, 1, (unsigned char*)pbuf, 
                    (unsigned long)nread, STDIN_DATA_MSG, &err_msg) != nread) {
                  DPRINTF(("tty_to_commlib: couldn't write all data\n"));
                } else {
                  DPRINTF(("tty_to_commlib: data successfully written\n"));
                }
            }
            comm_flush_write_messages(g_comm_handle, &err_msg);
         }
      } else {
         /*
          * We got either a select timeout or a select error. In both cases,
          * it's a good chance to check if our client is still alive.
          */
         DPRINTF(("tty_to_commlib: Checking if client is still alive\n"));
         if (comm_get_connection_count(g_comm_handle, &err_msg) == 0) {
            DPRINTF(("tty_to_commlib: Client is not alive! -> exiting.\n"));
            do_exit = 1;
         } else {
            DPRINTF(("tty_to_commlib: Client is still alive\n"));
         }
      }
   } /* while (do_exit == 0) */

   /* Send STDIN_CLOSE_MSG to the shepherd. That causes the shepherd to close its filedescriptor, also. */
   if (comm_write_message(g_comm_handle, g_hostname, COMM_CLIENT, 1, (unsigned char*)" ",
                      1, STDIN_CLOSE_MSG, &err_msg) != 1) {
      DPRINTF(("tty_to_commlib: couldn't write STDIN_CLOSE_MSG\n"));
   } else {
      DPRINTF(("tty_to_commlib: STDIN_CLOSE_MSG successfully written\n"));
   }
   /* clean up */
   sge_dstring_free(&dbuf);
   sge_free(&pbuf);
   thread_func_cleanup(t_conf);
   
   sge_dstring_free(&err_msg);
   DPRINTF(("tty_to_commlib: exiting tty_to_commlib thread!\n"));
   DEXIT;
   return NULL;
}

/****** commlib_to_tty() *******************************************************
*  NAME
*     commlib_to_tty() -- commlib_to_tty thread entry point and main loop
*
*  SYNOPSIS
*     void* commlib_to_tty(void *t_conf)
*
*  FUNCTION
*     Entry point and main loop of the commlib_to_tty thread.
*     Reads data from the commlib and writes it to the tty.
*
*  INPUTS
*     void *t_conf - pointer to cl_thread_settings_t struct of the thread
*
*  RESULT
*     void* - always NULL
*
*  NOTES
*     MT-NOTE: commlib_to_tty is MT-safe ?
*
*  SEE ALSO
*******************************************************************************/
void* commlib_to_tty(void *t_conf)
{
   recv_message_t       recv_mess;
   dstring              err_msg = DSTRING_INIT;
   int                  ret = 0, do_exit = 0;

   DENTER(TOP_LAYER, "commlib_to_tty");
   thread_func_startup(t_conf);

   while (do_exit == 0) {
      /*
       * wait blocking for a message from commlib
       */
      recv_mess.cl_message = NULL;
      recv_mess.data = NULL;

      DPRINTF(("commlib_to_tty: recv_message()\n"));
      ret = comm_recv_message(g_comm_handle, true, &recv_mess, &err_msg);
      if (ret != COMM_RETVAL_OK) {
         /* check if we are still connected to anybody. */
         /* if not - exit. */
         DPRINTF(("commlib_to_tty: error receiving message: %s\n", 
                  sge_dstring_get_string(&err_msg)));
         if (comm_get_connection_count(g_comm_handle, &err_msg) == 0) {
            DPRINTF(("commlib_to_tty: no endpoint found\n"));
            do_exit = 1;
            continue;
         }
      }
      DPRINTF(("commlib_to_tty: received a message\n"));

      thread_testcancel(t_conf);
      client_check_window_change(g_comm_handle);

      if (received_signal == SIGHUP ||
          received_signal == SIGINT ||
          received_signal == SIGQUIT ||
          received_signal == SIGTERM) {
         /* If we receive one of these signals, we must terminate */
         DPRINTF(("commlib_to_tty: shutting down because of signal %d\n",
                 received_signal));
         do_exit = 1;
         continue;
      }

      DPRINTF(("'parsing' message\n"));
      /*
       * 'parse' message
       * A 1 byte prefix tells us what kind of message it is.
       * See sge_ijs_comm.h for message types.
       */
      if (recv_mess.cl_message != NULL) {
         char buf[100];
         switch (recv_mess.type) {
            case STDOUT_DATA_MSG:
               /* copy recv_mess.data to buf to append '\0' */
               memcpy(buf, recv_mess.data, MIN(99, recv_mess.cl_message->message_length - 1));
               buf[MIN(99, recv_mess.cl_message->message_length - 1)] = 0;
               DPRINTF(("commlib_to_tty: received stdout message, writing to tty.\n"));
               DPRINTF(("commlib_to_tty: message is: %s\n", buf));
/* TODO: If it's not possible to write all data to the tty, retry blocking
 *       until all data was written. The commlib must block then, too.
 */
               if (sge_writenbytes(STDOUT_FILENO, recv_mess.data,
                          (int)(recv_mess.cl_message->message_length-1))
                       != (int)(recv_mess.cl_message->message_length-1)) {
                  DPRINTF(("commlib_to_tty: sge_writenbytes() error\n"));
               }
               break;
            case STDERR_DATA_MSG:
               DPRINTF(("commlib_to_tty: received stderr message, writing to tty.\n"));
/* TODO: If it's not possible to write all data to the tty, retry blocking
 *       until all data was written. The commlib must block then, too.
 */
               if (sge_writenbytes(STDERR_FILENO, recv_mess.data,
                          (int)(recv_mess.cl_message->message_length-1))
                       != (int)(recv_mess.cl_message->message_length-1)) {
                  DPRINTF(("commlib_to_tty: sge_writenbytes() error\n"));
               }
               break;
            case WINDOW_SIZE_CTRL_MSG:
               /* control message */
               /* we don't expect a control message */
               DPRINTF(("commlib_to_tty: received window size message! "
                        "This was unexpected!\n"));
               break;
            case REGISTER_CTRL_MSG:
               /* control message */
               /* a client registered with us. With the next loop, the 
                * cl_commlib_trigger function will send the WINDOW_SIZE_CTRL_MSG
                * (and perhaps some data messages),  which is already in the 
                * send_messages list of the connection, to the client.
                */
               DPRINTF(("commlib_to_tty: received register message!\n"));
               /* Send the settings in response */
               sprintf(buf, "noshell = %d", g_noshell);
               ret = (int)comm_write_message(g_comm_handle, g_hostname, COMM_CLIENT, 1,
                  (unsigned char*)buf, strlen(buf)+1, SETTINGS_CTRL_MSG, &err_msg);
               DPRINTF(("commlib_to_tty: sent SETTINGS_CTRL_MSG, ret = %d\n", ret));
               break;
            case UNREGISTER_CTRL_MSG:
               /* control message */
               /* the client wants to quit, as this is the last message the client
                * sends, we can be sure to have received all messages from the 
                * client. We answer with a UNREGISTER_RESPONSE_CTRL_MSG so
                * the client knows that it can quit now. We can quit, also.
                */
               DPRINTF(("commlib_to_tty: received unregister message!\n"));
               DPRINTF(("commlib_to_tty: writing UNREGISTER_RESPONSE_CTRL_MSG\n"));

               /* copy recv_mess.data to buf to append '\0' */
               memcpy(buf, recv_mess.data, MIN(99, recv_mess.cl_message->message_length - 1));
               buf[MIN(99, recv_mess.cl_message->message_length - 1)] = 0;

               /* the UNREGISTER_CTRL_MSG contains the exit status of the
                * qrsh_starter in case of qrsh <command> and the exit status
                * of the shell for qlogin/qrsh <no command>.
                * If the job was signalled, the exit code is 128+signal.
                */
               sscanf(buf, "%d", &g_exit_status);
               comm_write_message(g_comm_handle, g_hostname, COMM_CLIENT, 1, 
                  (unsigned char*)" ", 1, UNREGISTER_RESPONSE_CTRL_MSG, &err_msg);

               DPRINTF(("commlib_to_tty: received exit_status from shepherd: %d\n", 
                        g_exit_status));
               comm_flush_write_messages(g_comm_handle, &err_msg);
               do_exit = 1;
#if 0
               cl_log_list_set_log_level(cl_com_get_log_list(), CL_LOG_OFF);
               cl_com_set_error_func(NULL);
#endif
               break;
         }
      }
      comm_free_message(&recv_mess, &err_msg);
   }

   thread_func_cleanup(t_conf);
   DPRINTF(("commlib_to_tty: exiting commlib_to_tty thread!\n"));
   sge_dstring_free(&err_msg);
   DEXIT;
   return NULL;
}

/****** run_ijs_server() *******************************************************
*  NAME
*     run_ijs_server() -- The servers main loop
*
*  SYNOPSIS
*     int run_ijs_server(u_long32 job_id, int nostdin, int noshell,
*                        int is_rsh, int is_qlogin, int force_pty,
*                        int *p_exit_status)
*
*  FUNCTION
*     The main loop of the commlib server, handling the data transfer from
*     and to the client.
*
*  INPUTS
*     COMM_HANDLE *handle - Handle of the COMM server
*     u_long32 job_id    - SGE job id of this job
*     int nostdin        - The "-nostdin" switch
*     int noshell        - The "-noshell" switch
*     int is_rsh         - Is it a qrsh with commandline?
*     int is_qlogin      - Is it a qlogin or qrsh without commandline?
*     int suspend_remote - suspend_remote switch of qrsh
*     int force_pty      - The user forced use of pty by the "-pty yes" switch
*
*  OUTPUTS
*     int *p_exit_status - The exit status of qrsh_starter in case of
*                          "qrsh <command>" or the exit status of the shell in
*                          case of "qrsh <no command>"/"qlogin".
*                          If the job was signalled, the exit code is 128+signal.
*
*  RESULT
*     int - 0: Ok.
*           1: Invalid parameter
*           2: Log list not initialized
*           3: Error setting terminal mode
*           4: Can't create tty_to_commlib thread
*           5: Can't create commlib_to_tty thread
*           6: Error shutting down commlib connection
*           7: Error resetting terminal mode
*
*  NOTES
*     MT-NOTE: run_ijs_server is not MT-safe
*******************************************************************************/
int run_ijs_server(COMM_HANDLE *handle, const char *remote_host,
                   u_long32 job_id,
                   int nostdin, int noshell,
                   int is_rsh, int is_qlogin, ternary_t force_pty,
                   ternary_t suspend_remote,
                   int *p_exit_status, dstring *p_err_msg)
{
   int               ret = 0, ret_val = 0;
   THREAD_HANDLE     *pthread_tty_to_commlib = NULL;
   THREAD_HANDLE     *pthread_commlib_to_tty = NULL;
   THREAD_LIB_HANDLE *thread_lib_handle = NULL;
   cl_raw_list_t     *cl_com_log_list = NULL;

   DENTER(TOP_LAYER, "run_ijs_server");

   if (handle == NULL || p_err_msg == NULL || p_exit_status == NULL || remote_host == NULL) {
      return 1;
   }
   g_comm_handle = handle;
   g_hostname    = strdup(remote_host);

   cl_com_log_list = cl_com_get_log_list();
   if (cl_com_log_list == NULL) {
      return 2;
   }

   g_nostdin = nostdin;
   g_noshell = noshell;
   g_pid = getpid();
   g_is_rsh = is_rsh;

   if (suspend_remote == UNSET || suspend_remote == NO) {
      g_suspend_remote = 0;
   } else {
      g_suspend_remote = 1;
   }

   /*
    * qrsh without command and qlogin both have is_rsh == 0 and is_qlogin == 1
    * qrsh with command and qsh don't need to set terminal mode.
    * If the user requested a pty we also have to set terminal mode.
    * But only if stdout is still connected to a tty and not redirected
    * to a file or a pipe.
    */
   if (isatty(STDOUT_FILENO) == 1 &&
      ((force_pty == UNSET && is_rsh == 0 && is_qlogin == 1) || force_pty == YES)) {
      /*
       * Set this terminal to raw mode, just output everything, don't interpret
       * it. Let the pty on the client side interpret the characters.
       */
      ret = terminal_enter_raw_mode();
      if (ret != 0) {
         sge_dstring_sprintf(p_err_msg, "can't set terminal to raw mode: %s (%d)",
            strerror(ret), ret);
         return 3;
      } else {
        g_raw_mode_state = 1;
      }
   }

   /*
    * Setup thread list and create two worker threads
    */
   thread_init_lib(&thread_lib_handle);
   /*
    * From here on, we have to cleanup the list in case of errors, this is
    * why we "goto cleanup" in case of error.
    */

   DPRINTF(("creating worker threads\n"));
   DPRINTF(("creating tty_to_commlib thread\n"));
   ret = create_thread(thread_lib_handle, &pthread_tty_to_commlib, cl_com_log_list,
      "tty_to_commlib thread", 1, tty_to_commlib);
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(p_err_msg, "can't create tty_to_commlib thread: %s",
         cl_get_error_text(ret));
      ret_val = 4;
      goto cleanup;
   }

   DPRINTF(("creating commlib_to_tty thread\n"));
   ret = create_thread(thread_lib_handle, &pthread_commlib_to_tty, cl_com_log_list,
      "commlib_to_tty thread", 1, commlib_to_tty);
   if (ret != CL_RETVAL_OK) {
      sge_dstring_sprintf(p_err_msg, "can't create commlib_to_tty thread: %s",
         cl_get_error_text(ret));
      ret_val = 5;
      goto cleanup;
   }

   /*
    * From here on, the two worker threads are doing all the work.
    * This main thread is just waiting until the client closes the 
    * connection to us, which causes the commlib_to_tty thread to 
    * exit. Then it closes the tty_to_commlib thread, too, and 
    * cleans up everything.
    */
   DPRINTF(("waiting for end of commlib_to_tty thread\n"));
   thread_join(pthread_commlib_to_tty);

   DPRINTF(("shutting down tty_to_commlib thread\n"));
   thread_shutdown(pthread_tty_to_commlib);

   /*
    * Close stdin to awake the tty_to_commlib-thread from the select() call.
    * thread_shutdown() doesn't work on all architectures.
    */
   close(STDIN_FILENO);

   DPRINTF(("waiting for end of tty_to_commlib thread\n"));
   thread_join(pthread_tty_to_commlib);
cleanup:
   /*
    * Set our terminal back to 'unraw' mode. Should be done automatically
    * by OS on process end, but we want to be sure.
    */
   ret = terminal_leave_raw_mode();
   DPRINTF(("terminal_leave_raw_mode() returned %s (%d)\n", strerror(ret), ret));
   if (ret != 0) {
      sge_dstring_sprintf(p_err_msg, "error resetting terminal mode: %s", strerror(ret));
      ret_val = 7; 
   }

   *p_exit_status = g_exit_status;

   thread_cleanup_lib(&thread_lib_handle);
   DRETURN(ret_val);
}

/****** sge_client_ijs/start_ijs_server() **************************************
*  NAME
*     start_ijs_server() -- starts the commlib server for the builtin
*                           interactive job support
*
*  SYNOPSIS
*     int start_ijs_server(const char* username, int csp_mode,
*                          COMM_HANDLE **phandle, dstring *p_err_msg)
*
*  FUNCTION
*     Starts the commlib server for the commlib connection between the shepherd
*     of the interactive job (qrsh/qlogin) and the qrsh/qlogin command. 
*     Over this connection the stdin/stdout/stderr input/output is transferred.
*
*  INPUTS
*     bool csp_mode -        If false, the server uses unsecured communications,
*                            otherwise it uses secured communictions.
*     const char* username - The owner of the certificates that are used to
*                            secure the connection.
*                            Used only in CSP mode, otherwise ignored.
*  OUTPUTS
*     COMM_HANDLE **handle - Pointer to the COMM server handle.
*                            Gets initialized in this function.
*     dstring *p_err_msg -   Contains the error reason in case of error.
*
*  RESULT
*     int - 0: OK
*           1: Can't open connection
*           2: Can't set connection parameters
*
*  NOTES
*     MT-NOTE: start_builtin_ijs_server() is not MT safe 
*
*  SEE ALSO
*     sge_client_ijs/run_ijs_server()
*     sge_client_ijs/stop_ijs_server()
*     sge_client_ijs/force_ijs_server_shutdown()
*******************************************************************************/
int start_ijs_server(bool csp_mode, const char* username,
                     COMM_HANDLE **phandle, dstring *p_err_msg)
{
   int     ret, ret_val = 0;

   DENTER(TOP_LAYER, "start_ijs_server");

   /* we must copy the hostname here to a global variable, because the 
    * worker threads need it later.
    * It gets freed in force_ijs_server_shutdown().
    * TODO: Cleaner solution for this!
    */
   DPRINTF(("starting commlib server\n"));
   ret = comm_open_connection(true, csp_mode, COMM_SERVER, 0, COMM_CLIENT,
                              NULL, username, phandle, p_err_msg);
   if (ret != 0 || *phandle == NULL) {
      ret_val = 1;
   } else {
      ret = comm_set_connection_param(*phandle, HEARD_FROM_TIMEOUT,
                                      0, p_err_msg);
      if (ret != 0) {
         ret_val = 2;
      }
   }

   DRETURN(ret_val);
}

/****** sge_client_ijs/stop_ijs_server() ***************************************
*  NAME
*     stop_ijs_server() -- stops the commlib server for the builtin
*                          interactive job support
*
*  SYNOPSIS
*     int stop_ijs_server(COMM_HANDLE **phandle, dstring *p_err_msg) 
*
*  FUNCTION
*     Stops the commlib server for the commlib connection between the shepherd
*     of the interactive job (qrsh/qlogin) and the qrsh/qlogin command.
*     Over this connectin the stdin/stdout/stderr input/output is transferred.
*
*  INPUTS
*     COMM_HANDLE **phandle - Pointer to the COMM server handle. Gets set to
*                             NULL in this function.
*     dstring *p_err_msg    - Contains the error reason in case of error.
*
*  RESULT
*     int - 0: OK
*           1: Invalid Parameter: phandle = NULL
*           2: General error shutting down the COMM server,
*              see p_err_msg for details
*
*  NOTES
*     MT-NOTE: stop_ijs_server() is not MT safe 
*
*  SEE ALSO
*     sge_client_ijs/start_ijs_server()
*     sge_client_ijs/run_ijs_server()
*     sge_client_ijs/force_ijs_server_shutdown()
*******************************************************************************/
int stop_ijs_server(COMM_HANDLE **phandle, dstring *p_err_msg)
{
   int ret = 0;

   DENTER(TOP_LAYER, "stop_ijs_server");

   if (phandle == NULL) {
      ret = 1;
   } else if (*phandle != NULL) {
      cl_com_set_error_func(NULL);
#if 0
      cl_log_list_set_log_level(cl_com_get_log_list(), CL_LOG_OFF);
#endif
      cl_com_ignore_timeouts(true);
      DPRINTF(("shut down the connection from our side\n"));
      ret = cl_commlib_shutdown_handle(*phandle, false);
      if (ret != CL_RETVAL_OK) {
         sge_dstring_sprintf(p_err_msg, "error shutting down the connection: %s",
            cl_get_error_text(ret));
         ret = 2;
      }
      *phandle = NULL;
   }
   DRETURN(ret);
}

/****** sge_client_ijs/force_ijs_server_shutdown() *****************************
*  NAME
*     force_ijs_server_shutdown() -- forces the commlib server for the builtin
*                                    interactive job support to shut down
*
*  SYNOPSIS
*     int force_ijs_server_shutdown(COMM_HANDLE **phandle, const char 
*     *this_component, dstring *p_err_msg) 
*
*  FUNCTION
*     Forces the commlib server for the builtin interactive job support to shut
*     down immediately and ensures it is shut down.
*
*  INPUTS
*     COMM_HANDLE **phandle      - Handle of the COMM connection, gets set to
*                                  NULL in this function.
*     const char *this_component - Name of this component.
*     dstring *p_err_msg         - Contains the error reason in case of error.
*
*  RESULT
*     int - 0: OK
*           1: Invalid parameter: phandle == NULL or *phandle == NULL
*           2: Can't shut down connection, see p_err_msg for details
*
*  NOTES
*     MT-NOTE: force_ijs_server_shutdown() is not MT safe 
*
*  SEE ALSO
*     sge_client_ijs/start_ijs_server()
*     sge_client_ijs/run_ijs_server()
*     sge_client_ijs/stop_ijs_server_shutdown()
*******************************************************************************/
int force_ijs_server_shutdown(COMM_HANDLE **phandle,
                              const char *this_component,
                              dstring *p_err_msg)
{
   int     ret;

   DENTER(TOP_LAYER, "force_ijs_server_shutdown");

   if (phandle == NULL || *phandle == NULL) {
      sge_dstring_sprintf(p_err_msg, "invalid connection handle");
      DPRINTF(("invalid connection handle - nothing to shut down\n"));
      DRETURN(1);
   }

   DPRINTF(("connection is still alive\n"));

   /* This will remove the handle */
   ret = comm_shutdown_connection(*phandle, COMM_CLIENT, g_hostname, p_err_msg);
   sge_free(&g_hostname);
   if (ret != COMM_RETVAL_OK) {
      DPRINTF(("comm_shutdown_connection() failed: %s (%d)\n",
               sge_dstring_get_string(p_err_msg), ret));
      ret = 2;
   } else {
      DPRINTF(("successfully shut down the connection\n"));
   }
   *phandle = NULL;

   DRETURN(ret);
}

