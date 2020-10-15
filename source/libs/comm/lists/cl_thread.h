#ifndef __CL_THREAD_H
#define __CL_THREAD_H
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

/* 
   thread states 
   =============

   This defines are used to set the cl_thread_settings_t->thread_state
   varialbe.
*/


#define CL_THREAD_STARTING 1
#define CL_THREAD_RUNNING  2
#define CL_THREAD_WAITING  3
#define CL_THREAD_EXIT     4
#define CL_THREAD_CANCELED 5
#define CL_THREAD_CREATOR  6




/* 
   cl_thread_settings_t struct
   ===========================

   This is are the internal thread settings. Each thread has a pointer
   to his own cl_thread_settings_t struct.
*/

/* define cleanup functio type */
typedef void  (*cl_thread_cleanup_func_t) (cl_thread_settings_t* thread_config);

typedef enum cl_thread_type_def {
   CL_TT_UNDEFINED = 0,  /* free */
   CL_TT_CREATOR,        /* creator thread (main()) */
   CL_TT_COMMLIB,        /* commlib thread (do not use for threads created outside commlib) */
   CL_TT_LISTENER,       /* qmaster */
   CL_TT_WORKER,         /* qmaster */
   CL_TT_DELIVERER,      /* qmaster */
   CL_TT_JVM,            /* qmaster */
   CL_TT_SCHEDULER,      /* qmaster */
   CL_TT_SIGNALER,       /* qmaster */
   CL_TT_TESTER,         /* qmaster */
   CL_TT_TIMER,          /* qmaster */
   CL_TT_IJS,            /* interactive job support */
   CL_TT_IJS_REGISTER,   /* interactive job support */
   CL_TT_USER1,          /* free */
   CL_TT_USER2,          /* free */
   CL_TT_USER3,          /* free */
   CL_TT_USER4,          /* free */
   CL_TT_USER5           /* free */
} cl_thread_type_t;


struct cl_thread_settings_type {
#ifdef CL_DO_COMMLIB_DEBUG
   struct timeval           thread_last_cancel_test_time; /* time when thread did the last cl_thread_func_testcancel() call */
#endif
   char*                    thread_name;                  /* name of thread */
   int                      thread_id;                    /* thread id */
   int                      thread_state;                 /* thread state, e.g. CL_THREAD_WAITING */
   unsigned long            thread_event_count;           /* number of cl_thread_wait_for_event() calls */
   cl_raw_list_t*           thread_log_list;              /* list for log ( can be NULL ) */
   pthread_t*               thread_pointer;               /* pointer to thread (pthread lib) */
   cl_thread_condition_t*   thread_event_condition;       /* event call conditions */
   cl_thread_condition_t*   thread_startup_condition;     /* startup condition ( used by cl_thread_setup() ) */
   cl_thread_cleanup_func_t thread_cleanup_func;          /* thread cleanup function pointer */
   cl_thread_type_t         thread_type;                  /* thread type identifier */
   void*                    thread_user_data;             /* any user data ( e.g.: a pointer to a struct ) ) */
};

struct cl_thread_condition_type {
   pthread_mutex_t*   thread_mutex_lock;    /* mutex and condition variable for thread's */
   pthread_cond_t*    thread_cond_var;      /* event wait/trigger :     */

   pthread_mutex_t*   trigger_count_mutex;  /* used to lock trigger_count */
   unsigned long      trigger_count;        /* counts nr of cl_thread_trigger_thread_condition() calls for this condition */
};


/* 
   thread functions 
   ================

   This functions are more used more or less internal
*/
int cl_thread_setup(cl_thread_settings_t* thread_config,
                    cl_raw_list_t* log_list,
                    const char* name,
                    int id,
                    void * (*start_routine)(void *),
                    cl_thread_cleanup_func_t cleanup_func,
                    void* user_data,
                    cl_thread_type_t thread_type); 
int cl_thread_cleanup(cl_thread_settings_t* thread_config); 
cl_thread_settings_t* cl_thread_get_thread_config(void);
int cl_thread_set_thread_config(cl_thread_settings_t* thread_config);
int cl_thread_unset_thread_config(void);
int cl_thread_shutdown(cl_thread_settings_t* thread_config);
const char* cl_thread_get_state(cl_thread_settings_t* thread_config);
const char* cl_thread_convert_state_id(int thread_state);
int cl_thread_join(cl_thread_settings_t* thread_config);
void cl_thread_cleanup_global_thread_config_key(void);

int cl_thread_wait_for_event(cl_thread_settings_t *thread_config, long sec, long micro_sec);
int cl_thread_trigger_event(cl_thread_settings_t *thread_config);
int cl_thread_clear_events(cl_thread_settings_t *thread_config);


int cl_thread_create_thread_condition(cl_thread_condition_t** condition );
int cl_thread_delete_thread_condition(cl_thread_condition_t** condition );
int cl_thread_wait_for_thread_condition(cl_thread_condition_t* condition, long sec, long micro_sec);
int cl_thread_trigger_thread_condition(cl_thread_condition_t* condition, int do_broadcast);
int cl_thread_clear_triggered_conditions(cl_thread_condition_t* condition);



/* thread_func functions  */
int cl_thread_func_startup(cl_thread_settings_t* thread_config);
int cl_thread_func_testcancel(cl_thread_settings_t* thread_config);
int cl_thread_func_cleanup(cl_thread_settings_t* thread_config);
void cl_thread_default_cleanup_function(cl_thread_settings_t* thread_config);





/*
    thread_func functions 
    =====================

    This functions must be used in thread_func ( thread function ) implementations
    additional the first call of the thread implementation has to be

->  pthread_cleanup_push((void *) cl_thread_default_cleanup_function, (void*) thread_config );

    and the last has to be

->  pthread_cleanup_pop(0);

    the thread implementation gets one parameter: a pointer to cl_thread_settings_t structure

    After setup of thread:

->  cl_thread_func_startup(thread_config); 

    After thread main work:

->  cl_thread_func_cleanup(thread_config);
 
    Example:

    void *timeout_thread_main(void *t_conf) {
       cl_thread_settings_t *thread_config = (cl_thread_settings_t*)t_conf; 
       pthread_cleanup_push((void *) cl_thread_default_cleanup_function, (void*) thread_config );

       ...  initalization

       cl_thread_func_startup(thread_config);

       ...  thread main 
   
       cl_thread_func_cleanup(thread_config);  
       pthread_cleanup_pop(0);
       return (NULL);
    }
*/


#endif /* __CL_THREAD_H */
