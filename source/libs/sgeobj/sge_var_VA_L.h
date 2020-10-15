#ifndef __SGE_VAR_VA_L_H
#define __SGE_VAR_VA_L_H

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

/****** sgeobj/var/--VA_Type **************************************************
*  NAME
*     VA_Type - CULL variable element
*
*  ELEMENTS
*     SGE_STRING(VA_variable)
*        name of variable
*
*     SGE_STRING(VA_value)
*        value of variable
*
*  FUNCTION
*     CULL element holding a variable/value pair (variable[=value])
*
*  SEE ALSO
*     gdi/var/--VariableList
******************************************************************************/

enum {
   VA_variable = VA_LOWERBOUND,
   VA_value
};

LISTDEF(VA_Type)
   JGDI_MAP_OBJ(VA_variable, VA_value)
   SGE_STRING(VA_variable, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)
   SGE_STRING(VA_value, CULL_DEFAULT | CULL_SUBLIST)
LISTEND

NAMEDEF(VAN)
   NAME("VA_variable")
   NAME("VA_value")
NAMEEND

#define VAS sizeof(VAN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif

#endif 
