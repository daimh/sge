
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
#include <pthread.h>

#include "comm/lists/cl_lists.h"

void *timeout_thread_main(void *t_conf);  /* thread_func for timeout thread implementation */


#define THREAD_COUNT 10


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(void)
{
  int i;
  int retval;
  cl_thread_settings_t thread_list[THREAD_COUNT];

/*  timeout_thread = (cl_thread_settings_t*) malloc(sizeof(cl_thread_settings_t)); */

  for (i=0;i<THREAD_COUNT;i++) {
     char name[20];
     sprintf(name,"thread_%d_",i);
     if ( (retval=cl_thread_setup(&thread_list[i],NULL,name, i, timeout_thread_main, NULL, NULL, CL_TT_USER1)) != CL_RETVAL_OK) {
        printf("error: cl_thread_setup() - %d\n", retval);
     }
  }

  
  retval = cl_thread_shutdown(&thread_list[0]);  
  if (retval != CL_RETVAL_OK) {
     printf("error: cl_thread_shutdown() - %d\n", retval);
  }

  for (i=THREAD_COUNT/2;i<THREAD_COUNT;i++) {
     retval = cl_thread_trigger_event(&thread_list[i]);
     if (retval != CL_RETVAL_OK) {
        printf("error: cl_thread_trigger_event() - %d\n", retval);
     }

     retval = cl_thread_join(&thread_list[i]); 
     if (retval != CL_RETVAL_OK) {
        printf("error: cl_thread_join() - %d\n", retval);
     }
  }

  for (i=0;i<THREAD_COUNT;i++) {
     printf("main(): thread %s: state= %s\n",(&thread_list[i])->thread_name,cl_thread_get_state(&thread_list[i]));
  }
 
  for (i=0;i<(THREAD_COUNT/2);i++) {
     retval = cl_thread_join(&thread_list[i]);
     if (retval != CL_RETVAL_OK) {
        printf("error: cl_thread_join() - %d\n", retval);
     }
  }
 
  for (i=0;i<THREAD_COUNT;i++) {
     retval =  cl_thread_cleanup(&thread_list[i]);
     if (retval != CL_RETVAL_OK) {
        printf("error: cl_thread_cleanup() - %d, thread %d\n", retval,i);
     }
  }

/*  free(timeout_thread); */

  printf("main done\n");
  return 0;
}



/* thread implementation */

/* WARNING: a thread can be canceled per default in the cl_thread_wait_for_event()
   call. The programmer has to ensure, that all the tread don't leave any
   things on the stack. Use pthread_cleanup_push() and pthread_cleanup_pop() */

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "timeout_thread_main()"
void *timeout_thread_main(void *t_conf) {
   /* get pointer to cl_thread_settings_t struct */
   int ret_val;
   int pthread_cleanup_pop_execute = 0; /* workaround for irix compiler warning */
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 
   pthread_cleanup_push((void (*)(void *)) cl_thread_default_cleanup_function, (void*) thread_config );

   /* setup thread */
   if (thread_config) {
      printf("thread %d: initialize\n", thread_config->thread_id);
   
      /* thread init done, trigger startup conditon variable*/
      ret_val = cl_thread_func_startup(thread_config);
      if (ret_val != CL_RETVAL_OK) {
         printf("thread %d: cl_thread_func_startup() - %d\n", thread_config->thread_id, ret_val);
      }
   
      /* ok, thread main */
      printf("thread %d: enter mainloop\n", thread_config->thread_id);
      if ((ret_val=cl_thread_wait_for_event(thread_config,10,0)) != CL_RETVAL_OK) {  /* nothing to do */
         switch(ret_val) {
            case CL_RETVAL_CONDITION_WAIT_TIMEOUT:
               printf("thread %d: got timeout\n", thread_config->thread_id);
               break;
            default:
               printf("thread %d: got error: %d\n", thread_config->thread_id, ret_val);
         }
      }
   
      printf("thread %d: exit\n", thread_config->thread_id);
      
      /* at least set exit state */
      ret_val = cl_thread_func_cleanup(thread_config);  
      if (ret_val != CL_RETVAL_OK) {
         printf("thread %d: cl_thread_func_cleanup() - %d\n", thread_config->thread_id, ret_val);
      }
   }
   pthread_cleanup_pop(pthread_cleanup_pop_execute);
   return(NULL);
}



