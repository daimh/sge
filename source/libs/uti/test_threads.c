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
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

void *t1_errno(void *args)
{
   int i;
   int *ret = (int *)args;
   
   *ret = 0;

   errno = 5;
   for (i = 0; i < 5; i++) {
      if (errno != 5) {
         fprintf(stderr, "t1: errno = %d\n", errno);
         *ret = 1;
         break;
      }
      sleep(1);
   }

   return NULL;
}

void *t2_errno(void *args)
{
   int i;
   int *ret = (int *)args;
   FILE *fd;

   *ret = 0;

   errno = 0;
   fd = fopen("/tmp/blablablanotexisting", "r");
   if (fd != NULL) {
      fprintf(stderr, "file /tmp/blablablanotexisting does exist, please remove it!\n");
      *ret = 1;
   } else {
      for (i = 0; i < 5; i++) {
         if (errno != ENOENT) {
            fprintf(stderr, "t2: errno = %d\n", errno);
            *ret = 1;
            break;
         }
         sleep(1);
      }
   }

   return NULL;
}


int main(int argc, const char *argv[])
{
   pthread_t t1, t2;
   int ret1, ret2;

   printf("testing access to errno from two threads\n");

   pthread_create(&t1, NULL, t1_errno, (void*)&ret1);
   pthread_create(&t2, NULL, t2_errno, (void*)&ret2);

   pthread_join(t1, NULL);
   pthread_join(t2, NULL);

   if (ret1 != 0 || ret2 != 0) {
      return 1;
   }

   return 0;
}
