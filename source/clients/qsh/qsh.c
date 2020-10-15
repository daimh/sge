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
/*----------------------------------------------------
 * qsh.c
 *
 *
 *--------------------------------------------------*/

#include <string.h>
#include <stdlib.h>    /* need prototype for malloc */
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#if defined(INTERIX) || defined(HPUX)
#include <arpa/inet.h>
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_hostname.h"
#include "uti/sge_sl.h"
#include "uti/sge_profiling.h"
#include "uti/sge_stdio.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/setup_path.h" 
#include "uti/sge_afsutil.h"
#include "uti/sge_signal.h"
#include "uti/sge_unistd.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_var.h"
#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_job.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdi.h"
#include "gdi/sge_security.h"
#include "gdi/sge_qexec.h"
#include "gdi/qm_name.h"

#include "comm/commlib.h"

#include "sig_handlers.h"
#include "basis_types.h"
#include "sge_mt_init.h"
#include "symbols.h"
#include "usage.h"
#include "parse_qsub.h"
#include "parse_job_cull.h"
#include "parse_job_qsh.h"
#include "read_defaults.h"
#include "show_job.h"
#include "msg_clients_common.h"
#include "msg_qsh.h"
#include "msg_common.h"

#if defined(DARWIN)
#  include <termios.h>
#  include <sys/ttycom.h>
#  include <sys/ioctl.h>
#elif defined(HP11) || defined(HP1164)
#  include <termios.h>
#elif defined(INTERIX)
#  include <termios.h>
#  include <sys/ioctl.h>
#elif defined(FREEBSD) || defined(NETBSD)
#  include <termios.h>
#else
#  include <termio.h>
#endif

#include "uti/sge_pty.h"
#include "sge_ijs_comm.h"
#include "sge_ijs_threads.h"
#include "sge_client_ijs.h"
#include "uti/sge_parse_args.h"

#include "sgeobj/cull_parse_util.h"
#include "sgeobj/sge_jsv.h"

#if defined(__OpenBSD__)
#   define sigignore(x) signal(x,SIG_IGN)
#endif

/* global variables */
sge_gdi_ctx_class_t *ctx = NULL;

/* module variables */
static bool g_new_interactive_job_support = false;

static int open_qrsh_socket(int *port);
static int wait_for_qrsh_socket(int sock, int timeout);
static char *read_from_qrsh_socket(int msgsock);
static int get_remote_exit_code(int sock);
static const char *quote_argument(const char *arg);
static int parse_result_list(lList *alp, int *alp_error);
static int start_client_program(const char *client_name,
                                const char *host,
                                const char *port,
                                const char *job_dir,
                                const char *utilbin_dir,
                                int is_rsh,
                                int is_rlogin,
                                int nostdin,
                                int noshell,
                                int sock);
static int get_client_server_context(int msgsock, char **port, char **job_dir, char **utilbin_dir, const char **host);
static const char *get_client_name(sge_gdi_ctx_class_t *ctx, int is_rsh, int is_rlogin, int inherit_job);
static void set_job_info(lListElem *job, const char *name, int is_qlogin, int is_rsh, int is_rlogin);
static void remove_unknown_opts(lList *lp, u_long32 jb_now, int tightly_integrated, bool error,
                                int is_rsh);
static void delete_job(sge_gdi_ctx_class_t *ctx, u_long32 job_id, lList *lp);
static void set_builtin_ijs_signals_and_handlers(void);

#define VERBOSE_LOG(x) if (log_state_get_log_verbose()) { fprintf x; fflush(stderr); }

/****** Interactive/qsh/--Interactive ***************************************
*
*  NAME
*     qsh -- qsh, qlogin, qrsh, qrlogin, qrexec
*
*  SYNOPSIS
*     qsh    [options]
*     qlogin [options]
*     qrsh   [options] [arguments]
*
*  FUNCTION
*     Start an interactive job or task.
*     Different behavior is achieved by calling qsh with different names
*     (setting the links qlogin and qrsh to the qsh binary).
*
*     qsh starts an xterm on a host determined by the Sge/SGE scheduler
*     
*     qlogin start a telnet session on a host determined by the Sge/SGE
*     scheduler
*
*     qrsh starts an rsh or rlogin session on a host determined by the
*     Sge/SGE scheduler.
*     qrsh can also be used, to start a task in an already scheduled, 
*     parallel job on a specific host.
*
*  INPUTS
*
*  RESULT
*
*  EXAMPLE
*     qsh -l arch=solaris 
*     
*     qlogin -q balrog
*
*     qrsh -l netscape=1 netscape
*     
*     qsub -pe foo 3 job_script.sh
*     within job_script.sh:
*     qrsh -inherit <hostname> <command> <args>
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*     Interactive/qrsh/--qrsh_starter
*
****************************************************************************
*
*/

/****** Interactive/qsh/-Interactive-Global_Variables ***************************************
*
*  NAME
*     Global Variables -- global variables
*
*  SYNOPSIS
*     extern char **environ;
*     pid_t child_pid;
*
*  FUNCTION
*     environ                    - global array with environment of 
*                                  current process
*     child_pid                  - pid of the child rsh/rlogin/telnet client.
*                                  used to pass signals to child.
*
****************************************************************************
*
*/
extern char **environ;
pid_t child_pid = 0;

/****** Interactive/qsh/-Interactive-Defines ***************************************
*
*  NAME
*     Defines -- defines/constants 
*
*  SYNOPSIS
*     #define MAX_JOB_NAME 128
*     #define QSH_INTERACTIVE_POLLING_MIN 2
*     #define QSH_BATCH_POLLING_MIN 32
*     #define QSH_POLLING_MAX 256
*     #define QSH_SOCKET_FINAL_TIMEOUT 60
*
*  FUNCTION
*     MAX_JOB_NAME                - maximum length of job names
*     QSH_INTERACTIVE_POLLING_MIN - first polling interval between two polls
*                                   The actual polling interval will be calculated
*                                   by doubling the interval after each poll until
*                                   QSH_POLLING_MAX is reached.
*     QSH_POLLING_QRSH_MIN        - first polling interval in case of qrsh, qlogin
*     QSH_POLLING_MAX             - final polling interval between two polls
*     QSH_SOCKET_FINAL_TIMEOUT    - timeout in second (and final) wait for
*                                   qlogin_starter to connect to our socket
*     QRSH_CLIENT_CACHE           - name of file to cache client name for use
*                                   in qrsh - qrexec mode
*
****************************************************************************
*
*/
#define MAX_JOB_NAME 128
#define QSH_INTERACTIVE_POLLING_MIN 3
#define QSH_BATCH_POLLING_MIN 32
#define QSH_POLLING_MAX 256
#define QSH_SOCKET_FINAL_TIMEOUT 60

static void forward_signal(int sig)
{
   DENTER(TOP_LAYER, "forward_signal");
   if (child_pid > 0) {
      DPRINTF(("forwarding signal %d to child %d\n", sig, child_pid));
      kill(child_pid, sig);
   }
   DEXIT;
}

/****** Interactive/qsh/open_qrsh_socket() ***************************************
*
*  NAME
*     open_qrsh_socket -- create socket for messages to qrsh
*
*  SYNOPSIS
*     static int open_qrsh_socket(int *port)
*
*  FUNCTION
*     Creates a socket (server side) on any free port (assiged by the
*     operating system), allows access of one client per time.
*     If a serious error occurs, the program will exit.
*
*  INPUTS
*     port - reference to port number
*
*  RESULT
*     function returns the socket file descriptor, 
*     or 0, if an error occured
*     [port] - the port number of the socket is returned in the 
*              reference parameter
*
****************************************************************************
*
*/
static int open_qrsh_socket(int *port) {
   int sock;
   struct sockaddr_in server;
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HP1164)
   int length;
#else
   socklen_t length;
#endif
 
   DENTER(TOP_LAYER, "open_qrsh_socket");

   /* create socket */
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock == -1) {
      ERROR((SGE_EVENT, MSG_QSH_ERROROPENINGSTREAMSOCKET_S, strerror(errno)));
      sge_prof_cleanup();
      DEXIT;
      SGE_EXIT(NULL, 1);
   }

   /* bind socket using wildcards */
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = 0;
   if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1) {
      ERROR((SGE_EVENT, MSG_QSH_ERRORBINDINGSTREAMSOCKET_S, strerror(errno)));
      sge_prof_cleanup();
      DEXIT;
      SGE_EXIT(NULL, 1);
   }
   
   /* find out assigned port number and pass it to caller */
   length = sizeof server;
   if (getsockname(sock, (struct sockaddr *)&server, &length) == -1) {
      ERROR((SGE_EVENT, MSG_QSH_ERRORGETTINGSOCKETNAME_S, strerror(errno)));
      sge_prof_cleanup();
      DEXIT;
      SGE_EXIT(NULL, 1);
   }
   
   *port = ntohs(server.sin_port);
   DPRINTF(("qrsh will listen on port %d\n", *port));

   if (listen(sock, 1) == -1) {
      ERROR((SGE_EVENT, MSG_QSH_ERRORLISTENINGONSOCKETCONNECTION_S, strerror(errno)));
      DEXIT;
      return 0;
   }
   
   DEXIT;
   return sock;
}   

/****** Interactive/qsh/wait_for_qrsh_socket() ***************************************
*
*  NAME
*     wait_for_qrsh_socket -- wait for client connecting to socket
*
*  SYNOPSIS
*     static int wait_for_qrsh_socket(int sock, int timeout)
*
*  FUNCTION
*     Waits for a client to connect to the socket passed as parameter sock.
*     A timeout (in seconds) can be specified.
*
*  INPUTS
*     sock    - socket, that has been correctly initialized
*     timeout - timeout in seconds, if 0, wait forever
*
*  RESULT
*     the filedescriptor for the accepted client connection
*     -1, if an error occurs or the timeout expires
*
*  SEE ALSO
*     Interactive/qsh/open_qrsh_socket()
*
****************************************************************************
*
*/
static int wait_for_qrsh_socket(int sock, int timeout)
{
   int msgsock = -1;
   fd_set ready;
   struct timeval to;

   DENTER(TOP_LAYER, "wait_for_qrsh_socket");

   /* wait for anyone connecting to socket, with timeout */
   FD_ZERO(&ready);
   FD_SET(sock, &ready);
   to.tv_sec  = timeout;
   to.tv_usec = 0;
   switch(select(sock + 1, &ready, (fd_set *)0, (fd_set *)0, &to)) {
      case -1:
         ERROR((SGE_EVENT, MSG_QSH_ERRORWAITINGONSOCKETFORCLIENTTOCONNECT_S, strerror(errno)));
         break;
      case 0:
         VERBOSE_LOG((stderr, "timeout (%d s) expired while waiting on socket fd %d\n", timeout, sock));
         break;
      default: 
         if (FD_ISSET(sock, &ready)) {
            /* start accepting connections */
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HP1164)
            msgsock = accept(sock,(struct sockaddr *) 0,(int *) NULL);
#else
            msgsock = accept(sock,(struct sockaddr *) 0,(socklen_t *) NULL);
#endif

            if (msgsock == -1) {
               ERROR((SGE_EVENT, MSG_QSH_ERRORINACCEPTONSOCKET_S, strerror(errno)));
            }
            DPRINTF(("accepted client connection, fd = %d\n", msgsock));
         }
         break;
   } 
   DEXIT;

   return msgsock;
}               

/****** Interactive/qsh/read_from_qrsh_socket() ***************************************
*
*  NAME
*     read_from_qrsh_socket -- read a message from socket
*
*  SYNOPSIS
*     static char *read_from_qrsh_socket(int msgsock)
*
*  FUNCTION
*     Tries to read characters from the socket msgsock until a null byte
*     is received.
*
*  INPUTS
*     msgsock - file/socket descriptor to read from
*
*  RESULT
*     the read message,
*     NULL, if an error occured
*
*  NOTES
*     The result points to a static buffer - in subsequent calls of 
*     read_from_qrsh_socket, this buffer will be overwritten.
*
****************************************************************************
*
*/
static char *read_from_qrsh_socket(int msgsock)
{
   int rval = 0;
   static char buffer[1024];
   char *c  = buffer;
  
   DENTER(TOP_LAYER, "read_from_qrsh_socket");

   memset(buffer, 0, sizeof(buffer));
   do {
      *c = 0;

      if ((rval = read(msgsock, c, 1)) == -1) {
         ERROR((SGE_EVENT, MSG_QSH_ERRORREADINGSTREAMMESSAGE_S, strerror(errno)));
         DEXIT;
         return NULL;
      } else {
         if (rval == 0) {
            ERROR((SGE_EVENT, SFNMAX, MSG_QSH_ERRORENDINGCONNECTION));
            DEXIT;
            return NULL; 
         } 
      }
   } while (*c++ != 0); 

   close(msgsock);
   DEXIT;
   return buffer;
}

/****** Interactive/qsh/get_remote_exit() ***************************************
*
*  NAME
*     get_remote_exit -- read returncode from socket
*
*  SYNOPSIS
*     static int get_remote_exit(int sock);
*
*  FUNCTION
*     Reads the returncode of the client program from a socket connection 
*     to qlogin_starter.
*     The message read from the socket connection may contain an additional
*     text separated from the return code by a colon.
*     This text is interpreted as an error message and written to stderr.
*
*     Will wait for some time period (QSH_SOCKET_FINAL_TIMEOUT) for 
*     qlogin_starter to connect.
*
*  INPUTS
*     sock - socket file handle
*
*  RESULT
*     the returncode of the client program,
*     -1, if the read failed
*
*  SEE ALSO
*     Interactive/qsh/-Defines
*     Interactive/qsh/wait_for_qrsh_socket()
*     Interactive/qsh/read_from_qrsh_socket()
*
****************************************************************************
*
*/
static int get_remote_exit_code(int sock)                     
{
   int msgsock;

   DENTER(TOP_LAYER, "get_remote_exit_code");
   
   VERBOSE_LOG((stderr, SFNMAX, MSG_QSH_READINGEXITCODEFROMSHEPHERD));

   msgsock = wait_for_qrsh_socket(sock, QSH_SOCKET_FINAL_TIMEOUT);
   if (msgsock != -1) {
      char *s_ret = read_from_qrsh_socket(msgsock);
      if (s_ret && *s_ret) {
         char *message = strchr(s_ret, ':');
         if (message != NULL && strlen(message) > 0) {
            fprintf(stderr, "%s\n", message + 1);
            *message = 0;
         }
         VERBOSE_LOG((stderr, "%s\n", s_ret));
         DEXIT;
         return atoi(s_ret);
      }
   }

   ERROR((SGE_EVENT, SFNMAX, MSG_QSH_ERRORREADINGRETURNCODEOFREMOTECOMMAND));
   DEXIT;
   return -1;
}

/****** Interactive/qsh/quote_argument() ***************************************
*
*  NAME
*     quote_argument -- quote an argument
*
*  SYNOPSIS
*     static const char *quote_argument(const char *arg)
*
*  FUNCTION
*     Encloses the contents of string arg into  quotes ('').
*     Memory for the new string is allocated using malloc. 
*     It is in the responsibility of the user to free this memory.
*
*  INPUTS
*     arg - the argument string to quote
*
*  RESULT
*     the quoted string
*
*  EXAMPLE
*     const char *quoted = quote_argument("test");
*     printf("%s\n", quoted);
*     sge_free(&quoted);
*
*  NOTES
*     Function might be moved to some library, same code is used in other 
*     modules.
*
****************************************************************************
*
*/

static const char *quote_argument(const char *arg) {
   char *new_arg = NULL;

   DENTER(TOP_LAYER, "quote_argument");
   
   if (arg == NULL) {
      return arg;
   }   

   new_arg = malloc(strlen(arg) + 3);

   if (new_arg == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_QSH_MALLOCFAILED));
      DEXIT;
      return NULL;
   }
 
   sprintf(new_arg, "'%s'", arg);
   DEXIT;
   return new_arg;
}

/****** Interactive/qsh/parse_result_list() ***************************************
*
*  NAME
*     parse_result_list -- parse result list from sge function calls
*
*  SYNOPSIS
*     static int parse_result_list(lList *alp, int *alp_error);
*
*  FUNCTION
*     Parses through the list and outputs the errors and warnings contained
*     as list elements.
*     Returns info whether an error was contained or not in alp_error
*     and if qsh must exit in returncode.
*
*  INPUTS
*     alp - pointer to result list
*     [alp_error] - reference to return error status
*
*  RESULT
*     0, if no error was contained in list
*     1, if an error was contained in list
*     alp_error - 1, if alp contains an error condition, else 0
*
****************************************************************************
*
*/
static int parse_result_list(lList *alp, int *alp_error)
{
   lListElem *aep;
   int do_exit = 0;

   DENTER(TOP_LAYER, "parse_result_list");

   *alp_error = 0;

   for_each(aep, alp) {
      u_long32 status  = lGetUlong(aep, AN_status);
      u_long32 quality = lGetUlong(aep, AN_quality);
      if (quality == ANSWER_QUALITY_ERROR) {
         ERROR((SGE_EVENT, "%s", lGetString(aep, AN_text)));
         *alp_error = 1;

         switch(status) {
            case STATUS_EDENIED2HOST:
            case STATUS_ENOSUCHUSER:
            case STATUS_NOSUCHGROUP:
            case STATUS_DENIED:
            case STATUS_EVERSION:
               do_exit = 1;
            default:
               break;
         }
      } else if (quality == ANSWER_QUALITY_WARNING) {
         WARNING((SGE_EVENT, SFN SFN, MSG_WARNING, lGetString(aep, AN_text)));
      }
   }

   DEXIT;
   return do_exit;
}


/****** Interactive/qsh/start_client_program() ***************************************
*
*  NAME
*     start_client_program -- start a telnet/rsh/rlogin/... client
*
*  SYNOPSIS
*     static void start_client_program(const char *client_name,
*                                      const char *host,
*                                      const char *port,
*                                      const char *job_dir,
*                                      const char *utilbin_dir,
*                                      int is_rsh,
*                                      int is_rlogin
*                                      int nostdin,
*                                      int noshell,
*                                      int sock)
*
*  FUNCTION
*     Builds the argument vector for the start of the client program,
*     creates a process (fork) and starts the client program in the 
*     child process.
*     After termination of the child, read exit code of remote process
*     from qlogin_starter.
*
*  INPUTS
*     client_name - name of the program to start
*     host        - host on which to start command
*     port        - port on which to contact server process (telnetd etc.)
*     job_dir     - job directory
*     utilbin_dir - path of the utilbin directory on the remote host
*     is_rsh      - do we call a rsh (or similar) program?
*     is_rlogin   - do we call a rlogin (or similar) program?
*     nostdin     - shall stdin be suppressed (parameter -n to rsh)?
*     noshell     - shall command be executed without wrapping login shell
*     sock        - socket descriptor of connection to qlogin_starter
*
*  RESULT
*     returncode of the executed command or
*     EXIT_FAILURE, if exec failed
*
****************************************************************************
*
*/
static int start_client_program(const char *client_name,
                                const char *host,
                                const char *port,
                                const char *job_dir,
                                const char *utilbin_dir,
                                int is_rsh,
                                int is_rlogin,
                                int nostdin,
                                int noshell,
                                int sock)
{
   DENTER(TOP_LAYER, "start_client_program");

   child_pid = fork();

   if (child_pid == -1) {
      ERROR((SGE_EVENT, MSG_QSH_CANNOTFORKPROCESS_S, strerror(errno))); 
      DEXIT;
      return 1;
   }

   if (child_pid) {
      bool done;
      struct sigaction forward_action;

      forward_action.sa_handler = forward_signal;
      sigemptyset(&forward_action.sa_mask);
      forward_action.sa_flags=0;

      sigaction(SIGINT, &forward_action, NULL);
      sigaction(SIGQUIT, &forward_action, NULL);
      sigaction(SIGTERM, &forward_action, NULL);
      sge_unblock_all_signals();
  
      done = false;
      while (!done) {
         int status;

         /* Always try to get exit code (or error) from shepherd.
          * If rsh didn't return EXIT_SUCCESS or didn't exit (!WIFEXITED), 
          * output a message "cleaning up ..." and delete the job
          */
         if (waitpid(child_pid, &status, 0) == child_pid) {
            int ret = -1;  /* default: delete job */

            if (WIFEXITED(status)) {
               ret = WEXITSTATUS(status);
               VERBOSE_LOG((stderr, MSG_QSH_EXITEDWITHCODE_SI, client_name, ret));
               VERBOSE_LOG((stderr, "\n"));
            }

            if (WIFSIGNALED(status)) {
               int code = WTERMSIG(status);
               VERBOSE_LOG((stderr, MSG_QSH_EXITEDONSIGNAL_SIS, client_name, 
                           code, sge_sys_sig2str(code)));
               VERBOSE_LOG((stderr, "\n"));
               /* if not qrsh <command>: use default: delete job */
            }

            /* get exit code from shepherd for qrsh <command> */
            if (is_rsh) {
               ret = get_remote_exit_code(sock);
            }

            DEXIT;
            return ret;
         }
      }
   } else {
      sge_sl_list_t *sl_args;
      char          shellpath[SGE_PATH_MAX];
      char          **args;
      char          *command = strdup(client_name); /* needn't be freed, as we exec */
      int           ret;

      /* create an argument list */
      if (!sge_sl_create(&sl_args)) {
         ERROR((SGE_EVENT, SFNMAX, MSG_SGETEXT_NOMEM));
         exit(EXIT_FAILURE);
      }

      /* split command command line into single arguments in argument list */
      ret = parse_quoted_command_line(command, sl_args);
      if (ret != 0) {
         ERROR((SGE_EVENT, MSG_QSH_UNMATCHED_C, ret==1 ? '\"' : '\''));
         exit(EXIT_FAILURE);
      }

      /* add additional arguments to argument list */
      if (is_rsh || is_rlogin) {
         sge_set_def_sig_mask(NULL, NULL);
         sge_unblock_all_signals();
         
         if (is_rsh && nostdin) {
            sge_sl_insert(sl_args, "-n", SGE_SL_BACKWARD);
         }   

         sge_sl_insert(sl_args, "-p", SGE_SL_BACKWARD);
         sge_sl_insert(sl_args, (void*)port, SGE_SL_BACKWARD);
         sge_sl_insert(sl_args, (void*)host, SGE_SL_BACKWARD);
         if (is_rsh) {
            sprintf(shellpath, "%s/qrsh_starter", utilbin_dir);
            sge_sl_insert(sl_args, "exec", SGE_SL_BACKWARD);
            sge_sl_insert(sl_args, (void*)quote_argument(shellpath), SGE_SL_BACKWARD);
            sge_sl_insert(sl_args, (void*)quote_argument(job_dir), SGE_SL_BACKWARD);
            if (noshell) {
               sge_sl_insert(sl_args, "noshell", SGE_SL_BACKWARD);
            }   
         }   
      } else {
         sge_sl_insert(sl_args, (void*)host, SGE_SL_BACKWARD);
         sge_sl_insert(sl_args, (void*)port, SGE_SL_BACKWARD);
      }

      /* convert argument list to argument vector */
      convert_arg_list_to_vector(sl_args, &args);

      /* destroy the argument list, keep the argument buffers */
      sge_sl_destroy(&sl_args, NULL);
   
#if 0
      /* just for debugging purposes */
      {
         int i;
         fflush(stdout); fflush(stderr);
         for(i = 0; args[i] != NULL; i++) {
            printf("qsh: args[%d] = %s\n", i, args[i]);
         }  
         fflush(stdout); fflush(stderr);
      }
#endif

      execvp(args[0], args);
      ERROR((SGE_EVENT, MSG_EXEC_CANTEXECXYZ_SS, args[0], strerror(errno)));
      DEXIT;
      exit(EXIT_FAILURE);
   }

   /* should never be reached */
   return -1;
}

/****** Interactive/qsh/get_client_server_context() ***************************************
*
*  NAME
*     get_client_server_context -- get parameters for client/server connection
*
*  SYNOPSIS
*     static int get_client_server_context(int msgsock, 
*                                          char **port, 
*                                          char **job_dir,
*                                          char **utilbin_dir,
*                                          const char **host);
*
*  FUNCTION
*     Tries to read the parameters port and job_dir from the socket connection
*     to qlogin_starter (msgsock).
*     They are sent from qlogin_starter in the format
*     0:<port>:<utilbin_dir>:<job_dir>
*    
*     The communication partner may also report an error, using the format
*     <error>:<message> where error is a number != 0
*     In this case, the error code and message will be written to stderr.
*
*  INPUTS
*     msgsock     - socket descriptor of connection to qlogin_starter
*     port        - reference to port number, see RESULT
*     job_dir     - reference to job_dir, see RESULT
*     utilbin_dir - reference to utilbin_dir, see RESULT
*     host        - reference to host, see RESULT
*
*  RESULT
*     1, if everything ok,
*     0, if the required parameters are not found
*
*     [port]        - used to return the port number to caller
*     [job_dir]     - used to return the job directory to caller
*     [utilbin_dir] - used to return the utilbin directory on the
*                     remote host
*     [host]        - used to return the host, to which to connect
*                     with rsh/rlogin/telnet call
*
*  NOTES
*     port and data point to a static buffer in function
*     read_from_qrsh_socket(). Subsequent calls to this function
*     will overwrite this buffer.
*
*  SEE ALSO
*     Interactive/qsh/read_from_qrsh_socket()
*
****************************************************************************
*
*/
static int get_client_server_context(int msgsock, char **port, char **job_dir, char **utilbin_dir, const char **host)
{
   char *s_code = NULL;
   char *data   = NULL;
   
   DENTER(TOP_LAYER, "get_client_server_context");
   
   data = read_from_qrsh_socket(msgsock);

   if (data == NULL || *data == 0) {
      ERROR((SGE_EVENT, MSG_QSH_ERRORREADINGCONTEXTFROMQLOGIN_STARTER_S,"qlogin_starter"));
      DEXIT;
      return 0;
   }

   DPRINTF(("qlogin_starter sent: %s\n", data));
  
   s_code = strtok(data, ":");

   if (s_code == NULL) {
      ERROR((SGE_EVENT, MSG_QSH_ERRORREADINGCONTEXTFROMQLOGIN_STARTER_S,"qlogin_starter"));
      DEXIT;
      return 0;
   }
   
   if (strcmp(s_code, "0") == 0) {  /* qlogin_starter reports valid data */
      if ((*port = strtok(NULL, ":")) != NULL) {
         if ((*utilbin_dir = strtok(NULL, ":")) != NULL) {
            if ((*job_dir = strtok(NULL, ":")) != NULL) {
               if ((*host = strtok(NULL, ":")) != NULL) {
                  return 1;
               }
            }
         }
      }
      ERROR((SGE_EVENT, MSG_QSH_ERRORREADINGCONTEXTFROMQLOGIN_STARTER_S,"qlogin_starter"));
      DEXIT;
      return 0;
   } else {                        /* qlogin_starter reports error */
      char *message = strtok(NULL, "\n");
      ERROR((SGE_EVENT, "%s: %s\n", s_code, message == NULL ? "" : message));
   }

   DEXIT;
   return 0;
}

/****** Interactive/qsh/get_client_name() ***************************************
*
*  NAME
*     get_client_name -- get path and name of client program to start
*
*  SYNOPSIS
*     static const char *
*     get_client_name(int is_rsh, int is_rlogin, int inherit_job);
*
*  FUNCTION
*     Determines path and name of the client program to start 
*     (telnet, rsh, rlogin etc.).
*     Takes into consideration the cluster and host configuration.
*     If no value is set for the client program (value none), use
*     telnet in qlogin mode (try to find in path) or
*     $SGE_ROOT/utilbin/$ARCH/[rsh|rlogin] in rsh/rlogin mode.
*
*     In case of qrexec (inherit_job), try to read client name
*     from environment variable SGE_RSH_COMMAND, which is set by
*     sge_execd in the job environment.
*  INPUTS
*     is_rsh      - are we treating a qrsh call
*     is_rlogin   - are we treating a qrsh call without commands 
*                   (-> rlogin)
*     inherit_job - we have inherited an existing environment
*
*  RESULT
*     path and name of client program
*     NULL, if an error occures
*
*  NOTE
*     It is in the responsibility of the caller to free the client name!
*
*  MT-NOTE
*     get_client_name is thread safe 
****************************************************************************
*
*/
static const char *
get_client_name(sge_gdi_ctx_class_t *ctx, int is_rsh, int is_rlogin, int inherit_job)
{
   /* this is what we return */
   const char *client_name  = NULL;

   /* config lists and entries to retrieve client_name from qmaster */
   lList     *conf_list       = NULL;
   lListElem *global          = NULL; 
   lListElem *local           = NULL;
   lListElem *qlogin_cmd_elem = NULL;

   /* session type and config entry name */
   const char *session_type;
   const char *config_name;

   u_long32 progid = ctx->get_who(ctx);
   const char *qualified_hostname = ctx->get_qualified_hostname(ctx);
   const char *cell_root = ctx->get_cell_root(ctx);
   const char *sge_root = ctx->get_sge_root(ctx);

   DENTER(TOP_LAYER, "get_client_name");

   /* 
    * In case of qrsh -inherit, try to read client_command from
    * an environment variable set by sge_execd
    *
    * In case of new IJS, the environment variable contains "builtin" if the
    * rsh_command, rlogin_command or qlogin_command is builtin. So we can use
    * it like before.
    */
   if (inherit_job) {
      client_name = getenv("SGE_RSH_COMMAND");
      if (client_name != NULL && strlen(client_name) > 0) {
         DPRINTF(("rsh client name: %s\n", client_name));
         if (strcasecmp(client_name, "builtin") == 0) {
            g_new_interactive_job_support = true;
         }
         DRETURN(strdup(client_name));
      }
   }
 
   /* set session type and the corresponding config entry name */
   if (is_rsh) { 
      session_type = "rsh"; 
      config_name  = "rsh_command";
   } else if (is_rlogin) { 
      session_type = "rlogin"; 
      config_name  = "rlogin_command";
   } else {
      session_type = "telnet"; 
      config_name  = "qlogin_command";
   }
  
   /* get configuration from qmaster */
   if (gdi2_get_configuration(ctx, qualified_hostname, &global, &local) ||
      merge_configuration(NULL, progid, cell_root, global, local, &conf_list)) {
      ERROR((SGE_EVENT, SFNMAX, MSG_CONFIG_CANTGETCONFIGURATIONFROMQMASTER));
      lFreeList(&conf_list);
      lFreeElem(&global);
      lFreeElem(&local);
      DEXIT;
      return NULL;
   }

   /* search for config entry */
   qlogin_cmd_elem = lGetElemStr(conf_list, CF_name, config_name);
   if (qlogin_cmd_elem == NULL) {
      ERROR((SGE_EVENT, SFNMAX, MSG_CONFIG_CANTGETCONFIGURATIONFROMQMASTER));
      lFreeList(&conf_list);
      lFreeElem(&global);
      lFreeElem(&local);
      DEXIT;
      return NULL;
   }

   /* use config entry, if found */
   client_name = lGetString(qlogin_cmd_elem, CF_value);

   /* 
    * If no command is configured, or none is configured as command,
    * use a default: The rsh/rlogin distributed with SGE or telnet.
    */
   if (client_name == NULL || sge_strnullcasecmp(client_name, "none") == 0) {
      if (is_rsh || is_rlogin) {
         char default_buffer[SGE_PATH_MAX];
         dstring default_dstring;

         sge_dstring_init(&default_dstring, default_buffer, SGE_PATH_MAX);
         sge_dstring_sprintf(&default_dstring, "%s/utilbin/%s/%s", 
                             sge_root, sge_get_arch(), 
                             session_type);
         client_name = strdup(sge_dstring_get_string(&default_dstring));
      } else {
         /* try to find telnet in PATH */
         client_name = strdup(session_type);
      }
   } else {
      client_name = strdup(client_name);
   }

   if (strcasecmp(client_name, "builtin") == 0) {
      g_new_interactive_job_support = true;
   }

   lFreeList(&conf_list);
   lFreeElem(&global);
   lFreeElem(&local);

   return client_name;
}

/****** Interactive/qsh/set_job_info() ****************************************
*
*  NAME
*     set_job_info -- info about interactive job for execd/shepherd
*
*  SYNOPSIS
*     static void set_job_info(lListElem *job, const char *name,
*                              int is_qlogin, int is_rsh, 
*                              int is_rlogin);
*
*  FUNCTION
*     Sets the flag JB_type and the jobname JB_job_name to reasonable
*     values depending on the startmode of the qsh binary:
*
*  INPUTS
*     job       - job data structure
*     name      - name of the program to start - if called with path, 
*                 the path is stripped
*     is_qlogin - are we treating a qlogin call
*     is_rsh    - are we treating a qrsh call
*     is_rlogin - are we treating a qrsh call without commands (-> rlogin)
*******************************************************************************/
static void set_job_info(lListElem *job, const char *name, int is_qlogin, 
                         int is_rsh, int is_rlogin)
{
   lList *stdout_stderr_path = NULL;
   u_long32 jb_now = lGetUlong(job, JB_type);
   const char *job_name  = lGetString(job, JB_job_name);
   
   if (is_rlogin) {
      JOB_TYPE_SET_QRLOGIN(jb_now);
      if (job_name == NULL) {
         job_name = JOB_TYPE_STR_QRLOGIN;
      } 
   } else {   
      if (is_rsh) {
         JOB_TYPE_SET_QRSH(jb_now);
         if (job_name == NULL) {
           job_name = name;
         } 
      } else {
         if (is_qlogin && !is_rsh) {
            JOB_TYPE_SET_QLOGIN(jb_now);
            if (job_name == NULL) {
               job_name = JOB_TYPE_STR_QLOGIN;
            }   
         } else {
            JOB_TYPE_SET_QSH(jb_now);
            if (job_name == NULL) {
               job_name = JOB_TYPE_STR_QSH;
            }   
         } 
      }   
   }

   lSetUlong(job, JB_type, jb_now);
   lSetString(job, JB_job_name, job_name);

   cull_parse_path_list(&stdout_stderr_path, "/dev/null");
   lSetList(job, JB_stdout_path_list, lCopyList("stdout_path_list", stdout_stderr_path));
   lSetList(job, JB_stderr_path_list, lCopyList("stderr_path_list", stdout_stderr_path));
   lFreeList(&stdout_stderr_path);
}

/****** Interactive/qsh/set_command_to_env() ***************************************
*
*  NAME
*     set_command_to_env() -- set command to execute in environment variable
*
*  SYNOPSIS
*     static void set_command_to_env(lList *envlp, lList *opts_qrsh);
*
*  FUNCTION
*     Creates an environment variable QRSH_COMMAND in the environment list
*     <envlp>, that contains the commandline passed in the argument list
*     <opts_qrsh>.
*     Arguments are separated by the character code 0xff.
*
*  INPUTS
*     envlp     - environment list
*     opts_qrsh - list of commandline arguments
*
*  BUGS
*     In general, passing the commandline via environment variable is only
*     a workaround for the problems occuring when passing the commandline
*     via rsh command (quoting, backquotes etc. through several shells).
*
*     Better would be to pass the commandline over a save (not modifying) 
*     mechanism, e.g. passing each argument as a line over file (not possible)
*     or a socket connection.
*
****************************************************************************
*
*/
static void set_command_to_env(lList *envlp, lList *opts_qrsh)
{
   dstring buffer = DSTRING_INIT;

   if (opts_qrsh) {
      lListElem *ep;
     
      ep = lFirst(opts_qrsh);
      if (ep) {
         char delimiter[2];
         const char *help = NULL;
         sprintf(delimiter, "%c", 0xff);
         help = lGetString(ep, SPA_argval_lStringT);
         if (help != NULL) {
            sge_dstring_copy_string(&buffer, help); 
         } 
         while((ep = lNext(ep)) != NULL) {
            const char *arg = lGetString(ep, SPA_argval_lStringT);
            sge_dstring_append(&buffer, delimiter);
            if (arg != NULL) {
               sge_dstring_append(&buffer, arg);
            } 
         }   
      }
   } 

#if 0
   fflush(stdout); fflush(stderr);
   fprintf(stderr, "qsh: QRSH_COMMAND = %s\n", buffer);
   fflush(stdout); fflush(stderr);
#endif

   var_list_set_string(&envlp, "QRSH_COMMAND", sge_dstring_get_string(&buffer));
   sge_dstring_free(&buffer);
}

/****** qsh/set_builtin_ijs_signals_and_handlers() *****************************
*  NAME
*     set_builtin_ijs_signals_and_handlers() -- sets signal mask and handlers
*                                               for the builtin interactive
*                                               job support
*
*  SYNOPSIS
*     static void set_builtin_ijs_signals_and_handlers() 
*
*  FUNCTION
*     In the builtin interactive job support, qsh/qrsh/qlogin need to catch
*     specific signals. The mask and handlers for this signals must be set
*     before the first thread (which is a commlib thread) is created, so that
*     every thread inherits the mask and handlers.
*
*  RESULT
*     static void - no result
*
*  NOTES
*     MT-NOTE: set_builtin_ijs_signals_and_handlers() is not MT safe 
*******************************************************************************/
static void set_builtin_ijs_signals_and_handlers()
{
   sge_set_def_sig_mask(NULL, NULL);
   sge_unblock_all_signals();
   set_signal_handlers();
}

/****** qsh/write_builtin_ijs_connection_data_to_job_object() ******************
*  NAME
*     write_builtin_ijs_connection_data_to_job_object() -- writes the connection
*                                                         data to the job object
*
*  SYNOPSIS
*     static void write_builtin_ijs_connection_data_to_job_object(const char* 
*     qualified_hostname, lListElem *job, lList *opts_qrsh) 
*
*  FUNCTION
*     Writes the data necessary to connect to the commlib server that was
*     started before this function gets called to the job object.
*
*  INPUTS
*     const char* qualified_hostname - The qualified hostname of this host.
*                                      Returned by sge_gethostbyname().
*     int port                       - The TCP/SSL/??? port this server is
*                                      listening on for new connections
*     lListElem *job                 - The job object where the connection data
*                                      gets written to.
*     lList *opts_qrsh               - The command line of this command. We need
*                                      the name of the binary from the command
*                                      line, the shepherd must know which kind
*                                      of interactive job this is.
*
*  OUTPUTS
*     lListElem *job                 - The job object where the connection data
*                                      gets written to.
*
*  RESULT
*     static void -  no result
*
*  NOTES
*     MT-NOTE: write_builtin_ijs_connection_data_to_job_object() is not MT safe 
*******************************************************************************/
static void write_builtin_ijs_connection_data_to_job_object(
const char* qualified_hostname,
int port,
lListElem *job,
lList *opts_qrsh)
{
   dstring connection_params = DSTRING_INIT;
   lList   *envlp            = NULL;

   sge_dstring_sprintf(&connection_params, "%s:%d",
                       qualified_hostname, port); 

   /*
    * Get environment from job object. If there is no environment yet,
    * create one.
    */
   envlp = lGetList(job, JB_env_list);
   if (envlp == NULL) {
      envlp = lCreateList("environment list", VA_Type);
      lSetList(job, JB_env_list, envlp);
   }   

   /* TODO: Instead of using env, use job field to transfer infos
    *       JB_qrsh_port field.
    * HP: Do this in the next minor release, not in a patch update.
    */
   var_list_set_string(&envlp, "QRSH_PORT", 
                       sge_dstring_get_string(&connection_params));

   /* TODO: write the command in JB_job_args field
    * HP: Do this in the next minor release, not in a patch update.
    */
   set_command_to_env(envlp, opts_qrsh);
   sge_dstring_free(&connection_params);
}

/****** qsh/block_notification_signals() ***************************************
*  NAME
*     block_notification_signals() -- block signals used by -notify
*
*  SYNOPSIS
*     void block_notification_signals(void)
*
*  FUNCTION
*     Makes sure that signals used for job notification are blocked
*     for qrsh -inherit.
*  
*     Background: A job that is submitted with the -notify option
*     will get a notification signal some time before certain actions
*     are performed on the job, e.g. it gets notified with a SIGUSR2
*     when it shall be deleted by qdel, before it actually gets killed
*     with a SIGKILL. See submit.1 for more details.
*
*     Problem: The master task (job script) of a job being submitted
*     with -notify will handle the notification signal.
*     But the notification signal is sent to the whole process group
*     of the job, meaning to all qrsh -inherit processes as well.
*     But the qrsh -inherit may not exit on receiving the signal.
*     Therefore they have to block it.
*
*     The default notification signals are SIGUSR1 and SIGUSR2.
*     They can be redefined by the execd_params NOTIFY_SUSP and NOTIFY_KILL.
*     In this case, execd sets environment variables SGE_NOTIFY_SUSP_SIGNAL
*     and SGE_NOTIFY_KILL_SIGNAL.
*
*  NOTES
*     MT-NOTE: block_notification_signals() is MT safe 
*******************************************************************************/
void block_notification_signals(void)
{
   /* default notification signals */
   int sig1 = SIGUSR1;
   int sig2 = SIGUSR2;

   const char *signal_name;

   DENTER(TOP_LAYER, "block_notification_signals");

   /* check if they have been redefined */
   signal_name = getenv("SGE_NOTIFY_SUSP_SIGNAL"); 
   if (signal_name != NULL) {
      sig1 = sge_sys_str2signal(signal_name);
   }
   signal_name = getenv("SGE_NOTIFY_KILL_SIGNAL"); 
   if (signal_name != NULL) {
      sig2 = sge_sys_str2signal(signal_name);
   }

   if (sig1 > 0) {
      sigignore(sig1);
      DPRINTF(("ignoring signal %d\n", (int)sig1));
   }
   if (sig2 > 0) {
      sigignore(sig2);
      DPRINTF(("ignoring signal %d\n", (int)sig2));
   }

   DRETURN_VOID;
}

int main(int argc, char **argv) 
{
   u_long32 my_who = QSH; 
   lList *opts_cmdline = NULL;
   lList *opts_defaults = NULL;
   lList *opts_scriptfile = NULL;
   lList *opts_all = NULL;
   lList *opts_qrsh = NULL;
   lListElem *job = NULL;
   lList *lp_jobs = NULL;
   lList *alp = NULL;
   lList *answer = NULL;

   lListElem *aep = NULL;
   u_long32 status = STATUS_OK;
   u_long32 quality;
   u_long32 job_id = 0;
   int do_exit = 0;
   int exit_status = 0;
   int is_qlogin = 0;
   int is_rsh = 0;
   int is_rlogin = 0;
   int is_qsh = 0;
   int just_verify = 0;
   int inherit_job  = 0;
   int existing_job = 0;
   int nostdin = 0;
   int noshell = 0;
   ternary_t pty_option = UNSET;
   ternary_t suspend_remote_option = UNSET;

   const char *host = NULL;
   char name[MAX_JOB_NAME + 1];
   const char *client_name = NULL;
   char *port = NULL;
   char *job_dir = NULL;
   char *utilbin_dir = NULL;
   bool csp_mode = false;

   int alp_error;

   lListElem *ep = NULL;

   int sock = 0;

   const char* progname = NULL;
   const char* unqualified_hostname = NULL;
   const char* qualified_hostname = NULL;
   const char* sge_root = NULL;
   const char* cell_root = NULL;
   u_long32 myuid = 0;
   const char* username = NULL;
   const char* mastername = NULL;
   COMM_HANDLE *comm_handle = NULL;

   sge_gdi_ctx_class_t *ctx = NULL;
   sge_bootstrap_state_class_t *bootstrap_state = NULL;

   DENTER_MAIN(TOP_LAYER, "qsh");

   /*
   ** get command name: qlogin, qrsh or qsh
   */
   if (!strcmp(sge_basename(argv[0], '/'), "qlogin")) {
      is_qlogin = 1;
      my_who = QLOGIN;
   } else if (!strcmp(sge_basename(argv[0], '/'), "qrsh")) {
      is_qlogin = 1;
      is_rsh    = 1;
      my_who = QRSH;
   } else if (!strcmp(sge_basename(argv[0], '/'), "qsh")) {
      is_qsh = 1; 
      my_who = QSH;
   }

   log_state_set_log_gui(1);
   sge_setup_sig_handlers(my_who);

   if (sge_gdi2_setup(&ctx, my_who, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   progname = ctx->get_progname(ctx);
   unqualified_hostname = ctx->get_unqualified_hostname(ctx);
   qualified_hostname = ctx->get_qualified_hostname(ctx);
   sge_root = ctx->get_sge_root(ctx);
   cell_root = ctx->get_cell_root(ctx);
   myuid = ctx->get_uid(ctx);
   username = ctx->get_username(ctx);
   mastername = ctx->get_master(ctx, false);

   bootstrap_state = ctx->get_sge_bootstrap_state(ctx);
   if (strcasecmp(bootstrap_state->get_security_mode(bootstrap_state), "csp") == 0) {
      csp_mode = true;
   }

   /*
   ** begin to work
   */

   /* 
   ** set verbosity default: qlogin and qsh: verbose to be backwards compatible,
   ** qrsh: quiet
   */
   if (is_rsh) {
      log_state_set_log_verbose(0); 
   } else {
      log_state_set_log_verbose(1);
   }

   /*
   ** read switches from the various defaults files
   */
   opt_list_append_opts_from_default_files(my_who, cell_root, username, &opts_defaults, &alp, environ);
   do_exit = parse_result_list(alp, &alp_error);
   lFreeList(&alp);

   if (alp_error) {
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 1);
   }

   /*
   ** append the commandline switches to the list
   */
   alp = cull_parse_cmdline(my_who, argv + 1, environ, &opts_cmdline, FLG_USE_PSEUDOS);
   do_exit = parse_result_list(alp, &alp_error);
   lFreeList(&alp);

   if (alp_error) {
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 1);
   }

   job = lCreateElem(JB_Type);
   
   if (job == NULL) {
      sprintf(SGE_EVENT, MSG_MEM_MEMORYALLOCFAILED_S, SGE_FUNC);
      answer_list_add(&answer, SGE_EVENT, 
                      STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      do_exit = parse_result_list(alp, &alp_error);
      lFreeList(&answer);
      
      if (alp_error) {
         sge_prof_cleanup();
         SGE_EXIT((void**)&ctx, 1);
      }
   }

   if (opt_list_has_X(opts_cmdline, "-help")) {
      sge_usage(my_who, stdout);
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 0);
   }

   /* set verbosity */
   while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-verbose"))) {
      lRemoveElem(opts_cmdline, &ep);
      log_state_set_log_verbose(1);
   }

   /* parse -noshell */
   while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-noshell"))) {
      u_long32 jb_type = lGetUlong(job, JB_type);

      JOB_TYPE_SET_NO_SHELL(jb_type);
      lSetUlong(job, JB_type, jb_type);
      lRemoveElem(opts_cmdline, &ep);
      noshell = 1;
   }

   /* parse -suspend_remote <yes|no> */
   if (opt_list_has_X(opts_cmdline, "-suspend_remote")) {
      suspend_remote_option = (ternary_t)opt_list_is_X_true(opts_cmdline, "-suspend_remote");
   }
   /* remove the suspend_remote option from commandline before proceeding */
   while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-suspend_remote"))) {
      lRemoveElem(opts_cmdline, &ep);
   }

   /* parse -pty <yes|no> */
   if (opt_list_has_X(opts_cmdline, "-pty")) {
      pty_option = (ternary_t)opt_list_is_X_true(opts_cmdline, "-pty");
   }
   lSetUlong(job, JB_pty, pty_option);
   /* remove the pty option from commandline before proceeding */
   while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-pty"))) {
      lRemoveElem(opts_cmdline, &ep);
   }

   /*
   ** if qrsh, parse command to call
   */
   if (is_rsh) {
      while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-nostdin"))) {
         lRemoveElem(opts_cmdline, &ep);
         nostdin = 1;
      }

      while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-inherit"))) {
         lRemoveElem(opts_cmdline, &ep);
         inherit_job = 1;
      }
     
      if (inherit_job) {
         char *s_existing_job = getenv("JOB_ID");
         if (s_existing_job != NULL) {
            if (sscanf(s_existing_job, "%d", &existing_job) != 1) {
               ERROR((SGE_EVENT, MSG_QSH_INVALIDJOB_ID_SS, "JOB_ID",
                      s_existing_job));
               sge_prof_cleanup();
               SGE_EXIT((void**)&ctx, 1);
            }
         } else {
            ERROR((SGE_EVENT, MSG_QSH_INHERITBUTJOB_IDNOTSET_SSS, "qrsh",
                   "-inherit","JOB_ID"));
            sge_prof_cleanup();
            SGE_EXIT((void**)&ctx, 1);
         }
      }   

      strcpy(name, "QRSH");
      host = NULL;

      opts_qrsh = lCreateList("opts_qrsh", lGetListDescr(opts_cmdline));

      if ((ep = lGetElemStr(opts_cmdline, SPA_switch, "script"))) {
         if (existing_job) {
            host = strdup(lGetString(ep, SPA_argval_lStringT));
            lRemoveElem(opts_cmdline, &ep);
            ep = lGetElemStr(opts_cmdline, SPA_switch, "jobarg");
         }

         if (ep != NULL) {
            const char *new_name = NULL;

            lSetString(job, JB_script_file, 
                       lGetString(ep, SPA_argval_lStringT));
            lDechainElem(opts_cmdline, ep);
            lAppendElem(opts_qrsh, ep);
           
            new_name = sge_jobname(lGetString(ep, SPA_argval_lStringT));
            if (new_name != NULL) {
               sge_strlcpy(name, new_name, MAX_JOB_NAME); 
            }

            while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "jobarg"))) {
               lDechainElem(opts_cmdline, ep);
               lAppendElem(opts_qrsh, ep);
            }   
         }
      }

      if (lGetNumberOfElem(opts_qrsh) <= 0) {
         if (inherit_job) {
            ERROR((SGE_EVENT, MSG_QSH_INHERITUSAGE_SS, "-inherit", 
                   "qrsh -inherit ... <host> <command> [args]"));
            sge_prof_cleanup();
            SGE_EXIT((void**)&ctx, 1);
         } else {
            is_rsh = 0;
            is_rlogin = 1;
         }
      }   
   }

   /* handle binary vs. script submission */
   if (is_rsh && !inherit_job) {
      bool binary = true;

      /* do we have -b in commandline? */
      if (opt_list_has_X(opts_cmdline, "-b")) {
         /* then commandline will determine over binary vs. script submiss. */
         binary = opt_list_is_X_true(opts_cmdline, "-b");
      } else if (opt_list_has_X(opts_defaults, "-b")) {
         /* we have -b in defaults files, this one will decide */
         binary = opt_list_is_X_true(opts_defaults, "-b");
      }

      /* remove the binary option from commandline before proceeding */
      while ((ep = lGetElemStr(opts_defaults, SPA_switch, "-b"))) {
         lRemoveElem(opts_defaults, &ep);
      }
      while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-b"))) {
         lRemoveElem(opts_cmdline, &ep);
      }

      /* set binary bit or handle script submission */
      if (binary) {
         u_long32 jb_type = lGetUlong(job, JB_type);

         JOB_TYPE_SET_BINARY(jb_type);
         lSetUlong(job, JB_type, jb_type);
      } else {
         DPRINTF(("handling script submission\n"));
     
         /* move -C directives into opts_qrsh - we need them for parsing 
          * the script 
          */
         while ((ep = lGetElemStr(opts_defaults, SPA_switch, "-C"))) {
            lDechainElem(opts_defaults, ep);
            lAppendElem(opts_qrsh, ep);
         }
         while ((ep = lGetElemStr(opts_cmdline, SPA_switch, "-C"))) {
            lDechainElem(opts_cmdline, ep);
            lAppendElem(opts_qrsh, ep);
         }
    
         /* read scriptfile and parse script options */
         opt_list_append_opts_from_script(my_who, 
                                          &opts_scriptfile, &alp, 
                                          opts_qrsh, environ);
         do_exit = parse_result_list(alp, &alp_error);
         lFreeList(&alp);

         if (alp_error) {
            sge_prof_cleanup();
            SGE_EXIT((void**)&ctx, 1);
         }

         /* set length and script contents in job */
         if ((ep = lGetElemStr(opts_scriptfile, SPA_switch, STR_PSEUDO_SCRIPTLEN))) {
            lSetUlong(job, JB_script_size, lGetUlong(ep, SPA_argval_lUlongT));
            lRemoveElem(opts_scriptfile, &ep);
         }
         if ((ep = lGetElemStr(opts_scriptfile, SPA_switch, STR_PSEUDO_SCRIPTPTR))) {
            lSetString(job, JB_script_ptr, lGetString(ep, SPA_argval_lStringT));
            lRemoveElem(opts_scriptfile, &ep);
         }
      }   
   }

   if (!existing_job) {
      set_job_info(job, name, is_qlogin, is_rsh, is_rlogin); 
   }

   remove_unknown_opts(opts_cmdline, lGetUlong(job, JB_type), existing_job,
                       true, is_rsh);
   remove_unknown_opts(opts_defaults, lGetUlong(job, JB_type), existing_job,
                       false, is_rsh);
   remove_unknown_opts(opts_scriptfile, lGetUlong(job, JB_type), existing_job,
                       false, is_rsh);

   opt_list_merge_command_lines(&opts_all, &opts_defaults, &opts_scriptfile,
                                &opts_cmdline);

   alp = cull_parse_qsh_parameter(my_who, myuid, username, cell_root, unqualified_hostname, qualified_hostname, opts_all, &job);
   do_exit = parse_result_list(alp, &alp_error);
   lFreeList(&alp);
   lFreeList(&opts_all);
   if (alp_error) {
      sge_prof_cleanup();
      SGE_EXIT((void**)&ctx, 1);
   }

   if (is_qlogin) {
      /* get configuration from qmaster */
      if ((client_name = get_client_name(ctx, is_rsh, is_rlogin, inherit_job)) == NULL) {
         sge_prof_cleanup();
         SGE_EXIT((void**)&ctx, 1);
      } 
   }   
   
   if (!existing_job) {
      DPRINTF(("Everything ok\n"));
#ifndef NO_SGE_COMPILE_DEBUG
      if (rmon_mlgetl(&RMON_DEBUG_ON, TOP_LAYER) & INFOPRINT) { 
         lWriteElemTo(job, stdout);
      }
#endif

      if (lGetUlong(job, JB_verify)) {
         cull_show_job(job, 0, false); 
         sge_prof_cleanup();
         SGE_EXIT((void**)&ctx, 0);
      }

      /*
      ** security hook
      */
      if (set_sec_cred(sge_root, mastername, job, &alp) != 0) {
         answer_list_output(&alp);
         SGE_EXIT((void**)&ctx, 1);
      }   

      just_verify = (lGetUlong(job, JB_verify_suitable_queues) == JUST_VERIFY ||
                     lGetUlong(job, JB_verify_suitable_queues) == POKE_VERIFY);
   }

   /*
   ** start a network server where the telnetd/rlogind or the shepherd (for
   ** builtin ijs) can connect to and set it's hostname and port
   ** in the job environment variable QRSH_PORT, so the qrsh_starter or the
   ** shepherd can parse it and establish a connection.
   */
   if (g_new_interactive_job_support == false) {
      /*
      ** open socket for qlogin communication and set host and port 
      ** in environment QRSH_PORT 
      */
      char buffer[1024];
      int my_port = 0;
      lList *envlp = NULL;

      sock = open_qrsh_socket(&my_port);
      sprintf(buffer, "%s:%d", qualified_hostname, my_port);

      if ((envlp = lGetList(job, JB_env_list)) == NULL) {
         envlp = lCreateList("environment list", VA_Type);
         lSetList(job, JB_env_list, envlp);
      }   

      var_list_set_string(&envlp, "QRSH_PORT", buffer);
      set_command_to_env(envlp, opts_qrsh);
   } else {
      int     ret = 0;
      dstring err_msg = DSTRING_INIT;

      /* before starting a commlib server, we must set signal masks and handlers */
      set_builtin_ijs_signals_and_handlers();

      /* then start the commlib server */
      ret = start_ijs_server(csp_mode, username, &comm_handle, &err_msg);
      if (ret != 0) {
         if (ret == 1) {
            ERROR((SGE_EVENT, MSG_QSH_CREATINGCOMMLIBSERVER_S,
               sge_dstring_get_string(&err_msg)));
         } else if (ret == 2) {
            ERROR((SGE_EVENT, MSG_QSH_SETTINGCONNECTIONPARAMS_S,
               sge_dstring_get_string(&err_msg)));
         }
         SGE_EXIT((void**)&ctx, 1);
      }

      /* if it started successfully, write the connection data to the job object,
       * so it can be sent over the QMaster to the execution host.
       */
      write_builtin_ijs_connection_data_to_job_object(qualified_hostname,
         comm_handle->service_port, job, opts_qrsh);
   }

   /* 
   ** if environment QRSH_WRAPPER is set, pass it trough environment
   */
   {
      char  *wrapper = getenv("QRSH_WRAPPER");

      if (wrapper != NULL) {
         lList *envlp = lGetList(job, JB_env_list);
         var_list_set_string(&envlp, "QRSH_WRAPPER", wrapper);
      }
   }

   /* 
   ** add the job
   */
   if (existing_job) {
#if 0
      int execd_status = -1;
#endif
      int msgsock   = -1;
      sge_tid_t tid;
     
      VERBOSE_LOG((stderr, MSG_QSH_SENDINGTASKTO_S, host)); 
      VERBOSE_LOG((stderr, "\n"));

      /* block certain signals to allow qrsh -inherit for jobs which
       * have been started with the -notify switch
       */
      block_notification_signals();

      /* connection to qmaster is not needed any longer */
      cl_commlib_shutdown_handle(cl_com_get_handle(progname, 0), false);

      /* start task in tightly integrated job */
      /* directly connect to commlib of exec daemon and submit task */
      tid = sge_qexecve(ctx,
                        host, NULL, 
                        lGetString(job, JB_cwd), 
                        lGetList(job, JB_env_list),
                        lGetList(job, JB_path_aliases)); 
      if (tid == NULL) {
         const char *qexec_lasterror = qexec_last_err();

         ERROR((SGE_EVENT, MSG_QSH_EXECUTINGTASKOFJOBFAILED_IS, existing_job,
            qexec_lasterror != NULL ? qexec_lasterror : "unknown"));
         sge_prof_cleanup();
         SGE_EXIT((void **)&ctx, EXIT_FAILURE);
      }

      VERBOSE_LOG((stderr, MSG_QSH_SERVERDAEMONSUCCESSFULLYSTARTEDWITHTASKID_S, tid)); 
      VERBOSE_LOG((stderr, "\n"));

      if (g_new_interactive_job_support == false) {
         /* wait for a client to connect */
         if ((msgsock = wait_for_qrsh_socket(sock, QSH_SOCKET_FINAL_TIMEOUT)) == -1) {
            ERROR((SGE_EVENT,MSG_QSH_CANNOTGETCONNECTIONTOQLOGIN_STARTER_SS,"shepherd", host));
            sge_prof_cleanup();
            SGE_EXIT((void **)&ctx, EXIT_FAILURE);
         }
      
         sge_free(&host);
         /* get host and port of rshd, job_dir and utilbin_dir over connection */
         if (!get_client_server_context(msgsock, &port, &job_dir, &utilbin_dir, &host)) {
            sge_prof_cleanup();
            SGE_EXIT((void **)&ctx, EXIT_FAILURE);
         }

         VERBOSE_LOG((stderr, MSG_QSH_ESTABLISHINGREMOTESESSIONTO_SS, 
            client_name, host));
         VERBOSE_LOG((stderr, "\n")); 

         /* start task via rsh on rshd */
         exit_status = start_client_program(client_name, host, port,
            job_dir, utilbin_dir, is_rsh, is_rlogin, nostdin, noshell, sock);
      } else { /* g_new_interactive_job_support == true */
         int     ret;
         dstring err_msg = DSTRING_INIT;

         /*
          * Wait for the client (=shepherd) to connect to us
          */
         DPRINTF(("waiting for connection\n"));
         ret = comm_wait_for_connection(comm_handle, COMM_CLIENT,
                                        QSH_SOCKET_FINAL_TIMEOUT, &host, &err_msg);
         if (ret != COMM_RETVAL_OK) {
            ERROR((SGE_EVENT, MSG_QSH_GOTNOCONNECTIONWITHINSECONDS_IS, 
                  QSH_SOCKET_FINAL_TIMEOUT, sge_dstring_get_string(&err_msg)));
            sge_dstring_free(&err_msg);
            return 1;
         }

         if (nostdin == 1) {
            close(STDIN_FILENO);
         }

         DPRINTF(("starting IJS server\n"));
         sge_dstring_sprintf(&err_msg, "<null>");
         ret = run_ijs_server(comm_handle, host, job_id, nostdin, noshell, 
                              is_rsh, is_qlogin, pty_option, suspend_remote_option,
                              &exit_status, &err_msg);
         if (ret != 0) {
            ERROR((SGE_EVENT, MSG_QSH_ERRORRUNNINGIJSSERVER_S,
               sge_dstring_get_string(&err_msg)));
         }
         stop_ijs_server(&comm_handle, &err_msg);
         DPRINTF(("stop_ijs_server returned error: %s\n",
            sge_dstring_get_string(&err_msg)));
         sge_dstring_free(&err_msg);
         if (ret != 0) {
            return 1;
         }
      }

      /* CR: TODO: This code is not active because there is no need to wait for an
       *           task exit message. The exit_status is already reported by 
       *           start_client_program().
       *
       *           The code which sends the task exit message is located in the
       *           execd code in file reaper_execd.c, function clean_up_job(). 
       *           Activate this code to enable task exit messages again and 
       *           use sge_qwaittid() to wait for task exit messages.
       */
#if 0
      sge_qwaittid(tid,&execd_status,1);
#endif
   } else {
      int polling_interval;
      dstring id_dstring = DSTRING_INIT;
      
      srand(getpid());

      /* initialize first polling interval, batch jobs have longer polling interval */
      if (JOB_TYPE_IS_IMMEDIATE(lGetUlong(job, JB_type))) {
         polling_interval = QSH_INTERACTIVE_POLLING_MIN;
      } else {
         polling_interval = QSH_BATCH_POLLING_MIN;
      }   

      job_add_parent_id_to_context(job);

      /*
       * fill in user and group
       *
       * following is not used for security. qmaster can not rely
       * on that information. but it is still necessary to set the
       * attributes in the job so that the information is available
       * in the JSV client context
       */
      job_set_owner_and_group(job, ctx->get_uid(ctx), ctx->get_gid(ctx),
                              ctx->get_username(ctx), ctx->get_groupname(ctx));

      lp_jobs = lCreateList("submitted jobs", JB_Type);
      lAppendElem(lp_jobs, job);
   
      DPRINTF(("B E F O R E     S E N D I N G! ! ! ! ! ! ! ! ! ! ! ! ! !\n"));
      DPRINTF(("=====================================================\n"));

      /* submit the job to the QMaster */
      alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, 
                     &lp_jobs, NULL, NULL, false);

      /* reinitialize 'job' with pointer to new version from qmaster */
      job = lFirst(lp_jobs);
      if (job) {
        job_id = lGetUlong(job, JB_job_number );
      } else {
        job_id = 0;
      }
      DPRINTF(("job id is: %ld\n", (long) job_id));

      status = 0; 
      
      for_each(aep, alp) {
         quality = lGetUlong(aep, AN_quality);
         if (quality == ANSWER_QUALITY_ERROR) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
            do_exit = 1;
            if (lGetUlong(aep, AN_status)==STATUS_NOTOK_DOAGAIN) {
               status = STATUS_NOTOK_DOAGAIN;
            } else {
               /* set return value to error when doing verification */
               status = 1;
            }
         } else if (quality == ANSWER_QUALITY_WARNING) {
            fprintf(stderr, "%s\n", lGetString(aep, AN_text));
         } else {
            if (log_state_get_log_verbose() || just_verify) {
               fprintf(stdout, "%s\n", lGetString(aep, AN_text));
            }
         }
      }

      lFreeList(&alp);

      if (just_verify)
         do_exit = 1;
   
      if (do_exit) {
         lFreeList(&lp_jobs);
         if (status == STATUS_NOTOK_DOAGAIN) { 
            sge_prof_cleanup();
            SGE_EXIT((void **)&ctx, status);
         } else if (just_verify) {
            sge_prof_cleanup();
            SGE_EXIT((void**)&ctx, status);
         } else {
            sge_prof_cleanup();
            SGE_EXIT((void **)&ctx, 1);
         }
      }
      
      VERBOSE_LOG((stderr, SFNMAX, MSG_QSH_WAITINGFORINTERACTIVEJOBTOBESCHEDULED));

      DPRINTF(("R E A D I N G    J O B ! ! ! ! ! ! ! ! ! ! !\n"));
      DPRINTF(("============================================\n"));
      
      while (!do_exit) {
         lCondition *where;
         lEnumeration *what;
         u_long32 job_status;
         int do_shut = 0;
         lList *lp_poll = NULL;
         lListElem *ja_task = NULL;
         lListElem* jep;
         bool b_already_logged_job_was_scheduled = false;
         int msgsock = -1;
         int random_poll = polling_interval + (rand() % polling_interval);

         DPRINTF(("random polling set to %d\n", random_poll));

         /* close connection to QMaster */
         cl_commlib_close_connection(cl_com_get_handle(progname,0),
                                     (char*)mastername,
                                     (char*)prognames[QMASTER],
                                     1, false);
   
         if (is_qlogin) {
            if (g_new_interactive_job_support == false) {
               /* if qlogin_starter is used (qlogin, rsh, rlogin): wait for context */
               msgsock = wait_for_qrsh_socket(sock, random_poll); 

               /* qlogin_starter reports "ready to start" */
               if (msgsock >= 0) {
                  if (!get_client_server_context(msgsock, &port, &job_dir, 
                     &utilbin_dir, &host)) {
                     cl_com_ignore_timeouts(false);
                     cl_commlib_open_connection(cl_com_get_handle(progname,0),
                                        (char*)mastername,
                                        (char*)prognames[QMASTER],
                                        1);
                     delete_job(ctx, job_id, lp_jobs);
                     do_exit = 1;
                     exit_status = 1;
                     break;
                  }
                  VERBOSE_LOG((stderr, "\n")); 
                  VERBOSE_LOG((stderr, MSG_QSH_INTERACTIVEJOBHASBEENSCHEDULED_S, 
                               job_get_id_string(job_id, 0, NULL, &id_dstring)));
                  VERBOSE_LOG((stderr, "\n")); 
                  VERBOSE_LOG((stderr, MSG_QSH_ESTABLISHINGREMOTESESSIONTO_SS, 
                     client_name, host));
                  VERBOSE_LOG((stderr, "\n"));
                  b_already_logged_job_was_scheduled = true;

                  /* 
                   * start client program (e.g. rsh) and wait blocking until
                   * it quits.
                   */
                  exit_status = start_client_program(client_name, 
                     host, port, job_dir, utilbin_dir, is_rsh, is_rlogin, 
                     nostdin, noshell, sock);

                  DPRINTF(("exit_status = %d\n", exit_status));

                  if (exit_status < 0) {
                     WARNING((SGE_EVENT, MSG_QSH_CLEANINGUPAFTERABNORMALEXITOF_S, 
                        client_name));
                     cl_com_ignore_timeouts(false);
                     cl_commlib_open_connection(cl_com_get_handle(progname,0),
                        (char*)mastername, (char*)prognames[QMASTER], 1);
                     DPRINTF(("deleting job\n"));
                     delete_job(ctx, job_id, lp_jobs);
                     exit_status = EXIT_FAILURE;
                  }

                  do_exit = 1;
                  continue;
               } else {
                  DPRINTF(("---- got NO valid socket! ----\n"));
               }
            } else { /* if (g_new_interactive_job_support == true) */
               int     ret;
               dstring err_msg = DSTRING_INIT;

               if (comm_handle == NULL) {
                  /* If we get here we already had a connection that was closed
                   * and the loop was done again accidentially.
                   * However, we can just exit the loop here.
                   */
                  do_exit = 1;
                  continue;
               }

               /*
                * Wait for the client to connect to us
                */
               DPRINTF(("waiting for connection\n"));
               sge_dstring_sprintf(&err_msg, "<null>");
               ret = comm_wait_for_connection(comm_handle, COMM_CLIENT, 
                                              random_poll, &host, &err_msg);

               if (ret != COMM_RETVAL_OK) {
                  if (ret == COMM_GOT_TIMEOUT) {
                     DPRINTF(("got no connection within timeout of %d s\n", random_poll));
                     /* Loop again */
                  } else {
                     /* comm_wait_for_connection() returned an error */
                     ERROR((SGE_EVENT, MSG_QSH_ERRORWHILEWAITINGFORBUILTINIJSCONNECTION_S,
                        sge_dstring_get_string(&err_msg)));

                     DPRINTF(("got error while waiting for connection\n"));
                     cl_com_ignore_timeouts(false);
                     /* Tell the master to delete the job */
                     cl_commlib_open_connection(cl_com_get_handle(progname,0),
                                        (char*)mastername,
                                        (char*)prognames[QMASTER],
                                        1);
                     delete_job(ctx, job_id, lp_jobs);

                     do_exit = 1;
                     exit_status = 1;
                     sge_dstring_free(&err_msg);
                     break;
                  } 
               } else { /* if (ret == COMM_RETVAL_OK) */
                  VERBOSE_LOG((stderr, "\n")); 
                  VERBOSE_LOG((stderr, MSG_QSH_INTERACTIVEJOBHASBEENSCHEDULED_S, 
                               job_get_id_string(job_id, 0, NULL, &id_dstring)));
                  VERBOSE_LOG((stderr, "\n")); 
                  VERBOSE_LOG((stderr, MSG_QSH_ESTABLISHINGREMOTESESSIONTO_SS, 
                     client_name, host));
                  VERBOSE_LOG((stderr, "\n"));
                  b_already_logged_job_was_scheduled = true;
           
                  if (nostdin == 1) {
                     DPRINTF(("closing STDIN\n"));
                     close(STDIN_FILENO);
                  }

                  /* run_ijs_server() loops until the client has disconnected */
                  sge_dstring_sprintf(&err_msg, "<null>");
                  ret = run_ijs_server(comm_handle, host, job_id, nostdin, noshell, 
                                       is_rsh, is_qlogin, pty_option, suspend_remote_option,
                                       &exit_status, &err_msg);
                  if (ret != 0) {
                     ERROR((SGE_EVENT, MSG_QSH_ERRORRUNNINGIJSSERVER_S,
                        sge_dstring_get_string(&err_msg)));
                  }
                  stop_ijs_server(&comm_handle, &err_msg);
                  DPRINTF(("stop_ijs_server returned: %s\n",
                     sge_dstring_get_string(&err_msg)));
                  if (ret != 0) {
                     sge_dstring_free(&err_msg);
                     do_exit = 1;
                     continue;
                  }
                  /* run_ijs_server() didn't return 0, some unexpected error
                   * occured -> do the while loop again. */
               }
               sge_dstring_free(&err_msg);
            }
         } else {
            /* wait for qsh job to be scheduled */
            sleep(random_poll);
         }   
         cl_com_ignore_timeouts(false);
         cl_commlib_open_connection(cl_com_get_handle(progname,0),
                                    (char*)mastername,
                                    (char*)prognames[QMASTER],
                                    1);

         /* get job from qmaster: to handle qsh and to detect deleted qrsh job */
         what = lWhat("%T(%I)", JB_Type, JB_ja_tasks); 
         where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, job_id); 
         alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &lp_poll, where, what, false);

         do_exit = parse_result_list(alp, &alp_error);
   
         lFreeWhere(&where);
         lFreeWhat(&what);
         lFreeList(&alp);
   
         do_shut = shut_me_down;
         if (do_shut || do_exit) {
            WARNING((SGE_EVENT, SFNMAX, MSG_QSH_REQUESTFORINTERACTIVEJOBHASBEENCANCELED));
            delete_job(ctx, job_id, lp_jobs);
            lFreeList(&lp_poll);
            do_exit = 1;
            exit_status = 1;
            continue;
         }
  
         if (alp_error) {
            lFreeList(&lp_poll);
            continue;
         }
  
         if ((lp_poll == NULL || lGetNumberOfElem(lp_poll) == 0 ) || !(jep = lFirst(lp_poll))) {
            log_state_set_log_verbose(1);
            WARNING((SGE_EVENT, MSG_QSH_REQUESTCANTBESCHEDULEDTRYLATER_S, progname));
            do_exit = 1;
            exit_status = 1;
            lFreeList(&lp_poll);
            continue;
         }
         
         ja_task = lFirst(lGetList(jep, JB_ja_tasks)); 
         if (ja_task) {
            job_status = lGetUlong(ja_task, JAT_status);
            DPRINTF(("Job Status is: %lx\n", (long) job_status));
         } else {
            job_status = JIDLE;
            DPRINTF(("Job Status is: %lx (unenrolled)\n", (long) job_status));
         }
   
         lFreeList(&lp_poll);

         switch(job_status) {
            /* qsh or future -wait case */
            case JIDLE:
               VERBOSE_LOG((stderr, "."));
               break;
   
            case JRUNNING:
            case JTRANSFERING:
               /* log this only once */
               if (b_already_logged_job_was_scheduled == false) {
                  VERBOSE_LOG((stderr, "\n")); 
                  VERBOSE_LOG((stderr, MSG_QSH_INTERACTIVEJOBHASBEENSCHEDULED_S, 
                               job_get_id_string(job_id, 0, NULL, &id_dstring)));
                  VERBOSE_LOG((stderr, "\n")); 
                  b_already_logged_job_was_scheduled = true;
               }
  
               /* For old IJS:
                * in case of qlogin: has been scheduled / is transitting just after 
                * timeout -> loop.
                * For new IJS:
                * If it was a qrsh with command, it's finished here even if the
                * QMaster didn't already recognize it -> exit.
                */
               if ((g_new_interactive_job_support == false && is_qlogin)
                  || (g_new_interactive_job_support == true && is_qlogin && !is_rsh)) {
                  continue;
               } else {
                  /* qsh: xterm has been started -> exit */   
                  do_exit = 1;
               }
               continue; /* continue loop immediatley, don't break out of switch first */
   
            case JFINISHED:
               /* This was (or might have been) true for the old IJS
                * - in the new IJS, a finished job is really finished,
                * so exit qrsh/qlogin with real exit_status.
                */
               if (g_new_interactive_job_support == false) {
                  WARNING((SGE_EVENT, SFNMAX, MSG_QSH_CANTSTARTINTERACTIVEJOB));
                  do_exit = 1;
                  exit_status = 1;
               } else {
                  do_exit = 1;
               }
               continue; /* continue loop immediatley, don't break out of switch first */

            default:
               ERROR((SGE_EVENT, MSG_QSH_UNKNOWNJOBSTATUS_X, sge_x32c(job_status)));
               do_exit = 1;
               exit_status = 1;
               break;
         }

         if (!do_exit && polling_interval < QSH_POLLING_MAX) {
            polling_interval *= 2;
            DPRINTF(("polling_interval set to %d\n", polling_interval));
         }
      } /* end of while (1) polling */
      if (g_new_interactive_job_support == true && comm_handle != NULL) {
         dstring err_msg = DSTRING_INIT;

         if (force_ijs_server_shutdown(&comm_handle, COMM_CLIENT, &err_msg) != 0) {
            ERROR((SGE_EVENT, "%s", sge_dstring_get_string(&err_msg)));
         }
         sge_dstring_free(&err_msg);
      }

      lFreeList(&lp_jobs);
      sge_dstring_free(&id_dstring);
   }

   sge_free(&client_name);
   sge_prof_cleanup();
   SGE_EXIT((void **)&ctx, exit_status);
   DEXIT;
   return exit_status;
}

static void delete_job(sge_gdi_ctx_class_t *ctx, u_long32 job_id, lList *jlp) 
{
   lListElem *jep;
   lList *idlp = NULL;
   lList *alp;
   char job_str[128];

   if (jlp == NULL) {
      return;
   }
   jep = lFirst(jlp);
   if (jep == NULL) {
      return;
   }

   sprintf(job_str, sge_u32, job_id);
   lAddElemStr(&idlp, ID_str, job_str, ID_Type);

   alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_DEL, &idlp, NULL, NULL, false);

   /* no error handling here, we try to delete the job if we can */
   lFreeList(&idlp);
   lFreeList(&alp);
}

static void remove_unknown_opts(lList *lp, u_long32 jb_now,
                                int tightly_integrated, bool error, int is_rsh)
{
   lListElem *ep, *next;

   DENTER(TOP_LAYER, "remove_unknown_opts");

   /* no options given - nothing to remove */
   if (lp == NULL) {
      DEXIT;
      return;
   }

   /* loop over all options and remove the ones that are not allowed */
   next = lFirst(lp);
   while ((ep = next) != NULL) {
      const char *cp;
      
      next = lNext(ep);
      cp = lGetString(ep, SPA_switch);

      if (cp != NULL) {
         /* these are the options allowed for all flavors of interactive jobs
          * all other will be deleted
          */
         
         /* -hold_jid and -h are only allowed in qrsh mode */
         if (!is_rsh && (!strcmp(cp, "-hold_jid") || !strcmp(cp, "-hold_jid_ad") 
             || !strcmp(cp, "-h"))){
            if (error) {
               ERROR((SGE_EVENT, MSG_ANSWER_UNKOWNOPTIONX_S, cp));
               sge_prof_cleanup();
               SGE_EXIT(NULL, EXIT_FAILURE);
            } else {
               lRemoveElem(lp, &ep);
               continue;
            }
         }

         if (strcmp(cp, "jobarg") && strcmp(cp, "script") && strcmp(cp, "-ar") &&
            strcmp(cp, "-binding") &&
            strcmp(cp, "-A") && strcmp(cp, "-cell") && strcmp(cp, "-clear") && 
            strcmp(cp, "-cwd") && strcmp(cp, "-hard") && strcmp(cp, "-help") &&
            strcmp(cp, "-hold_jid") && strcmp(cp, "-hold_jid_ad") && strcmp(cp, "-h") && 
            strcmp(cp, "-l") && strcmp(cp, "-m") && strcmp(cp, "-masterq") &&
            strcmp(cp, "-N") && strcmp(cp, "-noshell") && strcmp(cp, "-now") &&
            strcmp(cp, "-notify") && strcmp(cp, "-P") &&
            strcmp(cp, "-p") && strcmp(cp, "-pe") && strcmp(cp, "-q") && strcmp(cp, "-v") &&
            strcmp(cp, "-V") && strcmp(cp, "-display") && strcmp(cp, "-verify") &&
            strcmp(cp, "-soft") && strcmp(cp, "-M") && strcmp(cp, "-verbose") &&
            strcmp(cp, "-ac") && strcmp(cp, "-dc") && strcmp(cp, "-sc") &&
            strcmp(cp, "-S") && strcmp(cp, "-w") && strcmp(cp, "-js") && strcmp(cp, "-R") &&
            strcmp(cp, "-o") && strcmp(cp, "-e") && strcmp(cp, "-j") && strcmp(cp, "-wd") &&
            strcmp(cp, "-jsv")
           ) {
            if (error) {
               ERROR((SGE_EVENT, MSG_ANSWER_UNKOWNOPTIONX_S, cp));
               sge_prof_cleanup();
               SGE_EXIT(NULL, EXIT_FAILURE);
            } else {
               lRemoveElem(lp, &ep);
               continue;
            }
         }
         
         /* the login type interactive jobs do not allow some options - delete these ones */
         if (JOB_TYPE_IS_QLOGIN(jb_now) || JOB_TYPE_IS_QRLOGIN(jb_now)) {
            if (strcmp(cp, "-display") == 0 ||
               strcmp(cp, "-cwd") == 0 ||
               strcmp(cp, "-wd") == 0 ||
               strcmp(cp, "-v") == 0 ||
               strcmp(cp, "-V") == 0
              ) {
               if (error) {
                  ERROR((SGE_EVENT, MSG_ANSWER_UNKOWNOPTIONX_S, cp));
                  sge_prof_cleanup();
                  SGE_EXIT(NULL, EXIT_FAILURE);
               } else {
                  lRemoveElem(lp, &ep);
                  continue;
               }
            }
         }

         /* the -S switch is only allowed for qsh */
         if (!JOB_TYPE_IS_QSH(jb_now)) {
            if (strcmp(cp, "-S") == 0) {
               if (error) {
                  ERROR((SGE_EVENT, MSG_ANSWER_UNKOWNOPTIONX_S, cp));
                  sge_prof_cleanup();
                  SGE_EXIT(NULL, EXIT_FAILURE);
               } else {
                  lRemoveElem(lp, &ep);
                  continue;
               }
            }
         }

         /* qrsh -inherit only allowes -cwd and setting of environment */
         if (tightly_integrated) {
            if (strcmp(cp, "-cwd") &&
               strcmp(cp, "-wd") &&
               strcmp(cp, "-display") &&
               strcmp(cp, "-v") &&
               strcmp(cp, "-V")
              ) {
               if (error) {
                  ERROR((SGE_EVENT, MSG_ANSWER_UNKOWNOPTIONX_S, cp));
                  sge_prof_cleanup();
                  SGE_EXIT(NULL, EXIT_FAILURE);
               } else {
                  lRemoveElem(lp, &ep);
                  continue;
               }
            }
         }
      }
   }

   DEXIT;
}

