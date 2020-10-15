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

#if defined(__INTERIX)
#pragma message("WIN32 Program! Do not compile under Interix!")
DONT COMPILE THIS UNDER INTERIX!
#endif

#include <stdio.h>
#include <windows.h>

void print_help()
{
   printf("Usage: worker.exe <time to work in seconds>\n");
}

int main(int argc, char* argv[])
{
   int    i,j;
   double dx, dy, dz;
   DWORD  time_to_work = 0;
   DWORD  time_start = 0;
   BOOL   do_exit = FALSE;

   if(argc == 2) {
      time_to_work = atoi(argv[1]);
      if (time_to_work>0) {
         printf("Working for %d s\n", time_to_work);
      } else {
         printf("Nothing to do - exiting.\n");
      }
      fflush(stdout);
      time_start = GetTickCount();
      while (do_exit==FALSE) {
         for (i=0; i<1000000 && do_exit==FALSE; i++) {
            dx = (double)i;
            for (j=0; j<100 && do_exit==FALSE; j++) {
               dy = (double)j;
               dz = dx + dy;
            }
            if (GetTickCount()-time_start >= 1000*time_to_work) {
               do_exit = TRUE;
            }
         }
      }
      printf("Done with working.\n");
   } else {
      print_help();
      return 1;
   }

   return 0;
}
