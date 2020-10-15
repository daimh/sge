/* SGE GPU load sensor for CUDA/OpenCL devices
Copyright (C) 2012 Dave Love, University of Liverpool

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

In addition, as a special exception, the copyright holder(s) give(s)
permission to link this program with the NVIDIA Management Library
and/or an OpenCL library necessary to interface to the relevant
devices, and distribute the linked executables.  You must obey the GNU
General Public License in all respects for all of the code used other
than those libraries.  If you modify this file, you may extend this
exception to your version of the file, but you are not obligated to do
so.  If you do not wish to do so, delete this exception statement from
your version.
*/

/* 
   SGE needs proper resource management for GPUs and other devices,
   but in the meantime, there's a load sensor roughly like everyone
   has.  It deals with CUDA and (to some extent) OpenCL devices.  See
   "usage" below, e.g. via the --help flag of the built version.

   It uses the nvidia-ml library from the Tesla deployment kit for
   CUDA.  (Originally written against the TDK 1.285 but with
   conditions from testing against a v1.0 library.)  The nvidia-ml
   library is in the driver distribution, inter alia, but the header
   only seems to be in the TDK.  The OpenCL code has only been tested
   against v1.1.

   There are only a fairly small number of items dealt with that are
   likely to be useful for scheduling in the absence of proper
   resource management for the devices.  Via NVML it would easy to
   deal with, say, temperature and ECCs which could be used to put put
   queues into an alarm state, but such monitoring is probably best
   done elsewhere, and where do you stop?  The relevant information
   available from ATI devices currently appears much more limited and
   generally less useful, e.g. not reporting actual device usage.

   In principle this can deal with, say, NVIDIA and ATI devices on the
   same host, but in practice that's problematic from the point of
   view of double counting and possibly from library conflicts.

   Fixme:
   * Maybe add some more values, such as compute mode (need to check
     version dependence of results), and anything else useful from OpenCL
   * Use the AMD GPUPerfAPI to get activity via GPUBusy, at least,
     similar to nvmlDeviceGetUtilizationRates
   * Is there a better way to deal with CUDA plus OpenCL?
*/

#ifndef _XOPEN_SOURCE
  #define _XOPEN_SOURCE 500     /* gethostname */
#endif

#if !HAVE_NVML && !HAVE_OPENCL
  #error "Define HAVE_NVML and/or HAVE_OPENCL"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#if HAVE_NVML
  #include <nvml.h>
#endif

#if HAVE_OPENCL
#include <CL/cl.h>
#define MAX_CL_DEV 10
cl_device_id cl_devices[MAX_CL_DEV];
cl_uint cl_ndevices = 0;
cl_platform_id cl_platform;
#endif

/* It's convenient to share globals amongst CUDA and OpenCL in
   particular.  */
int debug = 0;                  /* Debugging mode */
char host[HOST_NAME_MAX + 1];
unsigned int n_cuda = 0, n_opencl = 0; /* device counts */
char names[1024];                      /* device model name list */

void
usage (int ret, const char *prog)
{
  fprintf (ret ? stderr : stdout,
           "Usage: %s [--help | --debug]\n"
           "Grid Engine load sensor for "
#if HAVE_NVML
  "CUDA"
  #if HAVE_OPENCL
    " and OpenCL"
  #endif
#else
  "OpenCL"
#endif
  " GPUs\n\n", prog);
  fprintf (ret ? stderr : stdout,
           "  -h, --help   Display this help and exit\n"
           "  -d, --debug  Print debugging information to stderr, including\n"
           "               some initial information and messages about library\n"
           "               call failures\n\n");
  if (0 == ret) {
    printf
      ("Possible complex settings:\n"
       "  gpu.ndev            gpu.ndev            INT    <= YES YES 0 0\n"
       "  gpu.names           gpu.names           STRING == YES NO  0 0\n"
#if HAVE_NVML
       "  gpu.ncuda           gpu.ncuda           INT    <= YES YES 0 0\n"
       "  gpu.cuda.0.mem_free gpu.cuda.0.mem_free MEMORY <= YES NO  0 0\n"
       "  gpu.cuda.0.clock    gpu.cuda.0.clock    INT    <= YES NO  0 0\n"
       "  gpu.cuda.0.procs    gpu.cuda.0.procs    INT    <= NO  NO  0 0\n"
       "  gpu.cuda.0.util     gpu.cuda.0.util     INT    <= NO  NO  0 0\n"
#endif
#if HAVE_OPENCL
       "  gpu.nopencl         gpu.nopencl         INT    <= YES YES 0 0\n"
       "  gpu.opencl.0.clock  gpu.opencl.0.clock  INT    <= YES NO  0 0\n"
       "  gpu.opencl.0.mem    gpu.opencl.0.mem    MEMORY <= YES YES 0 0\n"
#endif
       "    ...\n"
       "where:\n"
       "  gpu.ndev is the total number of GPUs on the host\n"
       "  gpu.names is a semi-colon-separated list of GPU model names\n"
#if HAVE_NVML
       "  gpu.ncuda is the number of CUDA GPUs on the host\n"
       "  gpu.cuda.N.mem_free is the free memory on CUDA GPU N\n"
       "  gpu.cuda.N.clock is the maximum clock speed of CUDA GPU N (in MHz)\n"
       "  gpu.cuda.N.procs is the number of processes on CUDA GPU N\n"
       "  gpu.cuda.N.util is the compute utilization of CUDA GPU N (in %%)\n"
#endif
#if HAVE_OPENCL
       "  gpu.nopencl is the number of OpenCL GPUs on the host\n"
       "  gpu.opencl.N.clock is the maximum clock speed of OpenCL GPU N (in MHz)\n"
       "  gpu.opencl.N.mem is the global memory of OpenCL GPU N (in MHz)\n"
#endif
#if HAVE_NVML
       "\nCUDA example output:\n"
       "  begin\n"
       "  comp035:gpu.ndev:2\n"
       "  comp035:gpu.ncuda:2\n"
       "  comp035:gpu.cuda.0.mem_free:4290838528\n"
       "  comp035:gpu.cuda.0.procs:0\n"
       "  comp035:gpu.cuda.0.util:0\n"
       "  comp035:gpu.cuda.1.mem_free:4290838528\n"
       "  comp035:gpu.cuda.1.procs:0\n"
       "  comp035:gpu.cuda.1.util:0\n"
       "  comp035:gpu.names:Tesla T10 Processor;Tesla T10 Processor;\n"
       "  end\n"
#endif
#if HAVE_OPENCL
       "\nOpenCL example output (for device which returns clock speed 0):\n"
       "  begin\n"
       "  comp040:gpu.nopencl:3\n"
       "  comp040:gpu.ndev:3\n"
       "  comp040:gpu.opencl.0.mem:1073741824\n"
       "  comp040:gpu.opencl.1.mem:1073741824\n"
       "  comp040:gpu.opencl.2.mem:1073741824\n"
       "  comp040:gpu.names:ATI RV770;ATI RV770;ATI RV770;\n"
       "  end\n"
#endif
       );
  }
  exit (ret);
}

/* stash the host name we need to print */
void
set_host ()
{
  errno = 0;
  if (gethostname (host, sizeof(host))) {
    if (debug)
      perror ("Can't get host name");
    exit (EXIT_FAILURE);
  }
}

#if HAVE_NVML

#if NVML_API_VERSION == 1
/* Convert return code to string  */
const char*
my_nvmlErrorString (nvmlReturn_t result)
{
  switch (result) {
    /* I'm not sure all the enum symbols are defined in v1.  */
  case 0 /* NVML_SUCCESS */:
    return "The operation was successful";
  case 1 /* NVML_ERROR_UNINITIALIZED */:
    return "NVML was not first initialized with nvmlInit()";
  case 2 /* NVML_ERROR_INVALID_ARGUMENT */:
    return "A supplied argument is invalid";
  case 3 /* NVML_ERROR_NOT_SUPPORTED */:
    return "The requested operation is not available on target device";
  case 4 /* NVML_ERROR_NO_PERMISSION */:
    return "The current user does not have permission for operation";
  case 5 /* NVML_ERROR_ALREADY_INITIALIZED */:
    return "Deprecated: Multiple initializations are now allowed through ref counting";
  case 6 /* NVML_ERROR_NOT_FOUND */:
    return "A query to find an object was unsuccessful";
  case 7  /* NVML_ERROR_INSUFFICIENT_SIZE */:
    return "An input argument is not large enough";
  case 8 /* NVML_ERROR_INSUFFICIENT_POWER */:
    return "A device's external power cables are not properly attached";
  case 9 /* NVML_ERROR_DRIVER_NOT_LOADED */:
    return "NVIDIA driver is not loaded";
  case 10 /* NVML_ERROR_TIMEOUT */:
    return "User provided timeout passed";
  case 999 /* NVML_ERROR_UNKNOWN */:
    return "An internal driver error occurred";
  default:
    return "Unknown error code";
  }
}

#else  /* NVML_API_VERSION != 1 */

/* nvmlSystemGetNVMLVersion, at least, has been seen to return a bogus
   (huge) value, and nvmlErrorString returns null when fed it.  */
const char*
my_nvmlErrorString (nvmlReturn_t result)
{
  const char *res;

  res = nvmlErrorString (result);
  if (res)
    return res;
  else
    return "Unknown error code";
}
#endif

/* Quit if we got an error  */
void
nv_maybe_quit (const char * routine, nvmlReturn_t ret)
{
  if (NVML_SUCCESS == ret)
    return;
  if (debug)
    fprintf (stderr, "NVML error in %s: %s\n", routine, my_nvmlErrorString (ret));
  exit (EXIT_FAILURE);
}

/* Debug print if we got an error */
int
nv_maybe_debug (const char *routine, nvmlReturn_t ret)
{
  if (NVML_SUCCESS == ret)
    return 0;
  if (debug)
    fprintf (stderr, "NVML error from %s: %s\n", routine,
             my_nvmlErrorString (ret));
  return 1;
}
#endif

#if HAVE_OPENCL

const char *
cl_strerr (cl_int ret)
{
  /* These are documented for the routines we call.  */
  switch (ret) {
  case CL_SUCCESS:
    return "success";
  case CL_INVALID_VALUE:
    return "invalid value";
  case CL_OUT_OF_HOST_MEMORY:
    return "can't allocate memory";
  case CL_INVALID_PLATFORM:
    return "invalid platform";
  case CL_INVALID_DEVICE_TYPE:
    return "invalid device type";
  case CL_INVALID_DEVICE:
    return "invalid device";
  case CL_OUT_OF_RESOURCES:
    return "can't allocate resources";
  default:
    return "unknown error";
  }
}

/* Quit if we got an error  */
void
cl_maybe_quit (const char * routine, cl_int ret)
{
  if (CL_SUCCESS == ret)
    return;
  if (debug)
    fprintf (stderr, "OpenCL error in %s: %s\n", routine, cl_strerr (ret));
  exit (EXIT_FAILURE);
}

/* Debug print if we got an error */
int
cl_maybe_debug (const char *routine, cl_int ret)
{
  if (CL_SUCCESS == ret)
    return 0;
  if (debug)
    fprintf (stderr, "OpenCL error from %s: %s\n", routine, cl_strerr (ret));
  return 1;
}
#endif

/* Do any necessary library initialization */
void
gpu_init (void)
{
#if HAVE_NVML
  {
    nv_maybe_quit ("nvmlInit", nvmlInit());
    if (debug) {
      char version[80];
      nvmlReturn_t ret;

#if NVML_API_VERSION >= 2
      if (nv_maybe_debug ("nvmlSystemGetNVMLVersion",
                          nvmlSystemGetNVMLVersion (version, sizeof version))
          == 0)
        fprintf (stderr, "NVML library version: %s\n", version);
#endif
      if (nv_maybe_debug ("nvmlSystemGetDriverVersion",
                          nvmlSystemGetDriverVersion (version, sizeof version))
          == 0)
        fprintf (stderr, "NVML driver version: %s\n", version);
    }
  }
#endif
#if HAVE_OPENCL
  {
    cl_uint nplatforms;

    cl_maybe_debug ("clGetPlatformIDs",
                    clGetPlatformIDs (1, &cl_platform, NULL));
  }
#endif
}

/* Do any necessary shutdown stuff */
void
shutdown (void)
{
#if HAVE_NVML
  nv_maybe_quit ("nvmlShutdown", nvmlShutdown());
#endif
}

/* Deal with device counts */
void
set_n_dev (void)
{
  names[0] = '\0';
#if HAVE_NVML
  {
    nvmlReturn_t ret = nvmlDeviceGetCount (&n_cuda);

    if (ret == NVML_SUCCESS) {
      printf ("%s:gpu.ncuda:%u\n", host, n_cuda);
    } else {
      n_cuda = 0;
      if (debug)
        fprintf (stderr, "nvmlUnitGetCount: %s\n", my_nvmlErrorString (ret));
    }
  }
#endif

#if HAVE_OPENCL
  {
    cl_uint ret = clGetDeviceIDs (cl_platform,
                                  CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR,
                                  MAX_CL_DEV, cl_devices, &cl_ndevices);
    if (ret == CL_SUCCESS) {
      n_opencl = cl_ndevices;
      printf ("%s:gpu.nopencl:%u\n", host, n_opencl);
    } else {
      n_opencl = 0;
      if (debug)
        fprintf (stderr, "clGetDeviceIDs: %s\n", cl_strerr (ret));
    }
  }
#endif
  /* Assume CUDA devices are also OpenCL and don't double count.  */
  if (HAVE_OPENCL)
    printf ("%s:gpu.ndev:%u\n", host, n_opencl);
  else
    printf ("%s:gpu.ndev:%u\n", host, n_cuda);
}

#if HAVE_NVML
/* print loads for CUDA devices */
void
print_nvml (void)
{
  unsigned int i;

  for (i = 0; i < n_cuda; i++) {
    nvmlDevice_t dev;
    char name[80];
    unsigned int uint;
    nvmlMemory_t mem;
    nvmlProcessInfo_t infos[128];
    nvmlUtilization_t utilization;

    nv_maybe_quit ("nvmlDeviceGetHandleByIndex",
                   nvmlDeviceGetHandleByIndex (i, &dev));
    /* Build a semicolon-separated list of device names for pattern
       matching in a resource request.  */
    if (nv_maybe_debug ("nvmlDeviceGetName",
                        nvmlDeviceGetName (dev, name, sizeof (name))) != 0)
      continue;
    snprintf (names + strlen (names), sizeof names - strlen (names),
              "%s;", name);
    if (nv_maybe_debug ("nvmlDeviceGetMemoryInfo",
                        nvmlDeviceGetMemoryInfo (dev, &mem)) == 0) {
      printf ("%s:gpu.cuda.%u.mem_free:%llu\n", host, i, mem.free);
    }
    uint = sizeof (infos);
#if NVML_API_VERSION >= 2
    if (nv_maybe_debug ("nvmlDeviceGetComputeRunningProcesses",
                        nvmlDeviceGetComputeRunningProcesses (dev, &uint,
                                                              infos))
        == 0)
      printf ("%s:gpu.cuda.%u.procs:%u\n", host, i, uint);
#endif
#if NVML_API_VERSION >= 2
    /* Otherwise, we only have the current clock speed, which isn't
       useful for scheduling.  */
    if (nv_maybe_debug ("nvmlDeviceGetMaxClockInfo",
                        nvmlDeviceGetMaxClockInfo (dev, NVML_CLOCK_SM, &uint))
        == 0)
      printf ("%s:gpu.cuda.%u.clock:%u\n", host, i, uint);
#endif
    if (nv_maybe_debug ("nvmlDeviceGetUtilizationRates",
                        nvmlDeviceGetUtilizationRates (dev, &utilization))
        == 0)
      printf ("%s:gpu.cuda.%u.util:%u\n", host, i, utilization.gpu);
  }
}
#endif

#if HAVE_OPENCL
/* print loads for OpenCL devices */
void
print_opencl (void)
{
  unsigned int i;
  char name[80];
  union {
    cl_uint uint;
    char name[80];
    cl_ulong ulong;
  } info;
  size_t info_size;
  char tnam[80];

  for (i = 0; i < n_opencl; i++) {
    cl_device_id dev = cl_devices[i];

    if (cl_maybe_debug ("clGetDeviceInfo",
                        clGetDeviceInfo (dev, CL_DEVICE_NAME,
                                         sizeof info, &info, &info_size))
	== 0) {
      snprintf (tnam, sizeof tnam, "%s;", info.name);
      if (n_cuda == 0 || strcmp (tnam, names) > 0)
	snprintf (names + strlen (names), sizeof names - strlen (names),
		  "%s;", info.name);
    }
    if (cl_maybe_debug ("clGetDeviceInfo",
                        clGetDeviceInfo (dev, CL_DEVICE_MAX_CLOCK_FREQUENCY,
                                         sizeof info, &info, &info_size))
	== 0)
      if (info.uint != 0)       /* is 0 on RV770 test system */
        printf ("%s:gpu.opencl.%u.clock:%u\n", host, i, info.uint);
    if (cl_maybe_debug ("clGetDeviceInfo",
                        clGetDeviceInfo (dev, CL_DEVICE_GLOBAL_MEM_SIZE,
                                         sizeof info, &info, &info_size))
	== 0)
      printf ("%s:gpu.opencl.%u.mem:%lu\n", host, i, info.ulong);
  }
}
#endif

/* command line args */
void
parse_args (int argc, char *argv[])
{
  if (argc == 1)
    return;
  else if (argc > 2)
    usage (1, argv[0]);
  else if ((strcmp (argv[1], "--help") == 0)
           || (strcmp (argv[1], "-h") == 0))
    usage (0, argv[0]);
  else if ((strcmp (argv[1], "--debug") == 0)
           || (strcmp (argv[1], "-d") == 0))
    debug = 1;
  /* fixme:  do --version */
  else
    usage (1, argv[0]);
}

int
main (int argc, char *argv[])
{
  char line[80];

  parse_args (argc, argv);
  set_host ();
  gpu_init ();
  while (fgets (line, sizeof (line), stdin)) {
    if ((strcmp (line, "quit\n") == 0) || (strcmp (line, "quit") == 0))
      break;
    printf("begin\n");
    set_n_dev ();
#if HAVE_NVML
    if (n_cuda > 0)
      print_nvml ();
#endif
#if HAVE_OPENCL
    if (n_opencl > 0)
      print_opencl ();
#endif
    printf ("%s:gpu.names:%s\n", host, names);
    printf("end\n");
    fflush(stdout);
  }
  shutdown ();
  exit (EXIT_SUCCESS);
}
