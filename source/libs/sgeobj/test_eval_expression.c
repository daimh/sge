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

/*----------------------------------------------------
 *
 * The litte test program matches an "attribute" against
 * a regular exression which follows the above grammar and the
 * regular expression from the Grid Engine sge_types(1) man page
 * and prints "TRUE" or "FALSE"
 * For Expect test returns 0 .. TRUE
 *                         1 .. FALSE
 *                        -1 .. ERROR
 *
 *----------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_string.h"

#include "sgeobj/sge_feature.h"
#include "sgeobj/sge_eval_expression.h"

#define T 0
#define F 1
#define ERROR -1
#define RESULT(match) (match == -1) ? "ERROR" : (match == 0) ? "TRUE" : "FALSE"

/* Local functions and variables */
static int test_match(u_long32 , const char *, const char *, int );
static int test_tolower( char *, char *, int );

/*-----------------------------------------------------------
 * call:   test_eval_expression or test_eval_expression expr value
 *         return 0, if all test are ok
 *-----------------------------------------------------------*/
int main(int argc, char *argv[]) {
   int ret;
   
   DENTER_MAIN(TOP_LAYER, "test_evel_expression");
   bootstrap_mt_init();
   feature_mt_init();

   ret = 0;
   if(argc!=3){
      ret=ret|test_tolower("TESTDATA*[]dda","TESTDATA*[]dda",T);
      ret=ret|test_tolower("TESTDATA*[]dda","testdata*[]dda",T);
      ret=ret|test_tolower("TESTDATA*[]dda","TESTDATA*[]DDA",T);

      /* Basic Tests */
      /* 1  a & b */
      ret=ret|test_match(TYPE_STR, "a & b", "a", F);
      ret=ret|test_match(TYPE_STR, "a & b", "b", F);
      ret=ret|test_match(TYPE_STR, "a* & b*", "a", F);
      ret=ret|test_match(TYPE_STR, "a* & b*", "b", F);
      /* 2  a & !b */
      ret=ret|test_match(TYPE_STR, "a & !b", "a", T);
      ret=ret|test_match(TYPE_STR, "a & !b", "b", F);
      ret=ret|test_match(TYPE_STR, "a* & !b*", "a", T);
      ret=ret|test_match(TYPE_STR, "a* & !b*", "b", F);
      /* 3  a*/
      ret=ret|test_match(TYPE_STR, "a", "a", T);
      ret=ret|test_match(TYPE_STR, "a*", "a", T);
      /* 4 !a & b */
      ret=ret|test_match(TYPE_STR, "!a & b", "a", F);
      ret=ret|test_match(TYPE_STR, "!a & b", "b", T);
      ret=ret|test_match(TYPE_STR, "!a* & b*", "a", F);
      ret=ret|test_match(TYPE_STR, "!a* & b*", "b", T);
      /* 6 (!a & b) | (a & !b) */
      ret=ret|test_match(TYPE_STR, "(!a & b) | (a & !b)", "a", T);
      ret=ret|test_match(TYPE_STR, "(!a & b) | (a & !b)", "b", T);
      ret=ret|test_match(TYPE_STR, "(!a* & b*) | (a* & !b*)", "a", T);
      ret=ret|test_match(TYPE_STR, "(!a* & b*) | (a* & !b*)", "b", T);
      /* 7 a | b */
      ret=ret|test_match(TYPE_STR, "a | b", "a", T);
      ret=ret|test_match(TYPE_STR, "a | b", "b", T);
      ret=ret|test_match(TYPE_STR, "a* | b*", "a", T);
      ret=ret|test_match(TYPE_STR, "a* | b*", "b", T);
      /* 8 !(a | b) */
      ret=ret|test_match(TYPE_STR, "!(a | b)", "a", F);
      ret=ret|test_match(TYPE_STR, "!(a | b)", "a", F);
      ret=ret|test_match(TYPE_STR, "!(a* | b*)", "a", F);
      ret=ret|test_match(TYPE_STR, "!(a* | b*)", "a", F);
      /* 9  (!a | b) & (a | !b) */
      ret=ret|test_match(TYPE_STR, "(!a | b) & (a | !b)", "a", F);
      ret=ret|test_match(TYPE_STR, "(!a | b) & (a | !b)", "b", F);
      ret=ret|test_match(TYPE_STR, "(!a* | b*) & (a* | !b*)", "a", F);
      ret=ret|test_match(TYPE_STR, "(!a* | b*) & (a* | !b*)", "b", F);
      /* 11  a | !b */
      ret=ret|test_match(TYPE_STR, "a | !b", "a", T);
      ret=ret|test_match(TYPE_STR, "a | !b", "b", F);
      ret=ret|test_match(TYPE_STR, "a* | !b*", "a", T);
      ret=ret|test_match(TYPE_STR, "a* | !b*", "b", F);
      /* 13  !a | b */
      ret=ret|test_match(TYPE_STR, "!a | b", "a", F);
      ret=ret|test_match(TYPE_STR, "!a | b", "b", T);
      ret=ret|test_match(TYPE_STR, "!a* | b*", "a", F);
      ret=ret|test_match(TYPE_STR, "!a* | b*", "b", T);
      /* 14  !(a & b) */
      ret=ret|test_match(TYPE_STR, "!(a & b)", "a", T);
      ret=ret|test_match(TYPE_STR, "!(a & b)", "b", T);
      ret=ret|test_match(TYPE_STR, "!(a* & b*)", "a", T);
      ret=ret|test_match(TYPE_STR, "!(a* & b*)", "b", T);
      
      /* Regular tests */
      ret=ret|test_match(TYPE_CSTR, "solaris", "solaris", T);
      ret=ret|test_match(TYPE_CSTR, "!solaris", "solaris", F);
      ret=ret|test_match(TYPE_CSTR, "*amd64&sol*", "sol-amd64", T);
      ret=ret|test_match(TYPE_CSTR, "(sol-*64|linux|hp*)&!sol-sparc", "hp11", T);
      ret=ret|test_match(TYPE_CSTR, "(sol-*64|linux|hp*)&!sol-sparc", "sol-sparc64", T);
      ret=ret|test_match(TYPE_CSTR, "(sol-*64|linux|hp*)&!sol-sparc", "sol-sparc", F);
      
      ret=ret|test_match(TYPE_CSTR, "!(sola*|lin*|hp*)&!sola*&!*sparc64&(!sole*|!lin*|!hp*)", "sol-sparc", T);
      
      ret=ret|test_match(TYPE_CSTR, "(((test)))", "test", T);
      ret=ret|test_match(TYPE_CSTR, "(((test)&pet*))", "test", F);
      /* Errors tests */
      ret=ret|test_match(TYPE_STR, "(sol-*64|linux|hp*)&!sol-sparc!&", "sol-sparc", ERROR);
      ret=ret|test_match(TYPE_STR, "a b c", "      ", F);
      ret=ret|test_match(TYPE_STR, "a|b c", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "a&", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "a|", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "a&a&", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "a|a|", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "(a b c", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "a)&b", "a", ERROR);
      ret=ret|test_match(TYPE_STR, "(a)&b)|c", "a", ERROR);
      /* test for case sensitive */

      ret=ret|test_match(TYPE_CSTR, "a", "A", T);
      ret=ret|test_match(TYPE_CSTR, "A", "a", T);
      ret=ret|test_match(TYPE_CSTR, "a*", "A", T);
      ret=ret|test_match(TYPE_CSTR, "A*", "a", T);
      ret=ret|test_match(TYPE_CSTR, "a&b|a", "A", T);
      ret=ret|test_match(TYPE_CSTR, "A&B|A", "a", T);
      ret=ret|test_match(TYPE_CSTR, "a*&b*|a*", "A", T);
      ret=ret|test_match(TYPE_CSTR, "A*&B*|A*", "a", T);
      /* test for host names */
      ret=ret|test_match(TYPE_HOST, "Latte*", "latte3.czech.sun.com", T);
      ret=ret|test_match(TYPE_HOST, "latte* & !*3.czech.sun.com", "latte3.czech.sun.com", F);
      ret=ret|test_match(TYPE_HOST, "Latte* | Mocca*", "latte3.czech.sun.com", T);
      ret=ret|test_match(TYPE_HOST, "!(a*|b*|c*|d*|e*|f*|g*|h*|i*|j*|k*|l*|m*|n*|o*|p*|q*|r*|s*|t*|u*|v*|w*|x*|y*|z*|baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa*)", "bla", F);

      
      fprintf(stdout, "For evaluation a single expression try: test_eval_expression <expr> <value> \n");
      fprintf(stdout, "Complex set of tests result is: %s \n", RESULT(ret));
   } else {
      ret=sge_eval_expression(TYPE_RESTR, argv[1], argv[2], NULL);
      fprintf(stdout, "eval_expr(%s,%s) => %s\n", argv[1], argv[2], RESULT(ret) );
   }
   
   DEXIT;
   return ret;
}

static int test_match(u_long32 type, const char *expression, const char *value, int expected) {
   int match;
   match = sge_eval_expression(type, expression, value, NULL);
   if(match!=expected) {
      fprintf(stderr, "!!!UNEXPECTED RESULT!!!: %s => eval_expr(%s,%s), expected: %s \n",
      RESULT(match) , expression, value, RESULT(expected) );
      return 1;
   } /* else {
    fprintf(stdout, "eval_expr(%s,%s) => %s\n", expression, value, RESULT(match) );
    }  */
   return 0;
}

static int test_tolower(char *expression, char *value, int expected) {
   int match;
   char *t1;
   char *t2;
   t1=strdup(expression);
   t2=strdup(value);
   sge_strtolower(t1,255);
   sge_strtolower(t2,255);
   match = strcmp(t1,t2);
   sge_free(&t1);
   sge_free(&t2);
   if(match!=expected) {
      fprintf(stderr, "!!!UNEXPECTED RESULT!!!: %s => strcmp(sge_strtolower(%s),sge_strtolower(%s)), expected: %s \n",
      RESULT(match) , expression, value, RESULT(expected) );
      return 1;
   } /* else {
    fprintf(stdout, "eval_expr(%s,%s) => %s\n", expression, value, RESULT(match) );
    }  */
   return 0;
}

