#ifndef _EXECD_SIGNAL_QUEUE_H
#define _EXECD_SIGNAL_QUEUE_H
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

#include "gdi/sge_gdi2.h"

int do_signal_queue(sge_gdi_ctx_class_t *ctx, struct_msg_t *aMsg, sge_pack_buffer *apb);

int signal_job(u_long32 jobid, u_long32 jataskid, u_long32 signal);

int sge_execd_deliver_signal(u_long32 sig, lListElem *jep, lListElem *jatep);


int sge_kill(int pid, u_long32 sge_signal, u_long32 jobid, u_long32 jataskid, 
             const char *petask);

void sge_send_suspend_mail(u_long32 signal, lListElem *jep, lListElem *jatep);

#endif /* _EXECD_SIGNAL_QUEUE_H */
