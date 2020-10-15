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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "comm/lists/cl_string_list.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_string_list_setup()"
int cl_string_list_setup(cl_raw_list_t** list_p, char* list_name ) {
   int ret_val = CL_RETVAL_OK;
   ret_val = cl_raw_list_setup(list_p,list_name, 1);
   return ret_val;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_string_list_cleanup()"
int cl_string_list_cleanup(cl_raw_list_t** list_p) {
   cl_string_list_elem_t* elem = NULL;
   if (list_p == NULL) {
      /* we expect an address of an pointer */
      return CL_RETVAL_PARAMS;
   }
   if (*list_p == NULL) {
      /* we expect an initalized pointer */
      return CL_RETVAL_PARAMS;
   }

   /* delete all entries in list */
   cl_raw_list_lock(*list_p);
   while ( (elem = cl_string_list_get_first_elem(*list_p)) != NULL) {
      cl_raw_list_remove_elem(*list_p, elem->raw_elem);
      free(elem->string);
      free(elem);
   }
   cl_raw_list_unlock(*list_p);
   return cl_raw_list_cleanup(list_p);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_string_list_append_string()"
int cl_string_list_append_string(cl_raw_list_t* list_p,char* string, int lock_list ) {

   int ret_val;
   cl_string_list_elem_t* new_elem = NULL;

   if (string == NULL || list_p == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock the list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   /* add new element list */
   new_elem = (cl_string_list_elem_t*) malloc(sizeof(cl_string_list_elem_t));
   if (new_elem == NULL) {
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->string = strdup(string);
   if (new_elem->string == NULL) {
      free(new_elem);
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }
   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      free(new_elem->string);
      free(new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }
   
   /* unlock the list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return CL_RETVAL_OK;
}





int cl_string_list_remove_string(cl_raw_list_t* list_p, char* string, int lock_list ) {
   int ret_val = CL_RETVAL_OK;
   int function_return = CL_RETVAL_UNKOWN_HOST_ERROR;
   cl_string_list_elem_t* elem = NULL;
   
   if (list_p == NULL || string == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (lock_list != 0) {   
      /* lock list */
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   elem = cl_string_list_get_first_elem(list_p);
   while ( elem != NULL) { 
      if (strcmp(elem->string,string) == 0 ) {
         /* found matching element */
         cl_raw_list_remove_elem(list_p, elem->raw_elem);
         function_return = CL_RETVAL_OK;
         free(elem->string);
         free(elem);
         elem = NULL;
         break;
      }
      elem = cl_string_list_get_next_elem(elem);
   } 

   if (lock_list != 0) {
      /* unlock list */
      if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return function_return;
}


cl_string_list_elem_t* cl_string_list_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_string_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

cl_string_list_elem_t* cl_string_list_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_string_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

cl_string_list_elem_t* cl_string_list_get_next_elem(cl_string_list_elem_t* elem) {
   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_string_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


cl_string_list_elem_t* cl_string_list_get_last_elem(cl_string_list_elem_t* elem) {
   cl_raw_list_elem_t* last_raw_elem = NULL;
   

   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_string_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}

