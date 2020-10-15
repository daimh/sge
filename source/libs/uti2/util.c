/* util.c -- extra utility functions

   Copyright (C) 2012, 2013, 2014 Dave Love, University of Liverpool

   This file is free software: you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of
   the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this file.  If not, a copy can be downloaded
   from http://www.gnu.org/licenses/lgpl.html.
*/

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <msg_common.h>
#include "sge_config.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "util.h"

/* Replace all occurrences of C1 in first N characters of STR with C2.
   Return STR.  */
char *
replace_char(char *str, size_t n, char c1, char c2)
{
   int i;

   for (i=0; i<n; i++)
      if (str[i] == c1) str[i] = c2;
   return str;
}

/* Read contents of FILE into BUFFER of size LBUFFER.
   Update LBUFFER with the number of bytes read and return BUFFER.
   Doesn't stat the file to find it's length, unlike sge_file2string,
   so works for /proc files.  */
char *
dev_file2string(const char* file, char *buffer, size_t *lbuffer)
{
   int fd;
   size_t i;

   DENTER(TOP_LAYER, "dev_file2string");
   errno = 0;
   buffer[0] = '\0';
   if ((fd = open(file, O_RDONLY)) == -1) {
      *lbuffer = 0;
      ERROR((SGE_EVENT, MSG_FILE_OPENFAILED_SS, file, strerror(errno)));
      DRETURN(NULL);
   }
   if ((i = read(fd, buffer, *lbuffer)) == 0) {
      ERROR((SGE_EVENT, MSG_FILE_FREADFAILED_SS, file, strerror(errno)));
      close(fd);
      *lbuffer = 0;
      buffer[0] = '\0';
      DRETURN(buffer);
   }
   close(fd);
   i = MIN(i, (*lbuffer)-1);
   *lbuffer = i;
   buffer[i] = '\0';
   DRETURN(buffer);
}

/* Check each line of FILE for a KEY (using BUFFER of size LBUFFER as
   temporary storage). If found, return a pointer to the rest
   of the line. */
char *
file_getvalue(char *buffer, int lbuffer, const char* file, const char *key) {
   int   lkey  = strlen(key);
   char *value = NULL;
   FILE *fp;
   if ((fp = fopen(file, "r"))) {
      while(fgets(buffer, lbuffer, fp)) {
         if (strncmp(buffer, key, lkey) == 0) {
            value = buffer + lkey;
            break;
         }
      }
      fclose(fp);
   }

   return value;
}

/* Return in ISADMIN whether running as SGE admin user or root unless
   an error occurred, indicated by ERROR true.  Intended for
   determining whether to switch2start_user or switch2admin_user in
   library routines that need privileges..  */
void
sge_running_as_admin_user(bool *error, bool *isadmin)
{
   uid_t uid;
   gid_t gid;

   if (sge_user2uid (get_admin_user_name(), &uid, &gid, 1) != 0) {
      *error = true;
      return;
   }
   *error = false;
   *isadmin = (geteuid() == uid) || geteuid() == SGE_SUPERUSER_UID;
}

/****** uti/file_exists() ***************************************
*
*  NAME
*     file_exists -- whether file exists
*
*  SYNOPSIS
*     bool file_exists(const char *file)
*
*  FUNCTION
*     Test whether a file exists (actually can be stat'ed).
*
*  INPUTS
*     file  - file name
*
*  RESULT
*     true iff file can be stat'ed
****************************************************************************
*/
bool
file_exists(const char *file)
{
   SGE_STRUCT_STAT statbuf;
   return file && *file && (SGE_STAT(file, &statbuf) == 0);
}

/****** uti/file_exists() ***************************************
*
*  NAME
*     copy_linewise -- copy a file line-by-line
*
*  SYNOPSIS
*     bool copy_linewise(const char *src, const char *dst)
*
*  FUNCTION
*     Copy line-by-line (unbuffered writes) from file SRC to file DST,
*     assuming line length < MAX_STRING_SIZE.
*
*  INPUTS
*     src  - source file name
*     dst  - destination file name
*
*  RESULT
*     true on success, false if an i/o error occurs
****************************************************************************
*/
bool
copy_linewise(const char *src, const char *dst)
{
   FILE *srcfp, *dstfp;
  char buffer[MAX_STRING_SIZE];
  bool ok = true;

  if (!(srcfp = fopen(src, "r")) ||
      !(dstfp = fopen(dst, "w")))
     return false;
  while (fgets(buffer, sizeof buffer, srcfp)) {
     size_t l = strlen(buffer);
     if (fwrite(buffer, 1, l, dstfp) != l) {
        fclose(dstfp);
        fclose(srcfp);
        return false;
     }
     fflush(dstfp);
  }
  if (ferror(srcfp) || ferror(dstfp)) ok = false;
  fclose(dstfp);
  fclose(srcfp);
  return ok;
}

#if !HAVE_STRSIGNAL
#include <signal.h>

typedef struct pair {
   int signal;
   const char *description;
} pair;

/* Taken from header file (not copyrightable).
   Non-conditionalized ones are in POSIX.  */
static const pair sigtable[] = {
   {SIGHUP, "Hangup"},
   {SIGINT, "Interrupt"},
   {SIGQUIT, "Quit"},
   {SIGILL, "Illegal instruction"},
   {SIGTRAP, "Trace/breakpoint trap"},
   {SIGABRT, "Aborted"},
#ifdef SIGIOT                   /* not in Cygwin */
   {SIGIOT, "IOT trap"},
#endif
   {SIGBUS, "Bus error"},
   {SIGFPE, "Floating point exception"},
   {SIGKILL, "Killed"},
   {SIGUSR1, "User defined signal 1"},
   {SIGSEGV, "Segmentation violation"},
   {SIGUSR2, "User defined signal 2"},
   {SIGPIPE, "Broken pipe"},
   {SIGALRM, "Alarm clock"},
   {SIGTERM, "Terminated"},
#ifdef SIGSTKFLT
   {SIGSTKFLT, "Stack fault"},
#endif
   {SIGCHLD, "Child exited"},
   {SIGCONT, "Continued"},
   {SIGSTOP, "Stopped"},
   {SIGTSTP, "Keyboard stop"},
   {SIGTTIN, "Background process attempting read"},
   {SIGTTOU, "Background process attempting write"},
   {SIGURG, "Urgent condition on socket"},
   {SIGXCPU, "CPU time limit exceeded"},
   {SIGXFSZ, "File size limit exceeded"},
#ifdef SIGVTALRM
   {SIGVTALRM, "Virtual timer expired"},
#endif
   {SIGPROF, "Profiling timer expired"},
#ifdef SIGWINCH
   {SIGWINCH, "Window changed"},
#endif
#ifdef SIGPOLL
   {SIGPOLL, "Pollable event occurred"},
#endif
#ifdef SIGIO
   {SIGIO, "I/O possible"},
#endif
#ifdef SIGPWR
   {SIGPWR, "Power failure"},
#endif
   {SIGSYS, "Bad system call"},
#ifdef SIGEMT
   {SIGEMT, "EMT trap"},
#endif
#ifdef SIGINFO
   {SIGINFO, "Information request"},
#endif
#ifdef SIGLOST
   {SIGLOST, "Resource lost"},
#endif
};

static const int nsig = sizeof(sigtable)/sizeof(pair);

/* see also sge_sys_sig2str */
char *strsignal(int signo) {
   int n = nsig;
   while (n--) {
      if (sigtable[n].signal == signo)
	 return (char *) sigtable[n].description;
   }
   return "Unknown signal";
}
#endif  /* !HAVE_STRSIGNAL */

#if !HAVE_GETGROUPLIST
/* This is for systems missing getgrouplist.  The obvious sources of a
   suitably-licensed version are mixed up with other routines.

   Not thread-safe.  */

#include <grp.h>
#include <string.h>
#include <limits.h>

int
getgrouplist (char const *user, gid_t gid, gid_t *groups, int *ngroups)
{
  int index = 0;                /* next index in groups */

  if (*ngroups < 0) return -1;

  if (*ngroups > 0)
    groups[index] = gid;
  index++;

  setgrent ();
  while (1) {
    char **cp;
    struct group *grp;

    grp = getgrent ();
    if (!grp) break;

    for (cp = grp->gr_mem; *cp; ++cp) {
      int n;

      if (strcmp (user, *cp) != 0) continue;
      /* check if already in list */
      for (n = 0; n < index; ++n)
        if (groups && groups[n] == grp->gr_gid)
          break;
      if (n == index) {        /* not in list */
        if (index < *ngroups)
          groups[index] = grp->gr_gid;
        if (index == INT_MAX) goto finish;
        index++;
      }
    }
  }

 finish:
  endgrent ();
  if (index > *ngroups || index == INT_MAX) {
    *ngroups = index;
    return -1;
  }
  *ngroups = index;
  return index;
}
#endif  /* !HAVE_GETGROUPLIST */
