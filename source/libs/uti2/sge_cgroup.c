/* sge_cgroup.c -- helpers for resource management with Linux cpusets/cgroups

   Copyright (C) 2012, 2013 Dave Love, University of Liverpool
   Copyright 2012, 2013 Mark Dixon, University of Leeds

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

/* This is to support aspects of Linux cgroups, if they are present
   (Linux 2.6.24+), and otherwise the older cpuset implementation
   (e.g. Red Hat 5, apparently available sometime post-Linux 2.6.9).
   The two cpuset implementations have the same interface, although
   the mount can be done differently with the cgroup emulation.
   [Actually, this has changed -- see below.]  We tend to refer to
   things as "cgroup" even if it's actually the old cpusets.

   There are existing libraries, libcpuset and libcgroup, but it seems
   best not to depend on them, and it's not clear that it's a good
   idea to mix them, given we want to support both the old cpusets and
   cgroups.

   See SLURM, Condor, and possibly OAR for existing cpuset/cgroup
   resource manager implementations.  Using their source isn't
   currently on due to incompatible licensing (SLURM GPL) or
   implementation language (Condor).

   execd and shepherd will try to do cgroup/cpuset management iff they
   run on Linux with suitable support (checked by entries in
   /proc/self) and directories /dev/cpuset/sge and/or /cgroup/sge and
   (currently) if USE_CGROUPS=true is set in the execd_params.  Execd
   maintains per-job/task sub-directories with names in the usual
   $JOB_ID.$SGE_TASK_ID format.  Shepherd creates a directory of that
   named as its pid to be able to identify the child processes.  Thus
   the cpuset structure is
      /dev/cpuset/sge/<job>.<task>/<shepherd pid>
   (SLURM has a user level cpuset under the top level (/slurm), but
   it's not clear why that's needed.)  There is also a "0"
   sub-directory in the task directory which is used for quarantining
   processes at the end of the job.

   The script util/resources/scripts/setup-cgroups-etc can be used
   (e.g. in the execd rc script) to mount the controllers and set up
   the sge directories.  The controller directories are owned by the
   SGE admin user for convenience.  There doesn't currently seem to be
   a need for locking.

   Execd creates a job controller directory, and shepherd will set the
   CPUs of the cpuset consistent with any core binding in force.
   Execd removes the controller after removing itself from the tasks
   and killing any remaining tasks in it (which have presumably escaped
   the process tree).  We don't currently use a release agent.

   Cpuset contents on Red Hat 5:
      cpu_exclusive   memory_pressure          mems
      cpus            memory_pressure_enabled  notify_on_release
      mem_exclusive   memory_spread_page       sched_relax_domain_level
      memory_migrate  memory_spread_slab       tasks

   On Red Hat 6 and Ubuntu 10.04 have extra:  cgroup.procs,
      release_agent, sched_load_balance mem_hardwall.

   Things are rather different in Ubuntu 12.04...

   Todo: Extend the use of cpusets (for process tracking); clean up
   dead cpusets in execd as for active_jobs; investigate use of memory
   pressure etc.; add cgroups.

   Fixme below:  Add doc headers, catalogue messages, tidy up.

   Sigh.  The file structure has changed, e.g. in Ubuntu 12.04.
   There's a default mount on /sys/fs/cgroup/cpuset with cpuset.mems
   and cpuset.cpus instead of mems and cpus.  It doesn't work to mount
   on /dev/cpuset with -onoprefix, which is supposed to make it
   compatible (but presumably doesn't work for a second mount).  We
   need to be prepared for both.  Presumably it's OK to link the
   filesystem to /dev/cpuset, but we probably need to make the mount
   point configurable.  Maybe just grovel /proc/mounts for
   cpuset/cgroup mounts and look for the sge directory in them.
*/

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <libgen.h>
#include <dirent.h>
#include "msg_common.h"
#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_uidgid.h"
#include "uti/sge_unistd.h"
#include "uti/sge_string.h"
#include "sgeobj/sge_conf.h"
#include "uti2/sge_cgroup.h"

#define PID_BSIZE 24            /* enough to hold formatted 64-bit */

#define build_path(buffer, dir, tail)                                   \
   do {                                                                 \
      buffer[0] = '\0';                                                 \
      if (*(dir))                                                       \
         snprintf((buffer), sizeof(buffer), "%s/%s", (dir), (tail));    \
   } while(0)

static bool half_initialized= false;
static bool initialized = false;
/* do we have "cpus" or "cpuset.cpus"? */
static bool cpuset_prefix = false;

/* cgroup names consistent with the cgroup_t enum */
static const char *group_name[cg_num] = {"cpuset",
                                         /* "freezer", "cpuacct", "cpu",
                                            "memory", "devices", "blkio" */
};
static char group_dir[cg_num][SGE_PATH_MAX];

static bool copy_from_parent(char *dir, const char *cfile);
static bool find_cgroup_dir(cgroup_t group, char *dir, size_t ldir);

/* Look for existing cgroups and set up the group_dir array.
   Not MT-safe:  call before thread setup.  */
void
init_cgroups(void) {
   cgroup_t group;

   DENTER(TOP_LAYER, "init_cgroups");
   for (group = (cgroup_t)0; group < cg_num; group++) {
      group_dir[group][0] = 0;
      if (find_cgroup_dir(group, group_dir[group], SGE_PATH_MAX))
         /* special treatment for cpuset */
         if (cg_cpuset == group) {
            char path[SGE_PATH_MAX], shortbuf[10];
            size_t l = sizeof shortbuf;
            half_initialized=true;
            build_path(path, group_dir[group], "cpuset.mems");
            cpuset_prefix = file_exists(path);
            if (!cpuset_prefix)
               build_path(path, group_dir[group], "mems");
            dev_file2string(path, shortbuf, &l);
            if (l <= 1) {       /* get a newline when it's empty */
               WARNING((SGE_EVENT, "Populating "SFN" in " SFN
                        " so cpusets will work",
                        cpuset_prefix ? "cpuset.mems" : "mems",
                        group_dir[group]));
               copy_from_parent (group_dir[group], "mems");
            }
            build_path(path, group_dir[group],
                       cpuset_prefix ? "cpuset.cpus" : "cpus");
            l = sizeof shortbuf;
            dev_file2string(path, shortbuf, &l);
            if (l <= 1) {
               WARNING((SGE_EVENT, "Populating "SFN" in " SFN
                        " so cpusets will work",
                        cpuset_prefix ? "cpuset.cpus" : "cpus",
                        group_dir[group]));
               copy_from_parent (group_dir[group], "cpus");
            }
         }
   }
   initialized = true;
   DRETURN_VOID;
}

char *
cgroup_dir(cgroup_t group)
{
   /*We need to retrieve the cpuset directory part way through the initializtion procedure others can wait*/
   if (group==cg_cpuset?!half_initialized:!initialized) return "";
   return group_dir[group];
}

const char *
cgroup_name(cgroup_t group)
{
   if (!initialized) abort();
   return group_name[group];
}

/* Can we use the controller GROUP, e.g. cpuset?  */
bool
have_cgroup (cgroup_t group)
{
#if ! __linux__
   return false;
#endif
   if (group >= cg_num) abort();
   return *(cgroup_dir(group)) != '\0';
}

/* look for an "sge" directory under a mount point of the cgroup */
static bool
find_cgroup_dir(cgroup_t group, char *dir, size_t ldir)
{
   FILE *fp;
   char path[2048], fstype[64], options[256];
   bool ret = false, is_cpuset = (cg_cpuset == group);

   *dir = '\0';
   if ((fp = fopen("/proc/self/mounts", "r")) == NULL)
      return ret;
   while (fscanf(fp, "%*s %2047s %63s %255s %*d %*d\n", path, fstype, options)
          == 3) {
      if (is_cpuset && strcmp(fstype, "cpuset") == 0) {
         ret = true;
         break;
      } else if ((strcmp(fstype, "cgroup") == 0)) {
         char *s = options, *tok, *save = NULL;
         bool rw = false, got_type = is_cpuset;
         while ((tok = strtok_r(s, ",", &save))) {
            if (strcmp(tok, "rw") == 0) rw = true;
            if (strcmp(tok, group_name[group]) == 0) got_type = true;
            if (rw && got_type) break;
            s = NULL;
         }
         sge_strlcat(path, "/sge", sizeof path);
         ret = rw && got_type && sge_is_directory(path);
         if (ret) break;
      }
   }
   fclose(fp);
   if (ret) sge_strlcpy(dir, path, ldir);
   return ret;
}

/* Return the directory for the given controller GROUP of JOB and TASK
   in buffer DIR of length LDIR (e.g. /dev/cpuset/sge/123.1).  DIR is
   zero-length on failure.  */
bool
get_cgroup_job_dir(cgroup_t group, char *dir, size_t ldir, u_long32 job, u_long32 task)
{
   const char *cdir;

   DENTER(TOP_LAYER, "get_cgroup_job_dir");
   dir[0] = '\0';
   cdir = cgroup_dir(group);
   if (*cdir == '\0') DRETURN(false);
   if (snprintf(dir, ldir, "%s/"sge_u32"."sge_u32, cdir, job, task) >= ldir) {
      WARNING((SGE_EVENT, "Can't build cgroup_job_dir value"));
      DRETURN(false);
   }
   if (!sge_is_directory(dir)) {
      dir[0] = '\0';
      DRETURN(false);
   }
   DRETURN(true);
}

/* Does the controller directory for the job exist?  */
bool
have_cgroup_job_dir(cgroup_t group, u_long32 job, u_long32 task)
{
   char dir[SGE_PATH_MAX];

   if (!get_cgroup_job_dir(group, dir, sizeof dir, job, task))
      return false;
   return *dir && sge_is_directory(dir);
}

/* Write string RECORD to the task's GROUP controller file CFILE with
   MODE copy or append.  For a cpuset, add "cpuset." prefix to CFILE if
   necessary.  */
bool
write_to_shepherd_cgroup(cgroup_t group, const char *cfile, const char *record, u_long32 job, u_long32 task, pid_t pid)
{
   char path[SGE_PATH_MAX], buf[64], *prefix = "";

   if (!get_cgroup_job_dir(group, path, sizeof path, job, task))
      return false;
   if (cg_cpuset == group && cpuset_prefix
       && strncmp("cpuset.", cfile, 7) != 0)
      prefix = "cpuset.";
   snprintf(buf, sizeof buf, "/"pid_t_fmt"/%s%s", pid, prefix, cfile);
   sge_strlcat(path, buf, sizeof path);
   return sge_string2file(record, strlen(record), path) == 0;
}

/* Put the process PID into controller directory DIR.  */
static bool
set_pid_cgroup(pid_t pid, char *dir)
{
   char path[SGE_PATH_MAX], spid[PID_BSIZE];
   int ret;
   bool is_admin, error;

   DENTER(TOP_LAYER, "set_pid_cgroup");
   if (!pid) pid = getpid();
   snprintf(spid, sizeof spid, pid_t_fmt, pid);
   build_path(path, dir, "tasks");
   sge_running_as_admin_user(&error, &is_admin);
   if (error) {
      CRITICAL((SGE_EVENT, "Can't get admin user"));
      abort();
   }
   /* We can't move tasks unless we have permission to signal them,
      and we need sgeadmin permission for the filesystem.  */
   sge_seteuid(SGE_SUPERUSER_UID);
   errno = 0;
   ret = sge_string2file(spid, strlen(spid), path);
   if (ret != 0)
      WARNING((SGE_EVENT, "Can't put task in controller "SFN": "SFN,
               dir, strerror(errno)));
   if (is_admin)
      ret = sge_switch2admin_user();
   else
      ret = sge_switch2start_user();
   if (ret != 0) {
      CRITICAL((SGE_EVENT, "Can't switch user"));
      DEXIT;
      abort();
   }
   DRETURN(ret ? false : true);
}

/* Put process PID into the task's controller GROUP.  */
bool
set_shepherd_cgroup(cgroup_t group, u_long32 job, u_long32 task, pid_t pid)
{
   char dir[SGE_PATH_MAX];

   if (!get_cgroup_job_dir(group, dir, sizeof dir, job, task))
      return false;
   snprintf(dir+strlen(dir), sizeof(dir)-strlen(dir), "/"pid_t_fmt, pid);
   errno = 0;
   return set_pid_cgroup(pid, dir);
}

/* Copy the controller CFILE file contents from the parent into DIR,
   e.g. to populate "cpus".  */
static bool
copy_from_parent(char *dir, const char *cfile)
{
   char parent_path[SGE_PATH_MAX], path[SGE_PATH_MAX], dirx[SGE_PATH_MAX];
   char *prefix = "";

   /* For some reason we're not getting the glibc version of dirname
      that doesn't alter its arg.  */
   sge_strlcpy(dirx, dir, sizeof dirx);
   if (!strncmp(dir, cgroup_dir(cg_cpuset), strlen(cgroup_dir(cg_cpuset)))
       && cpuset_prefix
       && strncmp("cpuset.", cfile, 7) != 0)
      prefix = "cpuset.";
   snprintf(path, sizeof path, "%s/%s%s", dir, prefix, cfile);
   snprintf(parent_path, sizeof parent_path, "%s/%s%s",
            dirname(dirx), prefix, cfile);
   return sge_copy_append(parent_path, path, SGE_MODE_COPY) ? false : true;
}

bool
make_sub_cgroup(cgroup_t group, char *parent, char *child)
{
   char child_dir[SGE_PATH_MAX];

   DENTER(TOP_LAYER, "make_sub_cgroup");
   build_path(child_dir, parent, child);
   if (!sge_is_directory(child_dir)) {
      errno = 0;
      if (mkdir(child_dir, 0755) != 0) {
         WARNING((SGE_EVENT, MSG_FILE_CREATEDIRFAILED_SS, child_dir,
                  strerror(errno)));
         DRETURN(false);
      }
   }
   /* You need to populate the mems and cpus before you can use the
      cpuset -- they're not inherited.
      Checkme: it looks as if clone_children can deal with this
      (present in Ubuntu's Linux 3.2, at least).  */
   if (cg_cpuset == group) {
      DRETURN(copy_from_parent(child_dir, "mems") &&
              copy_from_parent(child_dir, "cpus"));
   }
   DRETURN(true);
}

/* Create cpuset/cgroups directories corresponding to the job task  */
void
make_job_cgroups(u_long32 job, u_long32 task)
{
   char child[64];
   cgroup_t group;

   DENTER(TOP_LAYER, "make_job_cgroups");
   for (group = 0; group < cg_num; group++)
      if (have_cgroup(group)) {
         snprintf(child, sizeof child, sge_u32"."sge_u32, job, task);
         errno = 0;
         if (!make_sub_cgroup(group, cgroup_dir(group), child))
            WARNING((SGE_EVENT, "Can't make cgroup "SFN"/"SFN": "SFN,
                     cgroup_dir(group), child, strerror(errno)));
         if (cg_cpuset == group) {
            char path[SGE_PATH_MAX];
            build_path (path, cgroup_dir(group), child);
            if (!make_sub_cgroup(cg_cpuset, path, "0"))
              WARNING ((SGE_EVENT, "Can't make job "sge_u32"."sge_u32" cpuset 0",
                        job, task));
         }
      }
   DRETURN_VOID;
}

/* Put PROC (string version of pid) into the controller DIR */
static bool
reparent_proc(const char *proc, char *dir)
{
   char path[SGE_PATH_MAX];
   pid_t pid = atoi(proc);

   if (!pid) return false;
   /* Don't fail if the process no longer exists.  */
   build_path(path, "/proc", proc);
   if (!sge_is_directory(path)) return true;
   return set_pid_cgroup(pid, dir);
}

/* Remove the task's controller directory after moving out the
   shepherd and killing anything left.  */
bool
remove_shepherd_cpuset(u_long32 job, u_long32 task, pid_t pid)
{
   char dir[SGE_PATH_MAX], taskfile[SGE_PATH_MAX], spid[PID_BSIZE];
   FILE *fp;
   bool rogue = false;

   DENTER(TOP_LAYER, "remove_shepherd_cpuset");
   snprintf(dir, sizeof dir, "%s/"sge_u32"."sge_u32"/"pid_t_fmt,
            cgroup_dir(cg_cpuset), job, task, pid);
   build_path(taskfile, dir, "tasks");
   /* We should have an empty task list.  If we can't remove it, kill
      anything there.  Arguably this should be repeated in case of a
      race against things spawning if we don't have the freezer cgroup.  */
   errno = 0;
   if (rmdir(dir) == 0) DRETURN(true);
   /* EBUSY means it still has tasks.  */
   if (errno != EBUSY) {
      ERROR((SGE_EVENT, MSG_FILE_RMDIRFAILED_SS, dir, strerror(errno)));
      DRETURN(false);
   }
   if (!(fp = fopen(taskfile, "r"))) {
      WARNING((SGE_EVENT, MSG_FILE_NOOPEN_SS, taskfile, strerror(errno)));
      DRETURN(false);
   }
   while (fgets(spid, sizeof spid, fp)) {
      char buf[MAX_STRING_SIZE], file[SGE_PATH_MAX], *v, *cmd;
      pid_t tgid;
      size_t l;

      replace_char(spid, strlen(spid), '\n', '\0');

      /* Move the task away to avoid waiting for it to die.  */
      /* Fixme:  Keep the cpusetdir tasks open and just write to that.  */
      reparent_proc(spid, cgroup_dir(cg_cpuset));

      /* Kill rogue processes, avoiding the shepherd.  (Shepherd needs
         to be killed exactly once, otherwise sge_reap_children_execd
         is called multiple times.)  Only consider entries in the task
         list that are processes (Tgid == Pid), not threads; this
         copes with old-style cpusets, lacking cgroup.procs.  */
      snprintf(file, sizeof file, "/proc/%s/status", spid);
      v = file_getvalue(buf, MAX_STRING_SIZE, file, "Tgid:");
      if (! v) continue;
      tgid = atoi(v);
      if (strcmp(v, spid)       /* process */
          && (tgid != pid)) {   /* not shepherd */
          if (!rogue)
             WARNING((SGE_EVENT, "rogue process(es) found for task "
    		  sge_u32"."sge_u32, job, task));
          rogue = true;

          /* Extract and log process name */
          snprintf(file, sizeof file, "/proc/%s/cmdline", spid);
          errno = 0;
          l = sizeof buf;
          cmd = dev_file2string(file, buf, &l);
          if (l)
             INFO((SGE_EVENT, "rogue: "SFN2, replace_char(cmd, l, '\0', ' ')));

          sge_switch2start_user();
          kill(tgid, SIGKILL);
          sge_switch2admin_user();
      }
   }
   fclose(fp);
   errno = 0;
   if (rmdir(dir) == 0) DRETURN(true);
   ERROR((SGE_EVENT, MSG_FILE_RMDIRFAILED_SS, dir, strerror(errno)));
   DRETURN(false);
}

/* Remove the job task cpuset directory after first removing shepherd
   sub-directories.  */
bool
remove_job_cpuset(u_long32 job, u_long32 task)
{
#if ! __linux__
   return true;
#else  /* using non-portable dirent-isms */
   char dirpath[SGE_PATH_MAX];
   DIR *dir;
   struct dirent *dent;

   DENTER(TOP_LAYER, "remove_job_cpuset");
   snprintf(dirpath, sizeof dirpath, "%s/"sge_u32"."sge_u32,
            cgroup_dir(cg_cpuset), job, task);
   INFO((SGE_EVENT, "removing task cpuset "SFN, dirpath));
   if (!sge_is_directory(dirpath)) return true;
   errno = 0;
   /* Maybe this should be made reentrant, though there's existing code
      which does the same sort of thing in the same context.  */
   if ((dir = opendir(dirpath)) == NULL) {
      ERROR((SGE_EVENT, MSG_FILE_CANTOPENDIRECTORYX_SS, dirpath,
             strerror(errno)));
      DRETURN(false);
   }
   while ((dent = readdir(dir)))
      if ((DT_DIR == dent->d_type) && sge_strisint(dent->d_name))
         remove_shepherd_cpuset(job, task, atoi(dent->d_name));
   closedir(dir);

   if (rmdir(dirpath) != 0) {
      ERROR((SGE_EVENT, MSG_FILE_RMDIRFAILED_SS, dirpath, strerror(errno)));
      DRETURN(false);
   }
   DRETURN(true);
#endif  /* !__linux__ */
}

/* Move the shepherd tasks to the job cpuset 0 and remove the shepherd
   cpuset.  */
bool
empty_shepherd_cpuset(u_long32 job, u_long32 task, pid_t pid)
{
   char dir[SGE_PATH_MAX], taskfile[SGE_PATH_MAX], taskfile0[SGE_PATH_MAX];
   bool ret;

   snprintf(dir, sizeof dir, "%s/"sge_u32"."sge_u32"/"pid_t_fmt,
            cgroup_dir(cg_cpuset), job, task, pid);
   if (!sge_is_directory(dir)) return true;
   build_path(taskfile, dir, "tasks");
   snprintf(taskfile0, sizeof taskfile0, "%s/"sge_u32"."sge_u32"/0/tasks",
            cgroup_dir(cg_cpuset), job, task);
   errno = 0;
   sge_seteuid(SGE_SUPERUSER_UID);
   /* This could fail at least if a task is respawning.
      It needs to be done task-wise (line-wise) according to cpuset(7).  */
   ret = copy_linewise(taskfile, taskfile0);
   if (sge_switch2admin_user() != 0)
      abort();
   if (!ret) return false;
   /* If rmdir fails, execd will try to kill it. */
   return rmdir(dir) ? false : true;
}
