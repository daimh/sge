#ifndef __SGE_ACCESS_TREE_JRL_L_H
#define __SGE_ACCESS_TREE_JRL_L_H

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
 * access tree object jobreference list 
 */
enum {
   JRL_jobid = JRL_LOWERBOUND,
   JRL_submission_time,
   JRL_category
};

LISTDEF(JRL_Type)
   SGE_ULONG(JRL_jobid, CULL_DEFAULT)       /* jobid used for hasing */
   SGE_ULONG(JRL_submission_time, CULL_DEFAULT)     /* submission time of this job array */
   SGE_REF(JRL_category, CULL_ANY_SUBTYPE, CULL_DEFAULT)      /* category reference */
LISTEND 

NAMEDEF(JRLN)
   NAME("JRL_jobid")
   NAME("JRL_submission_time")
   NAME("JRL_category")
NAMEEND

#define JRLS sizeof(JRLN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_SORTL_H */
