#ifndef __SGE_JOB_REFL_H
#define __SGE_JOB_REFL_H

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

enum {
   JRE_job_number = JRE_LOWERBOUND,  /* used in JB_jid_successor_list */
   JRE_job_name                      /* used in JB_jid_predecessor_list */
};

LISTDEF(JRE_Type)
   JGDI_OBJ(JobReference)
   SGE_ULONG(JRE_job_number, CULL_PRIMARY_KEY | CULL_DEFAULT | CULL_SUBLIST)
   SGE_STRING(JRE_job_name, CULL_DEFAULT | CULL_SUBLIST)
LISTEND 

NAMEDEF(JREN)
   NAME("JRE_job_number")
   NAME("JRE_job_name")
NAMEEND

/* *INDENT-ON* */

#define JRES sizeof(JREN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_JOB_REFL_H */
