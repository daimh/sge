#ifndef __SGE_COMPLEX_SCHEDD_H
#define __SGE_COMPLEX_SCHEDD_H
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


void monitor_dominance(char *str, u_long32 mask);

int host_complexes2scheduler(lList **new_complex_list, lListElem *host, 
                             lList *exechost_list, lList *complex_list);  

int queue_complexes2scheduler(lList **new_complex_list, lListElem *queue, 
                              const lList *host_list, const lList *complex_list); 

lListElem *
get_attribute_by_name(lListElem *global, lListElem *host, lListElem *queue, 
             const char *attrname, const lList *centry_list, 
             u_long32 start_time, u_long32 duration);

int compare_complexes(int slots, lListElem *complex1, 
                      lListElem *complex2, char *availability_text, 
                      int is_threshold, int force_existence);

bool is_attr_prior(lListElem *upper_el, lListElem *lower_el);

bool get_queue_resource(lListElem *queue_elem, const lListElem *queue, const char *attrname);

lListElem* 
get_attribute(const char *attrname, lList *config_attr, lList *actual_attr, 
              lList *load_attr, const lList *centry_list, lListElem *queue, 
              u_long32 layer, double lc_factor, dstring *reason, bool zero_utilization,
              u_long32 start_time, u_long32 duration);

int string_base_cmp(u_long32 type, const char *s1, const char *s2);
int string_base_cmp_old(u_long32 type, const char *s1, const char *s2);

bool request_cq_rejected(const lList* hard_resource_list, const lListElem *cq,
      const lList *centry_list, bool single_slot, dstring *unsatisfied);


#endif /* __SGE_COMPLEX_SCHEDD_H */
