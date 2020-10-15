#ifndef __SGE_QREF_H__
#define __SGE_QREF_H__
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

#include "sgeobj/sge_qref_QR_L.h"

bool
qref_list_add(lList **this_list, lList **answer_list, 
              const char *qref_string);


bool
qref_list_resolve(const lList *src_qref_list, lList **answer_list, 
                  lList **qref_list, bool *found_something,
                  const lList *cqueue_list, const lList *hgroup_list, 
                  bool resolve_cqueue, bool resolve_qdomain);

bool
qref_list_trash_some_elemts(lList **this_list, const char *full_name);

bool
qref_list_is_valid(const lList *this_list, lList **answer_list);

void
qref_resolve_hostname(lListElem *this_elem);

void
qref_list_resolve_hostname(lList *this_list); 

bool
qref_cq_rejected(const char *qref_pattern, const char *cqname,
      const char *hostname, const lList *hgroup_list);
bool
qref_list_cq_rejected(const lList *qref_list, const char *cqname, const char *hostname, const lList *hgroup_list);

bool
qref_list_eh_rejected(const lList *qref_list, const char *hostname, const lList *hgroup_list);

bool
qref_list_host_rejected(const char *href, const char *hostname, 
                                 const lList *hgroup_list);

int 
cull_parse_destination_identifier_list(lList **lpp, const char *dest_str);

#endif /* __SGE_QREF_H__ */


