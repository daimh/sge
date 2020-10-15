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
 *   Copyright (C) 2012 Dave Love, University of Liverpool
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <fnmatch.h>
#include <errno.h>

#include "uti/sge_rmon.h"
#include "uti/sge_stdlib.h"
#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/msg_utilib.h"

#define IS_DELIMITOR(c,delimitor) \
   (delimitor?(strchr(delimitor, c)?1:0):isspace(c))

/****** uti/string/sge_basename() *********************************************
*  NAME
*     sge_basename() -- get basename for path
*
*  SYNOPSIS
*     char* sge_basename(const char *name, int delim) 
*
*  FUNCTION
*     Determines the basename for a path like string - the last field 
*     of a string where fields are separated by a fixed one character 
*     delimiter.
*
*  INPUTS
*     const char *name - contains the input string (path)
*     int delim        - delimiter
*
*  RESULT
*     char* - pointer to base of name after the last delimter
*             NULL if "name" is NULL or zero length string or 
*             delimiter is the last character in "name"
*
*  EXAMPLE
*     sge_basename("/usr/local/bin/flex", '/'); returns "flex"
*
*  NOTES
*     MT-NOTE: sge_basename() is MT safe
******************************************************************************/
const char *sge_basename(const char *name, int delim) 
{
   char *cp;

   DENTER(BASIS_LAYER, "sge_basename");

   if (!name) {
      DRETURN(NULL);
   }
   if (name[0] == '\0') {
      DRETURN(NULL);
   }
   
   cp = strrchr(name, delim);
   if (!cp) {
      DRETURN(name); 
   } else {
      cp++;
      if (*cp == '\0') {
         DRETURN(NULL);
      }
      else {
         DRETURN(cp);
      }
   }
}

/****** uti/string/sge_jobname() ***********************************************
*  NAME
*     sge_jobname() -- get jobname of command line string 
*
*  SYNOPSIS
*     const char* sge_jobname(const char *name) 
*
*  FUNCTION
*     Determine the jobname of a command line. The following definition is used
*     for the jobname:
*     - take everything up to the first semicolon
*     - take everything up to the first whitespace
*     - take the basename
*
*  INPUTS
*     const char *name - contains the input string (command line)
*
*  RESULT
*     const char* - pointer to the jobname
*                   NULL if name is NULL or only '\0'
*
*  EXAMPLE
*  Command line                       jobname
*  ----------------------------------------------
*  "cd /home/me/5five; hostname" --> cd
*  "/home/me/4Ujob"              --> 4Ujob (invalid, will be denied)
*  "cat /tmp/5five"              --> cat
*  "bla;blub"                    --> bla
*  "a b"                         --> a
*      
*
*  NOTES
*     MT-NOTE: sge_jobname() is not MT safe 
*
*  SEE ALSO
*     sge_basename()
*******************************************************************************/
const char *sge_jobname(const char *name) {

   const char *cp = NULL;
   
   DENTER(BASIS_LAYER, "sge_jobname");
   if (name && name[0] != '\0' ) {

      cp = sge_strtok(name, ";");
      cp = sge_strtok(cp, " ");
      cp = sge_basename(cp, '/');

   }

   DRETURN(cp);
}

/****** uti/string/sge_dirname() **********************************************
*  NAME
*     sge_dirname() -- Return first part of string up to deliminator 
*
*  SYNOPSIS
*     char* sge_dirname(const char *name, int delim) 
*
*  FUNCTION
*     The function will return a malloced string containing the first 
*     part of 'name' up to, but not including deliminator. NULL will 
*     be returned if 'name' is NULL or a zero length string or if 
*     'delim' is the first character in 'name' 
*
*  INPUTS
*     const char *name - string 
*     int delim        - deliminator 
*
*  RESULT
*     char* - malloced string
*
*  NOTES
*     This routine is called "dirname" in opposite to "basename"
*     but is mostly used to strip off the domainname of a FQDN     
*     MT-NOTE: sge_dirname() is MT safe
******************************************************************************/
char *sge_dirname(const char *name, int delim) 
{
   char *cp, *cp2;

   DENTER(BASIS_LAYER, "sge_dirname");

   if (!name) {
      DRETURN(NULL);
   }

   if (name[0] == '\0' || name[0] == delim) {
      DRETURN(NULL);
   }

   cp = strchr(name, delim);

   if (!cp) {                   /* no occurence of delim */
      cp2 = strdup(name);
      DRETURN(cp2);
   } else {
      if ((cp2 = malloc((cp - name) + 1)) == NULL) {
         DRETURN(NULL);
      } else {
         strncpy(cp2, name, cp - name);
         cp2[cp - name] = '\0';
         DRETURN(cp2);
      }
   }
}

/****** uti/string/sge_strtok() ***********************************************
*  NAME
*     sge_strtok() -- Replacement for strtok() 
*
*  SYNOPSIS
*     char* sge_strtok(const char *str, const char *delimiter)
*
*  FUNCTION
*     Replacement for strtok(). If no delimiter is given
*     isspace() is used.
*
*  INPUTS
*     const char *str       - string which should be tokenized 
*     const char *delimiter - delimiter string
*
*  RESULT
*     char* - first/next token of str.
*
*  NOTES
*     MT-NOTE: sge_strtok() is not MT safe, use sge_strtok_r() instead
*
*  SEE ALSO
*     uti/string/sge_strtok_r()     
******************************************************************************/
char *sge_strtok(const char *str, const char *delimitor) 
{
   char *cp;
   char *saved_cp;
   static char *static_cp = NULL;
   static char *static_str = NULL;
   static unsigned int alloc_len = 0;
   unsigned int n;
   bool done;

   DENTER(BASIS_LAYER, "sge_strtok");

   if (str) {
      n = strlen(str);
      if (static_str) {
         if (n > alloc_len) {
            /* need more memory */
            sge_free(&static_str);
            static_str = malloc(n + 1);
            alloc_len = n;
         }
      } else {
         static_str = malloc(n + 1);
         alloc_len = n;
      }
      strcpy(static_str, str);
      saved_cp = static_str;
   } else {
      saved_cp = static_cp;
   }

   /* seek first character which is not '\0' and not delimiter */
   done = false;
   while (!done) {

      /* found end of string */
      if (saved_cp == NULL || *saved_cp == '\0') {
         DRETURN(NULL);
      }

      /* eat white spaces */
      if (!IS_DELIMITOR((int) saved_cp[0], delimitor)) {
         done = true;
         break;
      }

      saved_cp++;
   }

   /* seek end of string given by '\0' or delimiter */
   cp = saved_cp;
   done = false;
   while (!done) {
      if (!cp[0]) {
         static_cp = cp;

         DRETURN(saved_cp);
      }

      /* test if we found a delimiter */
      if (IS_DELIMITOR((int) cp[0], delimitor)) {
         cp[0] = '\0';
         cp++;
         static_cp = cp;

         DRETURN(saved_cp);
      }
      cp++;
   }

   DRETURN(NULL);
}

/****** uti/string/sge_strlcat() ***********************************************
*  NAME
*     sge_strlcat() -- sge strlcat implementation
*
*  SYNOPSIS
*     size_t sge_strlcat(char *dst, const char *src, size_t dstsize)
*
*  FUNCTION
*     appends characters from from "src" to "dst" and terminates
*     the dst string with '\0'
*     Returns the size required for successfully completing the operation.
*
*  INPUTS
*     char *dst       - destination
*     const char *src - source string (must be '\0' terminated)
*     size_t dstsize  - size of destination string
*
*  RESULT
*     size_t - min{dstsize,strlen(dst)}+strlen(src)
*              this is the size required for successfully completing the strcat.
*
*  NOTES
*     MT-NOTE: sge_strlcat() is MT safe
*******************************************************************************/
size_t sge_strlcat(char *dst, const char *src, size_t dstsize) {
   size_t dst_idx = 0, src_idx = 0;

   /* no destination - do nothing */
   if (dst == NULL) {
      return 0;
   }

   /* no source or source empty - do nothing */
   if (src == NULL || src[0] == '\0') {
      return 0;
   }

   /* find end of dst */
   for (dst_idx = 0; (dst[dst_idx] != '\0') && (dst_idx < dstsize - 1); dst_idx++) {
      ;
   }

   /* copy until source ends or destination is full */
   for (src_idx = 0; (src[src_idx] != '\0') && (dst_idx < dstsize - 1); src_idx++, dst_idx++) {
      dst[dst_idx] = src[src_idx];
   }
   dst[dst_idx] = '\0';

   /* compute the space
    * actually consumed (if we could finish the strcat)
    * or would be needed to fully append all characters in src
    */
   while (src[src_idx++] != '\0') {
     dst_idx++;
   }
   dst_idx++; /* we need space for the trailing 0 byte */

   return dst_idx;
}

/****** uti/string/sge_strlcpy() ***********************************************
*  NAME
*     sge_strlcpy() -- sge strlcpy implementation
*
*  SYNOPSIS
*     size_t sge_strlcpy(char *dst, const char *src, size_t dstsize) 
*
*  FUNCTION
*     copies "dstsize"-1 characters from from "src" to "dst" and terminates
*     the src string with '\0'- Returns the size of the "src" string.
*
*  INPUTS
*     char *dst       - destination
*     const char *src - source string (must be '\0' terminated)
*     size_t dstsize  - size of destination string
*
*  RESULT
*     size_t - strlen of src, not dst !!!
*
*  NOTES
*     MT-NOTE: sge_strlcpy() is MT safe
*******************************************************************************/
size_t sge_strlcpy(char *dst, const char *src, size_t dstsize) {
   size_t index = 0;
   if (dst == NULL) {
      return 0;
   }
   if (src == NULL) {
      dst[0] = '\0';
      return 0;
   }
   for (index = 0; (src[index] != '\0') && (index < dstsize - 1); index++) {
      dst[index] = src[index];
   }
   dst[index] = '\0';
   while ( src[index] != '\0') {
     index++;
   }
   return index;
}

/****** uti/string/sge_strtok_r() *********************************************
*  NAME
*     sge_strtok_r() -- Reentrant version of strtok()
*
*  SYNOPSIS
*     char* sge_strtok_r(const char *str, const char *delimiter,
*                        struct saved_vars_s **context) 
*
*  FUNCTION
*     Reentrant version of sge_strtok. When 'str' is not NULL, 
*     '*context' has to be NULL. If 'str' is NULL, '*context'
*     must not be NULL. The caller is responsible for freeing 
*     '*context' with sge_free_saved_vars(). 
*     If no delimiter is given, isspace() is used.
*
*  INPUTS
*     const char *str               - str which should be tokenized 
*     const char *delimiter         - delimiter string
*     struct saved_vars_s **context - context
*
*  RESULT
*     char* - first/next token
*
*  SEE ALSO
*     uti/string/sge_strtok()     
*     uti/string/sge_free_saved_vars()
*
*  NOTES
*     MT-NOTE: sge_strtok_r() is MT safe
******************************************************************************/
char *sge_strtok_r(const char *str, const char *delimitor, 
                   struct saved_vars_s **context) 
{
   char *cp;
   char *saved_cp;
   struct saved_vars_s *saved;
   bool done;

   DENTER(BASIS_LAYER, "sge_strtok_r");

   if (str != NULL) {
      if (*context != NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_POINTER_INVALIDSTRTOKCALL));
      }
      *context = (struct saved_vars_s *)malloc(sizeof(struct saved_vars_s));
      memset(*context, 0, sizeof(struct saved_vars_s));
      saved = *context;

      saved->static_str = malloc(strlen(str) + 1);

      strcpy(saved->static_str, str);
      saved_cp = saved->static_str;
   } else {
      if (*context == NULL) {
         ERROR((SGE_EVENT, SFNMAX, MSG_POINTER_INVALIDSTRTOKCALL1));
         DRETURN(NULL);
      }
      saved = *context;
      saved_cp = saved->static_cp;
   }

   /* seek first character which is no '\0' and no delimiter */
   done = false;
   while (!done) {

      /* found end of string */
      if (saved_cp == NULL || *saved_cp == '\0') {
         DRETURN(NULL);
      }

      /* eat white spaces */
      if (!IS_DELIMITOR((int) saved_cp[0], delimitor)) {
         done = true;
         break;
      }

      saved_cp++;
   }

   /* seek end of string given by '\0' or delimiter */
   cp = saved_cp;
   done = false;
   while (!done) {
      if (!cp[0]) {
         saved->static_cp = cp;

         DRETURN(saved_cp);
      }

      /* test if we found a delimiter */
      if (IS_DELIMITOR((int) cp[0], delimitor)) {
         cp[0] = '\0';
         cp++;
         saved->static_cp = cp;

         DRETURN(saved_cp);
      }
      cp++;
   }

   DRETURN(NULL);
}

/****** uti/string/sge_free_saved_vars() **************************************
*  NAME
*     sge_free_saved_vars() -- Free 'context' of sge_strtok_r() 
*
*  SYNOPSIS
*     void sge_free_saved_vars(struct saved_vars_s *context) 
*
*  FUNCTION
*     Free 'context' of sge_strtok_r() 
*
*  INPUTS
*     struct saved_vars_s *context 
*
*  SEE ALSO
*     uti/string/sge_strtok_r() 
*
*  NOTES
*     MT-NOTE: sge_free_saved_vars() is MT safe
******************************************************************************/
void sge_free_saved_vars(struct saved_vars_s *context) 
{
   if (context) {
      if (context->static_str) {
         sge_free(&(context->static_str));
      }
      sge_free(&context);
   }
}

/****** uti/string/sge_strdup() ***********************************************
*  NAME
*     sge_strdup() -- Replacement for strdup() 
*
*  SYNOPSIS
*     char* sge_strdup(char *old, const char *s) 
*
*  FUNCTION
*     Duplicate string 's'. "Use" 'old' buffer.
*
*  INPUTS
*     char *old     - buffer (will be freed) 
*     const char *s - string 
*
*  RESULT
*     char* - malloced string
*
*  NOTES
*     MT-NOTE: sge_strdup() is MT safe
******************************************************************************/
char *sge_strdup(char *old, const char *s) 
{
   char *ret = NULL;

   /* 
    * target (old) and source (s) might point to the same object!
    * therefore free old only after the dup
    */
   if (s != NULL) {
      int n = strlen(s);
      ret = malloc(n + 1);
      if (ret != NULL) {
         strcpy(ret, s);
      }
   }

   /* free and NULL the old pointer */
   sge_free(&old);

   return ret;
}

/****** uti/string/sge_strip_blanks() *****************************************
*  NAME
*     sge_strip_blanks() -- Strip blanks from string 
*
*  SYNOPSIS
*     void sge_strip_blanks(char *str) 
*
*  FUNCTION
*     Strip all blanks contained in a string. The string is used
*     both as source and drain for the necessary copy operations.
*     The string is '\0' terminated afterwards. 
*
*  INPUTS
*     char *str - pointer to string to be condensed 
*
*  NOTES
*     MT-NOTE: sge_strip_blanks() is MT safe
******************************************************************************/
void sge_strip_blanks(char *str) 
{
   char *cp = str;

   DENTER(BASIS_LAYER, "sge_strip_blanks");

   if (!str) {
      DRETURN_VOID;
   }

   while (*str) {
      if (*str != ' ') {
         if (cp != str)
            *cp = *str;
         cp++;
      }
      str++;
   };
   *cp = '\0';

   DRETURN_VOID;
}

/****** uti/string/sge_strip_white_space_at_eol() ******************************
*  NAME
*     sge_strip_white_space_at_eol() -- truncate white space at EOL 
*
*  SYNOPSIS
*     void sge_strip_white_space_at_eol(char *str) 
*
*  FUNCTION
*     Truncate white space from the end of the string 
*
*  INPUTS
*     char *str - string to be modified 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_strip_white_space_at_eol() is MT safe 
*******************************************************************************/
void sge_strip_white_space_at_eol(char *str) 
{
   DENTER(BASIS_LAYER, "sge_strip_white_space_at_eol");

   if (str != NULL) {
      size_t length = strlen(str);

      while (str[length - 1] == ' ' || str[length - 1] == '\t') {
         str[length - 1] = '\0';
         length--;
      }
   }
   DRETURN_VOID;
}

/****** uti/string/sge_strip_slash_at_eol() ******************************
*  NAME
*     sge_strip_slash_at_eol() -- truncate slash at EOL 
*
*  SYNOPSIS
*     void sge_strip_slash_at_eol(char *str) 
*
*  FUNCTION
*     Truncate slash from the end of the string 
*
*  INPUTS
*     char *str - string to be modified 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: sge_strip_slash_at_eol() is MT safe 
*******************************************************************************/
void sge_strip_slash_at_eol(char *str) 
{
   DENTER(BASIS_LAYER, "sge_strip_slash_at_eol");

   if (str != NULL) {
      size_t length = strlen(str);

      while (str[length - 1] == '/') {
         str[length - 1] = '\0';
         length--;
      }
   }
   DRETURN_VOID;
}


/****** uti/string/sge_delim_str() *******************************************
*  NAME
*     sge_delim_str() -- Trunc. a str according to a delimiter set 
*
*  SYNOPSIS
*     char* sge_delim_str(char *str, char **delim_pos, 
*                         const char *delim) 
*
*  FUNCTION
*     Truncates a string according to a delimiter set. A copy of 
*     the string truncated according to the delimiter set will be 
*     returned.
*
*     ATTENTION: The user is responsible for freeing the allocated
*     memory outside this routine. If not enough space could be 
*     allocated, NULL is returned.
*
*  INPUTS
*     char *str         - string to be truncated 
*     char **delim_pos  - A placeholder for the delimiting position
*                         in str on exit. 
*                         If set on entry the position of the 
*                         delimiter in the input string 'str' is
*                         returned. If no delimiting character in
*                         string was found, the address of the
*                         closing '\0' in 'str' is returned.
*     const char *delim - string containing delimiter characters 
*
*  RESULT
*     char* - Truncated copy of 'str' (Has to be freed by the caller!)
*
*  NOTES
*     MT-NOTE: sge_delim_str() is MT safe
******************************************************************************/
char *sge_delim_str(char *str, char **delim_pos, const char *delim) 
{
   char *cp = NULL; 
   char *tstr = NULL;

   DENTER(BASIS_LAYER, "sge_delim_str");

   /* we want it non-destructive --> we need a copy of str */
   if ((tstr = strdup(str)) == NULL) {
      DRETURN(NULL);
   }

   /* walk through str to find a character contained in delim or a
    * closing \0
    */

   cp = tstr;
   while (*cp) {
      if (strchr(delim, (int) *cp))     /* found */
         break;
      cp++;
   }

   /* cp now either points to a closing \0 or to a delim character */
   if (*cp) {                    /* if it points to a delim character */
      *cp = '\0';                /* terminate str with a \0 */
   }
   if (delim_pos) {              /* give back delimiting position for name */
      *delim_pos = str + strlen(tstr);
   }
   /* delim_pos either points to the delimiter or the closing \0 in str */

   DRETURN(tstr);
}

/****** uti/string/sge_strnullcmp() *******************************************
*  NAME
*     sge_strnullcmp() -- Like strcmp() but honours NULL ptrs.
*
*  SYNOPSIS
*     int sge_strnullcmp(const char *a, const char *b) 
*
*  FUNCTION
*     Like strcmp() apart from the handling of NULL strings.
*     These are treated as being less than any not-NULL strings.
*     Important for sorting lists where NULL strings can occur.  
*
*  INPUTS
*     const char *a - 1st string 
*     const char *b - 2nd string 
*
*  RESULT
*     int - result
*         0 - strings are the same or both NULL 
*        -1 - a < b or a is NULL
*         1 - a > b or b is NULL
*
*  NOTES
*     MT-NOTE: sge_strnullcmp() is MT safe
******************************************************************************/
int sge_strnullcmp(const char *a, const char *b) 
{
   if (!a && b) {
      return -1;
   }
   if (a && !b) {
      return 1;
   }
   if (!a && !b) {
      return 0;
   }
   return strcmp(a, b);
}

/****** uti/string/sge_patternnullcmp() ****************************************
*  NAME
*     sge_patternnullcmp() -- like fnmatch 
*
*  SYNOPSIS
*     int sge_patternnullcmp(const char *str, const char *pattern) 
*
*  FUNCTION
*     Like fnmatch() apart from the handling of NULL strings.
*     These are treated as being less than any not-NULL strings.
*     Important for sorting lists where NULL strings can occur. 
*
*  INPUTS
*     const char *str     - string 
*     const char *pattern - pattern to match 
*
*  RESULT
*     int - result
*         0 - strings are the same or both NULL 
*        -1 - a < b or a is NULL
*         1 - a > b or b is NULL
*
*  NOTES
*   MT-NOTE: fnmatch uses static variables, not MT safe
*******************************************************************************/
int sge_patternnullcmp(const char *str, const char *pattern) 
{
   if (!str && pattern) {
      return -1;
   }
   if (str && !pattern) {
      return 1;
   }
   if (!str && !pattern) {
      return 0;
   }
   return fnmatch(pattern, str, 0);
}


/****** uti/string/sge_strnullcasecmp() ***************************************
*  NAME
*     sge_strnullcasecmp() -- Like strcasecmp() but honours NULL ptrs.
*
*  SYNOPSIS
*     int sge_strnullcasecmp(const char *a, const char *b) 
*
*  FUNCTION
*     Like strcasecmp() apart from the handling of NULL strings.
*     These are treated as being less than any not-NULL strings.
*     Important for sorting lists where NULL strings can occur.  
*
*  INPUTS
*     const char *a - 1st string 
*     const char *b - 2nd string 
*
*  RESULT
*     int - result
*         0 - strings are the same minus case or both NULL 
*        -1 - a < b or a is NULL
*         1 - a > b or b is NULL
*
*  NOTES
*     MT-NOTE: sge_strnullcasecmp() is MT safe
******************************************************************************/
int sge_strnullcasecmp(const char *a, const char *b) 
{
   if (!a && b)
      return -1;
   if (a && !b)
      return 1;
   if (!a && !b)
      return 0;
   return SGE_STRCASECMP(a, b);
}

/****** uti/string/sge_is_pattern() *******************************************
*  NAME
*     sge_is_pattern() -- Test if string contains  wildcard pattern
*
*  SYNOPSIS
*     int sge_is_pattern(const char *s)
*
*  FUNCTION
*     Check whether string 's' contains a wildcard pattern.
*
*  INPUTS
*     const char *s - string
*
*  RESULT
*     int - result
*         0 - no wildcard pattern
*         1 - it is a wildcard pattern
*
*  NOTES
*     MT-NOTE: sge_is_pattern() is MT safe
******************************************************************************/
bool sge_is_pattern(const char *s) 
{
   char c;
   while ((c = *s++)) {
      switch (c) {
         case '*':
         case '?':
         case '[':
         case ']':
         return true;
      }
   }
   return false;
}

/****** uti/string/sge_is_expression() *******************************************
*  NAME
*     sge_is_expression() -- Test if string contains expressions & wildcard pattern
*
*  SYNOPSIS
*     int sge_is_expression(const char *s)
*
*  FUNCTION
*     Check whether string 's' contains a expressions & a wildcard pattern.
*
*  INPUTS
*     const char *s - string
*
*  RESULT
*     int - result
*         0 - no wildcard pattern
*         1 - it is a wildcard pattern
*
*  NOTES
*     MT-NOTE: sge_is_expression() is MT safe
******************************************************************************/
bool sge_is_expression(const char *s) 
{
   char c;
   
   if (s != NULL) {
      while (*s != '\0') {
         c = *s;
         switch (c) {
            case '*':
            case '?':
            case '[':
            case ']':
            case '&':
            case '|':
            case '!':
            case '(':
            case ')':
           return true;
         }
         s++;
      }
   }
   return false;
}


/****** uti/string/sge_strisint() *********************************************
*  NAME
*     sge_strisint() -- Is string a integer value in characters?
*
*  SYNOPSIS
*     int sge_strisint(const char *str) 
*
*  FUNCTION
*     May we convert 'str' to int? 
*
*  INPUTS
*     const char *str - string 
*
*  RESULT
*     int - result
*         0 - It is no integer
*         1 - It is a integer
*
*  NOTES
*     MT-NOTE: sge_strisint() is MT safe
*
******************************************************************************/
int sge_strisint(const char *str) 
{
   const char *cp = str;
 
   while (*cp) {
      if (!isdigit((int) *cp++)) {
         return 0;
      }
   }
   return 1;
}            

/****** uti/string/sge_strtoupper() *******************************************
*  NAME
*     sge_strtoupper() -- Convert the first n to upper case 
*
*  SYNOPSIS
*     void sge_strtoupper(char *buffer, int max_len) 
*
*  FUNCTION
*     Convert the first 'max_len' characters to upper case. 
*
*  INPUTS
*     char *buffer - string 
*     int max_len  - number of chars 
*
*  NOTES
*     MT-NOTE: sge_strtoupper() is MT safe
******************************************************************************/
void sge_strtoupper(char *buffer, int max_len) 
{
   DENTER(BASIS_LAYER, "sge_strtoupper");

   if (buffer != NULL) {
      int i;
      int length = MIN(strlen(buffer), max_len);
      for (i = 0; i < length; i++) {
         buffer[i] = toupper(buffer[i]); 
      }
   }
   DRETURN_VOID;
} 

/****** uti/hostname/sge_strtolower() ********************************************
*  NAME
*     sge_strtolower() -- convert all upper character in the string to lower case
*
*  SYNOPSIS
*     int sge_strtolower(char *buffer)
*
*  FUNCTION
*     sge_strtolower() for hostnames. Honours some configuration values:
*
*  INPUTS
*     char *buffer - string to be lowered
*
*  RESULT
*     no result, this function modify the argument string
*
*  SEE ALSO
*     uti/string/sge_strtoupper()
*
*  NOTES:
*     MT-NOTE: sge_strtolower() is MT safe
******************************************************************************/
void sge_strtolower(char *buffer, int max_len)
{
   DENTER(BASIS_LAYER, "sge_strtolower");
   if (buffer != NULL) {
      int i;
      for(i=0;buffer[i]!='\0' && i<max_len ;i++){
         buffer[i]=tolower(buffer[i]);
      }
   }
   DRETURN_VOID;
}

/****** uti/string/sge_stradup() **********************************************
*  NAME
*     sge_stradup() -- Duplicate array of strings
*
*  SYNOPSIS
*     char** sge_stradup(char **cpp, int n) 
*
*  FUNCTION
*     Copy list of character pointers including the strings these
*     pointers refer to. If 'n' is 0 strings are '\0'-delimited, if 
*     'n' is not 0 we use n as length of the strings. 
*
*  INPUTS
*     char **cpp - pointer to array of strings 
*     int n      - '\0' terminated? 
*
*  RESULT
*     char** - copy of 'cpp'
*
*  NOTES
*     MT-NOTE: sge_stradup() is MT safe
******************************************************************************/
char **sge_stradup(char **cpp, int n)
{
   int count = 0, len;
   char **cpp1, **cpp2, **cpp3;
 
   /* first count entries */
   cpp2 = cpp;
   while (*cpp2) {
      cpp2++;
      count++;
   }
 
   /* alloc space */
   cpp1 = (char **) malloc((count + 1) * sizeof(char **));
   if (!cpp1)
      return NULL;
 
   /* copy  */
   cpp2 = cpp;
   cpp3 = cpp1;
   while (*cpp2) {
      /* alloc space */
      if (n)
         len = n;
      else
         len = strlen(*cpp2) + 1;
 
      *cpp3 = (char *) malloc(len);
      if (!(*cpp3)) {
         while ((--cpp3) >= cpp1) {
            sge_free(cpp3);
         }
         sge_free(&cpp1);
         return NULL;
      }
 
      /* copy string */
      memcpy(*cpp3, *cpp2, len);
      cpp3++;
      cpp2++;
   }
 
   *cpp3 = NULL;
 
   return cpp1;
}  

/****** uti/string/sge_strafree() *********************************************
*  NAME
*     sge_strafree() -- Free list of character pointers 
*
*  SYNOPSIS
*     void sge_strafree(char **cpp) 
*
*  FUNCTION
*     Free list of character pointers 
*
*  INPUTS
*     char ***cpp - Pointer to array of string pointers 
*
*  NOTES
*     MT-NOTE: sge_strafree() is MT safe
******************************************************************************/
void sge_strafree(char ***cpp)
{
   if (cpp != NULL && *cpp != NULL) {
      char **cpp1 = *cpp;
    
      while (*cpp1 != NULL) {
         sge_free(cpp1);
         cpp1++;
      }
      sge_free(cpp);
   }
}

/****** uti/string/sge_stramemncpy() ******************************************
*  NAME
*     sge_stramemncpy() -- Find string in string array 
*
*  SYNOPSIS
*     char** sge_stramemncpy(const char *cp, char **cpp, int n) 
*
*  FUNCTION
*     Compare string with string field and return the pointer to
*     the matched character pointer. Compare exactly n chars 
*     case insensitive.
*
*  INPUTS
*     const char *cp - string to be found 
*     char **cpp     - pointer to array of strings 
*     int n          - number of chars 
*
*  NOTES:
*     MT-NOTE: sge_stramemncpy() is MT safe
*
*  RESULT
*     char** - NULL or pointer a string
*
*  NOTES
*     MT-NOTE: sge_stramemncpy() is MT safe
******************************************************************************/
char **sge_stramemncpy(const char *cp, char **cpp, int n)
{
   while (*cpp) {
      if (!memcmp(*cpp, cp, n)) {
         return cpp;
      }
      cpp++;
   }
   return NULL;
}     

/****** uti/string/sge_stracasecmp() ******************************************
*  NAME
*     sge_stracasecmp() -- Find string in string array 
*
*  SYNOPSIS
*     char** sge_stracasecmp(const char *cp, char **cpp) 
*
*  FUNCTION
*     Compare string with string field and return the pointer to
*     the matched character pointer. Compare case sensitive. 
*
*  INPUTS
*     const char *cp - string 
*     char **cpp     - pointer to array of strings  
*
*  RESULT
*     char** - NULL or pointer a string
*
*  NOTES
*     MT-NOTE: sge_stracasecmp() is MT safe
******************************************************************************/
char **sge_stracasecmp(const char *cp, char **cpp)
{
   while (*cpp) {
      if (!strcasecmp(*cpp, cp))
         return cpp;
      cpp++;
   }
   return NULL;
}   

#if unused
void
stra_printf(char *stra[])
{
   int i = 0;

   while (stra[i] != NULL) {
      fprintf(stdout, "%s\n", stra[i]);
      i++;
   }
}
#endif

/****** uti/string/stra_from_str() ********************************************
*  NAME
*     str_from_str() -- Extract valid qstat options/paramers from qstat profile.
*
*  SYNOPSIS
*     char **str_from_str(const char *source_str, const char *delim);
*
*  FUNCTION
*     Parse string 'source_str' based on delimeter(s) 'delim' and store
*     resulting tokens in string array 'ret'. Supports comment lines.
*
*  INPUTS
*     const char *source_str - File content of qstat profile as plain string.
*     const char *delim      - Sequence of characters used to identify tokens
*                              and parameters.
*
*  RESULT
*     char** - String array containing tokens found based on 'delim'.
*
*  NOTES
*     It is the caller's responsibilty to free dynamic memory allocated in this
*     routine.
*     MT-NOTE: stra_from_str() is MT safe.
*
*************************************************************************************/
char **
stra_from_str(const char *source_str, const char *delim)
{
   char **ret = NULL;

   if (source_str != NULL && delim != NULL) {
      struct saved_vars_s *context1;
      struct saved_vars_s *context2;
      const char *token_1 = NULL;
      const char *token_2 = NULL;
      int number_of_tokens = 0;
      int index = 0;

      /*
       * Support of comment lines and multiple options per line
       * in qstat profiles requires two level parsing. First
       * level works on a per line basis (delimiter `\n') and
       * sorts out lines starting with comment sign '#'.
       *
       * The result of this process is passed to second
       * level parsing which scans for options and parameters
       * per line. Delimiters are ' ', '\n' and '\t'.
       *
       * We basically need to do this twice: first to determine
       * the number of valid tokens and then to populate the
       * string array.
       *
       */
      /*
       * Count tokens.
       */
      context1 = NULL;
      token_1 = sge_strtok_r(source_str, "\n", &context1);
      while (token_1 != NULL) {
         if (token_1[0] != '#') {
            context2 = NULL;
            token_2 = sge_strtok_r(token_1, " \t", &context2);
            while (token_2 != NULL) {
               token_2 = sge_strtok_r(NULL, " \t", &context2);
               number_of_tokens++;
            }
            sge_free_saved_vars(context2);
         }
         token_1 = sge_strtok_r(NULL, "\n", &context1);
      }
      sge_free_saved_vars(context1);

      /*
       * Note that we need to proceed from here even if we got
       * no valid options/parameters. This is because caller
       * expects a zero entry string array with stopper as minimum.
       */

      /* malloc array memory */
      ret = (char **) malloc(sizeof(char*) * (number_of_tokens + 1));

      if (ret != NULL) {
         /*
          * Allocate and populate string array.
          */
         index = 0;
         context1 = NULL;
         token_1 = sge_strtok_r(source_str, "\n", &context1);
         while (token_1 != NULL) {
            if (token_1[0] != '#') {
               context2 = NULL;
               token_2 = sge_strtok_r(token_1, " \t", &context2);
               while (token_2 != NULL) {
                  ret[index] = strdup(token_2);
                  token_2 = sge_strtok_r(NULL, " \t", &context2);
                  index++;
               }
               sge_free_saved_vars(context2);
            }
            token_1 = sge_strtok_r(NULL, "\n", &context1);
         }
         sge_free_saved_vars(context1);
         ret[index] = NULL; /* Stopper */
      }
   }
   return ret;
}

/****** uti/string/sge_compress_slashes() *************************************
*  NAME
*     sge_compress_slashes() -- compresses sequences of slashes 
*
*  SYNOPSIS
*     void sge_compress_slashes(char *str) 
*
*  FUNCTION
*     Compresses sequences of slashes in str to one slash 
*
*  INPUTS
*     char *str - string (e.g. path) 
*
*  NOTES
*     MT-NOTE: sge_compress_slashes() is MT safe
*******************************************************************************/
void sge_compress_slashes(char *str)
{
   char *p;
   int compressed = 0;
   DENTER(BASIS_LAYER, "sge_compress_slashes");

   for (p = str; *p; p++) {
      while (*p == '/' && *(p+1) == '/') {
         compressed = 1;
         *p = '\0';
         p++;
      }
      if (compressed) {
         strcat(str, p);
         compressed = 0;
      }
   }
   DRETURN_VOID;
}

/****** uti/string/sge_strip_quotes() *****************************************
*  NAME
*     sge_strip_quotes() -- Strip quotes from string
*
*  SYNOPSIS
*     void sge_strip_quotes(char **pstr) 
*
*  FUNCTION
*     Strip quotes from "pstr". 
*
*  INPUTS
*     char **pstr - string to be modified 
*
*  NOTES
*     MT-NOTE: sge_strip_quotes() is MT safe
******************************************************************************/
void sge_strip_quotes(char **pstr) 
{
   char *cp = NULL;
   char *cp2 = NULL;

   DENTER(TOP_LAYER, "sge_strip_quotes");
   
   if (!pstr) {
      DRETURN_VOID;
   }
   
   for (; *pstr; pstr++) {
      for (cp2 = cp = *pstr; *cp; cp++) {
         if ((*cp != '"') && (*cp != '\'')) {
            *cp2++ = *cp;
         }
      }
      
      *cp2 = '\0';
   }
   
   DRETURN_VOID;
}

/*
** problem: modifies input string,
** this is the most frequently used mode
** but allocating extra memory (as it was in
** sge_string2list) should also be possible
** problem: default delimiters should be possible
** note: there is a similar cull function lString2List
*/
/*
** NAME
**   string_list
** PARAMETER
**   str       -    string to be parsed
**   delis     -    string containing delimiters
**   pstr      -    NULL or string array to return
** RETURN
**   NULL      -    error
**   char **   -    pointer to an array of strings containing
**                  the string list
** EXTERNAL
**
** DESCRIPTION
**
** NOTES
**     MT-NOTE: string_list() is MT safe
**
*/
char **string_list(char *str, char *delis, char **pstr) 
{
   unsigned int i = 0, j = 0;
   bool is_space = false;
   int found_first_quote = 0;
   char **head = NULL;
   bool done;

   DENTER(BASIS_LAYER, "string_list");

   if (str == NULL) {
      DRETURN(NULL);
   }

   /* skip heading delimiters */
   while (str[0] != '\0' && strchr(delis, str[0]) != NULL) {
      str++;
   }

   /* at str end: str either was an empty string or only contained delimiters */
   if (str[0] == '\0') {
      DRETURN(NULL);
   }

   /*
    * not more items than length of string is possible
    */
   if (pstr == NULL) {
      head = malloc((sizeof(void *)) * (strlen(str) + 1));
      if (head == NULL) {
         DRETURN(NULL);
      }
   } else {
      head = pstr;
   }

   done = false;
   while (!done) {
      while ((str[i] != '\0') && (strchr(delis, str[i]) != NULL)) {
         i++;
      }

      if (str[i] == '\0') {
         done = true;
         break;
      }

      head[j] = &str[i];
      j++;
      /*
      ** parse one string
      */
      is_space = false;
      
      while ((str[i] != '\0') && !is_space) {
         if ((found_first_quote == 0) && (str[i] == '"')) {
            found_first_quote = 2;
         } else if ((found_first_quote == 0) && (str[i] == '\'')) {
            found_first_quote = 1;
         }

         i++;

         /* If we're inside quotes, we don't count spaces. */
         if (found_first_quote == 0) {
            is_space = (bool)(strchr(delis, str[i]) != NULL);
         }
         
         if (((found_first_quote == 2) && (str[i] == '"')) ||
             ((found_first_quote == 1) && (str[i] == '\''))) {
            found_first_quote = 0;
         }
      }
      
      if (str[i] == '\0') {
         done = true;
         break;
      }

      str[i] = '\0';
      i++;
   }
   
   head[j] = NULL;

   DRETURN(head);
}

/****** uti/string/sge_strerror() **********************************************
*  NAME
*     sge_strerror() -- replacement for strerror
*
*  SYNOPSIS
*     const char* 
*     sge_strerror(int errnum) 
*
*  FUNCTION
*     Returns a string describing an error condition set by system 
*     calls (errno).
*
*     Wrapper arround strerror. Access to strerrror is serialized by the
*     use of a mutex variable to make strerror thread safe.
*
*  INPUTS
*     int errnum        - the errno to explain
*     dstring *buffer   - buffer into which the error message is written
*
*  RESULT
*     const char* - pointer to a string explaining errnum
*
*  NOTES
*     MT-NOTE: sge_strerror() is MT safe
*******************************************************************************/
const char *
sge_strerror(int errnum, dstring *buffer)
{
   static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
   const char *ret;

   pthread_mutex_lock(&mtx);
   ret = strerror(errnum);
   ret = sge_dstring_copy_string(buffer, ret);
   pthread_mutex_unlock(&mtx);

   return ret;
}

/****** uti/string/sge_str_is_number() *****************************************
*  NAME
*     sge_str_is_number() -- check if a string represents a number
*
*  SYNOPSIS
*     bool sge_str_is_number(const char *string)
*
*  FUNCTION
*     This function returns true if the given string represents a
*     floating point number.
*
*  INPUTS
*     const char *string - string to parse
*
*  RESULT
*     bool - result
*        true  - string represents a number
*        false - string is not a number
*
*  NOTES
*     MT-NOTE: sge_str_is_number() is MT safe
*  SEE ALSO
*     man strtod, man strol contains a C example on GNU/Linux
*******************************************************************************/
bool sge_str_is_number(const char *string)
{
   bool ret = true;
   char *end = NULL;
   double val;

   errno = 0;
   val = strtod(string, &end);

   if (errno == ERANGE) {
      if (val == 0) {
         /* underflow - TODO: do we count this as number? */
         ret = false;
      } else {
         /* overflow - TODO: do we count this as number? */
         ret = false;
      }
   }

   if (end == string) {
      /* no digits found */
      ret = false;
   } else if (*end != '\0') {
      /* additional characters after number found */
      ret = false;
   }

   return ret;
}

/****** uti/string/sge_replace_substring() *****************************************
*  NAME
*     sge_replace_substring - replace sub strings in a string
*
*  SYNOPSIS
*     const char *sge_replace_substring(const char *input, char *old, char *new)
*
*  FUNCTION
*     Replaces all occurences of old with new.
*     If old is part of the given string input, a new string is returned
*     where the replacement is done.
*
*  INPUTS
*     const char *input - the input string
*     const char *old   - the string to replace
*     const char *new   - the replacement string
*
*  RESULT
*     NULL, if the input string didn't contain the pattern,
*     else a newly allocated string containing the input string with replacements.
*
*  NOTES
*     MT-NOTE: sge_str_is_number() is MT safe 
*     It is the responsibility of the caller to free the returned string!
*
*******************************************************************************/
const char *sge_replace_substring(const char *input, const char *old, const char *new)
{
   int to_replace = 0;
   int change, new_len;
   char *new_string = NULL;
   char *return_string = NULL;
   char *source = NULL;
   char *source_string = NULL;
   char *tail = NULL;
   char *current_tail = NULL;

   /*
    * Basic sanity checks first.
    */
   if (input == NULL || old == NULL || new == NULL) {
      return NULL;
   }
   /*
    * Determine number for of substrings to replace. We are
    * careful NOT to overrun source string.
    */
   source = source_string = (char *)input;
   tail = source_string + strlen(source_string) - 1;
   while (source <= tail) {
      current_tail = source + strlen(old) - 1;
      if (current_tail > tail) {
         break;
      }
      if (memcmp(old, source, strlen(old)) == 0) {
         to_replace++;
      }
      source++;
   }
   if (to_replace == 0) {
      return NULL;
   }
   /*
    * Calculate size of new string based on number of substrings to replace.
    */
   change = to_replace * (strlen(new) - strlen(old));
   new_len = strlen(source_string) + change + 1;
   /*
    * Allocate new string and re-shuffle original string.
    */
   return_string = new_string = malloc(new_len);
   if (new_string == NULL) {
      return NULL;
   }
   memset(new_string, 0x0, new_len);
   source = source_string;
   while (source <= tail) {
      current_tail = source + strlen(old) - 1;
      if (current_tail <= tail && memcmp(old, source, strlen(old)) == 0) {
         memcpy(new_string, new, strlen(new));
         new_string += strlen(new);
         source += strlen(old);
      } else {
         *new_string = *source;
         source++;
         new_string++;
      }
   }
   return return_string;
}

/****** sge_var/unescape_env_value() *******************************
*  NAME
*     unescape_env_value() -- 
*
*  SYNOPSIS
*     char * unescape_env_value(char *value)
*
*  FUNCTION
*     Undo any backslash escapes (\\ or \n) in value string, returning
*     a new string.
*
*  INPUTS
*     char *value                 - environment variable value
*
*  RESULT
*     new string
*
*  NOTES
*     MT-NOTE: unescape_env_value() is MT safe
*******************************************************************************/

const char *unescape_env_value(const char *value)
{
   char *new_value = strdup(value);
   int i, j, nchars = strlen(value);

   for (i = 0, j = 0; i <= nchars; j++) {
      if ('\\' == value[i])
         switch (value[i+1]) {
         case 'n':
            new_value[j] = '\n';
            i += 2;
            break;
         case '\\':
            new_value[j] = '\\';
            i += 2;
            break;
         default:
            new_value[j] = value[i];
            i++;
         }
      else {                    /* shouldn't happen */
         new_value[j] = value[i];
         i++;
      }
   }
   return new_value;
}
