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

#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

#include "basis_types.h"

#include "uti/sge_rmon.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/msg_utilib.h"
#include "uti/sge_profiling.h"

/****** uti/profiling/--Profiling ****************************************
*  NAME
*     Profiling -- A simple profiling utility 
*
*  FUNCTION
*     The profiling module provides a number of utility functions
*     to output some performance metrics about a program, like wallclock,
*     busy time, user and system cpu time etc.
*
*     Profiling can be started and stopped, the measured data can be 
*     reset to 0.
*
*     Profiling for individual code blocks can be done by starting 
*     and stopping a measurement.
*
*     Measured data can be queried individually for each metric variable,
*     or an informational message string can be requested containing the
*     most interesting variables.
*
*     To distinguish profiling data for different modules of a program,
*     (e.g. time/cpu consumed for communication, spooling, etc.),
*     multiple levels have been introduced.
*
*     Besides the predefined levels, there exist 10 custom levels 
*     (SGE_PROF_CUSTOM0 up to SGE_PROF_CUSTOM9) that can
*     be used to profile individual parts of a program.
*
*     The predefined levels shall only be used by the developers of the 
*     corresponding components.
*
*     The level SGE_PROF_OTHER is maintained by the profiling module itself 
*     and collects the remaining usage not covered by other levels. It shall 
*     NOT be used outside the profiling module itself.
*  
*     A measurement is always done for a certain level.
*     If a measurement for a certain level is started, while another
*     measurement for another level is still active, the data of the
*     subordinated measurement is remembered in the superordinated level.
*
*     When retrieving profiling information, a parameter defines whether
*     information for subordinated measurements shall be included or excluded.
*
*     Cyclic starting of measurements is not allowed and will result in 
*     profiling being switched off.
*
*  EXAMPLE
*     There is a test program (libs/uti/test_sge_profiling) and the
*     module is used in sge_schedd (daemons/schedd/scheduler.c),
*     in the event mirror module (libs/gdi/sge_mirror.c), the qmaster
*     and his threads, execd and the spooling
*     test program (libs/spool/test_sge_spooling.c).
*
*  SEE ALSO
*     uti/profiling/prof_is_active()
*     uti/profiling/prof_start()
*     uti/profiling/prof_stop()
*     uti/profiling/prof_start_measurement()
*     uti/profiling/prof_stop_measurement()
*     uti/profiling/prof_reset()
*     uti/profiling/prof_get_measurement_wallclock()
*     uti/profiling/prof_get_measurement_utime()
*     uti/profiling/prof_get_measurement_stime()
*     uti/profiling/prof_get_total_wallclock()
*     uti/profiling/prof_get_total_busy()
*     uti/profiling/prof_get_total_utime()
*     uti/profiling/prof_get_total_stime()
*     uti/profiling/prof_get_info_string()
*
*  NOTES
*     MT-NOTE: this module is MT safe, if prof_mt_init() and/or
*              sge_prof_set_enabled() are called before
*              profiling is started and sge_prof_cleanup() is called after
*              all threads which make profiling calls have been stopped!
*              sge_prof_cleanup() is used to free  the profiling array 
*
*  BUGS
*     In a multithreaded program, the values delivered by the times system call
*     for cpu system and user time are per process times.
*     This module assumes that the values delivered by times are per thread
*     values. 
*     In the profiling output, the sum of user and system time can be higher 
*     than the wallclock time, the utilization greater 100%.
*
*  TODO
*     - Replace all pthread_mutex_(un)lock() calls with sge_mutex_(un)lock()
*     - Separate public and private interfaces so that thread index can be
*       passed around instead of always being rediscovered
*     - Replace static array with thread specific storage?
*     - Document static functions
*     - Print levels without names
*******************************************************************************/

/****** uti/profiling/-Profiling-Defines ***************************************
*  NAME
*     Defines -- Defines and macros for profiling
*
*  SYNOPSIS
*     #define PROF_START_MEASUREMENT ...
*     #define PROF_STOP_MEASUREMENT ...
*
*  FUNCTION
*     Macros to start and stop a profiling measurement.
*     They test if profiling is enabled at all and then call the
*     functions to start or stop a measurement.
*
*  SEE ALSO
*     uti/profiling/prof_mt_init()
*     uti/profiling/sge_prof_cleanup()
*     uti/profiling/prof_is_active()
*     uti/profiling/prof_start_measurement()
*     uti/profiling/prof_stop_measurement()
*******************************************************************************/

/* this struct is used for pthread name, thread id mapping
   It also holds the profiling status information for each thread */
typedef struct {
   const char* thrd_name;
   pthread_t   thrd_id;
   bool        prof_is_active;
   int         is_terminated;
} sge_thread_info_t;

static int sge_prof_array_initialized = 0;

static void        prof_info_init(prof_level level);
static void        prof_info_level_init(prof_level i, int thread_num);
static void        prof_reset_thread(int thread_num, prof_level level);
static void        init_array(pthread_t num);
static void        init_array_first(void);
static void        init_thread_info(void);
static int         get_prof_info_thread_id(void);

static sge_prof_info_t **theInfo = NULL;
static sge_thread_info_t *thrdInfo = NULL;

static pthread_mutex_t thrdInfo_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t thread_id_key;

static bool profiling_enabled = true;

static pthread_once_t prof_once = PTHREAD_ONCE_INIT;

/*
 * TODO: If application starts more than MAX_THREAD_NUM threads,
 *       this value must be increased:
 *
 */
static const int MAX_THREAD_NUM = 64;

/****** uti/profiling/sge_prof_set_enabled() ***********************************
*  NAME
*     sge_prof_set_enabled() -- enables/disables profiling
*
*  SYNOPSIS
*     void sge_prof_set_enabled(bool enabled)
*
*  FUNCTION
*     Enables/disables profiling completely.  Profiling is enabled by
*     default.  This method is the fix to Issue 1471.
*
*  INPUTS
*     bool enabled   Whether profiling should be enabled
*  RESULT
*
*  NOTES
*     MT-NOTE: sge_prof_set_enabled() is MT safe only if called before any other
*              profiling calls
*
*  SEE ALSO
*******************************************************************************/
void sge_prof_set_enabled(bool enabled) {
   profiling_enabled = enabled;
}

/****** uti/profiling/prof_thread_local_once_init() ************************************
*  NAME
*     prof_thread_local_once_init() -- inititalizes the profiling array 
*
*  SYNOPSIS
*     void prof_thread_local_once_init(void) 
*
*  FUNCTION
*     Initializes the profiling array.
*
*  INPUTS
*
*  RESULT
*
*  NOTES
*     MT-NOTE: prof_thread_local_once_init() is MT safe only if called before any other
*              profiling calls other than sge_prof_set_enabled()
*
*  SEE ALSO
*******************************************************************************/
static void prof_thread_local_once_init(void)
{
   if (!profiling_enabled) {
      return;
   }
   
   init_thread_info();
   init_array_first();
   init_array(pthread_self());
}

void prof_mt_init(void) {
   pthread_once(&prof_once, prof_thread_local_once_init);
}


/****** uti/profiling/prof_set_level_name() ************************************
*  NAME
*     prof_set_level_name() -- set name of a custom level
*
*  SYNOPSIS
*     bool prof_set_level_name(prof_level level, const char *name) 
*
*  FUNCTION
*     Set the name of a custom profiling level.
*
*  INPUTS
*     prof_level level - level to edit
*     const char *name - new name for level
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_set_level_name() is MT safe
*
*  SEE ALSO
*     uti/profiling/--Profiling
*******************************************************************************/
bool prof_set_level_name(prof_level level, const char *name, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   bool ret = true;

   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_set_level_name", level);
      ret = false;
   } else if (name == NULL) {
      sge_dstring_sprintf_append(error, MSG_PROF_NULLLEVELNAME_S, "prof_set_level_name");
      ret = false;
   } else {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         ret = false;
      } else {
         theInfo[thread_num][level].name = name;
      }
   }
   
   return ret;
}

/****** uti/profiling/prof_is_active() ****************************************
*  NAME
*     prof_is_active() -- is profiling active?
*
*  SYNOPSIS
*     bool prof_is_active(void) 
*
*  FUNCTION
*     Returns true, if profiling is active, else false.
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  SEE ALSO
*     uti/profiling/prof_stop()
*     uti/profiling/prof_stop()
*
*  NOTES
*     MT-NOTE: prof_is_active() is MT safe
*******************************************************************************/
bool prof_is_active(prof_level level)
{
   int thread_num;
   pthread_t thread_id;
   bool ret = false;

   if (profiling_enabled && (level <= SGE_PROF_ALL)) {
      thread_id = pthread_self();
      
      thread_num = get_prof_info_thread_id();
      if ((thread_num >= 0) && (thread_num < MAX_THREAD_NUM)) {
         ret = theInfo[thread_num][level].prof_is_started;
      }
   }
   
   return ret;
}

/****** uti/profiling/prof_start() ****************************************
*  NAME
*     prof_start() -- start profiling
*
*  SYNOPSIS
*     bool prof_start(dstring *error) 
*
*  FUNCTION
*     Enables profiling. All internal variables are reset to 0.
*     Performance measurement has to be started and stopped by
*     calling profiling_start_measurement or profiling_stop_measurement.
*
*  INPUTS
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  SEE ALSO
*     uti/profiling/prof_stop()
*     uti/profiling/prof_start_measurement()
*     uti/profiling/prof_stop_measurement()
*
*  NOTES
*     MT-NOTE: prof_start() is MT safe
*******************************************************************************/
bool prof_start(prof_level level, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   bool ret = true;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_start", level);
      ret = false;
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_start");
         ret = false;
      } else if (theInfo[thread_num][level].prof_is_started) {
         sge_dstring_sprintf_append(error, MSG_PROF_ALREADYACTIVE_S, "prof_start");
         ret = false;
      } else {
         struct tms tms_buffer;
         prof_level i;
         clock_t start_time = times(&tms_buffer);

         if (level == SGE_PROF_ALL) {
            for (i = SGE_PROF_OTHER; i <= SGE_PROF_ALL; i++) {
               /* get start time */
               theInfo[thread_num][i].start_clock = start_time;

               /* initialize names and reset all data */
               ret = prof_reset(i, error);

               theInfo[thread_num][i].prof_is_started = true;

               theInfo[thread_num][i].ever_started = true;
            } 
         } else {
            /* get start time */
            theInfo[thread_num][level].start_clock = start_time;

            /* initialize names and reset all data */
            ret = prof_reset(level, error);

            theInfo[thread_num][level].prof_is_started = true;
            /* TODO
             * Is there a reason we do this?  This flag in never cleared. */
            theInfo[thread_num][SGE_PROF_ALL].prof_is_started = true;

            theInfo[thread_num][level].ever_started = true;
         }
         
         /* we have no actual profiling level */
         theInfo[thread_num][SGE_PROF_ALL].akt_level = SGE_PROF_NONE;

         /* implicitly start the OTHER level */
         prof_start_measurement(SGE_PROF_OTHER, error);
      }
   }

   return ret;
}

/****** uti/profiling/prof_stop() *****************************************
*  NAME
*     prof_stop() -- stop profiling
*
*  SYNOPSIS
*     bool prof_stop(dstring *error) 
*
*  FUNCTION
*     Profiling is disabled.
*
*     Subsequent calls to profiling_start_measurement or 
*     profiling_stop_measurement will have no effect.
*
*     Profiling can be re-enabled by calling profiling_start.
*
*  INPUTS
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  SEE ALSO
*     uti/profiling/prof_start()
*     uti/profiling/prof_start_measurement()
*     uti/profiling/prof_stop_measurement()
*
*  NOTES
*     MT-NOTE: prof_start() is MT safe
*******************************************************************************/
bool prof_stop(prof_level level, dstring *error)
{  
   pthread_t thread_id;
   int thread_num;
   bool ret = true;
   prof_level i;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_stop", level);
      ret = false;
   } else if (profiling_enabled) {
      thread_id = pthread_self();
      
      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_stop");
         ret = false;
      } else if (!theInfo[thread_num][level].prof_is_started) {
         sge_dstring_sprintf_append(error, MSG_PROF_NOTACTIVE_S, "prof_stop");
         ret = false;
      } else {
         prof_stop_measurement(SGE_PROF_OTHER, error);

         if (level == SGE_PROF_ALL) {
            for (i = SGE_PROF_OTHER; i <= SGE_PROF_ALL; i++) {
               theInfo[thread_num][i].prof_is_started = false;
            }
         } else {
            theInfo[thread_num][level].prof_is_started = false;
         }
      }
   }

   return ret;
}

/****** uti/profiling/prof_start_measurement() ****************************
*  NAME
*     prof_start_measurement() -- start measurement
*
*  SYNOPSIS
*     bool prof_start_measurement(prof_level level, dstring *error) 
*
*  FUNCTION
*     Starts measurement of performance data.
*     Retrieves and stores current time and usage information.
*
*  INPUTS
*     prof_level level - level to process
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  SEE ALSO
*     uti/profiling/prof_stop_measurement()
*
*  NOTES
*     MT-NOTE: prof_start_measurement() is MT safe
*******************************************************************************/
bool prof_start_measurement(prof_level level, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   bool ret = true;

   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_start_measurement", level);
      ret = false;
   } else if (profiling_enabled) {
      thread_id = pthread_self();
      
      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_start_measurement");
         ret = false;
      } else if (!theInfo[thread_num][level].prof_is_started) { 
         sge_dstring_sprintf_append(error, MSG_PROF_NOTACTIVE_S, "prof_start_measurement");
         ret = false;
      } else if (theInfo[thread_num][SGE_PROF_ALL].akt_level == level) {
         /* multiple start_measurement calls within one level are allowed */
         theInfo[thread_num][level].nested_calls++;
      } else if (theInfo[thread_num][level].pre != SGE_PROF_NONE) {
         /* we cannot yet handle cyclic measurements between multiple levels
          * produce an error and stop profiling
          */
         sge_dstring_sprintf_append(error, MSG_PROF_CYCLICNOTALLOWED_SD, "prof_start_measurement", level);
         prof_stop(level, error);
         ret = false;
      } else {
         theInfo[thread_num][level].pre = theInfo[thread_num][SGE_PROF_ALL].akt_level;
         theInfo[thread_num][SGE_PROF_ALL].akt_level = level;

         theInfo[thread_num][level].start = times(&(theInfo[thread_num][level].tms_start));
         /* when we start a level, we have no sub usage */
         theInfo[thread_num][level].sub = 0;
         theInfo[thread_num][level].sub_utime = 0;
         theInfo[thread_num][level].sub_utime= 0;
      }
   }

   return ret;
}

/****** uti/profiling/prof_stop_measurement() *****************************
*  NAME
*     prof_stop_measurement() -- stop measurement
*
*  SYNOPSIS
*     bool prof_stop_measurement(prof_level level, dstring *error) 
*
*  FUNCTION
*     Stops measurement for a certain code block.
*     Retrieves and stores current time and usage information.
*     Sums up global usage information.
*
*  INPUTS
*     prof_level level - level to process
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  SEE ALSO
*     uti/profiling/prof_start_measurement()
*
*  NOTES
*     MT-NOTE: prof_stop_measurement() is MT safe
*******************************************************************************/
bool prof_stop_measurement(prof_level level, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   bool ret = true;

   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_stop_measurement", level);
      ret = false;
   } else if (profiling_enabled) {
      thread_id = pthread_self();
      
      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_stop_measurement");
         ret = false;
      } else if (!theInfo[thread_num][level].prof_is_started) {
         sge_dstring_sprintf_append(error, MSG_PROF_NOTACTIVE_S, "prof_stop_measurement");
         ret = false;
      } else {
         clock_t time, utime, stime;

         if (theInfo[thread_num][level].nested_calls > 0) {
            theInfo[thread_num][level].nested_calls--;
         } else {
            theInfo[thread_num][level].end = times(&(theInfo[thread_num][level].tms_end));
            time  = theInfo[thread_num][level].end - theInfo[thread_num][level].start;
            utime = theInfo[thread_num][level].tms_end.tms_utime - theInfo[thread_num][level].tms_start.tms_utime;
            stime = theInfo[thread_num][level].tms_end.tms_stime - theInfo[thread_num][level].tms_start.tms_stime;

#if 0
if (time < (utime + stime)) {
   DPRINTF(("---> utime + stime > time, difference is %d clock ticks\n", (utime + stime) - time));
}
#endif
            theInfo[thread_num][level].total += time;
            theInfo[thread_num][level].total_utime += utime;
            theInfo[thread_num][level].total_stime += stime;


            if (theInfo[thread_num][level].pre != SGE_PROF_NONE) {
               prof_level pre = theInfo[thread_num][level].pre;

               theInfo[thread_num][pre].sub += time;
               theInfo[thread_num][pre].sub_utime += utime;
               theInfo[thread_num][pre].sub_stime += stime;

               theInfo[thread_num][pre].sub_total += time;
               theInfo[thread_num][pre].sub_total_utime += utime;
               theInfo[thread_num][pre].sub_total_stime += stime;

               theInfo[thread_num][SGE_PROF_ALL].akt_level = theInfo[thread_num][level].pre;
               theInfo[thread_num][level].pre = SGE_PROF_NONE;
            } else {
               theInfo[thread_num][SGE_PROF_ALL].akt_level = SGE_PROF_NONE;
            }
         }
      }
   }

   return ret;
}

/****** uti/profiling/prof_reset() ****************************************
*  NAME
*     prof_reset() -- reset usage information
*
*  SYNOPSIS
*     bool prof_reset(dstring *error) 
*
*  FUNCTION
*     Reset usage and timing information to 0.
*
*  INPUTS
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     bool - true on success, else false is returned and an error message 
*            is returned in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_reset() is MT safe
*******************************************************************************/
bool prof_reset(prof_level level, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   bool ret = true;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_reset", level);
      ret = false;
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_reset");
         ret = false;
      } else if (theInfo[thread_num][SGE_PROF_ALL].akt_level > SGE_PROF_OTHER) {
         sge_dstring_sprintf_append(error, MSG_PROF_RESETWHILEMEASUREMENT_S, "prof_reset");
         ret = false;
      } else {
         if (theInfo[thread_num][level].prof_is_started) {
            ret = prof_stop_measurement(SGE_PROF_OTHER, error);
         }
         if (level == SGE_PROF_ALL) {
            prof_level c;
            for (c = SGE_PROF_OTHER; c <= SGE_PROF_ALL; c++) {
               prof_reset_thread (thread_num, c);
            }
         } else {
            prof_reset_thread (thread_num, level);
         }

         if (theInfo[thread_num][level].prof_is_started) {
            ret = prof_start_measurement(SGE_PROF_OTHER, error);
         }
      }
   }

   return ret;
}

/****** uti/profiling/prof_reset_thread() **************************************
*  NAME
*     prof_reset_thread() -- reset usage information
*
*  SYNOPSIS
*     bool prof_reset_thread(int thread_num, prof_level level) 
*
*  FUNCTION
*     Reset usage and timing information for a single thread and level to 0.
*
*  INPUTS
*     int thread_num    the thread to reset
*     prof_level level  the profiling level to reset
*
*  NOTES
*     MT-NOTE: prof_reset() is MT safe
*******************************************************************************/
static void prof_reset_thread(int thread_num, prof_level level)
{
   struct tms tms_buffer;

   theInfo[thread_num][level].start = 0;
   theInfo[thread_num][level].end = 0;
   theInfo[thread_num][level].tms_start.tms_utime = 0;
   theInfo[thread_num][level].tms_start.tms_stime = 0;
   theInfo[thread_num][level].tms_start.tms_cutime = 0;
   theInfo[thread_num][level].tms_start.tms_cstime = 0;
   theInfo[thread_num][level].tms_end.tms_utime = 0;
   theInfo[thread_num][level].tms_end.tms_stime = 0;
   theInfo[thread_num][level].tms_end.tms_cutime = 0;
   theInfo[thread_num][level].tms_end.tms_cstime = 0;
   theInfo[thread_num][level].total = 0;
   theInfo[thread_num][level].total_utime = 0;
   theInfo[thread_num][level].total_stime = 0;

   theInfo[thread_num][level].pre = SGE_PROF_NONE;
   theInfo[thread_num][level].sub = 0;
   theInfo[thread_num][level].sub_utime = 0;
   theInfo[thread_num][level].sub_stime = 0;
   theInfo[thread_num][level].sub_total = 0;
   theInfo[thread_num][level].sub_total_utime = 0;
   theInfo[thread_num][level].sub_total_stime = 0;

   theInfo[thread_num][level].start_clock = times(&tms_buffer);
}

/****** uti/profiling/prof_get_measurement_wallclock() ********************
*  NAME
*     prof_get_measurement_wallclock() -- return wallclock of a measurement
*
*  SYNOPSIS
*     double prof_get_measurement_wallclock(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns the wallclock of the last measurement in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the wallclock time of the last measurement
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  RESULT
*     double - the wallclock time
*
*  NOTES
*     MT-NOTE: prof_get_measurement_wallclock() is MT safe
*******************************************************************************/
double prof_get_measurement_wallclock(prof_level level, bool with_sub, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   clock_t clock = 0;
   double ret = 0.0;
   
   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_measurement_wallclock", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_measurement_wallclock");
      } else {
         clock = theInfo[thread_num][level].end - theInfo[thread_num][level].start;

         if (!with_sub) {
            clock -= theInfo[thread_num][level].sub;
         }
      }

      ret = clock * 1.0 / sysconf(_SC_CLK_TCK);
   }
   
   return ret;
}

/****** uti/profiling/prof_get_measurement_utime() ************************
*  NAME
*     prof_get_measurement_utime() -- return user cpu time of measurement
*
*  SYNOPSIS
*     double prof_get_measurement_utime(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns the user cpu time of the last measurement in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the user cpu time of the last measurement
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_measurement_utime() is not MT safe
*******************************************************************************/
double prof_get_measurement_utime(prof_level level, bool with_sub, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   clock_t clock = 0;
   double ret = 0.0;
   
   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_measurement_utime", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_measurement_utime");
      } else {
         clock = (theInfo[thread_num][level].tms_end.tms_utime - theInfo[thread_num][level].tms_start.tms_utime);

         if (!with_sub) {
            clock -= theInfo[thread_num][level].sub_utime;
         }
      }

      ret = clock * 1.0 / sysconf(_SC_CLK_TCK);
   }
   
   return ret;
}

/****** uti/profiling/prof_get_measurement_stime() ************************
*  NAME
*     prof_get_measurement_stime() -- return system cpu time of measurement
*
*  SYNOPSIS
*     double prof_get_measurement_stime(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns the system cpu time of the last measurement in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the system cpu time of the last measurement
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_measurement_stime() is MT safe
*******************************************************************************/
double prof_get_measurement_stime(prof_level level, bool with_sub, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   clock_t clock = 0;
   double ret = 0.0;

   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_measurement_stime", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_measurement_stime");
      } else {
         clock = (theInfo[thread_num][level].tms_end.tms_stime - theInfo[thread_num][level].tms_start.tms_stime);

         if (!with_sub) {
            clock -= theInfo[thread_num][level].sub_stime;
         }
      }

      ret = clock * 1.0 / sysconf(_SC_CLK_TCK);
   }
   
   return ret;
}

/****** uti/profiling/prof_get_total_wallclock() **************************
*  NAME
*     prof_get_total_wallclock() -- get total wallclock time
*
*  SYNOPSIS
*     double prof_get_total_wallclock(dstring *error) 
*
*  FUNCTION
*     Returns the wallclock time since profiling was enabled in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the total wallclock time of the profiling run
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_total_wallclock() is MT safe
*******************************************************************************/

double prof_get_total_wallclock(prof_level level, dstring *error)
{
   pthread_t thread_id;
   int thread_num;
   double ret = 0.0;

   if (level >= SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_total_wallclock", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_total_wallclock");
      } else if (!theInfo[thread_num][level].prof_is_started) {
         sge_dstring_sprintf_append(error, MSG_PROF_NOTACTIVE_S, "prof_get_total_wallclock");
      } else {
         struct tms tms_buffer;
         clock_t now;

         now = times(&tms_buffer);

         ret = (now - theInfo[thread_num][level].start_clock) * 1.0 / sysconf(_SC_CLK_TCK);
      }
   }

   return ret;
}

/****** uti/profiling/prof_get_total_busy() *******************************
*  NAME
*     prof_get_total_busy() -- return total busy time
*
*  SYNOPSIS
*     double prof_get_total_busy(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns the total busy time since profiling was enabled in seconds.
*     Busy time is the time between starting and stopping a measurement.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the total busy time of the profiling run
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_total_busy() is MT safe
*******************************************************************************/
static double _prof_get_total_busy(prof_level level, bool with_sub, dstring *error _UNUSED)
{
   int thread_num;
   clock_t clock = 0;

   thread_num = get_prof_info_thread_id();

   clock = theInfo[thread_num][level].total;

   if (!with_sub) {
      clock -= theInfo[thread_num][level].sub_total;
   }

   return clock * 1.0 / sysconf(_SC_CLK_TCK);
}

double prof_get_total_busy(prof_level level, bool with_sub, dstring *error)
{
   double ret = 0.0;
   pthread_t thread_id;
   int thread_num;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_total_busy", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();
      
      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_total_busy");
      } else if (level == SGE_PROF_ALL) {
         prof_level i;

         for (i = SGE_PROF_OTHER; i < SGE_PROF_ALL; i++) {
            ret += _prof_get_total_busy(i, with_sub, error);
         }
      } else {
         ret = _prof_get_total_busy(level, with_sub, error);
      }
   }

   return ret;
}

/****** uti/profiling/prof_get_total_utime() ******************************
*  NAME
*     prof_get_total_utime() -- get total user cpu time
*
*  SYNOPSIS
*     double prof_get_total_utime(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns the user cpu time since profiling was enabled in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the total user cpu time of the profiling run
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_total_utime() is MT safe
*******************************************************************************/
static double _prof_get_total_utime(prof_level level, bool with_sub, dstring *error _UNUSED)
{
   int thread_num;
   clock_t clock = 0;

   thread_num = get_prof_info_thread_id();

   clock = theInfo[thread_num][level].total_utime;

   if (!with_sub) {
      clock -= theInfo[thread_num][level].sub_total_utime;
   }

   return clock * 1.0 / sysconf(_SC_CLK_TCK);
}

double prof_get_total_utime(prof_level level, bool with_sub, dstring *error)
{
   double ret = 0.0;
   pthread_t thread_id;
   int thread_num;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_total_utime", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_total_utime");         
      } else if (level == SGE_PROF_ALL) {
         prof_level i;

         for (i = SGE_PROF_OTHER; i < SGE_PROF_ALL; i++) {
            ret += _prof_get_total_utime(i, with_sub, error);
         }
      } else {
         ret = _prof_get_total_utime(level, with_sub, error);
      }
   }

   return ret;
}

/****** uti/profiling/prof_get_total_stime() ******************************
*  NAME
*     prof_get_total_stime() -- get total system cpu time
*
*  SYNOPSIS
*     double prof_get_total_stime(prof_level level, bool with_sub) 
*
*  FUNCTION
*     Returns the total system cpu time since profiling was enabled in seconds.
*     Resolution is clock ticks (_SC_CLK_TCK).
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     double - the total system cpu time of the profiling run
*              on error, 0 is returned and an error message is written 
*              to the buffer given in parameter error, if error != NULL
*
*  NOTES
*     MT-NOTE: prof_get_total_stime() is MT safe
*******************************************************************************/
static double _prof_get_total_stime(prof_level level, bool with_sub, dstring *error _UNUSED)
{
   int thread_num;
   clock_t clock = 0;

   thread_num = get_prof_info_thread_id();

   clock = theInfo[thread_num][level].total_stime;

   if (!with_sub) {
      clock -= theInfo[thread_num][level].sub_total_stime;
   }

   return clock * 1.0 / sysconf(_SC_CLK_TCK);
}

double prof_get_total_stime(prof_level level, bool with_sub, dstring *error)
{
   double ret = 0.0;
   pthread_t thread_id;
   int thread_num;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_total_stime", level);
   } else if (profiling_enabled) {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_total_stime");
      } else if (level == SGE_PROF_ALL) {
         prof_level i;

         for (i = SGE_PROF_OTHER; i < SGE_PROF_ALL; i++) {
            ret += _prof_get_total_stime(i, with_sub, error);
         }
      } else {
         ret = _prof_get_total_stime(level, with_sub, error);
      }
   }

   return ret;
}

/****** uti/profiling/prof_get_info_string() ******************************
*  NAME
*     prof_get_info_string() -- get informational message 
*
*  SYNOPSIS
*     const char* prof_get_info_string(prof_level level, bool with_sub, dstring *error) 
*
*  FUNCTION
*     Returns a string containing the most interesting data, both for the
*     last measurement and for the total runtime:
*        - wallclock of measurement
*        - user cpu time of measurement
*        - system cpu time of measurement
*        - total wallclock time (runtime)
*        - total busys time
*        - utilization (busy time / wallclock time) * 100 %
*
*  INPUTS
*     prof_level level - level to process
*     bool with_sub    - include usage of subordinated measurements?
*     dstring *error   - if != NULL, error messages will be put here
*
*  RESULT
*     const char* - pointer to result string. It is valid until the next
*                   call of prof_get_info_string()
*                   on error, 0 is returned and an error message is written 
*                   to the buffer given in parameter error, if error != NULL
*
*  EXAMPLE
*     The result can look like the following:
*     "wc = 0.190s, utime = 0.120s, stime = 0.000s, runtime 9515s, busy 105s, 
*     utilization 1%"
*
*  NOTES
*     MT-NOTE: prof_get_info_string() is MT safe
*******************************************************************************/

#define PROF_GET_INFO_FORMAT "%-15.15s: wc = %10.3fs, utime = %10.3fs, stime = %10.3fs, utilization = %3.0f%%\n"

static const char *
_prof_get_info_string(prof_level level, dstring *info_string, bool with_sub, dstring *error)
{  
   int thread_num;
   dstring level_string = DSTRING_INIT;
   const char *ret = NULL;
   double busy, utime, stime, utilization;

   thread_num = get_prof_info_thread_id();

   busy        = prof_get_total_busy(level, with_sub, error);
   utime       = prof_get_total_utime(level, with_sub, error);
   stime       = prof_get_total_stime(level, with_sub, error);
   utilization = busy > 0 ? (utime + stime) / busy * 100 : 0;

   sge_dstring_sprintf(&level_string, PROF_GET_INFO_FORMAT,
     theInfo[thread_num][level].name, busy, utime, stime, utilization);

   ret = sge_dstring_append_dstring(info_string, &level_string);
   sge_dstring_free(&level_string);

   return ret;
}


const char *
prof_get_info_string(prof_level level, bool with_sub, dstring *error)
{   
   pthread_t thread_id;
   int thread_num;
   const char *ret = NULL;

   if (level > SGE_PROF_ALL) {
      sge_dstring_sprintf_append(error, MSG_PROF_INVALIDLEVEL_SD, "prof_get_info_string", level);
      ret = sge_dstring_get_string (error);
   } else if (!profiling_enabled) {
      ret = "Profiling disabled";
   } else {
      thread_id = pthread_self();

      thread_num = get_prof_info_thread_id();

      if ((thread_num < 0) || (thread_num >= MAX_THREAD_NUM)) {
         sge_dstring_sprintf_append(error, MSG_PROF_MAXTHREADSEXCEEDED_S, "prof_get_info_string");
      /* total summary: one line for each level, one line for summary */
      } else if (level == SGE_PROF_ALL) {
         double busy, utime, stime, utilization;
         prof_level i;
         dstring total_string = DSTRING_INIT;

         for (i = SGE_PROF_OTHER; i <= SGE_PROF_ALL; i++) {
            /* clear previous contents */
            sge_dstring_clear(&(theInfo[thread_num][i].info_string));
         }

         prof_stop_measurement(SGE_PROF_OTHER, error);

         busy        = prof_get_total_busy(SGE_PROF_ALL, with_sub, error);
         utime       = prof_get_total_utime(SGE_PROF_ALL, with_sub, error);
         stime       = prof_get_total_stime(SGE_PROF_ALL, with_sub, error);
         utilization = busy > 0 ? (utime + stime) / busy * 100 : 0;

         for (i = SGE_PROF_OTHER; i < SGE_PROF_ALL; i++) {
            if (theInfo[thread_num][i].name != NULL && theInfo[thread_num][i].ever_started == true) {
               _prof_get_info_string(i, &theInfo[thread_num][SGE_PROF_ALL].info_string, with_sub, error);
            }
         }

         prof_start_measurement(SGE_PROF_OTHER, error);

         sge_dstring_sprintf(&total_string, PROF_GET_INFO_FORMAT,
            "total", busy, utime, stime, utilization);

         ret = sge_dstring_append_dstring(&theInfo[thread_num][SGE_PROF_ALL].info_string, &total_string);

         sge_dstring_free(&total_string);
      } else {

         /* clear previous contents */
         sge_dstring_clear(&(theInfo[thread_num][level].info_string));

         if (theInfo[thread_num][level].name != NULL) {
            ret = _prof_get_info_string(level, &theInfo[thread_num][level].info_string, with_sub, error);
         }
      }
   }

   return ret;
}


bool prof_output_info(prof_level level, bool with_sub, const char *info)
{
   bool ret = false;

   DENTER(TOP_LAYER, "prof_output_info");
   
   if (profiling_enabled && (level <= SGE_PROF_ALL)) {
      int thread_num;
      pthread_t thread_id;

      thread_id = pthread_self();
      thread_num = get_prof_info_thread_id();

      if ((thread_num >= 0) && (thread_num < MAX_THREAD_NUM) && prof_is_active(level)) {
         const char *info_message = NULL;
         const char *message = NULL;
         struct saved_vars_s *context = NULL;

         info_message = prof_get_info_string(level, with_sub, NULL);
         PROFILING((SGE_EVENT, "PROF(%d): %s%s", (int)thread_id, info, ""));
         for (message = sge_strtok_r(info_message, "\n", &context); message != NULL; 
              message = sge_strtok_r(NULL, "\n", &context)) {
            PROFILING((SGE_EVENT, "PROF(%d): %s", (int)thread_id, message));
         }
         prof_reset(level, NULL);

         sge_free_saved_vars(context);
         ret = true;
      }
   }

   DRETURN(ret);
}


/****** uti/profiling/prof_info_init() ******************************
*  NAME
*     prof_info_init() -- initialize the sge_prof_info_t struc array with default values 
*
*  SYNOPSIS
*     static void prof_info_init(prof_level level) 
*
*  FUNCTION
*     initialize the sge_prof_info_t struct array with default values
*
*  INPUTS
*     prof_level level 
*
*  RESULT
*     initialized sge_prof_info_t array for the given profiling level
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: prof_info_init() is MT safe
*******************************************************************************/
static void prof_info_init(prof_level level)
{
   int thread_num;

   thread_num = get_prof_info_thread_id();

   if (level <= SGE_PROF_ALL) {
      if (level == SGE_PROF_ALL) {
         prof_level i;
         for (i = SGE_PROF_OTHER; i <= SGE_PROF_ALL; i++) {
            prof_info_level_init (i, thread_num);
         }
      } else {
         prof_info_level_init (SGE_PROF_OTHER, thread_num);
      }     

      theInfo[thread_num][SGE_PROF_ALL].akt_level = SGE_PROF_NONE;
   }
   
   return;
}

/****** uti/profiling/prof_info_level_init() ******************************
*  NAME
*     prof_info_level_init() -- initialize the sge_prof_info_t struc array with default values 
*
*  SYNOPSIS
*     static void prof_info_level_init(prof_level level) 
*
*  FUNCTION
*     initialize the sge_prof_info_t struct array with default values
*
*  INPUTS
*     prof_level i
*     int thread_num
*
*  RESULT
*     initialized sge_prof_info_t array for the given profiling level
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: prof_info_level_init() is MT safe
*******************************************************************************/
static void prof_info_level_init(prof_level i, int thread_num)
{
   switch (i) {
     case SGE_PROF_OTHER:
       theInfo[thread_num][i].name = "other";
       break;
     case SGE_PROF_COMMUNICATION:
       theInfo[thread_num][i].name = "communication";
       break;
     case SGE_PROF_PACKING:
       theInfo[thread_num][i].name = "packing";
       break;
     case SGE_PROF_EVENTCLIENT:
       theInfo[thread_num][i].name = "eventclient";
       break;
     case SGE_PROF_EVENTMASTER:
       theInfo[thread_num][i].name = "eventmaster";
       break;
     case SGE_PROF_MIRROR:
       theInfo[thread_num][i].name = "mirror";
       break;
     case SGE_PROF_SPOOLING:
       theInfo[thread_num][i].name = "spooling";
       break;
     case SGE_PROF_SPOOLINGIO:
       theInfo[thread_num][i].name = "spooling-io";
       break;
     case SGE_PROF_JOBSCRIPT:
       theInfo[thread_num][i].name = "spooling-script";
       break;
     case SGE_PROF_GDI:
       theInfo[thread_num][i].name = "gdi";
       break;
     case SGE_PROF_GDI_REQUEST:
       theInfo[thread_num][i].name = "gdi_request";
       break;
     case SGE_PROF_HT_RESIZE:
       theInfo[thread_num][i].name = "ht-resize";
       break;
     case SGE_PROF_ALL:
       theInfo[thread_num][i].name = "all";
       break;
     default:
       theInfo[thread_num][i].name = NULL; /* "custom"*/  
       break;
   }

   theInfo[thread_num][i].nested_calls = 0;
   theInfo[thread_num][i].start = 0;
   theInfo[thread_num][i].end = 0;
   theInfo[thread_num][i].tms_start.tms_utime = 0;
   theInfo[thread_num][i].tms_start.tms_stime = 0;
   theInfo[thread_num][i].tms_start.tms_cutime = 0;
   theInfo[thread_num][i].tms_start.tms_cstime = 0;
   theInfo[thread_num][i].tms_end.tms_utime = 0;
   theInfo[thread_num][i].tms_end.tms_stime = 0;
   theInfo[thread_num][i].tms_end.tms_cutime = 0;
   theInfo[thread_num][i].tms_end.tms_cstime = 0;
   theInfo[thread_num][i].total = 0;
   theInfo[thread_num][i].total_utime = 0;
   theInfo[thread_num][i].total_stime = 0;

   theInfo[thread_num][i].pre = SGE_PROF_NONE;
   theInfo[thread_num][i].sub = 0;
   theInfo[thread_num][i].sub_utime = 0;
   theInfo[thread_num][i].sub_stime = 0;
   theInfo[thread_num][i].sub_total = 0;
   theInfo[thread_num][i].sub_total_utime = 0;
   theInfo[thread_num][i].sub_total_stime = 0;

   theInfo[thread_num][i].prof_is_started = false;
   theInfo[thread_num][i].start_clock = 0;
   theInfo[thread_num][i].ever_started = false;

   theInfo[thread_num][i].info_string.s = NULL;
   theInfo[thread_num][i].info_string.length = 0;
   theInfo[thread_num][i].info_string.size = 0;
   theInfo[thread_num][i].info_string.is_static = false;

}

/****** uti/profiling/init_array() ******************************
*  NAME
*     init_array() -- mallocs memory for the sge_prof_info_t array 
*
*  SYNOPSIS
*     void init_array(pthread_t num) 
*
*  FUNCTION
*     mallocs memory for sge_prof_info_t array for the number
*     of MAX_THREAD_NUM threads
*     mallocs memory for each thread if nedded 
*
*  INPUTS
*     the thread id, which needs malloced memory 
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: init_array() is MT safe
*******************************************************************************/
static void init_array(pthread_t num) {

   int i, c;
   
   DENTER(CULL_LAYER,"init_array");
   
   if (sge_prof_array_initialized == 0) {
      CRITICAL((SGE_EVENT,"Profiling array is not initialized!"));
      abort();
   }

   pthread_mutex_lock(&thrdInfo_mutex);
      
   for (i = 0; i < MAX_THREAD_NUM; i++) {
      if (theInfo[i] != NULL && theInfo[i][SGE_PROF_ALL].thread_id == num) {
         break;
      } else if (theInfo[i] == NULL) {
         long storage = 0;
         
         theInfo[i] = (sge_prof_info_t*)sge_malloc((SGE_PROF_ALL + 1) * sizeof(sge_prof_info_t));
         memset (theInfo[i], 0, (SGE_PROF_ALL + 1) * sizeof(sge_prof_info_t));

         for (c = 0; c <= SGE_PROF_ALL; c++) {
             theInfo[i][c].thread_id = num;
         }

         /* Rather than malloc'ing space for an int that we'll have to clean up
          * later, we just store the int directly in the thread local storage.
          * In order to keep Linux from complaining, the value we store must be
          * the same size as a void pointer, i.e. a long. */
         storage = i;
         pthread_setspecific (thread_id_key, (void *)storage);
         
         prof_info_init(SGE_PROF_ALL);

         break;
      }
   }
      
   pthread_mutex_unlock(&thrdInfo_mutex);

   DEXIT;
   return;
   
}

/* per process initialization */
static void init_array_first(void) {

   if (sge_prof_array_initialized == 0) {
      pthread_mutex_lock(&thrdInfo_mutex);

      if (pthread_key_create (&thread_id_key, NULL) == 0) {
         theInfo = (sge_prof_info_t**)sge_malloc(MAX_THREAD_NUM * sizeof(sge_prof_info_t*));
         memset (theInfo, 0, MAX_THREAD_NUM * sizeof(sge_prof_info_t*));
         sge_prof_array_initialized = 1;
      }

      pthread_mutex_unlock(&thrdInfo_mutex);
   }

}

/****** uti/profiling/init_thread_info() ******************************
*  NAME
*     init_thread_info() -- mallocs memory for the thread_info_t array 
*
*  SYNOPSIS
*     void init_thread_info(void) 
*
*  FUNCTION
*     mallocs memory for thread_info_t array (thread name/id mapping) 
*     for the number of MAX_THREAD_NUM threads. Must be called once
*     per process.
*
*  INPUTS
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: init_thread_info() is MT safe
*******************************************************************************/
static void init_thread_info(void) {

   if (!profiling_enabled) {
      return;
   }
   
   pthread_mutex_lock(&thrdInfo_mutex);

   if (thrdInfo == NULL) {
      thrdInfo = (sge_thread_info_t*)sge_malloc(MAX_THREAD_NUM * sizeof(sge_thread_info_t));
      memset(thrdInfo, 0, MAX_THREAD_NUM * sizeof(sge_thread_info_t));
   }

   pthread_mutex_unlock(&thrdInfo_mutex);

}



/****** uti/profiling/get_thread_info() ******************************
*  NAME
*     get_thread_info() -- get the thread name/id mapping array 
*
*  SYNOPSIS
*     thread_info_t* get_thread_info(void) 
*
*  FUNCTION
*     if the thread name/id mapping array is not initialized
*     it will be done
*
*  INPUTS
*
*  RESULT
*     returns a pointer to the thread name/id mapping array
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: get_thread_info() is MT safe
*******************************************************************************/
sge_thread_info_t* get_thread_info(void) {

   if (!profiling_enabled) {
      return NULL;
   }
   
   init_thread_info();
   return thrdInfo;

}



/****** uti/profiling/set_thread_name() ******************************
*  NAME
*     set_thread_name() -- set the thread name mapped to its id 
*
*  SYNOPSIS
*     void set_thread_name(pthread_t thread_id, const char* thread_name) 
*
*  FUNCTION
*     maps the name and the id of a thread
*     set the thread profiling status to false
*
*  INPUTS
*     pthread_t thread_id
*     const char* thread_name
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: set_thread_info() is MT safe
*******************************************************************************/
void set_thread_name(pthread_t thread_id, const char* thread_name) {

   int thread_num;

   if (!profiling_enabled) {
      return;
   }
   
   init_thread_info();

   init_array(thread_id); 

   thread_num = get_prof_info_thread_id();

   if ((thread_num >= 0) && (thread_num < MAX_THREAD_NUM)) {
      pthread_mutex_lock(&thrdInfo_mutex);

      thrdInfo[thread_num].thrd_id = thread_id;
      thrdInfo[thread_num].thrd_name = thread_name;
      thrdInfo[thread_num].prof_is_active = false;
      thrdInfo[thread_num].is_terminated = 0;

      pthread_mutex_unlock(&thrdInfo_mutex);
   }
}



/****** uti/profiling/set_thread_prof_status_by_id() ******************************
*  NAME
*     set_thread_prof_status_by_id() -- sets the profiling status for the thread
*                                       with the given thread id 
*
*  SYNOPSIS
*     void set_thread_prof_status_by_id(pthread_t thread_id, bool prof_status) 
*
*  FUNCTION
*     set the thread profiling status of the thread with the given id 
*
*  INPUTS
*     pthread_t thread_id
*     bool prof_status 
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: set_thread_prof_status_by_id() is MT safe
*******************************************************************************/
void set_thread_prof_status_by_id(pthread_t thread_id, bool prof_status) {

   int thread_num;

   if (!profiling_enabled) {
      return;
   }
   
   init_thread_info();

   thread_num = get_prof_info_thread_id();

   pthread_mutex_lock(&thrdInfo_mutex);

   if (thrdInfo[thread_num].thrd_id == thread_id) {
      thrdInfo[thread_num].prof_is_active = prof_status;
   }

   pthread_mutex_unlock(&thrdInfo_mutex);
}



/****** uti/profiling/set_thread_prof_status_by_name() ******************************
*  NAME
*     set_thread_prof_status_by_name() -- sets the profiling status for the thread
*                                         with the given thread id and thread name
*
*  SYNOPSIS
*     void set_thread_prof_status_by_name(pthread_t thread_id, const char* thread_name,  bool prof_status) 
*
*  FUNCTION
*     set the thread profiling status of the thread with the given id and name 
*
*  INPUTS
*     pthread_t thread_id
*     const char* thread_name
*     bool prof_status
*
*  RESULTS
*     return 0 - ok
*     return 1 - thread_name = NULL
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: set_thread_prof_status_by_name() is MT safe
*******************************************************************************/
int set_thread_prof_status_by_name(const char* thread_name, bool prof_status) {

   int i;

   if (!profiling_enabled) {
      return 0;
   } else if (thread_name == NULL) {
      return 1;
   }

   init_thread_info();

   pthread_mutex_lock(&thrdInfo_mutex);
   
   for (i = 0; i < MAX_THREAD_NUM; i++) {
      if (thrdInfo[i].thrd_name != NULL) {
         if (strcmp(thrdInfo[i].thrd_name, thread_name) == 0) {
            thrdInfo[i].prof_is_active = prof_status;
         }
      }
   }

   pthread_mutex_unlock(&thrdInfo_mutex);

   return 0;
}


/****** uti/profiling/sge_prof_cleanup() ******************************
*  NAME
*     sge_prof_cleanup() -- frees the profiling array 
*
*  SYNOPSIS
*     void sge_prof_cleanup(void) 
*
*  FUNCTION
*     frees the profiling array
*
*  INPUTS
*     
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: sge_prof_cleanup() is MT safe
*******************************************************************************/

void sge_prof_cleanup(void) {

   if (!profiling_enabled) {
      return;
   }
   
   pthread_mutex_lock(&thrdInfo_mutex);

   pthread_key_delete (thread_id_key);
  
   if (theInfo != NULL) {
      int c, i;

      for (c = 0; c < MAX_THREAD_NUM; c++) {
         for (i = 0; i <= SGE_PROF_ALL; i++) {
            if ( theInfo[c] != NULL) {
               sge_dstring_free(&theInfo[c][i].info_string);
            }
         }
         sge_free(&(theInfo[c]));
      }
      sge_free(&theInfo);
   }
   sge_free(&thrdInfo);
   
   sge_prof_array_initialized = 0;
   
   pthread_mutex_unlock(&thrdInfo_mutex);

}

/****** uti/profiling/thread_prof_active_by_id() ******************************
*  NAME
*     thread_prof_active_by_id() -- returns the status of a thread 
*
*  SYNOPSIS
*     bool thread_prof_active_by_id(void) 
*
*  FUNCTION
*     returns the profiling status of the thread
*
*  INPUTS
*     void
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: thread_prof_active_by_id() is MT safe
*******************************************************************************/
bool thread_prof_active_by_id(void) {

   int thread_num;
   bool ret = false;

   if (profiling_enabled) {
      init_thread_info();

      thread_num = get_prof_info_thread_id();

      if ((thread_num >= 0) && (thread_num < MAX_THREAD_NUM)) {
         pthread_mutex_lock(&thrdInfo_mutex);

         ret = thrdInfo[thread_num].prof_is_active;

         pthread_mutex_unlock(&thrdInfo_mutex);
      }
   }

   return ret;

}


/****** uti/profiling/thread_prof_active_by_name() ******************************
*  NAME
*     thread_prof_active_by_name() -- returns the status of a thread 
*
*  SYNOPSIS
*     bool thread_prof_active_by_name(pthread_t thread_id) 
*
*  FUNCTION
*     returns the profiling status of a thread
*
*  INPUTS
*     pthread_t thread_id
*
*  RESULT
*
*  EXAMPLE
*
*  NOTES
*     MT-NOTE: thread_prof_active_by_name() is MT safe
*******************************************************************************/
bool thread_prof_active_by_name(const char* thread_name) {

   bool ret = false;

   if (profiling_enabled && (thread_name != NULL)) {
      int c;

      init_thread_info();

      pthread_mutex_lock(&thrdInfo_mutex);

      for (c = 0; c < MAX_THREAD_NUM; c++) {
         if (thrdInfo[c].thrd_name != NULL && strstr(thrdInfo[c].thrd_name, thread_name)) {
            ret = thrdInfo[c].prof_is_active;
            break;
         }
      }

      pthread_mutex_unlock(&thrdInfo_mutex);
   }
   
   return ret;
}


static int get_prof_info_thread_id(void) {
   long ret = (long)pthread_getspecific (thread_id_key);

   return ret;
}

/****** uti/profiling/thread_start_stop_profiling() ****************************
*  NAME
*     thread_start_stop_profiling() -- start profiling for thread
*
*  SYNOPSIS
*     void 
*     thread_start_stop_profiling(void) 
*
*  FUNCTION
*     Checks if profiling has been enabled for the current thread.
*     If yes, starts profiling for all levels.
*     If profiling has been disabled for the current thread, profiling
*     is disabled for all levels.
*
*  NOTES
*     MT-NOTE: thread_start_stop_profiling() is MT safe 
*******************************************************************************/
void 
thread_start_stop_profiling(void)
{
   if (!profiling_enabled) {
      return;
   }
   
   if (thread_prof_active_by_id()) {
      prof_start(SGE_PROF_ALL, NULL);
   } else {
      prof_stop(SGE_PROF_ALL, NULL);
   }
}

/****** uti/profiling/thread_output_profiling() ********************************
*  NAME
*     thread_output_profiling() -- output profiling info for thread
*
*  SYNOPSIS
*     void 
*     thread_output_profiling(const char *title, time_t *next_prof_output) 
*
*  FUNCTION
*     Outputs profiling information for the current thread.
*     Information for all active profiling levels is dumped.
*     The first line dumped is a sort of title, that can/should be used
*     to identify the thread.
*
*     The variable next_prof_output will be set by this function.
*     This variable should be initialized to 0 (zero) by the caller before the
*     first call of this function.
*
*  INPUTS
*     const char *title        - title to print as first line
*     time_t *next_prof_output - time of next profiling output
*
*  NOTES
*     MT-NOTE: thread_output_profiling() is MT safe 
*
*  SEE ALSO
*     uti/profiling/prof_output_info()
*******************************************************************************/
void 
thread_output_profiling(const char *title, time_t *next_prof_output)
{
   if (prof_is_active(SGE_PROF_ALL)) {
      time_t now = (time_t)sge_get_gmt();

      if (*next_prof_output == 0) {
         unsigned int seed = (unsigned int)pthread_self();
         *next_prof_output = now + (rand_r(&seed) % 20);
      } else {
         if (now >= *next_prof_output) {
            prof_output_info(SGE_PROF_ALL, false, title);
            *next_prof_output = now + 60;
         }
      }
   }
}
