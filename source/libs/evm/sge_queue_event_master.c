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

#include "sge.h"

#include "uti/sge_rmon.h"

#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_event.h"
#include "sgeobj/sge_qinstance.h"

#include "sge_event_master.h"
#include "sge_queue_event_master.h"

void
qinstance_add_event(lListElem *this_elem, ev_event type)
{
   DENTER(TOP_LAYER, "qinstance_add_event");
   sge_add_event(0, type, 0, 0,
                 lGetString(this_elem, QU_qname),
                 lGetHost(this_elem, QU_qhostname), NULL, this_elem);
   DRETURN_VOID;
}

void
cqueue_add_event(lListElem *this_elem, ev_event type)
{
   DENTER(TOP_LAYER, "cqueue_add_event");
   sge_add_event(0, type, 0, 0,
                 lGetString(this_elem, CQ_name), NULL,
                 NULL, this_elem);
   DRETURN_VOID;
}
