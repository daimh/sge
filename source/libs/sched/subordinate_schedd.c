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

#include <stdio.h>

#include "uti/sge_rmon.h"

#include "cull/cull.h"

#include "sgeobj/sge_qinstance.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_subordinate.h"

#include "subordinate_schedd.h"

/*
qname: name of a queue that needs suspension on subordinate
qlist: complete queue list for recursivly suspension of other queues
*/
int sos_schedd(const char *qname, lList *qlist) 
{
   lListElem *q;
   u_long32 sos;
   int ret = 0;

   DENTER(TOP_LAYER, "sos_schedd");

   q = qinstance_list_locate2(qlist, qname);
   if (!q) {
      /* 
         In the qlist we got is only a subset of all
         queues. If the rest of the queues is really
         suspended then this is no error because
         they are already suspended.
      */
      DEXIT;
      return 1;
   }

   /* increment sos counter */
   sos = lGetUlong(q, QU_suspended_on_subordinate);
   lSetUlong(q, QU_suspended_on_subordinate, ++sos);

   /* first sos ? */
   if (sos==1) {
      DPRINTF(("QUEUE %s GETS SUSPENDED ON SUBORDINATE\n", qname));
      /* state transition */
      qinstance_state_set_susp_on_sub(q, true);
   }

   DEXIT;
   return ret;
}

