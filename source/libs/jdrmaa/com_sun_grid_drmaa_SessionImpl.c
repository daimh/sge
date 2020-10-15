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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <errno.h>

#include "japi/drmaa.h"
#include "japi/msg_drmaa.h"
#include "com_sun_grid_drmaa_SessionImpl.h"

#define BUFFER_LENGTH 1024
#define TEMPLATE_LIST_LENGTH 1024

enum {
   /* -------------- these are relevant to all sections ---------------- */
   DRMAAJ_ERRNO_SUCCESS = 0, /* Routine returned normally with success. */

   DRMAAJ_ERRNO_INTERNAL_ERROR, /* Unexpected or internal DRMAA error like
                                   memory allocation, system call failure,
                                   etc. */
   DRMAAJ_ERRNO_DRM_COMMUNICATION_FAILURE, /* Could not contact DRM system for
                                              this request. */
   DRMAAJ_ERRNO_AUTH_FAILURE, /* The specified request is not processed
                                 successfully due to authorization failure. */
   DRMAAJ_ERRNO_INVALID_ARGUMENT, /* The input value for an argument is
                                     invalid. */
   DRMAAJ_ERRNO_NO_ACTIVE_SESSION, /* Exit routine failed because there is no
                                      active session */
   DRMAAJ_ERRNO_NO_MEMORY, /* failed allocating memory */

   /* -------------- init and exit specific --------------- */
   DRMAAJ_ERRNO_INVALID_CONTACT_STRING, /* Initialization failed due to invalid
                                           contact string. */
   DRMAAJ_ERRNO_DEFAULT_CONTACT_STRING_ERROR, /* DRMAA could not use the default
                                                 contact string to connect to
                                                 DRM system. */
   DRMAAJ_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED, /* No defaults contact
                                                       string was provided or
                                                       selected. DRMAA requires
                                                       that the default contact
                                                       string is selected when
                                                       there is more than one
                                                       default contact string
                                                       due to multiple DRMAA
                                                       implementation contained
                                                       in the binary module. */
   DRMAAJ_ERRNO_DRMS_INIT_FAILED, /* Initialization failed due to failure to
                                     init DRM system. */
   DRMAAJ_ERRNO_ALREADY_ACTIVE_SESSION, /* Initialization failed due to existing
                                           DRMAA session. */
   DRMAAJ_ERRNO_DRMS_EXIT_ERROR, /* DRM system disengagement failed. */

   /* ---------------- job attributes specific -------------- */
   DRMAAJ_ERRNO_INVALID_ATTRIBUTE_FORMAT, /* The format for the job attribute
                                             value is invalid. */
   DRMAAJ_ERRNO_INVALID_ATTRIBUTE_VALUE, /* The value for the job attribute is
                                            invalid. */
   DRMAAJ_ERRNO_CONFLICTING_ATTRIBUTE_VALUES, /* The value of this attribute is
                                                 conflicting with a previously
                                                 set attributes. */

   /* --------------------- job submission specific -------------- */
   DRMAAJ_ERRNO_TRY_LATER, /* Could not pass job now to DRM system. A retry may
                              succeed however (saturation). */
   DRMAAJ_ERRNO_DENIED_BY_DRM, /* The DRM system rejected the job. The job will
                                  never be accepted due to DRM configuration or
                                  job template settings. */

   /* ------------------------------- job control specific ---------------- */
   DRMAAJ_ERRNO_INVALID_JOB, /* The job specified by the 'jobid' does not
                                exist. */
   DRMAAJ_ERRNO_RESUME_INCONSISTENT_STATE, /* The job has not been suspended.
                                              The RESUME request will not be
                                              processed. */
   DRMAAJ_ERRNO_SUSPEND_INCONSISTENT_STATE, /* The job has not been running, and
                                               it cannot be suspended. */
   DRMAAJ_ERRNO_HOLD_INCONSISTENT_STATE, /* The job cannot be moved to a HOLD
                                           state. */
   DRMAAJ_ERRNO_RELEASE_INCONSISTENT_STATE, /* The job is not in a HOLD
                                               state. */
   DRMAAJ_ERRNO_EXIT_TIMEOUT, /* We have encountered a time-out condition for
                                 drmaa_synchronize or drmaa_wait. */
   DRMAAJ_ERRNO_NO_RUSAGE, /* This error code is returned by drmaa_wait() when a
                              job has finished but no rusage and stat data could
                              be provided. */
   DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE, /* This error code is returned when an
                                         invalid job template is passed to a
                                         function. */
   DRMAAJ_ERRNO_NULL_POINTER, /* This error code is used for
                                 NullPointerExceptions */
/* DRMAAJ_ERRNO_NO_MORE_ELEMENTS is not listed here because it is unused in the
 * Java language binding. */
   DRMAAJ_NO_ERRNO
};

#define NO_EXECEPTION_CLASS "Unable to locate class, %s, for DRMAA error: %s: %s"

static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
static drmaa_job_template_t **job_templates = NULL;
static int list_length = 0;

static void print_message_and_throw_exception(JNIEnv *env, int errnum,
                                              const char *format, ...);
static void throw_exception (JNIEnv *env, int errnum, const char *message);
static jclass get_exception_class(JNIEnv *env, int errnum, const char *message);
static char *get_exception_class_name (int errnum);
static drmaa_job_template_t *get_from_list (int id);
static int insert_into_list (drmaa_job_template_t *jt);

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeControl
  (JNIEnv *env, jobject object, jstring jobId, jint action)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   const char *job_id = NULL;
   
   if (jobId == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S, "job id");
      
      return;
   }
   
   job_id = (*env)->GetStringUTFChars(env, jobId, NULL);

   errnum = drmaa_control (job_id, action, error, DRMAA_ERROR_STRING_BUFFER);

   (*env)->ReleaseStringUTFChars(env, jobId, job_id);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   }  
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeExit
  (JNIEnv *env, jobject object)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   int count = 0;
   
   /* Free all job templates */
   pthread_mutex_lock(&list_mutex);
   
   for (count = 0; count < list_length; count++) {
      if (job_templates[count] != NULL) {
         errnum = drmaa_delete_job_template(job_templates[count], error,
                                            DRMAA_ERROR_STRING_BUFFER);
         
         if (errnum != DRMAAJ_ERRNO_SUCCESS) {
            pthread_mutex_unlock(&list_mutex);
            
            throw_exception(env, errnum, error);
            
            return;
         }
         
         job_templates[count] = NULL;
      }
   }
   
   pthread_mutex_unlock(&list_mutex);
   
   errnum = drmaa_exit (error, DRMAA_ERROR_STRING_BUFFER);

   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   }  
}

JNIEXPORT jstring JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeGetContact
  (JNIEnv *env, jobject object)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char contact[DRMAA_CONTACT_BUFFER + 1];
   
   errnum = drmaa_get_contact (contact, DRMAA_CONTACT_BUFFER, error,
                              DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      
      return NULL;
   }
   
   return (*env)->NewStringUTF (env, contact);
}

JNIEXPORT jstring JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeGetDRMSInfo
  (JNIEnv *env, jobject object)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char system[DRMAA_DRM_SYSTEM_BUFFER + 1];
   
   errnum = drmaa_get_DRM_system (system, DRMAA_DRM_SYSTEM_BUFFER, error,
                                 DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      
      return NULL;
   }
   
   return (*env)->NewStringUTF (env, system);
}

JNIEXPORT jint JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeGetJobProgramStatus
  (JNIEnv *env, jobject object, jstring jobId)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   int status = 0;
   const char *job_id = NULL;
   
   if (jobId == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S, "job id");
      
      return -1;
   }
   
   job_id = (*env)->GetStringUTFChars(env, jobId, NULL);

   errnum = drmaa_job_ps (job_id, &status, error, DRMAA_ERROR_STRING_BUFFER);

   (*env)->ReleaseStringUTFChars(env, jobId, job_id);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);

      return -1;
   }
  
   return status;
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeInit
  (JNIEnv *env, jobject object, jstring contactString)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   const char *contact = NULL;

   if (contactString != NULL) {
      contact = (*env)->GetStringUTFChars(env, contactString, NULL);
   }

   errnum = drmaa_init (contact, error, DRMAA_ERROR_STRING_BUFFER);

   if (contactString != NULL) {
      (*env)->ReleaseStringUTFChars(env, contactString, contact);
   }
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   }  
}

JNIEXPORT jobjectArray JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeRunBulkJobs
  (JNIEnv *env, jobject object, jint id, jint start, jint end, jint step)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char buffer[DRMAA_JOBNAME_BUFFER + 1];
   drmaa_job_template_t *jt = NULL;
   drmaa_job_ids_t *ids = NULL;
   int num_elem = 0;
   int count = 0;
   jobjectArray ret_val = NULL;
   jclass clazz = NULL;
   jstring tmp_str = NULL;
   
   jt = get_from_list(id);
   
   if (jt == NULL) {
      print_message_and_throw_exception(env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                        MSG_JDRMAA_BAD_JOB_TEMPLATE);

      return NULL;
   }
   
   errnum = drmaa_run_bulk_jobs(&ids, jt, start, end, step, error,
                                DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception(env, errnum, error);
      drmaa_release_job_ids (ids);
      
      return NULL;
   }

   errnum = drmaa_get_num_job_ids(ids, &num_elem);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception(env, errnum, NULL);
      drmaa_release_job_ids (ids);
      
      return NULL;
   }
   
   clazz = (*env)->FindClass (env, "java/lang/String");
   ret_val = (*env)->NewObjectArray(env, num_elem, clazz, NULL);

   for (count = 0; count < num_elem; count++) {
      errnum = drmaa_get_next_job_id(ids, buffer, DRMAA_JOBNAME_BUFFER);
      
      if (errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception(env, errnum, "Reported incorrect number of job ids");
         drmaa_release_job_ids (ids);

         return NULL;
      }
      
      tmp_str = (*env)->NewStringUTF (env, buffer);
      (*env)->SetObjectArrayElement(env, ret_val, count, tmp_str);
   }

   drmaa_release_job_ids(ids);
   ids = NULL;
   
   return ret_val;
}

JNIEXPORT jstring JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeRunJob
  (JNIEnv *env, jobject object, jint id)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char job_id[DRMAA_JOBNAME_BUFFER + 1];
   drmaa_job_template_t *jt = NULL;
   
   jt = get_from_list (id);
   
   if (jt == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                         MSG_JDRMAA_BAD_JOB_TEMPLATE);

      return NULL;
   }
   
   errnum = drmaa_run_job (job_id, DRMAA_JOBNAME_BUFFER, jt, error,
                          DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      
      return NULL;
   }
   
   return (*env)->NewStringUTF (env, job_id);
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeSynchronize
  (JNIEnv *env, jobject object, jobjectArray ids, jlong timeout,
   jboolean dispose)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   const char **job_ids = NULL;
   jsize length = 0;
   jobject tmp_obj = NULL;
   jsize count = 0;

   if (ids == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "job ids list");
      
      return;
   }
   
   length = (*env)->GetArrayLength(env, ids);
   job_ids = (const char**)malloc ((length + 1) * sizeof (char *));
   
   for (count = 0; count < length; count++) {
      tmp_obj = (*env)->GetObjectArrayElement(env, ids, count);
      job_ids[count] = (*env)->GetStringUTFChars(env, (jstring)tmp_obj, NULL);
   }
   
   job_ids[count] = NULL;
   
   errnum = drmaa_synchronize (job_ids, (signed long)timeout, dispose, error,
                              DRMAA_ERROR_STRING_BUFFER);
   
   for (count = 0; count < length; count++) {
      tmp_obj = (*env)->GetObjectArrayElement(env, ids, count);
      (*env)->ReleaseStringUTFChars(env, (jstring)tmp_obj, job_ids[count]);
   }
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   }
}

JNIEXPORT jobject JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeWait
  (JNIEnv *env, jobject object, jstring jobId, jlong timeout)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char buffer[DRMAA_JOBNAME_BUFFER + 1];
   char rbuffer[BUFFER_LENGTH + 1];
   char signal[DRMAA_SIGNAL_BUFFER + 1];
   const char *job_id = NULL;
   jobject job_info = NULL;
   jmethodID meth = NULL;
   jclass clazz = NULL;
   jobjectArray resources = NULL;
   int status = -1;
   drmaa_attr_values_t *rusage = NULL;
   jstring tmp_str = NULL;
   int signaled = 0;
   int count = 0;
   int length = 0;
   int has_resources = 1;
   
   if (jobId == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S, "job id");
      
      return NULL;
   }
   
   job_id = (*env)->GetStringUTFChars (env, jobId, NULL);
   
   errnum = drmaa_wait (job_id, buffer, DRMAA_JOBNAME_BUFFER, &status,
                        (signed long)timeout, &rusage, error,
                        DRMAA_ERROR_STRING_BUFFER);
   (*env)->ReleaseStringUTFChars (env, jobId, job_id);

   if (errnum == DRMAAJ_ERRNO_NO_RUSAGE) {
       has_resources = 0;
   } else if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      drmaa_release_attr_values (rusage);   
   
      return NULL;
   }

   if (has_resources == 1) {
       errnum = drmaa_get_num_attr_values(rusage, &length);

       if (errnum != DRMAAJ_ERRNO_SUCCESS) {
          throw_exception(env, errnum, NULL);
          drmaa_release_attr_values (rusage);   

          return NULL;
       }

       clazz = (*env)->FindClass (env, "java/lang/String");
       resources = (*env)->NewObjectArray(env, length, clazz, NULL);

       for (count = 0; count < length; count++) {
          errnum = drmaa_get_next_attr_value (rusage, rbuffer, BUFFER_LENGTH);

          if (errnum != DRMAAJ_ERRNO_SUCCESS) {
             throw_exception (env, errnum, "Reported incorrect number of resource usage entries");
             drmaa_release_attr_values (rusage);   

             return NULL;
          }

          tmp_str = (*env)->NewStringUTF (env, rbuffer);
          (*env)->SetObjectArrayElement(env, resources, count, tmp_str);
       }

       drmaa_release_attr_values (rusage);
   }

   errnum = drmaa_wifsignaled (&signaled, status, error,
                              DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   
      return NULL;
   }
   else if (signaled != 0) {
      errnum = drmaa_wtermsig (signal, DRMAA_SIGNAL_BUFFER, status, error,
                              DRMAA_ERROR_STRING_BUFFER);

      if (errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception (env, errnum, error);

         return NULL;
      }
      
      tmp_str = (*env)->NewStringUTF (env, signal);
   }
   
   clazz = (*env)->FindClass (env, "com/sun/grid/drmaa/JobInfoImpl");
   meth = (*env)->GetMethodID (env, clazz, "<init>",
                 "(Ljava/lang/String;I[Ljava/lang/String;Ljava/lang/String;)V");
   job_info = (*env)->NewObject (env, clazz, meth,
                                 (*env)->NewStringUTF (env, buffer), status,
                                 resources, tmp_str);
   
   return job_info;
}

JNIEXPORT jint JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeAllocateJobTemplate
  (JNIEnv *env, jobject object)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   drmaa_job_template_t *jt = NULL;
   
   errnum = drmaa_allocate_job_template(&jt, error, DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      return -1;
   }
   
   return insert_into_list (jt);
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeSetAttributeValue
  (JNIEnv *env, jobject object, jint id, jstring nameStr, jstring valueStr)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   drmaa_job_template_t *jt = get_from_list (id);
   const char *name = NULL;
   const char *value = NULL;
   
   if (jt == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                         MSG_JDRMAA_BAD_JOB_TEMPLATE);
      
      return;
   }
   
   if (nameStr == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "attribute name");
      
      return;
   }
   
   if (valueStr == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "attribute value");
      
      return;
   }
   
   name = (*env)->GetStringUTFChars (env, nameStr, NULL);
   value = (*env)->GetStringUTFChars (env, valueStr, NULL);
   
   errnum = drmaa_set_attribute (jt, name, value, error,
                                DRMAA_ERROR_STRING_BUFFER);

   (*env)->ReleaseStringUTFChars (env, nameStr, name);
   (*env)->ReleaseStringUTFChars (env, valueStr, value);

   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   
      return;
   }
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeSetAttributeValues
  (JNIEnv *env, jobject object, jint id, jstring nameStr, jobjectArray values)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   drmaa_job_template_t *jt = NULL;
   const char *name = NULL;
   const char **value = NULL;
   jsize length = 0;
   jobject tmp_obj = NULL;
   jsize count = 0;
   
   jt = get_from_list (id);
   
   if (jt == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                         MSG_JDRMAA_BAD_JOB_TEMPLATE);

      return;
   }

   if (nameStr == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "attribute name");
      
      return;
   }

   if (values == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "attribute names list");
      
      return;
   }
   
   length = (*env)->GetArrayLength(env, values);
   
   /* Get the strings out of the Strings. */
   name = (*env)->GetStringUTFChars (env, nameStr, NULL);
   value = (const char**)malloc ((length + 1) * sizeof (char *));
   
   for (count = 0; count < length; count++) {
      tmp_obj = (*env)->GetObjectArrayElement(env, values, count);
      value[count] = (*env)->GetStringUTFChars(env, (jstring)tmp_obj, NULL);
   }

   value[count] = NULL;
   
   errnum = drmaa_set_vector_attribute (jt, name, value, error,
                                       DRMAA_ERROR_STRING_BUFFER);

   /* Release the strings. */
   (*env)->ReleaseStringUTFChars (env, nameStr, name);

   for (count = 0; count < length; count++) {
      tmp_obj = (*env)->GetObjectArrayElement(env, values, count);
      (*env)->ReleaseStringUTFChars(env, (jstring)tmp_obj, value[count]);
   }

   free (value);
   value = NULL;
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   }
}

JNIEXPORT jobjectArray JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeGetAttributeNames
  (JNIEnv *env, jobject object, jint id)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   char buffer[BUFFER_LENGTH + 1];
   jobjectArray retval = NULL;
   jclass clazz = NULL;
   jstring tmp_str = NULL;
   drmaa_attr_names_t *names = NULL;
   drmaa_attr_names_t *vnames = NULL;
   int size = 0;
   int vsize = 0;
   int count = 0;
   
   errnum = drmaa_get_attribute_names (&names, error,
                                       DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
   
      return NULL;
   }
   
   errnum = drmaa_get_vector_attribute_names (&vnames, error,
                                             DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, error);
      drmaa_release_attr_names (names);
   
      return NULL;
   }
   
   errnum = drmaa_get_num_attr_names (names, &size);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, NULL);
      drmaa_release_attr_names (names);
      drmaa_release_attr_names (vnames);
   
      return NULL;
   }
   
   errnum = drmaa_get_num_attr_names (vnames, &vsize);
   
   if (errnum != DRMAAJ_ERRNO_SUCCESS) {
      throw_exception (env, errnum, NULL);
      drmaa_release_attr_names (names);
      drmaa_release_attr_names (vnames);
   
      return NULL;
   }
   
   clazz = (*env)->FindClass (env, "java/lang/String");
   retval = (*env)->NewObjectArray(env, size + vsize, clazz, NULL);

   for (count = 0; count < size; count++) {
      errnum = drmaa_get_next_attr_name(names, buffer, BUFFER_LENGTH);
      
      if (errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception (env, errnum, "Reported incorrect number of attribute names");
         drmaa_release_attr_names (names);
         drmaa_release_attr_names (vnames);

         return NULL;
      }

      tmp_str = (*env)->NewStringUTF (env, buffer);
      (*env)->SetObjectArrayElement(env, retval, count, tmp_str);
   }
   
   drmaa_release_attr_names (names);
   
   for (count = 0; count < vsize; count++) {
      errnum = drmaa_get_next_attr_name(vnames, buffer, BUFFER_LENGTH);
      
      if (errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception (env, errnum, "Reported incorrect number of attribute names");
         drmaa_release_attr_names (vnames);

         return NULL;
      }

      tmp_str = (*env)->NewStringUTF (env, buffer);
      (*env)->SetObjectArrayElement(env, retval, count + size, tmp_str);
   }
   
   drmaa_release_attr_names (vnames);
   
   return retval;
}

JNIEXPORT jobjectArray JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeGetAttribute
  (JNIEnv *env, jobject object, jint id, jstring name)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   drmaa_job_template_t *jt = get_from_list (id);
   jobjectArray retval = NULL;
   drmaa_attr_names_t *names = NULL;
   drmaa_attr_values_t *values = NULL;
   char buffer[BUFFER_LENGTH + 1];
   const char *name_str = NULL;
   bool is_vector = false;
   jclass clazz = NULL;
   jstring tmp_str = NULL;
   
   if (jt == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                         MSG_JDRMAA_BAD_JOB_TEMPLATE);

      return NULL;
   }

   if (name == NULL) {
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_NULL_POINTER,
                                         MSG_JDRMAA_NULL_POINTER_S,
                                         "attribute name");
      
      return NULL;
   }
   
   name_str = (*env)->GetStringUTFChars(env, name, NULL);
   
   errnum = drmaa_get_vector_attribute_names(&names, error,
                                             DRMAA_ERROR_STRING_BUFFER);
   
   if (errnum == DRMAAJ_ERRNO_SUCCESS) {
      while (drmaa_get_next_attr_name(names, buffer, BUFFER_LENGTH)
                                                      == DRMAAJ_ERRNO_SUCCESS) {
         if (strcmp (buffer, name_str) == 0) {
            is_vector = true;
            break;
         }
      }
   }
   else {
      (*env)->ReleaseStringUTFChars(env, name, name_str);
      throw_exception (env, errnum, error);
   
      return NULL;
   }
   
   drmaa_release_attr_names (names);
   
   if (is_vector) {
      errnum = drmaa_get_vector_attribute (jt, name_str, &values, error,
                                           DRMAA_ERROR_STRING_BUFFER);
      (*env)->ReleaseStringUTFChars(env, name, name_str);

      if (errnum == DRMAAJ_ERRNO_INVALID_ATTRIBUTE_VALUE) {
         return NULL;
      }
      else if(errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception (env, errnum, error);

         return NULL;
      }
      else {
         int count = 0;
         int size = 0;
         
         errnum = drmaa_get_num_attr_values(values, &size);
         
         if (errnum != DRMAAJ_ERRNO_SUCCESS) {
            throw_exception(env, errnum, NULL);
            drmaa_release_attr_values(values);
            
            return NULL;
         }
         
         clazz = (*env)->FindClass (env, "java/lang/String");
         retval = (*env)->NewObjectArray(env, size, clazz, NULL);

         for (count = 0; count < size; count++) {
            errnum = drmaa_get_next_attr_value(values, buffer, BUFFER_LENGTH);
            
            if (errnum != DRMAAJ_ERRNO_SUCCESS) {
               throw_exception(env, errnum, "Reported incorrect number of attribute value elements");
               drmaa_release_attr_values(values);

               return NULL;
            }
            
            tmp_str = (*env)->NewStringUTF (env, buffer);
            (*env)->SetObjectArrayElement(env, retval, count, tmp_str);
         }

         drmaa_release_attr_values (values);
      }
   }
   else {
      errnum = drmaa_get_attribute (jt, name_str, buffer, BUFFER_LENGTH, error,
                                   DRMAA_ERROR_STRING_BUFFER);
      (*env)->ReleaseStringUTFChars(env, name, name_str);
      
      if (errnum == DRMAAJ_ERRNO_INVALID_ATTRIBUTE_VALUE) {
         return NULL;
      }
      else if(errnum != DRMAAJ_ERRNO_SUCCESS) {
         throw_exception (env, errnum, error);

         return NULL;
      }
      else {
         clazz = (*env)->FindClass (env, "java/lang/String");   
         retval = (*env)->NewObjectArray(env, 1, clazz, NULL);
         tmp_str = (*env)->NewStringUTF (env, buffer);
         (*env)->SetObjectArrayElement(env, retval, 0, tmp_str);
      }
   }
   
   return retval;
}

JNIEXPORT void JNICALL Java_com_sun_grid_drmaa_SessionImpl_nativeDeleteJobTemplate
  (JNIEnv *env, jobject object, jint id)
{
   char error[DRMAA_ERROR_STRING_BUFFER + 1];
   int errnum = DRMAAJ_ERRNO_SUCCESS;
   drmaa_job_template_t *jt = NULL;
   
   pthread_mutex_lock(&list_mutex);
      
   if ((job_templates != NULL) && (id < list_length)) {
      jt = job_templates[id];
   }
   
   if (jt != NULL) {
      errnum = drmaa_delete_job_template (jt, error, DRMAA_ERROR_STRING_BUFFER);
      
      if (errnum != DRMAAJ_ERRNO_SUCCESS) {
         pthread_mutex_unlock(&list_mutex);
         
         throw_exception (env, errnum, error);

         return;
      }
      
      job_templates[id] = NULL;
   
      pthread_mutex_unlock(&list_mutex);
   }
   else {   
      pthread_mutex_unlock(&list_mutex);
      
      print_message_and_throw_exception (env, DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE,
                                         MSG_JDRMAA_BAD_JOB_TEMPLATE);

      return;
   }
}

static void print_message_and_throw_exception(JNIEnv *env, int errnum,
                                              const char *format, ...)
{
   char message[MAX_STRING_SIZE + 1];

   if (format != NULL) {
      va_list ap;

      va_start(ap, format);
      vsnprintf(message, MAX_STRING_SIZE, format, ap);
      va_end(ap);
      throw_exception(env, errnum, message);
   } else {
      throw_exception(env, errnum, NULL);
   }
}

static void throw_exception(JNIEnv *env, int errnum, const char *message)
{
   const char *error = message;
   jclass newExcCls = NULL;

   if (error == NULL) {
      error = drmaa_strerror(errnum);
   }

   newExcCls = get_exception_class(env, errnum, error);

   if (newExcCls != NULL) {
      (*env)->ThrowNew(env, newExcCls, error);
   }
}

static jclass get_exception_class(JNIEnv *env, int errnum, const char *message)
{
   jclass newExcCls = NULL;

   newExcCls = (*env)->FindClass(env, get_exception_class_name(errnum));

   /* If we can't find the exception class, throw a RuntimeException. */
   if (newExcCls == NULL) {
      char no_class_message[MAX_STRING_SIZE];

      /* If we can't find the right exception, default to something we
       * really expect to be able to find. */
      jclass runtime = (*env)->FindClass(env,
                                         "java/lang/ClassNotFoundException");

      /* If it's still not found, give up. */
      if (runtime == NULL) {
         fprintf (stderr, NO_EXECEPTION_CLASS,
                  get_exception_class_name (errnum), drmaa_strerror (errnum),
                  message);

         /* This if-else structure should now dump the thread of control out at
          * the end of the method.  Not doing so is an error. */
      }
      /* Otherwise, throw the Runtime exception. */
      else {
         snprintf (no_class_message, MAX_STRING_SIZE, NO_EXECEPTION_CLASS,
                   get_exception_class_name (errnum), drmaa_strerror (errnum),
                   message);

         /* Throw an exception saying we couldn't find the exception. */
         (*env)->ThrowNew(env, runtime, no_class_message);
      }
   }

   return newExcCls;
}

static char *get_exception_class_name (int errnum)
{
   switch (errnum) {
      case DRMAAJ_ERRNO_INTERNAL_ERROR:
         return "org/ggf/drmaa/InternalException";
      case DRMAAJ_ERRNO_DRM_COMMUNICATION_FAILURE:
         return "org/ggf/drmaa/DrmCommunicationException";
      case DRMAAJ_ERRNO_AUTH_FAILURE:
         return "org/ggf/drmaa/AuthorizationException";
      case DRMAAJ_ERRNO_INVALID_ARGUMENT:
         return "java/lang/IllegalArgumentException";
      case DRMAAJ_ERRNO_NO_ACTIVE_SESSION:
         return "org/ggf/drmaa/NoActiveSessionException";
      case DRMAAJ_ERRNO_NO_MEMORY:
         return "java/lang/OutOfMemoryError";
      case DRMAAJ_ERRNO_INVALID_CONTACT_STRING:
         return "org/ggf/drmaa/InvalidContactStringException";
      case DRMAAJ_ERRNO_DEFAULT_CONTACT_STRING_ERROR:
         return "org/ggf/drmaa/DefaultContactStringException";
      case DRMAAJ_ERRNO_NO_DEFAULT_CONTACT_STRING_SELECTED:
         return "org/ggf/drmaa/NoDefaultContactStringException";
      case DRMAAJ_ERRNO_DRMS_INIT_FAILED:
         return "org/ggf/drmaa/DrmsInitException";
      case DRMAAJ_ERRNO_ALREADY_ACTIVE_SESSION:
         return "org/ggf/drmaa/AlreadyActiveSessionException";
      case DRMAAJ_ERRNO_DRMS_EXIT_ERROR:
         return "org/ggf/drmaa/DrmsExitException";
      case DRMAAJ_ERRNO_INVALID_ATTRIBUTE_FORMAT:
         return "org/ggf/drmaa/InvalidAttributeFormatException";
      case DRMAAJ_ERRNO_INVALID_ATTRIBUTE_VALUE:
         return "org/ggf/drmaa/InvalidAttributeValueException";
      case DRMAAJ_ERRNO_CONFLICTING_ATTRIBUTE_VALUES:
         return "org/ggf/drmaa/ConflictingAttributeValuesException";
      case DRMAAJ_ERRNO_TRY_LATER:
         return "org/ggf/drmaa/TryLaterException";
      case DRMAAJ_ERRNO_DENIED_BY_DRM:
         return "org/ggf/drmaa/DeniedByDrmException";
      case DRMAAJ_ERRNO_INVALID_JOB:
         return "org/ggf/drmaa/InvalidJobException";
      case DRMAAJ_ERRNO_RESUME_INCONSISTENT_STATE:
         return "org/ggf/drmaa/ResumeInconsistentStateException";
      case DRMAAJ_ERRNO_SUSPEND_INCONSISTENT_STATE:
         return "org/ggf/drmaa/SuspendInconsistentStateException";
      case DRMAAJ_ERRNO_HOLD_INCONSISTENT_STATE:
         return "org/ggf/drmaa/HoldInconsistentStateException";
      case DRMAAJ_ERRNO_RELEASE_INCONSISTENT_STATE:
         return "org/ggf/drmaa/ReleaseInconsistentStateException";
      case DRMAAJ_ERRNO_EXIT_TIMEOUT:
         return "org/ggf/drmaa/ExitTimeoutException";
       case DRMAAJ_ERRNO_INVALID_JOB_TEMPLATE:
         return "org/ggf/drmaa/InvalidJobTemplateException";
      case DRMAAJ_ERRNO_NULL_POINTER:
         return "java/lang/NullPointerException";
      default:
         return "java/lang/RuntimeException";
   }
}

static int insert_into_list (drmaa_job_template_t *jt)
{
   int count = 0;
   drmaa_job_template_t **tmp_list = NULL;
   int tmp_length = 0;
   
   pthread_mutex_lock(&list_mutex);

   /* If we haven't initialized the template list yet, do so. */
   if (job_templates == NULL) {
      list_length = TEMPLATE_LIST_LENGTH;
      job_templates = (drmaa_job_template_t **)malloc
                                (sizeof (drmaa_job_template_t *) * list_length);
      memset (job_templates, 0, list_length * sizeof (drmaa_job_template_t *));
   }

   /* Search for an empty slot. */
   for (count = 0; count < list_length; count++) {
      if (job_templates[count] == NULL) {
         /* Insert the template and return the index. */
         job_templates[count] = jt;
   
         pthread_mutex_unlock(&list_mutex);

         return count;
      }
   }

   /* If there are no empty slots, double the size of the list. */
   tmp_length = list_length * 2;
   tmp_list = (drmaa_job_template_t **)malloc (sizeof(drmaa_job_template_t *) *
                                               tmp_length);
   memcpy (tmp_list, job_templates, list_length *
                                               sizeof (drmaa_job_template_t *));
   memset (&tmp_list[count], 0, list_length * sizeof (drmaa_job_template_t *));
   
   list_length = tmp_length;
   free (job_templates);
   job_templates = tmp_list;
   
   /* Insert the template and return the index. */
   job_templates[count] = jt;
   
   pthread_mutex_unlock(&list_mutex);
   
   return count;
}

static drmaa_job_template_t *get_from_list (int id)
{
   drmaa_job_template_t *retval = NULL;
   
   if (id >= 0) {
      pthread_mutex_lock(&list_mutex);

      if ((job_templates != NULL) && (id < list_length)) {
         retval = job_templates[id];
      }

      pthread_mutex_unlock(&list_mutex);
   }
   
   return retval;
}
