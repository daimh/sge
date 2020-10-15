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

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_utility.h"

#include "basis_types.h"

int main(int argc, char *argv[]) 
{
   lList *answer_list = NULL;

   int i=0, ret=0;

   static const char* denied[] = {
      "forbiddencharacterwithin@thestring",
      ".forbiddencharacteratthebeginning",
      "TEMPLATE",
      "ALL",
      "NONE",
      "thisisawordwithmorethanfivehundredandtwelvecharactersitishardtowritesomethinglongbecauseidontknwowhatishouldwritesoidecidedtowritedownashortstoryaboutanythingwhichisnotrealonceuponatimetherewasalittlesoftwareprogrammerhewasinsanebecausehehastofindwordswhicharelongerthanfivehunderdandtwelvecharactersandhefoundithardtowritesuchlongwordsbuthediditandhedecidedtowritedownashortstoryaboutalittleprogrammerasoftwareprogrammerwhohastowritetestsfortestingfunctionwhichteststehlengthofstringsandaftmanymanymanycharactershesolvedtheproblem",
      "bla%sfoo",
      "has space?",
      "non-ascii \0377",
      NULL
   };

   static const char* allowed[] = {
      "forbiddencharacterwithingthestring",
      "forbiddencharacteratthebeginning",
      "EMPLATE",
      "boutanythingwhichisnotrealonceuponatimetherewasalittlesoftwareprogrammerhewasinsanebecausehehastofindwordswhicharelongerthanfivehunderdandtwelvecharactersandhefoundithardtowritesuchlongwordsbuthediditandhedecidedtowritedownashortstoryaboutalittleprogrammerasoftwareprogrammerwhohastowritetestsfortestingfunctionwhichteststehlengthofstringsandaftmanymanymanycharactershesolvedtheproblem",
      NULL
   };

   static const char* wcdenied[] = {".bogus", "{}", NULL};

   static const char* wcallowed[] = {"wc_*[]?", NULL};

   for (i=0; denied[i] != NULL; i++) {
      if (verify_str_key(
            &answer_list, denied[i], MAX_VERIFY_STRING, "test", KEY_TABLE) == STATUS_OK) {
         printf("%s should be forbidden\n",  denied[i]);
         ret++;
      }
   }

   for (i=0; allowed[i] != NULL; i++) {
      if (verify_str_key(
            &answer_list, allowed[i], MAX_VERIFY_STRING, "test", KEY_TABLE) != STATUS_OK) {
         printf("%s should be allowed\n",  allowed[i]);
         ret++;
      }
   }

   for (i=0; wcdenied[i] != NULL; i++) {
      if (verify_str_key(
            &answer_list, wcdenied[i], MAX_VERIFY_STRING, "test", WC_TABLE) == STATUS_OK) {
         printf("%s should be forbidden\n",  wcdenied[i]);
         ret++;
      }
   }

   for (i=0; wcallowed[i] != NULL; i++) {
      if (verify_str_key(
            &answer_list, wcallowed[i], MAX_VERIFY_STRING, "test", WC_TABLE) != STATUS_OK) {
         printf("%s should be allowed\n",  wcallowed[i]);
         ret++;
      }
   }

   if (ret == 0) {
      printf("PASS: test solved!\n");
   } else {
      printf("FAILED: test NOT solved!\n");
   }
   
   return ret;

}

