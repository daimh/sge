#ifndef __SGE_SUPPORT_H
#define __SGE_SUPPORT_H
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

typedef int (*sge_node_func_t) (lListElem *node, void *ptr);

void decay_userprj_usage(lListElem *userprj, bool user, const lList *decay_list, u_long seqno, u_long curr_time);

void calculate_default_decay_constant(int halftime);

void calculate_decay_constant(double halftime, double *decay_rate, double *decay_constant);

int sge_for_each_share_tree_node(lListElem *node, sge_node_func_t func, void *ptr);

int sge_zero_node_fields(lListElem *node, void *ptr);

int sge_init_node_fields(lListElem *root);

void sge_calc_node_proportion(lListElem *node, double total_usage);

double sge_calc_node_usage(lListElem *node,
                           const lList *user_list, 
                           const lList *project_list,
                           const lList *decay_list,
                           u_long curr_time, 
                           const char *projname,
                           u_long seqno );

void _sge_calc_share_tree_proportions(lList *share_tree,
                                      const lList *user_list,
                                      const lList *project_list,
                                      const lList *decay_list,
                                      u_long curr_time);

void sge_calc_share_tree_proportions(lList *share_tree, const lList *user_list, const lList *project_list, const lList *decay_list);

lListElem *search_userprj_node(lListElem *ep, const char *username, const char *projname, lListElem **pep);

void set_share_tree_project_flags(const lList *project_list, lListElem *node);

void sge_add_default_user_nodes(lListElem *root, const lList *user_list, const lList *project_list, const lList *userset_list);

void sgeee_sort_jobs(lList **job_list);
void sgeee_sort_jobs_by(lList **job_list , int by_SGEJ_field, int field_ascending, int jobnum_ascending);

#endif /* __SGE_SUPPORT_H */
