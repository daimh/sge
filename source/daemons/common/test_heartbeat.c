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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_profiling.h"

#include "sgeobj/sge_feature.h"

#include "qmaster_heartbeat.h"
 

/* 
 * The test repeatedly increments the heartbeat file (test.txt in local dir)
 * and reads it immediately after writing it. Until wraparound (99999 writes)
 * Access errors or heartbeat timeout errors are reported as exit value (see below)
 * and the test will stop.
 */


int main(int argc, char* argv[])
{
   int return_value = 0;
   int i;
   int runs = 0;
   char* filename = "test.txt";
   int   timeout  = 15;
   int todo = 0;
   struct timeval now;
   struct timeval last_time;
   int do_stop = 0;
   int beat_val;
   int only_write = 0;

   DENTER_MAIN(TOP_LAYER, "test_sge_qmaster_heartbeat");

   /* initialize last_time */
   gettimeofday(&last_time, NULL);

   if (argc==3) {
      if (strcmp(argv[1],"-only-write") == 0) {
         printf("only writing heartbeat file once\n");
         only_write=1;
         filename = argv[2];
      }
   }

   if ( only_write == 0) {
      /* delete file */
      unlink(filename);
   }
   
   /* now run till we start from 1 */
   while ( do_stop == 0 ) {
      return_value = inc_qmaster_heartbeat(filename, timeout, &beat_val);
      i            = get_qmaster_heartbeat(filename, timeout);
      if ( only_write == 1) {
         printf("incremented heartbeat file %s\n", filename);
         printf("heartbeat value is %d\n", i);
         exit(0);
      }

      todo++;
      if (beat_val != i) {
         printf("heartbeat value not correct\n");
         do_stop = 1;
         return_value = 20;
      }

      if (i <= 0) {
         printf("get_qmaster_heartbeat() returned %d\n", i);
         return_value = -100 + i;
      } else {
         if ( return_value != 0) {
            printf("(%d) inc_qmaster_heartbeat() returned %d\n", i, return_value);
         }
      }

      /* on error:  
       *
       * exit value > 100:   get_qmaster_hearbeat() returned:   - (exit value - 100)
       * exit value < 100:   inc_qmaster_heartbeat() returned:  - (exit value)
       * exit value == 20:   unexpected heartbeat value
       */
      if (return_value != 0) {
         unlink(filename);
         DEXIT;
         return (-return_value);
      }
      if (i==1 && runs++ != 0) {
         do_stop = 1;
      }
      gettimeofday(&now,NULL);
      if (now.tv_sec != last_time.tv_sec || do_stop != 0 ) {
         printf("%6.2f %% done\n", (double)(((double)todo/99999.0)*100.0));
         fflush(stdout);
         last_time.tv_sec = now.tv_sec;
      }
   }
   
   /* delete file */
   unlink(filename);

   DEXIT;
   return 0;
} /* main() */

