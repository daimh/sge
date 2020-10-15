#ifndef PARSE_QSUBL_H
#define PARSE_QSUBL_H

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
   SPA_number = SPA_LOWERBOUND,
   SPA_argtype,
   SPA_switch,
   SPA_switch_arg,
   SPA_occurrence,
   SPA_argval_lFloatT,
   SPA_argval_lDoubleT,
   SPA_argval_lUlongT,
   SPA_argval_lLongT,
   SPA_argval_lCharT,
   SPA_argval_lIntT,
   SPA_argval_lStringT,
   SPA_argval_lListT
};

LISTDEF(SPA_Type)
   SGE_ULONG(SPA_number, CULL_DEFAULT)
   SGE_ULONG(SPA_argtype, CULL_DEFAULT)
   SGE_STRING(SPA_switch, CULL_HASH)
   SGE_STRING(SPA_switch_arg, CULL_DEFAULT)
   SGE_ULONG(SPA_occurrence, CULL_DEFAULT)
   SGE_FLOAT(SPA_argval_lFloatT, CULL_DEFAULT)
   SGE_DOUBLE(SPA_argval_lDoubleT, CULL_DEFAULT)
   SGE_ULONG(SPA_argval_lUlongT, CULL_DEFAULT)
   SGE_LONG(SPA_argval_lLongT, CULL_DEFAULT)
   SGE_CHAR(SPA_argval_lCharT, CULL_DEFAULT)
   SGE_INT(SPA_argval_lIntT, CULL_DEFAULT)
   SGE_STRING(SPA_argval_lStringT, CULL_DEFAULT)
   SGE_LIST(SPA_argval_lListT, ST_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(SPAN)
   NAME("SPA_number")
   NAME("SPA_argtype")
   NAME("SPA_switch")
   NAME("SPA_switch_arg")
   NAME("SPA_occurrence")
   NAME("SPA_argval_lFloatT")
   NAME("SPA_argval_lDoubleT")
   NAME("SPA_argval_lUlongT")
   NAME("SPA_argval_lLongT")
   NAME("SPA_argval_lCharT")
   NAME("SPA_argval_lIntT")
   NAME("SPA_argval_lStringT")
   NAME("SPA_argval_lListT")
NAMEEND

/* *INDENT-ON* */ 

#define SPAS sizeof(SPAN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* PARSE_QSUBL_H */
