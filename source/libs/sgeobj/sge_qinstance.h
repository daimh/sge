#ifndef __SGE_QINSTANCE_H
#define __SGE_QINSTANCE_H

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
 *   Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_dstring.h"
#include "sgeobj/sge_qinstance_QU_L.h"

/* 
 * Q types values for QU_qtype 
 */
enum {
   BQ = 0x01,                /* batch Q */
   IQ = 0x02                 /* interactive Q */
};

enum {
   GDI_DO_LATER = 0x01
};

/* sequential scheduling uses: QU_available_at 
 * parallel scheduling uses: QU_tag_qend */
#define QU_tag_qend QU_available_at

bool
qinstance_validate(lListElem *this_elem, lList **answer_list, lList *master_exechost_list);

bool
qinstance_list_validate(lList *this_list, lList **answer_list, lList *master_exechost_list);

void
qinstance_set_full_name(lListElem *this_elem);

lListElem *
qinstance_list_locate(const lList *this_list, const char *hostname,
                      const char *cqueue_name);

lListElem *
qinstance_list_locate2(const lList *qinstance_list, const char *full_name);

const char *
qinstance_get_name(const lListElem *this_elem, dstring *string_buffer);

void
qinstance_list_set_tag(lList *this_list, u_long32 tag_value);

void
qinstance_increase_qversion(lListElem *this_elem);

bool 
qinstance_check_owner(const lListElem *queue, const char *user_name);

bool
qinstance_is_pe_referenced(const lListElem *this_elem, 
                           const lListElem *pe);

bool
qinstance_is_a_pe_referenced(const lListElem *this_elem);

bool
qinstance_is_ckpt_referenced(const lListElem *this_elem, 
                             const lListElem *ckpt);

bool
qinstance_is_a_ckpt_referenced(const lListElem *this_elem);

bool
qinstance_is_centry_a_complex_value(const lListElem *this_elem,
                                    const lListElem *name);

void
qinstance_set_slots_used(lListElem *this_elem, int new_slots);

int
qinstance_slots_used(const lListElem *this_elem);

int
qinstance_slots_reserved(const lListElem *this_elem);

void
qinstance_set_conf_slots_used(lListElem *this_elem);

bool
qinstance_is_calendar_referenced(const lListElem *this_elem, 
                                 const lListElem *calendar);

int
qinstance_debit_consumable(lListElem *this_elem, lListElem *job, 
                           lList *centry_list, int slots, bool is_master_task, bool *just_check);

bool
qinstance_message_add(lListElem *this_elem, u_long32 type, const char *message);

bool
qinstance_message_trash_all_of_type_X(lListElem *this_elem, u_long32 type);

/* EB: TODO: queue -> qinstance */

int queue_reference_list_validate(lList **alpp, lList *qr_list,
                                  const char *attr_name, const char *obj_descr,
                                  const char *obj_name);

int
rc_debit_consumable(lListElem *jep, lListElem *ep, lList *centry_list, int slots,
                 int config_nm, int actual_nm, const char *obj_name, bool is_master_task, bool *just_check);

lListElem *
explicit_job_request(lListElem *jep, const char *name);

bool
qinstance_list_find_matching(const lList *this_list, lList **answer_list,
                             const char *hostname_pattern, lList **qref_list);

bool
qinstance_list_verify_execd_job(const lList *queue_list, lList **answer_list);

bool
qinstance_verify(const lListElem *qep, lList **answer_list);

bool
qinstance_verify_full_name(lList **answer_list, const char *full_name);

void
qinstance_set_error(lListElem *qinstance, u_long32 type, const char *message, bool set_error);
#endif /* __SGE_QINSTANCE_H */
