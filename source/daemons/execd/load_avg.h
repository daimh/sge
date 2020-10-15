#ifndef __LOAD_AVG_H
#define __LOAD_AVG_H
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

#include "sge_report_execd.h"
#include "sgeobj/sge_binding.h"

extern report_source execd_report_sources[];
extern lUlong sge_execd_report_seqno;

lList *sge_build_load_report(const char* qualified_hostname, const char* binary_path);

void update_job_usage(const char *qualified_hostname);

void execd_merge_load_report(u_long32 seqno);

void execd_trash_load_report(void);

bool sge_get_flush_lr_flag(void);
void sge_set_flush_lr_flag(bool new_val);

bool sge_get_delay_job_reports_flag(void);
void sge_set_delay_job_reports_flag(bool new_val);

u_long32 sge_get_qmrestart_time(void);
void sge_set_qmrestart_time(u_long32 qmr);

void build_reserved_usage(const u_long32 now, const lListElem *ja_task, const lListElem *pe_task,
                          double *wallclock, double *cpu, double *mem, double *maxvmem);

#endif /* __LOAD_AVG_H */

