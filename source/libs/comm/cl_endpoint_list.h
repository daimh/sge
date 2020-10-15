#ifndef __CL_ENDPOINT_LIST_H
#define __CL_ENDPOINT_LIST_H

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

#include "uti/sge_htable.h"

#include "comm/lists/cl_lists.h"
#include "comm/cl_data_types.h"

#define CL_ENDPOINT_LIST_DEFAULT_LIFE_TIME     60 * 60 * 24  /* 24 h   (without hearing anything from and endpoint) */
#define CL_ENDPOINT_LIST_DEFAULT_REFRESH_TIME  10            /*   1 s   (refresh list every 10 seconds */

typedef struct cl_endpoint_list_elem_type {
   cl_com_endpoint_t*            endpoint;     /* data */
   
   /* endpoint specific data (use no malloced pointers here, expect endpoint ) */
   int                           service_port;
   cl_xml_connection_autoclose_t autoclose;
   bool                     is_static;
   long                          last_used;

   /* list data */
   cl_raw_list_elem_t*           raw_elem;
} cl_endpoint_list_elem_t;


typedef struct cl_endpoint_list_data_type {          /* list specific data */
   long                        entry_life_time;         /* max life time of an endpoint */
   long                        refresh_interval;        /* refresh interval */
   long                        last_refresh_time;       /* last life time check */
   htable                      ht;                      /* endpoint list hash table */
} cl_endpoint_list_data_t;


/* basic functions */
int cl_endpoint_list_setup(cl_raw_list_t** list_p, 
                           char* list_name, 
                           long entry_life_time,           /* max life time of an endpoint */
                           long refresh_interval,          /* check interval */
                           bool create_hash);         /* flag if hash table should be used */

int cl_endpoint_list_cleanup(cl_raw_list_t** list_p);

/* thread list functions that will lock the list */
int cl_endpoint_list_define_endpoint(cl_raw_list_t* list_p, cl_com_endpoint_t* endpoint, int service_port, cl_xml_connection_autoclose_t autoclose ,bool is_static);
int cl_endpoint_list_undefine_endpoint(cl_raw_list_t* list_p, cl_com_endpoint_t* endpoint);
int cl_endpoint_list_get_last_touch_time(cl_raw_list_t* list_p, cl_com_endpoint_t* endpoint, unsigned long* touch_time);
int cl_endpoint_list_get_service_port(cl_raw_list_t* list_p, cl_com_endpoint_t* endpoint, int* service_port);
int cl_endpoint_list_get_autoclose_mode(cl_raw_list_t* list_p, cl_com_endpoint_t* endpoint, cl_xml_connection_autoclose_t* autoclose);
int cl_endpoint_list_set_entry_life_time(cl_raw_list_t* list_p, long entry_life_time );


/* thread functions that will not lock the list */
cl_endpoint_list_data_t* cl_endpoint_list_get_data(cl_raw_list_t* list_p);
cl_endpoint_list_elem_t* cl_endpoint_list_get_first_elem(cl_raw_list_t* list_p);
cl_endpoint_list_elem_t* cl_endpoint_list_get_least_elem(cl_raw_list_t* list_p);
cl_endpoint_list_elem_t* cl_endpoint_list_get_next_elem(cl_endpoint_list_elem_t* elem);
cl_endpoint_list_elem_t* cl_endpoint_list_get_last_elem(cl_endpoint_list_elem_t* elem);
cl_endpoint_list_elem_t* cl_endpoint_list_get_elem_endpoint(cl_raw_list_t* list_p, cl_com_endpoint_t *endpoint);


#endif /* __CL_ENDPOINT_LIST_H */

