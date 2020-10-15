#ifndef __SGE_SUSER_SU_L_H
#define __SGE_SUSER_SU_L_H

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

/****** sgeobj/suser/SU_Type **************************************************
*  NAME
*     SU_Type - CULL submit users element
*
*  SYNOPSIS
*     SU_Type
*     +--- SU_name: username of a user submittet a job
*     +--- SU_jobs: currently active jobs for this user
*                   (Jobs in system which are in the 'finished'
*                    state are also accounted here)
*
*  FUNCTION
*     CULL element which holds information for a user which is able to
*     submit jobs into an SGE system. 
*     The variable 'Master_SUser_List' is used to hold multiple
*     'SU_Type' elements within the master deamon. Use the functions
*     mentioned in the 'see also' section to access, modify, delete
*     'SU_Type' elements and the 'Master_SUser_List'. 
*
*  SEE ALSO
*     gdi/suser/SU_Type
*     gdi/suser/Master_SUser_List
*     gdi/suser/suser_list_add()
*     gdi/suser/suser_list_find()
*     gdi/suser/suser_increase_job_counter()
*     gdi/suser/suser_decrease_job_counter()
*     gdi/suser/suser_get_job_counter()
*     gdi/suser/suser_register_new_job()
*     gdi/suser/suser_unregister_job()
******************************************************************************/

/* *INDENT-OFF* */

enum {
   SU_name = SU_LOWERBOUND,
   SU_jobs
};

LISTDEF(SU_Type)
   SGE_STRING(SU_name, CULL_HASH | CULL_UNIQUE)
   SGE_ULONG(SU_jobs, CULL_DEFAULT)
LISTEND 

NAMEDEF(SUN)
   NAME("SU_name")
   NAME("SU_jobs")
NAMEEND

/* *INDENT-ON* */ 

#define SUS sizeof(SUN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif

#endif
