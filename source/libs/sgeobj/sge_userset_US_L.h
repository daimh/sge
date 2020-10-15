#ifndef __SGE_USERSET_US_L_H
#define __SGE_USERSET_US_L_H

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

enum {
   US_name = US_LOWERBOUND,
   US_type,                  /* type of USERSET encoded as bitfield */
   US_fshare,                /* 960703 svd - functional share */
   US_oticket,               /* override tickets */
   US_job_cnt,               /* job count (internal to schedd) */
   US_pending_job_cnt,       /* job count (internal to schedd) */
   US_entries,
   US_consider_with_categories /* true, if userset plays role with categories */
};


LISTDEF(US_Type)
   JGDI_ROOT_OBJ(UserSet, SGE_US_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_USERSET_ADD) | MODIFY(sgeE_USERSET_MOD) | DELETE(sgeE_USERSET_DEL) | GET_LIST(sgeE_USERSET_LIST))
   SGE_STRING(US_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_SUBLIST | CULL_JGDI_CONF)       /* configured name spooled */
   SGE_ULONG_D(US_type, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, 1)         /* configured type spooled */
   SGE_ULONG(US_fshare, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)       /* configured share spooled */
   SGE_ULONG(US_oticket, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)      /* configured override tickets spooled */
   SGE_ULONG(US_job_cnt, CULL_DEFAULT | CULL_JGDI_CONF)     /* local to schedd */
   SGE_ULONG(US_pending_job_cnt, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* local to schedd */
   SGE_LIST(US_entries, UE_Type, CULL_DEFAULT  | CULL_SPOOL | CULL_JGDI_CONF)     /* UE_Type */
   SGE_BOOL(US_consider_with_categories, CULL_DEFAULT | CULL_JGDI_HIDDEN)
LISTEND 

NAMEDEF(USEN)
   NAME("US_name")
   NAME("US_type")
   NAME("US_fshare")
   NAME("US_oticket")
   NAME("US_job_cnt")
   NAME("US_pending_job_cnt")
   NAME("US_entries")
   NAME("US_consider_with_categories")
NAMEEND

/* *INDENT-ON* */

#define USES sizeof(USEN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_USERSETL_H */
