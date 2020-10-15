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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/


#include <unistd.h>
#include <stdio.h>

#include "test_sge_lock_main.h"

#include "uti/sge_lock.h"
#include "uti/sge_time.h"

#define THREAD_COUNT 8
#define THREAD_RUN_TIME 15

static int thread_count;
static u_long32 maxlocks;
static u_long32 results[THREAD_COUNT];

static void *thread_function(void *anArg);

void set_thread_count(int count) 
{
   thread_count = 0;
   return;
}


int get_thrd_demand(void)
{
   long p = THREAD_COUNT;  /* max num of threads */

   return (int)p;
}

void *(*get_thrd_func(void))(void *anArg)
{
   return thread_function;
}

void *get_thrd_func_arg(void)
{
   return NULL;
}

static void *thread_function(void *anArg)
{
   u_long32 start = sge_get_gmt();
   u_long32 count = 0;
   int thread_id = thread_count++;
   bool read_thread = true;
   bool do_loop = true;
    
   DENTER(TOP_LAYER, "thread_function");

   while (do_loop) {
      /*
         only the first thread is a write thread. We want to see if it starves from lock
         with some other competing read threads
       */
      if (thread_id == 0) {
         read_thread = false;
         SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE);
         sge_usleep(1);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);
      } else {
         SGE_LOCK(LOCK_GLOBAL, LOCK_READ);
         sge_usleep(1);
         SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);
      }
      count++;

      if (sge_get_gmt() - start >= THREAD_RUN_TIME) {
         break;
      }
   }

   results[thread_id] = count;
   
   printf("%s thread %d got "sge_U32CFormat" times the lock\n", read_thread?"read":"write", thread_id, count);

   DRETURN(NULL);
}

static int is_in_tolerance(u_long32 value1, u_long32 value2, int accepted_tolerance) {
   u_long32 first, second;
   int ret = 0;

   if (value1 > value2) {
      first = value1;
      second = value2;
   } else {
      first = value2;
      second = value1;
   }
   if (first/second > accepted_tolerance) {
      printf("error: tolerance is %d, expected was a max. of %d\n", (int)(first/second), accepted_tolerance);
      ret = 1;
   }
   return ret;
}

int validate(int thread_count) {
   u_long32 sum = 0;
   u_long32 mean;
   int i;
   int ret = 0;

   /* compute mean value */
   for (i=0; i < thread_count; i++) {
      sum += results[i];
   }
   mean = sum/thread_count;

   /* every thread should got the same lock amount. We accept a tolerance of 50% */
   for (i=0; i < thread_count; i++) {
      if (is_in_tolerance(results[i], mean, 50) != 0) {
         ret = 1;
         break;
      }
   }

   /* for more than one thread we expact every thread had half of the locks then
      the run with just one thread */
   if (ret == 0) {
      if (thread_count == 1) {
         printf("set max locks to "sge_U32CFormat"\n", mean);
         maxlocks = mean;  
      } else {
         for (i=0; i < thread_count; i++) {
            if (is_in_tolerance(results[i]*2, maxlocks, 50) != 0) {
               #if !defined(DARWIN) && !defined(AIX)
               /* pthreads on darwin and aix scales very bad and this test fail */
               ret = 1;
               break;
               #endif
            }
         }
      }
   }

   return ret;
}
