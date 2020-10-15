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

#define _POSIX_C_SOURCE 200112L /* try to avoid old nanosleep returning void */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/times.h> 

#ifndef WIN32NATIVE
#	include <sys/time.h>
#else 
#	include <winsock2.h>
#endif 

#include "uti/sge_dstring.h"
#include "uti/sge_time.h"
#include "uti/sge_unistd.h"
#include "uti/sge_log.h"

#ifdef WIN32
int gettimeofday(struct timeval *tz, struct timezone *tzp);
#endif

#define NESTLEVEL 5

/* MT-NOTE: stopwatch profiling used in qmaster, qstat and qmon only */
/* MT-NOTE: stopwatch is phased out and will be replaced by libs/uti/sge_profiling */
static struct tms begin[NESTLEVEL];
static struct tms end[NESTLEVEL];
 
static time_t wtot[NESTLEVEL];
static time_t wbegin[NESTLEVEL];
static time_t wprev[NESTLEVEL];
static time_t wdiff[NESTLEVEL];
 
static int clock_tick;
static int time_log_interval[NESTLEVEL] = { -1, -1, -1, -1, -1 };
 
#ifdef TIMES_RETURNS_ZERO
static time_t inittime;
#endif
 
static void sge_stopwatch_stop(int i);  

/* MT-NOTE: sge_stopwatch_stop() is not MT safe due to access to global variables */
static void sge_stopwatch_stop(int i)
{
   time_t wend;
 
   if (i < 0 || i >= NESTLEVEL) {
      return;
   }
   if (time_log_interval[i] == -1) {
      return;
   }
   wend = times(&end[i]);
 
#ifdef TIMES_RETURNS_ZERO
   /* times() returns 0 on these machines */
   wend = (sge_get_gmt() - inittime) * clock_tick;
#endif
 
   end[i].tms_utime =  end[i].tms_utime -  begin[i].tms_utime;
   end[i].tms_stime =  end[i].tms_stime -  begin[i].tms_stime;
   end[i].tms_cutime = end[i].tms_cutime - begin[i].tms_cutime;
   end[i].tms_cstime = end[i].tms_cstime - begin[i].tms_cstime;
 
   wtot[i]  = wend - wbegin[i];
   wdiff[i] = wend - wprev[i];
   wprev[i] = wend;
}

/****** uti/time/sge_get_gmt() ************************************************
*  NAME
*     sge_get_gmt() -- Return current time 
*
*  SYNOPSIS
*     u_long32 sge_get_gmt() 
*
*  FUNCTION
*     Return current time 
*
*  NOTES
*     MT-NOTE: sge_get_gmt() is MT safe (except for WIN32NATIVE)
*
*  RESULT
*     u_long32 - 32 bit time value
******************************************************************************/
u_long32 sge_get_gmt()
{
#ifndef WIN32NATIVE

   struct timeval now;

   gettimeofday(&now, NULL);

   return (u_long32)now.tv_sec;
#else
   time_t long_time;
   struct tm *gmtimeval;

	time(&long_time);                  /* Get time as long integer. */

   /* MT-NOTE: gmtime() is not MT safe (WIN32NATIVE) */
	gmtimeval = gmtime(&long_time);    /* Convert to local time. */
	long_time = mktime(gmtimeval);
	return long_time;
#endif
}

/****** uti/time/append_time() **************************************************
*  NAME
*     append_time() -- Convert time value into string 
*
*  SYNOPSIS
*     const char* append_time(time_t i, dstring *buffer) 
*
*  FUNCTION
*     Convert time value into string 
*
*  INPUTS
*     time_t i - time value 
*     dstring *buffer - dstring
*     bool is_xml - write in XML dateTime format?
*
*  RESULT
*     const char* - time string (current time if 'i' was 0) 
*     dstring *buffer - buffer provided by caller
*
*  NOTES
*     MT-NOTE: append_time() is MT safe if localtime_r() can be used
*
*     SHOULD BE REPLACED BY: sge_dstring_append_time()
*
******************************************************************************/
void append_time(time_t i, dstring *buffer, bool is_xml) 
{
   struct tm *tm;

   struct tm tm_buffer;
   
   tm = localtime_r(&i, &tm_buffer);

   if (is_xml) {
      sge_dstring_sprintf_append(buffer, "%04d-%02d-%02dT%02d:%02d:%02d", 
              1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, 
              tm->tm_hour, tm->tm_min, tm->tm_sec);
   } else {
      sge_dstring_sprintf_append(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
              tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year,
              tm->tm_hour, tm->tm_min, tm->tm_sec);
   }
}

/****** uti/time/sge_ctime() **************************************************
*  NAME
*     sge_ctime() -- Convert time value into string 
*
*  SYNOPSIS
*     const char* sge_ctime(time_t i, dstring *buffer) 
*
*  FUNCTION
*     Convert time value into string 
*
*  INPUTS
*     time_t i - 0 or time value 
*
*  RESULT
*     const char* - time string (current time if 'i' was 0) 
*     dstring *buffer - buffer provided by caller
*
*  NOTES
*     MT-NOTE: sge_at_time() is MT safe if localtime_r() can be used
*
*  SEE ALSO
*     uti/time/sge_ctime32()
******************************************************************************/
const char *sge_ctime(time_t i, dstring *buffer) 
{
   struct tm tm_buffer;
   struct tm *tm;

   if (!i)
      i = (time_t)sge_get_gmt();
   tm = localtime_r(&i, &tm_buffer);
   sge_dstring_sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
           tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year,
           tm->tm_hour, tm->tm_min, tm->tm_sec);

   return sge_dstring_get_string(buffer);
}

/* TODO: should be replaced by sge_dstring_append_time() */
const char *sge_ctimeXML(time_t i, dstring *buffer) 
{
   struct tm tm_buffer;
   struct tm *tm;

   if (!i)
      i = (time_t)sge_get_gmt();
   tm = localtime_r(&i, &tm_buffer);
   sge_dstring_sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d",
           1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec);

   return sge_dstring_get_string(buffer);
}


/****** uti/time/sge_ctime32() ************************************************
*  NAME
*     sge_ctime32() -- Convert time value into string (64 bit time_t) 
*
*  SYNOPSIS
*     const char* sge_ctime32(u_long32 *i, dstring *buffer) 
*
*  FUNCTION
*     Convert time value into string. This function is needed for 
*     systems with a 64 bit time_t because the ctime function would
*     otherwise try to interpret the u_long32 value as 
*     a 64 bit value => $&&%$?$%!
*
*  INPUTS
*     u_long32 *i - 0 or time value 
*     dstring *buffer - buffer provided by caller
*
*  RESULT
*     const char* - time string (current time if 'i' was 0)
* 
*  NOTE
*     MT-NOTE: if ctime_r() is not available sge_ctime32() is not MT safe
*
*  SEE ALSO
*     uti/time/sge_ctime()
******************************************************************************/
const char *sge_ctime32(u_long32 *i, dstring *buffer) 
{
   const char *s;
   char str[128]; 
#if SOLARIS64
   volatile
#endif
   time_t temp = (time_t)*i;

   s = ctime_r((time_t *)&temp, str);
   if (!s)
      return NULL;
   return sge_dstring_copy_string(buffer, s);
}

/****** uti/time/sge_at_time() ************************************************
*  NAME
*     sge_at_time() -- ??? 
*
*  SYNOPSIS
*     const char* sge_at_time(time_t i, dstring *buffer) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     time_t i - 0 or time value 
*     dstring *buffer - buffer provided by caller
*
*  RESULT
*     const char* - time string (current time if 'i' was 0) 
*
*  NOTES
*     MT-NOTE: sge_at_time() is MT safe if localtime_r() can be used
*
*  SEE ALSO
*     uti/time/sge_ctime() 
******************************************************************************/
const char *sge_at_time(time_t i, dstring *buffer) 
{
   struct tm tm_buffer;
   struct tm *tm;

   if (!i)
      i = (time_t)sge_get_gmt();
   tm = localtime_r(&i, &tm_buffer);
   return sge_dstring_sprintf(buffer, "%04d%02d%02d%02d%02d.%02d",
           tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/****** uti/time/sge_stopwatch_log() ******************************************
*  NAME
*     sge_stopwatch_log() -- ??? 
*
*  SYNOPSIS
*     void sge_stopwatch_log(int i, const char *str) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     int i           - ??? 
*     const char *str - ??? 
*
*  NOTES
*     MT-NOTE: sge_stopwatch_log() is not MT safe due to access to global variables
*
*  SEE ALSO
*     uti/time/sge_stopwatch_start() 
******************************************************************************/
void sge_stopwatch_log(int i, const char *str)
{
   if (i < 0 || i >= NESTLEVEL) {
      return;
   }
   if (time_log_interval[i] == -1) {
      return;
   }
 
   sge_stopwatch_stop(i);
 
   if ((wdiff[i] * 1000) / clock_tick >= time_log_interval[i]) {
      static char SGE_FUNC[] = "";
 
      WARNING((SGE_EVENT, "%-30s: %d/%d/%d", str,
               (int) ((wtot[i] * 1000) / clock_tick),
               (int) ((end[i].tms_utime * 1000)  / clock_tick),
               (int) ((end[i].tms_stime  * 1000) / clock_tick)));
   }
}          

/****** uti/time/sge_stopwatch_start() ****************************************
*  NAME
*     sge_stopwatch_start() -- ??? 
*
*  SYNOPSIS
*     void sge_stopwatch_start(int i) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     int i - ??? 
*
*  NOTES
*     MT-NOTE: sge_stopwatch_start() is not MT safe due to access to global variables
*
*  SEE ALSO
*     uti/time/sge_stopwatch_log() 
******************************************************************************/
void sge_stopwatch_start(int i)
{
   static int first = 1;
   int j;
   char *cp;
 
   if (first) {
      char buf[24];
 
      clock_tick = sysconf(_SC_CLK_TCK);
      for (j = 0; j < NESTLEVEL; j++) {
         wtot[j] = wbegin[j] = wprev[j] = wdiff[j] = 0;
         sprintf(buf, "SGE_TIMELOG%d", j);
         if ((cp = getenv(buf)) && (atoi(cp) >= 0)) {
            time_log_interval[j] = atoi(cp);
         } else {
            time_log_interval[j] = -1;
         }
      }
      first = 0;
   }
 
   if (i < 0 || i >= NESTLEVEL) {
      return;
   }
   if (time_log_interval[i] == -1) {
      return;
   }
   wbegin[i] = times(&begin[i]);
 
#ifdef TIMES_RETURNS_ZERO
   /* times() return 0 on these machines */
   wbegin[i] = (sge_get_gmt() - inittime) * clock_tick;
#endif
 
   wprev[i]  = wbegin[i];
}                                                                              

/****** uti/time/duration_add_offset() ****************************************
*  NAME
*     duration_add_offset() -- add function for time add
*
*  SYNOPSIS
*     u_long32 duration_add_offset(u_long32 duration, u_long32 offset) 
*
*  FUNCTION
*     add function to catch ulong overflow. Returns max ulong value if necessary
*
*  INPUTS
*     u_long32 duration - duration in seconds
*     u_long32 offset   - offset in seconds
*
*  RESULT
*     u_long32 - value < U_LONG32_MAX
*
*  NOTES
*     MT-NOTE: duration_add_offset() is not MT safe 
*******************************************************************************/
u_long32 duration_add_offset(u_long32 duration, u_long32 offset)
{
   if (duration == U_LONG32_MAX || offset == U_LONG32_MAX) {
      return U_LONG32_MAX;
   }

   if ((U_LONG32_MAX-offset) < duration) {
      duration = U_LONG32_MAX;
   } else {
      duration += offset;
   }

   return duration;
}

/****** uti/time/sge_usleep() ****************************************
*  NAME
*     sge_usleep() -- Mimics a non-iterruptable usleep() 
*
*  SYNOPSIS
*     void sge_usleep(int sleep_time) 
*
*  FUNCTION
*     Mimics a non-iterruptable usleep() to the caller.
*
*  INPUTS
*     int sleep_time - requested sleep time in microseconds
*
*  RESULT
*     n/a
*
*  NOTES
*     None.
*******************************************************************************/
void sge_usleep(int sleep_time)
{
   struct timespec req = {sleep_time / 1000000, 1000 * (sleep_time % 1000000)},
      rem;

   while (nanosleep(&req, &rem)) {
      req.tv_sec = rem.tv_sec;
      req.tv_nsec = rem.tv_nsec;
   }
}
