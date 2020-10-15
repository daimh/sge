#ifndef __SGE_CT_CCT_L_H
#define __SGE_CT_CCT_L_H

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

/**
 * 
 * The caching of dispatch results has to be done per PE. For jobs, which
 * do not request a PE, the pe_name is set to "NONE".
 *
 */
enum {
   CCT_pe_name = CCT_LOWERBOUND,   /* pe name */
   CCT_ignore_queues,         /* stores all queues, which now cannot run this job category */ 
   CCT_ignore_hosts,          /* stores all hosts, which now cannot run this job category */
   CCT_pe_job_slots,          /* stores the posible pe slots */   
   CCT_pe_job_slot_count      /* number of values in the array */   
};

LISTDEF(CCT_Type)
   SGE_STRING(CCT_pe_name, CULL_HASH | CULL_UNIQUE)
   SGE_LIST(CCT_ignore_queues, CTI_Type, CULL_DEFAULT)
   SGE_LIST(CCT_ignore_hosts, CTI_Type, CULL_DEFAULT)
   SGE_REF(CCT_pe_job_slots, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_ULONG(CCT_pe_job_slot_count, CULL_DEFAULT)
LISTEND 

NAMEDEF(CCTN)
   NAME("CCT_pe_name")
   NAME("CCT_ignore_queues")
   NAME("CCT_ignore_hosts")
   NAME("CCT_pe_job_slots")
   NAME("CCT_pe_job_slot_count")
NAMEEND

#define CCTS sizeof(CCTN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_CTL_H */
