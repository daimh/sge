#ifndef __SGE_HREF_H__
#define __SGE_HREF_H__
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

#include "sgeobj/sge_href_HR_L.h"

bool 
href_list_add(lList **this_list, lList **answer_list,
              const char *host_or_group);

bool
href_list_remove_existing(lList **this_list, lList **answer_list,
                          lList *list);

bool 
href_list_has_member(const lList *this_list, const char *host_or_group);

bool 
href_list_append_to_dstring(const lList *this_list, dstring *string);

bool 
href_list_compare(const lList *this_list, lList **answer_list,
                  const lList *list, lList **add_hosts,
                  lList **add_groups, lList **equity_hosts,
                  lList **equity_groups);

bool 
href_list_find_diff(const lList *this_list, lList **answer_list,
                    const lList *list, lList **add_hosts,
                    lList **rem_hosts, lList **add_groups,
                    lList **rem_groups);

bool
href_list_find_effective_diff(lList **answer_list, const lList *add_groups, 
                              const lList *rem_groups, const lList *master_list,
                              lList **add_hosts, lList **rem_hosts);

bool 
href_list_find_references(const lList *this_list, lList **answer_list,
                          const lList *master_list, lList **used_hosts,
                          lList **used_groups);

bool 
href_list_find_all_references(const lList *this_list, lList **answer_list,
                              const lList *master_list, lList **used_hosts,
                              lList **used_groups);

bool 
href_list_find_referencees(const lList *this_list, lList **answer_list,
                           const lList *master_list, lList **used_groups);

bool 
href_list_find_all_referencees(const lList *this_list, lList **answer_list,
                               const lList *master_list, 
                               lList **occupant_groups);

lListElem *
href_list_locate(const lList *this_list, const char *name);

bool 
href_list_resolve_hostnames(lList *this_list, lList **answer_list,
                            bool ignore_error);

void
href_list_debug_print(const lList *this_list, const char *prefix);

void
href_list_make_uniq(lList *this_list, lList **answer_list);

#endif /* __SGE_HREF_H__ */


