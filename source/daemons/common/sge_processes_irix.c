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
int dummy_processes_irix_c; /* Just for compilers complaining about empty files */

#if defined(IRIX)

/* Feature test switches. */
#define _KMEMUSER 1

/* ANSI C headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* POSIX headers. */
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/timers.h>
#include <sys/utsname.h>
/* to get MAXPID */
#include <sys/param.h>

/* SGI headers. */
#include <sys/acct.h>
#include <sys/cred.h>
#include <sys/ioctl.h>
#define _KERNEL 1
#include <limits.h>
#include <dirent.h>
#include <sys/procfs.h>
#undef _KERNEL
#include <sys/proc.h>
#include <sys/sysinfo.h>
#include <sys/sysmp.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/dnlc.h>
#include <sys/ksa.h>
#include <sys/mbuf.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>
#include <sys/statfs.h>
#include <sys/arsess.h>
#include <sys/syssgi.h>
#include <sys/stat.h>
#include <sys/swap.h>
#include <mntent.h>
#define _KERNEL 1
#include <net/if.h>
#undef _KERNEL
#include "sgedefs.h"
#include "exec_ifm.h"
#include "basis_types.h"

#include "msg_common.h"

extern void shepherd_trace(char *str);

struct prpsinfo pi;
struct prstatus ps;
struct prusage pu;
struct pracinfo pa;

/**********************************************************
 We want to get a list of all processes belonging to a job.
 We use the ash to find all processes.

 If this function is called from a nonpriviledged user. Only his
 processes are scanned.

 Be carefull. If npids is low only a subset of all processes 
 is returned.
 **********************************************************/
int getpidsOfJob(ash_t ash, pid_t pids[], int *npids)
{
   static int f = -1;
   char proc[PATH_MAX];
   struct dirent *de;
   DIR *dir;
   int fillindex = 0;
   char trace_str[128];

   if (!ash || *npids <= 0)
      return -1;

   if (!(dir = opendir("/proc")))
      return -1;

   while ((de = readdir(dir))) {

      if (!strncmp(".", de->d_name, 1))
         continue;

      sprintf(proc, "%s/%s", "/proc", de->d_name);
      if ((f = open(proc, O_RDONLY, 0)) < 0) {
         if (errno!=ENOENT) {
            sprintf(trace_str, MSG_FILE_OPENFAILED_SS , 
               proc, strerror(errno));
            shepherd_trace(trace_str);
         }
         continue;
      }

#if 0
      if (ioctl(f, PIOCPSINFO, &pi) < 0) {
         close(f);
         continue;
      }
      if (ioctl(f, PIOCUSAGE, &pu) < 0) {
         close(f);
         continue; 
      }
      if (ioctl(f, PIOCGETPR, &p) < 0) {
         close(f);
         continue;
      }
#endif

      if (ioctl(f, PIOCACINFO, &pa) < 0) {
         close(f);
         continue;
      }

      if (pa.pr_ash == ash) {
         if (ioctl(f, PIOCSTATUS, &ps) < 0) {
            sprintf(trace_str,MSG_SYSTEM_GETPIDSFAILED_S , proc);
            shepherd_trace(trace_str);
            close(f);
            continue;
         }

         pids[fillindex++] = ps.pr_pid;
         if (fillindex >= *npids) {
            *npids = fillindex;
            closedir(dir);
            close(f);
            return 0;
         }
      }
      close(f);
   }

   closedir(dir);
   *npids = fillindex;
   return 0;
}


/**********************************************
 Send a signal to all processes with an ash.

 Only root can send a signal to any process

 Be careful with until_vanished, because if signal is not
 -9 there might be an infinite loop
 **********************************************/

#define INCPTR(type, ptr, nbyte) ptr = (type *)((char *)ptr + nbyte)
#define INCJOBPTR(ptr, nbyte) INCPTR(struct psJob_s, ptr, nbyte)
#define INCPROCPTR(ptr, nbyte) INCPTR(struct psProc_s, ptr, nbyte)

int kill_ash(ash_t ash, int sig, int until_vanished)
{
   struct psJob_s *jp;
   struct psProc_s *pp;
   char err_str[4096];
   char *cmd;

   if ((cmd=getenv("SGE_IRIX_KILL_COMMAND"))) {

       char buf[2048];
       sprintf(buf, cmd, sig, ash);
       system(buf);
       sprintf(err_str, "kill_ash: %s", buf);
       shepherd_trace(err_str);

   } else {

      /* use PDC to get process info */

      psStartCollector();
      psWatchJob(ash);

      do {
         if ((jp=psGetOneJob(ash))) {
            if (!jp->jd_proccount)
               until_vanished = 0;
            else {
               int j;
               pp = (struct psProc_s *)((char *)jp + jp->jd_length);
               for(j=0; j<jp->jd_proccount; j++) {
                  if (kill(pp->pd_pid, sig)<0)
                     sprintf(err_str, MSG_PROC_KILL_IIS, (int) pp->pd_pid, sig, strerror(errno));
                  else
                     sprintf(err_str, MSG_PROC_KILLISSUED_II , (int) pp->pd_pid, sig);
                  shepherd_trace(err_str);
                  INCPROCPTR(pp, pp->pd_length);
               }
            }
         }
         sleep(1);
      } while (until_vanished);

      sge_free(&jp);
      psIgnoreJob(ash);
   }

   return 0;
}

#ifdef TEST
int main(int argc, char **argv)
{
   pid_t pids[1000];
   int npids, i, sig = 0;

   if (argc<=1)
      return -1;

   argv++;
   if (!strcmp("-k", *argv)) {
      argv++;
      if (!*argv)
         exit(1);
      sig = atoi(*argv++);
   }
   while (*argv) {
      npids = 1000;
      i = getpidsOfJob((ash_t)atoi(*argv), pids, &npids);
      printf("getpidsOfJob(%d) returns: %d, found %d pids\n",
             atoi(*argv), i, npids);
      if (!i && npids) {
         printf("Pids:\n");
         for (i=0; i<npids; i++) {
            printf("%d, ", pids[i]);
         }
         printf("\n");
      }
      if (sig) {
         printf("sending signal %d to all processes with ash %d\n",
                sig, atoi(*argv));
         i = kill_ash(atoi(*argv), sig, sig==9);
      }
      argv++;
   }

}

#endif /* TEST */
#endif /* IRIX */
