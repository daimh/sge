#ifndef __SGE_HOST_H
#define __SGE_HOST_H
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
 * Portions of this code are Copyright 2011 Univa Inc.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sgeobj/sge_host_EH_L.h"
#include "sgeobj/sge_host_RU_L.h"
#include "sgeobj/sge_host_AH_L.h"
#include "sgeobj/sge_host_SH_L.h"
#include "sgeobj/sge_host_HL_L.h"
#include "sgeobj/sge_host_HS_L.h"

/* 
 * sge standard load value names
 *
 * use these defined names for refering
 */

/* static load parameters */
#define LOAD_ATTR_ARCH           "arch"
#define LOAD_ATTR_NUM_PROC       "num_proc"

/* raw load parameters */
#define LOAD_ATTR_LOAD_SHORT     "load_short"
#define LOAD_ATTR_LOAD_MEDIUM    "load_medium"
#define LOAD_ATTR_LOAD_LONG      "load_long"
#define LOAD_ATTR_LOAD_AVG       "load_avg"

/* values divided by LOAD_ATTR_NUM_PROC */
#define LOAD_ATTR_NP_LOAD_SHORT  "np_load_short"
#define LOAD_ATTR_NP_LOAD_MEDIUM "np_load_medium"
#define LOAD_ATTR_NP_LOAD_LONG   "np_load_long"
#define LOAD_ATTR_NP_LOAD_AVG    "np_load_avg"
#define LOAD_ATTR_MEM_FREE       "mem_free"
#define LOAD_ATTR_SWAP_FREE      "swap_free"
#define LOAD_ATTR_VIRTUAL_FREE   "virtual_free"
#define LOAD_ATTR_MEM_TOTAL      "mem_total"
#define LOAD_ATTR_SWAP_TOTAL     "swap_total"
#define LOAD_ATTR_VIRTUAL_TOTAL  "virtual_total"
#define LOAD_ATTR_MEM_USED       "mem_used"
#define LOAD_ATTR_SWAP_USED      "swap_used"
#define LOAD_ATTR_VIRTUAL_USED   "virtual_used"
#define LOAD_ATTR_SWAP_RSVD      "swap_rsvd"

/* values for job to core binding */
#define LOAD_ATTR_TOPOLOGY       "m_topology"
#define LOAD_ATTR_SOCKETS        "m_socket"
#define LOAD_ATTR_CORES          "m_core"
#define LOAD_ATTR_THREADS        "m_thread"
#define LOAD_ATTR_TOPOLOGY_INUSE "m_topology_inuse"

bool host_is_referenced(const lListElem *host, lList **answer_list,
                        const lList *queue_list, const lList *hgrp_list);

const char *host_get_load_value(lListElem *host, const char *name);

int sge_resolve_host(lListElem *ep, int nm);

int sge_resolve_hostname(const char *hostname, char *unique, int nm, size_t lunique);

bool
host_is_centry_referenced(const lListElem *this_elem, const lListElem *centry);

bool
host_is_centry_a_complex_value(const lListElem *this_elem, 
                               const lListElem *centry);

lListElem *
host_list_locate(const lList *this_list, const char *hostname);

bool
host_list_merge(lList *this_list);

bool 
host_merge(lListElem *host, const lListElem *global_host);

#endif /* __SGE_HOST_H */

