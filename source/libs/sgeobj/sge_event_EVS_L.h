#ifndef __SGE_EVENT_EVS_L_H
#define __SGE_EVENT_EVS_L_H

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

#include "uti/sge_monitor.h"
#include "cull/cull.h"
#include "sgeobj/sge_boundaries.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* *INDENT-OFF* */

enum {
   EVS_id = EVS_LOWERBOUND,
   EVS_flush,
   EVS_interval,
   EVS_what,
   EVS_where
};

LISTDEF(EVS_Type)
   JGDI_OBJ(EventSubcribtion)
   SGE_ULONG(EVS_id, CULL_DEFAULT)
   SGE_BOOL(EVS_flush, CULL_DEFAULT)
   SGE_ULONG(EVS_interval, CULL_DEFAULT)
   SGE_OBJECT(EVS_what, CULL_ANY_SUBTYPE, CULL_DEFAULT)
   SGE_OBJECT(EVS_where, CULL_ANY_SUBTYPE, CULL_DEFAULT)
LISTEND

NAMEDEF(EVSN)
   NAME("EVS_id") 
   NAME("EVS_flush")
   NAME("EVS_interval")
   NAME("EVS_what")
   NAME("EVS_where")
NAMEEND   

#define EVSS sizeof(EVSN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_EVENTL_H */
