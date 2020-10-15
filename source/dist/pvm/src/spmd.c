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
/*
*    SPMD example using PVM3
*    also illustrating group functions
*
*    compile with -DSPMD_WORK to generate
*    cpu usage in each pvm task. 
*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define SPMD_WORK
#define SPMD_SLEEP
#if defined(SPMD_WORK)
#include <signal.h>
#endif

#include "pvm3.h"

#if defined(__STDC__) || defined(__cplusplus)
#define __PR__(x) x
#else
#define __PR__(x) ()
#endif

static int dowork __PR__(( int me, int nproc));

#if (SOLARIS)
int gethostname __PR__((char *name, int namelen));
#endif

#if defined(SPMD_WORK)
volatile int should_stop = 0;
void stop_work(int signo);
void stop_work(int signo)
{
   should_stop = 1;
}
#endif

/* linux sleep() is not a system call 
   and is implemented using SIGALRM 
   this causes an endless sleep */
#if (LINUX)
#define sleep sge_sleep
void sge_sleep(int seconds)
{
   struct timeval timeout;

   timeout.tv_usec = 0;
   timeout.tv_sec = seconds;
   select(0, NULL, NULL, NULL, &timeout);
}
#endif


int main __PR__(( int argc, char *argv[]));

int main(argc, argv)
int argc;
char *argv[];
{
   int mytid;                  /* my task id */
   int me;                     /* my process number */
   int ret;
   int nh, na;
   char myhostname[80];
   struct pvmhostinfo *hinfo = NULL;
   int nodes = 1;

   fprintf(stderr, "spmd\n");
   pvm_setopt(PvmAutoErr, 1);

   if (argc>1) 
      nodes = atoi(argv[1]);

   /* enroll in pvm */
   mytid = pvm_mytid();
   if (mytid<0) {
      pvm_perror("pvm_mytid()");
      fprintf(stderr, "spmd: failed enrolling to pvmd\n");
      pvm_exit();
      return 1;
   }
   fprintf(stderr, "spmd mytid = 0x%x\n", mytid);

   /* Join a group and if I am the first instance */
   /* i.e. me=0 spawn more copies of myself       */
   me = pvm_joingroup( "foo" );
   if (me<0) {
      fprintf(stderr, "spmd: failed joining into my group\n");
      pvm_exit();
      return 1;
   }

   if (gethostname(myhostname, sizeof(myhostname)-1)<0) {
      fprintf(stderr, "spmd: failed getting my hostname\n");
      pvm_exit();
      return 1;
   }
   printf("spmd: %s: me = %d mytid = 0x%x\n", myhostname, me, mytid);

   if( me == 0 ) {
      int numt;

      /* report hosts we got */
      if (pvm_config(&nh, &na, &hinfo)) {
         fprintf(stderr, "spmd: failed enrolling to pvmd\n");
         pvm_exit();
         return 1;
      } else {
         int i;
      
         for (i=0; i<nh; i++) 
            printf("spmd: 0x%x %s %s %d\n", 
               hinfo[i].hi_tid, 
               hinfo[i].hi_name,
               hinfo[i].hi_arch,
               hinfo[i].hi_speed);
      }

      if (nodes>1) {
         int *tids;
         tids = (int *)malloc(nodes*sizeof(int));
         /* spawn nodes-1 on other hosts */
         printf("spmd: pvm_spawn(\"%s\", ..)\n", argv[0]);
         numt = pvm_spawn(argv[0], (argc>1)?(&argv[1]):NULL, 
               0, NULL, nodes-1, &tids[1]);

         if (numt!=nodes-1) {
            fprintf(stderr, "spmd: failed spawning %d processes - got only %d\n", nodes-1, numt);
            pvm_exit();
            return 1;
         }
      }
   }

   /* Wait for everyone to startup before proceeding. */
   ret = pvm_barrier( "foo", nodes );
   if (ret)  {
      fprintf(stderr, "spmd: failed at barrier\n");
      pvm_exit();
      return 1;
   }
      
/*--------------------------------------------------------------------------*/
     
   if (dowork( me, nodes )<0) {
      fprintf(stderr, "spmd: failed doing work\n");
      pvm_exit();
      return 1;
   }

/*--------------------------------------------------------------------------*/

   /* Wait for everyone having done his work before shutting down */
   ret = pvm_barrier( "foo", nodes );
   if (ret)  {
      fprintf(stderr, "spmd: failed at barrier\n");
      pvm_exit();
      return 1;
   }

   /* program finished leave group and exit pvm */
   if (pvm_lvgroup( "foo" )<0) {
      fprintf(stderr, "spmd: failed at pvm_lvgroup\n");
      pvm_exit();
      return 1;
   }

   pvm_exit();
   return 0;
}

/* Simple example passes a token around a ring */
static int dowork( me, nodes )
int me;
int nodes;
{
   int token;
   int src, dest;
   int count  = 1;
   int stride = 1;
   int msgtag = 4;
   int in;
   
   /* Determine neighbors in the ring */
   in = (me==0)?nodes-1:me-1; 
   src = pvm_gettid("foo", in);
   if (src<0) {
      pvm_perror("pvm_gettid(src)");
      fprintf(stderr, "spmd: failed getting src tid of %d (got 0x%x)\n", in, src);
      pvm_exit();
      return -1;
   }
   printf("spmd: src_tid = 0x%x\n", src);

   in = (me==nodes-1)?0:me+1;
   dest= pvm_gettid("foo", in);
   if (dest<0) {
      pvm_perror("pvm_gettid(dst)");
      fprintf(stderr, "spmd: failed getting dest tid of instance %d\n", in);
      pvm_exit();
      return -1;
   }
   printf("spmd: dest_tid = 0x%x\n", dest);

   if( me == 0 ) { 
      token = dest;
      if (pvm_initsend( PvmDataDefault )<0) {
         fprintf(stderr, "spmd: failed initializing pack buffer\n");
         pvm_exit();
         return -1;
      }

      pvm_pkint( &token, count, stride );

      if (pvm_send( dest, msgtag)<0) {
         fprintf(stderr, "spmd: failed sending message\n");
         pvm_exit();
         return -1;
      }
      printf("spmd: (%d) succeed sending token\n", me);

      if (pvm_recv( src, msgtag )<0) {
         fprintf(stderr, "spmd: failed getting message\n");
         pvm_exit();
         return -1;
      }
      printf("spmd: (%d) got token\n", me);
      printf("spmd: (%d) token ring done\n", me); 

   } else {
      if (pvm_recv( src, msgtag )<0) {
         fprintf(stderr, "spmd: failed getting message\n");
         pvm_exit();
         return -1;
      }
      printf("spmd: (%d) got token\n", me);

      pvm_upkint( &token, count, stride );
      if (pvm_initsend( PvmDataDefault )<0) {
         fprintf(stderr, "spmd: failed initializing pack buffer\n");
         pvm_exit();
         return -1;
      }
      pvm_pkint( &token, count, stride );

      if (pvm_send( dest, msgtag )<0) {
         fprintf(stderr, "spmd: failed sending message\n");
         pvm_exit();
         return -1;
      }
      printf("spmd: (%d) succeed sending token\n", me);
   }

#if defined(SPMD_WORK)
   signal(SIGALRM, stop_work);
   alarm(120);

   {
      float a, b = 0.1;

      while (!should_stop) {
        a += b;
      }
   }


#endif
   return 0;
}
