#ifndef __SGE_RESOURCE_UTILIZATIONL_RDE_H
#define __SGE_RESOURCE_UTILIZATIONL_RDE_H

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

/****** sgeobj/RDE/--RDE_Type **************************************************
*  NAME
*     RDE_Type - Resource Diagram Entry
*
*  ELEMENTS
*
*     SGE_ULONG(RDE_time)
*        Time of resource diagram entry
*
*     SGE_DOUBLE(RDE_amount)
*        Amount since that time
*
*  FUNCTION
*     A list of resource diaram entries represents a diagram showing 
*     resource progression over time. E.g. the following resource
*     diagram 
*
*               N ^
*                 |             +-------+
*                 |   +-----+   |  J3   |
*                 |   |     +---+-------+--+
*                 |   | J1  |      J2      |
*                 +---+-----+--------------+-----> t
*                 0   4     10  14      22 25
*
*     is respresented by the the following table 
*
*                 t | N
*                ---+---
*                 0 | 0
*                 4 | 3
*                10 | 2
*                14 | 4
*                22 | 2
*                25 | 2
*
*  SEE ALSO
******************************************************************************/

enum {
   RDE_time = RDE_LOWERBOUND,
   RDE_amount
};

LISTDEF(RDE_Type)
   JGDI_OBJ(ResourceDiagramEntry)
   SGE_ULONG(RDE_time, CULL_DEFAULT)
   SGE_DOUBLE(RDE_amount, CULL_DEFAULT)
LISTEND 

NAMEDEF(RDEN)
   NAME("RDE_time")
   NAME("RDE_amount")
NAMEEND

#define RDES sizeof(RDEN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_RESOURCE_UTILIZATIONL_H */
