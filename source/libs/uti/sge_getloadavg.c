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
#include <errno.h>
#include <fcntl.h>  
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_getloadavg.h"
#include "uti/sge_stdlib.h"

#include "sge.h"

#if defined(SOLARIS) 
#  include <nlist.h> 
#  include <sys/cpuvar.h>
#  include <kstat.h> 
#  include <sys/loadavg.h> 
#elif defined(__linux__) || defined(__CYGWIN__)
#  include <ctype.h>
#elif defined(ALPHA4) || defined(ALPHA5)
#  include <nlist.h>
#  include <sys/table.h>
#elif defined(IRIX)
#  include <sys/sysmp.h> 
#  include <sys/sysinfo.h> 
#elif defined(HP11) || defined(HP1164)
#  include <sys/param.h>
#  include <sys/pstat.h>
#elif defined(DARWIN)
# include <mach/host_info.h>
# include <mach/mach_host.h>
# include <mach/mach_init.h>
# include <mach/machine.h>
#elif defined(FREEBSD)
#  if defined(__FreeBSD_version) && __FreeBSD_version < 500101
#     include <sys/dkstat.h>
#  endif
#  include <sys/resource.h>
#  include <fcntl.h>
#  include <kvm.h>
#elif defined(NETBSD)
#  include <sys/sched.h>
#  include <sys/param.h>
#  include <sys/sysctl.h>
#elif defined(AIX51)
#  include <sys/sysinfo.h>
#  include <nlist.h>

#if defined(HAS_AIX5_PERFLIB)
#  include <sys/proc.h>
#  include <libperfstat.h>
#endif

#endif

#define KERNEL_TO_USER_AVG(x) ((double)x/SGE_FSCALE)

#if defined(SOLARIS) 
#  define CPUSTATES 5
#  define CPUSTATE_IOWAIT 3
#  define CPUSTATE_SWAP 4    
#  if SOLARIS64
#     define KERNEL_AVG_TYPE long
#     define KERNEL_AVG_NAME "avenrun"
#  else
#     define SGE_FSCALE FSCALE 
#     define KERNEL_AVG_TYPE long
#     define KERNEL_AVG_NAME "avenrun"
#  endif
#elif defined(__linux__) || defined(__CYGWIN__)
#  define LINUX_LOAD_SOURCE "/proc/loadavg"
#  define CPUSTATES 4
#  define PROCFS "/proc" 
#elif defined(ALPHA4) || defined(ALPHA5)
#  define MP_KERNADDR 8
#  define MPKA_AVENRUN 19
#  define KERNEL_NAME_FILE "/vmunix"
#  define KERNEL_AVG_NAME "_avenrun"
#  define SGE_FSCALE 1000.0
#  define KERNEL_AVG_TYPE long
#  define CPUSTATES 4
#elif defined(IRIX)
#  define SGE_FSCALE 1024.0
#  define KERNEL_AVG_TYPE long
#  define KERNEL_AVG_NAME "avenrun"
#  define CPUSTATES 6
#elif defined(AIX51)
#  define KERNEL_NAME_FILE "/unix"
#  define KERNEL_AVG_NAME "avenrun"
#  define KERNEL_AVG_TYPE long long
#  define CPUSTATES 4 /* CPU_IDLE, CPU_USER, CPU_KERNEL, CPU_WAIT */
#  define SGE_FSCALE 1024.0
#endif

#if defined(FREEBSD)
typedef kvm_t* kernel_fd_type;
#else 
typedef int kernel_fd_type;
#endif

#ifdef SGE_LOADCPU
static long percentages(int cnt, double *out, long *new, long *old, long *diffs);
#endif

#if defined(ALPHA4) || defined(ALPHA5) || defined(HPUX) || defined(IRIX) || defined(__linux__) || defined(DARWIN) || defined(HAS_AIX5_PERFLIB) || defined(__CYGWIN__)

static double get_cpu_load(void);    

#endif

#if __linux__ || __CYGWIN__
static char* skip_token(char *p); 
#endif

#if defined(ALPHA4) || defined(ALPHA5) || defined(IRIX) || defined(HP10) || defined(FREEBSD)

static int sge_get_kernel_fd(kernel_fd_type *kernel_fd);

static int sge_get_kernel_address(char *name, long *address);
 
static int getkval(unsigned long offset, int *ptr, int size, char *refstr);  

#endif 

#if !defined(__linux__) && !defined(SOLARIS) && !defined(__CYGWIN__)
static kernel_fd_type kernel_fd;
#endif

/* MT-NOTE: code basing on kernel_initialized global variable needs not to be MT safe */
static int kernel_initialized = 0;

#if defined(ALPHA4) || defined(ALPHA5) || defined(IRIX) || defined(HP10) || defined(FREEBSD)

static int sge_get_kernel_address(
char *name,
long *address 
) {
   int ret = 0;

   DENTER(TOP_LAYER, "sge_get_kernel_address");

#if defined(IRIX)
   if (!strcmp(KERNEL_AVG_NAME, name)) {
      *address = sysmp(MP_KERNADDR, MPKA_AVENRUN); 
      ret = 1;
   } else {
      *address = 0;
      ret = 0; 
   }
#else
   {
#  if defined(AIX51)
      struct nlist64 kernel_nlist[2];
#  else
      struct nlist kernel_nlist[2];
#  endif

      kernel_nlist[0].n_name = name;
      kernel_nlist[1].n_name = NULL;
#  if defined(ALPHA4) || defined(ALPHA5) || defined(HPUX)
      if (nlist(KERNEL_NAME_FILE, kernel_nlist) >= 0)
#  elif defined(AIX51)
      if (nlist64(KERNEL_NAME_FILE, kernel_nlist) >= 0)
#  else
      if (kernel_initialized && (kvm_nlist(kernel_fd, kernel_nlist) >= 0)) 
#  endif
      {
         *address = kernel_nlist[0].n_value;
         ret = 1;
      } else {
         DPRINTF(("nlist(%s) failed: %s\n", name, strerror(errno)));
         *address = 0;
         ret = 0;
      }
   }
#endif
   DEXIT;
   return ret;
}    


static int sge_get_kernel_fd(
kernel_fd_type *fd 
) {
#if !(defined(IRIX) || defined(HP10) || defined(ALPHA4) || defined(ALPHA5) || defined(AIX51))
   char prefix[256] = "my_error:";
#endif   

   DENTER(TOP_LAYER, "sge_get_kernel_fd");

   if (!kernel_initialized) {

#if defined(IRIX) || defined(HP10) || defined(ALPHA4) || defined(ALPHA5) || defined(AIX51)
      kernel_fd = open("/dev/kmem", 0);
      if (kernel_fd != -1) 
#else 
      kernel_fd = kvm_open(NULL, NULL, NULL, O_RDONLY, prefix);
      if (kernel_fd != NULL) 
#endif
      {
         kernel_initialized = 1;
      } else {
         DPRINTF(("kvm_open() failed: %s\n", strerror(errno)));
         kernel_initialized = 0;
      }
   } 
   *fd = kernel_fd; 
   DEXIT;
   return kernel_initialized;
}

static int getkval(
unsigned long offset, 
int *ptr, 
int size, 
char *refstr 
) {
   kernel_fd_type kernel_fd;

#if defined(FREEBSD)
   if (sge_get_kernel_fd(&kernel_fd)
       && kvm_read(kernel_fd, offset, (char *)ptr, size) != size) {
      if (*refstr == '!') {
         return 0;
      } else {
         return -1;
      }
   }
#else
   if (sge_get_kernel_fd(&kernel_fd)) {
      if (lseek(kernel_fd, (long)offset, 0) == -1) {
         if (*refstr == '!') {
            refstr++;
         }
         return -1;
      }
      if (read(kernel_fd, (char *)ptr, size) == -1) {
         if (*refstr == '!') {
            return 0;
         } else {
            return -1;
         }
      }
   }
#endif
   return 0;
}

#endif

#if defined(SOLARIS)
/* MT-NOTE kstat is not thread save */
int get_freemem(
long *freememp 
) {
   kstat_ctl_t   *kc;  
   kstat_t       *ksp;  
   kstat_named_t *knp;
   kc = kstat_open();  
   ksp = kstat_lookup(kc, "unix", 0, "system_pages");
   if (kstat_read(kc, ksp, NULL) == -1) {
      kstat_close(kc);
      return -1;
   } 
   knp = kstat_data_lookup(ksp, "freemem");
   *freememp = (long)knp -> value.ul;
   kstat_close(kc);
   return 0;
}
#endif 


#if defined(SOLARIS)

static kstat_ctl_t *kc = NULL;
static kstat_t **cpu_ks = NULL;
static cpu_stat_t *cpu_stat = NULL;

#define UPDKCID(nk,ok) \
if (nk == -1) { \
  perror("kstat_read "); \
  return -1; \
} \
if (nk != ok)\
  goto kcid_changed;

int kupdate(int avenrun[3])
{
   kstat_t *ks;
   kid_t nkcid;
   int i;
   int changed = 0;
   static int ncpu = 0;
   static kid_t kcid = 0;
   kstat_named_t *kn;

   /*
   * 0. kstat_open
   */
   if (!kc) {
      kc = kstat_open();
      if (!kc) {
         perror("kstat_open ");
         return -1;
      }
      changed = 1;
      kcid = kc->kc_chain_id;
   }


   /* keep doing it until no more changes */
   kcid_changed:

   /*
   * 1.  kstat_chain_update
   */
   nkcid = kstat_chain_update(kc);
   if (nkcid) {
      /* UPDKCID will abort if nkcid is -1, so no need to check */
      changed = 1;
      kcid = nkcid;
   }
   UPDKCID(nkcid,0);

   ks = kstat_lookup(kc, "unix", 0, "system_misc");
   if (kstat_read(kc, ks, 0) == -1) {
      perror("kstat_read");
      return -1;
   }

#if 0
   /* load average */
   kn = kstat_data_lookup(ks, "avenrun_1min");
   if (kn)
      avenrun[0] = kn->value.ui32;
   kn = kstat_data_lookup(ks, "avenrun_5min");
   if (kn)
      avenrun[1] = kn->value.ui32;
   kn = kstat_data_lookup(ks, "avenrun_15min");
   if (kn)
      avenrun[2] = kn->value.ui32;

   /* nproc */
   kn = kstat_data_lookup(ks, "nproc");
   if (kn) {
      nproc = kn->value.ui32;
#ifdef NO_NPROC
      if (nproc > maxprocs)
      reallocproc(2 * nproc);
#endif
   }
#endif

   if (changed) {
      int ncpus = 0;

      /*
      * 2. get data addresses
      */

      ncpu = 0;

      kn = kstat_data_lookup(ks, "ncpus");
      if (kn && kn->value.ui32 > ncpus) {
         ncpus = kn->value.ui32;
         cpu_ks = (kstat_t **)sge_realloc(cpu_ks, ncpus * sizeof(kstat_t *), 1);
         cpu_stat = (cpu_stat_t *)sge_realloc(cpu_stat, ncpus * sizeof(cpu_stat_t), 1);
      }

      for (ks = kc->kc_chain; ks; ks = ks->ks_next) {
         if (strncmp(ks->ks_name, "cpu_stat", 8) == 0) {
            nkcid = kstat_read(kc, ks, NULL);
            /* if kcid changed, pointer might be invalid */
            UPDKCID(nkcid, kcid);

            cpu_ks[ncpu] = ks;
            ncpu++;
            if (ncpu >= ncpus) {
               break;
            }
         }
      }
      /* note that ncpu could be less than ncpus, but that's okay */
      changed = 0;
   }


   /*
   * 3. get data
   */

   for (i = 0; i < ncpu; i++) {
      nkcid = kstat_read(kc, cpu_ks[i], &cpu_stat[i]);
      /* if kcid changed, pointer might be invalid */
      UPDKCID(nkcid, kcid);
   }

   /* return the number of cpus found */
   return(ncpu);
}

double get_cpu_load(void) {
   int cpus_found, i, j;
   double cpu_load = -1.0;
   static long cpu_time[CPUSTATES] = { 0L, 0L, 0L, 0L, 0L};
   static long cpu_old[CPUSTATES]  = { 0L, 0L, 0L, 0L, 0L};
   static long cpu_diff[CPUSTATES] = { 0L, 0L, 0L, 0L, 0L}; 
   double cpu_states[CPUSTATES];
   int avenrun[3];

   DENTER(CULL_LAYER, "get_cpu_load.solaris");

   /* use kstat to update all processor information */
   if ((cpus_found = kupdate(avenrun)) < 0 ) {
      DEXIT;
      return cpu_load;
   }
   for (i=0; i<CPUSTATES; i++)
      cpu_time[i] = 0;

   for (i = 0; i < cpus_found; i++) {
      /* sum counters up to, but not including, wait state counter */
      for (j = 0; j < CPU_WAIT; j++) {
         cpu_time[j] += (long) cpu_stat[i].cpu_sysinfo.cpu[j];
         DPRINTF(("cpu_time[%d] = %ld (+ %ld)\n", j,  cpu_time[j],
               (long) cpu_stat[i].cpu_sysinfo.cpu[j]));
      }
      /* add in wait state breakdown counters */
      cpu_time[CPUSTATE_IOWAIT] += (long) cpu_stat[i].cpu_sysinfo.wait[W_IO] +
                                   (long) cpu_stat[i].cpu_sysinfo.wait[W_PIO];
      DPRINTF(("cpu_time[%d] = %ld (+ %ld)\n", CPUSTATE_IOWAIT,  cpu_time[CPUSTATE_IOWAIT],
                                (long) cpu_stat[i].cpu_sysinfo.wait[W_IO] +
                                (long) cpu_stat[i].cpu_sysinfo.wait[W_PIO]));


      cpu_time[CPUSTATE_SWAP] += (long) cpu_stat[i].cpu_sysinfo.wait[W_SWAP];
      DPRINTF(("cpu_time[%d] = %ld (+ %ld)\n", CPUSTATE_SWAP,  cpu_time[CPUSTATE_SWAP],
         (long) cpu_stat[i].cpu_sysinfo.wait[W_SWAP]));
   }
   percentages(CPUSTATES, cpu_states, cpu_time, cpu_old, cpu_diff);
   cpu_load = cpu_states[1] + cpu_states[2] + cpu_states[3] + cpu_states[4];
   DPRINTF(("cpu_load %f ( %f %f %f %f )\n", cpu_load,
         cpu_states[1], cpu_states[2], cpu_states[3], cpu_states[4]));
#if 0
#if defined(SOLARIS) && !defined(SOLARIS64)
   DPRINTF(("avenrun(%d %d %d) -> (%f %f %f)\n", avenrun[0], avenrun[1], avenrun[2],
      KERNEL_TO_USER_AVG(avenrun[0]), KERNEL_TO_USER_AVG(avenrun[1]), KERNEL_TO_USER_AVG(avenrun[2])));
#endif
#endif

   DEXIT;
   return cpu_load;
}
#elif defined(__linux__) || defined(__CYGWIN__)

static char* skip_token(char *p) {
   while (isspace(*p)) {
      p++;
   }
   while (*p && !isspace(*p)) {
      p++;
   }
   return p;
}

static double get_cpu_load()
{
   int fd = -1;
   int len, i;
   char buffer[SGE_PATH_MAX];
   char filename[SGE_PATH_MAX];
   char *p;
   double cpu_load;
   static long cpu_new[CPUSTATES];
   static long cpu_old[CPUSTATES];
   static long cpu_diff[CPUSTATES];
   static double cpu_states[CPUSTATES];

   snprintf(filename, sizeof(filename), "%s/stat", PROCFS);
   fd = open(filename, O_RDONLY);
   if (fd == -1) {
      return -1;
   }
   len = read(fd, buffer, sizeof(buffer)-1);
   close(fd);
   buffer[len] = '\0';
   p=skip_token(buffer);
   for (i=0; i<CPUSTATES; i++) {
      cpu_new[i] = strtoul(p, &p, 10);
   }
   percentages(CPUSTATES, cpu_states, cpu_new, cpu_old, cpu_diff);

   cpu_load = cpu_states[0] + cpu_states[1] + cpu_states[2];

   if (cpu_load < 0.0) {
      cpu_load = -1.0;
   }
   return cpu_load;
}                  

#elif defined(ALPHA4) || defined(ALPHA5)

double get_cpu_load() {
   static long cpu_old_ticks[CPUSTATES];
   long cpu_new_ticks[CPUSTATES];
   long cpu_diff_ticks[CPUSTATES];
   double cpu_states[CPUSTATES];
   long delta_ticks;
   double cpu_load;
   struct tbl_sysinfo sys_info;
   int i;

   if (table(TBL_SYSINFO,0, &sys_info, 1, sizeof(struct tbl_sysinfo)) < 0) {
      return -1.0;
   }  
   cpu_new_ticks[0] = sys_info.si_user;
   cpu_new_ticks[1] = sys_info.si_nice;
   cpu_new_ticks[2] = sys_info.si_sys;
   cpu_new_ticks[3] = sys_info.si_idle;
   delta_ticks = 0;
   for (i=0; i<CPUSTATES; i++) {
      cpu_diff_ticks[i] = cpu_new_ticks[i] - cpu_old_ticks[i];
      delta_ticks += cpu_diff_ticks[i];
      cpu_old_ticks[i] = cpu_new_ticks[i]; 
   }
   cpu_load = 0.0;
   if (delta_ticks) {
      for(i=0; i<CPUSTATES; i++) {
         cpu_states[i] = ((double)cpu_diff_ticks[i] / delta_ticks) * 100.0;
      }
   }
   cpu_load += cpu_states[0] + cpu_states[1] + cpu_states[2];
   if (cpu_load < 0.0) {
      cpu_load = -1.0;
   }
   return cpu_load;
}

#elif defined(IRIX)

double get_cpu_load() 
{
   static long cpu_new[CPUSTATES];
   static long cpu_old[CPUSTATES];
   static long cpu_diff[CPUSTATES]; 
   double cpu_states[CPUSTATES];
   double cpu_load;
   struct sysinfo sys_info;
   int i;

   if (sysmp(MP_SAGET, MPSA_SINFO, &sys_info, sizeof(struct sysinfo)) == -1) {
      return -1.0;
   }

   for (i = 0; i < CPUSTATES; i++) {
      cpu_new[i] = sys_info.cpu[i];
   }

   percentages(CPUSTATES, cpu_states, cpu_new, cpu_old, cpu_diff);

   cpu_load = cpu_states[1] + cpu_states[2] + cpu_states[3] 
      + cpu_states[4] + cpu_states[5];
   if (cpu_load < 0.0) {
      cpu_load = -1.0;
   }
   return cpu_load;
}

#elif defined(HP10) || defined(FREEBSD)

static double get_cpu_load()
{
   kernel_fd_type kernel_fd;
   long address = 0;
   static long cpu_time[CPUSTATES];
   static long cpu_old[CPUSTATES];
   static long cpu_diff[CPUSTATES];
   double cpu_states[CPUSTATES];
   double cpu_load;

   if (sge_get_kernel_fd(&kernel_fd)
       && sge_get_kernel_address("cp_time", &address)) {
      getkval(address, (void*)&cpu_time, sizeof(cpu_time), "cp_time"); 
      percentages(CPUSTATES, cpu_states, cpu_time, cpu_old, cpu_diff);
      cpu_load = cpu_states[0] + cpu_states[1] + cpu_states[2];
      if (cpu_load < 0.0) {
         cpu_load = -1.0;
      }
   } else {
      cpu_load = -1.0;
   }
   return cpu_load;
}    

#elif defined(HP11) || defined(HP1164)
static long percentages_new(int cnt, double *out, long *new, long *old, long *diffs, bool first)
{
   int i;
   long total_change = 0;

   DENTER(CULL_LAYER, "percentages_new");

   /* 
    * In the first call of this function, 
    * we will just remember the values for use in the subsequent calls 
    */
   if (first) {
      for (i = 0; i < cnt; i++) {
         *old++ = *new++;
         *out++ = 0.0;
      }
   } else {
      long change;
      long *dp;

      /* initialization */
      total_change = 0;
      dp = diffs;

      /* calculate changes for each state and the overall change */
      for (i = 0; i < cnt; i++) {
         change = *new - *old;

         /* when the counter wraps, we get a negative value */
         if (change < 0) {
            change = (long)
            ((unsigned long)*new-(unsigned long)*old);
         }
         *dp++ = change;
         total_change += change;
         *old++ = *new++;
      }

      /* 
       * If total change is 0, then all the diffs are 0,
       * then the result will be 0.
       */
      if (total_change == 0) {
         for (i = 0; i < cnt; i++) {
            *out++ = 0.0;
         }
      } else {
         /* calculate percentages based on overall change */
         for (i = 0; i < cnt; i++) {
            *out = (*diffs++) * 100.0 / (double)total_change;
            DPRINTF(("diffs: %lu total_change: %lu -> %f",
                  *diffs, total_change, *out));
            out++;
         }
      }
   }

   DRETURN(total_change);
}       


static double get_cpu_load()
{  
   struct pst_processor cpu_buffer;
   struct pst_dynamic dynamic_buffer;
   int ret, i, cpus;
   static long cpu_time[PST_MAX_CPUSTATES];
   static long cpu_old[PST_MAX_CPUSTATES];
   static long cpu_diff[PST_MAX_CPUSTATES];
   double cpu_states[PST_MAX_CPUSTATES];
   double cpu_load;
   static bool first = true;

   ret = pstat_getdynamic(&dynamic_buffer, sizeof(dynamic_buffer), 1, 0);
   if (ret != -1) {
      cpus = dynamic_buffer.psd_max_proc_cnt;
      for (i = 0; i < cpus; i++) {
         ret = pstat_getprocessor(&cpu_buffer, sizeof(cpu_buffer), 1, i);
         if (ret != -1) {
            percentages_new(PST_MAX_CPUSTATES, cpu_states, 
               (long *)cpu_buffer.psp_cpu_time, cpu_old, cpu_diff, first);
            cpu_load = cpu_states[0] + cpu_states[1] + cpu_states[2];
         }
      }
      first = false;
      return cpu_load;
   } else {
      return -1.0;
   }
}

#elif defined(DARWIN)

double get_cpu_load()
{
   static long cpu_new[CPU_STATE_MAX];
   static long cpu_old[CPU_STATE_MAX];
   static long cpu_diff[CPU_STATE_MAX];
   double cpu_states[CPU_STATE_MAX];
   double cpu_load;
   int i;

   kern_return_t error;
   struct host_cpu_load_info cpu_load_data;
   mach_msg_type_number_t host_count = sizeof(cpu_load_data)/sizeof(integer_t);
   mach_port_t host_priv_port = mach_host_self();

   error = host_statistics(host_priv_port, HOST_CPU_LOAD_INFO,
        (host_info_t)&cpu_load_data, &host_count);

   if (error != KERN_SUCCESS) {
      return -1.0;
   }

   for (i = 0; i < CPU_STATE_MAX; i++) {
      cpu_new[i] = cpu_load_data.cpu_ticks[i];
   }

   percentages (CPU_STATE_MAX, cpu_states, cpu_new, cpu_old, cpu_diff);

   cpu_load = cpu_states[CPU_STATE_USER] + cpu_states[CPU_STATE_SYSTEM] + cpu_states[CPU_STATE_NICE];

   if (cpu_load < 0.0) {
      cpu_load = -1.0;
   }
   return cpu_load;
}

#elif defined(NETBSD)

double get_cpu_load()
{
  int mib[2];
  static long cpu_time[CPUSTATES];
  static long cpu_old[CPUSTATES];
  static long cpu_diff[CPUSTATES];
  double cpu_states[CPUSTATES];
  double cpu_load;
  size_t size;

  mib[0] = CTL_KERN;
  #if defined(__OpenBSD__)
  mib[1] = KERN_CPTIME;
  #else
  mib[1] = KERN_CP_TIME;
  #endif

  size = sizeof(cpu_time);

  sysctl(mib, sizeof(mib)/sizeof(int), &cpu_time, &size, NULL, 0);
  percentages(CPUSTATES, cpu_states, cpu_time, cpu_old, cpu_diff);
  cpu_load = cpu_states[0] + cpu_states[1] + cpu_states[2];

  if (cpu_load < 0.0) {
    cpu_load = -1.0;
  }

  return cpu_load;

}

#elif defined(TEST_AIX51)

double get_cpu_load()
{
   static long cpu_time[CPUSTATES];
   static long cpu_old[CPUSTATES];
   static long cpu_diff[CPUSTATES];
   double cpu_states[CPUSTATES];
   double cpu_load;
   struct sysinfo sys_info;
   long address = 0;
   int i;

   if (sge_get_kernel_fd(&kernel_fd)
       && sge_get_kernel_address("sysinfo", &address)) {
      getkval(address, (long*)&cpu_time, sizeof(cpu_time), "sysinfo");
      percentages(CPUSTATES, cpu_states, cpu_time, cpu_old, cpu_diff);

      cpu_load = cpu_states[CPU_USER] + cpu_states[CPU_KERNEL];
      if (cpu_load < 0.0) {
         cpu_load = -1.0;
      }
   } else {
      cpu_load = -1.0;
   }
   return cpu_load;
}
#elif defined(INTERIX)

double get_cpu_load()
{
   /* always return -1 (invalid) to indicate that an
    * external loadsensor has to overwrite the value later.
    */
   return -1;
}
#endif

#if defined(ALPHA4) || defined(ALPHA5) || defined(IRIX) || defined(HP10) || defined(TEST_AIX51)

static int get_load_avg(
double loadavg[],
int nelem 
) {
   kernel_fd_type kernel_fd;
   long address;
   KERNEL_AVG_TYPE avg[3];
   int elements = 0;

   if (sge_get_kernel_fd(&kernel_fd)
       && sge_get_kernel_address(KERNEL_AVG_NAME, &address)) {
      getkval(address, (int*)&avg, sizeof(avg), KERNEL_AVG_NAME);

      while (elements < nelem) {
         loadavg[elements] = KERNEL_TO_USER_AVG(avg[elements]);

         elements++;
      }
   } else {
      elements = -1;
   }
   return elements;
}

#elif defined(HP11) || defined(HP1164)

static int get_load_avg(
double loadavg[],
int nelem 
) {
   struct pst_processor cpu_buffer;
   struct pst_dynamic dynamic_buffer;
   int ret, i, cpus;

   ret = pstat_getdynamic(&dynamic_buffer, sizeof(dynamic_buffer), 1, 0);
   if (ret != -1) {
      cpus = dynamic_buffer.psd_max_proc_cnt;
      loadavg[0] = 0.0;
      loadavg[1] = 0.0;
      loadavg[2] = 0.0;
      for (i = 0; i < cpus; i++) {
         ret = pstat_getprocessor(&cpu_buffer, sizeof(cpu_buffer), 1, i);
         if (ret != -1) {
            loadavg[0] += cpu_buffer.psp_avg_1_min;
            loadavg[1] += cpu_buffer.psp_avg_5_min;
            loadavg[2] += cpu_buffer.psp_avg_15_min;
         }
      }

      loadavg[0] /= (double)cpus;
      loadavg[1] /= (double)cpus;
      loadavg[2] /= (double)cpus;

      return 3;
   } else {
      return -1;
   }
}

#elif defined(HAS_AIX5_PERFLIB)

static int get_load_avg(double loadv[], int nelem)
{
   perfstat_cpu_total_t    cpu_total_buffer;

   perfstat_cpu_total(NULL, &cpu_total_buffer, sizeof(perfstat_cpu_total_t), 1);

   loadv[0] = cpu_total_buffer.loadavg[0]/(float)(1<< SBITS);
   loadv[1] = cpu_total_buffer.loadavg[1]/(float)(1<< SBITS);
   loadv[2] = cpu_total_buffer.loadavg[2]/(float)(1<< SBITS);

   return 0;
}

#elif (!defined(__GLIBC__) && defined(__linux__)) || defined(__CYGWIN__)
/* Currently always see 0 0 0 in /proc/loadavg under Cygwin.  Seems to
   be a fundamental Cygwin problem.  */
static int get_load_avg(
double loadv[],
int nelem 
) {
   char buffer[41];
   int fd, count;

   fd = open(LINUX_LOAD_SOURCE, O_RDONLY);
   if (fd == -1) {
      return -1;
   }
   count = read(fd, buffer, 40);
   buffer[count] = '\0';
   close(fd);
   if (count <= 0) {
      return -1;
   }
   {
      char *l = setlocale (LC_NUMERIC, NULL);
      setlocale (LC_NUMERIC, "C");
      count = sscanf(buffer, "%lf %lf %lf", &(loadv[0]), &loadv[1], &loadv[2]);
      setlocale (LC_NUMERIC, l);
   }
   if (count < 1) {
      return -1;
   }
   return 0;
}

#endif 


int get_channel_fd()
{
   if (kernel_initialized) {
#if defined(SOLARIS) || defined(__linux__) || defined(HP11) || defined(HP1164) || defined(FREEBSD) || defined(__CYGWIN__)
      return -1;
#else
      return kernel_fd;
#endif
   } else {
      return -1;
   }
}    

int sge_getloadavg(double loadavg[], int nelem)
{
   int   elem = 0;   

#if defined(SOLARIS) || defined(FREEBSD) || defined(NETBSD) || defined(DARWIN) || defined(__GLIBC__)
   elem = getloadavg(loadavg, nelem); /* <== library function */
   /* __linux__ here would imply without glibc */
#elif defined(ALPHA5) || defined(IRIX) || defined(HPUX) || defined(__linux__) || defined(HAS_AIX5_PERFLIB) || defined(__CYGWIN__)
   elem = get_load_avg(loadavg, nelem); 
#else
   elem = -2;
#endif
   if (elem >= 0) {
      elem = nelem;  
   }
   return elem;
}

#ifdef SGE_LOADCPU

/****** uti/os/sge_getcpuload() ***********************************************
*  NAME
*     sge_getcpuload() -- Retrieve cpu utilization percentage 
*
*  SYNOPSIS
*     int sge_getcpuload(double *cpu_load) 
*
*  FUNCTION
*     Retrieve cpu utilization percentage (load value "cpu")
*
*  INPUTS
*     double *cpu_load - caller passes adr of double variable 
*                        for cpu load
*
*  RESULT
*     int - error state
*         0 - OK
*        !0 - Error 
******************************************************************************/
int sge_getcpuload(double *cpu_load) 
{
   double load;
   int ret;

   DENTER(TOP_LAYER, "sge_getcpuload");

   if ((load = get_cpu_load()) < 0.0) {
      ret = -1;
   } else {
      *cpu_load = load;
      ret = 0;
   }

   DEXIT;
   return ret;
}

static long percentages(int cnt, double *out, long *new, long *old, long *diffs)
{
   int i;
   long change;
   long total_change;
   long *dp;
   long half_total;

   DENTER(CULL_LAYER, "percentages");

   /* initialization */
   total_change = 0;
   dp = diffs;
   /* calculate changes for each state and the overall change */
   for (i = 0; i < cnt; i++) {
      if ((change = *new - *old) < 0) {
         /* this only happens when the counter wraps */
         change = (int)
         ((unsigned long)*new-(unsigned long)*old);
      }
      total_change += (*dp++ = change);
      *old++ = *new++;
   }
   /* avoid divide by zero potential */
   if (total_change == 0) {
      total_change = 1;
   }
   /* calculate percentages based on overall change, rounding up */
   half_total = total_change / 2l;
   for (i = 0; i < cnt; i++) {
      *out = ((double)((*diffs++ * 1000 + half_total) / total_change))/10;
#if 0
      DPRINTF(("diffs: %lu half_total: %lu total_change: %lu -> %f",
            *diffs, half_total, total_change, *out));
#endif
      out++;
   }

   DEXIT;
   return total_change;
}       

#endif

#ifdef TEST
int main(
int argc,
char *argv[],
char *envp[] 
) {
   int naptime = -1;

   if (argc > 1)
      naptime = atoi(argv[1]);

   while (1) {
      double avg[3];
      int loads;
      double cpu_load;

      errno = 0;             
      loads = sge_getloadavg(avg, 3);
      if (loads == -1) {
         perror("Error getting load average");
         exit(1);
      }
      if (loads > 0)
         printf("load average: %.2f", avg[0]);
      if (loads > 1)
         printf(", %.2f", avg[1]);
      if (loads > 2)
         printf(", %.2f", avg[2]);
      if (loads > 0)
         putchar('\n');

      cpu_load = get_cpu_load();
      if (cpu_load >= 0.0) {
         printf("cpu load: %.2f\n", cpu_load);
      }

      if (naptime == -1) {
         break;
      }
      sleep(naptime);
   }

   exit(0);
}
#endif /* TEST */

