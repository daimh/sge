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

#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_sl.h"

#include "gdi/sge_gdi2.h"
#include "gdi/sge_gdi_ctx.h"

#include "sgeobj/sge_answer.h"

#include "sge_sched_order.h"

#include "msg_common.h"

gdi_request_queue_t Master_Request_Queue; 

bool
schedd_order_initialize(void) {
   bool ret = true;

   DENTER(TOP_LAYER, "schedd_order_initialize");
   Master_Request_Queue.order_list = NULL;
   ret &= sge_sl_create(&Master_Request_Queue.request_list);
   DRETURN(ret);
}

bool
schedd_order_destroy(void) {
   bool ret = true;

   DENTER(TOP_LAYER, "schedd_order_initialize");   
   ret &= sge_sl_destroy(&Master_Request_Queue.request_list, NULL);
   DRETURN(ret);
}


bool
sge_schedd_send_orders(sge_gdi_ctx_class_t *ctx, order_t *orders, lList **order_list, lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "sge_schedd_send_orders");

   if ((order_list != NULL) && (*order_list != NULL) && (lGetNumberOfElem(*order_list) != 0)) {
      /*
       * Add the new orders 
       */
      if (Master_Request_Queue.order_list == NULL) {
         Master_Request_Queue.order_list = *order_list;
         *order_list = NULL;
      } else {
         lAddList(Master_Request_Queue.order_list, order_list);
      }

      ret = sge_schedd_add_gdi_order_request(ctx, orders, answer_list, &Master_Request_Queue.order_list);
   }
   lFreeList(order_list);

   DRETURN(ret);
}

bool
sge_schedd_add_gdi_order_request(sge_gdi_ctx_class_t *ctx, order_t *orders, lList **answer_list, lList **order_list) 
{
   bool ret = true;
   state_gdi_multi *state = NULL;

   DENTER(TOP_LAYER, "sge_schedd_add_gdi_order_request");
   state = (state_gdi_multi *)sge_malloc(sizeof(state_gdi_multi));
   if (state != NULL) {
      int order_id;

      memset(state, 0, sizeof(state_gdi_multi));
      orders->numberSendOrders += lGetNumberOfElem(*order_list);
      orders->numberSendPackages++;

      /*
       * order_list will be NULL after the call of gdi_multi. This saves a copy operation.
       */
      order_id = ctx->gdi_multi(ctx, answer_list, SGE_GDI_SEND, SGE_ORDER_LIST, SGE_GDI_ADD,
                                order_list, NULL, NULL, false, state, false);

      if (order_id != -1) {
         sge_sl_insert(Master_Request_Queue.request_list, state, SGE_SL_BACKWARD);
         
         /* EB: TODO: Why is this needed? */
         state->next = NULL;
      } else {
         answer_list_log(answer_list, false, false);
         ret = false;
      }
   } else {
      answer_list_add(answer_list, MSG_SGETEXT_NOMEM, STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      ret = false;
   } 
   DRETURN(ret);
}

bool
sge_schedd_block_until_orders_processed(sge_gdi_ctx_class_t *ctx, 
                                        lList **answer_list)
{
   bool ret = true;
   sge_sl_elem_t *next_elem = NULL;
   sge_sl_elem_t *current_elem = NULL;

   DENTER(TOP_LAYER, "sge_schedd_block_until_orders_processed");

   /*
    * wait till all GDI order requests are finished
    */
   sge_sl_elem_next(Master_Request_Queue.request_list, &next_elem, SGE_SL_FORWARD); 
   while ((current_elem = next_elem) != NULL) {
      state_gdi_multi *current_state = sge_sl_elem_data(current_elem);
      lList *request_answer_list = NULL;
      lList *multi_answer_list = NULL;
      int order_id;

      /* get next element, dechain current and destroy it */
      sge_sl_elem_next(Master_Request_Queue.request_list, &next_elem, SGE_SL_FORWARD); 
      sge_sl_dechain(Master_Request_Queue.request_list, current_elem);
      sge_sl_elem_destroy(&current_elem, NULL);

      /* 
       * wait for answer. this call might block if the request
       * has not been handled by any worker until now
       */
      ctx->gdi_wait(ctx, answer_list, &multi_answer_list, current_state);

      /*
       * now we have an answer. is it positive? 
       */
      order_id = 1;
      sge_gdi_extract_answer(&request_answer_list, SGE_GDI_ADD, SGE_ORDER_LIST,
                             order_id, multi_answer_list, NULL);
      if (request_answer_list != NULL) {
         answer_list_log(&request_answer_list, false, false);
         ret = false;
      }

      /*
       * memory cleanup
       */
      lFreeList(&request_answer_list);
      lFreeList(&multi_answer_list);
      sge_free(&current_state);
   }
   DRETURN(ret);
}
