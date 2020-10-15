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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>

#ifndef NO_JNI

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_log.h"
#include "uti/sge_string.h"
#include "uti/sge_thread_ctrl.h"
#include "uti/sge_dstring.h"
#include "uti/sge_spool.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_profiling.h"
#include "uti/setup_path.h"
#include "uti/sge_mtutil.h"

#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_conf.h"
#include "sgeobj/sge_manop.h"

#include "gdi/sge_security.h"

#include "comm/cl_commlib.h"

#include "basis_types.h"
#include "setup_qmaster.h"
#include "sge_event_master.h"
#include "sge_reporting_qmaster.h"
#include "sge_qmaster_timed_event.h"
#include "sge_host_qmaster.h"
#include "configuration_qmaster.h"
#include "sge_thread_main.h"
#include "sge_thread_jvm.h"
#include "sge_qmaster_threads.h"
#include "msg_qmaster.h"



master_jvm_class_t Master_Jvm = {
   PTHREAD_MUTEX_INITIALIZER,
   PTHREAD_COND_INITIALIZER,
   false,
   0,
   true,
   false
};

static bool shutdown_main(void);
static void *sge_jvm_main(void *arg);

/****** qmaster/threads/sge_jvm_terminate() ***********************************
*  NAME
*     sge_jvm_terminate() -- trigger termination of JVM 
*
*  SYNOPSIS
*     void sge_jvm_terminate(sge_gdi_ctx_class_t *ctx) 
*
*  FUNCTION
*     A call of this function triggers the termination of the JVM thread .
*  
*     If ther JVM is running then JVM will be notified to terminate.
*     The thread running the JVM will get a cancel signal.
*
*     If the JVM should be running but if it was not able to start due
*     to some reason (missing library; wrong jvm parameters; ...) then
*     the JVM thread is waiting only for a termination call which will be 
*     send by this function.
*
*     'Master_Scheduler' is accessed by this function.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context object 
*     lList **answer_list      - answer list
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_jvm_terminate() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_jvm_initialize() 
*     qmaster/threads/sge_jvm_cleanup_thread() 
*     qmaster/threads/sge_jvm_terminate() 
*     qmaster/threads/sge_jvm_wait_for_terminate()
*     qmaster/threads/sge_jvm_main() 
*******************************************************************************/
void
sge_jvm_terminate(sge_gdi_ctx_class_t *ctx, lList **answer_list)
{

   DENTER(TOP_LAYER, "sge_jvm_terminate");

   sge_mutex_lock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

   if (Master_Jvm.is_running) {
      pthread_t thread_id; 
      cl_thread_settings_t* thread = NULL;

      /*
       * store thread id to use it later on
       */
      thread = cl_thread_list_get_first_thread(Main_Control.jvm_thread_pool);
      thread_id = *(thread->thread_pointer);

      /* 
       * send cancel signal
       * trigger shutdown in the JVM
       * and signal the continuation to realease the JVM thread if it was unable to setup the JVM 
       */
      pthread_cancel(thread_id);
      if (!shutdown_main()) {
         INFO((SGE_EVENT, "JVM thread shutdown_main failed"));
      }
      Master_Jvm.shutdown_started = true;
      pthread_cond_broadcast(&Master_Jvm.cond_var);

      /*
       * cl_thread deletion and cl_thread_pool deletion will be done at 
       * JVM threads cancelation point in sge_jvm_cleanup_thread() ...
       * ... therefore we have nothing more to do.
       */
      sge_mutex_unlock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

      /* 
       * after releasing the lock it is safe to wait for the termination. 
       * doing this inside the critical section (before the lock) could 
       * rise a deadlock situtaion this function would be called within a GDI request!
       */
      pthread_join(thread_id, NULL);

      INFO((SGE_EVENT, MSG_THREAD_XTERMINATED_S, threadnames[JVM_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
   } else {
      sge_mutex_unlock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

      WARNING((SGE_EVENT, MSG_THREAD_XNOTRUNNING_S, threadnames[JVM_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   }

   DRETURN_VOID;
}

/****** qmaster/threads/sge_jvm_initialize() ************************************
*  NAME
*     sge_jvm_initialize() -- setup and start the JVM thread 
*
*  SYNOPSIS
*     void sge_jvm_initialize(sge_gdi_ctx_class_t *ctx) 
*
*  FUNCTION
*     A call to this function initializes the JVM thread if it is not
*     already running.
*
*     The first call to this function (during qmaster start) starts
*     the JVM thread only if it is enabled in bootstrap configuration
*     file. Otherwise the JVM will not be started.
*
*     Each subsequent call (triggered via GDI) will definitely try to
*     start the JVM tread if it is not running.
*
*     Main routine for the created thread is sge_jvm_main().
*
*     'Master_Jvm' is accessed by this function.
*
*  INPUTS
*     sge_gdi_ctx_class_t *ctx - context object 
*     lList **answer_list      - answer list
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_jvm_initialize() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_jvm_initialize() 
*     qmaster/threads/sge_jvm_cleanup_thread() 
*     qmaster/threads/sge_jvm_terminate() 
*     qmaster/threads/sge_jvm_wait_for_terminate()
*     qmaster/threads/sge_jvm_main() 
*******************************************************************************/
void
sge_jvm_initialize(sge_gdi_ctx_class_t *ctx, lList **answer_list)
{
   DENTER(TOP_LAYER, "sge_jvm_initialize");

   sge_mutex_lock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));
  
   if (Master_Jvm.is_running == false) {
      bool start_thread = true;

      /* 
       * when this function is called the first time we will use the setting from
       * the bootstrap file to identify if the Jvm should be started or not
       * otherwise we have to start the thread due to a manual request through GDI.
       * There is no option. We have to start it.
       */
      if (Master_Jvm.use_bootstrap == true) {
         start_thread = ((ctx->get_jvm_thread_count(ctx) > 0) ? true : false);
         Master_Jvm.use_bootstrap = false;
      }

      if (start_thread == true) {
         cl_thread_settings_t* dummy_thread_p = NULL;
         dstring thread_name = DSTRING_INIT;

         /*
          * initialize the thread pool
          */
         cl_thread_list_setup(&(Main_Control.jvm_thread_pool), "thread pool");
  
         /* 
          * prepare a unique jvm thread name for each instance 
          */
         sge_dstring_sprintf(&thread_name, "%s%03d", threadnames[JVM_THREAD],
                             Master_Jvm.thread_id);

         /*
          * start the JVM thread 
          */
         cl_thread_list_create_thread(Main_Control.jvm_thread_pool, &dummy_thread_p,
                                      cl_com_get_log_list(), sge_dstring_get_string(&thread_name),
                                      Master_Jvm.thread_id, sge_jvm_main, NULL, NULL, CL_TT_JVM);
         sge_dstring_free(&thread_name);

         /*
          * Increase the thread id so that the next instance of a jvm will have a 
          * different name and flag that jvm is running and that shutdown has not been
          * triggered
          */
         Master_Jvm.shutdown_started = false;
         Master_Jvm.thread_id++;
         Master_Jvm.is_running = true;

         INFO((SGE_EVENT, MSG_THREAD_XHASSTARTED_S, threadnames[JVM_THREAD]));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
      } else {
         INFO((SGE_EVENT, MSG_THREAD_XSTARTDISABLED_S, threadnames[JVM_THREAD]));
         answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_INFO);
      }
   } else {
      ERROR((SGE_EVENT, MSG_THREAD_XISRUNNING_S, threadnames[JVM_THREAD]));
      answer_list_add(answer_list, SGE_EVENT, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
   }
   sge_mutex_unlock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

   DRETURN_VOID;
}


#include <jni.h>
#include <string.h>

#ifdef SOLARIS
#include <link.h>
#endif

#include "uti/sge_dlopen.h"

typedef int (*JNI_CreateJavaVM_Func)(JavaVM **pvm, void **penv, void *args);
typedef int (*JNI_GetCreatedJavaVMs_Func)(JavaVM **pvm, jsize size, jsize *real_size);

static void *libjvm_handle = NULL;
static JNI_CreateJavaVM_Func sge_JNI_CreateJavaVM = NULL;
static JNI_GetCreatedJavaVMs_Func sge_JNI_GetCreatedJavaVMs = NULL;
static pthread_mutex_t libjvm_mutex = PTHREAD_MUTEX_INITIALIZER;

static JavaVM* myjvm = NULL;
static pthread_mutex_t myjvm_mutex = PTHREAD_MUTEX_INITIALIZER;

static JNIEnv* create_vm(const char *libjvm_path, int argc, char** argv);
static int invoke_main(JNIEnv* env, jclass main_class, int argc, char** argv);


#ifdef DARWIN
int JNI_CreateJavaVM_Impl(JavaVM **pvm, void **penv, void *args);
#endif

static bool
sge_run_jvm(sge_gdi_ctx_class_t *ctx, void *anArg, monitoring_t *monitor);

static void
sge_jvm_cleanup_monitor(monitoring_t *monitor)
{
   DENTER(TOP_LAYER, "cleanup_monitor");
   sge_monitor_free(monitor);
   DRETURN_VOID;
}

/*-------------------------------------------------------------------------*
 * NAME
 *   shutdown_main - invoke the shutdown method of com.sun.grid.jgdi.management.JGDIAgent
 * PARAMETER
 *  env  - the JNI environment
 *  argc - number of arguments for the main method
 *  argv - the arguments for the main method
 *
 * RETURN
 *
 *  exit code of the JVM
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
static bool shutdown_main(void) 
{
   char main_class_name[] = "com/sun/grid/jgdi/management/JGDIAgent";
	jmethodID shutdown_mid = NULL;
   JNIEnv *env;
   jclass main_class;
   int ret = 0;
   bool error_occured = false;
   JavaVMAttachArgs attach_args = { JNI_VERSION_1_2, NULL, NULL };

   DENTER(TOP_LAYER, "shutdown_main");

   pthread_mutex_lock(&myjvm_mutex);

   if (myjvm == NULL) {
      pthread_mutex_unlock(&myjvm_mutex);
      DRETURN(true);
   }

   ret = (*myjvm)->AttachCurrentThread(myjvm, (void**) &env, &attach_args);

   if (ret < 0) { 
      CRITICAL((SGE_EVENT, "could not attach thread to vm"));
      pthread_mutex_unlock(&myjvm_mutex);
      DRETURN(false);
   }   

   if (env != NULL) {
      main_class = (*env)->FindClass(env, main_class_name);
      if (main_class != NULL) {
	      shutdown_mid = (*env)->GetStaticMethodID(env, main_class, "shutdown", "()V");
         if (shutdown_mid == NULL) {
            CRITICAL((SGE_EVENT, "class has no shutdown method"));
            error_occured = true;
         }
      } else {
         CRITICAL((SGE_EVENT, "main_class is NULL"));
         error_occured = true;
      }  

      if (!error_occured) {
         (*env)->CallStaticVoidMethod(env, main_class, shutdown_mid);

         if ((*env)->ExceptionOccurred(env)) {
            (*env)->ExceptionClear(env);
            CRITICAL((SGE_EVENT, "unexpected jvm exception"));
            error_occured = true;
         }
      }   
   }

   ret = (*myjvm)->DetachCurrentThread(myjvm);
   if (ret < 0) { 
      CRITICAL((SGE_EVENT, "could not detach thread from vm"));
      error_occured = true;
   }   

   pthread_mutex_unlock(&myjvm_mutex);

   if (error_occured) {
      DRETURN(false);
   } else {
      DRETURN(true);
   }
}

static jint (JNICALL printVMErrors)(FILE *fp, const char *format, va_list args) {
   jint rc = 0;
   const char* str = NULL;
   dstring ds = DSTRING_INIT;

   DENTER(TOP_LAYER, "printVMErrors");
   str = sge_dstring_vsprintf(&ds, format, args);
   if (str != NULL) {
      rc = strlen(str);
      DPRINTF(("%s\n", str));
      CRITICAL((SGE_EVENT, "JVM message: %s", str));
   }
   sge_dstring_free(&ds);
   DRETURN(rc);
}

#if 0
static void (JNICALL exitVM)(jint code) {
    DENTER(TOP_LAYER, "exitVM");
    DPRINTF(("CALLED exitVM %d\n", (int)code));
    fflush(stdout);
    DRETURN_VOID;
}
#endif


/*-------------------------------------------------------------------------*
 * NAME
 *   create_vm - create a jvm
 * PARAMETER
 *  argc - number of arguments for the jvm
 *  argv - arguments for the jvm
 *
 * RETURN
 *
 *   NULL -  jvm could not be created
 *   else -  jvm has been created
 *
 * EXTERNAL
 *
 * DESCRIPTION
 * #if 0
 *   options[1].optionString = "exit";
 *   options[1].extraInfo = exitVM;
 * #endif   
 *-------------------------------------------------------------------------*/
static JNIEnv* create_vm(const char *libjvm_path, int argc, char** argv)
{
   bool ok = true;
	JNIEnv* env = NULL;
	JavaVMInitArgs args;
   int i = 0;
	JavaVMOption* options = NULL;
   const int extraOptionCount = 1;

   DENTER(GDI_LAYER, "create_vm");

   options = sge_malloc((argc+extraOptionCount)*sizeof(JavaVMOption));

	/* There is a new JNI_VERSION_1_4, but it doesn't add anything for the purposes of our example. */
	args.version = JNI_VERSION_1_2;
	args.nOptions = argc+extraOptionCount;
   options[0].optionString = "vfprintf";
   options[0].extraInfo = (void*)printVMErrors;
   for(i=0; i < argc; i++) {
      /*printf("argv[%d] = %s\n", i, argv[i]);*/
      options[i+extraOptionCount].optionString = argv[i];
   }

	args.options = options;
	args.ignoreUnrecognized = JNI_FALSE;

   pthread_mutex_lock(&libjvm_mutex);
   if (libjvm_handle == NULL) {
      /* build the full name of the shared lib - append architecture dependent
       * shlib postfix 
       */
#ifdef HP1164
      /*
      ** need to switch to start user for HP
      */
      sge_switch2start_user();
#endif   

      /* open the shared lib */
      libjvm_handle = sge_dlopen(libjvm_path, NULL);

#ifdef HP1164
      /*
      ** switch back to admin user for HP
      */
      sge_switch2admin_user();
#endif
      if (libjvm_handle == NULL) {
         CRITICAL((SGE_EVENT, "could not load libjvm %s", dlerror()));
         ok = false;
      }

      /* retrieve function pointer of get_method function in shared lib */
      if (ok) {
#ifdef DARWIN
         /*
         ** for darwin there exists no JNI_CreateJavaVM, Why not maybe a fix in the future ???
         */
         const char* JNI_CreateJavaVM_FuncName = "JNI_CreateJavaVM_Impl";
         const char* JNI_GetCreatedJavaVMs_FuncName = "JNI_GetCreatedJavaVMs_Impl";
#else
         const char* JNI_CreateJavaVM_FuncName = "JNI_CreateJavaVM";
         const char* JNI_GetCreatedJavaVMs_FuncName = "JNI_GetCreatedJavaVMs";
#endif
         sge_JNI_CreateJavaVM = (JNI_CreateJavaVM_Func)dlsym(libjvm_handle, JNI_CreateJavaVM_FuncName);
         if (sge_JNI_CreateJavaVM == NULL) {
            CRITICAL((SGE_EVENT, "could not load sge_JNI_CreateJavaVM %s", dlerror()));
            ok = false;
         }
      
         sge_JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_Func)dlsym(libjvm_handle, JNI_GetCreatedJavaVMs_FuncName);
         if (sge_JNI_GetCreatedJavaVMs == NULL) {
            CRITICAL((SGE_EVENT, "could not load sge_JNI_GetCreatedJavaVMs %s", dlerror()));
            ok = false;
         }
      }
   }
   pthread_mutex_unlock(&libjvm_mutex);


   if (ok) {
      JavaVMAttachArgs attach_args = { JNI_VERSION_1_2, NULL, NULL };
      jsize have_jvm = 0;
	   JavaVM* jvm = NULL;
      pthread_mutex_lock(&myjvm_mutex);
      i = sge_JNI_GetCreatedJavaVMs(&jvm, 1, &have_jvm); 

      if (have_jvm && jvm != NULL) {
         if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_2) == JNI_EDETACHED) {
            if ((i = (*jvm)->AttachCurrentThread(jvm, (void**) &env, &attach_args)) < 0) {
               CRITICAL((SGE_EVENT, "can not get JNIEnv (error code %d)", i));
               env = NULL;
               myjvm = NULL;
            } else {
               myjvm = jvm;
            }
         } else {
            myjvm = jvm;
         }
      } else {
         if ((i = sge_JNI_CreateJavaVM(&jvm, (void **)&env, &args)) < 0) {
            CRITICAL((SGE_EVENT, "can not create JVM (error code %d)", i));
            env = NULL;
            myjvm = NULL;
         } else {
            myjvm = jvm;
         }  
      }
      pthread_mutex_unlock(&myjvm_mutex);

   }
   sge_free(&options);
	DRETURN(env);
}

/*-------------------------------------------------------------------------*
 * NAME
 *   invoke_main - invoke the main method of com.sun.grid.jgdi.management.JGDIAgent
 * PARAMETER
 *  env  - the JNI environment
 *  argc - number of arguments for the main method
 *  argv - the arguments for the main method
 *
 * RETURN
 *
 *  exit code of the JVM
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
static int invoke_main(JNIEnv* env, jclass main_class, int argc, char** argv) 
{
	jmethodID main_mid;
	jobjectArray main_args;
   int i = 0;

   DENTER(TOP_LAYER, "invoke_main");
   
	main_mid = (*env)->GetStaticMethodID(env, main_class, "main", "([Ljava/lang/String;)V");
   if (main_mid == NULL) {
      CRITICAL((SGE_EVENT, "class has no main method"));
      DRETURN(1);
   }

	main_args = (*env)->NewObjectArray(env, argc, (*env)->FindClass(env, "java/lang/String"), NULL);
   
   for(i=0; i < argc; i++) {
      jstring str = (*env)->NewStringUTF(env, argv[i]);
      DPRINTF(("argv[%d] = %s\n", i, argv[i]));
      (*env)->SetObjectArrayElement(env, main_args, i, str);
   }

   INFO((SGE_EVENT, "Starting up jvm thread (euid/uid: %d/%d)", geteuid(), getuid()));
	(*env)->CallStaticVoidMethod(env, main_class, main_mid, main_args);
   
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
      CRITICAL((SGE_EVENT, "unexpected exception in invoke_main"));
      DRETURN(1);
   }

   DRETURN(0);
}


static bool
sge_run_jvm(sge_gdi_ctx_class_t *ctx, void *anArg, monitoring_t *monitor)
{

   dstring ds = DSTRING_INIT;
   const char *libjvm_path = NULL;
   char **jvm_argv;
   int jvm_argc = 0;
   char *main_argv[1];
   int i;
   char main_class_name[] = "com/sun/grid/jgdi/management/JGDIAgent";
   JNIEnv* env = NULL;
   jobject main_class = NULL;
   char* additional_jvm_args = NULL;
   char** additional_jvm_argv = NULL;
   int additional_jvm_argc = 0;
   lListElem *confEntry = NULL;
   const int fixed_jvm_argc = 16;
   bool ret = true;
   char keystore_path[SGE_PATH_MAX];

   DENTER(TOP_LAYER, "sge_run_jvm");

   confEntry = sge_get_configuration_entry_by_name(ctx->get_qualified_hostname(ctx), "libjvm_path");
   if (confEntry != NULL) {
      const char *confVal = lGetString(confEntry, CF_value);
      if (confVal && *confVal != '\0') {
         libjvm_path = strdup(confVal);
      }   
      lFreeElem(&confEntry);
   }   

   if (libjvm_path == NULL) {
      WARNING((SGE_EVENT, "libjvm_path is NULL"));
      DRETURN(false);
   }  

   /*
   ** get keystore property
   */
   {
      #define NUM_PROPS 1
      dstring error_dstring = DSTRING_INIT;
      bootstrap_entry_t name[NUM_PROPS] = { {"com.sun.grid.jgdi.management.jmxremote.ssl.serverKeystore", true} };
      char value[NUM_PROPS][SGE_PATH_MAX];

      sge_dstring_sprintf(&ds, "%s/common/jmx/management.properties", ctx->get_cell_root(ctx));
      DPRINTF(("++ management.properties: %s\n", sge_dstring_get_string(&ds)));
      if (sge_get_management_entry(sge_dstring_get_string(&ds), NUM_PROPS, NUM_PROPS, name, 
                                    value, &error_dstring)) {
         WARNING((SGE_EVENT, "could not read keystore path %s", sge_dstring_get_string(&error_dstring)));
         sge_dstring_free(&error_dstring);
         sge_dstring_free(&ds);
         sge_free(&libjvm_path);
         DRETURN(false);
      }
      sge_strlcpy(keystore_path, value[0], SGE_PATH_MAX);
      DPRINTF(("++ keystore_path: %s\n", keystore_path));
   }
   
   /*
   ** get additional jvm args from configuration
   */
   confEntry = sge_get_configuration_entry_by_name(ctx->get_qualified_hostname(ctx), "additional_jvm_args");
   if (confEntry != NULL) {
      const char *confVal = lGetString(confEntry, CF_value);
      if (confVal && *confVal != '\0') {
         additional_jvm_args = strdup(confVal);
      }
      lFreeElem(&confEntry);
   }   

   if (additional_jvm_args != NULL) {
        DPRINTF(("additional_jvm_args: >%s<\n", additional_jvm_args));
        additional_jvm_argv = string_list(additional_jvm_args, " ", NULL);

        for (i = 0; additional_jvm_argv[i] != NULL; i++) {
           DPRINTF(("additional jvm arg[%d]: %s\n", i, additional_jvm_argv[i]));
           additional_jvm_argc++;
        }
        jvm_argc = fixed_jvm_argc + additional_jvm_argc;
   } else {
      additional_jvm_argv = NULL;
      jvm_argc = fixed_jvm_argc;
      additional_jvm_argc = 0;
   }

   DPRINTF(("fixed_jvm_argc + additional_jvm_argc = jvm_argc: %d + %d = %d\n", fixed_jvm_argc, additional_jvm_argc, jvm_argc));

   jvm_argv = sge_malloc(jvm_argc * sizeof(char*));
   /*
   ** adjust fixed_jvm_argc if an additional fixed argument line is added
   */
   jvm_argv[0] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.grid.jgdi.sgeRoot=%s", ctx->get_sge_root(ctx)));
   jvm_argv[1] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.grid.jgdi.sgeCell=%s", ctx->get_default_cell(ctx)));
   jvm_argv[2] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.grid.jgdi.caTop=%s", ctx->get_ca_root(ctx)));
   jvm_argv[3] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.grid.jgdi.serverKeystore=%s", keystore_path));
   jvm_argv[4] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.grid.jgdi.sgeQmasterSpoolDir=%s", ctx->get_qmaster_spool_dir(ctx)));
   jvm_argv[5] = strdup(sge_dstring_sprintf(&ds, "-Djava.class.path=%s/lib/jgdi.jar:%s/lib/juti.jar", ctx->get_sge_root(ctx), ctx->get_sge_root(ctx)));
   jvm_argv[6] = strdup(sge_dstring_sprintf(&ds, "-Djava.security.policy=%s/common/jmx/java.policy", ctx->get_cell_root(ctx)));
   jvm_argv[7] = strdup("-Djava.security.manager=com.sun.grid.jgdi.management.JGDISecurityManager");
   jvm_argv[8] = strdup(sge_dstring_sprintf(&ds, "-Djava.rmi.server.codebase=file://%s/lib/jgdi.jar file://%s/lib/juti.jar", ctx->get_sge_root(ctx), ctx->get_sge_root(ctx)));
   jvm_argv[9] = strdup(sge_dstring_sprintf(&ds, "-Djava.library.path=%s/lib/%s", ctx->get_sge_root(ctx), sge_get_arch()));
   jvm_argv[10] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.management.jmxremote.access.file=%s/common/jmx/jmxremote.access", ctx->get_cell_root(ctx)));
   jvm_argv[11] = strdup(sge_dstring_sprintf(&ds, "-Dcom.sun.management.jmxremote.password.file=%s/common/jmx/jmxremote.password", ctx->get_cell_root(ctx)));
   jvm_argv[12] = strdup(sge_dstring_sprintf(&ds, "-Djava.security.auth.login.config=%s/common/jmx/jaas.config", ctx->get_cell_root(ctx)));
   jvm_argv[13] = strdup(sge_dstring_sprintf(&ds, "-Djava.util.logging.config.file=%s/common/jmx/logging.properties", ctx->get_cell_root(ctx)));
/*    jvm_argv[13] = strdup("-Djava.util.logging.manager=com.sun.grid.jgdi.util.JGDILogManager"); */
   jvm_argv[14] = strdup("-Djava.util.logging.manager=java.util.logging.LogManager");
   jvm_argv[15] = strdup("-Xrs");

   /*
   ** add additional_jvm_args
   */
   for (i = 0; i < additional_jvm_argc; i++) {
      jvm_argv[fixed_jvm_argc + i] = strdup(additional_jvm_argv[i]);
      additional_jvm_argv[i] = NULL;
   }
   sge_free(&additional_jvm_argv);
   sge_free(&additional_jvm_args);

   /*
   ** process arguments of main method
   */
   main_argv[0] = strdup(sge_dstring_sprintf(&ds, "internal://%s@%s:"sge_u32, ctx->get_sge_root(ctx),
                         ctx->get_default_cell(ctx), ctx->get_sge_qmaster_port(ctx)));

   sge_dstring_free(&ds);


   /*
   ** for debugging
   */
   for (i=0; i<jvm_argc; i++) {
      DPRINTF(("jvm_argv[%d]: %s\n", i, jvm_argv[i]));
   }  

   for (i=0; i<sizeof(main_argv)/sizeof(char*); i++) {
      DPRINTF(("main_argv[%d]: %s\n", i, main_argv[i]));
   }  

   env = create_vm(libjvm_path, jvm_argc, jvm_argv);
   sge_free(&libjvm_path);

   if (env != NULL) {
      main_class = (*env)->FindClass(env, main_class_name);
      if (main_class != NULL) {
         if (invoke_main(env, main_class, sizeof(main_argv)/sizeof(char*), main_argv) != 0) {
            CRITICAL((SGE_EVENT, "invoke_main failed"));
            ret = false;
         }
      } else {
         CRITICAL((SGE_EVENT, "main_class is NULL"));
         ret = false;
      }  
   } else {
      ret = false;
   }


   /*
   ** free allocated jvm args
   */
   for (i=0; i<jvm_argc; i++) {
      sge_free(&(jvm_argv[i]));
   }  
   sge_free(&jvm_argv);

   /*
   ** free main_argv[0] argument
   */
   sge_free(&(main_argv[0]));

   DRETURN(ret);
}

/****** qmaster/threads/sge_jvm_wait_for_terminate() ****************************
*  NAME
*     sge_jvm_wait_for_terminate() -- blocks the executing thread till shutdown 
*
*  SYNOPSIS
*     void sge_jvm_wait_for_terminate(void) 
*
*  FUNCTION
*     A call of this finction blocks the executing thread until termination
*     is signalled.
*
*     The sign to continue execution can be triggered by signalling the
*     condition 'Master_Jvm.cond_var'. 
*
*     This function should only be called thy the JVM thread.
*
*  INPUTS
*     void - None 
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_jvm_wait_for_terminate() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_jvm_initialize() 
*     qmaster/threads/sge_jvm_cleanup_thread() 
*     qmaster/threads/sge_jvm_terminate() 
*     qmaster/threads/sge_jvm_wait_for_terminate()
*     qmaster/threads/sge_jvm_main() 
*******************************************************************************/
void                          
sge_jvm_wait_for_terminate(void)
{        
   DENTER(TOP_LAYER, "sge_thread_wait_for_signal");
         
   sge_mutex_lock("master jvm struct", SGE_FUNC, __LINE__, &Master_Jvm.mutex);
            
   while (Master_Jvm.shutdown_started == false) {
      pthread_cond_wait(&Master_Jvm.cond_var, &Master_Jvm.mutex);
   }     
            
   sge_mutex_unlock("master jvm struct", SGE_FUNC, __LINE__, &Master_Jvm.mutex);

   DEXIT;
   return;
}

/****** qmaster/threads/sge_jvm_cleanup_thread() ********************************
*  NAME
*     sge_jvm_cleanup_thread() -- cleanup routine for the JVM thread 
*
*  SYNOPSIS
*     void sge_jvm_cleanup_thread(void) 
*
*  FUNCTION
*     Cleanup JVM thread related stuff. 
*
*     This function has to be executed only by the JVM thread.
*     Ideally it should be the last function executed when the
*     pthreads cancelation point is passed.
*
*     'Master_Jvm' is accessed by this function. 
*
*  INPUTS
*     void - None 
*
*  RESULT
*     void - None
*
*  NOTES
*     MT-NOTE: sge_jvm_wait_for_terminate() is MT safe 
*
*  SEE ALSO
*     qmaster/threads/sge_jvm_initialize() 
*     qmaster/threads/sge_jvm_cleanup_thread() 
*     qmaster/threads/sge_jvm_terminate() 
*     qmaster/threads/sge_jvm_wait_for_terminate()
*     qmaster/threads/sge_jvm_main() 
*******************************************************************************/
static void
sge_jvm_cleanup_thread(void *ctx_ref)
{
   DENTER(TOP_LAYER, "sge_jvm_cleanup_thread");

   sge_mutex_lock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

   if (Master_Jvm.is_running) {
      cl_thread_settings_t* thread = NULL;

      /* 
       * The JVM thread itself executes this function (sge_jvm_cleanup_thread())
       * at the cancelation point as part of the cleanup. 
       * Therefore it has to unset the thread config before the
       * cl_thread is deleted. Otherwise we might run into a race condition when logging
       * is used after the call of cl_thread_list_delete_thread_without_join() 
       */
      cl_thread_unset_thread_config();

      /*
       * Delete the jvm thread but don't wait for termination
       */
      thread = cl_thread_list_get_first_thread(Main_Control.jvm_thread_pool);
      cl_thread_list_delete_thread_without_join(Main_Control.jvm_thread_pool, thread);

      /* 
       * Trash the thread pool 
       */
      cl_thread_list_cleanup(&Main_Control.jvm_thread_pool);

      /*
       * now a new jvm can start
       */
      Master_Jvm.is_running = false;

      /*
      ** free the ctx too
      */
      sge_gdi_ctx_class_destroy((sge_gdi_ctx_class_t **)ctx_ref);
   }

   sge_mutex_unlock("master jvm struct", SGE_FUNC, __LINE__, &(Master_Jvm.mutex));

   DRETURN_VOID;
}


/****** qmaster/threads/sge_jvm_main() ******************************************
*  NAME
*     sge_jvm_main() -- jvm thread function 
*
*  SYNOPSIS
*     static void* sge_jvm_main(void* arg) 
*
*  FUNCTION
*     The main function for the JVM thread 
*
*  INPUTS
*     void* arg - NULL 
*
*  RESULT
*     void* - none 
*
*  NOTES
*     MT-NOTE: sge_scheduler_main() is MT safe 
*
*     MT-NOTE: this is a thread function. Do NOT use this function
*     MT-NOTE: in any other way!
*
*  SEE ALSO
*     qmaster/threads/sge_jvm_initialize() 
*     qmaster/threads/sge_jvm_cleanup_thread() 
*     qmaster/threads/sge_jvm_terminate() 
*     qmaster/threads/sge_jvm_wait_for_terminate()
*     qmaster/threads/sge_jvm_main() 
*******************************************************************************/
static void *
sge_jvm_main(void *arg)
{
   cl_thread_settings_t *thread_config = (cl_thread_settings_t*)arg;
   time_t next_prof_output = 0;
   monitoring_t monitor;
   sge_gdi_ctx_class_t *ctx = NULL;
   volatile bool jvm_started = false;
   bool do_endlessly = true;

   DENTER(TOP_LAYER, "sge_jvm_main");

   cl_thread_func_startup(thread_config);
   sge_monitor_init(&monitor, thread_config->thread_name, GDI_EXT, MT_WARNING, MT_ERROR);
   sge_qmaster_thread_init(&ctx, QMASTER, JVM_THREAD, true);

   set_thread_name(pthread_self(), "JVM Thread");
   conf_update_thread_profiling("JVM Thread");

   /*
    * main loop of the JVM thread
    */
   while (do_endlessly) {
      int execute = 0;

      thread_start_stop_profiling();

      if (jvm_started == false) {
         jvm_started = sge_run_jvm(ctx, arg, &monitor);
      }

      thread_output_profiling("JVM thread profiling summary:\n", &next_prof_output);
      sge_monitor_output(&monitor);

      /* 
       * to prevent high cpu load if jvm is not started
       */
      sge_jvm_wait_for_terminate();

      /* 
       * pthread cancelation point 
       */
      do {
         /* 
          * sge_jvm_cleanup_thread() is the last function which should 
          * be called so it is pushed first 
          */
         pthread_cleanup_push(sge_jvm_cleanup_thread, (void*)&ctx);
         pthread_cleanup_push((void (*)(void *))sge_jvm_cleanup_monitor, (void *)&monitor);
         cl_thread_func_testcancel(thread_config);
         pthread_cleanup_pop(execute);
         pthread_cleanup_pop(execute);
      } while (sge_thread_has_shutdown_started()); 
   }

   /*
    * Don't add cleanup code here. It will never be executed. Instead register
    * a cleanup function with pthread_cleanup_push()/pthread_cleanup_pop() before 
    * the call of cl_thread_func_testcancel()
    *
    * NOTE: number of pthread_cleanup_push() and pthread_cleanup_pop() calls have
    *       no be equivalent. If this is not the case you might get funny 
    *       COMPILER ERRORS here.
    */
   DRETURN(NULL); 
} 

#endif

