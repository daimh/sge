#ifndef __SGE_QINSTANCE_TYPE_H
#define __SGE_QINSTANCE_TYPE_H

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

extern const char *queue_types[];

const char *
qtype_append_to_dstring(u_long32 dtype, dstring *string);

bool 
qinstance_is_batch_queue(const lListElem *this_elem);

bool 
qinstance_is_interactive_queue(const lListElem *this_elem);

bool 
qinstance_is_checkpointing_queue(const lListElem *this_elem);

bool 
qinstance_is_parallel_queue(const lListElem *this_elem);

bool
qinstance_print_qtype_to_dstring(const lListElem *this_elem,
                                 dstring *string, bool only_first_char);

bool
qinstance_parse_qtype_from_string(lListElem *queue, lList **answer_list,
                                  const char *value);

#endif /* __SGE_QINSTANCE_TYPE_H */
