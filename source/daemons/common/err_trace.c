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
/* Error and trace handling for shepherd
 * Error/Trace files will go to the actual directory at first time
 * shepherd_error() and trace() are called
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <pthread.h>

#if defined(DARWIN) || defined(FREEBSD) || defined(NETBSD)
#  include <sys/param.h>
#  include <sys/mount.h>
#elif defined(LINUX)
#  include <sys/vfs.h>
#else
#  include <sys/statvfs.h>
#endif

#include "uti/sge_dstring.h"
#include "uti/sge_stdio.h"
#include "uti/sge_unistd.h"
#include "uti/sge_time.h"
#include "uti/sge_uidgid.h"
#include "uti/config_file.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"

#include "basis_types.h"
#include "err_trace.h"
#include "qlogin_starter.h"

#if defined(INTERIX)
#  include "wingrid.h"
#endif

static FILE *shepherd_error_fp=NULL;
static FILE *shepherd_exit_status_fp=NULL;
static FILE *shepherd_trace_fp = NULL;

typedef enum st_shepherd_file_def {
   st_trace,
   st_exit_status,
   st_error
} st_shepherd_file_t;

static char *g_shepherd_file_name[3] = {"trace", "exit_status", "error"};
static char  g_shepherd_file_path[3][SGE_PATH_MAX];

static char g_job_owner[SGE_PATH_MAX] = "";
static bool g_keep_files_open = true; /* default: Open files at start and keep
                                                  them open for writing */
static pthread_mutex_t g_trace_mutex;

extern pid_t coshepherd_pid;
extern int   shepherd_state;  /* holds exit status for shepherd_error() */
extern bool  g_new_interactive_job_support;
int foreground = 1;           /* usability of stderr/out */

/* Forward declaration of static functions */

static void  dup_fd(int *src_fd);
static int   set_cloexec(int fd);
static int   sh_str2file(const char *header_str, const char *str, FILE *fp);
static FILE* shepherd_trace_init_intern(st_shepherd_file_t shepherd_file);
static void  shepherd_trace_chown_intern(const char* job_owner, FILE* fp,
                                         st_shepherd_file_t shepherd_file);
static bool  nfs_mounted(const char *path);
static void  shepherd_panic(const char *s);


/******************************************************************************
* "Public" functions
*******************************************************************************/
/****** shepherd_trace_init/shepherd_trace_exit *******************************
*  NAME
*     shepherd_trace_init() -- Begin shepherd's tracing.
*     shepherd_trace_exit() -- End shepherd's tracing.
*
*  SYNOPSIS
*    void shepherd_trace_init( void )
*    void shepherd_trace_exit( void )
*
*  FUNCTION
*     Init: Creates or opens trace file.
*     Exit: Closes trace file. Call it as shepherd's last function.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_trace_init(void)
{
   if (!shepherd_trace_fp) {
      shepherd_trace_fp = shepherd_trace_init_intern(st_trace);
   }
}

void shepherd_trace_exit(void)
{
    int  old_euid = SGE_SUPERUSER_UID;

   /*
    * Work around for CR 6293411:
    * Some NFS Server have a bug. They test permissions on
    * a already open file pointer again when trying to write
    * to the file.
    * This bug affects us only in case that root has rw-permissions
    * on the NFS mount, otherwise we can write as any user to
    * the trace file.
    * The work around consists of switching to root and back
    * for writing and closing the file.
    */
    if (getuid() == SGE_SUPERUSER_UID) {
        old_euid = geteuid();
        sge_seteuid(SGE_SUPERUSER_UID);
     }

    if (shepherd_trace_fp) {
       FCLOSE(shepherd_trace_fp);
FCLOSE_ERROR:
      shepherd_trace_fp=NULL;
    }

   /*
    * Switch back to admin user?
    */
    if (old_euid != SGE_SUPERUSER_UID) {
       sge_seteuid(old_euid);
    }

    shepherd_error_exit();
}

/****** shepherd_trace_chown **************************************************
*  NAME
*     shepherd_trace_chown() -- Change owner of trace file.
*
*  SYNOPSIS
*    void shepherd_trace_chown(const char* job_owner)
*
*  FUNCTION
*    Changes owner and group of trace file to job_owner.
*
*  INPUTS
*     job_owner: Name of the new owner of the file.
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_trace_chown(const char* job_owner)
{
   shepherd_trace_chown_intern(job_owner, shepherd_trace_fp, st_trace);
}

/****** err_trace/shepherd_trace_exit() **************************************
*  NAME
*     shepherd_trace_exit() -- End shepherd's tracing.
*
*  SYNOPSIS
*     static FILE* shepherd_trace_exit()
*
*  FUNCTION
*     Closes the shepherd's trace file.
*     Call it as shepherd's last function.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_error_init()
{
   if (shepherd_error_fp == NULL) {
      shepherd_error_fp = shepherd_trace_init_intern(st_error);
   }
   if (shepherd_exit_status_fp == NULL) {
      shepherd_exit_status_fp = shepherd_trace_init_intern(st_exit_status);
   }
}

void shepherd_error_exit()
{
    int  old_euid = SGE_SUPERUSER_UID;

   /*
    * Work around for CR 6293411:
    * See shepherd_trace_exit() for details.
    */
   if (getuid() == SGE_SUPERUSER_UID) {
      old_euid = geteuid();
      sge_seteuid(SGE_SUPERUSER_UID);
   }

   /*
    * Close file handles
    */
   if (shepherd_error_fp) {
      FCLOSE_IGNORE_ERROR(shepherd_error_fp);
      shepherd_error_fp = NULL;
   }
   if (shepherd_exit_status_fp) {
      FCLOSE_IGNORE_ERROR(shepherd_exit_status_fp);
      shepherd_exit_status_fp = NULL;
   }

   /*
    * Switch back to admin user?
    */
   if (old_euid != SGE_SUPERUSER_UID) {
      sge_seteuid(old_euid);
   }
}

/****** shepherd_error_chown **************************************************
*  NAME
*     shepherd_error_chown() -- Change owner of error and exit_status files.
*
*  SYNOPSIS
*    void shepherd_error_chown(const char* job_owner)
*
*  FUNCTION
*    Changes owner and group of error and exit_status file to job_owner.
*
*  INPUTS
*     job_owner: Name of the new owner of the file.
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_error_chown(const char* job_owner)
{
   shepherd_trace_chown_intern(job_owner, shepherd_error_fp, st_error);
   shepherd_trace_chown_intern(job_owner, shepherd_exit_status_fp, st_exit_status);
}

/****** shepherd_trace ********************************************************
*  NAME
*     shepherd_trace() -- Write line to trace file.
*
*  SYNOPSIS
*     int shepherd_trace(const char *format, ...)
*
*  FUNCTION
*     Writes a line to the trace file, preceding it with a date, time, uid
*     and pid stamp.
*
*  INPUTS
*     format: The format string of the line to be written to the error file.
*     ...: The parameters to the format string. See printf(3c).
*
*  RESULT
*     int - 0 if successful, 1 if an error occurred.
*******************************************************************************/
int shepherd_trace(const char *format, ...)
{
   dstring     ds;
   dstring     message = DSTRING_INIT;
   char        buffer[128];
   char        header_str[256];
   int         ret=1;
   int         old_cancelstate;
   struct stat statbuf;

   /* Protect the trace file pointer with a mutex */
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate);
   pthread_mutex_lock(&g_trace_mutex);

   /* File was closed (e.g. by an exec()) but fp was not set to NULL */
   if (shepherd_trace_fp
       && fstat(fileno(shepherd_trace_fp), &statbuf) == -1
       && errno == EBADF) {
      shepherd_trace_fp = NULL;
   }

   if (shepherd_trace_fp == NULL) {
      shepherd_trace_fp = shepherd_trace_init_intern(st_trace);
   }

   if (shepherd_trace_fp != NULL) {
      sge_dstring_init(&ds, buffer, sizeof(buffer));

      snprintf(header_str, sizeof(header_str), "%s ["uid_t_fmt":"pid_t_fmt"]: ",
               sge_ctime(0, &ds), geteuid(), getpid());

      if (format != NULL) {
         va_list     ap;

         va_start(ap, format);
         sge_dstring_vsprintf(&message, format, ap);
         va_end(ap);

         ret = sh_str2file(header_str, sge_dstring_get_string(&message),
                           shepherd_trace_fp);

         if (foreground) {
            printf("%s%s\n", header_str, sge_dstring_get_string(&message));
            fflush(stdout);
         }
         sge_dstring_free(&message);
      }
      /* There are cases where we have to open and close the files
       * for every write.
       */
      if (!g_keep_files_open) {
         shepherd_trace_exit();
      }
      ret=0;
   }

   pthread_mutex_unlock(&g_trace_mutex);
   pthread_setcancelstate(old_cancelstate, NULL);
   return ret;
}

/****** shepherd_error ********************************************************
*  NAME
*     shepherd_error() -- Write a line to the error file and optionally exit program.
*
*  SYNOPSIS
*     void shepherd_error(bool do_exit, const char *format, ...)
*
*  FUNCTION
*     Writes a line to the error file, preceding it with a
*     date, time, uid and pid stamp, and optionally exits the program.
*
*  INPUTS
*     do_exit: If true, this function calls exit(2).
*     format: The format string of the line to be written to the error file.
*     ...: The parameters to the format string. See printf(3c).
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_error(int do_exit, const char *format, ...)
{
   dstring     ds;
   dstring     message = DSTRING_INIT;
   char        buffer[128];
   char        header_str[256];
   struct stat statbuf;

   if (format != NULL) {
      va_list     ap;

      va_start(ap, format);
      sge_dstring_vsprintf(&message, format, ap);
      va_end(ap);
   }

   shepherd_trace("%s", sge_dstring_get_string(&message));

   /* File was closed (e.g. by an exec()) but fp was not set to NULL */
   if (shepherd_error_fp
       && fstat(fileno(shepherd_error_fp), &statbuf) == -1
       && errno==EBADF) {
      shepherd_error_fp = NULL;
   }
   if (shepherd_error_fp == NULL) {
      shepherd_error_fp = shepherd_trace_init_intern(st_error);
   }
   if (shepherd_error_fp != NULL) {
      sge_dstring_init(&ds, buffer, sizeof(buffer));
      snprintf(header_str, sizeof(header_str), "%s ["uid_t_fmt":"pid_t_fmt"]: ",
               sge_ctime(0, &ds), geteuid(), getpid());

      sh_str2file(header_str, sge_dstring_get_string(&message), shepherd_error_fp);
   }

   if (foreground) {
      fprintf(stderr, "%s%s\n", header_str, sge_dstring_get_string(&message));
   }

   /* File was closed (e.g. by an exec()) but fp was not set to NULL */
   if (shepherd_exit_status_fp
       && fstat(fileno(shepherd_exit_status_fp), &statbuf) == -1
       && errno==EBADF ) {
      shepherd_exit_status_fp = NULL;
   }
   if (shepherd_exit_status_fp == NULL) {
      shepherd_exit_status_fp = shepherd_trace_init_intern(st_exit_status);
   }
   if (shepherd_exit_status_fp != NULL) {
      sprintf(header_str, "%d", shepherd_state);
      sh_str2file(header_str, NULL, shepherd_exit_status_fp);
   }

   if (coshepherd_pid > 0) {
      sge_switch2start_user();
      kill(coshepherd_pid, SIGTERM);
      sge_switch2admin_user();
   }

   if (g_new_interactive_job_support == false &&
      search_conf_val("qrsh_control_port") != NULL) {
      char buffer[1024];
      snprintf(buffer, 1024, "1:%s", sge_dstring_get_string(&message));
      write_to_qrsh(buffer);
   }
   sge_dstring_free(&message);

   if (do_exit) {
      /* close all trace files before exit */
     shepherd_trace_exit();
     exit(shepherd_state);
   }
   /* There are cases where we have to open and close the files
    * for every write.
    */
   if (!g_keep_files_open) {
      shepherd_error_exit();
   }
}

void shepherd_error_ptr(const char *text)
{
   shepherd_error(1, "%s", text);
}

/****** shepherd_write_exit_status ********************************************
*  NAME
*     shepherd_write_exit_status() -- Write exit status to exit_status file.
*
*  SYNOPSIS
*     void shepherd_write_exit_status(const char *exit_status)
*
*  FUNCTION
*     Writes the exit status to the exit_status file.
*
*  INPUTS
*     exit_status: The exit status of the shepherd.
*
*  RESULT
*     void - none
*******************************************************************************/
void shepherd_write_exit_status(const char *exit_status)
{
   struct stat statbuf;
#if 1
   int old_euid = SGE_SUPERUSER_UID;
#endif

   if (exit_status != NULL) {

#if 1 /* on filesystems where root is mapped to nobody this will not work */
      /* set euid=0. Local files: root can write to every file.
       * NFS files: everyone is allowed to write to exit_status file.
       */
      if (getuid() == SGE_SUPERUSER_UID) {
         old_euid = geteuid();
         sge_seteuid(SGE_SUPERUSER_UID);
      }
#endif
      /* File was closed (e.g. by an exec()) but fp was not set to NULL */
      if (shepherd_exit_status_fp
          && fstat(fileno(shepherd_exit_status_fp), &statbuf) == -1
          && errno == EBADF) {
         shepherd_exit_status_fp = NULL;
      }
      if (!shepherd_exit_status_fp) {
         shepherd_exit_status_fp = shepherd_trace_init_intern(st_exit_status);
      }
      if (shepherd_exit_status_fp) {
         sh_str2file(exit_status, NULL, shepherd_exit_status_fp);
      } else {
         shepherd_trace("could not write exit_status file\n");
      }
#if 1
      if (old_euid != SGE_SUPERUSER_UID) {
         sge_seteuid(old_euid);
      }
#endif
      /* There are cases where we have to open and close the files
       * for every write.
       */
      if (!g_keep_files_open) {
         shepherd_error_exit();
      }
   }
}

/****** is_shepherd_trace_fd **************************************************
*  NAME
*     is_shepherd_trace_fd() -- Check if given file descriptor is a file descr.
*                               to the trace, error or exit_status file.
*
*  SYNOPSIS
*     int is_shepherd_trace_fd(int fd)
*
*  FUNCTION
*     Checks if the given file descriptor is a file descriptor of the open file
*     handles to the trace, error or exit_status file.
*     This function is needed during daemonizing to close all file descriptors
*     except the ones to the trace, error and exit_status files.
*
*  INPUTS
*     fd: The file descriptor that has to be checked.
*
*  RESULT
*     int - 1 if fd is a file descriptor to trace, error or exit_status file,
*           0 if it is not.
*******************************************************************************/
int is_shepherd_trace_fd(int fd)
{
   if ((shepherd_trace_fp && fd==fileno(shepherd_trace_fp))
      || (shepherd_error_fp && fd==fileno(shepherd_error_fp))
      || (shepherd_exit_status_fp && fd==fileno(shepherd_exit_status_fp))) {
      return 1;
   } else {
      return 0;
   }
}

/****** count_exit_status *****************************************************
*  NAME
*     count_exit_status() -- Return the number of lines in the exit status file
*
*  SYNOPSIS
*     int count_exit_status(void)
*
*  FUNCTION
*     Returns the number of lines in the exit_status file.
*     Each pe_start, prolog, job, epilog and pe_stop writes a line to the
*     exit_status file.
*     This function is used to detect where the shepherd fails.
*
*  INPUTS
*     void - none
*
*  RESULT
*     int - Number of lines in the exit_status file.
*******************************************************************************/
int count_exit_status(void)
{
   int n = 0;
   SGE_STRUCT_STAT sbuf;
   char buf[1024];

   if (!SGE_STAT("exit_status", &sbuf) && sbuf.st_size) {
      FILE *fp;
      int dummy;

      if ((fp = fopen("exit_status", "r"))) {
         while (fgets(buf, sizeof(buf), fp)) {
            if (sscanf(buf, "%d\n", &dummy) == 1) {
               n++;
            }
         }
         FCLOSE(fp);
      }
   }
FCLOSE_ERROR:
   return n;
}


/**********************************/
/* Static functions */

static void dup_fd(int *src_fd)
{
   int dup_fd[3];

   dup_fd[0] = dup(*src_fd);
   dup_fd[1] = dup(*src_fd);
   dup_fd[2] = dup(*src_fd);

   close(*src_fd);
   close(dup_fd[0]);
   close(dup_fd[1]);

   *src_fd = dup_fd[2];
}

static int set_cloexec(int fd)
{
   int oldflags;

   oldflags = fcntl(fd, F_GETFD, 0);
   if (oldflags < 0) {
      return 0;
   }

   oldflags |= FD_CLOEXEC;
   fcntl(fd, F_SETFD, oldflags);

   return 1;
}

/*-----------------------------------------------------------------*/
static int sh_str2file(const char *header_str, const char *str, FILE* fp)
{
   int     ret = 1;
   int     ret_fp = -1;
   int     ret_fl = EOF;
   dstring ds;
   char    buffer[MAX_STRING_SIZE];
   uid_t   old_euid = SGE_SUPERUSER_UID;

   if (fp) {
      /*
       * Work around for CR 6293411:
       * See shepherd_trace_exit() for details.
       */
      if (getuid() == SGE_SUPERUSER_UID) {
         old_euid = geteuid();
         sge_seteuid(SGE_SUPERUSER_UID);
      }

      if (!str && !header_str) {
         ret_fp = fprintf(fp, "function sh_str2file() called with "
                              "NULL arguments\n");
      } else if (!header_str && str) {
         ret_fp = fprintf(fp, "%s\n", str);
      } else if (header_str && !str) {
         ret_fp = fprintf(fp, "%s\n", header_str);
      } else {
         ret_fp = fprintf(fp, "%s%s\n", header_str, str);
      }

      if (ret_fp >= 0) {
         ret_fl = fflush(fp);
         if (ret_fl == 0) {
            ret = 0;
         }
      }
      /*
       * Switch back to admin user?
       */
      if (old_euid != SGE_SUPERUSER_UID) {
         sge_seteuid(old_euid);
         old_euid = SGE_SUPERUSER_UID;
      }

      /*
       * PANIC! Can't write to trace/error/exit_status file!
       */
      if (ret_fp < 0) {
         sge_dstring_init(&ds, buffer, sizeof(buffer));
         sge_dstring_sprintf(&ds, "fprintf(%lx,%s,%s) failed: %s",
                             (unsigned long)fp, header_str?header_str:"<null>",
                             str?str:"<null>", strerror(errno));
         shepherd_panic(buffer);
      }
      if(ret_fl != 0) {
         sge_dstring_init(&ds, buffer, sizeof(buffer));
         sge_dstring_sprintf(&ds, "fflush(%lx) failed: %s",
                             (unsigned long)fp, strerror(errno));
         shepherd_panic(buffer);
      }
   }
   return ret;
}

/****** err_trace/shepherd_trace_init_intern() *******************************
*  NAME
*     shepherd_trace_init_intern() -- Initialize shepherd's tracing.
*
*  SYNOPSIS
*     static FILE* shepherd_trace_init(char *trace_file_path, char *trace_file_name)
*
*  FUNCTION
*     Opens the shepherd's trace file and sets the FD_CLOEXEC-flag so it will
*     be closed automatically in an exec()-call.
*     Must be called with euid=admin user to work properly!
*
*  INPUTS
*     char *trace_file_path - either the whole path of the trace file (including
*                             the file itself)
*                             or NULL to retrieve the file pointer of an already
*                             opened trace file.
*     char *trace_file_name - the name of the trace file itself. Ignored when
*                             *trace_file_path is NULL.
*
*  RESULT
*     FILE* - If successfully opened, the file pointer of shepherd's trace file.
*           - Otherwise NULL.
*******************************************************************************/
static FILE* shepherd_trace_init_intern(st_shepherd_file_t shepherd_file)
{
   static char     path[SGE_PATH_MAX];
   static bool     called = false;
   SGE_STRUCT_STAT statbuf;
   dstring         ds;
   char            buffer[SGE_PATH_MAX+128];
   char            tmppath[SGE_PATH_MAX];
   int             fd       = -1;
   FILE            *fp      = NULL;
   int             do_chown = 0;

   /*
    *  after changing into the jobs cwd we need an
    *  absolute path to the error/trace file
    */
   if (called == false) {
      if (getcwd(path, sizeof(path)) == NULL) {
         sge_dstring_init(&ds, buffer, sizeof(buffer));
         sge_dstring_sprintf(&ds, "getcwd() failed: %s", strerror(errno));
         shepherd_panic(buffer);
      }
      called=true;
   }

   snprintf(tmppath, SGE_PATH_MAX,"%s/%s",path, g_shepherd_file_name[shepherd_file]);
   sge_strlcpy(g_shepherd_file_path[shepherd_file], tmppath, SGE_PATH_MAX);

   /* If the file does not exist, create it. Otherwise just open it. */
   if (SGE_STAT(tmppath, &statbuf)) {
      fd = open(tmppath, O_RDWR | O_CREAT | O_APPEND,
                /* (shepherd_file == st_trace) ? 0600 : */ 0644);
      if (fd<0) {
         sge_dstring_init(&ds, buffer, sizeof(buffer));
         sge_dstring_sprintf(&ds, "creat(%s) failed: %s", tmppath, strerror(errno));
         shepherd_panic(buffer);
      }

      if (getuid() == SGE_SUPERUSER_UID) {
         /* We must give the file to the job owner later */
         do_chown = 1;
      } else {
         /* We are not root, so we have to own all files anyway. */
         do_chown = 0;
      }
   } else {
      /* The file already exists. We get here when
       * a) a exec() failed or
       * b) after the execution of prolog/job, when the job/epilog
       * tries to init the error/exit status files.
       *
       * In a root system we can just open the file, because we are either
       * root or the job user who owns the file.
       * In a admin user system we must set our euid to root to open it, then
       * it is the same as the root system.
       * In a test user system we are the owner of the file and can open it.
       *
       * When we are root (masked or not), we gave this file to the
       * prolog user/job user right after its creation. But we can have
       * 3 different users for prolog, job and epilog, so we must give
       * the file here to the next user.
       * This must be done via shepherd_trace_chown() in the shepherd
       * before we switch to this user there.
       * It can't be done here because we don't know if we are in
       * case a) (exec failed) or case b) (after execution of prolog/job).
       */
      int  old_euid = SGE_SUPERUSER_UID;

      /*
       * Work around for CR 6293411:
       * See shepherd_trace_exit() for details.
       */
      if (getuid() == SGE_SUPERUSER_UID) {
         old_euid = geteuid();
         sge_seteuid(SGE_SUPERUSER_UID);
      }

      fd = SGE_OPEN2(tmppath, O_RDWR | O_APPEND);
      if (fd<0) {
         sge_dstring_init(&ds, buffer, sizeof(buffer));
         sge_dstring_sprintf(&ds, "open(%s) failed: %s",
                             tmppath, strerror(errno));
         shepherd_panic(buffer);
      }
      do_chown = 0;

      /*
       * Switch back to admin user?
       */
      if (old_euid != SGE_SUPERUSER_UID) {
         sge_seteuid(old_euid);
      }
   }

   /* Something went wrong. */
   if (fd<0) {
      return NULL;
   }

   /* To avoid to block stdin, stdout or stderr, dup the fd until it is >= 3 */
   if (fd<3) {
      dup_fd(&fd);
   }

   /* Set FD_CLOEXEC flag to automatically close the file in an exec() */
   if (!set_cloexec(fd)) {
      shepherd_panic("set_cloexec() failed");
      return NULL;
   }

   /*
    * Now open a FILE* from the file descriptor, so we can use fprintf().
    */
   fp = fdopen(fd, "a");
   if (!fp) {
      sge_dstring_init(&ds, buffer, sizeof(buffer));
      sge_dstring_sprintf(&ds, "can't open %s file \"%s\": %s\n",
                          g_shepherd_file_name[shepherd_file], tmppath,
                          strerror(errno));
      shepherd_panic(buffer);
      return NULL;
   }
   if (do_chown && strlen(g_job_owner) > 0) {
      shepherd_trace_chown_intern(g_job_owner, fp, shepherd_file);
   }
   return fp;
}

static void shepherd_panic(const char *s)
{
   dstring ds;
   char buffer[MAX_STRING_SIZE];

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   sge_dstring_sprintf_append(&ds, "["uid_t_fmt":"uid_t_fmt" "pid_t_fmt"]: PANIC: %s\n",
                              getuid(), geteuid(), getpid(), s);
   syslog(LOG_USER, "%s", sge_dstring_get_string(&ds));
   return;
}

/* In an admin user system, this function must be called as admin user! */
static void shepherd_trace_chown_intern(const char* job_owner, FILE* fp,
                                        st_shepherd_file_t shepherd_file)
{
   int   fd;
   uid_t jobuser_id;
   gid_t jobuser_gid;
   char  buffer[1024];
   int   old_euid = SGE_SUPERUSER_UID;

   if (fp && strlen(job_owner) > 0) {
      fd = fileno(fp);

      /* If uid != 0, the system is installed as a test user system.
       * We don't have to change any file ownerships there. */
      if (getuid() == SGE_SUPERUSER_UID) {
         /* root */
         if (g_job_owner != job_owner)
            strcpy(g_job_owner, job_owner);
         if (sge_user2uid(job_owner, &jobuser_id, &jobuser_gid, 1) == 0) {
            /* Now try to give the file to the job user. root (and later
             * the admin user) will still be able to write to it through
             * the open file descriptor.
             * We must do this as root, because only root has the permissions
             * to change the ownership of a file.
             */
            old_euid = geteuid();
            sge_seteuid(SGE_SUPERUSER_UID);

            /* Have to use chown() here, because fchown() has some bugs
             * on True64 and Irix.
             */
            if (chown(g_shepherd_file_path[shepherd_file],
                      jobuser_id, jobuser_gid) != 0) {
               /* chown failed. This means that user root is a normal user
                * for the file system (due to NFS rights). So we have no
                * other chance than open the file for writing for everyone.
                * We must do this as file owner = admin user.
                */
               sge_seteuid(old_euid);
               if (fchmod(fd, 0666) == -1) {
                  snprintf(buffer, sizeof(buffer),
                          "can't fchmod(fd, 0666): %s\n", strerror(errno));
                  shepherd_panic(buffer);
                  return;
               }
            } else {
               /* chown worked. But when we can chown but are on a NFS
                * mounted drive (which means root has all privileges on this
                * mounted drive), we cannot append from two places
                * (shepherd and son/job) to the shepherd files. So we
                * have to open and close the files for every write, which
                * is possible because we are root with all privileges
                * and we give the ownership to the
                * prolog/pe_start/job/pe_stop/epilog user.
                */
               if (g_keep_files_open && nfs_mounted(".")) {
                  g_keep_files_open = false;
               }
            }
            sge_seteuid(old_euid);
         } else {
            /* Can't get jobuser_id -> grant access for everyone */
            if (fchmod(fd, 0666)==-1) {
               snprintf(buffer, sizeof(buffer),
                        "could not fchmod(): %s", strerror(errno));
               shepherd_panic(buffer);
            }
         }
      }
   }
}

static bool nfs_mounted(const char *path)
{
   bool ret=true;

#if defined(LINUX) || defined(DARWIN) || defined(FREEBSD) || (defined(NETBSD) && !defined(ST_RDONLY))
   struct statfs buf;
   statfs(path, &buf);
#elif defined(INTERIX)
   struct statvfs buf;
   wl_statvfs(path, &buf);
#else
   struct statvfs buf;
   statvfs(path, &buf);
#endif

#if defined (DARWIN) || defined(FREEBSD) || defined(NETBSD)
   ret = (strcmp("nfs", buf.f_fstypename) == 0) ? true : false;
#elif defined(LINUX)
   ret = (buf.f_type == 0x6969);
#elif defined(INTERIX)
   ret = (strncasecmp("nfs", buf.f_fstypename, 3) == 0) ? true : false;
#else
   ret = (strncmp("nfs", buf.f_basetype, 3) == 0) ? true : false;
#endif
   return ret;
}
