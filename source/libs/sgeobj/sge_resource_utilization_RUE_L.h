#ifndef __SGE_RESOURCE_UTILIZATIONL_H
#define __SGE_RESOURCE_UTILIZATIONL_H

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
 *   Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */ 

/****** sgeobj/RUE/--RUE_Type **************************************************
*  NAME
*     RUE_Type - Resource Utilization Entry
*
*  ELEMENTS
*
*     SGE_STRING(RUE_name)
*        Name of the resource the entry is about
*
*     SGE_DOUBLE(RUE_utilized_now)
*        Currently used amount
*
*     SGE_LIST(RUE_utilized)
*        A resource diagram indicating future utilization
*
*     SGE_DOUBLE(RUE_utilized_now_nonexclusive)
*        Currently used amount of implicitly used exclusive resources
*
*     SGE_LIST(RUE_utilized_nonexclusive)
*        A resource diagram indicating future utilization of implicitly used
*        exclusive resources
*
*
*  FUNCTION
*        A resource utilization entry contains all information
*        about utilization of a particular resouce at any time.
*
*        The resources managed with resource utilization entries are
*        parallel environment slot resource, global resources, host
*        resources and resources.
*
*        For a not looking ahead information about the amount
*        utilized now (RUE_utilized_now) is sufficient:
*
*              A(0) = C - U(0)
*
*        For resource reservation schedulers also information about future
*        utilization over time is required (RUE_utilized):
*
*              A(t) = C - U(t)
*
*        For advance reservation additional information is needed for each
*        single reservation to reflect reserved resource amount over time
*        (RUE_reservations). Based on this the resource amount available for
*        a job can be determined depending on the advance reservation:
*
*              A1(t) = C - U(t) - (         R2(t) + ... + RN(t) )
*              A2(t) = C - U(t) - ( R1(t)         + ... + RN(t) )
*
*                :   :                  :
*
*              AN(t) = C - U(t) - ( R1(t) + R2(t) + ...         )
*
*        and for jobs that do not use an advance reservation the smallest
*        resource amount is available:
*
*              A(t)  = C - U(t) - ( R1(t) + R2(t) + ... + RN(t) )
*
*  SEE ALSO
******************************************************************************/

enum {
   RUE_name = RUE_LOWERBOUND,
   RUE_utilized_now,
   RUE_utilized,
   RUE_utilized_now_nonexclusive,
   RUE_utilized_nonexclusive
};

LISTDEF(RUE_Type)
   JGDI_OBJ(ResourceUtilization)
   SGE_STRING(RUE_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL | CULL_SUBLIST | CULL_PRIMARY_KEY)
   SGE_DOUBLE(RUE_utilized_now, CULL_DEFAULT)
   SGE_LIST(RUE_utilized, RDE_Type, CULL_DEFAULT)
   SGE_DOUBLE(RUE_utilized_now_nonexclusive, CULL_DEFAULT)
   SGE_LIST(RUE_utilized_nonexclusive, RDE_Type, CULL_DEFAULT)
LISTEND

NAMEDEF(RUEN)
   NAME("RUE_name")
   NAME("RUE_utilized_now")
   NAME("RUE_utilized")
   NAME("RUE_utilized_now_nonexclusive")
   NAME("RUE_utilized_nonexclusive")
NAMEEND

#define RUES sizeof(RUEN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_RESOURCE_UTILIZATIONL_H */
