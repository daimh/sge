
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

#include "comm/lists/cl_parameter_list.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(void)
{
  int ret;
  char* param1 = "param1";
  char* param2 = "param2";
  char* param3 = "param3";
  char* val1 = "val1";
  char* val2 = "val2";
  char* val3 = "val3";

  cl_raw_list_t* my_parameter_list = NULL;
  cl_parameter_list_elem_t* elem = NULL;


  printf("testing parameter list ...\n");
  if ((ret=cl_parameter_list_setup(&my_parameter_list,"parameter_list")) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_setup(): %d\n", ret);
     exit(1);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_parameter_list));   
  for (elem = cl_parameter_list_get_first_elem(my_parameter_list); elem != NULL ; elem = cl_parameter_list_get_next_elem(elem) ) {
     printf("parameter: %s\n",elem->parameter);
     printf("value: %s\n",elem->value);
  }

  printf("\nappend parameter ...\n");
  if ((ret=cl_parameter_list_append_parameter(my_parameter_list, param1, val1, 1)) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_append_parameter(): %d\n", ret);
     exit(1);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_parameter_list));   
  for (elem = cl_parameter_list_get_first_elem(my_parameter_list); elem != NULL ; elem = cl_parameter_list_get_next_elem(elem) ) {
     printf("Print elements:\n");
     printf("parameter: %s\n",elem->parameter);
     printf("value: %s\n",elem->value);
  }

  printf("\nappend parameter ...\n");
  if ((ret=cl_parameter_list_append_parameter(my_parameter_list, param2, val2, 1)) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_append_parameter(): %d\n", ret);
     exit(1);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_parameter_list));   
  for (elem = cl_parameter_list_get_first_elem(my_parameter_list); elem != NULL ; elem = cl_parameter_list_get_next_elem(elem) ) {
     printf("Print elements:\n");
     printf("parameter: %s\n",elem->parameter);
     printf("value: %s\n",elem->value);
  }

  printf("\nappend parameter ...\n");
  if ((ret=cl_parameter_list_append_parameter(my_parameter_list, param3, val3, 1)) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_append_parameter(): %d\n", ret);
     exit(1);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_parameter_list));   
  for (elem = cl_parameter_list_get_first_elem(my_parameter_list); elem != NULL ; elem = cl_parameter_list_get_next_elem(elem) ) {
     printf("Print elements:\n");
     printf("parameter: %s\n",elem->parameter);
     printf("value: %s\n",elem->value);
  }

  printf("\nremove parameter ...\n");
  if ((ret=cl_parameter_list_remove_parameter(my_parameter_list, param2, 1)) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_remove_parameter(): %d\n", ret);
     exit(1);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_parameter_list));   

  for (elem = cl_parameter_list_get_first_elem(my_parameter_list); elem != NULL ; elem = cl_parameter_list_get_next_elem(elem) ) {
     printf("Print elements:\n");
     printf("parameter: %s\n",elem->parameter);
     printf("value: %s\n",elem->value);
  }

  
  printf("\ncleanup parameter list ...\n");
  if ((ret=cl_parameter_list_cleanup(&my_parameter_list)) != CL_RETVAL_OK) {
     printf("error: cl_parameter_list_cleanup(): %d\n", ret);
     exit(1);
  }

  printf("main done\n");

  return 0;
}


