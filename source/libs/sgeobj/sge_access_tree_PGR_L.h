#ifndef __SGE_ACCESS_TREE_PGR_L_H
#define __SGE_ACCESS_TREE_PGR_L_H

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

/* *INDENT-OFF* */

/* 
 * access tree object priority group 
 */

enum {
   PGR_priority = PGR_LOWERBOUND,
   PGR_subordinated_list,
   PGR_sort_me,
   PGR_current
};

LISTDEF(PGR_Type)
   SGE_ULONG(PGR_priority, CULL_DEFAULT)    /* priority of the priority group */
   SGE_LIST(PGR_subordinated_list, USR_Type, CULL_DEFAULT)
      /* FCFS: jobreference list sorted by arrival time */
      /* USERSORT: list of jobreference lists grouped by users */
   SGE_ULONG(PGR_sort_me, CULL_DEFAULT)
      /* FCFS: subordinated jobreference list needs to be resorted */
      /* USERSORT: subordinated user list needs to be resorted */
   SGE_REF(PGR_current, JRL_Type, CULL_DEFAULT)
      /* FCFS: reference to the current entry in the jobreferences list */
      /* USERSORT: unused */
LISTEND 

NAMEDEF(PGRN)
   NAME("PGR_priority")
   NAME("PGR_subordinated_list")
   NAME("PGR_sort_me")
   NAME("PGR_current")
NAMEEND

#define PGRS sizeof(PGRN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_SORTL_H */
