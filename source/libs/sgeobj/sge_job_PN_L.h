#ifndef __SGE_JOB_PN_L_H
#define __SGE_JOB_PN_L_H
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

/* -------- path name list ----------------- */
/* [host:]path[,[host:]path...]              */
   
enum {
   PN_path = PN_LOWERBOUND,
   PN_host,
   PN_file_host,
   PN_file_staging
};

LISTDEF(PN_Type)
   JGDI_OBJ(PathName)
   SGE_STRING(PN_path, CULL_PRIMARY_KEY | CULL_DEFAULT | CULL_SUBLIST)
   SGE_HOST(PN_host, CULL_DEFAULT)                   
   SGE_HOST(PN_file_host, CULL_DEFAULT)
   SGE_BOOL(PN_file_staging, CULL_DEFAULT)
LISTEND

NAMEDEF(PNN)
   NAME("PN_path")
   NAME("PN_host")
   NAME("PN_file_host")
   NAME("PN_file_staging")
NAMEEND

#define PNS sizeof(PNN)/sizeof(char*)

/* *INDENT-ON* */

#ifdef  __cplusplus
}
#endif

#endif /* __SGE_JOBL_H */
