#ifndef __SGE_RESOURCE_QUOTAL_RQRL_H
#define __SGE_RESOURCE_QUOTAL_RQRL_H

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

/* Resource Quota Rule Limit */
enum {
   RQRL_name = RQRL_LOWERBOUND,
   RQRL_value,
   RQRL_type,
   RQRL_dvalue,
   RQRL_usage,
   RQRL_dynamic
};

LISTDEF(RQRL_Type)
 JGDI_OBJ(ResourceQuotaRuleLimit)
 SGE_STRING(RQRL_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_STRING(RQRL_value, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
 SGE_ULONG(RQRL_type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
 SGE_DOUBLE(RQRL_dvalue, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_RO)
 SGE_LIST(RQRL_usage, RUE_Type, CULL_DEFAULT | CULL_JGDI_RO)
 SGE_BOOL(RQRL_dynamic, CULL_DEFAULT | CULL_JGDI_RO)
LISTEND
 
NAMEDEF(RQRLN)
   NAME("RQRL_name")
   NAME("RQRL_value")
   NAME("RQRL_type")
   NAME("RQRL_dvalue")
   NAME("RQRL_usage")
   NAME("RQRL_dynamic")
NAMEEND

#define RQRLS sizeof(RQRLN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_RESOURCE_QUOTAL_H */
