#ifndef __SGE_JAPI_NSV_L_H
#define __SGE_JAPI_NSV_L_H

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

/****** japi/--NSV_Type **************************************************
*  NAME
*     NSV_Type - Named string vector
*
*  ELEMENTS
*     SGE_STRING(NSV_name)
*        name of the string vector
*
*     SGE_LIST(NSV_strings)
*        strings of this string vector
*
*  FUNCTION
*     CULL element implementing DRMAA vector job template attributes 
******************************************************************************/
enum {
   NSV_name = NSV_LOWERBOUND,
   NSV_strings
};

LISTDEF(NSV_Type)
   SGE_STRING(NSV_name, CULL_DEFAULT)
   SGE_LIST(NSV_strings, ST_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(NSVN)
   NAME("NSV_name")
   NAME("NSV_strings")
NAMEEND

#define NSVS sizeof(NSVN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif  /* __SGE_JAPIL_H */

