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
#include <unistd.h>

#include "uti/sge_stdlib.h"

#include "comm/lists/cl_lists.h"

cl_raw_list_t* thread_list = NULL;

void *my_test_thread(void *t_conf);
void *my_log_thread(void *t_conf);

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_log_flush_list()"
int my_log_flush_list(cl_raw_list_t* list_p) {
   int ret_val;
   cl_log_list_elem_t* elem = NULL;
   
   
   if (list_p == NULL) {
      return CL_RETVAL_LOG_NO_LOGLIST;
   }

   if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   }

   while ( (elem = cl_log_list_get_first_elem(list_p) ) != NULL) {
      char* param;
      if (elem->log_parameter == NULL) {
         param = "";
      } else {
         param = elem->log_parameter;
      }
      printf("%-15s|%-10s|%s%s\n",elem->log_thread_name,cl_log_list_convert_type_id(elem->log_type),elem->log_message,param);
      cl_log_list_del_log(list_p);
   }
   
   if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
      return ret_val;
   } 
   return CL_RETVAL_OK;
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(int argc, char** argv)
{
  cl_raw_list_t* log_list = NULL;
  cl_thread_settings_t* thread_p = NULL;
  cl_thread_settings_t* log_thread = NULL;
  cl_thread_settings_t* dummy_thread_p = NULL;
  int count = 2;



  if (argc != 2) {
     printf("please enter 0 for standard log function, 1 for special\n");
     exit(1);
  }


  /* setup log list */
  if (atoi(argv[1]) == 0) {
     cl_log_list_setup(&log_list,"application",0,CL_LOG_IMMEDIATE, NULL);
  } else {
     cl_log_list_setup(&log_list,"application",0,CL_LOG_IMMEDIATE, my_log_flush_list);
  }

  /* setup log thread */
  log_thread = (cl_thread_settings_t*) malloc(sizeof(cl_thread_settings_t));
  cl_thread_setup(log_thread, log_list, "log thread", 1,my_log_thread, NULL, NULL, CL_TT_USER1);
  cl_log_list_set_log_level(log_list,CL_LOG_DEBUG );

  /* setup thread list */
  cl_thread_list_setup(&thread_list,"thread list");

  /* setup first thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p, log_list, "1st thread", 1, my_test_thread, NULL, NULL, CL_TT_USER1);

  /* setup second thread */
  cl_thread_list_create_thread(thread_list, &dummy_thread_p, log_list, "2nd thread", 2, my_test_thread, NULL, NULL, CL_TT_USER1);

  
  thread_p = cl_thread_list_get_thread_by_id(thread_list, 1);
  CL_LOG_STR( CL_LOG_INFO,  "My thread name is ", thread_p->thread_name  );

  thread_p = cl_thread_list_get_thread_by_name(thread_list, "2nd thread");
  CL_LOG_STR( CL_LOG_INFO,  "My thread name is ", thread_p->thread_name  );

  while ( count  < 10  ) {
     int id;
     char new_thread_name[255];

     count++;

     CL_LOG_INT(CL_LOG_INFO,  "number of threads: ", (int)cl_raw_list_get_elem_count(thread_list) );

     thread_p = cl_thread_list_get_first_thread(thread_list);
     id = thread_p->thread_id;

     CL_LOG_INT( CL_LOG_INFO,  "delete thread: ", id );
     cl_thread_list_delete_thread_by_id(thread_list, id);
     CL_LOG_INT(CL_LOG_INFO,  "thread deleted, id: ", id );


     sprintf(new_thread_name,"thread nr %d", count);
     CL_LOG( CL_LOG_INFO,  "adding thread ...");
     cl_thread_list_create_thread(thread_list,&dummy_thread_p, log_list,new_thread_name, id, my_test_thread, NULL, NULL, CL_TT_USER1);
     CL_LOG( CL_LOG_INFO, "adding thread done");

  }

  
  /* remove all threads from thread list */
  while ( (thread_p=cl_thread_list_get_first_thread(thread_list)) != NULL ) {
     int id = thread_p->thread_id;
     CL_LOG_INT( CL_LOG_INFO,  "delete thread: ", id );
     CL_LOG_INT( CL_LOG_INFO,  "event calls: ", (int)thread_p->thread_event_count );

     cl_thread_list_delete_thread_by_id(thread_list, id);
     CL_LOG_INT( CL_LOG_INFO,  "thread deleted, id: ", id );
  }


  CL_LOG_INT( CL_LOG_INFO,  "log event calls: ", (int)log_thread->thread_event_count );

  CL_LOG( CL_LOG_INFO,  "cleaning up thread list");
  cl_thread_list_cleanup(&thread_list);

  /* shutdown of log thread */
  cl_thread_shutdown(log_thread);
  cl_thread_join(log_thread);
  cl_thread_cleanup(log_thread);
  sge_free(&log_thread);

  /* cleanup log list */
  CL_LOG( CL_LOG_INFO,  "cleaning up log list");
  printf( "cl_log_list_cleanup() returned: %s\n", cl_get_error_text(cl_log_list_cleanup(&log_list)));
  printf("main done\n");










  return 0;
}


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_log_thread()" 
void *my_log_thread(void *t_conf) {
   int do_exit = 0;
   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 


   /* setup thread config ( at least done by call to cl_thread_func_startup() ) */
   if ( cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      printf("cl_thread_set_thread_config() error\n");
   }


   CL_LOG( CL_LOG_INFO,   "starting initialization ...");
   /* setup thread  begin */

   /* enter setup code here ... */

   /* setup thread  end */

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);

   CL_LOG( CL_LOG_INFO,   "init done ...");

   CL_LOG( CL_LOG_INFO,  "starting main loop ...");

   /* ok, thread main */
   while (do_exit == 0) {
      int ret_val;

      /* check for cancel */
      cl_thread_func_testcancel(thread_config);

      /* sleep till event arives */
      if ((ret_val = cl_thread_wait_for_event(thread_config,0,25000)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               break;
            default: {
               CL_LOG_INT(CL_LOG_INFO,  ">got error<: ", ret_val);
               do_exit = 1;
            }
         }
      }
   }

   /* this only happens on error */
   CL_LOG( CL_LOG_INFO,  "exiting ...");

   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "my_test_thread()"
void *my_test_thread(void *t_conf) {
   cl_thread_settings_t* thread_p = NULL;
   int counter = 0;
   int do_exit = 0;
   /* get pointer to cl_thread_settings_t struct */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 

   /* setup thread config ( at least done by call to cl_thread_func_startup() ) */
   if (cl_thread_set_thread_config(thread_config) != CL_RETVAL_OK) {
      printf("cl_thread_set_thread_config() error\n");
   }
   
   CL_LOG( CL_LOG_INFO,   "starting initialization ...");
   /* setup thread  begin */

   /* enter setup code here ... */

   /* setup thread  end */

   /* thread init done, trigger startup conditon variable*/
   cl_thread_func_startup(thread_config);

   CL_LOG( CL_LOG_INFO,  "starting main loop ...");

   /* ok, thread main */
   while (do_exit == 0) {
      int ret_val;

      /* check for cancel */
      cl_thread_func_testcancel(thread_config); 

      CL_LOG( CL_LOG_INFO,  "try to get thread list lock ...");
      if (cl_raw_list_lock(thread_list) == CL_RETVAL_OK) {
         int id;
         CL_LOG( CL_LOG_INFO,  "locked thread list");

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
               CL_LOG_INT( CL_LOG_INFO,  "events triggered: ", counter);
            }
         } else {
            CL_LOG_INT( CL_LOG_INFO,  "can't find thread : ", id);
         }
         CL_LOG( CL_LOG_INFO,  "unlocking thread list ...");
         cl_raw_list_unlock(thread_list);
      }

      /* sleep till event arives */
      if ((ret_val = cl_thread_wait_for_event(thread_config,0,10000)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
/*               printf("thread %d: got timeout\n", thread_config->thread_id);  */
               break;
            default: {
               CL_LOG_INT( CL_LOG_INFO,  ">got error<: ", ret_val);
               do_exit = 1;
            }
         }
      }
   }


   /* this only happens on error */
   CL_LOG( CL_LOG_INFO,  "exiting ...");

   /* at least set exit state */
   cl_thread_func_cleanup(thread_config);  
   return(NULL);
}


