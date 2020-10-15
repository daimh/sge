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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


#include <pthread.h>

/* this timeout is in effect with SGE commprocs */
#define SGE_COMMPROC_TIMEOUT 60*5

#include "japi/drmaa.h"
#include "japi/japi.h"
#include "japi/japiP.h"

#include "cull/cull_list.h"

#include "sgeobj/sge_job.h"
#include "sgeobj/sge_answer.h"

#include "uti/sge_rmon_monitoring_level.h"
#include "uti/sge_rmon.h"
#include "uti/sge_profiling.h"
#include "uti/sge_stdio.h"

#include "comm/commlib.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdi.h"

#include "show_job.h"
#include "msg_common.h"




#define JOB_CHUNK 8
#define NTHREADS 3
#define NBULKS 3

#define NEXT_ARGV(argc, argv) \
      ((*argc)--, (*argv)++, (*argv)[0])

enum {
   ALL_TESTS = 0,    

   ST_SUBMIT_WAIT,    
      /* - one thread 
         - submit jobs 
         - wait for jobend */

   ST_SUBMIT_NO_RUN_WAIT,    
      /* - one thread 
         - submit jobs that won't run
         - wait for jobend */

   MT_SUBMIT_WAIT,        
      /* - multiple submission threads
         - wait is done by main thread */

   MT_SUBMIT_BEFORE_INIT_WAIT,
      /* - no drmaa_init() was called
         - multiple threads try to submit but fail
         - when drmaa_init() is called by main thread
           submission proceed
         - wait is done by main thread */

   ST_MULT_INIT,
      /* - drmaa_init() is called multiple times 
         - first time it must succeed - second time it must fail
         - then drmaa_exit() is called */

   ST_MULT_EXIT,
      /* - drmaa_init() is called
         - then drmaa_exit() is called multiple times
         - first time it must succeed - second time it must fail */

   MT_EXIT_DURING_SUBMIT,
      /* - drmaa_init() is called
         - multiple submission threads submitting (delayed) a series 
           of jobs
         - during submission main thread does drmaa_exit() */

   MT_SUBMIT_MT_WAIT,
      /* - drmaa_init() is called
         - multiple submission threads submit jobs and wait these jobs
         - when all threads are finished main thread calls drmaa_exit() */

   MT_EXIT_DURING_SUBMIT_OR_WAIT,
      /* - drmaa_init() is called
         - multiple submission threads submit jobs and wait these jobs
         - while submission threads are waiting their jobs the main 
           thread calls drmaa_exit() */

   ST_BULK_SUBMIT_WAIT,
      /* - drmaa_init() is called
         - a bulk job is submitted and waited 
         - then drmaa_exit() is called */

   ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL,
      /* - drmaa_init() is called
         - bulk and sequential jobs are submitted
         - all jobs are waited individually
         - then drmaa_exit() is called */

   ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE,
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) 
           to wait for all jobs to finish
         - then drmaa_exit() is called */

   ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE,
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, no-dispose) 
           to wait for all jobs to finish
         - do drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY) until 
           DRMAA_ERRNO_INVALID_JOB to reap all jobs
         - then drmaa_exit() is called */

   ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE,
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(all_jobids, dispose) 
           to wait for all jobs to finish
         - then drmaa_exit() is called */

   ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE,
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(all_jobids, no-dispose) 
           to wait for all jobs to finish
         - do drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY) until 
           DRMAA_ERRNO_INVALID_JOB to reap all jobs
         - then drmaa_exit() is called */


   ST_SUBMIT_PAUSE_SUBMIT_SYNC,
      /* - drmaa_init() is called
         - a job is submitted 
         - do a long sleep(SGE_COMMPROC_TIMEOUT+)
         - another job is submitted 
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) 
         - then drmaa_exit() is called */

   ST_INPUT_FILE_FAILURE,
   ST_OUTPUT_FILE_FAILURE,
   ST_ERROR_FILE_FAILURE,
      /* - drmaa_init() is called
         - a job is submitted with input/output/error path specification 
           that must cause the job to fail
         - use drmaa_synchronize() to ensure job was started
         - drmaa_job_ps() must return DRMAA_PS_FAILED
         - drmaa_wait() must report drmaa_wifaborted() -> true
         - then drmaa_exit() is called */

   ST_SUBMIT_IN_HOLD_RELEASE,
      /* - drmaa_init() is called
         - a job is submitted with a user hold 
         - use drmaa_job_ps() to verify user hold state
         - hold state is released using drmaa_control()
         - the job is waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_SUBMIT_IN_HOLD_DELETE,
      /* - drmaa_init() is called
         - a job is submitted with a user hold 
         - use drmaa_job_ps() to verify user hold state
         - job is terminated using drmaa_control()
         - the job is waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE,
      /* - drmaa_init() is called
         - a bulk job is submitted with a user hold 
         - hold state is released separately for each task using drmaa_control()
         - the job ids are waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE,
      /* - drmaa_init() is called
         - a bulk job is submitted with a user hold 
         - hold state is released for the session using drmaa_control()
         - the job ids are waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE,
      /* - drmaa_init() is called
         - a bulk job is submitted with a user hold 
         - use drmaa_job_ps() to verify user hold state
         - all session jobs are terminated using drmaa_control()
         - the job ids are waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE,
      /* - drmaa_init() is called
         - a bulk job is submitted with a user hold 
         - use drmaa_job_ps() to verify user hold state
         - all session jobs are terminated using drmaa_control()
         - the job ids are waited
         - then drmaa_exit() is called
         (still requires manual testing)
      */

   ST_INPUT_BECOMES_OUTPUT,
      /* - drmaa_init() is called
         - job input is prepared in local file 
         - a job is submitted that echoes it's input to output 
         - the job is waited
         - then drmaa_exit() is called
         - job output must be identical to job input
           (this requires manual testing) */

   ST_DRMAA_JOB_PS,
      /* - drmaa_init() is called
         - drmaa_job_ps() is used to retrieve DRMAA state 
           for each jobid passed *manually* in argv
         - then drmaa_exit() is called 
         (requires manual testing)
      */

   ST_DRMAA_CONTROL,
      /* - drmaa_init() is called
         - drmaa_control() is used to change DRMAA job state 
           for each jobid passed *manually* in argv
         - drmaa_control() must return with the exit status passed in argv
         - then drmaa_exit() is called 
         (still manual testing)
      */

   ST_EXIT_STATUS,
      /* - drmaa_init() is called
         - 255 job are submitted
         - job i returns i as exit status (8 bit)
         - drmaa_wait() verifies each job returned the 
           correct exit status
         - then drmaa_exit() is called */

   ST_SUPPORTED_ATTR,
      /* - drmaa_init() is called
         - drmaa_get_attribute_names() is called
         - the names of all supported non vector attributes are printed
         - then drmaa_exit() is called */

   ST_SUPPORTED_VATTR,
      /* - drmaa_init() is called
         - drmaa_get_vector_attribute_names() is called
         - the names of all supported vector attributes are printed
         - then drmaa_exit() is called */

   ST_VERSION,
      /* - drmaa_version() is called 
         - version information is printed */

   ST_CONTACT,
      /* - drmaa_get_contact() is called
         - the contact string is printed
         - drmaa_init() is called 
         - drmaa_get_contact() is called
         - the contact string is printed
         - then drmaa_exit() is called */

   ST_DRM_SYSTEM,
      /* - drmaa_get_DRM_system() is called
         - the contact string is printed
         - drmaa_init() is called 
         - drmaa_get_DRM_system() is called
         - the DRM system name is printed
         - then drmaa_exit() is called */

   ST_DRMAA_IMPL,
      /* - drmaa_get_DRM_system() is called
         - the contact string is printed
         - drmaa_init() is called 
         - drmaa_get_DRMAA_implementation() is called
         - the DRMAA implemention name is printed
         - then drmaa_exit() is called */

   ST_EMPTY_SESSION_WAIT,
      /* - drmaa_init() is called
         - drmaa_wait() must return DRMAA_ERRNO_INVALID_JOB
         - then drmaa_exit() is called */

   ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE,
      /* - drmaa_init() is called
         - drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose=true) must return DRMAA_ERRNO_SUCCESS
         - then drmaa_exit() is called */

   ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE,
      /* - drmaa_init() is called
         - drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose=false) must return DRMAA_ERRNO_SUCCESS
         - then drmaa_exit() is called */

   ST_EMPTY_SESSION_CONTROL,
      /* - drmaa_init() is called
         - drmaa_control(DRMAA_JOB_IDS_SESSION_ALL, <passed control operation>) must return DRMAA_ERRNO_SUCCESS
         - then drmaa_exit() is called */

   ST_SUBMIT_SUSPEND_RESUME_WAIT,
      /*  - drmaa_init() is called
          - a single job is submitted 
          - drmaa_job_ps() is used to actively wait until job is running
          - drmaa_control() is used to suspend the job
          - drmaa_job_ps() is used to verify job was suspended
          - drmaa_control() is used to resume the job
          - drmaa_job_ps() is used to verify job was resumed
          - drmaa_wait() is used to wait for the jobs regular end
          - then drmaa_exit() is called */

   ST_SUBMIT_POLLING_WAIT_TIMEOUT, 
      /*  - drmaa_init() is called
          - a single job is submitted 
          - repeatedly drmaa_wait() with a timeout is used until job is finished
          - then drmaa_exit() is called */

   ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT,
      /*  - drmaa_init() is called
          - a single job is submitted 
          - repeatedly do drmaa_wait(DRMAA_TIMEOUT_NO_WAIT) + sleep() until job is finished
          - then drmaa_exit() is called */

   ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT,
      /*  - drmaa_init() is called
          - a single job is submitted 
          - repeatedly drmaa_synchronize() with a timeout is used until job is finished
          - then drmaa_exit() is called */

   ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT,
      /*  - drmaa_init() is called
          - a single job is submitted 
          - repeatedly do drmaa_synchronize(DRMAA_TIMEOUT_NO_WAIT) + sleep() until job is finished
          - then drmaa_exit() is called */

   ST_ATTRIBUTE_CHECK,
      /* Need to test all DRMAA attributes:
         DRMAA_REMOTE_COMMAND - implicit
         DRMAA_JS_STATE
         DRMAA_WD
         DRMAA_JOB_NAME
         DRMAA_INPUT_PATH
         DRMAA_OUTPUT_PATH
         DRMAA_ERROR_PATH
         DRMAA_JOIN_FILES
         DRMAA_JOB_CATEGORY
         DRMAA_NATIVE_SPECIFICATION - test if it works and it if clashes
         DRMAA_BLOCK_EMAIL
         DRMAA_START_TIME
         DRMAA_V_ARGV
         DRMAA_V_EMAIL
         DRMAA_V_ENV */
   ST_USAGE_CHECK,
      /* - one thread 
         - submit jobs 
         - wait for jobend
         - print job usage */
   ST_TRANSFER_FILES_SINGLE_JOB,
   ST_TRANSFER_FILES_BULK_JOB,
      /* Set Job InputHost:/InputPath, OutputHost:/OutputPath, ErrorHost:/ErrorPath */

   ST_RESERVATION_FINISH_ORDER,
      /* ensure three jobs finish in the order foreseen for reservation */

   ST_BACKFILL_FINISH_ORDER,
      /* ensure three jobs finish in the order foreseen for backfilling */
            
   ST_WILD_PARALLEL,
      /* ensure 7 jobs finish in the order foreseen for wildcard parallel jobs */
   
   ST_UNSUPPORTED_ATTR,
      /* - drmaa_init() is called
         - drmaa_set_attribute() is called for an invalid attribute
         - then drmaa_exit() is called */
   
   ST_UNSUPPORTED_VATTR,
      /* - drmaa_init() is called
         - drmaa_set_vector_attribute() is called for an invalid attribute
         - then drmaa_exit() is called */
   
   ST_SYNCHRONIZE_NONEXISTANT,
      /* - Init session.
         - Create job template.
         - Run job.
         - Delete job template.
         - Use job id to create unknown, valid job id.
         - Synchronize against unknown id.
         - Wait for real job to finish.
         - Exit session. */
   
   ST_RECOVERABLE_SESSION,
      /* - Init session.
         - Create job template.
         - Run job.
         - Delete job template.
         - Exit session.
         - Init session.
         - Wait for job to finish.
         - Exit session. */
   
   ST_ERROR_CODES
      /* - Test that each error code has the right value. */
};

const struct test_name2number_map {
   char *test_name;       /* name of the test                                    */
   int test_number;       /* number the test is internally mapped to             */
   int nargs;             /* number of test case arguments required              */
   char *opt_arguments;   /* description of test case arguments for usage output */
} test_map[] = {

   /* all automated tests - ST_* and MT_* tests */
   { "ALL_AUTOMATED",                            ALL_TESTS,                                  3, "<sleeper_job> <exit_arg_job> <email_addr>" },

   /* one application thread - automated tests only */
   { "ST_ERROR_CODES",                            ST_ERROR_CODES,                            0, "" },
   { "ST_MULT_INIT",                              ST_MULT_INIT,                              0, "" },
   { "ST_MULT_EXIT",                              ST_MULT_EXIT,                              0, "" },
   { "ST_SUPPORTED_ATTR",                         ST_SUPPORTED_ATTR,                         0, "" },
   { "ST_SUPPORTED_VATTR",                        ST_SUPPORTED_VATTR,                        0, "" },
   { "ST_VERSION",                                ST_VERSION,                                0, "" },
   { "ST_DRM_SYSTEM",                             ST_DRM_SYSTEM,                             0, "" },
   { "ST_DRMAA_IMPL",                             ST_DRMAA_IMPL,                             0, "" },
   { "ST_CONTACT",                                ST_CONTACT,                                0, "" },
   { "ST_EMPTY_SESSION_WAIT",                     ST_EMPTY_SESSION_WAIT,                     0, "" },
   { "ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE",      ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE,      0, "" },
   { "ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE",    ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE,    0, "" },
   { "ST_EMPTY_SESSION_CONTROL",                  ST_EMPTY_SESSION_CONTROL,                  1, "DRMAA_CONTROL_*" },
   { "ST_SUBMIT_WAIT",                            ST_SUBMIT_WAIT,                            1, "<sleeper_job>" },
   { "ST_SUBMIT_NO_RUN_WAIT",                     ST_SUBMIT_NO_RUN_WAIT,                     1, "<sleeper_job>" },
   { "ST_BULK_SUBMIT_WAIT",                       ST_BULK_SUBMIT_WAIT,                       1, "<sleeper_job>" },
   { "ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL",      ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL,      1, "<sleeper_job>" },
   { "ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE",         ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE,         1, "<sleeper_job>" },
   { "ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE",       ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE,       1, "<sleeper_job>" },
   { "ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE",      ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE,      1, "<sleeper_job>" },
   { "ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE",    ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE,    1, "<sleeper_job>" },
   { "ST_SUBMIT_PAUSE_SUBMIT_SYNC",               ST_SUBMIT_PAUSE_SUBMIT_SYNC,               1, "<sleeper_job>" },
   { "ST_EXIT_STATUS",                            ST_EXIT_STATUS,                            1, "<exit_arg_job>" },
   { "ST_INPUT_FILE_FAILURE",                     ST_INPUT_FILE_FAILURE,                     1, "<sleeper_job>" },
   { "ST_OUTPUT_FILE_FAILURE",                    ST_OUTPUT_FILE_FAILURE,                    1, "<sleeper_job>" },
   { "ST_ERROR_FILE_FAILURE",                     ST_ERROR_FILE_FAILURE,                     1, "<sleeper_job>" },
   { "ST_SUBMIT_IN_HOLD_RELEASE",                 ST_SUBMIT_IN_HOLD_RELEASE,                 1, "<sleeper_job>" },
   { "ST_SUBMIT_IN_HOLD_DELETE",                  ST_SUBMIT_IN_HOLD_DELETE,                  1, "<sleeper_job>" },
   { "ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE",    ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE,    1, "<sleeper_job>" },
   { "ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE",     ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE,     1, "<sleeper_job>" },
   { "ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE",     ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE,     1, "<sleeper_job>" },
   { "ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE",      ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE,      1, "<sleeper_job>" },
   { "ST_SUBMIT_POLLING_WAIT_TIMEOUT",            ST_SUBMIT_POLLING_WAIT_TIMEOUT,            1, "<sleeper_job>" },
   { "ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT",        ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT,        1, "<sleeper_job>" },
   { "ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT",     ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT,     1, "<sleeper_job>" },
   { "ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT", ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT, 1, "<sleeper_job>" },
   { "ST_UNSUPPORTED_ATTR",                       ST_UNSUPPORTED_ATTR,                       0, "" },
   { "ST_UNSUPPORTED_VATTR",                      ST_UNSUPPORTED_VATTR,                      0, "" },
   { "ST_SYNCHRONIZE_NONEXISTANT",                ST_SYNCHRONIZE_NONEXISTANT,                1, "<sleeper_job>" },
   { "ST_RECOVERABLE_SESSION",                    ST_RECOVERABLE_SESSION,                    1, "<sleeper_job>" },

   /* multiple application threads - automated tests only */
   { "MT_SUBMIT_WAIT",                           MT_SUBMIT_WAIT,                             1, "<sleeper_job>" },
   { "MT_SUBMIT_BEFORE_INIT_WAIT",               MT_SUBMIT_BEFORE_INIT_WAIT,                 1, "<sleeper_job>" },
   { "MT_EXIT_DURING_SUBMIT",                    MT_EXIT_DURING_SUBMIT,                      1, "<sleeper_job>" },
   { "MT_SUBMIT_MT_WAIT",                        MT_SUBMIT_MT_WAIT,                          1, "<sleeper_job>" },
   { "MT_EXIT_DURING_SUBMIT_OR_WAIT",            MT_EXIT_DURING_SUBMIT_OR_WAIT,              1, "<sleeper_job>" },
  
   /* ------------------------------------------------------------------------------------ */
   /* tests that require test suite to be run in an automated fashion (file name creation) */
   { "ST_INPUT_BECOMES_OUTPUT",                  ST_INPUT_BECOMES_OUTPUT,                    2, "<input_path> <output_path>" },
   { "ST_ATTRIBUTE_CHECK",                       ST_ATTRIBUTE_CHECK,                         2, "<exit_arg_job> <email_addr>" },
   { "ST_SUBMIT_SUSPEND_RESUME_WAIT",            ST_SUBMIT_SUSPEND_RESUME_WAIT,              1, "<sleeper_job>" },

   /* tests that test_drmaa can't test in an automated fashion (so far) */
   { "ST_DRMAA_JOB_PS",                          ST_DRMAA_JOB_PS,                            1, "<jobid> ..."   },
   { "ST_DRMAA_CONTROL",                         ST_DRMAA_CONTROL,                           3, "DRMAA_CONTROL_* DRMAA_ERRNO_* <jobid> ..." },
   { "ST_USAGE_CHECK",                           ST_USAGE_CHECK,                             1, "<exit_job>" },

   { "ST_TRANSFER_FILES_SINGLE_JOB",             ST_TRANSFER_FILES_SINGLE_JOB,               6, "<sleeper_job> <file_staging_flags "
         "{\"i\"|\"o\"|\"e\" }> <merge_stderr {\"y\"|\"n\"}> <[inputhost]:/inputpath> <[outputhost]:/outputpath> <[errorhost]:/errorpath>" },


   { "ST_TRANSFER_FILES_BULK_JOB",               ST_TRANSFER_FILES_BULK_JOB,                 6, "<sleeper_job> <file_staging_flags "
         "{\"i\"|\"o\"|\"e\" }> <<merge_stderr {\"y\"|\"n\"}> [inputhost]:/inputpath> <[outputhost]:/outputpath> <[errorhost]:/errorpath>" },

   /* tests that have nothing to do with drmaa */
   { "ST_RESERVATION_FINISH_ORDER",              ST_RESERVATION_FINISH_ORDER,                4, "<sleeper_job> <native_spec0> <native_spec1> <native_spec2>" },
   { "ST_BACKFILL_FINISH_ORDER",                 ST_BACKFILL_FINISH_ORDER,                   4, "<sleeper_job> <native_spec0> <native_spec1> <native_spec2>" },
   { "ST_WILD_PARALLEL",                         ST_WILD_PARALLEL,                           4, "<sleeper_job> <native_spec0> <native_spec1> <native_spec2>" },

   { NULL,                                       0 }
};
#define FIRST_NON_AUTOMATED_TEST ST_INPUT_BECOMES_OUTPUT

static int test(sge_gdi_ctx_class_t *ctx, int *argc, char **argv[], int parse_args);
static int submit_and_wait(int n);
static int submit_sleeper(int n);
static int submit_input_mirror(int n, const char *mirror_job, 
                               const char *input_path, const char *output_path,
                               const char *error_path, int join, char* hostname);
static int do_submit(drmaa_job_template_t *jt, int n);
static int wait_all_jobs(int n);
static int wait_n_jobs(int n);
static drmaa_job_template_t *create_sleeper_job_template(int seconds,
                                                         int as_bulk_job,
                                                         int in_hold);
static drmaa_job_template_t *create_exit_job_template(const char *exit_job,
                                                      int as_bulk_job);
static void report_session_key(void);
static void *submit_and_wait_thread (void *v);
static void *submit_sleeper_thread (void *v);

int str2drmaa_state(const char *str);
static int str2drmaa_ctrl(const char *str);
static int str2drmaa_errno(const char *str);
static const char *drmaa_state2str(int state);
static const char *drmaa_ctrl2str(int control);
static const char *drmaa_errno2str(int ctrl);

static void array_job_run_sequence_adapt(int **sequence, int job_id, int count); 

static int set_path_attribute_plus_colon(drmaa_job_template_t *jt,
                                         const char *name, const char *value,
                                         char *error_diagnosis,
                                         size_t error_diag_len);
static int set_path_attribute_plus_colon(drmaa_job_template_t *jt,
                                         const char *name, const char *value,
                                         char *error_diagnosis,
                                         size_t error_diag_len);
static bool test_error_code(char *name, int code, int expected);
static void report_wrong_job_finish(const char *comment, const char *jobid,
                                    int stat);

typedef struct {
   char *native;
   int time;
} test_job_t;
static int test_dispatch_order_njobs(int n, test_job_t jobs[], char *jsr_str);
static int job_run_sequence_verify(int pos, const char *all_jobids[], int *order[]);
static int **job_run_sequence_parse(char *jrs_str);

static int test_case;
static int is_sun_grid_engine;

/* global test case parameters */
char *sleeper_job = NULL,
     *exit_job = NULL,
     *mirror_job = NULL,
     *input_path = NULL,
     *output_path = NULL,
     *error_path = NULL,
     *email_addr = NULL;
int ctrl_op = -1;

static void init_jobids(const char *jobids[], int size)
{
   int i = 0;
   for (i = 0; i < size; i++) {
      jobids[i] = NULL;
   }
}

static void free_jobids(const char *jobids[], int size)
{
   int i = 0;
   for (i = 0; i < size; i++) {
      if (jobids[i] != NULL) {
         sge_free(&(jobids[i]));
      }
   }
}

static void usage(void)
{
   int i;
   fprintf(stderr, "usage: test_drmaa <test_case>\n");

   fprintf(stderr, "  <test_case> is one of the keywords below including the enlisted test case arguments\n");
   for (i=0; test_map[i].test_name; i++)
      fprintf(stderr, "\t%-45.45s %s\n", test_map[i].test_name, test_map[i].opt_arguments);

   fprintf(stderr, "  <sleeper_job>  is an executable job that sleeps <argv1> seconds\n");
   fprintf(stderr, "                 the job must be executable at the target machine\n");
   fprintf(stderr, "  <mirror_job>   is an executable job that returns it's stdin stream to stdout (e.g. /bin/cat)\n");
   fprintf(stderr, "                 the job must be executable at the target machine\n\n");
   fprintf(stderr, "  <exit_arg_job> is an executable job that exits <argv1> as exit status\n");
   fprintf(stderr, "                 the job must be executable at the target machine\n");
   fprintf(stderr, "  <input_path>   is the path of an input file\n");
   fprintf(stderr, "                 the user must have read access to this file at the target machine\n");
   fprintf(stderr, "  <output_path>  is the path of an output file\n");
   fprintf(stderr, "                 the user must have write access to this file at the target machine\n");
   fprintf(stderr, "  <email_addr>   is an email address to which to send \n");
   fprintf(stderr, "                 job completion notices\n");
   fprintf(stderr, "  <native_spec0> a native specification\n");
   fprintf(stderr, "  <native_spec1> a native specification\n");
   fprintf(stderr, "  <native_spec2> a native specification\n");

   exit(1);
}

int main(int argc, char *argv[])
{
   int failed = 0;
   int i; 
   char diag[DRMAA_ERROR_STRING_BUFFER];
   sge_gdi_ctx_class_t *ctx = NULL;
   lList *alp = NULL;

   DENTER_MAIN(TOP_LAYER, "qsub");

   if (argc == 1) 
      usage();
   
   /* Print out an adivsory */
   printf ("The DRMAA test suite is now starting.\n");

   /* figure out which DRM system we are using */
   {
      char drm_name[DRMAA_DRM_SYSTEM_BUFFER];
      if (drmaa_get_DRM_system(drm_name, 255, diag, sizeof(diag)-1)!=DRMAA_ERRNO_SUCCESS) {
         fprintf(stderr, "drmaa_get_DRM_system() failed: %s\n", diag);
         sge_prof_cleanup();
         return 1;
      }
      printf("Connecting to DRM system \"%s\"\n", drm_name);
      if (!strncmp(drm_name, "SGE", 3))
         is_sun_grid_engine = 1;
      else
         is_sun_grid_engine = 0;
   }

   /*
   ** since drmaa doesn't give an explicit handle to the context and sge_gdi 
   ** is used below, we provide our own context here
   */
   if (sge_gdi2_setup(&ctx, JAPI, MAIN_THREAD, &alp) != AE_OK) {
      answer_list_output(&alp);
      SGE_EXIT((void**)&ctx, 1);
   }

   while (argc > 1) {
      /* map test name to test number */
      for (i=0; test_map[i].test_name; i++)
         if (!strcasecmp(argv[1], test_map[i].test_name))
            break;
      if (!test_map[i].test_name) {
         fprintf(stderr, "test_drmaa: %s is not a valid test name\n", argv[1]);
         usage();
      }
      test_case = test_map[i].test_number;
      argc--; argv++;
      if ((argc-1) < test_map[i].nargs)
         usage();

      if (test_case == ALL_TESTS) {
         int success = 1;
         sleeper_job = NEXT_ARGV(&argc, &argv);
         exit_job    = NEXT_ARGV(&argc, &argv);
         email_addr  = NEXT_ARGV(&argc, &argv);

         for (i=1; test_map[i].test_name && test_map[i].test_number != FIRST_NON_AUTOMATED_TEST && success; i++) {
            test_case = test_map[i].test_number;
            printf("---------------------\n");
            printf("starting test #%d (%s)\n", i, test_map[i].test_name);

            switch (test_case) {
            case ST_EMPTY_SESSION_CONTROL: 
            {
               int i;
               const int ctrl_ops[] = { DRMAA_CONTROL_SUSPEND, DRMAA_CONTROL_RESUME, 
                     DRMAA_CONTROL_HOLD, DRMAA_CONTROL_RELEASE, DRMAA_CONTROL_TERMINATE, -1 };
               for (i=0; ctrl_ops[i] != -1; i++) {
                  ctrl_op = ctrl_ops[i]; 
                  if (test(ctx, &argc, &argv, 0)!=0) {
                     printf("test \"%s\" with \"%s\" failed\n", 
                           test_map[i].test_name, drmaa_ctrl2str(ctrl_ops[i]));
                     failed = 1;
                     break;
                  } else
                     printf("successfully finished test \"%s\" with \"%s\"\n", 
                              test_map[i].test_name, drmaa_ctrl2str(ctrl_ops[i]));
               }
               break;
            }

            default:
               if (test(ctx, &argc, &argv, 0)!=0) {
                  printf("test #%d failed\n", i);
                  failed = 1;
                  break;
               } else
                  printf("successfully finished test #%d\n", i);
               break;
            }
            
            if (failed)
               success = 0;
         }
      } else {
         printf("starting test \"%s\"\n", test_map[i].test_name);
         if (test(ctx, &argc, &argv, 1)!=0) {
            printf("test \"%s\" failed\n", test_map[i].test_name);
            failed = 1;
            break;
         } else {
            printf("successfully finished test \"%s\"\n", test_map[i].test_name);
         }
      }
   } 
   drmaa_exit(NULL, 0);
   sge_gdi2_shutdown((void**)&ctx);
   sge_gdi_ctx_class_destroy(&ctx);

   sge_prof_cleanup();
   return failed;
}


static int test(sge_gdi_ctx_class_t *ctx, int *argc, char **argv[], int parse_args)
{
   bool bBulkJob = false;
   int  i;
   int  job_chunk = JOB_CHUNK;
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   drmaa_job_template_t *jt = NULL;
   int drmaa_errno=0;
   int do_while_end = 0;

   switch (test_case) {
   case ST_ERROR_CODES:
      {
         if (test_error_code("DRMAA_ERRNO_SUCCESS", DRMAA_ERRNO_SUCCESS, 0) &&
             test_error_code("DRMAA_ERRNO_INTERNAL_ERROR", DRMAA_ERRNO_INTERNAL_ERROR, 1) &&
             test_error_code("DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE", DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE, 2) &&
             test_error_code("DRMAA_ERRNO_AUTH_FAILURE", DRMAA_ERRNO_AUTH_FAILURE, 3) &&
             test_error_code("DRMAA_ERRNO_INVALID_ARGUMENT", DRMAA_ERRNO_INVALID_ARGUMENT, 4) &&
             test_error_code("DRMAA_ERRNO_NO_ACTIVE_SESSION", DRMAA_ERRNO_NO_ACTIVE_SESSION, 5) &&
             test_error_code("DRMAA_ERRNO_NO_MEMORY", DRMAA_ERRNO_NO_MEMORY, 6) &&
             test_error_code("DRMAA_ERRNO_INVALID_CONTACT_STRING", DRMAA_ERRNO_INVALID_CONTACT_STRING, 7) &&
             test_error_code("DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR", DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR, 8) &&
             test_error_code("DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED", DRMAA_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED, 9) &&
             test_error_code("DRMAA_ERRNO_DRMS_INIT_FAILED", DRMAA_ERRNO_DRMS_INIT_FAILED, 10) &&
             test_error_code("DRMAA_ERRNO_ALREADY_ACTIVE_SESSION", DRMAA_ERRNO_ALREADY_ACTIVE_SESSION, 11) &&
             test_error_code("DRMAA_ERRNO_DRMS_EXIT_ERROR", DRMAA_ERRNO_DRMS_EXIT_ERROR, 12) &&
             test_error_code("DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT", DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT, 13) &&
             test_error_code("DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE", DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE, 14) &&
             test_error_code("DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES", DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES, 15) &&
             test_error_code("DRMAA_ERRNO_TRY_LATER", DRMAA_ERRNO_TRY_LATER, 16) &&
             test_error_code("DRMAA_ERRNO_DENIED_BY_DRM", DRMAA_ERRNO_DENIED_BY_DRM, 17) &&
             test_error_code("DRMAA_ERRNO_INVALID_JOB", DRMAA_ERRNO_INVALID_JOB, 18) &&
             test_error_code("DRMAA_ERRNO_RESUME_INCONSISTENT_STATE", DRMAA_ERRNO_RESUME_INCONSISTENT_STATE, 19) &&
             test_error_code("DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE", DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE, 20) &&
             test_error_code("DRMAA_ERRNO_HOLD_INCONSISTENT_STATE", DRMAA_ERRNO_HOLD_INCONSISTENT_STATE, 21) &&
             test_error_code("DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE", DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE, 22) &&
             test_error_code("DRMAA_ERRNO_EXIT_TIMEOUT", DRMAA_ERRNO_EXIT_TIMEOUT, 23) &&
             test_error_code("DRMAA_ERRNO_NO_RUSAGE", DRMAA_ERRNO_NO_RUSAGE, 24) &&
             test_error_code("DRMAA_ERRNO_NO_MORE_ELEMENTS", DRMAA_ERRNO_NO_MORE_ELEMENTS, 25)
         ) {
            break;
         }
         else {
            return 1;
         }
      }
   case ST_MULT_INIT:
      { 
         /* no test case arguments */

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_ALREADY_ACTIVE_SESSION) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_MULT_EXIT:
      { 
         /* no test case arguments */

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit(1) failed: %s\n", diagnosis);
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_NO_ACTIVE_SESSION) {
            fprintf(stderr, "drmaa_exit(2) failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUBMIT_WAIT:
   case ST_SUBMIT_NO_RUN_WAIT:
      {
         int n = (test_case == ST_SUBMIT_WAIT)?JOB_CHUNK:1;
         char jobid[1024];

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }

         if (test_case == ST_SUBMIT_NO_RUN_WAIT) {
            drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, "-l a=fantasy_os -now yes -w n", NULL, 0);
         }

         for (i=0; i<n; i++) {
            if (drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               return 1;
            }
            printf("submitted job \"%s\"\n", jobid);
         }

         drmaa_delete_job_template(jt, NULL, 0);

         if (wait_all_jobs(n) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUBMIT_POLLING_WAIT_TIMEOUT:
   case ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT:
   case ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT:
   case ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT:
      {
         char jobid[1024];
         const int timeout = 5;
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }

         if (drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }
         printf("submitted job \"%s\"\n", jobid);

         drmaa_delete_job_template(jt, NULL, 0);
   
         switch (test_case) {

         case ST_SUBMIT_POLLING_WAIT_TIMEOUT:
            while ((drmaa_errno=drmaa_wait(jobid, NULL, 0, NULL, timeout, NULL, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               if (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
                  fprintf(stderr, "drmaa_wait(\"%s\", timeout = %d) failed: %s (%s)\n", 
                        jobid, timeout, diagnosis, drmaa_strerror(drmaa_errno));
                  return 1;
               }
               printf("still waiting for job \"%s\" to finish\n", jobid);
            }
            break;

         case ST_SUBMIT_POLLING_WAIT_ZEROTIMEOUT:
            while ((drmaa_errno=drmaa_wait(jobid, NULL, 0, NULL, DRMAA_TIMEOUT_NO_WAIT, NULL, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               if (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
                  fprintf(stderr, "drmaa_wait(\"%s\", no timeout) failed: %s (%s)\n", 
                        jobid, diagnosis, drmaa_strerror(drmaa_errno));
                  return 1;
               }
               printf("still waiting for job \"%s\" to finish\n", jobid);
               sleep(timeout);
               printf("slept %d seconds\n", timeout);
            }
            break;

         case ST_SUBMIT_POLLING_SYNCHRONIZE_TIMEOUT:
            while ((drmaa_errno=drmaa_synchronize(session_all, timeout, 1,
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               if (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
                  fprintf(stderr, "drmaa_synchronize(\"%s\", timeout = %d) failed: %s (%s)\n", 
                        jobid, timeout, diagnosis, drmaa_strerror(drmaa_errno));
                  return 1;
               }
               printf("still trying to synchronize with job \"%s\" to finish\n", jobid);
            }
            break;

         case ST_SUBMIT_POLLING_SYNCHRONIZE_ZEROTIMEOUT:
            while ((drmaa_errno=drmaa_synchronize(session_all, DRMAA_TIMEOUT_NO_WAIT, 1, 
                        diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               if (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
                  fprintf(stderr, "drmaa_synchronize(\"%s\", no timeout) failed: %s (%s)\n", 
                        jobid, diagnosis, drmaa_strerror(drmaa_errno));
                  return 1;
               }
               printf("still trying to synchronize with job \"%s\" to finish\n", jobid);
               sleep(timeout);
               printf("slept %d seconds\n", timeout);
            }
            break;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case MT_SUBMIT_WAIT:
      {
         pthread_t submitter_threads[NTHREADS]; 
         int n = -1;

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         for (i=0; i<NTHREADS; i++)
            pthread_create(&submitter_threads[i], NULL, submit_sleeper_thread, &job_chunk);
         for (i=0; i<NTHREADS; i++)
            if (pthread_join(submitter_threads[i], NULL)) 
               printf("pthread_join() returned != 0\n");
         if (wait_all_jobs(n) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case MT_SUBMIT_BEFORE_INIT_WAIT:
      {
         pthread_t submitter_threads[NTHREADS]; 
         int n = -1;

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);
         printf("sleeper_job = %s\n", sleeper_job);
         for (i=0; i<NTHREADS; i++) {
            pthread_create(&submitter_threads[i], NULL, submit_sleeper_thread, &job_chunk);
         }

         /* delay drmaa_init() */
         sleep(5);
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis); 
            return 1;
         }
         report_session_key();

         for (i=0; i<NTHREADS; i++)
            if (pthread_join(submitter_threads[i], NULL))
               printf("pthread_join() returned != 0\n");

         if (wait_all_jobs(n) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case MT_EXIT_DURING_SUBMIT:
      {
         pthread_t submitter_threads[NTHREADS]; 

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         putenv("SGE_DELAY_AFTER_SUBMIT=20");

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         for (i=0; i<NTHREADS; i++)
            pthread_create(&submitter_threads[i], NULL, submit_sleeper_thread, &job_chunk);
         sleep(1);
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         printf("drmaa_exit() succeeded\n");
         
         putenv("SGE_DELAY_AFTER_SUBMIT=0");

         for (i=0; i<NTHREADS; i++)
            if (pthread_join(submitter_threads[i], NULL))
               printf("pthread_join() returned != 0\n");
      }
      break;

   case MT_SUBMIT_MT_WAIT:
      {
         pthread_t submitter_threads[NTHREADS]; 

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         for (i=0; i<NTHREADS; i++)
            pthread_create(&submitter_threads[i], NULL, submit_and_wait_thread, &job_chunk);

         for (i=0; i<NTHREADS; i++)
            if (pthread_join(submitter_threads[i], NULL))
               printf("pthread_join() returned != 0\n");

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case MT_EXIT_DURING_SUBMIT_OR_WAIT:
      {
         pthread_t submitter_threads[NTHREADS]; 

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         for (i=0; i<NTHREADS; i++)
            pthread_create(&submitter_threads[i], NULL, submit_and_wait_thread, &job_chunk);
         sleep(20);
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         printf("drmaa_exit() succeeded\n");

         for (i=0; i<NTHREADS; i++)
            if (pthread_join(submitter_threads[i], NULL))
               printf("pthread_join() returned != 0\n");
      }
      break;

   case ST_BULK_SUBMIT_WAIT:
      {
         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<NBULKS; i++) {
            char jobid[100];
            drmaa_job_ids_t *jobids;
            int j = 0 ;
            int size = 0;
            
            if ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               printf("failed submitting bulk job (%s): %s\n", drmaa_strerror(drmaa_errno), diagnosis);
               return 1;
            } 
            
            printf("submitted bulk job with jobids:\n");

            drmaa_errno = drmaa_get_num_job_ids(jobids, &size);
            
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "failed getting # job ids: %s\n", drmaa_strerror(drmaa_errno));
               return 1;
            }
            
            for (j = 0; j < size; j++) {
               drmaa_errno = drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  printf("failed getting job id: %s\n", drmaa_strerror(drmaa_errno));
               }
               
               printf("\t \"%s\"\n", jobid);
            }

            drmaa_errno = drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);

            if (drmaa_errno != DRMAA_ERRNO_NO_MORE_ELEMENTS) {
               fprintf(stderr, "Got incorrect return value from drmaa_get_next_job_id()\n");
               return 1;
            }

            drmaa_release_job_ids(jobids);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         if (wait_n_jobs(JOB_CHUNK*NBULKS) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_BULK_SINGLESUBMIT_WAIT_INDIVIDUAL:
      {
         const int size_all_jobids = NBULKS*JOB_CHUNK + JOB_CHUNK + 1;
         const char *all_jobids[NBULKS*JOB_CHUNK + JOB_CHUNK + 1];
         char jobid[100];
         int pos = 0;
        
         init_jobids(all_jobids, size_all_jobids);

         if (parse_args) {
            sleeper_job = NEXT_ARGV(argc, argv);
         }

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some bulk jobs
          */
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i = 0; i < NBULKS; i++) {
            drmaa_job_ids_t *jobids;
            int j;

            while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                       sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s\n", diagnosis);
               sleep(1);
            } 
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

            printf("submitted bulk job with jobids:\n");
            for (j = 0; j < JOB_CHUNK; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               all_jobids[pos++] = strdup(jobid);
               printf("\t \"%s\"\n", jobid);
            } 
            drmaa_release_job_ids(jobids);
            sleep(1);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         for (i=0; i<JOB_CHUNK; i++) {
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            all_jobids[pos++] = strdup(jobid);
            printf("\t \"%s\"\n", jobid);
         }

         /* set string array end mark */
         all_jobids[pos] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   wait all those jobs
          */
         for (pos=0; pos<NBULKS*JOB_CHUNK + JOB_CHUNK; pos++) {
            do {
               int stat;
               drmaa_errno = drmaa_wait(all_jobids[pos], jobid, sizeof(jobid)-1, 
                  &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", all_jobids[pos], diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", all_jobids[pos], diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("waited job \"%s\"\n", all_jobids[pos]);
            sge_free(&(all_jobids[pos]));
         }

         free_jobids(all_jobids, size_all_jobids);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUBMITMIXTURE_SYNC_ALL_DISPOSE:
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) 
           to wait for all jobs to finish
         - then drmaa_exit() is called */
      {
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };
         char jobid[100];
         
         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some bulk jobs
          */
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<NBULKS; i++) {
            drmaa_job_ids_t *jobids;
            int j;

            while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                       sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s\n", diagnosis);
               sleep(1);
            } 
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s\n", diagnosis);
               return 1;
            }

            printf("submitted bulk job with jobids:\n");
            for (j=0; j<JOB_CHUNK; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               printf("\t \"%s\"\n", jobid);
            } 
            drmaa_release_job_ids(jobids);
            sleep(1);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<JOB_CHUNK; i++) {
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   synchronize with all jobs
          */
         drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 1, diagnosis, sizeof(diagnosis)-1);
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("waited all jobs\n");
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         break;
      }

   case ST_SUBMITMIXTURE_SYNC_ALL_NODISPOSE:

      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, no-dispose) 
           to wait for all jobs to finish
         - do drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY) until 
           DRMAA_ERRNO_INVALID_JOB to reap all jobs
         - then drmaa_exit() is called */

      {
         int size_all_jobids = NBULKS*JOB_CHUNK + JOB_CHUNK + 1;
         const char *all_jobids[NBULKS*JOB_CHUNK + JOB_CHUNK + 1];
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };
         char jobid[100];
         int pos = 0;
         
         init_jobids(all_jobids, size_all_jobids);

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some bulk jobs
          */
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<NBULKS; i++) {
            drmaa_job_ids_t *jobids;
            int j;

            while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                       sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s\n", diagnosis);
               sleep(1);
            } 
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

            printf("submitted bulk job with jobids:\n");
            for (j=0; j<JOB_CHUNK; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               all_jobids[pos++] = strdup(jobid);
               printf("\t \"%s\"\n", jobid);
            } 
            drmaa_release_job_ids(jobids);
            sleep(1);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<JOB_CHUNK; i++) {
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
            all_jobids[pos++] = strdup(jobid);
         }

         /* set string array end mark */
         all_jobids[pos] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   synchronize with all jobs
          */
         drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 0, diagnosis, sizeof(diagnosis)-1);
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         printf("synchronized with all jobs\n");

         /*
          *   wait all those jobs
          */
         for (pos=0; pos<NBULKS*JOB_CHUNK + JOB_CHUNK; pos++) {
            do {
               int stat;
               drmaa_errno = drmaa_wait(all_jobids[pos], jobid, sizeof(jobid)-1, 
                  &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", all_jobids[pos], diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", all_jobids[pos], diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("waited job \"%s\"\n", all_jobids[pos]);
            sge_free(&(all_jobids[pos]));
         }
         free_jobids(all_jobids, size_all_jobids);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         break;
      }

   case ST_SUBMITMIXTURE_SYNC_ALLIDS_DISPOSE:
      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(all_jobids, dispose) 
           to wait for all jobs to finish
         - then drmaa_exit() is called */
      {
         int size_all_jobids = NBULKS*JOB_CHUNK + JOB_CHUNK + 1;
         const char *all_jobids[NBULKS*JOB_CHUNK + JOB_CHUNK + 1];
         char jobid[100];
         int pos = 0;
        
         init_jobids(all_jobids, size_all_jobids);
        
         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some bulk jobs
          */
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<NBULKS; i++) {
            drmaa_job_ids_t *jobids;
            int j;

            while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                       sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s\n", diagnosis);
               sleep(1);
            } 
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

            printf("submitted bulk job with jobids:\n");
            for (j=0; j<JOB_CHUNK; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               printf("\t \"%s\"\n", jobid);
               all_jobids[pos++] = strdup(jobid);
            } 
            drmaa_release_job_ids(jobids);
            sleep(1);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         for (i=0; i<JOB_CHUNK; i++) {
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
            all_jobids[pos++] = strdup(jobid);
         }

         /* set string array end mark */
         all_jobids[pos] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   synchronize with all jobs
          */
         drmaa_errno = drmaa_synchronize(all_jobids, DRMAA_TIMEOUT_WAIT_FOREVER, 1, diagnosis, sizeof(diagnosis)-1);
         free_jobids(all_jobids, size_all_jobids);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("waited all jobs\n");
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         break;
      }

   case ST_SUBMITMIXTURE_SYNC_ALLIDS_NODISPOSE:

      /* - drmaa_init() is called
         - submit a mixture of single and bulk jobs
         - do drmaa_synchronize(all_jobids, no-dispose) 
           to wait for all jobs to finish
         - do drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY) until 
           DRMAA_ERRNO_INVALID_JOB to reap all jobs
         - then drmaa_exit() is called */

      {
         int size_all_jobids = NBULKS*JOB_CHUNK + JOB_CHUNK+1;
         const char *all_jobids[NBULKS*JOB_CHUNK + JOB_CHUNK+1];
         char jobid[100];
         int pos = 0;
         
         init_jobids(all_jobids, size_all_jobids);

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some bulk jobs
          */
         if (!(jt = create_sleeper_job_template(5, 1, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<NBULKS; i++) {
            drmaa_job_ids_t *jobids;
            int j;

            while ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis,
                       sizeof(diagnosis)-1))==DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed - retry: %s\n", diagnosis);
               sleep(1);
            } 
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

            printf("submitted bulk job with jobids:\n");
            for (j=0; j<JOB_CHUNK; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               all_jobids[pos++] = strdup(jobid);
               printf("\t \"%s\"\n", jobid);
            } 
            drmaa_release_job_ids(jobids);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         for (i=0; i<JOB_CHUNK; i++) {
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
            all_jobids[pos++] = strdup(jobid);
         }

         /* set string array end mark */
         all_jobids[pos] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   synchronize with all jobs
          */
         drmaa_errno = drmaa_synchronize(all_jobids, DRMAA_TIMEOUT_WAIT_FOREVER, 0, diagnosis, sizeof(diagnosis)-1);
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         printf("synchronized with all jobs\n");

         /*
          *   wait all those jobs
          */
         for (pos=0; pos<NBULKS*JOB_CHUNK + JOB_CHUNK; pos++) {
            do {
               int stat;
               drmaa_errno = drmaa_wait(all_jobids[pos], jobid, sizeof(jobid)-1, 
                  &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", all_jobids[pos], diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", all_jobids[pos], diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("waited job \"%s\"\n", all_jobids[pos]);
         }
         free_jobids(all_jobids, size_all_jobids);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         break;
      }

   case ST_SUBMIT_PAUSE_SUBMIT_SYNC:
      /* - drmaa_init() is called
         - a job is submitted
         - do a long sleep(ST_SUBMIT_PAUSE_SUBMIT_SYNC+)
         - another job is submitted
         - do drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose)
         - then drmaa_exit() is called */
      {
         int size_all_jobids = 2 + 1;
         const char *all_jobids[2 + 1];
         char jobid[100];
         int pos = 0;

         init_jobids(all_jobids, size_all_jobids);

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit some sequential jobs
          */
         if (!(jt = create_sleeper_job_template(5, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         for (i=0; i<2; i++) {
            drmaa_errno = drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1);
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
            all_jobids[pos++] = strdup(jobid);

            /* 
             * enforce SGE commproc timeout 
             * this timeout must be handled transparently by DRMAA implementation
             */
            if (i==0) {
               printf("sleeping %d seconds\n", SGE_COMMPROC_TIMEOUT+30);
               sleep(SGE_COMMPROC_TIMEOUT+30);
            }
         }
         /* set string array end mark */
         all_jobids[pos] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   synchronize with all jobs
          */
         drmaa_errno = drmaa_synchronize(all_jobids, DRMAA_TIMEOUT_WAIT_FOREVER, 1, diagnosis, sizeof(diagnosis)-1);
         free_jobids(all_jobids, size_all_jobids);
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("waited all jobs\n");

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_INPUT_FILE_FAILURE:
   case ST_ERROR_FILE_FAILURE:
   case ST_OUTPUT_FILE_FAILURE:
      {
         int aborted, stat, remote_ps;
         char jobid[100];
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
        
         /* submit a job that must fail */
         drmaa_allocate_job_template(&jt, NULL, 0);
         drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, sleeper_job, NULL, 0);

         switch (test_case) {
         case ST_OUTPUT_FILE_FAILURE:
            drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/etc/passwd", NULL, 0);
            break;

         case ST_ERROR_FILE_FAILURE:
            drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "n", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_ERROR_PATH, ":/etc/passwd", NULL, 0);
            break;

         case ST_INPUT_FILE_FAILURE:
            drmaa_set_attribute(jt, DRMAA_INPUT_PATH, ":<not existing file>", NULL, 0);
            break;
         }

         if ((drmaa_errno = drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                  sizeof(diagnosis)-1)) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }
         drmaa_delete_job_template(jt, NULL, 0);
         printf("submitted job \"%s\"\n", jobid);

         /* synchronize with job to finish but do not dispose job finish information */
         if ((drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 0, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("synchronized with job finish\n");

         /* get job state */
         drmaa_errno = drmaa_job_ps(jobid, &remote_ps, diagnosis, sizeof(diagnosis)-1);
         if (remote_ps != DRMAA_PS_FAILED) {
            fprintf(stderr, "job \"%s\" is not in failed state: %s\n", 
                     jobid, drmaa_state2str(remote_ps));
            return 1;
         }

         /* wait job */
         if ((drmaa_errno = drmaa_wait(jobid, NULL, 0, &stat, DRMAA_TIMEOUT_NO_WAIT, NULL, 
               diagnosis, sizeof(diagnosis)-1)) != DRMAA_ERRNO_SUCCESS) {
            printf("drmaa_wait() failed %s: %s\n", drmaa_strerror(drmaa_errno), diagnosis);
            return 1;
         }

         /* job finish information */
         drmaa_wifaborted(&aborted, stat, diagnosis, sizeof(diagnosis)-1);
         if (!aborted) {
            fprintf(stderr, "job \"%s\" failed but drmaa_wifaborted() returns false\n", 
                  jobid);
            return 1;
         }
         printf("waited job \"%s\" that never ran\n", jobid);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUPPORTED_ATTR:
   case ST_SUPPORTED_VATTR:
      /* - drmaa_init() is called
         - drmaa_get_attribute_names()/drmaa_get_vector_attribute_names() is called
         - the names of all supported non vector/vector attributes are printed
         - then drmaa_exit() is called */
      {
         drmaa_attr_names_t *vector;
         char attr_name[DRMAA_ATTR_BUFFER];
         int size = 0;
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         if (test_case == ST_SUPPORTED_ATTR)
            drmaa_errno = drmaa_get_attribute_names(&vector, diagnosis, sizeof(diagnosis)-1);
         else
            drmaa_errno = drmaa_get_vector_attribute_names(&vector, diagnosis, sizeof(diagnosis)-1);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_attribute_names()/drmaa_get_vector_attribute_names() failed: %s\n", 
                     diagnosis);
            return 1;
         }             

         drmaa_errno = drmaa_get_num_attr_names(vector, &size);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_num_attr_names() failed: %s\n", drmaa_strerror(drmaa_errno));
            return 1;
         }

         while ((drmaa_errno=drmaa_get_next_attr_name(vector, attr_name, sizeof(attr_name)-1))==DRMAA_ERRNO_SUCCESS) {
            size--;
            printf("%s\n", attr_name);
         }

         /* we don't need vector any longer - free it */
         drmaa_release_attr_names(vector);
         vector = NULL;

         if (size != 0) {
            fprintf(stderr, "Got incorrect size from drmaa_get_num_attr_names()\n");
            return 1;
         }
         
         if (drmaa_errno != DRMAA_ERRNO_NO_MORE_ELEMENTS) {
            fprintf(stderr, "Got incorrect return value from drmaa_get_next_attr_name()\n");
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_VERSION:
      /* - drmaa_version() is called 
         - version information is printed */
      {
         unsigned int major, minor;

         if (drmaa_version(&major, &minor, diagnosis, sizeof(diagnosis)-1)
               !=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_version() failed: %s\n", diagnosis);
            return 1;
         }

         printf("version %d.%d\n", major, minor);

         if ((major != 1) || (minor != 0)) {
            fprintf(stderr, "drmaa_version() failed -- incorrect version number : %d.%d\n", major, minor);
            return 1;
         }
      }
      break;

   case ST_CONTACT:
   case ST_DRM_SYSTEM:
   case ST_DRMAA_IMPL:
      {
         char output_string[1024];
         
         if (test_case == ST_CONTACT)
            drmaa_errno = drmaa_get_contact(output_string, sizeof(output_string)-1, 
                         diagnosis, sizeof(diagnosis)-1);
         else if (test_case == ST_DRM_SYSTEM)
            drmaa_errno = drmaa_get_DRM_system(output_string, sizeof(output_string)-1,
                         diagnosis, sizeof(diagnosis)-1);
         else if (test_case == ST_DRMAA_IMPL)
            drmaa_errno = drmaa_get_DRMAA_implementation(output_string, sizeof(output_string)-1,
                         diagnosis, sizeof(diagnosis)-1);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_contact()/drmaa_get_DRM_system() failed: %s\n", diagnosis);
            return 1;
         }
            
         if (test_case == ST_CONTACT)
            printf("drmaa_get_contact() returned \"%s\" before init\n", output_string);
         else if (test_case == ST_DRM_SYSTEM)
            printf("drmaa_get_DRM_system() returned \"%s\" before init\n", output_string);
         else if (test_case == ST_DRMAA_IMPL)
            printf("drmaa_get_DRMAA_implementation() returned \"%s\" before init\n", output_string);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         if (test_case == ST_CONTACT)
            drmaa_errno = drmaa_get_contact(output_string, sizeof(output_string)-1, 
                         diagnosis, sizeof(diagnosis)-1);
         else if (test_case == ST_DRM_SYSTEM)
            drmaa_errno = drmaa_get_DRM_system(output_string, sizeof(output_string)-1,
                         diagnosis, sizeof(diagnosis)-1);
         else if (test_case == ST_DRMAA_IMPL)
            drmaa_errno = drmaa_get_DRMAA_implementation(output_string, sizeof(output_string)-1,
                         diagnosis, sizeof(diagnosis)-1);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_contact()/drmaa_get_DRM_system() failed: %s\n", diagnosis);
            return 1;
         }
            
         if (test_case == ST_CONTACT)
            printf("drmaa_get_contact() returned \"%s\" after init\n", output_string);
         else if (test_case == ST_DRM_SYSTEM)
            printf("drmaa_get_DRM_system() returned \"%s\" after init\n", output_string);
         else if (test_case == ST_DRMAA_IMPL)
            printf("drmaa_get_DRMAA_implementation() returned \"%s\" after init\n", output_string);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_INPUT_BECOMES_OUTPUT:
      {
         const char *mirror_job = "/bin/cat",
         *input_path  = NULL,
         *output_path = NULL;
         FILE *fp = NULL;
         char buffer[1024];
         const char *mirror_text = "thefoxjumps...";
         char* local_host_name = NULL;

         if (parse_args) {
            input_path  = NEXT_ARGV(argc, argv);
            output_path = NEXT_ARGV(argc, argv);
         }

         if (!(fp = fopen(input_path, "w"))) {
            fprintf(stderr, "fopen(w) failed: %s\n", strerror(errno));
            return 1;
         }
         fprintf(fp, "%s\n", mirror_text);
         FCLOSE(fp);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         cl_com_gethostname(&local_host_name, NULL, NULL, NULL);
         if (local_host_name == NULL) {
            fprintf(stderr, "can't get local hostname\n");
            return 1;
         }
         if (submit_input_mirror(1, mirror_job, input_path, output_path, 
               NULL, 1, local_host_name)!=DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         sge_free(&local_host_name);

         if (wait_n_jobs(1) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }

         if (!(fp=fopen(output_path, "r"))) {
            fprintf(stderr, "fopen(%s) failed: %s\n", output_path, strerror(errno));
            return 1;
         }
         if (fscanf(fp, "%s", buffer) != 1) {
            fprintf(stderr, "fscanf error\n");
            return 1;
         }

         if (strcmp(buffer, mirror_text)) {
            fprintf(stderr, "wrong output file: %s\n", buffer);
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUBMIT_IN_HOLD_RELEASE:
   case ST_SUBMIT_IN_HOLD_DELETE:
      {
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };
         char jobid[1024];
         int job_state;

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
         if (!(jt = create_sleeper_job_template(5, 0, 1))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         if (drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }
         drmaa_delete_job_template(jt, NULL, 0);
         printf("submitted job in hold state \"%s\"\n", jobid);
         if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_job_ps(\"%s\")) failed: %s\n", jobid, diagnosis);
            return 1;
         }
         if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
            fprintf(stderr, "job \"%s\" is not in user hold state: %s\n", 
                     jobid, drmaa_state2str(job_state));
            return 1;
         }
         printf("verified user hold state for job \"%s\"\n", jobid);

         if (test_case == ST_SUBMIT_IN_HOLD_RELEASE) {
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, DRMAA_CONTROL_RELEASE) failed: %s\n", 
                        jobid, diagnosis);
               return 1;
            }
            printf("released user hold state for job \"%s\"\n", jobid);
         } else {
            if (drmaa_control(jobid, DRMAA_CONTROL_TERMINATE, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, DRMAA_CONTROL_TERMINATE) failed: %s\n", 
                        jobid, diagnosis);
               return 1;
            }
            printf("terminated job in hold state \"%s\"\n", jobid);
         }

         /* synchronize with job to finish but do not dispose job finish information */
         if ((drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 0, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("synchronized with job finish\n");

         /* report job finish state */
         if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_job_ps(\"%s\")) failed: %s\n", jobid, diagnosis);
            return 1;
         }
         printf("state of job \"%s\" is now %s\n", jobid, drmaa_state2str(job_state));
         if ((test_case == ST_SUBMIT_IN_HOLD_RELEASE && job_state != DRMAA_PS_DONE) ||
             (test_case != ST_SUBMIT_IN_HOLD_RELEASE && job_state != DRMAA_PS_FAILED)) {
            fprintf(stderr, "job \"%s\" terminated with unexpected state \"%s\"\n", jobid, drmaa_state2str(job_state));
            return 1;
         }

         if (wait_n_jobs(1) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE:
   case ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE:
   case ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE:
   case ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE:
      {
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };
         int size_all_jobids = JOB_CHUNK + 1;
         const char *all_jobids[JOB_CHUNK + 1];
         int job_state, pos = 0; 
         int ctrl_op;

         init_jobids(all_jobids, size_all_jobids);

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         if (!(jt = create_sleeper_job_template(5, 1, 1))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
        
         /* 
          * Submit a bulk job in hold and verify state using drmaa_job_ps() 
          */
         {
            drmaa_job_ids_t *jobids;
            int j;
            if ((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, JOB_CHUNK, 1, diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               printf("failed submitting bulk job (%s): %s\n", drmaa_strerror(drmaa_errno), diagnosis);
               return 1;
            } 
            printf("submitted bulk job with jobids:\n");
            for (j=0; j<JOB_CHUNK; j++) {
               char jobid[100];
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               printf("\t \"%s\"\n", jobid);

               /* copy jobid into jobid array */
               all_jobids[pos++] = strdup(jobid);

               if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_job_ps(\"%s\")) failed: %s\n", jobid, diagnosis);
                  free_jobids(all_jobids, size_all_jobids);
                  return 1;
               }
               if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
                  fprintf(stderr, "job \"%s\" is not in user hold state: %s\n", 
                           jobid, drmaa_state2str(job_state));
                  free_jobids(all_jobids, size_all_jobids);
                  return 1;
               }
            } 
            drmaa_release_job_ids(jobids);
         }
         drmaa_delete_job_template(jt, NULL, 0);

         printf("verified user hold state for bulk job\n");

         /*
          * Release or terminate all jobs using drmaa_control() depending on the test case 
          * drmaa_control() is applied muliple times on all tasks or on the whole session
          */
         if (test_case == ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE || 
             test_case == ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE) {
            int ctrl_op;
            if (test_case == ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE)
               ctrl_op = DRMAA_CONTROL_RELEASE;
            else
               ctrl_op = DRMAA_CONTROL_TERMINATE;

            for (pos=0; pos<JOB_CHUNK; pos++) {
               if (drmaa_control(all_jobids[pos], ctrl_op, diagnosis, 
                                 sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                           all_jobids[pos], drmaa_ctrl2str(ctrl_op), diagnosis);
                  free_jobids(all_jobids, size_all_jobids);
                  return 1;
               }
            }
         } else {
            if (test_case == ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE)
               ctrl_op = DRMAA_CONTROL_RELEASE;
            else
               ctrl_op = DRMAA_CONTROL_TERMINATE;

            if (drmaa_control(DRMAA_JOB_IDS_SESSION_ALL, ctrl_op, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        DRMAA_JOB_IDS_SESSION_ALL, drmaa_ctrl2str(ctrl_op), diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
         }
         printf("released/terminated all jobs\n");

         /* synchronize with job to finish but do not dispose job finish information */
         if ((drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 0, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            free_jobids(all_jobids, size_all_jobids);
            return 1;
         }
         printf("synchronized with job finish\n");

         /* 
          * Verify job state of all jobs in the job id array 
          */
         for (pos=0; pos<JOB_CHUNK; pos++) {
            if (drmaa_job_ps(all_jobids[pos], &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_job_ps(\"%s\")) failed: %s\n", all_jobids[pos], diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

            printf("state of job \"%s\" is now %s\n", all_jobids[pos], drmaa_state2str(job_state));
            if (((test_case == ST_BULK_SUBMIT_IN_HOLD_SINGLE_RELEASE || 
                  test_case == ST_BULK_SUBMIT_IN_HOLD_SESSION_RELEASE) && job_state != DRMAA_PS_DONE) ||
                 ((test_case == ST_BULK_SUBMIT_IN_HOLD_SINGLE_DELETE || 
                   test_case == ST_BULK_SUBMIT_IN_HOLD_SESSION_DELETE) && job_state != DRMAA_PS_FAILED)) {
               fprintf(stderr, "job \"%s\" terminated with unexpected state \"%s\"\n", all_jobids[pos], drmaa_state2str(job_state));
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
         }

         free_jobids(all_jobids, size_all_jobids);

         if (wait_n_jobs(JOB_CHUNK) != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SUBMIT_SUSPEND_RESUME_WAIT:
      {
         int job_state, stat, exited, exit_status;
         char jobid[1024];

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
  
         /* submit a job running long enough allowing it to be suspended and resumed */
         if (!(jt = create_sleeper_job_template(30, 0, 0))) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }
         if (drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }
         printf("submitted job \"%s\"\n", jobid);
         drmaa_delete_job_template(jt, NULL, 0);

         /* wait until job is running */
         do {
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_job_ps() failed: %s\n", diagnosis);
               return 1;
            }
            if (job_state != DRMAA_PS_RUNNING)
               sleep(1);
         } while (job_state != DRMAA_PS_RUNNING);
         printf("job \"%s\" is now running\n", jobid);

         /* drmaa_control() is used to suspend the job */
         if ((drmaa_errno=drmaa_control(jobid, DRMAA_CONTROL_SUSPEND, diagnosis, 
                     sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_control(\"%s\", DRMAA_CONTROL_SUSPEND) failed: %s (%s)\n", 
                     jobid, diagnosis, drmaa_strerror(drmaa_errno));
            return 1;
         }
         printf("suspended job \"%s\"\n", jobid);
      
         /* drmaa_job_ps() is used to verify job was suspended */
         if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_job_ps() failed: %s\n", diagnosis);
            return 1;
         }
         if (job_state != DRMAA_PS_USER_SUSPENDED) {
            fprintf(stderr, "drmaa_job_ps(\"%s\") failed returns unexpected job state after "
                  "job suspension: %s\n", jobid, drmaa_state2str(job_state));
            return 1;
         }
         printf("verified suspend was done for job \"%s\"\n", jobid);

         /* drmaa_control() is used to resume the job */
         if ((drmaa_errno=drmaa_control(jobid, DRMAA_CONTROL_RESUME, diagnosis, 
                     sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_control(\"%s\", DRMAA_CONTROL_RESUME) failed: %s (%s)\n", 
                     jobid, diagnosis, drmaa_strerror(drmaa_errno));
            return 1;
         }
         printf("resumed job \"%s\"\n", jobid);

         /* drmaa_job_ps() is used to verify job was resumed */
         if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_job_ps() failed: %s\n", diagnosis);
            return 1;
         }
         if (job_state != DRMAA_PS_RUNNING) {
            fprintf(stderr, "drmaa_job_ps(\"%s\") failed returns unexpected job state after "
                  "job resume: %s\n", jobid, drmaa_state2str(job_state));
            return 1;
         }
         printf("verified resume was done for job \"%s\"\n", jobid);

         /* drmaa_wait() is used to wait for the jobs regular end */
         if ((drmaa_errno=drmaa_wait(jobid, NULL, 0, &stat, 
               DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_wait(\"%s\") failed: %s\n", jobid, diagnosis);
            return 1;
         }

         drmaa_wifexited(&exited, stat, NULL, 0); 
         if (!exited || (drmaa_wexitstatus(&exit_status, stat, NULL, 0), exit_status != 0)) {
            report_wrong_job_finish("expected regular job end", jobid, stat);
            return 1;
         }
         printf("job \"%s\" finished as expected\n", jobid);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_EMPTY_SESSION_WAIT:
   case ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE:
   case ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE:
      {
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
   
         switch (test_case) {
         case ST_EMPTY_SESSION_WAIT:
            /* drmaa_wait() must return DRMAA_ERRNO_INVALID_JOB */
            if ((drmaa_errno=drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, NULL, 0, NULL, 
                  DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_INVALID_JOB) {
               fprintf(stderr, "drmaa_wait(empty session) failed: %s\n", diagnosis);
               return 1;
            }
            break;
         case ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE:
         case ST_EMPTY_SESSION_SYNCHRONIZE_NODISPOSE:
            {
               const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };
               /* drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL) must return DRMAA_ERRNO_SUCCESS */
               if ((drmaa_errno=drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 
                    (test_case == ST_EMPTY_SESSION_SYNCHRONIZE_DISPOSE) ? 1 : 0, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_synchronize(empty session) failed: %s\n", diagnosis);
                  return 1;
               }
            }
            break;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_EMPTY_SESSION_CONTROL:
      {
         const char *s;

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
   
         /* parse control operation */
         if (parse_args) {
            s = NEXT_ARGV(argc, argv);
            if ((ctrl_op = str2drmaa_ctrl(s)) == -1) {
               fprintf(stderr, "unknown DRMAA control operation \"%s\"\n", s);
               usage();
            }
         }

         if ((drmaa_errno=drmaa_control(DRMAA_JOB_IDS_SESSION_ALL, ctrl_op,
                diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_control(empty_session, %s) failed: %s (%s)\n", 
                  drmaa_ctrl2str(ctrl_op), diagnosis, drmaa_strerror(drmaa_errno));
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_DRMAA_JOB_PS:
      {
         char diagnosis[1024];
         const char *jobid;

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         while ( *argc > 1) {
            int state;

            /* for this test args must always be parsed */
            jobid = NEXT_ARGV(argc, argv);
            if (drmaa_job_ps(jobid, &state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
                return 1;
            }
            printf("%20s %s\n", jobid, drmaa_state2str(state));
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_DRMAA_CONTROL:
      {
         char diagnosis[1024];
         const char *s, *jobid;
         int drmaa_target_errno, drmaa_errno, drmaa_control_op = -1;

         /* parse control operation */
         s = NEXT_ARGV(argc, argv);
         if ((drmaa_control_op = str2drmaa_ctrl(s)) == -1) {
            fprintf(stderr, "unknown DRMAA control operation \"%s\"\n", s);
            usage();
         }
         /* parse aspired errno value */
         s = NEXT_ARGV(argc, argv);
         if ((drmaa_target_errno = str2drmaa_errno(s)) == -1) {
            fprintf(stderr, "unknown DRMAA errno constant \"%s\"\n", s);
            usage();
         }

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         while ( *argc > 1) {
            jobid = NEXT_ARGV(argc, argv);
            if ((drmaa_errno=drmaa_control(jobid, drmaa_control_op, diagnosis, sizeof(diagnosis)-1))
                  != drmaa_target_errno) {

                if (drmaa_target_errno == DRMAA_ERRNO_SUCCESS) {
                   fprintf(stderr, "drmaa_control(\"%s\", %s) failed: %s (%s)\n", jobid, 
                        drmaa_ctrl2str(drmaa_control_op), diagnosis, drmaa_strerror(drmaa_errno));
                   return 1;
                } else {
                   fprintf(stderr, "drmaa_control(\"%s\", %s) returned with wrong errno "
                        "(%s) instead of %s: %s\n", jobid, drmaa_ctrl2str(drmaa_control_op), 
                        drmaa_errno2str(drmaa_errno), drmaa_errno2str(drmaa_target_errno), 
                              drmaa_errno==DRMAA_ERRNO_SUCCESS?"<no error>":diagnosis);
                   return 1;
                }
            }
            printf("%20s %s\n", jobid, drmaa_ctrl2str(drmaa_control_op));
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_EXIT_STATUS:
      /* - drmaa_init() is called
         - 255 job are submitted
         - job i returns i as exit status (8 bit)
         - drmaa_wait() verifies each job returned the
           correct exit status
         - then drmaa_exit() is called */
      {
         char diagnosis[1024];
         int size_all_jobids = 256;
         const char *all_jobids[256];
         const char *job_argv[2];
         char jobid[1024];
         char buffer[100];

         init_jobids(all_jobids, size_all_jobids);

         if (parse_args) {
            exit_job = NEXT_ARGV(argc, argv);
         }   
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   submit sequential jobs
          */
         if (!(jt = create_exit_job_template(exit_job, 0))) {
            fprintf(stderr, "create_exit_job_template() failed\n");
            return 1;
         }
         for (i=0; i<255; i++) {
  
            /* parametrize exit job with job argument */
            sprintf(buffer, "%d", i);
            job_argv[0] = buffer; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);

            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            printf("\t \"%s\"\n", jobid);
            all_jobids[i] = strdup(jobid);
         }

         /* set string array end mark */
         all_jobids[i] = NULL;

         drmaa_delete_job_template(jt, NULL, 0);

         /*
          *   wait for all jobs and verify exit status
          */
         
         for (i=0; i<255; i++) {
            int stat = 0;
            int exit_status = 0;
            int exited = 0;

            do {
               drmaa_errno = drmaa_wait(all_jobids[i], jobid, sizeof(jobid)-1, 
                  &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", all_jobids[i], diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("job %d with job id %s finished\n", i, all_jobids[i]);
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", all_jobids[i], diagnosis);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
          
            drmaa_wifexited(&exited, stat, NULL, 0);
            if (!exited) {
               fprintf(stderr, "job \"%s\" did not exit cleanly\n", all_jobids[i]);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }
            drmaa_wexitstatus(&exit_status, stat, NULL, 0);
            if (exit_status != i) {
               fprintf(stderr, "job \"%s\" returned wrong exit status %d instead of %d\n", 
                                 all_jobids[i], exit_status, i);
               free_jobids(all_jobids, size_all_jobids);
               return 1;
            }

         }

         free_jobids(all_jobids, size_all_jobids);

         printf("waited all jobs\n");
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         break;
      }

   case ST_ATTRIBUTE_CHECK:
      /* Need to test all DRMAA attributes:
         DRMAA_REMOTE_COMMAND
         DRMAA_V_ARGV
          - submit exit job
          - wait for job and check status code
         DRMAA_JS_STATE
         DRMAA_JOB_NAME
          - submit job in hold state with name
          - use drmaa_ps_job to verify state
          - use custom routine to verfiy name
          - release hold
         DRMAA_WD
         DRMAA_INPUT_PATH
         DRMAA_OUTPUT_PATH
          - reuse ST_INPUT_BECOMES_OUTPUT with files in /tmp
         DRMAA_ERROR_PATH
          - submit tar job with error path set
          - wait for job to finish
          - check contents of error file
         DRMAA_JOIN_FILES
          - submit tar job to create a sample tar file
          - wait for job to finish
          - submit tar job with output path and join files set
          - wait for job to finish
          - check contents of output file
         DRMAA_JOB_CATEGORY
          - submit job with job category containing -h and -N
          - use drmaa_ps_job to verify state
          - use sge_gdi to verify name
          - release hold
          - submit job with job category containing -h and -N and DRMAA_JOB_NAME
          - use sge_gdi to verify name
          - use drmaa_ps_job to verify state
          - release hold
          - wait for job to complete
         DRMAA_NATIVE_SPECIFICATION
          - submit job with -h and -N
          - use drmaa_ps_job to verify state
          - use sge_gdi to verify name
          - release hold
          - submit job with -h and -N and DRMAA_JOB_NAME
          - use sge_gdi to verify name
          - use drmaa_ps_job to verify state
          - release hold
          - wait for job to complete
         DRMAA_START_TIME
          - store the time
          - submit job with start time +1 min
          - wait for job to complete
          - compare the current time to stored time
         DRMAA_V_ENV
          - submit echo job with output path set
          - wait for job to finish
          - check output file
         DRMAA_BLOCK_EMAIL
         DRMAA_V_EMAIL */
      {
         int status, job_state, exit_status;
         int failed_test = 0, test_failed = 0;
         char diagnosis[1024];
         const char *job_argv[4];
         char jobid[1024], new_jobid[1024];
         char buffer[100];
         lList *alp, *job_lp;
         lListElem *job_ep;
         FILE *fp;
         const char *mirror_text = "thefoxjumps...";
         mirror_job = "/bin/cat";
         input_path = "test.in";
         output_path = "test.out";
         error_path = "test.err";

         if (parse_args) {
            exit_job = NEXT_ARGV(argc, argv);
            email_addr = NEXT_ARGV(argc, argv);
         }
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         /*
          *   test remote command and argv
          */
         do {
            printf ("Testing remote command and argv\n");
            printf ("Getting job template\n");

            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "5"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;               
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;               
               continue;
            }

            drmaa_wexitstatus(&exit_status, status, NULL, 0);

            if (exit_status != 5) {
               fprintf(stderr, "job \"%s\" returned wrong exit status %d instead of 5\n", 
                                 jobid, exit_status);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }
         
         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");

         /*
          * testing job submission state and job name
          */

         do {
            printf ("Testing job submission state and job name\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "5"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JS_STATE, DRMAA_SUBMISSION_STATE_HOLD, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOB_NAME, "ExitTest", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Getting job name for job %lu from GDI\n", (unsigned long)atol(jobid));
            {
               lCondition* where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, (u_long32)atol(jobid));
               lEnumeration *what = lWhat("%T (%I %I)", JB_Type, JB_job_number, JB_job_name);
               alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &job_lp, where, what, false);
               job_ep = lFirst(job_lp);
               lFreeWhere(&where);
               lFreeWhat(&what);
            }
            {
               int tmp_ret = answer_list_print_err_warn(&alp, "GDI Critical", "GDI Error: ", "Message from GDI: ");

               if (tmp_ret > 0) {
                  fprintf (stderr, "problem talking to gdi\n");
                  failed_test = 1;
               }
            }
            lFreeList(&alp);

            if (job_ep == NULL) {
               printf ("No such job number.\n");
               
               failed_test = 1;
            }
            else if ((job_ep != NULL) && (strcmp (lGetString (job_ep, JB_job_name), "ExitTest") != 0)) {
               fprintf(stderr, "Job \"%s\" name was \"%s\" instead of \"ExitTest\"\n", 
                                 jobid, lGetString (job_ep, JB_job_name));
               failed_test = 1;
            }

            lFreeList(&job_lp);
            
            printf ("Checking job state\n");
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
               fprintf (stderr, "Job \"%s\" was not in hold state\n", jobid);
               failed_test = 1;
            }

            printf ("Releasing job\n");
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        jobid, drmaa_ctrl2str(DRMAA_CONTROL_RELEASE), diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
            }
            
            if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }
         
         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing working directory, input stream and output stream
          */

         do {
            char abs_path[128];

            printf ("Testing working directory, input stream and output stream\n");

            printf ("Writing input file\n");
            
            strcpy (abs_path, "/tmp/");
            if (!(fp = fopen (strcat (abs_path, input_path), "w"))) {
               fprintf(stderr, "fopen(%s, w) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            fprintf(fp, "%s\n", mirror_text);
            FCLOSE(fp);
            
            printf ("Clearing output file\n");
            strcpy (abs_path, "/tmp/");
            if ((unlink (strcat (abs_path, output_path)) == -1) && (errno != ENOENT)) {
               fprintf(stderr, "unlink(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            drmaa_set_attribute(jt, DRMAA_WD, "/tmp", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, mirror_job, NULL, 0);
            set_path_attribute_plus_colon(jt, DRMAA_INPUT_PATH, input_path, NULL, 0);
            set_path_attribute_plus_colon(jt, DRMAA_OUTPUT_PATH, output_path, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            strcpy (abs_path, "/tmp/");
            if (!(fp=fopen(strcat (abs_path, output_path), "r"))) {
               fprintf(stderr, "fopen(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            if (fscanf(fp, "%s", buffer) != 1) {
               fprintf(stderr, "fscanf error\n");
               failed_test = 1;
               continue;
            }
               
            if (strcmp(buffer, mirror_text)) {
               fprintf(stderr, "Wrong output file: %s\n", buffer);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing error path
          */
         do {
            char abs_path[128];
            printf ("Testing error stream\n");
            
            printf ("Clearing error file\n");
            
            strcpy (abs_path, "/tmp/");
            if ((unlink (strcat (abs_path, error_path)) == -1) && (errno != ENOENT)) {
               fprintf(stderr, "unlink(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            drmaa_set_attribute(jt, DRMAA_WD, "/tmp", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "tar", NULL, 0);         
            set_path_attribute_plus_colon(jt, DRMAA_ERROR_PATH, error_path, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            strcpy (abs_path, "/tmp/");
            if (!(fp=fopen(strcat (abs_path, error_path), "r"))) {
               fprintf(stderr, "fopen(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            if (fscanf(fp, "%10c", buffer) != 1) {
               fprintf(stderr, "wrong output file: %s\n", buffer);
               failed_test = 1;
            }
            buffer[10] = '\0';
            if (strcmp("Usage: tar", buffer) != 0) {
               fprintf(stderr, "wrong output file: %s\n", buffer);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);

         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");

         /*
          * testing join files
          */
         do {
            /* First submit job to create tar file */
            char *tar_path = "test.tar";
            char abs_path[128]; 

            printf ("Testing join files\n");
            printf ("Running job to prepare data\n");
            printf ("Clearing output file\n");
            strcpy (abs_path, "/tmp/");
            if ((unlink (strcat (abs_path, output_path)) == -1) && (errno != ENOENT)) {
               fprintf(stderr, "unlink(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }
            
            printf ("Clearing tar file\n");
            strcpy (abs_path, "/tmp/");
            if ((unlink (strcat (abs_path, tar_path)) == -1) && (errno != ENOENT)) {
               fprintf(stderr, "unlink(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }
            
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "cvf"; 
            job_argv[1] = tar_path;
            job_argv[2] = input_path;
            job_argv[3] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_WD, "/tmp", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "tar", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            drmaa_delete_job_template(jt, NULL, 0);
            jt = NULL;
            
            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }
            
            /* submit job to read tar file */
            printf ("Running job to read data\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "tvf"; 
            job_argv[1] = "test.tar"; 
            job_argv[2] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_WD, "/tmp", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "tar", NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);         
            set_path_attribute_plus_colon(jt, DRMAA_OUTPUT_PATH, output_path, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            strcpy (abs_path, "/tmp/");
            if (!(fp=fopen(strcat (abs_path, output_path), "r"))) {
               fprintf(stderr, "fopen(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            if (fscanf(fp, "%14c%*[^\n]\n", buffer) != 1) {
               fprintf(stderr, "fscanf error\n");
               failed_test = 1;
            }
            buffer[14] = '\0';
            if (strcmp("tar: blocksize", buffer) != 0) {
               fprintf(stderr, "missing stderr from output file: %s\n", buffer);
               failed_test = 1;
            }

            if (fscanf(fp, "%4c\n", buffer) != 1) {
               fprintf(stderr, "fscanf error\n");
               failed_test = 1;
            }
            buffer[4] = '\0';
            if (strcmp("-rw-", buffer) != 0) {
               fprintf(stderr, "missing stdout from output file: %s\n", buffer);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);

         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing job category
          */
         do {
            printf ("Testing job category\n");
            printf ("$SGE_ROOT/$SGE_CELL/common/qtask should contain the following entry:\n");
            printf ("test.cat -N ExitTest -h\n");
            
            /* first test that it works */
            printf ("Testing job category standalone\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "10"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOB_CATEGORY, "test.cat", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            drmaa_delete_job_template(jt, NULL, 0);
            jt = NULL;

            printf ("Getting job name for job %lu from GDI\n", (unsigned long)atol(jobid));
            {
               lCondition *where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, (u_long32)atol(jobid));
               lEnumeration *what = lWhat("%T (%I %I)", JB_Type, JB_job_number, JB_job_name);
               alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &job_lp, where, what, false);
               job_ep = lFirst(job_lp);
               lFreeWhere(&where);
               lFreeWhat(&what);
            }
            {
               int tmp_ret = answer_list_print_err_warn(&alp, "GDI Critical", "GDI Error: ", "Message from GDI: ");

               if (tmp_ret > 0) {
                  fprintf (stderr, "problem talking to gdi\n");
                  failed_test = 1;
                  continue;
               }
            }
            lFreeList(&alp);         

            if (job_ep == NULL) {
               printf ("No such job number.\n");
               
               failed_test = 1;
            }
            else if ((job_ep != NULL) && (strcmp (lGetString (job_ep, JB_job_name), "ExitTest") != 0)) {
               fprintf(stderr, "Job \"%s\" name was \"%s\" instead of \"ExitTest\"\n", 
                                 jobid, lGetString (job_ep, JB_job_name));
               failed_test = 1;
            }

            lFreeList(&job_lp);

            printf ("Checking job state\n");
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
               fprintf (stderr, "Job \"%s\" was not in hold state\n", jobid);
               failed_test = 1;
            }

            printf ("Releasing job\n");
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        jobid, drmaa_ctrl2str(DRMAA_CONTROL_RELEASE), diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            /* then test that it doesn't override DRMAA attributes */
            printf ("Testing job category v/s DRMAA attributes\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "10"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOB_NAME, "DRMAAExitTest", NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOB_CATEGORY, "test.cat", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Getting job name for job %lu from GDI\n", (unsigned long)atol(jobid));
            {
               lCondition *where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, (u_long32)atol(jobid));
               lEnumeration *what = lWhat("%T (%I %I)", JB_Type, JB_job_number, JB_job_name);
               alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &job_lp, where, what, false);
               job_ep = lFirst(job_lp);
               lFreeWhere(&where);
               lFreeWhat(&what);
            }
            {
               int tmp_ret = answer_list_print_err_warn(&alp, "GDI Critical", "GDI Error: ", "Message from GDI: ");

               if (tmp_ret > 0) {
                  fprintf (stderr, "problem talking to gdi\n");
                  failed_test = 1;
               }
            }
            lFreeList(&alp);         

            if (job_ep == NULL) {
               printf ("No such job number.\n");
               
               failed_test = 1;
            }
            else if ((job_ep != NULL) && (strcmp (lGetString (job_ep, JB_job_name), "DRMAAExitTest") != 0)) {
               fprintf(stderr, "Job \"%s\" name was \"%s\" instead of \"DRMAAExitTest\"\n", 
                                 jobid, lGetString (job_ep, JB_job_name));
               failed_test = 1;
            }

            lFreeList(&job_lp);

            printf ("Checking job state\n");
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
               fprintf (stderr, "job \"%s\" was not in hold state\n", jobid);
               failed_test = 1;
            }

            printf ("Releasing job\n");
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        jobid, drmaa_ctrl2str(DRMAA_CONTROL_RELEASE), diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);

         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }
         
         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing native specification
          */
         do {
            printf ("Testing native specification\n");
            /* first test that it works */
            printf ("Testing native specification standalone\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "10"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, "-h -N ExitTest", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            drmaa_delete_job_template(jt, NULL, 0);
            jt = NULL;

            printf ("Getting job name for job %lu from GDI\n", (unsigned long)atol(jobid));
            {
               lCondition* where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, (u_long32)atol(jobid));
               lEnumeration *what = lWhat("%T (%I %I)", JB_Type, JB_job_number, JB_job_name);
               alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &job_lp, where, what, false);
               job_ep = lFirst(job_lp);
               lFreeWhere(&where);
               lFreeWhat(&what);
            }
            {
               int tmp_ret = answer_list_print_err_warn(&alp, "GDI Critical", "GDI Error: ", "Message from GDI: ");

               if (tmp_ret > 0) {
                  fprintf (stderr, "problem talking to gdi\n");
                  failed_test = 1;
                  continue;
               }
            }
            lFreeList(&alp);         

            if (job_ep == NULL) {
               printf ("No such job number.\n");
               
               failed_test = 1;
            }
            else if ((job_ep != NULL) && (strcmp (lGetString (job_ep, JB_job_name), "ExitTest") != 0)) {
               fprintf(stderr, "Job \"%s\" name was \"%s\" instead of \"ExitTest\"\n", 
                                 jobid, lGetString (job_ep, JB_job_name));
               failed_test = 1;
            }

            lFreeList(&job_lp);

            printf ("Checking job state\n");
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
               fprintf (stderr, "Job \"%s\" was not in hold state\n", jobid);
               failed_test = 1;
            }

            printf ("Releasing job\n");
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        jobid, drmaa_ctrl2str(DRMAA_CONTROL_RELEASE), diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            /* then test that it doesn't override DRMAA attributes */
            printf ("Testing native specification v/s DRMAA attributes\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "10"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_JOB_NAME, "DRMAAExitTest", NULL, 0);         
            drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, "-h -N ExitTest", NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Getting job name for job %lu from GDI\n", (unsigned long)atol(jobid));
            {
               lCondition *where = lWhere ("%T(%I==%u)", JB_Type, JB_job_number, (u_long32)atol(jobid));
               lEnumeration *what = lWhat("%T (%I %I)", JB_Type, JB_job_number, JB_job_name);
               alp = ctx->gdi(ctx, SGE_JB_LIST, SGE_GDI_GET, &job_lp, where, what, false);
               job_ep = lFirst(job_lp);
               lFreeWhere(&where);
               lFreeWhat(&what);
            }
            {
               int tmp_ret = answer_list_print_err_warn(&alp, "GDI Critical", "GDI Error: ", "Message from GDI: ");

               if (tmp_ret > 0) {
                  fprintf (stderr, "problem talking to gdi\n");
                  failed_test = 1;
               }
            }
            lFreeList(&alp);         

            if (job_ep == NULL) {
               printf ("No such job number.\n");
               
               failed_test = 1;
            }
            else if ((job_ep != NULL) && (strcmp (lGetString (job_ep, JB_job_name), "DRMAAExitTest") != 0)) {
               fprintf(stderr, "Job \"%s\" name was \"%s\" instead of \"DRMAAExitTest\"\n", 
                                 jobid, lGetString (job_ep, JB_job_name));
               failed_test = 1;
            }

            lFreeList(&job_lp);

            printf ("Checking job state\n");
            if (drmaa_job_ps(jobid, &job_state, diagnosis, sizeof(diagnosis)-1)
                  !=DRMAA_ERRNO_SUCCESS) {
                fprintf(stderr, "drmaa_job_ps(\"%s\") failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            if (job_state != DRMAA_PS_USER_ON_HOLD && job_state != DRMAA_PS_USER_SYSTEM_ON_HOLD) {
               fprintf (stderr, "job \"%s\" was not in hold state\n", jobid);
               failed_test = 1;
            }

            printf ("Releasing job\n");
            if (drmaa_control(jobid, DRMAA_CONTROL_RELEASE, diagnosis, 
                              sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_control(%s, %s) failed: %s\n", 
                        jobid, drmaa_ctrl2str(DRMAA_CONTROL_RELEASE), diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);

         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing start time
          */
         do {
            time_t now, later;
            struct tm timenow;
            struct tm timelater;
            char timestr[32];
            int time_diff;  
            printf ("Testing start time\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "0"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);         
            
            time (&now);
            localtime_r (&now, &timenow);
            
            timenow.tm_min++;
            
            /* This fails at midnight. */
            if (timenow.tm_min == 0) {
               timenow.tm_hour++;
            }
            
            sprintf (timestr, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d", timenow.tm_year + 1900,
                     timenow.tm_mon + 1, timenow.tm_mday, timenow.tm_hour,
                     timenow.tm_min, timenow.tm_sec);
            printf ("%s\n", timestr);
            
            drmaa_set_attribute(jt, DRMAA_START_TIME, timestr, NULL, 0);         

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }
            
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }
            
            printf ("Waiting for job to complete\n");            
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            time (&later);
            
            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }
            
            localtime_r (&now, &timenow);            
            localtime_r (&later, &timelater);
            
            time_diff = (((timelater.tm_hour * 60) + timelater.tm_min) * 60 + timelater.tm_sec) -
                (((timenow.tm_hour * 60) + timenow.tm_min) * 60 + timenow.tm_sec);
            
            /* Allow 10 seconds for scheduling and run time.  This test will fail
             * if run at midnight. */
            if (time_diff > 80) {
               printf ("Job took %d seconds longer than expected\n", time_diff - 60);
               failed_test = 1;
            }
            else if (time_diff < 0) {
               printf ("Job finished in %d seconds\n", 
                       ((timelater.tm_hour * 60) + timelater.tm_min) * 60 + timelater.tm_sec);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);
            
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing job environment
          */

         do {
            const char *job_env[2];
            char abs_path[128];
            
            printf ("Testing job environment\n");

            printf ("Clearing output file\n");
            strcpy (abs_path, "/tmp/");
            if ((unlink (strcat (abs_path, output_path)) == -1) && (errno != ENOENT)) {
               fprintf(stderr, "unlink(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            job_argv[0] = "$YOU_ARE_MY_SUNSHINE"; 
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            job_env[0] = "YOU_ARE_MY_SUNSHINE=MyOnlySunshine";
            job_env[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ENV, job_env, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_WD, "/tmp", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, "/usr/bin/echo", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, "-shell y", NULL, 0);
            set_path_attribute_plus_colon(jt, DRMAA_OUTPUT_PATH, output_path, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            strcpy (abs_path, "/tmp/");
            if (!(fp=fopen(strcat (abs_path, output_path), "r"))) {
               fprintf(stderr, "fopen(%s) failed: %s\n", abs_path, strerror(errno));
               failed_test = 1;
               continue;
            }

            if (fscanf(fp, "%s", buffer) != 1) {
               fprintf(stderr, "fscanf error\n");
               failed_test = 1;
            }
            if (strcmp(buffer, "MyOnlySunshine")) {
               fprintf(stderr, "Wrong output file: %s\n", buffer);
               failed_test = 1;
            }
            else if (!failed_test) {
               printf ("Test succeeded!\n");
            }
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         failed_test = 0;
         printf("=====================\n");
         
         /*
          * testing email address
          */

         do {
            const char *email[3];
            
            printf ("Testing email address\n");
            printf ("$SGE_ROOT/$SGE_CELL/common/sge_request should contain the following entry:\n");
            printf ("-m e\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            email[0] = email_addr; 
            email[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_EMAIL, email, NULL, 0);
            job_argv[0] = "0";
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Check for email to find out if the test succeeded.\n");
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }
         
         if (failed_test) test_failed = 1;
         failed_test = 0;
         
         /*
          * testing email supression
          */
         printf("=====================\n");

         do {
            const char *email[2];
            
            printf ("Testing email supression\n");
            printf ("$SGE_ROOT/$SGE_CELL/common/sge_request should contain the following entry:\n");
            printf ("-m e\n");
            printf ("Getting job template\n");
            drmaa_allocate_job_template(&jt, NULL, 0);

            if (jt == NULL) {
               fprintf(stderr, "drmaa_allocate_job_template() failed\n");
               failed_test = 1;
               continue;
            }

            printf ("Filling job template\n");
            email[0] = email_addr; 
            email[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_EMAIL, email, NULL, 0);
            job_argv[0] = "0";
            job_argv[1] = NULL;
            drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
            drmaa_set_attribute(jt, DRMAA_BLOCK_EMAIL, "1", NULL, 0);
            drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);

            printf ("Running job\n");
            while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                     sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
               fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
               sleep(1);
            }

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Waiting for job to complete\n");
            do {
               drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
                  &status, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, diagnosis, sizeof(diagnosis)-1);
               if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
                  fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
                  sleep(1);
               }
            } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

            printf("Job with job id %s finished\n", jobid);

            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
               failed_test = 1;
               continue;
            }

            printf ("Check for email to find out if the test failed.\n");
         } while (do_while_end);
         
         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }

         if (failed_test) test_failed = 1;
         
         /*
          * shutdown session
          */
         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
         
         return test_failed;
      }
   case ST_USAGE_CHECK:
      {
         char jobid[1024], value[128], new_jobid[1024];
         drmaa_attr_values_t *rusage = NULL;
         int status;
         int size = 0;

         if (parse_args)
            exit_job = NEXT_ARGV(argc, argv);
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         report_session_key();

         jt = create_exit_job_template(exit_job, 0);

         printf ("Running job\n");
         while ((drmaa_errno=drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                  sizeof(diagnosis)-1)) == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) {
            fprintf(stderr, "drmaa_run_job() failed - retry: %s\n", diagnosis);
            sleep(1);
         }

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }

         if (jt != NULL) { drmaa_delete_job_template(jt, NULL, 0); jt = NULL; }
         
         printf ("Waiting for job to complete\n");
         do {
            drmaa_errno = drmaa_wait(jobid, new_jobid, sizeof(jobid)-1, 
               &status, DRMAA_TIMEOUT_WAIT_FOREVER, &rusage, diagnosis, sizeof(diagnosis)-1);
            if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_wait(%s) failed - retry: %s\n", jobid, diagnosis); 
               sleep(1);
            }
         } while (drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE);

         printf("Job with job id %s finished\n", jobid);

         if (drmaa_errno == DRMAA_ERRNO_NO_RUSAGE) {
            fprintf(stderr, "drmaa_wait(%s) did not return usage information.\n", jobid);
            return 1;
         } else if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_wait(%s) failed: %s\n", jobid, diagnosis);
            drmaa_release_attr_values(rusage);
            return 1;
         } else if (rusage == NULL) {
            fprintf (stderr, "drmaa_wait(%s) did not return usage information and did not return DRMAA_ERRNO_NO_RUSAGE\n", jobid);
            return 1;
         }
         
         drmaa_errno = drmaa_get_num_attr_values(rusage, &size);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_num_attr_values() failed: %s\n", drmaa_strerror(drmaa_errno));
            drmaa_release_attr_values(rusage);
            rusage = NULL;
            return 1;
         }
         
         while ((drmaa_errno=drmaa_get_next_attr_value(rusage, value, 127))==DRMAA_ERRNO_SUCCESS) {
            size--;
            printf("%s\n", value);
         }

         drmaa_release_attr_values(rusage);
         rusage = NULL;
      
         if (size != 0) {
            fprintf(stderr, "Got incorrect size from drmaa_get_num_attr_values()\n");
            return 1;
         }
         
         if (drmaa_errno != DRMAA_ERRNO_NO_MORE_ELEMENTS) {
            fprintf(stderr, "Got incorrect return value from drmaa_get_next_attr_value()\n");
            return 1;
         }
         
         break;
      }
   case ST_TRANSFER_FILES_BULK_JOB:
      bBulkJob = true;
   case ST_TRANSFER_FILES_SINGLE_JOB:
      {
         int aborted, stat, remote_ps;
         bool bFound=false;
         char jobid[100];
         char *szPath;
         char *szTemp;
         char attr_name[DRMAA_ATTR_BUFFER];
         drmaa_attr_names_t *vector = NULL;
         const char *session_all[] = { DRMAA_JOB_IDS_SESSION_ALL, NULL };

         if (parse_args)
            sleeper_job = NEXT_ARGV(argc, argv);

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();
       
         /* submit a working job from a local directory to the execution host */
         drmaa_allocate_job_template(&jt, NULL, 0);

         drmaa_errno = drmaa_get_attribute_names(&vector, diagnosis, sizeof(diagnosis)-1);
         while((drmaa_errno=drmaa_get_next_attr_name(vector, attr_name,
                sizeof(attr_name)-1)) == DRMAA_ERRNO_SUCCESS) {
            if( strcmp( attr_name, "drmaa_transfer_files" )==0 ) {
               bFound=true;
               break;
            }
         }
         /* we don't need vector any longer - free it */
         drmaa_release_attr_names(vector);
         vector = NULL;

         if( !bFound ) {
            fprintf( stderr, "DRMAA_TRANSFER_FILES is not supported!\n" );
            return 1;
         }
         drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, sleeper_job, NULL, 0);

         szTemp = NEXT_ARGV(argc,argv);
         drmaa_set_attribute(jt, DRMAA_TRANSFER_FILES, szTemp, NULL, 0);
        
         szTemp = NEXT_ARGV(argc,argv);
         drmaa_set_attribute(jt, DRMAA_JOIN_FILES, szTemp, NULL, 0);

         if( !strcmp( (szPath=NEXT_ARGV(argc,argv)), "NULL" )) {
            szPath="";
         }
         drmaa_set_attribute(jt, DRMAA_INPUT_PATH, szPath, NULL, 0);

         if( !strcmp( (szPath=NEXT_ARGV(argc,argv)), "NULL" )) {
            szPath="";
         }
         drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH,szPath, NULL, 0);

         if( !strcmp( (szPath=NEXT_ARGV(argc,argv)), "NULL" )) {
            szPath="";
         }
         drmaa_set_attribute(jt, DRMAA_ERROR_PATH, szPath, NULL, 0);

         if( bBulkJob ) {
            drmaa_job_ids_t *jobids;
            int j;
         
            if((drmaa_errno=drmaa_run_bulk_jobs(&jobids, jt, 1, 3, 1,
               diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
               printf("failed submitting bulk job (%s): %s\n", drmaa_strerror(drmaa_errno), diagnosis);
               return 1;
            } 

            printf("submitted bulk job with jobids:\n");
            for (j=0; j<3; j++) {
               drmaa_get_next_job_id(jobids, jobid, sizeof(jobid)-1);
               printf("\t \"%s\"\n", jobid);
            } 
            drmaa_release_job_ids(jobids);

         } else {
            /* synchronize with job to finish but do not dispose job finish information */
            if((drmaa_errno = drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis,
                sizeof(diagnosis)-1)) != DRMAA_ERRNO_SUCCESS) {
               fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
               return 1;
            }
         }

         drmaa_delete_job_template(jt, NULL, 0);

         /* synchronize with job to finish but do not dispose job finish information */
         if ((drmaa_errno = drmaa_synchronize(session_all, DRMAA_TIMEOUT_WAIT_FOREVER, 0, 
                     diagnosis, sizeof(diagnosis)-1))!=DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_synchronize(DRMAA_JOB_IDS_SESSION_ALL, dispose) failed: %s\n", diagnosis);
            return 1;
         }
         printf("synchronized with job finish\n");

         /* get job state */
         drmaa_errno = drmaa_job_ps(jobid, &remote_ps, diagnosis, sizeof(diagnosis)-1);
         if (remote_ps == DRMAA_PS_FAILED) {
            fprintf(stderr, "job \"%s\" is not in failed state: %s\n", 
                     jobid, drmaa_state2str(remote_ps));
            return 1;
         }

         /* wait job */
         if ((drmaa_errno = drmaa_wait(jobid, NULL, 0, &stat, DRMAA_TIMEOUT_WAIT_FOREVER, NULL, 
               diagnosis, sizeof(diagnosis)-1)) != DRMAA_ERRNO_SUCCESS) {
            printf("drmaa_wait() failed %s: %s\n", drmaa_strerror(drmaa_errno), diagnosis);
            return 1;
         }

         /* job finish information */
         drmaa_wifaborted(&aborted, stat, diagnosis, sizeof(diagnosis)-1);
         if(!aborted) {
            fprintf(stderr,
               "job \"%s\" failed but drmaa_wifaborted() returns false\n", jobid);
            return 1;
         }
         printf("waited job \"%s\" that never ran\n", jobid);

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_RESERVATION_FINISH_ORDER:
   case ST_BACKFILL_FINISH_ORDER:
      {
         test_job_t job_spec[3];

         if (parse_args) {
            sleeper_job            = NEXT_ARGV(argc, argv);
            job_spec[0].native     = NEXT_ARGV(argc, argv);
            job_spec[0].time       = 10;
            job_spec[1].native     = NEXT_ARGV(argc, argv);
            job_spec[1].time       = 10;
            job_spec[2].native     = NEXT_ARGV(argc, argv);
            job_spec[2].time       = 10;
         }

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         if (test_dispatch_order_njobs(3, job_spec, test_case==ST_RESERVATION_FINISH_ORDER?"0-1-2":"0,2-1")) {
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_WILD_PARALLEL:
      {
         test_job_t job_spec[7];

         if (parse_args) {
            sleeper_job            = NEXT_ARGV(argc, argv);

            job_spec[0].native     = NEXT_ARGV(argc, argv);
            job_spec[0].time       = 20;
            job_spec[1].native     = job_spec[0].native;
            job_spec[1].time       = 20;
            job_spec[2].native     = job_spec[0].native;
            job_spec[2].time       = 20;
            job_spec[3].native     = job_spec[0].native;
            job_spec[3].time       = 20;

            job_spec[4].native     = NEXT_ARGV(argc, argv);
            job_spec[4].time       = 20;
            job_spec[5].native     = job_spec[4].native;
            job_spec[5].time       = 20;

            job_spec[6].native     = NEXT_ARGV(argc, argv);
            job_spec[6].time       = 20;
         }

         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         report_session_key();

         if (test_dispatch_order_njobs(7, job_spec, "6-4,5-0,1,2,3")) {
            return 1;
         }

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_UNSUPPORTED_ATTR:
   case ST_UNSUPPORTED_VATTR:
      {
         drmaa_job_template_t *jt = NULL;
         const char *values[2];
         
         values[0] = "blah";
         values[1] = NULL;
         
         if (drmaa_init(NULL, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         /* submit a working job from a local directory to the execution host */
         drmaa_allocate_job_template(&jt, NULL, 0);
         
         if (test_case == ST_UNSUPPORTED_ATTR) {
            drmaa_errno = drmaa_set_attribute(jt, "blah", "blah", diagnosis, sizeof(diagnosis)-1);
         } else {
            drmaa_errno = drmaa_set_vector_attribute(jt, "blah", (const char**)values, diagnosis, sizeof(diagnosis)-1);
         }

         drmaa_delete_job_template(jt, NULL, 0);
         jt = NULL;

         if (drmaa_errno != DRMAA_ERRNO_INVALID_ARGUMENT) {
            fprintf(stderr, "drmaa_set_attribute()/drmaa_set_vector_attribute() allowed invalid attribute\n");
            return 1;
         }             

         if (drmaa_exit(diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_SYNCHRONIZE_NONEXISTANT:
      {
         char jobid[1024];
         const char *all_jobids[2];
         int new_id = 0;
         int drmaa_errno = DRMAA_ERRNO_SUCCESS;

         if (parse_args) {
            sleeper_job = NEXT_ARGV(argc, argv);
         }
         
         drmaa_errno = drmaa_init(NULL, diagnosis, sizeof(diagnosis) - 1);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         report_session_key();

         jt = create_sleeper_job_template(5, 0, 0);
         
         if (jt == NULL) {
            fprintf(stderr, "create_sleeper_job_template() failed\n");
            return 1;
         }

         drmaa_errno = drmaa_run_job(jobid, sizeof(jobid) - 1, jt, diagnosis,
                                     sizeof(diagnosis) - 1);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            return 1;
         }

         printf("submitted job \"%s\"\n", jobid);

         drmaa_delete_job_template(jt, NULL, 0);

         /* Convert job id into a number and add 1. */
         new_id = strtol(jobid, NULL, 10) + 1;
         printf ("Last job id is %s.  Using %d.\n", jobid, new_id);

         /* Build job id list. */
         sprintf(jobid, "%d", new_id);
         all_jobids[0] = jobid;
         all_jobids[1] = NULL;

         /* Synchronize on the new job id. */
         drmaa_errno = drmaa_synchronize(all_jobids, DRMAA_TIMEOUT_WAIT_FOREVER,
                                         0, diagnosis,
                                         DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "Synchronize on non-existant job id failed\n");
            return 1;
         }
         
         drmaa_errno = wait_all_jobs(1);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            return 1;
         }
         
         drmaa_errno = drmaa_exit(diagnosis, sizeof(diagnosis) - 1);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }
      }
      break;

   case ST_RECOVERABLE_SESSION:
      {
         char jobid1[DRMAA_JOBNAME_BUFFER + 1];
         char jobid2[DRMAA_JOBNAME_BUFFER + 1];
         char jobid3[DRMAA_JOBNAME_BUFFER + 1];
         char jobid4[DRMAA_JOBNAME_BUFFER + 1];
         char buffer[DRMAA_JOBNAME_BUFFER + 1];
         char contact[DRMAA_CONTACT_BUFFER + 1];
         int drmaa_errno = DRMAA_ERRNO_SUCCESS;
         int exit_code = 0;
         int stat = 0;
         drmaa_job_ids_t *bulk_job_ids = NULL;
         drmaa_attr_values_t *rusage = NULL;

         if (parse_args) {
            sleeper_job = NEXT_ARGV(argc, argv);
         }
         
         if (drmaa_init("", diagnosis, DRMAA_ERROR_STRING_BUFFER) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }
         
         if (drmaa_get_contact(contact, DRMAA_CONTACT_BUFFER, diagnosis,
                               DRMAA_ERROR_STRING_BUFFER) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_get_contact() failed: %s\n", diagnosis);
            return 1;
         }

         printf ("Contact string is \"%s\"\n", contact);
         
         /* Run long job. */
         jt = create_sleeper_job_template(120, 0, 0);

         if (jt == NULL) {
            fprintf(stderr, "create_job_template() failed\n");
            exit_code = 1;
            goto error;
         }

         drmaa_errno = drmaa_run_job(jobid1, DRMAA_JOBNAME_BUFFER, jt, diagnosis,
                                     DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s %s\n", diagnosis,
                    drmaa_strerror(drmaa_errno));
            exit_code = 1;
            goto error;
         }

         drmaa_delete_job_template(jt, diagnosis, DRMAA_ERROR_STRING_BUFFER);

         /* Run short job. */
         jt = create_sleeper_job_template(10, 0, 0);

         if (jt == NULL) {
            fprintf(stderr, "create_job_template() failed\n");
            exit_code = 1;
            goto error;
         }

         drmaa_errno = drmaa_run_job(jobid2, DRMAA_JOBNAME_BUFFER, jt, diagnosis,
                                     DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s %s\n", diagnosis,
                    drmaa_strerror(drmaa_errno));
            exit_code = 1;
            goto error;
         }

         drmaa_delete_job_template(jt, diagnosis, DRMAA_ERROR_STRING_BUFFER);

         /* Run bulk job. */
         jt = create_sleeper_job_template(10, 1, 1);

         if (jt == NULL) {
            fprintf(stderr, "create_job_template() failed\n");
            exit_code = 1;
            goto error;
         }

         drmaa_errno = drmaa_run_bulk_jobs(&bulk_job_ids, jt, 1, 2, 1,
                                           diagnosis,
                                           DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_bulk_jobs() failed: %s %s\n", diagnosis,
                    drmaa_strerror(drmaa_errno));
            exit_code = 1;
            goto error;
         }

         drmaa_delete_job_template(jt, diagnosis, DRMAA_ERROR_STRING_BUFFER);

         /* Release one of the bulk jobs */
         drmaa_get_next_job_id(bulk_job_ids, jobid3, DRMAA_JOBNAME_BUFFER);
         drmaa_get_next_job_id(bulk_job_ids, jobid4, DRMAA_JOBNAME_BUFFER);
         drmaa_release_job_ids(bulk_job_ids);
         
         drmaa_errno = drmaa_control(jobid3, DRMAA_CONTROL_RELEASE, diagnosis,
                                     DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_control() failed: %s %s\n", diagnosis,
                    drmaa_strerror(drmaa_errno));
            exit_code = 1;
            goto error;
         }
         
         /* Stop and restart the session. */
         if (drmaa_exit(diagnosis, DRMAA_ERROR_STRING_BUFFER) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            return 1;
         }

         /* Sleep long enough for the short jobs to finish. */
         printf ("Sleeping for 60 seconds\n");
         sleep(60);
         printf ("Done sleeping\n");
         
         if (drmaa_init(contact, diagnosis, DRMAA_ERROR_STRING_BUFFER) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_init() failed: %s\n", diagnosis);
            return 1;
         }

         /* Wait for the long job to finish. */
         drmaa_errno = drmaa_wait(jobid1, buffer, DRMAA_JOBNAME_BUFFER, &stat,
                                  DRMAA_TIMEOUT_WAIT_FOREVER, &rusage, diagnosis,
                                  DRMAA_ERROR_STRING_BUFFER);

         /* we don't use rusage here - free it! */
         drmaa_release_attr_values(rusage);
         rusage = NULL;

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_wait() failed: %s\n", diagnosis);
            exit_code = 1;
            goto error;
         }

         /* Wait for the short job to finish. */
         drmaa_errno = drmaa_wait(jobid2, buffer, DRMAA_JOBNAME_BUFFER, &stat,
                                  DRMAA_TIMEOUT_WAIT_FOREVER, &rusage, diagnosis,
                                  DRMAA_ERROR_STRING_BUFFER);

         /* we don't use rusage here - free it! */
         drmaa_release_attr_values(rusage);
         rusage = NULL;

         if ((drmaa_errno != DRMAA_ERRNO_INVALID_JOB) &&
             (drmaa_errno != DRMAA_ERRNO_NO_RUSAGE)) {
            fprintf(stderr, "drmaa_wait() did not fail as expected: %s\n",
                    diagnosis);
            exit_code = 1;
            goto error;
         }

         /* Wait for the bulk jobs to finish. */
         drmaa_errno = drmaa_wait(jobid3, buffer, DRMAA_JOBNAME_BUFFER, &stat,
                                  DRMAA_TIMEOUT_WAIT_FOREVER, &rusage, diagnosis,
                                  DRMAA_ERROR_STRING_BUFFER);

         /* we don't use rusage here - free it! */
         drmaa_release_attr_values(rusage);
         rusage = NULL;

         /* This one must be found, because another task in this job is still
          * held.  The only option is to complain about no rusage info. */
         if (drmaa_errno != DRMAA_ERRNO_NO_RUSAGE) {
            fprintf(stderr, "drmaa_wait() did not fail as expected: %s\n",
                    diagnosis);
            exit_code = 1;
            goto error;
         }

         drmaa_errno = drmaa_wait(jobid4, buffer, DRMAA_JOBNAME_BUFFER, &stat,
                                  DRMAA_TIMEOUT_NO_WAIT, &rusage, diagnosis,
                                  DRMAA_ERROR_STRING_BUFFER);

         /* we don't use rusage here - free it! */
         drmaa_release_attr_values(rusage);
         rusage = NULL;

         if (drmaa_errno != DRMAA_ERRNO_EXIT_TIMEOUT) {
            fprintf(stderr, "drmaa_wait() did not fail as expected: %s\n",
                    diagnosis);
            exit_code = 1;
            goto error;
         }

         drmaa_errno = drmaa_control(jobid4, DRMAA_CONTROL_TERMINATE, diagnosis,
                                     DRMAA_ERROR_STRING_BUFFER);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_control() failed: %s %s\n", diagnosis,
                    drmaa_strerror(drmaa_errno));
            exit_code = 1;
            goto error;
         }

      error:
         if (drmaa_exit(diagnosis, DRMAA_ERROR_STRING_BUFFER) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_exit() failed: %s\n", diagnosis);
            exit_code = 1;
         }

         if (exit_code != 0) {
            return exit_code;
         }
      }
      break;

   default:
      break;
   }

   return 0;
FCLOSE_ERROR:
   fprintf(stderr, MSG_FILE_NOCLOSE_SS, input_path, strerror(errno));
   return 1;
}


static void *submit_and_wait_thread (void *vp) {
   int n;
   
   if (vp != NULL) {
      n = *(int *)vp;
   }
   else {
      n = 1;
   }
   
   submit_and_wait (n);
   
   return (void *)NULL;
}

static int submit_and_wait(int n)
{
   int ret = DRMAA_ERRNO_SUCCESS;
   
   ret = submit_sleeper(n);
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = wait_all_jobs(n);
   }

   return ret;
}

static void *submit_sleeper_thread (void *vp) {
   int n;
   
   if (vp != NULL) {
      n = *(int *)vp;
   }
   else {
      n = 1;
   }

   submit_sleeper(n);
   
   return (void *)NULL;
}

static drmaa_job_template_t *create_exit_job_template(const char *exit_job, int as_bulk_job)
{
   const char *job_argv[2];
   drmaa_job_template_t *jt = NULL;
   int ret = DRMAA_ERRNO_SUCCESS;

   ret = drmaa_allocate_job_template(&jt, NULL, 0);
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_WD, DRMAA_PLACEHOLDER_HD, NULL, 0);
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, exit_job, NULL, 0);
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {
      job_argv[0] = "0"; 
      job_argv[1] = NULL;
      ret = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);
   }

#if 0
   if (!as_bulk_job) {
      ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB.$JOB_ID", NULL, 0);
   }
   else {
      ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB.$JOB_ID."DRMAA_PLACEHOLDER_INCR, NULL, 0);
   }
#else
   if (ret == DRMAA_ERRNO_SUCCESS) {
      /* no output please */
      ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/dev/null", NULL, 0);
   }
#endif

   if (ret == DRMAA_ERRNO_SUCCESS) {   
      return jt;
   } else {
      drmaa_delete_job_template(jt, NULL, 0);
      return NULL;
   }
}

static drmaa_job_template_t *create_sleeper_job_template(int seconds, int as_bulk_job, int in_hold)
{
   const char *job_argv[2];
   drmaa_job_template_t *jt = NULL;
   char buffer[100];
   int ret = DRMAA_ERRNO_SUCCESS;

   ret = drmaa_allocate_job_template(&jt, NULL, 0);
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_WD, DRMAA_PLACEHOLDER_HD, NULL, 0);
   }
      
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, sleeper_job, NULL, 0);
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      sprintf(buffer, "%d", seconds);
      job_argv[0] = buffer; 
      job_argv[1] = NULL;
      ret = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, job_argv, NULL, 0);
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {
#if 0
      if (!as_bulk_job) {
         ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB.$JOB_ID", NULL, 0);
      }
      else {
         ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":"DRMAA_PLACEHOLDER_HD"/DRMAA_JOB.$JOB_ID."DRMAA_PLACEHOLDER_INCR, NULL, 0);
      }
#else
      /* no output please */
      ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/dev/null", NULL, 0);
#endif
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {
      if (in_hold) {
         ret = drmaa_set_attribute(jt, DRMAA_JS_STATE, DRMAA_SUBMISSION_STATE_HOLD, NULL, 0);
      }
   }

   if (ret == DRMAA_ERRNO_SUCCESS) {   
      return jt;
   } else {
      drmaa_delete_job_template(jt, NULL, 0);
      return NULL;
   }
}

static int submit_sleeper(int n)
{
   drmaa_job_template_t *jt;
   int ret = DRMAA_ERRNO_SUCCESS;

   jt = create_sleeper_job_template(10, 0, 0);
   
   if (jt != NULL) {
      ret = do_submit(jt, n);
   
      /* We don't care about the error code from this one.  It doesn't affect
       * anything. */
      drmaa_delete_job_template(jt, NULL, 0);
      jt = NULL;
   }

   return ret;
}

static int submit_input_mirror(int n, const char *mirror_job, 
                               const char *input_path, const char *output_path,
                               const char *error_path, int join, char* hostname)
{
   drmaa_job_template_t *jt = NULL;
   char buffer[10000];
   int ret = DRMAA_ERRNO_SUCCESS;

   ret = drmaa_allocate_job_template(&jt, NULL, 0);
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_WD, DRMAA_PLACEHOLDER_HD, NULL, 0);
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, mirror_job, NULL, 0);
   }

   /*
    *  we use the local host for the cat job, because when job is running
    *  on other hosts there my be NFS problems when reading the input_path file
    */
   if (ret == DRMAA_ERRNO_SUCCESS && hostname != NULL) {
      snprintf(buffer, 10000, "-l h=%s", hostname);
      ret = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, buffer, NULL, 0);
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      if (join) {
         ret = drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "y", NULL, 0);
      }
      else {
         ret = drmaa_set_attribute(jt, DRMAA_JOIN_FILES, "n", NULL, 0);
      }
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      if (input_path) {
         strcpy(buffer, ":");
         strcat(buffer, input_path);
         ret = drmaa_set_attribute(jt, DRMAA_INPUT_PATH, buffer, NULL, 0);
      }
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      if (output_path) {
         strcpy(buffer, ":");
         strcat(buffer, output_path);
         ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, buffer, NULL, 0);
      }
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      if (error_path) {
         strcpy(buffer, ":");
         strcat(buffer, error_path);
         ret = drmaa_set_attribute(jt, DRMAA_ERROR_PATH, buffer, NULL, 0);
      }
   }
   
   if (ret == DRMAA_ERRNO_SUCCESS) {
      ret = do_submit(jt, n);

      /* We don't care about the error code here because it doesn't affect
       * anything. */
      drmaa_delete_job_template(jt, NULL, 0);
   }

   return ret;
}

static int do_submit(drmaa_job_template_t *jt, int n)
{
   int i;
   char diagnosis[1024];
   char jobid[100];
   int drmaa_errno = DRMAA_ERRNO_SUCCESS;
   int error = DRMAA_ERRNO_SUCCESS;
   bool done;

   for (i=0; i<n; i++) {
      /* submit job */
      done = false;
      while (!done) {
         drmaa_errno = drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1);

         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            printf("failed submitting job (%s)\n", drmaa_strerror(drmaa_errno));
         }
         
         /* Only retry on "try again" error. */
         if (drmaa_errno == DRMAA_ERRNO_TRY_LATER) {
            printf("retry: %s\n", diagnosis);
            sleep(1);
         } else {
            done = true;
            break; /* while */
         }
      }
      
      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {
         printf("submitted job \"%s\"\n", jobid);
      } else {
         printf("unable to submit job\n");
      }
      
      if (((test_case == MT_EXIT_DURING_SUBMIT_OR_WAIT) || 
           (test_case == MT_EXIT_DURING_SUBMIT)) &&
          (drmaa_errno == DRMAA_ERRNO_NO_ACTIVE_SESSION)) {
         /* It's supposed to do that. */
         drmaa_errno = DRMAA_ERRNO_SUCCESS;
      }
      
      if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
         /* If there is ever an error, we will return an error. */
         error = drmaa_errno;
      }
   }

   return error;
}

static int wait_all_jobs(int n)
{
   char jobid[100];
   int drmaa_errno = DRMAA_ERRNO_SUCCESS;
   int stat;
   drmaa_attr_values_t *rusage = NULL;

   do {
      drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid)-1, &stat, DRMAA_TIMEOUT_WAIT_FOREVER, &rusage, NULL, 0);
      /* we don't use rusage here - free it! */
      drmaa_release_attr_values(rusage);
      rusage = NULL;

      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {
         printf("waited job \"%s\"\n", jobid);
         if (n != -1) {
            if (--n == 0) {
               printf("waited for last job\n");
               break;
            }
         }
      } else if (drmaa_errno != DRMAA_ERRNO_INVALID_JOB) {
         printf("drmaa_wait() returned %s\n", drmaa_strerror(drmaa_errno));
      }
   } while (drmaa_errno == DRMAA_ERRNO_SUCCESS);

   /* that means we got all */
   if (drmaa_errno == DRMAA_ERRNO_INVALID_JOB) {
      printf("no more jobs to wait\n");
      drmaa_errno = DRMAA_ERRNO_SUCCESS;
   }
   else if (((drmaa_errno == DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE) &&
             (test_case == MT_EXIT_DURING_SUBMIT_OR_WAIT)) ||
            ((drmaa_errno == DRMAA_ERRNO_NO_RUSAGE) &&
             (test_case == ST_SUBMIT_NO_RUN_WAIT))) {
      /* It's supposed to do that. */
      drmaa_errno = DRMAA_ERRNO_SUCCESS;
   }

   return drmaa_errno;
}

static int wait_n_jobs(int n)
{
   char jobid[100];
   int i, stat;
   int drmaa_errno = DRMAA_ERRNO_SUCCESS;
   int error = DRMAA_ERRNO_SUCCESS;
   bool done;

   for (i=0; i<n; i++) {
      done = false;
      while (!done) {
         drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid,
                                  sizeof(jobid)-1, &stat,
                                  DRMAA_TIMEOUT_WAIT_FOREVER, NULL, NULL, 0);
         
         if (drmaa_errno != DRMAA_ERRNO_SUCCESS) {
            printf("failed waiting for job (%s)\n", drmaa_strerror(drmaa_errno));
         }
         
         /* Only retry on "try again" error. */
         if (drmaa_errno == DRMAA_ERRNO_TRY_LATER) {
            printf("retry...\n");
            sleep(1);
         } else {
            done = true;
            break;
         }
      }

      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {
         printf("waited job \"%s\"\n", jobid);
      } else {
         /* If there is ever an error, we will return an error. */
         error = drmaa_errno;
      }
   }

   return error;
}

static void report_session_key(void) 
{
   if (is_sun_grid_engine) {
      const char *session_key = getenv("SGE_SESSION_KEY");
      if (session_key) 
         printf("got \"%s\" as session key\n", session_key);
      else
         printf("no session key set\n");
      
   }
}

#if 0
static init_signal_handling()
{
   struct sigaction nact;
   
   nact.sa_handler = SIG_IGN;

   sigaction(SIGPIPE, &act, NULL);
}
#endif


const struct drmaa_errno_descr_s {
  char *descr;
  int drmaa_errno;
} errno_vector[] = {
  { "DRMAA_ERRNO_SUCCESS",                      DRMAA_ERRNO_SUCCESS },
  { "DRMAA_ERRNO_INTERNAL_ERROR",               DRMAA_ERRNO_INTERNAL_ERROR },
  { "DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE",    DRMAA_ERRNO_DRM_COMMUNICATION_FAILURE },
  { "DRMAA_ERRNO_AUTH_FAILURE",                 DRMAA_ERRNO_AUTH_FAILURE },
  { "DRMAA_ERRNO_INVALID_ARGUMENT",             DRMAA_ERRNO_INVALID_ARGUMENT },
  { "DRMAA_ERRNO_NO_ACTIVE_SESSION",            DRMAA_ERRNO_NO_ACTIVE_SESSION },
  { "DRMAA_ERRNO_NO_MEMORY",                    DRMAA_ERRNO_NO_MEMORY },
  { "DRMAA_ERRNO_INVALID_CONTACT_STRING",       DRMAA_ERRNO_INVALID_CONTACT_STRING },
  { "DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR", DRMAA_ERRNO_DEFAULT_CONTACT_STRING_ERROR },
  { "DRMAA_ERRNO_DRMS_INIT_FAILED",             DRMAA_ERRNO_DRMS_INIT_FAILED },
  { "DRMAA_ERRNO_ALREADY_ACTIVE_SESSION",       DRMAA_ERRNO_ALREADY_ACTIVE_SESSION },
  { "DRMAA_ERRNO_DRMS_EXIT_ERROR",              DRMAA_ERRNO_DRMS_EXIT_ERROR },
  { "DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT",     DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT },
  { "DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE",      DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE },
  { "DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES", DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES },
  { "DRMAA_ERRNO_TRY_LATER",                    DRMAA_ERRNO_TRY_LATER },
  { "DRMAA_ERRNO_DENIED_BY_DRM",                DRMAA_ERRNO_DENIED_BY_DRM },
  { "DRMAA_ERRNO_INVALID_JOB",                  DRMAA_ERRNO_INVALID_JOB },
  { "DRMAA_ERRNO_RESUME_INCONSISTENT_STATE",    DRMAA_ERRNO_RESUME_INCONSISTENT_STATE },
  { "DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE",   DRMAA_ERRNO_SUSPEND_INCONSISTENT_STATE },
  { "DRMAA_ERRNO_HOLD_INCONSISTENT_STATE",      DRMAA_ERRNO_HOLD_INCONSISTENT_STATE },
  { "DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE",   DRMAA_ERRNO_RELEASE_INCONSISTENT_STATE },
  { "DRMAA_ERRNO_EXIT_TIMEOUT",                 DRMAA_ERRNO_EXIT_TIMEOUT },
  { "DRMAA_ERRNO_NO_RUSAGE",                    DRMAA_ERRNO_NO_RUSAGE },
  { NULL, 0 }
};


/****** test_drmaa/str2drmaa_errno() ********************************************
*  NAME
*     str2drmaa_errno() -- Map string into DRMAA errno constant 
*
*  SYNOPSIS
*     static int str2drmaa_errno(const char *str) 
*
*  FUNCTION
*     Map string into DRMAA errno constant.
*
*  INPUTS
*     const char *str - ??? 
*
*  RESULT
*     static int - DRMAA_ERRNO_* constant or -1 on failre
*******************************************************************************/
static int str2drmaa_errno(const char *str)
{
  int i;
  for (i=0; errno_vector[i].descr != NULL; i++)
      if (!strcmp(errno_vector[i].descr, str))
         return errno_vector[i].drmaa_errno;
  return -1;
}

/****** test_drmaa/drmaa_errno2str() *******************************************
*  NAME
*     drmaa_errno2str() -- Map DRMAA errno constant into string
*
*  SYNOPSIS
*     static const char* drmaa_errno2str(int drmaa_errno) 
*
*  FUNCTION
*     Map DRMAA errno constant into string
*
*  INPUTS
*     int drmaa_errno - Any DRMAA errno
*
*  RESULT
*     static const char* - String representation
*******************************************************************************/
static const char *drmaa_errno2str(int drmaa_errno)
{
  int i;
  for (i=0; errno_vector[i].descr != NULL; i++)
      if (errno_vector[i].drmaa_errno == drmaa_errno)
         return errno_vector[i].descr;
  return "DRMAA_ERRNO_???UNKNOWN???";
}

const struct ctrl_descr_s {
  char *descr;
  int ctrl;
} ctrl_vector[] = {
  { "DRMAA_CONTROL_SUSPEND",          DRMAA_CONTROL_SUSPEND },
  { "DRMAA_CONTROL_RESUME",           DRMAA_CONTROL_RESUME },
  { "DRMAA_CONTROL_HOLD",             DRMAA_CONTROL_HOLD },
  { "DRMAA_CONTROL_RELEASE",          DRMAA_CONTROL_RELEASE },
  { "DRMAA_CONTROL_TERMINATE",        DRMAA_CONTROL_TERMINATE },
  { NULL, 0 }
};

/****** test_drmaa/drmaa_ctrl2str() ********************************************
*  NAME
*     drmaa_ctrl2str() -- Map DRMAA control constant into string 
*
*  SYNOPSIS
*     static const char* drmaa_ctrl2str(int ctrl) 
*
*  FUNCTION
*     Map DRMAA control constant into string 
*
*  INPUTS
*     int ctrl - Any DRMAA_CONTROL_* value
*
*  RESULT
*     static const char* - DRMAA constant name or "unknown" string
*
*******************************************************************************/
static const char *drmaa_ctrl2str(int ctrl)
{
  int i;
  for (i=0; ctrl_vector[i].descr != NULL; i++)
      if (ctrl_vector[i].ctrl == ctrl)
         return ctrl_vector[i].descr;
  return "DRMAA_CONTROL_???UNKNOWN???";
}

/****** test_drmaa/str2drmaa_ctrl() ********************************************
*  NAME
*     str2drmaa_ctrl() -- Map string into DRMAA control constant
*
*  SYNOPSIS
*     static int str2drmaa_ctrl(const char *str) 
*
*  FUNCTION
*     Map string into DRMAA control constant.
*
*  INPUTS
*     const char *str - ??? 
*
*  RESULT
*     static int - DRMAA_CONTROL_* constant or -1 on failure
*******************************************************************************/
static int str2drmaa_ctrl(const char *str)
{
  int i;
  for (i=0; ctrl_vector[i].descr != NULL; i++) {
      if (!strcmp(ctrl_vector[i].descr, str))
         return ctrl_vector[i].ctrl;
  }
  return -1;
}

const struct state_descr_s {
  char *descr;
  int state;
} state_vector[] = {
  { "DRMAA_PS_UNDETERMINED",          DRMAA_PS_UNDETERMINED },
  { "DRMAA_PS_QUEUED_ACTIVE",         DRMAA_PS_QUEUED_ACTIVE },
  { "DRMAA_PS_SYSTEM_ON_HOLD",        DRMAA_PS_SYSTEM_ON_HOLD },
  { "DRMAA_PS_USER_ON_HOLD ",         DRMAA_PS_USER_ON_HOLD },
  { "DRMAA_PS_USER_SYSTEM_ON_HOLD",   DRMAA_PS_USER_SYSTEM_ON_HOLD },
  { "DRMAA_PS_RUNNING",               DRMAA_PS_RUNNING },
  { "DRMAA_PS_SYSTEM_SUSPENDED",      DRMAA_PS_SYSTEM_SUSPENDED },
  { "DRMAA_PS_USER_SUSPENDED",        DRMAA_PS_USER_SUSPENDED },
  { "DRMAA_PS_USER_SYSTEM_SUSPENDED", DRMAA_PS_USER_SYSTEM_SUSPENDED },
  { "DRMAA_PS_DONE",                  DRMAA_PS_DONE },
  { "DRMAA_PS_FAILED",                DRMAA_PS_FAILED },
  { NULL, 0 }
};

/****** test_drmaa/drmaa_state2str() *******************************************
*  NAME
*     drmaa_state2str() -- Map DRMAA state constant into string
*
*  SYNOPSIS
*     static const char* drmaa_state2str(int state) 
*
*  FUNCTION
*     Map DRMAA state constant into string
*
*  INPUTS
*     int state - Any DRMAA_PS_* value.
*
*  RESULT
*     static const char* - 
*******************************************************************************/
static const char *drmaa_state2str(int state)
{
  int i;
  for (i=0; state_vector[i].descr != NULL; i++)
      if (state_vector[i].state == state)
         return state_vector[i].descr;
  return "DRMAA_PS_???UNKNOWN???";
}

/****** test_drmaa/str2drmaa_state() *******************************************
*  NAME
*     str2drmaa_state() -- Map string into DRMAA state constant
*
*  SYNOPSIS
*     int str2drmaa_state(const char *str) 
*
*  FUNCTION
*     Map string into DRMAA state constant.
*
*  INPUTS
*     const char *str - 
*
*  RESULT
*     static int - 
*******************************************************************************/
int str2drmaa_state(const char *str)
{
  int i;
  for (i=0; state_vector[i].descr != NULL; i++)
      if (!strcmp(state_vector[i].descr, str))
         return state_vector[i].state;
  return -1;
}


/****** test_drmaa/report_wrong_job_finish() ***********************************
*  NAME
*     report_wrong_job_finish() -- Report how job finished
*
*  SYNOPSIS
*     static void report_wrong_job_finish(const char *comment, const char 
*     *jobid, int stat) 
*
*  FUNCTION
*     Report how job finished based on the stat value returned by drmaa_wait().
*     The information is printed to stderr.
*
*  INPUTS
*     const char *comment - provided by the caller
*     const char *jobid   - provided by the caller
*     int stat            - stat value as returned by drmaa_wait()
*******************************************************************************/
static void report_wrong_job_finish(const char *comment, const char *jobid, int stat)
{
   int aborted, exited, exit_status, signaled;

   drmaa_wifaborted(&aborted, stat, NULL, 0);
   if (aborted)
      fprintf(stderr, "%s: job \"%s\" never ran\n", comment, jobid);
   else {
      drmaa_wifexited(&exited, stat, NULL, 0);
      if (exited) {
         drmaa_wexitstatus(&exit_status, stat, NULL, 0);
         fprintf(stderr, "%s: job \"%s\" finished regularly with exit status %d\n",
               comment, jobid, exit_status);
      } else {
         drmaa_wifsignaled(&signaled, stat, NULL, 0);
         if (signaled) {
            char termsig[DRMAA_SIGNAL_BUFFER+1];
            drmaa_wtermsig(termsig, DRMAA_SIGNAL_BUFFER, stat, NULL, 0);
            fprintf(stderr, "%s: job \"%s\" finished due to signal %s\n",
               comment, jobid, termsig);
         } else
            fprintf(stderr, "%s: job \"%s\" finished with unclear conditions\n",
               comment, jobid);
      }
   }
}

static bool extract_array_command(char *command_line, int *start, int *end, int *incr) 
{
   bool ret = false;
   char *t_option = NULL;
   char *start_value = NULL;
   char *end_value = NULL;
   char *incr_value = NULL;
   char *end_t_option = NULL;

   *start = 1;
   *end = 1;
   *incr = 1;

   t_option = strstr(command_line, "-t");
  
   if (t_option != NULL) {
      ret = true;
      start_value = t_option + 3;

      *start = atoi(start_value);

      if (*start <= 0) {
         goto error;
      }

      end_t_option = strstr(start_value, " ");
      end_value = strstr(start_value, "-");
      incr_value = strstr(start_value, ":");

      if ((end_value != NULL) && (end_value < end_t_option)) {
         *end = atoi(end_value+1);

         if (*end <= 0) {
            goto error;
         }
     
         if ((incr_value != NULL) && (incr_value < end_t_option)) {
            *incr = atoi(incr_value+1);

            if (*incr <= 0) {
               *incr = 1;
            }
         }
     
      }
      else {
         goto error;
      }

      if (end_t_option != NULL) {
         strcpy(t_option, end_t_option+1);
      }
      else {
         t_option = '\0';
      }
   }/* end if */
   
   return ret;

error:
   if (end_t_option != NULL) {
      strcpy(t_option, end_t_option);
   }
   else {
      t_option = '\0';
   }   
   *start = 1;
   *end = 1;
   *incr = 1;
   ret = false;
   fprintf(stderr, "could not parse \"%s\" for -t option\n", command_line);

   if (end_t_option != NULL) {
      strcpy(t_option, end_t_option);
   }
   else {
      t_option = '\0';
   }   
   
   return ret;
}

static void free_order(int **order)
{
   int i = 0;
   while (order[i] != NULL) {
      sge_free(&(order[i]));
      i++;
   }
   sge_free(&order);
}

static int test_dispatch_order_njobs(int njobs, test_job_t job[], char *jsr_str)
{
   char diagnosis[DRMAA_ERROR_STRING_BUFFER];
   const char *all_jobids[10];
   char jobid[100];
   drmaa_job_template_t *jt;
   int drmaa_errno, i, pos = 0;
   int stat;
   int **order = job_run_sequence_parse(jsr_str);
   int nwait = njobs;

   if (order == NULL) {
      fprintf(stderr, "failed parsing job run sequence string\n");
      return -1;
   }

   init_jobids(all_jobids, njobs);

   /* submit jobs in hold */
   for (i = 0; i < njobs; i++) {
      int start = 0;
      int end = 0;
      int incr = 1;
      bool bulk_job = false;

      bulk_job = extract_array_command(job[i].native, &start, &end, &incr);

      jt = create_sleeper_job_template(job[i].time, 0, 1);
      drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, job[i].native, NULL, 0);
      if (bulk_job) {
         int counter = 0;
         drmaa_job_ids_t *bulkJobId;
         if (drmaa_run_bulk_jobs(&bulkJobId, jt, start, end, incr, 
                                 diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            free_order(order);
            drmaa_delete_job_template(jt, NULL, 0);
            return -1;
         }
         
         while (drmaa_get_next_job_id(bulkJobId, jobid, sizeof(jobid)-1) == DRMAA_ERRNO_SUCCESS) {
            all_jobids[pos++] = strdup(jobid);
            printf("submitted job \"%s\"\n", jobid);
            counter++;
         }
          array_job_run_sequence_adapt(order,i,counter);
          drmaa_release_job_ids(bulkJobId);
      } else {
         if (drmaa_run_job(jobid, sizeof(jobid)-1, jt, diagnosis, sizeof(diagnosis)-1) != DRMAA_ERRNO_SUCCESS) {
            fprintf(stderr, "drmaa_run_job() failed: %s\n", diagnosis);
            free_order(order);
            free_jobids(all_jobids, njobs);
            drmaa_delete_job_template(jt, NULL, 0);
            return -1;
         }
      
         printf("submitted job \"%s\"\n", jobid);
         all_jobids[pos++] = strdup(jobid);
      }
      drmaa_delete_job_template(jt, NULL, 0);
   }
   all_jobids[pos] = NULL;

   nwait = pos;
   njobs = pos;

   /* release all three jobs in one operation to ensure they get runnable at once for scheduler */
   if (drmaa_control(DRMAA_JOB_IDS_SESSION_ALL, DRMAA_CONTROL_RELEASE, diagnosis, sizeof(diagnosis)-1)!=DRMAA_ERRNO_SUCCESS) {
      fprintf(stderr, "drmaa_control(DRMAA_JOB_IDS_SESSION_ALL, DRMAA_CONTROL_RELEASE) failed: %s\n", diagnosis);
      free_order(order);
      free_jobids(all_jobids, njobs);
      return -1;
   }

   do {
      drmaa_errno = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid)-1, &stat, 
                     DRMAA_TIMEOUT_WAIT_FOREVER, NULL, NULL, 0);
      if (drmaa_errno == DRMAA_ERRNO_SUCCESS) {

         printf("waited job \"%s\"\n", jobid);

         /* map jobid to job index */
         pos = -1;
         for (i = 0; i < njobs; i++) {
            if (all_jobids[i] != NULL && strcmp(jobid, all_jobids[i]) == 0) {
               pos = i;
               break;
            }
         }
         if (pos == -1) {
            fprintf(stderr, "drmaa_wait() returned unexpected job: %s\n", jobid);
            free_jobids(all_jobids, njobs);
            free_order(order);
            return -1;
         }


         if (job_run_sequence_verify(pos, all_jobids, order)) {
            free_jobids(all_jobids, njobs);
            free_order(order);
            return -1;
         }

         /* NULL-ify finished ones */
         sge_free(&(all_jobids[pos]));

         if (--nwait == 0) {
            printf("waited for last job\n");
            break;
         }
      } else if (drmaa_errno != DRMAA_ERRNO_INVALID_JOB) {
         printf("drmaa_wait() returned %s\n", drmaa_strerror(drmaa_errno));
      }
   } while (drmaa_errno == DRMAA_ERRNO_SUCCESS);

   free_jobids(all_jobids, njobs);
   free_order(order);
   return 0;
}

static int job_run_sequence_verify(int pos, const char *all_jobids[], int *order[])
{
   int test_index, i, j;
   int found_group = 0;
   int *group;

   /* search the group this job belongs to */
   for (i=0; order[i]; i++) {
      group = order[i];
      for (j=0; group[j] != -1; j++) {
         if (group[j] == pos) {
            found_group = 1;
            break;
         }
      }
      if (found_group)
         break;
   }
   if (!found_group) {
      fprintf(stderr, "test broken: could not find job index %d in finish order scheme\n", pos);
      return -1;
   }

   /* complain about previous group job that did not finish earlier */
   while (i>0) {
      i--;
      for (j=0; order[i][j] != -1; j++) {
         test_index = order[i][j];
         if (all_jobids[test_index] != NULL) {
            fprintf(stderr, "order broken: job \"%s\" [%d] did not finish before job \"%s\" [%d]\n",
                  all_jobids[test_index], test_index, all_jobids[pos], pos);
            return -1;
         }
      }
   }

   return 0;
}


/****** test_drmaa/job_run_sequence_parse() ************************************
*  NAME
*     job_run_sequence_parse() -- ??? 
*
*  SYNOPSIS
*     static int** job_run_sequence_parse(char *jrs_str) 
*
*  FUNCTION
*     Parse job run sequence strings into order data structures. 
*
*  INPUTS
*     char *jrs_str - ??? 
*
*  RESULT
*     static int** - 
*
*  EXAMPLE
*     For exmples the strings "0-1-2", "0,2-1" and "0,1-2-3" are parsed 
*     into data structures like the following ones:
*   
*        int rr0[] = { 0, -1 };
*        int rr1[] = { 1, -1 };
*        int rr2[] = { 2, -1 };
*        int *rr_order[] = { rr0, rr1, rr2, NULL };
*
*        int bf0[] = { 0, 2, -1 };
*        int bf1[] = { 1, -1 };
*        int *bf_order[] = { bf0, bf1, NULL };
*
*        int st0[] = { 0, 1, -1 };
*        int st1[] = { 2, -1 };
*        int st2[] = { 3, -1 };
*        int *st_order[] = { st0, st1, st2, NULL };
*******************************************************************************/
#define GROUP_CHUNK 5
#define NUMBER_CHUNK 10
static int **job_run_sequence_parse(char *jrs_str)
{
   char *s = NULL, *group_str = NULL;

   /* control outer loop */
   char *jrs_str_cp = strdup(jrs_str);
   char  *iter_dash = NULL;
   int **sequence = NULL;
   int groups_total = GROUP_CHUNK;
   int groups_used = 0;
   int i = 0;


   printf("parsing sequence: \"%s\"\n", jrs_str_cp);

   sequence = malloc(sizeof(int *)*(GROUP_CHUNK+1));

   /* groups are delimited by dashes '-' */
   for (group_str=strtok_r(jrs_str_cp, "-", &iter_dash); group_str; group_str=strtok_r(NULL, "-", &iter_dash)) {
      char  *iter_comma = NULL;
      int *group;
      int numbers_total;
      int numbers_used;
      int j = 0;
   
      if (++groups_used > groups_total) {
         groups_total += GROUP_CHUNK;
         sequence = sge_realloc(sequence, groups_total + 1, 1);
      }

      numbers_total = NUMBER_CHUNK;
      numbers_used = 0;

      group = malloc(sizeof(int *)*(NUMBER_CHUNK+1));

      /* sequence numbers within a group are delimited by comma ',' */
      for (s=strtok_r(group_str, ",", &iter_comma); s; s=strtok_r(NULL, ",", &iter_comma)) {
         if (++numbers_used > numbers_total) {
            numbers_total += NUMBER_CHUNK;
            group = sge_realloc(group, numbers_total + 1, 1);
         }
         printf("%s ", s);
         group[j] = atoi(s);
         j++;
      }
      printf("\n");
      group[j] = -1;

      sequence[i] = group;
      i++;
   }
   sequence[i] = NULL;
   
   sge_free(&jrs_str_cp);
   return sequence;
}

static void array_job_run_sequence_adapt(int **sequence, int job_id, int count) 
{
   int x = 0;
   int y = 0;

   if (count <= 1) {
      return;
   }

   printf("modify finish order:\n");

   while (sequence[x] != NULL) { 
      y = 0;
      while (sequence[x][y] != -1) {
         
         if (sequence[x][y] == job_id) {
            int dy = 0;
           
            printf("%d ", sequence[x][y]);
           
            while (sequence[x][y] != -1) {
               y++;
            }
            
            for (; dy < (count-1); dy++) {
               sequence[x][y+dy] = job_id + dy + 1; 
               sequence[x][y+dy+1] = -1;
               printf("[%d] ", sequence[x][y+dy]);
            }
            
            y += dy - 1; 
         }
         else if (sequence[x][y] > job_id) {
            sequence[x][y] += (count-1);
            printf("(%d) ", sequence[x][y]);

         }
         else {
            printf("%d ", sequence[x][y]);

         }
         y++;
      }
      printf("\n");
      x++;
   }
}

static int set_path_attribute_plus_colon(drmaa_job_template_t *jt,
                                         const char *name, const char *value,
                                         char *error_diagnosis,
                                         size_t error_diag_len)
{
   char path_buffer[10000];
   strcpy(path_buffer, ":"); 
   strcat(path_buffer, value);
   return drmaa_set_attribute(jt, name, path_buffer, error_diagnosis,
                              error_diag_len);         
}

static bool test_error_code(char *name, int code, int expected)
{
   if (code != expected) {
      fprintf(stderr, "%s = %d; should be %d\n", name, code, expected);
      return false;
   }
   else {
      return true;
   }
}
