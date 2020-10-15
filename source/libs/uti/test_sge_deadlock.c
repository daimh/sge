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

#include "test_sge_lock_main.h"

#include <unistd.h>
#include <stdio.h>

#include "uti/sge_lock.h"
#include "uti/sge_time.h"


static void *thread_function_1(void *anArg);
static void *thread_function_2(void *anArg);

int get_thrd_demand(void)
{
   long p = 2;  /* min num of threads */

#if defined(SOLARIS)
   p = sysconf(_SC_NPROCESSORS_ONLN);
#endif

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

void set_thread_count(int count) 
{
   return;
}

/****** test_sge_lock_simple/thread_function_1() *********************************
*  NAME
*     thread_function_1() -- Thread function to execute 
*
*  SYNOPSIS
*     static void* thread_function_1(void *anArg) 
*
*  FUNCTION
*     Acquire multiple locks and sleep. Release the locks. After each 'sge_lock()'
*     and 'sge_unlock()' sleep to increase the probability of interlocked execution. 
*
*     Note: This function for itself is perfectly reasonable. However, a race
*     condition, and thus a potential deadlock, does emerge if this function is
*     run in parallel with 'thread_function_2()'.
*
*     The reason for this is, that 'thread_function_1()' and 'thread_function_2()'
*     each follow their own local acquire/release protocol. As a consequence,
*     'thread_function_1()' and 'thread_function_2()' acquire and release their
*     respective locks in different orders.
*
*     This example does reveal how important it is to obey a GLOBAL acquire/release
*     protocol. 
*
*  INPUTS
*     void *anArg - thread function arguments 
*
*  RESULT
*     static void* - none
*
*  SEE ALSO
*     test_sge_deadlock/get_thrd_func()
*     test_sge_deadlock/thread_function_2()
*******************************************************************************/
static void *thread_function_1(void *anArg)
{
   DENTER(TOP_LAYER, "thread_function");

   SGE_LOCK(LOCK_GLOBAL, LOCK_READ);
   sleep(3);

   SGE_LOCK(LOCK_MASTER_CONF, LOCK_READ);
   sleep(3);

   DPRINTF(("Thread %u sleeping at %d\n", (unsigned int) sge_locker_id(), (int) sge_get_gmt()));
   sleep(5);

   SGE_UNLOCK(LOCK_MASTER_CONF, LOCK_READ);
   sleep(3);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);
   sleep(3);

   DEXIT;
   return (void *)NULL;
} /* thread_function_1 */

/****** test_sge_deadlock/thread_function_2() **********************************
*  NAME
*     thread_function_2() -- Thread function to execute
*
*  SYNOPSIS
*     static void* thread_function_2(void *anArg) 
*
*  FUNCTION
*     Acquire multiple locks and sleep. Release the locks. After each 'sge_lock()'
*     and 'sge_unlock()' sleep to increase the probability of interlocked execution. 
*
*     Note: This function for itself is perfectly reasonable. However, a race
*     condition, and thus a potential deadlock, does emerge if this function is
*     run in parallel with 'thread_function_1()'.
*
*     The reason for this is, that 'thread_function_2()' and 'thread_function_1()'
*     each follow their own local acquire/release protocol. As a consequence,
*     'thread_function_2()' and 'thread_function_1()' acquire and release their
*     respective locks in different orders.
*
*     This example does reveal how important it is to obey a GLOBAL acquire/release
*     protocol. 
*
*  INPUTS
*     void *anArg - thread function arguments
*
*  RESULT
*     static void* - none
*
*  SEE ALSO
*     test_sge_deadlock/get_thrd_func()
*     test_sge_deadlock/thread_function_1()
*******************************************************************************/
static void *thread_function_2(void *anArg)
{
   DENTER(TOP_LAYER, "thread_function");

   SGE_LOCK(LOCK_MASTER_CONF, LOCK_READ);
   sleep(3);

   SGE_LOCK(LOCK_GLOBAL, LOCK_READ);
   sleep(3);

   DPRINTF(("Thread %u sleeping\n", (unsigned int) sge_locker_id()));
   sleep(5);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);
   sleep(3);

   SGE_UNLOCK(LOCK_MASTER_CONF, LOCK_READ);
   sleep(3);

   DEXIT;
   return (void *)NULL;
} /* thread_function_2 */

int validate(int thread_count) {
   return 0;
}

