#ifndef __SGE_CGROUP_H
#define __SGE_CGROUP_H
#include <dlfcn.h>

const char *sge_shlib_ext(void);
void *sge_dlopen(const char *libbase, const char *libversion);
#endif
