
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
#include <stdlib.h>

#include "comm/dummy.h"
#include "comm/lists/cl_lists.h"


#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
int main(int argc, char **argv)
{
   int test = 1;
   int my_error = 0;
   cl_raw_list_t* list = NULL;
   cl_raw_list_t* log_list = NULL;

   my_error = cl_log_list_setup(&log_list, "test_dummy", 0, CL_LOG_FLUSHED , NULL);
   printf("log list setup: %s\n", cl_get_error_text(my_error));
   cl_log_list_set_log_level(log_list,CL_LOG_DEBUG);
   
   cl_raw_list_setup(&list, "dummy_list",1);
   cl_raw_list_append_elem( list , (void*) &test);
   printf("list entries: %ld\n", cl_raw_list_get_elem_count(list));
   
   my_error = CL_LOG(CL_LOG_INFO,"hallo");
   printf("log: %s\n", cl_get_error_text(my_error));

   printf("log list entries: %ld\n", cl_raw_list_get_elem_count(log_list));

   cl_raw_list_remove_elem( list , cl_raw_list_search_elem( list, (void*) &test) );

   printf("list entries: %ld\n", cl_raw_list_get_elem_count(list));
   my_error = cl_raw_list_cleanup(&list);
   if ( my_error != CL_RETVAL_OK) {
      printf("error cl_raw_list_cleanup() -> %s\n", cl_get_error_text(my_error));
      exit(1);
   }
   
   while (argc>1) {
      dummy(argv[argc-1]);
      argc--;
   }
   
   cl_log_list_cleanup(&log_list);
   return 0;
}
