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
#include <stdlib.h>
#include <string.h>

#include "sge_string.h"

int main(int argc, char *argv[])
{
   bool ret = true;

   char buffer[10];
   int len;

   sge_strlcpy(buffer, "12345678901234567890", sizeof(buffer));
   printf("%2d %s\n", (int)strlen(buffer), buffer);
   if (strlen(buffer) != 9) {
      fprintf(stderr, "strlen after sge_strlcpy should be 9\n");
      ret = false;
   }

   sge_strlcpy(buffer, "1234", sizeof(buffer));
   len = sge_strlcat(buffer, "1234", sizeof(buffer));
   printf("%2d %s\n", len, buffer);
   if (len != 9 || strcmp(buffer, "12341234") != 0) {
      fprintf(stderr, "sge_strlcat(1) failed\n");
      ret = false;
   }

   len = sge_strlcat(buffer, "1234", sizeof(buffer));
   printf("%2d %s\n", len, buffer);
   if (len != 13 || strcmp(buffer, "123412341") != 0) {
      fprintf(stderr, "sge_strlcat(1) failed\n");
      ret = false;
   }

   len = sge_strlcat(buffer, "1234", sizeof(buffer));
   printf("%2d %s\n", len, buffer);
   if (len != 14 || strcmp(buffer, "123412341") != 0) {
      fprintf(stderr, "sge_strlcat(1) failed\n");
      ret = false;
   }

   return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
