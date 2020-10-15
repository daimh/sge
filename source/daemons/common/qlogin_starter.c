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
/*___INFO__MARK_END__*/
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <grp.h>

#include "uti/sge_stdio.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_prog.h"
#include "uti/config_file.h"
#include "uti2/sge_execvlp.h"

#include "err_trace.h"
#include "qlogin_starter.h"
#include "basis_types.h"
#include "msg_common.h"

#if defined(INTERIX)
#  include "wingrid.h"
#endif

/****** qrsh_starter/delete_qrsh_pid_file() *****************************************
*  NAME
*     delete_qrsh_pid_file() -- delete the pid file from $TMPDIR
*
*  SYNOPSIS
*     static int delete_qrsh_pid_file() 
*
*  FUNCTION
*     Delete the pid file created by qrsh_starter
*
*  RESULT
*     1, if the file could be deleted
*     0, if and error occured. Possible error situations are:
*           - the environment variable TMPDIR cannot be read
*           - the file cannot be deleted
*
*  SEE ALSO
*  qrsh_starter
*******************************************************************************/
int delete_qrsh_pid_file()
{
   char *pid_file_name = NULL;
   int ret = 1;

   if((pid_file_name = search_conf_val("qrsh_pid_file")) == NULL) {
      shepherd_trace("cannot get variable %s", pid_file_name);
      return 0;
   }
   
   if (unlink(pid_file_name) != 0) {
      shepherd_trace("cannot delete qrsh pid file %s", pid_file_name);
      ret = 0;
   }   
   
   return ret;

}



/****** shepherd/qrsh/write_to_qrsh() *****************************************
*  NAME
*     write_to_qrsh -- short description
*
*  SYNOPSIS
*    int write_to_qrsh(const char *data);
*
*  FUNCTION
*     Writes the contents of <data> to an other (remote) process over
*     a socket connection.
*     Host and port of the communication partner are read from the 
*     configuration entry "qrsh_control_port".
*     A socket client connection is opened to the named host and port,
*     and the data is written.
*
*  INPUTS
*     data - null terminated string with data to write
*
*  RESULT
*     0, if function finishes correctly
*     1, if the config entry qrsh_control_port does not exist
*     2, if qrsh_control_port contains illegal data
*     3, if opening the socket failed
*     4, if the hostname cannot be resolved
*     5, if connecting to the socket fails
*     6, if writing the data fails
******************************************************************************/
int write_to_qrsh(const char *data)
{
   char *address = NULL;
   char *host;
   char *c;
   int   port    = 0;
   int   sock    = 0;
   int datalen   = 0;
   struct sockaddr_in server;
   struct hostent *hp;

   shepherd_trace("write_to_qrsh - data = %s", data);

   /* read destination host and port from config */
   address = get_conf_val("qrsh_control_port");

   if (address == NULL) {
      shepherd_trace("config does not contain entry for qrsh_control_port");
      return 1;
   }

   shepherd_trace("write_to_qrsh - address = %s", address);

   c = strchr(address, ':');
   if (c == NULL) {
      shepherd_trace("illegal value for qrsh_control_port: \"%s\". "
                     "Should be host:port", address);
      return 2;
   }
  
   *c = 0;
   host = address;
   port = atoi(c + 1);
  
   shepherd_trace("write_to_qrsh - host = %s, port = %d", host, port);

   /* create socket. */
   sock = socket( AF_INET, SOCK_STREAM, 0);
   if (sock == -1) {
      shepherd_trace("error opening stream socket: %s", strerror(errno));
      return 3;
   }

   /* connect socket using name specified by command line. */
   server.sin_family = AF_INET;
   hp = gethostbyname(host);
  
   /*
   * gethostbyname returns a structure including the network address
   * of the specified host.
   */
   if (hp == (struct hostent *) 0) {
      shepherd_trace("%s: unknown host", host);
      close(sock);
      return 4;
   }

   memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
   server.sin_port = htons(port);
 
   if (connect(sock, (struct sockaddr *) &server, sizeof server) == -1) {
      shepherd_trace("error connecting stream socket: %s", strerror(errno));
      close(sock);
      return 5;
   }

   /* write data */
   datalen = strlen(data) + 1;
   if (write(sock, data, datalen) != datalen) {
     shepherd_trace("error writing data to qrsh_control_port");
     close(sock);
     return 6;
   }

   /* close connection */
   close(sock);
   return 0;
}

/****** shepherd/qrsh/write_exit_code_to_qrsh() *******************************
*  NAME
*     write_exit_code_to_qrsh -- write an exit code to qrsh
*
*  SYNOPSIS
*     void write_exit_code_to_qrsh(int exit_code)
*
*  FUNCTION
*     If the program handled by this shepherd uses rsh mechanism
*     (configuration value "rsh_daemon" is set), then the function
*     writes an exit code to the corresponding qrsh process via a 
*     socket connection.
*
*     The exit code is either taken from parameter <exit_code>, if it is
*     notequal 0, to signal an error condition in the shepherd,
*     or read from a special file ($TMPDIR/qrsh_exit_code).
*
*  INPUTS
*     exit_code - status of the calling process
*
*  SEE ALSO
*     shepherd/qrsh/write_to_qrsh()
******************************************************************************/
void write_exit_code_to_qrsh(int exit_code)
{
   char buffer[1024];
   *buffer = 0;

   /* rshd exited with OK: try to get returncode from qrsh_starter file */
   shepherd_trace("write_exit_code_to_qrsh(%d)", exit_code);

   /* write exit code as string number to qrsh */
   sprintf(buffer, "%d", exit_code);
   if (write_to_qrsh(buffer) != 0) {
      shepherd_trace("writing exit code to qrsh failed");
   }
}

/****** shepherd/qrsh/get_exit_code_of_qrsh_starter() *************************
*  NAME
*     get_exit_code_of_qrsh_starter -- short description
*
*  SYNOPSIS
*     #include "qlogin_starter.h"
*     int get_exit_code_of_qrsh_starter(int* exit_code);
*
*  FUNCTION
*     Reads the exit code from a process started via qrsh - qrsh_starter
*     from a file in the jobs TMPDIR.
*
*  INPUTS
*     exit_code - exit code of qrsh_starter
*
*  RESULT
*     0, success
*     1, if an error occured while trying to get the exit code
******************************************************************************/
int get_exit_code_of_qrsh_starter(int* exit_code)
{
   char buffer[1024];
   int ret = 1;

   *exit_code = 1;
   *buffer = 0;

   /* rshd exited with OK: try to get returncode from qrsh_starter file */

   /* we only have an error file in TMPDIR in case of rsh, 
    * otherwise pass exit_code */
   if (search_conf_val("rsh_daemon") != NULL) {
      char *tmpdir;
      char *taskid;
      FILE *errorfile;

      tmpdir = search_conf_val("qrsh_tmpdir");
      taskid = search_conf_val("pe_task_id");
      shepherd_trace("get_exit_code_of_qrsh_starter - TMPDIR = %s, pe_task_id = %s",
                     tmpdir ? tmpdir : "0", taskid ? taskid : "0");
      if (tmpdir != NULL) {
         if (taskid != NULL) {
            snprintf(buffer, sizeof(buffer), "%s/qrsh_exit_code.%s", tmpdir, taskid);
         } else {
            snprintf(buffer, sizeof(buffer), "%s/qrsh_exit_code", tmpdir);
         }

         errorfile = fopen(buffer, "r");
         if (errorfile != NULL) {
            ret = 0;
            if (fscanf(errorfile, "%d", exit_code) == 1) {
               shepherd_trace("error code from remote command is %d", *exit_code);
            }
            FCLOSE(errorfile);
            if (unlink(buffer) != 0) {
               shepherd_trace("can't delete %s", buffer);
            }
         } else {
            shepherd_trace("can't open file %s: %s", buffer, strerror(errno));
         }
      } else {
        shepherd_trace("unable to get qrsh_tmpdir");
      }
   }
   return ret;        
FCLOSE_ERROR:
   shepherd_trace(MSG_FILE_NOCLOSE_SS, buffer, strerror(errno));
   return ret;
}

/****** shepherd/qrsh/get_error_of_qrsh_starter() *************************
*  NAME
*     get_error_of_qrsh_starter -- get error message from qrsh_starter
*
*  SYNOPSIS
*     #include "qlogin_starter.h"
*     const char *
*     get_error_of_qrsh_starter(void);
*
*  FUNCTION
*     Reads an error message that qrsh_starter may have written to the 
*     qrsh jobs tmpdir due to an error in the startup phase of the qrsh job.
*
*  RESULT
*     the error message from qrsh_starter or
*     NULL, if no error was generated (the job started up without problems)
*
*  NOTE
*     The returned string is dynamically allocated. It is in the responsibility
*     of the caller to free it.
******************************************************************************/
const char *get_error_of_qrsh_starter(void)
{
   char buffer[SGE_PATH_MAX];
   char *ret = NULL;
   
   *buffer = 0;

   /* rshd exited with OK: try to get error messages from qrsh_starter file */
   shepherd_trace("get_error_of_qrsh_starter()"); 

   /* we only have an error file in TMPDIR in case of rsh */
   if (search_conf_val("rsh_daemon") != NULL) {
      char *tmpdir;
      char *taskid;
      FILE *errorfile;

      tmpdir = search_conf_val("qrsh_tmpdir");
      taskid = search_conf_val("qrsh_task_id");
      shepherd_trace("get_error_of_qrsh_starter - TMPDIR = %s, qrsh_task_id = %s", 
                     tmpdir ? tmpdir : "0", taskid ? taskid : "0");
      if (tmpdir != NULL) {
         if (taskid != NULL) {
            snprintf(buffer, sizeof(buffer), "%s/qrsh_error.%s", tmpdir, taskid);
         } else {
            snprintf(buffer, sizeof(buffer), "%s/qrsh_error", tmpdir);
         }

         errorfile = fopen(buffer, "r");
         if (errorfile != NULL) {
            char buffer[MAX_STRING_SIZE];

            if (fgets(buffer, MAX_STRING_SIZE, errorfile) != NULL) {
               shepherd_trace("error string from qrsh_starter is %s", buffer);
               ret = strdup(buffer);
            }
            FCLOSE(errorfile);
            if (unlink(buffer) != 0) {
               shepherd_trace("can't delete %s", buffer);
            }
         }
      }
   }
   return ret;  
FCLOSE_ERROR:
   shepherd_trace(MSG_FILE_NOCLOSE_SS, buffer, strerror(errno));
   return ret;

}

/****** shepherd/qrsh/qlogin_starter() ****************************************
*
*  NAME
*     qlogin_starter -- start protocol daemon like rshd
*
*  SYNOPSIS
*     #include "qlogin_starter.h"
*     int qlogin_starter(const char *cwd, char *daemon);
*
*  FUNCTION
*     The function is called from shepherd to start a protocol daemon
*     like telnetd, rshd or rlogind.
*     The mechanism used to call these daemons is that of inetd:
*        - a socket is created (server side, any free port is assigned 
*          by the operating system)
*        - qlogin_starter waits for someone to connect to this socket
*        - the socket file handles are redirected to stdin, stdout 
*          and stderr
*        - the daemon process is started
*     Additionally to the inetd mechanism, the port number and some 
*     other information is sent to the qrsh process that initiated
*     (over qmaster, execd, shepherd) the qlogin_starter call.
*
*  INPUTS
*     cwd    - the current working directory (the active_jobs directory)
*     daemon - name and path of the daemon to start
*
*  RESULT
*     on success, the function will not return (it exec's)
*      4, if there is a problem with permissions
*      5, if a socket cannot be allocated
*      6, if a socket bind fails
*      7, if socket name (port) cannot be determined
*      8, if environment (to be passed to qrsh) cannot be read
*      9, if sending information to qrsh fails
*     10, if nobody connects to the socket within a one minute
*     11, if the acception of a connecting client fails
*     12, if the execution of the daemon fails
******************************************************************************/
int qlogin_starter(const char *cwd, char *daemon, char** env)
{
   int ret;
   int port;
   int fd;
   int maxfd;
   int sockfd;
   int on = 1;
   int sso = 1;
   int newsfd;
   fd_set fds;
   struct sockaddr_in serv_addr;
   struct timeval timeout;
   char buffer[2048];
   char *args[20]; /* JG: TODO: should be dynamically allocated */
   int argc = 0;
   const char *sge_root = NULL;
   const char *arch = NULL;
   
#if defined(IRIX65) || defined(INTERIX) || defined(DARWIN6) || defined(ALPHA5) || defined(HP1164)
   int length;
   int len;
#else
   socklen_t length;
   socklen_t len;
#endif

   len = sizeof(serv_addr);
   errno = 0;
   /* must be root because we must access /dev/something */
   if (sge_setgid(SGE_SUPERUSER_GID) ||
       sge_setuid(SGE_SUPERUSER_UID) ||
       sge_setegid(SGE_SUPERUSER_GID) ||
       sge_seteuid(SGE_SUPERUSER_UID)) {
      shepherd_trace("cannot change uid/gid: %s\n", strerror(errno));
      return 4;
   }
   shepherd_trace("uid = "uid_t_fmt", euid = "uid_t_fmt", gid = "gid_t_fmt 
                  ", egid = "gid_t_fmt, getuid(), geteuid(), getgid(), getegid());
   
   /* socket stuff from here */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd == -1) {
      shepherd_trace("cannot open socket.");
      return 5;
   }
   shepherd_trace("using sfd %d", sockfd);

   setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
   
   /* bind an address to any socket */
   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_port = 0; 
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
   if (ret != 0) {
      shepherd_trace("cannot bind socket: %s", strerror(errno));
      shutdown(sockfd, 2);
      close(sockfd);
      return 6;
   }

   /* find out assigned port number and pass it to caller */
   length = sizeof(serv_addr);
   if (getsockname(sockfd,(struct sockaddr *) &serv_addr, &length) == -1) {
      shepherd_trace("getting socket name failed: %s", strerror(errno));
      shutdown(sockfd, 2);
      close(sockfd);
      return 7;
   }
   
   /* listen on socked - make connections be accepted */
   if (listen(sockfd, 1) != 0) {
      shepherd_trace("listen failed: %s", strerror(errno));
      shutdown(sockfd, 2);
      close(sockfd);
      return 8;
   }

   /* send necessary info to qrsh: port + utilbin directory + active job 
    * directory 
    */
   port = ntohs(serv_addr.sin_port);
   shepherd_trace("bound to port %d", port);
 
   sge_root = sge_get_root_dir(0, NULL, 0, 1);
   arch = sge_get_arch();
   
   if (sge_root == NULL || arch == NULL) {
      shepherd_trace("reading environment SGE_ROOT and ARC failed");
      shutdown(sockfd, 2);
      close(sockfd);
      return 9;
   }
  
   snprintf(buffer, 2048, "0:%d:%s/utilbin/%s:%s:%s",
            port, sge_root, arch, cwd, get_conf_val("host"));

   if (write_to_qrsh(buffer) != 0) {
      shepherd_trace("communication with qrsh failed");
      shutdown(sockfd, 2);
      close(sockfd);
      return 10;
   }
   
   /* wait for connection */
   shepherd_trace("waiting for connection.");
   /* use a reasonable timeout (60 seconds) to prevent hanging here forever */
   FD_ZERO(&fds);
   FD_SET(sockfd, &fds);
   timeout.tv_sec = 60;
   timeout.tv_usec = 0;
   if (select(sockfd+1, &fds, NULL, NULL, &timeout) < 1) {
      shepherd_trace("nobody connected to the socket");
      shutdown(sockfd, 2);
      close(sockfd);
      return 11;
   }

   /* accept connection */
   newsfd = accept(sockfd, (struct sockaddr *)(&serv_addr), &len);
   if (newsfd == -1) {
      shepherd_trace("error when accepting socket conection");
      shutdown(sockfd, 2);
      close(sockfd);
      return 12;
   }
   shepherd_trace("accepted connection on fd %d", newsfd);

   /* now we have a connection and do no longer need the "well known" port 
    * free this resource.
    */
   shutdown(sockfd, 2);
   close(sockfd);

   /* don't close on exec */
   fcntl( newsfd, F_SETFD, 0 );

   /* speed up ;-) */
   setsockopt(newsfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &sso, sizeof(int));

   /* use this fd as stdin,out,err */
   dup2( newsfd, 0 );
   dup2( newsfd, 1 );
   dup2( newsfd, 2 );
   
   /* close all the rest */
#ifndef WIN32NATIVE
   maxfd = sysconf(_SC_OPEN_MAX);
#else /* WIN32NATIVE */
   maxfd = FD_SETSIZE;
   /* detect maximal number of fds under NT/W2000 (env: Files)*/
#endif /* WIN32NATIVE */
   
   /* we do not use any FD_SET call it is ok to use _SC_OPEN_MAX */
   for (fd=3; fd<maxfd; fd++) {
      close(fd);
   }

   shepherd_trace("daemon to start: |%s|", daemon);

   /* split daemon commandline into single arguments */
   /* JG: TODO: might contain quoted arguments containing spaces 
    *           make function to split or use an already existing one
    */
   args[argc++] = strtok(daemon, " ");
   while ((args[argc++] = strtok(NULL, " ")) != NULL);
#if 0
   {
      int i = 0;
      shepherd_trace("daemon commandline split to %d arguments", argc);
      while (args[i] != NULL) {
         shepherd_trace("daemon argv[%d] = |%s|", i, args[i]);
         i++;
      }
   }
#endif

   /* that's it. */
   execve(args[0], args, env);

   /* oh oh, exec failed */
   /* no way to tell anyone, becuase all FDs are closed */
   /* last chance -> tell parent process */
   shutdown(newsfd, 2);
   close(newsfd);
   return 13;
}
