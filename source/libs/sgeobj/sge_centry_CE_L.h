#ifndef __SGE_COMPLEXL_H
#define __SGE_COMPLEXL_H

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

#include "sgeobj/sge_boundaries.h"
#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */     

enum {
   CE_name = CE_LOWERBOUND,
   CE_shortcut,
   CE_valtype,
   CE_stringval,
   CE_doubleval,
   CE_relop,
   CE_consumable,
   CE_default,
   CE_dominant,
   CE_pj_stringval,          /* per job */
   CE_pj_doubleval,
   CE_pj_dominant,
   CE_requestable,
   CE_tagged,
   CE_urgency_weight
};

LISTDEF(CE_Type)
   JGDI_ROOT_OBJ(ComplexEntry, SGE_CE_LIST, ADD | MODIFY | DELETE | GET | GET_LIST)
   JGDI_EVENT_OBJ(ADD(sgeE_CENTRY_ADD) | MODIFY(sgeE_CENTRY_MOD) | DELETE(sgeE_CENTRY_DEL) | GET_LIST(sgeE_CENTRY_LIST))
   SGE_STRING(CE_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_SUBLIST | CULL_PRIMARY_KEY | CULL_JGDI_CONF) /* full name of attribute */
   SGE_STRING_D(CE_shortcut, CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_JGDI_CONF, "NONE")      /* shortcut name of attribute */
   SGE_ULONG_D(CE_valtype, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF, TYPE_INT)        /* type */
   SGE_STRING(CE_stringval, CULL_DEFAULT | CULL_SPOOL | CULL_SUBLIST | CULL_JGDI_CONF)     /* non overwritten value */
   SGE_DOUBLE(CE_doubleval, CULL_DEFAULT | CULL_JGDI_HIDDEN)    /* parsed CE_stringval */
   SGE_ULONG(CE_relop, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)          /* relational operator */
   SGE_ULONG(CE_consumable, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)      /* flag consumable */
   SGE_STRING(CE_default, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)      /* default request for consumable */
   SGE_ULONG(CE_dominant, CULL_DEFAULT | CULL_JGDI_HIDDEN)      /* monitoring facility */
   SGE_STRING(CE_pj_stringval, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* per job string value */
   SGE_DOUBLE(CE_pj_doubleval, CULL_DEFAULT | CULL_JGDI_HIDDEN) /* per job parsed CE_stringval */
   SGE_ULONG(CE_pj_dominant, CULL_DEFAULT | CULL_JGDI_HIDDEN)   /* per job monitoring facility */
   SGE_ULONG(CE_requestable, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF)
   SGE_ULONG(CE_tagged, CULL_DEFAULT | CULL_JGDI_HIDDEN)        /* used to tag resource request, which can be fulfilled */
   SGE_STRING(CE_urgency_weight, CULL_DEFAULT | CULL_SPOOL | CULL_JGDI_CONF) /* static weighting factor */
LISTEND 

NAMEDEF(CEN)
   NAME("CE_name")
   NAME("CE_shortcut")
   NAME("CE_valtype")
   NAME("CE_stringval")
   NAME("CE_doubleval")
   NAME("CE_relop")
   NAME("CE_consumable")
   NAME("CE_default")
   NAME("CE_dominant")
   NAME("CE_pj_stringval")
   NAME("CE_pj_doubleval")
   NAME("CE_pj_dominant")
   NAME("CE_requestable")
   NAME("CE_tagged")
   NAME("CE_urgency_weight")
NAMEEND

/* *INDENT-ON* */ 

#define CES sizeof(CEN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_COMPLEXL_H */
