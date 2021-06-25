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
/*
 *
 * pdc.c - Portable Data Collector Library and Test Module
 * 
 */

#if !defined(COMPILE_DC)

int verydummypdc;

#   if defined(MODULE_TEST) || defined(PDC_STANDALONE)
#include <stdio.h>
#include "basis_types.h"
#include "uti/sge_language.h"
#include "uti/sge_os.h"
#include "uti/sge_log.h"

int main(int argc,char *argv[])
{
#ifdef __SGE_COMPILE_WITH_GETTEXT__  
   /* init language output for gettext() , it will use the right language */
   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);  
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */
   printf("sorry - no pdc for this architecture yet\n");
   return 0;
}
#endif
#else

#define _KMEMUSER 1

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>



#if defined(AIX)
#  if defined(_ALL_SOURCE)
#     undef _ALL_SOURCE
#  endif
#include <procinfo.h>
#include <sys/types.h>
#endif

#if defined(FREEBSD)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#endif

#if defined(DARWIN)
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#endif


#if defined(HP1164)
#include <sys/param.h>
#include <sys/pstat.h>
#endif

#if defined(LINUX) || defined(SOLARIS) || defined(DARWIN) || defined (FREEBSD) || defined(NETBSD) || defined(HP1164) || defined(AIX)

#include "uti/sge_os.h"
#endif

#if   defined(LINUX) || defined(SOLARIS)
#  define F64 "%ld"
#  define S64 "%li"
#else
#  define F64 "%d"
#  define S64 "%i"
#endif

#  if DEBUG
      static FILE *df = NULL;
#  endif

#ifdef SOLARIS
int getpagesize(void);
#endif

#include <errno.h>

#include "uti/sge_language.h"
#include "uti/sge_rmon.h"
#include "uti/sge_uidgid.h"

#include "cull/cull.h"

#include "sgeobj/sge_feature.h"

#include "msg_sge.h"
#include "sgedefs.h"
#include "exec_ifm.h"
#include "pdc.h"
#include "ptf.h"
#include "procfs.h"
#include "basis_types.h"

#if defined(PDC_STANDALONE)
#  include "sge_log.h"
#  include "sge_language.h"
#  include "sge_proc.h"
#endif

typedef struct {
   int job_collection_interval;  /* max job data collection interval */
   int prc_collection_interval;  /* max process data collection interval */
   int sys_collection_interval;  /* max system data collection interval */
} ps_config_t;

/* default collection intervals */
static ps_config_t ps_config = { 0, 0, 5 };

time_t last_time = 0;
lnk_link_t job_list;
long pagesize;           /* size of a page of memory (probably 8k) */
int physical_memory;     /* size of real mem in KB                 */
char unixname[128];      /* the name of the booted kernel          */

#define INCPTR(type, ptr, nbyte) ptr = (type *)((char *)ptr + nbyte)
#define INCJOBPTR(ptr, nbyte) INCPTR(struct psJob_s, ptr, nbyte)
#define INCPROCPTR(ptr, nbyte) INCPTR(struct psProc_s, ptr, nbyte)

#if defined(LINUX) || defined(SOLARIS) || defined(ALPHA) || defined(FREEBSD) || defined(DARWIN)

void pdc_kill_addgrpid(gid_t add_grp_id, int sig,
   tShepherd_trace shepherd_trace)
{
#if defined(LINUX) || defined(SOLARIS) || defined(ALPHA)
   procfs_kill_addgrpid(add_grp_id, sig, shepherd_trace);      
#elif defined(FREEBSD)
   kvm_t *kd;
   int i, nprocs;
   struct kinfo_proc *procs;
   char kerrbuf[_POSIX2_LINE_MAX];

   kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, kerrbuf);
   if (kd == NULL) {
#if DEBUG
      fprintf(stderr, "kvm_openfiles: error %s\n", kerrbuf);
#endif
      return;
   }

   procs = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nprocs);
   if (procs == NULL) {
#if DEBUG
      fprintf(stderr, "kvm_getprocs: error %s\n", kvm_geterr(kd));
#endif
      kvm_close(kd);
      return;
   }
   for (; nprocs >= 0; nprocs--, procs++) {
      for (i = 0; i < procs->ki_ngroups; i++) {
         if (procs->ki_groups[i] == add_grp_id) {
            char err_str[256];

            if (procs->ki_uid != 0 && procs->ki_ruid != 0 &&
                procs->ki_svuid != 0 &&
                procs->ki_rgid != 0 && procs->ki_svgid != 0) {
               kill(procs->ki_pid, sig);
               snprintf(err_str, sizeof(err_str), MSG_SGE_KILLINGPIDXY_UI,
                        sge_u32c(procs->ki_pid), add_grp_id);
	    } else {
               snprintf(err_str, sizeof(err_str), MSG_SGE_DONOTKILLROOTPROCESSXY_UI,
                        sge_u32c(procs->ki_pid), add_grp_id);
	    }
	    if (shepherd_trace)
	       shepherd_trace(err_str);
	 }
      }
   }
   kvm_close(kd);
#elif defined(DARWIN)
   int i, nprocs;
   struct kinfo_proc *procs;
   struct kinfo_proc *procs_begin;
   int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
   size_t bufSize = 0;

   if (sysctl(mib, 4, NULL, &bufSize, NULL, 0) < 0) {
      return;
   }
   if ((procs = (struct kinfo_proc *)malloc(bufSize)) == NULL) {
      return;
   }
   if (sysctl(mib, 4, procs, &bufSize, NULL, 0) < 0) {
      sge_free(&procs);
      return;
   }
   procs_begin = procs;
   nprocs = bufSize/sizeof(struct kinfo_proc);

   for (; nprocs >= 0; nprocs--, procs++) {
      for (i = 0; i < procs->kp_eproc.e_ucred.cr_ngroups; i++) {
         if (procs->kp_eproc.e_ucred.cr_groups[i] == add_grp_id) {
            char err_str[256];

            if (procs->kp_eproc.e_ucred.cr_uid != 0 && procs->kp_eproc.e_pcred.p_ruid != 0 &&
                procs->kp_eproc.e_pcred.p_svuid != 0 &&
                procs->kp_eproc.e_pcred.p_rgid != 0 && procs->kp_eproc.e_pcred.p_svgid != 0) {
               kill(procs->kp_proc.p_pid, sig);
               snprintf(err_str, sizeof(err_str), MSG_SGE_KILLINGPIDXY_UI,
                        sge_u32c(procs->kp_proc.p_pid), add_grp_id);
            } else {
              snprintf(err_str, sizeof(err_str), MSG_SGE_DONOTKILLROOTPROCESSXY_UI,
                       sge_u32c(procs->kp_proc.p_pid), add_grp_id);
            }
            if (shepherd_trace)
               shepherd_trace(err_str);
         }
      }
   }
   sge_free(&procs_begin);
#endif
}
#endif

lnk_link_t * find_job(JobID_t jid) {
   lnk_link_t *curr;

   for (curr=job_list.next; curr != &job_list; curr=curr->next) {
      if (jid == LNK_DATA(curr, job_elem_t, link)->job.jd_jid)
         return curr;
   }
   return NULL;
}


static int
get_gmt(void)
{
   struct timeval now;

   gettimeofday(&now, NULL);

   return now.tv_sec;
}

#ifdef PDC_STANDALONE
static psSys_t sysdata;

#endif

#ifdef PDC_STANDALONE
static void
psSetCollectionIntervals(int jobi, int prci, int sysi)
{
   if (jobi != -1)
      ps_config.job_collection_interval = jobi;

   if (prci != -1)
      ps_config.prc_collection_interval = prci;

   if (sysi != -1)
      ps_config.sys_collection_interval = sysi;
}
static
int psRetrieveSystemData(void)
{
   time_t time_stamp = get_gmt();
   time_t prev_time_stamp;
   static time_t next;

   if (time_stamp <= next) {
      return 0;
   }
   next = time_stamp + ps_config.sys_collection_interval;

   prev_time_stamp = sysdata.sys_tstamp;

   /* Time of last snap */
   sysdata.sys_tstamp = time_stamp;

   return 0;
}

#endif

static int
get_numjobs(void)
{
   lnk_link_t *curr;
   int count = 0;
   for (curr=job_list.next; curr != &job_list; curr=curr->next)
      if (LNK_DATA(curr, job_elem_t, link)->precreated == 0)
         count++;
   return count;
}


static void
free_process_list(job_elem_t *job_elem)
{
   lnk_link_t *currp;

   /* free process list */
   while((currp=job_elem->procs.next) != &job_elem->procs) {
      proc_elem_t *ptr;

      LNK_DELETE(currp);
      ptr = LNK_DATA(currp, proc_elem_t, link);
      sge_free(&ptr);
   }
}

static void
free_job(job_elem_t *job_elem)
{

   free_process_list(job_elem);


   /* free job element */
   sge_free(&job_elem);
}

static int psRetrieveOSJobData(void) {
   lnk_link_t *curr, *next;
   time_t time_stamp = get_gmt();
   static time_t next_time, pnext_time;


   DENTER(TOP_LAYER, "psRetrieveOSJobData");

   if (time_stamp <= next_time) {
      DRETURN(0);
   }
   next_time = time_stamp + ps_config.job_collection_interval;

#if   defined(LINUX) || defined(ALPHA) || defined(SOLARIS)
   pt_dispatch_procs_to_jobs(&job_list, time_stamp, last_time);
#elif defined(AIX)
   {
      #define SIZE 16

      struct procsinfo pinfo[SIZE];

      int idx = 0, count;
      job_elem_t *job_elem;
      double old_time = 0.0;
      uint64 old_vmem = 0;
      pid_t index;

      while ((count = getprocs(pinfo, sizeof(struct procsinfo), NULL, 0, &index, SIZE)) > 0) {

        int i;
        /* for all processes */
        for (i=0; i < count; i++)
        {
          for (curr=job_list.next; curr != &job_list; curr=curr->next) {
            int group;

            job_elem = LNK_DATA(curr, job_elem_t, link);

            if (job_elem->job.jd_jid == pinfo[i].pi_pgrp) {

              lnk_link_t  *curr2;
              proc_elem_t *proc_elem;
              int newprocess = 1;

              for (curr2=job_elem->procs.next; curr2 != &job_elem->procs; curr2=curr2->next) {

                proc_elem = LNK_DATA(curr2, proc_elem_t, link);

                if (proc_elem->proc.pd_pid == pinfo[i].pi_pid) {
                  newprocess = 0;
                  break;
                }
              }

              if (newprocess) {
                proc_elem = (proc_elem_t *) malloc(sizeof(proc_elem_t));

                if (proc_elem == NULL) {
                    DRETURN(0);
                }

                memset(proc_elem, 0, sizeof(proc_elem_t));
                proc_elem->proc.pd_length = sizeof(psProc_t);
                proc_elem->proc.pd_state  = 1; /* active */

                LNK_ADD(job_elem->procs.prev, &proc_elem->link);
                job_elem->job.jd_proccount++;

              }
              else {
                  /* save previous usage data - needed to build delta usage */
                  old_time = proc_elem->proc.pd_utime + proc_elem->proc.pd_stime;
                  old_vmem  = proc_elem->vmem;
              }

              proc_elem->proc.pd_tstamp = time_stamp;
              proc_elem->proc.pd_pid    = pinfo[i].pi_pid;

              proc_elem->proc.pd_utime  = pinfo[i].pi_ru.ru_utime.tv_sec;
              proc_elem->proc.pd_stime  = pinfo[i].pi_ru.ru_stime.tv_sec;

              proc_elem->proc.pd_uid    = pinfo[i].pi_uid;
              proc_elem->vmem           = pinfo[i].pi_dvm +
                                          pinfo[i].pi_tsize + pinfo[i].pi_dsize;
              proc_elem->rss            = pinfo[i].pi_drss + pinfo[i].pi_trss;
              proc_elem->proc.pd_pstart = pinfo[i].pi_start;

              proc_elem->mem = ((proc_elem->proc.pd_stime + proc_elem->proc.pd_utime) - old_time) *
                                (( old_vmem + proc_elem->vmem)/2);

            } /* if */
          } /* for job_list */
        } /* process */
      }
   }
#elif defined(HP1164)
   {
      #define SIZE 16
      struct pst_status pstat_buffer[SIZE];
      int idx = 0, count;
      job_elem_t *job_elem;
      double old_time = 0;
      uint64 old_vmem = 0;

      while ((count = pstat_getproc(pstat_buffer, sizeof(struct pst_status), SIZE, idx)) > 0) {

        int i;
        /* for all processes */
        for (i=0; i < count; i++)
        {
          for (curr=job_list.next; curr != &job_list; curr=curr->next) {
            int group;

            job_elem = LNK_DATA(curr, job_elem_t, link);

            if (job_elem->job.jd_jid == pstat_buffer[i].pst_pgrp) {

              lnk_link_t  *curr2;
              proc_elem_t *proc_elem;
              int newprocess = 1;

              for (curr2=job_elem->procs.next; curr2 != &job_elem->procs; curr2=curr2->next) {

                proc_elem = LNK_DATA(curr2, proc_elem_t, link);

                if (proc_elem->proc.pd_pid == pstat_buffer[i].pst_pid) {
                  newprocess = 0;
                  break;
                }
              }

              if (newprocess) {
                proc_elem = (proc_elem_t *) malloc(sizeof(proc_elem_t));

                if (proc_elem == NULL) {
                    DRETURN(0);
                }

                memset(proc_elem, 0, sizeof(proc_elem_t));
                proc_elem->proc.pd_length = sizeof(psProc_t);
                proc_elem->proc.pd_state  = 1; /* active */

                LNK_ADD(job_elem->procs.prev, &proc_elem->link);
                job_elem->job.jd_proccount++;

              }
              else {
                  /* save previous usage data - needed to build delta usage */
                  old_time = proc_elem->proc.pd_utime + proc_elem->proc.pd_stime;
                  old_vmem  = proc_elem->vmem;
              }

              proc_elem->proc.pd_tstamp = time_stamp;
              proc_elem->proc.pd_pid    = pstat_buffer[i].pst_pid;

              proc_elem->proc.pd_utime  = pstat_buffer[i].pst_utime;
              proc_elem->proc.pd_stime  = pstat_buffer[i].pst_stime;

              proc_elem->proc.pd_uid    = pstat_buffer[i].pst_uid;
              proc_elem->proc.pd_gid    = pstat_buffer[i].pst_gid;
              proc_elem->vmem           = pstat_buffer[i].pst_vdsize +
                                          pstat_buffer[i].pst_vtsize + pstat_buffer[i].pst_vssize;
              proc_elem->rss            = pstat_buffer[i].pst_rssize;
              proc_elem->proc.pd_pstart = pstat_buffer[i].pst_start;

              proc_elem->vmem = proc_elem->vmem * getpagesize();
              proc_elem->rss  = proc_elem->rss  * getpagesize();

              proc_elem->mem = ((proc_elem->proc.pd_stime + proc_elem->proc.pd_utime) - old_time) *
                                (( old_vmem + proc_elem->vmem)/2);

            } /* if */
          } /* for job_list */
        } /* process */

        idx = pstat_buffer[count-1].pst_idx + 1;
      }
   }
#elif defined(FREEBSD)
   {
      kvm_t *kd;
      int i, nprocs;
      struct kinfo_proc *procs;
      char kerrbuf[_POSIX2_LINE_MAX];
      job_elem_t *job_elem;
      double old_time = 0.0;
      uint64 old_vmem = 0;

      kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, kerrbuf);
      if (kd == NULL) {
         DPRINTF(("kvm_openfiles: error %s\n", kerrbuf));
         DRETURN(-1);
      }

      procs = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nprocs);
      if (procs == NULL) {
         DPRINTF(("kvm_getprocs: error %s\n", kvm_geterr(kd)));
         kvm_close(kd);
         DRETURN(-1);
      }
      for (; nprocs >= 0; nprocs--, procs++) {
         for (curr=job_list.next; curr != &job_list; curr=curr->next) {
            job_elem = LNK_DATA(curr, job_elem_t, link);

            for (i = 0; i < procs->ki_ngroups; i++) {
               if (job_elem->job.jd_jid == procs->ki_groups[i]) {
                  lnk_link_t  *curr2;
                  proc_elem_t *proc_elem;
                  int newprocess = 1;
                  
                  if (job_elem->job.jd_proccount != 0) {
                     for (curr2=job_elem->procs.next; curr2 != &job_elem->procs; curr2=curr2->next) {
                        proc_elem = LNK_DATA(curr2, proc_elem_t, link);

                        if (proc_elem->proc.pd_pid == procs->ki_pid) {
                           newprocess = 0;
                           break;
                        }
                     }
                  }
                  if (newprocess) {
                     proc_elem = malloc(sizeof(proc_elem_t));
                     if (proc_elem == NULL) {
                        kvm_close(kd);
                        DRETURN(0);
                     }

                     memset(proc_elem, 0, sizeof(proc_elem_t));
                     proc_elem->proc.pd_length = sizeof(psProc_t);
                     proc_elem->proc.pd_state  = 1; /* active */
                     proc_elem->proc.pd_pstart = procs->ki_start.tv_sec;

                     LNK_ADD(job_elem->procs.prev, &proc_elem->link);
                     job_elem->job.jd_proccount++;
                  } else {
                     /* save previous usage data - needed to build delta usage */
                     old_time = proc_elem->proc.pd_utime + proc_elem->proc.pd_stime;
                     old_vmem  = proc_elem->vmem;
                  }
                  proc_elem->proc.pd_tstamp = time_stamp;
                  proc_elem->proc.pd_pid    = procs->ki_pid;

                  proc_elem->proc.pd_utime  = procs->ki_rusage.ru_utime.tv_sec;
                  proc_elem->proc.pd_stime  = procs->ki_rusage.ru_stime.tv_sec;

                  proc_elem->proc.pd_uid    = procs->ki_uid;
                  proc_elem->proc.pd_gid    = procs->ki_rgid;
                  proc_elem->vmem           = procs->ki_size;
                  proc_elem->rss            = procs->ki_rssize;
                  proc_elem->mem = ((proc_elem->proc.pd_stime + proc_elem->proc.pd_utime) - old_time) *
                                    ((old_vmem + proc_elem->vmem)/2);
               }
            }
         }
      }
      kvm_close(kd);
   }
#elif defined(DARWIN)
   {
      int i, nprocs;
      struct kinfo_proc *procs;
      struct kinfo_proc *procs_begin;
      job_elem_t *job_elem;
      double old_time = 0.0;
      uint64 old_vmem = 0;
      int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
      size_t bufSize = 0;

      if (sysctl(mib, 4, NULL, &bufSize, NULL, 0) < 0) {
         DPRINTF(("sysctl() failed(1)\n"));
         DRETURN(-1);
      }
      if ((procs = (struct kinfo_proc *)malloc(bufSize)) == NULL) {
         DPRINTF(("malloc() failed\n"));
         DRETURN(-1);
      }
      if (sysctl(mib, 4, procs, &bufSize, NULL, 0) < 0) {
         DPRINTF(("sysctl() failed(2)\n"));
         sge_free(&procs);
         DRETURN(-1);
      }
      procs_begin = procs;
      nprocs = bufSize/sizeof(struct kinfo_proc);
      
      for (; nprocs >= 0; nprocs--, procs++) {
         for (curr=job_list.next; curr != &job_list; curr=curr->next) {
            job_elem = LNK_DATA(curr, job_elem_t, link);

            for (i = 0; i < procs->kp_eproc.e_ucred.cr_ngroups; i++) {
               if (job_elem->job.jd_jid == procs->kp_eproc.e_ucred.cr_groups[i]) {
                  lnk_link_t  *curr2;
                  proc_elem_t *proc_elem;
                  int newprocess = 1;
                  
                  if (job_elem->job.jd_proccount != 0) {
                     for (curr2=job_elem->procs.next; curr2 != &job_elem->procs; curr2=curr2->next) {
                        proc_elem = LNK_DATA(curr2, proc_elem_t, link);

                        if (proc_elem->proc.pd_pid == procs->kp_proc.p_pid) {
                           newprocess = 0;
                           break;
                        }
                     }
                  }
                  if (newprocess) {
                     proc_elem = malloc(sizeof(proc_elem_t));
                     if (proc_elem == NULL) {
                        sge_free(&procs_begin);
                        DRETURN(0);
                     }

                     memset(proc_elem, 0, sizeof(proc_elem_t));
                     proc_elem->proc.pd_length = sizeof(psProc_t);
                     proc_elem->proc.pd_state  = 1; /* active */
                     proc_elem->proc.pd_pstart = procs->kp_proc.p_starttime.tv_sec;

                     LNK_ADD(job_elem->procs.prev, &proc_elem->link);
                     job_elem->job.jd_proccount++;
                  } else {
                     /* save previous usage data - needed to build delta usage */
                     old_time = proc_elem->proc.pd_utime + proc_elem->proc.pd_stime;
                     old_vmem  = proc_elem->vmem;
                  }
                  proc_elem->proc.pd_tstamp = time_stamp;
                  proc_elem->proc.pd_pid    = procs->kp_proc.p_pid;
                  DPRINTF(("pid: %d\n", proc_elem->proc.pd_pid));

                  {
                     struct task_basic_info t_info;
                     struct task_thread_times_info t_times_info;
                     mach_port_t task;
                     unsigned int info_count = TASK_BASIC_INFO_COUNT;

                     if (task_for_pid(mach_task_self(), proc_elem->proc.pd_pid, &task) != KERN_SUCCESS) {
                        DPRINTF(("task_for_pid() error"));
                     } else {
                        if (task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &info_count) != KERN_SUCCESS) {
                           DPRINTF(("task_info() error"));
                        } else {
                           proc_elem->vmem           = t_info.virtual_size;
                           DPRINTF(("vmem: %d\n", proc_elem->vmem));
                           proc_elem->rss            = t_info.resident_size;
                           DPRINTF(("rss: %d\n", proc_elem->rss));
                        }

                        info_count = TASK_THREAD_TIMES_INFO_COUNT;
                        if (task_info(task, TASK_THREAD_TIMES_INFO, (task_info_t)&t_times_info, &info_count) != KERN_SUCCESS) {
                           DPRINTF(("task_info() error\n"));
                        } else {
                           proc_elem->proc.pd_utime  = t_times_info.user_time.seconds;
                           DPRINTF(("user_time: %d\n", proc_elem->proc.pd_utime));
                           proc_elem->proc.pd_stime  = t_times_info.system_time.seconds;
                           DPRINTF(("system_time: %d\n", proc_elem->proc.pd_stime));
                        }
                     }
                  }

                  proc_elem->proc.pd_uid    = procs->kp_eproc.e_ucred.cr_uid;
                  DPRINTF(("uid: %d\n", proc_elem->proc.pd_uid));
                  proc_elem->proc.pd_gid    = procs->kp_eproc.e_pcred.p_rgid;
                  DPRINTF(("gid: %d\n", proc_elem->proc.pd_gid));
                  proc_elem->mem = ((proc_elem->proc.pd_stime + proc_elem->proc.pd_utime) - old_time) *
                                         ((old_vmem + proc_elem->vmem)/2);
                  DPRINTF(("mem %d\n", proc_elem->mem));
               }
            }
         }
      }
      sge_free(&procs_begin);
   }
#endif

   for (curr=job_list.next; curr != &job_list; curr=next) {
      psJob_t *job;
      job_elem_t *job_elem;

      next = curr->next;
      job_elem = LNK_DATA(curr, job_elem_t, link);
      job = &job_elem->job;

      /* if job has not been watched within 30 seconds of being pre-added
         to job list, delete it */

      if (job_elem->precreated) {

         if ((job_elem->precreated + 30) < time_stamp) {

#           if DEBUG

               fprintf(df, "%d deleting precreated "F64"\n", time_stamp,
                       job->jd_jid);
               fflush(df);

#           endif

            /* remove from list */
            LNK_DELETE(curr);
            free_job(job_elem);
         }

         continue;  /* skip precreated jobs */
      }

#if  defined(FREEBSD) || defined(LINUX) || defined(SOLARIS) || defined(HP1164) || defined(DARWIN)
      {
         lnk_link_t *currp, *nextp;

         /* sum up usage of each process for this job */
         job->jd_utime_a = job->jd_stime_a = 0;
         job->jd_vmem = 0;
         job->jd_rss = 0;

         for(currp=job_elem->procs.next; currp != &job_elem->procs;
             currp=nextp) {

            proc_elem_t *proc_elem = LNK_DATA(currp, proc_elem_t, link);
            psProc_t *proc = &proc_elem->proc;

            nextp = currp->next; /* in case currp is deleted */

            if (time_stamp == proc->pd_tstamp) { 
               /* maybe still living */
               job->jd_utime_a += proc->pd_utime;    
               job->jd_stime_a += proc->pd_stime;    
               job->jd_vmem += proc_elem->vmem;    
               job->jd_rss += proc_elem->rss;    
               job->jd_mem += (proc_elem->mem/1024.0);    
#if defined(LINUX)
               job->jd_chars += proc_elem->delta_chars;     
#endif
            } else { 
               /* most likely exited */
               job->jd_utime_c += proc->pd_utime;    
               job->jd_stime_c += proc->pd_stime;    
               job->jd_proccount--;
              
               /* remove process entry from list */
#ifdef MONITOR_PDC
               INFO((SGE_EVENT, "lost process "pid_t_fmt" for job "pid_t_fmt" (utime = %f stime = %f)\n", 
                     proc->pd_pid, job->jd_jid, proc->pd_utime, proc->pd_stime));
#endif
               LNK_DELETE(currp);
               sge_free(&proc_elem);
            }
         }
         /* estimate high water memory mark */
         if (job->jd_vmem > job->jd_himem)
            job->jd_himem = job->jd_vmem;
      } 

#endif  /* IRIX */
   }


   if (time_stamp > pnext_time)
      pnext_time = time_stamp + ps_config.prc_collection_interval;

   DRETURN(0);
}

static time_t start_time;

int psStartCollector(void)
{
   static int initialized = 0;
#ifdef PDC_STANDALONE
   int ncpus = 0;
#endif   


   if (initialized)
      return 0;

   initialized = 1;

   LNK_INIT(&job_list);
   start_time = get_gmt();

   /* page size */
   pagesize = getpagesize();

   /* retrieve static parameters */
#if defined(LINUX) || defined(SOLARIS) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(HP1164)
#  ifdef PDC_STANDALONE
   ncpus = sge_nprocs();
#  endif
#endif  /* ALPHA */
#ifdef PDC_STANDALONE
   /* Length of struct (set@run-time) [not actually used?] */
   sysdata.sys_length = sizeof(sysdata);
   sysdata.sys_ncpus = ncpus;
#endif
   init_procfs();
   return 0;
}


int psStopCollector(void)
{

   return 0;
}


int psWatchJob(JobID_t JobID)
{
   lnk_link_t *curr;

#  if DEBUG

      if (df == NULL)
         df = fopen("/tmp/pacct.out", "w");
      fprintf(df, "%d watching "F64"\n", get_gmt(), JobID);
      fflush(df);

#  endif

   /* if job to watch is not already in the list then add it */

   if ((curr=find_job(JobID))) {
      LNK_DATA(curr, job_elem_t, link)->precreated = 0;
   } else {
      job_elem_t *job_elem = sge_malloc(sizeof(job_elem_t));
      memset(job_elem, 0, sizeof(job_elem_t));
      job_elem->starttime = get_gmt();
      job_elem->job.jd_jid = JobID;
      job_elem->job.jd_length = sizeof(psJob_t);
      LNK_INIT(&job_elem->procs);
      LNK_INIT(&job_elem->arses);
      /* add to job list */
      LNK_ADD(job_list.prev, &job_elem->link);
   }

   return 0;
}


int psIgnoreJob(JobID_t JobID) {
   lnk_link_t *curr;

   /* if job is in the list, remove it */

   if ((curr = find_job(JobID))) {
      LNK_DELETE(curr);
      free_job(LNK_DATA(curr, job_elem_t, link));
   }

   return 0;
}


struct psStat_s *psStatus(void)
{
   psStat_t *pstat;
   static time_t last_time_stamp;
   time_t time_stamp = get_gmt();

   if ((pstat = (psStat_t *)malloc(sizeof(psStat_t)))==NULL) {
      return NULL;
   }

   /* Length of struct (set@run-time) */
   pstat->stat_length = sizeof(psStat_t);

   /* Time of last sample */
   pstat->stat_tstamp = last_time_stamp;

   /* our pid */
   pstat->stat_ifmpid = getpid();

   /* DC pid */
   pstat->stat_DCpid = getpid();

   /* IFM pid */
   pstat->stat_IFMpid = getpid();

   /* elapsed time (to *now*, not snap) */
   pstat->stat_elapsed = time_stamp - start_time;

   /* user CPU time used by DC */
   pstat->stat_DCutime = 0;

   /* sys CPU time used by DC */
   pstat->stat_DCstime = 0;

   /* user CPU time used by IFM */
   pstat->stat_IFMutime = 0;

   /* sys CPU time used by IFM */
   pstat->stat_IFMstime = 0;

   /* number of jobs tracked */
   pstat->stat_jobcount = get_numjobs();

   last_time_stamp = time_stamp;

   return pstat;
}


struct psJob_s *psGetOneJob(JobID_t JobID)
{
   psJob_t *job;
   lnk_link_t *curr;
   job_elem_t *job_elem = NULL;
   int found = 0;
   struct rjob_s {
      psJob_t job;
      psProc_t proc[1];
   } *rjob = NULL;

   /* retrieve job data */
   psRetrieveOSJobData();

   /* see if job is in list */

   for (curr=job_list.next; curr != &job_list; curr=curr->next) {
      job_elem = LNK_DATA(curr, job_elem_t, link);
      if (job_elem->precreated) continue; /* skip precreated jobs */
      if (job_elem->job.jd_jid == JobID) {
         found = 1;
         break;
      }
   }

   if (found) {
      unsigned long rsize;

      job = &job_elem->job;
      rsize = sizeof(psJob_t) + job->jd_proccount * sizeof(psProc_t);
      if ((rjob = (struct rjob_s *)malloc(rsize))) {
         memcpy(&rjob->job, job, sizeof(psJob_t));
         {
            lnk_link_t *currp;
            int nprocs = 0;

            for (currp=job_elem->procs.next; currp != &job_elem->procs;
                     currp=currp->next) {
               psProc_t *proc = &(LNK_DATA(currp, proc_elem_t, link)->proc);
               memcpy(&rjob->proc[nprocs++], proc, sizeof(psProc_t));
            }
         }
      }
   }

   return (struct psJob_s *)rjob;
}


struct psJob_s *psGetAllJobs(void)
{
   psJob_t *rjob, *jobs;
   lnk_link_t *curr;
   long rsize;
   uint64 jobcount = 0;

   /* retrieve job data */
   psRetrieveOSJobData();

   /* calculate size of return data */
#ifndef SOLARIS
   rsize = sizeof(uint64);
#else
   rsize = 8;
#endif

   for (curr=job_list.next; curr != &job_list; curr=curr->next) {
      job_elem_t *job_elem = LNK_DATA(curr, job_elem_t, link);
      psJob_t *job = &job_elem->job;
      if (job_elem->precreated) continue; /* skip precreated jobs */
      rsize += (sizeof(psJob_t) + (job->jd_proccount*sizeof(psProc_t)));
      jobcount++;
   }

   /* allocate space for return data */
   if ((rjob = (psJob_t *)malloc(rsize)) == NULL)
      return rjob;
  
   /* allign adress */

   /* fill in return data */
   jobs = rjob;
   *(uint64 *)jobs = jobcount;
#ifndef SOLARIS
   INCJOBPTR(jobs, sizeof(uint64));
#else
   INCJOBPTR(jobs, 8);
#endif

   /* copy the job data */
   for (curr=job_list.next; curr != &job_list; curr=curr->next) {
      job_elem_t *job_elem = LNK_DATA(curr, job_elem_t, link);
      psJob_t *job = &job_elem->job;
      psProc_t *procs;

      if (job_elem->precreated) continue; /* skip precreated jobs */
      memcpy(jobs, job, sizeof(psJob_t));
      INCJOBPTR(jobs, sizeof(psJob_t));

      /* copy the process data */
      procs = (psProc_t *)jobs;
      {
         lnk_link_t *currp;
         for (currp=job_elem->procs.next; currp != &job_elem->procs; currp=currp->next) {
            psProc_t *proc = &(LNK_DATA(currp, proc_elem_t, link)->proc);
            memcpy(procs, proc, sizeof(psProc_t));
            INCPROCPTR(procs, sizeof(psProc_t));
         }
      }
      jobs = (psJob_t *)procs;

   }

   return rjob;
}


int psVerify(void)
{
   return 0;
}

#ifdef PDC_STANDALONE
struct psSys_s *psGetSysdata(void)
{
   psSys_t *sd;

   /* go get system data */
   psRetrieveSystemData();

   if ((sd = (psSys_t *)malloc(sizeof(psSys_t))) == NULL) {
      return NULL;
   }
   memcpy(sd, &sysdata, sizeof(psSys_t));
   return sd;
}

#define INTOMEGS(x) (((double)x)/(1024*1024))



void
usage(void)
{
   fprintf(stderr, "\n%s\n\n", MSG_SGE_USAGE);
   fprintf(stderr, "\t-s\t%s\n",  MSG_SGE_s_OPT_USAGE);
   fprintf(stderr, "\t-n\t%s\n",  MSG_SGE_n_OPT_USAGE);
   fprintf(stderr, "\t-p\t%s\n",  MSG_SGE_p_OPT_USAGE);
   fprintf(stderr, "\t-i\t%s\n",  MSG_SGE_i_OPT_USAGE);
   fprintf(stderr, "\t-g\t%s\n",  MSG_SGE_g_OPT_USAGE);
   fprintf(stderr, "\t-j\t%s\n",  MSG_SGE_j_OPT_USAGE);
   fprintf(stderr, "\t-J\t%s\n",  MSG_SGE_J_OPT_USAGE);
   fprintf(stderr, "\t-k\t%s\n",  MSG_SGE_k_OPT_USAGE);
   fprintf(stderr, "\t-K\t%s\n",  MSG_SGE_K_OPT_USAGE);
   fprintf(stderr, "\t-P\t%s\n",  MSG_SGE_P_OPT_USAGE);
   fprintf(stderr, "\t-S\t%s\n",  MSG_SGE_S_OPT_USAGE);
}


static void
print_job_data(psJob_t *job)
{
   printf("%s\n", MSG_SGE_JOBDATA );
   printf("jd_jid="JOBID_T_FMT"\n", job->jd_jid);
   printf("jd_length=%d\n", job->jd_length);
   printf("jd_uid="uid_t_fmt"\n", job->jd_uid);
   printf("jd_gid="uid_t_fmt"\n", job->jd_gid);
   printf("jd_tstamp=%s\n", ctime(&job->jd_tstamp));
   printf("jd_proccount=%d\n", (int)job->jd_proccount);
   printf("jd_refcnt=%d\n", (int)job->jd_refcnt);
   printf("jd_etime=%8.3f\n", job->jd_etime);
   printf("jd_utime_a=%8.3f\n", job->jd_utime_a);
   printf("jd_stime_a=%8.3f\n", job->jd_stime_a);
   printf("jd_srtime_a=%8.3f\n", job->jd_srtime_a);
   printf("jd_utime_c=%8.3f\n", job->jd_utime_c);
   printf("jd_stime_c=%8.3f\n", job->jd_stime_c);
   printf("jd_srtime_c=%8.3f\n", job->jd_srtime_c);
   printf("jd_mem=%lu\n", job->jd_mem);
   printf("jd_chars=%8.3fM\n", INTOMEGS(job->jd_chars));
   printf("jd_vmem=%8.3fM\n", INTOMEGS(job->jd_vmem));
   printf("jd_rss=%8.3fM\n", INTOMEGS(job->jd_rss));
   printf("jd_himem=%8.3fM\n", INTOMEGS(job->jd_himem));
}
static void
print_process_data(psProc_t *proc)
{
   printf("\t******* Process Data *******\n");
   printf("\tpd_pid="pid_t_fmt"\n", proc->pd_pid);
   printf("\tpd_length=%d\n", (int)proc->pd_length);
   printf("\tpd_tstamp=%s\n", ctime(&proc->pd_tstamp));
   printf("\tpd_uid="uid_t_fmt"\n", proc->pd_uid);
   printf("\tpd_gid="uid_t_fmt"\n", proc->pd_gid);
   printf("\tpd_acid=%lu\n", proc->pd_acid);
   printf("\tpd_state=%d\n", (int)proc->pd_state);
   printf("\tpd_pstart=%8.3f\n", proc->pd_pstart);
   printf("\tpd_utime=%8.3f\n", proc->pd_utime);
   printf("\tpd_stime=%8.3f\n", proc->pd_stime);
}


#if 0
static void
print_system_data(psSys_t *sys)
{
   printf("%s\n", MSG_SGE_SYSTEMDATA );
   printf("sys_length=%d\n", (int)sys->sys_length);
   printf("sys_ncpus=%d\n", (int)sys->sys_ncpus);
   printf("sys_tstamp=%s\n", ctime(&sys->sys_tstamp));
   printf("sys_ttimet=%8.3f\n", sys->sys_ttimet);
   printf("sys_ttime=%8.3f\n", sys->sys_ttime);
   printf("sys_utimet=%8.3f\n", sys->sys_utimet);
   printf("sys_utime=%8.3f\n", sys->sys_utime);
   printf("sys_stimet=%8.3f\n", sys->sys_stimet);
   printf("sys_stime=%8.3f\n", sys->sys_stime);
   printf("sys_itimet=%8.3f\n", sys->sys_itimet);
   printf("sys_itime=%8.3f\n", sys->sys_itime);
   printf("sys_srtimet=%8.3f\n", sys->sys_srtimet);
   printf("sys_srtime=%8.3f\n", sys->sys_srtime);
   printf("sys_wtimet=%8.3f\n", sys->sys_wtimet);
   printf("sys_wtime=%8.3f\n", sys->sys_wtime);

   printf("sys_swp_total=%8.3fM\n", INTOMEGS(sys->sys_swp_total));
   printf("sys_swp_free=%8.3fM\n", INTOMEGS(sys->sys_swp_free));
   printf("sys_swp_used=%8.3fM\n", INTOMEGS(sys->sys_swp_used));
   printf("sys_swp_virt=%8.3fM\n", INTOMEGS(sys->sys_swp_virt));
   printf("sys_swp_rate=%8.3f\n", sys->sys_swp_rate);
   printf("sys_mem_avail=%8.3fM\n", INTOMEGS(sys->sys_mem_avail));
   printf("sys_mem_used=%8.3fM\n", INTOMEGS(sys->sys_mem_used));

   printf("sys_swpocc=%8.3f\n", sys->sys_swpocc);
   printf("sys_swpque=%8.3f\n", sys->sys_swpque);
   printf("sys_runocc=%8.3f\n", sys->sys_runocc);
   printf("sys_runque=%8.3f\n", sys->sys_runque);
   printf("sys_readch="F64"\n", sys->sys_readch);
   printf("sys_writech="F64"\n", sys->sys_writech);
}
#endif  /* 0 */


static void
print_status(psStat_t *stat)
{
   printf("%s\n", MSG_SGE_STATUS );
   printf("stat_length=%d\n", (int)stat->stat_length);
   printf("stat_tstamp=%s\n", ctime(&stat->stat_tstamp));
   printf("stat_ifmpid=%d\n", (int)stat->stat_ifmpid);
   printf("stat_DCpid=%d\n", (int)stat->stat_DCpid);
   printf("stat_IFMpid=%d\n", (int)stat->stat_IFMpid);
   printf("stat_elapsed=%d\n", (int)stat->stat_elapsed);
   printf("stat_DCutime=%8.3f\n", stat->stat_DCutime);
   printf("stat_DCstime=%8.3f\n", stat->stat_DCstime);
   printf("stat_IFMutime=%8.3f\n", stat->stat_IFMutime);
   printf("stat_IFMstime=%8.3f\n", stat->stat_IFMstime);
   printf("stat_jobcount=%d\n", (int)stat->stat_jobcount);
}


int
main(int argc, char **argv)
{
   char sgeview_bar_title[256] = "";  
   char sgeview_window_title[256] = ""; 
   int arg;
   JobID_t osjobid;
   int verbose = 1;
   int showproc = 0;
   int interval = 2;
   int system = 0;
   int use_getonejob = 0;
   int sgeview = 0;
   int killjob = 0;
   int forcekill = 0;
   int signo = 15;
   int c;
   int sysi=-1, jobi=-1, prci=-1;
   int numjobs = 0;
   double *curr_cpu=NULL, *prev_cpu=NULL, *diff_cpu=NULL;
   int jobid_count = 0;
   char *jobids[256];
   int stop = 1;
/* dstring ds;
   char buffer[256];

   sge_dstring_init(&ds, buffer, sizeof(buffer));
   sprintf(sgeview_bar_title, "%-.250s", MSG_SGE_CPUUSAGE  );
   sprintf(sgeview_window_title, "%-.100s %-.150s", feature_get_product_name(FS_SHORT, &ds) ,MSG_SGE_SGEJOBUSAGECOMPARSION );
*/

#ifdef __SGE_COMPILE_WITH_GETTEXT__ 
   /* init language output for gettext() , it will use the right language */

   sge_init_language_func((gettext_func_type)        gettext,
                         (setlocale_func_type)      setlocale,
                         (bindtextdomain_func_type) bindtextdomain,
                         (textdomain_func_type)     textdomain);
   sge_init_language(NULL,NULL);  
#endif /* __SGE_COMPILE_WITH_GETTEXT__  */
   
   psStartCollector();

   gen_procList();
   if (argc < 2) {
      usage();
      exit(1);
   }

   while ((c = getopt(argc, argv, "g1snpi:S:J:P:j:k:K:")) != -1)
      switch(c) {
         case 'g':
            sgeview = 1;
            break;
         case 'j':
            jobids[jobid_count++] = optarg;
            break;
         case 'K':
            forcekill = 1;
            /* no break here, fall into 'k' case */
         case 'k':
	    killjob = 1;
            if (sscanf(optarg, "%d", &signo)!=1) {
               fprintf(stderr, MSG_SGE_XISNOTAVALIDSIGNALNUMBER_S , optarg);
               fprintf(stderr, "\n");
               usage();
               exit(6);
            }
            break;
         case '1':
            use_getonejob = 1;
            break;
         case 's':
            system = 1;
            break;
         case 'n':
            verbose = 0;
            break;
	 case 'p':
	    showproc = 1;
	    break;
	 case 'S':
            if (sscanf(optarg, "%d", &sysi)!=1) {
               fprintf(stderr, MSG_SGE_XISNOTAVALIDINTERVAL_S, optarg);
               fprintf(stderr, MSG_SGE_XISNOTAVALIDSIGNALNUMBER_S , optarg);
               usage();
               exit(4);
            }
	    break;
	 case 'P':
            if (sscanf(optarg, "%d", &prci)!=1) {
               fprintf(stderr, MSG_SGE_XISNOTAVALIDINTERVAL_S, optarg);
               usage();
               exit(5);
            }
	    break;
	 case 'J':
            if (sscanf(optarg, "%d", &jobi)!=1) {
               fprintf(stderr, MSG_SGE_XISNOTAVALIDINTERVAL_S, optarg);
               usage();
               exit(6);
            }
	    break;
	 case 'i':
            if (sscanf(optarg, "%d", &interval)!=1) {
               fprintf(stderr, MSG_SGE_XISNOTAVALIDINTERVAL_S, optarg);
               fprintf(stderr, "\n");
               usage();
               exit(3);
            }
	    break;
	 case '?':
	 default:
	    usage();
	    exit(2);
      }

   for (arg=optind; arg<argc; arg++) {
      if (sscanf(argv[arg], JOBID_T_FMT, &osjobid) != 1) {
         fprintf(stderr, MSG_SGE_XISNOTAVALIDJOBID_S , argv[arg]);
         fprintf(stderr, "\n");
         exit(2);
      }
      psWatchJob(osjobid);
      numjobs++;
   }

   psSetCollectionIntervals(jobi, prci, sysi);

   if (sgeview) {
      int i;
      int base_interval = 2; /* in tenths of a second */
      int sample_rate = 1;
      int num_samples = 1000;
      int use_winsize = 0;

      curr_cpu = sge_malloc(numjobs * sizeof(double));
      memset(curr_cpu, 0, numjobs*sizeof(double));
      prev_cpu = sge_malloc(numjobs * sizeof(double));
      memset(prev_cpu, 0, numjobs*sizeof(double));
      diff_cpu = sge_malloc(numjobs * sizeof(double));
      memset(diff_cpu, 0, numjobs*sizeof(double));

      printf("%s\n", MSG_SGE_GROSVIEWEXPORTFILE );
      printf("=14 3\n");  /* arbsize */
      printf("=14 2 1\n");
      if (use_winsize)
         printf("=14 6 800 100\n"); /* winsize(x,y) */
      printf("=14 9 %d\n", base_interval);
      printf("=14 7 46\n");
      printf("=14 8 0\n");
      printf("=11 0  0x20000 0x4 0x%x 0x%x 0.000 1.000 0x%x 0 0 0 0 0 0x%x "
             "0 0 0x2e 0 0 0x1 0x7 0x4 0x%x 0x%x 0 0 0 0x4 0x1 0x6 "
             "0x%x 0x5 0x2e\n", sample_rate, sample_rate, numjobs+1,
             num_samples, sample_rate, sample_rate, numjobs+1);
      printf("h%s \n", sgeview_bar_title);
      for (i=0; i<numjobs; i++)
         if (jobid_count > i)
	    printf("ljob %s \n", jobids[i]);
         else
	    printf("ljob %d \n", i+1);
      printf("l \n");
      printf("=8\n");
      printf("%s\n", sgeview_window_title);
      fflush(stdout);

   }

   while(stop == 1) {
      psJob_t *jobs, *ojob;
      psProc_t *procs;
      psStat_t *stat = NULL;
      psSys_t *sys = NULL;
      int jobcount, proccount, i, j, activeprocs;

      activeprocs = 0;
      jobcount = 0;
      ojob = NULL;
      stat = NULL;
      sys = NULL;

      if (!sgeview && system) {

         if ((stat = psStatus()))
            if (verbose)
               print_status(stat);
#if 0
         if ((sys = psGetSysdata()))
            if (verbose)
               print_system_data(sys);
#endif      
      }

      ojob = jobs = psGetAllJobs();
      if (jobs) {
         jobcount = *(uint64 *)jobs;
         INCJOBPTR(jobs, sizeof(uint64));
         for (i=0; i<jobcount; i++) {

            if (sgeview) {
               prev_cpu[i] = curr_cpu[i];
               curr_cpu[i] = jobs->jd_utime_a + jobs->jd_stime_a +
                             jobs->jd_utime_c + jobs->jd_stime_c;
            } else if (use_getonejob) {
               psJob_t *jp, *ojp;
               psProc_t *pp;
               if ((ojp = jp = psGetOneJob(jobs->jd_jid))) {
                  if (verbose && !killjob)
                     print_job_data(jp);
                  proccount = jp->jd_proccount;
                  INCJOBPTR(jp, jp->jd_length);
                  pp = (psProc_t *)jp;
                  for (j=0; j<proccount; j++) {
                     if (verbose && showproc)
                        print_process_data(pp);
                     INCPROCPTR(pp, pp->pd_length);
                  }
                  sge_free(&ojp);
               } 

            } else if (verbose && !killjob)
               print_job_data(jobs);

            proccount = jobs->jd_proccount;
            activeprocs += proccount;
            INCJOBPTR(jobs, jobs->jd_length);
            procs = (psProc_t *)jobs;

            for (j=0; j<proccount; j++) {
               if (killjob) {
                  if (getuid() == SGE_SUPERUSER_UID ||
                      getuid() == procs->pd_uid) {
                     if (kill(procs->pd_pid, signo)<0) {
                        char buf[128];
                        sprintf(buf, "kill("pid_t_fmt", %d)", procs->pd_pid, signo);
                        perror(buf);
                     } else if (verbose)
                        printf("kill("pid_t_fmt", %d) issued\n", procs->pd_pid, signo);
                  } else {
                     fprintf(stderr, "kill: "pid_t_fmt ": %s\n",
                             procs->pd_pid, MSG_SGE_PERMISSIONDENIED);
                  }
               }
               if (verbose && showproc && !use_getonejob)
                  print_process_data(procs);
               INCPROCPTR(procs, procs->pd_length);
            }

            jobs = (psJob_t *)procs;
         }

      } else if (verbose)
         printf("%s\n", MSG_SGE_NOJOBS );

      if (sgeview && jobcount>0) {
         int i;
         double cpu_pct, total_cpu = 0, total_cpu_pct = 0;

         for(i=0; i<jobcount; i++) {
            diff_cpu[i] = curr_cpu[i] - prev_cpu[i];
            total_cpu += diff_cpu[i];
         }

         printf("=3 0 ");
         for(i=0; i<jobcount; i++) {
            cpu_pct = 0;
            if (total_cpu > 0)
               cpu_pct = diff_cpu[i] / total_cpu;
            total_cpu_pct += cpu_pct;
            printf("%8.5f ", cpu_pct);
         }
         printf("%8.5f ", 1.0 - total_cpu_pct);
         printf("\n=15\n");
         fflush(stdout);
      }

      if (ojob) {
         sge_free(&ojob);
      }
      if (stat) {
         sge_free(&stat);
      }
      if (sys) {
         sge_free(&sys);
      }
      if (killjob && (!forcekill || activeprocs == 0))
         break;

      sleep(interval);
   }
   free_procList();
   return 0;
}

#endif  /* PDC_STANDALONE */

#endif /* !defined(COMPILE_DC) */
