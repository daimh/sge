#ifndef __CL_APP_MESSAGE_QUEUE_H
#define __CL_APP_MESSAGE_QUEUE_H

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

typedef struct cl_app_message_queue_elem_t {

   /* when used as received_message_queue */
   cl_com_connection_t*  rcv_connection;  

   /* when used as send_message_queue */
   cl_com_endpoint_t*    snd_destination;
   cl_xml_ack_type_t     snd_ack_type;
   cl_byte_t*            snd_data;
   unsigned long         snd_size;
   unsigned long         snd_response_mid;
   unsigned long         snd_tag;
   
   /* common data */
   cl_raw_list_elem_t*   raw_elem;
} cl_app_message_queue_elem_t;


/* basic functions */
int cl_app_message_queue_setup(cl_raw_list_t** list_p, char* list_name, int enable_locking);
int cl_app_message_queue_cleanup(cl_raw_list_t** list_p);


/* thread list functions that will lock the list */
int cl_app_message_queue_append(cl_raw_list_t*        list_p,
                                cl_com_connection_t*  rcv_connection,
                                cl_com_endpoint_t*    snd_destination,
                                cl_xml_ack_type_t     snd_ack_type,
                                cl_byte_t*            snd_data,
                                unsigned long         snd_size,
                                unsigned long         snd_response_mid,
                                unsigned long         snd_tag,
                                int                   do_lock);
int cl_app_message_queue_remove(cl_raw_list_t* list_p, cl_com_connection_t* connection, int do_lock, bool remove_all_elements);


/* thread functions that will not lock the list */
cl_app_message_queue_elem_t* cl_app_message_queue_get_first_elem(cl_raw_list_t* list_p);
cl_app_message_queue_elem_t* cl_app_message_queue_get_least_elem(cl_raw_list_t* list_p);
cl_app_message_queue_elem_t* cl_app_message_queue_get_next_elem(cl_app_message_queue_elem_t* elem);
cl_app_message_queue_elem_t* cl_app_message_queue_get_last_elem(cl_app_message_queue_elem_t* elem);


#endif /* __CL_APP_MESSAGE_QUEUE_H */

