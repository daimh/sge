#ifndef __SGE_CT_CT_L_H
#define __SGE_CT_CT_L_H

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
   
/* 
 * this data structures describe the category list 
 */
enum {
   CT_str = CT_LOWERBOUND,    /* string of category */
   CT_refcount,               /* number of jobs referencing the string */
   CT_count,                  /* number of jobs used in this schuling run, if -1, than CT_refcount is used */ 
   CT_rejected,               /* has this category been rejected as it can not be dispached now */
   CT_cache,                  /* stores all info, which cannot run this job category */ 
   CT_messages_added,         /* if true, the scheduler info messages have been added for this category */
   CT_resource_contribution,  /* resource request dependent contribution on urgency 
                                 this value is common with all jobs of a category */
   CT_rc_valid,               /* indicates whether cached CT_resource_contribution is valid */
   CT_reservation_rejected    /* has this category been rejected as it can not be reserved */
};

LISTDEF(CT_Type)
   SGE_STRING(CT_str, CULL_HASH | CULL_UNIQUE)
   SGE_ULONG(CT_refcount, CULL_DEFAULT)
   SGE_INT(CT_count, CULL_DEFAULT)
   SGE_ULONG(CT_rejected, CULL_DEFAULT)
   SGE_LIST(CT_cache, CCT_Type, CULL_DEFAULT)
   SGE_BOOL(CT_messages_added, CULL_DEFAULT)
   SGE_DOUBLE(CT_resource_contribution, CULL_DEFAULT)
   SGE_BOOL(CT_rc_valid, CULL_DEFAULT)
   SGE_ULONG(CT_reservation_rejected, CULL_DEFAULT)
LISTEND 

NAMEDEF(CTN)
   NAME("CT_str")
   NAME("CT_refcount")
   NAME("CT_count")
   NAME("CT_rejected")
   NAME("CT_cache")
   NAME("CT_messages_added")
   NAME("CT_resource_contribution")
   NAME("CT_rc_valid")
   NAME("CT_reservation_rejected")
NAMEEND

#define CTS sizeof(CTN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_CTL_H */
