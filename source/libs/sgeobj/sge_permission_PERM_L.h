#ifndef __SGE_PERMISSIONL_H
#define __SGE_PERMISSIONL_H

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

/****** gdi/sge/PERM_Type *****************************************************
*  NAME
*     PERM_Type -- CULL Permission element
*
*  SYNOPSIS
*     PERM_Type
*     +--- PERM_manager: 0 or 1 (1 means user has the right)
*     +--- PERM_operator: 0 or 1 (1 means user has the right)
*     +--- PERM_req_host: Name of destination host
*     +--- PERM_req_username: Username on destination host
*     +--- PERM_sge_username: Username on master host
*     
*  FUNCTION
*     This list is used for an SGE_GDI_PERMCHECK gdi request. The 
*     sge_gdi() function is called with SGE_DUMMY_LIST. The qmaster 
*     will fill the list with the permisson of the user who is calling. 
*     If a PERM_Type list is given to the sge_gdi() function the 
*     master is looking for the PERM_req_host value. The 
*     PERM_req_username and PERM_sge_username values are filled up with 
*     the correct user mapping names.
*
*     sge_gdi_get_mapping_name() is using the SGE_GDI_PERMCHECK gdi 
*     request. 
* 
*  EXAMPLE
*        permList = lCreateList("permissions", PERM_Type);
*        ep = lCreateElem(PERM_Type);
*        lAppendElem(permList,ep);
*        lSetString(ep, PERM_req_host, requestedHost); 
*     
*        alp = sge_gdi(SGE_DUMMY_LIST, SGE_GDI_PERMCHECK,  &permList, 
*                      NULL, NULL);
*        
*        if (permList != NULL) {
*           ep = permList->first;
*           if (ep != NULL) {
*              mapName = lGetString(ep, PERM_req_username ); 
*           } 
*        }
*       
*  SEE ALSO
*    gdilib/sge_gdi_get_mapping_name()
*    gdilib/sge_gdi()
*****************************************************************************/

/* *INDENT-OFF* */ 

/*
 * permission list  
 */
enum {
   PERM_manager = PERM_LOWERBOUND,    /* is master ? */
   PERM_operator,                     /* is operator ? */
   PERM_req_host,                     /* Name of destination host */
   PERM_req_username,                 /* Username on destination host */
   PERM_sge_username                  /* username on master host */
};

LISTDEF(PERM_Type)
   SGE_ULONG(PERM_manager, CULL_DEFAULT)            /* 0 or 1 (1 means user has the right) */
   SGE_ULONG(PERM_operator, CULL_DEFAULT)           /* 0 or 1 (1 means user has the right) */
   SGE_HOST(PERM_req_host, CULL_DEFAULT)          /* Name of destination host */
   SGE_STRING(PERM_req_username, CULL_DEFAULT)      /* Username on destination host */
   SGE_STRING(PERM_sge_username, CULL_DEFAULT)      /* username on master host */
LISTEND 

NAMEDEF(PERMN)
   NAME("PERM_manager")
   NAME("PERM_operator")
   NAME("PERM_req_host")
   NAME("PERM_req_username")
   NAME("PERM_sge_username")
NAMEEND

/* *INDENT-ON* */

#define PERMS sizeof(PERMN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_PERMISSIONL_H */
