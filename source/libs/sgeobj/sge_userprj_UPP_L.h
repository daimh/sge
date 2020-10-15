#ifndef __SGE_USERPRJ_UPP_L_H
#define __SGE_USERPRJ_UPP_L_H

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

/* 
 *
 * This is the project usage list type we use to hold the usage for
 * a user on a project basis. Each entry contains a project name and
 * a usage list.
 */
enum {
   UPP_name = UPP_LOWERBOUND,
   UPP_usage,
   UPP_long_term_usage
};

enum {
   UPP_name_POS = 0,
   UPP_usage_POS,
   UPP_long_term_usage_POS
};


LISTDEF(UPP_Type)
   JGDI_OBJ(ProjectUsage)
   SGE_STRING(UPP_name, CULL_PRIMARY_KEY | CULL_HASH | CULL_UNIQUE | CULL_SUBLIST)    /* project name */
   SGE_MAP(UPP_usage, UA_Type, CULL_DEFAULT | CULL_SUBLIST)     /* 
                            * UA_Type; decayed usage set and used by
                            * schedd stored to qmaster; spooled
                            */
   SGE_MAP(UPP_long_term_usage, UA_Type, CULL_DEFAULT | CULL_SUBLIST) /* UA_Type; long term accumulated 
                                  * non-decayed usage; set by
                                  * schedd stored to qmaster; spooled */
LISTEND 

NAMEDEF(UPPN)
   NAME("UPP_name")
   NAME("UPP_usage")
   NAME("UPP_long_term_usage")
NAMEEND

/* *INDENT-ON* */ 

#define UPPS sizeof(UPPN)/sizeof(char*)

#ifdef  __cplusplus
}
#endif
#endif                          
