#ifndef __SGE_PACKET_PB_CULL_H
#define __SGE_PACKET_PB_CULL_H
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
#include "gdi/sge_gdi_packet_type.h"

#ifdef  __cplusplus
extern "C" {
#endif

u_long32
sge_gdi_packet_get_pb_size(sge_gdi_packet_class_t *packet);

bool
sge_gdi_packet_unpack(sge_gdi_packet_class_t **packet, lList **answer_list, sge_pack_buffer *pb);

bool
sge_gdi_packet_pack(sge_gdi_packet_class_t *packet, lList **answer_list, sge_pack_buffer *pb);

bool
sge_gdi_packet_pack_task(sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task,
                         lList **answer_list, sge_pack_buffer *pb);

#ifdef  __cplusplus
}
#endif

#endif 



