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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"

#include "cull/cull_listP.h"
#include "cull/cull_list.h"
#include "cull/cull_db.h"
#include "cull/cull_parse.h"
#include "cull/cull_multitype.h"
#include "cull/cull_whatP.h"
#include "cull/cull_lerrnoP.h"
#include "cull/pack.h"
#include "cull/cull_pack.h"

#include "gdi/msg_gdilib.h"

static lEnumeration *subscope_lWhat(cull_parse_state* state, va_list *app);

/****** cull/what/nm_set() ****************************************************
*  NAME
*     nm_set() -- Build an int vector
*
*  SYNOPSIS
*     void nm_set(int job_field[], int nm) 
*
*  FUNCTION
*     Build an int vector as used by lIntVector2What()
*
*  INPUTS
*     int job_field[] - int vector 
*     int nm          - field name id  
******************************************************************************/
void nm_set(int job_field[], int nm) 
{
   int i;

   DENTER(TOP_LAYER, "nm_set");

   /* seek it */
   for (i=0; job_field[i]!=NoName; i++)
      if (job_field[i] == nm) {
         DRETURN_VOID; /*found*/
      }

   /* set it */
   DPRINTF(("%s\n", lNm2Str(nm)));
   job_field[i++] = nm;
   job_field[i] = NoName;

   DRETURN_VOID;
}

/****** cull/what/lReduceDescr() **********************************************
*  NAME
*     lReduceDescr() -- Reduce a descriptor 
*
*  SYNOPSIS
*     int lReduceDescr(lDescr **dst_dpp, lDescr *src_dp, lEnumeration *enp) 
*
*  FUNCTION
*     Makes a new descriptor in 'dst_dpp' that contains only those
*     fields from 'src_dp' that are in 'enp'. 
*
*  INPUTS
*     lDescr **dst_dpp  - destination for reduced descriptor 
*     lDescr *src_dp    - source descriptor 
*     lEnumeration *enp - condition 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*******************************************************************************/
int lReduceDescr(lDescr **dst_dpp, lDescr *src_dp, lEnumeration *enp) 
{
   int n, index = 0;

   DENTER(TOP_LAYER, "lReduceDescr");

   if (!dst_dpp || !src_dp || !enp) {
      DRETURN(-1);
   }

   n = lCountWhat(enp, src_dp);

   if (n == 0) {
      DRETURN(0);
   }

   /* create new partial descriptor */
   if (!(*dst_dpp = (lDescr *) malloc(sizeof(lDescr) * (n + 1)))) {
      DRETURN(-1);
   }
   lPartialDescr(enp, src_dp, *dst_dpp, &index);

   DRETURN(0);
}

/* ------------------------------------------------------------ 
   _lWhat creates an enumeration array. This is used in lWhat
   to choose special fields of a list element.
 */
lEnumeration *_lWhat(const char *fmt, const lDescr *dp, 
                     const int *nm_list, int nr_nm) 
{
   int neg = 0;
   int i, j, k, n, size = 0;
   const char *s;
   lEnumeration *ep = NULL;
   lEnumeration *ep2 = NULL;
   int error_status;
   cull_parse_state state;

   DENTER(CULL_LAYER, "_lWhat");

   if (!fmt) {
      error_status = LENOFORMATSTR;
      goto error;
   }
   /* how many fields are selected (for malloc) */
   for (n = 0, s = fmt; *s; s++)
      if (*s == '%')
         n++;

   /* are the number of %'s the number of names - 1 ? */
   if (nr_nm != n - 1) {
      error_status = LESYNTAX;
      goto error;
   }

   memset(&state, 0, sizeof(state));
   if (scan(fmt, &state) != TYPE) {
      /* %T expected */
      error_status = LESYNTAX;
      goto error;
   }
   eat_token(&state);                 /* eat %T */
   if (scan(NULL, &state) != BRA) {
      /* ( expected */
      error_status = LESYNTAX;
      goto error;
   }
   eat_token(&state);                 /* eat ( */

   /* Nr of fields to malloc */
   /* for CULL_ALL and NONE we need one lEnumeration struct */
   if (n > 1)
      n--;
   if (!(ep = (lEnumeration *) malloc(sizeof(lEnumeration) * (n + 1)))) {
      error_status = LEMALLOC;
      goto error;
   }

   /* Special cases CULL_ALL fields are selected or NONE is selected */
   /* Normally the arguments nm_list and nr_nm should be NULL(0)     */
   switch (scan(NULL, &state)) {

   case CULL_ALL:
      ep[0].pos = WHAT_ALL;
      ep[0].nm = -99;
      ep[0].mt = -99;
      ep[0].ep = NULL;
      eat_token(&state);
      ep[1].pos = 0;
      ep[1].nm = NoName;
      ep[1].mt = lEndT;
      ep[1].ep = NULL;
      DRETURN(ep);

   case CULL_NONE:
      ep[0].pos = WHAT_NONE;
      ep[0].nm = -99;
      ep[0].mt = -99;
      ep[0].ep = NULL;
      eat_token(&state);
      ep[1].pos = 0;
      ep[1].nm = NoName;
      ep[1].mt = lEndT;
      ep[1].ep = NULL;
      DRETURN(ep);

   default:
      break;
   }

   if (scan(NULL, &state) == NEG) {
      neg = 1;
      if ((size = lCountDescr(dp)) == -1) {
         error_status = LECOUNTDESCR;
         goto error;
      }
      /* subtract the fields that shall not be enumerated */
      size -= n;
      eat_token(&state);              /* eat ! */
      if (scan(NULL, &state) != BRA) {
         /* ( expected */
         error_status = LESYNTAX;
         goto error;
      }
      else
         eat_token(&state);           /* eat_token BRA, if it was a BRA */
   }

   for (i = 0; i < n; i++) {

      ep[i].nm = nm_list[i];
      if ((ep[i].pos = lGetPosInDescr(dp, ep[i].nm)) < 0) {
         error_status = LENAMENOT;
         goto error;
      }
      ep[i].mt = dp[ep[i].pos].mt; 
      ep[i].ep = NULL;

      if (scan(NULL, &state) != FIELD) {
         error_status = LESYNTAX;
         goto error;
      }
      eat_token(&state);

   }

   ep[n].pos = 0;
   ep[n].nm = NoName;
   ep[n].mt = lEndT;
   ep[n].ep = NULL;

   if (neg) {
      if (scan(NULL, &state) != KET) {
         error_status = LESYNTAX;
         goto error;
      }
      eat_token(&state);

      if (!(ep2 = (lEnumeration *) malloc(sizeof(lEnumeration) * (size + 1)))) {
         error_status = LEMALLOC;
         goto error;
      }

      k = 0;
      for (i = 0; dp[i].nm != NoName; i++) {
         /* remove the elements listed in the format string */
         for (j = 0; ep[j].nm != NoName && ep[j].nm != dp[i].nm; j++);
         if (ep[j].nm != NoName)
            continue;
         ep2[k].pos = i;
         ep2[k].nm = dp[i].nm;
         ep2[k].mt = dp[i].mt;
         ep2[k].ep = NULL;
         k++;
      }
      ep2[k].pos = 0;
      ep2[size].nm = NoName;
      ep2[size].mt = lEndT;
      ep2[size].ep = NULL;
      lFreeWhat(&ep);
      ep = ep2;
   }

   if (scan(NULL, &state) != KET) {
      error_status = LESYNTAX;
      goto error;
   }
   eat_token(&state);                 /* eat ) */

   DRETURN(ep);

 error:
   LERROR(error_status);
   lFreeWhat(&ep);
   DPRINTF(("error_status = %d\n", error_status));
   DRETURN(NULL);
}

/****** cull/what/lWhat() *****************************************************
*  NAME
*     lWhat() -- Create a new field enumeration structure
*
*  SYNOPSIS
*     lEnumeration *lWhat(const char *fmt, ...) 
*
*  FUNCTION
*     Create a new enumeration described by 'fmt'
*
*  INPUTS
*     const char *fmt - format string: 
*
*       Syntax:
*         element := type "(" attribute_list ")" .
*         type := "%T" .
*         attribute_list := "ALL" | "NONE" | attributes ")" .
*         attributes = { "%I" | "%I" "->" element } .
*
*       where:
*         %T specifies the descriptor type attribute_list, ALL fields,
*         no fields, or the fields described by attributes; "->" selects
*         a sub-element
*
*     ...   - variable list of arguments corresponding to the % placeholders
*
*  EXAMPLES
*      fmt:
*         1) "%T(NONE)"
*         2) "%T(ALL)"
*         3) "%T(%I%I)"
*         4) "%T(%I%I->%T(%I%I))"
*      varargs corresponding to examples above:
*         1) JB_Type
*         2) JB_Type
*         3) JB_Type, JB_job_number, JB_ja_tasks
*         4) JB_Type, JB_job_number, JB_ja_tasks, JAT_Type, JAT_task_number,
*            JAT_status
*
*  RESULT
*     lEnumeration* - new enumeration
*
*  NOTES
*     "%I" is equivalent to "%I->%T(ALL)"
*     "" is NOT equivalent to "%I->%T(NONE)"
*******************************************************************************/
lEnumeration *lWhat(const char *fmt, ...)
{
   lEnumeration *enumeration = NULL;
   va_list ap;
   cull_parse_state state;

   DENTER(CULL_LAYER, "lWhat");

   if (!fmt) {
      LERROR(LENOFORMATSTR);
      DRETURN(NULL);
   }

   /* 
      initialize scan function, the actual token is scanned again 
      in the subscope function. There we call eat_token to go ahead
    */
   memset(&state, 0, sizeof(state));
   scan(fmt, &state);
   
   /* parse */
   va_start(ap, fmt);
   enumeration = subscope_lWhat(&state, &ap);
   va_end(ap);

   if (!enumeration) {
      LERROR(LEPARSECOND);
      DRETURN(NULL);
   }

   DRETURN(enumeration);
}

static lEnumeration *subscope_lWhat(cull_parse_state* state, va_list *app)
{
   lDescr *dp = NULL;
   lEnumeration *enumeration = NULL;
   int token;
   lEnumeration ep[1000];
   int next_id = 0;
   int i;

   DENTER(CULL_LAYER, "subscope_lWhat");

   if (scan(NULL, state) != TYPE) {
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat %T */
   if (!(dp = va_arg(*app, lDescr *))) {
      LERROR(LEDESCRNULL);
      DRETURN(NULL);
   }

   if (scan(NULL, state) != BRA) {
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat ( */

   token = scan(NULL, state);

   if (token == CULL_ALL) {
      ep[next_id].pos = WHAT_ALL;
      ep[next_id].nm = -99;
      ep[next_id].mt = -99;
      ep[next_id].ep = NULL;
      next_id++;
      eat_token(state);
   } else if (token == CULL_NONE) {
      ep[next_id].pos = WHAT_NONE;
      ep[next_id].nm = -99;
      ep[next_id].mt = -99;
      ep[next_id].ep = NULL;
      next_id++;
      eat_token(state);
   } else {
      do {
         int nm;

         if (token != FIELD) {
            LERROR(LESYNTAX);         
            DRETURN(NULL);
         }
         eat_token(state);                 /* eat %I */

         /* find field in current descriptor */ 
         nm = va_arg(*app, int);     
         for (i = 0; dp[i].nm != NoName; i++) { 
            if (dp[i].nm == nm) {
               ep[next_id].pos = i;
               ep[next_id].nm = nm;
               ep[next_id].mt = dp[i].mt;
               break;
            } 
         }

         token = scan(NULL, state);
         if (token == SUBSCOPE) {
            eat_token(state);                 /* eat -> */
            ep[next_id].ep = subscope_lWhat(state, app);
            token = scan(NULL, state);
         } else {
            ep[next_id].ep = NULL;
         }
         next_id++;

      } while (token == FIELD);
   }

   /* add last element */
   ep[next_id].pos = 0;
   ep[next_id].nm = NoName;
   ep[next_id].mt = lEndT;
   ep[next_id].ep = NULL;
   next_id++;

   if (scan(NULL, state) != KET) {
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat ) */

   /* make copy of local data */
   enumeration = malloc(sizeof(lEnumeration) * next_id);
   if (enumeration == NULL) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   for (i = 0; i < next_id; i++) {  
     /* fixme:  warning that ep[i].pos garbage or undefined */
      enumeration[i].pos = ep[i].pos;
      enumeration[i].nm = ep[i].nm;
      enumeration[i].mt = ep[i].mt;
      enumeration[i].ep = ep[i].ep;
   }

   DRETURN(enumeration);
}

/****** cull/what/lWhatAll() *****************************************************
*  NAME
*     lWhatAll() -- Creates a enumeration array requesting all elements. 
*
*  SYNOPSIS
*     lEnumeration* lWhatAll() 
*
*  FUNCTION
*     Creates a enumeration array that requests complete elements 
*     of whatever typed list. This is a shortcut for 
*     lWhat("%T(ALL)", <List_type>)), cause for all the descriptor is not
*     needed anyway, it is available from the list itself.
*
*  INPUTS
*
*  RESULT
*     lEnumeration* - enumeration 
******************************************************************************/
lEnumeration *lWhatAll()
{
   lEnumeration *ep;
   int error_status;

   DENTER(CULL_LAYER, "lWhatAll");

   if (!(ep = (lEnumeration *) malloc(sizeof(lEnumeration) * 2))) {
      error_status = LEMALLOC;
      goto error;
   }

   ep[0].pos = WHAT_ALL;
   ep[0].nm = -99;
   ep[0].mt = -99;
   ep[0].ep = NULL;
   ep[1].pos = 0;
   ep[1].nm = NoName;
   ep[1].mt = lEndT;
   ep[1].ep = NULL;

   DRETURN(ep);

 error:
   LERROR(error_status);
   DPRINTF(("error_status = %d\n", error_status));
   DRETURN(NULL);
}

/****** cull/what/lFreeWhat() *************************************************
*  NAME
*     lFreeWhat() -- Frees a enumeration array 
*
*  SYNOPSIS
*     void lFreeWhat(lEnumeration **ep) 
*
*  FUNCTION
*     Frees an enumeration array ep. Does nothing if it is null.
*
*  INPUTS
*     lEnumeration **ep - enumeration, will be set to NULL 
*
******************************************************************************/
void lFreeWhat(lEnumeration **ep) 
{
   int i;

   DENTER(CULL_LAYER, "lFreeWhat");

   if (ep == NULL || *ep == NULL) {
      DRETURN_VOID;
   }
   for (i = 0; mt_get_type((*ep)[i].mt) != lEndT; i++) {
      if ((*ep)[i].ep != NULL) {
         lFreeWhat(&((*ep)[i].ep));
      }
   }
   sge_free(ep);
   DRETURN_VOID;
}

/****** cull/what/lCountWhat() ************************************************
*  NAME
*     lCountWhat() -- Returns size of enumeration 
*
*  SYNOPSIS
*     int lCountWhat(const lEnumeration *enp, const lDescr *dp) 
*
*  FUNCTION
*     Returns size (number of fields) of enumeration
*
*  INPUTS
*     const lEnumeration *enp - enumeration 
*     const lDescr *dp        - descriptor 
*
*  RESULT
*     int - number of fields in enumeration, or -1 if one of the arguments
*           is NULL
******************************************************************************/
int lCountWhat(const lEnumeration *enp, const lDescr *dp) 
{
   int n;

   DENTER(CULL_LAYER, "lCountWhat");

   if (!enp) {
      LERROR(LEENUMNULL);
      DRETURN(-1);
   }
   if (!dp) {
      LERROR(LEDESCRNULL);
      DRETURN(-1);
   }
   switch (enp[0].pos) {
   case WHAT_NONE:
      n = 0;
      break;
   case WHAT_ALL:
      if ((n = lCountDescr(dp)) == -1) {
         LERROR(LECOUNTDESCR);
         DRETURN(-1);
      }
      break;
   default:
      for (n = 0; enp[n].nm != NoName; n++)
         ;
   }

   DRETURN(n);
}

/****** cull/what/lCopyWhat() *************************************************
*  NAME
*     lCopyWhat() -- Copy a enumeration array 
*
*  SYNOPSIS
*     lEnumeration* lCopyWhat(const lEnumeration *ep) 
*
*  FUNCTION
*     Copy a enumeration array 
*
*  INPUTS
*     const lEnumeration *ep - enumeration 
*
*  RESULT
*     lEnumeration* - new copy of enumeration 
******************************************************************************/
lEnumeration *lCopyWhat(const lEnumeration *ep) 
{
   int i, n;
   lEnumeration *copy = NULL;

   DENTER(CULL_LAYER, "lCopyWhat");

   if (!ep) {
      LERROR(LEENUMNULL);
      DRETURN(NULL);
   }

   for (n = 0; mt_get_type(ep[n].mt) != lEndT; n++);

   if (!(copy = (lEnumeration *) malloc(sizeof(lEnumeration) * (n + 1)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   for (i = 0; i <= n; i++) {
      copy[i].pos = ep[i].pos;
      copy[i].nm = ep[i].nm;
      copy[i].mt = ep[i].mt;
      copy[i].ep = lCopyWhat(ep[i].ep);
   }

   DRETURN(copy);
}

/****** cull/what/lIntVector2What() *******************************************
*  NAME
*     lIntVector2What() -- Create a enumeration from int array 
*
*  SYNOPSIS
*     lEnumeration* lIntVector2What(const lDescr *dp, const int intv[]) 
*
*  FUNCTION
*     Create a enumeration from int array 
*
*  INPUTS
*     const lDescr *dp - descriptor 
*     const int intv[] - int array 
*
*  RESULT
*     lEnumeration* - enumeration 
******************************************************************************/
lEnumeration *lIntVector2What(const lDescr *dp, const int intv[]) 
{
   lEnumeration *what;
   char fmtstr[2000];
   int i;

   DENTER(CULL_LAYER, "lIntVector2What");

   /*
      reduce a descriptor to get one with only
      these fields that are needed for modify
    */
   strcpy(fmtstr, "%T(");
   for (i = 0; intv[i] != NoName; i++) {
      sge_strlcat(fmtstr, "%I", sizeof(fmtstr));
   }
   sge_strlcat(fmtstr, ")", sizeof(fmtstr));

   DPRINTF(("fmtstr: %s\n", fmtstr));

   what = _lWhat(fmtstr, dp, intv, i);

   DRETURN(what);
}

int lMergeWhat(lEnumeration **what1, lEnumeration **what2)
{
   int ret = 0;

   DENTER(CULL_LAYER, "lMergeWhat");
   if (*what1 == NULL || 
       (*what1)[0].pos == WHAT_NONE ||
       (*what2)[0].pos == WHAT_ALL) {
      /*
       * what1 is empty or what2 contains all attributes
       */
      lFreeWhat(what1);
      *what1 = *what2;
      *what2 = NULL;
   } else if ((*what1)[0].pos == WHAT_ALL) {
      /*
       * what1 contains already all elements
       */
      lFreeWhat(what2);
   } else {
      lEnumeration tmp_result[1000];
      int next_id = 0;
      int i, j;

      /*
       * Add all elements of what1 
       */
      for (i = 0; mt_get_type((*what1)[i].mt) != lEndT; i++) { 
         tmp_result[next_id].pos = (*what1)[i].pos;
         tmp_result[next_id].mt = (*what1)[i].mt;
         tmp_result[next_id].nm = (*what1)[i].nm;
         tmp_result[next_id].ep = (*what1)[i].ep;
         (*what1)[i].ep = NULL;
         next_id++;
      }
      tmp_result[next_id].pos = 0;
      tmp_result[next_id].nm = NoName;
      tmp_result[next_id].mt = lEndT;
      tmp_result[next_id].ep = NULL;

      lFreeWhat(what1);

      /*
       * Add only those of what2 which are not already contained in what1
       * honour subwhat's
       */
      for (i = 0; mt_get_type((*what2)[i].mt) != lEndT; i++) {
         bool skip = false;

         for (j = 0; mt_get_type(tmp_result[j].mt) != lEndT; j++) {
            if (tmp_result[j].mt == (*what2)[i].mt &&
                tmp_result[j].nm == (*what2)[i].nm &&
                tmp_result[j].pos == (*what2)[i].pos) {
               /*TODO: SG is this correct? what happens, if we have a new subfilter
                 in what2 but none in temp? */
               if (tmp_result[next_id].ep != NULL && (*what2)[i].ep != NULL) {
                  lMergeWhat(&(tmp_result[next_id].ep), &((*what2)[i].ep));
               } else {
                  tmp_result[next_id].ep = NULL;
               }

               skip = true;
               break;
            }
         }
         if (!skip) {
            tmp_result[next_id].pos = (*what2)[i].pos;
            tmp_result[next_id].mt = (*what2)[i].mt;
            tmp_result[next_id].nm = (*what2)[i].nm;
            
            /*TODO: SG is this correct? what happens, if we have a new subfilter
              in what2 but none in temp? */
            if (tmp_result[next_id].ep != NULL && (*what2)[i].ep != NULL) {
               lMergeWhat(&(tmp_result[next_id].ep), &((*what2)[i].ep));
            } else { 
               tmp_result[next_id].ep = NULL;
            }
            /* add last element */
            next_id++;
            tmp_result[next_id].pos = 0;
            tmp_result[next_id].nm = NoName;
            tmp_result[next_id].mt = lEndT;
            tmp_result[next_id].ep = NULL;
         }
      }
      tmp_result[next_id].pos = 0;
      tmp_result[next_id].nm = NoName;
      tmp_result[next_id].mt = lEndT;
      tmp_result[next_id].ep = NULL;
      next_id++;
      lFreeWhat(what2);

      /*
       * create result what 
       */
      *what1 = malloc(sizeof(lEnumeration) * next_id);
      if (*what1 != NULL) {
         for (i = 0; i < next_id; i++) {
            (*what1)[i].pos = tmp_result[i].pos;
            (*what1)[i].nm = tmp_result[i].nm;
            (*what1)[i].mt = tmp_result[i].mt;
            (*what1)[i].ep = tmp_result[i].ep;
         }
      } else {
         LERROR(LEMALLOC);
         DTRACE;
         ret = -1;
      }
   }
   DRETURN(ret);
}

int lWhatSetSubWhat(lEnumeration *what1, int nm, lEnumeration **what2)
{
   int ret = -1;
   int i;

   DENTER(CULL_LAYER, "lWhatSetSubWhat");
   if (what1 != NULL && what2 != NULL) {
      for (i = 0; mt_get_type(what1[i].mt) != lEndT; i++) {
         if (what1[i].nm == nm) {
            if (what1[i].ep != NULL) {
               lFreeWhat(&(what1[i].ep));
            }
            what1[i].ep = *what2;
            *what2 = NULL; 
            ret = 0;
            break;
         } 
      }
      lFreeWhat(what2);
   }
   DRETURN(ret);
}
