#ifndef __SGE_JOB_ENFORCE_LIMIT_H
#define __SGE_JOB_ENFORCE_LIMIT_H
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

void 
sge_job_enfoce_limit_handler(sge_gdi_ctx_class_t *ctx, te_event_t event, monitoring_t *monitor);

void
sge_add_check_limit_trigger(void);

void 
sge_host_add_enforce_limit_trigger(const char *hostname);

void
sge_host_remove_enforce_limit_trigger(const char *hostname);

void 
sge_job_add_enforce_limit_trigger(lListElem *jep, lListElem *jatep);

void 
sge_job_remove_enforce_limit_trigger(u_long32 jid, u_long32 ja_task_id);

#endif /* __SGE_JOB_ENFORCE_LIMIT_H */

