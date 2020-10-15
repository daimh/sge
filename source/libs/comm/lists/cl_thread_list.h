#ifndef __CL_THREAD_LIST_H
#define __CL_THREAD_LIST_H

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



/* basic functions */
int cl_thread_list_setup(cl_raw_list_t** list_p, char* list_name);  /* CR check */
int cl_thread_list_cleanup(cl_raw_list_t** list_p);                 /* CR check */


/* thread list functions that will lock the list */
int cl_thread_list_create_thread(cl_raw_list_t* list_p,
                                 cl_thread_settings_t** new_thread_p,
                                 cl_raw_list_t* log_list ,
                                 const char* name,
                                 int id,
                                 void * (*start_routine)(void *),
                                 cl_thread_cleanup_func_t cleanup_func,
                                 void* user_data,
                                 cl_thread_type_t thread_type);
int cl_thread_list_delete_thread(cl_raw_list_t* list_p, cl_thread_settings_t* thread_p);
int cl_thread_list_delete_thread_without_join(cl_raw_list_t* list_p, cl_thread_settings_t* thread);
int cl_thread_list_delete_thread_from_list(cl_raw_list_t* list_p, cl_thread_settings_t* thread);
int cl_thread_list_delete_thread_by_id(cl_raw_list_t* list_p, int id);  /* CR check */


/* thread functions that will not lock the list */
cl_thread_settings_t* cl_thread_list_get_thread_by_id(cl_raw_list_t* list_p, int thread_id);  /* CR check */
cl_thread_settings_t* cl_thread_list_get_thread_by_name(cl_raw_list_t* list_p, char* thread_name); /* CR check */
cl_thread_settings_t* cl_thread_list_get_thread_by_self(cl_raw_list_t* list_p, pthread_t* thread); /* CR check */
cl_thread_settings_t* cl_thread_list_get_first_thread(cl_raw_list_t* list_p);  /* CR check */


cl_thread_list_elem_t* cl_thread_list_get_first_elem(cl_raw_list_t* list_p);
cl_thread_list_elem_t* cl_thread_list_get_next_elem(cl_thread_list_elem_t* elem);
#endif /* __CL_THREAD_LIST_H */

