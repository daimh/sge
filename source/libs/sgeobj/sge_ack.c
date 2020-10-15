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

#include "uti/sge_rmon.h"

#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ack.h"

int pack_ack(sge_pack_buffer *pb, u_long32 type, u_long32 id, u_long32 id2, const char *str)
{
   int ret;
   lListElem *ack = lCreateElem(ACK_Type);

   DENTER(TOP_LAYER, "pack_ack");

   lSetUlong(ack, ACK_type, type);
   lSetUlong(ack, ACK_id, id);
   lSetUlong(ack, ACK_id2, id2);
   lSetString(ack, ACK_str, str);

   ret = cull_pack_elem(pb, ack);
   lFreeElem(&ack);

   DRETURN(ret);
}

int sge_send_ack_to_qmaster(sge_gdi_ctx_class_t *ctx, u_long32 type, u_long32 ulong_val, 
                            u_long32 ulong_val_2, const char *str, lList **alpp)
{
   int ret;
   sge_pack_buffer pb;
   const char* commproc = prognames[QMASTER];
   const char* rhost = ctx->get_master(ctx, false);
   int         id   = 1;
   
   DENTER(TOP_LAYER, "sge_gdi2_send_ack_to_qmaster");

   /* send an ack to the qmaster for the events */
   if (init_packbuffer(&pb, 1024, 0) != PACK_SUCCESS) {
      DRETURN(CL_RETVAL_MALLOC);
   }

   pack_ack(&pb, type, ulong_val, ulong_val_2, str);

   ret = sge_gdi2_send_any_request(ctx, 0, NULL, rhost, commproc, id, &pb, TAG_ACK_REQUEST, 0, alpp);
   clear_packbuffer(&pb);
   answer_list_output (alpp);

   DRETURN(ret);
}
