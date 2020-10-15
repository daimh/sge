#ifndef __SGE_SPOOL_SPC_L_H
#define __SGE_SPOOL_SPC_L_H
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

/****** spool/--SPC_Type ***************************************
*
*  NAME
*     SPC_Type -- Spooling context
*
*  ELEMENTS
*     SGE_STRING(SPC_name, CULL_HASH | CULL_UNIQUE)
*        Unique name of the spooling context
*
*     SGE_LIST(SPC_rules, SPR_Type, CULL_DEFAULT)
*        List of spooling rules
*  
*     SGE_LIST(SPC_types, SPT_Type, CULL_DEFAULT)
*        List of spoolable object types with references to 
*        rules.
*
*  FUNCTION
*        A spooling context describes the way how objects
*        are spooled (read and written).
*
*        A spooling context contains one or multiple rules for 
*        spooling. A rule can for example describe a database 
*        connection.
*
*        It also contains a list of types that can be spooled.
*        A default entry for all types can be created; if type entries
*        for individual types exist, these entries will be used for spooling.
*        A type references one or multiple rules which will 
*        be executed for writing or deleting data.
*        Exactly one rule can be defined to be the default rule
*        for reading objects.
*
*        +----------+       1:n       +----------+
*        | SPC_Type |----------------<| SPT_Type | 
*        +----------+                 +----------+
*             |                             |
*             |                             |
*             |                             |
*             | 1                           |
*             | :                           |
*             | n                           |
*             |                             |
*             ^                             |
*        +----------+   1:n, one is default |
*        | SPR_Type |>----------------------+
*        +----------+
*
*
*  SEE ALSO
*     spool/--Spooling
*     spool/--SPR_Type
*     spool/--SPT_Type
*     spool/--SPTR_Type
****************************************************************************
*/

enum {
   SPC_name = SPC_LOWERBOUND,  /* name of spooling context */
   SPC_rules,                  /* list of spooling rules */
   SPC_types                   /* list of object types to spool */
};

LISTDEF(SPC_Type)
   SGE_STRING(SPC_name, CULL_HASH | CULL_UNIQUE)
   SGE_LIST(SPC_rules, SPR_Type, CULL_DEFAULT)
   SGE_LIST(SPC_types, SPT_Type, CULL_DEFAULT)
LISTEND

NAMEDEF(SPCN)
   NAME("SPC_name")
   NAME("SPC_rules")
   NAME("SPC_types")
NAMEEND

#define SPCS sizeof(SPCN)/sizeof(char *)

#ifdef  __cplusplus
}
#endif

#endif 
