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
#include <string.h>
#include <stdlib.h>

#include "comm/cl_host_alias_list.h"
#include "comm/cl_commlib.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_setup()"
int cl_host_alias_list_setup(cl_raw_list_t** list_p, char* list_name) {
   int ret_val = CL_RETVAL_OK;
   ret_val = cl_raw_list_setup(list_p, list_name, 1 );
   if (list_name != NULL) {
      CL_LOG_STR(CL_LOG_INFO,"host alias list setup ok for list:", list_name);
   }
   return ret_val;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_cleanup()"
int cl_host_alias_list_cleanup(cl_raw_list_t** list_p) {
   cl_host_alias_list_elem_t* elem = NULL;
   int ret_val = CL_RETVAL_OK;
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
   while ( (elem = cl_host_alias_list_get_first_elem(*list_p)) != NULL) {
      cl_raw_list_remove_elem(*list_p, elem->raw_elem);
      sge_free(&(elem->local_resolved_hostname));
      sge_free(&(elem->alias_name));
      sge_free(&elem);
   }
   cl_raw_list_unlock(*list_p);
   ret_val = cl_raw_list_cleanup(list_p);
   CL_LOG(CL_LOG_INFO,"host alias cleanup done");
   return ret_val;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_append_host()"
int cl_host_alias_list_append_host(cl_raw_list_t* list_p, char* local_resolved_name, char* alias_name, int lock_list) {

   cl_host_alias_list_elem_t* new_elem = NULL;
   int ret_val;
   char* help = NULL;

   if (list_p == NULL || local_resolved_name == NULL  || alias_name == NULL ) {
      return CL_RETVAL_PARAMS;
   }
 
   if ( cl_host_alias_list_get_alias_name(list_p, local_resolved_name, &help ) == CL_RETVAL_OK) {
      CL_LOG_STR(CL_LOG_ERROR,"alias for host exists already:", help);
      sge_free(&help);
      return CL_RETVAL_ALIAS_EXISTS;
   }

#if 0
   /* CR: 
    *
    * enable this code for 1:1 mapping or for virtual host mapping
    * (e.g. my_virtual_hostname real_host_name in alias file ) 
    *
    * DO NOT FORGET TO ALSO ENABLE CODE IN cl_com_cached_gethostbyname() 
    */

   if ( cl_host_alias_list_get_local_resolved_name(list_p, alias_name, &help ) == CL_RETVAL_OK) {
      CL_LOG_STR(CL_LOG_ERROR,"hostname for alias exists already:", help);
      sge_free(&help);
      return CL_RETVAL_ALIAS_EXISTS;
   }
#endif

   /* lock the list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
 
   /* add new element list */
   new_elem = (cl_host_alias_list_elem_t*) malloc(sizeof(cl_host_alias_list_elem_t));
   if (new_elem == NULL) {
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->local_resolved_hostname = strdup(local_resolved_name);
   if (new_elem->local_resolved_hostname == NULL) {
      sge_free(&new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->alias_name = strdup(alias_name);
   if (new_elem->alias_name == NULL) {
      sge_free(&(new_elem->local_resolved_hostname));
      sge_free(&new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      sge_free(&(new_elem->local_resolved_hostname));
      sge_free(&(new_elem->alias_name));
      sge_free(&new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

  
   /* unlock the thread list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_remove_host()"
int cl_host_alias_list_remove_host(cl_raw_list_t* list_p, cl_host_alias_list_elem_t* element, int lock_list) {
   cl_host_alias_list_elem_t* elem = NULL;
   int ret_val = CL_RETVAL_OK;
   int function_return = CL_RETVAL_UNKNOWN;

   if (  list_p == NULL || element == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (lock_list != 0) {   
      /* lock list */
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   
   elem = cl_host_alias_list_get_first_elem(list_p);
   while ( elem != NULL) { 
      if (elem == element) {
         /* found matching element */
         cl_raw_list_remove_elem(list_p, elem->raw_elem);
         function_return = CL_RETVAL_OK;
         sge_free(&(elem->local_resolved_hostname));
         sge_free(&(elem->alias_name));
         sge_free(&elem);
         elem = NULL;
         break;
      }
      elem = cl_host_alias_list_get_next_elem(elem);
   }

   if (lock_list != 0) {
      /* unlock list */
      if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return function_return;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_local_resolved_name()"
int cl_host_alias_list_get_local_resolved_name(cl_raw_list_t* list_p, char* alias_name,char** local_resolved_name ) {
   cl_host_alias_list_elem_t* elem = NULL;
   int ret_val;
   if   (list_p == NULL || alias_name == NULL || local_resolved_name == NULL)  {
      return CL_RETVAL_PARAMS;
   }
   if (*local_resolved_name != NULL) {
      CL_LOG(CL_LOG_ERROR,"need empty pointer pointer");
      return CL_RETVAL_PARAMS;
   }
   
   if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }


   elem = cl_host_alias_list_get_first_elem(list_p);
   while ( elem != NULL) { 
      if ( strcasecmp(alias_name,elem->alias_name) == 0) {
         *local_resolved_name = strdup(elem->local_resolved_hostname);
         if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
            sge_free(local_resolved_name);
            return ret_val;
         }
         if (*local_resolved_name == NULL) {
            return CL_RETVAL_MALLOC;
         }
         return CL_RETVAL_OK;
      }
      elem = cl_host_alias_list_get_next_elem(elem);
   }
   
   if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }
   return CL_RETVAL_UNKNOWN;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_alias_name()"
int cl_host_alias_list_get_alias_name(cl_raw_list_t* list_p, char* local_resolved_name, char** alias_name) {
   cl_host_alias_list_elem_t* elem = NULL;
   int ret_val;
   if (list_p == NULL || local_resolved_name == NULL || alias_name == NULL)  {
      return CL_RETVAL_PARAMS;
   }
   if (*alias_name != NULL) {
      CL_LOG(CL_LOG_ERROR,"need empty pointer pointer");
      return CL_RETVAL_PARAMS;
   }
   
   if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }

   elem = cl_host_alias_list_get_first_elem(list_p);
   while ( elem != NULL) { 
      if ( strcasecmp(local_resolved_name,elem->local_resolved_hostname) == 0) {
         *alias_name = strdup(elem->alias_name);
         if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
            sge_free(alias_name);
            return ret_val;
         }
         if (*alias_name == NULL) {
            return CL_RETVAL_MALLOC;
         }
         return CL_RETVAL_OK;
      }
      elem = cl_host_alias_list_get_next_elem(elem);
   }
   
   if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }
   return CL_RETVAL_UNKNOWN;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_first_elem()"
cl_host_alias_list_elem_t* cl_host_alias_list_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_host_alias_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_least_elem()"
cl_host_alias_list_elem_t* cl_host_alias_list_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_host_alias_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_next_elem()"
cl_host_alias_list_elem_t* cl_host_alias_list_get_next_elem(cl_host_alias_list_elem_t* elem) {
   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_host_alias_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_alias_list_get_last_elem()"
cl_host_alias_list_elem_t* cl_host_alias_list_get_last_elem(cl_host_alias_list_elem_t* elem) {
   cl_raw_list_elem_t* last_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_host_alias_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}


