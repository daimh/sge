#ifndef __CL_HANDLE_LIST_H
#define __CL_HANDLE_LIST_H

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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/


#include "comm/cl_data_types.h"

typedef struct cl_handle_list_elem {
   cl_com_handle_t* handle;   /* data */
   cl_raw_list_elem_t*   raw_elem;
} cl_handle_list_elem_t;




/* basic functions */
int cl_handle_list_setup(cl_raw_list_t** list_p, char* list_name);  /* CR check */
int cl_handle_list_cleanup(cl_raw_list_t** list_p);   /* CR check */


/* thread list functions that will lock the list */
int cl_handle_list_append_handle(cl_raw_list_t* list_p, cl_com_handle_t* handle, int do_lock);  /* CR check */
int cl_handle_list_remove_handle(cl_raw_list_t* list_p, cl_com_handle_t* handle, int do_lock);  /* CR check */

/* thread functions that will not lock the list */
cl_handle_list_elem_t* cl_handle_list_get_first_elem(cl_raw_list_t* list_p);  /* CR check */
cl_handle_list_elem_t* cl_handle_list_get_next_elem(cl_handle_list_elem_t* elem);  /* CR check */
cl_handle_list_elem_t* cl_handle_list_get_last_elem(cl_handle_list_elem_t* elem);  /* CR check */


#endif /* __CL_HANDLE_LIST_H */

