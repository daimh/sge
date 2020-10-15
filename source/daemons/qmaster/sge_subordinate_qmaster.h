#ifndef __SUBORDINATE_QMASTER_H
#define __SUBORDINATE_QMASTER_H
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
#include "gdi/sge_gdi_ctx.h"

bool
cqueue_list_x_on_subordinate_gdil(sge_gdi_ctx_class_t *ctx,
                                  lList *this_list, bool suspend,
                                  const lList *gdil, monitoring_t *monitor);

bool
cqueue_list_x_on_subordinate_so(sge_gdi_ctx_class_t *ctx, 
                                lList *this_list, lList **answer_list,
                                bool suspend, const lList *resolved_so_list,
                                monitoring_t *monitor);

void
qinstance_find_suspended_subordinates(const lListElem *this_elem,
                                      lList **answer_list,
                                      lList **resolved_so_list);

bool
qinstance_initialize_sos_attr(sge_gdi_ctx_class_t *ctx, lListElem *this_elem, monitoring_t *monitor);

bool
do_slotwise_x_on_subordinate_check(sge_gdi_ctx_class_t *ctx, lListElem *queue_instance,
      bool suspend, bool check_subtree_only, monitoring_t *monitor);

void
unsuspend_all_tasks_in_slotwise_sub_tree(sge_gdi_ctx_class_t *ctx,
      lListElem *qinstance, monitoring_t *monitor);

bool
check_new_slotwise_subordinate_tree(lListElem *qinstance,
      lList *new_so_list, lList **answer_list);

bool do_slotwise_subordinate_lists_differ(const lList *old_so_list,
      const lList *new_so_list);

#endif /* __SUBORDINATE_QMASTER_H */

