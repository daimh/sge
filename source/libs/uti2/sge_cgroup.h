#ifndef __SGE_CGROUP_H
#define __SGE_CGROUP_H
#include "basis_types.h"
#include "uti/sge_io.h"
#include "uti2/util.h"

typedef enum {
  /* "cpuset", for instance, clashes with other uses */
  cg_cpuset = 0,
  /*  cg_freezer,
  cg_cpuacct,
  cg_cpu,
  cg_memory,
  cg_devices,
  cg_blkio, */
  cg_num                        /* end marker */
} cgroup_t;

void init_cgroups(void);
char *cgroup_dir(cgroup_t group);
const char *cgroup_name(cgroup_t group);
bool have_cgroup (cgroup_t group);
void make_job_cgroups (u_long32 job, u_long32 task);
void make_shepherd_cgroups (u_long32 job, u_long32 task, pid_t pid);
bool remove_job_cpuset (u_long32 job, u_long32 task);
bool remove_shepherd_cpuset(u_long32 job, u_long32 task, pid_t pid);
bool have_cgroup_job_dir(cgroup_t group, u_long32 job, u_long32 task);
bool write_to_shepherd_cgroup(cgroup_t group, const char *cfile, const char *record, u_long32 job, u_long32 task, pid_t pid);
bool set_shepherd_cgroup(cgroup_t group, u_long32 job, u_long32 task, pid_t pid);
bool empty_shepherd_cpuset(u_long32 job, u_long32 task, pid_t pid);
bool get_cgroup_job_dir(cgroup_t group, char *dir, size_t ldir, u_long32 job, u_long32 task);
bool make_sub_cgroup(cgroup_t group, char *parent, char *child);
#endif  /* __SGE_CGROUP_H */
