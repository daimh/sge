#ifndef __SGE_PAL_H
#define __SGE_PAL_H

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

/****** sgeobj/path_alias/--PA_Type *******************************************
*  NAME 
*     RN_Type - CULL path alias element
*
*  ELEMENTS
*     SGE_STRING(PA_origin)
*        original path        
*
*     SGE_HOST(PA_submit_host)
*        submit hostname
*
*     SGE_HOST(PA_exec_host)
*        destination execution host
*
*     SGE_STRING(PA_translation)
*        path translation for the original path
*
*  FUNCTION
*     CULL element holding information necessary to realize 
*     path aliasing. (Find more information in the --PathAlias
*     ADOC comment)
*
*  SEE ALSO
*     gdi/path_alias/--PathAlias
*     gdi/path_alias/path_alias_read_from_file()
*     gdi/path_alias/path_alias_list_initialize()
*     gdi/path_alias/path_alias_list_get_path()
******************************************************************************/
enum {
   PA_origin = PA_LOWERBOUND,
   PA_submit_host,
   PA_exec_host,
   PA_translation
};

LISTDEF(PA_Type)
   JGDI_OBJ(PathAlias)
   SGE_STRING(PA_origin, CULL_PRIMARY_KEY | CULL_DEFAULT | CULL_SPOOL)
   SGE_HOST(PA_submit_host, CULL_DEFAULT | CULL_SPOOL) 
   SGE_HOST(PA_exec_host, CULL_DEFAULT | CULL_SPOOL) 
   SGE_STRING(PA_translation, CULL_DEFAULT | CULL_SPOOL)
LISTEND 

NAMEDEF(PAN)
   NAME("PA_origin")
   NAME("PA_submit_host")
   NAME("PA_exec_host")
   NAME("PA_translation")
NAMEEND

/* *INDENT-ON* */  

#define PAS sizeof(PAN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_PAL_H */
