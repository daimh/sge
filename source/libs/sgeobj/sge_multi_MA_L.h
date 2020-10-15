#ifndef __SGE_MULTIL_H
#define __SGE_MULTIL_H

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

#include "sgeobj/sge_boundaries.h"
#include "cull/cull.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-ON* */

enum {
   MA_id = MA_LOWERBOUND,
   MA_objects,
   MA_answers
};

LISTDEF(MA_Type)
   SGE_ULONG(MA_id, CULL_DEFAULT)
   SGE_LIST(MA_objects, CULL_ANY_SUBTYPE, CULL_DEFAULT)       /* list which is returned in get requests, *
                               * any type */
   SGE_LIST(MA_answers, AN_Type, CULL_DEFAULT)       /* AN_Type */
LISTEND 

NAMEDEF(MAN)
   NAME("MA_id")
   NAME("MA_objects")
   NAME("MA_answers")
NAMEEND

/* *INDENT-OFF* */     

#define MAS sizeof(MAN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_MULTIL_H */
