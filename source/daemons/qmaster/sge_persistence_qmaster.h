#ifndef __SGE_PERSISTENCE_QMASTER_H
#define __SGE_PERSISTENCE_QMASTER_H
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

#include "sgeobj/sge_event.h"
#include "sge_qmaster_timed_event.h"
#include "uti/sge_monitor.h"
#include "gdi/sge_gdi_ctx.h"


bool
sge_initialize_persistence(sge_gdi_ctx_class_t *ctx, lList **answer_list);

bool
sge_shutdown_persistence(lList **answer_list);

void
sge_initialize_persistance_timer(void);

void
spooling_trigger_handler(sge_gdi_ctx_class_t *ctx, te_event_t anEvent, monitoring_t *monitor);

bool
sge_event_spool(sge_gdi_ctx_class_t *ctx, lList **answer_list, u_long32 timestamp, ev_event type, 
                u_long32 intkey1, u_long32 intkey2, const char *strkey, 
                const char *strkey2, const char *session, lListElem *object, 
                lListElem *sub_object1, lListElem *sub_object2, 
                bool send_event, bool spool);

#endif /* __SGE_PERSISTENCE_QMASTER_H */
