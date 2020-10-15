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
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <fcntl.h>
 
#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_stdio.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_log.h"
#include "uti/sge_os.h"
#include "uti/msg_utilib.h"
#include "uti/sge_time.h"

#include "basis_types.h"

#include "msg_common.h"

#if defined(SOLARIS)
#   include "sge_smf.h"
#endif

#ifdef NO_SGE_COMPILE_DEBUG
#   undef SGE_EXIT
#   define SGE_EXIT(x)     exit(x)
#endif
 
static void addenv(char *, char *);
 
static void addenv(char *key, char *value)
{
   char *str;
 
   str = malloc(strlen(key) + strlen(value) + 2);
   if (!str) {
      return;
   }
 
   strcpy(str, key);
   strcat(str, "=");
   strcat(str, value);

   putenv(str);
   /* there is intentionally no sge_free(&str) */
   return;
}

/* 
 * TODO: CLEANUP
 *
 * This function is DEPRECATED and should be removed with the next
 * major release.
 *
 * This function can't be used in multi threaded environments because it
 * might cause a deadlock in the executing qmaster thread.
 * Use sge_peopen_r() instead.
 */
pid_t sge_peopen(const char *shell, int login_shell, const char *command,
                 const char *user, char **env,  FILE **fp_in, FILE **fp_out,
                 FILE **fp_err, bool null_stderr)
{
   pid_t pid;
   int pipefds[3][2];
   const char *could_not = MSG_SYSTEM_EXECBINSHFAILED;
   const char *not_root = MSG_SYSTEM_NOROOTRIGHTSTOSWITCHUSER;
   int i;
   char arg0[256];
   char err_str[256];
   uid_t myuid;
 
   DENTER(TOP_LAYER, "sge_peopen");
 
   /* open pipes - close on failure */
   for (i=0; i<3; i++) {
      if (pipe(pipefds[i]) != 0) {
         while (--i >= 0) {
            close(pipefds[i][0]);
            close(pipefds[i][1]);
         }
         ERROR((SGE_EVENT, MSG_SYSTEM_FAILOPENPIPES_SS,
                command, strerror(errno)));
         DEXIT;
         return -1;
      }
   }

#if defined(SOLARIS)
   pid = sge_smf_contract_fork(err_str, 256);
#else
   pid = fork();
#endif
   if (pid == 0) {  /* child */
      int keep_open[6];
      keep_open[0] = 0;
      keep_open[1] = 1;
      keep_open[2] = 2;
      keep_open[3] = pipefds[0][0];
      keep_open[4] = pipefds[1][1];
      keep_open[5] = pipefds[2][1];
      sge_close_all_fds(keep_open, 6);
      /* shall we redirect stderr to /dev/null? */
      if (null_stderr) {
         /* open /dev/null */
         int fd = open("/dev/null", O_WRONLY);
         if (fd == -1) {
            sprintf(err_str, MSG_ERROROPENINGFILEFORWRITING_SS, "/dev/null", strerror(errno));
            sprintf(err_str, "\n");
            if (write(2, err_str, strlen(err_str)) != strlen(err_str)) {
               /* nothing we can do here - we are anyway about to exit */
            }
            SGE_EXIT(NULL, 1);
         }

         /* set stderr to /dev/null */
         close(2);
         if (dup(fd) == -1) {
            /* we have a serious problem here - nothing we can do but exit */
            SGE_EXIT(NULL, 1);
         }

         /* we don't need the stderr the pipe - close it */
         close(pipefds[2][1]);
      } else {
         /* redirect stderr to the pipe */
         close(2);
         if (dup(pipefds[2][1]) == -1) {
            /* we have a serious problem here - nothing we can do but exit */
            SGE_EXIT(NULL, 1);
         }
      }

      /* redirect stdin and stdout to the pipes */
      close(0);
      close(1);
      if (dup(pipefds[0][0]) == -1 ||
          dup(pipefds[1][1]) == -1) {
         /* we have a serious problem here - nothing we can do but exit */
         SGE_EXIT(NULL, 1);
      }

      if (user) {
         struct passwd *pw;
         struct passwd pw_struct;
         char *buffer;
         int size;

         size = get_pw_buffer_size();
         buffer = sge_malloc(size);
         if (!(pw=sge_getpwnam_r(user, &pw_struct, buffer, size))) {
            sprintf(err_str, MSG_SYSTEM_NOUSERFOUND_SS , user, strerror(errno));
            sprintf(err_str, "\n");
            if (write(2, err_str, strlen(err_str)) != strlen(err_str)) {
               /* nothing we can do here - we are anyway about to exit */
            }
            sge_free(&buffer);
            SGE_EXIT(NULL, 1);
         }

         myuid = geteuid();
         if (myuid != pw->pw_uid) {
            /* Only change user if we differ from the wanted user */
            if(myuid != SGE_SUPERUSER_UID) {
               if (write(2, not_root, sizeof(not_root)) != sizeof(not_root)) {
                  /* nothing we can do here - we are anyway about to exit */
               }
               sge_free(&buffer);
               SGE_EXIT(NULL, 1);
            }
            sprintf(err_str, "%s %d\n", pw->pw_name, (int)pw->pw_gid);
            if (write(2, err_str, strlen(err_str)) != strlen(err_str)) {
               /* TODO: required protocol step? If sending fails, exit? */
            }
#if !(defined(WIN32) || defined(INTERIX))  /* initgroups not called */
            if (initgroups(pw->pw_name, pw->pw_gid) != 0) {
               sprintf(err_str, MSG_SYSTEM_INITGROUPSFORUSERFAILED_ISS,
                       user, strerror(errno));
               sprintf(err_str, "\n");
               if (write(2, err_str, strlen(err_str)) != strlen(err_str)) {
                  /* nothing we can do here - we are anyway about to exit */
               }
               sge_free(&buffer);
               SGE_EXIT(NULL, 1);
            }
#endif /* WIN32 */
 
            if (sge_setuid(pw->pw_uid)) {
               sprintf(err_str, MSG_SYSTEM_SWITCHTOUSERFAILED_SS , user,
                     strerror(errno));
               sprintf(err_str, "\n");
               if (write(2, err_str, strlen(err_str)) != strlen(err_str)) {
                  /* nothing we can do here - we are anyway about to exit */
               }
               sge_free(&buffer);
               SGE_EXIT(NULL, 1);
            }
         }
 
         addenv("HOME", pw->pw_dir);
         addenv("SHELL", pw->pw_shell);
         addenv("USER", pw->pw_name);
         addenv("LOGNAME", pw->pw_name);
         addenv("PATH", SGE_DEFAULT_PATH);

         sge_free(&buffer);
      }
 
      if (login_shell) {
         strcpy(arg0, "-");
      } else {
         strcpy(arg0, "");
      }
      strcat(arg0, shell);
 
      if (env) {
         for(; *env; env++) {
            putenv(*env);
         }
      }
 
      execlp(shell, arg0, "-c", command, (char *) NULL);
 
      if (write(2, could_not, sizeof(could_not)) != sizeof(could_not)) {
         /* nothing we can do here - we are anyway about to exit */
      }
      SGE_EXIT(NULL, 1);
   } /* end child */
 
   if (pid < 0) {
      for (i=0; i<3; i++) {
         close(pipefds[i][0]);
         close(pipefds[i][1]);
      }
#if defined(SOLARIS)
      if (pid < -1) {
          ERROR((SGE_EVENT, MSG_SMF_FORK_FAILED_SS, "sge_peopen()", err_str));
      }
#endif
      /* fork could have failed, report it */
      ERROR((SGE_EVENT, MSG_SMF_FORK_FAILED_SS, "sge_peopen()", strerror(errno)));
      DRETURN(-1);
   }
 
   /* close the childs ends of the pipes */
   close(pipefds[0][0]);
   close(pipefds[1][1]);
   close(pipefds[2][1]);
  
   /* return filehandles for stdin and stdout */
   *fp_in  = fdopen(pipefds[0][1], "a");
   *fp_out = fdopen(pipefds[1][0], "r");

   /* is stderr redirected to /dev/null? */
   if (null_stderr) {
      /* close the pipe and return NULL as filehandle */
      close(pipefds[2][0]);
      *fp_err = NULL;
   } else {
      /* return filehandle for stderr */
      *fp_err = fdopen(pipefds[2][0], "r");
   }

   DRETURN(pid);
}

/****** uti/stdio/sge_peopen_r() ************************************************
*  NAME
*     sge_peopen_r() -- Advanced popen()
*
*  SYNOPSIS
*     pid_t sge_peopen_r(const char *shell, int login_shell,
*                        const char *command, const char *user,
*                        char **env, FILE **fp_in, FILE **fp_out,
*                        FILE **fp_err)
*
*  FUNCTION
*     Advanced popen() with additional parameters:
*        - free shell usage
*        - login shell if wanted
*        - user under which to start (for root only)
*        - stdin and stderr file pointers
*        - wait for exactly the process we started
*     File descriptors have to be closed with sge_peclose().
*
*     This function is reentrant as long as env is not provided to
*     this function. This means that the function can be used in 
*     multi thread processed as long as env is not used. 
*
*  INPUTS
*     const char *shell   - which shell to use
*     int login_shell     - make it a login shell?
*     const char *command - name of the program
*     const char *user    - user under which to start (for root only)
*     char **env          - env variables to add to child
*     FILE **fp_in        - file input stream
*     FILE **fp_out       - file output stream
*     FILE **fp_err       - file error stream
*
*  RESULT
*     pid_t - process id
*
*  NOTES
*     MT-NOTE: sge_peopen_r() is MT safe
*
*     DO NOT ADD ASYNC SIGNAL UNSAFE FUNCTIONS BETWEEN FORK AND EXEC
*     DUE TO THE FACT THAT THIS FUNCTION WILL BE USED IN QMASTER
*     (MULTITHREADED ENVIRONMENT) THIS MIGHT CAUSE A DEADLOCK 
*     IN A MASTER THREAD. 
*
*  SEE ALSO
*     uti/stdio/sge_peclose()
******************************************************************************/ 
pid_t sge_peopen_r(const char *shell, int login_shell, const char *command,
                   const char *user, char **env,  FILE **fp_in, FILE **fp_out,
                   FILE **fp_err, bool null_stderr)
{
   pid_t pid;
   int pipefds[3][2];
   int i;
   char arg0[256];
#if defined(SOLARIS)
   char err_str[256];
#endif
   struct passwd *pw = NULL;
   uid_t myuid;
   uid_t tuid;
 
   DENTER(TOP_LAYER, "sge_peopen_r");

   if (sge_has_admin_user()) {
      errno = 0;
      if (sge_switch2start_user()) {
         ERROR((SGE_EVENT, MSG_SWITCH_USER_S, strerror(errno)));
         DRETURN(-1);
      }
   }
   myuid = geteuid();
   tuid = myuid;

   /* 
    * open pipes - close on failure 
    */
   for (i = 0; i < 3; i++) {
      if (pipe(pipefds[i]) != 0) {
         while (--i >= 0) {
            close(pipefds[i][0]);
            close(pipefds[i][1]);
         }
         ERROR((SGE_EVENT, MSG_SYSTEM_FAILOPENPIPES_SS, command, strerror(errno)));
         if (sge_has_admin_user()) {
            sge_switch2admin_user();
         }
         DRETURN(-1);
      }
   }

   /*
    * set arg0 for exec call correctly to that
    * either a normal shell or a login shell will be started
    */
   if (login_shell) {
      strcpy(arg0, "-");
   } else {
      strcpy(arg0, "");
   }
   strcat(arg0, shell);
   DPRINTF(("arg0 = %s\n", arg0));
   DPRINTF(("arg1 = -c\n"));
   DPRINTF(("arg2 = %s\n", command));


   /*
    * prepare the change of the user which might be done after fork()
    * if a user name is provided.
    *
    * this has to be done before the fork() afterwards it might cause
    * a deadlock of the child because getpwnam() is not async-thread safe.
    */
   if (user) {
      struct passwd pw_struct;
      int size = get_pw_buffer_size();
      char *buffer = sge_malloc(size);

      /*
       * get information about the target user
       */
      if (buffer != NULL) {
         pw = sge_getpwnam_r(user, &pw_struct, buffer, size);
         if (pw == NULL) {
            ERROR((SGE_EVENT, MSG_SYSTEM_NOUSERFOUND_SS, user, strerror(errno)));
            sge_free(&buffer);
            if (sge_has_admin_user()) {
               sge_switch2admin_user();
            }
            DRETURN(-1);
         }
      } else {
         ERROR((SGE_EVENT, SFNMAX, MSG_UTI_MEMPWNAM));
         sge_free(&buffer);
         if (sge_has_admin_user()) {
            sge_switch2admin_user();
         }
         DRETURN(-1);
      }

      DPRINTF(("was able to resolve user\n"));

      /* 
       * only prepare change of user if target user is different from current one
       */
      if (myuid != pw->pw_uid) {
         if (myuid != SGE_SUPERUSER_UID) {
            DPRINTF(("only root is allowed to switch to a different user\n"));
            ERROR((SGE_EVENT, SFNMAX, MSG_SYSTEM_NOROOTRIGHTSTOSWITCHUSER));
            sge_free(&buffer);
            DRETURN(-2);
         }                             

         DPRINTF(("Before initgroups\n"));

#if !(defined(WIN32) || defined(INTERIX))  /* initgroups not called */
         if (initgroups(pw->pw_name, pw->pw_gid) != 0) {
            ERROR((SGE_EVENT, MSG_SYSTEM_INITGROUPSFORUSERFAILED_ISS,
                   user, strerror(errno)));
            sge_free(&buffer);
            SGE_EXIT(NULL, 1);
         }

         DPRINTF(("Initgroups was successful\n"));

#endif /* WIN32 */
      }
      DPRINTF(("user = %s\n", user));
      DPRINTF(("myuid = %d\n", (int)myuid));
      if (pw != NULL) {
         tuid = pw->pw_uid;
         DPRINTF(("target uid = %d\n", (int)tuid));
      }

      sge_free(&buffer);
   }

   DPRINTF(("Now process will fork\n"));

#if defined(SOLARIS)
   pid = sge_smf_contract_fork(err_str, 256);
#else
   pid = fork();
#endif

   /* 
    * in the child pid is 0 
    */
   if (pid == 0) {  
      /*
       * close all fd's except that ones mentioned in keep_open 
       */
      int keep_open[6];
      keep_open[0] = 0;
      keep_open[1] = 1;
      keep_open[2] = 2;
      keep_open[3] = pipefds[0][0];
      keep_open[4] = pipefds[1][1];
      keep_open[5] = pipefds[2][1];
      sge_close_all_fds(keep_open, 6);

      /* 
       * shall we redirect stderr to /dev/null? Then
       *    - open "/dev/null"
       *    - set stderr to "dev/null"
       *    - close the stderr-pipe
       * otherwise
       *    - redirect stderr to the pipe
       */
      if (null_stderr) {
         int fd = open("/dev/null", O_WRONLY);

         if (fd != -1) {
            close(2);
            if (dup(fd) == -1) {
               SGE_EXIT(NULL, 1);
            }
            close(pipefds[2][1]);
         } else {
            SGE_EXIT(NULL, 1);
         }
      } else {
         close(2);
         if (dup(pipefds[2][1]) == -1) {
            SGE_EXIT(NULL, 1);
         }
      }

      /* 
       * redirect stdin and stdout to the pipes 
       */
      close(0);
      close(1);
      if (dup(pipefds[0][0]) == -1 ||
          dup(pipefds[1][1]) == -1) {
         SGE_EXIT(NULL, 1);
      }

      if (pw != NULL) {
         int lret = sge_setuid(tuid);
         if (lret) {
            SGE_EXIT(NULL, 1);
         }
      }

      /*
       * set the environment if we got one as argument
       */
      if (env != NULL) {
         if (pw != NULL) {
            addenv("HOME", pw->pw_dir);
            addenv("SHELL", pw->pw_shell);
            addenv("USER", pw->pw_name);
            addenv("LOGNAME", pw->pw_name);
         }
         addenv("PATH", SGE_DEFAULT_PATH);
         for(; *env; env++) {
            putenv(*env);
         }
      }
      execlp(shell, arg0, "-c", command, (char *) NULL);
   }
 
   if (pid < 0) {
      for (i=0; i<3; i++) {
         close(pipefds[i][0]);
         close(pipefds[i][1]);
      }
#if defined(SOLARIS)
      if (pid < -1) {
          ERROR((SGE_EVENT, MSG_SMF_FORK_FAILED_SS, "sge_peopen()", err_str));
      }
#endif
      if (sge_has_admin_user()) {
         sge_switch2admin_user();
      }
      DRETURN(-1);
   }
 
   /* close the childs ends of the pipes */
   close(pipefds[0][0]);
   close(pipefds[1][1]);
   close(pipefds[2][1]);
  
   /* return filehandles for stdin and stdout */
   *fp_in  = fdopen(pipefds[0][1], "a");
   *fp_out = fdopen(pipefds[1][0], "r");

   /* is stderr redirected to /dev/null? */
   if (null_stderr) {
      /* close the pipe and return NULL as filehandle */
      close(pipefds[2][0]);
      *fp_err = NULL;
   } else {
      /* return filehandle for stderr */
      *fp_err = fdopen(pipefds[2][0], "r");
   }

   if (sge_has_admin_user()) {
      sge_switch2admin_user();
   }
   DRETURN(pid);
}
 
/****** uti/stdio/sge_peclose() ***********************************************
*  NAME
*     sge_peclose() -- pclose() call which is suitable for sge_peopen()
*
*  SYNOPSIS
*     int sge_peclose(pid_t pid, FILE *fp_in, FILE *fp_out, 
*                     FILE *fp_err, struct timeval *timeout)
*
*  FUNCTION
*     ???
*
*  INPUTS
*     pid_t pid               - pid returned by peopen()
*     FILE *fp_in
*     FILE *fp_out
*     FILE *fp_err
*     struct timeval *timeout
*
*  RESULT
*     int - exit code of command or -1 in case of errors
*
*  SEE ALSO
*     uti/stdio/peopen()
*
*  NOTES
*     MT-NOTE: sge_peclose() is MT safe
******************************************************************************/
int sge_peclose(pid_t pid, FILE *fp_in, FILE *fp_out, FILE *fp_err,
                struct timeval *timeout)
{
   int i, status;
   long timeout_us = timeout ? (timeout->tv_sec * 1000000L + timeout->tv_usec)
                             : 0;
 
   DENTER(TOP_LAYER, "sge_peclose");
 
   if (fp_in != NULL) {
      FCLOSE(fp_in);
   }
   if (fp_out != NULL) {
      FCLOSE(fp_out);
   }
   if (fp_err != NULL) {
      FCLOSE(fp_err);
   }  

   do {
      errno = 0;
      i = waitpid(pid, &status, timeout?WNOHANG:0);
      if (i==-1) {
         DEXIT;
         return errno;
      }
      if (i==0) { /* not yet exited */
         if (timeout_us <= 0) {
#ifdef WIN32 /* kill not called */
            DPRINTF(("killing not yet implemented\n"));
            timeout = NULL;
            /* kill(pid, SIGKILL); */
#else
            DPRINTF(("killing\n"));
            timeout = NULL;
            kill(pid, SIGKILL);
#endif /* WIN32 */
         } else {
            /* Spin less for longer timeouts; for timeout > 1s, sleep
               for 0.1s, n the loop, otherwise 1ms.  */
            int sleep_for = (timeout_us > 1000000) ? 100000 : 1000;

            DPRINTF(("%f seconds waiting for exit\n",
                     timeout->tv_sec + timeout->tv_usec / 1e6));
            timeout_us -= sleep_for;
            sge_usleep(sleep_for);
         }
      }
   } while (i != pid);
 
   if (status & 0xff) {    /* terminated by signal */
      DEXIT;
      return -1;
   }
 
   DEXIT;
   return (status&0xff00) >> 8;  /* return exitcode */
FCLOSE_ERROR:
   return -1;
}

void 
print_option_syntax(FILE *fp, const char *option, const char *meaning) 
{
   if (!meaning)
      fprintf(fp,"   %s\n", option);
   else
      fprintf(fp,"   %-40.40s %s\n",  option, meaning);
}

bool sge_check_stdout_stream(FILE *file, int fd)
{
   if (fileno(file) != fd) {
      return false;
   }

   if(fprintf(file, "%s", "") < 0) {
      return false;
   }

   return true;
}

