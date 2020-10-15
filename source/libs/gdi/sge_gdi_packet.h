#ifndef __SGE_PACKET_H
#define __SGE_PACKET_H
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

#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define GDI_PACKET_MUTEX "gdi_pack_mutex"

sge_gdi_packet_class_t *
sge_gdi_packet_create_base(lList **answer_list);

sge_gdi_packet_class_t *
sge_gdi_packet_create(sge_gdi_ctx_class_t *ctx, lList **answer_list, bool use_euid_egid);

bool
sge_gdi_packet_free(sge_gdi_packet_class_t **packet_handle);

bool 
sge_gdi_packet_append_task(sge_gdi_packet_class_t *packet, lList **answer_list,
                           u_long32 target, u_long32 command, lList **lp, lList **a_list,
                           lCondition **condition, lEnumeration **enumeration,
                           bool do_copy);

u_long32 
sge_gdi_packet_get_last_task_id(sge_gdi_packet_class_t *packet);

bool 
sge_gdi_packet_verify_version(sge_gdi_packet_class_t *packet, lList **alpp);

const char *
sge_gdi_task_get_operation_name(sge_gdi_task_class_t *task);

#ifdef  __cplusplus
}
#endif

#endif 
