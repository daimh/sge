#ifndef _SGE_REPORTING_QMASTER_H_
#define _SGE_REPORTING_QMASTER_H_
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

#include "uti/sge_dstring.h"
#include "uti/sge_monitor.h"

#include "cull/cull.h"

#include "sgeobj/sge_object.h"
#include "sgeobj/sge_advance_reservation.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_qmaster_timed_event.h"


typedef enum {
   JL_UNKNOWN = 0,   /* job is in unknown state - should never be seen */
   JL_PENDING,       /* job is pending */
   JL_SENT,          /* job has been sent to execd */
   JL_RESENT,        /* job has been resent to execd - sent hasn't been ack */
   JL_DELIVERED,     /* job has been delivered - execd replied with ack */
   JL_RUNNING,       /* job is running (reported by execd) */
   JL_SUSPENDED,     /* job has been suspended */
   JL_UNSUSPENDED,   /* job has been unsuspended */
   JL_HELD,          /* a hold was applied */
   JL_RELEASED,      /* all holds were released */
   JL_RESTART,       /* a restart of the job was requested */
   JL_MIGRATE,       /* a migration was requested */
   JL_DELETED,       /* the job has been deleted */
   JL_FINISHED,      /* the job has finished */
   JL_ERROR,         /* job is in error state */

   JL_ALL
} job_log_t;

bool
reporting_initialize(lList **answer_list);

bool
reporting_shutdown(sge_gdi_ctx_class_t *ctx, lList **answer_list, bool do_spool);

void
reporting_trigger_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);

bool
reporting_create_new_job_record(lList **answer_list, const lListElem *job);

bool 
reporting_create_job_log(lList **answer_list,
                         u_long32 event_time,
                         const job_log_t,
                         const char *user,
                         const char *host,
                         const lListElem *job_report,
                         const lListElem *job, const lListElem *ja_task,
                         const lListElem *pe_task,
                         const char *message);

bool
reporting_create_acct_record(sge_gdi_ctx_class_t *ctx,
                             lList **answer_list, 
                             lListElem *job_report, 
                             lListElem *job, 
                             lListElem *ja_task, 
                             bool intermediate);

bool
reporting_create_host_record(lList **answer_list,
                             const lListElem *host,
                             u_long32 report_time);

bool
reporting_create_host_consumable_record(lList **answer_list,
                                        const lListElem *host,
                                        const lListElem *job,
                                        u_long32 report_time);

bool
reporting_create_queue_record(lList **answer_list,
                              const lListElem *queue,
                              u_long32 report_time);

bool
reporting_create_queue_consumable_record(lList **answer_list,
                                         const lListElem *host,
                                         const lListElem *queue,
                                         const lListElem *job,
                                         u_long32 report_time);

bool
reporting_is_intermediate_acct_required(const lListElem *job, 
                                        const lListElem *ja_task, 
                                        const lListElem *pe_task);

bool
reporting_create_new_ar_record(lList **answer_list, 
                               const lListElem *ar,
                               u_long32 report_time);

bool
reporting_create_ar_attribute_record(lList **answer_list,
                                     const lListElem *ar,
                                     u_long32 report_time);

bool
reporting_create_ar_log_record(lList **answer_list,
                               const lListElem *ar,
                               ar_state_event_t state,
                               const char *ar_description,
                               u_long32 report_time);

bool
reporting_create_ar_acct_records(lList **answer_list,
                                const lListElem *ar,
                                u_long32 report_time);

#endif /* _SGE_REPORTING_QMASTER_H_ */

