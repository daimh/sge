#ifndef __REAPER_EXECD_H
#define __REAPER_EXECD_H
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

#include "gdi/sge_gdi_ctx.h"

int sge_reap_children_execd(int max_count, bool is_qmaster_down);
lListElem *execd_job_start_failure(lListElem *jep, lListElem *jatep, lListElem *petep, const char *error_string, int general);
lListElem *execd_job_run_failure(lListElem *jep, lListElem *jatep, lListElem *petep, const char *error_string, int general);
void job_unknown(u_long32 jobid, u_long32 jataskid, char *qname);
int clean_up_old_jobs(sge_gdi_ctx_class_t *ctx, int startup);
void remove_acked_job_exit(sge_gdi_ctx_class_t *ctx, u_long32 job_id, u_long32 ja_task_id, const char *pe_task_id, lListElem *jr); 

void execd_slave_job_exit(u_long32 job_id, u_long32 ja_task_id);
int count_master_tasks(const lList *lp, u_long32 job_id);
#endif
