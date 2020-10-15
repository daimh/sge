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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdlib.h>
#include <stdio.h>

#include "uti/sge_rmon.h"
#include "uti/sge_err.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_sl.h"
#include "uti/sge_thread_ctrl.h"
#include "uti/sge_mtutil.h"

#include "msg_common.h"

#define SL_LAYER BASIS_LAYER
#define SL_MUTEX_NAME "sl_mutex"

/****** uti/sl/sge_sl_elem_create() ******************************************
*  NAME
*     sge_sl_elem_create() -- create a list element 
*
*  SYNOPSIS
*     bool sge_sl_elem_create(sge_sl_elem_t **elem, void *data) 
*
*  FUNCTION
*     This function creates a new sl element that can later on be inserted
*     into a sl list.
*
*     On success the function creates the element stores the 'data' pointer
*     in it and returns the element pointer in 'elem'.
*
*     On error false is returned by this function and 'elem' contains a
*     NULL pointer. 
*
*     sge_sl_elem_destroy() can be used to destroy elements that were
*     created with this function.
*
*  INPUTS
*     sge_sl_elem_t **elem - location were the pointer to the new 
*                            element will be stored 
*     void *data           - data pointer that will be stored in elem 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error 
*
*  NOTES
*     MT-NOTE: sge_sl_elem_create() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_destroy()
*******************************************************************************/
bool
sge_sl_elem_create(sge_sl_elem_t **elem, void *data) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_elem_create");
   if (elem != NULL) {
      const size_t size = sizeof(sge_sl_elem_t);
      sge_sl_elem_t *new_elem;

      new_elem = (sge_sl_elem_t *)malloc(size);
      if (new_elem != NULL) {
         new_elem->prev = NULL;
         new_elem->next = NULL;
         new_elem->data = data;
         *elem = new_elem;
      } else {
         sge_err_set(SGE_ERR_MEMORY, MSG_UNABLETOALLOCATEBYTES_DS, size, SGE_FUNC);
         *elem = NULL;
         ret = false;
      }
   } 
   DRETURN(ret);
}

/****** uti/sl/sge_sl_elem_destroy() *******************************************
*  NAME
*     sge_sl_elem_destroy() -- destroys a sl element 
*
*  SYNOPSIS
*     bool sge_sl_elem_destroy(sge_sl_elem_t **elem, sge_sl_destroy_f destroy) 
*
*     typedef bool (*sge_sl_destroy_f)(void **data);
*
*  FUNCTION
*     This function destroys the provided sl 'elem' and optionally destroys
*     also the data that is referenced in the element if a 'destroy'
*     function is passed.
*
*     If 'elem' is part of a list it has to be unchained before this
*     function can be called. Otherwise the list would be corrupted.
*
*     On success elem will be set to NULL and the function returns true.
*
*  INPUTS
*     sge_sl_elem_t **elem     - pointer to a sl element pointer
*     sge_sl_destroy_f destroy - destroy function 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  EXAMPLE
*     Here is an example for a destroy function for a C string.
*
*        bool
*        destroy(void **data_ptr) {
*           char *string = *(char **) data_ptr;
*
*           sge_free(&string);
*           return true;
*        }
*
*  NOTES
*     MT-NOTE: sge_sl_elem_destroy() is MT safe.
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_create()
*******************************************************************************/
bool
sge_sl_elem_destroy(sge_sl_elem_t **elem, sge_sl_destroy_f destroy) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_elem_destroy");
   if (elem != NULL && *elem != NULL) {
      if (destroy != NULL) {
         destroy(&(*elem)->data);
      }
      sge_free(elem);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_elem_data() ******************************************
*  NAME
*     sge_sl_elem_data() -- return first/last data pointer 
*
*  SYNOPSIS
*     void *sge_sl_elem_data(sge_sl_elem_t *elem) 
*
*  FUNCTION
*     returns the stored data pointer of an element 
*
*  INPUTS
*     sge_sl_elem_t *elem - sl element 
*
*  RESULT
*     void * - data pointer
*
*  NOTES
*     MT-NOTE: sge_sl_elem_data() is MT safe
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_create()
*     uti/sl/sge_sl_elem_destroy()
*******************************************************************************/
void * 
sge_sl_elem_data(sge_sl_elem_t *elem) {
   void *ret = NULL;

   DENTER(SL_LAYER, "sge_sl_elem_data");
   if (elem != NULL) {
      ret = elem->data; 
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_dechain() ************************************************
*  NAME
*     sge_sl_dechain() -- unchains a element from a list 
*
*  SYNOPSIS
*     bool sge_sl_dechain(sge_sl_list_t *list, sge_sl_elem_t *elem) 
*
*  FUNCTION
*     This functions unchains 'elem' from 'list'. 'elem' can afterwards
*     be inserted into a list again or can be destroyed.
*
*  INPUTS
*     sge_sl_list_t *list - sl list 
*     sge_sl_elem_t *elem - sl elem 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*        
*  NOTES
*     MT-NOTE: sge_sl_dechain() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_destroy()
*     uti/sl/sge_sl_dechain()
*     uti/sl/sge_sl_append()
*     uti/sl/sge_sl_append_after()
*     uti/sl/sge_sl_insert()
*     uti/sl/sge_sl_insert_before()
*     uti/sl/sge_sl_insert_search()
*******************************************************************************/
bool
sge_sl_dechain(sge_sl_list_t *list, sge_sl_elem_t *elem) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_dechain");
   if (list != NULL && elem != NULL) {
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      if (elem->prev) {
         elem->prev->next = elem->next;
      } else {
         list->first = elem->next;
      }
      if (elem->next) {
         elem->next->prev = elem->prev;
      } else {
         list->last = elem->prev;
      }

      elem->next = NULL;
      elem->prev = NULL;

      list->elements--;
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_insert_before() ******************************************
*  NAME
*     sge_sl_insert_before() -- inserts a new element before another one 
*
*  SYNOPSIS
*     bool 
*     sge_sl_insert_before(sge_sl_list_t *list, 
*                          sge_sl_elem_t *new_elem, sge_sl_elem_t *elem) 
*
*  FUNCTION
*     Inserts 'new_elem' before 'elem' in 'list'. 'elem' must be
*     an element already part of 'list'. 
*
*  INPUTS
*     sge_sl_list_t *list     - sl list 
*     sge_sl_elem_t *new_elem - new sl element 
*     sge_sl_elem_t *elem     - sl elem already part of 'list" 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_insert_before() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_destroy()
*     uti/sl/sge_sl_dechain()
*     uti/sl/sge_sl_append()
*     uti/sl/sge_sl_append_after()
*     uti/sl/sge_sl_insert()
*     uti/sl/sge_sl_insert_before()
*     uti/sl/sge_sl_insert_search()
*******************************************************************************/
bool
sge_sl_insert_before(sge_sl_list_t *list, sge_sl_elem_t *new_elem, sge_sl_elem_t *elem) {
   bool ret = true;
   
   DENTER(SL_LAYER, "sge_sl_insert_before");
   if (list != NULL && new_elem != NULL && elem != NULL) {
      sge_sl_elem_t *last;
 
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex); 
      last = elem->prev;
      if (last == NULL) {
         elem->prev = new_elem;
         new_elem->next = elem;
         list->first = new_elem;
      } else {
         last->next = new_elem;
         elem->prev = new_elem;
         new_elem->prev = last;
         new_elem->next = elem;
      }
      list->elements++;
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_append_after() ****************************************
*  NAME
*     sge_sl_append_after() -- Appends a new element after another one 
*
*  SYNOPSIS
*     bool 
*     sge_sl_append_after(sge_sl_list_t *list, 
*                         sge_sl_elem_t *new_elem, sge_sl_elem_t *elem) 
*
*  FUNCTION
*     This elements appends 'new_elem' into 'list' after 'elem'.
*
*  INPUTS
*     sge_sl_list_t *list     - sl list 
*     sge_sl_elem_t *new_elem - new sl elem 
*     sge_sl_elem_t *elem     - sl elem already part of list 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_append_after() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_destroy()
*     uti/sl/sge_sl_dechain()
*     uti/sl/sge_sl_append()
*     uti/sl/sge_sl_append_after()
*     uti/sl/sge_sl_insert()
*     uti/sl/sge_sl_insert_before()
*     uti/sl/sge_sl_insert_search()
*******************************************************************************/
bool
sge_sl_append_after(sge_sl_list_t *list, sge_sl_elem_t *new_elem, sge_sl_elem_t *elem) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_elem_insert_before");
   if (list != NULL && new_elem != NULL && elem != NULL) {
      sge_sl_elem_t *current;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      current = elem->next;
      if (current == NULL) {
         elem->next = new_elem;
         new_elem->prev = elem;
         list->last = new_elem;
      } else {
         elem->next = new_elem;
         current->prev = new_elem;
         new_elem->prev = elem;
         new_elem->next = current;
      }
      list->elements++;
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_elem_next() **********************************************
*  NAME
*     sge_sl_elem_next() -- provides the next element in sequence
*
*  SYNOPSIS
*     bool 
*     sge_sl_elem_next(sge_sl_list_t *list, 
*                      sge_sl_elem_t **elem, sge_sl_direction_t direction) 
*
*  FUNCTION
*     This function provides the possibility to iterate over all elements
*     in 'list'. 'elem' will contain a pointer to an element when the
*     function returns or the value NULL.  'elem' is also an input
*     parameter and it defines what element of 'list' is returned.
*   
*     If *elem is NULL and direction is SGE_SL_FORWARD them *elem will
*     contain the first element in 'list'. If direction is SGE_SL_BACKWARD
*     then it will contain the last.
*
*     If *elem is not NULL then the next element in the list sequence is
*     returned if direction is SGE_SL_FORWARD or the previous one if
*     direction is SGE_SL_BACKWARD.
*
*     If the list is empty or if there is no previous/next element then
*     NULL will be retuned in 'elem'.
*
*  INPUTS
*     sge_sl_list_t *list          - sl list 
*     sge_sl_elem_t **elem         - input/output sl elem 
*     sge_sl_direction_t direction - direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  EXAMPLE
*     Following code shows how it is possible to iterate over
*     the whole list:
*
*     {
*        sge_sl_list_t *list;
*        sge_sl_elem_t *next;
*        sge_sl_elem_t *current;
*
*        // assume that elements are added to the list here
*
*        next = NULL;
*        sge_sl_elem_next(list, &next, SGE_SL_FORWARD);
*        while ((current = next) != NULL) {
*           sge_sl_elem_next(list, &next, SGE_SL_FORWARD);
*
*           // so something with 'current' here
*        } 
*     }
*
*  NOTES
*     MT-NOTE: sge_sl_elem_next() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_search()
*******************************************************************************/
bool
sge_sl_elem_next(sge_sl_list_t *list, 
                 sge_sl_elem_t **elem, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(BASIS_LAYER, "sge_sl_elem_next");
   if (list != NULL && elem != NULL) {
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      if (*elem != NULL) {
         if (direction == SGE_SL_FORWARD) {
            *elem = (*elem)->next;
         } else { 
            *elem = (*elem)->prev;
         }
      } else {
         if (direction == SGE_SL_FORWARD) {
            *elem = list->first;
         } else { 
            *elem = list->last;
         }
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   } 
   DRETURN(ret);
}

/****** uti/sl/sge_sl_elem_search() ********************************************
*  NAME
*     sge_sl_elem_search() -- searches the next element in sequence 
*
*  SYNOPSIS
*     bool 
*     sge_sl_elem_search(sge_sl_list_t *list, sge_sl_elem_t **elem, 
*                        void *key, sge_sl_compare_f compare, 
*                        sge_sl_direction_t direction) 
*
*  FUNCTION
*     This function provides the possibility to iterate over certain
*     elements in 'list'. 'elem' will contain a pointer to an element when
*     the function returns or the value NULL.  'elem' is also an input
*     parameter and it defines what element of 'list' is returned.
*   
*     If *elem is NULL and direction is SGE_SL_FORWARD then *elem will
*     contain the first element in 'list' that is equivalent with the
*     provided 'key'.  If direction is SGE_SL_BACKWARD then it will contain
*     the last element that matches.
*
*     If *elem is not NULL then the next element in the list sequence is
*     returned if direction is SGE_SL_FORWARD or the previous one if
*     direction is SGE_SL_BACKWARD.
*
*     If the list is empty or if there is no previous/next element then
*     NULL will be retuned in 'elem'.
*
*     The provided 'compare' function is used to compare the provided 'key'
*     with the data that is contained in the element. 'key' is passed as
*     first parameter to the 'compare' function.
*
*  INPUTS
*     sge_sl_list_t *list          - sl list 
*     sge_sl_elem_t **elem         - input/output sl elem 
*     void *key                    - key that must match 
*     sge_sl_compare_f compare     - compare function 
*     sge_sl_direction_t direction - search direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  EXAMPLE
*     This compare function could match static C strings stored in
*     a sl list as data pointers.
*
*     int 
*     fnmatch_compare(const void *key_pattern, const void *data) {
*        int ret = 0;
*
*        if (key_pattern != NULL && data != NULL) {
*           ret = fnmatch(*(char**)key_pattern, *(char**)data, 0);
*        }
*        return ret;
*     }
*
*  NOTES
*     MT-NOTE: sge_sl_elem_search() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_elem_next()
*******************************************************************************/
bool
sge_sl_elem_search(sge_sl_list_t *list, sge_sl_elem_t **elem, void *key, 
                   sge_sl_compare_f compare, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_elem_search");
   if (list != NULL && elem != NULL && compare != NULL) {
      sge_sl_elem_t *next = NULL;
      sge_sl_elem_t *current = NULL;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      if (*elem != NULL) {
         if (direction == SGE_SL_FORWARD) {
            next = (*elem)->next;
         } else {
            next = (*elem)->prev;
         } 
      } else {
         if (direction == SGE_SL_FORWARD) {
            next = list->first; 
         } else {
            next = list->last;
         }
      }
      while ((current = next) != NULL && current != NULL && 
             compare((const void *)&key, (const void *)&current->data) != 0) {
         if (direction == SGE_SL_FORWARD) {
            next = current->next;
         } else {
            next = current->prev;
         }
      }
      *elem = current;
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_create() *************************************************
*  NAME
*     sge_sl_create() -- Create a new simple list 
*
*  SYNOPSIS
*     bool sge_sl_create(sge_sl_list_t **list) 
*
*  FUNCTION
*     This function creates a new simple list and returns the list in the
*     'list' parameter. In case of an error NULL will be returned. 
*
*  INPUTS
*     sge_sl_list_t **list - new simple list 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_create() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_destroy()
*******************************************************************************/
bool
sge_sl_create(sge_sl_list_t **list) {
   bool ret = true;
   
   DENTER(SL_LAYER, "sge_sl_create");
   if (list != NULL) {
      const size_t size = sizeof(sge_sl_list_t);
      sge_sl_list_t *new_list;

      new_list = (sge_sl_list_t *)malloc(size);
      if (new_list != NULL) {
         pthread_mutexattr_t mutex_attr;

         /* initialize the mutex */
         pthread_mutexattr_init(&mutex_attr);
         pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
         pthread_mutex_init(&new_list->mutex, &mutex_attr);
         pthread_mutexattr_destroy(&mutex_attr);

         /* other members */
         new_list->first = NULL;
         new_list->last = NULL;
         new_list->elements = 0;

         *list = new_list;
      } else {
         sge_err_set(SGE_ERR_MEMORY, MSG_UNABLETOALLOCATEBYTES_DS, size, SGE_FUNC);
         ret = false;
         *list = NULL;
      }
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_destroy() *********************************************
*  NAME
*     sge_sl_destroy() -- Destroys a simple list 
*
*  SYNOPSIS
*     bool sge_sl_destroy(sge_sl_list_t **list, sge_sl_destroy_f destroy) 
*
*  FUNCTION
*     This function destroys 'list' and sets the pointer to NULL.
*     If a 'destroy' function is provided then it will be used
*     to destroy all data elements that are referenced by the list
*     elements. 
*
*  INPUTS
*     sge_sl_list_t **list     - sl list
*     sge_sl_destroy_f destroy - destroy function 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_destroy() is not MT safe 
*
*  EXAMPLE
*     Here is an example for a destroy function for a C string.
*
*        bool
*        destroy(void **data_ptr) {
*           char *string = *(char **) data_ptr;
*
*           sge_free(&string);
*           return true;
*        }
*
*  SEE ALSO
*     uti/sl/sge_sl_create()
*     uti/sl/sge_sl_elem_destroy()
*******************************************************************************/
bool
sge_sl_destroy(sge_sl_list_t **list, sge_sl_destroy_f destroy) {
   bool ret = true;
   
   DENTER(SL_LAYER, "sge_sl_destroy");
   if (list != NULL && *list != NULL) {
      sge_sl_elem_t *next;
      sge_sl_elem_t *current;
  
      /* destroy content */ 
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &(*list)->mutex);
      next = (*list)->first;
      while ((current = next) != NULL) {
         next = current->next;

         ret &= sge_sl_elem_destroy(&current, destroy);
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &(*list)->mutex);

      /* final destroy */
      pthread_mutex_destroy(&(*list)->mutex);
      sge_free(list);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_lock() *************************************************
*  NAME
*     sge_sl_lock() -- locks a list 
*
*  SYNOPSIS
*     bool sge_sl_lock(sge_sl_list_t *list) 
*
*  FUNCTION
*     A call of this functions locks the provided 'list' so that all
*     list operations executed between the lock and unlock are
*     executed as atomic operation.
*
*  INPUTS
*     sge_sl_list_t *list - list 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_lock() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_unlock() 
*******************************************************************************/
bool
sge_sl_lock(sge_sl_list_t *list) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_lock");
   if (list != NULL) {
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_unlock() ***********************************************
*  NAME
*     sge_sl_unlock() -- unlocks a list 
*
*  SYNOPSIS
*     bool sge_sl_unlock(sge_sl_list_t *list) 
*
*  FUNCTION
*     A call of this functions unlocks the provided 'list' that was
*     previously locked with sge_sl_lock.
*
*  INPUTS
*     sge_sl_list_t *list - list 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_unlock() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_lock() 
*******************************************************************************/
bool
sge_sl_unlock(sge_sl_list_t *list) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_unlock");
   if (list != NULL) {
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_insert() *************************************************
*  NAME
*     sge_sl_insert() -- insert a new element 
*
*  SYNOPSIS
*     bool 
*     sge_sl_insert(sge_sl_list_t *list, 
*                   void *data, sge_sl_direction_t direction) 
*
*  FUNCTION
*     Insert a new node in 'list' that references 'data'. If 'direction' 
*     is SGE_SL_FORWARD then the element will be inserted at the beginning
*     of 'list' otherwise at the end.
*
*  INPUTS
*     sge_sl_list_t *list - simple list 
*     void *data          - data  
*     sge_sl_direction_t  - direction
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_insert() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_append() 
*     uti/sl/sge_sl_insert_search() 
*******************************************************************************/
bool
sge_sl_insert(sge_sl_list_t *list, void *data, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_insert");
   if (list != NULL) {
      sge_sl_elem_t *new_elem;

      ret = sge_sl_elem_create(&new_elem, data);
      if (ret) {
         sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
         if (direction == SGE_SL_FORWARD) {
            if (list->first != NULL) {
               list->first->prev = new_elem;
            }
            new_elem->next = list->first;
            list->first = new_elem;
            if (list->last == NULL) {
               list->last = new_elem;
            }
         } else {
            if (list->last != NULL) {
               list->last->next = new_elem;
            }
            new_elem->prev = list->last;
            list->last = new_elem;
            if (list->first == NULL) {
               list->first = new_elem;
            }
         }
         list->elements++;
         sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      } 
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_insert_search() *************************************************
*  NAME
*     sge_sl_insert_search() -- inserts a new element in a sorted list
*
*  SYNOPSIS
*     bool sge_sl_insert_search(sge_sl_list_t *list, void *data, 
*                        sge_sl_compare_f *compare) 
*
*  FUNCTION
*     Inserts a new element in 'list' that references 'data'. The function
*     assumes that 'list' is sorted in ascending order. To find the correct
*     position for the new element the 'compare' function will be used. 
*
*  INPUTS
*     sge_sl_list_t *list      - list 
*     void *data               - data reference 
*     sge_sl_compare_f compare - compare function 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  EXAMPLE
*     Example for a compare function when data is a C string
*
*     int 
*     compare(const void *data1, const void *data2) {
*        int ret = 0;
*
*        if (data1 != NULL && data2 != NULL) {
*           ret = strcmp(*(char**)data1, *(char**)data2);
*        }
*        return ret;
*     }
*
*  NOTES
*     MT-NOTE: sge_sl_insert_search() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_insert() 
*     uti/sl/sge_sl_append() 
*******************************************************************************/
bool
sge_sl_insert_search(sge_sl_list_t *list, void *data, sge_sl_compare_f compare) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_insert_search");
   if (list != NULL && compare != NULL) {
      sge_sl_elem_t *new_elem;

      ret = sge_sl_elem_create(&new_elem, data);
      if (ret) {
         sge_sl_elem_t *last = NULL;
         sge_sl_elem_t *current = NULL;

         sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
         current = list->first;
         while (current != NULL && 
                compare((const void *)&data, (const void *)&current->data) > 0) {
            last = current;
            current = current->next; 
         }

         if (last == NULL && current == NULL) {
            list->first = new_elem;
            list->last = new_elem;
         } else if (last == NULL) {
            current->prev = new_elem;
            new_elem->next = current;
            list->first = new_elem;
         } else if (current == NULL) {
            last->next = new_elem;
            new_elem->prev = last;
            list->last = new_elem;
         } else {
            last->next = new_elem;
            current->prev = new_elem;
            new_elem->prev = last;
            new_elem->next = current;
         }
         list->elements++;
         sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      }
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_data() ************************************************
*  NAME
*     sge_sl_data() -- returns the first or last data element 
*
*  SYNOPSIS
*     bool 
*     sge_sl_data(sge_sl_list_t *list, 
*                 void **data, sge_sl_direction_t direction) 
*
*  FUNCTION
*     Depending on 'direction' this function returns the pointer
*     to the first/last data object of 'list' in 'data'.
*
*  INPUTS
*     sge_sl_list_t *list          - list 
*     void **data                  - data pointer 
*     sge_sl_direction_t direction - direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_data() is MT safe 
*******************************************************************************/
bool
sge_sl_data(sge_sl_list_t *list, void **data, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_data");
   if (list != NULL && data != NULL) {
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      if (direction == SGE_SL_FORWARD && list->first != NULL) {
         *data = list->first->data;
      } else if (direction == SGE_SL_BACKWARD && list->last != NULL) {
         *data = list->last->data;
      } else {
         *data = NULL;
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_data_search() ***********************************************
*  NAME
*     sge_sl_data_search() --  search a elements in list
*
*  SYNOPSIS
*     bool 
*     sge_sl_data_search(sge_sl_list_t *list, void *key, void **data, 
*                        sge_sl_compare_f compare, sge_sl_direction_t direction) 
*
*  FUNCTION
*     This function tries to find a element in 'list'. As result of the
*     search the 'data' pointer will be returned. To find the data pointer
*     the 'compare' function and the 'key' will be used. 'key' is past as
*     first argument to the 'compare' function. 'direction' decides if
*     this function starts the search from beginning or end of the list.
*
*  INPUTS
*     sge_sl_list_t *list          - list 
*     void *key                    - search key 
*     void **data                  - returned data pointer 
*     sge_sl_compare_f compare     - compare function 
*     sge_sl_direction_t direction - direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_data_search() is MT safe 
*******************************************************************************/
bool
sge_sl_data_search(sge_sl_list_t *list, void *key, void **data, 
                   sge_sl_compare_f compare, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_data_search");
   if (list != NULL && data != NULL && compare != NULL) {
      sge_sl_elem_t *elem = NULL;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      ret &= sge_sl_elem_search(list, &elem, key, compare, direction);
      if (ret && elem != NULL) {
         *data = elem->data;
      } else {
         *data = NULL;
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_delete() *************************************************
*  NAME
*     sge_sl_delete() -- delete first/last element 
*
*  SYNOPSIS
*     bool 
*     sge_sl_delete(sge_sl_list_t *list, sge_sl_destroy_f destroy, 
*                   sge_sl_direction_t direction) 
*
*  FUNCTION
*     This function deletes the first/last element of 'list' depending
*     on the provided 'direction'. If 'destroy' is not NULL then
*     this function will be used to destroy the element data. 
*
*  INPUTS
*     sge_sl_list_t *list          - list 
*     sge_sl_destroy_f destroy     - destroy 
*     sge_sl_direction_t direction - direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_delete() is MT safe 
*******************************************************************************/
bool
sge_sl_delete(sge_sl_list_t *list, 
              sge_sl_destroy_f destroy, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_delete");
   if (list != NULL) {
      sge_sl_elem_t *elem;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      if (direction == SGE_SL_FORWARD) {
         elem = list->first;
      } else {
         elem = list->last;
      }
      ret &= sge_sl_dechain(list, elem);
      if (ret) {
         ret &= sge_sl_elem_destroy(&elem, destroy);
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}
  
/****** uti/sl/sge_sl_delete_search() ****************************************
*  NAME
*     sge_sl_delete_search() -- search a element and delete it 
*
*  SYNOPSIS
*     bool 
*     sge_sl_delete_search(sge_sl_list_t *list, void *key, 
*                          sge_sl_destroy_f destroy, 
*                          sge_sl_compare_f compare, 
*                          sge_sl_direction_t direction) 
*
*  FUNCTION
*     This function searches a element in 'list' using the 'key' and
*     'compare' function and then deletes it. If 'direction' is
*     SGE_SL_FORWARD then the search will start from beginning, otherwise
*     from the end of the list. The first matched element will be
*     destroyed. If there is a 'destroy' function provided then this
*     function will be used to destroy the element data.
*
*  INPUTS
*     sge_sl_list_t *list          - list
*     void *key                    - search key 
*     sge_sl_destroy_f destroy     - destroy function 
*     sge_sl_compare_f compare     - compare function 
*     sge_sl_direction_t direction - search direction 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_delete_search() is MT safe 
*******************************************************************************/
bool
sge_sl_delete_search(sge_sl_list_t *list, void *key, sge_sl_destroy_f destroy, 
                     sge_sl_compare_f compare, sge_sl_direction_t direction) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_delete_search");
   if (list != NULL && key != NULL && compare != NULL) {
      sge_sl_elem_t *elem = NULL;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      ret &= sge_sl_elem_search(list, &elem, key, compare, direction);
      if (ret) {
         ret &= sge_sl_dechain(list, elem);
      }
      if (ret) {
         ret &= sge_sl_elem_destroy(&elem, destroy);
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_elem_count() *****************************************
*  NAME
*     sge_sl_elem_count() -- returns the number of elements 
*
*  SYNOPSIS
*     u_long32 sge_sl_elem_count(sge_sl_list_t *list) 
*
*  FUNCTION
*     This function returns the number of elements contained in 'list'. 
*
*  INPUTS
*     sge_sl_list_t *list - list pointer 
*
*  RESULT
*     u_long32 - number of elements
*
*  NOTES
*     MT-NOTE: sge_sl_elem_count() is MT safe 
*******************************************************************************/
u_long32
sge_sl_get_elem_count(sge_sl_list_t *list) {
   u_long32 elems = 0; 
   
   DENTER(SL_LAYER, "sge_sl_elem_count");
   if (list != NULL) {
      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      elems = list->elements;
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(elems);
}

/****** uti/sl/sge_sl_sort() *************************************************
*  NAME
*     sge_sl_sort() -- Sorts the list 
*
*  SYNOPSIS
*     bool sge_sl_sort(sge_sl_list_t *list, sge_sl_compare_f compare) 
*
*  FUNCTION
*     This function sorts the 'list' with the quick sort algorithm.
*     'compare' function will be used to compare the list elements.
*
*  INPUTS
*     sge_sl_list_t *list      - list 
*     sge_sl_compare_f compare - compare function 
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*
*  NOTES
*     MT-NOTE: sge_sl_sort() is MT safe 
*
*  SEE ALSO
*     uti/sl/sge_sl_insert_search()
*******************************************************************************/
bool
sge_sl_sort(sge_sl_list_t *list, sge_sl_compare_f compare) {
   bool ret = true;

   DENTER(SL_LAYER, "sge_sl_sort");
   if (list != NULL && compare != NULL) {
      void **pointer_array;
      size_t size;

      sge_mutex_lock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
      size = sizeof(void *) * list->elements;
      pointer_array = (void**) malloc(size);
      if (pointer_array != NULL) {
         sge_sl_elem_t *elem = NULL;
         int i;

         /* fill the pointer array with the data pointers */
         i = 0; 
         for_each_sl(elem, list) {
            pointer_array[i++] = elem->data;   
         }

         /* sort */
         qsort((void *)pointer_array, list->elements, sizeof(void *), compare);
   
         /* now move the sorted pointers back to elemnts in the list */
         i = 0; 
         for_each_sl(elem, list) {
            elem->data = pointer_array[i++];   
         }
   
         /* cleanup */
         sge_free(&pointer_array);
      } else {
         sge_err_set(SGE_ERR_MEMORY, MSG_UNABLETOALLOCATEBYTES_DS, size, SGE_FUNC);
         ret = false;
      }
      sge_mutex_unlock(SL_MUTEX_NAME, SGE_FUNC, __LINE__, &list->mutex);
   }
   DRETURN(ret);
}

/****** uti/sl/sge_sl_get_mutex() **********************************************
*  NAME
*     sge_sl_get_mutex() -- returns the list mutex 
*
*  SYNOPSIS
*     pthread_mutex_t * sge_sl_get_mutex(sge_sl_list_t *list) 
*
*  FUNCTION
*     retrns the list mutex 
*
*  INPUTS
*     sge_sl_list_t *list - list 
*
*  RESULT
*     pthread_mutex_t * - mutex used in the list to secure actions
*
*  NOTES
*     MT-NOTE: sge_sl_get_mutex() is MT safe 
*******************************************************************************/
pthread_mutex_t *
sge_sl_get_mutex(sge_sl_list_t *list) {
   pthread_mutex_t *mutex = NULL; 
   
   DENTER(SL_LAYER, "sge_sl_get_mutex");
   if (list != NULL) {
      mutex = &list->mutex;
   }
   DRETURN(mutex);
}
