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
#include <sys/time.h>
#include <stdlib.h>

#include "uti/sge_stdlib.h"

#include "comm/cl_app_message_queue.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_setup()"
int cl_app_message_queue_setup(cl_raw_list_t** list_p, char* list_name, int enable_locking) {
   int ret_val;
   ret_val = cl_raw_list_setup(list_p, list_name , enable_locking); 
   return ret_val;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_cleanup()"
int cl_app_message_queue_cleanup(cl_raw_list_t** list_p) {
   return cl_raw_list_cleanup(list_p);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_append()"
int cl_app_message_queue_append(cl_raw_list_t*        list_p,
                                cl_com_connection_t*  rcv_connection,
                                cl_com_endpoint_t*    snd_destination,
                                cl_xml_ack_type_t     snd_ack_type,
                                cl_byte_t*            snd_data,
                                unsigned long         snd_size,
                                unsigned long         snd_response_mid,
                                unsigned long         snd_tag,
                                int                   do_lock) {

   int ret_val;
   cl_app_message_queue_elem_t* new_elem = NULL;

   if (list_p == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* add new element list */
   new_elem = (cl_app_message_queue_elem_t*) malloc(sizeof(cl_app_message_queue_elem_t));
   if (new_elem == NULL) {
      return CL_RETVAL_MALLOC;
   }

   new_elem->rcv_connection   = rcv_connection;
   new_elem->snd_destination  = snd_destination;
   new_elem->snd_ack_type     = snd_ack_type;
   new_elem->snd_data         = snd_data;
   new_elem->snd_size         = snd_size;
   new_elem->snd_response_mid = snd_response_mid;
   new_elem->snd_tag          = snd_tag;
   new_elem->raw_elem         = NULL;

   /* lock the list */
   if (do_lock != 0) {
      if ((ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
	 sge_free(&new_elem);
         return ret_val;
      }
   }
   
   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      if (do_lock != 0) {
         cl_raw_list_unlock(list_p);
      }
      sge_free(&new_elem);
      return CL_RETVAL_MALLOC;
   }
   
   /* unlock the thread list */
   if (do_lock != 0) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return CL_RETVAL_OK;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_remove()"
int cl_app_message_queue_remove(cl_raw_list_t* list_p, cl_com_connection_t* connection, int do_lock, bool remove_all_elements ) {
   int function_return = CL_RETVAL_CONNECTION_NOT_FOUND;
   int ret_val = CL_RETVAL_OK;
   cl_app_message_queue_elem_t* elem = NULL;
   cl_app_message_queue_elem_t* next_elem = NULL;


   if (list_p == NULL || connection == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock list */
   if (do_lock != 0) {
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   elem = cl_app_message_queue_get_first_elem(list_p);
   while ( elem != NULL) { 
      next_elem = cl_app_message_queue_get_next_elem(elem);
      if (elem->rcv_connection == connection) {
         /* found matching element */
         cl_raw_list_remove_elem(list_p, elem->raw_elem);
         sge_free(&elem);
         function_return = CL_RETVAL_OK;
         if (remove_all_elements == false) {
             break; /* break here, we don't want to remove all elems */
         }
      }
      elem = next_elem;
   } 


   /* unlock the thread list */
   if (do_lock != 0) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   return function_return;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_get_first_elem()"
cl_app_message_queue_elem_t* cl_app_message_queue_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_app_message_queue_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_get_least_elem()"
cl_app_message_queue_elem_t* cl_app_message_queue_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_app_message_queue_elem_t*) raw_elem->data;
   }
   return NULL;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_get_next_elem()"
cl_app_message_queue_elem_t* cl_app_message_queue_get_next_elem(cl_app_message_queue_elem_t* elem) {

   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_app_message_queue_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_app_message_queue_get_last_elem()"
cl_app_message_queue_elem_t* cl_app_message_queue_get_last_elem(cl_app_message_queue_elem_t* elem) {

   cl_raw_list_elem_t* last_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_app_message_queue_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}

