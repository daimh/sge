#ifndef __CL_LIST_TYPES_H
#define __CL_LIST_TYPES_H

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

#include <sys/time.h>
#include <pthread.h>

#include "basis_types.h"

#define CL_THREAD_LIST 1
#define CL_LOG_LIST    2
typedef struct cl_thread_settings_type  cl_thread_settings_t;
typedef struct cl_thread_condition_type cl_thread_condition_t;
/***********************************************************************/
/* RAW_LIST */
/***********************************************************************/
typedef struct cl_raw_list_elem_type cl_raw_list_elem_t;
struct cl_raw_list_elem_type {
  void* data;                         /* pointer to data */ 
  cl_raw_list_elem_t* next;
  cl_raw_list_elem_t* last;
};


typedef struct cl_raw_list_type {
   char*                 list_name;       /* name of list */
   int                   list_type;       /* type of list CL_THREAD_LIST, CL_LOG_LIST */
   pthread_mutex_t*      list_mutex;      /* list lock mutex */
   unsigned long         elem_count;      /* number of list entries */
   void*                 list_data;       /* user list data */

#ifdef CL_DO_COMMLIB_DEBUG
   char* last_locker;
   unsigned long         lock_count;
   unsigned long         unlock_count;
#endif
   cl_raw_list_elem_t*   first_elem;
   cl_raw_list_elem_t*   last_elem;
} cl_raw_list_t;

/***********************************************************************/


/***********************************************************************/
/* THREAD_LIST */
/***********************************************************************/
typedef struct cl_thread_list_elem_type cl_thread_list_elem_t;
struct cl_thread_list_elem_type {
   cl_thread_settings_t* thread_config;   /* data */
   cl_raw_list_elem_t*   raw_elem;
};




/***********************************************************************/

typedef enum cl_log_type {
   CL_LOG_OFF = 0,
   CL_LOG_ERROR,
   CL_LOG_WARNING,
   CL_LOG_INFO,
   CL_LOG_DEBUG
} cl_log_t;

/***********************************************************************/
/* LOG_LIST */
/***********************************************************************/
typedef struct cl_log_list_elem_type cl_log_list_elem_t;
struct cl_log_list_elem_type {                      /* list element specific data */
   char*                 log_parameter;                  /* additional parameter */
   char*                 log_message;                    /* log message */
   char*                 log_thread_name;                /* name of thread  */
   int                   log_thread_id;                  /* thread id   */
   int                   log_thread_state;               /* state of thread */
   cl_log_t              log_type;                       /* log level type */
   char*                 log_module_name;                /* name of c - module */
   cl_raw_list_elem_t*   raw_elem;
};

typedef struct cl_application_error_list_elem_t {
   cl_raw_list_elem_t*   raw_elem;         /* commlib internal list pointer to raw list element */
   int                   cl_error;         /* commlib error code 
                                              (use cl_get_error_text() to resolve error string) */
   char*                 cl_info;          /* additional error information */
   struct timeval        cl_log_time;      /* time when the message was added */
   bool                  cl_already_logged;/* CL_TRUE when this error was logged the last 
                                              CL_DEFINE_MESSAGE_DUP_LOG_TIMEOUT seconds */
   cl_log_t              cl_err_type;      /* commlib error message type */
} cl_application_error_list_elem_t;

typedef enum cl_log_list_flush_method_type {
   CL_LOG_FLUSHED,        /* flushing is done when ever user calls cl_log_list_flush() */
   CL_LOG_IMMEDIATE       /* cl_log_list_flush() is called on each log  */
} cl_log_list_flush_method_t;


/* function return value  function typedef              func parameter */
typedef int               (*cl_log_func_t)              (cl_raw_list_t* log_list);
typedef unsigned long     (*cl_app_status_func_t)       (char** info_message);
typedef void              (*cl_error_func_t)            (const cl_application_error_list_elem_t* commlib_error);
typedef const char*       (*cl_tag_name_func_t)         (unsigned long tag);
typedef void              (*cl_app_debug_client_func_t) (int cl_connected, int debug_level);


typedef struct cl_log_list_data_type {                      /* list specific data */
   cl_log_t                    current_log_level;           /* current log level */
   cl_log_list_flush_method_t  flush_type;                  /* flushing type */
   cl_log_func_t               flush_function;              /* function called for flushing */
   cl_thread_settings_t*       list_creator_settings;       /* log list creator thread settings */
} cl_log_list_data_t;

/***********************************************************************/



#endif /* __CL_LIST_TYPES_H */

