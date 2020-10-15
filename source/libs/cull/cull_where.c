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
/* Portions of this code are Copyright (c) 2011 Univa Corporation. */
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fnmatch.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_hostname.h"
#include "uti/sge_log.h"

#include "cull/msg_cull.h"
#include "cull/cull_listP.h"
#include "cull/cull_whereP.h"
#include "cull/cull_parse.h"
#include "cull/cull_lerrnoP.h"
#include "cull/pack.h"
#include "cull/cull_pack.h"

#include "gdi/msg_gdilib.h"

static lCondition *read_val(lDescr *dp, cull_parse_state *state, va_list *app);
static lCondition *factor(lDescr *dp, cull_parse_state *state, va_list *app);
static lCondition *product(lDescr *dp, cull_parse_state *state, va_list *app);
static lCondition *negfactor(lDescr *dp, cull_parse_state *state, va_list *app);
static lCondition *sum(lDescr *dp, cull_parse_state *state, va_list *app);
static lCondition *subscope(cull_parse_state *state, va_list *app);

static lCondition *_read_val(lDescr *dp, cull_parse_state *state, WhereArgList *wapp);
static lCondition *_factor(lDescr *dp, cull_parse_state *state, WhereArgList *wapp);
static lCondition *_product(lDescr *dp, cull_parse_state *state, WhereArgList *wapp);
static lCondition *_negfactor(lDescr *dp, cull_parse_state *state, WhereArgList *wapp);
static lCondition *_sum(lDescr *dp, cull_parse_state *state, WhereArgList *wapp);
static lCondition *_subscope(cull_parse_state *state, WhereArgList *wapp);

static void lWriteWhereTo_(const lCondition *cp, int depth, FILE *fp);

/****** cull/where/lOrWhere() *************************************************
*  NAME
*     lOrWhere() -- Combines two conditions with an OR
*
*  SYNOPSIS
*     lCondition* lOrWhere(const lCondition *cp0, const lCondition *cp1) 
*
*  FUNCTION
*     Combines the conditions 'cp0' and 'cp1' logically with an OR 
*
*  INPUTS
*     const lCondition *cp0 - first condition 
*     const lCondition *cp1 - second condition 
*
*  RESULT
*     lCondition* - cp0 OR cp1 
******************************************************************************/
lCondition *lOrWhere(const lCondition *cp0, const lCondition *cp1) 
{
   lCondition *newcp;

   DENTER(TOP_LAYER, "lOrWhere");

   if (!cp0 || !cp1) {
      LERROR(LECONDNULL);
      DRETURN(NULL);
   }

   if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   newcp->op = OR;
   newcp->operand.log.first = (lCondition *)cp0;
   newcp->operand.log.second = (lCondition *)cp1;

   DRETURN(newcp);
}

/****** cull/where/lAndWhere() ************************************************
*  NAME
*     lAndWhere() -- Combines two conditions with an AND
*
*  SYNOPSIS
*     lCondition* lAndWhere(const lCondition *cp0, const lCondition *cp1) 
*
*  FUNCTION
*     Combines the conditions 'cp0' and 'cp1' with an logical AND. 
*
*  INPUTS
*     const lCondition *cp0 - first condition 
*     const lCondition *cp1 - second condition 
*
*  RESULT
*     lCondition* - 'cp0' AND 'cp1' 
******************************************************************************/
lCondition *lAndWhere(const lCondition *cp0, const lCondition *cp1) 
{
   lCondition *newcp;

   DENTER(TOP_LAYER, "lAndWhere");

   if (!cp0 || !cp1) {
      LERROR(LECONDNULL);
      DRETURN(NULL);
   }

   if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   newcp->op = AND;
   newcp->operand.log.first = (lCondition *)cp0;
   newcp->operand.log.second = (lCondition *)cp1;

   DRETURN(newcp);
}

/****** cull/where/lWriteWhereTo() ********************************************
*  NAME
*     lWriteWhereTo() -- Write a condition struct to file stream.
*
*  SYNOPSIS
*     void lWriteWhereTo(const lCondition *cp, FILE *fp) 
*
*  FUNCTION
*     Write a condition struct to file stream. 
*
*  INPUTS
*     const lCondition *cp - condition 
*     FILE *fp             - file stream 
******************************************************************************/
void lWriteWhereTo(const lCondition *cp, FILE *fp) 
{
   lWriteWhereTo_(cp, 0, fp);
}
static void lWriteWhereTo_(const lCondition *cp, int depth, FILE *fp) 
{
   int i;
   char space[80];
   char out[256];

   DENTER(CULL_LAYER, "lWriteWhere");

   if (!cp) {
      LERROR(LECONDNULL);
      DRETURN_VOID;
   }

   out[0] = '\0';

   space[0] = '\0';
   for (i = 0; i < depth; i++)
      sge_strlcat(space, "   ", sizeof(space));

   switch (cp->op) {

   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case HOSTNAMECMP:
   case SUBSCOPE:

      snprintf(out, sizeof(out), "%s %s(%d) ", space,
               lNm2Str(cp->operand.cmp.nm), cp->operand.cmp.nm);

      switch (cp->op) {
      case EQUAL:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "==");
         break;
      case NOT_EQUAL:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "!=");
         break;
      case LOWER_EQUAL:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "<=");
         break;
      case LOWER:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "<");
         break;
      case GREATER_EQUAL:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), ">=");
         break;
      case GREATER:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), ">");
         break;

      case BITMASK:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "m=");
         break;

      case STRCASECMP:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "c=");
         break;

      case PATTERNCMP:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "p=");
         break;

      case HOSTNAMECMP:
         snprintf(&out[strlen(out)], sizeof(out) - strlen(out), "h=");
         break;

      case SUBSCOPE:
         if (!fp) {
            DPRINTF(("%s ->\n", out));
         }
         else {
            fprintf(fp, "%s ->\n", out);
         }
         lWriteWhereTo_(cp->operand.cmp.val.cp, depth+1, fp);
         break;

      default:
         LERROR(LEOPUNKNOWN);
         DRETURN_VOID;
      }

      switch (mt_get_type(cp->operand.cmp.mt)) {
      case lIntT:
         if (!fp) {
            DPRINTF(("%s %d\n", out, cp->operand.cmp.val.i));
         } else {
            fprintf(fp, "%s %d\n", out, cp->operand.cmp.val.i);
         }
         break;
      case lUlongT:
         if (!fp) {
            DPRINTF(("%s "sge_u32"\n", out, cp->operand.cmp.val.ul));
         } else {
            fprintf(fp, "%s "sge_u32"\n", out, cp->operand.cmp.val.ul);
         }
         break;
      case lUlong64T:
         if (!fp) {
            DPRINTF(("%s "sge_u64"\n", out, cp->operand.cmp.val.ul64));
         } else {
            fprintf(fp, "%s "sge_u64"\n", out, cp->operand.cmp.val.ul64);
         }
         break;
      case lStringT:
         if (!fp) {
            DPRINTF(("%s \"%s\"\n", out, cp->operand.cmp.val.str));
         } else {
            fprintf(fp, "%s \"%s\"\n", out, cp->operand.cmp.val.str);
         }
         break;

      case lHostT:
         if (!fp) {
            DPRINTF(("%s \"%s\"\n", out, cp->operand.cmp.val.host));
         } else {
            fprintf(fp, "%s \"%s\"\n", out, cp->operand.cmp.val.host);
         }
         break;

      case lListT:
/*          DPRINTF(("lWriteWhere error\n")); */
         break;
      case lFloatT:
         if (!fp) {
            DPRINTF(("%s %f\n", out, cp->operand.cmp.val.fl));
         } else {
            fprintf(fp, "%s %f\n", out, cp->operand.cmp.val.fl);
         }
         break;
      case lDoubleT:
         if (!fp) {
            DPRINTF(("%s %f\n", out, cp->operand.cmp.val.db));
         } else {
            fprintf(fp, "%s %f\n", out, cp->operand.cmp.val.db);
         }
         break;
      case lLongT:
         if (!fp) {
            DPRINTF(("%s %ld\n", out, cp->operand.cmp.val.l));
         } else {
            fprintf(fp, "%s %ld\n", out, cp->operand.cmp.val.l);
         }
         break;
      case lBoolT:
         if (!fp) {
            DPRINTF(("%s %s\n", out, cp->operand.cmp.val.b ? "true" : "false"));
         } else {
            fprintf(fp, "%s %s\n", out, cp->operand.cmp.val.b ? "true" : "false");
         }
         break;
      case lCharT:
         if (!fp) {
            DPRINTF(("%s %c\n", out, cp->operand.cmp.val.c));
         } else {
            fprintf(fp, "%s %c\n", out, cp->operand.cmp.val.c);
         }
         break;
      case lRefT:
         if (!fp) {
            DPRINTF(("%s %p\n", out, cp->operand.cmp.val.ref));
         } else {
            fprintf(fp, "%s %p\n", out, cp->operand.cmp.val.ref);
         }
         break;
      default:
         unknownType("lWriteWhere");
         DRETURN_VOID;
      }
      break;

   case AND:
      if (!fp) {
         DPRINTF(("%s(\n", space));
      }
      else {
         fprintf(fp,"%s(\n", space);
      }
      lWriteWhereTo_(cp->operand.log.first, depth+1, fp);
      if (!fp) {
         DPRINTF(("%s&&\n", space));
      }
      else {
         fprintf(fp, "%s&&\n", space);
      }
      lWriteWhereTo_(cp->operand.log.second, depth+1, fp);
      if (!fp) {
         DPRINTF(("%s)\n", space));
      }
      else {
         fprintf(fp, "%s)\n", space);
      }
      break;

   case OR:
      if (!fp) {
         DPRINTF(("%s(\n", space));
      }
      else {
         fprintf(fp, "%s(\n", space);
      }
      lWriteWhereTo_(cp->operand.log.first, depth+1, fp);
      if (!fp) {
         DPRINTF(("%s||\n", space));
      }
      else {
         fprintf(fp, "%s||\n", space);
      }
      lWriteWhereTo_(cp->operand.log.second, depth+1, fp);
      if (!fp) {
         DPRINTF(("%s)\n", space));
      }
      else {
         fprintf(fp, "%s)\n", space);
      }
      break;

   case NEG:
      if (!fp) {
         DPRINTF(("%s!(\n", space));
      }
      else {
         fprintf(fp, "%s!(\n", space);
      }
      lWriteWhereTo_(cp->operand.log.first, depth+1, fp);
      if (!fp) {
         DPRINTF(("%s)\n", space));
      }
      else {
         fprintf(fp, "%s)\n", space);
      }

      break;

   default:
      LERROR(LEOPUNKNOWN);
      DPRINTF(("error: LEOPUNKNOWN\n"));
      DRETURN_VOID;
   }

   DRETURN_VOID;
}

/****** cull/where/lWhere() ***************************************************
*  NAME
*     lWhere() -- Creates a condition tree
*
*  SYNOPSIS
*     lCondition* lWhere(const char *fmt, ...)
*
*  FUNCTION
*     Creates a condition tree. The condition is stated as a format
*     string and an associated list of additional parameters.
*
*  INPUTS
*     const char *fmt - format string
*         Syntax:
*          cond: type "(" negsimple [logop {negsimple | "%I" "->" cond } ] ")"
*          type: "%T"
*          negsimple: {simple | "!" "(" simple ")" }
*          simple: "%I" relop valuetype [logop simple ...]
*          logop: {"&&" | "||"}
*          relop: { "<" | ">" | "==" | "!=" | "<=" | ">=" | "m=" |
*                   "c=" | "p=" | "h=" }
*          valuetype: { "%d" | "%s" | "%u" | "%f" | "%g" | "%o" | "%c" }
*
*         The logop operators mean:
*          &&   logical and
*          ||   logical or
*          !    logical not
*         The relop specifiers have the following meaning:
*          <    comparison of numeric types as in the C language
*          >
*          <=
*          >=
*          ==
*          !=
*          ==   case sensitive comparison of strings
*          c=   case insensitive comparison of strings
*          m=   bitmask evaluation operator
*          p=   pattern matching string comparison
*          h=   host matching string comparison
*
*         For valuetype the specifiers represent in the order above:
*          { int | string | unsigned long | float | double | long | char }
*
*     ...  - additional arguments corresponding to fmt placeholders
*
*  EXAMPLES
*
*    	where1 = lWhere("%T(%I==%s && %I->%T(%I<%d || %I>%u || %I m= %u ))",
*    			type1, field1, "Hello", field2, subtype,
*    			subfield1, 12, subfield2, 34,
*    			bitmasksubfield, IDLE | RUNNING);
*    	where2 = lWhere( "%T(!(%I==%s))", type1, field1, "Hello");
*
*    The condition 'where1' says:
*
*    field1 of list element with descriptor type1 must contain "Hello"
*    AND the sub-list stored in field2 with descriptor subtype has a
*    field subfield1 and subfield2 shall fulfil subfield1 < 12 OR
*    subfield2 > 34 OR in the bitmasksubfield the bits for RUNNING and
*    IDLE are set.  Bitmasks are stored in an unsigned long
*    (ulong_32).
*
*    The condition 'where2' says:
*
*    NOT ( field1 == "Hello" ) for field1 of a list with descriptor type1.
*
*    _lWhere is equivalent to lWhere concerning the format string
*    describing the condition.  The variable argument list of lWhere
*    is replaced by an array of arguments delivering the required
*    information.
*
*    The WhereArg struct is built as follows:
*
*    	struct _WhereArg {
*    		lDescr      *descriptor;
*    		int         field;
*    		lMultitype  *value;
*    	};
*
*    The translation of the varargs lWhere functionality to the
*    WhereArgList mechanism is shown below:
*
*    	where = lWhere("%T( %I == %s && %I ->
*    			%T ( %I < %d ) )",
*    			QueueT, Q_hostname, "durin.q",
*    			Q_ownerlist, OwnerT, O_ownerage, 22);
*
*    The corresponding WhereArgList is:
*
*    	WhereArg whereargs[20];
*
*    	whereargs[0].descriptor = QueueT;
*    	whereargs[1].field      = Q_hostname;
*    	whereargs[1].value.str  = "durin.q";
*    	whereargs[2].field      = Q_ownerlist;
*    	whereargs[2].descriptor = OwnerT;
*    	whereargs[3].field      = O_ownerage;
*    	whereargs[3].value.i    = 22;
*
*    	where = _lWhere("%T( %I == %s && %I -> %T ( %I < %d ) )",
*    			whereargs);
*
*  RESULT
*     lCondition* - new condition; NULL means all elements will match
******************************************************************************/
lCondition *lWhere(const char *fmt,...)
{
   lCondition *cond;
   va_list ap;
   cull_parse_state state;

   DENTER(CULL_LAYER, "lWhere");

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
   cond = subscope(&state, &ap);
   va_end(ap);

   if (!cond) {
      LERROR(LEPARSECOND);
      DRETURN(NULL);
   }

   DRETURN(cond);
}

static lCondition *subscope(cull_parse_state* state, va_list *app) 
{
   lDescr *dp = NULL;
   lCondition *cp = NULL;

   DENTER(CULL_LAYER, "subscope");

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

   if (!(cp = sum(dp, state, app))) {
      LERROR(LESUM);
      DRETURN(NULL);
   }

   if (scan(NULL, state) != KET) {
      LERROR(LESYNTAX);
      lFreeWhere(&cp);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat ) */

   DRETURN(cp);
}

static lCondition *sum(lDescr *dp, cull_parse_state *state, va_list *app) 
{
   lCondition *cp, *newcp;

   DENTER(CULL_LAYER, "sum");

   cp = product(dp, state, app);

   while (scan(NULL, state) == OR) {
      eat_token(state);              /* eat || */

      if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         LERROR(LEMALLOC);
         lFreeWhere(&cp);
         DRETURN(NULL);
      }
      newcp->op = OR;
      newcp->operand.log.first = cp;
      newcp->operand.log.second = product(dp, state, app);
      cp = newcp;
   }

   DRETURN(cp);
}

static lCondition *product(lDescr *dp, cull_parse_state *state, va_list *app) 
{
   lCondition *cp, *newcp;

   DENTER(CULL_LAYER, "product");

   cp = factor(dp, state, app);

   while (scan(NULL, state) == AND) {
      eat_token(state);

      if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         lFreeWhere(&cp);
         LERROR(LEMALLOC);
         DRETURN(NULL);
      }
      newcp->op = AND;
      newcp->operand.log.first = cp;
      newcp->operand.log.second = factor(dp, state, app);
      cp = newcp;
   }

   DRETURN(cp);
}

static lCondition *factor(lDescr *dp, cull_parse_state *state, va_list *app) 
{
   lCondition *cp;

   DENTER(CULL_LAYER, "factor");

   if (scan(NULL, state) == NEG) {
      eat_token(state);

      if (!(cp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         lFreeWhere(&cp);
         LERROR(LEMALLOC);
         DRETURN(NULL);
      }
      cp->operand.log.first = negfactor(dp, state, app);
      cp->operand.log.second = NULL;
      cp->op = NEG;
   }
   else
      cp = negfactor(dp, state, app);

   DRETURN(cp);
}

static lCondition *negfactor(
lDescr *dp,
cull_parse_state *state, 
va_list *app 
) {
   lCondition *cp;

   DENTER(CULL_LAYER, "negfactor");

   if (scan(NULL, state) == BRA) {
      eat_token(state);

      cp = sum(dp, state, app);

      if (scan(NULL, state) != KET) {
         lFreeWhere(&cp);
         LERROR(LESYNTAX);
         DRETURN(NULL);
      }
      eat_token(state);

      DRETURN(cp);
   }

   DRETURN(read_val(dp, state, app));

}

static lCondition *read_val(lDescr *dp, cull_parse_state *state, va_list *app) 
{
   /*       
      without the usage of s insight throws
      a READ_OVERFLOW 
    */
   volatile char *s;

   lCondition *cp;
   int token;

   DENTER(CULL_LAYER, "read_val");

   if (!dp) {
      LERROR(LEDESCRNULL);
      DRETURN(NULL);
   }

   if (!(cp = (lCondition *)calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   if ((token = scan(NULL, state)) != FIELD) {
      lFreeWhere(&cp);
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);

   cp->operand.cmp.nm = va_arg(*app, int);
   if ((cp->operand.cmp.pos = lGetPosInDescr(dp, cp->operand.cmp.nm)) < 0) {
      lFreeWhere(&cp);
      LERROR(LENAMENOT);
      DRETURN(NULL);
   }
   cp->operand.cmp.mt = dp[cp->operand.cmp.pos].mt;

   switch (token = scan(NULL, state)) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case HOSTNAMECMP:
      cp->op = token;
      eat_token(state);
      break;

   case SUBSCOPE:
      cp->op = token;
      eat_token(state);
      if (mt_get_type(cp->operand.cmp.mt) != lListT) {
         lFreeWhere(&cp);
         LERROR(LEINCTYPE);
         DRETURN(NULL);
      }
      cp->operand.cmp.val.cp = subscope(state, app);
      DRETURN(cp);

   default:
      lFreeWhere(&cp);
      LERROR(LEOPUNKNOWN);
      DRETURN(NULL);
   }

   switch (scan(NULL, state)) {
   case INT:
      if (mt_get_type(cp->operand.cmp.mt) != lIntT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEINTT );
      cp->operand.cmp.val.i = va_arg(*app, lInt);
      break;

   case STRING:
      if ( (mt_get_type(cp->operand.cmp.mt) != lStringT ) && (mt_get_type(cp->operand.cmp.mt) != lHostT )   )
         incompatibleType(MSG_CULL_WHERE_SHOULDBESTRINGT );
      if ( mt_get_type(cp->operand.cmp.mt) == lStringT ) {
         s = va_arg(*app, char *);
         cp->operand.cmp.val.str = strdup((char *) s);
         /* cp->operand.cmp.val.str = strdup(va_arg(*app, char *)); */
      } 
      if ( mt_get_type(cp->operand.cmp.mt) == lHostT ) {
         s = va_arg(*app, char *);
         cp->operand.cmp.val.host = strdup((char *) s);
      }
      break;

   case ULONG:
      if (mt_get_type(cp->operand.cmp.mt) != lUlongT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEULONGT);
      cp->operand.cmp.val.ul = va_arg(*app, lUlong);
      break;

   case ULONG64:
      if (mt_get_type(cp->operand.cmp.mt) != lUlong64T)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEULONG64T);
      cp->operand.cmp.val.ul64 = va_arg(*app, lUlong64);
      break;

   case FLOAT:
      if (mt_get_type(cp->operand.cmp.mt) != lFloatT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEFLOATT);
      /* a float value is stored as a double in the va_list */
      /* so we have to read it as a double value                              */
      cp->operand.cmp.val.fl = (lFloat) va_arg(*app, lDouble);
      break;

   case DOUBLE:
      if (mt_get_type(cp->operand.cmp.mt) != lDoubleT)
         incompatibleType( MSG_CULL_WHERE_SHOULDBEDOUBLET);
      cp->operand.cmp.val.db = va_arg(*app, lDouble);
      break;

   case LONG:
      if (mt_get_type(cp->operand.cmp.mt) != lLongT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBELONGT);
      cp->operand.cmp.val.l = va_arg(*app, lLong);
      break;

   case CHAR:
      if (mt_get_type(cp->operand.cmp.mt) != lCharT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBECHART);
/* fixme: why is gcc special here? */
#if (__GNUC__ == 2 && __GNUC_MINOR__ > 95) || __GNUC__ >= 3 || __INSIGHT__
      cp->operand.cmp.val.c = va_arg(*app, int);
#else
      cp->operand.cmp.val.c = va_arg(*app, lChar);
#endif
      break;

   case BOOL:
      if (mt_get_type(cp->operand.cmp.mt) != lBoolT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEBOOL);
#if (__GNUC__ == 2 && __GNUC_MINOR__ > 95) || __GNUC__ >= 3 || __INSIGHT__
      cp->operand.cmp.val.b = va_arg(*app, int);
#else
      cp->operand.cmp.val.b = va_arg(*app, lBool);
#endif
      break;

   case REF:
      if (mt_get_type(cp->operand.cmp.mt) != lRefT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEREFT );
      cp->operand.cmp.val.ref = va_arg(*app, lRef);
      break;

   default:
      lFreeWhere(&cp);
      unknownType("lWhere");
      DRETURN(NULL);
   }
   eat_token(state);

   DRETURN(cp);
}


static lCondition *_subscope(cull_parse_state *state, WhereArgList *wapp) 
{
   lDescr *dp = NULL;
   lCondition *cp = NULL;

   DENTER(CULL_LAYER, "_subscope");

   if (scan(NULL, state) != TYPE) {
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat %T */
   /* 
    * Deliver descriptor & increment the WhereArgList to 
    * the next element 
    */
   /*    DPRINTF(("(*wapp) = %p\n", *wapp)); */
   dp = (*wapp)++->descriptor;
   if (!(dp)) {
      LERROR(LEDESCRNULL);
      DRETURN(NULL);
   }

   if (scan(NULL, state) != BRA) {
      LERROR(LESYNTAX);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat ( */

   if (!(cp = _sum(dp, state, wapp))) {
      LERROR(LESUM);
      DRETURN(NULL);
   }

   if (scan(NULL, state) != KET) {
      LERROR(LESYNTAX);
      lFreeWhere(&cp);
      DRETURN(NULL);
   }
   eat_token(state);                 /* eat ) */

   DRETURN(cp);
}

static lCondition *_sum(lDescr *dp, cull_parse_state *state, WhereArgList *wapp) 
{
   lCondition *cp, *newcp;

   DENTER(CULL_LAYER, "_sum");

   cp = _product(dp, state, wapp);

   while (scan(NULL, state) == OR) {
      eat_token(state);              /* eat || */

      if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         LERROR(LEMALLOC);
         lFreeWhere(&cp);
         DRETURN(NULL);
      }
      newcp->op = OR;
      newcp->operand.log.first = cp;
      newcp->operand.log.second = _product(dp, state, wapp);
      cp = newcp;
   }

   DRETURN(cp);
}

static lCondition *_product(lDescr *dp, cull_parse_state *state, WhereArgList *wapp) 
{
   lCondition *cp, *newcp;

   DENTER(CULL_LAYER, "_product");

   cp = _factor(dp, state, wapp);

   while (scan(NULL, state) == AND) {
      eat_token(state);

      if (!(newcp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         lFreeWhere(&cp);
         LERROR(LEMALLOC);
         DRETURN(NULL);
      }
      newcp->op = AND;
      newcp->operand.log.first = cp;
      newcp->operand.log.second = _factor(dp, state, wapp);
      cp = newcp;
   }

   DRETURN(cp);
}

static lCondition *_factor(lDescr *dp, cull_parse_state *state, WhereArgList *wapp) 
{
   lCondition *cp;

   DENTER(CULL_LAYER, "_factor");

   if (scan(NULL, state) == NEG) {
      eat_token(state);

      if (!(cp = (lCondition *) calloc(1, sizeof(lCondition)))) {
         lFreeWhere(&cp);
         LERROR(LEMALLOC);
         DRETURN(NULL);
      }
      cp->operand.log.first = _negfactor(dp, state, wapp);
      cp->operand.log.second = NULL;
      cp->op = NEG;
   }
   else
      cp = _negfactor(dp, state, wapp);

   DRETURN(cp);
}

static lCondition *_negfactor(lDescr *dp, cull_parse_state *state, WhereArgList *wapp) 
{
   lCondition *cp;

   DENTER(CULL_LAYER, "_negfactor");

   if (scan(NULL, state) == BRA) {
      eat_token(state);

      cp = _sum(dp, state, wapp);

      if (scan(NULL, state) != KET) {
         LERROR(LESYNTAX);
         lFreeWhere(&cp);
         DRETURN(NULL);
      }
      eat_token(state);

      DRETURN(cp);
   }

   DRETURN(_read_val(dp, state, wapp));

}

static lCondition *_read_val(lDescr *dp, cull_parse_state *state, WhereArgList *wapp) 
{
   lCondition *cp;
   int token;

   DENTER(CULL_LAYER, "_read_val");

   if (!dp) {
      LERROR(LEDESCRNULL);
      DRETURN(NULL);
   }

   if (!(cp = (lCondition *) calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   if ((token = scan(NULL, state)) != FIELD) {
      lFreeWhere(&cp);
      LERROR(LESYNTAX);
      DRETURN(0);
   }
   eat_token(state);

/*    DPRINTF(("(*wapp) = %p\n", *wapp)); */
/*    DPRINTF(("(*wapp)->field = %d\n", (*wapp)->field)); */
   cp->operand.cmp.nm = (*wapp)->field;
   if ((cp->operand.cmp.pos = lGetPosInDescr(dp, cp->operand.cmp.nm)) < 0) {
      lFreeWhere(&cp);
      LERROR(LENAMENOT);
      DRETURN(NULL);
   }
   cp->operand.cmp.mt = dp[cp->operand.cmp.pos].mt;

   switch (token = scan(NULL, state)) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case HOSTNAMECMP:
      cp->op = token;
      eat_token(state);
      break;

   case SUBSCOPE:
      cp->op = token;
      eat_token(state);
      if (mt_get_type(cp->operand.cmp.mt) != lListT) {
         lFreeWhere(&cp);
         LERROR(LEINCTYPE);
         DRETURN(NULL);
      }
      cp->operand.cmp.val.cp = _subscope(state, wapp);
      DRETURN(cp);

   default:
      lFreeWhere(&cp);
      LERROR(LEOPUNKNOWN);
      DRETURN(NULL);
   }

   switch (scan(NULL, state)) {
   case INT:
      if (mt_get_type(cp->operand.cmp.mt) != lIntT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEINTT);
/*       DPRINTF(("(*wapp)->value.i = %d\n", (*wapp)->value.i)); */
      cp->operand.cmp.val.i = (*wapp)++->value.i;
      break;

   case STRING:
      if ( (mt_get_type(cp->operand.cmp.mt) != lStringT) && (mt_get_type(cp->operand.cmp.mt) != lHostT)   )
         incompatibleType(MSG_CULL_WHERE_SHOULDBESTRINGT);

      if ( mt_get_type(cp->operand.cmp.mt) == lStringT ) {
         /* DPRINTF(("(*wapp)->value.str = %s\n", (*wapp)->value.str)); */
         cp->operand.cmp.val.str = (*wapp)++->value.str;
      } 
      if ( mt_get_type(cp->operand.cmp.mt) == lHostT   ) {
         cp->operand.cmp.val.host = (*wapp)++->value.host;
      }
      break;

   case ULONG:
      if (mt_get_type(cp->operand.cmp.mt) != lUlongT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEULONGT);
/*       DPRINTF(("(*wapp)->value.ul = %ul\n", (*wapp)->value.ul)); */
      cp->operand.cmp.val.ul = (*wapp)++->value.ul;
      break;

   case ULONG64:
      if (mt_get_type(cp->operand.cmp.mt) != lUlong64T)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEULONG64T);
/*       DPRINTF(("(*wapp)->value.ul64 = %ul\n", (*wapp)->value.ul64)); */
      cp->operand.cmp.val.ul64 = (*wapp)++->value.ul64;
      break;

   case FLOAT:
      if (mt_get_type(cp->operand.cmp.mt) != lFloatT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEFLOATT);
      /* a float value is stored as a double in the va_list */
      /* so we have to read it as a double value                              */
/*       DPRINTF(("(*wapp)->value.fl = %f\n", (*wapp)->value.fl)); */
      cp->operand.cmp.val.fl = (*wapp)++->value.fl;
      break;

   case DOUBLE:
      if (mt_get_type(cp->operand.cmp.mt) != lDoubleT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEDOUBLET);
/*       DPRINTF(("(*wapp)->value.db = %f\n", (*wapp)->value.db)); */
      cp->operand.cmp.val.db = (*wapp)++->value.db;
      break;

   case LONG:
      if (mt_get_type(cp->operand.cmp.mt) != lLongT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBELONGT);
/*       DPRINTF(("(*wapp)->value.l = %ld\n", (*wapp)->value.l)); */
      cp->operand.cmp.val.l = (*wapp)++->value.l;
      break;

   case CHAR:
      if (mt_get_type(cp->operand.cmp.mt) != lCharT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBECHART);
/*       DPRINTF(("(*wapp)->value.c = %c\n", (*wapp)->value.c)); */
      cp->operand.cmp.val.c = (*wapp)++->value.c;
      break;

   case BOOL:
      if (mt_get_type(cp->operand.cmp.mt) != lBoolT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEBOOL);
/*       DPRINTF(("(*wapp)->value.b = %c\n", (*wapp)->value.b)); */
      cp->operand.cmp.val.b = (*wapp)++->value.b;
      break;

   case REF:
      if (mt_get_type(cp->operand.cmp.mt) != lRefT)
         incompatibleType(MSG_CULL_WHERE_SHOULDBEREFT);
/*       DPRINTF(("(*wapp)->value.ref = %p\n", (*wapp)->value.ref)); */
      cp->operand.cmp.val.ref = (*wapp)++->value.ref;
      break;

   default:
      unknownType("_lWhere");
      DRETURN(NULL);
   }

   /* 
      "m=" as operator is not allowed with other types than lUlongT
      "c=" as operator is not allowed with other types than lStringT
      "p=" as operator is not allowed with other types than lStringT
      "h=" as operator is not allowed with other types than lStringT
    */
   if ((cp->op == BITMASK && mt_get_type(cp->operand.cmp.mt)     != lUlongT ) ||
       ( cp->op == BITMASK && mt_get_type(cp->operand.cmp.mt)     != lUlong64T ) ||
       ( (cp->op == STRCASECMP && mt_get_type(cp->operand.cmp.mt)  != lStringT) && (cp->op == STRCASECMP && mt_get_type(cp->operand.cmp.mt)  != lHostT) ) ||
       ( (cp->op == HOSTNAMECMP && mt_get_type(cp->operand.cmp.mt) != lStringT) && (cp->op == HOSTNAMECMP && mt_get_type(cp->operand.cmp.mt) != lHostT) ) ||
       ( (cp->op == PATTERNCMP && mt_get_type(cp->operand.cmp.mt)  != lStringT) && (cp->op == PATTERNCMP && mt_get_type(cp->operand.cmp.mt)  != lHostT) ))
      incompatibleType(MSG_CULL_WHERE_OPERANDHITNOTOPERATORERROR );

   eat_token(state);

   DRETURN(cp);
}

/****** cull/where/lFreeWhere() ***********************************************
*  NAME
*     lFreeWhere() -- Free a condition
*
*  SYNOPSIS
*     lFreeWhere(lCondition **cp) 
*
*  FUNCTION
*     Free a condition.
*
*  INPUTS
*     lCondition **cp - condition, will be set to NULL 
*
******************************************************************************/ 
void lFreeWhere(lCondition **cp) 
{
   DENTER(CULL_LAYER, "lFreeWhere");

   if (cp == NULL || *cp == NULL) {
      DRETURN_VOID;
   }

   switch ((*cp)->op) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case PATTERNCMP:
   case HOSTNAMECMP:
      if (mt_get_type((*cp)->operand.cmp.mt) == lStringT) {
         if ((*cp)->operand.cmp.val.str) {
            sge_free(&((*cp)->operand.cmp.val.str));
         }
      }
      if (mt_get_type((*cp)->operand.cmp.mt) == lHostT) {
         if ((*cp)->operand.cmp.val.host) {
            sge_free(&((*cp)->operand.cmp.val.host));
         }
      }
      if (mt_get_type((*cp)->operand.cmp.mt) == lListT) {
         lFreeWhere(&((*cp)->operand.cmp.val.cp));
      }
      break;
   case SUBSCOPE:
      if (mt_get_type((*cp)->operand.cmp.mt) == lListT) {
         lFreeWhere(&((*cp)->operand.cmp.val.cp));
      }
      break;
   case AND:
   case OR:
      lFreeWhere(&((*cp)->operand.log.first));
      lFreeWhere(&((*cp)->operand.log.second));
      break;
   case NEG:
      lFreeWhere(&((*cp)->operand.log.first));
      break;

   default:
      LERROR(LEOPUNKNOWN);
   }

   sge_free(cp);
   DRETURN_VOID;
}

/****** cull/where/lCompare() *************************************************
*  NAME
*     lCompare() -- Decide if a element suffices a condition 
*
*  SYNOPSIS
*     int lCompare(const lListElem *ep, const lCondition *cp) 
*
*  FUNCTION
*     Decide if a element suffices a condition.
*
*  INPUTS
*     const lListElem *ep  - element 
*     const lCondition *cp - condition 
*
*  RESULT
*     int - result
*         0 - false
*         1 - true 
******************************************************************************/
int lCompare(const lListElem *ep, const lCondition *cp) 
{
   int result = 0;
   const char *str1, *str2;

   DENTER(CULL_LAYER, "lCompare");

   if (!ep) {
      LERROR(LEELEMNULL);
      DRETURN(0);
   }

   /* no conditions ok */
   if (!cp) {
      DRETURN(1);
   }

   switch (cp->op) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case SUBSCOPE:

      switch (mt_get_type(cp->operand.cmp.mt)) {
      case lIntT:
         result = intcmp(lGetPosInt(ep, cp->operand.cmp.pos), cp->operand.cmp.val.i);
         break;
      case lStringT:
         if (!(str1 = lGetPosString(ep, cp->operand.cmp.pos))) {
            LERROR(LENULLSTRING);
            DPRINTF(("lGetPosString in lCompare\n"));
            DRETURN(0);
         }
         if (!(str2 = cp->operand.cmp.val.str)) {
            DPRINTF(("cp->operand.cmp.val.str in lCompare\n"));
            LERROR(LENULLSTRING);
            DRETURN(0);
         }
         result = strcmp(str1, str2);
         DPRINTF(("strcmp(%s, %s)(lStringT) = %d\n", str1, str2, result));
         break;
      case lHostT:
         if (!(str1 = lGetPosHost(ep, cp->operand.cmp.pos))) {
            LERROR(LENULLSTRING);
            DPRINTF(("lGetPosHost in lCompare\n"));
            DRETURN(0);
         }
         if (!(str2 = cp->operand.cmp.val.host)) {
            DPRINTF(("cp->operand.cmp.val.host in lCompare\n"));
            LERROR(LENULLSTRING);
            DRETURN(0);
         }
         result = strcmp(str1, str2);
         DPRINTF(("strcmp(%s, %s)(lhostT) = %d\n", str1, str2, result));
         break;

      case lUlongT:
         result = ulongcmp(lGetPosUlong(ep, cp->operand.cmp.pos), 
                           cp->operand.cmp.val.ul);
         break;
      case lUlong64T:
         result = ulong64cmp(lGetPosUlong64(ep, cp->operand.cmp.pos), 
                           cp->operand.cmp.val.ul64);
         break;
      case lListT:
         result = (lFindFirst(lGetPosList(ep, cp->operand.cmp.pos), 
                              cp->operand.cmp.val.cp) != NULL);
         DRETURN(result);
      case lFloatT:
         result = floatcmp(lGetPosFloat(ep, cp->operand.cmp.pos), 
                           cp->operand.cmp.val.fl);
         break;
      case lDoubleT:
         result = doublecmp(lGetPosDouble(ep, cp->operand.cmp.pos), 
                            cp->operand.cmp.val.db);
         break;
      case lLongT:
         result = longcmp(lGetPosLong(ep, cp->operand.cmp.pos), 
                          cp->operand.cmp.val.l);
         break;
      case lCharT:
         result = charcmp(lGetPosChar(ep, cp->operand.cmp.pos), 
                          cp->operand.cmp.val.c);
         break;
      case lBoolT:
         result = boolcmp(lGetPosBool(ep, cp->operand.cmp.pos), 
                          cp->operand.cmp.val.b);
         break;
      case lRefT:
         result = refcmp(lGetPosRef(ep, cp->operand.cmp.pos), 
                         cp->operand.cmp.val.ref);
         break;
      default:
         unknownType("lCompare");
         DRETURN(0);
      }

      switch (cp->op) {
      case EQUAL:
         result = (result == 0);
         break;
      case NOT_EQUAL:
         result = (result != 0);
         break;
      case LOWER_EQUAL:
         result = (result == -1 || result == 0);
         break;
      case LOWER:
         result = (result == -1);
         break;
      case GREATER_EQUAL:
         result = (result == 1 || result == 0);
         break;
      case GREATER:
         result = (result == 1);
         break;
      default:
         LERROR(LEOPUNKNOWN);
         DRETURN(0);
      }
      break;

   case STRCASECMP:
   case HOSTNAMECMP:
      if ((mt_get_type(cp->operand.cmp.mt) != lStringT) && (mt_get_type(cp->operand.cmp.mt) != lHostT)) {
         unknownType("lCompare");
         DRETURN(0);
      }

      if (mt_get_type(cp->operand.cmp.mt) == lStringT) {
         str1 = lGetPosString(ep, cp->operand.cmp.pos);
      } else {
         str1 = lGetPosHost(ep, cp->operand.cmp.pos);
      }
      if (str1 == NULL) {
          LERROR(LENULLSTRING);
          DPRINTF(("lGetPosString in lCompare\n"));
          DRETURN(0);
      }

      if (!(str2 = cp->operand.cmp.val.str)) {
         DPRINTF(("cp->operand.cmp.val.str in lCompare\n"));
         LERROR(LENULLSTRING);
         DRETURN(0);
      }

      if (cp->op == STRCASECMP ) {
         result = SGE_STRCASECMP(str1, str2);
      } else {
         result = sge_hostcmp(str1, str2);
      }
      result = (result == 0);
      break;

   case PATTERNCMP:
      if ((mt_get_type(cp->operand.cmp.mt) != lStringT) && (mt_get_type(cp->operand.cmp.mt) != lHostT)) {
         unknownType("lCompare");
         DRETURN(0);
      }

      if (mt_get_type(cp->operand.cmp.mt) == lStringT) {
         if (!(str1 = lGetPosString(ep, cp->operand.cmp.pos))) {
            str1 = "";
         }

         if (!(str2 = cp->operand.cmp.val.str)) {
            DPRINTF(("cp->operand.cmp.val.str in lCompare\n"));
            LERROR(LENULLSTRING);
            DRETURN(0);
         }
      } else {
         if (!(str1 = lGetPosHost(ep, cp->operand.cmp.pos))) {
            str1 = "";
         }

         if (!(str2 = cp->operand.cmp.val.host)) {
            DPRINTF(("cp->operand.cmp.val.host in lCompare\n"));
            LERROR(LENULLSTRING);
            DRETURN(0);
         }
      }

      result = !fnmatch(str2, str1, 0);
      break;


   case BITMASK:
      if (mt_get_type(cp->operand.cmp.mt) != lUlongT) {
         unknownType("lCompare");
         DRETURN(0);
      }
      result = bitmaskcmp(lGetPosUlong(ep, cp->operand.cmp.pos), 
                          cp->operand.cmp.val.ul);
      break;

   case AND:
      if (!lCompare(ep, cp->operand.log.first)) {
         result = 0;
         break;
      }
      result = lCompare(ep, cp->operand.log.second);
      break;

   case OR:
      if (lCompare(ep, cp->operand.log.first)) {
         result = 1;
         break;
      }
      result = lCompare(ep, cp->operand.log.second);
      break;

   case NEG:
      result = !lCompare(ep, cp->operand.log.first);
      break;

   default:
      DPRINTF(("lCompare(): unknown operator %d\n", cp->op));
      DEXIT;
      exit(-1);

   }
   DRETURN(result);
}

/****** cull/where/lCopyWhere() ***********************************************
*  NAME
*     lCopyWhere() -- Copy a condition
*
*  SYNOPSIS
*     lCondition* lCopyWhere(const lCondition *cp) 
*
*  FUNCTION
*     Copy a condition.
*
*  INPUTS
*     const lCondition *cp - condition 
*
*  RESULT
*     lCondition* - Copy of 'cp'
******************************************************************************/
lCondition *lCopyWhere(const lCondition *cp) 
{

   lCondition *new = NULL;

   DENTER(CULL_LAYER, "lCopyWhere");

   if (!cp) {
      DRETURN(NULL);
   }

   if (!(new = (lCondition *) calloc(1, sizeof(lCondition)))) {
      LERROR(LEMALLOC);
      DRETURN(NULL);
   }

   new->op = cp->op;

   switch (cp->op) {
   case EQUAL:
   case NOT_EQUAL:
   case LOWER_EQUAL:
   case LOWER:
   case GREATER_EQUAL:
   case GREATER:
   case BITMASK:
   case STRCASECMP:
   case HOSTNAMECMP:
   case PATTERNCMP:
      new->operand.cmp.pos = cp->operand.cmp.pos;
      new->operand.cmp.mt = cp->operand.cmp.mt;
      new->operand.cmp.nm = cp->operand.cmp.nm;

      switch (mt_get_type(cp->operand.cmp.mt)) {
      case lIntT:
         new->operand.cmp.val.i = cp->operand.cmp.val.i;
         break;
      case lUlongT:
         new->operand.cmp.val.ul = cp->operand.cmp.val.ul;
         break;
      case lUlong64T:
         new->operand.cmp.val.ul64 = cp->operand.cmp.val.ul64;
         break;
      case lStringT:
         new->operand.cmp.val.str = strdup(cp->operand.cmp.val.str);
         break;
      case lHostT:
         new->operand.cmp.val.host = strdup(cp->operand.cmp.val.host);
         break;
      case lListT:
         break;
      case lObjectT:
         break;
      case lFloatT:
         new->operand.cmp.val.fl = cp->operand.cmp.val.fl;
         break;
      case lDoubleT:
         new->operand.cmp.val.db = cp->operand.cmp.val.db;
         break;
      case lLongT:
         new->operand.cmp.val.l = cp->operand.cmp.val.l;
         break;
      case lBoolT:
         new->operand.cmp.val.b = cp->operand.cmp.val.b;
         break;
      case lCharT:
         new->operand.cmp.val.c = cp->operand.cmp.val.c;
         break;
      case lRefT:
         break;
      default:
         unknownType("lCopyWhere");
         lFreeWhere(&new);
         DRETURN(NULL);
      }
   case SUBSCOPE:
      if (mt_get_type(cp->operand.cmp.mt) == lListT) {
         new->operand.cmp.pos = cp->operand.cmp.pos;
         new->operand.cmp.mt = cp->operand.cmp.mt;
         new->operand.cmp.nm = cp->operand.cmp.nm;
         new->operand.cmp.val.cp = lCopyWhere(cp->operand.cmp.val.cp);
      }
      break;
   case AND:
   case OR:
      new->operand.log.second = lCopyWhere(cp->operand.log.second);
   case NEG:
      new->operand.log.first = lCopyWhere(cp->operand.log.first);
      break;

   default:
      LERROR(LEOPUNKNOWN);
      lFreeWhere(&new);
      DRETURN(NULL);
   }

   DRETURN(new);
}
