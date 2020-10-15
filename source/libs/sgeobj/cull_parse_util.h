#ifndef __CULL_PARSE_UTIL_H
#define __CULL_PARSE_UTIL_H
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

#define L_IS_NUM_TYPE(x) ((x == lFloatT) || (x == lDoubleT) || (x == lUlongT) || \
                          (x == lLongT)  || (x == lIntT))

/*
** flags for uni_print_list
*/
#define FLG_NO_DELIS_STRINGS 1
#define FLG_NO_DELIS_NUMBERS 2

/*
** flags for parse_list_simple
*/
#define FLG_LIST_APPEND   1
#define FLG_LIST_REPLACE  2
#define FLG_LIST_MERGE    4
#define FLG_LIST_MERGE_DOUBLE_KEY    8

int parse_list_simple(lList *cmdline, char *option, lListElem *job, int field, int nm_var, int nm_value, u_long32 flags);

int parse_list_hardsoft(lList *cmdline, char *option, lListElem *job, int hard_field, int soft_field);

int cull_parse_definition_list(char *str, lList **lpp, const char *name, lDescr *descr, int *interpretation_rule);

int cull_merge_definition_list(lList **lpp_old, lList *lp_new, int nm_var, int nm_value);

int cull_compress_definition_list(lList *lp_new, int nm_var, int nm_value, int double_keys);

int cull_parse_simple_list(char *str, lList **lpp, char *name, lDescr *descr, int *interpretation_rule);

int cull_parse_string_list(char **pstrlist, const char *listname, lDescr *descr, int *interpretation_rule, lList **pplist);

int uni_print_list(FILE *fp, char *buff, u_long32 max_len, const lList *lp, int *which_elements_rule, const char *pdelis[], unsigned long flags);

int fprint_cull_list(FILE *fp, char *str, lList *lp, int fi); 

int fprint_thresholds(FILE *fp, char *str, lList *thresholds, int print_slots); 

int fprint_resource_utilizations(FILE *fp, char *str, lList *thresholds, int print_slots); 

int cull_parse_path_list(lList **lpp, const char *path_str);

int cull_parse_jid_hold_list(lList **lpp, const char *str);

int sge_parse_hold_list(const char *hold_str, u_long32 prog_number);

#endif /* __CULL_PARSE_UTIL_H */
