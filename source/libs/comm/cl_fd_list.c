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

#include "comm/lists/cl_util.h"
#include "comm/cl_fd_list.h"
#include "comm/cl_commlib.h"

int cl_fd_list_setup(cl_raw_list_t** list_p, char* list_name) {
   return cl_raw_list_setup(list_p, list_name, 1);
}

int cl_fd_list_cleanup(cl_raw_list_t **list_p) {
   cl_fd_list_elem_t* elem = NULL;

   elem = cl_fd_list_get_first_elem(*list_p);
   if (elem != NULL) {
      CL_LOG(CL_LOG_WARNING, "The list was NOT empty. Unregister all external file descriptors before cleanup next time, please");
   }

   while(elem){
      cl_fd_list_unregister_fd(*list_p, elem, 1);
      elem = cl_fd_list_get_next_elem(elem);
   }

   return cl_raw_list_cleanup(list_p);
}

int cl_fd_list_register_fd(cl_raw_list_t* list_p, cl_com_fd_data_t* fd, int lock_list) {
   
   int ret_val;
   cl_fd_list_elem_t* new_elem = NULL;

   if (fd == NULL || list_p == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock the list */
   if (lock_list == 1) {
      if ((ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   /* add a new elemt to the list */
   new_elem = (cl_fd_list_elem_t*) malloc(sizeof(cl_fd_list_elem_t));
   if (new_elem == NULL) {
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->data = fd;
   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if (new_elem->raw_elem == NULL) {
      sge_free(&new_elem);
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   /* unlock the list */
   if (lock_list == 1) {
      if ((ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return CL_RETVAL_OK;
}

int cl_fd_list_unregister_fd(cl_raw_list_t* list_p, cl_fd_list_elem_t* elem, int lock_list) {
   int ret_val = CL_RETVAL_OK;

   if (list_p == NULL || elem == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (lock_list != 0) {
      /* lock list */
      if ((ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   cl_raw_list_remove_elem(list_p, elem->raw_elem);
   sge_free(&(elem->data));
   sge_free(&elem);

   if (lock_list != 0) {
      /* unlock list */
      if ((ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return ret_val;
}

cl_fd_list_elem_t* cl_fd_list_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_fd_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

cl_fd_list_elem_t* cl_fd_list_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_fd_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

cl_fd_list_elem_t* cl_fd_list_get_next_elem(cl_fd_list_elem_t* elem) {
   cl_raw_list_elem_t* next_raw_elem = NULL;

   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_fd_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}

cl_fd_list_elem_t* cl_fd_list_get_last_elem(cl_fd_list_elem_t* elem) {
   cl_raw_list_elem_t* last_raw_elem = NULL;

   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_fd_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}
