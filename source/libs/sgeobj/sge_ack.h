#ifndef __SGE_ACK_H 
#define __SGE_ACK_H 
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

#include "sgeobj/sge_ack_ACK_L.h"
#include "gdi/sge_gdi_ctx.h"

enum {
   ACK_JOB_EXIT,         /* sent back by qmaster, when execd sends a job_exit */
   ACK_SIGNAL_JOB,       /* sent back by qmaster, when execd reports a job as */
                         /* running - that was not supposed to be there       */
   ACK_EVENT_DELIVERY,   /* sent back by schedd, when master sends events     */
   ACK_SIGJOB,           /* send back by execd, when qmaster signals job      */
   ACK_SIGQUEUE,         /* send back by execd, when qmaster signals queue    */
   ACK_LOAD_REPORT,      /* send back by qmaster, when execd sends a load report */
   ACK_SIGNAL_SLAVE,     /* sent to slave execds when master task finished    */
   ACK_JOB_REPORT_RESEND /* sent to master execd to resend master task finish as trigger */
};

int pack_ack(sge_pack_buffer *pb, u_long32 type, u_long32 id, u_long32 id2, const char *str);

int sge_send_ack_to_qmaster(sge_gdi_ctx_class_t *ctx, u_long32 type, u_long32 ulong_val, 
                            u_long32 ulong_val_2, const char *str, lList **alpp);


#endif /* __SGE_ACK_H */

