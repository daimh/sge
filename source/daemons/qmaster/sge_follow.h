#ifndef __SGE_FOLLOW_H
#define __SGE_FOLLOW_H
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

#include "uti/sge_monitor.h"
#include "sgeobj/sge_object.h"
#include "gdi/sge_gdi_ctx.h"
#include "sge_qmaster_timed_event.h"

int
sge_follow_order(sge_gdi_ctx_class_t *ctx,
                 lListElem *order, char *ruser,
                 char *rhost, lList **topp, monitoring_t *monitor,
                 object_description *object_base);

int 
distribute_ticket_orders(sge_gdi_ctx_class_t *ctx, 
                 lList *ticket_orders, monitoring_t *monitor, 
                 object_description *object_base);

void 
sge_set_next_spooling_time(void);

/* EB: TODO: ST: remove this ? */
void sge_process_order_event(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, 
                        monitoring_t *monitor);

#endif /* __SGE_FOLLOW_H */
