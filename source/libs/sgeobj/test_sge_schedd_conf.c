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

#include "uti/sge_rmon.h"

#include "cull/cull.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_schedd_conf.h"

/**
 * This test only tests issue: 1826. A complete
 * test of the scheduler configuration is needed
 * and currently missing.
 *
 * TODO: a complete scheduler config test
 */

typedef struct {
   int  test_nr;                /* identifies the test */
   char *test_value;            /* the test setting */
   bool result;                 /* expected result; true = valid setting */
} conf_settings_t;

typedef struct {
   u_long      test_attribute;   /* identifies the attribute to test */
   u_long      type;             /* identifies the attribute type */
   char        *description;     /* the test description */
} schedd_conf_t;

/**
 * Defines the atribute, which is tested.
 * Supported types are:
 * - lStringT
 * - lDoubleT
 * - lUlongT
 */
static schedd_conf_t conf_tests[] = {
   {SC_halflife_decay_list, lStringT,"test the halflife_decay_list settings"},
   {0,0,NULL}
};

static conf_settings_t tests[] = {
   {0, "NONE", true},
   {0, "nonesense", false},
   {0, "what:ever=this_is", false},
   {0, "cpu=1", true},
   {0, "cpu=1:", true},
   {0, "cpu=1:io=-1", true},
   {0, "cpu=1:io=0:", true},
   {0, "cpu=1:io=-1:mem=0", true},
   {0, "cpu=1:io=-1:mem=0:help=-1", true},
   {-1, NULL, false}
};

static int
test(conf_settings_t *setting, schedd_conf_t *test, int test_counter) 
{
   lListElem *schedd_conf = sconf_create_default();
   lList *schedd_list = lCreateList("schedd_conf", SC_Type);
   lList *answer_list = NULL;
   int ret = 0;

   if (schedd_conf != NULL && schedd_list != NULL) {
      lAppendElem(schedd_list, schedd_conf); 

      switch (test->type) {
         case lStringT :
               lSetString(schedd_conf, (int)conf_tests->test_attribute, setting->test_value);
            break;
         case lDoubleT :
            break;
         case lUlongT :
            break;
         default :
               printf("ERROR: requested type %d is not suported by the test infrastructure\n", (int) test->type);
               ret = 1;
            break;
      }
     
      if (sconf_validate_config(&answer_list, schedd_list) != setting->result ) {
         printf("ERROR: the test failed\n");
         printf("ERROR: expected that the validate function returns %s\n", setting->result?"TRUE":"FALSE");
         answer_list_output(&answer_list);
      }
   }
   else {
      printf("ERROR: failed to create the required elements\n");
      ret = 1;
   }
            
   return ret;
}


/****** test_sge_calendar/main() ***********************************************
*  NAME
*     main() -- calendar test
*
*  SYNOPSIS
*     int main(int argc, char* argv[]) 
*
*  FUNCTION
*     calendar test
*
*  INPUTS
*     int argc     - nr. of args 
*     char* argv[] - args
*
*  RESULT
*     int -  nr of failed tests
*
*******************************************************************************/
int main(int argc, char* argv[])
{
   int test_counter = 0;
   int failed = 0;
   int past_test = -1;
   lInit(nmv);
   
   printf("==> Scheduler config test <==\n");

   while (tests[test_counter].test_nr != -1) {
      if (past_test != tests[test_counter].test_nr) {
         past_test = tests[test_counter].test_nr;
         printf("\n-----------------------\nNr: %d/%d\n", test_counter, past_test);
         printf("Test:    %s\n", conf_tests[past_test].description);
      }
      else {
         printf("-------------\nNr: %d/%d\n", test_counter, past_test);
      }
      printf("Setting: %s\n", tests[test_counter].test_value);
      printf("Should be accepted: %s\n", tests[test_counter].result?"YES":"NO");
      if (test(&(tests[test_counter]), 
               &(conf_tests[tests[test_counter].test_nr]), 
               test_counter) != 0) {
         failed++; 
      }   
      test_counter++;
   }
   printf("\n-----------------------");
   if (failed == 0) {
      printf("\n==> All tests are okay <==\n");
   }
   else {
      printf("\n==> %d/%d test(s) failed <==\n", failed, test_counter);
   }
   
   return failed;
}
