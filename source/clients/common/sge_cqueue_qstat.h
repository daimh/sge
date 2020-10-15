#ifndef __SGE_CQUEUE_QSTAT
#define __SGE_CQUEUE_QSTAT

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

bool cqueue_calculate_summary(const lListElem *cqueue,
                                     const lList *exechost_list,
                                     const lList *centry_list,
                                     double *load,
                                     bool *is_load_available,
                                     u_long32 *used,
                                     u_long32 *resv,
                                     u_long32 *total,
                                     u_long32 *suspend_manual,
                                     u_long32 *suspend_threshold,
                                     u_long32 *suspend_on_subordinate,
                                     u_long32 *suspend_calendar,
                                     u_long32 *unknown,
                                     u_long32 *load_alarm,
                                     u_long32 *disabled_manual,
                                     u_long32 *disabled_claendr,
                                     u_long32 *ambiguous,
                                     u_long32 *orphaned,
                                     u_long32 *error,
                                     u_long32 *available,
                                     u_long32 *temp_disabled,
                                     u_long32 *manual_intervention);
 

int select_by_qref_list(lList *cqueue_list, const lList *hgrp_list, const lList *qref_list);
int select_by_pe_list(lList *queue_list, lList *peref_list, lList *pe_list);
int select_by_queue_user_list(lList *exechost_list, lList *queue_list, lList *queue_user_list, lList *acl_list, lList *project_list);
int select_by_queue_state(u_long32 queue_states, lList *exechost_list, lList *queue_list, lList *centry_list);
int select_by_resource_list(lList *resource_list, lList *exechost_list, lList *queue_list, lList *centry_list, u_long32 empty_qs);
bool is_cqueue_selected(lList *queue_list);

int
qinstance_slots_reserved_now(const lListElem *this_elem);

#endif /* __SGE_CQUEUE_QSTAT */
