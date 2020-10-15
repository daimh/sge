#ifndef __SGE_SPOOL_SPTR_L_H
#define __SGE_SPOOL_SPTR_L_H
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

/****** spool/--SPTR_Type ***************************************
*
*  NAME
*     SPTR_Type -- references to rules for certain object types
*
*  ELEMENTS
*     SGE_BOOL(SPTR_default, CULL_DEFAULT)
*        Defines whether the referenced rule is the default rule
*        for reading the defined object type.
*
*     SGE_STRING(SPTR_rule_name, CULL_UNIQUE)
*        Name of the referenced rule.
*
*     SGE_REF(SPTR_rule, CULL_ANY_SUBTYPE, CULL_DEFAULT)
*        Pointer/reference to the rule to be used with the
*        defined object type.
*
*
*  FUNCTION
*     Elements of SPTR_Type define a mapping between object type (SPT_Type)
*     and spooling rules (SPR_Type).
*     One object type can be spooled (written) using multiple spooling rules.
*     One object type will be read using one (the default) spooling rule.
*     One spooling rule can be referenced by multiple object types.
*
*  SEE ALSO
*     spool/--Spooling
*     spool/--SPC_Type
*     spool/--SPT_Type
****************************************************************************
*/

enum {
   SPTR_default = SPTR_LOWERBOUND,  /* is this the default rule for this object type? */
   SPTR_rule_name,                  /* name of the rule in context's rule list */
   SPTR_rule                        /* reference to rule for quicker access */
};

LISTDEF(SPTR_Type)
   SGE_BOOL(SPTR_default, CULL_DEFAULT)
   SGE_STRING(SPTR_rule_name, CULL_UNIQUE)
   SGE_REF(SPTR_rule, CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND

NAMEDEF(SPTRN)
   NAME("SPTR_default")
   NAME("SPTR_rule_name")
   NAME("SPTR_rule")
NAMEEND

#define SPTRS sizeof(SPTRN)/sizeof(char *)

#ifdef  __cplusplus
}
#endif

#endif 
