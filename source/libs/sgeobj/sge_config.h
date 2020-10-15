#ifndef _SGE_CONFIG_H_
#define _SGE_CONFIG_H_
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
#include "sgeobj/cull_parse_util.h"

enum {
   MODE_RELATIVE,
   MODE_SET,
   MODE_ADD,
   MODE_SUB
};

int add_nm_to_set(int fields[], int name_nm);

enum {
   RCL_NO_VALUE = 0x0001
};

int read_config_list(FILE *fp, lList **clpp, lList **alpp, lDescr *dp, int nm1, int nm2, int nm3, char *delimitor, int flag, char *buf, int size);

char *get_conf_value(lList **alpp, lList *lp, int name_nm, int value_nm, const char *key);

lList *get_conf_sublist(lList **alpp, lList *lp, int name_nm, int value_nm, const char *key);

bool set_conf_string(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm);

bool set_conf_bool(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm);

bool set_conf_ulong(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm);

bool set_conf_double(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, int operation_nm);

bool set_conf_deflist(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, lDescr *descr, int *interpretation_rule);

bool set_conf_timestr(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm);

bool set_conf_memstr(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm);

bool set_conf_enum(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, const char **enum_strings);

bool set_conf_enum_none(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, const char **enum_strings);

bool set_conf_list(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, lDescr *descr, int sub_name_nm);

bool set_conf_subordlist(lList **alpp, lList **clpp, int fields[], const char *key, lListElem *ep, int name_nm, lDescr *descr, int subname_nm, int subval_nm);

#if unused
bool set_conf_str_attr_list(lList **alpp, lList **clpp, int fields[],
                            const char *key, lListElem *ep, int name_nm,
                            lDescr *descr, int sub_name_nm);

bool set_conf_strlist_attr_list(lList **alpp, lList **clpp, int fields[],
                                const char *key, lListElem *ep, int name_nm,
                                lDescr *descr, int sub_name_nm);

bool set_conf_usrlist_attr_list(lList **alpp, lList **clpp, int fields[],
                                const char *key, lListElem *ep, int name_nm,
                                lDescr *descr, int sub_name_nm);

bool set_conf_prjlist_attr_list(lList **alpp, lList **clpp, int fields[],
                                const char *key, lListElem *ep, int name_nm,
                                lDescr *descr, int sub_name_nm);

bool set_conf_celist_attr_list(lList **alpp, lList **clpp, int fields[],
                               const char *key, lListElem *ep, int name_nm,
                               lDescr *descr, int sub_name_nm);

bool set_conf_solist_attr_list(lList **alpp, lList **clpp, int fields[],
                               const char *key, lListElem *ep, int name_nm,
                               lDescr *descr, int sub_name_nm);

bool set_conf_qtlist_attr_list(lList **alpp, lList **clpp, int fields[],
                               const char *key, lListElem *ep, int name_nm,
                               lDescr *descr, int sub_name_nm);

bool set_conf_ulng_attr_list(lList **alpp, lList **clpp, int fields[],
                             const char *key, lListElem *ep, int name_nm,
                             lDescr *descr, int sub_name_nm);

bool set_conf_bool_attr_list(lList **alpp, lList **clpp, int fields[],
                             const char *key, lListElem *ep, int name_nm,
                             lDescr *descr, int sub_name_nm);

bool set_conf_time_attr_list(lList **alpp, lList **clpp, int fields[],
                             const char *key, lListElem *ep, int name_nm,
                             lDescr *descr, int sub_name_nm);

bool set_conf_mem_attr_list(lList **alpp, lList **clpp, int fields[],
                             const char *key, lListElem *ep, int name_nm,
                             lDescr *descr, int sub_name_nm);

bool set_conf_inter_attr_list(lList **alpp, lList **clpp, int fields[],
                              const char *key, lListElem *ep, int name_nm,
                              lDescr *descr, int sub_name_nm);
#endif

bool set_conf_centry_type(lList **alpp, lList **clpp, int fields[], 
                          const char *key, lListElem *ep, int name_nm);

bool set_conf_centry_relop(lList **alpp, lList **clpp, int fields[], 
                           const char *key, lListElem *ep, int name_nm);

bool set_conf_centry_requestable(lList **alpp, lList **clpp, int fields[], 
                           const char *key, lListElem *ep, int name_nm);

#endif /* _SGE_CONFIG_H_ */
