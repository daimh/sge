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

#include "uti/sge_rmon.h"

#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_all_listsL.h"

#include "sge_resource_utilization.h"
#include "sge_mt_init.h"

#include "sge_qeti.h"

typedef struct {
   u_long32 start_time;
   u_long32 duration;
   double uti;
} test_array_t;

static int do_utilization_test(lListElem *cr, test_array_t *ta);
static int do_qeti_test(lListElem *cr, u_long32 *qeti_expected_result);

static int test_normal_utilization(void);
static int test_extensive_utilization(void);

int main(int argc, char *argv[]) 
{
   int ret = 0;

   DENTER_MAIN(TOP_LAYER, "test_resource_utilization");

   sge_mt_init();

   lInit(nmv);

   ret += test_normal_utilization();
   ret += test_extensive_utilization();

   if (ret != 0) {
      printf("\ntest failed!\n");
   }

   return ret;
}

static int do_utilization_test(lListElem *cr, test_array_t *ta)
{
   int ret = 0;
   double uti;
   int i;

   for (i = 0; ta[i].start_time != 0; i++) {
      uti = utilization_max(cr, ta[i].start_time, ta[i].duration, false);
      if (uti != ta[i].uti) {
         printf("failed: utilization(cr, "sge_U32CFormat", "sge_U32CFormat") returned %f, expected %f\n",
                sge_u32c(ta[i].start_time), sge_u32c(ta[i].duration), uti, ta[i].uti);
         ret++;
      } else {
         printf("success: utilization(cr, "sge_U32CFormat", "sge_U32CFormat") returned %f\n",
                sge_u32c(ta[i].start_time), sge_u32c(ta[i].duration), uti);
      }
   }

   return ret;
}

static int do_qeti_test(lListElem *cr, u_long32 *qeti_expected_result)
{
   lList *cr_list;
   sge_qeti_t *iter;
   u_long32 pe_time;
   int ret = 0;
   int i = 0;

   /* sge_qeti_allocate() */
   cr_list = lCreateList("", RUE_Type);
   lAppendElem(cr_list, cr);
   iter = sge_qeti_allocate2(cr_list);

   /* sge_qeti_first() */
   for (pe_time = sge_qeti_first(iter), i=0; pe_time; pe_time = sge_qeti_next(iter), i++) {
      if (qeti_expected_result == NULL) {
         printf("failed: qeti returned "sge_U32CFormat", expected no iteration\n", sge_u32c(pe_time));
         ret++;
      } else if (qeti_expected_result[i] != pe_time) {
         printf("failed: qeti returned "sge_U32CFormat", expected "sge_U32CFormat"\n", sge_u32c(pe_time), sge_u32c(qeti_expected_result[i]));
         ret++;
      } else {
         printf("success: QETI returned "sge_U32CFormat"\n", sge_u32c(pe_time));
      }
   }

   lDechainElem(cr_list, cr);
   sge_qeti_release(&iter);
   lFreeList(&cr_list);

   return ret;
}

static int test_normal_utilization(void)
{
   /*
    *  8-|          --------    ----
    *    |
    *  4-|                  ---- 
    *    |
    *  0----------------------------------->
    *               |       |   |   |
    *              800     1000
    */
   int ret = 0;

   static u_long32 qeti_expected_result[] = {
      1200,
      1100,
      1000,
      800
   };

   test_array_t test_array[] = {
   {1000, 100, 4},
   {1200, 150, 0},
   {700, 150, 8},
   {0, 0, 0}
   };

   lListElem *cr = lCreateElem(RUE_Type);
   lSetString(cr, RUE_name, "slots");

   printf("\n - test simple reservation - \n\n");


   printf("adding a 200s now assignment of 8 starting at 800\n");
   utilization_add(cr, 800, 200, 8, 100, 1, PE_TAG, "pe_slots", "STARTING", false, false);

   printf("adding a 100s now assignment of 4 starting at 1000\n");
   utilization_add(cr, 1000, 100, 4, 101, 1, PE_TAG, "pe_slots", "STARTING", false, false);

   printf("adding a 100s reservation of 8 starting at 1100\n");
   utilization_add(cr, 1100, 100, 8, 102, 1, PE_TAG, "pe_slots", "RESERVING", false, false);

   ret += do_utilization_test(cr, test_array);
   ret += do_qeti_test(cr, qeti_expected_result);

   lFreeElem(&cr);
   return ret;
}

static int test_extensive_utilization(void) {
   int ret = 0;
   
   lListElem *cr = lCreateElem(RUE_Type);
   lSetString(cr, RUE_name, "slots");

   printf("\n - test INIFNITY reservation & unreservation - \n\n");

   {
      /*
       *  8-|          |-------|             |-----......
       *    |
       *  4-|                  |---|----------------.... 
       *    |
       *  0-------------------------------------->
       *               |       |   |         |
       *              800     1000          2000
       */

      static u_long32 qeti_expected_result[] = {
         U_LONG32_MAX,
         2000,
         1000,
         800
      };

      test_array_t test_array[] = {
      {1000, 100, 4},
      {1200, U_LONG32_MAX, 8},
      {200, U_LONG32_MAX, 8},
      {700, 150, 8},
      {700, 100, 0},
      {3600, 150, 8},
      {1000, 1000, 4},
      {0, 0, 0}
      };

      printf("1. reserved and verify result\n\n");

      printf("adding a 200s now assignment of 8 starting at 800\n");
      utilization_add(cr, 800, 200, 8, 100, 1, PE_TAG, "pe_slots", "STARTING", false, false);

      printf("adding a 100s now assignment of 4 starting at 1000\n");
      utilization_add(cr, 1000, 100, 4, 101, 1, PE_TAG, "pe_slots", "STARTING", false, false);

      printf("adding a unlimited reservation of 4 starting at 1100\n");
      utilization_add(cr, 1100, U_LONG32_MAX, 4, 102, 1, PE_TAG, "pe_slots", "RESERVING", false, false);

      printf("adding a unlimited reservation of 4 starting at 2000\n");
      utilization_add(cr, 2000, U_LONG32_MAX, 4, 103, 1, PE_TAG, "pe_slots", "RESERVING", false, false);

      ret += do_utilization_test(cr, test_array);
      ret += do_qeti_test(cr, qeti_expected_result);
   }

   {
      /*
       *  8-|          |-------|
       *    |
       *  4-|                                |-----......
       *    |
       *  0-------------------------------------->
       *               |       |   |         |
       *              800     1000          2000
       */

      static u_long32 qeti_expected_result[] = {
         U_LONG32_MAX,
         2000,
         1000,
         800
      };

      test_array_t test_array[] = {
      {1000, 100, 0},
      {1200, U_LONG32_MAX, 4},
      {200, U_LONG32_MAX, 8},
      {700, 150, 8},
      {700, 100, 0},
      {3600, 150, 4},
      {1000, 1000, 0},
      {0, 0, 0}
      };

      printf("2. unreserve some and test result\n\n");

      printf("removing a 100s now assignment of 4 starting at 1000\n");
      utilization_add(cr, 1000, 100, -4, 101, 1, PE_TAG, "pe_slots", "STARTING", false, false);

      printf("removing a unlimited reservation of 4 starting at 1100\n");
      utilization_add(cr, 1100, U_LONG32_MAX, -4, 102, 1, PE_TAG, "pe_slots", "RESERVING", false, false);

      ret += do_utilization_test(cr, test_array);
      ret += do_qeti_test(cr, qeti_expected_result);
   }

   {
      test_array_t test_array[] = {
      {1000, 100, 0},
      {1200, U_LONG32_MAX, 0},
      {200, U_LONG32_MAX, 0},
      {700, 150, 0},
      {700, 100, 0},
      {3600, 150, 0},
      {1000, 1000, 0},
      {0, 0, 0}
      };

      printf("3. unreserve all\n\n");

      printf("removing a 200s now assignment of 8 starting at 800\n");
      utilization_add(cr, 800, 200, -8, 100, 1, PE_TAG, "pe_slots", "STARTING", false, false);

      printf("removing a unlimited reservation of 4 starting at 2000\n");
      utilization_add(cr, 2000, U_LONG32_MAX, -4, 103, 1, PE_TAG, "pe_slots", "RESERVING", false, false);

      ret += do_utilization_test(cr, test_array);
      ret += do_qeti_test(cr, NULL);
   }

   lFreeElem(&cr);

   return ret;
}
