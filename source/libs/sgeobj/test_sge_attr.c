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
#include "sgeobj/sge_href.h"
#include "sgeobj/sge_hgroup.h"
#include "sgeobj/sge_attr.h"

bool check_attr_str_list_find_value(void) 
{
   bool ret = false;
   lList *attr_list = NULL;
   lList *answer_list = NULL;
   lList *ambiguous_hostref_list = NULL;
   lListElem *attr = NULL;

   {
      lList *hostref_list1 = NULL;
      lList *hostref_list2 = NULL;
      lList *hostref_list3 = NULL;
      lListElem *hgroup1;
      lListElem *hgroup2;
      lListElem *hgroup3;

      hostref_list_add(&hostref_list1, NULL, "a");
      hostref_list_add(&hostref_list1, NULL, "b");
      hostref_list_add(&hostref_list1, NULL, "c");
      hostref_list_add(&hostref_list1, NULL, "d");
      hostref_list_add(&hostref_list1, NULL, "e");

      hostref_list_add(&hostref_list2, NULL, "f");
      hostref_list_add(&hostref_list2, NULL, "a");
      hostref_list_add(&hostref_list2, NULL, "b");
      hostref_list_add(&hostref_list2, NULL, "g");
      hostref_list_add(&hostref_list2, NULL, "c");

      hostref_list_add(&hostref_list3, NULL, "f");
      hostref_list_add(&hostref_list3, NULL, "g");
      hostref_list_add(&hostref_list3, NULL, "h");
      hostref_list_add(&hostref_list3, NULL, "i");
      hostref_list_add(&hostref_list3, NULL, "j");
   
      hgroup1 = hgroup_create(NULL, "@A", hostref_list1);
      hgroup2 = hgroup_create(NULL, "@B", hostref_list2);
      hgroup3 = hgroup_create(NULL, "@C", hostref_list3);
      *(hgroup_list_get_master_list()) = lCreateList("", HGRP_Type);
      lAppendElem(*(hgroup_list_get_master_list()), hgroup1);
      lAppendElem(*(hgroup_list_get_master_list()), hgroup2);
      lAppendElem(*(hgroup_list_get_master_list()), hgroup3);
   }
   return ret;
}

int main(int argc, char *argv[])
{
   bool failed = false;

   DENTER_MAIN(TOP_LAYER, "execd");

   lInit(nmv);

   if (!failed) {
      failed = check_attr_str_list_find_value();
   }

   return failed ? 1 : 0;
}

