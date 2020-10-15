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

/*
 * Following define enables several functions which are defined in
 * the windows header files
 */
#define _WIN32_WINNT 0x0501

/*
 * winsock2.h has to be included before windows.h
 */
#include <winsock2.h>
#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "qloadsensor.h"
#include "loadvalues.h"
#include "pdhservice.h"
#include "pdhquery.h"
#include "simplelog.h"

t_pdhquery query_cpu;
t_pdhquery query_queue_length;
t_pdhcounterset counter_cpu;
t_pdhcounterset counter_queue_length;

t_pdhquery query_load;
t_pdhcounterset counter_state;
t_pdhcounterset counter_pid;

DWORD update_interval = 5000;

static HANDLE firsttime_event = 0;
static BOOL first_time = TRUE;

/* load sensor thread */
DWORD WINAPI
loadsensor_main(LPVOID Parameter)
{
   DWORD ret = 0;
   HANDLE timer = NULL;

   DENTER("loadsensor_main");

   timer = CreateWaitableTimer(NULL, TRUE, NULL);
   if (timer != NULL) {
      do {
         LARGE_INTEGER wait_time;
         LONGLONG factor = -10000;
         LONGLONG msec = update_interval;

         wait_time.QuadPart = factor * msec;
         if (SetWaitableTimer(timer, &wait_time, 0, NULL, NULL, 0)) {
            DWORD local_ret = WAIT_OBJECT_0;

            if(!first_time) {
               local_ret = WaitForSingleObject(timer, INFINITE);
            }
            if (local_ret == WAIT_OBJECT_0) {
#if 0
               DWORD TickCount = 0;
               fprintf(stderr, "Begin\n");
               fflush(stderr);
               TickCount = GetTickCount();
#endif
               if (!disable_cpu) {
                  local_ret = loadvalue_update_cpuusage(&loadvalue, 
                                                        &query_cpu,  
                                                        &counter_cpu);

                  if (local_ret != 0) {
                     ret = 7;
                     error_set(ret, "retrieving cpu usage failed"); 
                  }
               }
               if (!disable_load) {
                  local_ret = loadvalue_update_num_proc(&loadvalue);
                  if (local_ret != 0) {
                     ret = 6;
                     error_set(ret, "retrieving the number of procs failed"); 
                  }
#if 0
                  local_ret = loadvalue_update_load(&loadvalue,
                                                    &query_load,
                                                    &counter_state,
                                                    &counter_pid);
#endif
#if 1
                  local_ret = loadvalue_update_processor_queue_length(&loadvalue, 
                                                         &query_queue_length,  
                                                         &counter_queue_length);
#endif

                  if (local_ret != 0) {
                     ret = 5;
                     error_set(local_ret, "retrieving load failed"); 
                  }
               }
               if (!disable_memory) {
                  local_ret = loadvalue_update_memory(&loadvalue);
               }
               local_ret = loadvalue_update_hostname(&loadvalue);
               if (local_ret != 0) {
                  ret = 4;
                  error_set(ret, "retrieving hostname failed"); 
               }
#if 0
               fprintf(stderr, "End\n");
               fprintf(stderr, "Time elapsed: %lf\n\n", (GetTickCount()-TickCount)/1000.0);
               fflush(stderr);
#endif
            } else {
               ret = 3;
               error_set(ret, "Waiting for next epoch failed"); 
            }
         } else {
            ret = 2;
            error_set(ret, "Unable to initialize timer object");
         }
         if (first_time) {
            if(firsttime_event != NULL ) {
               SetEvent(firsttime_event);
            }
            first_time = FALSE;
         }
      } while (1);
   } else {
      ret = 1;
      error_set(ret, "Unable to create timer object");
   }
   DEXIT;
   return ret;
}

static void
show_usage(void) 
{
   DENTER("show_usage");
   printf("usage: qloadsensor.exe [options]\n");
   printf(" [-help]                  display this message\n");
   printf(" [-hostname <hostname>    <hostname> will overwite the real hostname\n");
   printf(" [-set-interval]          redefines the interval between the application\n");
   printf("                          collects data for the load reports (default 10s)\n");
   printf(" [-set-load-adj <value>]  set load adjustment value\n");
   printf(" [-set-load-fac <value>]  set load factor value\n");
   printf(" [-set-trace-file <file>] set the trace file and enable logging\n");
   printf(" [-disable-beginend]      the \"begin\" and \"end\" keywords are not printed\n");
   printf(" [-disable-cpu]           cpu usage won't be collected and printed\n");
   printf(" [-disable-load]          load_* values won't be collected and printed\n");
   printf(" [-disable-numproc]       num_proc won't be collected and printed\n");
   printf(" [-disable-memory]        all memory related values are not reported\n"); 
   fflush(stdout);
   DEXIT;
}

/* main thread */
int 
main(int argc, char* argv[])
{
   int ret = 0;
   BOOL do_exit = FALSE;
   HANDLE thread = 0;
   DWORD thread_id = 0;
   
   DENTER("main");
   while (argc > 1 && argv[1][0] == '-') {
      DPRINTF(("handling commandline options...\n"));
      if (!strcmp(argv[1], "-help")) {
         DPRINTF(("got \"-help\" switch\n"));
         show_usage();
         DEXIT;
         exit(0);
      } 
      if (!strcmp(argv[1], "-set-interval")) {
         argc--;
         argv++;
         if (argc != 1 && sscanf(argv[1], "%d", &update_interval) == 1) {
            DPRINTF(("got \"-set-interval %d\" switch\n", update_interval));
            argc--;
            argv++;
            continue;
         } else {
            show_usage();
            DEXIT;
            exit(1);
         }
      }
      if (!strcmp(argv[1], "-set-load-adj")) {
         DOUBLE load_adj;

         argc--;
         argv++;
         if (argc != 1 && sscanf(argv[1], "%lf", &load_adj) == 1) {
            DPRINTF(("got \"-set-load-adj %lf\" switch\n", load_adj));
            argc--;
            argv++;
            loadvalue_set_adj(load_adj);
            continue;
         } else {
            show_usage();
            DEXIT;
            exit(1);
         }
      }
      if (!strcmp(argv[1], "-set-load-fac")) {
         DOUBLE load_fac;

         argc--;
         argv++;
         if (argc != 1 && sscanf(argv[1], "%lf", &load_fac) == 1) {
            DPRINTF(("got \"-set-load-fac %lf\" switch\n", load_fac));
            argc--;
            argv++;
            loadvalue_set_fac(load_fac);
            continue;
         } else {
            show_usage();
            DEXIT;
            exit(1);
         }
      }
      if (!strcmp(argv[1], "-set-trace-file")) {
         TXCHAR tracefile = "";
         argc--;
         argv++;
         if (argc != 1 && sscanf(argv[1], "%s", &tracefile) == 1) {
            DPRINTF(("got \"-set-trace-file %s\" switch\n", tracefile));
            error_open_tracefile(tracefile);
            error_set_tracingenabled(TRUE);
            argc--;
            argv++;
            continue;
         } else {
            show_usage();
            DEXIT;
            exit(1);
         }
      }
      if (!strcmp(argv[1], "-hostname")) {
         argc--;
         argv++;
         if (argc != 1 && sscanf(argv[1], "%s", &default_hostname) == 1) {
            DPRINTF(("got \"-hostname %s\" switch\n", default_hostname));
            argc--;
            argv++;
            continue;
         } else {
            show_usage();
            DEXIT;
            exit(1);
         }
      }
      if (!strcmp(argv[1], "-disable-beginend")) {
         DPRINTF(("got \"-disable-beginend\" switch\n"));
         disable_beginend = TRUE;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-disable-numproc")) {
         DPRINTF(("got \"-disable-numproc\" switch\n"));
         disable_numproc = TRUE;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-disable-load")) {
         DPRINTF(("got \"-disable-load\" switch\n"));
         disable_load = TRUE;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-disable-memory")) {
         DPRINTF(("got \"-disable-memory\" switch\n"));
         disable_memory = TRUE;
         argc--;
         argv++;
         continue;
      }
      if (!strcmp(argv[1], "-disable-cpu")) {
         DPRINTF(("got \"-disable-cpu\" switch\n"));
         disable_cpu = TRUE;
         argc--;
         argv++;
         continue;
      }
      DPRINTF(("got unknown option \"%s\"\n", argv[1])); 
      show_usage();
      exit(1);
   }

   /* 
    * Initialisation
    */
   DPRINTF(("initialisation\n"));
   pdhservice_initialize();

   firsttime_event = CreateEvent(NULL, FALSE, FALSE, NULL);

   loadvalue_mutex = CreateMutex(NULL, FALSE, NULL);
   if (loadvalue_mutex != NULL) {
      thread = CreateThread(NULL, 0, loadsensor_main, 0, 0, &thread_id);
      if (thread == NULL) {
         ret = 2;
         error_set(ret, "Unable to create thread which collects data");
      }
      SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
   } else {
      ret = 1;
      error_set(ret, "Unable to create mutex object");
   }

   /*
    * Handle execd <-> loadsensor protocol
    */
   DPRINTF(("handling execd <-> loadsensor protocol\n"));
   while (!do_exit) {
      CHAR command[100];
      int strpos = 0;

      /*
       * read characters from stdin until we receive an "enter" character
       */
      memset(command, 0, 100);
      do {
         command[strpos] = getchar();
         if (command[strpos] == EOF) {
            if (errno != 0) {
               ret = 10;
               error_set(ret, "Unable to read from stdin");
            }
            do_exit = 1;
            break;
         } else if (command[strpos] == '\n' || command[strpos] == '\r') {
            break;
         }
         strpos++;
      } while (strpos < 99);

      /*
       * Check the command and execute it:
       * ------------------------------------------
       * "\n"
       *    begin
       *    <hostname>:<complex_attribute>:<value>
       *    ...
       *    end
       * ------------------------------------------
       * "_sge_error_report"
       *    begin
       *    _sge_pseudo_host:error:<error_message>
       *    end
       * ------------------------------------------
       * "quit"
       */
      if (command[0] == '\n' || command[0] == '\r') {
         /* Before we send the load values to the sge_execd,
          * we must make sure that the load sensors are initialized.
          * But we must send the "begin" immediately, so the 
          * sge_execd will wait for us.
          */
         if(ret == 0 && first_time && firsttime_event != NULL) {
            WaitForSingleObject(firsttime_event, INFINITE);
         }
         loadvalue_prefix();
         loadvalue_print(&loadvalue);
         error_print();
         loadvalue_postfix();
      } else if (strncmp(command, "_sge_error_report", 17) == 0) {
         loadvalue_prefix();
         error_print();
         loadvalue_postfix();
      } else if (strncmp(command, "quit", 4) == 0) {
         do_exit = 1;
      } else {
         ret = 20;
         error_set(ret, "Unknown command from stdin");
      }
   }

   /* 
    * cleanup 
    */
   pdhservice_free();
   WSACleanup();
   DEXIT;
   return ret;
}
