#ifndef __SGE_ULONG_H 
#define __SGE_ULONG_H 
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

#include "sgeobj/sge_ulong_ULNG_L.h"

bool 
double_print_infinity_to_dstring(double value, dstring *string);

bool 
double_print_time_to_dstring(double value, dstring *string);

bool 
double_print_memory_to_dstring(double value, dstring *string);

bool 
double_print_int_to_dstring(double value, dstring *string);

bool 
double_print_to_dstring(double value, dstring *string);

bool
ulong_parse_date_time_from_string(u_long32 *this_ulong,
                                  lList **alpp, const char *date_str);

bool
ulong_parse_centry_type_from_string(u_long32 *this_ulong,
                                    lList **answer_list, const char *string);

bool
ulong_parse_centry_relop_from_string(u_long32 *this_ulong,
                                     lList **answer_list, const char *string);

bool 
ulong_parse_from_string(u_long32 *this_ulong,
                        lList **answer_list, const char *string);

bool
ulong_list_parse_from_string(lList **this_list, lList **answer_list,
                             const char *string, const char *delimitor);

bool
ulong_parse_priority(lList **alpp, int *valp, const char *priority_str);

bool
ulong_parse_value_from_string(u_long32 *this_ulong, 
                           lList **answer_list, const char *string);

bool
ulong_parse_task_concurrency(lList **alpp, int *valp, const char *task_concurrency_str);

const char *
double_print_memory_to_string(double value, dstring *string);

int reformatDoubleValue(char *result, size_t lresult, const char *format, const char *oldmem);
#endif /* __SGE_ULONG_H */
