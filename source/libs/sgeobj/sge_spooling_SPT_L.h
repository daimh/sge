#ifndef __SGE_SPOOL_SPT_L_H
#define __SGE_SPOOL_SPT_L_H
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

/****** spool/--SPT_Type ***************************************
*
*  NAME
*     SPT_Type -- Spooling object types
*
*  ELEMENTS
*     SGE_ULONG(SPT_type, CULL_HASH | CULL_UNIQUE)
*        Unique type identifier.
*        See enum sge_object_type in libs/gdi/sge_mirror.h
*        SGE_TYPE_ALL describes a default type entry for all
*        object types.
*
*     SGE_STRING(SPT_name, CULL_DEFAULT)
*        Name of the type - used for informational messages etc.
*
*     SGE_LIST(SPT_rules, SPTR_Type, CULL_DEFAULT)
*        List of rules that can be applied for a certain object type.
*        Does not reference the rules themselves, but contains mapping
*        objects mapping between type and rule.
*
*
*  FUNCTION
*     Objects to be spooled have a certain type that can be identified
*     by the sge_object_type enum.
*     A spooling context can contain information about individual
*     types and/or define a default behaviour for all (not individually
*     handled) types.
*
*     The spooling behaviour for a type is defined by a list of references
*     to rules in the spooling context.
*     One of the referenced spooling rules has to be made default rule
*     for reading objects.
*
*  NOTES
*     The type identifiers should not come from the mirroring interface,
*     but be part of a more general type information handling in libgdi.
*
*  SEE ALSO
*     spool/--Spooling
*     spool/--SPC_Type
*     spool/--SPTR_Type
****************************************************************************
*/

enum {
   SPT_type = SPT_LOWERBOUND,      /* sge_object_type, SGE_TYPE_ALL = default */
   SPT_name,                       /* name of the type, e.g. "JB_Type" */
   SPT_rules                       /* list of rules to spool this object type */
};

LISTDEF(SPT_Type)
   SGE_ULONG(SPT_type, CULL_HASH | CULL_UNIQUE)
   SGE_STRING(SPT_name, CULL_DEFAULT)
   SGE_LIST(SPT_rules, SPTR_Type, CULL_DEFAULT)
LISTEND

NAMEDEF(SPTN)
   NAME("SPT_type")
   NAME("SPT_name")
   NAME("SPT_rules")
NAMEEND

#define SPTS sizeof(SPTN)/sizeof(char *)

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_SPOOLL_H */
