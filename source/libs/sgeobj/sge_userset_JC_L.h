#ifndef __SGE_USERSET_JC_L_H
#define __SGE_USERSET_JC_L_H

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
 * this list is used by schedd to keep the number 
 * of running jobs per user/group efficiently
 */
enum {
   JC_name = JC_LOWERBOUND,  /* user or group name */
   JC_jobs                   /* number of running jobs */
};

LISTDEF(JC_Type)
   SGE_STRING(JC_name, CULL_HASH | CULL_UNIQUE)
   SGE_ULONG(JC_jobs, CULL_DEFAULT)
LISTEND 

NAMEDEF(JCN)
   NAME("JC_name")
   NAME("JC_jobs")
NAMEEND

/* *INDENT-ON* */

#define JCS sizeof(JCN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_USERSETL_H */
