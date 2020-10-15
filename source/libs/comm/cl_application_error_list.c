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

#include "comm/cl_application_error_list.h"
#include "comm/cl_commlib.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_setup()"
int cl_application_error_list_setup(cl_raw_list_t** list_p, char* list_name) {
   int ret_val = CL_RETVAL_OK;
   int ret_val2 = CL_RETVAL_OK;
   cl_raw_list_t* logged_error_list = NULL;

   ret_val = cl_raw_list_setup(list_p, list_name, 1 );
   if (ret_val == CL_RETVAL_OK) {
      cl_raw_list_lock(*list_p);
      ret_val2 = cl_raw_list_setup(&logged_error_list, "already logged data", 1);
      if (ret_val2 != CL_RETVAL_OK) {
         CL_LOG_STR(CL_LOG_ERROR,"error creating already logged data list:", cl_get_error_text(ret_val2));
         cl_application_error_list_cleanup(list_p);
         ret_val = ret_val2;
      } else {
         (*list_p)->list_data = (void*) logged_error_list;
         CL_LOG(CL_LOG_INFO,"created already logged data list");
      }
      cl_raw_list_unlock(*list_p);
   }
      
   if (list_name != NULL) {
      CL_LOG_STR(CL_LOG_INFO,"application error list setup ok for list:", list_name);
   }
   return ret_val;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_cleanup()"
int cl_application_error_list_cleanup(cl_raw_list_t** list_p) {
   cl_application_error_list_elem_t* elem = NULL;
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
 
   /* first delete list_data list */
   if ((*list_p)->list_data != NULL) {
      cl_raw_list_t* logged_error_list = NULL;
      logged_error_list = (cl_raw_list_t*) (*list_p)->list_data;
      CL_LOG(CL_LOG_INFO,"cleanup of already logged data list");
      cl_application_error_list_cleanup(&logged_error_list);
      (*list_p)->list_data = NULL;
   }

   while ( (elem = cl_application_error_list_get_first_elem(*list_p)) != NULL) {
      cl_raw_list_remove_elem(*list_p, elem->raw_elem);
      sge_free(&(elem->cl_info));
      sge_free(&elem);
   }
   cl_raw_list_unlock(*list_p);
   ret_val = cl_raw_list_cleanup(list_p);
   CL_LOG(CL_LOG_INFO,"application error list cleanup done");
   return ret_val;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_push_error()"
int cl_application_error_list_push_error(cl_raw_list_t* list_p, cl_log_t cl_err_type, int cl_error, const char* cl_info, int lock_list) {

   cl_application_error_list_elem_t* new_elem = NULL;
   cl_application_error_list_elem_t* al_list_elem = NULL;
   int ret_val;
   bool do_log = true;

   if (list_p == NULL || cl_info == NULL ) {
      return CL_RETVAL_PARAMS;
   }
 
   /* lock the list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   /* check if we should log this error */
   if (list_p->list_data != NULL) {
      cl_raw_list_t* logged_error_list = NULL;
      cl_application_error_list_elem_t* next_elem = NULL;
      struct timeval now;

      logged_error_list = (cl_raw_list_t*) list_p->list_data;

      if (lock_list == 1) {
         cl_raw_list_lock(logged_error_list);
      }
      gettimeofday(&now, NULL);

      al_list_elem = cl_application_error_list_get_first_elem(logged_error_list);
      while (al_list_elem != NULL) {
         next_elem = cl_application_error_list_get_next_elem(al_list_elem);

         if ( al_list_elem->cl_log_time.tv_sec + CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT <= now.tv_sec ) {
            CL_LOG_INT(CL_LOG_INFO,"removing error log from already logged list. linger time =", (int) (now.tv_sec - al_list_elem->cl_log_time.tv_sec));
            cl_raw_list_remove_elem(logged_error_list, al_list_elem->raw_elem);
            sge_free(&(al_list_elem->cl_info));
            sge_free(&al_list_elem);
            al_list_elem = NULL;
         }

         al_list_elem = next_elem;      
      }

      

      al_list_elem = cl_application_error_list_get_first_elem(logged_error_list);
      while (al_list_elem != NULL) {
         if (al_list_elem->cl_error == cl_error) {
            if (strcmp(al_list_elem->cl_info, cl_info) == 0) {
               do_log = false;
               break;
            }
         }
         al_list_elem = cl_application_error_list_get_next_elem(al_list_elem);
      }
      if (lock_list == 1) {
         cl_raw_list_unlock(logged_error_list);
      }
   }

   /* add new element into application error push list */
   new_elem = (cl_application_error_list_elem_t*) malloc(sizeof(cl_application_error_list_elem_t));
   if (new_elem == NULL) {
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->cl_info  = strdup(cl_info);
   new_elem->cl_error = cl_error;
   gettimeofday(&(new_elem->cl_log_time),NULL);
   new_elem->cl_already_logged = false;
   new_elem->cl_err_type = cl_err_type;

   if (do_log == false) {
      /* This error was logged the least CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT seconds (= he is in
       * already logged list, so we set the cl_already_logged flag */
      
      new_elem->cl_already_logged = true;
      CL_LOG_STR(CL_LOG_WARNING, "ignore application error - found entry in already logged list:", cl_get_error_text(cl_error)); 
      CL_LOG_STR(CL_LOG_WARNING, "ignore application error - found entry in already logged list:", cl_info); 
   } else {
      /* store this error into already logged error list */
      if (list_p->list_data != NULL) {
         cl_raw_list_t* logged_error_list = NULL;
         logged_error_list = (cl_raw_list_t*) list_p->list_data;
         cl_application_error_list_push_error(logged_error_list, cl_err_type, cl_error, cl_info, lock_list);
      }
   }


   if (new_elem->cl_info == NULL) {
      sge_free(&new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      sge_free(&(new_elem->cl_info));
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
#define __CL_FUNCTION__ "cl_application_error_list_get_first_elem()"
cl_application_error_list_elem_t* cl_application_error_list_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_application_error_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_get_least_elem()"
cl_application_error_list_elem_t* cl_application_error_list_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_application_error_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_get_next_elem()"
cl_application_error_list_elem_t* cl_application_error_list_get_next_elem(cl_application_error_list_elem_t* elem) {
   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_application_error_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_application_error_list_get_last_elem()"
cl_application_error_list_elem_t* cl_application_error_list_get_last_elem(cl_application_error_list_elem_t* elem) {
   cl_raw_list_elem_t* last_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_application_error_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}


