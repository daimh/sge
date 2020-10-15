#ifndef __SGE_ADVANCE_RESERVATION_QMASTER_H
#define __SGE_ADVANCE_RESERVATION_QMASTER_H
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

#include "uti/sge_monitor.h"

#include "sgeobj/sge_advance_reservation.h"

#include "gdi/sge_gdi_ctx.h"

#include "sge_qmaster_timed_event.h"
#include "sge_c_gdi.h"

/* funtions called from within gdi framework in qmaster */
int ar_mod(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *new_ar, lListElem *ar, 
           int add, const char *ruser, const char *rhost, gdi_object_t *object, 
           int sub_command, monitoring_t *monitor);

int ar_spool(sge_gdi_ctx_class_t *ctx, lList **alpp, lListElem *pep, gdi_object_t *object);

int ar_success(sge_gdi_ctx_class_t *ctx, lListElem *ep, lListElem *old_ep, 
               gdi_object_t *object, lList **ppList, monitoring_t *monitor);

/* funtions called via gdi and inside the qmaster */
int ar_del(sge_gdi_ctx_class_t *ctx, lListElem *ep, lList **alpp, lList **ar_list, 
           const char *ruser, const char *rhost, monitoring_t *monitor);

void sge_store_ar_id(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);

void sge_init_ar_id(void);

int ar_do_reservation(lListElem *ar, bool incslots);

void ar_initialize_reserved_queue_list(lListElem *ar);

void sge_ar_event_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, 
                          monitoring_t *monitor);

bool
ar_list_has_reservation_due_to_ckpt(lList *ar_master_list, lList **answer_list,
                                    const char *qinstance_name, lList *ckpt_string_list);

bool
ar_list_has_reservation_due_to_pe(lList *ar_master_list, lList **answer_list,
                                  const char *qinstance_name, lList *pe_string_list);

bool
ar_list_has_reservation_for_pe_with_slots(lList *ar_master_list, lList **answer_list,
                                          const char *pe_name, u_long32 new_slots);

bool
sge_ar_remove_all_jobs(sge_gdi_ctx_class_t *ctx, u_long32 ar_id, int forced, monitoring_t *monitor);

bool
sge_ar_list_conflicts_with_calendar(lList **answer_list, const char *qinstance_name, lListElem *cal_ep,
                                lList *master_ar_list);

void sge_ar_state_set_running(lListElem *ar);
void sge_ar_state_set_waiting(lListElem *ar);
void sge_ar_state_set_deleted(lListElem *ar);
void sge_ar_state_set_exited(lListElem *ar);

void sge_ar_list_set_error_state(lList *ar_list, const char *qname, u_long32 error_type, bool set_error);

bool 
ar_list_has_reservation_due_to_qinstance_complex_attr(lList *ar_master_list, 
                                                      lList **answer_list,
                                                      lListElem *qinstance, 
                                                      lList *ce_master_list);

bool 
ar_list_has_reservation_due_to_host_complex_attr(lList *ar_master_list,
                                                 lList **answer_list,
                                                 lListElem *host, 
                                                 lList *ce_master_list);

void
ar_initialize_timer(sge_gdi_ctx_class_t *ctx, lList **answer_list, monitoring_t *monitor);


#endif
