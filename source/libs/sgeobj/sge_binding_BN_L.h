#ifndef __SGE_BINDINGL_H
#define __SGE_BINDINGL_H

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

/****** sgeobj/binding/--BN_Type ************************************************
*  NAME
*     BN_Type -- CULL binding element
*
*  ELEMENTS
*     DG TODO
*     SGE_ULONG(RN_min)
*        minimum or start value of an id range (e.g. 1)
*
*     SGE_ULONG(RN_max)
*        maximum or end value of an id range (e.g. 9)
*
*     SGE_ULONG(RN_step)
*        stepsize (e.g. 2)
* 
*  FUNCTION
*     CULL element holding values which define a id range
*     (e.g. 1-9:2 => 1, 3, 5, 7, 9). 
*     Lists of this CULL element are hold within a CULL job element
*     (JB_Type) to hold job array task ids.
*     Several functions may be used to access/modify/delete range 
*     elements and range lists. You may find them in the 'SEE ALSO' 
*     section. It is highly advised to use these access functions
*     because they assure and require a defined structure of 
*     elements and lists.
*     
*
*  SEE ALSO 
*     gdi/range/range_sort_uniq_compress()
*     gdi/job/JB_Type
******************************************************************************/

/* *INDENT-OFF* */ 

enum {
   BN_strategy = BN_LOWERBOUND,
   BN_type,
   BN_parameter_n,
   BN_parameter_socket_offset,
   BN_parameter_core_offset,
   BN_parameter_striding_step_size,
   BN_parameter_explicit
};

LISTDEF(BN_Type)
   JGDI_OBJ(Binding)
   SGE_STRING(BN_strategy, CULL_PRIMARY_KEY | CULL_SUBLIST)
   SGE_ULONG(BN_type, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(BN_parameter_n, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(BN_parameter_socket_offset, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(BN_parameter_core_offset, CULL_DEFAULT | CULL_SUBLIST)
   SGE_ULONG(BN_parameter_striding_step_size, CULL_DEFAULT | CULL_SUBLIST)
   SGE_STRING(BN_parameter_explicit, CULL_DEFAULT | CULL_SUBLIST)
LISTEND 

NAMEDEF(BNN)
   NAME("BN_strategy")
   NAME("BN_type")
   NAME("BN_parameter_n")
   NAME("BN_parameter_socket_offset")
   NAME("BN_parameter_core_offset")
   NAME("BN_parameter_striding_step_size")
   NAME("BN_parameter_explicit")
NAMEEND

/* *INDENT-ON* */  

#define BNS sizeof(BNN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_BINDINGL_H */

