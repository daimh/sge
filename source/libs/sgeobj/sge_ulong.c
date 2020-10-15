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

/* Fixme:  This doesn't work for a non-gcc compiler (e.g. clang) on a
   glibc system as fabs doesn't get defined with any features as far
   as I can tell.  */
#if !defined __sun__ && !defined __sun /* else complaints about non-C99 compilation */
#define _XOPEN_SOURCE 600       /* for fabs */
#endif
#include <math.h>
#include <float.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_time.h"
#include "uti/sge_string.h"
#include "uti/sge_dstring.h"
#include "uti/sge_parse_num_par.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_ulong.h"
#include "sgeobj/sge_centry.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "basis_types.h"

#define ULONG_LAYER TOP_LAYER

/*
* NOTES
*     MT-NOTE: double_print_infinity_to_dstring() is MT safe
*/
bool double_print_infinity_to_dstring(double value, dstring *string)
{
   bool ret = true;

   DENTER(ULONG_LAYER, "double_print_infinity_to_dstring");
   if (string != NULL) {
      if (value == DBL_MAX) {
         sge_dstring_append(string, "infinity");
      } else {
         ret = false;
      }
   }
   DEXIT;
   return ret;
}

/*
* NOTES
*     MT-NOTE: double_print_time_to_dstring() is MT safe
*/
bool double_print_time_to_dstring(double value, dstring *string) 
{
   bool ret = true;

   DENTER(ULONG_LAYER, "double_print_time_to_dstring");
   if (string != NULL) {
      if (!double_print_infinity_to_dstring(value, string)) {
         const u_long32 minute_in_seconds = 60;
         const u_long32 hour_in_seconds = minute_in_seconds * 60;
         int seconds = value, minutes, hours;

         hours = seconds / hour_in_seconds;
         seconds -= hours * hour_in_seconds;
         minutes = seconds / minute_in_seconds;
         seconds -= minutes * minute_in_seconds;
         sge_dstring_sprintf_append(string, "%2.2d:%2.2d:%2.2d",
                                    hours, minutes, seconds);
      }
   }
   DEXIT;
   return ret; 
}

/*
* NOTES
*     MT-NOTE: double_print_memory_to_dstring() is MT safe
*/
bool double_print_memory_to_dstring(double value, dstring *string)
{
   bool ret = true;

   DENTER(ULONG_LAYER, "double_print_memory_to_dstring");
   if (string != NULL) {
      if (!double_print_infinity_to_dstring(value, string)) {
         const double kilo_byte = 1024;
         const double mega_byte = kilo_byte * 1024;
         const double giga_byte = mega_byte * 1024;
         const double tera_byte = giga_byte * 1024;
         double absolute_value = fabs(value); 
         char unit = '\0';

         if (absolute_value >= tera_byte) {
            value /= tera_byte;
            unit = 'T';
         } else if (absolute_value >= giga_byte) {
            value /= giga_byte;
            unit = 'G';
         } else if (absolute_value >= mega_byte) {
            value /= mega_byte;
            unit = 'M';
         } else if (absolute_value >= kilo_byte) {
            value /= kilo_byte;
            unit = 'K';
         }
         if (unit != '\0') {
            sge_dstring_sprintf_append(string, "%.3f%c", value, unit);
         } else {
            sge_dstring_sprintf_append(string, "%.3f", absolute_value);
         }
      } 
   }
   DEXIT;
   return ret;
}

int reformatDoubleValue(char *result, size_t lresult, const char *format, const char *oldmem)
{
   double dval;
   int ret = 1;

   DENTER(TOP_LAYER, "reformatDoubleValue");

   if (parse_ulong_val(&dval, NULL, TYPE_MEM, oldmem, NULL, 0)) {
      if (dval==DBL_MAX) {
         strcpy(result, "infinity");
      } else {
         int c = '\0';

         if (fabs(dval) >= 1024.0*1024*1024*1024) {
            dval /= 1024.0*1024*1024*1024;
            c = 'T';
         } else if (fabs(dval) >= 1024*1024*1024) {
            dval /= 1024*1024*1024;
            c = 'G';
         } else if (fabs(dval) >= 1024*1024) {
            dval /= 1024*1024;
            c = 'M';
         } else if (fabs(dval) >= 1024) {
            dval /= 1024;
            c = 'K';
         }
         snprintf(result, lresult, format, dval, c);
      }
   }
   else {
      strcpy(result, "?E");
      ret = 0;
   }

   DRETURN(ret);
}

const char *double_print_memory_to_string(double value, dstring *string)
{
   sge_dstring_clear(string);
   double_print_memory_to_dstring(value, string);
   return sge_dstring_append_char(string, 'B');
}

/****** sgeobj/double_print_int_to_dstring() ***********************************
*  NAME
*     double_print_int_to_dstring() -- Print a double into a dstring as an int
*
*  SYNOPSIS
*     lListElem * 
*     double_print_int_to_dstring(double value, dstring *string)
*
*  FUNCTION
*    Print a double into a dstring as an int.
*
*  INPUTS
*     double value      - the value to print
*     dstring *string   - the dstring to receive the value
*
*  RESULT
*     bool - returns false if value is out of range for an int
*
*  NOTES
*     MT-NOTE: double_print_int_to_dstring() is MT safe
*
*******************************************************************************/
bool double_print_int_to_dstring(double value, dstring *string)
{
   bool ret = true;

   DENTER(ULONG_LAYER, "double_print_int_to_dstring");
   
   if (string != NULL) {
      if (!double_print_infinity_to_dstring(value, string)) {
         const double min_as_dbl = INT_MIN;
         const double max_as_dbl = INT_MAX;

         if (value > max_as_dbl || value < min_as_dbl) {
            sge_dstring_append(string, "integer_overflow");
            DEXIT;
            return false;
         }
     
         sge_dstring_sprintf_append(string, "%d", (int)value);
      } 
   }
   DEXIT;
   return ret;
}

/****** sgeobj/double_print_to_dstring() ***********************************
*  NAME
*     double_print_to_dstring() -- Print a double into a dstring
*
*  SYNOPSIS
*     lListElem * 
*     double_print_to_dstring(double value, dstring *string)
*
*  FUNCTION
*    Print a double into a dstring.
*
*  INPUTS
*     double value      - the value to print
*     dstring *string   - the dstring to receive the value
*
*  RESULT
*     bool - returns false if something goes wrong
*
*  NOTES
*     MT-NOTE: double_print_to_dstring() is MT safe
*
*******************************************************************************/
bool double_print_to_dstring(double value, dstring *string)
{
   bool ret = true;

   DENTER(ULONG_LAYER, "double_print_to_dstring");
   if (string != NULL) {
      if (!double_print_infinity_to_dstring(value, string)) {
         sge_dstring_sprintf_append(string, "%f", value);
      } 
   }
   DEXIT;
   return ret;
}

/****** sge_ulong/ulong_parse_date_time_from_string() **************************
*  NAME
*     ulong_parse_date_time_from_string() -- Parse string into date/time ulong
*
*  SYNOPSIS
*     bool ulong_parse_date_time_from_string(u_long32 *this_ulong, lList 
*     **answer_list, const char *string) 
*
*  FUNCTION
*     ??? 
*
*  INPUTS
*     u_long32 *this_ulong - ??? 
*     lList **answer_list  - ??? 
*     const char *string   - ??? 
*
*  RESULT
*     bool - 
*
*  NOTES
*     MT-NOTE: ulong_parse_date_time_from_string() is MT safe 
*******************************************************************************/
bool 
ulong_parse_date_time_from_string(u_long32 *this_ulong, 
                                  lList **answer_list, const char *string) 
{
   int i;
   int year_fieldlen=2;
   const char *seconds;
   const char *non_seconds;
   stringT tmp_str;
   stringT inp_date_str;

   time_t gmt_secs;
   struct tm res;
   struct tm *tmp_timeptr,timeptr;
   struct saved_vars_s *context = NULL;

   DENTER(TOP_LAYER, "ulong_parse_date_time_from_string");

   memset(tmp_str, 0, sizeof(tmp_str));

   if (!string || string[0] == '\0') {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_NODATE));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;   
      DEXIT;
      return false;
   }

   if (strlen(string) > sizeof(stringT)) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_STARTTIMETOOLONG));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   strcpy(inp_date_str, string); /* RATS: ignore */
   non_seconds=sge_strtok_r(inp_date_str, ".", &context);
   seconds=sge_strtok_r(NULL, ".", &context);

   if (seconds) {
      i=strlen(seconds);
   } else {
      i = 0;
   }

   if ((i != 0) && (i != 2)) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDSECONDS));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   i=strlen(non_seconds);

   if ((i != 8) && (i != 10) && (i != 12)) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDHOURMIN));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   memset((char *)&timeptr, 0, sizeof(timeptr));

   if (i==12) {
      year_fieldlen=4;
   }

   if (i>=10) {
      memset(tmp_str, 0, sizeof(tmp_str));
      memcpy(tmp_str, non_seconds, year_fieldlen);
      timeptr.tm_year=atoi(tmp_str);
      if (i==12) {
         timeptr.tm_year -= 1900;
      }
      else {
         /* the date is before 1970, thus we assume, that
            20XX is ment. This works only till 2069, but
            that should be sufficent for now */
         if (timeptr.tm_year < 70)
            timeptr.tm_year += 100;
      }
      non_seconds+=year_fieldlen;
   } else {
      gmt_secs=(time_t)sge_get_gmt();
      tmp_timeptr=localtime_r(&gmt_secs, &res);
      timeptr.tm_year=tmp_timeptr->tm_year;
   }

   memset(tmp_str, 0, sizeof(tmp_str));
   memcpy(tmp_str, non_seconds, 2);
   timeptr.tm_mon=atoi(tmp_str)-1;/* 00==Jan, we don't like that do we */
   if ((timeptr.tm_mon>11)||(timeptr.tm_mon<0)) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDMONTH));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   non_seconds+=2;

   memset(tmp_str, 0, sizeof(tmp_str));
   memcpy(tmp_str, non_seconds, 2);
   timeptr.tm_mday=atoi(tmp_str);
   non_seconds+=2;

   /* yea, we should do it by mon ths */
   if ((timeptr.tm_mday > 31) || (timeptr.tm_mday < 1)) {
      /* actually mktime() should frigging do it */
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDDAY));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   memset(tmp_str, 0, sizeof(tmp_str));
   memcpy(tmp_str, non_seconds, 2);
   timeptr.tm_hour=atoi(tmp_str);
   non_seconds+=2;

   if ((timeptr.tm_hour > 23) || (timeptr.tm_hour < 0)) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDHOUR));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   memset(tmp_str, 0, sizeof(tmp_str));
   memcpy(tmp_str, non_seconds, 2);
   timeptr.tm_min=atoi(tmp_str);

   if ((timeptr.tm_min > 59)||(timeptr.tm_min < 0)) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDMINUTE));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   if (seconds) {
      timeptr.tm_sec=atoi(seconds);
   }
   if ((timeptr.tm_sec>59)||(timeptr.tm_mday<0)) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_INVALIDSECOND));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   /*
   ** for daylight saving corrections
   */
   timeptr.tm_isdst = -1;

   gmt_secs=mktime(&timeptr);

   DPRINTF(("mktime returned: %ld\n",gmt_secs));

   if (gmt_secs < 0) {
      sge_free_saved_vars(context);
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_PARSE_NODATEFROMINPUT));
      if (answer_list) {
         answer_list_add(answer_list, SGE_EVENT, 
                         STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      } else {
         fprintf(stderr,"\n%s\n", SGE_EVENT);
      }
      *this_ulong = -1;
      DEXIT;
      return false;
   }

   sge_free_saved_vars(context);

   *this_ulong = gmt_secs;
   DEXIT;
   return true;
}

bool
ulong_parse_centry_type_from_string(u_long32 *this_ulong,
                                    lList **answer_list, const char *string)
{
   bool ret = true;
   int i;
   DENTER(TOP_LAYER, "ulong_parse_centry_type_from_string");

   *this_ulong = 0;
   for (i = TYPE_FIRST; i <= TYPE_CE_LAST; i++) {
      if (!strcasecmp(string, map_type2str(i))) {
         *this_ulong = i;
         break;
      }
   }
   if (*this_ulong == 0) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_INVALID_CENTRY_TYPE_S, string);
      ret = false;
   }
   DEXIT;
   return ret;
}

bool
ulong_parse_centry_relop_from_string(u_long32 *this_ulong,
                                     lList **answer_list, const char *string)
{
   bool ret = true;
   int i;
   DENTER(TOP_LAYER, "ulong_parse_centry_relop_from_string");

   *this_ulong = 0;
   for (i = CMPLXEQ_OP; i <= CMPLXNE_OP; i++) {
      if (!strcasecmp(string, map_op2str(i))) {
         *this_ulong = i;
         break;
      }
   }
   if (*this_ulong == 0) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_INVALID_CENTRY_RELOP_S, string);
      ret = false;
   }
   DEXIT;
   return ret;
}

bool
ulong_parse_from_string(u_long32 *this_ulong,
                        lList **answer_list, const char *string) 
{
   bool ret = true;
      
   DENTER(TOP_LAYER, "ulong_parse_from_string");
   if (this_ulong != NULL && string != NULL) {
      if (!parse_ulong_val(NULL, this_ulong, TYPE_INT, string, NULL, 0)) {
         answer_list_add(answer_list, MSG_PARSE_INVALID_ID_MUSTBEUINT,
                         STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR);
         ret = false;
      }
   }
   DRETURN(ret);
}


bool
ulong_list_parse_from_string(lList **this_list, lList **answer_list,
                             const char *string, const char *delimitor)
{
   bool ret = true;
                                
   DENTER(TOP_LAYER, "ulong_list_parse_from_string");
   if (this_list != NULL && string != NULL && delimitor != NULL) {
      struct saved_vars_s *context = NULL;
      const char *token;
            
      token = sge_strtok_r(string, delimitor, &context);
      while (token != NULL) {   
         u_long32 value;

         ret = ulong_parse_from_string(&value, answer_list, token);
         if (ret) {
            lAddElemUlong(this_list, ULNG_value, value, ULNG_Type);
         } else {
            break;
         }
         token = sge_strtok_r(NULL, delimitor, &context);
      }
      sge_free_saved_vars(context);
   }        
   DRETURN(ret);
}

/* EB: TODO: JSV: add ADOC */
bool
ulong_parse_priority(lList **answer_list, int *valp, const char *priority_str) 
{
   bool ret = true;
   char *s;

   DENTER(TOP_LAYER, "ulong_parse_priority");
   errno = 0;
   *valp = strtol(priority_str, &s, 10);
   if (priority_str == s || *s != '\0' || *valp > 1024 || *valp < -1023
       || errno != 0) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ULNG_INVALIDPRIO_S, priority_str));
      answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DRETURN(ret);
}

#if unused
/* DG: TODO: add ADOC */
bool
ulong_parse_value_from_string(u_long32 *this_ulong, 
                           lList **answer_list, const char *string)
{
   bool ret = true;
   char *s;
   
   DENTER(TOP_LAYER, "ulong_parse_value_from_string");
   
   *this_ulong = strtol(string, &s, 10);
   if (string == s) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, SFNMAX, MSG_ULNG_INVALID_VALUE));
      answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      ret = false;
   }

   DRETURN(ret);
}
#endif

bool
ulong_parse_task_concurrency(lList **answer_list, int *valp, const char *task_concurrency_str)
{
   bool ret = true;
   char *s;

   DENTER(TOP_LAYER, "ulong_parse_task_concurrency");
   errno = 0;
   *valp = strtol(task_concurrency_str, &s, 10);
   if (task_concurrency_str == s || *s != '\0' || errno != 0 || *valp < 0) {
      SGE_ADD_MSG_ID(sprintf(SGE_EVENT, MSG_ULNG_INVALID_TASK_CONCURRENCY_S,
                             task_concurrency_str));
      answer_list_add(answer_list, SGE_EVENT, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR);
      ret = false;
   }
   DRETURN(ret);
}

