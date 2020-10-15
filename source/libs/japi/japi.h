#ifndef __JAPI_H
#define __JAPI_H

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

#include "drmaa.h"
#include "sge_dstring.h"
#include "basis_types.h"
#include "sge_japi_JJ_L.h"
#include "sge_japi_JJAT_L.h"
#include "sge_japi_NSV_L.h"
#include "cull.h"

/****** JAPI/-JAPI_Interface *******************************************************
*  NAME
*     JAPI_Interface -- The enlisted functions are the interface of the JAPI library
* 
*  SEE ALSO
*     JAPI/japi_init()
*     JAPI/japi_exit()
*     JAPI/japi_run_job()
*     JAPI/japi_run_bulk_jobs()
*     JAPI/japi_control()
*     JAPI/japi_synchronize()
*     JAPI/japi_wait()
*     JAPI/japi_wifexited()
*     JAPI/japi_wexitstatus()
*     JAPI/japi_wifsignaled()
*     JAPI/japi_wtermsig()
*     JAPI/japi_wifcoredump()
*     JAPI/japi_wifaborted()
*     JAPI/japi_job_ps()
*     JAPI/japi_strerror()
*     JAPI/japi_get_contact()
*     JAPI/japi_version()
*     JAPI/japi_get_drm_system()
*     JAPI/japi_allocate_string_vector()
*     JAPI/japi_string_vector_get_next()
*     JAPI/japi_delete_string_vector()
*     JAPI/japi_standard_error()
*     JAPI/japi_init_mt()
*******************************************************************************/
#ifdef  __cplusplus
extern "C" {
#endif

/* Bitfield for japi_wait() event flag */
enum japi_events {
   JAPI_JOB_FINISH = 0x01,
   JAPI_JOB_START = 0x02
/* JAPI_NEW_EVENT = 0x04 */
};

/* values for japi_exit() job exit flag */
enum japi_flags {
   JAPI_EXIT_NO_FLAG,
   JAPI_EXIT_KILL_ALL,
   JAPI_EXIT_KILL_PENDING
};

/* Type for japi_int()/japi_enable_job_wait() error handler callback.  The
 * callback function should not free the const char* parameter. */
typedef void (*error_handler_t)(const char *);

/* init/exit routines ------------------- */
/*
 * Initialize DRMAA API library and create a new DRMAA Session. 'Contact'
 * is an implementation dependent string which may be used to specify
 * which DRM system to use. This routine must be called before any
 * other DRMAA calls, except for japi_version().
 * If 'contact' is NULL, the default DRM system will be used.
 */ 
int japi_init(const char *contact, const char *session_key_in, 
              dstring *session_key_out, int prog_number, bool enable_wait,
              error_handler_t handler, dstring *diag);

/*
 * Starts the event client.  If japi_init() is called with the start_ec
 * parameter set to false, this method must be called before calling japi_wait()
 * or japi_synchronize().  This method is useful if, for example, one doesn't
 * know whether japi_wait() will be needed or not at the time that japi_init()
 * is called.
 */ 
int japi_enable_job_wait (const char*username, const char *unqualified_hostname, const char *session_key_in, dstring *session_key_out,
                          error_handler_t handler, dstring *diag);


/*
 * Disengage from DRMAA library and allow the DRMAA library to perform
 * any necessary internal clean up.
 * This routine ends this DRMAA Session, but does not effect any jobs (e.g.,
 * queued and running jobs remain queued and running).
 */
int japi_exit(int flag, dstring *diag);

/*
 * Tests if the current session has been initialized.  Returns
 * DRMAA_ERRNO_SUCCESS if so, and DRMAA_ERRNO_NO_ACTIVE_SESSION if not.
 */
int japi_was_init_called(dstring* diag);


/* ------------------- job submission routines ------------------- */

/*
 * Submit a job with attributes defined in the job template 'jt'.
 * The job identifier 'job_id' is a printable, NULL terminated string,
 * identical to that returned by the underlying DRM system.  If
 * use_euid_egid is true, submit with the current effective uid/gid,
 * not the real ones.
 */
int japi_run_job(dstring *jobid, lListElem **sge_job_template, bool use_euid_egid, dstring *diag);

/* 
 * Submit a set of parametric jobs, dependent on the implied loop index, each
 * with attributes defined in the job template 'jt'.
 * The job identifiers 'job_ids' are all printable,
 * NULL terminated strings, identical to those returned by the underlying
 * DRM system. Nonnegative loop bounds are mandated to avoid file names
 * that start with minus sign like command line options.
 * The special index placeholder is a DRMAA defined string
 * drmaa_incr_ph == $incr_pl$
 * that is used to construct parametric job templates.
 * For example:
 * drmaa_set_attribute(pjt, "stderr", drmaa_incr_ph + ".err" ); (C++/java string syntax used)
 */

int japi_run_bulk_jobs(drmaa_attr_values_t **values, lListElem **sge_job_template, int start, int end, int incr, bool use_euid_egid, dstring *diag);

/* ------------------- job control routines ------------------- */

/*
 * Start, stop, restart, or kill the job identified by 'job_id'.
 * If 'job_id' is DRMAA_JOB_IDS_SESSION_ALL, then this routine
 * acts on all jobs *submitted* during this DRMAA session.
 * The legal values for 'action' and their meanings are:
 * DRMAA_CONTROL_SUSPEND: stop the job,
 * DRMAA_CONTROL_RESUME: (re)start the job,
 * DRMAA_CONTROL_HOLD: put the job on-hold,
 * DRMAA_CONTROL_RELEASE: release the hold on the job, and
 * DRMAA_CONTROL_TERMINATE: kill the job.
 * This routine returns once the action has been acknowledged by
 * the DRM system, but does not necessarily wait until the action
 * has been completed.
 */
int japi_control(const char *jobid, int action, dstring *diag);


/* 
 * Wait until all jobs specified by 'job_ids' have finished
 * execution. If 'job_ids' is DRMAA_JOB_IDS_SESSION_ALL, then this routine
 * waits for all jobs *submitted* during this DRMAA session. To prevent
 * blocking indefinitely in this call the caller could use timeout specifying
 * after how many seconds to time out in this call.
 * If the call exits before timeout all the jobs have been waited on
 * or there was an interrupt.
 * If the invocation exits on timeout, the return code is DRMAA_ERRNO_EXIT_TIMEOUT.
 * The caller should check system time before and after this call
 * in order to check how much time has passed.
 * Dispose parameter specifies how to treat reaping information:
 * True=1 "fake reap", i.e. dispose of the rusage data
 * False=0 do not reap
 */ 
int japi_synchronize(const char *job_ids[], signed long timeout, bool dispose, dstring *diag);


/* 
 * This routine waits for a job with job_id to fail or finish execution. Passing a special string
 * DRMAA_JOB_IDS_SESSION_ANY instead job_id waits for any job. If such a job was
 * successfully waited its job_id is returned as a second parameter. This routine is
 * modeled on wait3 POSIX routine. To prevent
 * blocking indefinitely in this call the caller could use timeout specifying
 * after how many seconds to time out in this call.
 * If the call exits before timeout the job has been waited on
 * successfully or there was an interrupt.
 * If the invocation exits on timeout, the return code is DRMAA_ERRNO_EXIT_TIMEOUT.
 * The caller should check system time before and after this call
 * in order to check how much time has passed.
 * The routine reaps jobs on a successful call, so any subsequent calls
 * to drmaa_wait should fail returning an error DRMAA_ERRNO_INVALID_JOB meaning
 * that the job has been already reaped. This error is the same as if the job was
 * unknown. Failing due to an elapsed timeout has an effect that it is possible to
 * issue drmaa_wait multiple times for the same job_id.
 */
int japi_wait(const char *job_id, dstring *job_id_out, int *stat,
              signed long timeout, int event_mask, int *event,
              drmaa_attr_values_t **rusage, dstring *diag);

/* 
 * Evaluates into 'exited' a non-zero value if stat was returned for a
 * job that terminated normally. A zero value can also indicate that
 * altough the job has terminated normally an exit status is not available
 * or that it is not known whether the job terminated normally. In both
 * cases japi_wexitstatus() will not provide exit status information.
 * A non-zero 'exited' value indicates more detailed diagnosis can be provided
 * by means of japi_wifsignaled(), japi_wtermsig() and japi_wcoredump(). 
 */
int japi_wifexited(int *exited, int stat, dstring *diag);

/* 
 * If the OUT parameter 'exited' of japi_wifexited() is non-zero,
 * this function evaluates into 'exit_code' the exit code that the
 * job passed to _exit() (see exit(2)) or exit(3C), or the value that
 * the child process returned from main. 
 */
int japi_wexitstatus(int *exit_status, int stat, dstring *diag);

/* 
 * Evaluates into 'signaled' a non-zero value if status was returned
 * for a job that terminated due to the receipt of a signal. A zero value
 * can also indicate that altough the job has terminated due to the receipt
 * of a signal the signal is not available or that it is not known whether
 * the job terminated due to the receipt of a signal. In both cases
 * japi_wtermsig() will not provide signal information. 
 */
int japi_wifsignaled(int *signaled, int stat, dstring *diag);

/* 
 * If the OUT parameter 'signaled' of japi_wifsignaled(stat) is
 * non-zero, this function evaluates into signal a string representation of the signal
 * that caused the termination of the job. For signals declared by POSIX, the symbolic
 * names are returned (e.g., SIGABRT, SIGALRM).
 * For signals not declared by POSIX, any other string may be returned. 
 */
int japi_wtermsig(dstring *signal, int stat, dstring *diag);

/* 
 * If the OUT parameter 'signaled' of japi_wifsignaled(stat) is
 * non-zero, this function evaluates into 'core_dumped' a non-zero value
 * if a core image of the terminated job was created. 
 */
int japi_wifcoredump(int *core_dumped, int stat, dstring *diag);

/* 
 * Evaluates into 'aborted' a non-zero value if 'stat'
 * was returned for a job that ended before entering the running state. 
 */
int japi_wifaborted(int *aborted, int stat, dstring *diag);



/* 
 * Get the program status of the job identified by 'job_id'.
 * The possible values returned in 'remote_ps' and their meanings are:
 * DRMAA_PS_UNDETERMINED = 00H : process status cannot be determined,
 * DRMAA_PS_QUEUED_ACTIVE = 10H : job is queued and active,
 * DRMAA_PS_SYSTEM_ON_HOLD = 11H : job is queued and in system hold,
 * DRMAA_PS_USER_ON_HOLD = 12H : job is queued and in user hold,
 * DRMAA_PS_USER_SYSTEM_ON_HOLD = 13H : job is queued and in user and system hold,
 * DRMAA_PS_RUNNING = 20H : job is running,
 * DRMAA_PS_SYSTEM_SUSPENDED = 21H : job is system suspended,
 * DRMAA_PS_USER_SUSPENDED = 22H : job is user suspended,
 * DRMAA_PS_USER_SYSTEM_SUSPENDED = 23H : job is user and system suspended,
 * DRMAA_PS_DONE = 30H : job finished normally, and
 * DRMAA_PS_FAILED = 40H : job finished, but failed.
 */
int japi_job_ps( const char *job_id, int *remote_ps, dstring *diag);

/* ------------------- auxiliary routines ------------------- */
 
/*
 * Get the error message text associated with the errno number. 
 */
const char *japi_strerror(int drmaa_errno);

/* 
 * Current contact information for DRM system (string)
 */ 
int japi_get_contact(dstring *contact, dstring *diag);

/* 
 * OUT major - major version number (non-negative integer)
 * OUT minor - minor version number (non-negative integer)
 * Returns the major and minor version numbers of the DRMAA library;
 * for DRMAA 1.0, 'major' is 1 and 'minor' is 0. 
 */
void japi_version(unsigned int *major, unsigned int *minor);


/* 
 * returns DRM system implementation information
 * Output (string) is implementation dependent and could contain the DRM system and the
 * implementation vendor as its parts.
 */
int japi_get_drm_system(dstring *drm, dstring *diag, int me);

/* get next string attribute from iterator 
 * DRMAA_ERRNO_SUCCESS or DRMAA_ERRNO_NO_MORE_ELEMENTS if no such exists */
int japi_string_vector_get_next(drmaa_attr_values_t* values, dstring *val);

/* Get the number of elements from iterator */
int japi_string_vector_get_num(drmaa_attr_values_t* values, int *size);

/* release opaque iterator */
void japi_delete_string_vector(drmaa_attr_values_t* values);


void japi_standard_error(int drmaa_errno, dstring *ds);
drmaa_attr_values_t *japi_allocate_string_vector(int type); 
int japi_init_mt(dstring *diag);

bool japi_is_delegated_file_staging_enabled(dstring *diag);

#ifdef  __cplusplus
}
#endif

#endif /* __JAPI_H */
