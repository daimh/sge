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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *  Copyright (C) 2011, 2012 Dave Love, University or Liverpool
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "uti/sge_binding_hlp.h"
#include "uti/sge_string.h"
#include "shepherd_binding.h"
/* TODO: (SH) get_path.h is a header-file of execd. We have to do a CLEANUP here. */
#include "get_path.h"

#if defined(LINUX)
#  include <grp.h>
#endif

#include "uti/config_file.h"
#include "uti/sge_dstring.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_stdio.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_stdio.h"
#include "uti/sge_signal.h"
#include "uti/sge_time.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_afsutil.h"
#include "uti/sge_os.h"
#include "uti/sge_pty.h"
#include "uti2/sge_cgroup.h"

#include "gdi/version.h"

#include "sgeobj/sge_job.h"
#include "sgeobj/sge_report.h"
#include "sgeobj/sge_feature.h"

#include "binding_support.h"

#if defined(IRIX)
#  include "sge_processes_irix.h"
#endif

#if defined(INTERIX)
#include "../../../utilbin/sge_passwd.h"
#include "wingrid/windows_gui.h"
#endif

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

#include "sge_ijs_comm.h"
#include "sge_shepherd_ijs.h"
#include "shepherd.h"
#include "sge_pset.h"
#include "sge_shepconf.h"
#include "sge_mt_init.h"
#include "sge_fileio.h"
#include "basis_types.h"
#include "qlogin_starter.h"
#include "sgedefs.h"
#include "exec_ifm.h"
#include "pdc.h"
#include "builtin_starter.h"
#include "err_trace.h"
#include "setrlimits.h"
#include "signal_queue.h"
#include "execution_states.h"
#include "msg_common.h"

#if defined(FREEBSD) || defined(DARWIN6) || defined(__OpenBSD__)
#   define sigignore(x) signal(x,SIG_IGN)
#endif

#define NO_CKPT          0x000
#define CKPT             0x001     /* set for all ckpt jobs                  */
#define CKPT_KERNEL      0x002     /* set for kernel level ckpt jobs         */
#define CKPT_USER        0x004     /* set for userdefined ckpt job           */
#define CKPT_TRANS       0x008     /* set for transparent ckpt jobs          */
#define CKPT_HIBER       0x010     /* set for hibernator ckpt jobs           */
#define CKPT_CPR         0x020     /* set for cpr ckpt jobs                  */
#if 0                              /* obsolete */
#define CKPT_CRAY        0x040     /* set for cray ckpt jobs                 */
#endif
#define CKPT_REST_KERNEL 0x080     /* set for all restarted kernel ckpt jobs */
#define CKPT_REST        0x100     /* set for all restarted ckpt jobs        */
#define CKPT_APPLICATION 0x200     /* application checkpointing              */

#define RESCHEDULE_EXIT_STATUS 99
#define APPERROR_EXIT_STATUS 100

/* global variables */
bool g_new_interactive_job_support = false;
int  g_noshell = 0;

char shepherd_job_dir[SGE_PATH_MAX];
int  received_signal=0;  /* set by signalhandler, when a signal arrives */


/* module variables */
static char ckpt_command[SGE_PATH_MAX], migr_command[SGE_PATH_MAX];
static char rest_command[SGE_PATH_MAX], clean_command[SGE_PATH_MAX];

static int notify;      /* 0 if no notify or # of seconds to delay signal */
static int exit_status_for_qrsh = 0;

static int ckpt_signal = 0;        /* signal to send to ckpt job */
static int signalled_ckpt_job = 0; /* marker if signalled a ckpt job */


/* function forward declarations */
static int start_child(const char *childname, char *script_file,
                       size_t lscript, pid_t *pidp,
                       int timeout, int ckpt_type, char *status_env);
static int wait_my_builtin_ijs_child(int pid, const char *childname, int timeout,
   ckpt_info_t *p_ckpt_info, ijs_fds_t *p_ijs_fds, struct rusage *rusage,
   dstring *err_msg);
static int do_wait(pid_t pid);

/* checkpointing functions */
static int check_ckpttype(void);
static void set_ckpt_params(int ckpt_type, char *ckpt_command, int ckpt_len,
   char *migr_command, int migr_len, char *clean_command, int clean_len,
   int *ckpt_interval);
static void set_ckpt_restart_command(const char *childname, int ckpt_type, 
   char *rest_command, int rest_len);

static void handle_job_pid(int ckpt_type, int pid, int *ckpt_pid);

static int start_async_command(const char *descr, char *cmd);
static void start_clean_command(char *cmd);

static pid_t start_token_cmd(int wait_for_finish, const char *cmd,
   const char *arg1, const char *arg2, const char *arg3);

/* overridable control methods */
static void verify_method(const char *method_name);
void shepherd_signal_job(pid_t pid, int sig);

/* signal functions */
static void signal_handler(int signal);
static void set_shepherd_signal_mask(void);
static void change_shepherd_signal_mask(void);
static void shepherd_deliver_signal(int sig,
                                    int pid,
                                    int *postponed_signal,
                                    int remaining_alarm,
                                    pid_t *ctrl_pid);
static int map_signal(int signal);
static void set_sig_handler(int sig_num);
static void shepherd_signal_handler(int dummy);
static void shepconf_deliver_signal_or_method(int sig, int pid, pid_t *ctrl_pid);
static void forward_signal_to_job(int pid, int timeout, int *postponed_signal, 
                       int remaining_alarm, pid_t ctrl_pid[3]);

static int do_prolog(int timeout, int ckpt_type);
static int do_epilog(int timeout, int ckpt_type);
static int do_pe_start(int timeout, int ckpt_type, pid_t *pe_pid);
static int do_pe_stop(int timeout, int ckpt_type, pid_t *pe_pid);
static void cpusetting(void);

/****** shepherd/handle_io_file() ********************************************
*  NAME
*     handle_io_file() -- opens the given file as the given user 
*
*  SYNOPSIS
*     static int handle_io_file(const char* file, const char* owner, bool rw)
*
*  FUNCTION
*
*  Opens the given file as the given user. Therefore the euid and the egid
*  will be change temporarily to those of the given user.
*
*  INPUTS
*     const char* file      - file which should be opened
*     const char* owner     - username which should become the owner
*     bool rw               - if true, file will be opened/created with
*                             write permissions.
*
* RESULT
*     int -  : fd of the created file
*          -1: file could not be created
*
* NOTE
* 
* The caller has to have SGE_SUPERUSER_UID as user-id.
*
*******************************************************************************/
static int handle_io_file(const char* file, const char* owner, bool rw) {
   int fd;
   uid_t jobuser_id;
   gid_t jobuser_gid;
   int old_euid = SGE_SUPERUSER_UID;
   int old_egid;

   if (file == NULL) {
      shepherd_trace("Invalid output-file!");
      return -1;
   }

   /* obtain user-id and group-id of job-owner */
   if (sge_user2uid(owner, &jobuser_id, &jobuser_gid, 1) != 0) {
      shepherd_trace("Cannot get user-id of %s", owner);
      return -1;
   }

   /* store current effective user-id */
   old_euid = geteuid();

   /* set effective user-id to root, because only root is allowed to change
    * the euid to any other than current the user-id.
    */
   if (sge_seteuid(SGE_SUPERUSER_UID) != 0) {
      shepherd_trace("Cannot become root due to %s", strerror(errno));
      return -1;
   }

   /* store current effective group-id */
   old_egid = getegid();
   /* set effective group-id to the group-id of job-owner */
   if (sge_setegid(jobuser_gid) != 0) {
      shepherd_trace("Cannot change egid due to %s", strerror(errno));
      return -1;
   }

   /* set effective user-id to the user-id of job-owner */
   if (sge_seteuid(jobuser_id) != 0) {
      shepherd_trace("Cannot become %s due to %s", owner, strerror(errno));
      return -1;
   }

   if (rw == true) {
      /* create the given file with the credentials of the job-owner */
      fd = SGE_OPEN3(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (fd == -1) {
         shepherd_trace("Cannot open %s due to %s", file, strerror(errno));
         return -1;
      }
   } else {
      /* open file as job-owner */
      fd = SGE_OPEN2(file, O_RDONLY); 
      if (fd == -1) {
         shepherd_trace("Cannot open %s.", file);
         return -1;
      }
   }

   /* reset egid and euid to the stored values */
   if (sge_seteuid(old_euid) != 0) {
      shepherd_trace("Cannot reset euid %s due to %s", owner, strerror(errno));
      SGE_CLOSE(fd);
      return -1;
   }
   if (sge_setegid(old_egid) != 0) {
      shepherd_trace("Cannot reset egid %s due to %s", owner, strerror(errno));
      SGE_CLOSE(fd);
      return -1;
   }

   return fd;
}

static int wait_until_parent_has_registered_to_server(int fd_pipe_to_child[])
{
   int  ret;
   char tmpbuf[100];

   memset(tmpbuf, 0, sizeof(tmpbuf));

   /* TODO: Why do we ingore SIGWINCH here? Why do we ignore only SIGWINCH here?*/
   sigignore(SIGWINCH);

   /* close parents end of our copy of the pipe */
   shepherd_trace("child: closing parents end of the pipe");
   close(fd_pipe_to_child[1]);
   fd_pipe_to_child[1] = -1;

   /* wait until parent has registered at the server */
   shepherd_trace("child: trying to read from parent through the pipe");
   ret = read(fd_pipe_to_child[0], tmpbuf, 11);
   if (ret <= 0) {
      shepherd_trace("child: error communicating with parent: %d, %s", 
                     errno, strerror(errno));
      ret = -1;
   } else {
      /* close other side of our copy of the pipe */
      close(fd_pipe_to_child[0]);
      fd_pipe_to_child[0] = -1;
      shepherd_trace("child: parent sent us '%s'", tmpbuf);
      sscanf(tmpbuf, "noshell = %d", &g_noshell);
      if (g_noshell != 0 && g_noshell != 1) {
         shepherd_trace("child: parent didn't register to server. %d, %s", 
                        errno, strerror(errno));
         ret = -1;
      }
   }
   return ret;
}

static int map_signal(int sig) 
{
   int ret = sig;

   if (sig == SIGTTIN) {
     /* SIGSTOP would also be delivered using SIGTTIN due to problems with
      * delivering SIGWINCH to shepherd, see CR 6623174
      */ 
      FILE *signal_file = fopen("signal", "r");

      if (signal_file != NULL) {
         if (fscanf(signal_file, "%d", &ret) != 1) {
            shepherd_trace("error reading signal file");
         }
         FCLOSE(signal_file);
      } 
   } else if (sig == SIGTTOU) {
      /* checkpoint signal value */
      ;
   } else if (sig == SIGUSR2) {
#ifdef SIGXCPU
      ret = SIGXCPU;
#else
      ret = SIGKILL;
#endif
   } else if (sig == SIGTSTP) {
      ret = SIGKILL;
   } else if (sig == SIGUSR1 || sig == SIGCONT) {
      /* identically */
      ;
   } else {
      /* unmapable signal */
      shepherd_trace("should map unmapable signal");
   }

   if (sig != ret) {
      shepherd_trace("mapped signal %s to signal %s", 
                     sge_sys_sig2str(sig), sge_sys_sig2str(ret));
   } else {
      shepherd_trace("no need to map signal %s", sge_sys_sig2str(sig));
   }
   return ret;
FCLOSE_ERROR:
   shepherd_trace(MSG_FILE_NOCLOSE_SS, "signal", strerror(errno));
   return ret;
}

static int do_prolog(int timeout, int ckpt_type)
{
   char *prolog;
   char command[10000];
   int exit_status;

   shepherd_state = SSTATE_BEFORE_PROLOG;

   /* start prolog */
   prolog = get_conf_val("prolog");
   if (strcasecmp("none", prolog)) {
      int i, n_exit_status = count_exit_status();

      replace_params(prolog, command, sizeof(command), prolog_epilog_variables);
      exit_status = start_child("prolog", command, sizeof(command),
                                NULL, timeout, ckpt_type, NULL);

      if (n_exit_status<(i=count_exit_status())) {
         shepherd_trace("exit states increased from %d to %d", n_exit_status, i);
         /* in this case the child didn't get to the exec call or it failed */
         shepherd_trace("failed starting prolog");
         return SSTATE_BEFORE_PROLOG;
      }

      if (exit_status) {
         /*
          * We should not exit here:
          *    We may neither start the job nor pe_start procedure, 
          *    but we should try to run the epilog procedure if it 
          *    exists, to allow some cleanup. The exit status of 
          *    this prolog failure may not get lost.
          */
         switch( exit_status ) {
            case RESCHEDULE_EXIT_STATUS:
               shepherd_state = SSTATE_AGAIN;
               break;
            case APPERROR_EXIT_STATUS:
               shepherd_state = SSTATE_APPERROR;
               break;
            default:
               shepherd_state = SSTATE_PROLOG_FAILED;
         }
         shepherd_error(0, "exit_status of prolog = %d", exit_status);
         return SSTATE_PROLOG_FAILED;
      }
   } else {
      shepherd_trace("no prolog script to start");
   }

   return 0;
}

static int do_epilog(int timeout, int ckpt_type)
{
   char *epilog;
   char command[10000];
   int exit_status;

   shepherd_state = SSTATE_BEFORE_EPILOG;

   epilog = get_conf_val("epilog");
   if (strcasecmp("none", epilog)) {
      int i, n_exit_status = count_exit_status();

      /* start epilog */
      replace_params(epilog, command, sizeof(command),
                     prolog_epilog_variables);
      exit_status = start_child("epilog", command, sizeof(command),
                                NULL, timeout, ckpt_type, NULL);
      if (n_exit_status<(i=count_exit_status())) {
         shepherd_trace("exit states increased from %d to %d", 
                                n_exit_status, i);
         /*
         ** in this case the child didn't get to the exec call or it failed
         ** the status that waitpid and finally start_child returns is
         ** reserved for the exit status of the job
         */
         shepherd_trace("failed starting epilog");
         return SSTATE_BEFORE_EPILOG;
      }

      if (exit_status) {
         switch( exit_status ) {
            case RESCHEDULE_EXIT_STATUS:
               shepherd_state = SSTATE_AGAIN;
               break;
            case APPERROR_EXIT_STATUS:
               shepherd_state = SSTATE_APPERROR;
               break;
            default:
               shepherd_state = SSTATE_EPILOG_FAILED;
         }
         shepherd_error(0, "exit_status of epilog = %d", exit_status);
         return SSTATE_EPILOG_FAILED;
      }
   }
   else
      shepherd_trace("no epilog script to start");
   
   return 0;
}

static int do_pe_start(int timeout, int ckpt_type, pid_t *pe_pid)
{
   char *pe_start;
   char command[10000];
   int exit_status;

   shepherd_state = SSTATE_BEFORE_PESTART;

   /* start pe_start */
   pe_start = get_conf_val("pe_start");
   if (strcasecmp("none", pe_start)) {
      int i, n_exit_status = count_exit_status();

      shepherd_trace("%s", pe_start);
      replace_params(pe_start, command, sizeof(command), pe_variables);
      shepherd_trace("%s", command);

      /* 
         starters of parallel environments may not get killed 
         in case of success - so we save their pid for later use
      */
      exit_status = start_child("pe_start", command, sizeof(command),
                                pe_pid, timeout, ckpt_type, NULL);
      if (n_exit_status<(i=count_exit_status())) {
         shepherd_trace("exit states increased from %d to %d", n_exit_status, i);
         /*
         ** in this case the child didn't get to the exec call or it failed
         ** the status that waitpid and finally start_child returns is
         ** reserved for the exit status of the job
         */
         shepherd_trace("failed starting pe_start");
         return SSTATE_BEFORE_PESTART;
      }
      if (exit_status) {
         /*
          * We should not exit here:
          *    We may not start the job, but we should try to run 
          *    the pe_stop and epilog procedures if they exist, to 
          *    allow some cleanup. The exit status of this pe_start
          *    failure may not get lost.
          */
         switch( exit_status ) {
            case RESCHEDULE_EXIT_STATUS:
               shepherd_state = SSTATE_AGAIN;
               break;
            case APPERROR_EXIT_STATUS:
               shepherd_state = SSTATE_APPERROR;
               break;
            default:
               shepherd_state = SSTATE_PESTART_FAILED;
         }

         shepherd_error(0, "exit_status of pe_start = %d", exit_status);
         return SSTATE_PESTART_FAILED;
      }
   } else {
      shepherd_trace("no pe_start script to start");
   }

   return 0;
}

static int do_pe_stop(int timeout, int ckpt_type, pid_t *pe_pid)
{
   char *pe_stop;
   char command[10000];
   int exit_status;
   
   pe_stop = get_conf_val("pe_stop");
   if (strcasecmp("none", pe_stop)) {
      int i, n_exit_status = count_exit_status();

      shepherd_state = SSTATE_BEFORE_PESTOP;

      shepherd_trace("%s", pe_stop);
      replace_params(pe_stop, command, sizeof(command), pe_variables);
      shepherd_trace("%s", command);
      exit_status = start_child("pe_stop", command, sizeof(command),
                                NULL, timeout, ckpt_type, NULL);

      /* send a kill to pe_start process
       *
       * For now, don't do it !!!! This murders the PVMD, and if
       * another job uses it ...
       * shepherd_signal_job(-*pe_pid, SIGKILL);
       */

      if (n_exit_status<(i=count_exit_status())) {
         shepherd_trace("exit states increased from %d to %d", n_exit_status, i);
         /*
         ** in this case the child didn't get to the exec call or it failed
         ** the status that waitpid and finally start_child returns is
         ** reserved for the exit status of the job
         */
         shepherd_trace("failed starting pe_stop");
         return SSTATE_BEFORE_PESTOP;
      }

      if (exit_status) {
         /*
          * We should not exit here:
          *    We should try to run the epilog procedure if it 
          *    exists, to allow cleanup. The exit status of this 
          *    pe_start failure may not get lost.
          */
         switch( exit_status ) {
            case RESCHEDULE_EXIT_STATUS:
               shepherd_state = SSTATE_AGAIN;
               break;
            case APPERROR_EXIT_STATUS:
               shepherd_state = SSTATE_APPERROR;
               break;
            default:
               shepherd_state = SSTATE_PESTOP_FAILED;
         }

         shepherd_error(0, "exit_status of pe_stop = %d", exit_status);
         return SSTATE_PESTOP_FAILED;
      }
   } else {
      shepherd_trace("no pe_stop script to start");
   }

   return 0;
}

/************************************************************************
 This is the shepherd. For each job we start under the control of SGE
 we first start a shepherd.

 shepherd is responsible for:

 - setting up the environment for the job
 - starting the job
 - retrieve the job usage information for the finished job
 - passing through of signals sent by execd to the job

 Strategy: Do all communication with the execd per filesystem. This gives
           us a chance to see what's happening from outside. This decouples
           shepherd from the execd. In filesystems we trust, in communication
           we do not.

 Each Shepherd has its own directory. The caller has to cd to this directory
 before starting sge_shepherd.

 Input to the shepherd:
    "config" file   holds configuration data (config_file.c)
    "signal" file   when execd wants a signal to be delivered

 Output from shepherd:
    "trace" file    reports activities of shepherd (err_trace.c)
    "pid" file      contains pid of shepherd
    "error" file    contains error strings. This can be used by execd for
                    problem reports. (err_trace.c):w

    "exit_status" file
                    contains the exit status of shepherd after termination.

 

 How can a caller see what's going on:

 If the shepherd was started, the "pid" file exists. This file contains the
 pid of the shepherd and can be used to search the process table.

 If shepherd can't be found in the process table and the "exit_status" file
 does not exist the shepherd terminated abnormally. Only kill(-9) or a system
 breakdown should cause this.

 If the "exit_status" file exists the shepherd has terminated normally.
 If exit_status==0 everything is fine. If exit_status!=0 a problem occurred.
 The error file should give hints about what happened.
 exit_status values: see shepherd_states header file

 ************************************************************************/

static void signal_handler(int signal) 
{
   /* may not log in signal handler 
      as long as Async-Signal-Safe functions such as fopen() are used in 
      shepherd logging code */
#if 0
   shepherd_trace("received signal %s", sge_sys_sig2str(signal));
#endif

   received_signal = signal;
}

static void show_shepherd_version(void) {

   printf("%s %s\n", GE_SHORTNAME, GDI_VERSION);
   printf("%s %s [options]\n", MSG_GDI_USAGE_USAGESTRING , "sge_shepherd");
   printf("   %-40.40s %s\n", MSG_GDI_USAGE_help_OPT , MSG_GDI_UTEXT_help_OPT);

}

int main(int argc, char **argv) 
{
   char err_str[2*SGE_PATH_MAX+128];
   char *admin_user;
   char *script_file;
   char *pe, *cp;
   pid_t pe_pid;
   int script_timeout;
   int exit_status = 0;
   int uid, i, ret;
   pid_t pid;
   SGE_STRUCT_STAT buf;
   int ckpt_type;
   int return_code = 0;
   int run_epilog, run_pe_stop;
   dstring ds;
   char buffer[256];

   init_topology();
   sge_mt_init();

   if (argc >= 2) {
      if ( strcmp(argv[1],"-help") == 0) {
         show_shepherd_version();
         return 0;
      }
   }
#if defined(INTERIX) && defined(SECURE)
   sge_init_shared_ssl_lib();
#endif
   shepherd_trace_init( );

   sge_dstring_init(&ds, buffer, sizeof(buffer));

   shepherd_trace("shepherd called with uid = "uid_t_fmt", euid = "uid_t_fmt, 
                  getuid(), geteuid());

   shepherd_state = SSTATE_BEFORE_PROLOG;
   if (getcwd(shepherd_job_dir, SGE_PATH_MAX-1) == NULL) {
      shepherd_error(1, "can't read cwd - getcwd failed: %s", strerror(errno));
   }

   if (argc >= 2 && !strcmp("-bg", argv[1])) {
      foreground = 0;   /* no output to stderr */
   }

   set_shepherd_signal_mask();

   init_cgroups();
      
   config_errfunc = shepherd_error_ptr;

   /*
    * read configuration file
    * this is done because we need to know the name of the admin user
    * for file creation
    */
   if ((ret=read_config("config"))) {

      /* if we can't read the config file, try to switch to admin user
       * that our error/trace message can be successfully logged
       * this is done in case where our getuid() is root and our
       * geteuid() is a normal user id
       */
       if(getuid() == SGE_SUPERUSER_UID &&
          geteuid() != SGE_SUPERUSER_UID) { 
          char name[128];
          if (!sge_uid2user(geteuid(), name, sizeof(name), MAX_NIS_RETRIES)) {
             sge_set_admin_username(name, NULL, 0);
             sge_switch2admin_user();
          }
      }
      if (ret == 1) {
         shepherd_error(1, "can't read configuration file: %s", strerror(errno));
      } else {
         shepherd_error(1, "can't read configuration file: malloc() failure");
      }
   }

   {
      char *tmp_rsh_daemon;
      char *tmp_rlogin_daemon;
      char *tmp_qlogin_daemon;
      
      /*
       * Check if we have to use the old or the new (builtin)
       * interactive job support.
       * TODO: allow dynamic configuration for each job?
       */
      config_errfunc = NULL;
      script_file = get_conf_val("script_file");
      if (script_file != NULL
          && strcasecmp(script_file, JOB_TYPE_STR_QRSH) == 0) {
         tmp_rsh_daemon = get_conf_val("rsh_daemon");
         if (tmp_rsh_daemon != NULL
             && strcasecmp(tmp_rsh_daemon, "builtin") == 0) {
            g_new_interactive_job_support = true;
            shepherd_trace("rsh_daemon = %s", tmp_rsh_daemon);
         }
      }

      if (script_file != NULL
          && strcasecmp(script_file, JOB_TYPE_STR_QRLOGIN) == 0) {
         tmp_rlogin_daemon = get_conf_val("rlogin_daemon");
         if (tmp_rlogin_daemon != NULL
             && strcasecmp(tmp_rlogin_daemon, "builtin") == 0) {
            g_new_interactive_job_support = true;
            shepherd_trace("rlogin_daemon = %s", tmp_rlogin_daemon);
         }
      }

      if (script_file != NULL
          && strcasecmp(script_file, JOB_TYPE_STR_QLOGIN) == 0) {
         tmp_qlogin_daemon = get_conf_val("qlogin_daemon");
         if (tmp_qlogin_daemon != NULL
             && strcasecmp(tmp_qlogin_daemon, "builtin") == 0) {
            g_new_interactive_job_support = true;
            shepherd_trace("qlogin_daemon = %s", tmp_qlogin_daemon);
         }
      }
      config_errfunc = shepherd_error_ptr;
   }
   
#if defined( INTERIX )
   wl_set_use_sgepasswd((bool)atoi(get_conf_val("enable_windomacc")));
#endif

   /* init admin user stuff */
   admin_user = get_conf_val("admin_user");
   if (sge_set_admin_username(admin_user, err_str, sizeof(err_str))) {
      shepherd_error(1, "%s", err_str);
   }
   errno = 0;
   if (sge_switch2admin_user()) {
      shepherd_error(1, "can't switch to admin user: sge_switch2admin_user() failed: %s",
                     strerror(errno));
   }

   /* finalize initialization of shepherd_trace - give the trace file
    * to the job owner so not only root/admin user but also he can use
    * the trace file later.
    */
   shepherd_trace_chown(get_conf_val("job_owner"));
   /*
    * Close trace file to force a new open() with super user credentials for
    * NFS writes. Otherwise we will see EACCESS for all writes from now on
    * until start bracketing file operations with seteuid(0), seteuid(admin_user).
    * The next shepherd_trace() will open the trace file automatically.
    */
   shepherd_trace_exit();

   if (shepherd_trace("starting up %s", feature_get_product_name(FS_VERSION, &ds)) != 0) {
      shepherd_error(1, "can't write to \"trace\" file");
   }
 
   /* do not start job in cases of wrong 
      configuration of job control methods */
   verify_method("terminate_method");
   verify_method("suspend_method");
   verify_method("resume_method");

   /* write our pid to file */
   pid = getpid();

   if(!shepherd_write_pid_file(pid, &ds)) {
      shepherd_error(1, "%s", sge_dstring_get_string(&ds));
   }

   uid = getuid();

   if (!sge_is_start_user_superuser()) {
      shepherd_trace("warning: starting not as superuser (uid=%d)", uid);
   }

   /* Shepherd in own process group */
   i = setpgid(pid, pid);
   shepherd_trace("setpgid("pid_t_fmt", "pid_t_fmt") returned %d", pid, pid, i);

   if ((ckpt_type = check_ckpttype()) == -1) {
      shepherd_error(1, "checkpointing with incomplete specification or "
                     "unknown interface requested");
   }

   script_timeout = atoi(get_conf_val("script_timeout"));
   notify = atoi(get_conf_val("notify"));

   /*
    * Create processor set
    */
   sge_pset_create_processor_set();

   /* 
    * Perform core binding (do not use processor set together with core binding) 
    */ 
   do_core_binding();

   cpusetting();

   /*
    * this blocks sge_shepherd until the first time the token is
    * successfully set, then we start the sge_coshepherd in background
    */
   if ((cp = search_conf_val("use_afs")) && atoi(cp) > 0) {
      char *coshepherd = search_conf_val("coshepherd");
      char *set_token_cmd = search_conf_val("set_token_cmd");
      char *job_owner = search_conf_val("job_owner");
      char *token_extend_time = search_conf_val("token_extend_time");
      char *tokenbuf;

      shepherd_trace("beginning AFS setup");
      if (!(coshepherd && set_token_cmd && job_owner && token_extend_time)) {
         shepherd_state = SSTATE_AFS_PROBLEM;
         shepherd_error(1, "AFS with incomplete specification requested");
      }
      if ((tokenbuf = sge_read_token(TOKEN_FILE)) == NULL) {
         shepherd_state = SSTATE_AFS_PROBLEM;
         shepherd_error(1, "can't read AFS token");
      }  else { 

         if (sge_afs_extend_token(set_token_cmd, tokenbuf, job_owner,
                                  atoi(token_extend_time), err_str,
                                  sizeof(err_str))) {
            shepherd_state = SSTATE_AFS_PROBLEM;
            shepherd_error(1, "%s", err_str);
         }

         shepherd_trace("%s", err_str);
         shepherd_trace("successfully set AFS token");

         memset(tokenbuf, 0, strlen(tokenbuf));
         sge_free(&tokenbuf);


         if ((coshepherd_pid = start_token_cmd(0, coshepherd, set_token_cmd,
                                               job_owner, token_extend_time)) < 0) {
            shepherd_state = SSTATE_AFS_PROBLEM;
            shepherd_error(1, "can't start coshepherd");
         }

         shepherd_trace("AFS setup done");
      }
   }

   pe = get_conf_val("pe");
   if (!strcasecmp(pe, "none")) {
      pe = NULL;
   }

   run_epilog = 1;
   if ((exit_status = do_prolog(script_timeout, ckpt_type))) {
      if (exit_status == SSTATE_BEFORE_PROLOG)
         run_epilog = 0;
   } else if (pending_sig(SIGTTOU)) {
      /* got a signal during prolog causing job to migrate? */ 
      shepherd_trace("got SIGMIGR before job start");
      exit_status = 0; /* no error */
      create_checkpointed_file(0);
   } else if (pending_sig(SIGKILL)) {
      /* got a signal during prolog causing job exit ? */ 
      shepherd_trace("got SIGKILL before job start");
      exit_status = 0; /* no error */
   } else {
      /* start pe_start */
      run_pe_stop = 1;
      if (pe && (exit_status = do_pe_start(script_timeout, ckpt_type, &pe_pid))) {
         if (exit_status == SSTATE_BEFORE_PESTART) {
            run_pe_stop = 0;
         }
      } else {
         shepherd_state = SSTATE_BEFORE_JOB;
         /* start job script */
         script_file = get_conf_val("script_file");
         if (strcasecmp("none", script_file)) {
            if (pending_sig(SIGKILL)) { 
               shepherd_trace("got SIGKILL before job start");
               exit_status = 0; /* no error */
            } else if (pending_sig(SIGTTOU)) { 
               shepherd_trace("got SIGMIGR before job start");
               create_checkpointed_file(0);
               exit_status = 0; /* no error */
            } else {
               char command[SGE_PATH_MAX];
               sge_strlcpy(command, script_file, sizeof(command));
               exit_status = start_child("job", command, sizeof(command), NULL,
                                         0, ckpt_type, "SGE_JOBEXIT_STAT");

               if (count_exit_status()>0) {
                  /*
                  ** in this case the child didn't get to the exec call or it failed
                  ** the status that waitpid and finally start_child returns is
                  ** reserved for the exit status of the job
                  */

                  /* we may not give up here 
                     start pe_stop and epilog before exiting */
                  shepherd_trace("failed starting job");
               } else {
                  bool call_error_impl = false;

                  switch( exit_status ) {
                     case RESCHEDULE_EXIT_STATUS:
                        if( !atoi(get_conf_val("forbid_reschedule"))) {
                           shepherd_state = SSTATE_AGAIN;
                           call_error_impl = true;
                        }
                        break;
                     case APPERROR_EXIT_STATUS:
                        if( !atoi(get_conf_val("forbid_apperror"))) {
                           shepherd_state = SSTATE_APPERROR;
                           call_error_impl = true;
                        }
                        break;
                  }
                  if( call_error_impl ) {
                     shepherd_error(0, "exit_status of job start = %d", 
                                    exit_status);
                  }
               } 
            }
         } else {
            shepherd_trace("no script to start");
         }

         clear_queued_signals();
      }

      /* start pe_stop */
      if (pe && run_pe_stop) {
         do_pe_stop(script_timeout, ckpt_type, &pe_pid);
      }
   }

   if (run_epilog) {
      do_epilog(script_timeout, ckpt_type);
   }

   /*
    * Free previously created processor set
    */
   sge_pset_free_processor_set();

   /* Clean up our cpuset */
   {
      u_long32 job = atoi(get_conf_val("job_id"));
      u_long32 task = MAX(1, atoi(get_conf_val("ja_task_id")));

      if (!empty_shepherd_cpuset (job, task, getpid()))
         shepherd_trace("failed to empty cpuset: "SFN, strerror(errno));
   }

   if (coshepherd_pid > 0) {
      shepherd_trace("sending SIGTERM to sge_coshepherd");
      sge_switch2start_user();
      kill(coshepherd_pid, SIGTERM);
      sge_switch2admin_user();
   }

   if (!SGE_STAT("exit_status", &buf) && buf.st_size) {
      shepherd_read_exit_status_file(&return_code);
   } else {
      /* ensure an exit status file exists */
      shepherd_write_exit_status("0");
      return_code = 0;
   }

   if (search_conf_val("qrsh_control_port") != NULL) {
      shepherd_write_shepherd_about_to_exit_file();
      if (g_new_interactive_job_support == false) {
         write_exit_code_to_qrsh(exit_status_for_qrsh);
      } else {
         shepherd_trace("writing exit status to qrsh: %d", exit_status_for_qrsh);
         close_parent_loop(exit_status_for_qrsh);
      }
   }

   shepherd_trace_exit( );
   return return_code;
}

/********************************************************************
 start child process
 returns exit status of script

PARAMETER

   childname
      "prolog", "job", or "epilog"

   script_file
      Executable to start

   lscript
      Size of script_file

   pidp 

      If NULL the process gets killed after child exit
      to ensure nothing hangs around.

      If non NULL a kill is only sent in case of an error.
      In case of no errors the pid is saved in *pidp 
      for later use.

   timeout

   ckpt_type

   status_env
      Name of an environment to store the job exit status

 *******************************************************************/
static int start_child(const char *childname, /* prolog, job, epilog */
                       char *script_file, size_t lscript, pid_t *pidp, int timeout,
                       int ckpt_type, char *status_env) {
   SGE_STRUCT_STAT buf;
   struct rusage rusage;
   u_long32 start_time, end_time;
   u_long32 wait_status = 0;
   int pid, status, core_dumped, ret;
   int child_signal = 0;
   int exit_status = 0;
   int wexit_flag_true = 1; /* to please IRIX compiler */
   int fd_pipe_in[2] = {-1, -1};
   int fd_pipe_out[2] = {-1, -1};
   int fd_pipe_err[2] = {-1, -1};
   int fd_pipe_to_child[2] = {-1, -1};
   int fd_pty_master = -1;
   ternary_t use_pty = UNSET;
   dstring err_msg = DSTRING_INIT;
   bool is_interactive = false;
   ckpt_info_t ckpt_info = {0, 0, 0};
#if defined(IRIX)
   ash_t ash = 0;
#endif

   ckpt_info.type = ckpt_type;

   /* Do we have an interactive job? */
   if (strcasecmp(script_file, JOB_TYPE_STR_QLOGIN) == 0 ||
       strcasecmp(script_file, JOB_TYPE_STR_QRSH) == 0 ||
       strcasecmp(script_file, JOB_TYPE_STR_QRLOGIN) == 0) {
      is_interactive = true;
   }

   /* Try to read "pty" from config, if it is not there or set to "2", use default */
   {
      char *conf_val = get_conf_val("pty");
      if (conf_val != NULL) {
         sscanf(conf_val, "%d", (int*)&use_pty);
      }
      if (use_pty == UNSET) {  /* use default */
         if (strcasecmp(script_file, JOB_TYPE_STR_QRSH) == 0) {
            /* by default, qrsh <no command> doesn't use a pty */
            use_pty = NO;
         } else {
            /* by default, qrsh <command> and qlogin use a pty */
            use_pty = YES;
         }
      }
   }
   
   /* Don't care about checkpointing for "commands" other than "job" */
   if (strcmp(childname, "job")) {
      ckpt_info.type = 0;
   }

   if (ISSET(ckpt_info.type, CKPT_REST_KERNEL) && strcmp(childname, "job") == 0) {
      set_ckpt_restart_command(childname, ckpt_info.type,
                               rest_command, sizeof(rest_command) -1);
      shepherd_trace("restarting job from checkpoint arena");

#if defined(IRIX)
      /* reuse old osjobid for the migrated job and forward this one to ptf */
      shepherd_write_osjobid_file(get_conf_val("ckpt_osjobid"));

#if defined(IRIX)
      sscanf(get_conf_val("ckpt_osjobid"), "%lld", &ash);
      shepherd_trace("reusing old array session handle %lld", ash);
#endif
#endif

      shepherd_trace("restarting job from checkpoint arena");
      pid = start_async_command("restart", rest_command);
   }
   else { /* not job or job and not checkpointing */
      if (g_new_interactive_job_support == false || !is_interactive) {
         if (use_pty == YES && strcasecmp(script_file, JOB_TYPE_STR_QSH) != 0) {
            char* job_owner = get_conf_val("job_owner");
            uid_t jobuser_id; gid_t jobuser_gid;
            if (job_owner == NULL) {
               shepherd_trace("Cannot determine the job-owner!");
               return 1;
            }
            if (sge_user2uid(job_owner, &jobuser_id, &jobuser_gid, 1) != 0) {
               shepherd_trace("Cannot get user-id of %s", job_owner);
               return 1;
            }
            shepherd_trace("calling fork_pty()");
            pid = fork_pty(&fd_pty_master, fd_pipe_err, &err_msg, jobuser_id);
         } else {
            pid = fork();
         }
      } else {
         /*
          * Create a pipe to tell child when communication to client is set up
          * and child can start the job.
          */
         ret = pipe(fd_pipe_to_child);
         shepherd_trace("pipe to child uses fds %d and %d",
                        fd_pipe_to_child[0], fd_pipe_to_child[1]);
         if (ret < 0) {
            shepherd_error(1, "can't create pipe to child! %s (%d)",
                           strerror(errno), errno);
            return 1;
         }

         /*
          * Open a pty and do the fork. The parent gets the pty master, the
          * child gets the pty slave. The slave pty becomes stdin/stdout/stderr
          * of the child.
          */
         if (use_pty == YES) {
            char* job_owner = get_conf_val("job_owner");
            uid_t jobuser_id; gid_t jobuser_gid;
            if (job_owner == NULL) {
               shepherd_trace("Cannot determine the job-owner!");
               return 1;
            }
            if (sge_user2uid(job_owner, &jobuser_id, &jobuser_gid, 1) != 0) {
               shepherd_trace("Cannot get user-id of %s", job_owner);
               return 1;
            }
            shepherd_trace("calling fork_pty()");
            pid = fork_pty(&fd_pty_master, fd_pipe_err, &err_msg, jobuser_id);
         } else {
            shepherd_trace("calling fork_no_pty()");
            pid = fork_no_pty(fd_pipe_in, fd_pipe_out, fd_pipe_err, &err_msg);
         }
      } 

      if (pid==0) { /* child */
         if (g_new_interactive_job_support == true && is_interactive) {
            ret = wait_until_parent_has_registered_to_server(fd_pipe_to_child);
            if (ret < 0) {
               return ret;
            }
         }
         shepherd_trace("child: starting son(%s, %s, 0, %ld);", childname,
                        script_file, (unsigned long) lscript);
         son(childname, script_file, 0, lscript);
      }
   }

   if (pid == -1) {
      if (g_new_interactive_job_support == false || !is_interactive) {
         shepherd_error(1, "can't fork \"%s\"", childname);
      } else {
         shepherd_error(1, "can't fork \"%s\": %s", 
            childname, sge_dstring_get_string(&err_msg));
      }
   }

   /* parent */
   shepherd_trace("parent: forked \"%s\" with pid %d", childname, pid);

   change_shepherd_signal_mask();
   
   start_time = sge_get_gmt();

   /* Write pid to job_pid file and set ckpt_pid to original job pid 
    * Kill job if we can't write job_pid file and exit with error
    * sets ckpt_pid to 0 for non kernel level checkpointing jobs
    */
   if (!strcmp(childname, "job"))
      handle_job_pid(ckpt_info.type, pid, &(ckpt_info.pid));

   /* Does not affect pe/prolog/epilog etc. since ckpt_type is set to 0 */
   set_ckpt_params(ckpt_info.type,
                   ckpt_command, 
                   sizeof(ckpt_command) - 1,
                   migr_command,
                   sizeof(migr_command) - 1,
                   clean_command,
                   sizeof(clean_command) - 1,
                   &(ckpt_info.interval));
   
   if (received_signal > 0 && received_signal != SIGALRM) {
      int sig = map_signal(received_signal);
      int tmp_ret = add_signal(sig);

      received_signal = 0;
      if (tmp_ret) {
         shepherd_trace("failed to store received signal");
      }
   }
 
   if (timeout) {
      shepherd_trace("using signal delivery delay of %d seconds", timeout);
      alarm(timeout);
   } else {
      int number_of_signals = get_n_sigs();

      if (number_of_signals > 0) {
         shepherd_trace("there are %d signals to deliver. Wait until "
                                "job has been started.", number_of_signals);
         alarm(10);
      }
   }

   if (ckpt_info.type) {
      shepherd_trace("parent: %s-pid: %d - ckpt_pid: %d - "
                     "ckpt_interval: %d - ckpt_signal %d", childname,
                     pid, ckpt_info.pid, ckpt_info.interval, ckpt_signal);
   } else {
      shepherd_trace("parent: %s-pid: %d", childname, pid);
   }

   if (g_new_interactive_job_support == false || !is_interactive) {
      /* Wait until child finishes ----------------------------------------*/
      status = wait_my_child(pid, childname, timeout, &ckpt_info, &rusage,
                             fd_pty_master, fd_pipe_err[0]);
   } else { /* g_new_interactive_job_support == true && is_interactive */
      ijs_fds_t ijs_fds;

      ijs_fds.pty_master    = fd_pty_master;
      ijs_fds.pipe_in       = fd_pipe_in[1];        /* write end of pipe */
      ijs_fds.pipe_out      = fd_pipe_out[0];       /* read end of pipe */
      ijs_fds.pipe_err      = fd_pipe_err[0];       /* read end of pipe */
      ijs_fds.pipe_to_child = fd_pipe_to_child[1];  /* write end of pipe */

      /* close the not used ends of the pipes */
      close(fd_pipe_in[0]);       fd_pipe_in[0] = -1;
      close(fd_pipe_out[1]);      fd_pipe_out[1] = -1;
      close(fd_pipe_err[1]);      fd_pipe_err[1] = -1;
      close(fd_pipe_to_child[0]); fd_pipe_to_child[0] = -1;

      /* wait blocking until the child process has ended */
      status = wait_my_builtin_ijs_child(pid, childname, timeout,
                  &ckpt_info, &ijs_fds, &rusage, &err_msg);
   }
   alarm(0);
   end_time = sge_get_gmt();

   shepherd_trace("reaped \"%s\" with pid %d", childname, pid);

   if (ckpt_info.type) {
      /* 
       * empty file is a hint to reschedule that job. If we already have a
       * checkpoint in the arena there is a dummy string in the file
       */
      if (!checkpointed_file_exists()) {
         create_checkpointed_file(0);
      }
   }

   if (WIFSIGNALED(status)) {
      shepherd_trace("%s exited due to signal", childname);

      if (ckpt_info.type != 0 && signalled_ckpt_job == false) {
         unlink("checkpointed");
         shepherd_trace("%s exited due to signal but not due to checkpoint", childname);
         if (ISSET(ckpt_info.type, CKPT_KERNEL)) {
            shepherd_trace("starting ckpt clean command");
            start_clean_command(clean_command);
         }
      }

      child_signal = WTERMSIG(status);
#ifdef WCOREDUMP
      core_dumped = WCOREDUMP(status);
#else
      core_dumped = status & 80;
#endif
      if (timeout && child_signal == SIGALRM) {
         shepherd_trace("%s expired timeout of %d seconds and was "
                        "killed%s", childname, timeout, 
                        core_dumped ? " (core dumped)": "");
      } else {
         shepherd_trace("%s signaled: %d%s", childname, 
                        child_signal, 
                        core_dumped ? " (core dumped)": "");
      }

      exit_status = 128 + child_signal;

      wait_status = SGE_SET_WSIGNALED(wait_status, wexit_flag_true);
      wait_status = SGE_SET_WCOREDUMP(wait_status, core_dumped);
      wait_status = SGE_SET_WSIGNAL(wait_status, sge_map_signal(child_signal));
   } else {
      shepherd_trace("%s exited not due to signal", childname);

      if (ckpt_info.type != 0 && signalled_ckpt_job == false) {
         shepherd_trace("checkpointing job exited normally");
         unlink("checkpointed");
         if (ISSET(ckpt_info.type, CKPT_KERNEL)) {
            shepherd_trace("starting ckpt clean command");
            start_clean_command(clean_command);
         }
      }

      if (WIFEXITED(status)) {
         exit_status = WEXITSTATUS(status);
         shepherd_trace("%s exited with status %d%s",
            childname, exit_status, 
            (exit_status==RESCHEDULE_EXIT_STATUS || exit_status==APPERROR_EXIT_STATUS)?
            " -> rescheduling":"");

         wait_status = SGE_SET_WEXITED(wait_status, wexit_flag_true);
         wait_status = SGE_SET_WEXITSTATUS(wait_status, exit_status);

      } else {
         /* should be virtually impossible, see wait_my_child() why */
         exit_status = -1;
         shepherd_trace("%s did not exit and was not signaled", 
                                childname);
      }
   }

   if (!pidp || exit_status != 0) {
      /* Kill all processes of the process group of the forked child.
         This is to ensure that nothing hangs around. */
      if (!strcmp("job", childname)) {
         if (ckpt_info.pid != 0)   {
            shepherd_signal_job(-ckpt_info.pid, SIGKILL);
         } else {
            shepherd_signal_job(-pid, SIGKILL);
         }
      }
   } else {
      *pidp = pid;
   }

   /*
   ** write no usage in case of error (son() has written to error file)
   */
   if (!SGE_STAT("exit_status", &buf) && buf.st_size) {
      /*
      ** in this case the child didn't get to the exec call or it failed
      ** the status that waitpid and finally start_child returns is
      ** reserved for the exit status of the job
      */
      return exit_status;
   }

   if (!strcmp("job", childname)) {
      /*
       * The new interactive job support uses the qrsh_starter in case of
       * qrsh <with command> jobs.
       */
      if (g_new_interactive_job_support == false ||
          (g_new_interactive_job_support == true &&
           strcasecmp(script_file, JOB_TYPE_STR_QRSH) == 0)) {
         if (search_conf_val("rsh_daemon") != NULL) {
            int qrsh_exit_code = -1;
            int success = 1; 

            sge_switch2start_user();
            success = get_exit_code_of_qrsh_starter(&qrsh_exit_code);
            delete_qrsh_pid_file();
            sge_switch2admin_user();

            if (success != 0) {
               /* This case should never happen */
               /* See Issue 1679 */
               shepherd_trace("can't get qrsh_exit_code");
            }

            /* normal exit */
            if (WIFEXITED(qrsh_exit_code)) {
               const char *qrsh_error;

               exit_status = WEXITSTATUS(qrsh_exit_code);

               qrsh_error = get_error_of_qrsh_starter();
               if (qrsh_error != NULL) {
                  shepherd_error(1, "startup of qrsh job failed: "SFN, qrsh_error);
                  sge_free(&qrsh_error);
               } else {
                  shepherd_trace("job exited normally, exit code is %d", exit_status);
               }
            }

            /* qrsh job was signaled */
            if (WIFSIGNALED(qrsh_exit_code)) {
               child_signal = WTERMSIG(qrsh_exit_code);
               exit_status = 128 + child_signal;
               shepherd_trace("job exited on signal %d, exit code is %d",
                              child_signal, exit_status);
            }
         }
      }

      /******* write usage to file "usage" ************/
      shepherd_write_usage_file(wait_status, exit_status,
                                child_signal, start_time,
                                end_time, &rusage);

      /* exit status before conversion */
      if (status_env) {
         char buffer[16];

         snprintf (buffer, sizeof (buffer), "%d", exit_status);
         sge_set_env_value (status_env, buffer);
      }

      exit_status_for_qrsh = exit_status;

      /* 
       *  we are not interested in exit status of job
       *  except it was the RESCHEDULE_EXIT_STATUS or APPERROR_EXIT_STATUS
       */
      if( exit_status != RESCHEDULE_EXIT_STATUS
       && exit_status != APPERROR_EXIT_STATUS ) {
         exit_status = 0;
      } else if( exit_status == RESCHEDULE_EXIT_STATUS ) {
         shepherd_trace("keep RESCHEDULE_EXIT_STATUS");
      } else if( exit_status == APPERROR_EXIT_STATUS ) {
         shepherd_trace("keep APPERROR_EXIT_STATUS");
      }

   } /* if child == job */

   /* All done. Leave the rest to the execd */
   return exit_status;
}

/****** shepherd/get_remote_host_and_port_from_config() ************************
*  NAME
*     get_remote_host_and_port_from_config() -- reads the hostname and the port
*                                               of the builtin ijs server from
*                                               the job config
*
*  SYNOPSIS
*     static int get_remote_host_and_port_from_config(char **hostname, int 
*     *port, dstring *err_msg) 
*
*  FUNCTION
*     Reads the hostname and the port of the builtin ijs server (in the
*     qrsh or qlogin client) from the job config.
*
*  OUTPUTS
*     char **hostname  - The name of the remote host (= submit host) of this
*                        builtin qrsh/qlogin job. The buffer of this string
*                        gets allocated in this function, it has to be freed
*                        after use.
*     int *port        - The port of the builtin ijs server.
*     dstring *err_msg - Gets filled with an error message in case of error.
*                        Doesn't get modified if this function succeeds.
*
*  RESULT
*     int - 0: OK
*           1: err_msg points to NULL
*           2: "qrsh_control_port" not set in config
*           3: "qrsh_control_port" value is invalid
*
*  NOTES
*     MT-NOTE: get_remote_host_and_port_from_config() is not MT safe 
*******************************************************************************/
static int get_remote_host_and_port_from_config(
char **hostname,
int *port,
dstring *err_msg)
{
   char *address;
   char *separator;

   if (err_msg == NULL) {
      return 1;
   }

   address = get_conf_val("qrsh_control_port");
   if (address == NULL) {
      sge_dstring_sprintf(err_msg, "config does not contain entry for qrsh_control_port");
      return 2;
   }
   /* address now points to the configuration buffer, but we want to keep it
    * independently of the configuration buffer, so we copy it.
    */
   address = strdup(address);
   separator = strchr(address, ':');
   if (separator == NULL) {
      sge_dstring_sprintf(err_msg, "illegal value for qrsh_control_port: "
                        "\"%s\". Should be host:port", address);
      sge_free(&address);
      return 3;
   }
     
   *separator = '\0';
   *hostname = address;
   *port = atoi(separator + 1);

   return 0;
}

/****** shepherd/wait_my_builtin_ijs_child() ***********************************
*  NAME
*     wait_my_builtin_ijs_child() -- waits until the builtin ijs job finishes
*
*  SYNOPSIS
*     static int wait_my_builtin_ijs_child(int pid, const char *childname, int 
*     timeout, ckpt_info_t *p_ckpt_info, ijs_fds_t *p_ijs_fds, struct rusage 
*     *rusage, dstring *err_msg) 
*
*  FUNCTION
*     Waits until the builtin ijs job has finished.
*     In case of severe errors this function exits the shepherd!
*
*  INPUTS
*     int           pid          - PID of the builtin ijs job
*     const char    *childname   - What kind of job is it? Can be 
*     int           timeout      - Timeout for prolog/epilog script. 
*     ckpt_info_t   *p_ckpt_info - Checkpointing info
*     ijs_fds_t     *p_ijs_fds   - The fds that connect us to the job
*
*  OUTPUTS
*     struct rusage *rusage      - Gets filled with the usage of the job
*     dstring       *err_msg     - Gets filled with an error message if an
*                                  error happens
*
*  RESULT
*     int - The exit status of the job.
*
*  NOTES
*     MT-NOTE: wait_my_builtin_ijs_child() is not MT safe 
*
*  SEE ALSO
*     shepherd/wait_my_child
*******************************************************************************/
static int wait_my_builtin_ijs_child(
int           pid,           /* IN: pid of child/job */
const char    *childname,    /* IN: "job", "pe_start", ...     */
int           timeout,       /* IN: used for prolog/epilog script, 0 for job */
ckpt_info_t   *p_ckpt_info,  /* IN: data for checkpointing handling */
ijs_fds_t     *p_ijs_fds,    /* IN: file descriptors needed for IJS */
struct rusage *rusage,       /* OUT: accounting information */
dstring       *err_msg       /* OUT: error message - if any */
) {
   char    *job_owner;
   char    *remote_host = NULL;
   bool    csp_mode     = false;
   int     ret;
   int     remote_port  = 0;
   int     exit_status  = -1;

   /* close child's end of the pipe */
   shepherd_trace("parent: closing child's end of the pipe");

   /* read destination host and port from config */
   ret = get_remote_host_and_port_from_config(&remote_host, &remote_port, err_msg);
   if (ret != 0 || remote_host == NULL || remote_port == 0) {
      shepherd_error(1, "startup of qrsh job failed: "SFN"",
                     sge_dstring_get_string(err_msg));
   }
   job_owner = get_conf_val("job_owner");

   /* 
    * For performance reasons, we read the csp state from the config,
    * not from the bootstrap file. The bootstrap file might reside on
    * a very, very slow drive.
    */
   if (strcasecmp(get_conf_val("csp"), "true") == 0 ||
       strcasecmp(get_conf_val("csp"), "1") == 0) {
      csp_mode = true;
   }
   shepherd_trace("csp = %d", csp_mode);

   ret = parent_loop(pid, childname, timeout, p_ckpt_info, p_ijs_fds, job_owner,
            remote_host, remote_port, csp_mode, &exit_status, rusage, err_msg);
   sge_free(&remote_host);
   if (ret != 0) {
      shepherd_error(1, "startup of qrsh job failed: "SFN"",
                     sge_dstring_get_string(err_msg));
   }

   /*shepherd_signal_job(-pid, SIGKILL);*/
   sge_switch2start_user();
   kill(-pid, SIGKILL);
   sge_switch2admin_user();

   p_ckpt_info->interval = 0;

   return exit_status;
}

static void verify_method(const char *method_name) 
{
   char *override_signal;

   if ((override_signal = search_nonone_conf_val(method_name))) {

      if (override_signal[0] != '/' && 
         shepherd_sys_str2signal(override_signal)<0) {
         shepherd_error(1, "cannot convert " SFN " " SFQ" into a "
                              "valid signal number at this machine", 
                              method_name, override_signal);
      } /* else: 
         it must be path of a signal script:
         AH: I'd like to do a stat(2) on the path to ensure that the 
         signal script can be started before starting the job itself.
         But therefore we had to change into the jobs target user .. 
      */
   }
}

/****** shepherd/core/forward_signal_to_job() *********************************
*  NAME
*     forward_signal_to_job() -- Forward signal to job. 
*
*  SYNOPSIS
*     static void forward_signal_to_job(int pid, int timeout, 
*                                       int *postponed_signal, 
*                                       int remaining_alarm, 
*                                       pid_t ctrl_pid[3]) 
*
*  FUNCTION
*     We have gotten a signal. Look for it and forward it to the
*     group of the jobs. 
*
*  INPUTS
*     int pid               - pid of the process (group) to be signaled 
*     int timeout           - timeout value (0 means implicitly the job) 
*     int *postponed_signal - input/output parameter. 
*                             used to detect and prevent repeated 
*                             initiation of notify mechanism
*                             as this can delay final job 
*                             suspension endlessly
*     int remaining_alarm   - if it is necessary to set the alarm
*                             clock, then this timeout value
*                             will be used.
*     pid_t ctrl_pid[3]     - used to store the pids of:
*                             resume_method, stop_method, terminate_method 
*******************************************************************************/
static void forward_signal_to_job(int pid, int timeout, 
                                  int *postponed_signal, 
                                  int remaining_alarm, 
                                  pid_t ctrl_pid[3])
{
   int sig;
   static int replace_qrsh_pid = 1; /* cache */

   if (received_signal==SIGALRM) {
      /* a timeout expired */
      if (timeout) {
         shepherd_trace("SIGALRM with timeout");
         if (get_n_sigs() > 0) {
            /* kill directly - a timeout of a script expired */
            shepherd_trace("timeout expired - killing process");
            shepherd_signal_job(-pid, SIGKILL);
         }
         return;
      } else {
         shepherd_trace("SIGALRM without timeout");
      }
      received_signal = 0;
   }

   /* store signal if we got one */
   if (received_signal) {
      shepherd_trace("forward_signal_to_job(): mapping signal %d %s",
         received_signal, sge_sys_sig2str(received_signal));
      sig = map_signal(received_signal);

      received_signal = 0;
      /* store signal in ring buffer */
      if (add_signal(sig)) {
         shepherd_trace("failed to store received signal");
         return; 
      }     
   }

   /*
   ** jg: in qrsh case, replace job_pid with pid from qrsh_starter
   */
   /* cached info exists? */
   if (replace_qrsh_pid) {
      /* do we signal a qrsh job? */
      if (search_conf_val("qrsh_pid_file") == NULL) {
         replace_qrsh_pid = 0;
      } else {
         char *qrsh_pid_file;
         pid_t qrsh_pid;

         /* read pid from qrsh_starter */
         qrsh_pid_file = get_conf_val("qrsh_pid_file");

         shepherd_read_qrsh_pid_file(qrsh_pid_file, &qrsh_pid,
                                     &replace_qrsh_pid);

      }
   }
    
   /* forward signals */
   if (!timeout) { /* signals for scripts with timeouts are stored */
      sig = get_signal();
      while (sig != -1 && sig != 0) {
         switch (sig) {
         case SIGCONT:
            shepherd_deliver_signal(sig, pid, postponed_signal,
                                    remaining_alarm, &(ctrl_pid[0]));
            break;
         case SIGSTOP:
            shepherd_deliver_signal(sig, pid, postponed_signal, 
                                    remaining_alarm, &(ctrl_pid[1]));
            break;
         case SIGKILL:
            signalled_ckpt_job = 0;
            shepherd_deliver_signal(sig, pid, postponed_signal, 
                                    remaining_alarm, &(ctrl_pid[2]));
            break;
         case SIGTTOU:
            signalled_ckpt_job = 1;
            sig = SIGKILL;
            shepherd_signal_job(-pid, sig);
            shepherd_trace("kill(%d, %s) -> checkpoint initiation -> "
                           "killing job", -pid, sge_sys_sig2str(sig));
            break;
         case SIGUSR1:
            signalled_ckpt_job = 0;
            shepherd_trace("kill(%d, %s)", -pid, sge_sys_sig2str(sig));
            shepherd_signal_job(-pid, sig);
            break;
         case SIGUSR2:
            signalled_ckpt_job = 0;
            shepherd_trace("kill(%d, %s)", -pid, sge_sys_sig2str(sig));
            shepherd_signal_job(-pid, sig);
            break;
         default:
            shepherd_trace("kill(%d, %s)", -pid, sge_sys_sig2str(sig));
            shepherd_signal_job(-pid, sig);
            break;
         }
         sig = get_signal();
      }
   }
}

static const char *method_name[] = { 
   "resume_method", 
   "suspend_method", 
   "terminate_method", 
   NULL 
};


static int signal_array[] = { 
   SIGCONT, 
   SIGSTOP, 
   SIGKILL, 
   0 
};

static const char *notify_name[] = { 
      "",                    
      "notify_susp",
      "notify_kill", 
      NULL 
};

/****** shepherd/core/shepherd_find_method() *********************************
*  NAME
*     shepherd_find_method() -- find the method_name for a signal 
*
*  SYNOPSIS
*     static const char* shepherd_find_method(int sig) 
*
*  FUNCTION
*
*    find the method_name for a signal
*
*  INPUTS
*     int pid               - the signal 
*******************************************************************************/
static const char* shepherd_find_method(int sig) {
   
   int index;

   /*
    * Find names of variables stored in the config file
    */
   for (index = 0; method_name[index] != NULL; index++) {
      if (signal_array[index] == sig) {
         break;
      }
   }
   return method_name[index];   
}

/****** shepherd/core/shepherd_find_notify() *********************************
*  NAME
*     shepherd_find_method() -- find the notify_name for a signal 
*
*  SYNOPSIS
*     static const char* shepherd_find_notify(int sig) 
*
*  FUNCTION
*
*    find the notify_name for a signal
*
*  INPUTS
*     int pid               - the signal 
*******************************************************************************/
static const char* shepherd_find_notify(int sig) {
   
   int index;

   /*
    * Find names of variables stored in the config file
    */
   for (index = 0; notify_name[index] != NULL; index++) {
      if (signal_array[index] == sig) {
         break;
      }
   }
   return notify_name[index];
}

/****** shepherd/core/shepherd_deliver_signal() ********************************
*  NAME
*     shepherd_deliver_signal() -- Resume/Suspend/Terminate processgroup
*
*  SYNOPSIS
*     void shepherd_deliver_signal(int sig, 
*                                  int pid, 
*                                  int *postponed_signal, 
*                                  int *remaining_alarm, 
*                                  pid_t *ctrl_pid) 
*
*  FUNCTION
*     Depending on "sig" the function either resumes, suspends or
*     terminates all processes being part of process group "-pid".
*     This function heeds the configured resume, suspend and terminate
*     method just as the notify mechanism.
*
*     "postponed_signal" and "remaining_alarm" will be used to detect
*     repeated signals which arrive during the time between the processes
*     got their notification signal and the final signal/method
*     (e.g. SIGKILL arrives but SIGUSR2 was already sent due to a
*      previous SIGKILL).
*
*     "ctrl_pid" will only be used if a script has to be started. 
*     This variable will then contain the pid of the scripts process.
*
*  INPUTS
*     int sig               - signal (SIGCONT, SIGSTOP or SIGKILL)
*     int pid               - pid 
*     int *postponed_signal - used for notification mechanism 
*     int remaining_alarm   - used for notification mechanism 
*     pid_t *ctrl_pid       - pid of applications which might be started 
*
*  RESULT
*     void - none 
*******************************************************************************/
void shepherd_deliver_signal(int sig, int pid, int *postponed_signal,
                             int remaining_alarm, pid_t *ctrl_pid)
{
   const char* notify_name = shepherd_find_notify(sig);

   /*
    * When notify is used for SIGSTOP the SIGCONT might be sent by execd before
    * the actual SIGSTOP has been delivered. We must clean this state at this point
    * otherwise the next SIGSTOP signal will be delivered without a notify because 
    * it is then seen just as repeated initiation of notify mechanism 
    */
   if (sig == SIGCONT && notify && *postponed_signal == SIGSTOP)
      *postponed_signal = 0;

   /*
    * Job notification
    */
   if (sig == SIGSTOP || sig == SIGKILL) {
      int seconds = 0;

      if (shepconf_has_to_notify_before_signal(&seconds)) {

         /* sig  *postponed_signal
          *
          * STOP KILL    --> Termination in progress
          *                  Don't send a stop notification
          * KILL KILL    --> Termination notification already handled
          * STOP STOP    --> Suspension notification already handled
          *
          * KILL STOP    ++> Suspension in progress
          *                  Notify the processes about the termination
          */
         if ((sig == SIGSTOP && *postponed_signal == SIGKILL) ||
             (sig == SIGKILL && *postponed_signal == SIGKILL) ||
             (sig == SIGSTOP && *postponed_signal == SIGSTOP)) {
            /*
             * Detect and prevent repeated initiation of notify mechanism
             * as this can delay termination endlessly
             */
            if (sig == *postponed_signal) {
               shepherd_trace("ignoring repeated %s notification", 
                              sge_sys_sig2str(*postponed_signal));
            } else if (sig == SIGSTOP) {
               shepherd_trace("termination in progress - ignoring %s notification", 
                              sge_sys_sig2str(*postponed_signal)); 
            }
            alarm(remaining_alarm);
            return;
         } else {
            int notify_signal = 0;

            if (shepconf_has_notify_signal(notify_name,&notify_signal)) {
               shepherd_trace("kill(%d, %s) -> notification "
                              "for delayed (%d s) signal %s", -pid,
                              sge_sys_sig2str(notify_signal),
                              seconds, sge_sys_sig2str(sig));
            } else {
               shepherd_trace("No notif. signal -> notification "
                              "for delayed (%d s) signal %s",
                              seconds, sge_sys_sig2str(sig));
            }

            if (notify_signal) {
               *postponed_signal = sig;
               shepherd_signal_job(-pid, notify_signal);
               alarm(notify);
               return;
            } 
         }
      } 
   }
   
   shepconf_deliver_signal_or_method(sig, pid, ctrl_pid);
}

/****** shepherd/core/shepconf_deliver_signal_or_method() *********************************
*  NAME
*     shepconf_deliver_signal_or_method() -- deliver a signal to a process or start user-defined method
*
*  SYNOPSIS
*     static void shepconf_deliver_signal_or_method(int sig, int pid, 
*                                                   pid_t *ctrl_pid)
*
*  FUNCTION
*
*    deliver a signal to a process or start a user-defined method
*    (terminate_method, resume_method or suspend_method) 
*
*  INPUTS
*     int pid               - the signal which should be delivered
*     int pid               - pid of the job
*     pid_t *ctrl_pid       - pid of applications which might be started 
*
*******************************************************************************/
static void shepconf_deliver_signal_or_method(int sig, int pid, pid_t *ctrl_pid) {
   
   const char* method_name = shepherd_find_method(sig);
   int new_sig;
   dstring method = DSTRING_INIT;
   
   /*
    * There are three different ways to signal the processes:
    *
    *    a) Userdefined signal 
    *    b) Userdefined method
    *    c) Default signal 
    */
   if (shepconf_has_userdef_signal(method_name, &new_sig)) {
      shepherd_trace("kill(%d, %s) -> overrides kill(%d, %s)",
                     -pid, sge_sys_sig2str(new_sig),
                     -pid, sge_sys_sig2str(sig));
      shepherd_signal_job(-pid, new_sig);
   } else if (shepconf_has_userdef_method(method_name, &method)) {
      shepherd_trace("%s -> overrides kill(%d, %s)",
                     sge_dstring_get_string(&method), -pid,
                     sge_sys_sig2str(sig));
      if (*ctrl_pid == -999) {
         char command[10000];

         replace_params(sge_dstring_get_string(&method), command,
                        sizeof(command), ctrl_method_variables);
         *ctrl_pid = start_async_command(method_name, command);
      } else {
         shepherd_trace("Skipped start of suspend: previous command "
                        "(pid= "pid_t_fmt") is still active", *ctrl_pid);
      }
   } else {
      shepherd_trace("kill(%d, %s)", -pid, sge_sys_sig2str(sig));
      shepherd_signal_job(-pid, sig);
   }
   sge_dstring_free(&method);
}

/*--------------------------------------------------------------------
 * set_shepherd_signal_mask
 * set signal mask that shepherd can handle signals from execd
 *--------------------------------------------------------------------*/
static void set_shepherd_signal_mask(void)   
{
   struct sigaction sigact, sigact_old;
   sigset_t mask;

   /* make sure pending SIGPIPE signals logged in trace file */ 
   set_sig_handler(SIGPIPE);
   {
      sigset_t sigset;
      sigemptyset(&sigset);

      /* already set a handler for SIGPIPE */
      sigaddset(&sigset, SIGPIPE);
       /* don't touch shepherds known signals (handlers set in set_shepherd_signal_mask) */
      sigaddset(&sigset, SIGTTOU);
      sigaddset(&sigset, SIGTTIN);
      sigaddset(&sigset, SIGUSR1);
      sigaddset(&sigset, SIGUSR2);
      sigaddset(&sigset, SIGCONT);
      sigaddset(&sigset, SIGWINCH);
      sigaddset(&sigset, SIGTSTP);
      sge_set_def_sig_mask(&sigset, NULL);
   }

      
   /* get mask */
   sigprocmask(SIG_SETMASK, NULL, &mask);

   sigact.sa_handler = signal_handler;
   sigact.sa_mask = mask;

   /* SA_INTERRUPT is at least needed on SUN4 to interrupt waitxx()
    * when a signal arrives 
    */

#ifdef SA_INTERRUPT
   sigact.sa_flags = SA_INTERRUPT;
#else
   sigact.sa_flags = 0;
#endif

   sigaction(SIGTTOU, &sigact, &sigact_old); /* init checkpointing    */
   sigaction(SIGTTIN, &sigact, &sigact_old); /* read signal from file */
   sigaction(SIGUSR1, &sigact, &sigact_old);
   sigaction(SIGUSR2, &sigact, &sigact_old);
   sigaction(SIGCONT, &sigact, &sigact_old);
   sigaction(SIGWINCH, &sigact, &sigact_old);
   sigaction(SIGTSTP, &sigact, &sigact_old);
   sigaction(SIGALRM, &sigact, &sigact_old);

   /* allow some more signals */
   sigdelset(&mask, SIGTTOU);
   sigdelset(&mask, SIGTTIN);
   sigdelset(&mask, SIGUSR1);
   sigdelset(&mask, SIGUSR2);
   sigdelset(&mask, SIGCONT);
   sigdelset(&mask, SIGWINCH);
   sigdelset(&mask, SIGTSTP);
   sigdelset(&mask, SIGALRM);

   /* set mask */
   sigprocmask(SIG_SETMASK, &mask, NULL);
}

static void change_shepherd_signal_mask()
{
   sigset_t mask;

   sigprocmask(SIG_SETMASK, NULL, &mask);
   sigdelset(&mask, SIGTERM);
   sigprocmask(SIG_SETMASK, &mask, NULL);
}

/*------------------------------------------------------------------------
 * check_ckpttype
 * Return:   -1 error,
 *           0 no checkpointing
 *           Bitmask of checkpointing type
 *------------------------------------------------------------------------*/
static int check_ckpttype(void)
{
   char *ckpt_job, *ckpt_interface, *ckpt_restarted, *ckpt_migr_command, 
        *ckpt_rest_command, *ckpt_command, *ckpt_pid, *ckpt_osjobid, 
        *ckpt_clean_command, *ckpt_dir, *ckpt_signal_str;
   int ckpt_type, ckpt;
   
   ckpt_type = 0;
   
   if ((ckpt_job = get_conf_val("ckpt_job")) && (atoi(ckpt_job) > 0))
      ckpt = 1;
   else 
      ckpt = 0;   

   ckpt_interface = search_conf_val("ckpt_interface");
   ckpt_restarted = search_conf_val("ckpt_restarted");
   ckpt_command = search_conf_val("ckpt_command");
   ckpt_migr_command = search_conf_val("ckpt_migr_command");
   ckpt_rest_command = search_conf_val("ckpt_rest_command");
   ckpt_clean_command = search_conf_val("ckpt_clean_command");
   ckpt_pid = search_conf_val("ckpt_pid");
   ckpt_osjobid = search_conf_val("ckpt_osjobid");
   ckpt_dir = search_conf_val("ckpt_dir");
   ckpt_signal_str = search_conf_val("ckpt_signal");

   ckpt_type = 0;
   if (!ckpt)
      ckpt_type = 0;
   else if (!(ckpt_interface && ckpt_restarted &&  
       ckpt_command && ckpt_migr_command && ckpt_rest_command &&
       ckpt_clean_command && ckpt_pid && ckpt_osjobid && ckpt_dir && ckpt_signal_str ))
      ckpt_type = -1;
   else if (!strcasecmp(ckpt_interface, "hibernator") ||
            !strcasecmp(ckpt_interface, "cpr") ||
            !strcasecmp(ckpt_interface, "application-level")) {

      ckpt_type = CKPT | CKPT_KERNEL;

      if (!strcasecmp(ckpt_interface, "hibernator"))
         ckpt_type |= CKPT_HIBER;
      else if (!strcasecmp(ckpt_interface, "cpr"))
         ckpt_type |= CKPT_CPR;
      else if (!strcasecmp(ckpt_interface, "application-level"))
         ckpt_type |= CKPT_APPLICATION;   

      if (atoi(ckpt_restarted) > 1)
         ckpt_type |= CKPT_REST;
        
      if ((atoi(ckpt_restarted) > 1) && !(ckpt_type & CKPT_APPLICATION))
         ckpt_type |= CKPT_REST_KERNEL;
         
      /* special hack to avoid restart from restart command if restart = "none" */
      if ((ckpt_type & CKPT_REST_KERNEL) && !strcasecmp(ckpt_rest_command, "none"))
         ckpt_type &= ~CKPT_REST_KERNEL;    

      if (strcasecmp(ckpt_signal_str, "none")) {
         ckpt_signal = sge_sys_str2signal(ckpt_signal_str);
         if (ckpt_signal == -1)
            ckpt_type = -1;
      }
   }
   else if (!strcasecmp(ckpt_interface, "transparent")) {
      ckpt_type = CKPT | CKPT_TRANS;

      if (atoi(ckpt_restarted) > 1)
         ckpt_type |= CKPT_REST;
          
      if (!strcasecmp(ckpt_signal_str, "none"))  
         ckpt_type = -1;
      else {
         ckpt_signal = sge_sys_str2signal(ckpt_signal_str);
         if (ckpt_signal == -1)
            ckpt_type = -1;
      }      
   }
   else if (!strcasecmp(ckpt_interface, "userdefined")) {
      ckpt_type = CKPT | CKPT_USER;

      if (atoi(ckpt_restarted) > 1)
         ckpt_type |= CKPT_REST; 
      if (strcasecmp(ckpt_signal_str, "none")) {
         ckpt_signal = sge_sys_str2signal(ckpt_signal_str);
         if (ckpt_signal == -1)
            ckpt_type = -1;
      }
   }
      
   return ckpt_type;
}

/*------------------------------------------------------------------------*/
static void handle_signals_and_methods(
   int npid,
   int pid,
   int *postponed_signal,
   pid_t ctrl_pid[3],
   ckpt_info_t *p_ckpt_info,
   int *ckpt_cmd_pid,
   int *rest_ckpt_interval,
   int timeout,
   int *migr_cmd_pid,
   int *kill_job_after_checkpoint,
   int status,
   int *inArena,
   int *inCkpt,
   const char *childname,
   int *job_status,
   int *job_pid)
{
   int remaining_alarm  = 0;
   int i;

   remaining_alarm = alarm(0);
     
   /* got a signal? should compare errno with EINTR */
   if (npid == -1) {
      /* got SIGALRM */ 
      if (received_signal == SIGALRM) {
         /* notify: postponed signals SIGSTOP, SIGKILL */
         if (*postponed_signal) {
            shepherd_trace("kill(%d, %s) -> delivering postponed signal", -pid, 
                           sge_sys_sig2str(*postponed_signal));
            
            shepconf_deliver_signal_or_method(*postponed_signal, pid, ctrl_pid);
            /*shepherd_signal_job(-pid, postponed_signal);*/
            *postponed_signal = 0;
         } else {
            shepherd_trace("no postponed signal");

            if (p_ckpt_info->interval) {
               if (p_ckpt_info->type & CKPT_KERNEL) {
                  shepherd_trace("initiate regular kernel level checkpoint");
                  *ckpt_cmd_pid = start_async_command("ckpt", ckpt_command);
                  rest_ckpt_interval = 0;
                  if (*ckpt_cmd_pid == -1) {
                     *ckpt_cmd_pid = -999;
		     /* fixme: had null pointer deref -- should it do
			something else? */
                     /* *rest_ckpt_interval = p_ckpt_info->interval; */
                  }
                  else
                     *inCkpt = 1;
               } else if (p_ckpt_info->type & CKPT_TRANS) {
                  shepherd_trace("send checkpointing signal to transparent "
                                 "checkpointing job");
                  sge_switch2start_user();
                  kill(-pid, ckpt_signal);
                  sge_switch2admin_user();
               } else if (p_ckpt_info->type & CKPT_USER) {
                  shepherd_trace("send checkpointing signal to userdefined "
                                 "checkpointing job");
                  sge_switch2start_user();
                  kill(-pid, ckpt_signal);
                  sge_switch2admin_user();
               }
            } 
            forward_signal_to_job(pid, timeout, postponed_signal,
                                  remaining_alarm, ctrl_pid);
         }
      } else if (p_ckpt_info->pid && (received_signal == SIGTTOU)) {
         shepherd_trace("initiate checkpoint due to migration request");
         rest_ckpt_interval = 0; /* disable regular checkpoint */
         if (!(*inCkpt)) {
            *migr_cmd_pid = start_async_command("migrate", migr_command);
            if (*migr_cmd_pid == -1) {
               *migr_cmd_pid = -999;
            } else {
               *inCkpt = 1;
               signalled_ckpt_job = 1;
            }
         } else {
            /* kill job after end of ckpt_command */
            *kill_job_after_checkpoint = 1; 
         }
      } else if (received_signal != 0 || *postponed_signal != 0) { /* received any other signal */
#if defined(INTERIX)
         sge_set_environment(true);
         if(strcmp(childname, "job") == 0 &&
            wl_get_GUI_mode(get_conf_val("display_win_gui")) == true) {
            /*
             * forward SIGKILL, swallow all other signals
             */
            int sig = map_signal(received_signal);
            if(sig == SIGKILL) { 
               char errormsg[MAX_STRING_SIZE];
               wl_forward_signal_to_job(get_conf_val("job_id"),
                                     &sig,
                                     errormsg, MAX_STRING_SIZE);
            }
         } else
#endif
         {
            forward_signal_to_job(pid, timeout, postponed_signal, 
                                  remaining_alarm, ctrl_pid);
         }
      } else {
         /*
          * Didn't receive a signal, just continue, 
          * the following code is prepared for it.
          */
      }
   }
         
   /* here we reap the control action methods */
   for (i=0; i<3; i++) {
      static char *cm_descr[] = {
         "resume",
         "suspend",
         "terminate"
      };
      if ((npid == ctrl_pid[i]) && ((WIFSIGNALED(status) || WIFEXITED(status)))) {
         shepherd_trace("reaped %s command", cm_descr[i]);
         ctrl_pid[i] = -999;
      }
   }

   /* here we reap the checkpoint command */
   if ((npid == *ckpt_cmd_pid) && ((WIFSIGNALED(status) || WIFEXITED(status)))) {
      *inCkpt = 0;
      shepherd_trace("reaped checkpoint command");
      if (!(*kill_job_after_checkpoint))
         *rest_ckpt_interval = p_ckpt_info->interval; 
      *ckpt_cmd_pid = -999;
      if (WIFEXITED(status)) {
         shepherd_trace("checkpoint command exited normally");
         if (WEXITSTATUS(status)==0 && !(*inArena)) {
            shepherd_trace("checkpoint is in the arena after regular checkpoint");
            create_checkpointed_file(1);
            *inArena = 1;
         }                     
      }
      /* A migration request occurred and we just did a regular checkpoint */
      if (*kill_job_after_checkpoint) {
          shepherd_trace("killing job after regular checkpoint due to migration request");
          shepherd_signal_job(-pid, SIGKILL);   
      }    
   }

   /* here we reap the migration command */
   if ((npid == *migr_cmd_pid) && 
       ((WIFSIGNALED(status) || WIFEXITED(status)))) {
      /*
       * If the migrate command exited but the job did 
       * not stop (due to error in the migrate script) we have
       * to reset internal state. As result further migrate
       * commands will be executed (qmod -f -s).
       */
      *inCkpt = 0;
      shepherd_trace("if jobs and shepherd do not exit there is some error "
                     "in the migrate command");
      shepherd_trace("reaped migration checkpoint command");
      *migr_cmd_pid = -999;
      if (WIFEXITED(status)) {
         shepherd_trace("checkpoint command exited normally");
         if (WEXITSTATUS(status) == 0 && !(*inArena)) {
            shepherd_trace("checkpoint is in the arena after migration request");
            create_checkpointed_file(1);
            *inArena = 1;
         }
      }
   }
  
   /* reap job */
   if ((npid == pid) && ((WIFSIGNALED(status) || WIFEXITED(status)))) {
      shepherd_trace("%s exited with exit status %d", childname, WEXITSTATUS(status));
      *job_status = status;
      *job_pid = -999;
   }   

   if ((npid == coshepherd_pid) && ((WIFSIGNALED(status) || WIFEXITED(status)))) {
      shepherd_trace("reaped sge_coshepherd");
      coshepherd_pid = -999;
   }   
}         
/*------------------------------------------------------------------------*/
int wait_my_child(
int pid,                   /* pid of job */
const char *childname,     /* "job", "pe_start", ...     */
int timeout,               /* used for prolog/epilog script, 0 for job */
ckpt_info_t *p_ckpt_info,  /* infos used for checkpointing */
struct rusage *rusage,     /* accounting information */
int fd_pty_master,         /* fd of the pty-master. -1 if not set */
int fd_std_err             /* fd of stderr. -1 if not set */
) {
   int i, npid, status, job_status;
   int ckpt_cmd_pid, migr_cmd_pid;
   int rest_ckpt_interval;
   int inArena, inCkpt, kill_job_after_checkpoint, job_pid;
   int postponed_signal = 0; /* used for implementing SIGSTOP/SIGKILL notify mechanism */
   pid_t ctrl_pid[3];
   int wait_options = 0;
   char* stdout_path = NULL;
   char* stderr_path = NULL;
   char* stdin_path = NULL;
   int fdout = -1;
   int fderr = -1;
   int fdin = -1;
   int poll_size = 0;
   struct pollfd* pty_fds = NULL;

#if defined(HPUX) || defined(INTERIX)
   struct rusage rusage_hp10;
#endif

   /* handle qsub -pty */
   if (fd_pty_master != -1) {
      char* job_owner = NULL;
      /* in case of qsub -pty, the wait call should not block until
         the job finishes */
      wait_options |= WNOHANG;

      /* get and open the right output-files */
      stdout_path = build_path(SGE_STDOUT);
      stderr_path = build_path(SGE_STDERR);
      stdin_path = get_conf_val("stdin_path");

      /* get gid and uid of job_owner (stored in pw) */
      job_owner = get_conf_val("job_owner");

      if (job_owner == NULL) {
         shepherd_trace("Cannot determine the job-owner!");
         shepherd_signal_job(-pid, SIGTERM);
      }

      /* open input file if set */
      if (strcasecmp(stdin_path, "/dev/null") != 0) {
         fdin = handle_io_file(stdin_path, job_owner, false);
         if (fdin == -1) {
            shepherd_signal_job(-pid, SIGTERM);
         }
      }

      /* create the file as job_owner */
      fdout = handle_io_file(stdout_path, job_owner, true);
      if (fdout == -1) {
         shepherd_signal_job(-pid, SIGTERM);
      }

      /* create the file as job_owner */
      fderr = handle_io_file(stderr_path, job_owner, true);
      if (fderr == -1) {
         shepherd_signal_job(-pid, SIGTERM);
      }

      /* register the necessary fds for the poll call */
      if (fd_std_err != -1) {
         poll_size = 2;
      } else {
         poll_size = 1;
      }
      pty_fds = (struct pollfd*)sge_malloc(sizeof (struct pollfd) * poll_size);

      pty_fds[0].fd = fd_pty_master;
      if (fdin != -1) {
         pty_fds[0].events = POLLIN | POLLPRI | POLLOUT;
      } else {
         pty_fds[0].events = POLLIN | POLLPRI;
      }
      if (fd_std_err != -1) {
         pty_fds[1].fd = fd_std_err;
         pty_fds[1].events = POLLIN | POLLPRI;
      }
   }

   memset(rusage, 0, sizeof(*rusage));
   kill_job_after_checkpoint = 0;
   inCkpt = 0;
   rest_ckpt_interval = p_ckpt_info->interval;
   job_pid = pid;
   ckpt_cmd_pid = migr_cmd_pid = -999;
   job_status = 0;
   for (i=0; i<3; i++)
      ctrl_pid[i] = -999;

   rest_ckpt_interval = p_ckpt_info->interval;

   /* Write info that we already have a checkpoint in the arena */
   if (ISSET(p_ckpt_info->type, CKPT_REST_KERNEL)) {
      inArena = 1;
      create_checkpointed_file(1);
   } else {
      inArena = 0;
   }
   
   do {
      if (p_ckpt_info->interval != 0 && rest_ckpt_interval != 0) {
         alarm(rest_ckpt_interval);
      }

#if defined(INTERIX)
      npid = waitpid(-1, &status, wait_options);
#else
      npid = wait3(&status, wait_options, rusage);
#endif

#if defined(INTERIX)
      /* <Windows_GUI> */
      sge_set_environment(true);
      if (strcmp(childname, "job") == 0 &&
         wl_get_GUI_mode(get_conf_val("display_win_gui")) == true) {
         if (npid != -1 && npid != 0) {      
            char errormsg[MAX_STRING_SIZE];

            memset(&rusage_hp10, 0, sizeof(rusage_hp10));

            shepherd_trace("retrieving remote usage");
            status = 0;
            if (wl_getrusage_remote(get_conf_val("job_id"),
                                   &status, &rusage_hp10, errormsg) != 0) {
               shepherd_trace(errormsg);
            }
            shepherd_trace("retrieved remote usage: %ld %ld %ld %ld %d",
                           (long) rusage_hp10.ru_stime.tv_sec,
                           (long) rusage_hp10.ru_stime.tv_usec,
                           (long) rusage_hp10.ru_utime.tv_sec,
                           (long) rusage_hp10.ru_utime.tv_usec,
                           status);
         }
      } else 
      /* </Windows_GUI> */
#endif
#if defined(HPUX) || defined(INTERIX)
      {
         /* wait3 doesn't return CPU usage */
         getrusage(RUSAGE_CHILDREN, &rusage_hp10);
      }
#endif

      if (npid == -1) {
         shepherd_trace("wait3 returned -1");
         if (errno == ECHILD) {
            shepherd_trace("all children have exited, quit wait3() loop.");
            break;
         }
      } else if (npid != 0){
         shepherd_trace("wait3 returned %d (status: %d; WIFSIGNALED: %d, "
                        " WIFEXITED: %d, WEXITSTATUS: %d)", npid, status,
                        WIFSIGNALED(status), WIFEXITED(status),
                        WEXITSTATUS(status));
      }

      /* qsub -pty handling */
      if (fd_pty_master != -1) {
         int ret;
         int stop_job = 0;    /* This flag will be set if an error occurred */
         int stop_poll = 1;   /* This flag will be reset if polling is still necessary */

         /* look for data to write */
         ret = poll(pty_fds, poll_size, 10);
         if (ret > 0) {
            /* write the data from the job to the output-files */
            if (pty_fds[0].revents & POLLIN || pty_fds[0].revents & POLLPRI) {
               char buffer[MAX_STRING_SIZE];
               int read_bytes = -1;

               read_bytes = read(pty_fds[0].fd, buffer, MAX_STRING_SIZE-1);
               if (read_bytes > 0) {
                  int written_bytes = 0;
                  while (written_bytes != read_bytes) {
                     int tmp_written = 0;
                     tmp_written = write(fdout, buffer, read_bytes);
                     if (tmp_written == -1) {
                        stop_job = 1;
                        shepherd_trace("Could not write to %s. Error: %s", stdout_path, strerror(errno));
                     }
                     written_bytes += tmp_written;
                  }
               } else if (read_bytes == -1) {
                  /* if read was interrupted by a signal we do not have to abort */
                  if (errno != EINTR) {
                     stop_job = 1;
                     shepherd_trace("Could not read from pty_master fd. Error: %s", strerror(errno));
                  }
               }
            }

            if (fd_std_err != -1) {
               if (pty_fds[1].revents & POLLIN || pty_fds[1].revents & POLLPRI) {
                  char buffer[MAX_STRING_SIZE];
                  int read_bytes = -1;

                  read_bytes = read(pty_fds[1].fd, buffer, MAX_STRING_SIZE-1);
                  if (read_bytes > 0) {
                     int written_bytes = 0;
                     while (written_bytes != read_bytes) {
                        int tmp_written = 0;
                        tmp_written = write(fderr, buffer, read_bytes);
                        if (tmp_written == -1) {
                           stop_job = 1;
                           shepherd_trace("Could not write to %s. Error: %s", stderr_path, strerror(errno));
                        }
                        written_bytes += tmp_written;
                     }
                  } else if (read_bytes == -1) {
                     /* if read was interrupted by a signal we do not have to abort */
                     if (errno != EINTR) {
                        stop_job = 1;
                        shepherd_trace("Could not read from std_err fd. Error: %s", strerror(errno));
                     }
                  }
               }
            }

            if (pty_fds[0].revents & POLLOUT && fdin != -1) {
               char buffer[MAX_STRING_SIZE];
               int read_bytes = -1;

               read_bytes = read(fdin, buffer, MAX_STRING_SIZE-1);

               if (read_bytes > 0) {
                  int written_bytes = 0;
                  while (written_bytes != read_bytes) {
                     int tmp_written = 0;
                     tmp_written = write(pty_fds[0].fd, &buffer[written_bytes], read_bytes - written_bytes);
                     if (tmp_written == -1) {
                        /* if write was interrupted by a signal we do not have to abort */
                        if (errno != EINTR) {
                           stop_job = 1;
                           shepherd_trace("Could not write to PTY. Error: %s", strerror(errno));
                        } else {
                           tmp_written = 0;
                        }
                     }
                     written_bytes += tmp_written;
                  }
               } else if (read_bytes == -1) {
                  /* if read was interrupted by a signal we do not have to abort */
                  if (errno != EINTR) {
                     stop_job = 1;
                     shepherd_trace("Could not read from %s. Error: %s", stdin_path, strerror(errno));
                  }
               } else if (read_bytes == 0) {
                  shepherd_trace("Reached end of %s", stdin_path);
                  pty_fds[0].events = POLLIN | POLLPRI;
               }
            }
            /* Handle possible error-events of the poll call */

            for (i = 0; i < poll_size; i++) {
               if (pty_fds[i].revents & POLLHUP) {
                  pty_fds[i].events = 0;
                  shepherd_trace("Poll received POLLHUP (Hang up). Unregister the FD.");
               }
               if(pty_fds[i].revents & POLLERR || pty_fds[i].revents & POLLNVAL) {
                  stop_job = 1;
                  shepherd_trace("Poll received an error from pty. Exit!");
               }
               /* Check if there are still fds which has to be polled */
               if (pty_fds[i].events != 0) {
                  stop_poll = 0;
               }
            }

            /* There is no fd which has to be polled. Reset poll and wait */
            if (stop_poll == 1) {
               /* Reset fd_pty_master as we do not have to poll again. */
               fd_pty_master = -1;
               /* Remove WNOHANG from wait-options as we just have to wait for the end of the job */
               wait_options &= ~WNOHANG;
            }
         } else if (ret == -1) {
            /* if read was interrupted by a signal we do not have to abort */
            if (errno != EINTR) {
               /* On all other errors we have to exit. */
               stop_job = 1;
               shepherd_trace("Could not poll pty fds. Error: %s", strerror(errno));
            }
         }

         /* If an error occurred we have to terminate the job. */
         if (stop_job == 1) {
            shepherd_trace("An error occurred while executing the job. Job will be terminated.");
            shepherd_signal_job(-pid, SIGTERM);
            /* Reset fd_pty_master as we do not have to poll again. */
            fd_pty_master = -1;
            /* Remove WNOHANG from wait-options as we just have to wait for the end of the job */
            wait_options &= ~WNOHANG;
         }
      }

      handle_signals_and_methods(
         npid,
         pid,
         &postponed_signal,
         ctrl_pid,
         p_ckpt_info,
         &ckpt_cmd_pid,
         &rest_ckpt_interval,
         timeout,
         &migr_cmd_pid,
         &kill_job_after_checkpoint,
         status,
         &inArena,
         &inCkpt,
         childname,
         &job_status,
         &job_pid);

      
   } while ((job_pid > 0) || (migr_cmd_pid > 0) || (ckpt_cmd_pid > 0) ||
            (ctrl_pid[0] > 0) || (ctrl_pid[1] > 0) || (ctrl_pid[2] > 0));

#if defined(HPUX) || defined(INTERIX)
   rusage->ru_utime.tv_sec = rusage_hp10.ru_utime.tv_sec;
   rusage->ru_utime.tv_usec = rusage_hp10.ru_utime.tv_usec;
   rusage->ru_stime.tv_sec = rusage_hp10.ru_stime.tv_sec;
   rusage->ru_stime.tv_usec = rusage_hp10.ru_stime.tv_usec;
#endif

   if (fdout != -1) {
      SGE_CLOSE(fdout);
   }
   if (fderr != -1) {
      SGE_CLOSE(fderr);
   }
   if (fdin != -1) {
      SGE_CLOSE(fdin);
   }

   sge_free(&pty_fds);
   return job_status;
}

/*-------------------------------------------------------------------------
 * set_ckpt_params
 * may not called before "job_pid" is known and set in the configuration
 * don't do anything for non ckpt jobs except setting ckpt_interval = 0
 *-------------------------------------------------------------------------*/
static void set_ckpt_params(int ckpt_type, char *ckpt_command, int ckpt_len,
                            char *migr_command, int migr_len,
                            char *clean_command, int clean_len,
                            int *ckpt_interval) 
{
   char *cmd;
   
   strncpy(ckpt_command, "none", ckpt_len);
   strncpy(migr_command, "none", migr_len);
   strncpy(clean_command,"none", clean_len);

   if (ckpt_type & CKPT_KERNEL) {
      cmd = get_conf_val("ckpt_command");
      if (strcasecmp("none", cmd)) {
         shepherd_trace("%s", cmd);
         replace_params(cmd, ckpt_command, ckpt_len, ckpt_variables);
         shepherd_trace("%s", ckpt_command);
      }

      cmd = get_conf_val("ckpt_migr_command");
      if (strcasecmp("none", cmd)) {
         shepherd_trace("%s", cmd);
         replace_params(cmd, migr_command, migr_len,
                        ckpt_variables);
         shepherd_trace("%s", migr_command);
      }

      cmd = get_conf_val("ckpt_clean_command");
      if (strcasecmp("none", cmd)) {
         shepherd_trace("%s", cmd);
         replace_params(cmd, clean_command, clean_len,
                        ckpt_variables);
         shepherd_trace("%s", clean_command);
      }
   }
      
   /* Don't perform regular checkpoint for userdefined checkpointing */
   if (ckpt_type) {
#if 0
      if (ckpt_type & CKPT_USER)
         *ckpt_interval = 0;
      else   
#endif
         *ckpt_interval = atoi(get_conf_val("ckpt_interval"));
   } else {
      *ckpt_interval = 0;      
   }
}      

/*-------------------------------------------------------------------------
 * set_ckpt_restart_command
 *   may only be called for childname == "job" and in case of restarting
 *   a kernel level checkpointing mechanism
 *   This sets the config entry "job_pid" from "ckpt_pid", which was saved
 *   during the migration. 
 *   "job_pid" is the original pid of the checkpointed job and not the pid 
 *   of the restart command.
 *-------------------------------------------------------------------------*/
static void set_ckpt_restart_command(const char *childname, int ckpt_type, 
                                     char *rest_command, int rest_len) 
{
   char *cmd;
   
   strcpy(rest_command, "none");
   
   /* Need to retrieve job pid here */
   if (add_config_entry("job_pid", get_conf_val("ckpt_pid"))) {
      shepherd_error(1, "adding config entry \"job_pid\" failed");
   }
      
   /* Only set rest_command for "job" */
   if ((!strcmp(childname, "job")) && (ckpt_type & CKPT_REST_KERNEL)) {
      cmd = get_conf_val("ckpt_rest_command");
      if (strcasecmp("none", cmd)) {
         shepherd_trace("%s", cmd);
         replace_params(cmd, rest_command, rest_len, ckpt_variables);
         shepherd_trace("%s", rest_command);
      }    
   }
}

/*-------------------------------------------------------------------------
 * handle_job_pid
 * add job_pid to the configuration
 * set it to pid of job or to original pid of job in case of kernel
 * checkpointing
 * set ckpt_pid to 0 for non kernel level checkpointing jobs
 *-------------------------------------------------------------------------*/
static void handle_job_pid(int ckpt_type, int pid, int *ckpt_pid) 
{
   char pidbuf[64];
   const char *job_pid = NULL;

   /* Set job_pid to real pid or to saved pid for Hibernator restart 
    * for Hibernator restart a part of the job is already done
    */
   if (ckpt_type & CKPT_REST_KERNEL) {
      sprintf(pidbuf, "%s", get_conf_val("ckpt_pid"));
      *ckpt_pid = atoi(get_conf_val("ckpt_pid"));
   } else {   
      sprintf(pidbuf, "%d", pid);
      if (add_config_entry("job_pid", pidbuf))
         shepherd_error(1, "can't add \"job_pid\" entry");
      if (ckpt_type & CKPT_KERNEL)
         *ckpt_pid = pid;
      else
         *ckpt_pid = 0;   
   }
   job_pid = get_conf_val("job_pid");
      
   if (shepherd_write_job_pid_file(job_pid) == false) {
      /* No use to go on further */
      shepherd_signal_job(pid, SIGKILL);
      shepherd_error(1, "can't write \"job_pid\" file");   
   } 
}   

/*-------------------------------------------------------------------------*/
static int start_async_command(const char *descr, char *cmd)
{
   int pid;
   char err_str[512];
   char *cwd;
   struct passwd *pw=NULL;
   struct passwd pw_struct;
   char *buffer;
   int size;

   size = get_pw_buffer_size();
   buffer = sge_malloc(size);
   pw = sge_getpwnam_r(get_conf_val("job_owner"), &pw_struct, buffer, size);

   if (!pw) {
      shepherd_error(1, "can't get password entry for user \"%s\"",
                     get_conf_val("job_owner"));
   }
   /* the getpwnam is only a verification - the result is not used - free it */
   sge_free(&buffer);

   /* Create "error" and "exit_status" files here */
   shepherd_error_init();
                            
   if ((pid = fork()) == -1) {
      shepherd_trace("can't fork for starting %s command", descr);
   } else if (pid == 0) {
      int use_qsub_gid;
      gid_t gid;
      char *tmp_str;
      bool skip_silently = false;
      
      shepherd_trace("starting %s command: %s", descr, cmd);
      pid = getpid();
      setpgid(pid, pid);
      setrlimits(0);
      sge_set_environment(true);
      umask(022);
      tmp_str = search_conf_val("qsub_gid");
      if (tmp_str && strcmp(tmp_str, "no")) {
         use_qsub_gid = 1;
         gid = atol(tmp_str);
      } else {
         use_qsub_gid = 0;       
         gid = 0;
      }
      tmp_str = search_conf_val("skip_ngroups_max_silently");
      if (tmp_str && strcmp(tmp_str, "yes")) {
         skip_silently = true;
      }
      {
         int min_gid = atoi(get_conf_val("min_gid"));
         int min_uid = atoi(get_conf_val("min_uid"));
         gid_t add_grp_id = 0;
         char *cp = search_conf_val("add_grp_id");

         if (cp) add_grp_id = atol(cp);
         if (sge_set_uid_gid_addgrp(get_conf_val("job_owner"), NULL,
                                    min_gid, min_uid, add_grp_id,
                                    err_str, sizeof(err_str), use_qsub_gid,
                                    gid, skip_silently) > 0) {
            shepherd_trace("%s", err_str);
            exit(1);
         }
      }

      sge_close_all_fds(NULL, 0);

      /* we have to provide the async command with valid io file handles
       * else it might fail 
       */
      if((open("/dev/null", O_RDONLY, 0) != 0) || 
         (open("/dev/null", O_WRONLY, 0) != 1) || 
         (open("/dev/null", O_WRONLY, 0) != 2)) {
         shepherd_trace("error opening std* file descriptors");
         exit(1);
      }   

      foreground = 0;

      cwd = get_conf_val("cwd");
      if (sge_chdir(cwd)) {
         shepherd_trace("%s: can't chdir to %s", descr, cwd);
      }   

      sge_set_def_sig_mask(NULL, NULL);
      /* fixme: sanitize environment to avoid possible trouble, c.f. #1448 */
      start_command(descr, "/bin/sh", cmd, cmd, "start_as_command",
                    0, 0, 0, 0, "", 0);
      return 0;   
   }

   return pid;
}

/*-------------------------------------------------------------------------*/
static void start_clean_command(char *cmd) 
{
   int pid, npid, status;

   shepherd_trace("starting ckpt clean command");

   if (!cmd || *cmd == '\0' || !strcasecmp(cmd, "none")) {
      shepherd_trace("no checkpointing clean command to start");
      return;
   }
         
   if ((pid = start_async_command("clean", cmd)) == -1) {
      shepherd_trace("couldn't start clean command");
      return;
   }   
   
   
   do {
      npid = waitpid(pid, &status, 0);
   } while ((npid <= 0) || (!WIFSIGNALED(status) && !WIFEXITED(status)) ||
            (npid != pid));

   if (WIFSIGNALED(status))
      shepherd_trace("clean command died through signal");
}     

/****************************************************************
 Special version of signal for the shepherd.
 Should be used to signal the job. 
 We have special handling for architectures which have a reliable 
 grouping mechanism like sgi or cray. This version reads the osjobid
 and uses it instead of the pid. If reading or killing fails, the normal
 mechanism is used.
 ****************************************************************/
void 
shepherd_signal_job(pid_t pid, int sig) {
#if defined(IRIX)
   static int first = 1;
   static ash_t osjobid = 0;
# endif


   /* 
    * Normal signaling for OSes without reliable grouping mechanisms and if
    * special signaling fails (e.g. not running as root)
    */

   /*
    * if child is a qrsh job (config rsh_daemon exists), get pid of started command
    * and pass signal to that one
    * if the signal is the kill signal, we first kill the pid of the started command.
    * subsequent kills are passed to the shepherds child.
    */
   {
      static int first_kill = 1;
      static u_long32 first_kill_ts = 0;
      static bool is_qrsh = false;
   
      if (first_kill == 1 || sig != SIGKILL) {
         if (search_conf_val("qrsh_pid_file") != NULL) {
            char *pid_file_name = NULL;
            pid_t qrsh_pid = 0;

            pid_file_name = get_conf_val("qrsh_pid_file");

            sge_switch2start_user();

            if (shepherd_read_qrsh_file(pid_file_name, &qrsh_pid)) {
               is_qrsh = true;
               pid = -qrsh_pid;
               shepherd_trace("found pid of qrsh client command: "pid_t_fmt, pid);
            }
            sge_switch2admin_user();
         }
      }

     /*
      * It is possible that one signal request from qmaster contains severeal
      * kills for the same process. If this process is a tight integrated job
      * the master task can be killed twice. For the slave tasks this means the
      * qrshd is killed in the same time as the qrsh_starter child and so no
      * qrsh_exit_code file is written (see Issue: 1679)
      */
      if ((first_kill == 1) || (sge_get_gmt() - first_kill_ts > 10) || (sig != SIGKILL)) {
        shepherd_trace("now sending signal %s to pid "pid_t_fmt, sge_sys_sig2str(sig), pid);
        sge_switch2start_user();
        kill(pid, sig);
        sge_switch2admin_user();

#if defined(SOLARIS) || defined(LINUX) || defined(ALPHA) || defined(IRIX) || defined(FREEBSD) || defined(DARWIN)
        if (first_kill == 0 || sig != SIGKILL || is_qrsh == false) {
#   if defined(SOLARIS) || defined(LINUX) || defined(ALPHA) || defined(FREEBSD) || defined(DARWIN)
#      ifdef COMPILE_DC
            if (atoi(get_conf_val("enable_addgrp_kill")) == 1) {
                gid_t add_grp_id;
                char *cp = search_conf_val("add_grp_id");

                if (cp) {
                    add_grp_id = atol(cp);
                } else {
                    add_grp_id = 0;
                }

                shepherd_trace("pdc_kill_addgrpid: %d %d", (int) add_grp_id , sig);
                sge_switch2start_user();
                pdc_kill_addgrpid(add_grp_id, sig, shepherd_trace);
                sge_switch2admin_user();
            }
#      endif
#   elif defined(IRIX)
            if (first == 1) {
                shepherd_read_osjobid_file(&osjobid, false);
                first = 0;
            }
            if (osjobid == 0) {
                shepherd_trace("value in \"osjobid\" file = 0, not using kill_ash/killm");
            } else {
                sge_switch2start_user();
                kill_ash(osjobid, sig, sig == 9);
                sge_switch2admin_user();
            }
#   endif
        }
# endif
      } else {
        shepherd_trace("ignored signal %s to pid "pid_t_fmt, sge_sys_sig2str(sig), pid);
      }

      if (sig == SIGKILL) {
        first_kill = 0;
        first_kill_ts = sge_get_gmt();
      }
   }
}

/*------------------------------------------------------------------*/
static pid_t start_token_cmd(int wait_for_finish, const char *cmd,
   const char *arg1, const char *arg2, const char *arg3) 
{
   pid_t pid;
   pid_t ret;

   pid = fork();
   if (pid < 0) {
      return -1;
   } else if (pid == 0) {
      if (!wait_for_finish && (getenv("SGE_DEBUG_LEVEL"))) {
         putenv("SGE_DEBUG_LEVEL=0 0 0 0 0 0 0 0");
      }   
      execle(cmd, cmd, arg1, arg2, arg3, (char *) NULL, sge_get_environment ());
      exit(1);
   } else if (wait_for_finish) {
        ret = do_wait(pid);
        return ret;
   } else {
      return pid;
   }

   /* just to please insure */
   return -1;
}

/*------------------------------------------------------------------*/
static int do_wait(pid_t pid) {
   pid_t npid;
   int status, exit_status;

   /* This loop only ends if the process exited normally or
    * died through signal
    */
   do {
      npid = waitpid(pid, &status, 0);
   } while ((npid <= 0) || (!WIFSIGNALED(status) && !WIFEXITED(status)) ||
            (npid != pid));

   if (WIFEXITED(status)) {
      exit_status = WEXITSTATUS(status);
   } else if (WIFSIGNALED(status)) {
      exit_status = 128 + WTERMSIG(status);
   } else {
      exit_status = 255;
   }

   return exit_status;
}

/*-------------------------------------------------------------------
 * set_sig_handler
 * define a signal handler for SIGPIPE
 * to avoid that unblocked signal will kill us
 *-------------------------------------------------------------------*/
static void set_sig_handler(int sig_num) {
   struct sigaction sa;
      
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = shepherd_signal_handler;
   sigemptyset(&sa.sa_mask);

#ifdef SA_RESTART
   sa.sa_flags = SA_RESTART;
#endif
      
   sigaction(sig_num, &sa, NULL);
}

/*-------------------------------------------------------------------*/
static void shepherd_signal_handler(int dummy) {
   /* may not log in signal handler 
      as long as Async-Signal-Safe functions such as fopen() are used in 
      shepherd logging code */
#if 0
   shepherd_trace("SIGPIPE received");
#endif

}

static void
cpusetting(void)
{
   u_long32 job = atoi(get_conf_val("job_id"));
   u_long32 task = MAX(1, atoi(get_conf_val("ja_task_id")));
   char *binding = (char *) getenv("SGE_BINDING");
   char path[SGE_PATH_MAX], child[64];
   pid_t pid = getpid();

   if (have_cgroup(cg_cpuset)) {
      if (!get_cgroup_job_dir(cg_cpuset, path, sizeof path, job, task)) {
         shepherd_trace("no job task cpuset");
         goto cpuset_end;
      }
      snprintf(child, sizeof child, pid_t_fmt, pid);
      errno = 0;
      if (!make_sub_cgroup(cg_cpuset, path, child)) {
         shepherd_trace("Can't make cpuset %s/%s: %s",
                        path, child, strerror(errno));
         goto cpuset_end;
      }
      if (binding) {
         binding = strdup(binding);
         /* We got a space-separated binding string which needs to be
            comma-separated.  */
         replace_char(binding, strlen(binding), ' ', ',');
         errno = 0;
         if (write_to_shepherd_cgroup (cg_cpuset, "cpus",
                                       binding, job, task, pid))
            shepherd_trace("set cpuset cpus per core binding");
         else
            shepherd_trace("failed to set cpuset with euid "uid_t_fmt": "SFN,
                           geteuid(), strerror(errno));
         free (binding);
      }
      /* maybe the shepherd shouldn't be in the group, and just the
         child should be in it */
      if (!set_shepherd_cgroup(cg_cpuset, job, task, pid)) {
         shepherd_trace("Can't set shepherd cpuset: %s", strerror(errno));
         goto cpuset_end;
      }
   }
 cpuset_end:
   return;
}
