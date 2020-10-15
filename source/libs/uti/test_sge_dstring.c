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
#include <time.h>
#include <sys/time.h>

#include "uti/sge_dstring.h"

#define STATIC_SIZE 20

static bool
check_dstring(dstring *sb) {
   bool ret = true;

   printf("%5d : %5d : %-50s\n", (int)sb->size, (int)sb->length, 
          sb->s == NULL ? "(null)" : sb->s);

   return ret;
}

static bool 
check_all(dstring *sb)
{
   bool ret = true;
   int i;

   /* sge_dstring_append */
   printf("\nchecking sge_dstring_append\n");
   sge_dstring_append(NULL, NULL);

   sge_dstring_append(sb, NULL);
   check_dstring(sb);

   sge_dstring_append(sb, "blah");
   check_dstring(sb);

   sge_dstring_clear(sb);
   sge_dstring_append(sb, "too long string to fit into a static string buffer");
   check_dstring(sb);

   sge_dstring_clear(sb);
   sge_dstring_append(sb, 
                      "long string that requires multiple chunks ....... ");
   check_dstring(sb);
   for (i = 0; i < 20; i++) {
      sge_dstring_append(sb, 
                         "long string that requires multiple chunks ....... ");
   }
   check_dstring(sb);

   /* sge_dstring_append_dstring */
   printf("\nchecking sge_dstring_append_dstring\n");
   sge_dstring_clear(sb);
   sge_dstring_append_dstring(NULL, NULL);
   {
      dstring second = DSTRING_INIT;
      sge_dstring_append(&second, "dstring");
      sge_dstring_append_dstring(NULL, &second);
      sge_dstring_append_dstring(sb, NULL);
      sge_dstring_append_dstring(sb, &second);
      check_dstring(sb);
   
      sge_dstring_free(&second);
   }

   /* sge_dstring_append_char */
   printf("\nchecking sge_dstring_append_char\n");
   sge_dstring_clear(sb);
   sge_dstring_append_char(NULL, 'a');
   sge_dstring_append_char(sb, '\0');
   check_dstring(sb);
   sge_dstring_append_char(sb, 'a');
   check_dstring(sb);
   sge_dstring_append_char(sb, 'b');
   check_dstring(sb);

   /* sge_dstring_sprintf */
   printf("\nchecking sge_dstring_sprintf\n");
   sge_dstring_sprintf(NULL, "test %s", "string");
   sge_dstring_sprintf(sb, NULL);
   sge_dstring_sprintf(sb, "test %s", "string");
   check_dstring(sb);
  
#if 0
   /* does not build on irix */
   /* sge_dstring_vsprintf */
   printf("\nchecking sge_dstring_vsprintf\n");
   {
      const char *args[] = { "string", NULL };
      sge_dstring_clear(sb);
      sge_dstring_vsprintf(NULL, "test %s", args);
      sge_dstring_vsprintf(sb, NULL, args);
      sge_dstring_vsprintf(sb, "test %s", args);
      check_dstring(sb);
   }
#endif
   
   /* sge_dstring_sprintf_append */
   printf("\nchecking sge_dstring_sprintf_append\n");
   sge_dstring_clear(sb);
   sge_dstring_sprintf_append(NULL, "test %s", "string");
   sge_dstring_sprintf_append(sb, NULL);
   sge_dstring_sprintf_append(sb, "test %s", "string");
   sge_dstring_sprintf_append(sb, " appended test %s", "string");
   check_dstring(sb);
   
   /* sge_dstring_clear */
   printf("\nchecking sge_dstring_clear\n");
   sge_dstring_clear(NULL);
   sge_dstring_clear(sb);
   check_dstring(sb);
   
   /* sge_dstring_free */
   printf("\nchecking sge_dstring_free\n");
   sge_dstring_free(NULL);
   sge_dstring_free(sb);
   check_dstring(sb);
   
   /* sge_dstring_get_string */
   printf("\nchecking sge_dstring_get_string\n");
   sge_dstring_clear(sb);
   sge_dstring_append(sb, "test string");
   { 
      const char *result;

      result = sge_dstring_get_string(NULL);
      printf("sge_dstring_get_string(NULL) = %s\n", 
             result == NULL ? "NULL" : result);
      result = sge_dstring_get_string(sb);
      printf("sge_dstring_get_string(sb) = %s\n", 
             result == NULL ? "NULL" : result);
   }
   
   /* sge_dstring_copy_string */
   printf("\nchecking sge_dstring_copy_string\n");
   sge_dstring_copy_string(NULL, NULL);
   sge_dstring_copy_string(sb, NULL);
   sge_dstring_copy_string(NULL, "new test string");
   sge_dstring_copy_string(sb, "new test string");
   check_dstring(sb);
   
   /* sge_dstring_copy_dstring 
    * check only NULL pointer behaviour, it just calls sge_dstring_copy_string
    */
   printf("\nchecking sge_dstring_copy_dstring\n");
   sge_dstring_copy_dstring(NULL, NULL);
   sge_dstring_copy_dstring(sb, NULL);
   check_dstring(sb);
   
   /* sge_dstring_strlen */
   printf("\nchecking sge_dstring_strlen\n");
   {
      int len;
      sge_dstring_copy_string(sb, "test string");
      len = sge_dstring_strlen(NULL);
      printf("sge_dstring_strlen(NULL) = %d\n", len);
      len = sge_dstring_strlen(sb);
      printf("sge_dstring_strlen(sb) = %d\n", len);
   }
   
   /* sge_dstring_remaining */
   printf("\nchecking sge_dstring_remaining\n");
   {
      int len;
      sge_dstring_copy_string(sb, "test string");
      len = sge_dstring_remaining(NULL);
      printf("sge_dstring_remaining(NULL) = %d\n", len);
      len = sge_dstring_remaining(sb);
      printf("sge_dstring_remaining(sb) = %d\n", len);
   }

   return ret;
}

static void test_dstring_performance(dstring *ds, int max, const char *data)
{
   int i;
   struct timeval before;
   struct timeval after;
   double time;

   gettimeofday(&before, NULL);
   for (i = 0; i < max; i++) {
      sge_dstring_sprintf(ds, "%s", data);
   }
   gettimeofday(&after, NULL);

   time = after.tv_usec - before.tv_usec;
   time = after.tv_sec - before.tv_sec + (time/1000000);

   printf("%d sge_dstring_sprintf took %.2fs\n", max, time);
}

static void test_dstring_performance_static(int max, const char *data)
{
   int i;
   struct timeval before;
   struct timeval after;
   double time;

   gettimeofday(&before, NULL);
   for (i = 0; i < max; i++) {
      dstring ds;
      char ds_buffer[MAX_STRING_SIZE];
      sge_dstring_init(&ds, ds_buffer, sizeof(ds_buffer));
      sge_dstring_sprintf(&ds, "%s/%s", data, data);
   }
   gettimeofday(&after, NULL);

   time = after.tv_usec - before.tv_usec;
   time = after.tv_sec - before.tv_sec + (time/1000000);

   printf("%d static dstring creations took %.2fs\n", max, time);
}

static void test_dstring_performance_dynamic(int max, const char *data)
{
   int i;
   struct timeval before;
   struct timeval after;
   double time;

   gettimeofday(&before, NULL);
   for (i = 0; i < max; i++) {
      dstring ds = DSTRING_INIT;
      sge_dstring_sprintf(&ds, "%s/%s", data, data);
      sge_dstring_free(&ds);
   }
   gettimeofday(&after, NULL);

   time = after.tv_usec - before.tv_usec;
   time = after.tv_sec - before.tv_sec + (time/1000000);

   printf("%d dstring creations took %.2fs\n", max, time);
}

int main(int argc, char *argv[])
{
   bool ret = true;
   dstring dynamic_dstring = DSTRING_INIT;
   dstring static_dstring;
   char    static_buffer[MAX_STRING_SIZE];
   
   sge_dstring_init(&static_dstring, static_buffer, STATIC_SIZE);

   printf("running all checks with a dynamic dstring\n");
   ret = check_all(&dynamic_dstring);
   test_dstring_performance(&dynamic_dstring, 100000, "test_data"); 
   test_dstring_performance_dynamic(100000, "test_data"); 
   printf("%s\n", sge_dstring_get_string(&dynamic_dstring));

   if (ret) {
      printf("\n\nrunning all checks with a static dstring of length %d\n", 
             STATIC_SIZE);
         ret = check_all(&static_dstring);
         test_dstring_performance(&static_dstring, 100000, "test_data"); 
         test_dstring_performance_static(100000, "test_data"); 
         printf("%s\n", sge_dstring_get_string(&static_dstring));
   }



   sge_dstring_free(&dynamic_dstring);

   return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
