
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
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "comm/cl_commlib.h"
#include "comm/cl_host_alias_list.h"

extern int
main(int argc, char** argv)
{
  cl_raw_list_t* alias_list = NULL;
  cl_host_alias_list_elem_t* elem = NULL;
  char* help = NULL;
  int ret_val;



  printf("list setup ... ");
  ret_val = cl_host_alias_list_setup(&alias_list, "alias host list");
  printf("%s\n", cl_get_error_text(ret_val));

  printf("adding entries ... ");
  ret_val = cl_host_alias_list_append_host(alias_list, "one", "1", 1);
  printf("%s\n", cl_get_error_text(ret_val));
  printf("adding entries ... ");
  ret_val = cl_host_alias_list_append_host(alias_list, "two", "2", 1);
  printf("%s\n", cl_get_error_text(ret_val));
  printf("adding entries ... ");
  ret_val = cl_host_alias_list_append_host(alias_list, "three", "3", 1);
  printf("%s\n", cl_get_error_text(ret_val));
  printf("adding entries ... ");
  ret_val = cl_host_alias_list_append_host(alias_list, "four", "4", 1);
  printf("%s\n", cl_get_error_text(ret_val));

  printf("adding duplicate entry ...");
  ret_val = cl_host_alias_list_append_host(alias_list, "four", "5-1", 1);
  printf("%s\n", cl_get_error_text(ret_val));


  printf("first entry ...\n");  

  cl_raw_list_lock(alias_list);
  elem = cl_host_alias_list_get_first_elem(alias_list);
  while (elem) {
     printf("resolved host: \"%s\" has alias name \"%s\"\n", elem->local_resolved_hostname,elem->alias_name);
     elem = cl_host_alias_list_get_next_elem(elem);
  }

  printf("and the other way ...\n");
  elem = cl_host_alias_list_get_least_elem(alias_list);
  while (elem) {
     printf("resolved host: \"%s\" has alias name \"%s\"\n", elem->local_resolved_hostname,elem->alias_name);
     elem = cl_host_alias_list_get_last_elem(elem);
  }
  cl_raw_list_unlock(alias_list);

  printf("removing alias 2 ... ");
  elem = cl_host_alias_list_get_first_elem(alias_list);
  elem = cl_host_alias_list_get_next_elem(elem);
  ret_val = cl_host_alias_list_remove_host(alias_list,elem,1);
  printf("%s\n", cl_get_error_text(ret_val));

  printf("first entry ...\n");  

  cl_raw_list_lock(alias_list);
  elem = cl_host_alias_list_get_first_elem(alias_list);
  while (elem) {
     printf("resolved host: \"%s\" has alias name \"%s\"\n", elem->local_resolved_hostname,elem->alias_name);
     elem = cl_host_alias_list_get_next_elem(elem);
  }
  cl_raw_list_unlock(alias_list);

  printf("get undefined entry ... ");
  ret_val = cl_host_alias_list_get_alias_name(alias_list, "blub", &help);
  printf("%s\n", cl_get_error_text(ret_val));

  printf("get defined entry \"four\" ... ");
  ret_val = cl_host_alias_list_get_alias_name(alias_list, "four", &help);
  printf("%s\n", cl_get_error_text(ret_val));
  if (ret_val == CL_RETVAL_OK) {
     printf("alias name is \"%s\"\n", help);
     sge_free(&help);
  }

  printf("list cleanup ... ");
  ret_val = cl_host_alias_list_cleanup(&alias_list);
  printf("%s\n", cl_get_error_text(ret_val));
  
  printf("main done\n");
  return 0;
}
 




