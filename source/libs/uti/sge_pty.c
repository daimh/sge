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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>

#if defined(DARWIN) || defined(INTERIX) || defined __CYGWIN__
#  include <termios.h>
#  include <sys/ioctl.h>
#  include <grp.h>
#elif defined(HP1164) || defined(HP11)
#  include <termios.h>
#  include <stropts.h>
#elif defined(SOLARIS64) || defined(SOLARIS86) || defined(SOLARISAMD64)
#  include <stropts.h>
#  include <termio.h>
#elif defined(IRIX65)
#  include <sys/ioctl.h>
#  include <stropts.h>
#  include <termio.h>
#elif defined(FREEBSD) || defined(NETBSD)
#  include <sys/ioctl.h>
#  include <termios.h>
#else
#  include <termio.h>
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_unistd.h"
#include "uti/sge_uidgid.h"

extern char *ptsname(int); /* prototype not in any system header */

static struct termios prev_termios;
static int            g_raw_mode = 0;
int                   g_newpgrp = -1;

/****** uti/pty/ptym_open() ****************************************************
*  NAME
*     ptym_open() -- Opens a pty master device
*
*  SYNOPSIS
*     int ptym_open(char *pts_name) 
*
*  FUNCTION
*     Searches for a free pty master device and opens it.
*
*  INPUTS
*     char *pts_name - A buffer that is to receive the name of the
*                      pty master device. Must be at least 12 bytes large.
*
*  RESULT
*     int - The file descriptor of the pty master device.
*           -1 in case of error.
*
*  NOTES
*     MT-NOTE: ptym_open() is not MT safe 
*
*  SEE ALSO
*     pty/ptys_open()
*******************************************************************************/
#if defined(DARWIN) || defined(__OpenBSD__)
int ptym_open(char *pts_name)
{
   char ptr1[] = "pqrstuvwxyzPQRST"; 
   char ptr2[] = "0123456789abcdef"; 
   int  fdm, i, j;

   strcpy(pts_name, "/dev/ptyXY");

   /*
    * iterate over all possible pty names: /dev/ptyXY
    * X = ptr1, Y = ptr2
    */
   for (i=0; ptr1[i] != '\0'; i++) {
      pts_name[8] = ptr1[i];
      for (j=0; ptr2[j] != '\0'; j++) {
         pts_name[9] = ptr2[j];

         /* try to open master */
         if ((fdm = open(pts_name, O_RDWR)) < 0) {
            if (errno == ENOENT) { /* different from EIO */
               return -1;        /* out of pty devices */
            } else {
               continue;         /* try next pty device */
            }
         }

         pts_name[5] = 't';   /* change "pty" to "tty" */
         return fdm;      /* got it, return fd of master */
      }
   }
   return -1;  /* out of pty devices */
}
#else
int ptym_open(char *pts_name)
{
   char *ptr;
   int  fdm;
#if defined(AIX43) || defined(AIX51)
   char default_pts_name[] = "/dev/ptc";
#else
   char default_pts_name[] = "/dev/ptmx";
#endif

   strcpy(pts_name, default_pts_name);   /* in case open fails */
   if ((fdm = open(pts_name, O_RDWR)) < 0) {
      return -1;
   }

   if (grantpt(fdm) < 0) {    /* grant access to slave */
      close(fdm);
      return -2;
   }
   if (unlockpt(fdm) < 0) {   /* clear slave's lock flag */
      close(fdm);
      return -3;
   }
   if ((ptr = ptsname(fdm)) == NULL) {   /* get slave's name */
      close(fdm);
      return -4;
   }

   strcpy(pts_name, ptr);  /* return name of slave */
   return fdm;             /* return fd of master */
}
#endif

/****** uti/pty/ptys_open() ****************************************************
*  NAME
*     ptys_open() -- Opens a pty slave device.
*
*  SYNOPSIS
*     int ptys_open(int fdm, char *pts_name) 
*
*  FUNCTION
*     Opens a pty slave device that matches to a given pty master device.
*
*  INPUTS
*     int fdm        - File descriptor of the pty master device.
*     char *pts_name - The name of the master slave device.
*
*  RESULT
*     int - File descriptor of the pty slave device.
*           -1 in case of error.
*
*  NOTES
*     MT-NOTE: ptys_open() is not MT safe 
*
*  SEE ALSO
*     pty/ptym_open
*******************************************************************************/
#if defined(DARWIN)
int ptys_open(int fdm, char *pts_name)
{
   struct group gr_struct;
   struct group *grptr;
   int          gid, fds;
   char *gr_buffer;
   size_t gr_buffer_size;

   gr_buffer_size = get_group_buffer_size();
   gr_buffer = sge_malloc(gr_buffer_size);

   if (getgrnam_r("tty", &gr_struct, gr_buffer, gr_buffer_size, &grptr) == 0) {
      gid = grptr->gr_gid;
   } else {
      gid = -1;      /* group tty is not in the group file */
   }

   sge_free(&gr_buffer);

   /* following two functions don't work unless we're root */
   chown(pts_name, getuid(), gid);
   chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);

   if ((fds = open(pts_name, O_RDWR)) < 0) {
      close(fdm);
      return -1;
   }
   return fds;
}
#else
int ptys_open(int fdm, char *pts_name)
{
   int      fds;

   /* following should allocate controlling terminal */
   if ((fds = open(pts_name, O_RDWR)) < 0) {
      close(fdm);
      return -5;
   }
#if defined(SOLARIS64) || defined(SOLARIS86) || defined(SOLARISAMD64) || defined(HP11) || defined(HP1164) || defined(IRIX65)
   if (ioctl(fds, I_PUSH, "ptem") < 0) {
      close(fdm);
      close(fds);
      return -6;
   }
   if (ioctl(fds, I_PUSH, "ldterm") < 0) {
      close(fdm);
      close(fds);
      return -7;
   }
#if !defined(HP11) && !defined(HP1164) && !defined(IRIX65)
   if (ioctl(fds, I_PUSH, "ttcompat") < 0) {
      close(fdm);
      close(fds);
      return -8;
   }
#endif
#endif

   return fds;
}
#endif

/****** uti/pty/fork_pty() *****************************************************
*  NAME
*     fork_pty() -- Opens a pty, forks and redirects the std handles
*
*  SYNOPSIS
*     pid_t fork_pty(int *ptrfdm, int *fd_pipe_err, dstring *err_msg) 
*
*  FUNCTION
*     Opens a pty, forks and redirects stdin, stdout and stderr of the child
*     to the pty.
*
*  INPUTS
*     int *ptrfdm      - Receives the file descriptor of the master side of
*                        the pty.
*     int *fd_pipe_err - A int[2] array that receives the file descriptors
*                        of a pipe to separately redirect stderr.
*                        To achieve the same behaviour like rlogin/rsh, this
*                        is normally disabled, compile with
*                        -DUSE_PTY_AND_PIPE_ERR to enable this feature.
*     dstring *err_msg - Receives an error string in case of error.
*     uid_t uid        - uid for pty owner
*
*  RESULT
*     pid_t - -1 in case of error,
*              0 in the child process,
*              or the pid of the child process in the parent process.
*
*  NOTES
*     MT-NOTE: fork_pty() is not MT safe 
*
*  SEE ALSO
*     pty/fork_no_pty
*******************************************************************************/
pid_t fork_pty(int *ptrfdm, int *fd_pipe_err, dstring *err_msg, uid_t uid)
{
   pid_t pid;
   int   fdm, fds;
   char  pts_name[20];
   int   old_euid;
   struct termios tio;

   /* 
    * We run this either as root with euid="sge admin user" or as an unprivileged 
    * user.  If we are root with euid="sge admin user", we must change our
    * euid back to root for this function.
    */
   old_euid = geteuid();
   if (getuid() == SGE_SUPERUSER_UID) {
      sge_seteuid(SGE_SUPERUSER_UID);
   }
   if ((fdm = ptym_open(pts_name)) < 0) {
      sge_dstring_sprintf(err_msg, "can't open master pty \"%s\": %d, %s",
                          pts_name, errno, strerror(errno));
      return -1;
   }
   if (chown(pts_name, uid, -1) != 0) {
      sge_dstring_sprintf(err_msg, "can't chown slave pty \"%s\": %d, %s",
                          pts_name, errno, strerror(errno));
      close(fdm);
      return -1;
   }
#if defined(USE_PTY_AND_PIPE_ERR)
   if (pipe(fd_pipe_err) == -1) {
      sge_dstring_sprintf(err_msg, "can't create pipe for stderr: %d, %s",
                          errno, strerror(errno));
      close(fdm);
      return -1;
   }
#endif
   if ((pid = fork()) < 0) {
      close(fdm);
      return -1;
   } else if (pid == 0) {     /* child */
      if ((g_newpgrp = setsid()) < 0) {
         sge_dstring_sprintf(err_msg, "setsid() error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }

      /* Open pty slave */
      if ((fds = ptys_open(fdm, pts_name)) < 0) {
         sge_seteuid(old_euid);
         sge_dstring_sprintf(err_msg, "can't open slave pty: %d", fds);
         return -1;
      }
      sge_seteuid(old_euid);
      close(fdm);  fdm = -1;   /* all done with master in child */

      /*
       * Set the remote pty to break lines with NL, not CR NL. This ensures
       * line breaks are not modified when e.g.  "cat file.txt" is run in
       * the qrsh session.
       */
      if (tcgetattr(fds, &tio) == 0) {
         tio.c_oflag &= ~ONLCR;
         tcsetattr(fds, TCSANOW, &tio);
      }

#if   defined(TIOCSCTTY) && !defined(CIBAUD)
      /* 44BSD way to acquire controlling terminal */
      /* !CIBAUD to avoid doing this under SunOS */
      if (ioctl(fds, TIOCSCTTY, (char *) 0) < 0) {
         sge_dstring_sprintf(err_msg, "TIOCSCTTY error: %d, %s", 
                             errno, strerror(errno));
         return -1;
      }
#endif
      /* slave becomes stdin/stdout/stderr of child */
      if ((dup2(fds, STDIN_FILENO)) != STDIN_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stdin error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }
      if ((dup2(fds, STDOUT_FILENO)) != STDOUT_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stdout error: %d, %s",
                             errno, strerror(errno));
         return -1;
      } 
#if defined(USE_PTY_AND_PIPE_ERR)
      close(fd_pipe_err[0]); fd_pipe_err[0] = -1;
      if ((dup2(fd_pipe_err[1], STDERR_FILENO)) != STDERR_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stderr error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }
      close(fd_pipe_err[1]); fd_pipe_err[1] = -1;
#else
      if ((dup2(fds, STDERR_FILENO)) != STDERR_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stderr error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }
#endif

      if (fds > STDERR_FILENO) {
         close(fds); fds = -1;
      }
      return 0;      /* child returns 0 just like fork() */
   } else {          /* parent */
      *ptrfdm = fdm; /* return fd of master */
      close(fd_pipe_err[1]); fd_pipe_err[1] = -1;
      sge_seteuid(old_euid);
      return pid;    /* parent returns pid of child */
   }
}

/****** uti/pty/fork_no_pty() **************************************************
*  NAME
*     fork_no_pty() -- Opens pipes, forks and redirects the std handles
*
*  SYNOPSIS
*     pid_t fork_no_pty(int *fd_pipe_in, int *fd_pipe_out, int *fd_pipe_err, 
*     dstring *err_msg) 
*
*  FUNCTION
*     Opens three pipes, forks and redirects stdin, stdout and stderr of the
*     child to the pty.
*
*  INPUTS
*     int *fd_pipe_in  - int[2] array for the two stdin pipe file descriptors
*     int *fd_pipe_out - int[2] array for the two stdout pipe file descriptors
*     int *fd_pipe_err - int[2] array for the two stderr pipe file descriptors
*     dstring *err_msg - Receives an error string in case of error.
*
*  RESULT
*     pid_t - -1 in case of error,
*              0 in the child process,
*              or the pid of the child process in the parent process.
*
*  NOTES
*     MT-NOTE: fork_no_pty() is not MT safe 
*
*  SEE ALSO
*     pty/fork_pty()
*******************************************************************************/
pid_t fork_no_pty(int *fd_pipe_in, int *fd_pipe_out, 
                  int *fd_pipe_err, dstring *err_msg)
{
   int   ret;
   pid_t pid;

   DENTER(TOP_LAYER, "fork_no_pty");
   
   ret = pipe(fd_pipe_in);
   if (ret == -1) {
      sge_dstring_sprintf(err_msg, "can't create pipe for stdin: %d: %s",
         errno, strerror(errno));
      return -1;
   }

   ret = pipe(fd_pipe_out);
   if (ret == -1) {
      sge_dstring_sprintf(err_msg, "can't create pipe for stdout: %d: %s",
         errno, strerror(errno));
      return -1;
   }

   ret = pipe(fd_pipe_err);
   if (ret == -1) {
      sge_dstring_sprintf(err_msg, "can't create pipe for stderr: %d: %s",
         errno, strerror(errno));
      return -1;
   }

   if ((pid = fork()) < 0) {
      return -1;
   } else if (pid == 0) {     /* child */
      if (setsid() < 0) {
         sge_dstring_sprintf(err_msg, "setsid() error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }

      /* attach pipes to stdin/stdout/stderr of child */
      close(fd_pipe_in[1]);  fd_pipe_in[1] = -1;
      if ((dup2(fd_pipe_in[0], STDIN_FILENO)) != STDIN_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stdin error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }
      close(fd_pipe_in[0]); fd_pipe_in[0] = -1;

      close(fd_pipe_out[0]); fd_pipe_out[0] = -1;
      if ((dup2(fd_pipe_out[1], STDOUT_FILENO)) != STDOUT_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stdout error: %d, %s",
                             errno, strerror(errno));
         return -1;
      } 
      close(fd_pipe_out[1]); fd_pipe_out[1] = -1;

      close(fd_pipe_err[0]); fd_pipe_out[0] = -1;
      if ((dup2(fd_pipe_err[1], STDERR_FILENO)) != STDERR_FILENO) {
         sge_dstring_sprintf(err_msg, "dup2 to stderr error: %d, %s",
                             errno, strerror(errno));
         return -1;
      }
      close(fd_pipe_err[1]); fd_pipe_err[1] = -1;
   } else {  /* parent */
      close(fd_pipe_in[0]);  fd_pipe_in[0]  = -1;
      close(fd_pipe_out[1]); fd_pipe_out[1] = -1;
      close(fd_pipe_err[1]); fd_pipe_err[1] = -1;
   }
   DEXIT;
   return pid;
}

/****** uti/pty/terminal_enter_raw_mode() **************************************
*  NAME
*     terminal_enter_raw_mode() -- Sets terminal to raw mode 
*
*  SYNOPSIS
*     int terminal_enter_raw_mode(void) 
*
*  FUNCTION
*     Sets terminal to raw mode, i.e. no control characters are interpreted any
*     more, but are simply printed.
*
*  RESULT
*     int - 0 if Ok, else errno
*
*  NOTES
*     MT-NOTE: terminal_enter_raw_mode() is not MT safe 
*
*  SEE ALSO
*     pty/terminal_leave_raw_mode
*******************************************************************************/
int terminal_enter_raw_mode(void)
{
   struct termios tio;
   int            ret = 0;

   if (tcgetattr(STDOUT_FILENO, &tio) == -1) {
      ret = errno;
   } else {
      memcpy(&prev_termios, &tio, sizeof(struct termios));
      tio.c_iflag |= IGNPAR;
      tio.c_iflag &=  ~(BRKINT | ISTRIP | INLCR | IGNCR | ICRNL | IXANY | IXOFF);
   #ifdef IUCLC
      tio.c_iflag &= ~IUCLC;
   #endif
      tio.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL);
   #ifdef IEXTEN
      tio.c_lflag &= ~IEXTEN;
   #endif
      tio.c_oflag &= (OPOST | ONLCR);
      tio.c_cc[VMIN] = 1;
      tio.c_cc[VTIME] = 0;

      if (tcsetattr(STDOUT_FILENO, TCSADRAIN, &tio) == -1) {
         ret = errno;
      } else {
         g_raw_mode = 1;
      }
   }
   return ret;
}

/****** uti/pty/terminal_leave_raw_mode() **************************************
*  NAME
*     terminal_leave_raw_mode() -- restore previous terminal mode
*
*  SYNOPSIS
*     int terminal_leave_raw_mode(void) 
*
*  FUNCTION
*     Restores the previous terminal mode.
*
*  RESULT
*     int - 0 if Ok, else errno
*
*  NOTES
*     MT-NOTE: terminal_leave_raw_mode() is not MT safe 
*
*  SEE ALSO
*     pty/terminal_enter_raw_mode()
*******************************************************************************/
int terminal_leave_raw_mode(void)
{
   int ret = 0;

   if (g_raw_mode == 1) {
      if (tcsetattr(STDOUT_FILENO, TCSADRAIN, &prev_termios) == -1) {
         ret = errno;
      } else {
         g_raw_mode = 0;
      }
   }
   return ret;
}

