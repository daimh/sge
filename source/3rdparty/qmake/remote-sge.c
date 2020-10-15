/* Template for the remote job exportation interface to GNU Make.
Copyright (C) 1988, 1989, 1992, 1993, 1996 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Most of this file is actually copyright Sun Microsystems, Inc.
   It seems to date originally from before 2001, with some changes
   since then.  It is distributed under GPL v3 for consistency with
   the current GNU make.  */

#include <errno.h>
#include <string.h>   /* strerror */
#include <sys/types.h>
#include <sys/stat.h> /* file permissions */
#include <sys/time.h>
#include <sys/wait.h> /* waitpid */
#include <fcntl.h>
#include <stdio.h>    /* remove */
#include <stdlib.h>   /* getenv() */
#include <netdb.h>    /* MAXHOSTNAMELEN (should be in sys/param.h) */
#include <sys/param.h>
#include <unistd.h>   /* usleep */
#include <limits.h>   /* PATH_MAX */
#include <signal.h>   /* kill */

/****** Interactive/qmake/--Introduction ***************************************
*
*  NAME
*     qmake -- Scheduled parallel distributed make
*
*  SYNOPSIS
*     qmake <sge options> -- <gmake options>
*
*  FUNCTION
*     Scheduled, parallel, distributed make.
*     qmake is implemented based on GNU make 3.82 using the remote stub
*     mechanism of gmake.
*     qmake will start a parallel job and run make tasks as task in the
*     parallel job using the Grid Engine qrsh -inherit command.
*
*  INPUTS
*     sge options - all options that can be specified with qsub command
*     gmake options  - all possible gmake options
*
*  RESULT
*     returncode from gmake or EXIT_FAILURE, if remote mechanism fails.
*     On error, an appropriate error description is written to stderr.
*
*  EXAMPLE
*     qmake -cwd -v PATH -- -j 5
*     Build sge system: aimk -qmake -parallel 10
*
*  NOTES
*     Not yet internationalized
*     Should be updated to latest gmake version or (better) be contributed
*     to the GNU make project
*
****************************************************************************
*/

/****** Interactive/qmake/-Typedefs ***************************************
*
*  NAME
*     Typedefs -- type definitions
*
*  SYNOPSIS
*     struct job_info {
*        pid_t pid;
*        off_t offset;
*     } job_info;
*
*     struct hostfile_info {
*        off_t free_slots;
*        off_t offset;
*     } hostfile;
*
*     struct finished_job {
*        struct finished_job *next;
*  
*        pid_t pid;
*        int   exit_code;
*        int   signal;
*        int   coredump;
*     } finished_job;
*     
*  FUNCTION
*     struct job_info      - Used to store all child processes a qmake process with 
*                            an offset to the hostfile of the host, on which the 
*                            executed command runs.
*
*     struct hostfile_info - Information about hostfile, is stored at the beginning
*                            of the hostfile.
*                            Contains the number of free slots (used to control
*                            multible parallel recursive makes) and the offset
*                            to the next free host.
*
*     struct finished_job  - Information about finished jobs, that have not
*                            yet been waited for by gmake. 
*                            Linked list, new jobs are appended.
*
****************************************************************************
*/

struct job_info {
   pid_t pid;
   off_t offset;
} job_info;

struct hostfile_info {
   off_t free_slots;
   off_t offset;
} hostfile_info;

struct finished_job {
   struct finished_job *next;

   pid_t pid;
   int   exit_code;
   int   signal;
   int   coredump;
} finished_job;

/****** Interactive/qmake/-Defines ***************************************
*
*  NAME
*     Defines -- constant and macro definitions
*
*  SYNOPSIS
*     #define LOCK_SLEEP_TIME 500
*     #define WAIT_SLOT_TIME    5
*
*  FUNCTION
*     LOCK_SLEEP_TIME - Defines how long qmake should (u)sleep
*                       after an unsuccessful try to get a lock to the 
*                       hostfile
*     WAIT_SLOT_TIME  - Time to wait, if no slot is free in hostfile
*
****************************************************************************
*/

#  define LOCK_SLEEP_TIME 500 /* [ms] */

#define WAIT_SLOT_TIME    5 /* [s]  */


/****** Interactive/qmake/-Global_Variables ************************************
*
*  NAME
*     global Variables -- global Variables used for remote mechanism
*
*  SYNOPSIS
*     int be_verbose;
*     int qrsh_wrapper_cmdline;
*
*     int dynamic_mode;
*
*     char *program_name = 0;
*
*     const char *remote_description = "xxx";
*
*     char *localhost;
*
*     int remote_enabled = 0;
*     
*     int sge_argc = 0;
*     char **sge_argv = NULL;
*     
*     int sge_v_argc = 0;
*     char **sge_v_argv = NULL;
*     
*     int gmake_argc = 0;
*     char **gmake_argv = NULL;
*     
*     int pass_cwd = 0;
*
*     char *lockfile_name = NULL;
*    
*     int   hostfile_locked = 0;
*     int   hostfile = -1;
*     char *hostfile_name = NULL;   
*     char *sge_hostfile_name = NULL;
*     
*     int   host_count = 0;
*     char *jobid = NULL;
*     char *makelevel = NULL;
*
*     struct job_info jobs[];
*     int next_job = 0;
*
*
*  FUNCTION
*     be_verbose           - flag that indicates verbose output of program 
*                            flow etc.
*     qrsh_wrapper_cmdline - the option -v QRSH_WRAPPER[=value] was specified
*                            in the commandline options
*     dynamic_mode         - are tasks started with qrsh -inherit within an 
*                            existing parallel job, or are they submitted as
*                            individual qrsh jobs?
*     program_name         - argv[0] that was used to start the actual process
*     remote_description   - string that is output with --version text from 
*                            gmake
*     localhost            - fully qualified host name of the host, 
*                            where qmake is executing
*     remote_enabled       - flag that indicates, if remote execution of 
*                            commands is enabled
*     sge_argc             - argument counter for sge options
*     sge_argv             - argument vector for sge options
*     sge_v_argc           - argument counter for sge -v options
*     sge_v_argv           - argument vector for sge -v options
*     gmake_argc           - argument counter for gmake options
*     gmake_argv           - argument vector for gmake options
*     pass_cwd             - do we have to pass option -cwd to qrsh for rules?
*     lockfile_name        - name of the qmake lockfile for access of the 
*                            qmake hostfile
*     hostfile_locked      - flag, that tells if we currently owne a lock
*                            to the hostfile
*     hostfile             - filehandle to qmake hostfile
*     hostfile_name        - name of the qmake hostfile
*     sge_hostfile_name    - name of the sge hostfile
*     host_count           - number of hosts resp. slots in hostfile
*     jobid                - environment variable JOBID
*     makelevel            - environment variable MAKELEVEL or ""
*     jobs                 - array with information about running children of 
*                            qmake
*     next_job             - next free position in array jobs
*     saved_status         - first job that has finished and not been waited 
*                            for by gmake
*
****************************************************************************
*/

int be_verbose = 0;
int qrsh_wrapper_cmdline = 0;

int dynamic_mode;

static char *program_name = 0;

const char *remote_description = "distributed make\nload balancing by Grid Engine\n";

static char *localhost;

static int remote_enabled = 0;

static int sge_argc = 0;
static char **sge_argv = NULL;

static int sge_v_argc = 0;
static char **sge_v_argv = NULL;

static int gmake_argc = 0;
static char **gmake_argv = NULL;

static int pass_cwd = 0;

static char *lockfile_name = NULL;

static int   hostfile_locked = 0;
static int   hostfile = -1;
static char *hostfile_name = NULL;   
static char *sge_hostfile_name = NULL;

static int   host_count = 0;

static char *jobid = NULL;
static char *makelevel = NULL;

static struct job_info *jobs;
static int next_job = 0;

struct finished_job *saved_status = NULL;

static int read_remote_status(int *exit_code_ptr, int *signal_ptr, int *coredump_ptr, int block);
static void read_and_save_remote_status(void);

/****** Interactive/qmake/remote_exit() ***************************************
*
*  NAME
*     remote_exit() -- exit qmake 
*
*  SYNOPSIS
*     static void remote_exit(int code, const char *message,
*                                       const char *reason);
*
*  FUNCTION
*     Outputs the error messages passed as parameters to stderr
*     and then exits with the error code passed as parameter.
*
*  INPUTS
*     code    - exit code
*     message - message to output before exit, should describe the 
*               situation when error occurs
*     reason  - description of the error reason, e.g. result from
*               system call strerror(errno)
*
*  RESULT
*     program exits
*
*  EXAMPLE
*     #include <stdlib.h>
*     #include <string.h>
*     #include <errno.h>
*
*     ...
*     
*     if(write(filehandle, buffer, size) != size) {
*        remote_exit(EXIT_FAILURE, "writing to file failed", strerror(errno));
*     }
*
****************************************************************************
*/
static void remote_exit(int code, const char *message, const char *reason)
{
   if(be_verbose) {
      fprintf(stderr, "remote_exit called\n");
   }

   if(message) {
      fprintf(stderr, "%s", message);
   }

   if(reason) {
      fprintf(stderr, ": %s\n", reason);
   }

   fprintf(stderr, "qmake: *** exit triggered from remote module\n");

   exit(code);
}

#if 0
/* debugging code, dump qmake's hostfile */
static void dump_hostfile() {
   char buffer[MAXHOSTNAMELEN];
   char lock;
   struct hostfile_info hostinfo;
   int i;

   lseek(hostfile, 0, SEEK_SET);
   read(hostfile, &hostinfo, sizeof(hostfile_info));
   printf("---------- Hostfile --------------\n");
   printf("\t%d\t%d\n", hostinfo.free_slots, hostinfo.offset);
   for(i = 0; i < host_count; i++) {
      read(hostfile, &lock, sizeof(char));
      read(hostfile, buffer, MAXHOSTNAMELEN);
      printf("\t%d\t%s\n", lock, buffer);
   }
   printf("----------------------------------\n");
}
#endif

/****** Interactive/qmake/read_hostinfo() ***************************************
*
*  NAME
*     read_hostinfo() -- read info record from qmake hostfile
*
*  SYNOPSIS
*     static struct hostfile_info *read_hostinfo();
*
*  FUNCTION
*     reads and returns the hostfile_info record at the beginning of 
*     the qmake hostfile.
*     If an error occurs, the program exits.
*
*  INPUTS
*
*  RESULT
*     Pointer to a static structure hostfile_info containing data from
*     the hostfile.
*
*  EXAMPLE
*
*  NOTES
*     The result points to a static buffer. This buffer will be overwritten
*     on subsequent calls to read_hostinfo().
*     It is in the responsibility of the caller to save values for later use.
*
*  SEE ALSO
*     Interactive/qmake/write_hostinfo()
*
****************************************************************************
*/

static struct hostfile_info *read_hostinfo() 
{
   static struct hostfile_info hostinfo;

   if(lseek(hostfile, 0, SEEK_SET) < 0) {
      remote_exit(EXIT_FAILURE, "unable to position in qmake hostfile", strerror(errno));
   }

   if(read(hostfile, &hostinfo, sizeof(hostfile_info)) != sizeof(hostfile_info)) {
      remote_exit(EXIT_FAILURE, "unable to read from qmake hostfile", strerror(errno));
   }
 
   return &hostinfo;
}
/****** Interactive/qmake/write_hostinfo() ***************************************
*
*  NAME
*     write_hostinfo() -- write info record to qmake hostfile
*
*  SYNOPSIS
*     static void write_hostinfo(const struct hostfile_info *hostinfo);
*
*  FUNCTION
*     Writes the record given with parameter hostinfo to the qmake
*     hostfile.
*     On error, the program will exit.
*
*  INPUTS
*     hostinfo - pointer to structure hostfile_info to write to the hostfile
*
*  SEE ALSO
*     Interactive/qmake/read_hostinfo()
*
****************************************************************************
*/
static void write_hostinfo(const struct hostfile_info *hostinfo)
{
   if(lseek(hostfile, 0, SEEK_SET) < 0) {
      remote_exit(EXIT_FAILURE, "unable to position in qmake hostfile", strerror(errno));
   }
   
   if(write(hostfile, hostinfo, sizeof(hostfile_info)) != sizeof(hostfile_info)) {
      remote_exit(EXIT_FAILURE, "unable to write to qmake hostfile", strerror(errno));
   }
}

/****** Interactive/qmake/get_host_count() ***************************************
*
*  NAME
*     get_host_count() -- get number of entries in qmake hostfile
*
*  SYNOPSIS
*     static int get_host_count();
*
*  FUNCTION
*     Calculates the number of entries in the hostfile.
*     Uses the filehandle "hostfile".
*     If access to the hostfile fails, qmake will exit with an appropriate 
*     error description and error code.
*
*  RESULT
*     host_count - number of entries in hostfile
*
*  NOTES
*     The filehandle "hostfile" (global variable) must have been initialized
*     by a call to open(...) and be a valid qmake hostfile.
*
*  SEE ALSO
*     Interactive/qmake/-Global_Variables
*     Interactive/qmake/init_remote
*     Interactive/qmake/create_hostfile
*
****************************************************************************
*/
static int get_host_count()
{
   struct stat fileinfo;
   int host_count;

   if(fstat(hostfile, &fileinfo) < 0) {
      remote_exit(EXIT_FAILURE, "cannot access fileinfo for qmake hostfile", strerror(errno));
   }

   host_count = (fileinfo.st_size - sizeof(hostfile_info)) / (MAXHOSTNAMELEN + sizeof(char));

   if(be_verbose) {
      fprintf(stdout, "number of slots for qmake execution is %d\n", host_count);
   }

   return host_count;
}


/****** Interactive/qmake/create_hostfile() ***************************************
*
*  NAME
*     create_hostfile -- create special qmake hostfile
*
*  SYNOPSIS
*     static void create_hostfile();
*
*  FUNCTION
*     Creates a qmake hostfile from the sge hostfile created for the
*     requested parallel environment.
*
*     The qmake hostfile is a binary file with the following structure:
*     <info><host1><host2> ... <hostn>
*        <info> is a structure containing <free_slots> and <offset>
*           <free_slots> is the number of still free slots/hosts in 
*                        the hostfile
*           <offset> is the offset to the next host to use
*        <hostn>  describes one host and has the following structure:
*           <lock><hostname>
*              <lock> is one character describing, if the host is in use
*                     (1 means in use, 0 means free)
*              <hostname> is a fixed size character array with the hostname,
*                         padded with 0 bytes
* 
*     If any system call (file access) fails, qmake exits with an appropriate
*     error description and error code.
*
*  NOTES
*     The global variables sge_hostfile_name and hostfile_name must be
*     initialized.
*
*  SEE ALSO
*     Interactive/qmake/-Global_Variables
*     Interactive/qmake/init_remote()
*
****************************************************************************
*/
static void create_hostfile()
{
   FILE* sge_hostfile;
   char line  [MAXHOSTNAMELEN + 20];
   struct hostfile_info hostinfo;

   if(be_verbose) {
      fprintf(stdout, "creating qmake hostfile\n");
   }
   
   /* open qmake hostfile */
   hostfile = open(hostfile_name, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
   if(hostfile < 0) {
      if(errno == EEXIST) {
         return;
      } else {
         remote_exit(EXIT_FAILURE, "cannot open qmake hostfile", strerror(errno));
      }   
   }

   /* open sge hostfile */
   sge_hostfile = fopen(sge_hostfile_name, "r");
   if(sge_hostfile == NULL) {
      fprintf(stderr, "cannot open sge hostfile %s\ndisable remote execution\n", sge_hostfile_name);
      remote_enabled = 0;
      close(hostfile);
      hostfile = -1;
      return;
   }
   
   /* write initial offset */
   hostinfo.free_slots = 0;
   hostinfo.offset     = 0;
   write_hostinfo(&hostinfo);
   
   /* parse sge hostfile and write qmake hostfile */
   while(fgets(line, MAXHOSTNAMELEN + 20, sge_hostfile)) {
      char buffer[MAXHOSTNAMELEN];
      char hostname[MAXHOSTNAMELEN + 1];
      char lock = 0;
      int slots = 0;
      int i;
      
      sscanf(line, "%s %d", hostname, &slots); /* RATS: ignore */
      memset(buffer, 0, MAXHOSTNAMELEN);
      strncpy(buffer, hostname, MAXHOSTNAMELEN - 1);
      hostinfo.free_slots += slots;
      
      for(i = 0; i < slots; i++) {
         if(write(hostfile, &lock, sizeof(char)) != sizeof(char)) {
            remote_exit(EXIT_FAILURE, "cannot write to qmake hostfile", strerror(errno));
         }
         if(write(hostfile, buffer, MAXHOSTNAMELEN) != MAXHOSTNAMELEN) {
            remote_exit(EXIT_FAILURE, "cannot write to qmake hostfile", strerror(errno));
         }
      }
   }

   /* rewrite actual hostinfo */
   write_hostinfo(&hostinfo);

   close(hostfile);
   hostfile = -1;
   fclose(sge_hostfile);
}

/****** Interactive/qmake/init_remote() ***************************************
*
*  NAME
*     init_remote() -- initialize remote execution of jobs
*
*  SYNOPSIS
*     static void init_remote();
*
*  FUNCTION
*     Initializes global variables for remote execution:
*        - lockfile_name
*        - hostfile_name
*        - sge_hostfile_name
*        - host_count
*        - jobs
*
*     Creates qmake hostfile
*
*  NOTES
*     If an error occures, that makes remote execution impossible,
*     qmake exits with an appropriate error message and error code.
*
*  SEE ALSO
*     Interactive/qmake/-Global_Variables
*     Interactive/qmake/create_hostfile()
*
****************************************************************************
*/

static void init_remote()
{
   int i;

   if (!dynamic_mode) {
      char *tmpdir;
      char  buffer[1024];
      char *c;
      
      /* tmpdir from environment */
      tmpdir = getenv("TMPDIR");
      if(tmpdir == NULL) {
         remote_exit(EXIT_FAILURE, "cannot read environment variable TMPDIR", 
                     strerror(errno));
      }
      
      /* lockfile_name */
      snprintf(buffer, sizeof buffer, "%s/qmake_lockfile", tmpdir);
      lockfile_name = strdup(buffer);
      
      /* hostfile_name */
      snprintf(buffer, sizeof buffer, "%s/qmake_hostfile", tmpdir);
      hostfile_name = strdup(buffer);
      
      /* sge_hostfile_name */
      c = getenv("PE_HOSTFILE");
      if(c == NULL) {
         remote_exit(EXIT_FAILURE, "cannot read environment variable "
                     "PE_HOSTFILE", strerror(errno));
      }

      sge_hostfile_name = strdup(c);
     
      if(be_verbose) {
         fprintf(stdout, "sge hostfile = %s\n", sge_hostfile_name);
         fprintf(stdout, "qmake  hostfile = %s\n", hostfile_name);
         fprintf(stdout, "qmake  lockfile = %s\n", lockfile_name);
      }
     
      /* hostfile */
      create_hostfile();

      hostfile = open(hostfile_name, O_RDWR);
      if(hostfile < 0) {
         remote_exit(EXIT_FAILURE, "cannot open qmake hostfile", 
                     strerror(errno));
      }

      /* host_count - it is already initialized from -j option, but make sure
       * it really matches the information from the pe_hostfile
       */
      host_count = get_host_count();
   }

   /* job_info */
   jobs = (struct job_info *)malloc(host_count * sizeof(job_info));
   if(jobs == NULL) {
      remote_exit(EXIT_FAILURE, "malloc failed", NULL);
   }

   for(i = 0; i < host_count; i++) {
      jobs[i].pid     = 0;
      jobs[i].offset  = -1;
   }
}

/****** Interactive/qmake/lock_hostfile() ***************************************
*
*  NAME
*     lock_hostfile() -- get lock on hostfile
*
*  SYNOPSIS
*     static void lock_hostfile();
*
*  FUNCTION
*     Locks acces to the qmake hostfile. The lock is achieved by 
*     creating a lockfile.
*     The name of the lockfile is contained in the global variable
*     lockfile_name, that is set by function init_remote.
*     The function loops until an exclusive create operation can be 
*     performed on the lockfile. Between two loops, qmake sleeps for
*     a timeperiod, that is defined by LOCK_SLEEP_TIME in ms.
*     If an error occurs when creating the lockfile, qmake exits with an 
*     appropriate error message and error code.
*
*  NOTES
*     Probably, a timeout for getting the lock should be introduced to 
*     avoid deadlocks in case of errors.
*
*  SEE ALSO
*     Interactive/qmake/-Defines
*     Interactive/qmake/-Global_Variables
*     Interactive/qmake/init_remote()
*     Interactive/qmake/unlock_hostfile()
*
****************************************************************************
*/

static void lock_hostfile()
{
   int lockfile;
   
   while(1) {
      if((lockfile = open(lockfile_name, O_CREAT | O_EXCL, S_IRUSR)) >= 0) {
         hostfile_locked = 1;
         close(lockfile);
         if(be_verbose) {
            fprintf(stdout, "obtained lock to qmake lockfile\n");
         }
         return;
      }

      if(errno == EEXIST) {
         if(be_verbose) {
            fprintf(stdout, "waiting for lock to qmake lockfile\n");
         }
         usleep(LOCK_SLEEP_TIME); /* fixme: use nanosleep */
      } else {
         remote_exit(EXIT_FAILURE, "unable to access lockfile", strerror(errno));
      }
   }
}

/****** Interactive/qmake/unlock_hostfile() ***************************************
*
*  NAME
*     unlock_hostfile() -- unlock hostfile
*
*  SYNOPSIS
*     static void unlock_hostfile();
*
*  FUNCTION
*     Remove the lock to the qmake hostfile.
*     The lockfile (lockfile_name) is removed.
*
*  SEE ALSO
*     Interactive/qmake/-Global_Variables
*     Interactive/qmake/init_remote()
*     Interactive/qmake/lock_hostfile()
*
****************************************************************************
*/

static void unlock_hostfile()
{
   if(hostfile_locked) {
      hostfile_locked = 0;
      remove(lockfile_name);
      if(be_verbose) {
         fprintf(stdout, "clearing lock to hostfile\n");
      }
   }   
}

/****** Interactive/qmake/unlock_hostentry() ***************************************
*
*  NAME
*     unlock_hostentry() -- unlock one host in hostfile
*
*  SYNOPSIS
*     static void unlock_hostentry(off_t offset);
*
*  FUNCTION
*     Unlock one hostentry in the hostfile. The lock field for the host
*     given by parameter offset is set to 0.
*     If an error occurs in positioning in the hostfile or writing the
*     lockfield, qmake exits with an appropriate error message and
*     error code.
*
*  INPUTS
*     offset - offset to the host to be unlocked. Describes the position
*              of the host in the hostfile, e.g. 2 for the 2nd host.
*
*  SEE ALSO
*     Interactive/qmake/next_host();
*
****************************************************************************
*/

static void unlock_hostentry(off_t offset) {
   char lock = 0;
   struct hostfile_info *hostinfo;
  
   if(offset >= 0) {
      lock_hostfile();
   
      if(be_verbose) {
         fprintf(stdout, "unlock_hostentry %d\n", (int) offset);
      }
   
      if(lseek(hostfile, offset * (MAXHOSTNAMELEN + sizeof(char)) + sizeof(hostfile_info), SEEK_SET) < 0) {
         remote_exit(EXIT_FAILURE, "unable to position in qmake hostfile", strerror(errno));
      }
   
      if(write(hostfile, &lock, sizeof(char)) != sizeof(char)) {
         remote_exit(EXIT_FAILURE, "unable to write to qmake hostfile", strerror(errno));
      }
   
      /* update number of free slots in hostinfo */
      hostinfo = read_hostinfo();
      hostinfo->free_slots++;
      write_hostinfo(hostinfo);
   
      unlock_hostfile();
   }
}

/****** Interactive/qmake/next_host() ***************************************
*
*  NAME
*     next_host -- determine next free remote host
*
*  SYNOPSIS
*     static const char *next_host();
*
*  FUNCTION
*     Determines the next host to be used for a remote operation.
*     Waits until a host is free (info from hostfile_info.free_hosts),
*     during this time, check for finished child processes and store
*     info about them in structures of type finished_job.
*     Then reads the host pointed to by hostfile_info.offset.
*     Checks, if the host is really free, while not,
*        - the next host from the hostfile is read
*        - it is checked, whether this host is still used/locked
*     The found host is locked.
*     The offset of the found host is stored in the jobs array.
*     If an error occurs during file operations, qmake exits with an 
*     appropriate error message and error code.
*
*  RESULT
*     hostname - pointer to character string with name of host to use
*
****************************************************************************
*/

static const char *next_host() 
{
   struct hostfile_info *hostinfo;
   off_t offset;
   off_t file_offset;
   char lock;
   static char buffer[MAXHOSTNAMELEN];

   /* wait until slot free */
   do { 
      /* lock and read hostinfo */
      lock_hostfile();
      hostinfo = read_hostinfo();

      /* if no slot free, unlock to allow others to free slots and wait */
      if(hostinfo->free_slots == 0) {
         unlock_hostfile();

         if(be_verbose) {
            fprintf(stdout, "waiting for a free slot\n");
         }

         sleep(WAIT_SLOT_TIME);
         /* check for dead children */
         read_and_save_remote_status(); 
      } 
   } while(hostinfo->free_slots == 0);
 
   hostinfo->free_slots--;

   /* search free host */
   do {
      /* get offset */
      file_offset = hostinfo->offset * (MAXHOSTNAMELEN + sizeof(char)) + sizeof(hostfile_info);

      /* try to find unlocked host */
      if(lseek(hostfile, file_offset, SEEK_SET) < 0) {
         remote_exit(EXIT_FAILURE, "unable to position in qmake hostfile", strerror(errno));
      }
  
      if(read(hostfile, &lock, sizeof(char)) != sizeof(char)) {
         remote_exit(EXIT_FAILURE, "unable to read from qmake hostfile", strerror(errno));
      }
      offset           = hostinfo->offset;
      hostinfo->offset = (hostinfo->offset + 1) % host_count;
   } while(lock != 0);

   /* lock this host */
   lock = 1;

   if(lseek(hostfile, file_offset, SEEK_SET) < 0) {
      remote_exit(EXIT_FAILURE, "unable to position in qmake hostfile", strerror(errno));
   }

   if(write(hostfile, &lock, sizeof(char)) != sizeof(char)) {
      remote_exit(EXIT_FAILURE, "unable to write to qmake hostfile", strerror(errno));
   }

   /* read hostname */
   if(read(hostfile, buffer, MAXHOSTNAMELEN) != MAXHOSTNAMELEN) {
      remote_exit(EXIT_FAILURE, "unable to read from qmake hostfile", strerror(errno));
   }

   /* rewrite hostinfo */
   write_hostinfo(hostinfo);

   /* unlock */
   unlock_hostfile();

   /* store job_info */
   jobs[next_job].offset = offset;

   if(be_verbose) {
      fprintf(stdout, "next host for qmake job is %s\n", buffer);
   }   

   return buffer;
}

static void build_submit_argv()
{
   int i;
   int v_counter = 0;

   /* copy sge -v options for later calls to qrsh -inherit
    * for dynamic mode, we also have to copy job requests (-l)
    */
   for(i = 0; i < sge_argc; i++) {
      if(strcmp(sge_argv[i], "-V") == 0) {
         v_counter++;
      } else if(strcmp(sge_argv[i], "-v") == 0) {
         v_counter += 2;
         i++;
      } else if (dynamic_mode && strcmp(sge_argv[i], "-l") == 0) {
         v_counter += 2;
         i++;
      }
   }
   sge_v_argc = 0;

   if(v_counter > 0) {
      sge_v_argv = (char **)malloc(v_counter * sizeof(char *));

      if(sge_v_argv == NULL) {
         remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
      }
   }   
   
   for(i = 0; i < sge_argc; i++) {
      if(strcmp(sge_argv[i], "-v") == 0 && (i + 1) < sge_argc) {
         sge_v_argv[sge_v_argc++] = sge_argv[i];
         sge_v_argv[sge_v_argc++] = sge_argv[i + 1];
      } else if(strcmp(sge_argv[i], "-V") == 0) {
         sge_v_argv[sge_v_argc++] = sge_argv[i];
      } else if (dynamic_mode && 
                 strcmp(sge_argv[i], "-l") == 0 && (i + 1) < sge_argc) {
         sge_v_argv[sge_v_argc++] = sge_argv[i];
         sge_v_argv[sge_v_argc++] = sge_argv[i + 1];
      }
   }
}

/****** Interactive/qmake/parse_options() ***************************************
*
*  NAME
*     parse_options() -- split sge and gmake options
*
*  SYNOPSIS
*     static int parse_options(int *p_argc, char **p_argv[]);
*
*  FUNCTION
*     Parses the command line options passed to qmake.
*     If the keyword "-verbose" is found, start verbose reporting of
*     control flow etc.
*     Splits sge and gmake options at the option "--".
*     If memory for the new argument vectors cannot be allocated, qmake
*     exits with an appropriate error message and error code.
*
*  INPUTS
*     p_argc - pointer to argument counter
*     p_argv - pointer to argument vector
*
*  RESULT
*     if everything is OK: 1
*     if no sge options found (no option --): 0 --> standard gmake
*
****************************************************************************
*/

static int parse_options(int *p_argc, char **p_argv[])
{
   int argc;
   char **argv;
   
   int i;
   int first_gmake_option;

   int unhandled_recursive = 0;

   argc = *p_argc;
   argv = *p_argv;
   
   /* detect if there are any sge parameters and count them */
   first_gmake_option = 0;
   for(i = 1; i < argc; i++) {
      if(!strcmp(argv[i], "--")) {
         first_gmake_option = i;
         break;
      }
   }
    
   /* no -- option set to split sge and gmake options? */
   /* if JOBID, assume we are started recursively from any shellscript fragments */
   /* and insert -inherit */

   if(first_gmake_option == 0) {
      if(jobid != NULL) {
         unhandled_recursive = 1;
      } else {
         return 0;
      }
   }
     
   if(unhandled_recursive) {
      /* case: might be recursive make call */
      /* try to copy sge parameters from environment variable */
      char *passed_sge_options = getenv("RECURSIVE_QMAKE_OPTIONS");
      if(passed_sge_options != NULL) {
         /* count number of sge options */
         int counter = 1;
         char *s = passed_sge_options;

         while(*s) {
            if(*s++ == '\n') {
               counter++;
            }
         }
         /* copy options */
         sge_argc = 0;
         sge_argv = (char **)malloc(counter * sizeof(char *));
         
         if(sge_argv == NULL) {
            remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
         }

         sge_argv[sge_argc++] = strtok(passed_sge_options, "\n");
         for(i = 1; i < counter; i++) {
            sge_argv[sge_argc++] = strtok(NULL, "\n");
         }
      }
   } else {  
      /* case: normal qmake call */
      /* copy sge parameters */
      sge_argc = 0;
      sge_argv = (char **)malloc(first_gmake_option * sizeof(char *));

      if(sge_argv == NULL) {
         remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
      }
      
      for(i = 0; i < first_gmake_option; i++) {
         sge_argv[sge_argc++] = argv[i];
      }
   }

   /* determine dynamic_mode */
   dynamic_mode = getenv("PE") == NULL;

   /* parse sge options and set some flags */
   for(i = 0; i < sge_argc; i++) {
      if(strcmp(sge_argv[i], "-verbose") == 0) {
         be_verbose = 1;
      } else if(strcmp(sge_argv[i], "-cwd") == 0) {
         pass_cwd = 1;
      } else if(strcmp(sge_argv[i], "-v") == 0) {
         i++;
         if(i < sge_argc) {
            if(strncmp(sge_argv[i], "QRSH_WRAPPER", 
                       sizeof("QRSH_WRAPPER") - 1) == 0) {
               qrsh_wrapper_cmdline = 1;
            }
         }
      }
   }

   /* copy gmake parameters */
   gmake_argc = 0;
   gmake_argv = (char **)malloc((argc - first_gmake_option) * sizeof(char *));
   
   if(gmake_argv == NULL) {
      remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
   }
   
   gmake_argv[gmake_argc++] = argv[0];
   for(i = first_gmake_option + 1; i < argc; i++, gmake_argc++) {
      gmake_argv[gmake_argc] = argv[i];
   }
  
   /* in case of dynamic mode, set number of slots to use from -j option */
   if (dynamic_mode) {
      for(i = 0; i < gmake_argc; i++) {
         if(strcmp(gmake_argv[i], "-j") == 0) {
            i++;
            /* no further parameter - would core dump? */
            /* JG: TODO: in dynamic mode, -j without parameter would be ok.
             * it would mean: allow any number of parallel jobs.
             * to allow this, we have to change storage of job_info from 
             * array to linked list.
             */
            if(i >= gmake_argc) {
              remote_exit(EXIT_FAILURE, "-j option requires parameter", NULL);
            }
      
            host_count = atoi(gmake_argv[i]);
            break;
         }
      }
     
      if(host_count < 1) {
         host_count = 1;
      }
   }
  
   /* return new gmake parameters */
   *p_argc = gmake_argc;
   *p_argv = gmake_argv;

   if(be_verbose) {
      if (dynamic_mode) {
         fprintf(stdout, "dynamic task allocation mode\n");
      }
      
      for(i = 0; i < sge_argc; i++) {
         fprintf(stdout, "sge_argv[%d] = %s\n", i, sge_argv[i]);
      }
      for(i = 0; i < sge_v_argc; i++) {
         fprintf(stdout, "sge_v_argv[%d] = %s\n", i, sge_v_argv[i]);
      }
      for(i = 0; i < gmake_argc; i++) {
         fprintf(stdout, "gmake_argv[%d]  = %s\n", i, gmake_argv[i]);
      }
   }

   return 1;
}

/****** Interactive/qmake/inherit_job() ***************************************
*
*  NAME
*     inherit_job() -- is qmake option -inherit set?
*
*  SYNOPSIS
*     static int inherit_job();
*
*  FUNCTION
*     Checks if the option "-inherit" is contained in the sge options.
*     If yes, then it is deleted from the sge options.
*
*  RESULT
*     1, if sge options contain "-inherit", else 0
*
****************************************************************************
*/

static int inherit_job() 
{
   int i, j;
   
   for(i = 0; i < sge_argc; i++) {
      if(!strcmp(sge_argv[i], "-inherit")) {
         sge_argc--;
         for(j = i; j < sge_argc; j++) {
            sge_argv[j] = sge_argv[j + 1];
         }
         return 1;
      }   
   }

   return 0;
}

/****** Interactive/qmake/set_default_options() ********************************
*
*  NAME
*     set_default_options() -- initialize remote mechanism before gmake startup
*
*  SYNOPSIS
*     void set_default_options();
*
*  FUNCTION
*     Adds default options to the sge argument vector:
*        - if no resource request is contained, insert resource request for 
*          the architecture (-l arch=$SGE_ARCH)
*     If a system call failes (reading environment, malloc), qmake exits with
*     an appropriate error message and error code.
*
****************************************************************************
*/

void set_default_options()
{
   int i;
   char **argv;
   static char buffer[1024];
   int insert_resource_request = 1;

   /* check if sge options contain resource requests */
   for(i = 0; i < sge_argc; i++) {
      if(!strcmp(sge_argv[i], "-l")) {
         insert_resource_request = 0;
         break;
      }
   }
   
   if(insert_resource_request) {
      char *architecture;
      /* determine architecture */
      architecture = getenv("SGE_ARCH");
      if(architecture == NULL || strlen(architecture) == 0) {
         fprintf(stdout, "qmake: *** cannot determine architecture from environment variable SGE_ARCH\n");
         fprintf(stdout, "           no default architecture set\n");
         return;
      }

      /* if no resource requests, insert to use same architecture */
      /* copy old sge options */
      argv = sge_argv;
      sge_argv = (char **)malloc((sge_argc +  3) * sizeof(char *));

      if(sge_argv == NULL) {
         remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
      }

      /* copy existing options */
      for(i = 0; i < sge_argc; i++) {
         sge_argv[i] = argv[i];
      }

      /* append architecture */
      if(insert_resource_request) {
         snprintf(buffer, sizeof buffer, "arch=%s", architecture);
   
         if(be_verbose) {
            fprintf(stdout, "setting default options: -l %s\n", buffer);
         }
   
         sge_argv[sge_argc++] = "-l";
         sge_argv[sge_argc++] = buffer;
      }
   
      /* free old sge_argv */
      free(argv);
   }
}



/****** Interactive/qmake/equalize_nslots() ************************************
*
*  NAME
*     equalize_nslots() -- equalize -j option with NSLOTS environment
*
*  SYNOPSIS
*     static void equalize_nslots(int *p_argc, char **p_argv[]);
*
*  FUNCTION
*     Reads the number of slots from environment variable NSLOTS,
*     if -j option is not set or differs from NSLOTS, it is 
*     inserted/corrected.
*     If a system call fails (reading environment, malloc), qmake exits
*     with an appropriate error message and error code.
*
*  INPUTS
*     p_argc - pointer to argument counter
*     p_argv - pointer to argument vector
*
****************************************************************************
*/
static void equalize_nslots(int *p_argc, char **p_argv[])
{
   int i;
   char *nslots;
   char **argv;

   /* get NSLOTS from environment */
   nslots = getenv("NSLOTS");
   if(nslots == NULL) {
      remote_exit(EXIT_FAILURE, "NSLOTS not set in environment", strerror(errno));
   }

   /* if -j option differs, set NSLOTS as -j option */
   for(i = 0; i < gmake_argc; i++) {
      if(!strcmp(gmake_argv[i], "-j")) {
         i++;
         /* no further parameter - would core dump? */
         if(i >= gmake_argc) {
           remote_exit(EXIT_FAILURE, "-j option requires parameter", NULL);
         }
         /* NSLOTS differs from -j parameter? */
         if(strcmp(gmake_argv[i], nslots)) {
            if(be_verbose) {
               fprintf(stdout, "equalizing -j option with NSLOTS environment: -j %s\n", nslots);
            }
            gmake_argv[i] = nslots;
            return;
         } else {
            return;
         }   
      }
   }

   if(be_verbose) {
      fprintf(stdout, "inserting -j option from NSLOTS environment: -j %s\n", nslots);
   }

   /* no -j option set */
   /* copy old gmake options */
   argv = gmake_argv;
   gmake_argv = (char **)malloc((gmake_argc + 2) * sizeof(char *));

   if(gmake_argv == NULL) {
      remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
   }

   for(i = 0; i < gmake_argc; i++) {
      gmake_argv[i] = argv[i];
   }

   /* append architecture */
   gmake_argv[gmake_argc++] = "-j";
   gmake_argv[gmake_argc++] = nslots;

   /* free old gmake_argv */
   free(argv);

   /* pass new options to gmake */
   *p_argc = gmake_argc;
   *p_argv = gmake_argv;
}

/****** Interactive/qmake/equalize_pe_j() ***************************************
*
*  NAME
*     equalize_pe_j() -- equalize no slots from -pe and -j option
*
*  SYNOPSIS
*     static void equalize_pe_j();
*
*  FUNCTION
*     If no parallel environment is requested in the sge options
*     and more than 1 slot is requested by the gmake -j option, 
*     a request for a parallel environment "make" with the a range
*     of slots from 1 to the value given with the -j option is
*     inserted into the sge options.
*     If an error occurs (invalid -j option, malloc) qmake exits with
*     an appropriate error message and error code.
*
****************************************************************************
*/
#if 0
static void equalize_pe_j()
{
   int i;
   int nslots = 0;
   char **argv;
   static char buffer[100];

   /* -pe sge option set? Then take this one */
   for(i = 0; i < sge_argc; i++) {
      if(!strcmp(sge_argv[i], "-pe")) {
         return;
      }   
   }
  
   /* gmake option -j requests more than 1 slot? */
   for(i = 0; i < gmake_argc; i++) {
      if(!strcmp(gmake_argv[i], "-j")) {
         i++;
         /* no further parameter - would core dump? */
         if(i >= gmake_argc) {
           remote_exit(EXIT_FAILURE, "-j option requires parameter", NULL);
         }
   
         nslots = atoi(gmake_argv[i]);
         break;
      }
   }
  
   if(nslots < 1) {
      nslots = 1;
   }

   if(be_verbose) {
      fprintf(stdout, "inserting pe request to sge options: -pe make 1-%d\n", nslots);
   }
   
   /* insert pe into sge options */
   /* copy old sge options */
   argv = sge_argv;
   sge_argv = (char **)malloc((sge_argc + 3) * sizeof(char *));

   if(sge_argv == NULL) {
      remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
   }

   for(i = 0; i < sge_argc; i++) {
      sge_argv[i] = argv[i];
   }

   /* append pe */
   snprintf(buffer, sizeof buffer, "1-%d", nslots);
   sge_argv[sge_argc++] = "-pe";
   sge_argv[sge_argc++] = "make";
   sge_argv[sge_argc++] = buffer;

   /* free old sge_argv */
   free(argv);
}
#endif   


/****** Interactive/qmake/submit_qmake() ***************************************
*
*  NAME
*     submit_qmake() -- start a scheduled qmake with qrsh
*
*  SYNOPSIS
*     static void submit_qmake();
*
*  FUNCTION
*     Builds a new argument vector for a qrsh call to start a scheduled
*     qmake. 
*     Inserts the option -inherit into the argument vector to signal 
*     qmake that it is called from qrsh or from within a batch script.
*     Pass option -verbose to qrsh and the scheduled qmake.
*     qrsh is called by forking and exec to qrsh.
*     The parent process waits for qrsh to exit and then exits with 
*     the exit status from qrsh or EXIT_FAILURE, if qrsh exited because
*     it was signaled.
*
****************************************************************************
*/
static void submit_qmake()
{
   int i;
   int argc;
   char **argv;
   int qrsh_pid;
   int insert_qrsh_wrapper = 0;

   /* do we have to pass QRSH_WRAPPER from environment? */
   if(!qrsh_wrapper_cmdline && getenv("QRSH_WRAPPER") != NULL) {
      insert_qrsh_wrapper = 1;
   }

   /* build argv for qrsh */
   argc = 0;
   argv = (char **)malloc((sge_argc + sge_v_argc + gmake_argc + 4 + (be_verbose ? 1 : 0) + (insert_qrsh_wrapper ? 2 : 0) + pass_cwd) * sizeof(char *));

   if(argv == NULL) {
      remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
   }

   argv[argc++] = "qrsh";

   argv[argc++] = "-noshell";
   
   for(i = 1; i < sge_argc; i++) {
      argv[argc++] = sge_argv[i];
   }
   
   argv[argc++] = gmake_argv[0];
   
   argv[argc++] = "-inherit";

   if(be_verbose) {
      argv[argc++] = "-verbose";
   }

   if(pass_cwd) {
      argv[argc++] = "-cwd";
   }

   if(insert_qrsh_wrapper) {
      argv[argc++] = "-v";
      argv[argc++] = "QRSH_WRAPPER";
   }

   for(i = 0; i < sge_v_argc; i++) {
      argv[argc++] = sge_v_argv[i];
   }

   argv[argc++] = "--";
   for(i = 1; i < gmake_argc; i++) {
      argv[argc++] = gmake_argv[i];
   }

   argv[argc] = NULL;
  
   /* build subprocess */
   qrsh_pid = fork();

   if(qrsh_pid < 0) {
      remote_exit(EXIT_FAILURE, "unable to create qrsh process", strerror(errno));
   }
   
   if(qrsh_pid) {
      /* in parent, wait for child to exit */
      while(1) {
         int status;
         
         if(waitpid(qrsh_pid, &status, 0) == qrsh_pid) {
            if(WIFEXITED(status)) {
               exit(WEXITSTATUS(status));
            }

            if(WIFSIGNALED(status)) {
               char buffer[1024];
               snprintf(buffer, sizeof buffer, "qrsh exited on signal %d",
                        WTERMSIG(status));
               remote_exit(EXIT_FAILURE, buffer, NULL);
            }
         }   
      }
   } else {
      /* in child, start qrsh */
      if(be_verbose) {
         fprintf(stdout, "creating scheduled qmake\n");
         for(i = 0; i < argc; i++) {
            fprintf(stdout, "argv[%3d] = %s\n", i, argv[i]);
         }
      }
      
      execvp("qrsh", argv);
      remote_exit(EXIT_FAILURE, "start of qrsh failed", strerror(errno));
   }
}


/****** Interactive/qmake/remote_options() *************************************
*
*  NAME
*     remote_options() -- initialize remote mechanism before gmake startup
*
*  SYNOPSIS
*     void remote_options(int *p_argc, char **p_argv[]);
*
*  FUNCTION
*     Determine the qmake startmode and create an appropriate program
*     flow.
*     The following start modes are defined:
*        - qmake called interactively without any special options
*        - qmake called interactively with special options
*        - qmake called from sge (qrsh or batch job)
*        - qmake called recursively from scheduled qmake
*
*  INPUTS
*     p_argc - pointer to argument counter
*     p_argv - pointer to argument vector
*
****************************************************************************
*/

void remote_options(int *p_argc, char **p_argv[])
{
   jobid     = getenv("JOB_ID");
   makelevel = getenv("MAKELEVEL");

   /* store program name to detect recursive make calls */
   program_name = strdup((*p_argv)[0]);
   if(program_name == 0) {
      remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
   }
 
   /* split sge and gmake options */
   if(!parse_options(p_argc, p_argv)) {
      /* no sge options set: behave as gmake */
      return;
   }

   if(be_verbose) {
      fprintf(stdout, "determine qmake startmode\n");
   }
   
   /* option -inherit set? */
   if(inherit_job()) {
      /* is MAKELEVEL set in environment? */
      if(makelevel == NULL) {
         makelevel = "1";

         /* in non dynamic mode, equalize NSLOTS (from pe) with -j option */
         if (!dynamic_mode) {
            equalize_nslots(p_argc, p_argv);
         }
      } 
      /* enable remote execution */
      remote_enabled = 1;
      build_submit_argv();
   } else {
      /* set default sge options (architecture) */
      set_default_options();
      build_submit_argv();
      /* equalize -pe and -j options */
      /* JG: If no pe is given, we use dynamic allocation */
#if 0
      equalize_pe_j();
#endif
      /* start a scheduled qmake with qrsh, wait for qrsh to exit and exit */
      submit_qmake();
   }
}

/****** Interactive/qmake/remote_setup() ***************************************
*
*  NAME
*     remote_setup() -- setup remote mechanism after gmake startup
*
*  SYNOPSIS
*     void remote_setup();
*
*  FUNCTION
*     If remote execution is enabled, initialize the remote mechanisms.
*     Calls init_remote and
*     initializes the global variables
*        - localhost
*     If an error occurs in a system call (gethostname, gethostbyname, malloc)
*     qmake exits with an appropriate error message and error code.
*
*  NOTES
*     Initialization of localhost should be moved to init_remote.
*
*  SEE ALSO
*     Interactive/qmake/init_remote()
*     Interactive/qmake/remote_cleanup()
*
****************************************************************************
*/

void remote_setup ()
{
   static char hostbuffer[MAXHOSTNAMELEN];
   struct hostent *hostinfo;
   
   /* if remote enabled, initialize filenames, hostfile, filehandles, number of hosts  */
   if(remote_enabled) {
      init_remote();

      if(gethostname(hostbuffer, sizeof hostbuffer) != 0) {
         remote_exit(EXIT_FAILURE, "gethostname failed", strerror(errno));
      }
   
      hostinfo = gethostbyname(hostbuffer);
      if(hostinfo == NULL) {
         remote_exit(EXIT_FAILURE, "gethostbyname failed", strerror(errno));
      }

      localhost = strdup(hostinfo->h_name);
      if(localhost == NULL) {
         remote_exit(EXIT_FAILURE, "malloc failed", strerror(errno));
      }
   }
}

/****** Interactive/qmake/remote_cleanup() *************************************
*
*  NAME
*     remote_cleanup() -- cleanup remote mechanism before exit
*
*  SYNOPSIS
*     void remote_cleanup();
*
*  FUNCTION
*     Cleans up some files ...
*        - hostfile
*        - lockfile
*
*  NOTES
*     Probably not complete.
*
*  SEE ALSO
*     Interactive/qmake/remote_setup()
*
****************************************************************************
*/

void remote_cleanup ()
{
   /* if remote start is enabled */
   if(remote_enabled) {
      if(be_verbose) {
         fprintf(stdout, "cleanup of remote mechanism\n");
      }

      if (!dynamic_mode) {
         /* close hostfile */
         if(hostfile >= 0) {
            close(hostfile);
            hostfile = -1;
         }
         /* unlock, if exit is forced within locked situation */
         unlock_hostfile();
      }
   }
}
/* Return nonzero if the next job should be done remotely.  */

/****** Interactive/qmake/start_remote_job_p() *********************************
*
*  NAME
*     start_remote_job_p() -- shall next job be started remote?
*
*  SYNOPSIS
*     int start_remote_job_p(int first_p);
*
*  FUNCTION
*     If remote execution is enabled, prepare job_info record for next
*     job and return true (1).
*
*  INPUTS
*     first_p - no idea what this parameter is for :-( 
*
*  RESULT
*     start_remote - 0 if next job shall be started locally,
*                    1 if it may be started remotely
*
*  NOTES
*     Probably determination of next host to use and the decision, if this
*     host is really the local host (-> local execution) should be done here.
*
*  SEE ALSO
*     Interactive/qmake/start_remote_job
*
****************************************************************************
*/

int start_remote_job_p (int first_p)
{
   /* if remote is enabled, always return true */
   if(remote_enabled) {
      if (!dynamic_mode) {
         int i;
         
         /* set pointer to next free entry in job_info */
         next_job = -1;
         for(i = 0; i < host_count; i++) {
            if(jobs[i].pid == 0) {
               next_job = i;
               break;
            }
         }

         if(next_job == -1) {
            remote_exit(EXIT_FAILURE, "disaranged job_info list", NULL);
         }
      }

      if(be_verbose) {
         if (dynamic_mode) {
            fprintf(stdout, 
                    "enabling next task to be scheduled as Grid Engine "
                    "job\n");
         } else {
            fprintf(stdout, 
                    "enabling next task to be executed as Grid Engine "
                    "parallel task\n");
         }
      }

      return 1;
   }

   return 0;
}


/****** Interactive/qmake/is_recursive_make() **********************************
*
*  NAME
*     is_recursive_make -- is a command to execute a recursive make?
*
*  SYNOPSIS
*     static int is_recursive_make(const char *argv_0)
*
*  FUNCTION
*     Determines from the command name to call (argv[0]), if a job
*     is a recursive call to make.
*     If the command to execute ends with the name (argv[0]) of the 
*     actual make process, a recursive make is detected.
*
*  INPUTS
*     argv_0  - command to execute
*
*  RESULT
*     0, if no recursive make is detected
*     1, if a recursive make is detected
*
*  NOTES
*     This function will only detect directly called recursive make calls,
*     if make is called from within a shellscript, it will not be detected.
*
*  SEE ALSO
*     Interactive/qmake/might_be_recursive_make()
*     Interactive/qmake/start_remote_job()
*
****************************************************************************
*/

static int is_recursive_make(const char *argv_0) {
   char *substring;

   substring = strstr(argv_0, program_name);
   if(substring != NULL) {
      if(strcmp(substring, program_name) == 0) {
         if(be_verbose) {
            fprintf(stdout, "detected recursive make - starting on local machine\n");
         }   
         return 1;
      }
   }

   return 0;
}

/****** Interactive/qmake/might_be_recursive_make() ****************************
*
*  NAME
*     might_be_recursive_make -- might a command to exec be recursive make?
*
*  SYNOPSIS
*     static int might_be_recursive_make(char argv[])
*
*  FUNCTION
*     Tries to detect recursive make calls, that are done from within shell
*     script fragments in a makefile - this case is not handled by function
*     is_recursive_make().
*     Scans through the argument vector and searches for occurence of the
*     name of the actual make process (argv[0]).
*
*  INPUTS
*     argv - argument vector of the command to execute
*
*  RESULT
*     1, if a potential recursive make is detected,
*     0, else
*
*  NOTES
*     This function will probably deliver true much too often.
*     Imagine, you call qmake and the compiler is passed a define containing
*     the string "qmake":
*     cc -DMADE_BY=qmake -c foo.c
*     In this case each cc call will be supposed to be a recursive make and
*     be executed locally without considering the number of slots reserved
*     on this host - this behavior might lead to overload on the local host.
*
*  SEE ALSO
*     Interactive/qmake/is_recursive_make()
*     Interactive/qmake/start_remote_job()
*
****************************************************************************
*/

static int might_be_recursive_make(char *argv[]) {
   int i;

   for(i = 0; argv[i] != NULL; i++) {
      if(strstr(argv[i], program_name) != NULL) { 
         fprintf(stdout, "\nthis call might lead to a recursive qmake call:\n");
         fprintf(stdout, "%s\n", argv[i]);
         fprintf(stdout, "starting on local machine\n\n"); 
         return 1;
      }
   }

   return 0;
}

static char *get_sge_resource_request(char **args)
{
   char *ret = NULL;

   char **arg = args;
   while (*arg != NULL && ret == NULL) {
      const char *s;
      s = strstr(*arg, "SGE_RREQ=");
      if (s != NULL) {
         s += strlen("SGE_RREQ=");
         if (*s != '"') {
            fprintf(stderr, "syntax error in sge resource request\n");
         } else {
            char buffer[4096];
            char *d = buffer;
            s++;
            while (*s != '\0' && *s != '"') {
               *d++ = *s++;
            }
            *d = '\0';
            if (*s != '"') {
               fprintf(stderr, "syntax error in sge resource request\n");
            } else {
               ret = strdup(buffer);
            }
         }
      }
      
      arg++;
   }

   return ret;
}

static int count_sge_resource_request(const char *request)
{
   char *copy = strdup(request);
   int count = 0;

   if (strtok(copy, " \t") != NULL) {
      count++;
      while (strtok(NULL, " \t") != NULL) {
         count++;
      }
   }

   free(copy);
   return count;
}

static int copy_sge_resource_request(const char *request, char **args, int argc)
{
   char *copy = strdup(request);
   char *token;

   token = strtok(copy, " \t");
   while (token != NULL) {
      args[argc++] = strdup(token);
      token = strtok(NULL, " \t");
   }

   free(copy);
   return argc;
}

/* Start a remote job running the command in ARGV,
   with environment from ENVP.  It gets standard input from STDIN_FD.  On
   failure, return nonzero.  On success, return zero, and set *USED_STDIN
   to nonzero if it will actually use STDIN_FD, zero if not, set *ID_PTR to
   a unique identification, and set *IS_REMOTE to zero if the job is local,
   nonzero if it is remote (meaning *ID_PTR is a process ID).  */

/****** Interactive/qmake/start_remote_job() ***********************************
*
*  NAME
*     start_remote_job() -- start a remote job
*
*  SYNOPSIS
*     int start_remote_job(char **argv, char **envp,
*                          int stdin_fd, 
*                          int *is_remote, int *id_ptr, int *used_stdin);
*
*  FUNCTION
*     Starts a make task.
*
*     If the task is a recursive make call or looks as if it could be a 
*     recursive make, it is started on the local host.
*    
*     The next free execution host is read from qmake's hostfile,
*     if it is the localhost, the task is started locally.
*
*     The commandline and the tasks environment are setup and 
*     the task is started by forking and executing qrsh -inherit ...
*
*     Some administrative information is passed back to the caller.
*
*  INPUTS
*     argv       - argument vector of task to start
*     envp       - pointer to process environment
*     stdin_fd   - stdin filehandle, if != 0, stdin will be closed in
*                  calls to qrsh (qrsh -nostdin)
*     is_remote  - will task be executed on remote host?
*     id_ptr     - pid of forked child process
*     used_stdin - did we use stdin?
*
*  RESULT
*     0 if function completed successfully
*
*  SEE ALSO
*     Interactive/qmake/next_host()
*     Interactive/qmake/is_recursive_make()
*     Interactive/qmake/might_be__recursive_make()
*
*
****************************************************************************
*/
#define ADDTL_ENV_VARS 100
#define ADDTL_ENV_SIZE 4095
int start_remote_job (char **argv, char **envp, 
                      int stdin_fd, int *is_remote, 
                      int *id_ptr, int *used_stdin)
{
   pid_t child_pid;
   const char *hostname;
   int exec_remote = 1;
   int recursive_make = 0;
   char *addtl_env[ADDTL_ENV_VARS];
   char addtl_env_pass[ADDTL_ENV_SIZE + 1];
   char envvar[ADDTL_ENV_SIZE + 1];

   envvar[0] = '\0'; 
   addtl_env_pass[0] = '\0';
   {
      char **env = envp;
      int first_hit = 1;
      int var_idx = 0;

      /* 
       * Parse the given environment and search for variables that are not in the 
       * current environment or have a changed value.
       * These variables have to be set in the execution environment, 
       * if the task is started by qrsh, the variable names have to be part of a 
       * -v statement.
       */
      while (*env != NULL) {
         char *copy, *variable, *value, *old_value;

         /* we have to dup env as we split it into variable and value */
         copy = strdup(*env);
         variable = strtok(copy, "=");
         value = strtok(NULL, "=");
         if (value == NULL) {
            value = "";
         }

         /* retrieve variable from current environment */
         old_value = getenv(variable);

         /* if variable isn't set in current environment, or has been changed */
         if (old_value == NULL || strcmp(old_value, value) != 0) {
            if (var_idx >= ADDTL_ENV_VARS) {
               free(copy); copy = NULL;
               fprintf(stderr, "qmake: too many additional environment variables to set\n");
               return 1;
            }
            /* store additional environment to set */
            addtl_env[var_idx++] = strdup(*env);
           
            /* store variable name for -v option */
            if (strlen(addtl_env_pass) + strlen(variable) >= ADDTL_ENV_SIZE) {
               free(copy); copy = NULL;
               fprintf(stderr, "qmake: additional environment variable names exeed buffer\n");
               return 1;
            }
            if (!first_hit) {
               strcat(addtl_env_pass, ","); /* RATS: ignore */
            }
            strcat(addtl_env_pass, variable); /* RATS: ignore */

            first_hit = 0;
         }

         /* free the duplicated env entry */
         free(copy); copy = NULL;
         env++;
      }

      /* addtl_env is a NULL terminated list (array) */
      addtl_env[var_idx] = NULL;

      if(be_verbose) {
         fprintf(stdout, "export the following environment variables: %s\n", addtl_env_pass);
      }
   } 
   
   /* check for recursive make */
   if (is_recursive_make(argv[0])) {
      /* force local execution */
      exec_remote = 0;
      recursive_make = 1;
      if (dynamic_mode) {
         hostname = "dynamic mode";
      } else {
         hostname = localhost;
         unlock_hostentry(jobs[next_job].offset);
      }
      jobs[next_job].offset = -1;
   } else {
      if (might_be_recursive_make(argv)) {
         /* argv contains program name */
         exec_remote = 0;
         if (dynamic_mode) {
            hostname = "dynamic mode";
         } else {
            hostname = localhost;
            unlock_hostentry(jobs[next_job].offset);
         }
         jobs[next_job].offset = -1;
         /* dump environment variable RECURSIVE_QMAKE_OPTIONS */
         {
            int   i;

            strcpy(envvar, "RECURSIVE_QMAKE_OPTIONS=-inherit"); /* RATS: ignore */

            if(pass_cwd) {
               strcat(envvar, "\n-cwd"); /* RATS: ignore */
            }

            if(be_verbose) {
               strcat(envvar, "\n-verbose"); /* RATS: ignore */
            }
            
            for(i = 0; i < sge_v_argc; i++) {
               if (strlen(envvar) + strlen(sge_v_argv[i]) >= ADDTL_ENV_SIZE) {
                  fprintf(stderr, "qmake: RECURSIVE_QMAKE_OPTIONS too big\n");
                  return 1;
               }
               strcat(envvar, "\n");          /* RATS: ignore */
               strcat(envvar, sge_v_argv[i]); /* RATS: ignore */
            }
           
            if(be_verbose) {
               fprintf(stdout, "saving sge options: %s\n", envvar);
            }
         }
      } else {
         /* remote execution possible */
         if (dynamic_mode) {
            hostname = "dynamic mode";
         } else {
            hostname = next_host();
         }
      }
   }
 
   /* can we use stdin? */
   if(stdin_fd == 0) {
      *used_stdin = 1;
   }
 
   printf("%s\n", hostname); 
  
   child_pid = fork();

   if(child_pid) {
      int i;

      /* parent */
      *is_remote = 1;
      *id_ptr = child_pid;
      jobs[next_job].pid = child_pid;

      /* free the additional environment variables */
      i = 0;
      while (addtl_env[i] != NULL) {
         free(addtl_env[i]);
         addtl_env[i] = NULL;
         i++;
      }
   } else {
      /* child */
      int argc, no_args, no_requests, i;
      char **args;
      char *resource_request = NULL;

      /* set PAREND env var */
      if(getenv("JOB_ID")) {
         static char buffer[1024];
         snprintf(buffer, sizeof buffer, "PARENT=%s", getenv("JOB_ID"));
         putenv(buffer);
      }
  
      argc = 0;
      i    = 0;
      no_args = 0;
      no_requests = 0;
     
      /* count arguments */
      while(argv[no_args++]);

      /* do we have individual job requests? */
      if (dynamic_mode) {
         resource_request = get_sge_resource_request(argv);
         if (resource_request != NULL) {
            if (be_verbose) {
               fprintf(stdout, "add SGE resource request for this rule: %s\n", 
                       resource_request);
            }
            no_requests = count_sge_resource_request(resource_request);
         }
      }
  
      args = (char **)malloc((no_args + no_requests + sge_v_argc + 8 + pass_cwd + be_verbose) * sizeof(char *));

      if(exec_remote) {
         args[argc++] = "qrsh";

         args[argc++] = "-noshell";

         if(be_verbose) {
            args[argc++] = "-verbose";
         }
        
         if (!dynamic_mode) {
            args[argc++] = "-inherit";
         }

         if(stdin_fd != 0) {
            args[argc++] = "-nostdin";
         }

         if(pass_cwd) {
            args[argc++] = "-cwd";
         }

         if (dynamic_mode) {
            args[argc++] = "-now";
            args[argc++] = "no";

            if (resource_request) {
               argc = copy_sge_resource_request(resource_request, args, argc);
               free(resource_request);
               resource_request = NULL;
            }
         }

         if (addtl_env_pass[0] != '\0') {
            args[argc++] = "-v";
            args[argc++] = addtl_env_pass;
         }

         for(i = 0; i < sge_v_argc; i++) {
            args[argc++] = sge_v_argv[i];
         }

         if (!dynamic_mode) {
            args[argc++] = (char *)hostname;
         }
         i = 0;
      } else {
         if (recursive_make) {
            args[argc++] = argv[0];
            if(be_verbose) {
               args[argc++] = "-verbose";
            }
            args[argc++] = "-inherit";

            if(pass_cwd) {
               args[argc++] = "-cwd";
            }

            if (addtl_env_pass[0] != '\0') {
               args[argc++] = "-v";
               args[argc++] = addtl_env_pass;
            }

            for(i = 0; i < sge_v_argc; i++) {
               args[argc++] = sge_v_argv[i];
            }
            
            args[argc++] = "--";
            i = 1;
         }
      }   

      for(; argv[i] != NULL; i++) {
         args[argc++] = argv[i];
      }

      args[argc] = NULL;
     
      if(be_verbose) {
         fprintf(stdout, "starting job: \n");
         for(i = 0; args[i] != NULL; i++) {
            fprintf(stdout, "args[%3d] = %s\n", i, args[i]);
         }
      }

      /* set the RECURSIVE_QMAKE_OPTIONS environment variable */
      if ( envvar[0] != 0 ) {
         putenv(envvar);
      }

      /* set the additional environment variables */
      i = 0;
      while (addtl_env[i] != NULL) {
         putenv(addtl_env[i]);
         i++;
      }

      execvp(args[0], args);
   }
   
  return 0;
}
/* Get the status of a dead remote child.  Block waiting for one to die
   if BLOCK is nonzero.  Set *EXIT_CODE_PTR to the exit status, *SIGNAL_PTR
   to the termination signal or zero if it exited normally, and *COREDUMP_PTR
   nonzero if it dumped core.  Return the ID of the child that died,
   0 if we would have to block and !BLOCK, or < 0 if there were none.  */

/****** qmake/remote_status() ***************************************
*
*  NAME
*     remote_status() -- return status of dead children
*
*  SYNOPSIS
*     int remote_status(int *exit_code_ptr, int *signal_ptr, 
*                       int *coredump_ptr, int block);
*
*  FUNCTION
*     Reports to the caller (gmake) information about the next child that
*     has exited.
*     First checks, whether information has been cached in saved_status, 
*     if yes, returns information about the first record in saved_status
*     and deletes it,
*     if not, calls read_remote_status to check for recently finished 
*     children.
*
*  INPUTS
*     exit_sge_ptr - see RESULT 
*     signal_ptr   - see RESULT
*     coredump_ptr - see RESULT
*     block        - flag whether to block when waiting for child to die
*
*  RESULT
*     remote_status - the pid of the dead child, 
*                     0 if we would have to block and block is 0
*                     -1, if block is 0 
*     exit_sge_ptr  - exit code of the child process 
*     signal_ptr    - 0 when process exited normally, else the signal by
*                     which the process was terminated
*     coredump_ptr  - nonzero, if the childprocess dumped core
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*     qmake/read_remote_status()
*
****************************************************************************
*/

int remote_status (int *exit_code_ptr, int *signal_ptr, 
               int *coredump_ptr, int block)
{
   pid_t child_pid;

   if(be_verbose) {
      fprintf(stdout, "gmake requesting status of dead child processes\n");
   }

   if(saved_status != NULL) {
      struct finished_job *jobinfo;
      
      jobinfo        = saved_status;     
      saved_status   = saved_status->next;
      
      child_pid      = jobinfo->pid;
      *exit_code_ptr = jobinfo->exit_code;
      *signal_ptr    = jobinfo->signal;
      *coredump_ptr  = jobinfo->coredump;
      
      free(jobinfo);
      
      return child_pid;
   } 

   return read_remote_status(exit_code_ptr, signal_ptr, coredump_ptr, block);
}   

/****** qmake/read_and_save_remote_status() ***************************************
*
*  NAME
*     read_and_save_remote_status() -- read and cache status of dead children
*
*  SYNOPSIS
*     static void read_and_save_remote_status(int *exit_code_ptr, int *signal_ptr, 
*                                             int *coredump_ptr, int block);
*
*  FUNCTION
*     Waits or checks for dead child processes by calling read_remote_status.
*     If a child has finished, stores information from read_remote_status
*     to a finished_job structure and appends it to the global list
*     saved_status.
*
*  INPUTS
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*     qmake/read_remote_status()
*
****************************************************************************
*/

static void read_and_save_remote_status()
{
   static struct finished_job jobinfo;
   pid_t child_pid;

   child_pid = read_remote_status(&(jobinfo.exit_code), &(jobinfo.signal), &(jobinfo.coredump), 0);

   if(child_pid > 0) {
      struct finished_job *job;
      
      jobinfo.pid = child_pid;

      job = (struct finished_job *)malloc(sizeof(finished_job));
      memcpy(job, &jobinfo, sizeof(finished_job));

      job->next = NULL;
      if(saved_status == NULL) {
         saved_status = job;
      } else {
         struct finished_job *j;

         j = saved_status;
         while(j->next != NULL) {
            j = j->next;
         }
         j->next = job;
      }
   }
}

/****** qmake/read_remote_status() ***************************************
*
*  NAME
*     read_remote_status() -- return status of dead children
*
*  SYNOPSIS
*     static int read_remote_status(int *exit_code_ptr, int *signal_ptr, 
*                                   int *coredump_ptr, int block);
*
*  FUNCTION
*     Waits or checks for dead child processes.
*     Reports to caller
*        - the pid of a dead child process
*        - the exit code
*        - evtl. the signal by which a process was terminated
*        - evtl. a flag, that the process dumped core
*     Cleans up the job_info for a dead child, unlocks the host where
*     the child process was executed and removes an evtl. existing
*     environmentfile.
*
*  INPUTS
*     exit_sge_ptr - see RESULT 
*     signal_ptr   - see RESULT
*     coredump_ptr - see RESULT
*     block        - flag whether to block when waiting for child to die
*
*  RESULT
*     remote_status - the pid of the dead child, 
*                     0 if we would have to block and block is 0
*                     -1, if block is 0 
*     exit_sge_ptr  - exit code of the child process 
*     signal_ptr    - 0 when process exited normally, else the signal by
*                     which the process was terminated
*     coredump_ptr  - nonzero, if the childprocess dumped core
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*
****************************************************************************
*/

static int read_remote_status(int *exit_code_ptr, int *signal_ptr, int *coredump_ptr, int block)
{
   int status;
   pid_t child_pid;
   int i;
   
   *exit_code_ptr = 0;
   *signal_ptr    = 0;
   *coredump_ptr  = 0;

   /* suppress misleading error messages */
   errno = 0;

   /* get info about dead children */
   child_pid = waitpid(-1, &status, block ? 0 : WNOHANG);

   /* waitpid failed? */
   if(child_pid <= 0) {
      if(be_verbose) {
         fprintf(stderr, "waiting for child failed: %s\n", errno == 0 ? "timeout" : strerror(errno));
      }   
      return child_pid;
   }

   /* waitpid reported stopped or continued child? */
   if(WIFSTOPPED(status)) {
      fprintf(stderr, "child %d was stopped\n", child_pid);
      return -1;
   }
  
#ifdef WIFCONTINUED
   if(WIFCONTINUED(status)) {
      fprintf(stderr, "child %d is continuing\n", child_pid);
      return -1;
   }
#endif   
   
   if(WIFEXITED(status)) {
      *exit_code_ptr = WEXITSTATUS(status);
   }

   if(WIFSIGNALED(status)) {
      *signal_ptr = WTERMSIG(status);
#ifdef WCOREDUMP
      *coredump_ptr = WCOREDUMP(status);
#endif      
   }

   /* cleanup job_info */
   for(i = 0; i < host_count; i++) {
      if(jobs[i].pid == child_pid) {
         unlock_hostentry(jobs[i].offset);
         jobs[i].pid = 0;
         jobs[i].offset = -1;
         break;
      }
   }

   return child_pid;
}

/****** qmake/block_remote_children() ***************************************
*
*  NAME
*     block_remote_children() -- ??
*
*  SYNOPSIS
*     void block_remote_children();
*
*  FUNCTION
*     Block asynchronous notification of remote child death.
*     If this notification is done by raising the child termination
*     signal, do not block that signal.
*
*  INPUTS
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     No idea, what this function is meant for :-(
*     Hope, we will not need it.
*
*  BUGS
*
*  SEE ALSO
*     qmake/unblock_remote_children()
*
****************************************************************************
*/

void block_remote_children ()
{
   if(be_verbose) {
      fprintf(stdout, "gmake called block_remote_children()\n");
   }

   return;
}

/* Restore asynchronous notification of remote child death.
   If this is done by raising the child termination signal,
   do not unblock that signal.  */
/****** qmake/unblock_remote_children() ***************************************
*
*  NAME
*     unblock_remote_children() -- ??
*
*  SYNOPSIS
*     void unblock_remote_children();
*
*  FUNCTION
*     Restore asynchronous notification of remote child death.
*     If this is done by raising the child termination signal,
*     do not unblock that signal.
*
*  INPUTS
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     No idea, what this function is meant for :-(
*     Hope, we will not need it.
*
*  BUGS
*
*  SEE ALSO
*     qmake/block_remote_children()
*
****************************************************************************
*/

void unblock_remote_children ()
{
   if(be_verbose) {
      fprintf(stdout, "gmake called unblock_remote_children()\n");
   }

   return;
}

/* Send signal SIG to child ID.  Return 0 if successful, -1 if not.  */
/****** qmake/remote_kill() ***************************************
*
*  NAME
*     remote_kill -- send a signal to remote job
*
*  SYNOPSIS
*     int remote_kill(int id, int sig);
*
*  FUNCTION
*     Sends the signal given as parameter to the given process.
*
*  INPUTS
*     id  - process id of child process to notify
*     sig - signal to send to child process
*
*  RESULT
*     result of kill system call: 0 if 0K, else -1
*
*  EXAMPLE
*
*  NOTES
*
*  BUGS
*
*  SEE ALSO
*
****************************************************************************
*/

int remote_kill (int id, int sig)
{
   if(be_verbose) {
      fprintf(stdout, "gmake requested to send signal %d to process %d\n", sig, id);
   }
   
   return kill(id, sig);
}

/****** qmake/main() ***************************************
*
*  NAME
*     main() -- main program for testing some functions
*
*  SYNOPSIS
*     int main(int argc, char *argv[]);
*
*  FUNCTION
*     main function to create a test program. It will be compiled, 
*     if the define TEST_REMOTE is set.
*     The following features/functions will be tested:
*        - splitting of sge and gmake options
*        - parsing of option -inherit
*        - equalize NSLOTS
*        - create -pe option from -j option
*        - setting of architecture as default resource request
*        - creation of a qmake hostfile
*        - initialization of filenames, filehandles, number of slots
*        - determination of next host to use
*
*     Usage:
*        - set the environmentvariable TMPDIR
*        - create a sge pe hostfile in $TMPDIR
*          called hostfile
*        - call test program with commandline options
*        - analyze output of test program
*
*  INPUTS
*     argc  - number of commandline parameters
*     argv  - commandline parameters, first is program name
*
*  RESULT
*     exitcode - allways 0
*
*  EXAMPLE
*     set TMPDIR:
*        setenv TMPDIR /tmp
*     hostfile:
*        cat >$TMPDIR/hostfile
*        SOWA.mydomain.de 1 sowa UNDEFINED
*        BALROG.mydomain.de 2 balrog UNDEFINED
*        BILBUR.mydomain.de 2 bilbur UNDEFINED
*        SARUMAN.mydomain.de 1 saruman UNDEFINED
*        ^d
*     compiling test program:
*        gcc -o test -DTEST_REMOTE remote-sge.c
*     program call:
*        ./test -pe make 3 -- -j 3 all install
*
*  NOTES
*
*  BUGS
*     !!!! Tests program does no longer work !!!!
*
*  SEE ALSO
*
****************************************************************************
*/

#ifdef TEST_REMOTE
int main(int argc, char *argv[])
{
   int i;

   printf("\nTest program for gmake remote functions using Sge/SGE\n");

   printf("\nTesting commandline parsing ... \n");

   printf("\tbefore splitting\n");
   for(i = 0; i < argc; i++) {
      printf("\t\targv[%d]        = %s\n", i, argv[i]);
   }

   parse_options(&argc, &argv);
   
   printf("\tafter splitting\n");
   for(i = 0; i < sge_argc; i++) {
      printf("\t\tsge_argv[%d] = %s\n", i, sge_argv[i]);
   }   
   for(i = 0; i < gmake_argc; i++) {
      printf("\t\tgmake_argv[%d]  = %s\n", i, gmake_argv[i]);
   }   
   for(i = 0; i < argc; i++) {
      printf("\t\targv[%d]        = %s\n", i, argv[i]);
   }   

   printf("\thandling of option -inherit - recognize and strip\n");
   printf("\t\t%s\n", inherit_job() ? "found -inherit option" : "no -inherit option set");

   for(i = 0; i < sge_argc; i++) {
      printf("\t\tsge_argv[%d] = %s\n", i, sge_argv[i]);
   }

   printf("\tcreate -pe option from -j option\n");
   /* JG: If no pe is given, we use dynamic allocation */
#if 0
   equalize_pe_j();
#endif
   for(i = 0; i < sge_argc; i++) {
      printf("\t\tsge_argv[%d] = %s\n", i, sge_argv[i]);
   }   

   printf("\tequalize NSLOTS with -j option\n");
   equalize_nslots(&argc, &argv);
   for(i = 0; i < argc; i++) {
      printf("\t\targv[%d]        = %s\n", i, argv[i]);
   }   

   printf("\tdefault options ...\n");
   set_default_options();

   for(i = 0; i < sge_argc; i++) {
      printf("\t\tsge_argv[%d] = %s\n", i, sge_argv[i]);
   }   



   printf("\ninitializing remote execution ... ");
   init_remote();
   printf("done\n");

   printf("\ntesting qmake hostfile and next_host function\n");

   printf("\tnumber of slots is %d\n", host_count);

   remote_enabled = 1;
   
   for(i = 0; i < 20; i++) {
      /* reserve job_info reservieren */
      start_remote_job_p(1);
      jobs[next_job].pid = 123;

      /* request host */
      printf("\thost for job %3d = %s\n", i, next_host());

      /* release some hosts */
      if(jobs[next_job].offset % 3) { 
         unlock_hostentry(jobs[next_job].offset);
         jobs[next_job].pid = 0;
         jobs[next_job].offset = -1;
      } 
   }
}

#endif
