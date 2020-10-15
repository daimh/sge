#ifndef __SGE_GROUPS_H__
#define __SGE_GROUPS_H__
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

#include "sgeobj/sge_hgroup_HGRP_L.h"

bool hgroup_check_name(lList **answer_list, const char* name);

/* --- */

lListElem *
hgroup_create(lList **answer_list, const char *name, 
              lList *hostref_or_groupref, bool is_name_validate);

bool 
hgroup_add_references(lListElem *this_elem, lList **answer_list,
                      const lList *hostref_or_groupref);

bool 
hgroup_find_references(const lListElem *this_elem, lList **answer_list,
                       const lList *master_list, lList **used_hosts,
                       lList **used_groups);

bool 
hgroup_find_all_references(const lListElem *this_elem, lList **answer_list,
                           const lList *master_list, lList **used_hosts,
                           lList **used_groups);

bool 
hgroup_find_all_referencees(const lListElem *this_elem, 
                            lList **answer_list,
                            const lList *master_list, lList **used_groups);

bool
hgroup_find_referencees(const lListElem *this_elem,
                        lList **answer_list,
                        const lList *master_hgroup_list,
                        const lList *master_cqueue_list,
                        lList **occupants_groups,
                        lList **occupants_queues);


/* --- */

lListElem *
hgroup_list_locate(const lList *this_list, const char *group);

lList **
hgroup_list_get_master_list(void);

bool
hgroup_list_exists(const lList *this_list, lList **answer_list,
                   const lList *href_list);

bool
hgroup_list_find_matching_and_resolve(const lList *this_list,
                                      const char *hgroup_pattern,
                                      lList **used_hosts);

bool
hgroup_list_find_matching(const lList *this_list,
                          const char *hgroup_pattern, lList **used_hosts);

#endif /* __SGE_GROUPS_H__ */


