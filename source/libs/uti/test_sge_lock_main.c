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

#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "uti/test_sge_lock_main.h"
#include "uti/sge_lock.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_time.h"

/****** test_sge_lock_main/main() **********************************************
*  NAME
*     main() -- Generic skeleton for lock API test programs. 
*
*  SYNOPSIS
*     int main(int argc, char *argv[]) 
*
*  FUNCTION
*     Generic driver for lock API test programs utilizing pthreads and pthread
*     mutexes. Setup locking API by registering the needed callbacks. Determine
*     the number of threads needed. Create threads and execute the provided thread
*     function. Wait until all the threads have finished. Teardown the locking API.
*
*     If a program wants to utilize this skeleton it has to implement the functions
*     of a small API defined in 'sge_lock_main.h'.
*
*  INPUTS
*     int argc     - number of arguments 
*     char *argv[] - argument array 
*
*  RESULT
*     int - always 0
*
*  SEE ALSO
*     lck/test_sge_lock_main.h
*******************************************************************************/
int main(int argc, char *argv[])
{
   pthread_t *t;
   int i;
   int j;
   int thrd_count;
   u_long32 before, after, time_new;
   int ret = 0;
   
   DENTER_MAIN(TOP_LAYER, "main");

   thrd_count = get_thrd_demand();
   t = (pthread_t *)malloc(thrd_count * sizeof(pthread_t));

   for (i = 1; i <= thrd_count; i++) {
      for(j = 0; j < i; j++) {
         t[j] = 0;
      }
     
      set_thread_count(i);
      
      before = sge_get_gmt();

      printf("\n%s Create %d threads\n\n", SGE_FUNC, i);

      for(j = 0; j < i; j++) {
         pthread_create(&(t[j]), NULL, get_thrd_func(), get_thrd_func_arg());
      }
      for(j = 0; j < i; j++) {
         pthread_join(t[j], NULL);
      }

      after = sge_get_gmt();
      time_new = after - before;

      ret = validate(i);
      printf("the test took "sge_U32CFormat"s and was %s\n", time_new, ret==0?"successfull":"unsuccessfull");
      if (ret != 0) {
         break;
      }
   }
   sge_free(&t);

   DEXIT;
   return ret;
} /* main */
