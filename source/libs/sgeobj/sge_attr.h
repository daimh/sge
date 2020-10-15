#ifndef __SGE_ATTR_H__
#define __SGE_ATTR_H__
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

#include "sgeobj/sge_attr_ASTR_L.h"
#include "sgeobj/sge_attr_AULNG_L.h"
#include "sgeobj/sge_attr_ABOOL_L.h"
#include "sgeobj/sge_attr_ATIME_L.h"
#include "sgeobj/sge_attr_AMEM_L.h"
#include "sgeobj/sge_attr_AINTER_L.h"
#include "sgeobj/sge_attr_ASTRING_L.h"
#include "sgeobj/sge_attr_ASTRLIST_L.h"
#include "sgeobj/sge_attr_AUSRLIST_L.h"
#include "sgeobj/sge_attr_APRJLIST_L.h"
#include "sgeobj/sge_attr_ACELIST_L.h"
#include "sgeobj/sge_attr_ASOLIST_L.h"
#include "sgeobj/sge_attr_AQTLIST_L.h"

#define HOSTATTR_DEFAULT            0x0000
#define HOSTATTR_ALLOW_AMBIGUITY    0x0001
#define HOSTATTR_OVERWRITE          0x0002

/*
 * Internally this name should be handled as hostgroup name. Therefore it 
 * begins with a @ sign. The name does not cause a conflict with
 * user defined hostgroup names because in these names slashes are not
 * allowed.
 */ 
#define HOSTREF_DEFAULT             "@/"

#define TEMPLATE_ATTR_PROTO(PREFIX, TYPE, INTERNAL_TYPE)                      \
                                                                              \
lListElem *                                                                   \
PREFIX##_create(lList **answer_list, const char *href, TYPE value);           \
                                                                              \
bool                                                                          \
PREFIX##_list_add(lList **this_list, lList **answer_list,                     \
                  lListElem **attr, int flags, lList **href_list);            \
bool                                                                          \
PREFIX##_list_add_set_del(lList **this_list, lList **answer_list,             \
                  const char *hostname, void *value, bool remove);            \
lListElem *                                                                   \
PREFIX##_list_find(const lList *this_list, const char *href);                 \
                                                                              \
bool                                                                          \
PREFIX##_list_find_value(const lList *this_list, lList **answer_list,         \
                         const char *hostname, INTERNAL_TYPE *value,          \
                         const char **matching_host_or_group,                 \
                         const char **matching_group,                         \
                         bool *is_ambiguous);                                 \
                                                                              \
bool                                                                          \
PREFIX##_list_find_value_href(const lList *this_list, lList **answer_list,    \
                         const char *hostname, INTERNAL_TYPE *value,          \
                         bool *found);                                        \
bool                                                                          \
PREFIX##_list_append_to_dstring(const lList *this_list, dstring *string);     \
                                                                              \
bool                                                                          \
PREFIX##_list_parse_from_string(lList **this_list, lList **answer_list,       \
                                const char *string, int flags);               \
bool                                                                          \
PREFIX##_has_hgroup_reference(const lList *this_list,                         \
                              const char *host_or_group);                     \
                                                                              \
lListElem *                                                                   \
PREFIX##_list_locate(const lList *this_list, const char *host_or_group);
                                                                              
TEMPLATE_ATTR_PROTO(str_attr, const char *, const char *)

TEMPLATE_ATTR_PROTO(ulng_attr, u_long32, u_long32)                    

TEMPLATE_ATTR_PROTO(bool_attr, bool, bool)                   

TEMPLATE_ATTR_PROTO(time_attr, const char *, const char *)

TEMPLATE_ATTR_PROTO(mem_attr, const char *, const char *)

TEMPLATE_ATTR_PROTO(inter_attr, const char *, const char *)

TEMPLATE_ATTR_PROTO(qtlist_attr, u_long32, u_long32)


TEMPLATE_ATTR_PROTO(strlist_attr, const char *, lList *)

TEMPLATE_ATTR_PROTO(usrlist_attr, const char *, lList *)

TEMPLATE_ATTR_PROTO(prjlist_attr, const char *, lList *)                    

TEMPLATE_ATTR_PROTO(celist_attr, const char *, lList *)                    

TEMPLATE_ATTR_PROTO(solist_attr, const char *, lList *)                    

bool
attr_list_append_to_dstring(const lList *this_list, dstring *string,
                            int href_nm, int value_nm);

#undef ATTR_TYPE_PROTOTYPES 

#endif /* __SGE_ATTR_H__ */


