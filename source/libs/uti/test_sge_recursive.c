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

#include "uti/sge_lock.h"
#include "uti/test_sge_lock_main.h"


static void *thread_function_1(void *anArg);
static void *thread_function_2(void *anArg);
static void lock_recursive(void);


int get_thrd_demand(void)
{
   long p = 2;  /* min num of threads */

   return (int)p;
}

void *(*get_thrd_func(void))(void *anArg)
{
   static int i = 0;

   return ((i++ % 2) ? thread_function_1 : thread_function_2) ;
}

void *get_thrd_func_arg(void)
{
   return NULL;
}

static void *thread_function_1(void *anArg)
{
   DENTER(TOP_LAYER, "thread_function_1");

   SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE);

   sleep(3);

   lock_recursive();

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
   return (void *)NULL;
}

static void lock_recursive(void)
{
   DENTER(TOP_LAYER, "lock_recursive");

   SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE);

   sleep(15);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
   return;
}

static void *thread_function_2(void *anArg)
{
   DENTER(TOP_LAYER, "thread_function_2");

   sleep(6);

   SGE_LOCK(LOCK_GLOBAL, LOCK_WRITE);

   sleep(2);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_WRITE);

   DEXIT;
   return (void *)NULL;
} /* thread_function_2 */

int validate(int thread_count) {
   return 0;
}

void set_thread_count(int count) 
{
   return;
}
