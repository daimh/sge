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
#include "comm/cl_host_list.h"
#include "comm/cl_host_alias_list.h"
#include "comm/cl_commlib.h"
#include "uti/sge_hostname.h"

static struct in_addr*   cl_com_copy_in_addr(struct in_addr *in_addr);
static cl_com_hostent_t* cl_com_copy_hostent(cl_com_hostent_t* hostent);

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_setup()"
int cl_host_list_setup(cl_raw_list_t** list_p, 
                       char* list_name,
                       cl_host_resolve_method_t method, 
                       char* host_alias_file, 
                       char* local_domain_name,
                       unsigned long entry_life_time,
                       unsigned long entry_update_time,
                       unsigned long entry_reresolve_time,
                       bool create_hash) {
   int ret_val = CL_RETVAL_OK;
   cl_host_list_data_t* ldata = NULL;

   ldata = (cl_host_list_data_t*) malloc(sizeof(cl_host_list_data_t));
   if (ldata == NULL ) {
      return CL_RETVAL_MALLOC;
   }
   ldata->host_alias_file      = NULL;
   ldata->alias_file_changed   = 0;
   ldata->host_alias_list      = NULL;
   ldata->resolve_method       = method;
   ldata->entry_life_time      = entry_life_time;
   ldata->entry_update_time    = entry_update_time;
   ldata->entry_reresolve_time = entry_reresolve_time;
   ldata->last_refresh_time    = 0;

   if (local_domain_name == NULL && method == CL_LONG) {
      CL_LOG(CL_LOG_WARNING,"can't compare short host names without default domain when method is CL_LONG");
   }


   if (entry_life_time == 0) {
      unsigned long help_value = 0;

      help_value = cl_util_get_ulong_value(getenv("SGE_COMMLIB_HOST_LIST_LIFE_TIME"));
      if (help_value > 0) {
         CL_LOG(CL_LOG_INFO,"environment variable SGE_COMMLIB_HOST_LIST_LIFE_TIME is set");
         ldata->entry_life_time = help_value;
      } else {
         CL_LOG(CL_LOG_INFO,"using default value for entry_life_time");
         ldata->entry_life_time = CL_HOST_LIST_DEFAULT_LIFE_TIME;
      }
   }

   if (entry_update_time == 0) {
      unsigned long help_value = 0;

      help_value = cl_util_get_ulong_value(getenv("SGE_COMMLIB_HOST_LIST_UPDATE_TIME"));
      if (help_value > 0) {
         CL_LOG(CL_LOG_INFO,"environment variable SGE_COMMLIB_HOST_LIST_UPDATE_TIME is set");
         ldata->entry_update_time = help_value;
      } else {
         CL_LOG(CL_LOG_INFO,"using default value for entry_update_time");
         ldata->entry_update_time = CL_HOST_LIST_DEFAULT_UPDATE_TIME;
      }
   }

   if (entry_reresolve_time == 0) {
      unsigned long help_value = 0;

      help_value = cl_util_get_ulong_value(getenv("SGE_COMMLIB_HOST_LIST_RERESOLVE_TIME"));
      if (help_value > 0) {
         CL_LOG(CL_LOG_INFO,"environment variable SGE_COMMLIB_HOST_LIST_RERESOLVE_TIME is set");
         ldata->entry_reresolve_time = help_value;
      } else {
         CL_LOG(CL_LOG_INFO,"using default value for entry_reresolve_time");
         ldata->entry_reresolve_time = CL_HOST_LIST_DEFAULT_RERESOLVE_TIME;
      }
   }

   if ( ldata->entry_life_time > CL_HOST_LIST_MAX_LIFE_TIME) {
      CL_LOG_INT(CL_LOG_WARNING,"entry_life_time exceeds maximum of",CL_HOST_LIST_MAX_LIFE_TIME);
      CL_LOG(CL_LOG_WARNING,"using default value for entry_life_time");
      ldata->entry_life_time = CL_HOST_LIST_DEFAULT_LIFE_TIME;
   }

   if ( ldata->entry_update_time > CL_HOST_LIST_MAX_UPDATE_TIME) {
      CL_LOG_INT(CL_LOG_WARNING,"entry_update_time exceeds maximum of",CL_HOST_LIST_MAX_UPDATE_TIME);
      CL_LOG(CL_LOG_WARNING,"using default value for entry_update_time");
      ldata->entry_update_time = CL_HOST_LIST_DEFAULT_UPDATE_TIME;
   }

   if ( ldata->entry_reresolve_time > CL_HOST_LIST_MAX_RERESOLVE_TIME) {
      CL_LOG_INT(CL_LOG_WARNING,"entry_reresolve_time exceeds maximum of",CL_HOST_LIST_MAX_RERESOLVE_TIME);
      CL_LOG(CL_LOG_WARNING,"using default value for entry_reresolve_time");
      ldata->entry_reresolve_time = CL_HOST_LIST_DEFAULT_RERESOLVE_TIME;
   }

   if (ldata->entry_life_time <= ldata->entry_update_time || ldata->entry_life_time <= ldata->entry_reresolve_time) {
      sge_free(&ldata); 
      CL_LOG(CL_LOG_ERROR,"entry_life_time must be >= entry_update_time and >= entry_reresolve_time");
      cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_PARAMS, "SGE_COMMLIB_HOST_LIST_LIFE_TIME must be >= SGE_COMMLIB_HOST_LIST_UPDATE_TIME and >= SGE_COMMLIB_HOST_LIST_RERESOLVE_TIME");
      return CL_RETVAL_PARAMS;
   }
   if (ldata->entry_update_time <= ldata->entry_reresolve_time) {
      sge_free(&ldata); 
      CL_LOG(CL_LOG_ERROR,"entry_update_time must be >= entry_reresolve_time");
      cl_commlib_push_application_error(CL_LOG_ERROR, CL_RETVAL_PARAMS, "SGE_COMMLIB_HOST_LIST_UPDATE_TIME must be >= SGE_COMMLIB_HOST_LIST_RERESOLVE_TIME");
      return CL_RETVAL_PARAMS;
   }

   ret_val = cl_host_alias_list_setup(&(ldata->host_alias_list), "host alias list");
   if (ret_val != CL_RETVAL_OK) {
      sge_free(&ldata);
      CL_LOG(CL_LOG_ERROR,"error setting up host alias list");
      return ret_val;
   }

   if (host_alias_file != NULL) {
      ldata->host_alias_file = strdup(host_alias_file);
      ldata->alias_file_changed = 1;
      if (ldata->host_alias_file == NULL) {
         sge_free(&ldata);
         return CL_RETVAL_MALLOC;
      }
   } else {
      ldata->host_alias_file = NULL;
   }

   if (local_domain_name != NULL) {
      ldata->local_domain_name = strdup(local_domain_name);
      if (ldata->local_domain_name == NULL) {
         if (ldata->host_alias_file != NULL) {
            sge_free(&(ldata->host_alias_file));
         }
         sge_free(&ldata);
         return CL_RETVAL_MALLOC;
      }
   } else {
      ldata->local_domain_name = NULL;
   }

   

   ret_val = cl_raw_list_setup(list_p,list_name, 1);
   if (ret_val != CL_RETVAL_OK) {
      if (ldata->host_alias_file != NULL) {
         sge_free(&(ldata->host_alias_file));
      }
      if (ldata->local_domain_name != NULL) {
         sge_free(&(ldata->local_domain_name));
      }
      sge_free(&ldata);
      return ret_val;
   }

   switch(ldata->resolve_method) {
      case CL_SHORT:
         CL_LOG(CL_LOG_INFO,"using short hostname for host compare operations");
         break;

      case CL_LONG:
         CL_LOG(CL_LOG_INFO,"using long hostname for host compare operations");
         break;

      default:
         CL_LOG(CL_LOG_WARNING,"undefined resolving method");
         break;
   }
 
   if (ldata->host_alias_file != NULL) {
      CL_LOG_STR(CL_LOG_INFO,"using host alias file:", ldata->host_alias_file);
   } else {
      CL_LOG(CL_LOG_INFO,"no host alias file specified");
   }
   if (ldata->local_domain_name != NULL) {
      CL_LOG_STR(CL_LOG_INFO,"using local domain name:", ldata->local_domain_name);
   } else {
      CL_LOG(CL_LOG_INFO,"no local domain specified");
   }

   /* create hashtable */
   if (create_hash == true) {
      ldata->ht = sge_htable_create(4, dup_func_string, hash_func_string, hash_compare_string);
      if (ldata->ht == NULL) {
         cl_raw_list_cleanup(list_p);
         if (ldata->host_alias_file != NULL) {
            sge_free(&(ldata->host_alias_file));
         }
         if (ldata->local_domain_name != NULL) {
            sge_free(&(ldata->local_domain_name));
         }
         sge_free(&ldata);
         return CL_RETVAL_MALLOC;
      }
      CL_LOG_INT(CL_LOG_INFO,"created hash table with size =", 4);
   } else {
      CL_LOG(CL_LOG_INFO,"created NO hash table!");
      ldata->ht = NULL;
   }

   /* set private list data */
   (*list_p)->list_data = ldata;

   CL_LOG_INT(CL_LOG_INFO,"entry_life_time is", (int)ldata->entry_life_time);
   CL_LOG_INT(CL_LOG_INFO,"entry_update_time is", (int)ldata->entry_update_time);
   CL_LOG_INT(CL_LOG_INFO,"entry_reresolve_time is", (int)ldata->entry_reresolve_time);

   return ret_val;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_copy()"
int cl_host_list_copy(cl_raw_list_t** destination, cl_raw_list_t* source, bool create_hash) {
   int ret_val = CL_RETVAL_OK;
   cl_host_list_data_t* ldata_source = NULL;
   cl_host_list_data_t* ldata_dest = NULL;
   cl_host_alias_list_elem_t* alias_elem = NULL;
   cl_host_list_elem_t* host_elem = NULL;

   if (source == NULL) {
      return CL_RETVAL_PARAMS;
   }  

   ret_val = cl_raw_list_lock(source);
   if (ret_val != CL_RETVAL_OK) {
      return ret_val;
   }

   /* create a new host list */
   ldata_source = (cl_host_list_data_t*) source->list_data;
   if (ldata_source != NULL) {
      ret_val = cl_host_list_setup(destination, 
                                   source->list_name,
                                   ldata_source->resolve_method,
                                   ldata_source->host_alias_file,
                                   ldata_source->local_domain_name,
                                   ldata_source->entry_life_time,
                                   ldata_source->entry_update_time,
                                   ldata_source->entry_reresolve_time,
                                   create_hash);
   } else {
      CL_LOG(CL_LOG_ERROR,"not list data specified");
      ret_val = CL_RETVAL_UNKNOWN;
   }

   if (ret_val != CL_RETVAL_OK) {
      cl_raw_list_unlock(source);
      cl_host_list_cleanup(destination);
      return ret_val;
   }

   /* list created, now get private data structures */ 
   ldata_dest = (cl_host_list_data_t*) (*destination)->list_data;

   ldata_dest->alias_file_changed = ldata_source->alias_file_changed;
   ldata_dest->last_refresh_time  = ldata_source->last_refresh_time;

   /* now copy alias list */
   cl_raw_list_lock(ldata_source->host_alias_list);

   alias_elem = cl_host_alias_list_get_first_elem(ldata_source->host_alias_list);
   while(alias_elem) {
      ret_val = cl_host_alias_list_append_host(ldata_dest->host_alias_list, 
                                               alias_elem->local_resolved_hostname,
                                               alias_elem->alias_name, 0);
      if (ret_val != CL_RETVAL_OK) {
         cl_raw_list_unlock(ldata_source->host_alias_list);
         cl_raw_list_unlock(source);
         cl_host_list_cleanup(destination);
         return ret_val;
      }
      alias_elem = cl_host_alias_list_get_next_elem(alias_elem);
   }
   cl_raw_list_unlock(ldata_source->host_alias_list);

   /* ok, now copy the entries */
   host_elem = cl_host_list_get_first_elem(source);
   while(host_elem) {
      cl_com_host_spec_t* new_host_spec = NULL;
      
      new_host_spec = ( cl_com_host_spec_t*) malloc( sizeof(cl_com_host_spec_t) );
      if (new_host_spec == NULL) {
         cl_raw_list_unlock(source);
         cl_host_list_cleanup(destination);
         return CL_RETVAL_MALLOC;
      }

      /* copy host_spec_ type */
      new_host_spec->resolve_error     = host_elem->host_spec->resolve_error;
      new_host_spec->last_resolve_time = host_elem->host_spec->last_resolve_time;
      new_host_spec->creation_time     = host_elem->host_spec->creation_time;
       
      if ( host_elem->host_spec->resolved_name ) {
         new_host_spec->resolved_name = strdup(host_elem->host_spec->resolved_name);
         if ( new_host_spec->resolved_name == NULL) {
            cl_com_free_hostspec(&new_host_spec);
            cl_raw_list_unlock(source);
            cl_host_list_cleanup(destination);
            return CL_RETVAL_MALLOC;
         }
      } else {
         new_host_spec->resolved_name = NULL;
      }

      if ( host_elem->host_spec->unresolved_name ) {
         new_host_spec->unresolved_name = strdup(host_elem->host_spec->unresolved_name);
         if ( new_host_spec->unresolved_name == NULL) {
            cl_com_free_hostspec(&new_host_spec);
            cl_raw_list_unlock(source);
            cl_host_list_cleanup(destination);
            return CL_RETVAL_MALLOC;
         }
      } else {
         new_host_spec->unresolved_name = NULL;
      }

      if ( host_elem->host_spec->in_addr) {
         new_host_spec->in_addr = cl_com_copy_in_addr(host_elem->host_spec->in_addr);
         if ( new_host_spec->in_addr == NULL) {
            cl_com_free_hostspec(&new_host_spec);
            cl_raw_list_unlock(source);
            cl_host_list_cleanup(destination);
            return CL_RETVAL_MALLOC;
         }
      } else {
         new_host_spec->in_addr = NULL;
      }
 
      if ( host_elem->host_spec->hostent) {
         new_host_spec->hostent = cl_com_copy_hostent(host_elem->host_spec->hostent);
         if ( new_host_spec->hostent == NULL) {
            cl_com_free_hostspec(&new_host_spec);
            cl_raw_list_unlock(source);
            cl_host_list_cleanup(destination);
            return CL_RETVAL_MALLOC;
         }
      } else {
         new_host_spec->hostent = NULL;
      }

      cl_host_list_append_host((*destination), new_host_spec, 0);
      host_elem = cl_host_list_get_next_elem(host_elem);
   }
   
   ret_val = cl_raw_list_unlock( source );
   return ret_val;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_data()"
cl_host_list_data_t* cl_host_list_get_data(cl_raw_list_t* list_p) {

   cl_host_list_data_t* ldata = NULL;
   cl_raw_list_t* hostlist = NULL;

   if (list_p == NULL) {
      hostlist = cl_com_get_host_list();
   } else {
      hostlist = list_p;
   }
   if (hostlist == NULL) {
      CL_LOG(CL_LOG_WARNING,"no global hostlist");
      return NULL;
   }

   ldata = (cl_host_list_data_t*) hostlist->list_data;
   return ldata;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_set_alias_file_dirty()"
int cl_host_list_set_alias_file_dirty(cl_raw_list_t* list_p) {
   int ret_val;
   cl_host_list_data_t* ldata = NULL;

   if (list_p == NULL ) {
      return CL_RETVAL_PARAMS;
   }
   
   /* lock host list */
   ret_val = cl_raw_list_lock(list_p);
   if (ret_val != CL_RETVAL_OK) {
      return ret_val;
   }

   /* list_p should be a hostlist */
   ldata = (cl_host_list_data_t*) list_p->list_data;
   if (ldata != NULL) {
      ldata->alias_file_changed = 1;
   } else {
      cl_raw_list_unlock(list_p);
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   /* unlock host list */
   ret_val = cl_raw_list_unlock(list_p);
   if (ret_val != CL_RETVAL_OK) {
      return ret_val;
   }
   return CL_RETVAL_OK;
}




#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_set_alias_file()"
int cl_host_list_set_alias_file(cl_raw_list_t* list_p, const char *host_alias_file) {
   int ret_val;
   cl_host_list_data_t* ldata = NULL;

   if (list_p == NULL || host_alias_file == NULL) {
      return CL_RETVAL_PARAMS;
   }
   
   /* lock host list */
   ret_val = cl_raw_list_lock(list_p);
   if (ret_val != CL_RETVAL_OK) {
      return ret_val;
   }

   /* list_p should be a hostlist */
   ldata = (cl_host_list_data_t*) list_p->list_data;
   if (ldata != NULL) {
      if (ldata->host_alias_file != NULL) {
         sge_free(&(ldata->host_alias_file));
         ldata->host_alias_file = NULL;
      }
      ldata->host_alias_file = strdup(host_alias_file);
      CL_LOG_STR(CL_LOG_INFO,"using host alias file:",ldata->host_alias_file);
      ldata->alias_file_changed = 1;
      if (ldata->host_alias_file == NULL) {
         cl_raw_list_unlock(list_p);
         return CL_RETVAL_MALLOC;
      }
   } else {
      cl_raw_list_unlock(list_p);
      return CL_RETVAL_NO_FRAMEWORK_INIT;
   }

   /* unlock host list */
   ret_val = cl_raw_list_unlock(list_p);
   if (ret_val != CL_RETVAL_OK) {
      return ret_val;
   }
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_cleanup()"
int cl_host_list_cleanup(cl_raw_list_t** list_p) {
   cl_host_list_data_t* ldata = NULL;
   cl_host_list_elem_t* elem = NULL;
   
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
   while ( (elem = cl_host_list_get_first_elem(*list_p)) != NULL) {
      cl_raw_list_remove_elem(*list_p, elem->raw_elem);
      cl_com_free_hostspec(&( elem->host_spec ));
      sge_free(&elem);
   }
   cl_raw_list_unlock(*list_p);

   /* clean list private data */
   ldata = (*list_p)->list_data;
   if (ldata != NULL) {
      if (ldata->ht != NULL) {
         sge_htable_destroy(ldata->ht);
      }
      cl_host_alias_list_cleanup(&(ldata->host_alias_list));
      if (ldata->local_domain_name != NULL) {
         sge_free(&(ldata->local_domain_name));
      }
      if (ldata->host_alias_file != NULL) {
         sge_free(&(ldata->host_alias_file));
      }
      sge_free(&ldata);
   }
   (*list_p)->list_data = NULL;

   return cl_raw_list_cleanup(list_p);
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_append_host()"
int cl_host_list_append_host(cl_raw_list_t* list_p,cl_com_host_spec_t* host, int lock_list ) {

   int ret_val;
   cl_host_list_elem_t* new_elem = NULL;

   if (host == NULL || list_p == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock the list */
   if (lock_list == 1) {
      if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   /* add new element list */
   new_elem = (cl_host_list_elem_t*) malloc(sizeof(cl_host_list_elem_t));
   if (new_elem == NULL) {
      if (lock_list == 1) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->host_spec = host;
   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      sge_free(&new_elem);
      if (lock_list == 1) { 
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   /* add element to hash table */
   if (host->unresolved_name != NULL) {
      cl_host_list_data_t* ldata = list_p->list_data;
      if (ldata->ht != NULL) {
         sge_htable_store(ldata->ht, host->unresolved_name, new_elem);
      }
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
#define __CL_FUNCTION__ "cl_host_list_remove_host()"
int cl_host_list_remove_host(cl_raw_list_t* list_p, cl_com_host_spec_t* host, int lock_list) {
   int ret_val = CL_RETVAL_OK;
   int function_return = CL_RETVAL_UNKOWN_HOST_ERROR;
   cl_host_list_elem_t* elem = NULL;
   
   if (list_p == NULL || host == NULL) {
      return CL_RETVAL_PARAMS;
   }

   if (lock_list != 0) {   
      /* lock list */
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

    elem = cl_host_list_get_elem_host(list_p, host->unresolved_name);
    if (elem != NULL) {

         /* remove element from hash table */
         if (host->unresolved_name != NULL) {
            cl_host_list_data_t* ldata = list_p->list_data;
            if (ldata->ht != NULL) {
               sge_htable_delete(ldata->ht, host->unresolved_name);
            }
         }
   
         cl_raw_list_remove_elem(list_p, elem->raw_elem);
         function_return = CL_RETVAL_OK;
         cl_com_free_hostspec(&(elem->host_spec));
         sge_free(&elem);
    }

   if (lock_list != 0) {
      /* unlock list */
      if ((ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return function_return;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_elem_host()"
cl_host_list_elem_t* cl_host_list_get_elem_host(cl_raw_list_t* list_p, const char *unresolved_hostname) {
   cl_host_list_elem_t *elem = NULL;

   if (list_p != NULL && unresolved_hostname != NULL) {
      cl_host_list_data_t* ldata = list_p->list_data;
      if (ldata->ht != NULL) {
         if (sge_htable_lookup(ldata->ht, unresolved_hostname, (const void **)&elem) == True) {
            return elem;
         }
      } else {
         /* Search without having hash table */
         CL_LOG(CL_LOG_INFO,"no hash table available, searching sequential");
         elem = cl_host_list_get_first_elem(list_p);
         while ( elem != NULL) {
            if (elem->host_spec != NULL && elem->host_spec->unresolved_name != NULL ) {
               if (strcmp(elem->host_spec->unresolved_name,unresolved_hostname) == 0) {
                  /* found matching element */
                  return elem;
               }
            }
            elem = cl_host_list_get_next_elem(elem);
         }
      }
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_first_elem()"
cl_host_list_elem_t* cl_host_list_get_first_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_host_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_least_elem()"
cl_host_list_elem_t* cl_host_list_get_least_elem(cl_raw_list_t* list_p) {
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_host_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_next_elem()"
cl_host_list_elem_t* cl_host_list_get_next_elem(cl_host_list_elem_t* elem) {
   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_host_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_host_list_get_last_elem()"
cl_host_list_elem_t* cl_host_list_get_last_elem(cl_host_list_elem_t* elem) {
   cl_raw_list_elem_t* last_raw_elem = NULL;
   

   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_host_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_copy_in_addr()"
static struct in_addr* cl_com_copy_in_addr(struct in_addr *addr) {
   struct in_addr* copy = NULL;    
   
   if (addr == NULL) {
      return NULL;
   }

   copy = (struct in_addr*) malloc(sizeof(struct in_addr));
   if (copy != NULL) {
      memcpy((char*) copy , addr, sizeof(struct in_addr));
   }
   return copy;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_com_copy_hostent()"
static cl_com_hostent_t* cl_com_copy_hostent(cl_com_hostent_t* hostent) {
   cl_com_hostent_t* copy = NULL;

   if (hostent == NULL) {
      return NULL;
   }

   copy = (cl_com_hostent_t*)malloc(sizeof(cl_com_hostent_t));
   if (copy != NULL) {
      copy->he = NULL;

      if (hostent->he != NULL) {
         copy->he = sge_copy_hostent(hostent->he);
         if (copy->he == NULL ) {
            CL_LOG(CL_LOG_ERROR,"could not copy hostent structure");
            sge_free(&copy);
            return NULL;
         }
      } 
   }
   return copy;
}

