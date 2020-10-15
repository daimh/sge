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
#include <string.h>
#include <ctype.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "uti/sge_rmon.h"

#include "cull/cull_listP.h"
#include "cull/cull_parse.h"
#include "cull/cull_lerrnoP.h"

/* ---------- static funcs --------------------------------- */
static const char *eatws(const char *s);
/* ---------- global variable --------------------------------- */

/* =========== implementation ================================= */

/* ------------------------------------------------------------ 

   sets pointer s to next character after
   white space character 

   returns NULL if there is '\0'

 */
static const char *eatws(
const char *s 
) {
   while (*s && isspace((int) *s))
      s++;
   return *s ? s : NULL;
}

/* ------------------------------------------------------------ 

   eat_token() informs parsing system that the current token 
   is proceed

 */
void eat_token(cull_parse_state *state) {
   state->token_is_valid = 0;
}

/* ------------------------------------------------------------ 

   scan() scans a string s for valid tokens 

   for getting the first token call scan with s = &string
   s != ÎNULL initializes scan()
   s == NULL uses the previous string for scanning
   if scan is called once again it gets the same token as before if there
   was no call to eat_token() otherwise a new token is delivered

 */
int scan(
const char *s,
cull_parse_state *state
) {
   static char *opv[] =
   {
      "%T",                     /* DESCRIPTOR TYPE OF LIST */
      "%I",                     /* FIELD                   */

      "->",                     /* SUBSCOPE                */
      "+",                      /* PLUS                    */
      "-",                      /* MINUS                   */
      "%d",                     /* INT                     */
      "%s",                     /* STRING                  */
      "%u",                     /* ULONG                   */
      "%lu",                    /* ULONG64                 */
      "%l",                     /* SUBLIST                 */
      "%f",                     /* FLOAT                   */
      "%g",                     /* DOUBLE                  */
      "%o",                     /* LONG                    */
      "%c",                     /* CHAR                    */
      "%b",                     /* BOOL                    */
      "%p",                     /* REF                     */

      "ALL",                    /* CULL_ALL                */
      "NONE",                   /* CULL_NONE               */
      "==",                     /* EQUAL                   */
      "!=",                     /* NOT_EQUAL               */
      "<=",                     /* LOWER_EQUAL             */
      "<",                      /* LOWER                   */
      ">=",                     /* GREATER_EQUAL           */
      ">",                      /* GREATER                 */
      "m=",                     /* BITMASK                 */
      "c=",                     /* STRCASECMP              */
      "p=",                     /* PATTERNCMP              */
      "h=",                     /* HOSTNAMECMP             */

      "&&",                     /* AND */
      "||",                     /* OR  */
      "!",                      /* NEG */
      "(",                      /* BRA */
      ")"                       /* KET */
   };

   int i, j, len;
   int n = sizeof(opv) / sizeof(char *);
   int found;

   DENTER(CULL_LAYER, "scan");

   if (s) {                     /* initialize scan() with a new string to parse */
      state->t = s;
      state->token_is_valid = 0;
   }

   if (state->token_is_valid) {        /* no need for a new token to parse */
      DEXIT;
      return state->token;
   }

   state->t = eatws(state->t);
   if (state->t == NULL) {       /* end of the string to parse */
      state->token_is_valid = 1;
      state->token = NO_TOKEN;
      DEXIT;
      return state->token;
   }

   /* try every possible token */
   for (i = 0; i < n; i++) {
      found = 1;
      len = strlen(opv[i]);
      for (j = 0; j < len; j++)
         if (state->t[j] == '\0' || state->t[j] != opv[i][j]) {
            found = 0;
            break;
         }
      if (found) {
         state->t += len;
         state->token_is_valid = 1;
         state->token = i + 1;

         DEXIT;
         return (state->token);
      }
   }

   state->token_is_valid = 1;
   state->token = NO_TOKEN;

   DEXIT;
   return state->token;
}
