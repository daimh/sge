#ifndef __SGE_ACCESS_TREE_USR_L_H
#define __SGE_ACCESS_TREE_USR_L_H

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
 * access tree object user reference list 
 */
enum {
   USR_name = USR_LOWERBOUND,
   USR_nrunning_el,
   USR_nrunning_dl,
   USR_job_references,
   USR_sort_me,
   USR_current
};

LISTDEF(USR_Type)
   SGE_STRING(USR_name, CULL_DEFAULT)       /* user name */
   SGE_ULONG(USR_nrunning_el, CULL_DEFAULT) /* # of running jobs in event layer */
   SGE_ULONG(USR_nrunning_dl, CULL_DEFAULT) /* # of running jobs in dispatch layer */
   SGE_LIST(USR_job_references, JRL_Type, CULL_DEFAULT)       /* sublist of jobsreferences for that 
                                       * user */
   SGE_ULONG(USR_sort_me, CULL_DEFAULT)     /* job reference list must be resorted */
   SGE_REF(USR_current, JRL_Type, CULL_DEFAULT)       /* reference to the current entry in the *
                               * jobreferences list */
LISTEND 

NAMEDEF(USRN)
   NAME("USR_name")
   NAME("USR_nrunning_el")
   NAME("USR_nrunning_dl")
   NAME("USR_job_references")
   NAME("USR_sort_me")
   NAME("USR_current")
NAMEEND

#define USRS sizeof(USRN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_SORTL_H */
