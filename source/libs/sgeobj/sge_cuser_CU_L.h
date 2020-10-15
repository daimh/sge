#ifndef __SGE_CUSERL_H
#define __SGE_CUSERL_H

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
   CU_name = CU_LOWERBOUND,       
   CU_ruser_list,
   CU_ulong32,
   CU_bool,
   CU_time,
   CU_mem,
   CU_inter
};

LISTDEF(CU_Type)
   SGE_STRING(CU_name, CULL_HASH | CULL_UNIQUE | CULL_SPOOL)
   SGE_LIST(CU_ruser_list, ASTR_Type, CULL_DEFAULT | CULL_SPOOL)
   SGE_LIST(CU_ulong32, AULNG_Type, CULL_DEFAULT)
   SGE_LIST(CU_bool, ABOOL_Type, CULL_DEFAULT)
   SGE_LIST(CU_time, ATIME_Type, CULL_DEFAULT)
   SGE_LIST(CU_mem, AMEM_Type, CULL_DEFAULT)
   SGE_LIST(CU_inter, AINTER_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(CUN)
   NAME("CU_name")
   NAME("CU_ruser_list")
   NAME("CU_ulong32")
   NAME("CU_bool")
   NAME("CU_time")
   NAME("CU_mem")
   NAME("CU_inter")
NAMEEND

#define CUS sizeof(CUN)/sizeof(char*)

/* *INDENT-ON* */   

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_CUSERL_H */
