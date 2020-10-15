
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

#include "comm/lists/cl_lists.h"

#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "main()"
extern int main(void)
{
  int ret;
  char* data1 = "string_1";
  char* data2 = "string_2";
  char* data3 = "string_3";

  cl_raw_list_elem_t* elem = NULL;
  cl_raw_list_t* my_list = NULL;
  cl_raw_list_t* my_string_list = NULL;
  cl_string_list_elem_t* selem = NULL;

  printf("setting up list ...\n");
  if ( (ret=cl_raw_list_setup(&my_list,"my raw list",1)) != CL_RETVAL_OK) {
     printf("error: cl_raw_list_setup(): %d\n",ret);
  }
  
  printf("call lock list ...\n");
  if ((ret=cl_raw_list_lock(my_list)) != CL_RETVAL_OK) {
     printf("error: cl_raw_list_lock(): %d\n", ret);
  }


  printf("append elem ...\n");
  if (cl_raw_list_append_elem(my_list, data1) == NULL) {
     printf("error: cl_raw_list_append_elem()\n");
  }
  printf("append elem ...\n");
  if (cl_raw_list_append_elem(my_list, data2) == NULL) {
     printf("error: cl_raw_list_append_elem()\n");
  }
  printf("append elem ...\n");
  if (cl_raw_list_append_elem(my_list, data3) == NULL) {
     printf("error: cl_raw_list_append_elem()\n");
  }

  
  printf("search elem ...\n");
  elem = cl_raw_list_search_elem(my_list, data2);
  if (elem == NULL) {
     printf("error: cl_raw_list_search_elem(): elem not found!!!\n");
  } else {
     printf("next elem from %s : %s\n", (char*) elem->data  , (char*) (cl_raw_list_get_next_elem(elem))->data ); 
     printf("last elem from %s : %s\n", (char*) elem->data  , (char*) (cl_raw_list_get_last_elem(elem))->data ); 
  }

  printf("get first ...\n");
  elem = cl_raw_list_get_first_elem(my_list);
  while (elem) {
     printf("elem: %s\n", (char*)elem->data);
     printf("get next ...\n");
     elem = cl_raw_list_get_next_elem(elem);
  }

  printf("search elem 2 ...\n");
  elem = cl_raw_list_search_elem(my_list, data2);
  if (elem == NULL) {
     printf("error: cl_raw_list_search_elem(): elem not found!!!\n");
  }
  printf("removing elem 2 \n");
  if (cl_raw_list_remove_elem(my_list, elem) != data2) {
     printf("error: cl_raw_list_remove_elem()\n");
  }

  printf("get first ...\n");
  elem = cl_raw_list_get_first_elem(my_list);
  while (elem) {
     printf("elem: %s\n", (char*)elem->data);
     printf("get next ...\n");
     elem = cl_raw_list_get_next_elem(elem);
  }


  printf("removing elem\n");
  if (cl_raw_list_remove_elem(my_list, cl_raw_list_search_elem(my_list, data1)) != data1) {
     printf("error: cl_raw_list_remove_elem()\n");
  }
  printf("removing elem\n");
  if (cl_raw_list_remove_elem(my_list, cl_raw_list_search_elem(my_list, data3)) != data3) {
     printf("error: cl_raw_list_remove_elem()\n");
  }

  printf("removing not existing elem\n");
  if (cl_raw_list_remove_elem(my_list, cl_raw_list_search_elem(my_list, data3)) != NULL) {
     printf("error: cl_raw_list_remove_elem(): object should be unfindable\n");
  }
  

  printf("call unlock list ...\n");
  if ((ret=cl_raw_list_unlock(my_list)) != CL_RETVAL_OK) {
     printf("error: cl_raw_list_unlock(): %d\n", ret);
  }

  printf("cleanup list ...\n");
  if ((ret=cl_raw_list_cleanup(&my_list)) != CL_RETVAL_OK) {
     printf("error: cl_raw_list_cleanup(): %d\n", ret);
  }


  printf("testing string list ...\n");
  if ((ret=cl_string_list_setup(&my_string_list,"string_list")) != CL_RETVAL_OK) {
     printf("error: cl_string_list_setup(): %d\n", ret);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_string_list));   
  for (selem = cl_string_list_get_first_elem(my_string_list); selem != NULL ; selem = cl_string_list_get_next_elem(selem) ) {
     printf("elem: %s\n",selem->string);
  }

  printf("append string ...\n");
  if ((ret=cl_string_list_append_string(my_string_list,"blub", 1)) != CL_RETVAL_OK) {
     printf("error: cl_string_list_append_string(): %d\n", ret);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_string_list));   
  for (selem = cl_string_list_get_first_elem(my_string_list); selem != NULL ; selem = cl_string_list_get_next_elem(selem) ) {
     printf("elem: %s\n",selem->string);
  }

  printf("append string ...\n");
  if ((ret=cl_string_list_append_string(my_string_list,"blab", 1)) != CL_RETVAL_OK) {
     printf("error: cl_string_list_append_string(): %d\n", ret);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_string_list));   
  for (selem = cl_string_list_get_first_elem(my_string_list); selem != NULL ; selem = cl_string_list_get_next_elem(selem) ) {
     printf("elem: %s\n",selem->string);
  }

  printf("remove string ...\n");
  if ((ret=cl_string_list_remove_string(my_string_list,"blab", 1)) != CL_RETVAL_OK) {
     printf("error: cl_string_list_remove_string(): %d\n", ret);
  }

  printf("nr of elems: %ld\n", cl_raw_list_get_elem_count(my_string_list));   

  for (selem = cl_string_list_get_first_elem(my_string_list); selem != NULL ; selem = cl_string_list_get_next_elem(selem) ) {
     printf("elem: %s\n",selem->string);
  }

  
  printf("cleanup string list ...\n");
  if ((ret=cl_string_list_cleanup(&my_string_list)) != CL_RETVAL_OK) {
     printf("error: cl_string_list_cleanup(): %d\n", ret);
  }

  printf("main done\n");

  


  return 0;
}


