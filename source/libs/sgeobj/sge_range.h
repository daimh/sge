#ifndef __SGE_RANGE_H
#define __SGE_RANGE_H

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

#include "uti/sge_dstring.h"

#include "sgeobj/sge_range_RN_L.h"

#define JUST_PARSE        true
#define INF_ALLOWED       true
#define INF_NOT_ALLOWED   false

#define MAX_IDS_PER_LINE  8
#define MAX_LINE_LEN      70

#define RANGE_INFINITY (9999999)

typedef void (*range_remove_insert_t) (lList **, lList **, u_long32);

/*
 * range element
 */

void
range_get_all_ids(const lListElem *this_elem, u_long32 *min,
                  u_long32 *max, u_long32 *step);

void
range_set_all_ids(lListElem *this_elem, u_long32 min,
                  u_long32 max, u_long32 step);

bool range_containes_id_less_than(const lListElem *this_elem, u_long32 id);

bool range_is_id_within(const lListElem *this_range, u_long32 id);

u_long32 range_get_number_of_ids(const lListElem *this_elem);

void range_correct_end(lListElem *this_elem);

void
range_parse_from_string(lListElem **this_elem, lList **answer_list,
                        const char *string, int step_allowed, int inf_allowed);

/*
 * range list
 */

void
range_list_print_to_string(const lList *this_list,
                           dstring * string, bool ignore_step,
                           bool comma_as_separator, bool print_always_as_range);

void range_to_dstring(u_long32 start, u_long32 end, int step,
                      dstring * dyn_taskrange_str, int ignore_step,
                      bool use_comma_as_separator, bool print_always_as_range);

void range_list_insert_id(lList **this_list, lList **answer_list, u_long32 id);

void range_list_remove_id(lList **this_list, lList **answer_list, u_long32 id);

void
range_list_move_first_n_ids(lList **this_list, lList **answer_list,
                            lList **list, u_long32 n);

bool range_list_is_id_within(const lList *this_list, u_long32 id);

bool range_list_is_empty(const lList *this_list);

void range_list_compress(lList *this_list);

void range_list_sort_uniq_compress(lList *this_list, lList **answer_list, bool correct_end);

u_long32 range_list_get_first_id(const lList *this_list, lList **answer_list);

u_long32 range_list_get_last_id(const lList *this_list, lList **answer_list);

double range_list_get_average(const lList *this_list, u_long32 upperbound);

void range_list_initialize(lList **this_list, lList **answer_list);

u_long32 range_list_get_number_of_ids(const lList *this_list);

bool
range_list_parse_from_string(lList **this_list, lList **answer_list,
                             const char *string, bool just_parse,
                             bool step_allowed, bool inf_allowed);

bool range_list_containes_id_less_than(const lList *range_list, u_long32 id);

void
range_list_calculate_union_set(lList **this_list, lList **answer_list,
                               const lList *list1, const lList *list2);

void
range_list_calculate_difference_set(lList **this_list, lList **answer_list,
                                    const lList *list1, const lList *list2);

void
range_list_calculate_intersection_set(lList **this_list, lList **answer_list,
                                      const lList *list1, const lList *list2);

#endif /* __SGE_RANGE_H */
