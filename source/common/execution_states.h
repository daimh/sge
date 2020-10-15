#ifndef __EXECUTION_STATES_H
#define __EXECUTION_STATES_H
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

#include <sys/types.h>



/*
** when we dont know more than that
*/
#define SSTATE_FAILURE_BEFORE_JOB   1

/* these error conditions are recovered by the execd
 * they are shepherd errors too and the numbers must be distinct from
 * SSTATE_*
 */
#define ESSTATE_NO_SHEPHERD         2  /* not used at the moment */
#define ESSTATE_NO_CONFIG           3
#define ESSTATE_NO_PID              4
/* these states are returned by the shepherd and written to the 
 * exit_status file
 * jobs with failure <= SSTATE_BEFORE_JOB are rescheduled
 * but see src/job_exit.c for actual implementation
 */
#define SSTATE_READ_CONFIG          5
#define SSTATE_PROCSET_NOTSET       6
#define SSTATE_BEFORE_PROLOG        7
#define SSTATE_PROLOG_FAILED        8
#define SSTATE_BEFORE_PESTART       9
#define SSTATE_PESTART_FAILED      10
#define SSTATE_BEFORE_JOB          11 
#define SSTATE_BEFORE_PESTOP       12
#define SSTATE_PESTOP_FAILED       13
#define SSTATE_BEFORE_EPILOG       14
#define SSTATE_EPILOG_FAILED       15
#define SSTATE_PROCSET_NOTFREED    16

#define ESSTATE_DIED_THRU_SIGNAL   17
#define ESSTATE_SHEPHERD_EXIT      18
#define ESSTATE_NO_EXITSTATUS      19
#define ESSTATE_UNEXP_ERRORFILE    20
#define ESSTATE_UNKNOWN_JOB        21

/*
 * these error conditions can be met
 * by the qmaster
 */
#define ESSTATE_EXECD_LOST_RUNNING 22

/* this is an error that occurs only in a SGE execd */
#define ESSTATE_PTF_CANT_GET_PIDS  23

#define SSTATE_MIGRATE             24
#define SSTATE_AGAIN               25

#define SSTATE_OPEN_OUTPUT         26
#define SSTATE_NO_SHELL            27
#define SSTATE_NO_CWD              28
#define SSTATE_AFS_PROBLEM         29
#define SSTATE_APPERROR            30
#define SSTATE_PASSWD_FILE_ERROR   31
#define SSTATE_PASSWD_MISSING      32
#define SSTATE_PASSWD_WRONG        33
#define SSTATE_HELPER_SERVICE_ERROR 34
#define SSTATE_HELPER_SERVICE_BEFORE_JOB 35
#define SSTATE_CHECK_DAEMON_CONFIG 36
#define SSTATE_QMASTER_ENFORCED_LIMIT 37
#define SSTATE_ADD_GRP_SET_ERROR   38

#define MAX_SSTATE SSTATE_ADD_GRP_SET_ERROR

#define SSTATE_FAILURE_AFTER_JOB  100

/*
 * we differentiate between several general failure states
 * the queue, all queues on that host or all queues in all
 * might have to be halted
 */
#define GFSTATE_NO_HALT           0
#define GFSTATE_QUEUE             1
#define GFSTATE_HOST              2
#define GFSTATE_SYSTEM            3
#define GFSTATE_JOB               4
#define GFSTATE_QUEUE_NO_RERUN    5 /* queue in error but don't re-run job */

char *get_sstate_description(int sstate);

extern int shepherd_state;
extern pid_t coshepherd_pid;

#endif /* __EXECUTION_STATES_H */



