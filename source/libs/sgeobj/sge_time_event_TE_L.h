#ifndef __SGE_TIME_EVENT_TE_L_H
#define __SGE_TIME_EVENT_TE_L_H

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

/* *INDENT-OFF* */  

enum {
   TE_when = TE_LOWERBOUND,  /* time when this event must be delivered       */
   TE_type,                  /* to differ between different event categories */
   TE_mode,                  /* one-time or recurring event                  */
   TE_interval,              /* event interval, if recurring event           */
   TE_uval0,                 /* 1st ulong key                                */
   TE_uval1,                 /* 2nd ulong key                                */
   TE_sval,                  /* str key                                      */
   TE_seqno
};

LISTDEF(TE_Type)
   SGE_ULONG(TE_when,     CULL_DEFAULT)
   SGE_ULONG(TE_type,     CULL_DEFAULT)
   SGE_ULONG(TE_mode,     CULL_DEFAULT)
   SGE_ULONG(TE_interval, CULL_DEFAULT)
   SGE_ULONG(TE_uval0,    CULL_DEFAULT)
   SGE_ULONG(TE_uval1,    CULL_DEFAULT)
   SGE_STRING(TE_sval,    CULL_DEFAULT)
   SGE_ULONG(TE_seqno,    CULL_DEFAULT)
LISTEND 

NAMEDEF(TEN)
   NAME("TE_when")
   NAME("TE_type")
   NAME("TE_mode")
   NAME("TE_interval")
   NAME("TE_uval0")
   NAME("TE_uval1")
   NAME("TE_sval")
   NAME("TE_seqno")
NAMEEND

/* *INDENT-ON* */

#define TES sizeof(TEN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_TIME_EVENTL_H */
