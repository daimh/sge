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

#define XMALLINFO

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#ifdef MALLINFO
#include <malloc.h>
#endif

#include "uti/sge_bitfield.h"
#include "uti/sge_stdlib.h"

const unsigned int test_bf_max_size = 100;
unsigned int test_bf_loops    = 1000;

static void 
test_nullpointer_actions(void)
{
   bitfield *b1;

   /* bitfields of size 0 */
   b1 = sge_bitfield_new(0);
   b1 = sge_bitfield_free(b1);

   b1 = sge_bitfield_new(10);

   /* pass nullpointer */
   sge_bitfield_free(NULL);
   sge_bitfield_free_data(NULL);

   sge_bitfield_copy(b1, NULL);
   sge_bitfield_copy(NULL, b1);
   sge_bitfield_copy(NULL, NULL);

   sge_bitfield_bitwise_copy(b1, NULL);
   sge_bitfield_bitwise_copy(NULL, b1);
   sge_bitfield_bitwise_copy(NULL, NULL);

   sge_bitfield_set(NULL, 0);
   sge_bitfield_get(NULL, 0);
   sge_bitfield_clear(NULL, 0);
   sge_bitfield_reset(NULL);
   sge_bitfield_changed(NULL);
   sge_bitfield_print(NULL, stdout);
   sge_bitfield_print(b1, NULL); /* shall output to stdout */
   printf("\n");

   /* free bitfield */
   b1 = sge_bitfield_free(b1);
}

static void test_bitop_set(bitfield *bf, unsigned int bit)
{
   sge_bitfield_set(bf, bit);
   if (!sge_bitfield_get(bf, bit)) {
      printf("bit %d should be set\n", bit);
   }
}

static void test_bitop_not_set(const bitfield *bf, unsigned int bit)
{
   if (sge_bitfield_get(bf, bit)) {
      printf("bit %d should not be set\n", bit);
   }
}

static void test_bitop_clear(bitfield *bf, unsigned int bit)
{
   sge_bitfield_clear(bf, bit);
   test_bitop_not_set(bf, bit);
}

static void test_bitfield_changed(const bitfield *bf, bool expected)
{
   if (sge_bitfield_changed(bf) != expected) {
      printf("sge_bitfield_changed reported invalid result: %s\n", 
             expected ? "false" : "true");
      
   }
}

static void test_bitfield_copy(const bitfield *bf)
{
   bitfield *copy;

   /* copy to field with differing size must fail */
   copy = sge_bitfield_new(30);
   if (sge_bitfield_copy(bf, copy)) {
      printf("sge_bitfield_copy to bitfield of differing size should have failed\n");
   }
   copy = sge_bitfield_free(copy);
  
   /* this copy should succeed */
   copy = sge_bitfield_new(sge_bitfield_get_size(bf));
   sge_bitfield_copy(bf, copy);
   sge_bitfield_print(bf, stdout); printf("\n");
   sge_bitfield_print(copy, stdout); printf("\n");
   copy = sge_bitfield_free(copy);
}

static void test_bitfield_bitwise_copy(const bitfield *bf)
{
   bitfield *copy;

   /* output original bitfield */
   sge_bitfield_print(bf, stdout); printf("\n");

   /* copy to smaller bitfield */
   copy = sge_bitfield_new(10);
   sge_bitfield_bitwise_copy(bf, copy);
   sge_bitfield_print(copy, stdout); printf("\n");
   copy = sge_bitfield_free(copy);

   /* copy to same size bitfield */
   copy = sge_bitfield_new(sge_bitfield_get_size(bf));
   sge_bitfield_bitwise_copy(bf, copy);
   sge_bitfield_print(copy, stdout); printf("\n");
   copy = sge_bitfield_free(copy);

   /* copy to bigger bitfield */
   copy = sge_bitfield_new(80);
   sge_bitfield_bitwise_copy(bf, copy);
   sge_bitfield_print(copy, stdout); printf("\n");
   copy = sge_bitfield_free(copy);
}

static void test_bitops(void)
{
   /* we need a fixed size and a dynamic bitfield */
   bitfield *fixed;
   bitfield *dynamic;

   fixed = sge_bitfield_new(20);
   dynamic = sge_bitfield_new(70);

   /* output bitfields, all bits must be 0 */
   printf("new bitfields, all bits 0\n");
   sge_bitfield_print(fixed, stdout); printf("\n");
   sge_bitfield_print(dynamic, stdout); printf("\n");
   printf("\n");

   /* test changed operation */
   test_bitfield_changed(fixed, false);
   test_bitfield_changed(dynamic, false);

   /* set some bits */
   printf("set some bits\n");
   test_bitop_set(fixed, 0);
   test_bitop_set(fixed, 2);
   test_bitop_set(fixed, 10);
   test_bitop_set(fixed, 18);
   test_bitop_not_set(fixed, 1);
   test_bitop_not_set(fixed, 19);
   sge_bitfield_print(fixed, stdout); printf("\n");

   test_bitop_set(dynamic, 0);
   test_bitop_set(dynamic, 10);
   test_bitop_set(dynamic, 63);
   test_bitop_set(dynamic, 64);
   test_bitop_not_set(fixed, 1);
   test_bitop_not_set(fixed, 69);
   sge_bitfield_print(dynamic, stdout); printf("\n");
   printf("\n");

   /* test clear operations */
   printf("clear some bits\n");
   test_bitop_clear(fixed, 2);
   test_bitop_clear(fixed, 2);
   sge_bitfield_print(fixed, stdout); printf("\n");

   test_bitop_clear(dynamic, 10);
   test_bitop_clear(dynamic, 10);
   sge_bitfield_print(dynamic, stdout); printf("\n");
   printf("\n");

   /* test changed operation */
   test_bitfield_changed(fixed, true);
   test_bitfield_changed(dynamic, true);

   /* test copy operations */
   printf("copy\n");
   test_bitfield_copy(fixed);
   printf("\n");
   test_bitfield_copy(dynamic);
   printf("\n");

   /* test bitwise copy operations */
   printf("bitwise copy\n");
   test_bitfield_bitwise_copy(fixed);
   printf("\n");
   test_bitfield_bitwise_copy(dynamic);
   printf("\n");

   /* test reset */
   printf("reset\n");
   sge_bitfield_reset(fixed);
   sge_bitfield_reset(dynamic);
   sge_bitfield_print(fixed, stdout); printf("\n");
   sge_bitfield_print(dynamic, stdout); printf("\n");

   /* free the bitfields */
   fixed   = sge_bitfield_free(fixed);
   dynamic = sge_bitfield_free(dynamic);
}

int 
main(int argc, char *argv[])
{
   unsigned int i;
   bitfield *b1, *b2;

#ifdef MALLINFO
   struct mallinfo meminfo;
#endif

   long clk_tck = 0;
   const char * header_format = "%5s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s\n";
   const char * data_format   = "%5d %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f"
                                " %8.3f %8.3f %8d\n";

   /* evaluate commandline args */
   if (argc > 1) {
      test_bf_loops = atoi(argv[1]);
   }

   /* create array to hold bitfields, to be used per test loop */
   b1 = (bitfield *)malloc(test_bf_loops * sizeof (bitfield));
   b2 = (bitfield *)malloc(test_bf_loops * sizeof (bitfield));
   printf("performing %d loops per action\n", test_bf_loops);

   /* test nullpointer actions */
   printf("\ntesting nullpointer actions\n");
   test_nullpointer_actions();

   printf("\ntesting bit operations\n");
   test_bitops();

   /* test performance */
   clk_tck = sysconf(_SC_CLK_TCK); /* JG: TODO: sge_sysconf? */
   printf("\ntesting performance of all actions\n");
   printf(header_format, "size", "create", "free",
          "copy", "bwcopy",
          "set", "get",
          "clear", "reset", "changed", "mem");

   for (i = 0; i <= test_bf_max_size; i += 10) {
      struct tms tms_buffer;
      clock_t now, start;
      double prof_create = 0.0;
      double prof_free = 0.0;
      double prof_copy = 0.0;
      double prof_bwcopy = 0.0;
      double prof_set = 0.0;
      double prof_get = 0.0;
      double prof_clear = 0.0;
      double prof_reset = 0.0;
      double prof_changed = 0.0;

      unsigned int loops, index;

      srand(time(0));

      /* creation of bitfields */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_init(&b1[loops], i);
         sge_bitfield_init(&b2[loops], i);
      }
      now = times(&tms_buffer);
      prof_create = (now - start) * 1.0 / clk_tck;

      /* copy */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_copy(&b1[loops], &b2[loops]);
      }
      now = times(&tms_buffer);
      prof_copy = (now - start) * 1.0 / clk_tck;

      /* bitwise copy */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_bitwise_copy(&b2[loops], &b2[loops]);
      }
      now = times(&tms_buffer);
      prof_bwcopy = (now - start) * 1.0 / clk_tck;

      /* set/get/clear */
      index = rand() % (i + 1) - 1;
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_set(&b1[loops], index);
      }
      now = times(&tms_buffer);
      prof_set = (now - start) * 1.0 / clk_tck;

      index = rand() % (i + 1) - 1;
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_get(&b1[loops], index);
      }
      now = times(&tms_buffer);
      prof_get = (now - start) * 1.0 / clk_tck;

      index = rand() % (i + 1) - 1;
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_clear(&b1[loops], index);
      }
      now = times(&tms_buffer);
      prof_clear = (now - start) * 1.0 / clk_tck;

      /* reset */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_reset(&b1[loops]);
      }
      now = times(&tms_buffer);
      prof_reset = (now - start) * 1.0 / clk_tck;

      /* changed */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_changed(&b1[loops]);
      }
      now = times(&tms_buffer);
      prof_changed = (now - start) * 1.0 / clk_tck;

      /* evaluate memory usage */
#ifdef MALLINFO
      meminfo = mallinfo();
#endif

      /* freeing of bitfields */
      start = times(&tms_buffer);
      for (loops = 0; loops < test_bf_loops; loops++) {
         sge_bitfield_free_data(&b1[loops]);
         sge_bitfield_free_data(&b2[loops]);
      }
      now = times(&tms_buffer);
      prof_free = (now - start) * 1.0 / clk_tck;

      printf(data_format, i, prof_create, prof_free,
             prof_copy, prof_bwcopy,
             prof_set, prof_get, prof_clear,
             prof_reset, prof_changed,
#ifdef MALLINFO
             (meminfo.usmblks + meminfo.uordblks) / 1024
#else
             0
#endif
            );
   }

   /* free memory used for bitfields */
   sge_free(&b1);
   sge_free(&b2);
   return EXIT_SUCCESS;
}
