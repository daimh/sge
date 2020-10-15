#ifndef _CULL_STATE_H_
#define _CULL_STATE_H_

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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "cull/cull_list.h"


void cull_state_set_lerrno(int i);
void cull_state_set_noinit(char *s);
void cull_state_set_global_sort_order(const lSortOrder *so);
void cull_state_set_chunk_size(int chunk_size);
void cull_state_set_name_space(const lNameSpace *ns);

int               cull_state_get_lerrno(void);
const char *      cull_state_get_noinit(void);
const lSortOrder *cull_state_get_global_sort_order(void);
int               cull_state_get_chunk_size(void);
const lNameSpace *cull_state_get_name_space(void);

#endif  /* _CULL_STATE_H_ */
