#ifndef __PACK_PACK_L_H
#define __PACK_PACK_L_H
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
   PACK_id = PACK_LOWERBOUND,
#if 1 /* TODO EB: should be removed */
   PACK_compressed,
#endif
   PACK_string
};

LISTDEF( PACK_Type )
   SGE_ULONG(PACK_id, CULL_DEFAULT) /* id = what or where date structure */
#if 1 /* TODO EB: should be removed, not used anymore */
   SGE_BOOL(PACK_compressed, CULL_DEFAULT)   /* is the date compressed */
#endif
   SGE_STRING(PACK_string, CULL_DEFAULT)  /* the raw data in ascii */
LISTEND

NAMEDEF( PACKN )
   NAME   ( "PACK_id") 
#if 1 /* TODO EB: should be removed */
   NAME   ( "PACK_compressed")
#endif
   NAME   ( "PACK_string")
NAMEEND

/* *INDENT-ON* */ 

#define PACKS sizeof(PACKN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif /* __PACKL_H */
