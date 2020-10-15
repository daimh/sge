
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "comm/cl_commlib.h"
#include "comm/lists/cl_log_list.h"
#include "comm/cl_endpoint_list.h"
#include "uti/sge_profiling.h"

void sighandler_issue_tests(int sig);

static int do_shutdown = 0;
static long issue_list[] = { 2728, 0};
static char* issue_desc[] = {"Commlib does not compare hosts with strcasecmp", NULL};

void sighandler_issue_tests(int sig) {
   if (sig == SIGPIPE) {
      return;
   }

   if (sig == SIGHUP) {
      return;
   }
   do_shutdown = 1;
}

void usage(void) {
   long i = 0;
   printf("usage: test_commlib_issue [ISSUE_NR]\n");
   printf("supported issues are:\n");
   while (issue_list[i] != 0) {
      printf("issue %ld - %s\n", issue_list[i], issue_desc[i]);
      i++;      
   }
}

int issue_2728_test(void) {
   char* host1 = "Foo.domain.Net";
   char* host2 = "foo.domain.net";
   char* host3 = "notFoo";
   int retval = CL_RETVAL_UNKNOWN;

   printf("issue 2728 test ...\n");

   printf("Comparing host \"%s\" with host \"%s\": ", host1, host2);
   retval = cl_com_compare_hosts(host1, host2);
   if (retval != CL_RETVAL_OK) {
      printf("hosts are not equal - this is issue 2728!\n");
      return 1;
   } 
   printf("host are equal as expected!\n");


   printf("Comparing host \"%s\" with host \"%s\": ", host1, host3);
   retval = cl_com_compare_hosts(host1, host3);
   if (retval == CL_RETVAL_OK) {
      printf("hosts are equal - this is error!\n");
      return 1;
   }
   printf("host are not equal as expected!\n");

   return 0;
}

int run_test(long nr) {
   int test_result = 0;
   printf("running test for issue %ld - %s\n", issue_list[nr], issue_desc[nr]);
   switch (issue_list[nr]) {
      case 2728: {
         test_result = issue_2728_test();
         break;
      }
      default: {
         printf("Unknown issue %ld\n", issue_list[nr]);
         test_result = 1;
         break;
      }
   }

   if (test_result == 0) {
      printf("ok!\n");
   } else {
      printf("failed!\n");
   }
   return test_result;
}

extern int main(int argc, char** argv) {
   struct sigaction sa;
   int test_result = 0;
   bool issue_found = false;
   long i = 0;
   long issue = -1;
   bool do_all = false;

   if (argc == 1) {
      do_all = true;        
   } else if (argc != 2) {
      usage();
      exit(1);
   }

   if (do_all == false) {
      issue = atol(argv[1]);
      i=0;
      while(issue_list[i] != 0) {
         if (issue_list[i] == issue) {
            issue_found = true;
            break;
         }
         i++;      
      }

      if (issue_found == false) {
         printf("Issue test %ld not found!\n\n", issue);
         usage();
         exit(1);
      }
   }
   prof_mt_init();

   /* setup signalhandling */
   
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = sighandler_issue_tests;  /* one handler for all signals */
   sigemptyset(&sa.sa_mask);
   sigaction(SIGINT, &sa, NULL);
   sigaction(SIGTERM, &sa, NULL);
   sigaction(SIGHUP, &sa, NULL);
   sigaction(SIGPIPE, &sa, NULL);
   printf("commlib setup ...\n");
   cl_com_setup_commlib(CL_RW_THREAD, CL_LOG_OFF, NULL);

   if (do_all == false) {
      test_result = run_test(i);
   } else {
      i=0;
      while (issue_list[i] != 0 && test_result == 0 && do_shutdown == 0) {
         test_result = run_test(i);
         i++;      
      }
      if (do_shutdown != 0) {
         test_result = 100;
      }
   }

   printf("commlib cleanup ...\n");
   cl_com_cleanup_commlib();

   printf("main done\n");
   return test_result;
}



