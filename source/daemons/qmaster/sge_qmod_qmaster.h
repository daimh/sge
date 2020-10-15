#ifndef __SGE_QMOD_QMASTER_H
#define __SGE_QMOD_QMASTER_H
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

#ifndef __SGE_GDIP_H
#   include "sge_gdiP.h"
#endif

#include "uti/sge_monitor.h"

#include "gdi/sge_gdi_ctx.h"
#include "gdi/sge_gdi_packet.h"

#include "sge_qmaster_timed_event.h"


void resend_signal_event(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);
void rebuild_signal_events(void);

void        
sge_gdi_qmod(sge_gdi_ctx_class_t *ctx, sge_gdi_packet_class_t *packet, sge_gdi_task_class_t *task,
             monitoring_t *monitor);

int sge_signal_queue(sge_gdi_ctx_class_t *ctx, int how, lListElem *qep, lListElem *jep, lListElem *jatep, monitoring_t *monitor);

#endif /* __SGE_QMOD_QMASTER_H */

