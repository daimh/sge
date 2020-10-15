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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "uti/config_file.h"
#include "uti/sge_stdio.h"
#include "uti/sge_uidgid.h"

#include "basis_types.h"
#include "err_trace.h"
#include "execution_states.h"
#include "msg_common.h"

bool shepherd_write_pid_file(pid_t pid, dstring *errmsg)
{
   bool ret = true;
   FILE *fp = NULL;

   fp = fopen("pid", "w");
   if (fp != NULL) {
      if (fprintf(fp, pid_t_fmt"\n", pid) < 0) {
         sge_dstring_sprintf(errmsg, MSG_FILE_CANNOT_WRITE_SS, "pid", strerror(errno));
         ret = false;
      } else {
         if (fflush(fp) < 0) {
            sge_dstring_sprintf(errmsg, MSG_FILE_CANNOT_FLUSH_SS, "pid", strerror(errno));
            ret = false;
         }
      }
      FCLOSE(fp);
   } else {
      sge_dstring_sprintf(errmsg, MSG_FILE_NOOPEN_SS, "pid", strerror(errno));
      ret = false;
   }
   return ret;
FCLOSE_ERROR:
   sge_dstring_sprintf(errmsg, MSG_FILE_NOCLOSE_SS, "pid", strerror(errno));
   return false;
}

bool
shepherd_read_qrsh_pid_file(const char *filename, pid_t *qrsh_pid,
                            int *replace_qrsh_pid)
{
   bool ret = true;
   FILE *fp = NULL;

   fp = fopen(filename, "r");
   if (fp != NULL) {
      int arguments = fscanf(fp, pid_t_fmt, qrsh_pid);

      if (arguments == 1) {
         char buffer[50];

         /* set pid from qrsh_starter as job_pid */
         sprintf(buffer, pid_t_fmt, *qrsh_pid);
         /* TODO: should better be add_or_replace */
         add_config_entry("job_pid", buffer);
         *replace_qrsh_pid = 0;
      } else {
         shepherd_trace("could not read qrsh_pid file");
         ret = false;
      }
      FCLOSE(fp);
   } else {
      /*
       * CR 6588743 - raising a shepherd_error here would set the queue in
       *              error state and rerun the job
       */
      shepherd_trace(MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FCLOSE_ERROR:
   /*
    * CR 6588743 - raising a shepherd_error here would set the queue in
    *              error state and rerun the job
    */
   shepherd_trace(MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool
shepherd_write_usage_file(u_long32 wait_status, int exit_status,
                          int child_signal, u_long32 start_time,
                          u_long32 end_time, struct rusage *rusage)
{
   bool ret = true;
   const char *const filename = "usage";
   FILE *fp = NULL;

   shepherd_trace("writing usage file to \"usage\"");

   fp = fopen(filename, "w");
   if (fp != NULL) {
      /*
       * the wait status is returned by japi_wait()
       * see sge_reportL.h for bitmask and makro definition
       */
      FPRINTF((fp, "wait_status="sge_u32"\n", wait_status));
      FPRINTF((fp, "exit_status=%d\n", exit_status));
      FPRINTF((fp, "signal=%d\n", child_signal));

      FPRINTF((fp, "start_time=%d\n", (int) start_time));
      FPRINTF((fp, "end_time=%d\n", (int) end_time));
      FPRINTF((fp, "ru_wallclock="sge_u32"\n", (u_long32) end_time-start_time));

      FPRINTF((fp, "ru_utime=%f\n", (double)rusage->ru_utime.tv_sec + (double)rusage->ru_utime.tv_usec / 1000000.0));
      FPRINTF((fp, "ru_stime=%f\n", (double)rusage->ru_stime.tv_sec + (double)rusage->ru_stime.tv_usec / 1000000.0));
      FPRINTF((fp, "ru_maxrss="sge_u64"\n", (u_long64)rusage->ru_maxrss));
      FPRINTF((fp, "ru_ixrss="sge_u64"\n", (u_long64)rusage->ru_ixrss));
      FPRINTF((fp, "ru_idrss="sge_u64"\n", (u_long64)rusage->ru_idrss));
      FPRINTF((fp, "ru_isrss="sge_u64"\n", (u_long64)rusage->ru_isrss));
      FPRINTF((fp, "ru_minflt="sge_u64"\n", (u_long64)rusage->ru_minflt));
      FPRINTF((fp, "ru_majflt="sge_u64"\n", (u_long64)rusage->ru_majflt));
      FPRINTF((fp, "ru_nswap="sge_u64"\n", (u_long64)rusage->ru_nswap));
      FPRINTF((fp, "ru_inblock="sge_u64"\n", (u_long64)rusage->ru_inblock));
      FPRINTF((fp, "ru_oublock="sge_u64"\n", (u_long64)rusage->ru_oublock));
      FPRINTF((fp, "ru_msgsnd="sge_u64"\n", (u_long64)rusage->ru_msgsnd));
      FPRINTF((fp, "ru_msgrcv="sge_u64"\n", (u_long64)rusage->ru_msgrcv));
      FPRINTF((fp, "ru_nsignals="sge_u64"\n", (u_long64)rusage->ru_nsignals));
      FPRINTF((fp, "ru_nvcsw="sge_u64"\n", (u_long64)rusage->ru_nvcsw));
      FPRINTF((fp, "ru_nivcsw="sge_u64"\n", (u_long64)rusage->ru_nivcsw));

      FCLOSE(fp);

   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool
shepherd_write_job_pid_file(const char *job_pid)
{
   bool ret = true;
   const char *const filename = "job_pid";
   FILE *fp = NULL;

   fp = fopen(filename, "w");
   if (fp != NULL) {
      FPRINTF((fp, "%s\n", job_pid));
      FCLOSE(fp);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool
shepherd_write_sig_info_file(const char *filename, const char *task_id,
                             u_long32 exit_status)
{
   bool ret = true;
   FILE *fp = NULL;

   fp = fopen(filename, "a");
   if (fp != NULL) {
      FPRINTF((fp, "%s "sge_u32"\n", task_id, exit_status));
      FCLOSE(fp);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}


bool shepherd_write_osjobid_file(const char *osjobid)
{
   bool ret = true;
   const char *const filename = "osjobid";
   FILE *fp = NULL;

   fp = fopen(filename, "w");
   if (fp != NULL) {
      FPRINTF((fp, "%s\n", osjobid));
      FCLOSE(fp);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool 
shepherd_write_processor_set_number_file(int proc_set)
{
   bool ret = true;
   const char *const filename = "processor_set_number";
   FILE *fp = NULL;

   fp = fopen(filename, "w");
   if (fp != NULL) {
      FPRINTF((fp, "%d\n", proc_set));
      FCLOSE(fp);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool 
shepherd_write_shepherd_about_to_exit_file(void)
{
   bool ret = true;
   const char *const filename = "shepherd_about_to_exit";
   FILE *fd = NULL;

   fd = fopen(filename, "w");
   if (fd != NULL) {
      FCLOSE(fd);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   return ret;
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool 
shepherd_read_exit_status_file(int *return_code)
{
   bool ret = true;
   FILE *fp = NULL;
   const char *const filename = "exit_status";

   fp = fopen(filename, "r");
   if (fp != NULL) {
      int arguments = fscanf(fp, "%d\n", return_code);
      /* retrieve first exit status from exit status file */

      if (arguments != 1) {
         shepherd_trace("could not read exit_status file");
         *return_code = ESSTATE_NO_EXITSTATUS;
         ret = false;
      }
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      ret = false;
   }
   FCLOSE(fp);
   return ret;
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}

bool 
shepherd_read_qrsh_file(const char* pid_file_name, pid_t *qrsh_pid)
{
   bool ret = true;
   FILE *fp = NULL;

   fp = fopen(pid_file_name, "r");
   if (fp != NULL) {
      int arguments = fscanf(fp, pid_t_fmt, qrsh_pid);

      /* retrieve first exit status from exit status file */
      if (arguments != 1) {
         shepherd_trace("could not read qrsh_pid_file '%s'", pid_file_name);
         *qrsh_pid = 0;
         ret = false;
      } 
      FCLOSE(fp);
   } else {
      /*
       * CR 6588743 - raising a shepherd_error here would set the queue in
       *              error state and rerun the job
       */
      shepherd_trace(MSG_FILE_NOOPEN_SS, pid_file_name, strerror(errno));
      ret = false;
   }
   return ret;
FCLOSE_ERROR:
   /*
    * CR 6588743 - raising a shepherd_error here would set the queue in
    *              error state and rerun the job
    */
   shepherd_trace(MSG_FILE_NOCLOSE_SS, pid_file_name, strerror(errno));
   return false;
}

#if defined(IRIX)
bool 
shepherd_read_osjobid_file(
   ash_t *return_code,
   bool is_error
)
{
   bool ret = true;
   FILE *fp = NULL;
   const char *const filename = "osjobid";

   fp = fopen(filename, "r");
   if (fp != NULL) {
      int arguments = 0;

      arguments = fscanf(fp, "%lld\n", return_code);

      if (arguments != 1) {
         shepherd_trace("could not read osjobid file");
         *return_code = 0;
         ret = false;
      }
      FCLOSE(fp);
   } else {
      if (is_error == true) {
         shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      } else {
         shepherd_trace(MSG_FILE_NOOPEN_SS, filename, strerror(errno));
      }
      ret = false;
   }
   return ret;
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return false;
}
#endif

void 
create_checkpointed_file(int ckpt_is_in_arena)
{
   const char *const filename = "checkpointed";
   FILE *fp = NULL;

   fp = fopen(filename, "w");
   if (fp != NULL) {
      if (ckpt_is_in_arena) {
         FPRINTF((fp, "1\n"));
      }
      FCLOSE(fp);
   } else {
      shepherd_error(1, MSG_FILE_NOOPEN_SS, filename, strerror(errno));
   }
   return;
FPRINTF_ERROR:
FCLOSE_ERROR:
   shepherd_error(1, MSG_FILE_NOCLOSE_SS, filename, strerror(errno));
   return;
}

int 
checkpointed_file_exists(void)
{
   SGE_STRUCT_STAT buf;
   return !SGE_STAT("checkpointed", &buf);
}



