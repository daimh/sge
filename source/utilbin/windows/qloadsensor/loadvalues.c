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

#include <windows.h>
#include <pdhmsg.h>

#include <stdio.h>
#include <math.h>

#include "loadvalues.h"
#include "simplelog.h"

#define NOT_INITIALIZED ((TCHAR) '\0')

/* 
 * NOTE:
 *
 * Only the fist member is initialized NULL is the indicator that none of the
 * of the elements has got a value.
 */
t_loadvalues loadvalue = { NOT_INITIALIZED };   

BOOL disable_beginend = FALSE;
BOOL disable_numproc = FALSE;
BOOL disable_load = FALSE;
BOOL disable_memory = FALSE;
BOOL disable_cpu = FALSE;
TXCHAR default_hostname = "";
DOUBLE load_adj = 0.0;
DOUBLE load_fac = 1.0;

HANDLE loadvalue_mutex = NULL;

static void 
get_current_load(double load[], double processes);

static void 
get_current_load(double load[], double processes)
{
   static double divisor[] = {6514.0, 32572.0, 97716.0}; // 1.0E-4 precision after 1/5/15 minutes
   static double load_start[] = {0.01, 0.01, 0.01};
   static double time_previous = 0.0;
   double time_now, time_difference;
   int    i;
   double x;


   DENTER("get_current_load");
   if (time_previous == 0.0) {
      time_previous = (double) GetTickCount();
      for (i = 0; i < 3; i++) {
         load[i] = load_start[i];
      }
   } else {
      time_now = (double) GetTickCount();
      time_difference  = time_now - time_previous;

      time_previous = time_now;
      for (i = 0; i < 3; i++) {
         x = 1 / exp(time_difference / divisor[i]);
         load[i] = load[i] * x + processes * (1 - x);
      }
   }
   DEXIT;
}

void
loadvalue_set_adj(DOUBLE adj)
{
   DENTER("loadvalue_set_adj");
   load_adj = adj;
   DEXIT;
}

void
loadvalue_set_fac(DOUBLE fac)
{
   DENTER("loadvalue_set_fac");
   load_fac = fac;
   DEXIT;
}

void
loadvalue_prefix(void)
{
   FILE *out = stdout;

   DENTER("loadvalue_prefix");
   if (!disable_beginend) {
      fprintf(out, "begin\n");
   }
   DEXIT;
}

void
loadvalue_postfix(void)
{
   FILE *out = stdout;

   DENTER("loadvalue_postfix");
   if (!disable_beginend) {
      fprintf(out, "end\n");
      fflush(out);
   }
   DEXIT;
}

void
loadvalue_print(t_loadvalues *loadvalue) 
{
   FILE *out = stdout;
   DWORD local_ret;

   DENTER("loadvalue_print");
   local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
   if (local_ret == WAIT_OBJECT_0) {
      LPSTR host = loadvalue->hostname;

      if (host[0] != NOT_INITIALIZED) {
         if (default_hostname[0] != NOT_INITIALIZED) {
            host = default_hostname;
         }
         if (!disable_numproc) {
            fprintf(out, "%s:num_proc:%d\n", host, loadvalue->num_proc);
         }
         if (!disable_load) {
            DOUBLE val_short = loadvalue->load_avg[0] + load_adj;
            DOUBLE val_medium = loadvalue->load_avg[1] + load_adj;
            DOUBLE val_long = loadvalue->load_avg[2] + load_adj;

            if (val_short < 0.0) {
               val_short = 0.0;
            }
            if (val_medium < 0.0) {
               val_medium = 0.0;
            }
            if (val_long < 0.0) {
               val_long = 0.0;
            }
            val_short *= load_fac;
            val_medium *= load_fac;
            val_long *= load_fac;

            fprintf(out, "%s:load_short:%.3f\n", host, val_short);
            fprintf(out, "%s:load_medium:%.3f\n", host, val_medium);
            fprintf(out, "%s:load_long:%.3f\n", host, val_long);
            fprintf(out, "%s:load_avg:%.3f\n", host, val_medium);
         }
         if (!disable_cpu) {
            fprintf(out, "%s:cpu:%lld\n", host, loadvalue->cpu);
         }
         if (!disable_memory) {
            fprintf(out, "%s:swap_free:%.0f\n", host, loadvalue->swap_free);
            fprintf(out, "%s:swap_total:%.0f\n", host, loadvalue->swap_total);
            fprintf(out, "%s:swap_used:%.0f\n", host, loadvalue->swap_used);
            fprintf(out, "%s:mem_free:%.0f\n", host, loadvalue->mem_free);
            fprintf(out, "%s:mem_total:%.0f\n", host, loadvalue->mem_total);
            fprintf(out, "%s:mem_used:%.0f\n", host, loadvalue->mem_used);
            fprintf(out, "%s:virtual_free:%.0f\n", host, 
                                                   loadvalue->virtual_free);
            fprintf(out, "%s:virtual_total:%.0f\n", host,
                                                   loadvalue->virtual_total);
            fprintf(out, "%s:virtual_used:%.0f\n", host, 
                                                   loadvalue->virtual_used);
         }
         fflush(out);
      }
   }
   ReleaseMutex(loadvalue_mutex);
   DEXIT;
}

int 
loadvalue_update_hostname(t_loadvalues *loadvalue)
{
   static BOOL initialized = FALSE;
   int ret = 0;
   int local_ret;
   
   DENTER("loadvalue_update_hostname");
   if (!initialized) {
      WORD wVersionRequested;
      WSADATA wsaData;
      int err;
       
      wVersionRequested = MAKEWORD(2, 2);
       
      err = WSAStartup(wVersionRequested, &wsaData);
      if (err != 0) {
         // error handling
         /* Tell the user that we could not find a usable */
         /* WinSock DLL.                                  */
         DEXIT;
         return 1;
      }

      /* 
       * Confirm that the WinSock DLL supports 2.2.
       * Note that if the DLL supports versions greater    
       * than 2.2 in addition to 2.2, it will still return 
       * 2.2 in wVersion since that is the version we      
       * requested.                                        
       */
      if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
         // error handling
         /* Tell the user that we could not find a usable */
         /* WinSock DLL.                                  */
         WSACleanup( );
         DEXIT;
         return 2;
      }
      initialized = TRUE;
   }

   local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
   if (local_ret == WAIT_OBJECT_0) {
      local_ret = gethostname(loadvalue->hostname, TXCHAR_LENGTH);
      if (local_ret != 0) {
         // error handling
      }
      ReleaseMutex(loadvalue_mutex);
   }
   DEXIT;
   return ret;
}

int 
loadvalue_update_num_proc(t_loadvalues *loadvalue)
{
   int ret = 0;
   SYSTEM_INFO system_info;
   DWORD local_ret;

   DENTER("loadvalue_update_num_proc");
   local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
   if (local_ret == WAIT_OBJECT_0) {
      GetSystemInfo(&system_info);
      loadvalue->num_proc = system_info.dwNumberOfProcessors;
      ReleaseMutex(loadvalue_mutex);
   }
   DEXIT;
   return ret;
}

int 
loadvalue_update_memory(t_loadvalues *loadvalue)
{
   int ret = 0;
   MEMORYSTATUSEX buffer;
   DWORD local_ret;

   DENTER("loadvalue_update_memory");
   local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
   if (local_ret == WAIT_OBJECT_0) {
      buffer.dwLength = sizeof(buffer);
      GlobalMemoryStatusEx(&buffer);

      loadvalue->swap_total = (double)buffer.ullTotalPageFile;
      loadvalue->swap_free  = (double)buffer.ullAvailPageFile;
      loadvalue->swap_used  = (double)buffer.ullTotalPageFile 
                              - (double)buffer.ullAvailPageFile;
      loadvalue->mem_total  = (double)buffer.ullTotalPhys;
      loadvalue->mem_free   = (double)buffer.ullAvailPhys;
      loadvalue->mem_used   = (double)buffer.ullTotalPhys 
                              - (double)buffer.ullAvailPhys;
      loadvalue->virtual_free  = loadvalue->swap_free + loadvalue->mem_free;
      loadvalue->virtual_total = loadvalue->swap_total + loadvalue->mem_total;
      loadvalue->virtual_used  = loadvalue->swap_used + loadvalue->mem_used;

      ReleaseMutex(loadvalue_mutex);
   }
   DEXIT;
   return ret;
}

int
loadvalue_update_processor_queue_length(t_loadvalues *loadvalue, t_pdhquery *query,
                           t_pdhcounterset *counter_queue_length)
{
   static BOOL initialized = FALSE;
   int         ret = 0;
   DWORD       local_ret = 0;
   double      count = 0;

   DENTER("loadvalue_update_cpuusage");

   /* Do query counter initialisation only once */ 
   if (!initialized) { 
      local_ret = pdhquery_initialize(query);
      if (local_ret == 0) {
         local_ret = pdhcounterset_initialize(counter_queue_length,
                                              "System",
                                              "",
                                              "Processor Queue Length");
         if (local_ret == 0) {
            local_ret = pdhquery_add_counterset(query, counter_queue_length);
            if (local_ret != 0) {
               // error handling
               ret = 3;
            }
         } else {
            // error handling
            ret = 2;
         }
      } else {
         // error handling
         ret = 1;
      }
      if (ret != 0) {
         return ret;
      }
      initialized = TRUE;
   }

   /* We are here - no error occured during initialisation */
   local_ret = pdhquery_update(query);
   if (local_ret == 0) {
      PDH_FMT_COUNTERVALUE queue_length;
   
      local_ret = PdhGetFormattedCounterValue(
                     counter_queue_length->counter_handles[0], 
                     PDH_FMT_LONG, NULL, &queue_length);
#if 0
            fprintf(stderr, "queue length: %ld\n", queue_length.longValue);
#endif

      // error handling
      local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
      if (local_ret == WAIT_OBJECT_0) {
         /* The Windows Processor Queue Length does not corresponded well to 
          * the Unix/Linux queue length, so we have to fake the load:
          * As long as the CPU is not 100% used, we use the CPU load as load.
          * As soon as the CPU is 100% used, we use the Processor Queue Length
          * to estimate the load. On Windows, the load is not equal to the
          * Processor Queue Length, but it seems that
          * load = 10^(Processor_Queue_Length/17) - 1.5
          * I have estimted this formula due to measures with single CPU
          * Windows XP hosts and confirmed it for hyperthreaded CPUs.
          */
         if(loadvalue->cpu < 100) {
            count = loadvalue->cpu/100.0;
         } else {
            count = pow(10, (double)(queue_length.longValue)/17.0) - 1.5;
#if 0
            fprintf(stderr, "load: %lf\n", count);
#endif
            if(count < 1) {
               count = 1;
            }
         }
         count *= loadvalue->num_proc;
         get_current_load(loadvalue->load_avg, count);
         ReleaseMutex(loadvalue_mutex);
      }
   } else {
      // error handling
      ret = 10;
   }
    
   DEXIT; 
   return ret;
}

int 
loadvalue_update_cpuusage(t_loadvalues *loadvalue, t_pdhquery *query,
                          t_pdhcounterset *counter_cpuusage)
{
   static BOOL initialized = FALSE;
   int ret = 0;
   DWORD local_ret;

   DENTER("loadvalue_update_cpuusage");

   /* Do query counter initialisation only once */ 
   if (!initialized) { 
      local_ret = pdhquery_initialize(query);
      if (local_ret == 0) {
         local_ret = pdhcounterset_initialize(counter_cpuusage,
                                              "Processor",
                                              "_Total",
                                              "% Processor Time");
         if (local_ret == 0) {
            local_ret = pdhquery_add_counterset(query, counter_cpuusage);
            if (local_ret != 0) {
               // error handling
               ret = 3;
            }
         } else {
            // error handling
            ret = 2;
         }
      } else {
         // error handling
         ret = 1;
      }
      if (ret != 0) {
         return ret;
      }
      initialized = TRUE;
   }

   /* We are here - no error occured during initialisation */
   local_ret = pdhquery_update(query);
   if (local_ret == 0) {
      PDH_FMT_COUNTERVALUE cpu_usage;
   
      local_ret = PdhGetFormattedCounterValue(
                                          counter_cpuusage->counter_handles[0], 
                                          PDH_FMT_LONG, NULL, &cpu_usage);
      // error handling
      local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
      if (local_ret == WAIT_OBJECT_0) {
         loadvalue->cpu = cpu_usage.longValue;
      }
      ReleaseMutex(loadvalue_mutex);
   } else {
      // error handling
      ret = 10;
   }
    
   DEXIT; 
   return ret;
}

int 
loadvalue_update_load(t_loadvalues *loadvalue, t_pdhquery *query,
                      t_pdhcounterset *counter_state,
                      t_pdhcounterset *counter_pid)
{
   static BOOL initialized = FALSE;
   int ret = 0;
   DWORD local_ret = 0;

   DENTER("loadvalue_update_load");
   if (!initialized) {
      local_ret = pdhquery_initialize(query);
      initialized = TRUE;
   } 
   if (local_ret == 0) {
      local_ret = pdhcounterset_initialize(counter_state,
                                           "Thread",
                                           "*",
                                           "Thread State");
      if (local_ret == 0) {
         local_ret = pdhquery_add_counterset(query, counter_state);
         if (local_ret == 0) {
            local_ret = pdhcounterset_initialize(counter_pid,
                                                 "Thread",
                                                 "*",
                                                 "ID Process");
            if (local_ret == 0) {
               local_ret = pdhquery_add_counterset(query, counter_pid); 
               if (local_ret != 0) {
                  // error handling
                  ret = 5;
               }
            } else {
               // error handling
               ret = 4;
            }
         } else {
            // error handling
            ret = 3;
         }
      } else {
         // error handling
         ret = 2;
      }
   } else {
      // error handling
      ret = 1;
   }
   if (ret != 0) {
      return ret;
   }

   /* We are here - no error occured during initialisation */
   local_ret = pdhquery_update(query);
   if (local_ret == 0) {
      DWORD state[8];
      DWORD size;
      BOOL *is_done;
  
      /*
       * State    Descrition
       * -------- -----------------------------------------------------------
       * 0        Initialized
       * 1        Ready (Bereit)                
       *              Waiting for a Processor
       * 2        Running (Wird ausgefuehrt)    
       *              Currently uses a processor
       * 3        Standby (Standy)              
       *              Will get a processor soon
       * 4        Terminated (Abgebrochen)      
       * 5        Waiting (Wartend)             
       *              Waiting for a peripheral process or resource 
       * 6        Transition (Uebergang)        
       *              Is waiting for a resource (swapspace ...)
       * 7        Unknown (Unbekannt)
       */

      memset(state, 0, sizeof(DWORD) * 8); 
      size = counter_state->number_of_counters * sizeof(BOOL);
      is_done = calloc(size, 1);
      if (is_done != NULL) {
         PDH_FMT_COUNTERVALUE state_id;
         PDH_FMT_COUNTERVALUE pid;
         DWORD j, k;
         DWORD count;
#if 0
         fprintf(stderr, "\n\n");
         fflush(stderr);
#endif
         for (j = 0; j < counter_state->number_of_counters; j++) {
            local_ret = PdhGetFormattedCounterValue(
                                          counter_state->counter_handles[j], 
                                          PDH_FMT_LONG, NULL, &state_id);
            if (local_ret == 0) {
               local_ret = PdhGetFormattedCounterValue(
                                          counter_pid->counter_handles[j],
                                          PDH_FMT_LONG, NULL, &pid);

               if (state_id.longValue == 1 || state_id.longValue == 2) {
#if 0 
                     fprintf(stderr, "%50s\t%d\t%ld\t%ld\n", 
                             counter_state->pdh_name[j], j, 
                             state_id.longValue, pid.longValue);
                     fflush(stderr);
#endif
                  if (is_done[j] == FALSE) {
                     state[state_id.longValue]++;
                     for (k = j; k < counter_state->number_of_counters; k++) { 
                        PDH_FMT_COUNTERVALUE pid2;

                        local_ret = PdhGetFormattedCounterValue(
                                             counter_pid->counter_handles[k],
                                             PDH_FMT_LONG, NULL, &pid2);
                        if (local_ret == 0) {
                           if (pid2.longValue == pid.longValue) {
                              is_done[k] = TRUE;
                           }
                        } else {
                           if (pid2.CStatus == PDH_CSTATUS_NO_INSTANCE) {
                              /* 
                               * It might be possible that the underlaying 
                               * instance was deleted meanwile (no error!)
                               */
                              ;
                           } else {
                              // error handling
                              ret = 13;
                           }
                        }
                     }
#if 0
                     fprintf(stderr, "\tC\n");
                     fflush(stderr);
#endif
                  } else {
#if 0
                     fprintf(stderr, "\tR\n");
                     fflush(stderr);
#endif
                  }
               }
            } else {
               if (state_id.CStatus == PDH_CSTATUS_NO_INSTANCE) {
                  /* 
                   * It might be possible that the underlaying 
                   * instance was deleted meanwile (no error!)
                   */
                  ;
               } else {
                  // error handling
                  ret = 12;
               }
            }
         }
         free(is_done);
         is_done = NULL;
#if 0
         for (j = 0; j < 8; j++) {
            fprintf(stderr, "state %d: %d\n", j, state[j]);
            fflush(stderr);
         }
#endif
         /*
          * the idle thread and the loadsensor itself 
          * have the state 2 if we collect data. These values
          * should not influence the loadaverage.
          */
         count = state[1] + state[2];
         if (count >= 2) {
            count -= 2;
         } else {
            count = 0;
         }
         local_ret = WaitForSingleObject(loadvalue_mutex, INFINITE);
         if (local_ret == WAIT_OBJECT_0) {
            get_current_load(loadvalue->load_avg, count);
            ReleaseMutex(loadvalue_mutex);
         }
      } else {
         // error handling
         ret = 11;
      }
      
      // error handling
   } else {
      // error handling
      ret = 10;
   }

   local_ret = pdhquery_remove_counterset(query, counter_state);
   local_ret = pdhquery_remove_counterset(query, counter_pid);
    
   DEXIT; 
   return ret;
}
