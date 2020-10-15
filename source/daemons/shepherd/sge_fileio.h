#ifndef __SGE_FILEIO_H
#define __SGE_FILEIO_H
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
bool 
shepherd_write_pid_file(pid_t pid, dstring *errmsg);

bool
shepherd_read_qrsh_pid_file(const char *filename, pid_t *qrsh_pid,
                            int *replace_qrsh_pid);

bool
shepherd_write_usage_file(u_long32 wait_status, int exit_status,
                          int child_signal, u_long32 start_time,
                          u_long32 end_time, struct rusage *rusage);

bool
shepherd_write_job_pid_file(const char *job_pid);

bool 
shepherd_write_osjobid_file(const char *osjobid);

bool 
shepherd_write_shepherd_about_to_exit_file(void);

bool 
shepherd_read_exit_status_file(int *return_code);

void
create_checkpointed_file(int ckpt_is_in_arena);

int 
checkpointed_file_exists(void);

bool
shepherd_write_sig_info_file(const char *filename, const char *task_id,
                             u_long32 exit_status);

#if defined(IRIX)
bool
shepherd_read_osjobid_file(
#if (IRIX)
   ash_t *return_code,
#endif
   bool is_error
);
#endif

bool
shepherd_read_qrsh_file(const char *filename, pid_t *qrsh_pid);

bool
shepherd_write_processor_set_number_file(int proc_set);

#endif /* __SGE_FILEIO_H */
