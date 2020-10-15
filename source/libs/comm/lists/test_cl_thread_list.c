
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
#include <sys/time.h>
#include <stdlib.h>

#include "comm/lists/cl_lists.h"

cl_raw_list_t* thread_list = NULL;

void *my_thread(void *t_conf);
void *my_thread_test(void *t_conf);



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(void)
{
  cl_thread_settings_t* thread_p = NULL;
  cl_thread_settings_t* dummy_thread_p = NULL;

  int count = 0;

  /* setup thread list */
  cl_thread_list_setup(&thread_list,"thread list");

  /* setup first thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p,NULL, "1st thread", 1, my_thread, NULL, NULL, CL_TT_USER1);

  /* setup second thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p, NULL, "2nd thread", 2, my_thread, NULL, NULL, CL_TT_USER1);

  thread_p = cl_thread_list_get_thread_by_id(thread_list, 1);
  printf("first thread (%s) has id: %d\n", thread_p->thread_name, thread_p->thread_id);

  thread_p = cl_thread_list_get_thread_by_name(thread_list, "2nd thread");
  printf("second thread (%s) has id: %d\n" , thread_p->thread_name, thread_p->thread_id); 

  while (count++ < 10) {
     int id;
     
     printf("we have %ld threads.\n", cl_raw_list_get_elem_count(thread_list));

     thread_p = cl_thread_list_get_first_thread(thread_list);
     id = thread_p->thread_id;

     printf("-----------> delete thread %d ...\n", id);
     cl_thread_list_delete_thread_by_id(thread_list, id);
     printf("<----------- delete thread %d done\n", id);

     printf("-----------> add thread ...\n");
     cl_thread_list_create_thread(thread_list, &dummy_thread_p, NULL,"new thread", id, my_thread, NULL, NULL, CL_TT_USER1);
     printf("<----------- add thread done\n");

  }

  /* delete all threads */
  while ( (thread_p=cl_thread_list_get_first_thread(thread_list)) != NULL ) {
     int id = thread_p->thread_id;
     printf("-----------> delete thread %d ...\n",id );

     cl_thread_list_delete_thread(thread_list, thread_p);
     printf("<----------- delete thread %d done\n",id );

  }

  cl_thread_list_cleanup(&thread_list);
  printf("main done\n");
  return 0;
}



#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_thread()"
void *my_thread(void *t_conf) {
   cl_thread_settings_t* thread_p = NULL;
   int counter = 0;
   int do_exit = 0;
   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 

   /* setup thread */
   printf("thread %d: initialize\n", thread_config->thread_id);



   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);


   printf("thread %d: enter mainloop\n", thread_config->thread_id);

   /* ok, thread main */
   while (do_exit == 0) {
      int ret_val;

      printf("thread %d: try locking thread_list ...\n", thread_config->thread_id);
 
      /* check for cancel */
      cl_thread_func_testcancel(thread_config);
      if (cl_raw_list_lock(thread_list) == CL_RETVAL_OK) {
         int id;
         printf("thread %d: locked thread_list\n", thread_config->thread_id);

         if (thread_config->thread_id == 1) {
            thread_p = cl_thread_list_get_thread_by_id(thread_list,2);
            id = 2;
         } else {
            thread_p = cl_thread_list_get_thread_by_id(thread_list,1);
            id = 1;
         }
         if (thread_p) {
            if (thread_config->thread_id == 1) {
               cl_thread_trigger_event(thread_p);
               counter++;
               printf("thread %d: triggered %d events\n", thread_config->thread_id, counter); 
            }
         } else {
            printf("thread %d: thread %d not found\n", thread_config->thread_id,id);
         }
         printf("thread %d: unlocking thread_list\n", thread_config->thread_id);

         cl_raw_list_unlock(thread_list);
      }

      if ((ret_val = cl_thread_wait_for_event(thread_config,0,10000)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
/*               printf("thread %d: got timeout\n", thread_config->thread_id);  */
               break;
            default: {
               printf("thread %d: got error: %d\n", thread_config->thread_id, ret_val);
               do_exit = 1;
            }
         }
      }
   }

   printf("thread %d: exit\n", thread_config->thread_id);
   
   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


