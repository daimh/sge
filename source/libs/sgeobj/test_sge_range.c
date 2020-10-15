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

#include "cull/cull.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_range.h"

bool check_range_get_all_ids(void)
{
   bool failed = false;
   lListElem *range;

   /* Create preinitialized range element */
   if (!failed) {
      range = lCreateElem(RN_Type);
      if (range == NULL) {
         failed = true;
      }
   }

   /* Check NULL pointer */
   if (!failed) {
      u_long32 min = 1, max = 6, step = 2;

      range_get_all_ids(NULL, &min, &max, &step);
      if (max != 0 || min != 0 || step != 0) {
         fprintf(stderr, "NULL range is not correct in range_get_all_ids()\n");
         failed = true;
      }
   }

   /* Check preinitialized range */
   if (!failed) {
      u_long32 min = 1, max = 6, step = 2;

      range_get_all_ids(range, &min, &max, &step);
      if (max != 0 || min != 0 || step != 0) {
         fprintf(stderr, "Init range is not correct in range_get_all_ids()\n");
         failed = true;
      }
   }

   /* Check range */
   if (!failed) {
      u_long32 min = 1, max = 6, step = 2;
   
      range_set_all_ids(range, min, max, step);
      range_get_all_ids(range, &min, &max, &step);
      if (min != 1 || max != 6 || step != 2) {
         fprintf(stderr, "Range is not correct in range_get_all_ids()\n");
         failed = true;
      }
   }

   lFreeElem(&range);
   return failed;
}

bool check_range_set_all_ids(void)
{
   bool failed = false;
   lListElem *range;

   /* Create preinitialized range element */
   if (!failed) {
      range = lCreateElem(RN_Type);
      if (range == NULL) {
         failed = true;
      }
   }

   /* Check NULL pointer */
   if (!failed) {
      u_long32 min = 1, max = 6, step = 2;

      range_set_all_ids(NULL, min, max, step);
   }

   /* Check normal range */
   if (!failed) {
      u_long32 min = 1, max = 6, step = 2;
   
      range_set_all_ids(range, min, max, step);
      range_get_all_ids(range, &min, &max, &step);
      if (min != 1 || max != 6 || step != 2) {
         fprintf(stderr, "Range is not correct in range_set_all_ids()\n");
         failed = true;
      }
   }

   /* Check unnormalized range */
   if (!failed) {
      u_long32 min = 5, max = 5, step = 2;
   
      range_set_all_ids(range, min, max, step);
      range_get_all_ids(range, &min, &max, &step);
      if (min != 5 || max != 5 || step != 1) {
         fprintf(stderr, "range_set_all_ids() does not normalize()\n");
         failed = true;
      }
   }

   lFreeElem(&range);
   return failed;
}
int main(int argc, char *argv[])
{
   bool failed = false;

   lInit(nmv);

   if (!failed) {
      failed = check_range_get_all_ids();
   }
   if (!failed) {
      failed = check_range_set_all_ids();
   }

   /*
    * EB: TEST: add additional tests
    */

   if (failed) {
      printf("failed\n");
   }
   else {
      printf("successful\n");
   }
   
   return failed ? 1 : 0;
}
