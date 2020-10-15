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
 *  Portions of this code are Copyright 2011 Univa Inc.
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/resource.h>

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
#  include <sys/ioctl.h>
#  include <termios.h>
#else
#  include <termio.h>
#endif

#include "uti/sge_stdlib.h"
#include "uti/sge_dstring.h"
#include "uti/sge_pty.h"
#include "uti/sge_io.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_unistd.h"
#include "uti/sge_signal.h"

#include "sge_ijs_comm.h"
#include "sge_ijs_threads.h"
#include "sge_fileio.h"
#include "basis_types.h"
#include "err_trace.h"
#include "shepherd.h"

#define RESPONSE_MSG_TIMEOUT 120

#define COMM_SERVER "qrsh_ijs"
#define COMM_CLIENT "shepherd_ijs"

/* 
 * Compile with EXTENSIVE_TRACING defined to get lots of trace messages from
 * the two worker threads.
 */
#undef EXTENSIVE_TRACING
#if 0
#define EXTENSIVE_TRACING
#endif

static ijs_fds_t     *g_p_ijs_fds         = NULL;
static COMM_HANDLE   *g_comm_handle       = NULL;
static THREAD_HANDLE g_thread_main;
static int           g_raised_event        = 0;
static int           g_job_pid             = 0;

static char          *g_hostname           = NULL;
extern int           received_signal; /* defined in shepherd.c */

/*
 * static functions 
 */
/****** trace_buf() **********************************************************
*  NAME
*     trace_buf() -- writes contents of a buffer partially to the trace file
*
*  SYNOPSIS
*     int trace_buf(const char *buffer, int length, const char *format, ...)
*
*  FUNCTION
*     Writes the contents of a buffer, preceeded by a formatted string,
*     partially to the trace file. I.e. it first writes the formatted
*     string to the trace file, replacing the printf placeholders with
*     the variables provided in ..., and then adds the first 99 or "length" 
*     characters (which ever is smaller), encapsulated by hyphens, to
*     the formatted string.
*
*  INPUTS
*     const char *buffer - the buffer that is to be written to the trace file
*     int        length  - length of the content of the buffer
*     const char *format - 
*     int  fd          - file descriptor to read from
*     char *pbuf       - working buffer, must be of size BUFSIZE
*     int  *buf_bytes  - number of bytes already in the buffer
*
*  OUTPUTS
*     int *buf_bytes   - number of bytes in the buffer
*
*  RESULT
*     int - >0: OK, number of bytes read from the fd
*           =0: EINTR or EAGAIN occured, just call select() and read again
*           -1: error occcured, connection was closed
*
*  NOTES
*     MT-NOTE: 
*
*  SEE ALSO
*******************************************************************************/
#ifdef EXTENSIVE_TRACING
static int trace_buf(const char *buffer, int length, const char *format, ...)
{
   int         ret;
   va_list     ap;
   dstring     message = DSTRING_INIT;
   char        tmpbuf[100];

   if (length > 0) {
      snprintf(tmpbuf, MIN(99,length), "%s", buffer);
   } else {
      strcpy(tmpbuf, "");
   }

   va_start(ap, format);
   
   sge_dstring_vsprintf(&message, format, ap);
   sge_dstring_append(&message, "\"");
   sge_dstring_append(&message, tmpbuf);
   sge_dstring_append(&message, "\"");
   ret = shepherd_trace("%s", sge_dstring_get_string(&message));

   sge_dstring_free(&message);
   va_end(ap);

   return ret;
}
#endif

/****** append_to_buf() ******************************************************
*  NAME
*     append_to_buf() -- copies data from pty (or pipe) to a buffer
*
*  SYNOPSIS
*     int append_to_buf(int fd, char *pbuf, int *buf_bytes)
*
*  FUNCTION
*     Reads data from the pty or pipe and appends it to the given buffer.
*
*  INPUTS
*     int  fd          - file descriptor to read from
*     char *pbuf       - working buffer, must be of size BUFSIZE
*     int  *buf_bytes  - number of bytes already in the buffer
*
*  OUTPUTS
*     int *buf_bytes   - number of bytes in the buffer
*
*  RESULT
*     int - >0: OK, number of bytes read from the fd
*           =0: EINTR or EAGAIN occured, just call select() and read again
*           -1: error occcured, connection was closed
*
*  NOTES
*     MT-NOTE: 
*
*  SEE ALSO
*******************************************************************************/
static int append_to_buf(int fd, char *pbuf, int *buf_bytes)
{
   int nread = 0;

   if (fd >= 0) {
      nread = read(fd, &pbuf[*buf_bytes], BUFSIZE-1-(*buf_bytes));

      if (nread < 0 && (errno == EINTR || errno == EAGAIN)) {
         nread = 0;
      } else if (nread <= 0) {
         nread = -1;
      } else {
         *buf_bytes += nread;
      }
   }
   return nread;
}

/****** send_buf() ***********************************************************
*  NAME
*     send_buf() -- sends the content of the buffer over the commlib
*
*  SYNOPSIS
*     int send_buf(char *pbuf, int buf_bytes, int message_type)
*
*  FUNCTION
*     Sends the content of the buffer over to the commlib to the receiver.
*
*  INPUTS
*     char *pbuf        - buffer to send
*     int  buf_bytes   - number of bytes in the buffer to send
*     int  message_tye - type of the message that is to be sent over
*                        commlib. Can be STDOUT_DATA_MSG or
*                        STDERR_DATA_MSG, depending on where the data 
*                        came from (pty and stdout = STDOUT_DATA_MSG,
*                        stderr = STDERR_DATA_MSG)
*
*  RESULT
*     int - 0: OK
*           1: could'nt write all data
*
*  NOTES
*     MT-NOTE: 
*
*  SEE ALSO
*******************************************************************************/
static int send_buf(char *pbuf, int buf_bytes, int message_type)
{
   int ret = 0;
   dstring err_msg = DSTRING_INIT;

   if (comm_write_message(g_comm_handle, g_hostname, 
      COMM_SERVER, 1, (unsigned char*)pbuf, 
      (unsigned long)buf_bytes, message_type, &err_msg) != buf_bytes) {
      shepherd_trace("couldn't write all data: %s",
                     sge_dstring_get_string(&err_msg));
      ret = 1;
   } else {
#ifdef EXTENSIVE_TRACING
      shepherd_trace("successfully wrote all data: %s", 
                     sge_dstring_get_string(&err_msg));
#endif
   }

   sge_dstring_free(&err_msg);
   return ret;
}

/****** pty_to_commlib() *******************************************************
*  NAME
*     pty_to_commlib() -- pty_to_commlib thread entry point and main loop
*
*  SYNOPSIS
*     void* pty_to_commlib(void *t_conf)
*
*  FUNCTION
*     Entry point and main loop of the pty_to_commlib thread.
*     Reads data from the pty and writes it to the commlib.
*
*  INPUTS
*     void *t_conf - pointer to cl_thread_settings_t struct of the thread
*
*  RESULT
*     void* - always NULL
*
*  NOTES
*     MT-NOTE: 
*
*  SEE ALSO
*******************************************************************************/
static void* pty_to_commlib(void *t_conf)
{
   int                  do_exit = 0;
   int                  fd_max = 0;
   int                  ret;
   fd_set               read_fds;
   struct timeval       timeout;
   char                 *stdout_buf = NULL;
   char                 *stderr_buf = NULL;
   int                  stdout_bytes = 0;
   int                  stderr_bytes = 0;
   bool                 b_select_timeout = false;
   dstring              err_msg = DSTRING_INIT;

   /* Report to thread lib that this thread starts up now */
   thread_func_startup(t_conf);

   /* Allocate working buffers, BUFSIZE = 64k */
   stdout_buf = sge_malloc(BUFSIZE);
   stderr_buf = sge_malloc(BUFSIZE);

   /* The main loop of this thread */
   while (do_exit == 0) {
      /* Fill fd_set for select */
      FD_ZERO(&read_fds);

      if (g_p_ijs_fds->pty_master != -1) {
         FD_SET(g_p_ijs_fds->pty_master, &read_fds);
      }
      if (g_p_ijs_fds->pipe_out != -1) {
         FD_SET(g_p_ijs_fds->pipe_out, &read_fds);
      }
      if (g_p_ijs_fds->pipe_err != -1) {
         FD_SET(g_p_ijs_fds->pipe_err, &read_fds);
      }
      fd_max = MAX(g_p_ijs_fds->pty_master, g_p_ijs_fds->pipe_out);
      fd_max = MAX(fd_max, g_p_ijs_fds->pipe_err);

#ifdef EXTENSIVE_TRACING
      shepherd_trace("pty_to_commlib: g_p_ijs_fds->pty_master = %d, "
                     "g_p_ijs_fds->pipe_out = %d, "
                     "g_p_ijs_fds->pipe_err = %d, fd_max = %d",
                     g_p_ijs_fds->pty_master, g_p_ijs_fds->pipe_out,
                     g_p_ijs_fds->pipe_err, fd_max);
#endif
      /* Fill timeout struct for select */
      b_select_timeout = false;
      if (do_exit == 1) {
         /* If we know that we have to exit, don't wait for data,
          * just peek into the fds and read all data available from the buffers. */
         timeout.tv_sec  = 0;
         timeout.tv_usec = 0;
      } else {
         if ((stdout_bytes > 0 && stdout_bytes < 256)
             || cl_com_messages_in_send_queue(g_comm_handle) > 0) {
            /* 
             * If we just received few data, wait another 10 milliseconds if new 
             * data arrives to avoid sending data over the network in too small junks.
             * Also retry to send the messages that are still in the queue ASAP.
             */
            timeout.tv_sec  = 0;
            timeout.tv_usec = 10000;
         } else {
            /* Standard timeout is one second */
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
         }
      }
#ifdef EXTENSIVE_TRACING
      shepherd_trace("pty_to_commlib: doing select() with %d seconds, "
                     "%d usec timeout", timeout.tv_sec, timeout.tv_usec);
#endif
      /* Wait blocking for data from pty or pipe */
      errno = 0;
      ret = select(fd_max+1, &read_fds, NULL, NULL, &timeout);
      thread_testcancel(t_conf);
/* This is a workaround for Darwin and HP11, where thread_testcancel() doesn't work.
 * TODO: Find the reason why it doesn't work and remove the workaround
 */
      if (g_raised_event > 0) {
         do_exit = 1;
      }
#ifdef EXTENSIVE_TRACING
      shepherd_trace("pty_to_commlib: select() returned %d", ret);
#endif
      if (ret < 0) {
         /* select error */
#ifdef EXTENSIVE_TRACING
         shepherd_trace("pty_to_commlib: select() returned %d, reason: %d, %s",
                        ret, errno, strerror(errno));
#endif
         if (errno == EINTR) {
            /* If we have a buffer, send it now (as we don't want to care about
             * how long we acutally waited), then just continue, the top of the 
             * loop will handle signals.
             * b_select_timeout tells the bottom of the loop to send the buffer.
             */

            /* ATTENTION: Don't call shepherd_trace() here, because on some
             * architectures it causes another EINTR, leading to a infinte loop.
             */
            b_select_timeout = true;
         } else {
            shepherd_trace("pty_to_commlib: select() error -> exiting");
            do_exit = 1;
         }
      } else if (ret == 0) {
         /* timeout, if we have a buffer, send it now */
#ifdef EXTENSIVE_TRACING
         shepherd_trace("pty_to_commlib: select() timeout");
#endif
         b_select_timeout = true;
      } else {
         /* at least one fd is ready to read from */
         ret = 0;
         /* now we can be sure that our child has started the job,
          * we can close the pipe_to_child now
          */
         if (g_p_ijs_fds->pty_master != -1 && FD_ISSET(g_p_ijs_fds->pty_master, &read_fds)) {
#ifdef EXTENSIVE_TRACING
            shepherd_trace("pty_to_commlib: reading from ptym");
#endif
            ret = append_to_buf(g_p_ijs_fds->pty_master, stdout_buf, &stdout_bytes);
#ifdef EXTENSIVE_TRACING
            trace_buf(stdout_buf, ret, "pty_to_commlib: appended %d bytes, stdout_buf = ", 
                      ret, stdout_buf);
#endif
         }
         if (ret >= 0 && g_p_ijs_fds->pipe_out != -1
             && FD_ISSET(g_p_ijs_fds->pipe_out, &read_fds)) {
#ifdef EXTENSIVE_TRACING
            shepherd_trace("pty_to_commlib: reading from pipe_out");
#endif
            ret = append_to_buf(g_p_ijs_fds->pipe_out, stdout_buf, &stdout_bytes);

#ifdef EXTENSIVE_TRACING
            trace_buf(stdout_buf, ret, "pty_to_commlib: appended %d bytes, stdout_buf = ",
                      ret, stdout_buf);
#endif
         }
         if (ret < 0) {
            shepherd_trace("pty_to_commlib: STDOUT was closed. Our child seems to have exited -> exiting");
            do_exit = 1;
         }
         if (g_p_ijs_fds->pipe_err != -1
             && FD_ISSET(g_p_ijs_fds->pipe_err, &read_fds)) {
#ifdef EXTENSIVE_TRACING
            shepherd_trace("pty_to_commlib: reading from pipe_err");
#endif
            ret = append_to_buf(g_p_ijs_fds->pipe_err, stderr_buf, &stderr_bytes);
#ifdef EXTENSIVE_TRACING
            trace_buf(stderr_buf, ret, "pty_to_commlib: appended %d bytes, stderr_buf = ",
                      ret, stderr_buf);
#endif
         }
         if (ret < 0) {
            /* A fd was closed, likely our child has exited, we can exit, too. */
            shepherd_trace("pty_to_commlib: STDERR was closed. Our child seems to have exited -> exiting");
            do_exit = 1;
         } else if (ret > 0) {
            if (g_p_ijs_fds->pipe_to_child != -1) {
               shepherd_trace("pty_to_commlib: closing pipe to child");
               close(g_p_ijs_fds->pipe_to_child);
               g_p_ijs_fds->pipe_to_child = -1;
            }
         }
      }
      /* Always send stderr buffer immediately */
      if (stderr_bytes != 0) {
         ret = send_buf(stderr_buf, stderr_bytes, STDERR_DATA_MSG);
         if (ret == 0) {
            stderr_bytes = 0;
         } else {
            shepherd_trace("pty_to_commlib: send_buf() returned %d "
                           "-> exiting", ret);
            do_exit = 1;
         }
      }
      /* 
       * Send stdout_buf if there is enough data in it
       * OR if there was a select timeout (don't wait too long to send data)
       * OR if we will exit the loop now and there is data in the stdout_buf
       */
      if (stdout_bytes >= 256 ||
          (b_select_timeout == true && stdout_bytes > 0) ||
          (do_exit == 1 && stdout_bytes > 0)) {
#ifdef EXTENSIVE_TRACING
         shepherd_trace("pty_to_commlib: sending stdout buffer");
#endif
         ret = send_buf(stdout_buf, stdout_bytes, STDOUT_DATA_MSG);
         stdout_bytes = 0;
         if (ret != 0) {
            shepherd_trace("pty_to_commlib: send_buf() failed -> exiting");
            do_exit = 1;
         }
         comm_flush_write_messages(g_comm_handle, &err_msg);
      }
   }

#ifdef EXTENSIVE_TRACING
   shepherd_trace("pty_to_commlib: shutting down thread");
#endif
   sge_free(&stdout_buf);
   sge_free(&stderr_buf);
   thread_func_cleanup(t_conf);

/* TODO: This could cause race conditions in the main thread, replace with pthread_condition */
/* HP: How can this cause race conditions? */
#ifdef EXTENSIVE_TRACING
   shepherd_trace("pty_to_commlib: raising event for main thread");
#endif
   g_raised_event = 2;
   thread_trigger_event(&g_thread_main);
   sge_dstring_free(&err_msg);

#ifdef EXTENSIVE_TRACING
   shepherd_trace("pty_to_commlib: leaving pty_to_commlib thread");
#endif
   return NULL;
}

/****** commlib_to_pty() *******************************************************
*  NAME
*     commlib_to_pty() -- commlib_to_pty thread entry point and main loop
*
*  SYNOPSIS
*     void* commlib_to_pty(void *t_conf)
*
*  FUNCTION
*     Entry point and main loop of the commlib_to_pty thread.
*     Reads data from the commlib and writes it to the pty.
*
*  INPUTS
*     void *t_conf - pointer to cl_thread_settings_t struct of the thread
*
*  RESULT
*     void* - always NULL
*
*  NOTES
*     MT-NOTE:
*
*  SEE ALSO
*******************************************************************************/
static void* commlib_to_pty(void *t_conf)
{
   recv_message_t       recv_mess;
   int                  b_was_connected = 0;
   bool                 b_sent_to_child = false;
   int                  ret;
   int                  do_exit = 0;
   int                  fd_write = -1;
   dstring              err_msg = DSTRING_INIT;

   /* report to thread lib that thread starts up now */
   thread_func_startup(t_conf);

   if (g_p_ijs_fds->pty_master != -1) {
      fd_write = g_p_ijs_fds->pty_master;
   } else if (g_p_ijs_fds->pipe_in != -1) {
      fd_write = g_p_ijs_fds->pipe_in;
   } else {
      do_exit = 1;
      shepherd_trace("commlib_to_pty: no valid handle for stdin available. Exiting!");
   }

   cl_com_set_synchron_receive_timeout(g_comm_handle, 1);

   while (do_exit == 0) {
      /* wait blocking for a message from commlib */
      recv_mess.cl_message = NULL;
      recv_mess.data       = NULL;
      sge_dstring_free(&err_msg);
      sge_dstring_sprintf(&err_msg, "%s", "");

      ret = comm_recv_message(g_comm_handle, true, &recv_mess, &err_msg);

      /* 
       * Check if the thread was cancelled. Exit thread if it was.
       * It shouldn't be neccessary to do the check here, as the cancel state 
       * of the thread is 1, i.e. the thread may be cancelled at any time,
       * but this doesn't work on some architectures (Darwin, older Solaris).
       */
      thread_testcancel(t_conf);
      if (g_raised_event > 0) {
         do_exit = 1;
         continue;
      }
#ifdef EXTENSIVE_TRACING
      shepherd_trace("commlib_to_pty: comm_recv_message() returned %d, err_msg: %s",
                     ret, sge_dstring_get_string(&err_msg));
#endif

      if (ret != COMM_RETVAL_OK) {
         /* handle error cases */
         switch (ret) {
            case COMM_NO_SELECT_DESCRIPTORS: 
               /*
                * As long as we're not connected, this return value is expected.
                * If we were already connected, it means the connection was closed.
                */
               if (b_was_connected == 1) {
                  shepherd_trace("commlib_to_pty: was connected, "
                                 "but lost connection -> exiting");
                  do_exit = 1;
               }
               break;
            case COMM_CONNECTION_NOT_FOUND:
               if (b_was_connected == 0) {
                  shepherd_trace("commlib_to_pty: our server is not running -> exiting. Error: %s", sge_dstring_get_string(&err_msg));
                  /*
                   * On some architectures (e.g. Darwin), the child doesn't recognize
                   * when the pipe gets closed, so we have to send the information
                   * that the parent will exit soon to the child.
                   * "noshell" may be 0 or 1, everything else indicates an error.
                   */
                  if (b_sent_to_child == false) {
                     if (write(g_p_ijs_fds->pipe_to_child, "noshell = 9", 11) != 11) {
                        shepherd_trace("commlib_to_pty: error in communicating "
                           "with child -> exiting");
                     } else {
                        b_sent_to_child = true;
                     }
                  }
               } else {
                  shepherd_trace("commlib_to_pty: was connected and still have "
                                 "selectors, but lost connection -> exiting. Error: %s", sge_dstring_get_string(&err_msg));
               }
               do_exit = 1;
               break;
            case COMM_INVALID_PARAMETER:
               shepherd_trace("commlib_to_pty: communication handle or "
                        "message buffer is invalid -> exiting");
               do_exit = 1;
               break;
            case COMM_CANT_TRIGGER:
               shepherd_trace("commlib_to_pty: can't trigger communication, likely the "
                              "communication was shut down by other thread -> exiting");
               shepherd_trace("commlib_to_pty: err_msg: %s", 
                              sge_dstring_get_string(&err_msg));
               do_exit = 1;
               break;
            case COMM_CANT_RECEIVE_MESSAGE:
               if (check_client_alive(g_comm_handle, 
                                      COMM_SERVER,
                                      g_hostname,
                                      &err_msg) != COMM_RETVAL_OK) {
                  shepherd_trace("commlib_to_pty: not connected any more -> exiting.");
                  do_exit = 1;
               } else {
#ifdef EXTENSIVE_TRACING
                  shepherd_trace("commlib_to_pty: can't receive message, reason: %s "
                                 "-> trying again", 
                                 sge_dstring_get_string(&err_msg));
#endif
               }
               b_was_connected = 1;
               break;
            case COMM_GOT_TIMEOUT:
#ifdef EXTENSIVE_TRACING
               shepherd_trace("commlib_to_pty: got timeout -> trying again");
#endif
               b_was_connected = 1;
               break;
            case COMM_SELECT_INTERRUPT:
               /* Don't do tracing here */
               /* shepherd_trace("commlib_to_pty: interrupted select"); */
               b_was_connected = 1;
               break;
            case COMM_NO_MESSAGE_AVAILABLE:
#ifdef EXTENSIVE_TRACING
               shepherd_trace("commlib_to_pty: didn't receive a message within 1 s "
                  "timeout -> trying again");
#endif
               b_was_connected = 1; 
               break;
            default:
               /* Unknown error, just try again */
#ifdef EXTENSIVE_TRACING
               shepherd_trace("commlib_to_pty: comm_recv_message() returned %d -> "
                              "trying again", ret);
#endif
               b_was_connected = 1;
               break;
         }
      } else {  /* if (ret == COMM_RETVAL_OK) */
         /* We received a message, 'parse' it */
         switch (recv_mess.type) {
            case STDIN_DATA_MSG:
               /* data message, write data to stdin of child */
#ifdef EXTENSIVE_TRACING
               shepherd_trace("commlib_to_pty: received data message");
               shepherd_trace("commlib_to_pty: writing data to stdin of child, "
                              "length = %d", recv_mess.cl_message->message_length-1);
#endif

               if (sge_writenbytes(fd_write,  
                          recv_mess.data, 
                          (int)(recv_mess.cl_message->message_length-1)) 
                       != (int)(recv_mess.cl_message->message_length-1)) {
                  shepherd_trace("commlib_to_pty: error writing to stdin of "
                                 "child: %d, %s", errno, strerror(errno));
               }
               b_was_connected = 1;
               break;

            case STDIN_CLOSE_MSG:
               /* If we receive a STDIN_CLOSE_MSG we have to close the filedescriptor!
                * This is needed for GE-3580
                */
               shepherd_trace("commlib_to_pty: received stdin_close message");
               SGE_CLOSE(fd_write);
               b_was_connected = 1;
               break;

            case SUSPEND_CTRL_MSG:
               shepherd_trace("commlib_to_pty: received suspend message, "
                   "suspend the process");
               shepherd_signal_job(g_job_pid, SIGSTOP);
               break;

            case UNSUSPEND_CTRL_MSG:
               shepherd_trace("commlib_to_pty: received unsuspend message, "
                   "awake the process");
               shepherd_signal_job(g_job_pid, SIGCONT);
               break;

            case WINDOW_SIZE_CTRL_MSG:
               /* control message, set size of pty */
               shepherd_trace("commlib_to_pty: received window size message, "
                  "changing window size");
               ioctl(fd_write, TIOCSWINSZ, &(recv_mess.ws));
               b_was_connected = 1;
               break;
            case SETTINGS_CTRL_MSG:
               /* control message */
               shepherd_trace("commlib_to_pty: received settings message");
               /* Forward the settings to the child process.
                * This is also tells the child process that it can start
                * the job 'in' the pty now.
                */
               shepherd_trace("commlib_to_pty: writing to child %ld bytes: %s",
                              (long) strlen(recv_mess.data), recv_mess.data);
               if (write(g_p_ijs_fds->pipe_to_child, recv_mess.data, 
                         strlen(recv_mess.data)) != strlen(recv_mess.data)) {
                  shepherd_trace("commlib_to_pty: error in communicating "
                     "with child -> exiting");
                  do_exit = 1;
               } else {
                  b_sent_to_child = true;
               }
               b_was_connected = 1;
               break;
            default:
               shepherd_trace("commlib_to_pty: received unknown message");
               break;
         }
      }
      comm_free_message(&recv_mess, &err_msg);
   }
   /*
    * When we get here, likely the commlib connection was shut down
    * from the other side. We have to kill the job here to wake up
    * the main thread.
    */
   /* 
    * TODO: Use SIGINT if qrsh client was quit with Ctrl-C
    */
   shepherd_signal_job(g_job_pid, SIGKILL);

   sge_dstring_free(&err_msg);

#ifdef EXTENSIVE_TRACING
   shepherd_trace("commlib_to_pty: leaving commlib_to_pty function");
#endif
   thread_func_cleanup(t_conf);
/* TODO: pthread_condition, see other thread*/
#ifdef EXTENSIVE_TRACING
   shepherd_trace("commlib_to_pty: raising event for main thread");
#endif
   g_raised_event = 3;
   thread_trigger_event(&g_thread_main);
#ifdef EXTENSIVE_TRACING
   shepherd_trace("commlib_to_pty: leaving commlib_to_pty thread");
#endif
   return NULL;
}

int
parent_loop(int job_pid, const char *childname, int timeout, ckpt_info_t *p_ckpt_info,
   ijs_fds_t *p_ijs_fds, const char *job_owner, const char *remote_host,
   int remote_port, bool csp_mode, int *exit_status, struct rusage *rusage,
   dstring *err_msg)
{
   int               ret;
   THREAD_LIB_HANDLE *thread_lib_handle     = NULL;
   THREAD_HANDLE     *thread_pty_to_commlib = NULL;
   THREAD_HANDLE     *thread_commlib_to_pty = NULL;
   cl_raw_list_t     *cl_com_log_list = NULL;

   shepherd_trace("parent: starting parent loop with remote_host = %s, "
                  "remote_port = %d, job_owner = %s, fd_pty_master = %d, "
                  "fd_pipe_in = %d, fd_pipe_out = %d, "
                  "fd_pipe_err = %d, fd_pipe_to_child = %d",
                  remote_host, remote_port, job_owner, p_ijs_fds->pty_master,
                  p_ijs_fds->pipe_in, p_ijs_fds->pipe_out, p_ijs_fds->pipe_err,
                  p_ijs_fds->pipe_to_child);

   g_hostname  = strdup(remote_host);
   g_p_ijs_fds = p_ijs_fds;
   g_job_pid   = job_pid;

   /*
    * Initialize err_msg, so it's never NULL.
    */
   sge_dstring_sprintf(err_msg, "%s", "");

   ret = comm_init_lib(err_msg);
   if (ret != COMM_RETVAL_OK) {
      shepherd_trace("parent: init comm lib failed: %d", ret);
      return 1;
   }

   /*
    * Setup thread list.
    */
   ret = thread_init_lib(&thread_lib_handle);
   if (ret != CL_RETVAL_OK) {
      shepherd_trace("parent: init thread lib thread failed: %d", ret);
      return 1;
   }

   /*
    * Get log list of communication before a connection is opened.
    */
   cl_com_log_list = cl_com_get_log_list();

   /* 
    * Register this main thread at the thread library, so it can
    * be triggered and create two worker threads.
    */
   ret = register_thread(cl_com_log_list, &g_thread_main, "main thread");
   if (ret != CL_RETVAL_OK) {
      shepherd_trace("parent: registering main thread failed: %d", ret);
      return 1;
   }

   /*
    * Open the connection port so we can connect to our server
    */
   shepherd_trace("parent: opening connection to qrsh/qlogin client");
   ret = comm_open_connection(false, csp_mode, COMM_CLIENT, remote_port,
                              COMM_SERVER, g_hostname, job_owner,
                              &g_comm_handle, err_msg);
   if (ret != COMM_RETVAL_OK) {
      shepherd_trace("parent: can't open commlib stream, err_msg = %s", 
                     sge_dstring_get_string(err_msg));
      return 1;
   }

   /*
    * register at qrsh/qlogin client, which is the server of the communication.
    * The answer of the server (a WINDOW_SIZE_CTRL_MSG) will be handled in the
    * commlib_to_pty thread.
    */
   shepherd_trace("parent: sending REGISTER_CTRL_MSG to qrsh/qlogin client");
   ret = (int)comm_write_message(g_comm_handle, g_hostname, COMM_SERVER, 1, 
                      (unsigned char*)" ", 1, REGISTER_CTRL_MSG, err_msg);
   if (ret == 0) {
      /* No bytes written - error */
      shepherd_trace("parent: can't send REGISTER_CTRL_MSG, comm_write_message() "
                     "returned: %s", sge_dstring_get_string(err_msg));
   /* Don't exit here, the error handling is done in the commlib_to_tty-thread */
   /* Most likely, the qrsh client is not running, so it's not the shepherds fault,
    * we shouldn't return 1 (which leads to a shepherd_exit which sets the whole
    * queue in error!).
    */
   /*   return 1;*/
   }
#ifdef EXTENSIVE_TRACING
   else {
      shepherd_trace("parent: Sent %d bytes to qrsh client", ret);
   }
#endif

   {
      sigset_t old_sigmask;
      sge_thread_block_all_signals(&old_sigmask);

      shepherd_trace("parent: creating pty_to_commlib thread");
      ret = create_thread(thread_lib_handle,
                          &thread_pty_to_commlib,
                          cl_com_log_list,
                          "pty_to_commlib thread",
                          2,
                          pty_to_commlib);
      if (ret != CL_RETVAL_OK) {
         shepherd_trace("parent: creating pty_to_commlib thread failed: %d", ret);
      }

      shepherd_trace("parent: creating commlib_to_pty thread");
      ret = create_thread(thread_lib_handle,
                          &thread_commlib_to_pty,
                          cl_com_log_list,
                          "commlib_to_pty thread",
                          3,
                          commlib_to_pty);

      pthread_sigmask(SIG_SETMASK, &old_sigmask, NULL);
   }

   if (ret != CL_RETVAL_OK) {
      shepherd_trace("parent: creating commlib_to_pty thread failed: %d", ret);
   }

   /* From here on, the two worker threads are doing all the work.
    * This main thread is just waiting until one of the to worker threads
    * wants to exit and sends an event to the main thread.
    * A worker thread wants to exit when either the communciation to
    * the server was shut down or the user application (likely the shell)
    * exited.
    * On some architectures, this thread awakens from the wait whenever
    * a signal arrives. Therefore we have to check if it was a signal
    * or a event that awoke this thread.
    */
   shepherd_trace("parent: created both worker threads, now waiting for jobs end");

   *exit_status = wait_my_child(job_pid, childname, timeout, p_ckpt_info, rusage, -1, -1);
   alarm(0);

   shepherd_trace("parent: wait_my_child returned exit_status = %d", *exit_status);
   shepherd_trace("parent:            rusage.ru_stime.tv_sec  = %ld",
                  (long) rusage->ru_stime.tv_sec);
   shepherd_trace("parent:            rusage.ru_stime.tv_usec = %d",
                  (int) rusage->ru_stime.tv_usec);
   shepherd_trace("parent:            rusage.ru_utime.tv_sec  = %ld",
                  (long) rusage->ru_utime.tv_sec);
   shepherd_trace("parent:            rusage.ru_utime.tv_usec = %d",
                  (int) rusage->ru_utime.tv_usec);

   /*
    * We are sure the job exited when we get here, but there could still be
    * some output in the buffers, so wait for the communication threads
    * to give them time to read, transmit and flush the buffers.
    */
   while (g_raised_event == 0) {
      ret = thread_wait_for_event(&g_thread_main, 0, 0);
   }
   shepherd_trace("parent: received event %d, g_raised_event = %d", 
                  ret, g_raised_event);

   /* 
    * One of the worker threads sent an event, so shut down both threads now.
    * Shutdown the threads thread_pty_to_commlib and thread_commlib_to_pty
    */
   cl_raw_list_lock(thread_lib_handle);
   cl_thread_list_delete_thread_from_list(thread_lib_handle, thread_pty_to_commlib);
   cl_thread_list_delete_thread_from_list(thread_lib_handle, thread_commlib_to_pty);
   cl_raw_list_unlock(thread_lib_handle);

   shepherd_trace("parent: shutting down pty_to_commlib thread");
   cl_thread_shutdown(thread_pty_to_commlib);
   shepherd_trace("parent: shutting down commlib_to_pty thread");
   cl_thread_shutdown(thread_commlib_to_pty);

   /*
    * This will wake up all threads waiting for a message 
    */
   cl_thread_trigger_thread_condition(g_comm_handle->app_condition, 1);


   close(g_p_ijs_fds->pty_master);

   close(g_p_ijs_fds->pipe_in);
   close(g_p_ijs_fds->pipe_out);
   close(g_p_ijs_fds->pipe_err);

   /*
    * Wait until threads have shut down and call cleanup functions
    */
   cl_thread_join(thread_pty_to_commlib);
   cl_thread_join(thread_commlib_to_pty);
   cl_thread_cleanup(thread_pty_to_commlib);
   cl_thread_cleanup(thread_commlib_to_pty);


#if 0
   {
     struct timeb ts;
     ftime(&ts);
     shepherd_trace("+++++ timestamp: %d.%03d ++++", (int)ts.time, (int)ts.millitm);
   }
#endif

   /* From here on, only the main thread is running */
   shepherd_trace("parent: thread_cleanup_lib()");
   thread_cleanup_lib(&thread_lib_handle); 

   /* The communication will be freed in close_parent_loop() */
   sge_dstring_free(err_msg);
   shepherd_trace("parent: leaving main loop. From here on, only the main thread is running.");
   return 0;
}

int close_parent_loop(int exit_status)
{
   int     ret = 0;
   char    sz_exit_status[21]; 
   dstring err_msg = DSTRING_INIT;

   /*
    * Send UNREGISTER_CTRL_MSG
    */
   snprintf(sz_exit_status, 20, "%d", exit_status);
   shepherd_trace("sending UNREGISTER_CTRL_MSG with exit_status = \"%s\"", 
                  sz_exit_status);
   shepherd_trace("sending to host: %s", 
                  g_hostname != NULL ? g_hostname : "<null>");
   ret = (int)comm_write_message(g_comm_handle, g_hostname,
      COMM_SERVER, 1, (unsigned char*)sz_exit_status, strlen(sz_exit_status), 
      UNREGISTER_CTRL_MSG, &err_msg);

   if (ret != strlen(sz_exit_status)) {
      shepherd_trace("comm_write_message returned: %s", 
                             sge_dstring_get_string(&err_msg));
      shepherd_trace("close_parent_loop: comm_write_message() returned %d "
                             "instead of %d!!!", ret, (int) strlen(sz_exit_status));
   }

   /*
    * Wait for UNREGISTER_RESPONSE_CTRL_MSG
    */
   {
      int                  count = 0;
      recv_message_t       recv_mess;

      shepherd_trace("waiting for UNREGISTER_RESPONSE_CTRL_MSG");
      while (count < RESPONSE_MSG_TIMEOUT) {
         memset(&recv_mess, 0, sizeof(recv_message_t));
#if defined(INTERIX)
/*
 * TODO: comm_recv_message() should return immediatley when the server
 *       is not running any more. On Interix, it waits until a timeout
 *       occurs (60s), so we check if the server is running before
 *       we wait for the message.
 *       This has to be fixed in comm_recv_message() or the commlib.
 * HP: I guess this was fixed in commlib, can't reproduce an 60 s timeout
 *     any more.
 */
         if (check_client_alive(g_comm_handle, 
                                COMM_SERVER, g_hostname, &err_msg) != COMM_RETVAL_OK) {
            shepherd_trace("Server already exited");
            break;
         }
#endif
         ret = comm_recv_message(g_comm_handle, true, &recv_mess, &err_msg);
         count++;
         if (recv_mess.type == UNREGISTER_RESPONSE_CTRL_MSG) {
            shepherd_trace("Received UNREGISTER_RESPONSE_CTRL_MSG");
            comm_free_message(&recv_mess, &err_msg);
            break;
         } else if (ret == COMM_NO_MESSAGE_AVAILABLE) {
            /* trace this only every 10 loops (default: 1 loop = 1 s) */
            if (count%10 == 0) {
               shepherd_trace("still waiting for UNREGISTER_RESPONSE_CTRL_MSG");
            }
         } else if (ret == COMM_CONNECTION_NOT_FOUND) {
            shepherd_trace("client disconnected - break");
            break;
         } else {
            shepherd_trace("No connection or problem while waiting for message: %d", ret);
            break;
         }
         comm_free_message(&recv_mess, &err_msg);
      }
   }
   /* Now we are completely logged of from the server and can shut down */

   /*
    * Tell the communication to shut down immediately, don't wait for
    * the next timeout
    */
   shepherd_trace("parent: cl_com_ignore_timeouts");
   comm_ignore_timeouts(true, &err_msg); 

   /*
    * Do cleanup
    */
   ret = comm_cleanup_lib(&err_msg);
   if (ret != COMM_RETVAL_OK) {
      shepherd_trace("parent: error in comm_cleanup_lib(): %d", ret);
   }

   sge_free(&g_hostname);
   sge_dstring_free(&err_msg);
   shepherd_trace("parent: leaving close_parent_loop()");
   return 0;
}
