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


#include "comm/cl_data_types.h"
#include "comm/lists/cl_thread.h"

#define THREAD_LIB_HANDLE cl_raw_list_t
#define THREAD_HANDLE     cl_thread_settings_t


int thread_init_lib(THREAD_LIB_HANDLE **thread_lib_handle);
int thread_cleanup_lib(THREAD_LIB_HANDLE **thread_lib_handle);

int create_thread(THREAD_LIB_HANDLE *thread_lib_handle,
                  THREAD_HANDLE **thread,
                  cl_raw_list_t *log_list,
                  const char *thread_name,
                  int thread_id,
                  void* thread_func(void*));

int register_thread(cl_raw_list_t     *log_list,
                    THREAD_HANDLE     *thread,
                    const char *thread_name);

int thread_func_startup(void *t_conf);
int thread_func_cleanup(void *t_conf);

int thread_shutdown(THREAD_HANDLE *thread);

int thread_trigger_event(THREAD_HANDLE *thread);
int thread_wait_for_event(THREAD_HANDLE *thread, int sec, int msec);
int thread_join(THREAD_HANDLE *thread);

int thread_setcancelstate(int enabled);
int thread_testcancel(void *t_conf);
