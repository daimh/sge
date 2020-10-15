#ifndef __SGE_SUBORDINATE_H
#define __SGE_SUBORDINATE_H

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
#include "sgeobj/sge_subordinate_SO_L.h"

/* SO_action defines */
#define SO_ACTION_SR 0x00000001
#define SO_ACTION_LR 0x00000010

bool
tst_sos(int used, int total, lListElem *so);

const char *
so_list_append_to_dstring(const lList *this_list, dstring *string);

bool
so_list_add(lList **this_list, lList **answer_list, const char *so_name,
            u_long32 threshold, u_long32 slots_sum, u_long32 seq_no, u_long32 action);

bool
so_list_resolve(const lList *so_list, lList **answer_list,
                lList **resolved_so_list, const char *qinstance_name,
                const char *hostname);

#endif /* __SGE_SUBORDINATE_H */
