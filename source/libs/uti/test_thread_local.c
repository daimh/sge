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
 *   Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

static pthread_key_t thread_local_key;

void init_local_storage(int thread)
{
   char *data = pthread_getspecific(thread_local_key);
   if (data == NULL) {
      data = malloc(100);
      snprintf(data, 100, "Thread %d", thread);
      pthread_setspecific(thread_local_key, data);
   }
}

void free_local_storage(void *data)
{
   free(data);
}

void thread_work(void) 
{
   int i;

   for (i = 0; i < 2; i++) {
      char *data = pthread_getspecific(thread_local_key);
      if (data == NULL) {
         printf("no thread local data\n");
      } else {
         printf("%s\n", data);
      }
      sleep(1);
   }
}

void *t1_main(void *args)
{
   init_local_storage(1);
   thread_work();

   return NULL;
}

void *t2_main(void *args)
{
   init_local_storage(2);
   thread_work();

   return NULL;
}


int main(int argc, const char *argv[])
{
   pthread_t t1, t2;
   int ret1 = 0, ret2 = 0;

   printf("testing access to thread local storage\n");

   pthread_key_create(&thread_local_key, free_local_storage);

   pthread_create(&t1, NULL, t1_main, (void*)&ret1);
   pthread_create(&t2, NULL, t2_main, (void*)&ret2);

   pthread_join(t1, NULL);
   pthread_join(t2, NULL);

   return 0;
}
