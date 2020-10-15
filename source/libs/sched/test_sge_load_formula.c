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

#include "uti/sge_rmon.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_centry.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_all_listsL.h"

#include "sort_hosts.h"

typedef struct {
   char *formula;
   double value;
} filter_test_t;

int main(int argc, char *argv[])
{
   int pos_tests_failed = 0;
   int neg_tests_failed = 0;
   int i = 0;
   lList *answer_list = NULL;


   filter_test_t positiv_test[] = {
      {"num_proc", 4}, 
      {"$num_proc", 4},
      {"$num_proc*2", 8},
      {"$num_proc*0.5", 2.0},
      {"num_proc*2", 8},
      {"num_proc+1", 5}, 
      {"$num_proc-2", 2}, 
      {"$num_proc+0.1", 4.1}, 
      {"1+$num_proc+0.1", 5.1}, 
      {NULL, 0}
   };

   filter_test_t negativ_test[] = {
      {"2*num_proc", 0}, 
      {"2,0+num_proc", 0}, 
      {"none", 0}, 
      {NULL, 0}
   };

   lList *centry_list;
   lList *host_centry_list;
   lListElem *centry;
   lListElem *host;

   DENTER_MAIN(TOP_LAYER, "test_sge_load_formula");

   lInit(nmv);

   /* set up centry */
   centry_list = lCreateList("", CE_Type);
   centry = lCreateElem(CE_Type);
   lSetString(centry, CE_name, "num_proc");
   lSetString(centry, CE_stringval, "4");
   lSetDouble(centry, CE_doubleval, 4);
   lAppendElem(centry_list, centry);

   /* set up host */
   host_centry_list = lCreateList("", CE_Type);
   lAppendElem(host_centry_list, lCopyElem(centry));
   host = lCreateElem(EH_Type);
   lSetList(host, EH_consumable_config_list, host_centry_list);

   for (i=0; ; i++){
      double val;
      if (positiv_test[i].formula == NULL) {
         break;
      }
      if (!validate_load_formula(positiv_test[i].formula, &answer_list, centry_list, "load_formula")) {
         answer_list_output(&answer_list);
         pos_tests_failed++;
      }

      val = scaled_mixed_load(positiv_test[i].formula, NULL, host, centry_list);
      if (val != positiv_test[i].value) {
         printf("got %f, but expected %f(%g,%g)\n", val, positiv_test[i].value, val, positiv_test[i].value);
         pos_tests_failed++;
      }
   }
   
   for (i=0; ; i++){
     if (negativ_test[i].formula == NULL) {
         break;
      }
      if (validate_load_formula(negativ_test[i].formula, &answer_list, centry_list, "load_formula") == true) {
         printf("load_formula \"%s\" returned no error\n", negativ_test[i].formula);
         neg_tests_failed++;
      }
      lFreeList(&answer_list);
   }

   lFreeList(&centry_list);
   lFreeElem(&host);

   printf("\n");
   printf("%d positiv test(s) failed\n", pos_tests_failed);
   printf("%d negativ test(s) failed\n", neg_tests_failed);

   DRETURN(pos_tests_failed + neg_tests_failed);
}
