#ifndef __SGE_RESOURCE_QUOTAL_RQRF_H
#define __SGE_RESOURCE_QUOTAL_RQRF_H

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

/* Resource Quota Rule Filter */
enum {
   RQRF_expand = RQRF_LOWERBOUND,
   RQRF_scope,
   RQRF_xscope
};

LISTDEF(RQRF_Type)
 JGDI_OBJ(ResourceQuotaRuleFilter)
 SGE_BOOL(RQRF_expand, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_LIST(RQRF_scope, ST_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_LIST(RQRF_xscope, ST_Type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
LISTEND

NAMEDEF(RQRFN)
   NAME("RQRF_expand")
   NAME("RQRF_scope")
   NAME("RQRF_xscope")
NAMEEND

#define RQRFS sizeof(RQRFN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_RESOURCE_QUOTAL_H */
