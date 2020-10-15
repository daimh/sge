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
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "uti/sge_profiling.h"

int   do_test (void);
int   test_params (void);
void* do_sleep(void*);
void* do_calc(void*);
void* do_calc2(void*);
void* do_malloc(void*);


int main(int argc, char *argv[])
{
   int ret = EXIT_SUCCESS;

   /* First with profiling enabled */
   printf ("Testing with profiling enabled.\n");
   ret = do_test ();
   
   if (ret == EXIT_SUCCESS) {
      /* Then with profiling disabled */
      printf ("Testing with profiling disabled.\n");
      sge_prof_set_enabled (false);
      ret = do_test ();
   }
   
   if (ret == EXIT_SUCCESS) {
      /* Then again with profiling re-enabled */
      printf ("Testing with profiling re-enabled.\n");
      sge_prof_set_enabled (true);
      ret = do_test ();
   }

   if (ret == EXIT_SUCCESS) {
      ret = test_params();
   }
   
   return ret;
}

int test_params ()
{
   int ret = EXIT_SUCCESS;

   /* Test formerly broken actions for SGE_PROF_ALL level */
   prof_mt_init();

   prof_start(SGE_PROF_ALL, NULL);
   
   if (prof_is_active(SGE_PROF_ALL) != 1) {
      printf("prof_is_active(SGE_PROF_ALL) did not return 1!");
      ret = EXIT_FAILURE;
   }

   printf("the following prof_output_info call should output multiple profiling lines\n");
   prof_output_info(SGE_PROF_ALL, false, "test:\n");

   prof_stop(SGE_PROF_ALL, NULL);

   sge_prof_cleanup();

   if (ret == EXIT_SUCCESS) {
      printf("test_params successfull\n\n");
   }

   return ret;
}

int do_test () {
   pthread_t sleep_thread, calc_thread, calc2_thread, malloc_thread;

   dstring error = DSTRING_INIT;

   prof_mt_init();

/*   set_thread_name(pthread_self(), "Main Loop");
   set_thread_prof_status_by_name(pthread_self(), "Main Loop", true);*/


/*   if (thread_prof_active_by_id(pthread_self()) == true ) {*/

      if(!prof_start(SGE_PROF_OTHER, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_start(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM1, "Main Loop", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_OTHER, "other", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }
   /*}*/

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   pthread_create(&sleep_thread, NULL, do_sleep, NULL);
   sleep(1);
/*   set_thread_name(sleep_thread, "Sleep Thread");*/
/*   set_thread_prof_status_by_name(sleep_thread, "Sleep Thread", true);*/

   pthread_create(&calc_thread, NULL, do_calc, NULL);
   sleep(1);
/*   set_thread_name(calc_thread, "Calc Thread");
   set_thread_prof_status_by_name(calc_thread, "Calc Thread", true);*/

   pthread_create(&calc2_thread, NULL, do_calc2, NULL);
   sleep(1);
/*   set_thread_name(calc2_thread, "Calc2 Thread");
   set_thread_prof_status_by_name(calc2_thread, "Calc2 Thread", true);*/

   pthread_create(&malloc_thread, NULL, do_malloc, NULL);
/*   set_thread_name(malloc_thread, "Malloc Thread");
   set_thread_prof_status_by_name(malloc_thread, "Malloc Thread", true);*/

   pthread_join(sleep_thread, NULL);
   pthread_join(calc_thread, NULL);
   pthread_join(calc2_thread, NULL);
   pthread_join(malloc_thread, NULL);

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);


   printf("after nested profiling:\n");
   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);
   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

   if(!prof_reset(SGE_PROF_CUSTOM1, &error)) {
      fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
      fprintf(stderr, "\n");
      sge_dstring_clear(&error);
   }

   printf("after reset: \n");
   printf("%s\n", prof_get_info_string(SGE_PROF_CUSTOM1, false, &error));

   if(!prof_stop(SGE_PROF_CUSTOM1, &error)) {
      fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
      fprintf(stderr, "\n");
      sge_dstring_clear(&error);
   }

   if(!prof_stop(SGE_PROF_OTHER, &error)) {
      fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
      fprintf(stderr, "\n");
      sge_dstring_clear(&error);
   }

   sge_dstring_free(&error);
   sge_prof_cleanup();

   return EXIT_SUCCESS;
}

void* do_sleep(void* p) {


   dstring error = DSTRING_INIT;

/*   if (thread_prof_active_by_id(pthread_self()) == true ) {*/

      if(!prof_start(SGE_PROF_OTHER, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_start(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_start(SGE_PROF_CUSTOM2, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM1, "sleep thread", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM2, "sleep_thread_printf", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }  
/*   }*/

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   sleep(10);

   
   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM2);

   printf("Hello World!\n");
   sleep(1);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM2);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);
   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

   /*if (thread_prof_active_by_id(pthread_self()) == true ) {*/
      if(!prof_stop(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_stop(SGE_PROF_CUSTOM2, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_stop(SGE_PROF_OTHER, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }
   /*}*/  
   sge_dstring_free(&error);
    
   return NULL;
}


void* do_calc(void* p) {
   int num = 3000;
   int i = 0;
   double x, y, z;
   dstring error = DSTRING_INIT;


   /*if (thread_prof_active_by_id(pthread_self()) == true ) {   */
      if(!prof_start(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM1, "calc thread", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }
   /*}*/

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   for(i = 0; i < num; i++) {
      x = sin(i % 10);
      y = cos(i % 10);
      z = x * y;
   }

   for(i = 0; i < num; i++) {
      x++;
   }

   for(i = 0; i < num; i++) {
      x = tan(i % 10);
   }

   sleep(4);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);
   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

      if(!prof_stop(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

   sge_dstring_free(&error);

   /* to please the compiler on irix */
   x = z;

   return NULL;
}

void* do_calc2(void* p) {
   int num = 7000;
   int i = 0;
   int x,y;
   dstring error = DSTRING_INIT;

   /*if (thread_prof_active_by_id(pthread_self()) == true ) {*/
      if(!prof_start(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM1, "calc2 thread", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }
   /*}*/

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   for(i = 0; i < num; i++) {
      x = sin(i % 10);
      y = cos(i % 10);
   }

   sleep(5);

   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);
   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

      if(!prof_stop(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

   /* to please the compiler on irix */
   y = x;
   x = y;

   sge_dstring_free(&error);

   return NULL;
}

void* do_malloc(void* p) {
   char* text;
   int count = 90000;
   int i;
   dstring error = DSTRING_INIT;

   /*if (thread_prof_active_by_id(pthread_self()) == true ) {*/
      if(!prof_start(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

      if(!prof_set_level_name(SGE_PROF_CUSTOM1, "malloc thread", &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }   
   /*}*/

   PROF_START_MEASUREMENT(SGE_PROF_CUSTOM1);

   for (i = 0; i < count; i++) {
      text = strdup("malloc thread");
      sge_free(&text);
   }
   
   PROF_STOP_MEASUREMENT(SGE_PROF_CUSTOM1);

   printf("%s\n", prof_get_info_string(SGE_PROF_ALL, false, &error));

      if(!prof_stop(SGE_PROF_CUSTOM1, &error)) {
         fprintf(stderr, SFNMAX, sge_dstring_get_string(&error)); fflush(stderr);
         fprintf(stderr, "\n");
         sge_dstring_clear(&error);
      }

   sge_dstring_free(&error);

   return NULL;
}

