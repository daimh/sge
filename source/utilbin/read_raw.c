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
#include <string.h>

/*
 * Implements a binary which reads in from stdin in "raw" mode until
 * the first newline character occurs.
 *
 * This command can be used as a replacement for the builtin command
 * "read" in bourne shell. On most platforms this not able to read
 * in raw mode and does backslash escaping.
 *
 * "Binary" data with masked newline and backslash characters can be
 * read into a bourne shell scripts with this command.
 */

#define BUF_SIZE 8 * 1024

int main(int argc, char *argv[])
{
   char buffer[BUF_SIZE];
   char *ret;

   setvbuf(stdin,NULL,_IONBF,0);

   ret = fgets(buffer, BUF_SIZE, stdin);
   if (ret != NULL) {
      size_t length = strlen(ret);

      while (length-- > 0) {
         fputc(*(ret++), stdout);
      }
   }
   return feof(stdin);
}
