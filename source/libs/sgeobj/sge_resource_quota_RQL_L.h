#ifndef __SGE_RESOURCE_QUOTAL_RQL_H
#define __SGE_RESOURCE_QUOTAL_RQL_H

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

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

/* Resource Quota Limit - used only for caching scheduler-internally */
enum {
   RQL_name = RQL_LOWERBOUND,
   RQL_result,      /* dispatch_t */
   RQL_time,        /* sequential dispatching only */
   RQL_slots,       /* parallel dispatching only */
   RQL_slots_qend,   /* parallel dispatching only */
   RQL_tagged4schedule /* parallel dispatching only */
};

LISTDEF(RQL_Type)
   SGE_STRING(RQL_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE )
   SGE_INT(RQL_result, CULL_DEFAULT )
   SGE_ULONG(RQL_time, CULL_DEFAULT )
   SGE_INT(RQL_slots, CULL_DEFAULT )
   SGE_INT(RQL_slots_qend, CULL_DEFAULT )
   SGE_ULONG(RQL_tagged4schedule, CULL_DEFAULT)
LISTEND
 
NAMEDEF(RQLN)
   NAME("RQL_name")
   NAME("RQL_result")
   NAME("RQL_time")
   NAME("RQL_slots")
   NAME("RQL_slots_qend")
   NAME("RQL_tagged4schedule")
NAMEEND

#define RQLS sizeof(RQLN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif 
