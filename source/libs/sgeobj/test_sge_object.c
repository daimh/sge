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

#include "uti/sge_string.h"

#include "cull/cull.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_object.h"
#include "sgeobj/sge_job.h"

static int exit_code = EXIT_SUCCESS;

static void 
check_result_bool(bool value, bool target, const char *function)
{
   if (value != target) {
      fprintf(stderr, "function "SFQ" failed\n", function);
      exit_code = EXIT_FAILURE;
   } else {
      fprintf(stdout, "function "SFQ" ok\n", function);
   }
}

static void 
check_result_int(int value, int target, const char *function)
{
   if (value != target) {
      fprintf(stderr, "function "SFQ" failed\n", function);
      exit_code = EXIT_FAILURE;
   } else {
      fprintf(stdout, "function "SFQ" ok\n", function);
   }
}

static void 
check_result_string(const char * value, const char *target, const char *function)
{
   if (sge_strnullcmp(value, target) != 0) {
      fprintf(stderr, "function "SFQ" failed\n", function);
      exit_code = EXIT_FAILURE;
   } else {
      fprintf(stdout, "function "SFQ" ok\n", function);
   }
}

static void 
check_result_pointer(const void *value, const void *target, 
                     const char *function)
{
   if (value != target) {
      fprintf(stderr, "function "SFQ" failed\n", function);
      exit_code = EXIT_FAILURE;
   } else {
      fprintf(stdout, "function "SFQ" ok\n", function);
   }
}

int main(int argc, char *argv[])
{
   bool ret_bool;
   int ret_int;
   const char *ret_string;
   const void *ret_pointer;
   
   lListElem *queue = NULL;
   lList *pe_list = NULL;

   dstring buffer = DSTRING_INIT;

   lInit(nmv);

   queue = lCreateElem(QU_Type);
   lAddElemStr(&pe_list, ST_name, "test pe", ST_Type); 

   /* object_has_type */
   ret_bool = object_has_type(queue, QU_Type);
   check_result_bool(ret_bool, true, "object_has_type");
   ret_bool = object_has_type(queue, JB_Type);
   check_result_bool(ret_bool, false, "object_has_type");

   ret_bool = object_has_type(NULL, JB_Type);
   check_result_bool(ret_bool, false, "object_has_type");
   ret_bool = object_has_type(queue, NULL);
   check_result_bool(ret_bool, false, "object_has_type");

   /* object_get_type */
   ret_pointer = object_get_type(queue);
   check_result_pointer(ret_pointer, QU_Type, "object_get_type");
   ret_pointer = object_get_type(NULL);
   check_result_pointer(ret_pointer, NULL, "object_get_type");

   /* object_get_subtype */
   ret_pointer = object_get_subtype(QU_acl);
   check_result_pointer(ret_pointer, US_Type, "object_get_subtype");
   ret_pointer = object_get_subtype(QU_qname);
   check_result_pointer(ret_pointer, NULL, "object_get_subtype");
   ret_pointer = object_get_subtype(NoName);
   check_result_pointer(ret_pointer, NULL, "object_get_subtype");

   /* object_get_primary_key */
   ret_int = object_get_primary_key(JB_Type);
   check_result_int(ret_int, JB_job_number, "object_get_primary_key");

   ret_int = object_get_primary_key(NULL);
   check_result_int(ret_int, NoName, "object_get_primary_key");

   /* object_get_name_prefix */
   ret_string = object_get_name_prefix(QU_Type, &buffer);
   check_result_string(ret_string, "QU_", "object_get_name_prefix");
   ret_string = object_get_name_prefix(JB_Type, &buffer);
   check_result_string(ret_string, "JB_", "object_get_name_prefix");

   ret_string = object_get_name_prefix(NULL, &buffer);
   check_result_string(ret_string, NULL, "object_get_name_prefix");
   ret_string = object_get_name_prefix(JB_Type, NULL);
   check_result_string(ret_string, NULL, "object_get_name_prefix");

   sge_dstring_free(&buffer);

   /* object_[gs]et_field_contents is tested in test_sge_spooling_utilities */

   /* test object verification */
   /* NULL pointer actions */
   ret_bool = object_verify_cull(NULL, NULL);
   check_result_bool(ret_bool, false, "object_verify_cull");
   ret_bool = object_list_verify_cull(NULL, NULL);
   check_result_bool(ret_bool, false, "object_list_verify_cull");
 
   /* object type verification */
   ret_bool = object_verify_cull(queue, NULL);
   check_result_bool(ret_bool, true, "object_verify_cull");
   ret_bool = object_verify_cull(queue, QU_Type);
   check_result_bool(ret_bool, true, "object_verify_cull");
   ret_bool = object_verify_cull(queue, JB_Type);
   check_result_bool(ret_bool, false, "object_verify_cull");

   /* list verification */
   ret_bool = object_list_verify_cull(pe_list, NULL);
   check_result_bool(ret_bool, true, "object_verify_cull");
   ret_bool = object_list_verify_cull(pe_list, ST_Type);
   check_result_bool(ret_bool, true, "object_verify_cull");
   ret_bool = object_list_verify_cull(pe_list, QU_Type);
   check_result_bool(ret_bool, false, "object_verify_cull");


   /* object sublist verification */
   lSetList(queue, QU_pe_list, pe_list);
   ret_bool = object_verify_cull(queue, QU_Type);
   check_result_bool(ret_bool, true, "object_verify_cull");

   lSetList(queue, QU_owner_list, lCopyList("pe list copy", pe_list));
   ret_bool = object_verify_cull(queue, QU_Type);
   check_result_bool(ret_bool, false, "object_verify_cull");

   lFreeElem(&queue);

   return exit_code;
}
