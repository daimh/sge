#ifndef __CL_HOST_LIST_H
#define __CL_HOST_LIST_H

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
#include "uti/sge_htable.h"

#define CL_HOST_LIST_DEFAULT_RERESOLVE_TIME  1  * 60       /*  1 min */
#define CL_HOST_LIST_DEFAULT_UPDATE_TIME     2  * 60       /*  2 min */
#define CL_HOST_LIST_DEFAULT_LIFE_TIME      10  * 60       /* 10 min */ 

#define CL_HOST_LIST_MAX_RERESOLVE_TIME  10 * 60       /* 10 min */
#define CL_HOST_LIST_MAX_UPDATE_TIME     30 * 60       /* 30 min */
#define CL_HOST_LIST_MAX_LIFE_TIME       24 * 60 * 60  /*  1 day */

typedef struct cl_host_list_elem_t {
   cl_com_host_spec_t*   host_spec;     /* data */
   cl_raw_list_elem_t*   raw_elem;
} cl_host_list_elem_t;


typedef struct cl_host_list_data_type {                      /* list specific data */
   cl_host_resolve_method_t    resolve_method;
   char*                       host_alias_file;
   int                         alias_file_changed;      /* if set, alias file has changed */
   char*                       local_domain_name;
   cl_raw_list_t*              host_alias_list;
   unsigned long               entry_life_time;         /* max life time of an resolved host */
   unsigned long               entry_update_time;       /* max valid time of entry before reresolving */
   unsigned long               entry_reresolve_time;    /* time for reresolving if host is not resolvable */
   long                        last_refresh_time;       /* last refresh check */
   htable                      ht;                      /* hashtable for host_list */
} cl_host_list_data_t;


/* basic functions */
int cl_host_list_setup(cl_raw_list_t** list_p, 
                       char* list_name, 
                       cl_host_resolve_method_t method, 
                       char* host_alias_file,
                       char* local_domain_name,
                       unsigned long entry_life_time,         /* max life time of an resolved host */
                       unsigned long entry_update_time,       /* max valid time of entry before reresolving */
                       unsigned long entry_reresolve_time,    /* time for reresolving if host is not resolvable */
                       bool create_hash);                /* flag if list should create hash table */
int cl_host_list_cleanup(cl_raw_list_t** list_p);

int cl_host_list_copy(cl_raw_list_t** destination, cl_raw_list_t* source, bool create_hash);  /* make a copy of this list (will lock source) */

/* thread list functions that will lock the list */
int cl_host_list_append_host(cl_raw_list_t* list_p, cl_com_host_spec_t* newhost, int lock_list);
int cl_host_list_remove_host(cl_raw_list_t* list_p, cl_com_host_spec_t* delhost, int lock_list);
int cl_host_list_set_alias_file(cl_raw_list_t* list_p, const char *host_alias_file);
int cl_host_list_set_alias_file_dirty(cl_raw_list_t* list_p);


/* thread functions that will not lock the list */
cl_host_list_elem_t* cl_host_list_get_elem_host(cl_raw_list_t* list_p, const char *unresolved_hostname);
cl_host_list_data_t* cl_host_list_get_data(cl_raw_list_t* list_p);
cl_host_list_elem_t* cl_host_list_get_first_elem(cl_raw_list_t* list_p);
cl_host_list_elem_t* cl_host_list_get_least_elem(cl_raw_list_t* list_p);
cl_host_list_elem_t* cl_host_list_get_next_elem(cl_host_list_elem_t* elem);
cl_host_list_elem_t* cl_host_list_get_last_elem(cl_host_list_elem_t* elem);


#endif /* __CL_HOST_LIST_H */

