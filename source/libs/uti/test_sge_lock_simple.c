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

static void *thread_function(void *anArg);

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
   return thread_function;
}

void *get_thrd_func_arg(void)
{
   return NULL;
}

void set_thread_count(int count) 
{
   return;
}

/****** test_sge_lock_simple/thread_function() *********************************
*  NAME
*     thread_function() -- Thread function to execute 
*
*  SYNOPSIS
*     static void* thread_function(void *anArg) 
*
*  FUNCTION
*     Lock the global lock in read mode and sleep. Unlock the global lock. 
*
*  INPUTS
*     void *anArg - thread function arguments 
*
*  RESULT
*     static void* - none
*
*  SEE ALSO
*     test_sge_lock_simple/get_thrd_func()
*******************************************************************************/
static void *thread_function(void *anArg)
{
   DENTER(TOP_LAYER, "thread_function");
   
   SGE_LOCK(LOCK_GLOBAL, LOCK_READ);

#if 1
   DPRINTF(("Thread %u sleeping at %d\n",(unsigned int) sge_locker_id(),
            (int) sge_get_gmt()));
#endif
   sleep(5);

   SGE_UNLOCK(LOCK_GLOBAL, LOCK_READ);

   DEXIT;
   return (void *)NULL;
} /* thread_function */

int validate(int thread_count) {
   return 0;
}
