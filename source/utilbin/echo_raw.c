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
 *   Copyright: 2008 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "uti/sge_dstring.h"

/*
 * Implements a binary which prints all paramaters it gets as arguments
 * without interpreting "\<character>" character sequences like \n
 *
 * This command can be used a a replacement for the bourne shell buildin
 * command echo and also /bin/echo
 *
 * new line character will be appended as last character
 */
int main(int argc, char *argv[])
{
   int i = 1;
   int first = 1;
   int mode = 0;
   dstring output_buffer = DSTRING_INIT;
   const char *output = NULL;
   
   if (argc >= 2 && strcmp(argv[1], "-e") == 0) {
      /* skip this argument and change the mode */
      mode = 1;
      i++;
   } else if (argc >= 2 && 
              (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
      /* show the usage and terminate */
      printf("usage: echo_raw [-h|-e] [arg] ...\n");
      printf("write arguments separated by blank and followed by new line to stdout\n");
      printf("\n");
      printf(" -h    show this help\n");
      printf(" -e    enable interpretation of backslash escapes\n");
      printf("       and skip new line character at the end\n");
      printf("       following sequences are recoginzed:\n");
      printf("        \\    backslash\n");
      printf("        \\a   alert (BEL)\n");
      printf("        \\b   backspace\n");
      printf("        \\f   form feed\n");
      printf("        \\n   new line\n");
      printf("        \\r   carriage return\n");
      printf("        \\t   horizontal tab\n");
      printf("        \\v   vertical tab\n");
      exit(0); 
   }

   while (i < argc) {
      if (!first) {
         fputc(' ', stdout);
      }
      if (mode == 0) { 
         fprintf(stdout, "%s", argv[i++]);
      } else {
         if (argv[i] != NULL) {
            size_t length = strlen(argv[i]);
            size_t j = 0;

            while (j <= length) {
               char first = argv[i][j++];
            
               if (first == '\\') {
                  char second = argv[i][j];
                  char out;

                  switch (second) {
                     case '\\':
                        out = '\\';
                        break;
                     case 'a':
                        out = '\a';
                        break;
                     case 'b':
                        out = '\b';
                        break;
                     case 'n':
                        out = '\n';
                        break;
                     case 'f':
                        out = '\f';
                        break;
                     case 'r':
                        out = '\r';
                        break;
                     case 't':
                        out = '\t';
                        break;
                     case 'v':
                        out = '\v';
                        break;
                     default:
                        /*
                         * should not happen. we got an unknown escape sequence.
                         * we do the same bourne shell does: 
                         * print both characters - backlash now and the following
                         * character in the next loop.
                         */
                        j--;
                        out = first;
                  }
                  j++;
                  sge_dstring_append_char(&output_buffer, out);
               } else {
                  sge_dstring_append_char(&output_buffer, first);
               } 
            }
            i++;
         }
      }
      first = 0;
   }
   if (mode == 0) {
      sge_dstring_append_char(&output_buffer, '\n');
   }
   output = sge_dstring_get_string(&output_buffer);
   if (output != NULL) {
      printf("%s", output);
   }
   return 0;   
}

