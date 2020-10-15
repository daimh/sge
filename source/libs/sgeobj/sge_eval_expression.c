/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2006
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
 *   Copyright: 2006 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <string.h>
#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_hostname.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_eval_expression.h"
#include "sgeobj/msg_sgeobjlib.h"

#include "msg_common.h"

/* Local variables and definitions  */
typedef struct _s_token {
   u_long32 type;       /* resource type, how to be compared */
   const char *value;   /* Pointer to tested value */
   const char *expr;    /* Pointer to tested expression */
   const char *s;       /* Index pointer to expression */
         char *pattern; /* regular expression subpattern */
   bool has_patterns;   /* pattern has patterns switch, to help be more effective */
   int  tt;             /* Type of token */
   int  et;             /* Expected Token type */
   lList **answer_list; /* Answer list, or null */
} s_token;


/* Private function definitions */
static int OrExpression(s_token *, bool);
static int AndExpression(s_token *, bool);
static int SimpleExpression(s_token *, bool);
static void NextToken(s_token *, bool);
static int Error(s_token *, int);
static int ParseTerminal(s_token *, bool);
static void ParseNonTerminal(s_token *, bool);
static int indexOfTerminal(const char );
static bool is_pattern(const char );
static int MatchPattern(s_token *, bool);
static void uncaseValue(s_token *,char *);

/* arrays and enums.  */
enum { T_NOT, T_OR, T_AND, T_BRACEOPEN, T_BRACECLOSE, T_END, T_EXP, T_ERROR };

/* ATTENTION! The order of TERMINALS and enumTypes have to match */
const int eTypes[] = {T_NOT, T_OR, T_AND, T_BRACEOPEN, T_BRACECLOSE, T_END};
char *tTypes[] = { "!<pattern>", "|<pattern>", "&<pattern>", "(", ")", "<end>",
"<pattern>", "<error>" };


/****** sgeobj/sge_eval_expression/sge_eval_expression() **********************
*  NAME
*     sge_eval_expression() -- boolean expression extension 
*
*  SYNOPSIS
*     int sge_eval_expression(u_long32 type, 
*                             const char *expr, 
*                             const char *value)
*
*  FUNCTION
*     boolean expression extension of regular expression evaluation function 
*     fnmatch()
*
*  INPUTS
*     u_long32 type - type of resource
*     const char *expr - expression string to be evaluated
*     const char *value - value string to be compared for
*     lList **answer_list - answer list to pass back error messages
*
*  RESULT
*     int - result if expression is true or
*         0 - strings are the same or both NULL
*         1 - if is false
*        -1 - for empty expression, null, or other error
*
*  SEE ALSO
*     uti/hostname/sge_hostcmp()
*     fnmatch()
*
*  NOTES:
*     MT-NOTE: sge_eval_expression() is MT safe
*****************************************************************************/
int 
sge_eval_expression(u_long32 type, const char *expr, const char *value, lList **answer_list) 
{
   int match;
   char pattern_buf[MAX_STRING_SIZE], value_buf[MAX_STRING_SIZE];
   
   DENTER(BASIS_LAYER, "sge_eval_expression");
   
   /* Null values are supported in str_cmp_null way */
   if (expr==NULL && value!=NULL) {
      DRETURN(-1);
   }              
   if (expr!=NULL && value==NULL) {
      DRETURN(1);
   }
   if (expr == NULL && value == NULL) {
      DRETURN(0);
   }

   /* To long arguments */
   if (strlen(value) >= MAX_STRING_SIZE) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_EVAL_EXPRESSION_LONG_VALUE, MAX_STRING_SIZE);
      ERROR((SGE_EVENT, MSG_EVAL_EXPRESSION_LONG_VALUE, MAX_STRING_SIZE));
      DRETURN(-1);
   }
   if (strlen(expr) >= MAX_STRING_SIZE) {
      answer_list_add_sprintf(answer_list, STATUS_ESYNTAX, ANSWER_QUALITY_ERROR,
                              MSG_EVAL_EXPRESSION_LONG_EXPRESSION, MAX_STRING_SIZE);
      ERROR((SGE_EVENT, MSG_EVAL_EXPRESSION_LONG_EXPRESSION, MAX_STRING_SIZE));
      DRETURN(-1);
   }   
  
   { 
	   s_token token;             /* The main token structure */
	   token.value=value;         /* the copybuffer will be on demand */
	   token.expr=expr;           /* copy per partes to pattern_buf */
	   token.pattern=pattern_buf;
	   token.s=token.expr;        /* Set the index. */
	   token.tt=T_END;            /* Type of token */
	   token.et=T_EXP;            /* Type of expected token */
	   token.type=type;
	   token.answer_list=answer_list;
	   token.has_patterns = sge_is_expression(token.expr); /*  pattern is detected latter */

	   if(token.has_patterns){
	      uncaseValue(&token,value_buf);   /* Check the value needs to be lowered */
	      match = OrExpression(&token, false);
	      /*
	       * after the expression has been evaluated
	       * the input stream must be empty
	       * and the token must be T_END
	       */
	      if (token.tt != T_END) {
            match=Error(&token, T_END);
	      } else if (token.s[0] != '\0') { /*Something is missing? */
            match=Error(&token, token.et);
	      }
	   } else {
         token.pattern=(char *) token.expr;
         match = MatchPattern(&token, false);
	   }
   }

   DRETURN(match);
}

/*-----------------------------------------------------------
 * NextTok[0]en
 *    set "s" to next token
 *    set "token" to T_NOT, T_AND, T_OR, T_BRACEOPEN, T_BRACECLOSE, T_EXP
 *    set "s" to NULL and "token" to T_END of string is completely read
 *-----------------------------------------------------------*/
static void NextToken(s_token *token_p, bool skip)
{
   token_p->et=token_p->tt;
   
   while (token_p->s[0] == ' ') token_p->s++; /*skip white chars */
   
   if (token_p->s == NULL ) return;    /* Should not happen */
   if (token_p->tt==T_ERROR) return;   /*Previous error */
   
   if (token_p->s[0] == '\0' ) {       /* At the end of expression */
      token_p->tt = T_END;
      return;
   }
   if (!ParseTerminal(token_p, skip)){
      ParseNonTerminal(token_p, skip);
   }
}


/*-----------------------------------------------------------
 * ParseTerminal
 * return 1 .. isTeminal, 0..isNonTerminal
 *-----------------------------------------------------------*/
static int ParseTerminal(s_token *token_p, bool skip)
{
   int index;
   if ((index=indexOfTerminal(token_p->s[0]))!=-1){
      token_p->tt=eTypes[index]; /*The order is same as enum */
      token_p->s++; /*Move pointer after terminal */
      return 1;
   }
   return 0;
}
/*-----------------------------------------------------------
 * ParseNonTerminal
 * Cat the pattern on next terminal and
 * move the s pointer together and stop on next terminal
 *-----------------------------------------------------------*/
static void ParseNonTerminal(s_token *token_p, bool skip)
{
   char *index;
   token_p->tt = T_EXP;
   if (skip==false) { /* expression is not in skip mode */
      token_p->has_patterns=false; /* the pattern detected */
      index = token_p->pattern; /* skip first test, allready tested */
      while (index==token_p->pattern || indexOfTerminal(token_p->s[0])==-1) {
         if (token_p->has_patterns==false && is_pattern(token_p->s[0])) {
            token_p->has_patterns=true; /* the pattern detected */
         }
         switch (token_p->type){
            case TYPE_CSTR:
            case TYPE_HOST:
               index[0]=tolower(token_p->s[0]);
               break;
            default:
               index[0]=token_p->s[0];
         }
         
         index++;
         token_p->s++;
      }
      index[0]='\0'; /*terminate pattern copy */
   } else {
      while (indexOfTerminal(token_p->s[0])==-1) {
         token_p->s++;
      }
   }
}

/*-----------------------------------------------------------
 * indexOfTerminal
 *    return index of terminal, -1 if it is not terminal
 *-----------------------------------------------------------*/
static int indexOfTerminal(const char c)
{
   switch (c) {
      case '!': return 0;
      case '|': return 1;
      case '&': return 2;
      case '(': return 3;
      case ')': return 4;
      case ' ': return 5;
      case '\0': return 5;
   }
   return -1;
}

/*-----------------------------------------------------------
 * is_pattern
 *    return bool is there is an pattern sign
 *-----------------------------------------------------------*/
static bool is_pattern(const char c)
{
   switch (c) {
      case '*':
      case '?':
      case '[':
      case ']':
         return true;
   }
   return false;
}
/*-----------------------------------------------------------
 * Error
 *    print simple error message
 *-----------------------------------------------------------*/
static int Error(s_token *token_p, int expected)
{
   DENTER(GUI_LAYER, "sge_eval_expression:Error");
   if (token_p->tt!=T_ERROR){
      answer_list_add_sprintf(token_p->answer_list, STATUS_ESYNTAX,
                              ANSWER_QUALITY_ERROR, MSG_EVAL_EXPRESSION_PARSE_ERROR,
                              token_p->s - token_p->expr , token_p->expr);
      ERROR((SGE_EVENT, MSG_EVAL_EXPRESSION_PARSE_ERROR, (int)(token_p->s - token_p->expr) , token_p->expr));
      token_p->et=expected;
      token_p->tt=T_ERROR;
   }
   DRETURN((-1)); /*negative return value is error index */
}

/*----------------------------------------------------------
 * OrExpression
 * Evaluate an OR expression
 * same input/output parameters as for Expression()
 *----------------------------------------------------------*/
static int OrExpression(s_token *token_p, bool skip)
{
   int match;
   
   NextToken(token_p, skip);
   match = AndExpression(token_p, skip);
   while (token_p->tt == T_OR) {
      NextToken(token_p, skip); /* Negative logic 0=true,  or is and */
      if (match==0) {
         AndExpression(token_p, true); /* skip other for true */
      } else {
         match = AndExpression(token_p, skip);
      }
   }
   return match;
}

/*----------------------------------------------------------
 * AndExpression
 * Evaluate an AND expression
 *----------------------------------------------------------*/
static int AndExpression(s_token *token_p, bool skip)
{
   int match;
   
   match = SimpleExpression(token_p, skip);
   while (token_p->tt == T_AND) {
      NextToken(token_p, skip); /* Negative logic 0=true,  or is and */
      if (match!=0) {
         SimpleExpression(token_p, true); /* skip other for false */
      } else {
         match = SimpleExpression(token_p, skip);
      }
   }
   return match;
}

/*----------------------------------------------------------
 * SimpleExpression
 *    Evalute a simple expression
 *    this can be a "expression"
 *         1) we can match with fnmatch()
 *         2) !expression
 *         3) (exp)
 *----------------------------------------------------------*/
static int SimpleExpression(s_token *token_p, bool skip)
{
   int match;
   if (token_p->tt==T_ERROR) {
      match = -1;
   } else if (token_p->tt == T_BRACEOPEN) {
      match = OrExpression(token_p, skip);
      if (token_p->tt != T_BRACECLOSE) {
         match = Error(token_p, T_BRACECLOSE); /*Indicate what I need? */
         return match;
      }
      NextToken(token_p, skip);
   } else if (token_p->tt == T_EXP)  {
      match=MatchPattern(token_p, skip);
      NextToken(token_p, skip);
   } else if (token_p->tt == T_NOT) {
      NextToken(token_p, skip);
      match = SimpleExpression(token_p, skip);
      match = !match;
   } else {
      match = Error(token_p, token_p->et);
   }
   return match;
}

/*----------------------------------------------------------
 * MatchPattern
 *    Evalute a pattern expression
 *    RETURNS match depend on type of resource
 *----------------------------------------------------------*/
static int MatchPattern(s_token *token_p, bool skip)
{
   int match;
   /*printf("Match pattern %i: '%s'=='%s'\n", skip,token_p->pattern, token_p->value); */
   if (skip==true){
      return -1;
   }
   if (token_p->pattern==NULL){
      return -1;
   }
   if (token_p->has_patterns ){
      switch(token_p->type){
         case TYPE_STR:
         case TYPE_CSTR:
         case TYPE_RESTR: match = fnmatch(token_p->pattern, token_p->value, 0);
         break;
         case TYPE_HOST:  match = sge_hostmatch(token_p->pattern, token_p->value);
         break;
         default: match = -1;
      }
   } else { /* optimized for non pattern stuff */
      switch(token_p->type){
         case TYPE_STR:
         case TYPE_RESTR:
            match = strcmp(token_p->pattern, token_p->value);
            break;
         case TYPE_CSTR:
            match = strcasecmp(token_p->pattern, token_p->value);
            break;
         case TYPE_HOST:
            match = sge_hostcmp(token_p->pattern, token_p->value);
            break;
         default: match = -1;
      }
   }
   return ((match==0) ? 0 : 1);
}

/*----------------------------------------------------------
 * uncaseValue
 *    Change token_p->value to case lowered value stored in buffer
 *----------------------------------------------------------*/
static void uncaseValue(s_token *token_p, char *value_buf)
{
   int i;
   switch (token_p->type){
      case TYPE_CSTR:
      case TYPE_HOST:
         for (i = 0; token_p->value[i] != '\0' && i < MAX_STRING_SIZE; i++) {
            value_buf[i]=tolower(token_p->value[i]);
         }
         value_buf[i]='\0'; /*Terminate string */
         token_p->value=value_buf; /* Set up the buffer */
         break;
   }
}
