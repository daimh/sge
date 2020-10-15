#ifndef __SGE_USERPRJ_UPU_L_H
#define __SGE_USERPRJ_UPU_L_H

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

/*
 * This is the list type we use to hold the 
 * information for user/project. This objects are targets of throwing
 * tickets to them and as usage accumulators. There are no real differences 
 * at the moment, so putting them together is convenient.
 */

enum {
   UPU_job_number = UPU_LOWERBOUND,  /* job number */
   UPU_old_usage_list        /* UA_Type still debited usage set and used
                              * via orders by ted_job_usageschedd by
                              * qmaster */
};

LISTDEF(UPU_Type)
   JGDI_OBJ(JobUsage)
   SGE_ULONG(UPU_job_number, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)
   SGE_MAP(UPU_old_usage_list, UA_Type, CULL_DEFAULT | CULL_SUBLIST)
LISTEND 

NAMEDEF(UPUN)
   NAME("UPU_job_number")
   NAME("UPU_old_usage_list")
NAMEEND

#define UPUS sizeof(UPUN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                         
