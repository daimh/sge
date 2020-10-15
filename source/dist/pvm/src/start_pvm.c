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
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "pvm3.h"

#if defined(__STDC__) || defined(__cplusplus)
#define __PR__(x) x
#else
#define __PR__(x) ()
#endif

#define uid_t_fmt "%ld"

#define SLEEPTIME 5
#define RETRIES   10

static void delete_hanging_socket __PR__((void));
static int wait_until_pvm_runs __PR__((int numhosts));
static void usage __PR__((FILE *out));

int main __PR__(( int argc, char *argv[]));

/* 
 * start pvm and return when pvm is started
 * arguments are the number of nodes to be
 * controlled by start_pvm
 *
 */

/*-------------------------------------------------------------------------*/
static void delete_hanging_socket()
{
   char socknam[256];

   sprintf(socknam, "/tmp/pvmd.%ld", (u_long)getuid());
   unlink(socknam);
}

/*-------------------------------------------------------------------------*/
static int wait_until_pvm_runs(numhosts)
int numhosts;
{
   int nh, na;
   struct pvmhostinfo *hinfo = NULL;
   int retry = RETRIES;
   int rc;
   int tid;

   sleep(SLEEPTIME);            /* give pvm time to enroll local daemon */

   /* first enroll */
   while (retry--)
      if ((tid = pvm_mytid()) < 0)
         sleep(SLEEPTIME);
      else
         break;
   if (tid < 0) {
      fprintf(stderr, "start_pvm: Couldn't enroll to pvm\n");
      return 1;
   }

   printf("start_pvm: enrolled to local pvmd\n");
   
   if (!numhosts) {
      pvm_exit();
      return 0;
   }

   sleep(numhosts * SLEEPTIME); /* give pvm time to enroll remote daemons */
   while (retry--) {
      if (!(rc = pvm_config(&nh, &na, &hinfo))) {
         if (nh == numhosts) {
            printf("start_pvm: got %d hosts\n", numhosts);
            pvm_exit();
            return 0;
         }

/* may be this is useful if problems occur */
#if 1
{
   int i;
   printf("start_pvm: got %d of %d hosts\n", nh, numhosts);
   for (i=0; i<nh; i++) {
      printf("start_pvm: %d %s %s %d\n", 
      hinfo->hi_tid, 
      hinfo->hi_name, 
      hinfo->hi_arch, 
      hinfo->hi_speed); 
   }
}   
#endif

      }
      sleep(SLEEPTIME);
   }

   fprintf(stderr, "startpvm: Couldn't get all of the %d requested hosts\n",
           numhosts);
   return 1;
}

/* start_pvm is not responsible for converting hostfiles   */
/* so the hostfile is nothing more than a simple parameter */ 
/* that gets passed to pvmd                                */ 
static void usage(out)
FILE *out;
{
   fprintf(out, "usage: start_pvm [-h numhosts] pvmd-path [pvmd-parameters ..]\n");
}

/*-------------------------------------------------------------------------*/
int main(argc, argv)
int argc;
char *argv[];
{
   int numhosts = 0;
   char *s=NULL;

   if (argc < 2) {
      usage(stderr);
      return 1;
   }

   if (!strcmp(argv[1], "-h")) {
      argc--;
      argv++;

      numhosts = strtol(argv[1], &s, 10);
      if (*s) {
         usage(stderr);
         return 1;
      }

      argc--;
      argv++;
   }

   delete_hanging_socket();

   if (fork())
      _exit(wait_until_pvm_runs(numhosts));

   execvp(argv[1], &argv[1]);
   fprintf(stderr, "exec %s failed\n", argv[1]);
   exit(1);
}
