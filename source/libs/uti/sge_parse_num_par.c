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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_parse_num_par.h"
#include "uti/msg_utilib.h"

#include "sgeobj/sge_answer.h"

#include "symbols.h"


#if defined(IRIX)
   /* to be independent from irix' compiler options */
#  undef RLIM_INFINITY
#  define  RLIM_INFINITY  0x7fffffffffffffffLL
#elif defined(WIN32NATIVE)
#	define RLIM_INFINITY 0
#endif

#if !defined(SOLARIS64) && !defined(SOLARISAMD64)
#  define RLIM_MAX RLIM_INFINITY
#else
#  define RLIM_MAX 0x7fffffffffffffff
#endif

#ifdef WIN32NATIVE
#	define strcasecmp( a, b) stricmp( a, b)
#	define strncasecmp( a, b, n) strnicmp( a, b, n)
#endif

u_long32 sge_parse_num_val(sge_rlim_t *rlimp, double *dvalp, 
                           const char *str, const char *where, 
                           char *err_str, int err_len);

static double get_multiplier(sge_rlim_t *rlimp, char **dptr,
                             const char *where, char *err_str, int err_len);

static sge_rlim_t add_infinity(sge_rlim_t rlim, sge_rlim_t offset);


/* -----------------------------------------

NAME
   parse_ulong_val()

DESCR
   is a wrapper around sge_parse_num_val()
   for migration to code that returns an
   error and does not exit() 

   parse_ulong_val() should be used instead 
   of sge_parse_num_val()

PARAM
   uvalp - where to write the parsed value
   type  - one of :
            TYPE_INT   any number
            TYPE_TIM   h:m:s
            TYPE_MEM   [mMkK] postfix
            TYPE_BOO   true or false
            TYPE_ACC   user access to queue
           if type == 0 all above types will get accepted

   s     - string to parse

RETURN
      1 - ok, value in *uvalp is valid
      0 - parsing error

NOTES
   MT-NOTE: parse_ulong_val() is MT safe
*/
int parse_ulong_val(double *dvalp, u_long32 *uvalp, u_long32 type, 
                    const char *s, char *error_str, int error_len) 
{
   return extended_parse_ulong_val(dvalp, uvalp, type, s, error_str, error_len, 1, false);
}

/* enable_infinity enhancement: if 0 no infinity value is allowed */
/*    MT-NOTE: extended_parse_ulong_val() is MT safe */
int extended_parse_ulong_val(double *dvalp, u_long32 *uvalp, u_long32 type,
                             const char *s, char *error_str, int error_len,
                             int enable_infinity, bool only_positive) 
{
   int retval = 0; /* error */
   char dummy[10];
   u_long32 dummy_uval;

   if (s == NULL) {
      return 0;
   }

   if (only_positive && (strchr(s, '-') != NULL)) {
      if (error_str) {
         sge_strlcpy(error_str, MSG_GDI_NUMERICALVALUENOTPOSITIVE, error_len); 
      } 
      return 0;
   }   

   if ((enable_infinity == 0) && (strcasecmp(s,"infinity") == 0)) {
      if (error_str) {
         sge_strlcpy(error_str, MSG_GDI_VALUETHATCANBESETTOINF, error_len); 
      } 
      return 0;
   }

   if (uvalp == NULL) {
      uvalp = &dummy_uval;
   }   

   /*
      here we have to convert from string
      representation of the request value
      into an ulong value
   */
   switch (type) {
   case TYPE_LOG:
      retval = sge_parse_loglevel_val(uvalp, s);
      if (retval != 1) {
         if (error_str != NULL) {
            sge_strlcpy(error_str, "loglevel value", error_len); 
         }
      } 
      break;

   case TYPE_INT:
   case TYPE_TIM:
   case TYPE_MEM:
   case TYPE_BOO:
   case TYPE_DOUBLE:
      /* dirty but isolated .. */
      if (error_str != NULL) {
         *uvalp = sge_parse_num_val(NULL, dvalp, s, s, error_str, error_len);
         if (!error_str[0]) /* err msg written ? */
            retval = 1; /* no error */
         else {
            if (type==TYPE_INT)
               sge_strlcpy(error_str, "integer value", error_len); 
            else if (type==TYPE_TIM)
               sge_strlcpy(error_str, "time value", error_len); 
            else if (type==TYPE_BOO)
               sge_strlcpy(error_str, "boolean value", error_len); 
            else if (type==TYPE_DOUBLE)
               sge_strlcpy(error_str, "double value", error_len); 
            else
               sge_strlcpy(error_str, "memory value", error_len); 
         }
      } else {
         dummy[0] = '\0';
         *uvalp = sge_parse_num_val(NULL, dvalp, s, s, dummy, sizeof(dummy));
         if (!dummy[0]) { /* err msg written ? */
            retval = 1; /* no error */
         }        
      }
      break;

   default:
      break;
   }
   return retval;
}

/*----------------------------------------------------------------------*/
/*    MT-NOTE: sge_parse_loglevel_val() is MT safe */
bool sge_parse_loglevel_val(u_long32 *uval, const char *s) 
{
   bool ret = true;

   if (s == NULL) {
      ret = false;
   } else {
      if (!strcasecmp("log_crit", s)) {
         *uval = LOG_CRIT;
      } else if (!strcasecmp("log_err", s)) {
         *uval = LOG_ERR;
      } else if (!strcasecmp("log_warning", s)) {
         *uval = LOG_WARNING;
      } else if (!strcasecmp("log_notice", s)) {
         *uval = LOG_NOTICE;
      } else if (!strcasecmp("log_info", s)) {
         *uval = LOG_INFO;
      } else if (!strcasecmp("log_debug", s)) {
         *uval = LOG_DEBUG;
      } else {
         ret = false;
      }
   }
   return ret;
}

/* 
 * return rlim multiplied with muli 
 * 
 * if the result would exceed sge_rlim_t
 * the result is set to RLIM_INFINITY
 *
 * NOTES
 *     MT-NOTE: mul_infinity() is MT safe
 */
sge_rlim_t mul_infinity(sge_rlim_t rlim, sge_rlim_t muli) 
{
   if (rlim == RLIM_INFINITY ||
       muli == RLIM_INFINITY )
      return RLIM_INFINITY;

   if ((sge_rlim_t)(RLIM_MAX/muli)<rlim)
      rlim = RLIM_INFINITY;
   else
      rlim *= muli;
   return rlim;
}

/* 
 * return rlim added to offset 
 * 
 * if the result would exceed sge_rlim_t
 * the result is set to RLIM_INFINITY
 *
 * NOTES
 *     MT-NOTE: add_infinity() is MT safe
 */
static sge_rlim_t add_infinity(sge_rlim_t rlim, sge_rlim_t offset) 
{
   if (rlim == RLIM_INFINITY || offset == RLIM_INFINITY) {
      return RLIM_INFINITY;
   }

   if ((sge_rlim_t)(RLIM_MAX-offset) < rlim) {
      rlim = RLIM_INFINITY;
   } else {
      rlim += offset;
   }

   return rlim;
}

/***********************************************************************
 * get_multiplier -
 *       get a multiplier for number value attribute requests
 *
 *       Attribute requests may look like -l sc=123x,...
 *       with x being one of
 *
 *                k : Multiplier = 1000
 *                K : Multiplier = 1024
 *                m : Multiplier = 1000*1000
 *                M : Multiplier = 1024*1024
 *                g : Multiplier = 1000*1000*1000
 *                G : Multiplier = 1024*1024*1024
 *                t : Multiplier = 1000*1000*1000*1000
 *                T : Multiplier = 1024*1024*1024*1024
 *
 * NOTES
 *     MT-NOTE: get_multiplier() is MT safe
 **********************************************************************/

static double get_multiplier(sge_rlim_t *rlimp, char **dptr, 
                             const char *where, char *err_str, int err_len)
{
   double mul = 1;
   *rlimp = 1;

   /* parse for k,K,m,M multipliers at the end of the value
    * string; strtod returns dptr to point to this location
    */
   switch (**dptr) {
   case 'k':
      mul = 1000;
      *rlimp = 1000;
      (*dptr)++;
      break;
   case 'K':
      mul = 1024;
      *rlimp = 1024;
      (*dptr)++;
      break;
   case 'm':
      mul = 1000 * 1000;
      *rlimp = 1000 * 1000;
      (*dptr)++;
      break;
   case 'M':
      mul = 1024 * 1024;
      *rlimp = 1024 * 1024;
      (*dptr)++;
      break;
   case 'g':
      mul = 1000 * 1000 * 1000;
      *rlimp = mul_infinity(mul_infinity(1000, 1000), 1000);
      (*dptr)++;
      break;
   case 'G':
      mul = 1024 * 1024 * 1024;
      *rlimp = mul_infinity(mul_infinity(1024, 1024), 1024);
      (*dptr)++;
      break;
   case 't':
      mul = 1000.0 * 1000 * 1000 * 1000;
      *rlimp = mul_infinity(mul_infinity(mul_infinity(1000, 1000), 1000), 1000);
      (*dptr)++;
      break;
   case 'T':
      mul = 1024.0 * 1024 * 1024 * 1024;
      *rlimp = mul_infinity(mul_infinity(mul_infinity(1024, 1024), 1024), 1024);
      (*dptr)++;
      break;
   case ',':                    /* no multiplier */
   case '\0':                   /* no multiplier */
   case '/':                    /* no multiplier */
   case ' ':                    /* no multiplier */
      break;
   default:
      snprintf(err_str, err_len, MSG_GDI_UNRECOGNIZEDVALUETRAILER_SS , *dptr, where);
      return 0;
   }

   if ((**dptr != ',') && (**dptr != '\0') && (**dptr != '/')) {
      snprintf(err_str, err_len, MSG_GDI_UNEXPECTEDENDOFNUMERICALVALUE_SC , where, **dptr);
      return 0;
   }

   return mul;
}

/***********************************************************************
 * sge_parse_num_val -
 *    parse numeric attribute values from command-line
 *
 *    Numeric attribute values may look like -l <attr>=<num><m>,...
 *    with <num> being a decimal (both integer and fixed float),
 *    hex and octal constant. <m> is a multiplier (see get_multiplier
 *    above).  hh:mm:ss time values are accepted and returned as seconds.
 *
 * !new feature!
 *    
 *    call the good old parse_num_val() with a non null argument for err_str
 *    and it will ++NOT++ abort your daemon.
 *
 *    in case of a parsing error the err_str gets filled with an error
 *    message
 *
 * NOTES
 *     MT-NOTE: sge_parse_num_val() is MT safe
 **********************************************************************/

u_long32 
sge_parse_num_val(sge_rlim_t *rlimp, double *dvalp, 
                  const char *str, const char *where, 
                  char *err_str, int err_len)
{
   double dummy;
   sge_rlim_t rlim, rlmuli;
   double dval;
   u_long32 ldummy;
   char *dptr;
   double muli;

   if (!rlimp)
      rlimp = &rlim;
   if (!dvalp)
      dvalp = &dval;

   if (err_str)
      err_str[0] = '\0';

   if (!strcasecmp(str, "true")) {
      /* C-language says bool is a numeric. For reasons of simplicity we 
         agree. */
      *dvalp = 1;
      *rlimp = 1;
      return 1;
   } else if (!strcasecmp(str, "false")) {
      *dvalp = 0;
      *rlimp = 0;
      return 0;
   } else if (!strcasecmp(str, "infinity")) {
      *dvalp = DBL_MAX;       /* use this for comparing limits */
      *rlimp = RLIM_INFINITY; /* use this for limit setting */
      return 0xFFFFFFFF;      /* return max ulong in 32 bit */
   } else if (strchr(str, ':')) {
      /* This is a time value in the format hr:min:sec */

      /* find the hours first */
      double t = strtod(str, &dptr);
      if (t > 0x7fffffff) {
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEFORHOUREXCEEDED_SS , where, str);
         return 0;
      }
      ldummy = (u_long32)(3600 * t);
      *rlimp = (sge_rlim_t)(long)mul_infinity(t, 3600.0);
      *dvalp = 3600 * t;

      if ((*dptr) != ':') {  
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEINVALID_SS , where, str);
         return 0;
      }
      /* now go for the minutes */
      dptr++;
      t = strtod(dptr, &dptr);
      if (t > 0x7fffffff) {
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEFORMINUTEEXCEEDED_SS , where, str);
         return 0;
      }
      ldummy += (u_long32)(60 * t);
      *rlimp = add_infinity(*rlimp, (sge_rlim_t)(60*t));
      *dvalp += 60 * t;

      if ((*dptr) != ':') {
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEINVALID_SS , where, str);
         return 0;
      }
      /* the seconds finally */
      dptr++;

      t = strtod(dptr, &dptr);
      ldummy += (u_long32)t;
      *rlimp = (sge_rlim_t)(long)add_infinity(*rlimp, t);
      *dvalp += t;

      while(*dptr) {
         if (!isspace((int) *dptr)) {
            snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEINVALID_SS , where, str);
            return 0;
         }
         dptr++;
      }

      return (ldummy);

   } else if (strchr(str, '.') || *str != '0') {
      /* obviously this is no hex and no oct
       * ==> allow for both decimal and fixed float
       */
       
      double t = strtod(str, &dptr);
      
      if (t > 0x7fffffff)
         dummy = 0x7fffffff;
      else
         dummy = t;
   
      if ((dummy == 0.0) && (dptr == str)) {    /* no valid number ==> bail */
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEINVALIDNONUMBER_SS , where, str);
         return 0;
      }

      /* OK, we got it */
      if (!(muli = get_multiplier(&rlmuli, &dptr, where, err_str, err_len)))
         return 0;
      dummy =    (u_long32) (dummy * muli);
      *dvalp =              t * muli;

      if (t > RLIM_MAX || rlmuli>= RLIM_MAX || (double)(RLIM_MAX/muli)<t)
         *rlimp = RLIM_INFINITY;
      else 
         *rlimp = (sge_rlim_t)(t*rlmuli);

      return (u_long32)dummy;

   } else { /* if (strchr(str, '.') || *str != '0') */
      /* This is either a hex or an octal ==> no fixed float allowed;
       * just use strtol
       */
      u_long32 t = strtol(str, &dptr, 0);   /* base=0 will handle both hex and oct */
      ldummy = t;
      *rlimp = t;
      *dvalp = t;

      if (dptr == str) {        /* no valid number ==> bail */
         snprintf(err_str, err_len, MSG_GDI_NUMERICALVALUEINVALIDNOHEXOCTNUMBER_SS , where, str);
         return 0;
      }
      /* OK, we got it */
      if (!(muli = get_multiplier(&rlmuli, &dptr, where, err_str, err_len)))
         return 0;
      ldummy *= (u_long32)muli;
      *rlimp = mul_infinity(*rlimp, rlmuli);
      *dvalp *= muli;

      return (u_long32)ldummy;
   }
}
