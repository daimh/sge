
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
#include <sys/time.h>
#include <sys/resource.h>

#include "comm/cl_commlib.h"
#include "comm/lists/cl_log_list.h"
#include "comm/cl_endpoint_list.h"

#include "uti/sge_profiling.h"
#define CL_DO_SLOW 0

void sighandler_issue_tests(int sig);

static int do_shutdown = 0;

void sighandler_issue_tests(int sig) {
   if (sig == SIGPIPE) {
      return;
   }

   if (sig == SIGHUP) {
      return;
   }

   do_shutdown = 1;
}



extern int main(int argc, char** argv)
{
  struct sigaction sa;

  cl_com_handle_t* handle = NULL; 
  cl_com_handle_t* handle1 = NULL; 
  cl_com_handle_t* handle2 = NULL; 

  cl_com_message_t* message = NULL;
  cl_com_endpoint_t* sender = NULL;
  int i;
  cl_log_t log_level;
  cl_framework_t framework = CL_CT_TCP;
  bool server_mode = false;
  int com_port = 0;
  char* com_host = NULL;
  int main_return = 0;
#if defined(IRIX)
  struct rlimit64 test_issues_limits;
#else
  struct rlimit test_issues_limits;
#endif

  
  if (argc < 4) {
     printf("param1=debug_level param2=framework(TCP/SSL) param3=server/client param4=port param5=hostname\n");
     exit(1);
  }

  if (argv[2]) {
     framework = CL_CT_UNDEFINED;
     if (strcmp(argv[2], "TCP") == 0) {
        framework=CL_CT_TCP;
        printf("using TCP framework\n");
     }
     if (strcmp(argv[2], "SSL") == 0) {
        framework=CL_CT_SSL;
        printf("using SSL framework\n");
     }
     if (framework == CL_CT_UNDEFINED) {
        printf("unexpected framework type\n");
        exit(1);
     }
  }

  if (argv[3]) {
     if (strcmp(argv[3], "server") == 0) {
        server_mode=true;
     }
  }

  if (argv[4]) {
     com_port = atoi(argv[4]);
  }

  if (argv[5]) {
     com_host = argv[5];
  }

  prof_mt_init();

  /* setup signalhandling */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sighandler_issue_tests;  /* one handler for all signals */
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);

#if defined(RLIMIT_VMEM) 
#if defined(IRIX)
   getrlimit64(RLIMIT_VMEM, &test_issues_limits);
#else
   getrlimit(RLIMIT_VMEM, &test_issues_limits);
#endif 
   test_issues_limits.rlim_max = RLIM_INFINITY;
   test_issues_limits.rlim_cur = RLIM_INFINITY;
#if defined(IRIX)
   setrlimit64(RLIMIT_VMEM, &test_issues_limits);
#else
   setrlimit(RLIMIT_VMEM, &test_issues_limits);
#endif

#else  /* if defined(RLIMIT_VMEM) */
#if defined(RLIMIT_AS)
#if defined(IRIX)
   getrlimit64(RLIMIT_AS, &test_issues_limits);
#else
   getrlimit(RLIMIT_AS, &test_issues_limits);
#endif 
   test_issues_limits.rlim_max = RLIM_INFINITY;
   test_issues_limits.rlim_cur = RLIM_INFINITY;
#if defined(IRIX)
   setrlimit64(RLIMIT_AS, &test_issues_limits);
#else
   setrlimit(RLIMIT_AS, &test_issues_limits);
#endif
#endif /* if defined(RLIMIT_AS) */
#endif /* if defined(RLIMIT_VMEM) */
  
#if defined(RLIMIT_VMEM) 
#if defined(IRIX)
   getrlimit64(RLIMIT_VMEM, &test_issues_limits);
#else
   getrlimit(RLIMIT_VMEM, &test_issues_limits);
#endif 
   printf("vmem limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);
#else  /* if defined(RLIMIT_VMEM) */
#if defined(RLIMIT_AS)
#if defined(IRIX)
   getrlimit64(RLIMIT_AS, &test_issues_limits);
#else
   getrlimit(RLIMIT_AS, &test_issues_limits);
#endif 
   printf("vmem limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);
#endif /* if defined(RLIMIT_AS) */
#endif /* if defined(RLIMIT_VMEM) */
 

#if defined(IRIX)
   getrlimit64(RLIMIT_STACK, &test_issues_limits);
#else
   getrlimit(RLIMIT_STACK, &test_issues_limits);
#endif 
   test_issues_limits.rlim_cur = test_issues_limits.rlim_max;
#if defined(IRIX)
   setrlimit64(RLIMIT_STACK, &test_issues_limits);
#else
   setrlimit(RLIMIT_STACK, &test_issues_limits);
#endif

#if defined(IRIX)
   getrlimit64(RLIMIT_STACK, &test_issues_limits);
#else
   getrlimit(RLIMIT_STACK, &test_issues_limits);
#endif 
   printf("stacksize limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);

#if defined(IRIX)
   getrlimit64(RLIMIT_DATA, &test_issues_limits);
#else
   getrlimit(RLIMIT_DATA, &test_issues_limits);
#endif 
   test_issues_limits.rlim_max = RLIM_INFINITY;
   test_issues_limits.rlim_cur = RLIM_INFINITY;
#if defined(IRIX)
   setrlimit64(RLIMIT_DATA, &test_issues_limits);
#else
   setrlimit(RLIMIT_DATA, &test_issues_limits);
#endif

#if defined(IRIX)
   getrlimit64(RLIMIT_DATA, &test_issues_limits);
#else
   getrlimit(RLIMIT_DATA, &test_issues_limits);
#endif 
   printf("data size limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);


#if defined(RLIMIT_RSS)
#if defined(IRIX)
   getrlimit64(RLIMIT_RSS, &test_issues_limits);
#else
   getrlimit(RLIMIT_RSS, &test_issues_limits);
#endif 
   test_issues_limits.rlim_cur = test_issues_limits.rlim_max;
#if defined(IRIX)
   setrlimit64(RLIMIT_RSS, &test_issues_limits);
#else
   setrlimit(RLIMIT_RSS, &test_issues_limits);
#endif

#if defined(IRIX)
   getrlimit64(RLIMIT_RSS, &test_issues_limits);
#else
   getrlimit(RLIMIT_RSS, &test_issues_limits);
#endif 
   printf("residet set size limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);
#endif


#if defined(IRIX)
   getrlimit64(RLIMIT_FSIZE, &test_issues_limits);
#else
   getrlimit(RLIMIT_FSIZE, &test_issues_limits);
#endif 
   test_issues_limits.rlim_cur = test_issues_limits.rlim_max;
#if defined(IRIX)
   setrlimit64(RLIMIT_FSIZE, &test_issues_limits);
#else
   setrlimit(RLIMIT_FSIZE, &test_issues_limits);
#endif

#if defined(IRIX)
   getrlimit64(RLIMIT_FSIZE, &test_issues_limits);
#else
   getrlimit(RLIMIT_FSIZE, &test_issues_limits);
#endif 
   printf("file size size limit is set to %ld\n", (unsigned long) test_issues_limits.rlim_cur);



  printf("commlib setup ...\n");
  /* this is for compiler warning on irix65 */

  switch (atoi(argv[1])) {
     case 0:
        log_level=CL_LOG_OFF;
        break;
     case 1:
        log_level=CL_LOG_ERROR;
        break;
     case 2:
        log_level=CL_LOG_WARNING;
        break;
     case 3:
        log_level=CL_LOG_INFO;
        break;
     case 4:
        log_level=CL_LOG_DEBUG;
        break;
     default:
        log_level=CL_LOG_OFF;
        break;
  }
  cl_com_setup_commlib(CL_RW_THREAD, log_level, NULL);

  if (server_mode == true) {
     handle=cl_com_create_handle(NULL, framework, CL_CM_CT_MESSAGE, true, com_port, CL_TCP_DEFAULT, "server", 1, 1, 0 );
     cl_com_set_max_connection_close_mode(handle,CL_ON_MAX_COUNT_DISABLE_ACCEPT);
  } else {
     handle=cl_com_create_handle(NULL, framework, CL_CM_CT_MESSAGE, false, com_port, CL_TCP_DEFAULT, "client", 0, 1, 0 );
  }

#define TEST_ISSUES_READ_WRITE_TIMEOUT 3
  if (handle == NULL) {
     printf("could not get handle\n");
     sge_prof_cleanup();
     exit(101);
  } else {
     /* This is a "hack" to set the read/write timeout to only 4 seconds */
     handle->read_timeout = TEST_ISSUES_READ_WRITE_TIMEOUT;
     handle->write_timeout = TEST_ISSUES_READ_WRITE_TIMEOUT;
  }

  cl_com_get_service_port(handle,&i), 
  printf("running on host \"%s\", port %d, component name is \"%s\", id is %ld\n", 
         handle->local->comp_host, 
         i, 
         handle->local->comp_name,  
         handle->local->comp_id);

  if (server_mode == true) {
     int actual_issue = 0;
     int max_con_test_count = 2;
     unsigned long max_connection_count;

     while (do_shutdown != 1) {
        int ret_val;

        if (actual_issue == 1400) {
           cl_raw_list_lock(handle->connection_list);
           printf("actual connection count     = %lu\n", cl_raw_list_get_elem_count(handle->connection_list) );
           printf("actual max connection count = %lu\n", handle->max_open_connections);
           if (cl_raw_list_get_elem_count(handle->connection_list) > handle->max_open_connections) {
              printf("issue 1400 error found\n");
              main_return = 1;
           }
           cl_raw_list_unlock(handle->connection_list);
        }

        cl_commlib_trigger(handle, 1); 
   
        ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
        if (message != NULL) {
           int do_reply = 1;
           CL_LOG_STR(CL_LOG_INFO,"received message from",sender->comp_host);
           if (strstr((char*)message->message,"#1400i") && actual_issue != 1400) {
              printf("switching to test #1400\n");
              do_reply = 0;
              actual_issue = 1400;
              cl_com_get_max_connections(handle,&max_connection_count);
              printf("setting max_connection_count from %lu to %lu\n", (unsigned long)max_connection_count, (unsigned long)max_con_test_count);
              cl_com_set_max_connections(handle,max_con_test_count);
           } 

           if (strstr((char*)message->message,"#1400r")) {
              printf("resetting to defaults\n");
              do_reply = 0;
              actual_issue = 0;
              printf("setting max_connection_count from %lu to %lu\n", (unsigned long)max_con_test_count, (unsigned long)max_connection_count);
              cl_com_set_max_connections(handle,max_connection_count);
           } 


           if (do_reply == 1) {
              ret_val = cl_commlib_send_message(handle, 
                                sender->comp_host, 
                                sender->comp_name, 
                                sender->comp_id, CL_MIH_MAT_NAK,  
                                &message->message, 
                                message->message_length, 
                                NULL, 0,0, 
                                false, false);
              if (ret_val != CL_RETVAL_OK) {
                 CL_LOG_STR(CL_LOG_ERROR,"cl_commlib_send_message() returned:",cl_get_error_text(ret_val));
              }
           }
           cl_com_free_message(&message);
           cl_com_free_endpoint(&sender);
        } 
     }
  } else {
     int ret_val;
     char* data = NULL;
     unsigned long data_size = 1024*1024;
     struct timeval now;
     struct timeval start;
     int runtime = 0;
     char iz1400_data[7] = "#1400i";
     cl_byte_t* iz1400_pointer = (cl_byte_t *)iz1400_data;
     char test_data[5] = "test";
     cl_byte_t* test_pointer = (cl_byte_t *)test_data;
     int timeout_reached_count = 0;

     printf("\ntesting issue #1389 ...\n");

     
     while (timeout_reached_count < 5 && do_shutdown == 0 && main_return == 0) {
        if (data_size > 1024*1024*1024) {
           printf("skip malloc() more than 1GB\n");
           printf("issue #1389 failed\n");
           main_return = 1;
           break;
        }
        data = sge_malloc(data_size * sizeof(char));
        memset(data, 0, data_size * sizeof(char));
        if (data == NULL) {
           printf("malloc() error: can't malloc(%ld)\n", data_size * sizeof(char) );
           printf("issue #1389 failed\n");
           main_return = 1;
           break;
        }
        printf("malloced %.2f MB\n", (double) (data_size*sizeof(char))/(1024*1024)  );
   
        sprintf(data, "hallo du");
      
        cl_commlib_send_message(handle, com_host, "server", 1, 
                            CL_MIH_MAT_NAK, 
                            (cl_byte_t**)&data, data_size * sizeof(char), 
                            NULL, 0,0, false, false);
   
        printf("starting measurement...\n");
        gettimeofday(&start, NULL);
        while (do_shutdown == 0) {
           cl_commlib_trigger(handle, 1); 
           ret_val = cl_commlib_receive_message(handle,NULL, NULL, 0, false, 0, &message, &sender);
           if (ret_val != CL_RETVAL_OK && ret_val != CL_RETVAL_NO_MESSAGE) {
              printf("cl_commlib_receive_message returned: %s\n", cl_get_error_text(ret_val));
              printf("issue #1389 failed\n");
              main_return = 1;
              break;
           }
           if (message != NULL) {
              printf("received response message from %s with size %lu\n", sender->comp_host, message->message_length);
              cl_com_free_message(&message);
              cl_com_free_endpoint(&sender);
              break;
           } 
        }
        gettimeofday(&now,NULL);
        runtime = now.tv_sec - start.tv_sec;
        printf("send/receive took %d seconds\n", runtime );
        /* adding 2 seconds to be on the save side */
        if (runtime < TEST_ISSUES_READ_WRITE_TIMEOUT + 2) {
           if (data_size < 64*1024*1024) {
              data_size = data_size * 2;
           } else {
              data_size = data_size + 128*1024*1024;
           }
        } else {
           timeout_reached_count++;
           printf("test timeout reached %d times!\n", timeout_reached_count);
        }
     }

     printf("\ntesting issue #1400 ...\n");
     {
        printf("setting up server for test 1400 ...\n");
        cl_commlib_send_message(handle, com_host, "server", 1,
                                CL_MIH_MAT_ACK,
                                &iz1400_pointer, 7,
                                NULL, 0,0, true, true);
     }


     /* to be sure that server waits */
     printf("waiting 10 seconds for server to set max. connected client count ...\n");
     sleep(10);

     printf("creating new connections ...\n");

     handle1=cl_com_create_handle(NULL, framework, CL_CM_CT_MESSAGE, false, com_port, CL_TCP_DEFAULT, "client1", 0, 1, 0 );
     handle2=cl_com_create_handle(NULL, framework, CL_CM_CT_MESSAGE, false, com_port, CL_TCP_DEFAULT, "client2", 0, 1, 0 );
 
     printf("sending via connection 1 ...\n");
     cl_commlib_send_message(handle, com_host, "server", 1,
                            CL_MIH_MAT_ACK,
                            &test_pointer, 5,
                            NULL, 0,0, true, true);

     printf("sending via connection 2 ...\n");
     cl_commlib_send_message(handle1, com_host, "server", 1,
                            CL_MIH_MAT_ACK,
                            &test_pointer, 5,
                            NULL, 0,0, true, true);

     printf("sending via connection 3 ...\n");
     ret_val = cl_commlib_send_message(handle2, com_host, "server", 1,
                            CL_MIH_MAT_ACK,
                            &test_pointer, 5,
                            NULL, 0,0, true, true);

     if (ret_val == CL_RETVAL_OK) {
        printf("issue #1400 failed\n");
        main_return = 2;
     } else {
        printf("3. connection send returned: \"%s\" and failed - ok\n" , cl_get_error_text(ret_val));
     }

     printf("resetting server ...\n");

     iz1400_data[5] = 'r';
     cl_commlib_send_message(handle, com_host, "server", 1,
                            CL_MIH_MAT_ACK,
                            &iz1400_pointer, 7,
                            NULL, 0,0, true, true);
  }

  printf("shutting down ...\n");
  cl_commlib_shutdown_handle(handle, false);

  printf("commlib cleanup ...\n");
  cl_com_cleanup_commlib();

  printf("main done\n");
  return main_return;
}


