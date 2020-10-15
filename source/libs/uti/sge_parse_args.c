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

#include <ctype.h>

#include "uti/sge_rmon.h"
#include "uti/sge_string.h"
#include "uti/sge_sl.h"

/* This method counts the number of arguments in the string using a quick and
 * dirty algorithm.  The algorithm may incorrectly report the number of arguments
 * to be too large because it does not parse quotations correctly. 
 * MT-NOTE: sge_quick_count_num_args() is MT safe
 */
int sge_quick_count_num_args (
const char* args /* The argument string to count by whitespace tokens */
) {
   int num_args = 0;
   char *resreq = (char *)malloc (strlen (args)+1);
   char *s;
   struct saved_vars_s *context = NULL;
   
   DENTER(TOP_LAYER, "sge_quick_count_num_args");
   
   /* This function may return a larger number than required since it does not
    * parse quotes.  This is ok, however, since it's current usage is for
    * mallocing arrays, and a little too big is fine.
    */
   strcpy(resreq, args);
   for (s = sge_strtok_r(resreq, " \t", &context); s != NULL; s = sge_strtok_r(NULL, " \t", &context)) {
      num_args++;
   }
   sge_free(&resreq);
   sge_free_saved_vars(context);
   
   DRETURN(num_args);
}


/* This method should probably be moved out of this file into somewhere more
 * common so that other routines can use it. */
int sge_parse_args (
const char* args, /* The argument string to parse by whitespace and quotes */
char** pargs /* The array to contain the parsed arguments */
) {
   const char *s; 
   char *d;
   char quote;
   char *start;
   char *resreq;
   int finished, count = 0;

   DENTER(TOP_LAYER, "sge_parse_args");
   
   resreq = malloc(strlen(args) + 1);
   d = resreq;
   s = args;
   start = resreq;
   finished = 0;
   
   while (!finished) {
      if (*s == '"' || *s == '\'') {      /* copy quoted arguments */
         quote = *s++;                   /* without quotes */
         while (*s && *s != quote) 
           *d++ = *s++;
         if (*s == quote) 
            s++;
      }

      if (*s == '\0') finished = 1;          /* line end ? */

      if (finished || isspace(*s)) {      /* found delimiter or line end */
         *d++ = 0;                       /* terminate token */
         pargs[count++] = strdup(start);   /* assign argument */
         if (!finished) {
            while (isspace(*(++s)));      /* skip any number whitespace */
         }   
         if (*s == '\0') {
            finished = 1;
         } else {
            start = d;                      /* assign start of next token */
         }
      } else {
         *d++ = *s++;                    /* copy one character */
      }
   } 
   sge_free(&resreq);

   DRETURN(count);
}

/* command will be modifed in this function!
 * The elements of the sl_args list are pointers to parts of command,
 * i.e. if command is freed all datas of all elements are freed, too
 */
/* Return values:
 * 0 - OK
 * 1 - unmatched quote "
 * 2 - unmatched quote '
 */
/****** sge_parse_args/parse_quoted_command_line() *****************************
*  NAME
*     parse_quoted_command_line() -- parses a command line with quoted arguments
*
*  SYNOPSIS
*     int parse_quoted_command_line(char *command, sge_sl_list_t *sl_args) 
*
*  FUNCTION
*     Parses a command line with quoted arguments and stores the single
*     arguments into a SGE simple list (sge_sl). The command line is modified
*     during the parse process. This function recognizes the quotes " and '.
*     This function can't handle escaped quotes, see last example.
*
*  INPUTS
*     char *command          - the command line to be parsed. "command" gets
*                              modified during the parse process, so provide
*                              a copy of the real command line if you need it
*                              afterwards.
*     sge_sl_list_t *sl_args - pointer to the list where the single arguments
*                              get stored to. This must be a properly created
*                              list (using sge_sl_create()).
*                              In case of error, the list contains all arguments
*                              not including the last quoted one (with the
*                              missing closing quote)
*
*  RESULT
*     int - 0: OK
*           1: unmatched quote "
*           2: unmatched quote '
*
*  EXAMPLE
*     command line: foo "bar trala 'hey hu'"
*     list:
*     + foo
*     + bar trala 'hey hu'
*
*     command line: 'foo bar' trala "hey" hu
*     list:
*     + foo bar
*     + trala
*     + hey
*     + hu
*     
*     command line: foo "bar trala '"hey hu'
*     list:
*     + foo
*     + bar trala '
*     + hey hu'
*
*     command line: foo "bar trala hey hu
*     return value 1
*     list:
*     + foo
*
*     command line: "foo \"bar huhu\" trala" hey
*     list:
*     + foo \
*     + bar
*     + huhu\
*     +  trala
*     + hey
*
*  NOTES
*     MT-NOTE: parse_quoted_command_line() is not MT safe 
*
*  SEE ALSO
*     sge_parse_args/convert_arg_list_to_vector
*******************************************************************************/
int parse_quoted_command_line(
char *command,
sge_sl_list_t *sl_args)
{
   const char quotes[]    = "\"\'";
   char       last_quote  = 0;
   char       *p_cur_arg  = NULL;
   char       *p_cur_char = command;

   while (*p_cur_char != '\0') {
      /* skip leading whitespaces */
      while (isspace(*p_cur_char)) {
         p_cur_char++;
      }
      
      /* quoted argument */
      if (*p_cur_char == quotes[0] || *p_cur_char == quotes[1]) {
         /* save this opening quote, so we know when this term ends */
         last_quote = *p_cur_char;

         /* skip quote */
         p_cur_char++;

         /* here the current argument begins */
         p_cur_arg = p_cur_char;

         /* search for closing quote */
         while (*p_cur_char != '\0' && *p_cur_char != last_quote) {
            p_cur_char++;
         }
         if (*p_cur_char == '\0') {
            /* closing quote is missing! */
            int ret = 1;
            if (last_quote == '\'') {
               ret = 2;
            }
            return ret;
         } else {
            /* replace closing quote with '\0' to terminate this argument */
            *p_cur_char = '\0';
            /* add current argument to argument list */
            sge_sl_insert(sl_args, p_cur_arg, SGE_SL_BACKWARD);
            /* move on to next argument in command line */
            p_cur_char++;
         }
      } else if (*p_cur_char != '\0') {  /* unquoted argument */
         /* here the current argument begins */
         p_cur_arg = p_cur_char;

         /* search trailing whitespace */
         while (*p_cur_char != '\0' && !isspace(*p_cur_char)) {
            p_cur_char++;
         }

         /*
          * only if we are not at the end of the line, we have to terminate this
          * argument and move on to the next argument.
          */
         if (*p_cur_char != '\0') {
            /* replace trailing whitspace with '\0' to terminate this argument */
            *p_cur_char = '\0';
            /* move on to next argument in command line */
            p_cur_char++;
         }
         /* add current argument to argument list */
         sge_sl_insert(sl_args, p_cur_arg, SGE_SL_BACKWARD);
      }
   }
   return 0;
}

/****** sge_parse_args/convert_arg_list_to_vector() ****************************
*  NAME
*     convert_arg_list_to_vector() -- creates an argument vector from a list
*
*  SYNOPSIS
*     void convert_arg_list_to_vector(sge_sl_list_t *sl_args, char ***pargs) 
*
*  FUNCTION
*     Takes the list of arguments (i.e. a list of strings) and creates an
*     argument vector out of it, like you get it in the main() function.
*     The strings in the argument vector are still the ones from the argument
*     list, i.e. the string buffers are not copied, the elements of the
*     argument vector just point to them.
*
*  INPUTS
*     sge_sl_list_t *sl_args - a list with single C strings as elements
*     char ***pargs          - an argument vector like main() provides, filled
*                              with all arguments from the list.
*                              This argument vector is terminated by an element
*                              that points to NULL.
*
*  RESULT
*     int - The length of the argument vector including the terminal NULL
*           element.
*
*  EXAMPLE
*     sge_sl_list_t *sl_args;
*     char          **args;
*     char          command_line[] = "foo bla \"trala\"";
* 
*     sge_sl_create(&sl_args);
*
*     parse_quoted_command_line(command_line, sl_args);
*     convert_arg_list_to_vector(sl_args, &args);
*
*     ... use args ...
*     ... done using args ...
*     sge_sl_destroy(&sl_args, NULL);
*     
*  NOTES
*     MT-NOTE: convert_arg_list_to_vector() is not MT safe 
*
*  SEE ALSO
*     sge_parse_args/parse_quoted_command_line
*******************************************************************************/
int convert_arg_list_to_vector(
sge_sl_list_t *sl_args, 
char ***pargs)
{
   int           count, i;
   sge_sl_elem_t *pelem;

   count = sge_sl_get_elem_count(sl_args);
   *pargs = (char**)malloc((count+1)*sizeof(char*));

   i = 0;
   for_each_sl(pelem, sl_args) {
     (*pargs)[i++] = pelem->data;
   }
   (*pargs)[i] = NULL;

   return count+1;
}



