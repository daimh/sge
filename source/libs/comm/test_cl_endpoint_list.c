
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
#include "comm/cl_endpoint_list.h"

#include "uti/sge_profiling.h"




extern int
main(int argc, char** argv)
{
  int retval = 0;
  int arg = 2;
  int touches = 5;
  int service_port = 0;
  char* local_host = NULL;
  struct in_addr in_addr;
  cl_raw_list_t* endpoint_list = NULL;
  cl_endpoint_list_data_t* list_data = NULL;
  cl_endpoint_list_elem_t* elem = NULL;
  cl_com_endpoint_t* last_endpoint = NULL;
  cl_com_handle_t* handle = NULL;

  in_addr.s_addr = 0;

  
  if (argc < 3) {
      printf("usage: test_cl_endpoint_list <DEBUGLEVEL> endpoint_names\n");
      exit(1);
  }
  
  prof_mt_init();

  printf("\nfirst testing only endpoint list functionality ...\n");
  printf(  "================================================================\n");

  sleep(4);

  printf("commlib setup ...\n");
  retval = cl_com_setup_commlib(CL_NO_THREAD , (cl_log_t)atoi(argv[1]), NULL);
  printf("%s\n\n",cl_get_error_text(retval));


  printf("setup endpoint list ...\n");
  retval = cl_endpoint_list_setup(&endpoint_list, "endpoint list", 5, 2, true);
  printf("%s\n\n",cl_get_error_text(retval));

 
  while( argv[arg] != NULL) {
     cl_com_endpoint_t* new = NULL;

     printf("append \"%s\" (static):\n", argv[arg]);
     new = cl_com_create_endpoint(argv[arg], "name", 1, &in_addr);
     retval = cl_endpoint_list_define_endpoint(endpoint_list, new, 1024+ arg, CL_CM_AC_DISABLED, true);
     printf("%s\n\n",cl_get_error_text(retval));
     retval = cl_endpoint_list_define_endpoint(endpoint_list, new, 1024+ arg, CL_CM_AC_DISABLED, true);
     printf("%s\n\n",cl_get_error_text(retval));

     cl_com_free_endpoint(&new);

     

     printf("append \"%s\" (non static):\n", argv[arg]);
     new = cl_com_create_endpoint(argv[arg], "name", 2, &in_addr);
     retval = cl_endpoint_list_define_endpoint(endpoint_list, new, 1024 +arg,CL_CM_AC_DISABLED, false);
     printf("%s\n\n",cl_get_error_text(retval));
     cl_com_free_endpoint(&new);

     arg++;
  }
 
  arg--;
  last_endpoint = cl_com_create_endpoint(argv[arg], "name", 2, &in_addr);


  list_data = cl_endpoint_list_get_data(endpoint_list);
  if (list_data != NULL) {
     printf("list data:\n");
     printf("entry life time:   %ld\n", list_data->entry_life_time );
     printf("refresh interval:  %ld\n", list_data->refresh_interval );
     printf("last refresh time: %ld\n", list_data->last_refresh_time );
     
  } else {
     printf("ERROR: could not get list data");
     sge_prof_cleanup();
     exit(1);
  }
  
  cl_raw_list_lock(endpoint_list);
  elem = cl_endpoint_list_get_first_elem(endpoint_list);
  while(elem) {
     printf("actual endpoint: %s/%s/%ld (service:%d) (is_static:%d) (last_used:%ld)\n", 
            elem->endpoint->comp_host, 
            elem->endpoint->comp_name, 
            elem->endpoint->comp_id,
            elem->service_port,
            elem->is_static,
            elem->last_used );
     elem = cl_endpoint_list_get_next_elem(elem);
  }
  cl_raw_list_unlock(endpoint_list);

  
  arg = (int)cl_raw_list_get_elem_count(endpoint_list);
  arg = arg/2;
  while(cl_raw_list_get_elem_count(endpoint_list) != arg ) {
     printf("waiting for list count getting %d\n", arg);
     printf("elements in list: %ld\n", cl_raw_list_get_elem_count(endpoint_list));
     sleep(1);

     cl_raw_list_lock(endpoint_list);
     elem = cl_endpoint_list_get_first_elem(endpoint_list);
     while(elem) {
        printf("actual endpoint: %s/%s/%ld (service:%d) (is_static:%d) (last_used:%ld)\n", 
            elem->endpoint->comp_host, 
            elem->endpoint->comp_name, 
            elem->endpoint->comp_id,
            elem->service_port,
            elem->is_static,
            elem->last_used );
        elem = cl_endpoint_list_get_next_elem(elem);
     }
     cl_raw_list_unlock(endpoint_list);



     if ( cl_raw_list_get_elem_count(endpoint_list) == arg + 1 && touches > 0) {
        touches--;
     }

     if (touches) {
        printf("touch endpoint %s/%s/%ld:\n",last_endpoint->comp_host,last_endpoint->comp_name,last_endpoint->comp_id );
        retval = cl_endpoint_list_define_endpoint(endpoint_list, last_endpoint, 1024+arg, CL_CM_AC_DISABLED, false);
        printf("%s\n\n",cl_get_error_text(retval));
     }

     printf("try to get port for endpoint %s/%s/%ld:\n",last_endpoint->comp_host,last_endpoint->comp_name,last_endpoint->comp_id );
     retval = cl_endpoint_list_get_service_port(endpoint_list, last_endpoint, &service_port);
     printf("%s\n",cl_get_error_text(retval));
     printf("-> port is %d\n\n", service_port );

     cl_com_endpoint_list_refresh(endpoint_list);
  }

  printf("touch endpoint %s/%s/%ld:\n",last_endpoint->comp_host,last_endpoint->comp_name,last_endpoint->comp_id );
  retval = cl_endpoint_list_get_service_port(endpoint_list,last_endpoint, &service_port );

  if (retval == CL_RETVAL_OK) {
     printf("error this endpoint should not be in list\n");
     sge_prof_cleanup();
     exit(1);
  } else {
     printf("%s (hint: last action must have produced an error)\n\n",cl_get_error_text(retval));
  }

  printf("try to get port for endpoint %s/%s/%ld:\n",last_endpoint->comp_host,last_endpoint->comp_name,last_endpoint->comp_id );
  retval = cl_endpoint_list_get_service_port(endpoint_list, last_endpoint, &service_port);
  if (retval == CL_RETVAL_OK) {
     printf("error this endpoint should not be in list\n");
     sge_prof_cleanup();
     exit(1);
  } else {
     printf("%s (hint: last action must have produced an error)\n\n",cl_get_error_text(retval));
  }

  

  cl_com_free_endpoint(&last_endpoint);


  arg=2;
  while( argv[arg] != NULL) {
     cl_com_endpoint_t* new = NULL;
     printf("elements in list: %ld\n", cl_raw_list_get_elem_count(endpoint_list));
     printf("delete %s/%s/%ld (static):\n", argv[arg], "name", (unsigned long)1);
     new = cl_com_create_endpoint(argv[arg], "name", 1, &in_addr);
     retval = cl_endpoint_list_undefine_endpoint(endpoint_list,new);
     printf("%s\n\n",cl_get_error_text(retval));
     cl_com_free_endpoint(&new);
     arg++;
     printf("elements in list: %ld\n", cl_raw_list_get_elem_count(endpoint_list));
  }


  printf("cleanup endpoint list ...\n");
  retval = cl_endpoint_list_cleanup(&endpoint_list);
  printf("%s\n\n",cl_get_error_text(retval));
  
 
  printf("commlib cleanup ...\n");
  retval = cl_com_cleanup_commlib();
  printf("%s\n\n",cl_get_error_text(retval));


  printf("\nnow testing commlib endpoint list functionality without threads ...\n");
  printf(  "===================================================================\n");
  sleep(4);

  printf("commlib setup ...\n");
  retval = cl_com_setup_commlib(CL_NO_THREAD , (cl_log_t)atoi(argv[1]), NULL);
  printf("%s\n\n",cl_get_error_text(retval));

  handle = cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , true, 4545 , CL_TCP_DEFAULT,"client", 1,1,0 );
  if (handle == NULL) {
     printf("could not get handle\n");
     sge_prof_cleanup();
     exit(1);
  }



  cl_com_gethostname(&local_host , NULL, NULL, NULL);
  
  list_data = cl_endpoint_list_get_data(cl_com_get_endpoint_list());
  if (list_data) {
     printf("setting entry life time to 10 seconds\n");
     list_data->entry_life_time = 10;
  }

  printf("append endpoint host/name/1 on (port 1024) (not static)...\n");
  retval = cl_com_append_known_endpoint_from_name(local_host,"name",1,1024,CL_CM_AC_DISABLED, false);
  printf("%s\n\n",cl_get_error_text(retval));

  printf("append endpoint host/name/2 on (port 1025) (static)...\n");
  retval = cl_com_append_known_endpoint_from_name(local_host,"name",2,1025,CL_CM_AC_DISABLED, true);
  printf("%s\n\n",cl_get_error_text(retval));

  printf("try to get port for endpoint host/name/1...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1024 ) {
     printf("port is not 1024\n");
     sge_prof_cleanup();
     exit(1);
  }

  printf("try to get port for endpoint host/name/2...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",2,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1025 ) {
     printf("port is not 1025\n");
     sge_prof_cleanup();
     exit(1);
  }
  
  
  service_port = 0;

  while ( (retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port) ) == CL_RETVAL_OK ) {
     printf("try to get port for endpoint host/name/1...\n");
     printf("port is %d\n",service_port);
     printf("%s\n\n",cl_get_error_text(retval));
     cl_commlib_trigger(handle, 1);
     service_port = 0;
  }

  cl_commlib_shutdown_handle(handle, false);

  printf("try to get port for endpoint host/name/1...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n",cl_get_error_text(retval));
  printf("(last error was expected!)\n");

  printf("try to get port for endpoint host/name/2...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",2,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1025 ) {
     printf("port is not 1025\n");
     sge_prof_cleanup();
     exit(1);
  }

  if (cl_raw_list_get_elem_count(  cl_com_get_endpoint_list() ) != 1 ) {
     printf("error: number of endpoint entries should be 1\n");
     sge_prof_cleanup();
     exit(1);
  }

  printf("remove known endpoint ... host/name/2\n");
  retval = cl_com_remove_known_endpoint_from_name(local_host,"name",2);
  printf("%s\n\n",cl_get_error_text(retval));

  if (cl_raw_list_get_elem_count(  cl_com_get_endpoint_list() ) != 1 ) {
     printf("error: number of endpoint entries should be 1, but is %ld\n",cl_raw_list_get_elem_count(  cl_com_get_endpoint_list() ) );
     sge_prof_cleanup();
     exit(1);
  }

  printf("commlib cleanup ...\n");
  retval = cl_com_cleanup_commlib();
  printf("%s\n\n",cl_get_error_text(retval));


  printf("\nnow testing commlib endpoint list functionality with threads ...\n");
  printf(  "================================================================\n");
  sleep(4);
  
    printf("commlib setup ...\n");
  retval = cl_com_setup_commlib(CL_RW_THREAD , (cl_log_t)atoi(argv[1]), NULL);
  printf("%s\n\n",cl_get_error_text(retval));

  handle = cl_com_create_handle(NULL,CL_CT_TCP,CL_CM_CT_MESSAGE , true, 4545 , CL_TCP_DEFAULT,"client", 1,1, false );
  if (handle == NULL) {
     printf("could not get handle\n");
     sge_prof_cleanup();
     exit(1);
  }



  cl_com_gethostname(&local_host , NULL, NULL, NULL);
  
  list_data = cl_endpoint_list_get_data(cl_com_get_endpoint_list());
  if (list_data) {
     printf("setting entry life time to 10 seconds\n");
     list_data->entry_life_time = 10;
  }

  printf("append endpoint host/name/1 on (port 1024) (not static)...\n");
  retval = cl_com_append_known_endpoint_from_name(local_host,"name",1,1024,CL_CM_AC_DISABLED, false);
  printf("%s\n\n",cl_get_error_text(retval));

  printf("append endpoint host/name/2 on (port 1025) (static)...\n");
  retval = cl_com_append_known_endpoint_from_name(local_host,"name",2,1025,CL_CM_AC_DISABLED, true);
  printf("%s\n\n",cl_get_error_text(retval));

  printf("try to get port for endpoint host/name/1...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1024 ) {
     printf("port is not 1024\n");
     sge_prof_cleanup();
     exit(1);
  }

  printf("try to get port for endpoint host/name/2...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",2,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1025 ) {
     printf("port is not 1025\n");
     sge_prof_cleanup();
     exit(1);
  }
  
  
  service_port = 0;

  while ( (retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port) ) == CL_RETVAL_OK ) {
     printf("try to get port for endpoint host/name/1...\n");
     printf("port is %d\n",service_port);
     printf("%s\n\n",cl_get_error_text(retval));
     cl_commlib_trigger(handle, 1);
     service_port = 0;
  }

  cl_commlib_shutdown_handle(handle, false);

  printf("try to get port for endpoint host/name/1...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",1,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n",cl_get_error_text(retval));
  printf("(last error was expected!)\n");

  printf("try to get port for endpoint host/name/2...\n");
  service_port = 0;
  retval = cl_com_get_known_endpoint_port_from_name(local_host,"name",2,&service_port);
  printf("port is %d\n",service_port);
  printf("%s\n\n",cl_get_error_text(retval));
  if ( service_port != 1025 ) {
     printf("port is not 1025\n");
     sge_prof_cleanup();
     exit(1);
  }

  if (cl_raw_list_get_elem_count(  cl_com_get_endpoint_list() ) != 1 ) {
     printf("error: number of endpoint entries should be 1\n");
     sge_prof_cleanup();
     exit(1);
  }

  printf("remove known endpoint ... host/name/2\n");
  retval = cl_com_remove_known_endpoint_from_name(local_host,"name",2);
  printf("%s\n\n",cl_get_error_text(retval));

  if (cl_raw_list_get_elem_count(  cl_com_get_endpoint_list() ) != 1 ) {
     printf("error: number of endpoint entries should be 1\n");
     sge_prof_cleanup();
     exit(1);
  }

  printf("commlib cleanup ...\n");
  retval = cl_com_cleanup_commlib();
  printf("%s\n\n",cl_get_error_text(retval));

  printf("main done\n");
  return 0;
}
 




