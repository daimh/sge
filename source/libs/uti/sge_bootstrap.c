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

#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "basis_types.h"

#include "uti/sge_rmon.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_dstring.h"
#include "uti/sge_parse_num_par.h"
#include "uti/sge_hostname.h"
#include "uti/sge_spool.h"
#include "uti/setup_path.h"
#include "uti/msg_utilib.h"
#include "uti/sge_error_class.h"
#include "uti/sge_bootstrap.h"

#include "sgeobj/sge_answer.h"

typedef struct {
    char* admin_user;      
    char* default_domain;
    bool  ignore_fqdn;
    char* spooling_method;
    char* spooling_lib;
    char* spooling_params;
    char* binary_path;
    char* qmaster_spool_dir;
    char* security_mode;
    int   listener_thread_count;
    int   worker_thread_count;
    int   scheduler_thread_count;
    int   jvm_thread_count;
    bool  job_spooling;
} sge_bootstrap_state_t;

typedef struct {
   sge_bootstrap_state_class_t* current;
   sge_bootstrap_state_class_t* original;
} sge_bootstrap_thread_local_t;

static pthread_once_t bootstrap_once = PTHREAD_ONCE_INIT;
static pthread_key_t sge_bootstrap_thread_local_key;

static void bootstrap_thread_local_once_init(void);
static void bootstrap_thread_local_destroy(void* theState);
static void bootstrap_thread_local_init(sge_bootstrap_thread_local_t* theState);
 

static void bootstrap_state_destroy(sge_bootstrap_state_t* theState);

static bool sge_bootstrap_state_setup(sge_bootstrap_state_class_t *thiz, sge_path_state_class_t *sge_paths, sge_error_class_t *eh);
static void sge_bootstrap_state_dprintf(sge_bootstrap_state_class_t *thiz);
static const char* get_admin_user(sge_bootstrap_state_class_t *thiz);
static const char* get_default_domain(sge_bootstrap_state_class_t *thiz);
static const char* get_spooling_method(sge_bootstrap_state_class_t *thiz);
static const char* get_spooling_lib(sge_bootstrap_state_class_t *thiz);
static const char* get_spooling_params(sge_bootstrap_state_class_t *thiz);
static const char* get_binary_path(sge_bootstrap_state_class_t *thiz);
static const char* get_qmaster_spool_dir(sge_bootstrap_state_class_t *thiz);
static const char* get_security_mode(sge_bootstrap_state_class_t *thiz);
static bool get_ignore_fqdn(sge_bootstrap_state_class_t *thiz);
static bool get_job_spooling(sge_bootstrap_state_class_t *thiz);
static int get_listener_thread_count(sge_bootstrap_state_class_t *thiz);
static int get_worker_thread_count(sge_bootstrap_state_class_t *thiz);
static int get_scheduler_thread_count(sge_bootstrap_state_class_t *thiz);
static int get_jvm_thread_count(sge_bootstrap_state_class_t *thiz);
static void set_admin_user(sge_bootstrap_state_class_t *thiz, const char *admin_user);
static void set_default_domain(sge_bootstrap_state_class_t *thiz, const char *default_domain);
static void set_spooling_method(sge_bootstrap_state_class_t *thiz, const char *spooling_method);
static void set_spooling_lib(sge_bootstrap_state_class_t *thiz, const char *spooling_lib);
static void set_spooling_params(sge_bootstrap_state_class_t *thiz, const char *spooling_params);
static void set_binary_path(sge_bootstrap_state_class_t *thiz, const char *binary_path);
static void set_qmaster_spool_dir(sge_bootstrap_state_class_t *thiz, const char *qmaster_spool_dir);
static void set_security_mode(sge_bootstrap_state_class_t *thiz, const char *security_mode);
static void set_ignore_fqdn(sge_bootstrap_state_class_t *thiz, bool ignore_fqdn);
static void set_job_spooling(sge_bootstrap_state_class_t *thiz, bool job_spooling);
static void set_listener_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count);
static void set_worker_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count);
static void set_scheduler_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count);
static void set_jvm_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count);

static bool sge_bootstrap_state_class_init(sge_bootstrap_state_class_t *st, sge_error_class_t *eh);

/*** AA **/ 

/****** uti/bootstrap/bootstrap_mt_init() **********************************
*  NAME
*     bootstrap_mt_init() -- Initialize bootstrap code for multi threading use.
*
*  SYNOPSIS
*     void bootstrap_mt_init(void) 
*
*  FUNCTION
*     Set up bootstrap code. This function must be called at least once before
*     any of the bootstrap oriented functions can be used. This function is
*     idempotent, i.e. it is safe to call it multiple times.
*
*     Thread local storage for the bootstrap state information is reserved. 
*
*  INPUTS
*     void - NONE 
*
*  RESULT
*     void - NONE
*
*  NOTES
*     MT-NOTE: bootstrap_mt_init() is MT safe 
*
*******************************************************************************/
void bootstrap_mt_init(void)
{
   pthread_once(&bootstrap_once, bootstrap_thread_local_once_init);
}

void sge_bootstrap_state_set_thread_local(sge_bootstrap_state_class_t* ctx) {
   DENTER(TOP_LAYER, "sge_bootstrap_state_set_thread_local");
   
   {   
      GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                   "sge_bootstrap_state_set_thread_local");
      if (ctx != NULL) {
         handle->current = ctx;
      } else {
         handle->current = handle->original;
      }
   }
   DEXIT;
}

const char *bootstrap_get_admin_user(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_admin_user");
   bootstrap = handle->current;                
   return bootstrap->get_admin_user(bootstrap);
}

const char *bootstrap_get_default_domain(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_default_domain");
   bootstrap = handle->current;                
   return bootstrap->get_default_domain(bootstrap);
}

bool bootstrap_get_ignore_fqdn(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_ignore_fqdn");
   bootstrap = handle->current;                
   return bootstrap->get_ignore_fqdn(bootstrap);
}

const char *bootstrap_get_spooling_method(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_spooling_method");
   bootstrap = handle->current;                
   return bootstrap->get_spooling_method(bootstrap);
}

const char *bootstrap_get_spooling_lib(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_spooling_lib");
   bootstrap = handle->current;                
   return bootstrap->get_spooling_lib(bootstrap);
}

const char *bootstrap_get_spooling_params(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_spooling_params");
   bootstrap = handle->current;                
   return bootstrap->get_spooling_params(bootstrap);
}

const char *bootstrap_get_binary_path(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_binary_path");
   bootstrap = handle->current;                
   return bootstrap->get_binary_path(bootstrap);
}

const char *bootstrap_get_qmaster_spool_dir(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_qmaster_spool_dir");
   bootstrap = handle->current;                
   return bootstrap->get_qmaster_spool_dir(bootstrap);
}

const char *bootstrap_get_security_mode(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_security_mode");
   bootstrap = handle->current;                
   return bootstrap->get_security_mode(bootstrap);
}

void bootstrap_set_admin_user(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_admin_user");
   bootstrap = handle->current;                
   bootstrap->set_admin_user(bootstrap, value);
}

void bootstrap_set_default_domain(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_default_domain");
   bootstrap = handle->current;                
   bootstrap->set_default_domain(bootstrap, value);
}

void bootstrap_set_ignore_fqdn(bool value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_ignore_fqdn");
   bootstrap = handle->current;                
   bootstrap->set_ignore_fqdn(bootstrap, value);
}

void bootstrap_set_spooling_method(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_spooling_method");
   bootstrap = handle->current;                
   bootstrap->set_spooling_method(bootstrap, value);
}

void bootstrap_set_spooling_lib(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_spooling_lib");
   bootstrap = handle->current;                
   bootstrap->set_spooling_lib(bootstrap, value);
}

void bootstrap_set_spooling_params(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_spooling_params");
   bootstrap = handle->current;                
   bootstrap->set_spooling_params(bootstrap, value);
}

void bootstrap_set_binary_path(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_binary_path");
   bootstrap = handle->current;                
   bootstrap->set_binary_path(bootstrap, value);
}

void bootstrap_set_qmaster_spool_dir(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_qmaster_spool_dir");
   bootstrap = handle->current;                
   bootstrap->set_qmaster_spool_dir(bootstrap, value);
}

void bootstrap_set_security_mode(const char *value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_security_mode");
   bootstrap = handle->current;                
   bootstrap->set_security_mode(bootstrap, value ); 
}

void bootstrap_set_listener_thread_count(int value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_listener_thread_count");
   bootstrap = handle->current;                
   bootstrap->set_listener_thread_count(bootstrap, value); 
}

void bootstrap_set_worker_thread_count(int value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_worker_thread_count");
   bootstrap = handle->current;                
   bootstrap->set_worker_thread_count(bootstrap, value); 
}

void bootstrap_set_scheduler_thread_count(int value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_scheduler_thread_count");
   bootstrap = handle->current;                
   bootstrap->set_scheduler_thread_count(bootstrap, value); 
}

void bootstrap_set_jvm_thread_count(int value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_jvm_thread_count");
   bootstrap = handle->current;                
   bootstrap->set_jvm_thread_count(bootstrap, value); 
}

int bootstrap_get_listener_thread_count(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_listener_thread_count");
   bootstrap = handle->current;                
   return bootstrap->get_listener_thread_count(bootstrap);
}

int bootstrap_get_worker_thread_count(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_worker_thread_count");
   bootstrap = handle->current;                
   return bootstrap->get_worker_thread_count(bootstrap);
}

int bootstrap_get_scheduler_thread_count(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_scheduler_thread_count");
   bootstrap = handle->current;                
   return bootstrap->get_scheduler_thread_count(bootstrap);
}

int bootstrap_get_jvm_thread_count(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_jvm_thread_count");
   bootstrap = handle->current;                
   return bootstrap->get_jvm_thread_count(bootstrap);
}

void bootstrap_set_job_spooling(bool value)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_set_job_spooling");
   bootstrap = handle->current;                
   bootstrap->set_job_spooling(bootstrap, value ); 
}

bool bootstrap_get_job_spooling(void)
{
   sge_bootstrap_state_class_t* bootstrap = NULL;
   GET_SPECIFIC(sge_bootstrap_thread_local_t, handle, bootstrap_thread_local_init, sge_bootstrap_thread_local_key, 
                "bootstrap_get_job_spooling");
   bootstrap = handle->current;                
   return bootstrap->get_job_spooling(bootstrap);
}

/****** uti/bootstrap/sge_bootstrap() ******************************************
*  NAME
*     sge_bootstrap() -- read and process bootstrap file 
*
*  SYNOPSIS
*     bool sge_bootstrap(void) 
*
*  FUNCTION
*     Reads the bootstrap file ($SGE_ROOT/$SGE_CELL/common/bootstrap).
*     
*     Initializes the policy for hostcpy and hostcmp (ignore_fqdn and 
*     default_domain).
*
*  INPUTS
*     dstring *error_dstring - dynamic string buffer to return error messages.
*                              if error_string is NULL, error messages will be
*                              written using the sge_log functions, else they
*                              will be returned in error_dstring.
*
*  RESULT
*     bool - true on success, else false
*
*  NOTES
*     MT-NOTE: sge_bootstrap() is MT safe
*******************************************************************************/
#define NUM_BOOTSTRAP 14
#define NUM_REQ_BOOTSTRAP 9
bool sge_bootstrap(const char *bootstrap_file, dstring *error_dstring) 
{
   bool ret = true;
   int i = 0;
   /*const char **/
   bootstrap_entry_t name[NUM_BOOTSTRAP] = { {"admin_user", true},
                                             {"default_domain", true},
                                             {"ignore_fqdn", true},
                                             {"spooling_method", true},
                                             {"spooling_lib", true}, 
                                             {"spooling_params", true},
                                             {"binary_path", true}, 
                                             {"qmaster_spool_dir", true},
                                             {"security_mode", true},
                                             {"job_spooling", false},
                                             {"listener_threads", false},
                                             {"worker_threads", false},
                                             {"scheduler_threads", false},
                                             {"jvm_threads", false}
                                     };
   char value[NUM_BOOTSTRAP][1025];

   DENTER(TOP_LAYER, "sge_bootstrap");

   for (i = 0; i < NUM_BOOTSTRAP; i++) {
      value[i][0] = '\0';
   }

   /* get filepath of bootstrap file */
   if (bootstrap_file == NULL) {
      if (error_dstring == NULL) {
         CRITICAL((SGE_EVENT, SFNMAX, MSG_UTI_CANNOTRESOLVEBOOTSTRAPFILE));
      } else {
         sge_dstring_sprintf(error_dstring, "%s", MSG_UTI_CANNOTRESOLVEBOOTSTRAPFILE);
      }
      ret = false;
   /* read bootstrapping information */   
   } else if (sge_get_confval_array(bootstrap_file, NUM_BOOTSTRAP, NUM_REQ_BOOTSTRAP, name, 
                                    value, error_dstring)) {
      ret = false;
   } else {
      /* store bootstrapping information */
      bootstrap_set_admin_user(value[0]);
      bootstrap_set_default_domain(value[1]);
      {
         u_long32 uval;
         parse_ulong_val(NULL, &uval, TYPE_BOO, value[2], 
                         NULL, 0);
         bootstrap_set_ignore_fqdn(uval ? true : false);
      }
      bootstrap_set_spooling_method(value[3]);
      bootstrap_set_spooling_lib(value[4]);
      bootstrap_set_spooling_params(value[5]);
      bootstrap_set_binary_path(value[6]);
      bootstrap_set_qmaster_spool_dir(value[7]);
      bootstrap_set_security_mode(value[8]);
      if (strcmp(value[9], "")) {
         u_long32 uval = 0;
         parse_ulong_val(NULL, &uval, TYPE_BOO, value[9], 
                         NULL, 0);
         bootstrap_set_job_spooling(uval ? true : false);
      } else {
         bootstrap_set_job_spooling(true);
      }   
      {
         u_long32 uval = 0;
         parse_ulong_val(NULL, &uval, TYPE_INT, value[10], NULL, 0);
         bootstrap_set_listener_thread_count(uval);
      }
      {
         u_long32 uval = 0;
         parse_ulong_val(NULL, &uval, TYPE_INT, value[11], NULL, 0);
         bootstrap_set_worker_thread_count(uval);
      }
      {
         u_long32 uval = 0;
         parse_ulong_val(NULL, &uval, TYPE_INT, value[12], NULL, 0);
         bootstrap_set_scheduler_thread_count(uval);
      }
      {
         u_long32 uval = 0;
         parse_ulong_val(NULL, &uval, TYPE_INT, value[13], NULL, 0);
         bootstrap_set_jvm_thread_count(uval);
      }

      DPRINTF(("admin_user          >%s<\n", bootstrap_get_admin_user()));
      DPRINTF(("default_domain      >%s<\n", bootstrap_get_default_domain()));
      DPRINTF(("ignore_fqdn         >%s<\n", bootstrap_get_ignore_fqdn() ? 
                                             "true" : "false"));
      DPRINTF(("spooling_method     >%s<\n", bootstrap_get_spooling_method()));
      DPRINTF(("spooling_lib        >%s<\n", bootstrap_get_spooling_lib()));
      DPRINTF(("spooling_params     >%s<\n", bootstrap_get_spooling_params()));
      DPRINTF(("binary_path         >%s<\n", bootstrap_get_binary_path()));
      DPRINTF(("qmaster_spool_dir   >%s<\n", bootstrap_get_qmaster_spool_dir()));
      DPRINTF(("security_mode       >%s<\n", bootstrap_get_security_mode()));
      DPRINTF(("job_spooling        >%s<\n", bootstrap_get_job_spooling() ? 
                                             "true":"false"));
      DPRINTF(("listener_threads    >%d<\n", bootstrap_get_listener_thread_count()));
      DPRINTF(("worker_threads      >%d<\n", bootstrap_get_worker_thread_count()));
      DPRINTF(("scheduler_threads   >%d<\n", bootstrap_get_scheduler_thread_count()));
      DPRINTF(("jvm_threads         >%d<\n", bootstrap_get_jvm_thread_count()));
   } 

   DEXIT;
   return ret;
}


/****** uti/bootstrap/bootstrap_thread_local_once_init() ********************************
*  NAME
*     bootstrap_thread_local_once_init() -- One-time bootstrap code initialization.
*
*  SYNOPSIS
*     static bootstrap_thread_local_once_init(void) 
*
*  FUNCTION
*     Create access key for thread local storage. Register cleanup function.
*
*     This function must be called exactly once.
*
*  INPUTS
*     void - none
*
*  RESULT
*     void - none 
*
*  NOTES
*     MT-NOTE: bootstrap_thread_local_once_init() is MT safe. 
*
*******************************************************************************/
static void bootstrap_thread_local_once_init(void)
{
   pthread_key_create(&sge_bootstrap_thread_local_key, bootstrap_thread_local_destroy);
}

/****** uti/bootstrap/bootstrap_thread_local_destroy() ****************************
*  NAME
*     bootstrap_thread_local_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void bootstrap_thread_local_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memory which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: bootstrap_thread_local_destroy() is MT safe.
*
*******************************************************************************/
static void bootstrap_thread_local_destroy(void* theState)
{
   sge_bootstrap_thread_local_t *handle = (sge_bootstrap_thread_local_t*)theState;
   sge_bootstrap_state_class_destroy(&(handle->original));
   handle->current = NULL;
}

/****** uti/bootstrap/bootstrap_thread_local_init() *******************************
*  NAME
*     bootstrap_thread_local_init() -- Initialize bootstrap state.
*
*  SYNOPSIS
*     static void bootstrap_thread_local_init(sge_bootstrap_state_class_t* theState) 
*
*  FUNCTION
*     Initialize bootstrap state.
*
*  INPUTS
*     bootstrap_state_t* theState - Pointer to bootstrap state structure.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: bootstrap_thread_local_init() is MT safe. 
*
*******************************************************************************/
static void bootstrap_thread_local_init(sge_bootstrap_thread_local_t* theState)
{
   memset(theState, 0, sizeof(sge_bootstrap_thread_local_t));
   theState->original = (sge_bootstrap_state_class_t *)sge_malloc(sizeof(sge_bootstrap_state_class_t));
   
   sge_bootstrap_state_class_init(theState->original, NULL);
   theState->current = theState->original;
}


/*-------------------------------------------------------------------------*/

sge_bootstrap_state_class_t *sge_bootstrap_state_class_create(sge_path_state_class_t *sge_paths, sge_error_class_t *eh)
{
   sge_bootstrap_state_class_t *ret = (sge_bootstrap_state_class_t *)sge_malloc(sizeof(sge_bootstrap_state_class_t));

   DENTER(TOP_LAYER, "sge_bootstrap_state_class_create");

   if (!ret) {
      if (eh != NULL) {
         eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s", MSG_MEMORY_MALLOCFAILED);
      }
      DEXIT;
      return NULL;
   }
   
   if( !sge_bootstrap_state_class_init(ret, eh) ) {
      sge_bootstrap_state_class_destroy(&ret);
      DEXIT;
      return NULL;
   }

   /* TODO move the following block into sge_bootstrap_state_class_init and
           delete bootstrap_setup */
   if (!sge_bootstrap_state_setup(ret, sge_paths, eh)) {
      sge_bootstrap_state_class_destroy(&ret);
      DEXIT;
      return NULL;
   }

   DEXIT;
   return ret;
}   

static bool sge_bootstrap_state_class_init(sge_bootstrap_state_class_t *st, sge_error_class_t *eh) {
   
   DENTER(TOP_LAYER, "sge_bootstrap_state_class_init");
   
   st->dprintf = sge_bootstrap_state_dprintf;

   st->get_admin_user = get_admin_user;
   st->get_default_domain = get_default_domain;
   st->get_ignore_fqdn = get_ignore_fqdn;
   st->get_spooling_method = get_spooling_method;
   st->get_spooling_lib = get_spooling_lib;
   st->get_spooling_params = get_spooling_params;
   st->get_binary_path = get_binary_path;
   st->get_qmaster_spool_dir = get_qmaster_spool_dir;
   st->get_security_mode = get_security_mode;
   st->get_job_spooling = get_job_spooling;
   st->get_listener_thread_count = get_listener_thread_count;
   st->get_worker_thread_count = get_worker_thread_count;
   st->get_scheduler_thread_count = get_scheduler_thread_count;
   st->get_jvm_thread_count = get_jvm_thread_count;

   st->set_admin_user = set_admin_user;
   st->set_default_domain = set_default_domain;
   st->set_ignore_fqdn = set_ignore_fqdn;
   st->set_spooling_method = set_spooling_method;
   st->set_spooling_lib = set_spooling_lib;
   st->set_spooling_params = set_spooling_params;
   st->set_binary_path = set_binary_path;
   st->set_qmaster_spool_dir = set_qmaster_spool_dir;
   st->set_security_mode = set_security_mode;   
   st->set_job_spooling = set_job_spooling;   
   st->set_listener_thread_count = set_listener_thread_count;   
   st->set_worker_thread_count = set_worker_thread_count;   
   st->set_scheduler_thread_count = set_scheduler_thread_count;   
   st->set_jvm_thread_count = set_jvm_thread_count;   
   
   st->sge_bootstrap_state_handle = sge_malloc(sizeof(sge_bootstrap_state_t));
   
   if (st->sge_bootstrap_state_handle == NULL ) {
      if (eh != NULL) {
         eh->error(eh, STATUS_EMALLOC, ANSWER_QUALITY_ERROR, "%s", MSG_MEMORY_MALLOCFAILED);
      }
      DEXIT;
      return false;
   }
   memset(st->sge_bootstrap_state_handle, 0, sizeof(sge_bootstrap_state_t));
   bootstrap_mt_init();

   DEXIT;
   return true;
}

void sge_bootstrap_state_class_destroy(sge_bootstrap_state_class_t **pst)
{
   DENTER(TOP_LAYER, "sge_bootstrap_state_class_destroy");
   if (!pst || !*pst) {
      DEXIT;
      return;
   }   
   bootstrap_state_destroy((*pst)->sge_bootstrap_state_handle);
   sge_free(pst);
   DEXIT;
}

/****** uti/bootstrap/bootstrap_state_destroy() ****************************
*  NAME
*     bootstrap_state_destroy() -- Free thread local storage
*
*  SYNOPSIS
*     static void bootstrap_state_destroy(void* theState) 
*
*  FUNCTION
*     Free thread local storage.
*
*  INPUTS
*     void* theState - Pointer to memory which should be freed.
*
*  RESULT
*     static void - none
*
*  NOTES
*     MT-NOTE: bootstrap_state_destroy() is MT safe.
*
*******************************************************************************/
static void bootstrap_state_destroy(sge_bootstrap_state_t* theState)
{
   sge_free(&(theState->admin_user));
   sge_free(&(theState->default_domain));
   sge_free(&(theState->spooling_method));
   sge_free(&(theState->spooling_lib));
   sge_free(&(theState->spooling_params));
   sge_free(&(theState->binary_path));
   sge_free(&(theState->qmaster_spool_dir));
   sge_free(&(theState->security_mode));
   sge_free(&theState);
}

static bool sge_bootstrap_state_setup(sge_bootstrap_state_class_t *thiz, sge_path_state_class_t *sge_paths, sge_error_class_t *eh)
{
   #define NUM_BOOTSTRAP 14
   #define REQ_BOOTSTRAP 9

   dstring error_dstring = DSTRING_INIT;
   const char *bootstrap_file = NULL;
   bootstrap_entry_t name[NUM_BOOTSTRAP] = { {"admin_user", true},
                                             {"default_domain", true},
                                             {"ignore_fqdn", true},
                                             {"spooling_method", true},
                                             {"spooling_lib", true}, 
                                             {"spooling_params", true},
                                             {"binary_path", true}, 
                                             {"qmaster_spool_dir", true},
                                             {"security_mode", true},
                                             {"job_spooling", false},
                                             {"listener_threads", false},
                                             {"worker_threads", false},
                                             {"scheduler_threads", false},
                                             {"jvm_threads", false}
                                     };
   char value[NUM_BOOTSTRAP][1025];
   int i;

   DENTER(TOP_LAYER, "sge_bootstrap_state_setup");

   for (i = 0; i < NUM_BOOTSTRAP; i++) {
      value[i][0] = '\0';
   }

   if (!sge_paths) {
      eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "sge_paths is NULL");
      DEXIT;
      return false;
   }

   /* get filepath of bootstrap file */
   bootstrap_file = sge_paths->get_bootstrap_file(sge_paths);
   if (bootstrap_file == NULL) {
      eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "%s", MSG_UTI_CANNOTRESOLVEBOOTSTRAPFILE);
      DEXIT;
      return false;
   } 
   
   /* read bootstrapping information */   
   if (sge_get_confval_array(bootstrap_file, NUM_BOOTSTRAP, NUM_REQ_BOOTSTRAP, name, 
                                    value, &error_dstring)) {
      eh->error(eh, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, "%s", sge_dstring_get_string(&error_dstring));
      sge_dstring_free(&error_dstring);
      DEXIT;
      return false;
   } 

   /* store bootstrapping information */
   thiz->set_admin_user(thiz, value[0]);
   thiz->set_default_domain(thiz, value[1]);
   {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_BOO, value[2], NULL, 0);
      thiz->set_ignore_fqdn(thiz, uval ? true : false);
   }
   thiz->set_spooling_method(thiz, value[3]);
   thiz->set_spooling_lib(thiz, value[4]);
   thiz->set_spooling_params(thiz, value[5]);
   thiz->set_binary_path(thiz, value[6]);
   thiz->set_qmaster_spool_dir(thiz, value[7]);
   thiz->set_security_mode(thiz, value[8]);
   if (strcmp(value[9], "")) {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_BOO, value[9], NULL, 0);
      thiz->set_job_spooling(thiz, uval ? true : false);
   } else {
      thiz->set_job_spooling(thiz, true);
   }
   {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_INT, value[10], NULL, 0);
      thiz->set_listener_thread_count(thiz, uval);
   }
   {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_INT, value[11], NULL, 0);
      thiz->set_worker_thread_count(thiz, uval);
   }
   {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_INT, value[12], NULL, 0);
      thiz->set_scheduler_thread_count(thiz, uval);
   }
   {
      u_long32 uval = 0;
      parse_ulong_val(NULL, &uval, TYPE_INT, value[13], NULL, 0);
      thiz->set_jvm_thread_count(thiz, uval);
   }

#if 0
   thiz->dprintf(thiz);
#endif

   DEXIT;
   return true;
}

static void sge_bootstrap_state_dprintf(sge_bootstrap_state_class_t *thiz)
{
   sge_bootstrap_state_t *bs = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   
   DENTER(TOP_LAYER, "sge_bootstrap_state_dprintf");

   DPRINTF(("admin_user          >%s<\n", bs->admin_user));
   DPRINTF(("default_domain      >%s<\n", bs->default_domain));
   DPRINTF(("ignore_fqdn         >%s<\n", bs->ignore_fqdn ?  "true" : "false"));
   DPRINTF(("spooling_method     >%s<\n", bs->spooling_method));
   DPRINTF(("spooling_lib        >%s<\n", bs->spooling_lib));
   DPRINTF(("spooling_params     >%s<\n", bs->spooling_params));
   DPRINTF(("binary_path         >%s<\n", bs->binary_path));
   DPRINTF(("qmaster_spool_dir   >%s<\n", bs->qmaster_spool_dir));
   DPRINTF(("security_mode       >%s<\n", bs->security_mode));
   DPRINTF(("job_spooling        >%s<\n", bs->job_spooling ? "true" : "false"));
   DPRINTF(("listener_threads    >%d<\n", bs->listener_thread_count));
   DPRINTF(("worker_threads      >%d<\n", bs->worker_thread_count));
   DPRINTF(("scheduler_threads   >%d<\n", bs->scheduler_thread_count));
   DPRINTF(("jvm_threads         >%d<\n", bs->jvm_thread_count));

   DEXIT;
}

static const char* get_admin_user(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->admin_user;
}

static const char* get_default_domain(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->default_domain;
}

static const char* get_spooling_method(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->spooling_method;
}

static const char* get_spooling_lib(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->spooling_lib;
}

static const char* get_spooling_params(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->spooling_params;
}

static const char* get_binary_path(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->binary_path;
}

static const char* get_qmaster_spool_dir(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->qmaster_spool_dir;
}

static const char* get_security_mode(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->security_mode;
}

static bool get_ignore_fqdn(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->ignore_fqdn;
}

static bool get_job_spooling(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->job_spooling;
}

static int get_listener_thread_count(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->listener_thread_count;
}

static int get_worker_thread_count(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->worker_thread_count;
}

static int get_scheduler_thread_count(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->scheduler_thread_count;
}

static int get_jvm_thread_count(sge_bootstrap_state_class_t *thiz) 
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   return es->jvm_thread_count;
}

static void set_admin_user(sge_bootstrap_state_class_t *thiz, const char *admin_user)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->admin_user = sge_strdup(es->admin_user, admin_user);
}

static void set_default_domain(sge_bootstrap_state_class_t *thiz, const char *default_domain)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->default_domain = sge_strdup(es->default_domain, default_domain);
}

static void set_spooling_method(sge_bootstrap_state_class_t *thiz, const char *spooling_method)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->spooling_method = sge_strdup(es->spooling_method, spooling_method);
}

static void set_spooling_lib(sge_bootstrap_state_class_t *thiz, const char *spooling_lib)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->spooling_lib = sge_strdup(es->spooling_lib, spooling_lib);
}

static void set_spooling_params(sge_bootstrap_state_class_t *thiz, const char *spooling_params)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->spooling_params = sge_strdup(es->spooling_params, spooling_params);
}

static void set_binary_path(sge_bootstrap_state_class_t *thiz, const char *binary_path)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->binary_path = sge_strdup(es->binary_path, binary_path);
}

static void set_qmaster_spool_dir(sge_bootstrap_state_class_t *thiz, const char *qmaster_spool_dir)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->qmaster_spool_dir = sge_strdup(es->qmaster_spool_dir, qmaster_spool_dir);
}

static void set_security_mode(sge_bootstrap_state_class_t *thiz, const char *security_mode)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->security_mode = sge_strdup(es->security_mode, security_mode);
}

static void set_ignore_fqdn(sge_bootstrap_state_class_t *thiz, bool ignore_fqdn)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->ignore_fqdn = ignore_fqdn;
}

static void set_job_spooling(sge_bootstrap_state_class_t *thiz, bool job_spooling)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;
   es->job_spooling = job_spooling;
}

static void set_listener_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;

   if (thread_count <= 0) {
      thread_count = 2;
   } else if (thread_count > 16) {
      thread_count = 16;
   }   
   es->listener_thread_count = thread_count;   
}

static void set_worker_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;

   if (thread_count <= 0) {
      thread_count = 2;
   } else if (thread_count > 16) {
      thread_count = 16;
   }   
   es->worker_thread_count = thread_count;   
}

static void set_scheduler_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;

   if (thread_count <= 0) {
      thread_count = 0;
   } else if (thread_count > 1) {
      thread_count = 1;
   }   
   es->scheduler_thread_count = thread_count;   
}

static void set_jvm_thread_count(sge_bootstrap_state_class_t *thiz, int thread_count)
{
   sge_bootstrap_state_t *es = (sge_bootstrap_state_t *) thiz->sge_bootstrap_state_handle;

   if (thread_count <= 0) {
      thread_count = 0;
   } else if (thread_count > 1) {
      thread_count = 1;
   }   
   es->jvm_thread_count = thread_count;   
}

