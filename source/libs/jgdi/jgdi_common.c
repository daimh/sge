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

#include "uti/sge_rmon.h"
#include "uti/sge_prog.h"
#include "uti/sge_bootstrap.h"
#include "uti/sge_edit.h"
#include "uti/sge_log.h"
#include "uti/sge_error_class.h"

#include "cull/cull_list.h"
#include "cull/cull.h"

#include "commlib.h"
#include "cl_errors.h"

#include "gdi/version.h"
#include "gdi/sge_gdi.h"
#include "gdi/sge_gdi2.h"

#include "sgeobj/sge_all_listsL.h"
#include "sgeobj/sge_answer.h"
#include "sgeobj/sge_calendar.h"
#include "sgeobj/sge_qinstance_state.h"
#include "sgeobj/sge_cqueue.h"
#include "sgeobj/sge_ja_task.h"
#include "sgeobj/sge_sharetree.h"
#include "sgeobj/sge_utility.h"
#include "sgeobj/sge_event.h"

#include "jgdi.h"
#include "basis_types.h"
#include "jgdi_common.h"
#include "jgdi_wrapper.h"
#include "jgdi_factory.h"
#include "jgdi_logging.h"

#include "msg_sgeobjlib.h"
#include "msg_common.h"

#define MAX_GDI_CTX_ARRAY_SIZE 1024

static pthread_mutex_t sge_gdi_ctx_mutex = PTHREAD_MUTEX_INITIALIZER;
static sge_gdi_ctx_class_t* sge_gdi_ctx_array[MAX_GDI_CTX_ARRAY_SIZE];

typedef struct object_mapping_str object_mapping_t;

static jgdi_result_t get_map(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList **list, lList **alpp);
static jgdi_result_t set_map(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList *lp, lList **alpp);
static jgdi_result_t get_map_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList **list, lList **alpp);
static jgdi_result_t set_map_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList *lp, lList **alpp);
static jgdi_result_t set_value_in_elem(JNIEnv *env, jobject value_obj, lListElem *elem, int cullType, int pos, lList** alpp);
static jgdi_result_t create_object_from_elem(JNIEnv *env, lListElem *ep, jobject *obj, int cullType, int pos, lList **alpp);
static void exception_to_string(JNIEnv* env, jobject exc, dstring* buf);
static void print_stacktrace(JNIEnv* env, jobject exc, dstring* buf);
static void print_exception(JNIEnv* env, jobject exc, dstring* buf);
static jgdi_result_t get_descriptor_for_property(JNIEnv *env, jobject property_descr, lDescr **descr, lList **alpp);
static jgdi_result_t get_list_descriptor_for_property(JNIEnv *env, jobject property_descr, lDescr **descr, lList **alpp);
static jgdi_result_t string_list_to_list_elem(JNIEnv *env, jobject list, lList **lpp, lDescr *descr, int nm,  lList **alpp);
static jgdi_result_t build_field_filter(JNIEnv *env, jobject field, lCondition **where, lList **alpp);
static jgdi_result_t calendar_to_elem(object_mapping_t *thiz, JNIEnv *env, jobject obj, lListElem *elem, lList **alpp);
static jgdi_result_t elem_to_calendar(object_mapping_t *thiz, JNIEnv *env, lListElem *elem, jobject* obj, lList **alpp);
static object_mapping_t* get_object_mapping(const lDescr *descr);
static jgdi_result_t set_object(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lObject cob, lList **alpp);
static jgdi_result_t get_object(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lObject *cob, lList **alpp);


struct object_mapping_str {
   lDescr* descr;
   jgdi_result_t(*object_to_elem)(object_mapping_t *thiz, JNIEnv *env, jobject obj, lListElem *elem, lList **alpp);
   jgdi_result_t(*elem_to_object)(object_mapping_t *thiz, JNIEnv *env, lListElem *elem, jobject* obj, lList **alpp);
} ;

static object_mapping_t OBJECT_MAPPINGS [] = {
   { &(TM_Type[0]),
     calendar_to_elem,
     elem_to_calendar },
     { NULL, NULL, NULL }
};




/*
 * Class:     com_sun_grid_jgdi_jni_JGDI
 * Method:    nativeClose
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClose(JNIEnv *env, jobject jgdi, jint ctx_index) {
   sge_gdi_ctx_class_t *ctx = NULL;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClose");

   pthread_mutex_lock(&sge_gdi_ctx_mutex);
   ctx = sge_gdi_ctx_array[ctx_index];
   sge_gdi_ctx_array[ctx_index] = NULL;
   pthread_mutex_unlock(&sge_gdi_ctx_mutex);
   if (ctx) {
      cl_com_handle_t *handle = cl_com_get_handle(ctx->get_component_name(ctx), 0);
      cl_commlib_shutdown_handle(handle, false);
      sge_gdi_ctx_class_destroy(&ctx);
   } else {
      THROW_ERROR((env, JGDI_ERROR, "ctx is NULL"));
   }

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_JGDIBaseImpl
 * Method:    nativeInit
 * Signature: ()V
 */
JNIEXPORT jint JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeInit(JNIEnv *env, jobject jgdi, jstring url_obj) {
   
   char* argv[] = { "jgdi" };
   int argc = 1;
   jint ret = -1;
   const char *url = NULL;
   jstring username_obj = NULL;
   const char *username = NULL;
   jobject private_key_obj = NULL;
   jobject certificate_obj = NULL;
   const char* private_key = NULL;
   const char* certificate = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   int i;
   int ctx_index = -1;
   jgdi_result_t res = JGDI_SUCCESS;
   lList *alp = NULL;
   
   DENTER_MAIN(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeInit");

   if (url_obj == NULL) {
      THROW_ERROR((env, JGDI_NULL_POINTER, "url_obj is null"));
      ret = -1;
      goto error;
   }

   res = SecurityHelper_static_getUsername(env, &username_obj, &alp);
   if (res != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, res, alp);
      ret = -1;
      goto error;
   }
   res = SecurityHelper_static_getPrivateKey(env, &private_key_obj, &alp);
   if (res != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, res, alp);
      ret = -1;
      goto error;
   }
   
   res = SecurityHelper_static_getCertificate(env, &certificate_obj, &alp);
   if (res != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, res, alp);
      ret = -1;
      goto error;
   }
   
   url = (*env)->GetStringUTFChars(env, url_obj, 0);
   if (username_obj != NULL) {
      username = (*env)->GetStringUTFChars(env, username_obj, 0);
   }
   if (private_key_obj != NULL) {
      private_key = (*env)->GetStringUTFChars(env, private_key_obj, 0);
   }
   if (certificate_obj != NULL) {
      certificate = (*env)->GetStringUTFChars(env, certificate_obj, 0);
   }

   pthread_mutex_lock(&sge_gdi_ctx_mutex);
   i = 0;
   while(true) {
      if (i>=MAX_GDI_CTX_ARRAY_SIZE) {
         pthread_mutex_unlock(&sge_gdi_ctx_mutex);
         THROW_ERROR((env, JGDI_ILLEGAL_STATE, "sge_gdi_ctx_array is full"));
         ret = -1;
         goto error;
      }
      if (sge_gdi_ctx_array[i] == NULL) {
         dstring component_name = DSTRING_INIT;
         
         sge_dstring_sprintf(&component_name, "%s-%d", prognames[JGDI_PROGNAME], i);
         
         ctx = sge_gdi_ctx_class_create_from_bootstrap(JGDI_PROGNAME,
                                                       sge_dstring_get_string(&component_name),
                                                       MAIN_THREAD,
                                                       threadnames[MAIN_THREAD],
                                                       url, username, &alp);
         sge_dstring_free(&component_name);

         /*
         ** TODO: find a more consistent solution for logging -> sge_log()
         **       to suppress any console log output
         */
         log_state_set_log_verbose(0);
         sge_gdi_set_thread_local_ctx(ctx);
   
         if (ctx == NULL) {
            pthread_mutex_unlock(&sge_gdi_ctx_mutex);
            throw_error_from_answer_list(env, JGDI_ERROR, alp);
            ret = -1;
            goto error;
         } else {
            sge_gdi_ctx_array[i] = ctx;
            ctx_index = i;
            pthread_mutex_unlock(&sge_gdi_ctx_mutex);
            ret = 0;
            break;
         }
      }
      i++;
   }
   
   /* for csp system we need the private key and the certificate of the user */
   ctx->set_private_key(ctx, private_key);
   ctx->set_certificate(ctx, certificate);
   
   ret = ctx->connect(ctx);
   if (ret != CL_RETVAL_OK) {
      ctx->get_errors(ctx, &alp, true);
      throw_error_from_answer_list(env, JGDI_ERROR, alp);
      ret = -1;
      goto error;
   }

error:

   if (url != NULL) {
      (*env)->ReleaseStringUTFChars(env, url_obj, url);
   }
   if (username != NULL) {
      (*env)->ReleaseStringUTFChars(env, username_obj, username);
   }
   if (private_key != NULL) {
      (*env)->ReleaseStringUTFChars(env, private_key_obj, private_key);
   }
   if (certificate != NULL) {
      (*env)->ReleaseStringUTFChars(env, certificate_obj, certificate);
   }
   
   lFreeList(&alp);
   
   sge_gdi_set_thread_local_ctx(NULL);
   if (ret < 0) {
      if (ctx_index >= 0) {
         pthread_mutex_lock(&sge_gdi_ctx_mutex);
         sge_gdi_ctx_array[ctx_index] = NULL;
         pthread_mutex_unlock(&sge_gdi_ctx_mutex);
      }
      sge_gdi_ctx_class_destroy(&ctx);
   } else {
      ret = ctx_index;
   }
   
   DRETURN(ret);
}

/*
 * Class:     com_sun_grid_jgdi_JGDI
 * Method:    nativeGetEnv
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDI_nativeGetEnv(JNIEnv *env, jobject jgdi, jstring name) {
   const char * env_name = NULL;
   char* buf = NULL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDI_nativeGetEnv");
   
   if (name == NULL) {
      DRETURN(NULL);
   }
   env_name = (*env)->GetStringUTFChars(env, name, 0);
   if (env_name == NULL) {
      DRETURN(NULL);
   }
   
   buf = getenv(env_name);

   (*env)->ReleaseStringUTFChars(env, name, env_name);
   
   if (buf) {
      DRETURN((*env)->NewStringUTF(env, buf));
   } else {
      DRETURN(NULL);
   }
}

jgdi_result_t getGDIContext(JNIEnv *env, jobject jgdi, sge_gdi_ctx_class_t **ctx, lList **alpp) {
   static jmethodID get_ctx_mid = NULL;
   int ctx_index = 0;

   DENTER(JGDI_LAYER, "getGDIContext");

   if (get_ctx_mid == NULL) {
      jclass cls = (*env)->GetObjectClass(env, jgdi);
      
      get_ctx_mid = get_methodid(env, cls, "getCtxIndex", "()I", alpp);
      
      if (get_ctx_mid == NULL) {
         answer_list_add(alpp, "method getCtxIndex in jgdi class not found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
   }
   
   ctx_index = (*env)->CallIntMethod(env, jgdi, get_ctx_mid);
   if (test_jni_error(env, "getGDIContext failed", alpp)) {
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   *ctx = sge_gdi_ctx_array[ctx_index];
   DRETURN(JGDI_SUCCESS);
}


/*
 * Class:    com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeGetActQMaster
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetActQMaster(JNIEnv *env, jobject jgdi) {

   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_bootstrap_state_class_t *bs = NULL;
   const char* master = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDI_nativeGetActQMaster");
   
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS)  {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(NULL);
   }
   bs = ctx->get_sge_bootstrap_state(ctx);
   if (!bs) {
      THROW_ERROR((env, JGDI_ILLEGAL_STATE, "bootstrap state not found"));
      DRETURN(NULL);
   }

   master = ctx->get_master(ctx, false);
   if (master != NULL) {
      DRETURN((*env)->NewStringUTF(env, master));
   }
   DRETURN(NULL);
   
}

/*
 * Class:    com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeGetSgeQmasterPort
 * Signature: ()I;
 */
JNIEXPORT jint JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSgeQmasterPort(JNIEnv *env, jobject jgdi) {

   lList *alp = NULL;
   jint master_port = -1;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_bootstrap_state_class_t *bs = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;

   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDI_nativeGetSgeQmasterPort");
   
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS)  {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(master_port);
   }
   bs = ctx->get_sge_bootstrap_state(ctx);
   if (!bs) {
      THROW_ERROR((env, JGDI_ILLEGAL_STATE, "bootstrap state not found"));
      DRETURN(master_port);
   }

   master_port = ctx->get_sge_qmaster_port(ctx);
   DRETURN(master_port);
}

/*
 * Class:    com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeGetSgeExecdPort
 * Signature: ()I;
 */
JNIEXPORT jint JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSgeExecdPort(JNIEnv *env, jobject jgdi) {

   lList *alp = NULL;
   jint execd_port = -1;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_bootstrap_state_class_t *bs = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;

   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDI_nativeGetSgeExecdPort");
   
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS)  {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(execd_port);
   }
   bs = ctx->get_sge_bootstrap_state(ctx);
   if (!bs) {
      THROW_ERROR((env, JGDI_ILLEGAL_STATE, "bootstrap state not found"));
      DRETURN(execd_port);
   }

   execd_port = ctx->get_sge_execd_port(ctx);
   DRETURN(execd_port);
}

/*
 * Class:     com_sun_grid_jgdi_JGDI
 * Method:    nativeGetAdminUser
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetAdminUser(JNIEnv *env, jobject jgdi) {
   
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   sge_bootstrap_state_class_t *bs = NULL;
   const char *admin_user = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDI_nativeGetAdminUser");

   if ((ret=getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(NULL);
   }
   bs = ctx->get_sge_bootstrap_state(ctx);
   if (!bs) {
      THROW_ERROR((env, JGDI_ILLEGAL_STATE, "bootstrap state not found"));
      DRETURN(NULL);
   }

   admin_user = bs->get_admin_user(bs);
   if (admin_user != NULL) {
      DRETURN((*env)->NewStringUTF(env, admin_user));
   }
   DRETURN(NULL);
}


JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSGERoot(JNIEnv *env, jobject jgdi) {
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   const char *sge_root = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSGERoot");

   if ((ret=getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(NULL);
   }
   sge_root = ctx->get_sge_root(ctx);
   if (sge_root != NULL) {
      DRETURN((*env)->NewStringUTF(env, sge_root));
   }
   DRETURN(NULL);
}

JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSGECell(JNIEnv *env, jobject jgdi) {
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   const char *sge_cell = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(JGDI_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSGECell");

   if ((ret=getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
      lFreeList(&alp);
      DRETURN(NULL);
   }
   sge_cell = ctx->get_cell_root(ctx);
   if (sge_cell != NULL) {
      DRETURN((*env)->NewStringUTF(env, sge_cell));
   }
   DRETURN(NULL);
}

jgdi_result_t listelem_to_obj(JNIEnv *env, lListElem *ep, jobject *obj, const lDescr* descr, jclass clazz, lList **alpp) {
   jobject obj_descr = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   jint prop_count;
   int i;

   DENTER(JGDI_LAYER, "listelem_to_obj");

   if (obj == NULL) {
      answer_list_add(alpp, "listelem_to_obj: obj must not be null", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_NULL_POINTER);
   }

   /* If the elem is NULL, we simple return NULL */
   if (ep == NULL) {
       *obj = NULL;
       DRETURN(ret);
   }
   
   /* Get the descriptor class of the bean class */
   if ((ret=Util_static_getDescriptor(env, clazz, &obj_descr, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   /* Create a new instance of the class by its class descriptor */
   if ((ret=ClassDescriptor_newInstance(env, obj_descr, obj, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   /* get the property count */
   if ((ret=ClassDescriptor_getPropertyCount(env, obj_descr, &prop_count, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   /* fill all property fields */
   for (i = 0; i < prop_count; i++) {
     jobject prop_descr = NULL;
     if ((ret=ClassDescriptor_getProperty(env, obj_descr, i, &prop_descr, alpp)) != JGDI_SUCCESS) {
        DRETURN(ret);
     }
     if ((ret=set_object_attribute(env, ep, descr, *obj, prop_descr, alpp)) != JGDI_SUCCESS) {
        DRETURN(ret);
     }
   }

   DRETURN(ret);
}

jgdi_result_t obj_to_listelem(JNIEnv *env, jobject obj, lListElem **elem, const lDescr* descr, lList **alpp) {
   jobject obj_descr;
   jint prop_count;
   int i;
   jobject prop_descr;
   jclass clazz;
   jgdi_result_t ret = JGDI_SUCCESS;

   DENTER(JGDI_LAYER, "obj_to_listelem");
   
   if (obj == NULL) {
      *elem = NULL;
      ret = JGDI_SUCCESS;
      goto error;
   }
   
   if ((ret = Object_getClass(env, obj, &clazz, alpp)) != JGDI_SUCCESS) {
      goto error;
   }

   /* Get the descriptor class of the bean class */
   if ((ret=Util_static_getDescriptor(env, clazz, &obj_descr, alpp)) != JGDI_SUCCESS) {
      goto error;
   }

   /* Create a new instance of the class by its class descriptor */
   *elem = lCreateElem(descr);
   if (!(*elem)) {
      answer_list_add(alpp, "lCreateElem failed", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      ret = JGDI_ILLEGAL_STATE;
      goto error;
   }
   
   /* get the property count */
   if ((ret=ClassDescriptor_getPropertyCount(env, obj_descr, &prop_count, alpp)) != JGDI_SUCCESS) {
      goto error;
   }
   
   for (i = 0; i < prop_count; i++) {
     jboolean is_set = false;

     if ((ret=ClassDescriptor_getProperty(env, obj_descr, i, &prop_descr, alpp)) != JGDI_SUCCESS) {
        goto error;
     }
     if ((ret=PropertyDescriptor_isSet(env, prop_descr, obj, &is_set, alpp)) != JGDI_SUCCESS) {
        goto error;
     }
     if (is_set == true) {
        if ((ret=set_elem_attribute(env, *elem, descr, obj, prop_descr, alpp)) != JGDI_SUCCESS) {
           goto error;
        }
     }
   }
#if 0 
{
   lInit(nmv);
   lWriteElemTo(*elem, stdout);
}
#endif

error:

   if (ret != JGDI_SUCCESS) {
      lFreeElem(elem);
   }
   DRETURN(ret);
}

/**
 *   set a attribute of a java object into a cull element
 *   @param  elem          the cull object
 *   @param  target        java object
 *   @param  prop_descr the property descriptor
 */
jgdi_result_t set_elem_attribute(JNIEnv* env, lListElem *ep, const lDescr* descr, jobject obj, jobject prop_descr, lList **alpp) {
   jclass object_class;
   int pos, type = lEndT;
   jstring property_name_str;
   const char* property_name;
   bool unknown_type=false;
   jint elem_field_name;
   jgdi_result_t result = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "set_elem_attribute");
   
   if (obj == NULL) {
      answer_list_add(alpp, "set_elem_attribute: obj is NULL, can't call GetObjectClass(env, NULL)", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   object_class = (*env)->GetObjectClass(env, obj);

   if (PropertyDescriptor_getPropertyName(env, prop_descr, &property_name_str, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   if (property_name_str == NULL) {
       answer_list_add(alpp, "set_elem_attribute: property_name_str is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
       DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   if (PropertyDescriptor_getCullFieldName(env, prop_descr, &elem_field_name, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   if (elem_field_name == 0) {
       answer_list_add(alpp, "set_elem_attribute: elem_field_name is 0", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
       DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   property_name = (*env)->GetStringUTFChars(env, property_name_str, 0);
   if (property_name == NULL) {
       answer_list_add(alpp, "set_elem_attribute: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
       DRETURN(JGDI_ERROR);
   }
   
   pos = lGetPosInDescr(descr, elem_field_name);
   if (pos < 0) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "field %s not found in descriptor", lNm2Str(elem_field_name));
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   type = lGetPosType(descr, pos);
   
   switch (type) {
      case lBoolT:
         {
            lBool b;
            result = get_bool(env, object_class, obj, property_name, &b, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosBool(ep, pos, b);
            }
            break;
         }
      case lUlongT:
         {
            u_long32 u;
            result = get_int(env, object_class, obj, property_name, &u, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosUlong(ep, pos, u);
            }
            break;
         }
      case lLongT:
         {
            lLong u;
            result = get_long(env, object_class, obj, property_name, &u, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosLong(ep, pos, u);
            }
            break;
         }
      case lStringT:
         {
            char* str = NULL;
            result = get_string(env, object_class, obj, property_name, &str, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosString(ep, pos, str);
               sge_free(&str);
            } else {
               lSetPosString(ep, pos, NULL);
            }            
            break;
         }
      case lHostT:
         {
            char* str = NULL;
            result = get_string(env, object_class, obj, property_name, &str, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosHost(ep, pos, str);
               sge_free(&str);
            } else {
               lSetPosHost(ep, pos, NULL);
            }            
            break;
         }
      case lDoubleT:
         {
            double value;
            result = get_double(env, object_class, obj, property_name, &value, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosDouble(ep, pos, value);
            }
            break;
         }
      case lFloatT:
         {
            float value;
            result = get_float(env, object_class, obj, property_name, &value, alpp);
            if (result == JGDI_SUCCESS)  {
               lSetPosFloat(ep, pos, value);
            }
            break;
         }
      case lListT:
         {
           jclass map_prop_descr_class;
           jclass map_list_prop_descr_class;
           jclass prop_descr_class; 
           jboolean is_map;
           lList *list = NULL;
           jboolean is_map_list;
           
           map_prop_descr_class = MapPropertyDescriptor_find_class(env, alpp);
           if (map_prop_descr_class == NULL) {
              result = JGDI_ERROR;
              break;
           } 
           
           map_list_prop_descr_class = MapListPropertyDescriptor_find_class(env, alpp);
           if (map_list_prop_descr_class == NULL) {
              result = JGDI_ERROR;
              break;
           } 

           if (Object_getClass(env, prop_descr, &prop_descr_class, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }

           if (Class_isAssignableFrom(env, map_list_prop_descr_class, prop_descr_class, &is_map_list, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }
           if (Class_isAssignableFrom(env, map_prop_descr_class, prop_descr_class, &is_map, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }

           if (is_map_list) {
              result = get_map_list(env, object_class, obj, prop_descr, &list, alpp);
           } else if (is_map) {
              result = get_map(env, object_class, obj, prop_descr, &list, alpp);
           } else {
              result = get_list(env, object_class, obj, prop_descr, &list, alpp);
           }
           if (result == JGDI_SUCCESS) {
              lSetPosList(ep, pos, list);
           }
           break;
         }
      case lObjectT:
         {
           jclass prop_descr_class; 
           lObject cob = NULL;
           
           if (Object_getClass(env, prop_descr, &prop_descr_class, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }

           result = get_object(env, object_class, obj, prop_descr, &cob, alpp);

           if (result == JGDI_SUCCESS) {
              lSetPosObject(ep, pos, cob);
           }
           break;
         }
      case lRefT:
         /* TODO implement reference converion */
         break;
      case lEndT:
         /* Attribute not set in cull object, skip it */
         /* TODO set flag in java object */
         break;
      default:
        /* error handling */
        unknown_type = true;
   }
   
   if (property_name) {
      (*env)->ReleaseStringUTFChars(env, property_name_str, property_name);
   }
   
   if (unknown_type) {
     answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                             "unknown cull type %d found", type);
     DRETURN(JGDI_ERROR);
   }

   DRETURN(result);
}

/**
 *   set a attribute of a cull object into the java object
 *   @param  elem          the cull object
 *   @param  target        java object
 *   @param  prop_descr the property descriptor
 */
jgdi_result_t set_object_attribute(JNIEnv* env, lListElem *ep, const lDescr* descr, jobject target, jobject prop_descr, lList **alpp) {
   jclass target_class;
   int pos, type = lEndT;
   jstring property_name_str;
   const char* property_name;
   bool unknown_type=false;
   jint elem_field_name;
   jgdi_result_t result = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "set_object_attribute");

   target_class = (*env)->GetObjectClass(env, target);
   if (!target_class) {
      DRETURN(JGDI_ERROR);
   }
   if ((result = PropertyDescriptor_getPropertyName(env, prop_descr, &property_name_str, alpp)) != JGDI_SUCCESS) {
      DRETURN(result);
   }
   if (property_name_str == NULL) {
       answer_list_add(alpp, "set_object_attribute: property_name_str is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
       DRETURN(JGDI_ERROR);
   }
   
   if ((result = PropertyDescriptor_getCullFieldName(env, prop_descr, &elem_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(result);
   }
   if (elem_field_name == 0) {
       answer_list_add(alpp, "set_object_attribute: elem_field_name is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
       DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   property_name = (*env)->GetStringUTFChars(env, property_name_str, 0);
   if (property_name == NULL) {
      answer_list_add(alpp, "set_object_attribute: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ERROR);
   }
   
   pos = lGetPosInDescr(descr, elem_field_name);
   if (pos < 0) {
#if 0   
      /* 
      ** AA TODO: for reduced descr, a field may not be available 
      **     how to we handle this correctly ?
      **     java obj has more props than specific cull obj
      **       
      */
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "field %s not found in descriptor", lNm2Str(elem_field_name));
      DRETURN(JGDI_ILLEGAL_STATE);
#else
      DRETURN(result);
#endif
   }
   
   jgdi_log_printf(env, JGDI_LOGGER, FINER, "Convert property %s", property_name);
   
   type = lGetPosType(descr, pos);
   
   switch (type) {
      case lBoolT:
         {
            lBool b = lGetPosBool(ep, pos);
            result = set_bool(env, target_class, target, property_name, b, alpp);
            break;
         }
      case lUlongT:
         {
            u_long32 u = lGetPosUlong(ep, pos);
            result = set_int(env, target_class, target, property_name, u, alpp);
            break;
         }
      case lLongT:
         {
            lLong u = lGetPosLong(ep, pos);
            result = set_long(env, target_class, target, property_name, u, alpp);
            break;
         }
      case lStringT:
         {
            const char* str = lGetPosString(ep,pos);
            result = set_string(env, target_class, target, property_name, str, alpp);
            break;
         }
      case lHostT:
         {
            const char* str = lGetPosHost(ep,pos);
            result = set_string(env, target_class, target, property_name, str, alpp);
            break;
         }
      case lDoubleT:
         {
            double value = lGetPosDouble(ep,pos);
            result = set_double(env, target_class, target, property_name, value, alpp);
            break;
         }
      case lFloatT:
         {
            float value = lGetPosFloat(ep,pos);
            result = set_float(env, target_class, target, property_name, value, alpp);
            break;
         }
      case lListT:
         {
           lList* list = lGetPosList(ep, pos);
           
           jclass map_prop_descr_class;
           jclass map_list_prop_descr_class;
           jclass prop_descr_class; 
           jboolean is_map;
           jboolean is_map_list;
           
           map_prop_descr_class = MapPropertyDescriptor_find_class(env, alpp);
           if (map_prop_descr_class == NULL) {
              result = JGDI_ERROR;
              break;
           }     
           
           map_list_prop_descr_class = MapListPropertyDescriptor_find_class(env, alpp);
           if (map_list_prop_descr_class == NULL) {
              result = JGDI_ERROR;
              break;
           } 

           if (Object_getClass(env, prop_descr, &prop_descr_class, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }

           if (Class_isAssignableFrom(env, map_list_prop_descr_class, prop_descr_class, &is_map_list, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }
           
           if (Class_isAssignableFrom(env, map_prop_descr_class, prop_descr_class, &is_map, alpp) != JGDI_SUCCESS) {
              result = JGDI_ERROR;
              break;
           }
           
           if (is_map_list) {
              jgdi_log_printf(env, JGDI_LOGGER, FINER, "Property %s is a map list", property_name);
              
              result = set_map_list(env, target_class, target, prop_descr, list, alpp);
              if (result != JGDI_SUCCESS) {
                 answer_list_add_sprintf(alpp,  STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                         "set_object_attribute: set_map_list of property %s failed", 
                                         property_name);
              }
           } else if (is_map) {
              jgdi_log_printf(env, JGDI_LOGGER, FINER, "Property %s is a map", property_name);

              result = set_map(env, target_class, target, prop_descr, list, alpp);
              if (result != JGDI_SUCCESS) {
                 answer_list_add_sprintf(alpp,  STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                         "set_object_attribute: set_map of property %s failed", 
                                         property_name);
              }
           } else {
/* TODO handle primitive types in set_list */                 
              jgdi_log_printf(env, JGDI_LOGGER, FINER, "Property %s is a list", property_name);
              if (result != JGDI_SUCCESS) {
                 break;
              }
              result = set_list(env, target_class, target, prop_descr, list, alpp);
              if (result != JGDI_SUCCESS) {
                 answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                         "set_object_attribute: set_list of property %s failed", 
                                         property_name);
              }
           }
           break;
         }
      case lObjectT:
         {
           lObject cob = lGetPosObject(ep,pos);
           
           if (cob) {
              jclass prop_descr_class; 
              
              if (Object_getClass(env, prop_descr, &prop_descr_class, alpp) != JGDI_SUCCESS) {
                 result = JGDI_ERROR;
                 break;
              }
              jgdi_log_printf(env, JGDI_LOGGER, FINER, "Property %s is an object", property_name);
              result = set_object(env, target_class, target, prop_descr, cob, alpp);
              if (result != JGDI_SUCCESS) {
                 answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                         "set_object_attribute: set_object of property %s failed", 
                                         property_name);
              }
           }
           break;
         }
      case lEndT:
         /* Attribute not set in cull object, skip it */
         /* TODO set flag in java object */
         break;
      default:
        /* error handling */
        unknown_type = true;
   }
   
   if (property_name) {
      (*env)->ReleaseStringUTFChars(env, property_name_str, property_name);
   }
   
   if (unknown_type) {
     answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                             "unknown cull type %d found", type);
     DRETURN(JGDI_ERROR);
   }

   DRETURN(result);
}


static jgdi_result_t get_map(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList **list, lList **alpp) 
{

   jobject    iter = NULL;
   jobject key_set = NULL;
   jint        key_field_name;
   int        key_field_pos;
   int        key_field_type;
   jint        value_field_name;
   lList      *tmp_list = NULL;
   jboolean   has_next = false;
   lDescr     *descr = NULL;
   jgdi_result_t        ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "get_map");

   if ((ret=MapPropertyDescriptor_getKeys(env, property_descr, bean, &key_set, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=MapPropertyDescriptor_getKeyCullFieldName(env, property_descr, &key_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=MapPropertyDescriptor_getValueCullFieldName(env, property_descr, &value_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=Set_iterator(env, key_set, &iter, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if ((ret = get_descriptor_for_property(env, property_descr, &descr, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   key_field_pos = lGetPosInDescr(descr, key_field_name);
   if (key_field_pos < 0) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "key field %s not found in descriptor", lNm2Str(key_field_pos));
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   key_field_type = lGetPosType(descr, key_field_pos);

   if ((ret=Iterator_hasNext(env, iter, &has_next, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   } else if (has_next == false) {
      /* intialize the default value with an empty list */
      jstring key_obj;
      const char* key;
      lListElem *elem = NULL;
      
      if ((ret=MapPropertyDescriptor_getDefaultKey(env, property_descr, &key_obj, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }

      if (key_obj == NULL) {
         answer_list_add(alpp, "get_map: key_obj is NULL (1)", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      
      *list = lCreateList("", descr);
      if (!*list) {
         answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
         
      elem =  lCreateElem(descr);
      if (!elem) {
         answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         lFreeList(list);
         DRETURN(JGDI_ILLEGAL_STATE);
      }

      lAppendElem(*list, elem);
      
      key = (*env)->GetStringUTFChars(env, key_obj, 0);
      if (key == NULL) {
         answer_list_add(alpp, "get_map: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         lFreeList(list);
         DRETURN(JGDI_ERROR);
      }
      switch(key_field_type) {
         case lHostT:
            lSetPosHost(elem, key_field_pos, key);
            break;
         case lStringT:
            lSetPosString(elem, key_field_pos, key);
            break;
         default:
            answer_list_add(alpp, "type key field must be string or host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = JGDI_ERROR;
      }
      (*env)->ReleaseStringUTFChars(env, key_obj, key);
      if (ret != JGDI_SUCCESS) {
         lFreeList(list);
         DRETURN(ret);
      }
      DRETURN(JGDI_SUCCESS);
   } else {
      int value_field_pos;
      int value_field_type;
      jboolean  has_cull_wrapper = false;
      jint      content_field_name = 0;
      int       content_field_type = lEndT;
      int       content_field_pos = lEndT;

      value_field_pos = lGetPosInDescr(descr, value_field_name);
      if (value_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "value field %s not found in descriptor", lNm2Str(value_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      
      value_field_type = lGetPosType(descr, value_field_pos);
      
      if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      
      if (has_cull_wrapper) {
         if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
         if (content_field_name >= 0) {
            content_field_pos = lGetPosInDescr(&descr[value_field_pos], content_field_name);
            if (content_field_pos < 0) {
               answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                       "content field %s not found in descriptor", lNm2Str(content_field_name));
               DRETURN(JGDI_ILLEGAL_STATE);
            }
            content_field_type = lGetPosType(&descr[value_field_pos], content_field_pos);
         }
      }
      
      
      tmp_list = lCreateList("", descr);
      if (!tmp_list) {
         answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      while (TRUE) {
         lListElem *elem;
         jstring key_obj;
         jobject value_obj;
         const char* key;
         
         if ((ret=Iterator_hasNext(env, iter, &has_next, alpp)) != JGDI_SUCCESS) {
            break;
         } else if (has_next == false) {
            break;
         }

         if ((ret=Iterator_next(env, iter, &key_obj, alpp)) != JGDI_SUCCESS) {
            break;
         }
         if ((ret=MapPropertyDescriptor_get(env, property_descr, bean, key_obj, &value_obj, alpp)) != JGDI_SUCCESS) {
            break;
         }
         if (key_obj == NULL) {
            answer_list_add(alpp, "get_map: key_obj is NULL (2)", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = JGDI_ILLEGAL_STATE;
            break;
         }
         
         elem = lCreateElem(descr);
         if (elem == NULL) {
            answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            ret = JGDI_ILLEGAL_STATE;
            break;
         }
         
         lAppendElem(tmp_list, elem);

         key = (*env)->GetStringUTFChars(env, key_obj, 0);
         if (key == NULL) {
            answer_list_add(alpp, "get_map: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            ret = JGDI_ERROR;
            break;
         }
         switch(key_field_type) {
            case lStringT:
              lSetPosString(elem, key_field_pos, key);
              break;
            case lHostT:
              lSetPosHost(elem, key_field_pos, key);
              break;
            default:
              answer_list_add(alpp, "type key field must be string or host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
              ret = JGDI_ERROR;
         }                
         (*env)->ReleaseStringUTFChars(env, key_obj, key);
         if (ret != JGDI_SUCCESS) {
            break;
         }
         
         if (has_cull_wrapper) {
            lListElem *sub_elem = lCreateElem(&descr[value_field_pos]);
            if (sub_elem == NULL) {
               answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
               ret = JGDI_ILLEGAL_STATE;
               break;
            }

            if ((ret=set_value_in_elem(env, value_obj, sub_elem, content_field_type, content_field_pos, alpp)) != JGDI_SUCCESS) {
               lFreeElem(&sub_elem);
               break;
            }
            lSetPosObject(elem, content_field_pos, sub_elem);
         } else {
            if ((ret=set_value_in_elem(env, value_obj, elem, value_field_type, value_field_pos, alpp)) != JGDI_SUCCESS) {
               break;
            }
         }
      }
   }
   
   if (ret != JGDI_SUCCESS) {
      lFreeList(&tmp_list);
      DRETURN(ret);
   } else {
      *list = tmp_list;
      DRETURN(JGDI_SUCCESS);
   }
}

static jgdi_result_t set_map_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList *lp, lList **alpp)
{

   const lDescr*   descr;
   lDescr*   elem_descr;
   lListElem* ep = NULL;
   jint key_field_name;
   int key_field_pos;
   int key_field_type;
   jint value_field_name;
   int value_field_pos;
   jclass elem_class;
   const char* elem_class_name = NULL;
   char property_name[512];
   jboolean has_cull_wrapper = false;
   jint content_field_name = 0;
   int  content_field_pos = lEndT;
   int  content_field_type = 0;
   jgdi_result_t  ret = JGDI_SUCCESS;

   DENTER(BASIS_LAYER, "set_map_list");

   {
      jstring property_name_obj = NULL;
      const char* tmp_property_name = NULL;
      if ((ret=PropertyDescriptor_getPropertyName(env, property_descr, &property_name_obj, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if (property_name_obj == NULL) {
         answer_list_add(alpp, "set_map_list: property_name_obj is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      tmp_property_name = (*env)->GetStringUTFChars(env, property_name_obj, 0);
      if (tmp_property_name == NULL) {
         answer_list_add(alpp, "set_map_list: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      strncpy(property_name, tmp_property_name, sizeof(tmp_property_name) - 1);
      (*env)->ReleaseStringUTFChars(env, property_name_obj, tmp_property_name);
   }
   
   if ((ret=MapListPropertyDescriptor_getKeyCullFieldName(env, property_descr, &key_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if ((ret=MapListPropertyDescriptor_getValueCullFieldName(env, property_descr, &value_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if (lp == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "property %s cannot be an empty list", property_name);
      DRETURN(JGDI_NULL_POINTER);
   }

   descr = lGetListDescr(lp);
   
   ret = get_list_descriptor_for_property(env, property_descr, &elem_descr, alpp); 
   if (ret != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   elem_class_name = get_classname_for_descr(elem_descr);
   if (elem_class_name == NULL) {
      answer_list_add(alpp, "set_map_list: class name for elem_descr not found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ERROR);
   }
   
   elem_class = (*env)->FindClass(env, elem_class_name);
   if (test_jni_error(env, "", NULL)) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "set_map_list: class %s for elem_descr not found", elem_class_name);
      DRETURN(JGDI_ERROR);
   }
   key_field_pos = lGetPosInDescr(descr, key_field_name);
   if (key_field_pos < 0) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "key field %s not found in descriptor", lNm2Str(key_field_name));
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   key_field_type =  lGetPosType(descr, key_field_pos);
   
   value_field_pos = lGetPosInDescr(descr, value_field_name);
   if (value_field_pos < 0) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "value field %s not found in descriptor", lNm2Str(value_field_name));
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   if (has_cull_wrapper == true) {
      DPRINTF(("Property %s has a cull wrapper\n", property_name));
      if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
   } else {
      DPRINTF(("Property %s has no cull wrapper\n", property_name));
   }

   for_each(ep, lp) {
      lList  *sub_list = lGetPosList(ep, value_field_pos);
      lListElem *sub_ep = NULL;
      const char *key = NULL;
      jstring key_obj = NULL;
      int value_count = 0;
      switch(key_field_type) {
         case lStringT:
            key = lGetPosString(ep, key_field_pos);
            break;
         case lHostT:
            key = lGetPosHost(ep, key_field_pos);
            break;
         default:
            answer_list_add(alpp, "key of a map must be of type String or Host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
      }
      key_obj = (*env)->NewStringUTF(env, key);
      
      for_each(sub_ep, sub_list) {
         jobject value_obj = NULL;
         if (has_cull_wrapper == true) {
            
            if (content_field_pos == lEndT) {
               content_field_pos = lGetPosInDescr(sub_ep->descr, content_field_name);
               if (content_field_pos < 0) {
                  answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                          "content field %s not found in descriptor", lNm2Str(content_field_name));
                  DRETURN(JGDI_ILLEGAL_STATE);
               }
               content_field_type = lGetPosType(sub_ep->descr, content_field_pos);
            }
            if ((ret = create_object_from_elem(env, sub_ep, &value_obj, content_field_type, content_field_pos, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            }
         } else {
            if ((ret = listelem_to_obj(env, sub_ep, &value_obj, elem_descr, elem_class, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            }
         }
         if (value_obj != NULL) {
            if ((ret=MapListPropertyDescriptor_add(env, property_descr, bean, key_obj, value_obj, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            }
            value_count++;
         } else {
            /* skip NULL value_obj */
         }   
      }
      if (value_count == 0) {
         if ((ret=MapListPropertyDescriptor_addEmpty(env, property_descr, bean, key_obj, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
      }
   }

   DRETURN(JGDI_SUCCESS);
}

static jgdi_result_t get_map_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList **list, lList **alpp) 
{

   jclass    property_descr_class = NULL;
   jobject   iter = NULL;
   jobject   key_set = NULL;
   jint      key_field_name = 0;
   int       key_field_pos = lEndT;
   int       key_field_type = 0;
   jint      value_field_name = 0;
   lList     *tmp_list = NULL;
   char      property_name[100];
   jboolean  has_next = false;
   lDescr    *descr = NULL;
   jgdi_result_t       ret = JGDI_SUCCESS;

   DENTER(BASIS_LAYER, "get_map_list");
   
   {
      jstring property_name_obj = NULL;
      const char* tmp_name = NULL;
      lInit(nmv);
      if ((ret=PropertyDescriptor_getPropertyName(env, property_descr, &property_name_obj, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if (property_name_obj == NULL) {
         answer_list_add(alpp, "get_map_list: property_name_obj is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      tmp_name = (*env)->GetStringUTFChars(env, property_name_obj, 0);
      if (tmp_name == NULL) {
         answer_list_add(alpp, "get_map_list: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      strncpy(property_name, tmp_name, 100);
      (*env)->ReleaseStringUTFChars(env, property_name_obj, tmp_name);
   }
   
   if ((ret=Object_getClass(env, property_descr, &property_descr_class, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=MapListPropertyDescriptor_getKeys(env, property_descr, bean, &key_set, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=MapListPropertyDescriptor_getKeyCullFieldName(env, property_descr, &key_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=MapListPropertyDescriptor_getValueCullFieldName(env, property_descr, &value_field_name, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=get_descriptor_for_property(env, property_descr, &descr, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   key_field_pos = lGetPosInDescr(descr, key_field_name);
   if (key_field_pos < 0) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "field %s not found in desriptor of property %s",
                              lNm2Str(key_field_name), property_name); 
      DRETURN(JGDI_ERROR);
   }

   key_field_type = lGetPosType(descr, key_field_pos);

   if ((ret = Set_iterator(env, key_set, &iter, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if ((ret=Iterator_hasNext(env, iter, &has_next, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if (has_next == 0) {
      /* new have empty map */
      /* intialize the default value with an empty list */
      jstring key_obj;
      const char* key;
      lListElem *elem = NULL;
      
      if ((ret=MapListPropertyDescriptor_getDefaultKey(env, property_descr, &key_obj, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if (key_obj == NULL) {
         answer_list_add(alpp, "get_map_list: key_obj is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      *list = lCreateList("", descr);
      if (!*list) {
         answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
         
      elem =  lCreateElem(descr);
      if (!elem) {
         answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         lFreeList(list);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
         
      key = (*env)->GetStringUTFChars(env, key_obj, 0);
      if (key == NULL) {
         answer_list_add(alpp, "get_map_list: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         lFreeList(list);
         lFreeElem(&elem);
         DRETURN(JGDI_ERROR);
      }
      switch(key_field_type) {
         case lHostT:
            lSetPosHost(elem, key_field_pos, key);
            break;
         case lStringT:
            lSetPosString(elem, key_field_pos, key);
            break;
         default:
            answer_list_add(alpp, "type key field must be string or host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = JGDI_ERROR;
      }
      (*env)->ReleaseStringUTFChars(env, key_obj, key);
      if (ret != JGDI_SUCCESS) {
         lFreeList(list);
         lFreeElem(&elem);
      } else {
         lAppendElem(*list, elem);
      }
      DRETURN(ret);
   } else {
      int value_field_pos;
      int key_field_name_pos;
      lDescr    *elem_descr = NULL;

      jboolean  has_cull_wrapper = false;
      jint      content_field_name = 0;
      int       content_field_type = lEndT;
      int       content_field_pos = lEndT;

      value_field_pos = lGetPosInDescr(descr, value_field_name);
      if (value_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "value field %s not found", lNm2Str(value_field_name));
         DRETURN(JGDI_ERROR);
      }
      
      
      key_field_name_pos = lGetPosInDescr(descr, key_field_name);
      if (key_field_name_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "key field %s not found", lNm2Str(key_field_name));
         DRETURN(JGDI_ERROR);
      }
      
      ret = get_list_descriptor_for_property(env, property_descr, &elem_descr, alpp);
      if (ret != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      
      if (has_cull_wrapper) {
         if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
         if (content_field_name >= 0) {
            content_field_pos = lGetPosInDescr(elem_descr, content_field_name);
            if (content_field_pos < 0) {
               answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                       "content field %s not found in descriptor", lNm2Str(content_field_name));
               DRETURN(JGDI_ILLEGAL_STATE);
            }
            content_field_type = lGetPosType(elem_descr, content_field_pos);
         }
      }
      
      
      tmp_list = lCreateList("", descr);
      if (!tmp_list) {
         answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      
      while (TRUE) {
         jstring key_obj;
         jobject value_list = NULL;
         jobject value_iter = NULL;
         jboolean has_next_value = false;
         const char *key;
         lListElem *elem = NULL;
         
         if ((ret=Iterator_hasNext(env, iter, &has_next, alpp)) != JGDI_SUCCESS) {
            break;
         } 
         
         if (has_next == 0) {
            break;
         }

         if ((ret=Iterator_next(env, iter, &key_obj, alpp)) != JGDI_SUCCESS) {
            break;
         }
         if (key_obj == NULL) {
            answer_list_add(alpp, "get_map_list: key_obj is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            ret = JGDI_ILLEGAL_STATE;
            break;
         }
         
         if ((ret=MapListPropertyDescriptor_getList(env, property_descr, bean, key_obj, &value_list, alpp)) != JGDI_SUCCESS) {
            break;
         }
         if ((ret=List_iterator(env, value_list, &value_iter, alpp)) != JGDI_SUCCESS) {
            break;
         }

         if ((ret=Iterator_hasNext(env, value_iter, &has_next_value, alpp)) != JGDI_SUCCESS) {
            break;
         }
         
         /*
         ** there is always an element added for a map list
         ** it is appended to tmp_list to free also the element
         ** in case of error
         */
         elem = lCreateElem(descr);
         if (!elem) {
            answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            ret = JGDI_ILLEGAL_STATE;
            break;
         }
         lAppendElem(tmp_list, elem);

         if (has_next_value) {
            lList     *value_list = NULL;
            value_list = lCreateList("", elem_descr);
            if (!value_list) {
               answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
               ret = JGDI_ILLEGAL_STATE;
               break;
            }
            
            while (has_next_value) {
               jobject value_obj;
               lListElem *value_elem = NULL;
               
               if ((ret=Iterator_next(env, value_iter, &value_obj, alpp)) != JGDI_SUCCESS) {
                  break;
               }
               if (value_obj != NULL) {
                  if (has_cull_wrapper) {
                     value_elem = lCreateElem(elem_descr);
                     if (!value_elem) {
                        answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                        ret = JGDI_ILLEGAL_STATE;
                        break;
                     }
                     if ((ret=set_value_in_elem(env, value_obj, value_elem, content_field_type, content_field_pos, alpp)) != JGDI_SUCCESS) {
                        lFreeElem(&value_elem);
                        break;
                     }
                  } else {
                     if ((ret=obj_to_listelem(env, value_obj, &value_elem, elem_descr, alpp)) != JGDI_SUCCESS) {
                        break;
                     }
                  }
                  lAppendElem(value_list, value_elem);
               }

               if ((ret=Iterator_hasNext(env, value_iter, &has_next_value, alpp)) != JGDI_SUCCESS) {
                  break;
               }
            }
            if (ret != JGDI_SUCCESS) {
               lFreeList(&value_list);
               break;
            }
            
            if (lGetNumberOfElem(value_list) > 0) {
               lSetPosList(elem, value_field_pos, value_list);
            } else {
               lFreeList(&value_list);
               lSetPosList(elem, value_field_pos, NULL);
            }
         } else { /*not has_next_value */
            lSetPosList(elem, value_field_pos, NULL);
         }

         key = (*env)->GetStringUTFChars(env, key_obj, 0);
         if (key == NULL) {
            answer_list_add(alpp, "get_map_list: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            ret = JGDI_ILLEGAL_STATE;
            break;
         }
         
         switch(key_field_type) {
            case lHostT:
               lSetPosHost(elem, key_field_pos, key);
               break;
            case lStringT:
               lSetPosString(elem, key_field_pos, key);
               break;
            default:
               answer_list_add(alpp, "type key field must be string or host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               ret = JGDI_ILLEGAL_STATE;
         }
         (*env)->ReleaseStringUTFChars(env, key_obj, key);
         
         if (ret != JGDI_SUCCESS) {
            break;
         }
      }
   }
   
   if (ret != JGDI_SUCCESS) {
     lFreeList(&tmp_list);
     DRETURN(ret);
   } else {
     *list = tmp_list;
     DRETURN(JGDI_SUCCESS);
   }
}


static jgdi_result_t create_object_from_elem(JNIEnv *env, lListElem *ep, jobject *value_obj, int cullType, int pos, lList **alpp) {
   
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(JGDI_LAYER, "create_object_from_elem");
   
   switch(cullType) {
         case lBoolT:
            {
               lBool value = lGetPosBool(ep, pos);
               if ((ret=Boolean_init(env, value_obj, (jboolean)value, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
               break;
            }
         case lUlongT:
            {
               u_long32 value = lGetPosUlong(ep, pos);
               
               if ((ret = Integer_init(env, value_obj, (jint)value, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
               break;
            }
         case lLongT:
            {
               lLong value = lGetPosLong(ep, pos);
               if ((ret = Long_init_0(env, value_obj, (jlong)value, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
               break;
            }
         case lDoubleT:
            {
               lDouble value = lGetPosDouble(ep, pos);
               
               if ((ret = Double_init(env, value_obj, (jdouble)value, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
               break;
            }
         case lFloatT:
            {
               lDouble value = lGetPosDouble(ep, pos);
               if ((ret = Float_init(env, value_obj, (jfloat)value, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
               break;
            }
         case lStringT:
            {
               const char *value = lGetPosString(ep, pos);
               if (value == NULL) {
                  *value_obj = (*env)->NewStringUTF(env, "NONE");
               } else {
                  *value_obj = (*env)->NewStringUTF(env, value);
               }   
               if (test_jni_error(env, "create_object_from_elem: can not create instanceof java.lang.String", alpp)) {
                  DRETURN(JGDI_ERROR);
               }
               break;
            }
         case lHostT:
            {
               const char *value = lGetPosHost(ep, pos);
               if (value == NULL) {
                  *value_obj = (*env)->NewStringUTF(env, "NONE");
               } else {
                  *value_obj = (*env)->NewStringUTF(env, value);
               }   
               if (test_jni_error(env, "create_object_from_elem: can not create instanceof java.lang.String", alpp)) {
                  DRETURN(JGDI_ERROR);
               }
               break;
            }
         case lObjectT:
            {
               const lDescr *descr = lGetElemDescr(ep);
               object_mapping_t *mapping = get_object_mapping(descr);
               
               if (mapping != NULL) {
                  if ((ret=mapping->elem_to_object(mapping, env, ep, value_obj, alpp)) != JGDI_SUCCESS) {
                     DRETURN(ret);
                  }
               } else {
                  answer_list_add(alpp, "No mapping for cull element found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
                  DRETURN(JGDI_ERROR);
               }
            }
            break;
         case lListT:
            {
               /*  value_obj = obj_to_listelem(env, value_obj, descr, alpp); */
               answer_list_add(alpp, "create_object_from_elem: lList type not yet implemented", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               DRETURN(JGDI_ERROR);
            }
         case lEndT:
            /* Attribute not set in cull object, skip it */
            /* TODO set flag in java object */
            break;
         default:
            /* error handling */
            answer_list_add(alpp, "create_object_from_elem: unknown type", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
   }
   DRETURN(JGDI_SUCCESS);
}

static jgdi_result_t set_value_in_elem(JNIEnv *env, jobject value_obj, lListElem *elem, int cullType, int pos, lList** alpp) {
   
   jgdi_result_t ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "set_value_in_elem");
   switch (cullType) {
      case lBoolT:
         {
            jboolean value = false;
            if ((ret=Boolean_booleanValue(env, value_obj, &value, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            } else {
               lSetPosBool(elem, pos, (bool)value);
            }   
            break;
         }
      case lUlongT:
         {
            jint value = 0;
            if ((ret=Number_intValue(env, value_obj, &value, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            } else {
               lSetPosUlong(elem, pos, (u_long32)value);
            }   
            break;
         }
      case lLongT:
         {
            jlong value = 0;
            if ((ret=Number_longValue(env, value_obj, &value, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            } else {
               lSetPosLong(elem, pos, (long)value);
            }   
            break;
         }
      case lStringT:
         {
            char *str = NULL;
            if (value_obj == NULL) {
               lSetPosString(elem, pos, NULL);
            } else {
               str = (char*) (*env)->GetStringUTFChars(env, (jstring)value_obj, 0);
               if (str == NULL) {
                  answer_list_add(alpp, "set_object_in_elem: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                  DRETURN(JGDI_ERROR);
               }
               lSetPosString(elem, pos, str);
               (*env)->ReleaseStringUTFChars(env, (jstring)value_obj, str);
            }
            break;
         }
      case lHostT:
         {
            char *str = NULL;
            if (value_obj == NULL) {
               lSetPosHost(elem, pos, NULL);
            } else {
               str = (char*) (*env)->GetStringUTFChars(env, (jstring)value_obj, 0);
               if (str == NULL) {
                  answer_list_add(alpp, "set_object_in_elem: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                  DRETURN(JGDI_ERROR);
               }
               lSetPosHost(elem, pos, str);
               (*env)->ReleaseStringUTFChars(env, (jstring)value_obj, str);
            }
            break;
         }
      case lDoubleT:
         {
            jdouble value = 0;
            if ((ret=Number_doubleValue(env, value_obj, &value, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            } else {
               lSetPosDouble(elem, pos, (double)value);
            }   
            break;
         }
      case lFloatT:
         {
            jfloat value = 0;
            if ((ret=Number_floatValue(env, value_obj, &value, alpp)) != JGDI_SUCCESS) {
               DRETURN(ret);
            } else {
               lSetPosDouble(elem, pos, (float)value);
            }   
            break;
         }
      case lObjectT:
         {
            const lDescr *descr = lGetElemDescr(elem);
            object_mapping_t *mapping = get_object_mapping(descr);
            
            if (mapping != NULL) {
               if ((ret=mapping->object_to_elem(mapping, env, value_obj, elem, alpp)) != JGDI_SUCCESS) {
                  DRETURN(ret);
               }
            } else {
               answer_list_add(alpp, "No mapping for cull element found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               DRETURN(JGDI_ERROR);
            }
            break;
         }
      case lListT:
         {
            /* value_obj = obj_to_listelem(env, value_obj, descr, alpp); */
            answer_list_add(alpp, "get_map: lList type yet not implemented", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
         }
      case lEndT:
         /* Attribute not set in cull object, skip it */
         /* TODO set flag in java object */
         break;
      default:
        /* error handling */
        answer_list_add(alpp, "get_map: unknown type", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
        DRETURN(JGDI_ERROR);
   }
   DRETURN(JGDI_SUCCESS);
}

static jgdi_result_t set_map(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList *lp, lList **alpp)
{

   jgdi_result_t ret = JGDI_SUCCESS;

   DENTER(BASIS_LAYER, "set_map");

   if (lp != NULL && lGetNumberOfElem(lp) > 0) {
      const lDescr*   descr;
      lListElem* ep = NULL;
      jint key_field_name;
      int key_field_pos;
      int key_field_type;
      jint value_field_name;
      int value_field_pos;
      int value_field_type=0;
      /* for primitve wrappers */
      jint content_field_name = 0;
      int  content_field_pos = lEndT;
      int  content_field_type = 0;
      jboolean has_cull_wrapper = false;

      if ((ret=MapPropertyDescriptor_getKeyCullFieldName(env, property_descr, &key_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      
      if ((ret=MapPropertyDescriptor_getValueCullFieldName(env, property_descr, &value_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }

      descr = lGetListDescr(lp);
      
      key_field_pos = lGetPosInDescr(descr, key_field_name);
      if (key_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "key field %s not found in descriptor", lNm2Str(key_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      
      key_field_type = lGetPosType(descr,key_field_pos);
      
      value_field_pos = lGetPosInDescr(descr, value_field_name);
      if (value_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "value field %s not found in descriptor", lNm2Str(value_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      
      value_field_type = lGetPosType(descr, value_field_pos);
      
      if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }

      if (has_cull_wrapper == true) {
         if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
      }
      
      for_each(ep, lp) {
         jobject value_obj = NULL;
         const char *key = NULL;
         jstring key_obj = NULL;
         
         switch(key_field_type) {
            case lStringT:
               key = lGetPosString(ep, key_field_pos);
               break;
            case lHostT:
               key = lGetPosHost(ep, key_field_pos);
               break;
            default:
               answer_list_add(alpp, "key of a map must be of type String or Host", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
               ret = JGDI_ILLEGAL_STATE;
         }
         
         if (ret != JGDI_SUCCESS) {
            break;
         }
         
         key_obj = (*env)->NewStringUTF(env, key);
         
         if (has_cull_wrapper == true) {
            lListElem *wrapper = lGetPosObject(ep, value_field_pos);
            
            if (content_field_pos == lEndT) {
               content_field_pos = lGetPosInDescr(wrapper->descr, content_field_name);
               if (content_field_pos < 0) {
                  answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                          "content field %s not found in descriptor", lNm2Str(content_field_name));
                  DEXIT;
                  DRETURN(JGDI_ILLEGAL_STATE);
               }
               content_field_type = lGetPosType(wrapper->descr, content_field_pos);
            }
            ret = create_object_from_elem(env, wrapper, &value_obj, content_field_type, content_field_pos, alpp);
         } else {
            ret = create_object_from_elem(env, ep, &value_obj, value_field_type, value_field_pos, alpp);
         }

         if (ret != JGDI_SUCCESS) {
            break;
         }
         
         if ((ret=MapPropertyDescriptor_put(env, property_descr, bean, key_obj, value_obj, alpp)) != JGDI_SUCCESS) {
            break;
         }
      }
   }

   DRETURN(ret);
}

jgdi_result_t get_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList**list, lList **alpp) {

   lDescr*    descr;
   jint        count;
   int        i;
   jobject    obj;
   lList      *tmp_list = NULL;
   lListElem  *ep = NULL;
   jboolean  has_cull_wrapper = false;
   jint      content_field_name = 0;
   int       content_field_type = lEndT;
   int       content_field_pos = lEndT;
   jgdi_result_t       ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "get_list");

   if ((ret=get_descriptor_for_property(env, property_descr, &descr, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if ((ret=ListPropertyDescriptor_getCount(env, property_descr, bean, &count, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if (count == 0) {
      /* new have an empty list */
      *list = NULL;
      DRETURN(JGDI_SUCCESS);
   }

   if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if (has_cull_wrapper) {
      if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      content_field_pos = lGetPosInDescr(descr, content_field_name);
      if (content_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "content field %s not found in descriptor", lNm2Str(content_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      content_field_type =lGetPosType(descr, content_field_pos);
      if (content_field_type == lEndT) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "type of content field of attr %s not found", lNm2Str(content_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
   }
   
   tmp_list = lCreateList("", descr);
   if (!tmp_list) {
      answer_list_add(alpp, "lCreateList failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ERROR);
   }
   

   for(i = 0; i < count; i++) {
      if ((ret=ListPropertyDescriptor_get(env, property_descr, bean, i, &obj, alpp)) != JGDI_SUCCESS) {
         break;
      }
      if (has_cull_wrapper) {
         ep = lCreateElem(descr);
         if ((ret=set_value_in_elem(env, obj, ep, content_field_type, content_field_pos, alpp)) != JGDI_SUCCESS) {
            lFreeElem(&ep);
            break;
         }
      } else if ((ret=obj_to_listelem(env, obj, &ep, descr, alpp)) != JGDI_SUCCESS) {
         break;
      }
      lAppendElem(tmp_list, ep);
   }
   
   if (ret != JGDI_SUCCESS) {
      lFreeList(&tmp_list);
      DRETURN(ret);
   } else {
      *list = tmp_list;
      DRETURN(JGDI_SUCCESS);
   }
}

jgdi_result_t set_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList *lp, lList **alpp) {

   jclass    property_class;
   jgdi_result_t  ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "set_list");

   if ((ret=PropertyDescriptor_getPropertyType(env, property_descr, &property_class, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   
   if (lGetNumberOfElem(lp) > 0) {
      const lDescr* descr;
      lListElem* ep = NULL;
      /* for primitive wrappers */
      jint content_field_name = 0;
      int  content_field_pos = lEndT;
      int  content_field_type = 0;
      jboolean has_cull_wrapper = false;

      descr = lGetListDescr(lp);

      if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if (has_cull_wrapper == true) {
         if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
      }
  
      for_each(ep,lp) {
         jobject obj = NULL;

         if (has_cull_wrapper == true) {
            if (content_field_pos == lEndT) {
               content_field_pos = lGetPosInDescr(descr, (jint)content_field_name);
               if (content_field_pos < 0) {
                  answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR, 
                                          "content field %s not found in descriptor", lNm2Str(content_field_name));
                  DRETURN(JGDI_ILLEGAL_STATE);
               }
               content_field_type = lGetPosType(descr, content_field_pos);
            }
            ret = create_object_from_elem(env, ep, &obj, content_field_type, content_field_pos, alpp);
         } else {
            ret = listelem_to_obj(env, ep, &obj, descr, property_class, alpp);
         }
         
         if (ret != JGDI_SUCCESS) {
            DRETURN(ret);
         }
         if ((ret=ListPropertyDescriptor_add(env, property_descr, bean, obj, alpp)) != JGDI_SUCCESS) {
            DRETURN(ret);
         }
      }   
   } else {
      jgdi_log_printf(env, JGDI_LOGGER, FINER, "set_list: is empty list");
   }

   DRETURN(ret);
}

static jgdi_result_t get_object(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lObject *cob, lList **alpp) {

   lDescr*    descr;
   jobject    obj;
   lListElem* ep;
   jboolean  has_cull_wrapper = false;
   jint      content_field_name = 0;
   int       content_field_type = lEndT;
   int       content_field_pos = lEndT;
   jgdi_result_t       ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "get_object");

   if ((ret=get_descriptor_for_property(env, property_descr, &descr, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   ret = SimplePropertyDescriptor_getValue(env, property_descr, bean, &obj, alpp);

   if (ret != JGDI_SUCCESS) {
      DRETURN(ret);
   }

   if (obj == NULL) {
      /* new have an empty object */
      *cob = NULL;
      DRETURN(JGDI_SUCCESS);
   }

   
   if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   if (has_cull_wrapper) {
      if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      content_field_pos = lGetPosInDescr(descr, content_field_name);
      if (content_field_pos < 0) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "content field %s not found in descriptor", lNm2Str(content_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      content_field_type = lGetPosType(descr, content_field_pos);
      if (content_field_type == lEndT) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "type of content field of attr %s not found", lNm2Str(content_field_name));
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      ep = lCreateElem(descr);
      if (!ep) {
         answer_list_add(alpp, "lCreateElem failed", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      ret=set_value_in_elem(env, obj, ep, content_field_type, content_field_pos, alpp);
   } else {
      ret=obj_to_listelem(env, obj, &ep, descr, alpp);
   }

   if (ret != JGDI_SUCCESS) {
      lFreeElem(&ep);
      DRETURN(ret);
   } else {
      *cob = ep;
      DRETURN(JGDI_SUCCESS);
   }
}

static jgdi_result_t set_object(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lObject cob, lList **alpp) {

   jobject   obj;
   const lDescr*   descr;
   jclass    property_class;

   /* for primitive wrappers */
   jint content_field_name = 0;
   int  content_field_pos = lEndT;
   int  content_field_type = 0;
   jboolean has_cull_wrapper = false;
   jgdi_result_t  ret = JGDI_SUCCESS;
   
   DENTER(BASIS_LAYER, "set_object");

   if ((ret=PropertyDescriptor_getPropertyType(env, property_descr, &property_class, alpp)) != JGDI_SUCCESS) {
      DEXIT;
      DRETURN(ret);
   }
   
   descr = lGetElemDescr(cob);
   
   if ((ret=PropertyDescriptor_hasCullWrapper(env, property_descr, &has_cull_wrapper, alpp)) != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   if (has_cull_wrapper == true) {
      jgdi_log_printf(env, JGDI_LOGGER, FINER, "Property has a cull wrapper");
      if (ret != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      
      if ((ret=PropertyDescriptor_getCullContentField(env, property_descr, &content_field_name, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if (content_field_pos == lEndT) {
         content_field_pos = lGetPosInDescr(descr, (jint)content_field_name);
         if (content_field_pos < 0) {
            answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                    "content field %s not found in descriptor", lNm2Str(content_field_name));
            DRETURN(JGDI_ILLEGAL_STATE);
         }
         content_field_type = lGetPosType(descr, content_field_pos);
      }
      ret = create_object_from_elem(env, cob, &obj, content_field_type, content_field_pos, alpp);
   } else {
      ret = listelem_to_obj(env, cob, &obj, descr, property_class, alpp);
   }
   
   jgdi_log_printf(env, JGDI_LOGGER, FINER, "add converter property to list");
   
   ret=SimplePropertyDescriptor_setValue(env, property_descr, bean, obj, alpp);

   DRETURN(ret);
}

jgdi_result_t get_double(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, double *retdou, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jdouble jd = 0;

   DENTER(BASIS_LAYER, "get_double");
   
   sprintf(buf, "get%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()D", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   jd = (*env)->CallDoubleMethod(env, obj, mid);
   if (test_jni_error(env, "get_double: CallDoubleMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   *retdou = jd;

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_double(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, double value, lList **alpp) {
   char buf[1024];
   jmethodID mid;
   
   DENTER(BASIS_LAYER, "set_double");
   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(D)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, (jdouble)value);
   if (test_jni_error(env, "set_double: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t get_float(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, float *retfl, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jfloat jf = 0;

   DENTER(BASIS_LAYER, "get_float");
   
   sprintf(buf, "get%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()F", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }


   jf = (*env)->CallFloatMethod(env, obj, mid);
   if (test_jni_error(env, "get_float: CallFloatMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   *retfl = jf;

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_float(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, float value, lList **alpp) {
   char buf[1024];
   jmethodID mid;
   
   DENTER(BASIS_LAYER, "set_float");
   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(F)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, (jfloat)value);
   if (test_jni_error(env, "set_double: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t get_string(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, char **retstr, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jstring jstr;
   const char *name = NULL;

   DENTER(BASIS_LAYER, "get_string");
   
   sprintf(buf, "get%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()Ljava/lang/String;", alpp);
   if (!mid) {
      *retstr = NULL;
      DRETURN(JGDI_ERROR);
   }

   jstr = (jstring) (*env)->CallObjectMethod(env, obj, mid);
   if (test_jni_error(env, "get_string: CallObjectMethod failed", alpp)) {
      *retstr = NULL;
      DRETURN(JGDI_ERROR);
   }

   if (jstr == NULL) {
      *retstr = NULL;
   } else {
      name = (*env)->GetStringUTFChars(env, jstr, 0);
      if (name == NULL) {
         answer_list_add(alpp, "get_string: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      *retstr = strdup(name);
      (*env)->ReleaseStringUTFChars(env, jstr, name);
   }

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_string(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, const char* value, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jstring str;

   DENTER(BASIS_LAYER, "set_string");
   
   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(Ljava/lang/String;)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   str = (*env)->NewStringUTF(env, value);
   if (test_jni_error(env, "set_string: NewStringUTF failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, str);
   if (test_jni_error(env, "set_string: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }
   
   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t get_int(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, u_long32 *reti, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jint     ji = 0;

   DENTER(BASIS_LAYER, "get_int");
   
   sprintf(buf, "get%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()I", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }


   ji = (*env)->CallIntMethod(env, obj, mid);
   if (test_jni_error(env, "get_int: CallIntMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   *reti = ji;

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_int(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, u_long32 value, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   
   DENTER(BASIS_LAYER, "set_int");
   /* jint overflow */
   if (value > LONG32_MAX) {
      DPRINTF(("set_int: ulong32 to jint overflow (returning -1)\n"));
      value = -1;
   }
   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(I)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, (jint)value);
   if (test_jni_error(env, "set_int: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t get_long(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, lLong *ret, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jlong     jl = 0;

   DENTER(BASIS_LAYER, "get_long");
   
   sprintf(buf, "get%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()J", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   jl = (*env)->CallLongMethod(env, obj, mid);
   if (test_jni_error(env, "get_int: CallLongMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   *ret = (lLong)jl;

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_long(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, lLong value, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   
   DENTER(BASIS_LAYER, "set_long");

   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(J)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, (jlong)value);
   if (test_jni_error(env, "set_long: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   DRETURN(JGDI_SUCCESS);
}


jgdi_result_t get_bool(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, lBool *retb, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   jboolean  jb = 0;

   DENTER(BASIS_LAYER, "get_bool");
   
   sprintf(buf, "is%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "()Z", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }


   jb = (*env)->CallBooleanMethod(env, obj, mid);
   if (test_jni_error(env, "get_bool: CallIntMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   *retb = (lBool)jb;
   jgdi_log_printf(env, JGDI_LOGGER, FINER, "property %s = %c", property_name, *retb);

   DRETURN(JGDI_SUCCESS);
}

jgdi_result_t set_bool(JNIEnv *env, jclass bean_class, jobject obj, const char* property_name, lBool value, lList **alpp) {
   char buf[1024];
   jmethodID mid = NULL;
   
   DENTER(BASIS_LAYER, "set_bool");

   sprintf(buf, "set%c%s", toupper(property_name[0]), &property_name[1]);
   mid = get_methodid(env, bean_class, buf, "(Z)V", alpp);
   if (!mid) {
      DRETURN(JGDI_ERROR);
   }

   (*env)->CallVoidMethod(env, obj, mid, (jboolean)value);
   if (test_jni_error(env, "set_bool: CallVoidMethod failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }

   jgdi_log_printf(env, JGDI_LOGGER, FINER, "property %s = %c", property_name, value);
   
   DRETURN(JGDI_SUCCESS);
}


jstring get_class_name(JNIEnv *env, jclass cls, lList **alpp) {

   jclass cls_cls;
   jmethodID mid;
   jstring ret = NULL;
   
   DENTER(BASIS_LAYER, "get_class_name");
   
   cls_cls = (*env)->FindClass(env, "java/lang/Class");
   if ((*env)->ExceptionOccurred(env)) {
      answer_list_add(alpp, "class java/lang/Class not found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }

   mid = (*env)->GetMethodID(env, cls_cls, "getName", "()Ljava/lang/String;");
   if (mid == NULL) {
      DRETURN(NULL);
   }
   
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
      answer_list_add(alpp, "exception occured in GetMethodID", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }

   ret = (*env)->CallObjectMethod(env, cls, mid);
#if 0   
   if ((*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionClear(env);
      answer_list_add(alpp, "exception occured in CallObjectMethod", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }
#endif

   DRETURN(ret);
}

jmethodID get_static_methodid(JNIEnv *env, jclass cls, const char* methodName,
                               const char* signature, lList **alpp) {
   jmethodID mid = NULL;

   DENTER(BASIS_LAYER, "get_static_methodid");

   mid = (*env)->GetStaticMethodID(env, cls, methodName, signature);
   
   /* error occured */
   if (mid == NULL) {
      jstring class_name_str;
      const char* class_name = NULL;

      test_jni_error(env, "GetMethodID failed", alpp);
      clear_error(env);
      
      class_name_str = get_class_name(env, cls, alpp);
      
      if (class_name_str != NULL) {
         class_name = (*env)->GetStringUTFChars(env, class_name_str, 0);
      }
      
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "static method %s(%s) not found in class %s", 
                              signature, class_name ? class_name : "NA",
                              class_name ? class_name : "unknown");
                              
      if (class_name) {
         (*env)->ReleaseStringUTFChars(env, class_name_str, class_name);
      }
   }

   DRETURN(mid);
}

jfieldID get_static_fieldid(JNIEnv *env, jclass cls, const char* fieldName,
                               const char* signature, lList **alpp) {
   jfieldID mid = NULL;

   DENTER(BASIS_LAYER, "get_static_fieldid");

   mid = (*env)->GetStaticFieldID(env, cls, fieldName, signature);
   
   /* error occured */
   if (mid == NULL) {
      jstring class_name_str;
      const char* class_name = NULL;

      test_jni_error(env, "GetStaticFieldID failed", alpp);
      clear_error(env);
      
      class_name_str = get_class_name(env, cls, alpp);
      
      if (class_name_str != NULL) {
         class_name = (*env)->GetStringUTFChars(env, class_name_str, 0);
      }
      
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "static field %s(%s) not found in class %s", 
                              signature, class_name ? class_name : "NA",
                              class_name ? class_name : "unknown");
               
      if (class_name) {
         (*env)->ReleaseStringUTFChars(env, class_name_str, class_name);
      }
   }

   DRETURN(mid);
}


jmethodID get_methodid(JNIEnv *env, jclass cls, const char* methodName,
                               const char* signature, lList **alpp) {
   jmethodID mid = NULL;

   DENTER(BASIS_LAYER, "get_methodid");

   if (env == NULL) {
      DPRINTF(("env must not be null\n"));
      abort();
      DRETURN(NULL);
   }
   
   if (cls == NULL) {
      DPRINTF(("cls must not be null\n"));
      abort();
      DRETURN(NULL);
   }
   
   if (methodName == NULL) {
      answer_list_add(alpp, "methodName must not be null",
                              STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }
   
   if (signature == NULL) {
      answer_list_add(alpp, "signature must not be null",
                              STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(NULL);
   }

   mid = (*env)->GetMethodID(env, cls, methodName, signature);
   
   /* error occured */
   if (mid == NULL) {
      jstring class_name_str;
      const char* class_name = NULL;

      test_jni_error(env, "GetMethodID failed", alpp);
      clear_error(env);
      
      class_name_str = get_class_name(env, cls, alpp);
      
      if (class_name_str != NULL) {
         class_name = (*env)->GetStringUTFChars(env, class_name_str, 0);
      }
      
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "method %s(%s) not found in class %s", methodName,
                              signature, class_name ? class_name : "NA");
      
      if (class_name) {
         (*env)->ReleaseStringUTFChars(env, class_name_str, class_name);
      }
   }

   DRETURN(mid);
}

static const char* JGDI_EXCEPTION = "com/sun/grid/jgdi/JGDIException";
static const char* ILLEGAL_STATE_EXCEPTION = "java/lang/IllegalStateException";
static const char* ILLEGAL_ARGUMENT_EXCEPTION = "java/lang/IllegalArgumentException";
static const char* NULL_POINTER_EXCEPTION = "java/lang/NullPointerException";

void throw_error(JNIEnv *env, jgdi_result_t result, const char* message, ...) {
   jclass new_exc_cls;
   va_list ap;
   char buf[BUFSIZ];
   const char* exc_name = NULL;
   jthrowable exc = NULL;

   
   DENTER(BASIS_LAYER, "throw_error");
   
   exc = (*env)->ExceptionOccurred(env);
   
   if (exc) {
      dstring ds = DSTRING_INIT;
      (*env)->ExceptionClear(env);
      exc = (*env)->NewGlobalRef(env, exc);
      printf("Warning: can not throw a new exception: previous exception not cleared\n");
      print_exception(env, exc, &ds);
      printf("%s\n", sge_dstring_get_string(&ds));
      sge_dstring_free(&ds);
      (*env)->DeleteGlobalRef(env, exc);
   }

   va_start(ap, message);
   vsnprintf(buf, BUFSIZ-1, message, ap);
   va_end(ap);
   clear_error(env);     
   switch(result) {
      case JGDI_ERROR:
         exc_name = JGDI_EXCEPTION;
         break;
      case JGDI_ILLEGAL_STATE:
         exc_name = ILLEGAL_STATE_EXCEPTION;
         break;
      case JGDI_ILLEGAL_ARGUMENT:
         exc_name = ILLEGAL_ARGUMENT_EXCEPTION;
         break;
      case JGDI_NULL_POINTER:
         exc_name = NULL_POINTER_EXCEPTION;
         break;
      default:
         abort();
   }
   new_exc_cls = (*env)->FindClass(env, exc_name);
   
   {
      jthrowable exc = (*env)->ExceptionOccurred(env);
      if (exc) {
         dstring ds = DSTRING_INIT;
         (*env)->ExceptionClear(env);
         exc = (*env)->NewGlobalRef(env, exc);
         printf("Fatal Error: exception %s not found\n", exc_name);
         print_exception(env, exc, &ds);
         printf("%s\n", sge_dstring_get_string(&ds));
         sge_dstring_free(&ds);
         abort();
      }
   }
   (*env)->ThrowNew(env, new_exc_cls, buf);
   DRETURN_VOID;
}

void throw_error_from_answer_list(JNIEnv *env, jgdi_result_t result, lList* alp) {
   
   dstring ds = DSTRING_INIT;
   
   DENTER(BASIS_LAYER, "throw_error_from_answer_list");
   
   answer_list_to_dstring(alp, &ds);
   
   throw_error(env, result, sge_dstring_get_string(&ds));
   
   sge_dstring_free(&ds);

   DRETURN_VOID;
}


void throw_error_from_handler(JNIEnv *env, sge_error_class_t *eh) {
   sge_error_iterator_class_t *iter = NULL;
   dstring ds = DSTRING_INIT;
   bool first = true;

   DENTER(BASIS_LAYER, "throw_error_from_handler");
   
   iter = eh->iterator(eh);

   while (iter && iter->next(iter)) {
      if (first) {
         first = false;
      } else {   
         sge_dstring_append(&ds, "\n");
      }
      sge_dstring_append(&ds, iter->get_message(iter));
   }
   throw_error(env, JGDI_ERROR, sge_dstring_get_string(&ds));
   
   sge_dstring_free(&ds);
   DRETURN_VOID;
}


void clear_error(JNIEnv* env) {
   jthrowable exc;

   DENTER(BASIS_LAYER, "clear_error");
   exc = (*env)->ExceptionOccurred(env);
   if (exc) {
      (*env)->ExceptionClear(env);
   }
   DRETURN_VOID;
}

jboolean test_jni_error(JNIEnv* env, const char* message, lList **alpp) {
   jthrowable exc;
   
   DENTER(BASIS_LAYER, "test_jni_error");
   
   exc = (*env)->ExceptionOccurred(env);
   
   if (exc) {
      DPRINTF(("An exception occured\n"));
#if 1
      if (alpp != NULL) {
         dstring buf = DSTRING_INIT;
         jobject newExc = NULL;
         (*env)->ExceptionClear(env);
         
         newExc = (*env)->NewGlobalRef(env, exc);
         if ((*env)->ExceptionOccurred(env)) {
            DPRINTF(("NewGlobalRef did not work\n"));
            abort();
         }
         
         exception_to_string(env, newExc, &buf);
         DPRINTF(("Exception text: %s\n", sge_dstring_get_string(&buf)));
         sge_dstring_clear(&buf);
         
         sge_dstring_append(&buf, message);
         sge_dstring_append(&buf, "\n");
         print_exception(env, newExc, &buf);
         sge_dstring_append(&buf, "\n");
         answer_list_add(alpp, sge_dstring_get_string(&buf), STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         sge_dstring_free(&buf);
         
         (*env)->DeleteGlobalRef(env, newExc);
      }
#endif
      DRETURN(TRUE);
   } else {
      DRETURN(FALSE);
   }
}

static void exception_to_string(JNIEnv* env, jobject exc, dstring* buf) {
   jclass cls = (*env)->FindClass(env, "java/lang/Throwable");
   jmethodID to_string_mid;
   jstring msg_obj;
   const char* msg;

   DENTER(BASIS_LAYER, "exception_to_string");
   
   to_string_mid = (*env)->GetMethodID(env, cls, "toString", "()Ljava/lang/String;");
   if (to_string_mid == NULL) {
      sge_dstring_append(buf, "ERROR: method toString not found in java.lang.Throwable");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   msg_obj = (jstring)(*env)->CallObjectMethod(env, exc, to_string_mid);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "ERROR: method java.lang.Throwable.toString failed");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   
   msg = (*env)->GetStringUTFChars(env, msg_obj, 0);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "ERROR: method GetStringUTFChars failed");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   if (msg == NULL || strlen(msg) == 0) {
      sge_dstring_append(buf, "null");
   } else {
      sge_dstring_append(buf, msg);
   }
   
   (*env)->ReleaseStringUTFChars(env, msg_obj, msg);
   
   DRETURN_VOID;
}

static void print_exception(JNIEnv* env, jobject exc, dstring* buf) {
   
   jclass cls = (*env)->FindClass(env, "java/lang/Throwable");
   jmethodID to_string_mid;
   jmethodID get_cause_mid;
   jstring msg_obj;
   const char* msg;
   
   DENTER(BASIS_LAYER, "print_exception");
   
   if (exc == NULL) {
      sge_dstring_append(buf, "ERROR: exc is null");
      DRETURN_VOID;
   }
   
   if (cls==NULL) {
      sge_dstring_append(buf, "ERROR: class java.lang.Throwable not found");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   
   to_string_mid = (*env)->GetMethodID(env, cls, "toString", "()Ljava/lang/String;");
   if (to_string_mid == NULL) {
      sge_dstring_append(buf, "ERROR: method toString not found in java.lang.Throwable");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   
   msg_obj = (jstring)(*env)->CallObjectMethod(env, exc, to_string_mid);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "ERROR: method java.lang.Throwable.toString failed");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   
   msg = (*env)->GetStringUTFChars(env, msg_obj, 0);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "ERROR: method GetStringUTFChars failed");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   if (msg == NULL || strlen(msg) == 0) {
      sge_dstring_append(buf, "null");
   } else {
      sge_dstring_append(buf, msg);
   }
   
   (*env)->ReleaseStringUTFChars(env, msg_obj, msg);
   
   print_stacktrace(env, exc, buf);
   
   /* Looking for cause execeptions */
   get_cause_mid = (*env)->GetMethodID(env, cls, "getCause", "()Ljava/lang/Throwable;");
   if (get_cause_mid == NULL) {
      sge_dstring_append(buf, "ERROR: method getCause not found in java.lang.Throwable");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   
   exc = (*env)->CallObjectMethod(env, exc, get_cause_mid);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "ERROR: method java.lang.Throwable.getCause failed");
      (*env)->ExceptionClear(env);
      DRETURN_VOID;
   }
   if (exc != NULL) {
      sge_dstring_append(buf, "\n   caused by ");
      print_exception(env, exc, buf);
   }
   DRETURN_VOID;
}


static void print_stacktrace(JNIEnv* env, jobject exc, dstring* buf) {
   
   jclass throwable_class;
   jclass stacktrace_element_class;
   jmethodID get_stacktrace_mid;
   jmethodID to_string_mid;
   jobjectArray stacktrace;
   jint len;
   int i;
   
   throwable_class = (*env)->GetObjectClass(env, exc);
   if (throwable_class == NULL) {
      sge_dstring_append(buf, "\nERROR: Can't find class java/lang/Throwable");
      (*env)->ExceptionClear(env);
      return;
   }
   
   stacktrace_element_class = (*env)->FindClass(env, "java/lang/StackTraceElement");
   if (stacktrace_element_class == NULL) {
      sge_dstring_append(buf, "\nERROR: Can't find class java.lang.StackTraceElement");
      (*env)->ExceptionClear(env);
      return;
   }
   
   get_stacktrace_mid = (*env)->GetMethodID(env, throwable_class,
                                          "getStackTrace", "()[Ljava/lang/StackTraceElement;");
   if (get_stacktrace_mid == NULL) {
      sge_dstring_append(buf, "\nERROR: Can't find method getStacktrace in class java.lang.StackTraceElement");
      (*env)->ExceptionClear(env);
      return;
   }

   to_string_mid = (*env)->GetMethodID(env, stacktrace_element_class,
                                     "toString", "()Ljava/lang/String;");
   if (to_string_mid == NULL) {
      sge_dstring_append(buf, "\nERROR: Can't find method toString in class java.lang.StackTraceElement");
      (*env)->ExceptionClear(env);
      return;
   }
   
   stacktrace = (*env)->CallObjectMethod(env, exc, get_stacktrace_mid);
   if (stacktrace == NULL) {
      sge_dstring_append(buf, "\nERROR: Call of method getStacktrace in class java.lang.StackTraceElement failed");
      (*env)->ExceptionClear(env);
      return;
   }
   
   len = (*env)->GetArrayLength(env, stacktrace);
   if ((*env)->ExceptionOccurred(env)) {
      sge_dstring_append(buf, "\nERROR: Call of method GetArrayLength on stacktrace failed");
      (*env)->ExceptionClear(env);
      return;
   }
   
   for(i = 0; i < len; i++) {
     jobject stacktrace_elem;
     jstring stacktrace_str_obj;
     const char* stacktrace_str;
     
     stacktrace_elem = (*env)->GetObjectArrayElement(env, stacktrace, i);
     if (stacktrace_elem==NULL) {
        sge_dstring_append(buf, "\nERROR: Call of method GetObjectArrayElement on stacktrace failed");
        (*env)->ExceptionClear(env);
        break;
     }
     
     stacktrace_str_obj = (jstring)(*env)->CallObjectMethod(env, stacktrace_elem, to_string_mid);
     if (stacktrace_str_obj==NULL) {
        sge_dstring_append(buf, "\nERROR: Call of method StackTraceElement.toString failed");
        break;
     }
     
     stacktrace_str = (*env)->GetStringUTFChars(env, stacktrace_str_obj, 0);
     if (stacktrace_str != NULL) {
        sge_dstring_append(buf, "\n     ");
        sge_dstring_append(buf, stacktrace_str);
        (*env)->ReleaseStringUTFChars(env, stacktrace_str_obj, stacktrace_str);
     } else {
        sge_dstring_append(buf, "\nERROR: Call of method GetStringUTFChars failed");
        break;
     }
   }
   (*env)->ExceptionClear(env);
}



void object_to_str(JNIEnv* env, jobject obj, char* buf, size_t max_len) {
   
   if (obj == NULL) {
      snprintf(buf, max_len, "%s", "null");
   } else {
      
      lList *alpp;
      jclass clazz;
      jstring classname_obj;
      const char* classname = NULL;
      jmethodID mid;
      jstring obj_str_obj;
      const char* objStr;
      
      clazz = (*env)->GetObjectClass(env, obj);
      
      classname_obj = get_class_name(env, clazz, &alpp);
      if (classname_obj == NULL) {
         snprintf(buf, max_len, "object_to_string: classname_obj is NULL");
         return;
      }
      mid = get_methodid(env, clazz, "toString", "()Ljava/lang/String;", &alpp);
      
      obj_str_obj = (jstring)(*env)->CallObjectMethod(env, obj, mid);
      if (obj_str_obj == NULL) {
         snprintf(buf, max_len, "NULL");
         return;
      }

      classname = (*env)->GetStringUTFChars(env, classname_obj, 0);
      objStr = (*env)->GetStringUTFChars(env, obj_str_obj, 0);
      if (classname == NULL || objStr == NULL) {
         snprintf(buf, max_len, "object_to_string: GetStringUTFChars failed. Out of memory.");
         return;
      }
      
      snprintf(buf, max_len, "%s (%s)", objStr, classname);
      
      (*env)->ReleaseStringUTFChars(env, classname_obj, classname);
      (*env)->ReleaseStringUTFChars(env, obj_str_obj, objStr);
   }
}

static jgdi_result_t get_descriptor_for_property(JNIEnv *env, jobject property_descr, lDescr **descr, lList **alpp) {

   jstring   cull_type_name_obj;
   const char* cull_type_name;
   jgdi_result_t  ret = JGDI_SUCCESS;

   DENTER(BASIS_LAYER, "get_descriptor_for_property");

   ret = PropertyDescriptor_getCullType(env, property_descr, &cull_type_name_obj, alpp);
   if (ret != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   if (cull_type_name_obj == NULL) {
      answer_list_add(alpp, "get_descriptor_for_property: cull_type_name_obj is NULL.", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   
   cull_type_name = (*env)->GetStringUTFChars(env, cull_type_name_obj, 0);
   if (cull_type_name == NULL) {
      answer_list_add(alpp, "get_descriptor_for_property: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ERROR);
   }   

   *descr = get_descr(cull_type_name);
   if (*descr == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "get_descriptor_for_property: no descr for cull type %s found",  cull_type_name);
      ret = JGDI_ERROR;
   }
   (*env)->ReleaseStringUTFChars(env, cull_type_name_obj, cull_type_name);
   
   DRETURN(ret);
}
   
static jgdi_result_t get_list_descriptor_for_property(JNIEnv *env, jobject property_descr, lDescr **descr, lList **alpp) {

   jstring   cull_type_name_obj;
   const char* cull_type_name;
   jgdi_result_t  ret = JGDI_SUCCESS;

   DENTER(BASIS_LAYER, "get_list_descriptor_for_property");

   ret = MapListPropertyDescriptor_getCullListType(env, property_descr, &cull_type_name_obj, alpp);
   if (ret != JGDI_SUCCESS) {
      DRETURN(ret);
   }
   if (cull_type_name_obj == NULL) {
      answer_list_add(alpp, "get_list_descriptor_for_property: cull_type_name_obj is NULL. ", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }

   cull_type_name = (*env)->GetStringUTFChars(env, cull_type_name_obj, 0);
   if (cull_type_name == NULL) {
      answer_list_add(alpp, "get_list_descriptor_for_property: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ERROR);
   }

   *descr = get_descr(cull_type_name);
   if (descr == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "get_list_descriptor_for_property: no descr for cull type %s found",  cull_type_name);
      ret = JGDI_ERROR;
   }
   (*env)->ReleaseStringUTFChars(env, cull_type_name_obj, cull_type_name);
   
   DRETURN(ret);
}


jgdi_result_t get_string_list(JNIEnv *env, jobject obj, const char* getter, lList **lpp, lDescr* descr, int nm, lList **alpp) {
   jmethodID get_mid;
   jclass    cls;
   jobject   list;

   DENTER(BASIS_LAYER, "get_string_list");
   
   cls = (*env)->GetObjectClass(env, obj);
   if (test_jni_error(env, "get_string_list: class for obj not found", alpp)) {
      DRETURN(JGDI_ERROR);
   }
   
   get_mid = get_methodid(env, cls, getter, "()Ljava/util/List;", alpp);
   if (!get_mid) {
      DRETURN(JGDI_ERROR);
   }

   list = (*env)->CallObjectMethod(env, obj, get_mid);
   if (test_jni_error(env, "get_string_list: call of getter failed", alpp)) {
      DRETURN(JGDI_ERROR);
   }
   
   if (string_list_to_list_elem(env, list, lpp, descr, nm, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   DRETURN(JGDI_SUCCESS);
}

static jgdi_result_t string_list_to_list_elem(JNIEnv *env, jobject list, lList **lpp, lDescr *descr, int nm, lList **alpp) {
   
   jobject  iter = NULL;
   jboolean has_next = 0;
   
   DENTER(BASIS_LAYER, "string_list_to_list_elem");
   
   if (List_iterator(env, list, &iter, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   while (TRUE) {
      if (Iterator_hasNext(env, iter, &has_next, alpp) != JGDI_SUCCESS) {
        DRETURN(JGDI_ERROR);
      } else if (has_next == false) {
         break;
      } else {
         jstring str_obj = NULL;
         if (Iterator_next(env, iter, &str_obj, alpp) != JGDI_SUCCESS) {
            DRETURN(JGDI_ERROR);
         } else {
           const char* str;
           if (str_obj != NULL) {
              str = (*env)->GetStringUTFChars(env, str_obj, 0);
              if (str == NULL) {
                 answer_list_add(alpp, "string_list_to_list_elem: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                 DRETURN(JGDI_ERROR);
              }
           } else {
              str = NULL;
           }
           DPRINTF(("Got %s from list\n", str));
           lAddElemStr(lpp, nm, str, descr);
           if (str) {
               (*env)->ReleaseStringUTFChars(env, str_obj, str);
           }
         }
      }
   }
   
   DRETURN(JGDI_SUCCESS);
}



/*-------------------------------------------------------------------------*
 * NAME
 *   build_filter - build a lCondition from a PrimaryKeyFilter filter object
 * PARAMETER
 *  env    - the JNI Environment
 *  filter - the PrimaryKeyFilter object (instanceof com.sun.grid.jgdi.filter.PrimaryKeyFilter)
 *  where  - the resulting condition (output)
 *  alpp   - answer list for error reporting
 *
 * RETURN
 *
 *  JGDI_SUCCESS  - condition has been built
 *  JGDI_ERROR    - error reason has been reported in alpp
 *
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
jgdi_result_t build_filter(JNIEnv *env, jobject filter, lCondition **where, lList **alpp) {
   
   jclass pk_filter_class = NULL;
   jclass filter_class = NULL;
   jboolean is_pk_filter = false;
   
   DENTER(JGDI_LAYER, "build_filter");
   
   pk_filter_class = PrimaryKeyFilter_find_class(env, alpp);
   
   if (pk_filter_class == NULL) {
      DRETURN(JGDI_ERROR);
   }
   
   if (Object_getClass(env, filter, &filter_class, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   if (Class_isAssignableFrom(env, pk_filter_class, filter_class, &is_pk_filter, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   if (is_pk_filter) {
      
      jobject fields = NULL;
      jobject field = NULL;
      jobject iter = NULL;
      jboolean has_next = false;
      
      jstring type_obj = NULL;  /* string representation of the cull type */
      
      lCondition *result = NULL;
      
      if (PrimaryKeyFilter_getType(env, filter, &type_obj, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      
      if (PrimaryKeyFilter_getFields(env, filter, &fields, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      if (List_iterator(env, fields, &iter, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      
      while (1) {
         lCondition *field_where = NULL;
         
         if (Iterator_hasNext(env, iter, &has_next, alpp) != JGDI_SUCCESS) {
            lFreeWhere(&result);
            DRETURN(JGDI_ERROR);
         }
         
         if (has_next == false) {
            break;
         }
         
         if (Iterator_next(env, iter, &field, alpp) != JGDI_SUCCESS) {
            lFreeWhere(&result);
            DRETURN(JGDI_ERROR);
         }
         
         if (build_field_filter(env, field, &field_where, alpp) != JGDI_SUCCESS) {
            lFreeWhere(&result);
            DRETURN(JGDI_ERROR);
         }
         
         /* Add the condition for the primary key field to the result */
         if (result == NULL) {
            result = field_where;
         } else {
            result = lAndWhere(result, field_where);
         }
      } /* end of while */
      
      *where = result;
      
      
#if 0
      printf("build where from filter -----------------\n");
      lWriteWhereTo(result, stdout);
#endif
      DRETURN(JGDI_SUCCESS);
      
   } else {
      /* filter is not an instanceof of pk filter */
      jstring class_name_obj = NULL;
      const char* class_name = NULL;
      
      if (Class_getName(env, pk_filter_class, &class_name_obj, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      class_name =  (*env)->GetStringUTFChars(env, class_name_obj, 0);
      if (class_name == NULL) {
         answer_list_add(alpp, "build_filter: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "build_filter: filter must be an instanceof %s",
                              class_name);
      (*env)->ReleaseStringUTFChars(env, class_name_obj, class_name);
      DRETURN(JGDI_ERROR);
   }
}

/*-------------------------------------------------------------------------*
 * NAME
 *   build_field_filter - build a condition from a primary key filter
 * PARAMETER
 *  env   -  the jni environment
 *  field -  the primary key filter (instance of com.sun.grid.jgdi.filter.WhereClause)
 *  where -  the condition (output parameter)
 *  alpp -   answer list for error reporting
 *
 * RETURN
 *   JGDI_SUCCESS - if the condition has been successfully built
 *   JGDI_ERROR   - error
 *
 *
 * DESCRIPTION
 *
 *  This method builds a lCondition element from a object of type
 *  com.sun.grid.jgdi.filter.StringWhereClause or
 *  com.sun.grid.jgdi.filter.IntWhereClause
 *
 *  EXAMPLE
 *
 *  Java:
 *  IntWhereClause wc = new IntWhereClause("JB_Type", CullConstants.JB_Name, 1);
 *
 *  Result of build_field_filter:
 *
 *   lCondition where = lWhere("%T(%I==%s)", JB_Type, JB_Name, "MyJob");
 *   lCondition where = lWhere("%T(%I==%u)", JB_Type, JB_job_number, 1);
 *
 *  Java:
 *
 *  StringWhereClause wc = new StringWhereClause("CQ_Type", "CQ_Name", "all.q");
 *
 *  Result of build_field_filter:
 *
 *   lCondition where = lWhere("%T(%I==%s)", CQ_Type, CQ_Name, "all.q");
 *
 *-------------------------------------------------------------------------*/
static jgdi_result_t build_field_filter(JNIEnv *env, jobject field, lCondition **where, lList **alpp) {
   jstring type_obj = NULL;
   lDescr  *descr = NULL;
   jstring pattern_obj = NULL;
   jclass  field_class = NULL;
   jint    field_name = 0;
   jclass  string_where_class = NULL;
   jboolean is_string_field = false;
   
   DENTER(JGDI_LAYER, "build_field_filter");
   
   if (WhereClause_getType(env, field, &type_obj, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   {
      const char* type = NULL;
      if (type_obj != NULL) {
         type = (*env)->GetStringUTFChars(env, type_obj, 0);
         if (type == NULL) {
            answer_list_add(alpp, "build_field_filter: GetStringUTFChars failed. Out of the memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
         }
      } else {
         answer_list_add(alpp, "build_field_filter: type_obj is NULL.", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ILLEGAL_STATE);
      }
      descr = get_descr(type);
      if (descr == NULL) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "build_field_filter: descriptor for %s not found",
                                 type);
      }
      (*env)->ReleaseStringUTFChars(env, type_obj, type);
      if (descr == NULL) {
         DRETURN(JGDI_ERROR);
      }
   }
   if (WhereClause_getPattern(env, field, &pattern_obj, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   if (WhereClause_getField(env, field, &field_name, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }

   if (Object_getClass(env, field, &field_class, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   string_where_class = StringWhereClause_find_class(env, alpp);
   if (string_where_class == NULL) {
      DRETURN(JGDI_ERROR);
   }
   
   if (Class_isAssignableFrom(env, string_where_class, field_class, &is_string_field, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   if (is_string_field) {
      /* we have an string expression */
      jstring value_obj = NULL;
      const char* value = NULL;
      const char* pattern = NULL;
      if (StringWhereClause_getValue(env, field, &value_obj, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      if (value_obj == NULL) {
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "build_field_filter: value of filtered primary key field %d is NULL",
                                 field_name);
         DRETURN(JGDI_ERROR);
      }
      
      value = (*env)->GetStringUTFChars(env, value_obj, 0);
      pattern  = (*env)->GetStringUTFChars(env, pattern_obj, 0);
      if (value == NULL || pattern == NULL) {
         answer_list_add(alpp, "build_field_filter: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
         DRETURN(JGDI_ERROR);
      }
      
      *where = lWhere(pattern, descr, field_name, value);
      
      if (value) {
         (*env)->ReleaseStringUTFChars(env, value_obj, value);
      }
      if (pattern) {
         (*env)->ReleaseStringUTFChars(env, pattern_obj, pattern);  
      }
      
      DRETURN(JGDI_SUCCESS);
      
   } else {
      jboolean is_int_field = false;
      jclass  int_where_class = NULL;

      int_where_class = IntWhereClause_find_class(env, alpp);
      if (int_where_class == NULL) {
         DRETURN(JGDI_ERROR);
      }
      
      if (Class_isAssignableFrom(env, int_where_class, field_class, &is_int_field, alpp) != JGDI_SUCCESS) {
         DRETURN(JGDI_ERROR);
      }
      
      if (is_int_field) {
         /* we have an integer expression */
         jint value = 0;
         const char* pattern = NULL;
         
         if (IntWhereClause_getValue(env, field, &value, alpp) != JGDI_SUCCESS) {
            DRETURN(JGDI_ERROR);
         }
         
         pattern  = (*env)->GetStringUTFChars(env, pattern_obj, 0);
         if (pattern == NULL) {
            answer_list_add(alpp, "build_field_filter: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
         }

         *where = lWhere(pattern, descr, field_name, value);
         
         (*env)->ReleaseStringUTFChars(env, pattern_obj, pattern);
         
         DRETURN(JGDI_SUCCESS);
         
      } else {
         jstring class_name_obj = NULL;
         const char* class_name = NULL;
         
         if (Class_getName(env, field_class, &class_name_obj, alpp) != JGDI_SUCCESS) {
            DRETURN(JGDI_ERROR);
         }                 
         class_name =  (*env)->GetStringUTFChars(env, class_name_obj, 0);
         if (class_name == NULL) {
            answer_list_add(alpp, "build_field_filter: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
            DRETURN(JGDI_ERROR);
         }
         answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                                 "build_field_filter: filter for class %s not implemented",
                                 class_name);
         (*env)->ReleaseStringUTFChars(env, class_name_obj, class_name);
         DRETURN(JGDI_ERROR);
      }
   }
}


jgdi_result_t generic_fill_list(JNIEnv *env, jobject list, const char *classname, lList *lp, lList **alpp) 
{
   const lDescr *listdescr = NULL;
   lListElem *ep = NULL;
   jobject obj;
   jclass obj_class;
   jgdi_result_t ret = JGDI_SUCCESS;
   int count = 0;

   DENTER(TOP_LAYER, "generic_fill_list");

   jgdi_log_printf(env, JGDI_LOGGER, FINE,
                   "BEGIN ------------------ fill %s ---------------------", classname);
  
   jgdi_log_list(env, JGDI_LOGGER, FINE, lp);

   obj_class = (*env)->FindClass(env, classname);
   if (!obj_class) {
      answer_list_add_sprintf(alpp,
                              STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "class %s not found",  classname);
      DRETURN(JGDI_ERROR);
   }
 
   listdescr = lGetListDescr(lp);
   for_each(ep, lp) {
      jboolean add_result = false;
      /* convert to Java representation */
      if ((ret = listelem_to_obj(env, ep, &obj, listdescr, obj_class, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      if ((ret=List_add(env, list, obj, &add_result, alpp)) != JGDI_SUCCESS) {
         DRETURN(ret);
      }
      count++;
   }
   
   jgdi_log_printf(env, JGDI_LOGGER, FINE,
                   "END fill %s, got %d objects ", classname, count);

   DRETURN(ret);
}                   

void jgdi_fill(JNIEnv *env, jobject jgdi, jobject list, jobject filter, const char *classname, int target_list, lDescr *descr, jobject answers) {


   /* receive Cull Object */
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   lEnumeration *what  = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;

   DENTER(TOP_LAYER, "jgdi_fill");
   
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   if (filter != NULL && target_list != SGE_STN_LIST) { 
     ret=build_filter(env, filter, &where, &alp);
     if (ret != JGDI_SUCCESS) {
        goto error;
     }
   }
   
   /* get context */
   ret = getGDIContext(env, jgdi, &ctx, &alp);
   if (ret != JGDI_SUCCESS) {
      goto error;
   }
   
   sge_gdi_set_thread_local_ctx(ctx);

   /* create what and where */
   what = lWhat("%T(ALL)", descr);
   
   /* get list */
   alp = ctx->gdi(ctx, target_list, SGE_GDI_GET, &lp, where, what, false);
   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   } else {
      lFreeList(&alp);
   }   

   if (target_list == SGE_STN_LIST) {
      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
         goto error;
      } else {
         lFreeList(&alp);
      }   
   }
   ret = generic_fill_list(env, list, classname, lp, &alp);
   

error:

   /*
   ** this must be called before throw_error_from_answer_list, otherwise there is a pending 
   ** exception in the way
   */
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
   
   lFreeWhat(&what);
   lFreeWhere(&where);
   lFreeList(&lp);
   lFreeList(&alp);
   DRETURN_VOID;
}

void jgdi_add(JNIEnv *env, jobject jgdi, jobject jobj, const char *classname, int target_list, lDescr *descr, jobject answers)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   lListElem *ep = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER(JGDI_LAYER, "jgdi_add");

   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      goto error;
   }

   sge_gdi_set_thread_local_ctx(ctx);
   
   
   ret = obj_to_listelem(env, jobj, &ep, descr, &alp);

   if (ret == JGDI_SUCCESS) {
      
      lp = lCreateList("add", descr);
      lAppendElem(lp, ep);

      
      jgdi_log_printf(env, JGDI_LOGGER, FINE,
                     "BEGIN --------------- jgdi_add %s -------------------------------", classname); 
      
      jgdi_log_list(env, JGDI_LOGGER, FINE, lp);
      
      jgdi_log_printf(env, JGDI_LOGGER, FINE,
                      "END --------------- jgdi_add %s -------------------------------", classname); 
   
      what = lWhat("%T(ALL)", descr);
      
      /* add to list */
      if (target_list == SGE_JB_LIST || target_list == SGE_AR_LIST) {
        alp = ctx->gdi(ctx, target_list, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, &lp, where, what, false);
         if (answer_list_has_error(&alp)) {
            ret = JGDI_ERROR;
            goto error;
         }
#if 0         
         else {
            lFreeList(&alp);
         }   
#endif

         if ((ep = lFirst(lp)) != NULL) {
            jclass elem_class = NULL;
            jobject obj_descr = NULL;
            jint prop_count;
            int i;
            /* get jobj's class */
            if ((ret = Object_getClass(env, jobj, &elem_class, &alp)) != JGDI_SUCCESS) {
               goto error;
            }
            /* Get the descriptor class of the bean class */
            if ((ret=Util_static_getDescriptor(env, elem_class, &obj_descr, &alp)) != JGDI_SUCCESS) {
               goto error;
            }

            /* get the property count */
            if ((ret=ClassDescriptor_getPropertyCount(env, obj_descr, &prop_count, &alp)) != JGDI_SUCCESS) {
               goto error;
            }
   
            for (i = 0; i < prop_count; i++) {
              jobject prop_descr = NULL;
              if ((ret=ClassDescriptor_getProperty(env, obj_descr, i, &prop_descr, &alp)) != JGDI_SUCCESS) {
                 goto error;
              }
              if ((ret=set_object_attribute(env, ep, descr, jobj, prop_descr, &alp)) != JGDI_SUCCESS) {
                 goto error;
              }
            }
         }   
         lFreeList(&lp);
      } else if (target_list == SGE_CONF_LIST) {
        alp = ctx->gdi(ctx, target_list, SGE_GDI_MOD, &lp, where, what, false);
         lFreeList(&lp);
      } else {   
        alp = ctx->gdi(ctx, target_list, SGE_GDI_ADD | SGE_GDI_SET_ALL, &lp, where, what, false);
         lFreeList(&lp);
      }

      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
         goto error;
      }
      
   }
   
error:

   /*
   ** this must be called before throw_error_from_answer_list, otherwise there is a pending 
   ** exception in the way
   */
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
 
   lFreeList(&alp);
   lFreeWhat(&what);
   DRETURN_VOID;
}

void jgdi_delete_array(JNIEnv *env, jobject jgdi, jobjectArray obj_array, const char *classname, int target_list, lDescr *descr, jboolean force, jobject userFilter, jobject answers)
{
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   lList *alp = NULL;
   lList *ref_list = NULL;
   
   DENTER(TOP_LAYER, "jgdi_delete_array");
  
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);

   if (obj_array != NULL) {
      int i;
      jsize asize = (*env)->GetArrayLength(env, obj_array);
      
      for (i=0; i<asize; i++) {
          jobject obj = (*env)->GetObjectArrayElement(env, obj_array, i);
          if (obj) {
             if (target_list == SGE_JB_LIST || target_list == SGE_AR_LIST) {
                lListElem *iep = NULL;
                const char* name = (*env)->GetStringUTFChars(env, obj, 0);
                if (name == NULL) {
                   answer_list_add(&alp, "jgdi_delete_array: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                   goto error;
                }
                if (target_list == SGE_JB_LIST) {
                   if (sge_parse_jobtasks(&ref_list, &iep, name, &alp, true, NULL) == -1) {
                      answer_list_add_sprintf(&alp, STATUS_ESEMANTIC, ANSWER_QUALITY_ERROR, MSG_JOB_XISINVALIDJOBTASKID_S, name);
                   }
                   lSetUlong(iep, ID_force, force);
                 } else {
                  iep = lAddElemStr(&ref_list, ID_str, name, ID_Type);
                  lSetUlong(iep, ID_force, force);
                }
                if (name) {
                  (*env)->ReleaseStringUTFChars(env, obj, name);
                }
             } else {   
                lListElem *ep = NULL;
                if ((ret = obj_to_listelem(env, obj, &ep, descr, &alp)) != JGDI_SUCCESS) {
                   goto error;
                }
                if (ref_list == NULL) {
                   ref_list = lCreateList("", descr);
                }   
                lAppendElem(ref_list, ep);
             }
          }   
      }
   }      

   /*
   ** handle the userFilter for Job and AdvanceReservation in addition
   */
   if (userFilter != NULL && (target_list == SGE_JB_LIST || target_list == SGE_AR_LIST)) {
      lList *user_list = NULL;
      if (get_string_list(env, userFilter, "getUsers", &user_list, ST_Type, ST_name, &alp) != JGDI_SUCCESS) {
         lFreeList(&user_list);
         goto error;
      }
      if (user_list) {
         lListElem *iep;
         if (lGetNumberOfElem(ref_list) == 0){
            iep = lAddElemStr(&ref_list, ID_str, "0", ID_Type);
            lSetList(iep, ID_user_list, user_list);
            lSetUlong(iep, ID_force, force);
         } else {
            for_each(iep, ref_list){
               lSetList(iep, ID_user_list, user_list);
               lSetUlong(iep, ID_force, force);
            }
         }
      }
   }
   
   jgdi_log_printf(env, JGDI_LOGGER, FINER,
                   "jgdi_delete_array: ref_list BEGIN ----------------------------------------");
   
   jgdi_log_list(env, JGDI_LOGGER, FINER, ref_list);

   jgdi_log_printf(env, JGDI_LOGGER, FINER,
                   "jgdi_delete_array: ref_list END ----------------------------------------");
      
   if (ref_list != NULL) {      
      sge_gdi_ctx_class_t *ctx = NULL;
      /* get context */
      ret = getGDIContext(env, jgdi, &ctx, &alp);
      if (ret != JGDI_SUCCESS) {
         goto error;
      }

      sge_gdi_set_thread_local_ctx(ctx);

      alp = ctx->gdi(ctx, target_list, SGE_GDI_DEL, &ref_list, NULL, NULL, false);
      lFreeList(&ref_list);
      
      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
      }
      
   }

error:
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
   lFreeList(&alp);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);
   
   DRETURN_VOID;
}

void jgdi_delete(JNIEnv *env, jobject jgdi, jobject jobj, const char* classname, int target_list, lDescr *descr, jboolean force, jobject answers)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   lListElem *ep = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   lListElem *iep = NULL;
   char id_buf[BUFSIZ];
   
   DENTER(TOP_LAYER, "jgdi_delete");
   
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);

   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      goto error;
   }
   
   sge_gdi_set_thread_local_ctx(ctx);
   
   /* we don't have an element for SGE_STN_LIST */
   if (target_list != SGE_STN_LIST) {
      if ((ret = obj_to_listelem(env, jobj, &ep, descr, &alp)) != JGDI_SUCCESS) {
         goto error;
      }
   }
 
   /*
   ** special handling for JB_Type and AR_Type needed since 
   ** they are using ID_Type elements to delete
   ** TODO: not yet mapped: deletion of more than one element at once
   **                       forced flag
   **                       user list support
   */
   if (target_list == SGE_JB_LIST) {
      sprintf(id_buf, sge_u32, lGetUlong(ep, JB_job_number));
      lFreeElem(&ep);
      iep = lAddElemStr(&lp, ID_str, id_buf, ID_Type); 
      lSetUlong(iep, ID_force, force);
      what = lWhat("%T(ALL)", ID_Type);
   } else if (target_list == SGE_AR_LIST) {
      sprintf(id_buf, sge_u32, lGetUlong(ep, AR_id));
      lFreeElem(&ep);
      iep = lAddElemStr(&lp, ID_str, id_buf, ID_Type);
      lSetUlong(iep, ID_force, force);
      what = lWhat("%T(ALL)", ID_Type);
   } else if (target_list == SGE_STN_LIST) {
      /* special handling: lp remains NULL */
   } else {
      lp = lCreateList("", descr);
      lAppendElem(lp, ep);
      what = lWhat("%T(ALL)", descr);
   }   

   /* delete from target_list */
   alp = ctx->gdi(ctx, target_list, SGE_GDI_DEL, &lp, where, what, false);
   lFreeList(&lp);

   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }

   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }
   
error:

   /*
   ** this must be called before throw_error_from_answer_list, otherwise there is a pending 
   ** exception in the way
   */
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
 
   lFreeList(&alp);
   lFreeWhat(&what);

   DRETURN_VOID;
}

void jgdi_update(JNIEnv *env, jobject jgdi, jobject jobj, const char *classname, int target_list, lDescr *descr, jobject answers)
{
   lList *lp = NULL;
   lList *alp = NULL;
   lCondition *where = NULL;
   static lEnumeration *what  = NULL;
   lListElem *ep = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER(TOP_LAYER, "jgdi_update");

   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      goto error;
   }
   
   sge_gdi_set_thread_local_ctx(ctx);

   ret = obj_to_listelem(env, jobj, &ep, descr, &alp);

   if (ret != JGDI_SUCCESS) {
      goto error;
   }
 
   lp = lCreateList("", descr);
   lAppendElem(lp, ep);
   
   jgdi_log_printf(env, JGDI_LOGGER, FINE,
                   "BEGIN --------------- jgdi_update %s -------------------------------", classname); 
   
   jgdi_log_list(env, JGDI_LOGGER, FINE, lp);
   
   jgdi_log_printf(env, JGDI_LOGGER, FINE,
                   "END --------------- jgdi_update %s -------------------------------", classname);
                   
   /* create what and where */
   what = lWhat("%T(ALL)", descr);

   alp = ctx->gdi(ctx, target_list, SGE_GDI_MOD | SGE_GDI_SET_ALL, &lp, where, what, false);
   
   lFreeList(&lp);
   lFreeWhat(&what);

   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }

   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }
   
error:
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
 
   lFreeList(&alp);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   DRETURN_VOID;
}

/*
** -km
** -ks
** -ke host_list | all
** -kej host_list | all
** -kec id_list | all
*/
static void jgdi_kill(JNIEnv *env, jobject jgdi, lList* lp, int kill_target, jobject answers)
{
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   const char *default_cell = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER(TOP_LAYER, "jgdi_kill");
   
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   /* get context */
   ret = getGDIContext(env, jgdi, &ctx, &alp);

   sge_gdi_set_thread_local_ctx(ctx);

   if (ret == JGDI_SUCCESS) {
      default_cell = ctx->get_default_cell(ctx);
      alp = ctx->kill(ctx, lp, default_cell, 0, kill_target);
      
      /* if error throw exception */
      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
      }
   }
   
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }

   lFreeList(&alp);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   DRETURN_VOID;
}

/*
** qconf -clearusage
** TODO: this operation should be done by qmaster in one sweep,
**       implement via SGE_GDI_TRIGGER operation
**       otherwise use gdi_multi
*/
static void jgdi_clearusage(JNIEnv *env, jobject jgdi, jobject answers)
{
   lList *alp = NULL;
   lList *lp = NULL;
   lList *lp2 = NULL;
   lListElem *ep = NULL;
   static lEnumeration *what = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER(TOP_LAYER, "jgdi_clearusage");

   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      goto error;
   }
   sge_gdi_set_thread_local_ctx(ctx);

   what = lWhat("%T(ALL)", STN_Type);

   alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_GET, &lp, NULL, what, false);
   
   /* if error throw exception */
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }
   lFreeList(&alp);

   alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_GET, &lp2, NULL, what, false);

   /* if error throw exception */
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }
   lFreeList(&alp);

   /* clear user usage */
   for_each(ep, lp) {
      lSetList(ep, UU_usage, NULL);
      lSetList(ep, UU_project, NULL);
   }

   /* clear project usage */
   for_each(ep, lp2) {
      lSetList(ep, PR_usage, NULL);
      lSetList(ep, PR_project, NULL);
   }
   /* update user usage */
   if (lp && lGetNumberOfElem(lp) > 0) {
     alp = ctx->gdi(ctx, SGE_UU_LIST, SGE_GDI_MOD, &lp, NULL, NULL, false);
   }

   /* if error throw exception */
   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }
   lFreeList(&alp);
   
   /* update project usage */
   if (lp2 && lGetNumberOfElem(lp2) > 0) {
     alp = ctx->gdi(ctx, SGE_PR_LIST, SGE_GDI_MOD, &lp2, NULL, NULL, false);
   }

   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }

error:
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }

   lFreeList(&alp);
   lFreeList(&lp);
   lFreeList(&lp2);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   DRETURN_VOID;
}


/*
     qmod [ options ] [ wc_job_range_list | wc_queue_list ]

DESCRIPTION
     Qmod enables users classified as owners  (see  queue_conf(5)
     for  details)  of  a workstation to modify the state of Grid
     Engine queues for his/her machine as well as  the  state  of
     his/her  own  jobs.   A manager/operator or root can execute
     qmod for any queue and job in  a  cluster.  Find  additional
     information  concerning  wc_queue_list  and  wc_job_list  in
     sge_types(1).

OPTIONS
     -c   Clears   the   error    state    of    the    specified
          jobs(s)/queue(s).   Do not use this switch anymore, use
          -cj / -cq instead.

     -cj  Clears the error state of the specified jobs(s).

     -cq  Clears the error state of the specified queue(s).

     -d   Disables  the  queue(s),  i.e.  no  further  jobs   are
          dispatched  to  disabled queues while jobs already exe-
          cuting in these queues are allowed to finish.

     -e   Enables the queue(s).

     -f   Force the modification action for the queue despite the
          apparent  current  state of the queue. For example if a
          queue appears to be suspended  but  the  job  execution
          seems to be continuing the manager/operator can force a
          suspend operation which will  send  a  SIGSTOP  to  the
          jobs.  In any case, the queue or job status will be set
          even if the sge_execd(8)  controlling  the  queues/jobs
          cannot    be    reached.    Requires   manager/operator
          privileges.

     -r   If applied to queues, reschedules  all  jobs  currently
          running  in  this  queue.   If applied to running jobs,
          reschedules  the  jobs.  Requires   root   or   manager
          privileges.  Do not use this switch anymore, use -rj /-
          rq instead.

     -rj  If applied  to  running  jobs,  reschedules  the  jobs.
          Requires root or manager privileges.

     -rq  If applied to queues, reschedules  all  jobs  currently
          running  in  this  queue.   Requires  root  or  manager
          privileges.

     -s   If applied to queues, suspends the queues and any  jobs
          which  might  be  active.  If  applied to running jobs,
          suspends the jobs. Do not use this switch anymore,  use
          -sj / -sq instead.

     -sj  If applied to running jobs, suspends the jobs. If a job
          is  both suspended explicitly and via suspension of its
          queue, a following unsuspend  of  the  queue  will  not
          release the suspension state on the job.

     -sq  If applied to queues, suspends the queues and any  jobs
          which might be active.

     -us  If applied to queues, unsuspends  the  queues  and  any
          jobs  which  might  be  active.  If  applied  to  jobs,
          unsuspends the jobs. Do not use  this  switch  anymore,
          use -usj / -usq instead.

     -usj If applied to jobs, unsuspends the jobs. If  a  job  is
          both  suspended  explicitly  and  via suspension of its
          queue, a following unsuspend  of  the  queue  will  not
          release the suspension state on the job.

     -usq If applied to queues, unsuspends  the  queues  and  any
          jobs which might be active.

   qconf -cq wc_queue_list             <clean queue>
     Cleans queue from jobs which haven't been reaped.  Primarily a
     development tool. Requires root/manager/operator privileges.
     Find a description of wc_queue_list in sge_types(1).

*/
static void jgdi_qmod(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, u_long32 transition, u_long32 option, jobject answers)
{
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   lList *alp = NULL;
   
   DENTER(TOP_LAYER, "jgdi_qmod");
  
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);

   if (obj_array != NULL) {
      
      sge_gdi_ctx_class_t *ctx = NULL;
      int i;
      lList *ref_list = NULL;
      jsize asize = (*env)->GetArrayLength(env, obj_array);
      
      if (transition != QI_DO_CLEAN) {
         if (!transition_is_valid_for_qinstance(transition, &alp)) {
            ret = JGDI_ERROR;
            goto error;
         }
      }
      
      if (!transition_option_is_valid_for_qinstance(option, &alp)) {
         ret = JGDI_ERROR;
         goto error;
      }
      
      for (i=0; i<asize; i++) {
          jobject obj = (*env)->GetObjectArrayElement(env, obj_array, i);
          if (obj) {
             lListElem *idep = NULL;
             const char* name = (*env)->GetStringUTFChars(env, obj, 0);
             if (name == NULL) {
                answer_list_add(&alp, "jgdi_qmod: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
                ret = JGDI_ERROR;
                goto error;
             }
             idep = lAddElemStr(&ref_list, ID_str, name, ID_Type);
             lSetUlong(idep, ID_action, transition);
             lSetUlong(idep, ID_force, option);
             if (name) {
               (*env)->ReleaseStringUTFChars(env, obj, name);
             }
          }
      }
      
      jgdi_log_printf(env, JGDI_LOGGER, FINER,
                      "jgdi_mod: ref_list BEGIN ----------------------------------------");
      
      jgdi_log_list(env, JGDI_LOGGER, FINER, ref_list);

      jgdi_log_printf(env, JGDI_LOGGER, FINER,
                      "jgdi_mod: ref_list END ----------------------------------------");
      
      /* get context */
      ret = getGDIContext(env, jgdi, &ctx, &alp);
      if (ret != JGDI_SUCCESS) {
         goto error;
      }
      sge_gdi_set_thread_local_ctx(ctx);

      alp = ctx->gdi(ctx, SGE_CQ_LIST, SGE_GDI_TRIGGER, &ref_list, NULL, NULL, false);
      lFreeList(&ref_list);
      
      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
      }
      
   }

error:
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
   lFreeList(&alp);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);
   
   DRETURN_VOID;
}

static void jgdi_detached_settings(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jstring *jdetachedStrPtr, jobject answers) {
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   lList *lp = NULL;
   lList *hgroup_list = NULL;
   lList *cqueue_list = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   lList *alp =NULL;
   jstring jdetachedStr = NULL;
   
   lEnumeration *hgrp_what = NULL; 
   lEnumeration *cqueue_what = NULL;
   int hgrp_id = 0; 
   int cq_id = 0;
   lList *local_answer_list = NULL;
   lList *multi_answer_list = NULL;
   state_gdi_multi state = STATE_GDI_MULTI_INIT;

   DENTER(TOP_LAYER, "jgdi_detached_settings");

   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   
   if (obj_array != NULL) {
      int i;
      jsize asize = (*env)->GetArrayLength(env, obj_array);
      for (i=0; i<asize; i++) {
         jobject obj = (*env)->GetObjectArrayElement(env, obj_array, i);
         if (obj) {
            const char* queuename = (*env)->GetStringUTFChars(env, obj, 0);
            if (queuename == NULL) {
               answer_list_add(&alp, "jgdi_detached_settings: GetStringUTFChars failed. Out of memory.", STATUS_EMALLOC, ANSWER_QUALITY_ERROR);
               ret = JGDI_ERROR;
               goto error;
            }
            DPRINTF(("queuename: %s\n", queuename));
            lAddElemStr(&lp, CQ_name, queuename, CQ_Type);
            (*env)->ReleaseStringUTFChars(env, obj, queuename);
         }
      }
      jgdi_log_printf(env, JGDI_LOGGER, FINER,
      "jgdi_show_detached_settings: lp BEGIN ----------------------------------------");
      
      jgdi_log_list(env, JGDI_LOGGER, FINER, lp);
      
      jgdi_log_printf(env, JGDI_LOGGER, FINER,
      "jgdi_show_detached_settings: lp END ----------------------------------------");
      
   }
   
   /* get context */
   ret = getGDIContext(env, jgdi, &ctx, &alp);
   if (ret != JGDI_SUCCESS) {
      goto error;
   }
   
   sge_gdi_set_thread_local_ctx(ctx);

   /* HGRP */
   hgrp_what = lWhat("%T(ALL)", HGRP_Type);
   hgrp_id = ctx->gdi_multi(ctx, &alp, SGE_GDI_RECORD, SGE_HGRP_LIST,
                           SGE_GDI_GET, NULL, NULL, hgrp_what, false, &state, true);
   lFreeWhat(&hgrp_what);

   /* CQ */
   cqueue_what = lWhat("%T(ALL)", CQ_Type);
   cq_id = ctx->gdi_multi(ctx, &alp, SGE_GDI_SEND, SGE_CQ_LIST,
                         SGE_GDI_GET, NULL, NULL, cqueue_what, false,
                         &state, true);
   ctx->gdi_wait(ctx, &alp, &multi_answer_list, &state);
   lFreeWhat(&cqueue_what);

   /* HGRP */
   sge_gdi_extract_answer(&local_answer_list, SGE_GDI_GET,
                   SGE_HGRP_LIST, hgrp_id, multi_answer_list, &hgroup_list);
   if (local_answer_list != NULL) {
      lListElem *answer = lFirst(local_answer_list);

      if (lGetUlong(answer, AN_status) != STATUS_OK) {
         lDechainElem(local_answer_list, answer);
         answer_list_add_elem(&alp, answer);
      }
   }
   lFreeList(&local_answer_list);
   
   /* CQ */   
   sge_gdi_extract_answer(&local_answer_list, SGE_GDI_GET, 
                SGE_CQ_LIST, cq_id, multi_answer_list, &cqueue_list);
   if (local_answer_list != NULL) {
      lListElem *answer = lFirst(local_answer_list);

      if (lGetUlong(answer, AN_status) != STATUS_OK) {
         lDechainElem(local_answer_list, answer);
         answer_list_add_elem(&alp, answer);
      }
   } 
   lFreeList(&local_answer_list);
   lFreeList(&multi_answer_list);
      
   if (answers != NULL) {
      generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
   }
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
   } else {
      jgdi_log_answer_list(env, JGDI_LOGGER, alp);
   }
   
   if (ret != JGDI_ERROR) {
      dstring ds = DSTRING_INIT;
      lListElem *cqueue = NULL;
      
      for_each(cqueue, cqueue_list) {
         cqueue_sick(cqueue, &alp, hgroup_list, &ds);
      }
      if (sge_dstring_get_string(&ds)) {
         const char *detached_str = sge_dstring_get_string(&ds);
         jdetachedStr = (*env)->NewStringUTF(env, detached_str);
         sge_dstring_free(&ds);
      }
   }
   *jdetachedStrPtr = jdetachedStr;
   
error:
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }
   
   lFreeList(&alp);
   lFreeList(&lp);
   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   DRETURN_VOID;
}


/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillAllExecdsWithAnswer
 * Signature: (ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillAllExecdsWithAnswer(JNIEnv *env, jobject jgdi, jboolean terminate_jobs, jobject answers)
{
   int kill_target = EXECD_KILL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillAllExecdsWithAnswer");
  
   if (terminate_jobs) {
      kill_target |= JOB_KILL;
   }
   jgdi_kill(env, jgdi, NULL, kill_target, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillExecdWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillExecdWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean terminate_jobs, jobject answers)
{
   lList *lp = NULL;
   int kill_target = EXECD_KILL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillExecdWithAnswer");
  
   if (obj_array != NULL) {
      int i;
      jsize asize = (*env)->GetArrayLength(env, obj_array);
      for (i=0; i<asize; i++) {
         jobject obj = (*env)->GetObjectArrayElement(env, obj_array, i);
         if (obj) {
            const char* hostname = (*env)->GetStringUTFChars(env, obj, 0);
            if (hostname == NULL) {
               return;  /*LP Out of memoery is already thrown in JVM, just return*/
            }
            DPRINTF(("hostname: %s\n", hostname));
            lAddElemHost(&lp, EH_name, hostname, EH_Type);
            (*env)->ReleaseStringUTFChars(env, obj, hostname);
         }
      }
   
      if (terminate_jobs) {
         kill_target |= JOB_KILL;
      }
      jgdi_kill(env, jgdi, lp, kill_target, answers);
      lFreeList(&lp);
   }

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillEventClientsWithAnswer
 * Signature: ([ILjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillEventClientsWithAnswer(JNIEnv *env, jobject jgdi, jintArray iarray, jobject answers)
{
   jsize length = 0;
   jint *ibuf = NULL;
   int i;
   lList *lp = NULL;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillEventClientsWithAnswer");

   if (iarray == NULL) {
      DEXIT;
      return;
   }

   length = (*env)->GetArrayLength(env, iarray);

   if (length <= 0) {
      DEXIT;
      return;
   }

   ibuf = (jint *) malloc(sizeof(jint)*length);
   
   (*env)->GetIntArrayRegion(env, iarray, 0, length, ibuf);
   for (i=0; i<length; i++) {
      char buffer[BUFSIZ];
      sprintf(buffer, "%d", (int) ibuf[i]);
      DPRINTF(("ec: %s\n", buffer));
      lAddElemStr(&lp, ID_str, buffer, ID_Type);
   }
   sge_free(&ibuf);
   jgdi_kill(env, jgdi, lp, EVENTCLIENT_KILL, answers);
   lFreeList(&lp);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeTriggerSchedulerMonitoringWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeTriggerSchedulerMonitoringWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   lList *alp = NULL;
   sge_gdi_ctx_class_t *ctx = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   rmon_ctx_t rmon_ctx;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeTriggerSchedulerMonitoringWithAnswer");
  
   jgdi_init_rmon_ctx(env, JGDI_LOGGER, &rmon_ctx);
   rmon_set_thread_ctx(&rmon_ctx);
   

   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) == JGDI_SUCCESS) {
      alp = ctx->tsm(ctx, NULL, NULL);
      if (answer_list_has_error(&alp)) {
         ret = JGDI_ERROR;
      }
      sge_gdi_set_thread_local_ctx(ctx);

      if (answers != NULL) {
         generic_fill_list(env, answers, "com/sun/grid/jgdi/configuration/JGDIAnswer", alp, NULL);
      }
   }
   
   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }

   lFreeList(&alp);

   sge_gdi_set_thread_local_ctx(NULL);
   rmon_set_thread_ctx(NULL);
   jgdi_destroy_rmon_ctx(&rmon_ctx);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillAllEventClientsWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillAllEventClients(JNIEnv *env, jobject jgdi, jobject answers)
{
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillAllEventClientsWithAnswer");

   jgdi_kill(env, jgdi, NULL, EVENTCLIENT_KILL, answers);

   DRETURN_VOID;
}
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillMasterWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillMasterWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillMasterWithAnswer");
   
   jgdi_kill(env, jgdi, NULL, MASTER_KILL, answers);

   DRETURN_VOID;
}
/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeKillSchedulerWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillSchedulerWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeKillSchedulerWithAnswer");

   jgdi_kill(env, jgdi, NULL, SCHEDD_KILL, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeStartSchedulerWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeStartSchedulerWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   lList *lp = NULL;
   lListElem *ep = NULL;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeStartSchedulerWithAnswer");

   ep = lAddElemStr(&lp, ID_str, "scheduler", ID_Type);
   lSetUlong(ep, ID_action, SGE_THREAD_TRIGGER_START); 
   jgdi_kill(env, jgdi, lp, THREAD_START, answers);
   lFreeList(&lp);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeCleanQueuesWithAnswer
 * Signature: ([Ljava/lang/String;Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeCleanQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jobject answers)
{
   u_long32 transition = QI_DO_CLEAN;
   u_long32 option = false;
   jboolean force = false;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeCleanQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeUnsuspendWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers) 
{
   u_long32 transition = QI_DO_UNSUSPEND;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeUnsuspendQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers) 
{
   u_long32 transition = QI_DO_UNSUSPEND | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeUnsuspendJobsWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendJobsWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_UNSUSPEND | JOB_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeUnsuspendJobs");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeSuspendWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_SUSPEND;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeSuspendQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_SUSPEND | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeSuspendJobsWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendJobsWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_SUSPEND | JOB_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeSuspendJobsWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeRescheduleJobsWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleJobsWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_RESCHEDULE | JOB_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleJobsWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeRescheduleWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_RESCHEDULE;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeRescheduleQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_RESCHEDULE | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeRescheduleQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeClearJobsWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearJobsWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_CLEARERROR | JOB_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearJobsWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeClearQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_CLEARERROR | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeDisableQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeDisableQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_DISABLE | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeDisableQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeEnableQueuesWithAnswer
 * Signature: ([Ljava/lang/String;ZLjava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeEnableQueuesWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jboolean force, jobject answers)
{
   u_long32 transition = QI_DO_ENABLE | QUEUE_DO_ACTION;
   u_long32 option = force;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeEnableQueuesWithAnswer");

   jgdi_qmod(env, jgdi, obj_array, force, transition, option, answers);

   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeClearShareTreeUsageWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearShareTreeUsageWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeClearShareTreeUsageWithAnswer");

   jgdi_clearusage(env, jgdi, answers);
   
   DRETURN_VOID;
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeShowDetachedSettingsAllWithAnswer
 * Signature: (Ljava/util/List;)Ljava/lang/String
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeShowDetachedSettingsAllWithAnswer(JNIEnv *env, jobject jgdi, jobject answers) {
   
   jstring jdetachedStr = NULL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeShowDetachedSettingsAllWithAnswer");
   
   jgdi_detached_settings(env, jgdi, NULL, &jdetachedStr, answers);
   
   DRETURN(jdetachedStr);
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeShowDetachedSettingsWithAnswer
 * Signature: ([Ljava/lang/String;Ljava/util/List)Ljava/lang/String
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeShowDetachedSettingsWithAnswer(JNIEnv *env, jobject jgdi, jobjectArray obj_array, jobject answers) {
   
   jstring jdetachedStr = NULL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeShowDetachedSettingsWithAnswer");
   
   jgdi_detached_settings(env, jgdi, obj_array, &jdetachedStr, answers);
   
   DRETURN(jdetachedStr);
}

/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeGetSchedulerHost
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSchedulerHost(JNIEnv *env, jobject jgdi)
{
   jstring jschedd_host = NULL;
   lList *lp = NULL;
   lList *alp = NULL;
   static lCondition *where = NULL;
   static lEnumeration *what = NULL;
   jgdi_result_t ret = JGDI_SUCCESS;
   sge_gdi_ctx_class_t *ctx = NULL;
   
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeGetSchedulerHost");

   /* get context */
   if ((ret = getGDIContext(env, jgdi, &ctx, &alp)) != JGDI_SUCCESS) {
      goto error;
   }
   sge_gdi_set_thread_local_ctx(ctx);

   what = lWhat("%T(%I)", EV_Type, EV_host);
   where = lWhere("%T(%I==%u))", EV_Type, EV_id, EV_ID_SCHEDD);

   /* get list */
   alp = ctx->gdi(ctx, SGE_EV_LIST, SGE_GDI_GET, &lp, where, what, false);
   
   lFreeWhat(&what);
   lFreeWhere(&where);

   /* if error throw exception */
   if (answer_list_has_error(&alp)) {
      ret = JGDI_ERROR;
      goto error;
   }

   if (lp) {
      const char *schedd_host = lGetHost(lFirst(lp), EV_host);
      if (schedd_host) {
         jschedd_host = (*env)->NewStringUTF(env, schedd_host);
      }
   }
   
error:

   /* if error throw exception */
   if (ret != JGDI_SUCCESS) {
      throw_error_from_answer_list(env, ret, alp);
   }

   lFreeList(&alp);
   lFreeList(&lp);
   sge_gdi_set_thread_local_ctx(NULL);

   DRETURN(jschedd_host);
}

/*
 * Class:     com_sun_grid_jgdi_JGDIFactory
 * Method:    nativeSetJGDIVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_grid_jgdi_JGDIFactory_nativeSetJGDIVersion(JNIEnv *env, jclass jgdi_factory)
{
   char version_string[BUFSIZ];

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_JGDIFactory_nativeSetJGDIVersion");
   sprintf(version_string, "%s %s", GE_SHORTNAME, GDI_VERSION);
   DRETURN((*env)->NewStringUTF(env, version_string));
}


/*
 * Class:     com_sun_grid_jgdi_util_shell_editor_EditorUtil
 * Method:    nativeSgeEdit
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_sun_grid_jgdi_util_shell_editor_EditorUtil_nativeSgeEdit(JNIEnv *env, jclass clazz, jstring path) {
   jint ret = 0;
   uid_t uid = getuid();
   gid_t gid = getgid();
   const char *strpath = NULL;

   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_util_shell_editor_EditorUtil_nativeSgeEdit");

   strpath = (*env)->GetStringUTFChars(env, path, 0);
   ret = sge_edit(strpath, uid, gid);
   if (strpath) { 
      (*env)->ReleaseStringUTFChars(env, path, strpath);
   }

   DRETURN(ret);
}


/*
 * Class:     com_sun_grid_jgdi_jni_JGDIBaseImpl
 * Method:    nativeDeleteShareTreeWithAnswer
 * Signature: (Ljava/util/List;)V
 */
JNIEXPORT void JNICALL Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeDeleteShareTreeWithAnswer(JNIEnv *env, jobject jgdi, jobject answers)
{
   DENTER(TOP_LAYER, "Java_com_sun_grid_jgdi_jni_JGDIBaseImpl_nativeDeleteShareTreeWithAnswer");
   
   jgdi_delete(env, jgdi, NULL, "com/sun/grid/jgdi/configuration/ShareTree", SGE_STN_LIST, STN_Type, false, answers);

   DRETURN_VOID;
}

/*-------------------------------------------------------------------------*
 * NAME
 *   calendar_to_elem - Converts a java.util.Calendar object into
 *                      a cull object of type TM_Type
 * PARAMETER
 *  thiz - the object mapping
 *  env  - JNI environment
 *  obj  - the java.util.Calendar object
 *  elem - the TM_Type object
 *  alpp - answer list for error reporting
 *
 * RETURN
 *
 *  JGDI_SUCCESS - if the java.util.Calendar object has been successfully
 *                 converted into a TM_Type cull object
 *
 *  JGDI_ERROR   - on error (reason has been reported in alpp)
 *
 *-------------------------------------------------------------------------*/
static jgdi_result_t calendar_to_elem(object_mapping_t *thiz, JNIEnv *env, jobject obj, lListElem *elem, lList **alpp) {
   jlong  time = 0;
   time_t clock = 0;
   struct tm time_str;

   DENTER(JGDI_LAYER, "calendar_to_elem");
   
   if (Calendar_getTimeInMillis(env, obj, &time, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   memset(&time_str, 0, sizeof(struct tm));
   
   clock = (time_t)time;
   
   gmtime_r(&clock, &time_str);
   
   cullify_tm(elem, &time_str);
   
   DRETURN(JGDI_SUCCESS);
}


/*-------------------------------------------------------------------------*
 * NAME
 *   elem_to_calendar - Convert a cull object of type TM_Type into
 *                      a java.util.Calendar object
 * PARAMETER
 *  thiz - the object mapping
 *  env  - the JNI environment
 *  elem - the cull object
 *  obj  - pointer to the java object reference
 *  alpp - answer list for error reporting
 *
 * RETURN
 *  JGDI_SUCCESS - if the cull object has been successfully converted
 *                 into a java.util.Calendar object
 *
 *  JGDI_ERROR   - on error (reason has been reported in alpp)
 *
 *-------------------------------------------------------------------------*/
static jgdi_result_t elem_to_calendar(object_mapping_t *thiz, JNIEnv *env, lListElem *elem, jobject* obj, lList **alpp) {
   
   jlong  time = 0;
   struct tm time_str;
   
   DENTER(JGDI_LAYER, "calendar_to_elem");
   
   memset(&time_str, 0, sizeof(struct tm));
   
   uncullify_tm(elem, &time_str);
   
   time = (jlong)mktime(&time_str);
   
   if (Calendar_static_getInstance(env, obj, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   if (Calendar_setTimeInMillis(env, *obj, time, alpp) != JGDI_SUCCESS) {
      DRETURN(JGDI_ERROR);
   }
   
   DRETURN(JGDI_SUCCESS);
}


/*-------------------------------------------------------------------------*
 * NAME
 *   get_object_mapping - Get a object mapping for a cull descriptor
 * PARAMETER
 *  descr - the cull descriptor
 *
 * RETURN
 *
 *  The object mapping of null.
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *-------------------------------------------------------------------------*/
static object_mapping_t* get_object_mapping(const lDescr *descr) {

   int i = 0;
   object_mapping_t *mapping = NULL;
   
   DENTER(JGDI_LAYER, "get_object_mapping");
   
   for (mapping = OBJECT_MAPPINGS; mapping->descr != NULL; mapping++) {
      for(i = lCountDescr(mapping->descr)-1; i >= 0; i--) {
         int name = lGetPosName(mapping->descr, i);
         if (lGetPosInDescr(descr, name) >= 0) {
            DEXIT;
            DRETURN(mapping);
         }
      }
   }

   DRETURN(NULL);
}

