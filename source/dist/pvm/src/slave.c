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
#include <unistd.h>
#include "pvm3.h"

float work(int mytid, int me, int n, int *tids, int nproc, float *data);

/* linux sleep() is not a system call
   and is implemented using SIGALRM
   this causes an endless sleep 
   when used with checkpointing */
#if (LINUX || SUN4)
#define sleep sge_sleep
void sge_sleep(int seconds)
{
   struct timeval timeout;

   timeout.tv_usec = 0;
   timeout.tv_sec = seconds;
   select(0, NULL, NULL, NULL, &timeout);
}
#endif


int main(int argc, char **argv)
{
   int mytid;       /* my task id */
   int tids[32];    /* task ids   */
   int n, me, i, nproc, master, msgtype;
   float data[100], result;

   printf("before pvm_mytid()\n"); fflush(stdout);
   /* enroll in pvm */
   mytid = pvm_mytid();
   printf("pvm_mytid() = 0x%x\n", mytid); fflush(stdout);

   /* Receive data from master */
   msgtype = 0;
   pvm_recv( -1, msgtype );
   pvm_upkint(&nproc, 1, 1);
   pvm_upkint(tids, nproc, 1);
   pvm_upkint(&n, 1, 1);
   pvm_upkfloat(data, n, 1);

   /* Determine which slave I am (0 -- nproc-1) */
   for( i=0; i<nproc ; i++ )
   if( mytid == tids[i] ) { 
      me = i; 
      break; 
   }

   printf("0x%x before work()\n", mytid); fflush(stdout);

   /* Do calculations with data */
   result = work( mytid, me, n, tids, nproc, data );

   /* Send result to master */
   pvm_initsend( PvmDataDefault );
   pvm_pkint( &me, 1, 1 );
   pvm_pkfloat( &result, 1, 1 );
   msgtype = 5;
   master = pvm_parent();
   printf("0x%x before pvm_send()\n", mytid); fflush(stdout);
   pvm_send( master, msgtype );

   /* Program finished. Exit PVM before stopping */
   pvm_exit();
   return 0;
}

/* Simple example: slaves exchange data with left neighbor (wrapping) */
float
work(mytid, me, n, tids, nproc, data)
int mytid, me, n, *tids, nproc;
float *data;
{
   int i, dest;
   float psum = 0.0;
   float sum = 0.0;
   for( i=0 ; i<n ; i++ ){
      sum += me * data[i];
   }

   /* ensure we get at least wallclock time as usage */
   for (i=0; i<3*60; i++) {
      fprintf(stderr, "sleeping i = %d\n", i);
      sleep(1);
   }

   /* illustrate node-to-node communication */
   pvm_initsend( PvmDataDefault );
   pvm_pkfloat( &sum, 1, 1 );
   dest = me+1;
   if( dest == nproc ) 
      dest = 0;
   pvm_send( tids[dest], 22 );
   printf ("0x%x send to 0x%x \n", mytid, tids [dest]); fflush(stdout);
   if (pvm_recv( -1, 22 )==PvmSysErr)
      return 0;
   pvm_upkfloat( &psum, 1, 1 );

   return( sum+psum );
}
