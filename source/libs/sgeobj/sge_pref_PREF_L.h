#ifndef __QMON_PREFL_H
#define __QMON_PREFL_H

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
 * this data structures describes a qmon preferences element
 */
enum {
   PREF_job_filter_resources = PREF_LOWERBOUND,
   PREF_job_filter_owners,
   PREF_job_filter_fields,
   PREF_job_filter_compact,
   PREF_queue_filter_resources,
   PREF_queue_filter_pe,
   PREF_queue_filter_user,
   PREF_queue_filter_q,
   PREF_queue_filter_state,
   PREF_ar_filter_fields
};

LISTDEF(PREF_Type)
   /* ------ internal fields ----------------------------------- */
   SGE_LIST(PREF_job_filter_resources, CE_Type, CULL_DEFAULT)
   SGE_LIST(PREF_job_filter_owners, ST_Type, CULL_DEFAULT)
   SGE_LIST(PREF_job_filter_fields, ST_Type, CULL_DEFAULT)
   SGE_BOOL(PREF_job_filter_compact, CULL_DEFAULT)
   SGE_LIST(PREF_queue_filter_resources, CE_Type, CULL_DEFAULT)
   SGE_LIST(PREF_queue_filter_pe, ST_Type, CULL_DEFAULT)
   SGE_LIST(PREF_queue_filter_user, ST_Type, CULL_DEFAULT)
   SGE_LIST(PREF_queue_filter_q, ST_Type, CULL_DEFAULT)
   SGE_STRING(PREF_queue_filter_state, CULL_DEFAULT)
   SGE_LIST(PREF_ar_filter_fields, ST_Type, CULL_DEFAULT)
LISTEND 

NAMEDEF(PREFS)
   /* ------ internal fields ----------------------------------- */
   NAME("PREF_job_filter_resources")
   NAME("PREF_job_filter_owners")
   NAME("PREF_job_filter_fields")
   NAME("PREF_job_filter_compact")
   NAME("PREF_queue_filter_resources")
   NAME("PREF_queue_filter_pe")
   NAME("PREF_queue_filter_user")
   NAME("PREF_queue_filter_q")
   NAME("PREF_queue_filter_state")
   NAME("PREF_ar_filter_fields")
NAMEEND

/* *INDENT-ON* */

#define PREFN sizeof(PREFS)/sizeof(char*)
#ifdef  __cplusplus
}
#endif
#endif                          /* __QMON_PREFL_H */
