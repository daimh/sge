#ifndef __SGE_SL_H
#define __SGE_SL_H

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

#include <pthread.h>

/****** uti/sl/--SimpleList() **********************************************
*
*  NAME
*     sl - A Simple List Implementation for Grid Engine
*
*  SYNOPSIS
*
*  FUNCTION
*     This module provides a simple and thread safe list
*     implementation for Grid Engine. Lists can be created, destroyed
*     and the number of the contained nodes can be retrieved. 
*
*        sge_sl_create()
*        sge_sl_destroy()
*        sge_sl_get_elements()
*
*     The list is implemented as double links list so that data
*     can be accessed forward and backward easily. Function arguments
*     decide if the first or last element should be addressed.
*
*        sge_sl_insert()
*        sge_sl_delete()
*        sge_sl_data()
*
*     Elements can also be inserted into a sorted list or they can
*     be searched using a user defined compare function.
*
*        sge_sl_insert_search()
*        sge_sl_delete_search()
*        sge_sl_data_search()
*        sge_sl_sort()
*
*     The underlaying list data structure is thread safe so that one list
*     can be accessed without additional synchronisation between the
*     threads. 
*
*     Node data structures are NOT thread safe for that reason
*     following functions need additional synchronisation because they
*     provide access to node data structures.
*
*        sge_sl_elem_create()
*        sge_sl_elem_destroy()
*        sge_sl_elem_data()
*        sge_sl_elem_next()
*        sge_sl_elem_search()
*        sge_sl_dechain()
*        sge_sl_insert_before()
*        sge_sl_append_after()
*
*     The list implementation provides locking functions that provide a way
*     to synchronize access with a list object. Therefore it is easily
*     possible to secure the above node based functions and execute them in
*     an atomic block.
*
*        sge_sl_lock()
*        sge_sl_unlock()
*
*     Please be cautious when you need other locks in a code block between
*     the lock and unlock function. If the sequence in demanding the locks
*     is not correct this might cause a deadlock.
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_create()
*     uti/sl/sge_sl_elem_destroy()
*     uti/sl/sge_sl_elem_data()
*     uti/sl/sge_sl_elem_next()
*     uti/sl/sge_sl_elem_search()
*     uti/sl/sge_sl_dechain()
*     uti/sl/sge_sl_insert_before()
*     uti/sl/sge_sl_append_after()
*     uti/sl/sge_sl_create()
*     uti/sl/sge_sl_destroy()
*     uti/sl/sge_sl_lock()
*     uti/sl/sge_sl_unlock()
*     uti/sl/sge_sl_insert()
*     uti/sl/sge_sl_insert_search()
*     uti/sl/sge_sl_data()
*     uti/sl/sge_sl_data_search()
*     uti/sl/sge_sl_delete()
*     uti/sl/sge_sl_delete_search()
*     uti/sl/sge_sl_get_elements()
*     uti/sl/sge_sl_sort()
****************************************************************************/

#define for_each_sl_locked(elem, list) \
   for(sge_sl_lock(list), elem = NULL, sge_sl_elem_next(list, &elem, SGE_SL_FORWARD); \
       elem != NULL || (sge_sl_unlock(list), false); \
       sge_sl_elem_next(list, &elem, SGE_SL_FORWARD))

#define for_each_sl(elem, list) \
   for(elem = NULL, sge_sl_elem_next(list, &elem, SGE_SL_FORWARD); \
       elem != NULL; \
       sge_sl_elem_next(list, &elem, SGE_SL_FORWARD))

typedef int (*sge_sl_compare_f)(const void *data1, const void *data2);

typedef bool (*sge_sl_destroy_f)(void **data);

typedef struct _sge_sl_elem_t sge_sl_elem_t;
typedef struct _sge_sl_list_t sge_sl_list_t;

enum _sge_sl_direction_t {
   SGE_SL_FORWARD,
   SGE_SL_BACKWARD
};

typedef enum _sge_sl_direction_t sge_sl_direction_t;

struct _sge_sl_elem_t {
   sge_sl_elem_t *prev;
   sge_sl_elem_t *next;
   void *data;
};

struct _sge_sl_list_t {
   /* mutex to secure other elements in the struct */
   pthread_mutex_t mutex; 

   /* fist last element pointer and number of elements */
   sge_sl_elem_t *first;
   sge_sl_elem_t *last;
   u_long32 elements;
};

bool
sge_sl_elem_create(sge_sl_elem_t **elem, void *data);

bool
sge_sl_elem_destroy(sge_sl_elem_t **elem, sge_sl_destroy_f destroy);

void * 
sge_sl_elem_data(sge_sl_elem_t *elem);

bool
sge_sl_elem_next(sge_sl_list_t *list, 
                 sge_sl_elem_t **elem, sge_sl_direction_t direction);


bool
sge_sl_elem_search(sge_sl_list_t *list, sge_sl_elem_t **elem, void *data, 
                   sge_sl_compare_f compare, sge_sl_direction_t direction);

bool
sge_sl_create(sge_sl_list_t **list);

bool
sge_sl_destroy(sge_sl_list_t **list, sge_sl_destroy_f destroy);

bool
sge_sl_dechain(sge_sl_list_t *list, sge_sl_elem_t *elem);

bool
sge_sl_insert_before(sge_sl_list_t *list, sge_sl_elem_t *new_elem, sge_sl_elem_t *elem);

bool
sge_sl_append_after(sge_sl_list_t *list, sge_sl_elem_t *new_elem, sge_sl_elem_t *elem);

bool
sge_sl_lock(sge_sl_list_t *list);

bool
sge_sl_unlock(sge_sl_list_t *list);

bool
sge_sl_insert(sge_sl_list_t *list, void *data, sge_sl_direction_t direction);

bool
sge_sl_insert_search(sge_sl_list_t *list, void *data, sge_sl_compare_f compare);

bool
sge_sl_data(sge_sl_list_t *list, void **data, sge_sl_direction_t direction);

bool
sge_sl_data_search(sge_sl_list_t *list, void *key, void **data, 
                   sge_sl_compare_f compare, sge_sl_direction_t direction);

bool
sge_sl_delete(sge_sl_list_t *list, 
              sge_sl_destroy_f destroy, sge_sl_direction_t direction);
  
bool
sge_sl_delete_search(sge_sl_list_t *list, void *key, sge_sl_destroy_f destroy, 
                     sge_sl_compare_f compare, sge_sl_direction_t direction);

u_long32
sge_sl_get_elem_count(sge_sl_list_t *list);

pthread_mutex_t *
sge_sl_get_mutex(sge_sl_list_t *list);

bool
sge_sl_sort(sge_sl_list_t *list, sge_sl_compare_f compare);

#endif
