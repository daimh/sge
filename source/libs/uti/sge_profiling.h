#ifndef __SGE_PROFILING_H
#define __SGE_PROFILING_H
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

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#if defined(FREEBSD) || defined(__OpenBSD__)
#include <pthread.h>
#endif

#include "basis_types.h"

#include "sge_dstring.h"

typedef enum {
   SGE_PROF_NONE = -1,
   SGE_PROF_OTHER = 0,
   SGE_PROF_COMMUNICATION,
   SGE_PROF_PACKING,
   SGE_PROF_EVENTCLIENT,
   SGE_PROF_EVENTMASTER,
   SGE_PROF_MIRROR,
   SGE_PROF_SPOOLING,
   SGE_PROF_SPOOLINGIO,
   SGE_PROF_JOBSCRIPT,
   SGE_PROF_GDI,
   SGE_PROF_GDI_REQUEST,
   SGE_PROF_HT_RESIZE,
   SGE_PROF_SCHEDULER,
   SGE_PROF_CUSTOM0,
   SGE_PROF_CUSTOM1,
   SGE_PROF_CUSTOM2,
   SGE_PROF_CUSTOM3,
   SGE_PROF_CUSTOM4,
   SGE_PROF_CUSTOM5,
   SGE_PROF_CUSTOM6,
   SGE_PROF_CUSTOM7,
   SGE_PROF_CUSTOM8,
   SGE_PROF_CUSTOM9,
   SGE_PROF_SCHEDLIB0,
   SGE_PROF_SCHEDLIB1,
   SGE_PROF_SCHEDLIB2,
   SGE_PROF_SCHEDLIB3,
   SGE_PROF_SCHEDLIB4, /* used in libs/schedd/sgeee.c; it is not enabled for the overall overview in the scheduler */
   SGE_PROF_ALL
} prof_level;

typedef struct {
   const char *name;
   int nested_calls;          /* number of nested calls within same level  */
   clock_t start;             /* start time of actual measurement          */
   clock_t end;               /* end time of last measurement              */
   struct tms tms_start;      /* time struct of measurement start          */
   struct tms tms_end;        /* time struct of measurement end            */
   clock_t total;             /* summed up clock ticks of all measurements */
   clock_t total_utime;       /* total user time                           */
   clock_t total_stime;       /* total system time                         */

   prof_level pre;            /* predecessor of this level - ALL = no      */
   clock_t sub;               /* time spent in sub measurements            */
   clock_t sub_utime;         /* utime spent in sub measurements           */
   clock_t sub_stime;         /* stime spent in sub measurements           */
   clock_t sub_total;         /* total sub time                            */
   clock_t sub_total_utime;   /* total sub utime                           */
   clock_t sub_total_stime;   /* total sub stime                           */

   bool prof_is_started;
   clock_t start_clock;
   prof_level akt_level;
   bool ever_started;
   pthread_t thread_id;
   dstring info_string;
} sge_prof_info_t;

void sge_prof_set_enabled(bool enabled);
void prof_mt_init(void);
void sge_prof_cleanup(void);
bool thread_prof_active_by_id(void);
bool thread_prof_active_by_name(const char* thread_name);
void set_thread_name(pthread_t thread_id, const char* thread_name);
void set_thread_prof_status_by_id(pthread_t thread_id, bool prof_status);
int  set_thread_prof_status_by_name(const char* thread_name, bool prof_status);
bool prof_set_level_name(prof_level level, const char *name, dstring *error);

bool prof_is_active(prof_level level);

bool prof_start(prof_level level, dstring *error);
bool prof_stop(prof_level level, dstring *error);

bool prof_start_measurement(prof_level level, dstring *error);
bool prof_stop_measurement(prof_level level, dstring *error);

/****** uti/profiling/PROF_START_MEASUREMENT() ******************************
*  NAME
*     PROF_START_MEASUREMENT() -- starts the measurement for the specified level 
*
*  SYNOPSIS
*      PROF_START_MEASUREMENT(prof_level level) 
*
*  FUNCTION
*     starts the measurement for the specified level
*     to use profiling the sge_prof_setup() function
*     must be called in the main program first.
*     At the end of the main program, sge_prof_cleanup()
*     should be called to make sure, that all that
*     stuff becomes clean.
*
*  INPUTS
*     prof_level level 
*
*  RESULT
*
*  EXAMPLE
*     PROF_START_MEASUREMENT(SGE_PROF_GDI)
*
*  NOTES
*     MT-NOTE: PROF_START_MEASUREMENT() is MT safe
*******************************************************************************/
#define PROF_START_MEASUREMENT(level) \
   if(prof_is_active(level)) {\
      prof_start_measurement(level,NULL);\
   }   

/****** uti/profiling/PROF_STOP_MEASUREMENT() ******************************
*  NAME
*     PROF_STOP_MEASUREMENT() -- stops the measurement for the specified level 
*
*  SYNOPSIS
*      PROF_STOP_MEASUREMENT(prof_level level) 
*
*  FUNCTION
*     stops the measurement for the specified level
*     to use profiling the sge_prof_setup() function
*     must be called in the main program first.
*     At the end of the main program, sge_prof_cleanup()
*     should be called to make sure, that all that
*     stuff becomes clean.
*
*  INPUTS
*     prof_level level 
*
*  RESULT
*
*  EXAMPLE
*     PROF_STOP_MEASUREMENT(SGE_PROF_GDI)
*
*  NOTES
*     MT-NOTE: PROF_STOP_MEASUREMENT() is MT safe
*******************************************************************************/
#define PROF_STOP_MEASUREMENT(level) \
   if(prof_is_active(level)) {\
      prof_stop_measurement(level, NULL);\
   }   

bool prof_reset(prof_level level, dstring *error);

double prof_get_measurement_wallclock(prof_level level, bool with_sub, dstring *error);
double prof_get_measurement_utime(prof_level level, bool with_sub, dstring *error);
double prof_get_measurement_stime(prof_level level, bool with_sub, dstring *error);

double prof_get_total_wallclock(prof_level level, dstring *error);
double prof_get_total_busy(prof_level level, bool with_sub, dstring *error);
double prof_get_total_utime(prof_level level, bool with_sub, dstring *error);
double prof_get_total_stime(prof_level level, bool with_sub, dstring *error);

const char *prof_get_info_string(prof_level level, bool with_sub, dstring *error);

bool prof_output_info(prof_level level, bool with_sub, const char *info);

void thread_start_stop_profiling(void);
void thread_output_profiling(const char *title, time_t *next_prof_output);


#endif /* __SGE_PROFILING_H */
