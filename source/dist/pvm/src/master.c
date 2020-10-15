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
#include <unistd.h>

#include <signal.h>

#include "pvm3.h"

#define SLAVENAME "slave"
#define SLAVENAME_CKPT "slave_ckpt"

int main(int argc, char **argv)
{
   int mytid;                  /* my task id */
   int tids[32];				/* slave task ids */
   int n, nproc, numt, i, who, msgtype, nhost, narch;
   float data[100], result[32];
   struct pvmhostinfo *hostp[32];

   printf("before pvm_mytid()\n"); fflush(stdout);
   /* enroll in pvm */
   mytid = pvm_mytid();
   printf("pvm_mytid() = 0x%x\n", mytid); fflush(stdout);

   /* Set number of slaves to start */
   /* Can not do stdin from spawned task */
   if( pvm_parent() == PvmNoParent ) {
      printf("no parents\n"); fflush(stdout);
      if (argc>1) {
         nproc = atoi(argv[1]);
      }
      else {
         puts("How many slave programs (1-32)?");
         scanf("%d", &nproc);
      }
   } else{
      pvm_config( &nhost, &narch, hostp );
      nproc = nhost;
      if( nproc > 32 ) 
         nproc = 32 ;
   }

   /* start up slave tasks */
   printf("Try to spawn %d slaves\n", nproc); fflush(stdout);
   numt=pvm_spawn(getenv("SGE_CKPT_ENV")?SLAVENAME_CKPT:SLAVENAME, 
      (char**)0, 0, "", nproc, tids);
   if( numt < nproc ) {
      printf("Trouble spawning slaves. Aborting. Error codes are:\n"); fflush(stdout);
      for( i=numt ; i<nproc ; i++ ) {
         printf("TID %d %d\n",i,tids[i]); fflush(stdout);
      }
      for( i=0 ; i<numt ; i++ ) {
         pvm_kill( tids[i] );
      }
      pvm_exit();
      exit(1);
   }

   printf("got %d tasks\n", numt); fflush(stdout);

   /* Begin User Program */
   n = 100;
   /* initialize_data( data, n ); */
   for( i=0 ; i<n ; i++ ) {
      data[i] = 1;
   }

   printf("broadcasting data to tasks\n"); fflush(stdout);

   /* Broadcast initial data to slave tasks */
   pvm_initsend(PvmDataDefault);
   pvm_pkint(&nproc, 1, 1);
   pvm_pkint(tids, nproc, 1);
   pvm_pkint(&n, 1, 1);
   pvm_pkfloat(data, n, 1);
   while (pvm_mcast(tids, nproc, 0)) {
      fprintf(stderr, "failure broadcasting data - retry\n");
      sleep(1);
   }

   printf("wait for results from slaves\n"); fflush(stdout);

   /* Wait for results from slaves */
   msgtype = 5;
   for( i=0 ; i<nproc ; i++ ) {
      pvm_recv( -1, msgtype );
      pvm_upkint( &who, 1, 1 );
      pvm_upkfloat( &result[who], 1, 1 );
      printf("I got %f from %d\n",result[who],who);
   }
   /* Program Finished exit PVM before stopping */
   pvm_exit();
   return 0;
}

