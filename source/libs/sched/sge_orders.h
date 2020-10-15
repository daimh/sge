#ifndef _SGE_ORDERS_H_
#define _SGE_ORDERS_H_
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

#include "cull.h"
#include "evc/sge_event_client.h"

typedef struct {
   lList *configOrderList;   /* Type: ORT_unsuspend_on_threshold, ORT_suspend_on_threshold */
   lList *pendingOrderList;  /* Type: ORT_tickets, ORT_ptickets, ORT_remove_job, ORT_update_user_usage, 
                                      ORT_update_project_usage, ORT_share_tree, ORT_sched_conf */
   lList *jobStartOrderList; /* Type: ORT_remove_immediate_job, job start orders, job info orders */
   lList *sentOrderList;     /* already send job start orders, need to get a correct order 
                                amount for the profiling. It is also needed for a warring
                                message, which informs about policy conflict:
                                MSG_SUBORDPOLICYCONFLICT_UUSS */
   u_long32 numberSendOrders; /* number of send orders */
   u_long32 numberSendPackages; /* number sends inbetween */
}order_t;

#define ORDER_INIT {NULL,NULL,NULL, NULL, 0, 0}


lList *sge_add_schedd_info(lList *or_list, int *global_mes_count, int *job_mes_count);

lList *sge_create_orders(lList *or_list, u_long32 type, lListElem *job, lListElem *ja_task, 
                         lList *queue_list, bool update_execd);

lList *create_delete_job_orders(lList *finished_jobs, lList *order_list);

lList *sge_join_orders(order_t *orders);
int sge_GetNumberOfOrders(order_t *orders); 

int sge_send_orders2master(sge_evc_class_t *evc, lList **orders);

#if 0
int sge_send_job_start_orders(sge_evc_class_t *evc, order_t *orders);
#endif

#endif /* _SGE_ORDERS_H_ */



