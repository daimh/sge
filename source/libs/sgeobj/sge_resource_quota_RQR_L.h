#ifndef __SGE_RESOURCE_QUOTAL_RQR_L_H
#define __SGE_RESOURCE_QUOTAL_RQR_L_H

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

/* Resource Quota Rule */
enum {
   RQR_name = RQR_LOWERBOUND,
   RQR_filter_users,
   RQR_filter_projects,
   RQR_filter_pes,
   RQR_filter_queues,
   RQR_filter_hosts,
   RQR_limit,
   RQR_level
};

LISTDEF(RQR_Type)
 JGDI_OBJ(ResourceQuotaRule)
 SGE_STRING(RQR_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_OBJECT(RQR_filter_users, RQRF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_OBJECT(RQR_filter_projects, RQRF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_OBJECT(RQR_filter_pes, RQRF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_OBJECT(RQR_filter_queues, RQRF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_OBJECT(RQR_filter_hosts, RQRF_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_LIST(RQR_limit, RQRL_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_ULONG(RQR_level, CULL_DEFAULT | CULL_JGDI_RO) /* maintained by qmaster needed only in parallel 
                                                      scheduling for accumulating slots after queues were tagged */
LISTEND

NAMEDEF(RQRN)
   NAME("RQR_name")
   NAME("RQR_filter_users")
   NAME("RQR_filter_projects")
   NAME("RQR_filter_pes")
   NAME("RQR_filter_queues")
   NAME("RQR_filter_hosts")
   NAME("RQR_limit")
   NAME("RQR_level")
NAMEEND

#define RQRS sizeof(RQRN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_RESOURCE_QUOTAL_H */
