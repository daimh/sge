#ifndef __SGE_IDL_H
#define __SGE_IDL_H

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
   ID_str = ID_LOWERBOUND,
   ID_ja_structure,
   ID_action,
   ID_force,
   ID_user_list
};

LISTDEF(ID_Type)
   SGE_STRING(ID_str, CULL_DEFAULT)
   SGE_LIST(ID_ja_structure, RN_Type, CULL_DEFAULT)
   SGE_ULONG(ID_action, CULL_DEFAULT)
   SGE_ULONG(ID_force, CULL_DEFAULT)
   SGE_LIST(ID_user_list, ST_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(IDN)
   NAME("ID_str")
   NAME("ID_ja_structure")
   NAME("ID_action")
   NAME("ID_force")
   NAME("ID_user_list")
NAMEEND

/* *INDENT-ON* */ 

#define IDS sizeof(IDN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif
