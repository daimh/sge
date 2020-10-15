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
#include <stdio.h>
#include <jni.h>
#include "jgdi.h"
#include "basis_types.h"
#include "cull.h"
#include "commlib.h"
#include "sge_rmon.h"
#include "sge_all_listsL.h"
#include "sge_answer.h"
#include "sge_prog.h"
#include "sge_bootstrap.h"
#include "sge_gdi_ctx.h"
#include "cl_errors.h"
#include "sge_log.h"
#include "sge_error_class.h"
#include "jgdi_common.h"
#include "jgdi_wrapper.h"


static jclass find_class(JNIEnv *env, const char *fullClassname, lList** alpp) {
   jclass clazz = NULL;
   jclass tmpclazz = NULL;

   DENTER(BASIS_LAYER, "find_class");

   if (fullClassname == NULL) {
      answer_list_add(alpp, "fullClassname is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return NULL;
   }
   tmpclazz = (*env)->FindClass(env, fullClassname);

   if (test_jni_error(env, "Class not found", alpp)) {
       DEXIT;
       return NULL;
   }

   /* aquire a global ref for the class object */
   clazz = (jclass)(*env)->NewGlobalRef(env, tmpclazz);
   if (clazz == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                      "Can not get a global reference on the %s class object",
                      fullClassname);
      DEXIT;
      return NULL;
   }
   DEXIT;
   return clazz;
}

static jgdi_result_t get_static_method_id_for_fullClassname(JNIEnv *env, jclass *clazzref, jmethodID *midref, const char *fullClassname,
                                      const char *methodName, const char *methodSignature, lList **alpp) {                                      

   DENTER(BASIS_LAYER, "get_static_method_id_for_fullClassname");

   /* Test preconditions */
   if (env == NULL) {
      answer_list_add(alpp, "env is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   if (midref == NULL) {
      answer_list_add(alpp, "midref is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   if (clazzref == NULL) {
      answer_list_add(alpp, "clazzref is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   
   *clazzref = find_class(env, fullClassname, alpp);
   if (*clazzref == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "class %s not found", fullClassname);
      DEXIT;
      return JGDI_ILLEGAL_STATE;
   }
   
   *midref = get_static_methodid(env, *clazzref, methodName, methodSignature, alpp);
   if (*midref == NULL) {
      DEXIT;
      return JGDI_ILLEGAL_STATE;
   }
   
   DEXIT;
   return JGDI_SUCCESS;
}


static jgdi_result_t get_method_id_for_fullClassname(JNIEnv *env, jobject obj, jmethodID *midref, const char *fullClassname,
                                      const char *methodName, const char *methodSignature, lList **alpp) {                                      

   jclass clazz = NULL;

   DENTER(BASIS_LAYER, "get_method_id_for_fullClassname");

   /* Test preconditions */
   if (env == NULL) {
      answer_list_add(alpp, "env is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   if (obj == NULL) {
      answer_list_add(alpp, "obj is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   if (midref == NULL) {
      answer_list_add(alpp, "midref is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DEXIT;
      return JGDI_NULL_POINTER;
   }
   
   clazz = find_class(env, fullClassname, alpp);
   if (clazz == NULL) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                              "class %s not found", fullClassname);
      DEXIT;
      return JGDI_ILLEGAL_STATE;
   }
   if (!(*env)->IsInstanceOf(env, obj, clazz)) {
      answer_list_add_sprintf(alpp, STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR,
                      "obj is not an instanceof %s", fullClassname);
      DEXIT;
      return JGDI_ILLEGAL_STATE;
   }
   
   *midref = get_methodid(env, clazz, methodName, methodSignature, alpp);
   if (*midref == NULL) {
      DEXIT;
      return JGDI_ILLEGAL_STATE;
   }
   
   DEXIT;
   return JGDI_SUCCESS;
}
