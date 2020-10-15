#ifndef __CL_FD_LIST_H
#define __CL_FD_LIST_H

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

#include "comm/lists/cl_lists.h"
#include "comm/cl_data_types.h"

typedef struct cl_fd_list_elem_t {
   cl_com_fd_data_t*    data;     /* actual data */
   cl_raw_list_elem_t*  raw_elem; /* needed for list chaining */
} cl_fd_list_elem_t;

/* basic functions */
int cl_fd_list_setup(cl_raw_list_t** list_p, char* list_name); 
int cl_fd_list_cleanup(cl_raw_list_t** list_p);

/* thread list functions that will lock the list */
int cl_fd_list_register_fd(cl_raw_list_t* list_p, cl_com_fd_data_t* regfd, int lock_list);
int cl_fd_list_unregister_fd(cl_raw_list_t* list_p, cl_fd_list_elem_t* elem, int lock_list);


/* thread functions that will not lock the list */
cl_fd_list_elem_t* cl_fd_list_get_first_elem(cl_raw_list_t* list_p);
cl_fd_list_elem_t* cl_fd_list_get_least_elem(cl_raw_list_t* list_p);
cl_fd_list_elem_t* cl_fd_list_get_next_elem(cl_fd_list_elem_t* elem);
cl_fd_list_elem_t* cl_fd_list_get_last_elem(cl_fd_list_elem_t* elem);

#endif /* __CL_FD_LIST_H */
