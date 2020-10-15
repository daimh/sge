#ifndef __SGE_REPORT_REP_L_H
#define __SGE_REPORT_REP_L_H

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
 * definition for REP_Type, sge report type
 */
enum {
   REP_type = REP_LOWERBOUND,
   REP_host,
   REP_list,
   REP_version,
   REP_seqno
};

LISTDEF(REP_Type)
   SGE_ULONG(REP_type, CULL_DEFAULT)        /* type of report, e.g. load report */
   SGE_HOST(REP_host, CULL_DEFAULT)       /* hostname as it is seen by sender of report */
   SGE_LIST(REP_list, CULL_ANY_SUBTYPE, CULL_DEFAULT)         /* list type depends on REP_type */
   SGE_ULONG(REP_version, CULL_DEFAULT)     /* used to report software version of execd */
   SGE_ULONG(REP_seqno, CULL_DEFAULT)       /* used to recognize old reports sent by execd */
LISTEND 

NAMEDEF(REPN)
   NAME("REP_type")
   NAME("REP_host")
   NAME("REP_list")
   NAME("REP_version")
   NAME("REP_seqno")
NAMEEND

#define REPS sizeof(REPN)/sizeof(char*)

/* *INDENT-ON* */ 

#ifdef __cplusplus
}
#endif

#endif                          /* __SGE_REPORTL_H */
