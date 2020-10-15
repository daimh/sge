#ifndef __DRMAA_H
#define __DRMAA_H

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

#include <string.h>             /* for size_t */

#ifdef  __cplusplus
extern "C" {
#endif

/* see www.drmaa.org for more details on the DRMAA specification */ 
/****** DRMAA/-DRMAA_Interface *************************************************
*  NAME
*     DRMAA_Interface -- DRMAA interface
*
*  FUNCTION
*     The enlisted functions specify the C/C++ binding of the DRMAA interface 
*     specification.
* 
*  SEE ALSO
*     DRMAA/drmaa_get_next_attr_name()
*     DRMAA/drmaa_get_next_attr_value()
*     DRMAA/drmaa_get_next_job_id()
*     DRMAA/drmaa_release_attr_names()
*     DRMAA/drmaa_release_attr_values()
*     DRMAA/drmaa_release_job_ids()
*     DRMAA/drmaa_init()
*     DRMAA/drmaa_exit()
*     DRMAA/drmaa_allocate_job_template()
*     DRMAA/drmaa_delete_job_template()
*     DRMAA/drmaa_set_attribute()
*     DRMAA/drmaa_get_attribute()
*     DRMAA/drmaa_set_vector_attribute()
*     DRMAA/drmaa_get_vector_attribute()
*     DRMAA/drmaa_get_attribute_names()
*     DRMAA/drmaa_get_vector_attribute_names()
*     DRMAA/drmaa_run_job()
*     DRMAA/drmaa_run_bulk_jobs()
*     DRMAA/drmaa_control()
*     DRMAA/drmaa_synchronize()
*     DRMAA/drmaa_wait()
*     DRMAA/drmaa_wifexited()
*     DRMAA/drmaa_wexitstatus()
*     DRMAA/drmaa_wifsignaled()
*     DRMAA/drmaa_wtermsig()
*     DRMAA/drmaa_wcoredump()
*     DRMAA/drmaa_wifaborted()
*     DRMAA/drmaa_job_ps()
*     DRMAA/drmaa_strerror()
*     DRMAA/drmaa_get_contact()
*     DRMAA/drmaa_version()
*     DRMAA/drmaa_get_DRM_system()
*******************************************************************************/

/* ------------------- Constants ------------------- */
/* 
 * some not yet agreed buffer length constants 
 * these are recommended minimum values 
 */

/* drmaa_get_attribute() */
#define DRMAA_ATTR_BUFFER 1024

/* drmaa_get_contact() */
#define DRMAA_CONTACT_BUFFER 1024

/* drmaa_get_DRM_system() */
#define DRMAA_DRM_SYSTEM_BUFFER 1024

/* drmaa_get_DRM_system() */
#define DRMAA_DRMAA_IMPLEMENTATION_BUFFER 1024

/* 
 * Agreed buffer length constants 
 * these are recommended minimum values 
 */
#define DRMAA_ERROR_STRING_BUFFER   1024
#define DRMAA_JOBNAME_BUFFER        1024
#define DRMAA_SIGNAL_BUFFER         32

/*
 * Agreed constants 
 */ 
#define DRMAA_TIMEOUT_WAIT_FOREVER  -1
#define DRMAA_TIMEOUT_NO_WAIT       0 

#define DRMAA_JOB_IDS_SESSION_ANY   "DRMAA_JOB_IDS_SESSION_ANY"
#define DRMAA_JOB_IDS_SESSION_ALL   "DRMAA_JOB_IDS_SESSION_ALL"

#define DRMAA_SUBMISSION_STATE_ACTIVE "drmaa_active"
#define DRMAA_SUBMISSION_STATE_HOLD   "drmaa_hold"

/*
 * Agreed placeholder names
 */
#define DRMAA_PLACEHOLDER_INCR       "$drmaa_incr_ph$"
#define DRMAA_PLACEHOLDER_HD         "$drmaa_hd_ph$"
#define DRMAA_PLACEHOLDER_WD         "$drmaa_wd_ph$"

/*
 * Agreed names of job template attributes 
 */
#define DRMAA_REMOTE_COMMAND         "drmaa_remote_command"
#define DRMAA_JS_STATE               "drmaa_js_state"
#define DRMAA_WD                     "drmaa_wd"
#define DRMAA_JOB_CATEGORY           "drmaa_job_category"
#define DRMAA_NATIVE_SPECIFICATION   "drmaa_native_specification"
#define DRMAA_BLOCK_EMAIL            "drmaa_block_email"
#define DRMAA_START_TIME             "drmaa_start_time"
#define DRMAA_JOB_NAME               "drmaa_job_name"
#define DRMAA_INPUT_PATH             "drmaa_input_path"
#define DRMAA_OUTPUT_PATH            "drmaa_output_path"
#define DRMAA_ERROR_PATH             "drmaa_error_path"
#define DRMAA_JOIN_FILES             "drmaa_join_files"
#define DRMAA_TRANSFER_FILES         "drmaa_transfer_files"
#define DRMAA_DEADLINE_TIME          "drmaa_deadline_time"
#define DRMAA_WCT_HLIMIT             "drmaa_wct_hlimit"
#define DRMAA_WCT_SLIMIT             "drmaa_wct_slimit"
#define DRMAA_DURATION_HLIMIT        "drmaa_duration_hlimit"
#define DRMAA_DURATION_SLIMIT        "drmaa_duration_slimit"
#define DRMAA_SUBMIT_AS_EUID         "drmaa_submit_as_euid"


/* names of job template vector attributes */
#define DRMAA_V_ARGV                 "drmaa_v_argv"
#define DRMAA_V_ENV                  "drmaa_v_env"
#define DRMAA_V_EMAIL                "drmaa_v_email"

/* 
 * DRMAA errno values 
 *
 * do not touch these values are agreed !!!
 */
enum {
   /* -------------- these are relevant to all sections ---------------- */
   DRMAA_ERRNO_SUCCESS = 0, /* Routine returned normally with success. */
   DRMAA_ERRNO_INTERNAL_ERROR, /* Unexpected or internal DRMAA error like memory
                                  allocation, system call failure, etc. */
   DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE, /* Could not contact DRM system for
                                             this request. */
   DRMAA_ERRNO_AUTH_FAILURE, /* The specified request is not processed
                                successfully due to authorization failure. */
   DRMAA_ERRNO_INVALID_ARGUMENT, /* The input value for an argument is
                                    invalid. */
   DRMAA_ERRNO_NO_ACTIVE_SESSION, /* Exit routine failed because there is no
                                     active session */
   DRMAA_ERRNO_NO_MEMORY, /* failed allocating memory */

   /* -------------- init and exit specific --------------- */
   DRMAA_ERRNO_INVALID_CONTACT_STRING, /* Initialization failed due to invalid
                                          contact string. */
   DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR, /* DRMAA could not use the default
                                                contact string to connect to DRM
                                                      system. */
   DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED, /* No default contact string
                                                      was provided or selected.
                                                      DRMAA requires that the
                                                      default contact string is
                                                      selected when there is
                                                      more than one default
                                                      contact string due to
                                                      multiple DRMAA
                                                      implementation contained
                                                      in the binary module. */
   DRMAA_ERRNO_DRMS_INIT_FAILED, /* Initialization failed due to failure to init
                                    DRM system. */
   DRMAA_ERRNO_ALREADY_ACTIVE_SESSION, /* Initialization failed due to existing
                                          DRMAA session. */
   DRMAA_ERRNO_DRMS_EXIT_ERROR, /* DRM system disengagement failed. */

   /* ---------------- job attributes specific -------------- */
   DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT, /* The format for the job attribute
                                            value is invalid. */
   DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE, /* The value for the job attribute is
                                           invalid. */
   DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES, /* The value of this attribute is
                                                conflicting with a previously
                                                set attributes. */

   /* --------------------- job submission specific -------------- */
   DRMAA_ERRNO_TRY_LATER, /* Could not pass job now to DRM system. A retry may
                             succeed however (saturation). */
   DRMAA_ERRNO_DENIED_BY_DRM, /* The DRM system rejected the job. The job will
                                 never be accepted due to DRM configuration or
                                 job template settings. */

   /* ------------------------------- job control specific ---------------- */
   DRMAA_ERRNO_INVALID_JOB, /* The job specified by the 'jobid' does not
                               exist. */
   DRMAA_ERRNO_RESUME_INCONSISTENT_STATE, /* The job has not been suspended. The
                                             RESUME request will not be
                                             processed. */
   DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE, /* The job has not been running, and
                                              it cannot be suspended. */
   DRMAA_ERRNO_HOLD_INCONSISTENT_STATE, /* The job cannot be moved to a HOLD
                                           state. */
   DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE, /* The job is not in a HOLD state. */
   DRMAA_ERRNO_EXIT_TIMEOUT, /* We have encountered a time-out condition for
                                drmaa_synchronize or drmaa_wait. */
   DRMAA_ERRNO_NO_RUSAGE, /* This error code is returned by drmaa_wait() when a
                             job has finished but no rusage and stat data could
                             be provided. */
   DRMAA_ERRNO_NO_MORE_ELEMENTS, /* There are no more elements in the opaque
                                    string vector. */

   DRMAA_NO_ERRNO
};

/* 
 * Agreed DRMAA job states as returned by drmaa_job_ps() 
 */
enum {
 DRMAA_PS_UNDETERMINED          = 0x00, /* process status cannot be
                                           determined */
 DRMAA_PS_QUEUED_ACTIVE         = 0x10, /* job is queued and active */
 DRMAA_PS_SYSTEM_ON_HOLD        = 0x11, /* job is queued and in system hold */
 DRMAA_PS_USER_ON_HOLD          = 0x12, /* job is queued and in user hold */
 DRMAA_PS_USER_SYSTEM_ON_HOLD   = 0x13, /* job is queued and in user and system
                                           hold */
 DRMAA_PS_RUNNING               = 0x20, /* job is running */
 DRMAA_PS_SYSTEM_SUSPENDED      = 0x21, /* job is system suspended */
 DRMAA_PS_USER_SUSPENDED        = 0x22, /* job is user suspended */
 DRMAA_PS_USER_SYSTEM_SUSPENDED = 0x23, /* job is user and system suspended */
 DRMAA_PS_DONE                  = 0x30, /* job finished normally */
 DRMAA_PS_FAILED                = 0x40  /* job finished, but failed */
};

/* 
 * Agreed DRMAA actions for drmaa_control() 
 */
enum {
 DRMAA_CONTROL_SUSPEND = 0,
 DRMAA_CONTROL_RESUME,
 DRMAA_CONTROL_HOLD,
 DRMAA_CONTROL_RELEASE,
 DRMAA_CONTROL_TERMINATE
};

/* ------------------- Data types ------------------- */
/* 
 * Agreed opaque DRMAA job template 
 * struct drmaa_job_template_s is in japiP.h
 */
typedef struct drmaa_job_template_s drmaa_job_template_t;

/* ---------- C/C++ language binding specific interfaces -------- */

typedef struct drmaa_attr_names_s drmaa_attr_names_t;
typedef struct drmaa_attr_values_s drmaa_attr_values_t;
typedef struct drmaa_job_ids_s  drmaa_job_ids_t;

/*
 * get next string attribute from iterator 
 * 
 * returns DRMAA_ERRNO_SUCCESS or DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE 
 * if no such exists 
 */

int drmaa_get_next_attr_name(drmaa_attr_names_t* values, char *value,
                             size_t value_len);
int drmaa_get_next_attr_value(drmaa_attr_values_t* values, char *value,
                              size_t value_len);
int drmaa_get_next_job_id(drmaa_job_ids_t* values, char *value,
                          size_t value_len);

/*
 * get element count of opaque string vector
 *
 * Gives the number of elements in the opaque string vector.  Useful for
 * copying the contents into an array.
 */
int drmaa_get_num_attr_names(drmaa_attr_names_t* values, int *size);
int drmaa_get_num_attr_values(drmaa_attr_values_t* values, int *size);
int drmaa_get_num_job_ids(drmaa_job_ids_t* values, int *size);

/* 
 * release opaque string vector 
 *
 * Opaque string vectors can be used without any constraint
 * until the release function has been called.
 */
void drmaa_release_attr_names(drmaa_attr_names_t* values);
void drmaa_release_attr_values(drmaa_attr_values_t* values);
void drmaa_release_job_ids(drmaa_job_ids_t* values);

/* ------------------- init/exit routines ------------------- */
/*
 * Initialize DRMAA API library and create a new DRMAA Session. 'Contact'
 * is an implementation dependent string which MAY be used to specify
 * which DRM system to use. This routine MUST be called before any
 * other DRMAA calls, except for drmaa_version().
 * If 'contact' is NULL, the default DRM system SHALL be used provided there is
 * only one DRMAA implementation in the provided binary module.  When these is
 * more than one DRMAA implementation in the binary module, drmaa_init() SHALL
 * return the DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED error. drmaa_init()
 * SHOULD be called by only one of the threads. The main thread is RECOMMENDED.
 * A call by another thread SHALL return DRMAA_ERRNO_ALREADY_ACTIVE_SESSION.
 * When 'contact' is a a semi-colon separated list of name=value strings, the
 * strings will be parsed and interpreted.  The current list of accepted names
 * is:
 *    session -- the id of the session to which to reconnect
#if 0
 *    sge_root -- the SGE_ROOT to use
 *    sge_cell -- the SGE_CELL to use
#endif
 *
 * drmaa_init() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_INVALID_CONTACT_STRING,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_ALREADY_ACTIVE_SESSION,
 *    DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED, or
 *    DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR.
 */ 
int drmaa_init(const char *contact, char *error_diagnosis,
               size_t error_diag_len);


/*
 * Disengage from DRMAA library and allow the DRMAA library to perform
 * any necessary internal clean up.
 * This routine SHALL end the current DRMAA Session, but SHALL NOT effect any
 * jobs (e.g., queued and running jobs SHALL remain queued and running).
 * drmaa_exit() SHOULD be called by only one of the threads. Other thread calls
 * to drmaa_exit() MAY fail since there is no active session.
 *
 * drmaa_exit() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_DRMS_EXIT_ERROR or
 *    DRMAA_ERRNO_NO_ACTIVE_SESSION.
 */
int drmaa_exit(char *error_diagnosis, size_t error_diag_len);

/* ------------------- job template routines ------------------- */

/* 
 * Allocate a new job template.
 *
 * drmaa_allocate_job_template() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE,
 *    DRMAA_ERRNO_INTERNAL_ERROR or
 *    DRMAA_ERRNO_NO_MEMORY.
 */
int drmaa_allocate_job_template(drmaa_job_template_t **jt,
                                char *error_diagnosis, size_t error_diag_len);

/* 
 * Deallocate a job template. This routine has no effect on jobs.
 *
 * drmaa_delete_job_template() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE or
 *    DRMAA_ERRNO_INTERNAL_ERROR.
 */
int drmaa_delete_job_template(drmaa_job_template_t *jt, char *error_diagnosis,
                              size_t error_diag_len);


/* 
 * Adds ('name', 'value') pair to list of attributes in job template 'jt'.
 * Only non-vector attributes SHALL be passed.
 *
 * drmaa_set_attribute() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT,
 *    DRMAA_ERRNO_INVALID_ARGUMENT,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE or
 *    DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES.
 */
int drmaa_set_attribute(drmaa_job_template_t *jt, const char *name,
                        const char *value, char *error_diagnosis,
                        size_t error_diag_len);


/* 
 * If 'name' is an existing non-vector attribute name in the job 
 * template 'jt', then the value of 'name' SHALL be returned; otherwise, 
 * NULL is returned.
 *
 * drmaa_get_attribute() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE.
 */ 
int drmaa_get_attribute(drmaa_job_template_t *jt, const char *name, char *value,
                        size_t value_len, char *error_diagnosis,
                        size_t error_diag_len);

/* Adds ('name', 'values') pair to list of vector attributes in job template
 * 'jt'. Only vector attributes SHALL be passed.
 * A 'value' string vector containing n elements must be n+1 elements long, with
 * the nth value, i.e. value[n], being set to NULL as a delimitor.
 *
 * drmaa_set_vector_attribute() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT,
 *    DRMAA_ERRNO_INVALID_ARGUMENT,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES.
 */
int drmaa_set_vector_attribute(drmaa_job_template_t *jt, const char *name,
                               const char *value[], char *error_diagnosis,
                               size_t error_diag_len);


/* 
 * If 'name' is an existing vector attribute name in the job template 'jt',
 * then the values of 'name' are returned; otherwise, NULL is returned.
 *
 * drmaa_get_vector_attribute() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE.
 */
int drmaa_get_vector_attribute(drmaa_job_template_t *jt, const char *name,
                               drmaa_attr_values_t **values,
                               char *error_diagnosis, size_t error_diag_len);


/* 
 * SHALL return the set of supported attribute names whose associated   
 * value type is String. This set SHALL include supported DRMAA reserved 
 * attribute names and native attribute names. 
 *
 * drmaa_get_attribute_names() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_NO_MEMORY.
 */
int drmaa_get_attribute_names(drmaa_attr_names_t **values,
                              char *error_diagnosis, size_t error_diag_len);

/*
 * SHALL return the set of supported attribute names whose associated 
 * value type is String Vector.  This set SHALL include supported DRMAA reserved 
 * attribute names and native attribute names.
 *
 * drmaa_get_vector_attribute_names() SHALL return DRMAA_ERRNO_SUCCESS on 
 * success, otherwise:
 *    DRMAA_ERRNO_NO_MEMORY.
 */
int drmaa_get_vector_attribute_names(drmaa_attr_names_t **values,
                                     char *error_diagnosis,
                                     size_t error_diag_len);

/* ------------------- job submission routines ------------------- */

/*
 * Submit a job with attributes defined in the job template 'jt'.
 * The job identifier 'job_id' is a printable, NULL terminated string,
 * identical to that returned by the underlying DRM system.
 *
 * drmaa_run_job() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_TRY_LATER,
 *    DRMAA_ERRNO_DENIED_BY_DRM,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE or
 *    DRMAA_ERRNO_AUTH_FAILURE.
 */
int drmaa_run_job(char *job_id, size_t job_id_len,
                  const drmaa_job_template_t *jt, char *error_diagnosis,
                  size_t error_diag_len);

/* 
 * Submit a set of parametric jobs, dependent on the implied loop index, each
 * with attributes defined in the job template 'jt'.
 * The job identifiers 'job_ids' SHALL all be printable,
 * NULL terminated strings, identical to those returned by the underlying
 * DRM system. Nonnegative loop bounds SHALL NOT use file names
 * that start with minus sign like command line options.
 * DRMAA defines a special index placeholder, drmaa_incr_ph, (which has the
 * value "$incr_pl$") that is used to construct parametric job templates.
 * For example:
 * //C++ string syntax used
 * drmaa_set_attribute(pjt, "stderr", drmaa_incr_ph + ".err" );
 *
 * drmaa_run_bulk_jobs() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_TRY_LATER,
 *    DRMAA_ERRNO_DENIED_BY_DRM,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE or
 *    DRMAA_ERRNO_AUTH_FAILURE.
 */
int drmaa_run_bulk_jobs(drmaa_job_ids_t **jobids,
                        const drmaa_job_template_t *jt, unsigned start, unsigned end,
                        int incr, char *error_diagnosis, size_t error_diag_len);

/* ------------------- job control routines ------------------- */

/*
 * Start, stop, restart, or kill the job identified by 'job_id'.
 * If 'job_id' is DRMAA_JOB_IDS_SESSION_ALL, then this routine
 * acts on all jobs *submitted* during this DRMAA session.
 * The legal values for 'action' and their meanings SHALL be:
 * DRMAA_CONTROL_SUSPEND:     stop the job,
 * DRMAA_CONTROL_RESUME:      (re)start the job,
 * DRMAA_CONTROL_HOLD:        put the job on-hold,
 * DRMAA_CONTROL_RELEASE:     release the hold on the job, and
 * DRMAA_CONTROL_TERMINATE:   kill the job.
 *
 * This routine SHALL return once the action has been acknowledged by
 * the DRM system, but does not necessarily wait until the action
 * has been completed.
 *
 * drmaa_control() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE,
 *    DRMAA_ERRNO_AUTH_FAILURE,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_RESUME_INCONSISTENT_STATE,
 *    DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE,
 *    DRMAA_ERRNO_HOLD_INCONSISTENT_STATE,
 *    DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE or
 *    DRMAA_ERRNO_INVALID_JOB.
 */
int drmaa_control(const char *jobid, int action, char *error_diagnosis,
                  size_t error_diag_len);


/* 
 * Wait until all jobs specified by 'job_ids' have finished
 * execution. If 'job_ids' is DRMAA_JOB_IDS_SESSION_ALL, then this routine
 * waits for all jobs *submitted* during this DRMAA session. The timeout value
 * is used to specify the number of seconds to wait for the job to fail finish
 * before returning if a result is not immediately available.  The value
 * DRMAA_TIMEOUT_WAIT_FOREVER can be used to specify that routine should wait
 * indefinitely for a result. The value DRMAA_TIMEOUT_NO_WAIT can be used to
 * specify that the routine should return immediately if no result is available.
 * If the call exits before timeout, all the jobs have
 * been waited on or there was an interrupt.
 * If the invocation exits on timeout, the return code is
 * DRMAA_ERRNO_EXIT_TIMEOUT. The caller SHOULD check system time before and
 * after this call in order to check how much time has passed.
 * 
 * The dispose parameter specifies how to treat reaping information:
 * True=1      "fake reap", i.e. dispose of the rusage data
 * False=0     do not reap
 * 
 * A 'job_ids' string vector containing n elements must be n+1 elements long,
 * with the nth value, i.e. job_ids[n], being set to NULL as a delimitor.
 *
 * drmaa_synchronize() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE,
 *    DRMAA_ERRNO_AUTH_FAILURE,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_EXIT_TIMEOUT or
 *    DRMAA_ERRNO_INVALID_JOB.
 */ 
int drmaa_synchronize(const char *job_ids[], signed long timeout, int dispose,
                      char *error_diagnosis, size_t error_diag_len);


/* 
 * This routine SHALL wait for a job with job_id to fail or finish execution. If
 * the special string, DRMAA_JOB_IDS_SESSION_ANY, is provided as the job_id,
 * this routine SHALL wait for any job from the session. This routine is modeled
 * on the wait3 POSIX routine. The timeout value is used to specify the number
 * of seconds to wait for the job to fail finish before returning if a result is
 * not immediately available.  The value DRMAA_TIMEOUT_WAIT_FOREVER can be
 * used to specify that routine should wait indefinitely for a result. The value
 * DRMAA_TIMEOUT_NO_WAIT may be specified that the routine should return
 * immediately if no result is available.
 * If the call exits before timeout ,the job has been waited on
 * successfully or there was an interrupt.
 * If the invocation exits on timeout, the return code is
 * DRMAA_ERRNO_EXIT_TIMEOUT. The caller SHOULD check system time before and
 * after this call in order to check how much time has passed.
 * The routine reaps jobs on a successful call, so any subsequent calls
 * to drmaa_wait SHOULD fail returning an error DRMAA_ERRNO_INVALID_JOB meaning
 * that the job has been already reaped. This error is the same as if the job
 * was unknown. Failing due to an elapsed timeout has an effect that it is
 * possible to issue drmaa_wait multiple times for the same job_id.  When
 * successful, the rusage information SHALL be provided as an array of strings,
 * where each string complies with the format <name>=<value>. The string portion
 * <value> contains the amount of resources consumed by the job and is opaque.
 * The 'stat' drmaa_wait parameter is used in the drmaa_w* functions for
 * providing more detailed information about job termination if available. An
 * analogous set of macros is defined in POSIX for analyzing the wait3(2) OUT
 * parameter 'stat'.
 *
 * drmaa_wait() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE,
 *    DRMAA_ERRNO_AUTH_FAILURE,
 *    DRMAA_ERRNO_NO_RUSAGE,
 *    DRMAA_ERRNO_NO_MEMORY,
 *    DRMAA_ERRNO_EXIT_TIMEOUT or
 *    DRMAA_ERRNO_INVALID_JOB.
 */
int drmaa_wait(const char *job_id, char *job_id_out, size_t job_id_out_len,
               int *stat, signed long timeout, drmaa_attr_values_t **rusage, 
               char *error_diagnosis, size_t error_diag_len);

/* 
 * Evaluates into 'exited' a non-zero value if stat was returned for a
 * job that terminated normally. A zero value can also indicate that
 * altough the job has terminated normally an exit status is not available
 * or that it is not known whether the job terminated normally. In both
 * cases drmaa_wexitstatus() SHALL NOT provide exit status information.
 * A non-zero 'exited' value indicates more detailed diagnosis can be provided
 * by means of drmaa_wifsignaled(), drmaa_wtermsig() and drmaa_wcoredump(). 
 */
int drmaa_wifexited(int *exited, int stat, char *error_diagnosis,
                    size_t error_diag_len);

/* 
 * If the OUT parameter 'exited' of drmaa_wifexited() is non-zero,
 * this function evaluates into 'exit_code' the exit code that the
 * job passed to _exit() (see exit(2)) or exit(3C), or the value that
 * the child process returned from main. 
 */
int drmaa_wexitstatus(int *exit_status, int stat, char *error_diagnosis,
                      size_t error_diag_len);

/* 
 * Evaluates into 'signaled' a non-zero value if status was returned
 * for a job that terminated due to the receipt of a signal. A zero value
 * can also indicate that altough the job has terminated due to the receipt
 * of a signal the signal is not available or that it is not known whether
 * the job terminated due to the receipt of a signal. In both cases
 * drmaa_wtermsig() SHALL NOT provide signal information. 
 */
int drmaa_wifsignaled(int *signaled, int stat, char *error_diagnosis,
                      size_t error_diag_len);

/* 
 * If the OUT parameter 'signaled' of drmaa_wifsignaled(stat) is
 * non-zero, this function evaluates into signal a string representation of the
 * signal that caused the termination of the job. For signals declared by POSIX,
 * the symbolic names SHALL be returned (e.g., SIGABRT, SIGALRM).
 * For signals not declared by POSIX, any other string MAY be returned. 
 */
int drmaa_wtermsig(char *signal, size_t signal_len, int stat,
                   char *error_diagnosis, size_t error_diag_len);

/* 
 * If the OUT parameter 'signaled' of drmaa_wifsignaled(stat) is
 * non-zero, this function evaluates into 'core_dumped' a non-zero value
 * if a core image of the terminated job was created. 
 */
int drmaa_wcoredump(int *core_dumped, int stat, char *error_diagnosis,
                    size_t error_diag_len);

/* 
 * Evaluates into 'aborted' a non-zero value if 'stat'
 * was returned for a job that ended before entering the running state. 
 */
int drmaa_wifaborted(int *aborted, int stat, char *error_diagnosis,
                     size_t error_diag_len);



/* 
 * Get the program status of the job identified by 'job_id'.
 * The possible values returned in 'remote_ps' and their meanings SHALL be:
 *
 * DRMAA_PS_UNDETERMINED          = 0x00: process status cannot be determined
 * DRMAA_PS_QUEUED_ACTIVE         = 0x10: job is queued and active
 * DRMAA_PS_SYSTEM_ON_HOLD        = 0x11: job is queued and in system hold
 * DRMAA_PS_USER_ON_HOLD          = 0x12: job is queued and in user hold
 * DRMAA_PS_USER_SYSTEM_ON_HOLD   = 0x13: job is queued and in user and system
 *                                        hold
 * DRMAA_PS_RUNNING               = 0x20: job is running
 * DRMAA_PS_SYSTEM_SUSPENDED      = 0x21: job is system suspended
 * DRMAA_PS_USER_SUSPENDED        = 0x22: job is user suspended
 * DRMAA_PS_USER_SYSTEM_SUSPENDED = 0x23: job is user and system suspended
 * DRMAA_PS_DONE                  = 0x30: job finished normally
 * DRMAA_PS_FAILED                = 0x40: job finished, but failed
 *
 * DRMAA SHOULD always get the status of job_id from DRM system, unless the
 * previous status has been DRMAA_PS_FAILED or DRMAA_PS_DONE and the status has
 * been successfully cached. Terminated jobs get DRMAA_PS_FAILED status.
 *
 * drmaa_synchronize() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE,
 *    DRMAA_ERRNO_AUTH_FAILURE,
 *    DRMAA_ERRNO_NO_MEMORY or
 *    DRMAA_ERRNO_INVALID_JOB.
 */
int drmaa_job_ps(const char *job_id, int *remote_ps, char *error_diagnosis,
                 size_t error_diag_len);

/* ------------------- auxiliary routines ------------------- */
 
/*
 * SHALL return the error message text associated with the errno number. The
 * routine SHALL return null string if called with invalid ERRNO number.
 */
const char *drmaa_strerror(int drmaa_errno);

/* 
 * If called before drmaa_init(), it SHALL return a comma delimited default
 * DRMAA implementation contacts string, one per each DRM system provided
 * implementation. If called after drmaa_init(), it SHALL return the selected
 * contact string. The output string is Implementation dependent.
 * drmaa_get_contact() SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_INTERNAL_ERROR.
 */ 
int drmaa_get_contact(char *contact, size_t contact_len, 
         char *error_diagnosis, size_t error_diag_len);

/* 
 * OUT major - major version number (non-negative integer)
 * OUT minor - minor version number (non-negative integer)
 * SHALL return the major and minor version numbers of the DRMAA library;
 * for DRMAA 1.0, 'major' is 1 and 'minor' is 0.
 * Return codes:
 * DRMAA_ERRNO_SUCCESS -- success.
 * DRMAA_ERRNO_INTERNAL_ERROR -- unexpected or internal error.
 * DRMAA_ERRNO_INVALID_ARGUMENT -- an argument value is invalid.
 * DRMAA_ERRNO_NO_MEMORY -- not enough free memory to perform the operation.
 * DRMAA_ERRNO_NO_ACTIVE_SESSION -- no active session.
 */
int drmaa_version(unsigned int *major, unsigned int *minor, 
         char *error_diagnosis, size_t error_diag_len);


/* 
 * If called before drmaa_init(), it SHALL return a comma delimited DRM systems
 * string, one per each DRM system provided implementation. If called after
 * drmaa_init(), it SHALL return the selected DRM system. The output string is
 * implementation dependent.
 *
 * drmaa_get_DRM_system() SHALL return DRMAA_ERRNO_SUCCESS on success,
 * otherwise:
 *    DRMAA_ERRNO_INTERNAL_ERROR.
 */
int drmaa_get_DRM_system(char *drm_system, size_t drm_system_len, 
         char *error_diagnosis, size_t error_diag_len);


/* 
 * If called before drmaa_init(), it SHALL return a comma delimited DRMAA
 * implementations string, one per each DRM system provided implementation. If
 * called after drmaa_init(), it SHALL return the selected DRMAA implementation.
 * The output (string) is implementation dependent. drmaa_get_DRM_implementation
 * routine SHALL return DRMAA_ERRNO_SUCCESS on success, otherwise:
 *    DRMAA_ERRNO_INTERNAL_ERROR.
 */
int drmaa_get_DRMAA_implementation(char *drmaa_impl, size_t drmaa_impl_len, 
         char *error_diagnosis, size_t error_diag_len);

#ifdef  __cplusplus
}
#endif

#endif /* __DRMAA_H */
