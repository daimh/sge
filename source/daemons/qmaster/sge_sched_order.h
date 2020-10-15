#ifndef _SGE_SCHED_ORDER_H_
#define _SGE_SCHED_ORDER_H_
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
 *  Copyright: 2007 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "uti/sge_sl.h"

#include "sched/sge_orders.h"

typedef struct {
   lList *order_list;
   sge_sl_list_t *request_list;
} gdi_request_queue_t;

extern gdi_request_queue_t Master_Request_Queue;

bool
schedd_order_initialize(void);

bool
schedd_order_destroy(void);

bool
sge_schedd_add_gdi_order_request(sge_gdi_ctx_class_t *ctx, order_t *orders, lList **answer_list, lList **order_list);

bool
sge_schedd_send_orders(sge_gdi_ctx_class_t *ctx, order_t *orders, lList **order_list, lList **answer_list, const char *name);

bool
sge_schedd_block_until_orders_processed(sge_gdi_ctx_class_t *ctx, lList **answer_list);

#endif
