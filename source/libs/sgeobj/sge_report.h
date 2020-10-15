#ifndef __SGE_REPORT_H
#define __SGE_REPORT_H
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

#include "cull/cull.h"

#include "sgeobj/sge_report_REP_L.h"
#include "sgeobj/sge_report_JR_L.h"
#include "sgeobj/sge_report_LIC_L.h"
#include "sgeobj/sge_report_LR_L.h"

/* 
 * ** valid values for REP_type 
 * */

/* REP_list is LR_Type */
#define NUM_REP_REPORT_LOAD    1

/* REP_list is ET_Type */
#define NUM_REP_REPORT_EVENTS  2

/* REP_list is CONF_Type */
#define NUM_REP_REPORT_CONF    3

/* REP_list is LIC_Type */
#define NUM_REP_REPORT_PROCESSORS 4

/* REP_list is JR_Type */
#define NUM_REP_REPORT_JOB     5

/* REP_list is LR_Type */
#define NUM_REP_FULL_REPORT_LOAD  6 

#define SGE_WEXITED_BIT      0x00000001
#define SGE_WSIGNALED_BIT    0x00000002
#define SGE_WCOREDUMP_BIT    0x00000004
#define SGE_NEVERRAN_BIT     0x00000008
/* POSIX exit status has only 8 bit */
#define SGE_EXIT_STATUS_BITS 0x00000FF0
/* SGE signal numbers are high numbers so we use 16 bit */
#define SGE_SIGNAL_BITS      0x0FFFF000

/* these macros shall be used for read access on JR_wait_status */
#define SGE_GET_WEXITED(status)   ((status)&SGE_WEXITED_BIT)
#define SGE_GET_WSIGNALED(status) ((status)&SGE_WSIGNALED_BIT)
#define SGE_GET_WCOREDUMP(status)   ((status)&SGE_WCOREDUMP_BIT)
#define SGE_GET_NEVERRAN(status)    ((status)&SGE_NEVERRAN_BIT)
#define SGE_GET_WEXITSTATUS(status) (((status)&SGE_EXIT_STATUS_BITS)>>4)
#define SGE_GET_WSIGNAL(status)     (((status)&SGE_SIGNAL_BITS)>>12)

/* these macros shall be used for write access on JR_wait_status */

#define SGE_SET_WEXITED(status, flag) \
   ((status) & ~SGE_WEXITED_BIT)   | ((flag)?SGE_WEXITED_BIT:0)
#define SGE_SET_WSIGNALED(status, flag) \
   ((status) & ~SGE_WSIGNALED_BIT) | ((flag)?SGE_WSIGNALED_BIT:0)
#define SGE_SET_WCOREDUMP(status, flag) \
   ((status) & ~SGE_WCOREDUMP_BIT) | ((flag)?SGE_WCOREDUMP_BIT:0)
#define SGE_SET_NEVERRAN(status, flag) \
   ((status) & ~SGE_NEVERRAN_BIT)  | ((flag)?SGE_NEVERRAN_BIT:0)
#define SGE_SET_WEXITSTATUS(status, exit_status) \
   ((status) & ~SGE_EXIT_STATUS_BITS)  |(((exit_status)<<4) & SGE_EXIT_STATUS_BITS)
#define SGE_SET_WSIGNAL(status, signal) \
   ((status) & ~SGE_SIGNAL_BITS)       |(((signal)<<12) & SGE_SIGNAL_BITS)


void job_report_print_usage(const lListElem *jr, FILE *fp);
void job_report_init_from_job(lListElem *jr, const lListElem *jep, 
                              const lListElem *jatep, const lListElem *petep);
void job_report_init_from_job_with_usage(lListElem *job_report,
                                         lListElem *job,
                                         lListElem *ja_task,
                                         lListElem *pe_task,
                                         u_long32 time_stamp);

#endif /* __SGE_REPORT_H */

