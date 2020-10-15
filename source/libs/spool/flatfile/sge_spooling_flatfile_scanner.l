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
/* scanner for Grid Engine ascii spool format
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! when you change this file, remove sge_spooling_flatfile_scanner.c !!
 * !! before building Grid Engine                                       !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

%option noyywrap
%pointer

%{
/* need this for the system calls below */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

#include "spool/flatfile/sge_spooling_flatfile_scanner.h"
#include "basis_types.h"
#include "uti/sge_log.h"

/* Internal function declarations */
static void remove_markers(void);
static void remove_backslash(void);
static void remove_newline(void);

/* We define SGE_FUNC so that we can use SGE_ASSERT. */
#define SGE_FUNC "spool_lex"
#define YY_BREAK
#ifndef YYLMAX
#  define YYLMAX MAX_STRING_SIZE
#endif

int spool_line;
int spool_return_whitespace;
int spool_finish_line;
static char* tmp_str = NULL;

int spool_scanner_initialize(FILE *input)
{
   spool_line = 1;
   spool_return_whitespace = 0;
   spool_finish_line = 0;
   spool_restart(input);

   return spool_lex();
}

void spool_scanner_shutdown(void)
{
}

static void remove_markers(void)
{
   tmp_str = (char *)strchr(spool_text, '\xFF');

   /* Find continuation markers. */
   while (tmp_str != NULL) {
      char *rw = tmp_str;
      char *ff = tmp_str + 1;
      bool add_space = false;

      /* Replace the marker with a space */
      *tmp_str = ' ';

      /* Find previous non-whitespace character. */
      while (rw != spool_text) {
         if (!isspace(*rw)) {
            if (*rw != ',') {
               add_space = true;
            }

            break;
         }

         rw--;
      }

      /* Find next non-whitespace character. */
      while (*ff != '\0') {
         if (!isspace(*ff)) {
            break;
         }

         ff++;
      }

      /* Advance to first whitespace character. */
      if (rw != spool_text) {
         rw++;
      }

      /* Add a space if needed. */
      if (add_space) {
         *rw = ' ';
         rw++;
      }

      /* Overwrite the whitespace with the remainder
         of the string. */
      while (*ff != '\0') {
         *rw = *ff;
         rw++;
         ff++;
      }

      /* Terminate the new string. */
      *rw = '\0';

      /* Look for the next marker. */
      tmp_str = (char *)strchr(spool_text, '\xFF');
   }
}

static void remove_backslash(void)
{
   /* Replace the blackslash with a marker. */
   tmp_str = (char *)strrchr(spool_text, '\\');

   /* VERY BAD */
   SGE_ASSERT(tmp_str != NULL);

   *tmp_str = '\xFF';
}

static void remove_newline(void)
{
   /* Replace the newline with a space. */
   tmp_str = (char *)strrchr(spool_text, '\n');

   /* VERY BAD */
   SGE_ASSERT(tmp_str != NULL);

   *tmp_str = ' ';
}

%}

DIGIT    "-"?[0-9]
ID       [a-zA-Z0-9_/.#$\-]*
COLON    :
DELIMITER [,;:|={}[\]]
COMPOP   [=<>!]
NL       \r?\n

%x finish_line

%%
   if (spool_finish_line) {
      BEGIN(finish_line);
   }
   else  {
      BEGIN(INITIAL);
   }

{DIGIT}+{COLON}{DIGIT}+{COLON}{DIGIT}+  return SPFT_TIME;

{DIGIT}+    return SPFT_INT; /* JG: TODO: handle negative */

{DIGIT}+"."{DIGIT}*        return SPFT_FLOAT; /* JG: TODO: handle negative */

{DIGIT}+"-"{DIGIT}+  return SPFT_RANGE;

{COMPOP}{COMPOP}  return SPFT_COMPOP;

{DELIMITER}      return SPFT_DELIMITER;

{ID}        return SPFT_WORD;

#[^\n]*{NL}   spool_line++; break;        /* eat up comments */

^[ \t\r]*{NL}   spool_line++; break;      /* Ignore blank lines */

 /* Remove line breaks */
[ \t\r]*[\\][ \t\r]*{NL}[ \t\r]* {
   unput(' ');
   spool_line++;
   break;
}

[,][ \t\r]*[\\][ \t\r]*{NL}[ \t\r]* {
   unput(',');
   spool_line++;
   break;
}


 /* In order to make sure that whitespace before the newline gets removed, we
  * need these two rules.  Just using unput() doesn't work because it could
  * change the token order. */
 /* Newlines. */
{NL} {
   spool_line++;
   return SPFT_NEWLINE;
} 

[ \t\r]*{NL} {
   sprintf(yytext, "\n");
   spool_line++;
   return SPFT_NEWLINE;
} 

 /* In order to make sure that whitespace gets compressed into a
  * single space, we need these three rules.  Just using unput() doesn't work
  * because that can result in the token order being changed. */
 /* Whitespace longer than 1 character. */
[ \t\r]{2,} { 
   unput(' ');
   break;
}

 /* Single whitespace that isn't a space. */
[\t\r] {
   unput(' ');
   break;
}

 /* Single whitespace that is a space. */
[ ] {
   if (spool_return_whitespace) {
      return SPFT_WHITESPACE;
   }

   break;
}

 /* Here's the algorithm continuable value fields.  If a line ends in backslash,
  * replace the backslash with \0xFF, replace the newline with a space, and call
  * yymore().  If a line ends normally, search the line for \0xFF.  Every time
  * one is found, remove all whitespace before and after it, up to the first
  * non-whitespace character.  If the first preceeding non-whitespace character
  * is not a comma, leave a space. */

 /* Match lines ending in backslash. */
<finish_line>[^\n]*[\\][ \t\r]*{NL} %{
   remove_backslash();
   remove_newline();
   spool_line++;

   /* Append the next line to the current line. */
   yymore ();
   break;
%}

 /* Match lines not ending in backslash. */
<finish_line>[^\n]*[ \t\r]*$ %{
   remove_markers();
   spool_line++;

   return SPFT_WORD;
%}

<*>.           return SPFT_UNKNOWN;
%%

#ifdef BUILD_FLATFILE_SCANNER_MAIN 
int main(int argc, char *argv[])
{
   int ret;

    ++argv, --argc;  /* skip over program name */
    if ( argc > 0 )
            spool_in = fopen( argv[0], "r" );
    else
            spool_in = stdin;

    while((ret = spool_lex())) {
      printf("line %3d: token %3d: %s\n", spool_line, ret, spool_text);
    }
}

#endif
