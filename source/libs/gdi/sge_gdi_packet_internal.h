#ifndef __SGE_PACKET_INTERNAL_H
#define __SGE_PACKET_INTERNAL_H
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

#include "basis_types.h"
#include "uti/sge_tq.h"

#ifdef  __cplusplus
extern "C" {
#endif

extern sge_tq_queue_t *Master_Task_Queue;

void
sge_gdi_packet_wait_till_handled(sge_gdi_packet_class_t *packet);

void
sge_gdi_packet_broadcast_that_handled(sge_gdi_packet_class_t *packet);

bool
sge_gdi_packet_is_handled(sge_gdi_packet_class_t *packet);

bool
sge_gdi_packet_execute_external(sge_gdi_ctx_class_t* ctx, lList **answer_list,
                                sge_gdi_packet_class_t *packet);

bool
sge_gdi_packet_execute_internal(sge_gdi_ctx_class_t* ctx, lList **answer_list,
                                sge_gdi_packet_class_t *packet);

bool 
sge_gdi_packet_wait_for_result_external(sge_gdi_ctx_class_t* ctx, lList **answer_list,
                                        sge_gdi_packet_class_t **packet_handle, lList **malpp);

bool 
sge_gdi_packet_wait_for_result_internal(sge_gdi_ctx_class_t* ctx, lList **answer_list,
                                        sge_gdi_packet_class_t **packet_handle, lList **malpp);

#ifdef  __cplusplus
}
#endif

#endif 



