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

#if defined(IRIX)
#include <sys/sysmp.h>
#include <sys/syssgi.h>
#include <sys/arsess.h>
#include <sys/procfs.h>
#include <sys/sysinfo.h>
#include <sys/tcpipstats.h>
#include <sys/systeminfo.h>
#include <sys/swap.h>
#endif

#if defined(ALPHA)
#   include <nlist.h>
#   include <sys/sysinfo.h>
#   include <machine/hal_sysinfo.h>
#   include <mach.h>
#   include </sys/include/vm/vm_perf.h>
#   include <paths.h>
#endif

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

#if defined(LINUX) || defined(ALPHA) || defined(IRIX) || defined(SOLARIS) || defined(DARWIN) || defined (FREEBSD) || defined(NETBSD) || defined(HP1164) || defined(AIX)

#include "uti/sge_os.h"
#endif

#if defined(IRIX)
#  define F64 "%lld"
#  define S64 "%lli"
#elif defined(ALPHA)
#  define F64 "%ld"
#  define S64 "%li"
#elif defined(LINUX) || defined(SOLARIS)
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

#if defined(IRIX)

/*
 * This is a structure containing all the fields that we need
 * out of the arsess_t structure.  It is filled in by the
 * pdc_get_arsess() and pdc_get_arsess64() routines.
 */

typedef struct {
    ash_t  ash;
    pid_t  pid;
    uint64 prid;
    uint64 start;
    uint64 refcnt;
    uint64 utime;
    uint64 stime;
    uint64 bwtime;
    uint64 rwtime;
    uint64 qwtime;
    uint64 mem;
    uint64 chr;
    uint64 chw;
} pdc_arsess_t;

int
pdc_get_arsess(pdc_arsess_t *parse, arsess_t *arse)
{
   parse->ash = arse->as_handle;
   parse->pid = arse->as_pid;
   parse->prid = arse->as_prid;
   parse->start = arse->as_start;
   parse->refcnt = arse->as_refcnt;
   parse->utime = arse->as_timers.ac_utime;
   parse->stime = arse->as_timers.ac_stime;
   parse->bwtime = arse->as_timers.ac_bwtime;
   parse->rwtime = arse->as_timers.ac_rwtime;
   parse->qwtime = arse->as_timers.ac_qwtime;
   parse->mem = arse->as_counts.ac_mem;
   parse->chr = arse->as_counts.ac_chr;
   parse->chw = arse->as_counts.ac_chw;
   return 0;
}

/*
 * define a 64-bit version of arsess_t for use on 64-bit IRIX
 */

typedef struct arsess64 {
        ash_t           as_handle;      /* array session handle */
        prid_t          as_prid;        /* project ID */

        lock_t          as_lock;        /* update lock */
#ifdef notdef
        struct arsess   *as_next;       /* next arsess in act/free list */
        struct arsess   *as_prev;       /* previous arsess in act list */
#else
        __uint64_t      as_next;
        __uint64_t      as_prev;
#endif
        int             as_refcnt;      /* reference count */
        time_t          as_start;       /* start time (secs since 1970) */
        time_t          as_ticks;       /* lbolt at start */
        pid_t           as_pid;         /* pid that started this session */
        ushort_t        as_flag;        /* various flags */
        char            as_nice;        /* initial nice value of as_pid */

        /* Accounting data */
        acct_spi_t      as_spi;         /* Service Provider Information */
        acct_timers_t   as_timers;      /* accounting timers */
        acct_counts_t   as_counts;      /* accounting counters */

	__uint64_t      as_fill;        /* fill for 64-bit structure */
} arsess64_t;


typedef struct arsess65 {
        ash_t           as_handle;      /* array session handle */
        prid_t          as_prid;        /* project ID */
        int             as_refcnt;      /* reference count */
        time_t          as_start;       /* start time (secs since 1970) */
        time_t          as_ticks;       /* lbolt at start */
        pid_t           as_pid;         /* pid that started this session */
        int             as_spilen;      /* length of Service Provider Info */
        ushort_t        as_flag;        /* various flags */
        char            as_nice;        /* initial nice value of as_pid */
        char            as_rsrv1[985];  /*   reserved */

        /* Accounting data */
        char            as_spi[1024];   /* Service Provider Info */
        acct_timers_t   as_timers;      /* accounting timers */
        acct_counts_t   as_counts;      /* accounting counters */
        char            as_rsrv2[1888]; /*   reserved */
} arsess65_t;


int pdc_get_arsess65(pdc_arsess_t *parse, arsess_t *arsein)
{
   arsess65_t *arse = (arsess65_t *)arsein;

   parse->ash = arse->as_handle;
   parse->pid = arse->as_pid;
   parse->prid = arse->as_prid;
   parse->start = arse->as_start;
   parse->refcnt = arse->as_refcnt;
   parse->utime = arse->as_timers.ac_utime;
   parse->stime = arse->as_timers.ac_stime;
   parse->bwtime = arse->as_timers.ac_bwtime;
   parse->rwtime = arse->as_timers.ac_rwtime;
   parse->qwtime = arse->as_timers.ac_qwtime;
   parse->mem = arse->as_counts.ac_mem;
   parse->chr = arse->as_counts.ac_chr;
   parse->chw = arse->as_counts.ac_chw;
   return 0;
}


int pdc_get_arsess64(pdc_arsess_t *parse, arsess_t *arsein)
{
   arsess64_t *arse = (arsess64_t *)arsein;

   parse->ash = arse->as_handle;
   parse->pid = arse->as_pid;
   parse->prid = arse->as_prid;
   parse->start = arse->as_start;
   parse->refcnt = arse->as_refcnt;
   parse->utime = arse->as_timers.ac_utime;
   parse->stime = arse->as_timers.ac_stime;
   parse->bwtime = arse->as_timers.ac_bwtime;
   parse->rwtime = arse->as_timers.ac_rwtime;
   parse->qwtime = arse->as_timers.ac_qwtime;
   parse->mem = arse->as_counts.ac_mem;
   parse->chr = arse->as_counts.ac_chr;
   parse->chw = arse->as_counts.ac_chw;
   return 0;
}
#elif defined(ALPHA)

static struct nlist mem_nl[] = {
   { "vm_perfsum" }, /* PERFSUM */
   { 0 },
};
int kmem_fd = -1;

#define PERFSUM      0

int readk(off_t where, char *addr, int size) {
   if (lseek(kmem_fd, where, SEEK_SET) == -1)
      return -1;
   if (read(kmem_fd, addr, size) == -1)
      return -1;
   return 0;
}

#endif

static int
get_gmt(void)
{
   struct timeval now;

   gettimeofday(&now, NULL);

   return now.tv_sec;
}

#ifdef PDC_STANDALONE
static psSys_t sysdata;

#if defined(IRIX)
static struct {
   int initialized;
   double utime;
   double stime;
   double itime;
   double srtime;
   double wtime;
   double ttime;
} base;
#endif
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
#if defined(IRIX)
   struct sysinfo si;
   struct rminfo rmi;
   struct minfo mi;
#ifdef ever_needed
   struct dinfo di;
   struct syserr se;
   struct kna k;
#endif
   off_t swapmax, swapvirt, swaprsrv, swaptot, swapfree;
   double utime, stime, itime, srtime, wtime, ttime;
   double period;
   static uint64 prev_runque, prev_runocc, prev_swpque, prev_swpocc;
   long clock_tick = sysconf(_SC_CLK_TCK);
#elif defined(ALPHA)
   struct vm_statistics vmstats;
#endif
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

#if defined(IRIX)


   if (sysmp(MP_SAGET, MPSA_SINFO, &si, sizeof(si))<0) {
      return -1;
   }

   if (sysmp(MP_SAGET, MPSA_RMINFO, &rmi, sizeof(rmi))<0) {
      return -1;
   }

   if (sysmp(MP_SAGET, MPSA_MINFO, &mi, sizeof(mi))<0) {
      return -1;
   }

#ifdef ever_needed

   if (sysmp(MP_SAGET, MPSA_SERR, &se, sizeof(se))<0) {
      return -1;
   }

   if (sysmp(MP_SAGET, MPSA_DINFO, &di, sizeof(di))<0) {
      return -1;
   }

   if (sysmp(MP_SAGET, MPSA_TCPIPSTATS, &k, sizeof(k))<0) {
      return -1;
   }

#endif

   if (swapctl(SC_GETFREESWAP, &swapfree)<0) {
      return -1;
   }
   
   if (swapctl(SC_GETSWAPMAX, &swapmax)<0) {
      return -1;
   }
   
   if (swapctl(SC_GETSWAPVIRT, &swapvirt)<0) {
      return -1;
   }
   
   if (swapctl(SC_GETRESVSWAP, &swaprsrv)<0) {
      return -1;
   }
   
   if (swapctl(SC_GETSWAPTOT, &swaptot)<0) {
      return -1;
   }

   /* convert CPU time values to double CPU seconds */
   utime = (double)si.cpu[CPU_USER] / (double)clock_tick;
   stime = (double)si.cpu[CPU_KERNEL] / (double)clock_tick;
   itime = (double)si.cpu[CPU_IDLE] / (double)clock_tick;
   srtime = 0;
   wtime = (double)si.cpu[CPU_WAIT] / (double)clock_tick;
   ttime = ((double)si.cpu[CPU_IDLE] + (double)si.cpu[CPU_USER] +
            (double)si.cpu[CPU_KERNEL] + (double)si.cpu[CPU_WAIT] +
            (double)si.cpu[CPU_SXBRK] + (double)si.cpu[CPU_INTR]) /
            (double)clock_tick;

   /* if this is the first time, intialize base CPU time values */

   if (!base.initialized) {
      base.initialized = 1;
      base.utime = utime;
      base.stime = stime;
      base.itime = itime;
      base.srtime = srtime;
      base.wtime = wtime;
      base.ttime = ttime;
      prev_runque = si.runque;
      prev_runocc = si.runocc;
      prev_swpque = si.swpque;
      prev_swpocc = si.swpocc;
   }

   /* total cpu time avail (this int) */
   sysdata.sys_ttime = ttime - (base.ttime + sysdata.sys_ttimet);

   /* total cpu time avail (since start) */
   sysdata.sys_ttimet = ttime - base.ttime;

   /* user time this interval */
   sysdata.sys_utime = utime - (base.utime + sysdata.sys_utimet);

   /* user time (since start) */
   sysdata.sys_utimet = utime - base.utime;

   /* system time this interval */
   sysdata.sys_stime = stime - (base.stime + sysdata.sys_stimet);

   /* system time (since start) */
   sysdata.sys_stimet = stime - base.stime;

   /* idle time this interval */
   sysdata.sys_itime = itime - (base.itime + sysdata.sys_itimet);

   /* idle time (since start) */
   sysdata.sys_itimet = itime - base.itime;

   /* srun wait this interval */
   sysdata.sys_srtime = srtime - (base.srtime + sysdata.sys_srtimet);

   /* srun wait (since start) */
   sysdata.sys_srtimet = srtime - base.srtime;

   /* I/O wait time this interval */
   sysdata.sys_wtime = wtime - (base.wtime + sysdata.sys_wtimet);

   /* I/O wait time (since start) */
   sysdata.sys_wtimet = wtime - base.wtime;

   /* Total Swap space available */
   sysdata.sys_swp_total = (uint64)swaptot * 512;

   /* Swap space free */
   sysdata.sys_swp_free = (uint64)swapfree * 512;

   /* Swap space in use (bytes) */
   sysdata.sys_swp_used = ((uint64)swaptot - (uint64)swapfree) * 512;

   /* swaprsrv is the amount of space currently reserved by processes 
      which is not the same as that which is in use by processes 
      see swapctl(SC_GETRESVSWAP) */

   /* Swap space reserved (bytes) */
   sysdata.sys_swp_rsvd = (uint64)swaprsrv * 512;

   /* Virtual Swap space avail (bytes) */
   sysdata.sys_swp_virt = (uint64)swapvirt * 512;

   /* Swap rate in bytes/second */
   sysdata.sys_swp_rate = 0;

   /* Memory available (unused, free) */
   sysdata.sys_mem_avail = ((uint64)rmi.freemem + (uint64)rmi.chunkpages) *
	 pagesize;

   /* Memory in use (bytes) (SVD 10/19/98 - s/rmi.availrmem/rmi.physmem/) */ 
   sysdata.sys_mem_used = (uint64)rmi.physmem*pagesize - sysdata.sys_mem_avail;

   /* Memory + swap used (bytes) */
   sysdata.sys_mswp_used = sysdata.sys_swp_used + sysdata.sys_mem_used;

   /* Memory + swap avail (bytes) */
   sysdata.sys_mswp_avail = sysdata.sys_swp_free + sysdata.sys_mem_avail;

   if ((time_stamp - prev_time_stamp) > 0)
      period = (time_stamp - prev_time_stamp);
   else
      period = 1.0;
   
   /* Swap "Occ" delta */
   sysdata.sys_swpocc = ((double)si.swpocc - prev_swpocc) / period;
   prev_swpocc = si.swpocc;

   /* Swap Queue delta */
   sysdata.sys_swpque = ((double)si.swpque - prev_swpque) / period;
   prev_swpque = si.swpque;

   /* Run "Occ" delta */
   sysdata.sys_runocc = ((double)si.runocc - prev_runocc) / period;
   prev_runocc = si.runocc;

   /* Run Queue delta */
   sysdata.sys_runque = ((double)si.runque - prev_runque) / period;
   if (sysdata.sys_ncpus > 1) sysdata.sys_runque /= sysdata.sys_ncpus;
   prev_runque = si.runque;

   /* characters read */
   sysdata.sys_readch = si.readch;

   /* characters written */
   sysdata.sys_writech = si.writech;

#elif defined(ALPHA)
   {
      struct vm_perf   perf;
   
      /* memory information */
      /* this is possibly bogus - we work out total # pages by */
      /* adding up the free, active, inactive, wired down, and */
      /* zero filled. Anyone who knows a better way, TELL ME!  */
      /* Change: dont use zero filled. */

      if (mem_nl[PERFSUM].n_value) {
         if (readk((off_t)mem_nl[PERFSUM].n_value,(char *)&perf,sizeof perf))
         /* Virtual Swap space avail (bytes) */
         sysdata.sys_swp_free = perf.vpf_swapspace*pagesize;
      }

      (void) vm_statistics(current_task(),&vmstats);

      /* free mem */
      sysdata.sys_mem_avail = vmstats.free_count*pagesize; 

      /* Memory in use (bytes) */
      sysdata.sys_mem_used = (physical_memory*1024) - sysdata.sys_mem_avail;

      /* Swap space reserved (bytes) */
      sysdata.sys_swp_rsvd = sysdata.sys_swp_used + sysdata.sys_mem_used;

      /* Memory + swap used (bytes) */
      sysdata.sys_mswp_used = sysdata.sys_swp_used + sysdata.sys_mem_used;

      /* Memory + swap avail (bytes) */
      sysdata.sys_mswp_avail = sysdata.sys_swp_free + sysdata.sys_mem_avail;

   }
#endif
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

#ifdef IRIX

/* only used on IRIX 6 */
typedef struct {
   lnk_link_t link;
   pdc_arsess_t arse;
} arsess_elem_t;

#define ASHMAXINC 100

/* only used on IRIX 6 */
static int
get_arsess_list(lnk_link_t *arsess_list)
{
   int num_ashes, i;
   static ash_t *ashes;
   static int ash_max;
   union {
      arsess_t arse;
      arsess64_t arse64;
      arsess65_t arse65;
   } ar;
   static int (*get_arsess_p)(pdc_arsess_t *, arsess_t *);

   if (get_arsess_p == NULL) {
      char irix_release[10];
      sysinfo(SI_RELEASE, irix_release, sizeof(irix_release));
      if (strcmp(irix_release, "6.5")>=0)
         get_arsess_p = &pdc_get_arsess65;
      else if (sysconf(_SC_KERN_POINTERS) == 64)
         get_arsess_p = &pdc_get_arsess64;
      else
         get_arsess_p = &pdc_get_arsess;
   }

   if (ashes == NULL) {
      ash_max = ASHMAXINC;
      ashes = sge_malloc(sizeof(ash_t)*ash_max);
      memset(ashes, 0, sizeof(ash_t)*ash_max);
   }

   LNK_INIT(arsess_list);

   while ((num_ashes = syssgi(SGI_ENUMASHS, ashes, ash_max)) < 0 &&
          errno == ENOMEM) {
      ash_max += ASHMAXINC;
      ashes = (ash_t *)sge_realloc(ashes, sizeof(ash_t)*ash_max, 1);
   }

   if (num_ashes > 0) {
      for (i=0; i<num_ashes; i++) {
         if (syssgi(SGI_GETARSESS, &ashes[i], &ar) >= 0) {
            arsess_elem_t *arse_elem;
            arse_elem = malloc(sizeof(arsess_elem_t));
            memset(arse_elem, 0, sizeof(arsess_elem_t));
            (*get_arsess_p)(&arse_elem->arse, &ar.arse);
            LNK_ADD(arsess_list->prev, &arse_elem->link);
         }
      }
   }

   return num_ashes;
}


/* only used on IRIX 6 */
static void
free_arsess_list(lnk_link_t *arsess_list)
{
   lnk_link_t *curra;
   while((curra=arsess_list->next) != arsess_list) {
      arsess_elem_t *ptr;

      LNK_DELETE(curra);
      ptr = LNK_DATA(curra, arsess_elem_t, link);
      sge_free(&ptr);
   }
}

/* only used on IRIX 6 */
static arsess_elem_t *
find_arsess(lnk_link_t *arsess_list, ash_t ash)
{
   lnk_link_t *curra;
   for(curra=arsess_list->next; curra!=arsess_list; curra=curra->next) {
      arsess_elem_t *arsess_elem = LNK_DATA(curra, arsess_elem_t, link);
      if (arsess_elem->arse.ash == ash)
         return arsess_elem;
   }
   return NULL;
}

/* only used on IRIX 6 */
static int
in_pidlist(pid_t *pidlist, int max, pid_t pid)
{
   int j;
   for (j=0; pidlist[j] && j<max; j++)
      if (pidlist[j] == pid)
         return j+1;
   return 0;
}

#endif /* IRIX */

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
#ifdef IRIX
   lnk_link_t *currp;
#endif

   free_process_list(job_elem);

#ifdef IRIX
   /* free arse list */
   while((currp=job_elem->arses.next) != &job_elem->arses) {
      arsess_elem_t *ptr;
   
      LNK_DELETE(currp);
      ptr = LNK_DATA(currp, arsess_elem_t, link);
      sge_free(&ptr);
   }
#endif

   /* free job element */
   sge_free(&job_elem);
}

static int psRetrieveOSJobData(void) {
   lnk_link_t *curr, *next;
   time_t time_stamp = get_gmt();
   static time_t next_time, pnext_time;

#if defined(IRIX)
   lnk_link_t arsess_list;
   arsess_elem_t *arse_elem;
#endif

   DENTER(TOP_LAYER, "psRetrieveOSJobData");

   if (time_stamp <= next_time) {
      DRETURN(0);
   }
   next_time = time_stamp + ps_config.job_collection_interval;

#if defined(IRIX)

   /* go get all the array sessions */

   get_arsess_list(&arsess_list);

#elif defined(LINUX) || defined(ALPHA) || defined(SOLARIS)
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

#if defined(IRIX)

      if ((arse_elem = find_arsess(&arsess_list, job->jd_jid)) == NULL) {

         job->jd_refcnt = 0;
         job->jd_proccount = 0;
         free_process_list(job_elem);
	 job->jd_utime_c += job->jd_utime_a;
	 job->jd_stime_c += job->jd_stime_a;
	 job->jd_bwtime_c += job->jd_bwtime_a;
	 job->jd_rwtime_c += job->jd_rwtime_a;
	 job->jd_srtime_c += job->jd_srtime_a;
	 job->jd_utime_a = 0;
	 job->jd_stime_a = 0;
	 job->jd_bwtime_a = 0;
	 job->jd_rwtime_a = 0;
	 job->jd_srtime_a = 0;

      } else {
         pid_t pidlist[2048], ses_pidlist[1024];
         int pidmax = sizeof(pidlist)/sizeof(pid_t);
         int ses_pidmax = sizeof(ses_pidlist)/sizeof(pid_t);
         lnk_link_t *curra, *nexta;
         pdc_arsess_t *arse = &arse_elem->arse;
         static int pagesize;

         if (!pagesize)
            pagesize = getpagesize();

         memset(&pidlist, 0, sizeof(pidlist));
         memset(&ses_pidlist, 0, sizeof(ses_pidlist));

         /* get pids in the array session */

         syssgi(SGI_PIDSINASH, &job->jd_jid, &pidlist, pidmax);

         if (job->jd_tstamp == 0) {
            job->jd_uid = -1;
            job->jd_gid = -1;
         }
         job->jd_tstamp = time_stamp;
         job->jd_mem = arse->mem * ((double)pagesize/1024.0/(double)HZ);
         job->jd_chars = arse->chr + arse->chw;
         /* Account ID of this job */
         job->jd_acid = arse->prid;
         /* total user time used (completed processes) */
         job->jd_utime_c = arse->utime*1E-9;
         /* total system time used (completed processes) */
         job->jd_stime_c = arse->stime*1E-9;
         /* total block-io-wait time used (completed processes) */
         job->jd_bwtime_c = arse->bwtime*1E-9;
         /* total raw-io-wait time used (completed processes) */
         job->jd_rwtime_c = arse->rwtime*1E-9;
         /* total srun-wait time used (completed processes) */
         job->jd_srtime_c = arse->qwtime*1E-9;
         /* Elapsed time of the job */
         job->jd_etime = time_stamp - arse->start;
         /* attached process count (from OS) */
         job->jd_refcnt = (long)arse->refcnt;

         /* get pids in the POSIX session */

         syssgi(SGI_GETSESPID, arse_elem->arse.pid, &ses_pidlist, ses_pidmax);

         /* search for any array sessions created in the POSIX session 
            by checking to see if the pid creating the array session
            is one of the POSIX session pids. */
           
         for(curra=arsess_list.next; curra != &arsess_list; curra=nexta) {
            arsess_elem_t *arsess_elem = LNK_DATA(curra, arsess_elem_t, link);
            pdc_arsess_t *arse = &arsess_elem->arse;
            nexta = curra->next;

            if (arse->ash != job->jd_jid &&
                in_pidlist(ses_pidlist, ses_pidmax, arse->pid)) {

               arsess_elem_t *elem;

               /* remove array session element from main array session list
                  and chain it onto the job array session list */

               LNK_DELETE(curra);
               if ((elem=find_arsess(&job_elem->arses, arse->ash))) {
                  LNK_DELETE(&elem->link);
                  sge_free(&elem);
               }
               LNK_ADD(job_elem->arses.prev, &arsess_elem->link);

               /* attached process count (from OS) */
               job->jd_refcnt += arse->refcnt;

            }
         }

	 /* get pids for all of the array sessions associated with the job */

	 for(curra=job_elem->arses.next; curra != &job_elem->arses;
	     curra=curra->next) {

            arsess_elem_t *arsess_elem = LNK_DATA(curra, arsess_elem_t, link);
            pdc_arsess_t *arse = &arsess_elem->arse;
            int j;
         
            /* append pids in this array session to the pidlist */

            for(j=0; pidlist[j] && j<pidmax; j++) ;

            syssgi(SGI_PIDSINASH, &arse->ash, &pidlist[j],
                   pidmax-j);
         }

         /* if it is not time to collect process data then just
            add the process usage times to the job data. */

         if (time_stamp <= pnext_time) {
            lnk_link_t *currp;

            /* initialize active process times */
            job->jd_utime_a = 0;
            job->jd_stime_a = 0;
            job->jd_bwtime_a = 0;
            job->jd_rwtime_a = 0;
            job->jd_srtime_a = 0;

            for(currp=job_elem->procs.next; currp != &job_elem->procs;
                currp=currp->next) {

               proc_elem_t *proc_elem = LNK_DATA(currp, proc_elem_t, link);
               psProc_t *proc = &proc_elem->proc;

               /* Note: if the process interval is larger than the
               job interval, then there is a possibility that the
               usage for a completed job will be counted both in the
               in the active and complete process usage.  We avoid this by
               only adding the process's usage to the job usage if the
               process is in the ASH table active pid list of the job. */

               int j;
               for (j=0; pidlist[j] && j<sizeof(pidlist)/sizeof(pid_t); j++)
               if (pidlist[j] == proc->pd_pid) {

                  /* total user time used (active processes) */
                  job->jd_utime_a += proc->pd_utime;

                  /* total system time used (active processes) */
                  job->jd_stime_a += proc->pd_stime;

                  /* total block-io-wait time used (active processes) */
                  job->jd_bwtime_a += proc_elem->bwtime;

                  /* total raw-io-wait time used (active processes) */
                  job->jd_rwtime_a += proc_elem->rwtime;

                  /* total srun-wait time used (active processes) */
                  job->jd_srtime_a += proc_elem->qwtime;

                  /* add active process memory usage to job */
                  job->jd_mem += proc_elem->mem;

                  /* add active process I/O usage to job */
                  job->jd_chars += proc_elem->chars;
                  break;
               }

            }

         } else {
            proc_elem_t *proc_elem;
            int j, proccount=0;
            lnk_link_t old_procs;

            LNK_INIT(&old_procs);

            /* save old process list */
            if (job_elem->procs.next != &job_elem->procs) {
               old_procs.next = job_elem->procs.next;
               old_procs.prev = job_elem->procs.prev;
               old_procs.next->prev = &old_procs;
               old_procs.prev->next = &old_procs;
               LNK_INIT(&job_elem->procs);
            }

            /* build new process list */
            
            /* initialize active process times */
            job->jd_utime_a = 0;
            job->jd_stime_a = 0;
            job->jd_bwtime_a = 0;
            job->jd_rwtime_a = 0;
            job->jd_srtime_a = 0;
            job->jd_vmem = 0;
            job->jd_rss = 0;

            for (j=0; pidlist[j] && j<sizeof(pidlist)/sizeof(pid_t); j++) {

               if ((proc_elem=(proc_elem_t *)malloc(sizeof(proc_elem_t)))) {

                  prpsinfo_t psinfo;
                  pracinfo_t prinfo;
                  char fname[32];
                  int fd;
                  psProc_t *proc = &proc_elem->proc;
                  
                  memset(proc_elem, 0, sizeof(proc_elem_t));
                  proc->pd_length = sizeof(psProc_t);

                  /* get data from /proc file system */

                  sprintf(fname, "/proc/%05ld", pidlist[j]);
                  fd = open(fname, O_RDONLY);
                  if (fd < 0) continue;

                  if (ioctl(fd, PIOCPSINFO, &psinfo) < 0 ||
                      ioctl(fd, PIOCACINFO, &prinfo) < 0) {
                     close(fd);
                     sge_free(&proc_elem);
                     pidlist[j] = -pidlist[j]; /* force report of old usage */
                     continue;
                  }

                  proc->pd_tstamp = time_stamp;
                  proc->pd_pid = pidlist[j];
                  proc->pd_uid = psinfo.pr_uid;
                  proc->pd_gid = psinfo.pr_gid;
                  if (job->jd_uid == -1) {
                     /* user ID of this job */
                     job->jd_uid = proc->pd_uid;
                     /* group ID of this job */
                     job->jd_gid = proc->pd_gid;
                  }
                  proc->pd_acid = prinfo.pr_prid;
                  proc->pd_state = 1;
                  proc->pd_pstart = psinfo.pr_start.tv_sec +
                        psinfo.pr_start.tv_nsec*1E-9;
                  proc->pd_utime = prinfo.pr_timers.ac_utime*1E-9;
                  proc->pd_stime = prinfo.pr_timers.ac_stime*1E-9;
                  proc_elem->jid = prinfo.pr_ash;
                  proc_elem->bwtime = prinfo.pr_timers.ac_bwtime*1E-9;
                  proc_elem->rwtime = prinfo.pr_timers.ac_rwtime*1E-9;
                  proc_elem->qwtime = prinfo.pr_timers.ac_qwtime*1E-9;
                  proc_elem->mem = prinfo.pr_counts.ac_mem *
                          ((double)pagesize/1024.0/(double)HZ);
                  proc_elem->chars = prinfo.pr_counts.ac_chr +
                        prinfo.pr_counts.ac_chw;
                  proc_elem->vmem = psinfo.pr_size * pagesize;
                  proc_elem->rss = psinfo.pr_rssize * pagesize;

                  job->jd_vmem += proc_elem->vmem;

                  job->jd_rss += proc_elem->rss;

                  /* total user time used (active processes) */
                  job->jd_utime_a += proc->pd_utime;

                  /* total system time used (active processes) */
                  job->jd_stime_a += proc->pd_stime;

                  /* total block-io-wait time used (active processes) */
                  job->jd_bwtime_a += proc_elem->bwtime;

                  /* total raw-io-wait time used (active processes) */
                  job->jd_rwtime_a += proc_elem->rwtime;

                  /* total srun-wait time used (active processes) */
                  job->jd_srtime_a += proc_elem->qwtime;

                  /* add active process memory usage to job */
                  job->jd_mem += proc_elem->mem;

                  /* add active process I/O usage to job */
                  job->jd_chars += proc_elem->chars;

                  close(fd);

                  /* add process element to end of process list */
                  LNK_ADD(job_elem->procs.prev, &proc_elem->link);

                  proccount++;
               }

            }

            job->jd_proccount = proccount;
            job->jd_himem = MAX(job->jd_himem, job->jd_vmem);

            /* free old process list. If one of the old processes is not
               in the new pid list and the old process belongs to a
               different ASH than the main job ASH, then accumulate its
               usage. Also if a process is in the pid list but is deleted
               before we are able to collect its process usage, then report
               its process usage as completed usage. */
            {
               lnk_link_t *currp;
               while((currp=old_procs.next) != &old_procs) {
                  proc_elem_t *tproc_elem = LNK_DATA(currp, proc_elem_t, link);
                  psProc_t *tproc = &tproc_elem->proc;
                  if (!in_pidlist(pidlist, pidmax, tproc->pd_pid)) {
                     if (job->jd_jid != tproc_elem->jid) {
                        job_elem->utime += tproc->pd_utime;
                        job_elem->stime += tproc->pd_stime;
                        job_elem->bwtime += tproc_elem->bwtime;
                        job_elem->rwtime += tproc_elem->rwtime;
                        job_elem->srtime += tproc_elem->qwtime;
			job_elem->mem += tproc_elem->mem;
			job_elem->chars += tproc_elem->chars;
                     } else if (in_pidlist(pidlist, pidmax, -tproc->pd_pid)) {
                        job->jd_utime_c += tproc->pd_utime;
                        job->jd_stime_c += tproc->pd_stime;
                        job->jd_bwtime_c += tproc_elem->bwtime;
                        job->jd_rwtime_c += tproc_elem->rwtime;
                        job->jd_srtime_c += tproc_elem->qwtime;
			job->jd_mem += tproc_elem->mem;
			job->jd_chars += tproc_elem->chars;
                     }
                  }
                  LNK_DELETE(currp);
                  sge_free(&tproc_elem);
               }
            }
         }

         /* add in usage for completed processes from other ASHes */
         job->jd_utime_c += job_elem->utime;
         job->jd_stime_c += job_elem->stime;
         job->jd_bwtime_c += job_elem->bwtime;
         job->jd_rwtime_c += job_elem->rwtime;
         job->jd_srtime_c += job_elem->srtime;
         job->jd_mem += job_elem->mem;
         job->jd_chars += job_elem->chars;

#ifdef notdef
         /* add in memory and I/O usage from other ASHes */
         for(curra=job_elem->arses.next; curra!=&job_elem->arses;
             curra=curra->next) {
            arsess_elem_t *arsess_elem = LNK_DATA(curra, arsess_elem_t, link);
            pdc_arsess_t *arse = &arsess_elem->arse;

            job->jd_mem += arse->mem;
            job->jd_chars += arse->chr + arse->chw;
         }
#endif

      }

#elif defined(ALPHA) || defined(FREEBSD) || defined(LINUX) || defined(SOLARIS) || defined(HP1164) || defined(DARWIN)
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
#if defined(ALPHA) || defined(LINUX)
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

#ifdef IRIX
   free_arsess_list(&arsess_list);
#endif

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

#if defined(ALPHA)
   int start=0;
#endif

   if (initialized)
      return 0;

   initialized = 1;

   LNK_INIT(&job_list);
   start_time = get_gmt();

   /* page size */
   pagesize = getpagesize();

   /* retrieve static parameters */
#if defined(LINUX) || defined(IRIX) || defined(SOLARIS) || defined(DARWIN) || defined(FREEBSD) || defined(NETBSD) || defined(HP1164)
#  ifdef PDC_STANDALONE
   ncpus = sge_nprocs();
#  endif
#elif defined(ALPHA)
   {
#ifdef PDC_STANDALONE
      /* Number of CPUs */
      ncpus = sge_nprocs();
      if (getsysinfo(GSI_PHYSMEM, (caddr_t)&physical_memory,sizeof(int),0,NULL)==-1) {
         return -1;
      }

      unixname[0] = '/';
      if ((getsysinfo(GSI_BOOTEDFILE, &unixname[1],
         sizeof(unixname), NULL, NULL)) <= 0) {
         sge_strlcpy(unixname, _PATH_UNIX, sizeof(unixname));
      }

      if (nlist(unixname, mem_nl) == -1) {
         return -1;
      }
      if (mem_nl[PERFSUM].n_value == 0) {
         return -1;
      }

      if ((kmem_fd = open(_PATH_KMEM,O_RDONLY,0)) == -1) {
         return -1;
      }

#endif  /* PDC_STANDALONE */
   } 

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
#if defined(ALPHA)
   close(kmem_fd);
#endif

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
#if defined(IRIX)
   printf("jd_acid="F64"\n", job->jd_acid);
#endif
   printf("jd_tstamp=%s\n", ctime(&job->jd_tstamp));
   printf("jd_proccount=%d\n", (int)job->jd_proccount);
   printf("jd_refcnt=%d\n", (int)job->jd_refcnt);
   printf("jd_etime=%8.3f\n", job->jd_etime);
   printf("jd_utime_a=%8.3f\n", job->jd_utime_a);
   printf("jd_stime_a=%8.3f\n", job->jd_stime_a);
#if defined(IRIX)
   printf("jd_bwtime_a=%8.3f\n", job->jd_bwtime_a);
   printf("jd_rwtime_a=%8.3f\n", job->jd_rwtime_a);
#endif
   printf("jd_srtime_a=%8.3f\n", job->jd_srtime_a);
   printf("jd_utime_c=%8.3f\n", job->jd_utime_c);
   printf("jd_stime_c=%8.3f\n", job->jd_stime_c);
#if defined(IRIX)
   printf("jd_bwtime_c=%8.3f\n", job->jd_bwtime_c);
   printf("jd_rwtime_c=%8.3f\n", job->jd_rwtime_c);
#endif
   printf("jd_srtime_c=%8.3f\n", job->jd_srtime_c);
#if defined(IRIX)
   printf("jd_mem="F64"\n", job->jd_mem);
#else
   printf("jd_mem=%lu\n", job->jd_mem);
#endif
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
#if defined(IRIX)
   printf("\tpd_acid="F64"\n", proc->pd_acid);
#else
   printf("\tpd_acid=%lu\n", proc->pd_acid);
#endif
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
