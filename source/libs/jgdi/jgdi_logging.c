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

#include <ctype.h>
#include <string.h>
#include "jni.h"

#include "uti/rmon.h"

#include "cull/cull.h"
#include "cull/cull_list.h"
#include "sge_all_listsL.h"
#include "sgeobj/sge_answer.h"

#include "basis_types.h"
#include "jgdi_logging.h"

struct level_str {
   const char* name;
   jobject object;
   jmethodID  mid;
   const char* log_method_name;
};

static struct level_str LEVEL_CACHE [] = {
   { "SEVERE", NULL, NULL, "severe" },
   { "WARNING", NULL, NULL, "warning" },
   { "INFO", NULL, NULL, "info" },
   { "CONFIG", NULL, NULL, "config" },
   { "FINE", NULL, NULL, "fine" },
   { "FINER", NULL, NULL, "finer" },
   { "FINEST", NULL, NULL, "finest" }
};

typedef struct {
   JNIEnv *env;
   jobject logger;
} jgdi_rmon_ctx_t;

static jclass Level_find_class(JNIEnv *env);
static jclass find_logger_class(JNIEnv *env);
static jobject get_level(JNIEnv *env, log_level_t level);
static jobject load_level(JNIEnv *env, const char* level);
static void jgdi_log_entering(JNIEnv *env, jobject logger, const char* classname, const char *func);
static void jgdi_log_exiting(JNIEnv *env, jobject logger, const char* classname, const char *func);
static int  jgdi_rmon_is_loggable(rmon_ctx_t* ctx, int layer, int debug_class);
static void jgdi_rmon_menter(rmon_ctx_t* ctx, const char* func);
static void jgdi_rmon_mexit(rmon_ctx_t* ctx, const char* func, const char *file, int line);
static void jgdi_rmon_mtrace(rmon_ctx_t* ctx, const char *func, const char *file, int line);
static void jgdi_rmon_mprintf(rmon_ctx_t* ctx, int debug_class, const char* fmt, va_list args);



static jclass find_logger_class(JNIEnv *env)
{
   static jclass clazz = NULL;

   if( clazz == NULL) {
      jclass tmpclazz = (*env)->FindClass(env, "java/util/logging/Logger");
      if (tmpclazz == NULL) {
         abort();
      }
      /* aquire a global ref for the class object */
      clazz = (jclass)(*env)->NewGlobalRef(env, tmpclazz);
      if (clazz == NULL) {
         abort();
      }
   }
   return clazz;
}


/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_get_logger - get a java logger object
 * PARAMETER
 *  env  - JNI environment
 *  name - name of the logger
 *
 * RETURN
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
jobject jgdi_get_logger(JNIEnv *env , const char* name) {

   jmethodID mid = NULL; 
   jclass clazz = NULL;

   jstring name_obj = NULL;
   jobject temp = NULL;

   clazz = find_logger_class(env);
   if (clazz == NULL) {
      return NULL;
   }
   
   if (mid == NULL) {
      mid = (*env)->GetStaticMethodID(env, clazz, "getLogger", "(Ljava/lang/String;)Ljava/util/logging/Logger;");
      if (mid == NULL) {
         (*env)->ExceptionClear(env);
         return NULL;
      }
   }
   name_obj = (*env)->NewStringUTF(env, name ); 

   temp = (*env)->CallStaticObjectMethod(env, clazz, mid , name_obj );  
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionDescribe(env);
      (*env)->ExceptionClear(env);
      return NULL;
   }  else {
      return temp;
   }
}

static jclass Level_find_class(JNIEnv *env)
{
   static jclass clazz = NULL;
   if (clazz == NULL) {
      clazz = (*env)->FindClass(env, "java/util/logging/Level");
      if(clazz == NULL) {
         abort();
      }
      clazz = (jclass)(*env)->NewGlobalRef(env, clazz);
   }
   return clazz;
}

static jobject load_level(JNIEnv *env, const char* level)
{
   jclass  clazz = Level_find_class(env);
   jobject ret = NULL;
   jfieldID mid = (*env)->GetStaticFieldID(env, clazz, level, "Ljava/util/logging/Level;");
   
   ret = (*env)->GetStaticObjectField(env, clazz, mid);
   if( (*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionDescribe(env);
      abort();
   }
   
   return ret;
}


/*-------------------------------------------------------------------------*
 * NAME
 *   get_level - Get the level jobject for a log level
 *
 * PARAMETER
 *  env   - JNI environment
 *  level - the log level
 *
 * RETURN
 *
 *  the log level object
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
static jobject get_level(JNIEnv *env, log_level_t level)
{
   if (LEVEL_CACHE[level].object == NULL) {
      jobject level_obj = load_level(env, LEVEL_CACHE[level].name);
      LEVEL_CACHE[level].object = (jclass)(*env)->NewGlobalRef(env, level_obj);
   }
   return LEVEL_CACHE[level].object;
}


/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_is_loggable - Determine where a log level is loggable in a logger
 
 * PARAMETER
 *  env -    JNI env 
 *  logger - logger object
 *  level -  the log level
 *
 * RETURN
 *      true  - if the level is loggable in the logger
 *      false - if the level is not loggable in the logger or if
 *              in the jni env an exception has occurred
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
jboolean jgdi_is_loggable(JNIEnv *env, jobject logger, log_level_t level)
{
   jobject level_obj = NULL;
   static jmethodID mid = NULL;
   jboolean ret = false;
   
   /* We can not write a log message of a exception has occureed */
   if((*env)->ExceptionOccurred(env)) {
      return false;
   }
   
   if (logger == NULL) {
      return false;
   }

   level_obj = get_level(env, level);
   
   if (level_obj == NULL) {
      abort();
   }
   
   if (mid == NULL) {
      jclass clazz = (*env)->FindClass(env, "java/util/logging/Logger");
      mid = (*env)->GetMethodID(env, clazz, "isLoggable", "(Ljava/util/logging/Level;)Z");
      if (mid == NULL) {
         abort();
      }
   }
   ret = (*env)->CallBooleanMethod(env, logger, mid , level_obj );  
   if( (*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
      ret = false;
   }
   return ret;
}


static void jgdi_log_entering(JNIEnv *env, jobject logger, const char* classname, const char *func)
{
   static jmethodID mid = NULL;
   jclass clazz = NULL;
   jobject classname_obj = NULL;
   jobject func_obj = NULL;

   clazz = (*env)->GetObjectClass(env, logger);
   if(clazz == NULL) {
      abort();
   }
   
   if (mid == NULL ) {
      mid = (*env)->GetMethodID(env, clazz, "entering", "(Ljava/lang/String;Ljava/lang/String;)V");
      if(mid == NULL) {
         return;
      }
   }
   classname_obj = (*env)->NewStringUTF(env, classname); 
   func_obj = (*env)->NewStringUTF(env, func); 
   
   (*env)->CallVoidMethod(env, logger, mid , classname_obj, func_obj );  
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
   } 
}


static void jgdi_log_exiting(JNIEnv *env, jobject logger, const char* classname, const char *func)
{
   static jmethodID mid = NULL;
   jclass clazz = NULL;
   jobject classname_obj = NULL;
   jobject func_obj = NULL;
   
   /* We can not write a log message of a exception has occureed */
   if((*env)->ExceptionOccurred(env)) {
      return;
   }

   clazz = (*env)->GetObjectClass(env, logger);
   if(clazz == NULL) {
      abort();
   }
   
   if (mid == NULL ) {
      mid = (*env)->GetMethodID(env, clazz, "exiting", "(Ljava/lang/String;Ljava/lang/String;)V");
      if(mid == NULL) {
         return;
      }
   }
   classname_obj = (*env)->NewStringUTF(env, classname); 
   func_obj = (*env)->NewStringUTF(env, func); 
   
   (*env)->CallVoidMethod(env, logger, mid , classname_obj, func_obj );  
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
   } 
}


/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_log - write a log message into a logger
 * PARAMETER
 *  env    - JNI environment
 *  logger - the logger object
 *  level  - the log level
 *  msg    -  the log message
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
void jgdi_log(JNIEnv *env, jobject logger, log_level_t level, const char* msg)
{
   jmethodID mid = NULL;
   jclass clazz = NULL;
   jstring msg_obj = NULL;

   clazz = (*env)->GetObjectClass(env, logger);
   if(clazz == NULL) {
      abort();
   }
   if (level >= LOG_LEVEL_COUNT || level < 0) {
      abort();
   } 
   if (LEVEL_CACHE[level].mid == NULL ) {
      LEVEL_CACHE[level].mid = (*env)->GetMethodID(env, clazz, LEVEL_CACHE[level].log_method_name, "(Ljava/lang/String;)V");
   }
   mid = LEVEL_CACHE[level].mid;
   
   if (mid == NULL) {
      return;
   }
   
   msg_obj = (*env)->NewStringUTF(env, msg); 
   
   (*env)->CallVoidMethod(env, logger, mid , msg_obj );  
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
   } 
}

/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_log_printf - write a log message with printf into java logger 
 * PARAMETER
 *  env    - JNI enviromnent
 *  logger - the logger object
 *  level  - the log level
 *  fmt    - the log format
 *
 * RETURN
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
void jgdi_log_printf(JNIEnv *env, const char* logger, log_level_t level, const char* fmt, ...)
{
   jobject logger_obj = NULL;
  
   logger_obj = jgdi_get_logger(env, logger);
   if(logger_obj == NULL) {
      return;
   }
   
   if (jgdi_is_loggable(env, logger_obj, level)) {
      dstring ds = DSTRING_INIT;
      va_list ap;

      va_start(ap, fmt);
      sge_dstring_vsprintf(&ds, fmt, ap);
      va_end(ap);
      jgdi_log(env, logger_obj, FINE, sge_dstring_get_string(&ds));
      sge_dstring_free(&ds);
   }
}


/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_log_list - write a cull list into a java logger
 * PARAMETER
 *  env    - the java environment
 *  logger - the java logger
 *  level  - the log level
 *  list   - the cull list
 *
 * RETURN
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
void jgdi_log_list(JNIEnv *env, const char* logger, log_level_t level, lList *list)
{
   jobject logger_obj = NULL;
   
   logger_obj = jgdi_get_logger(env, logger);
   if(logger_obj == NULL) {
      return;
   }
   
   if (jgdi_is_loggable(env, logger_obj, level)) {
      dstring ds = DSTRING_INIT;
      
      lInit(nmv); 
      lWriteListToStr(list, &ds);
      jgdi_log(env, logger_obj, FINE, sge_dstring_get_string(&ds));
      
      sge_dstring_free(&ds);
   }
}


void jgdi_log_listelem(JNIEnv *env, const char* logger, log_level_t level, lListElem *elem)
{
   jobject logger_obj = NULL;
   
   logger_obj = jgdi_get_logger(env, logger);
   if(logger_obj == NULL) {
      return;
   }
   
   if (jgdi_is_loggable(env, logger_obj, level)) {
      dstring ds = DSTRING_INIT;
      
      lInit(nmv); 
      lWriteElemToStr(elem, &ds);
      jgdi_log(env, logger_obj, FINE, sge_dstring_get_string(&ds));
      
      sge_dstring_free(&ds);
   }
}

/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_log_answer_list - write a answer list into a java logger
 *
 * PARAMETER
 *  env    - JNI environment
 *  logger - the logger object
 *  alp    - the answer list
 *
 * RETURN
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *
 *    Writes the content of an answer_list into a logger. The message of the 
 *    log record is the content of AN_text. The level of the log message depends
 *    on the AN_quality field.
 *    ANSWER_QUALITY_ERROR -> SEVERE
 *    ANSWER_QUALITY_WARNING -> WARINING
 *    ANSWER_QUALITY_INFO -> INFO
 *    
 *-------------------------------------------------------------------------*/
void jgdi_log_answer_list(JNIEnv *env, const char* logger, lList *alp)
{
   if (alp != NULL) {
      jobject logger_obj = NULL;
      lListElem *answer;   /* AN_Type */
      
      logger_obj = jgdi_get_logger(env, logger);
      if(logger_obj == NULL) {
         return;
      }
   
      for_each(answer, alp) {
         switch(lGetUlong(answer, AN_quality)) {
            case ANSWER_QUALITY_ERROR:
               jgdi_log(env, logger_obj, SEVERE, lGetString(answer, AN_text));
               break;
            case ANSWER_QUALITY_WARNING:
               jgdi_log(env, logger_obj, WARNING, lGetString(answer, AN_text));
               break;
            case ANSWER_QUALITY_INFO:
               jgdi_log(env, logger_obj, INFO, lGetString(answer, AN_text));
               break;
         }
      }
   }
}


/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_init_rmon_ctx - setup a rmon_ctx which write log messages into a java logger
 * PARAMETER
 *
 *  env    - JNI environment
 *  logger - the name of the java logger
 *  ctx    - the rmon context
 *
 * RETURN
 *
 *    0
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
int jgdi_init_rmon_ctx(JNIEnv *env, const char* logger, rmon_ctx_t *ctx)
{
   jgdi_rmon_ctx_t *myctx = NULL;
   
   myctx = (jgdi_rmon_ctx_t*)sge_malloc(sizeof(jgdi_rmon_ctx_t));
  
   ctx->ctx = myctx;
   ctx->is_loggable = jgdi_rmon_is_loggable;
   ctx->menter = jgdi_rmon_menter;
   ctx->mexit = jgdi_rmon_mexit;
   ctx->mtrace = jgdi_rmon_mtrace;
   ctx->mprintf = jgdi_rmon_mprintf;
   
   myctx->env = env;
   myctx->logger = jgdi_get_logger(env, logger);
   
   return 0;
}


static log_level_t get_rmon_log_level(int layer, int debug_class)
{
   switch(layer) {
      case TOP_LAYER:
         return FINE;
      default:
         return FINER;
   }
}

static int  jgdi_rmon_is_loggable(rmon_ctx_t* ctx, int layer, int debug_class)
{
   jgdi_rmon_ctx_t *myctx = (jgdi_rmon_ctx_t*)ctx->ctx;
   return jgdi_is_loggable(myctx->env, myctx->logger, get_rmon_log_level(layer, debug_class));
}

static void jgdi_rmon_menter(rmon_ctx_t* ctx, const char* func)
{
   jgdi_rmon_ctx_t *myctx = (jgdi_rmon_ctx_t*)ctx->ctx;
   jgdi_log_entering(myctx->env, myctx->logger, "native", func);
}

static void jgdi_rmon_mexit(rmon_ctx_t* ctx, const char* func, const char *file, int line)
{
   jgdi_rmon_ctx_t *myctx = (jgdi_rmon_ctx_t*)ctx->ctx;
   jgdi_log_exiting(myctx->env, myctx->logger, "native", func);
}

static void jgdi_rmon_mtrace(rmon_ctx_t* ctx, const char *func, const char *file, int line)
{
   /* dummy func */
}

static void jgdi_rmon_mprintf(rmon_ctx_t* ctx, int debug_class, const char* fmt, va_list args)
{
   jgdi_rmon_ctx_t *myctx = (jgdi_rmon_ctx_t*)ctx->ctx;
   log_level_t level = get_rmon_log_level(TOP_LAYER, debug_class);
   
   if (jgdi_is_loggable(myctx->env, myctx->logger, level)) {
      dstring ds = DSTRING_INIT;

      sge_dstring_vsprintf(&ds, fmt, args);
      
      jgdi_log(myctx->env, myctx->logger, level, sge_dstring_get_string(&ds));
      sge_dstring_free(&ds);
   }
}

/*-------------------------------------------------------------------------*
 * NAME
 *   jgdi_destroy_rmon_ctx - destroy a rmon context 
 * PARAMETER
 *  rmon_ctx -  the rmon context
 *
 * RETURN
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *
 *   Destroy a rmon context which has been initialized with the method jgdi_init_rmon_ctx
 *-------------------------------------------------------------------------*/
void jgdi_destroy_rmon_ctx(rmon_ctx_t *rmon_ctx)
{
   if (rmon_ctx->ctx) {
      sge_free(&(rmon_ctx->ctx));
      rmon_ctx->ctx = NULL;
   }
}

