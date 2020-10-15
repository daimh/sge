#ifndef JGDI_COMMON_H
#define JGDI_COMMON_H

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

#include "gdi/sge_gdi_ctx.h"

typedef enum {
   JGDI_SUCCESS,
   JGDI_ERROR,
   JGDI_ILLEGAL_STATE,
   JGDI_ILLEGAL_ARGUMENT,
   JGDI_NULL_POINTER
} jgdi_result_t;

jgdi_result_t getGDIContext(JNIEnv *env, jobject jgdi, sge_gdi_ctx_class_t** ctx, lList **alpp);
jgdi_result_t listelem_to_obj(JNIEnv *env, lListElem *ep, jobject *object, const lDescr *descr, jclass clazz, lList **alpp );
jgdi_result_t obj_to_listelem(JNIEnv *env, jobject obj, lListElem **elem, const lDescr* descr, lList **alpp);
jgdi_result_t set_object_attribute(JNIEnv* env, lListElem *elem, const lDescr *descr, jobject target, jobject propDescr, lList **alpp);
jgdi_result_t set_elem_attribute(JNIEnv* env, lListElem *ep, const lDescr* descr, jobject obj, jobject propDescr, lList **alpp);


jgdi_result_t get_double(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, double *retdou, lList **alpp);
jgdi_result_t set_double( JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, double value, lList **alpp );
jgdi_result_t get_float(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, float *retfl, lList **alpp);
jgdi_result_t set_float( JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, float value, lList **alpp );
jgdi_result_t get_string(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, char **retstr, lList **alpp);
jgdi_result_t set_string(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, const char* value, lList **alpp );
jgdi_result_t get_int(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, u_long32 *reti, lList **alpp);
jgdi_result_t get_long(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, lLong *ret, lList **alpp);
jgdi_result_t set_long( JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, lLong value, lList **alpp);
jgdi_result_t set_int( JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, u_long32 value, lList **alpp);
jgdi_result_t get_bool(JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, lBool *retb, lList **alpp);
jgdi_result_t set_bool( JNIEnv *env, jclass beanClass, jobject obj, const char* propertyName, lBool value, lList **alpp );
jgdi_result_t get_list(JNIEnv *env, jclass bean_class, jobject bean, jobject property_descr, lList**list, lList **alpp);
jgdi_result_t set_list(JNIEnv *env, jclass beanClass, jobject bean, jobject propertyDescr, lList *lp, lList **alpp);


jgdi_result_t get_string_list(JNIEnv *env, jobject obj, const char* getter, lList **lpp, lDescr *descr, int nm, lList **alpp);

void throw_error(JNIEnv *env, jgdi_result_t result, const char* message, ... );
void throw_error_from_answer_list(JNIEnv *env, jgdi_result_t result, lList* alp);
void throw_error_from_handler(JNIEnv *env, sge_error_class_t *eh);

/** generated in jgdi_mapping.c */
lDescr* get_descr(const char* name);
lDescr* get_descr_for_classname(const char* className );
const char* get_classname_for_descr(const lDescr *descr);
int get_master_list_for_classname(const char* classname);
jstring get_class_name(JNIEnv *env, jclass cls, lList **alpp);

void object_to_str(JNIEnv* env, jobject obj, char* buf, size_t buf_len);

jmethodID get_methodid( JNIEnv *env, jclass cls, const char* methodName,
                               const char* signature, lList **alpp);
jmethodID get_static_methodid( JNIEnv *env, jclass cls, const char* methodName,
                               const char* signature, lList **alpp);
                               
jfieldID get_static_fieldid( JNIEnv *env, jclass cls, const char* fieldName,
                               const char* signature, lList **alpp);
                               

void clear_error(JNIEnv* env);
jboolean test_jni_error(JNIEnv* env, const char* message, lList **alpp);


#define THROW_ERROR(msg)            DTRACE;throw_error msg
#define THROW_ERROR_H(env, h)       DTRACE;throw_error_from_handler(env, h)

#define JGDI_LAYER      TOP_LAYER
#define JGDI_PROGNAME   JGDI


jgdi_result_t build_filter(JNIEnv *env, jobject filter, lCondition **where, lList **alpp);
jgdi_result_t build_resource_filter(JNIEnv *env, jobject resource_filter, lList** resource_list_ref, lList **alpp);

void jgdi_fill(JNIEnv *env, jobject jgdi, jobject list, jobject filter, const char *classname, int target_list, lDescr *descr, jobject answers);

void jgdi_add(JNIEnv *env, jobject jgdi, jobject jobj, const char *classname, int target_list, lDescr *descr, jobject answers);

void jgdi_delete(JNIEnv *env, jobject jgdi, jobject jobj, const char* classname, int target_list, lDescr *descr, jboolean force, jobject answers);
void jgdi_delete_array(JNIEnv *env, jobject jgdi, jobjectArray obj_array, const char *classname, int target_list, lDescr *descr, jboolean force, jobject options, jobject answers);

void jgdi_update(JNIEnv *env, jobject jgdi, jobject jobj, const char *classname, int target_list, lDescr *descr, jobject answers);


#endif
