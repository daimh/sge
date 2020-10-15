#ifndef __SGE_ADVANCE_RESERVATIONL_ARA_H
#define __SGE_ADVANCE_RESERVATIONL_ARA_H

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

/* Advance Reservation ACL Entry Object */
enum {
   ARA_name = ARA_LOWERBOUND,   /* user or userset */
   ARA_group                    /* group name for user */
};

LISTDEF(ARA_Type)
   SGE_STRING(ARA_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SPOOL)
   SGE_STRING(ARA_group, CULL_DEFAULT | CULL_SPOOL)
LISTEND

NAMEDEF(ARAN)
   NAME("ARA_name")
   NAME("ARA_group")
NAMEEND

#define ARAS sizeof(ARAN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif /* __SGE_ADVANCE_RESERVATIONL_H */
