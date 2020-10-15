#ifndef __SGE_QEXEC_RT_L_H__
#define __SGE_QEXEC_RT_L_H__

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

/* 
 * this data structure describes a remote task
 */
enum {
   RT_tid = RT_LOWERBOUND,       /* task id */
   RT_hostname,                  /* remote host where task runs on */
   RT_status,                    /* status as it comes from waitpid(2) */
   RT_state                      /* for internally use of qrexec module */
};

LISTDEF(RT_Type)
   SGE_STRING(RT_tid, CULL_DEFAULT)
   SGE_HOST(RT_hostname, CULL_DEFAULT)
   SGE_ULONG(RT_status, CULL_DEFAULT)
   SGE_ULONG(RT_state, CULL_DEFAULT)
LISTEND 

NAMEDEF(RTN)
   NAME("RT_tid")
   NAME("RT_hostname")
   NAME("RT_status")
   NAME("RT_state")
NAMEEND

/* *INDENT-ON* */

#define RTS sizeof(RTN)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __SGE_QEXECL_H__ */
