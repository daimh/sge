#ifndef __SGE_GIVE_JOBS_H
#define __SGE_GIVE_JOBS_H
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

#include "sge_qmaster_timed_event.h"
#include "gdi/sge_gdi_ctx.h"

#define MAX_JOB_DELIVER_TIME (5*60)

typedef enum {
   COMMIT_DEFAULT          = 0x0000,
   COMMIT_NO_SPOOLING      = 0x0001,   /* don't spool the job */
   COMMIT_NO_EVENTS        = 0x0002,   /* don't create events */
   COMMIT_UNENROLLED_TASK  = 0x0004,   /* handle unenrolled pending tasks */ 
   COMMIT_NEVER_RAN        = 0x0008    /* job never ran */
} sge_commit_flags_t; 

/* sge_commit_job() state transitions */
typedef enum {
   COMMIT_ST_SENT = 0,               /* job was sent and is now transfering */
   COMMIT_ST_ARRIVED = 1,            /* job was reported as running by execd */
   COMMIT_ST_RESCHEDULED = 2,        /* job gets rescheduled */             
   COMMIT_ST_FINISHED_FAILED = 3,    /* GE job finished or failed (FINISH/ABORT) */
   COMMIT_ST_FINISHED_FAILED_EE = 4, /* GEEE job finished or failed (FINISH/ABORT) */
   COMMIT_ST_DEBITED_EE = 5,         /* remove after GEEE scheduler debited usage */
   COMMIT_ST_NO_RESOURCES = 6,       /* remove interacive job (FINISH/ABORT) */
   COMMIT_ST_DELIVERY_FAILED = 7,    /* delivery failed and rescheduled */
   COMMIT_ST_FAILED_AND_ERROR = 8,   /* job failed and error state set */
   COMMIT_ST_USER_RESCHEDULED = 9    /* job get rescheduled due to exit 99 or qmod -rj executed by job user */
} sge_commit_mode_t;

int sge_give_job(sge_gdi_ctx_class_t *ctx,
                 lListElem *jep, lListElem *jatep, lListElem *master_qep, 
                 lListElem *pep, lListElem *hep, monitoring_t *monitor);

void sge_commit_job(sge_gdi_ctx_class_t *ctx,
                    lListElem *jep, lListElem *jatep, lListElem *jr, sge_commit_mode_t mode, 
                    int commit_flags, monitoring_t *monitor);

bool gdil_del_all_orphaned(sge_gdi_ctx_class_t *ctx, const lList *gdil_list, lList **alpp);

void sge_zombie_job_cleanup_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);

void sge_job_resend_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);

void trigger_job_resend(u_long32 now, lListElem *hep, u_long32 jid, u_long32 tid, int delta);

void cancel_job_resend(u_long32 jid, u_long32 tid);

#endif /* __SGE_GIVE_JOBS_H */
