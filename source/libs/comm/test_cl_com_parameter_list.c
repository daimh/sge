
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

#include "comm/cl_commlib.h"
#include "comm/lists/cl_parameter_list.h"

#include "uti/sge_stdlib.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(void)
{
  int retval;
  const char* param2 = "param2";
  const char* param3 = "param3";
  char* val2 = "val2";
  char* val3 = "val3";
  char* ret_char = NULL;

   printf("commlib setup ...\n");
   retval = cl_com_setup_commlib(CL_NO_THREAD , CL_LOG_INFO, NULL );
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   retval = cl_com_set_parameter_list_value(param2, val2);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   retval = cl_com_set_parameter_list_value(param3, val3);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   retval = cl_com_get_parameter_list_value(param3, &ret_char);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   retval = cl_com_remove_parameter_list_value(param2);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   printf("Print element got from commlib:\n");
   printf("parameter: %s\n", param3);
   printf("value: %s\n", ret_char);

   sge_free(&ret_char);

   printf("\nappend parameter ...\n");
   retval = cl_com_set_parameter_list_value(param2, val2);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   printf("\nappend parameter ...\n");
   retval = cl_com_set_parameter_list_value("param11", "val11");
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   retval = cl_com_get_parameter_list_string(&ret_char);
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }
   printf("Print string got from commlib:\n");
   printf("Parameter string: %s\n", ret_char);

   sge_free(&ret_char);

   printf("commlib cleanup ...\n");
   retval = cl_com_cleanup_commlib();
   if (retval != CL_RETVAL_OK) {
      printf("%s\n\n",cl_get_error_text(retval));
      exit(1);
   }

   printf("main done\n");

   return 0;
}
