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
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include "utils.h"

#include <stdio.h>
#include <string.h>

/* SGE GDI */
#include "sge_answerL.h"
#include "sge_answer.h"

extern char* strdup(const char*);

#ifdef __cplusplus
extern "C" {
#endif

char* jstring_to_charptr(JNIEnv* env, jstring js)
{
  char* str = 0;
  
  if(js) {
    char* tmp = (char *)env->GetStringUTFChars(js, 0);
    str = strdup(tmp);
    env->ReleaseStringUTFChars(js, tmp);
  }
  return str;
}
  
int callIntMethod(JNIEnv* env,
                  jobject jo,
                  char* funName)
{
  int ret = -1;
  
  jclass jc = env->GetObjectClass(jo);
  if(jc) {  
    jmethodID method = env->GetMethodID(jc, funName, "()I");
    if(method) {
      ret = (int)env->CallIntMethod(jo, method);
    }
  }
  return ret;
}
  
char* callStringMethod(JNIEnv* env,
                       jobject jo,
                       char* funName)
{
  char* str;
  
  jclass jc = env->GetObjectClass(jo);
  if(jc) {
    jmethodID method = env->GetMethodID(jc, funName, "()Ljava/lang/String;");
    if(method) {
      jstring js = (jstring)env->CallObjectMethod(jo, method);
      str = jstring_to_charptr(env, js);
    }
  }
  return str;
}

jobject callObjectMethod(JNIEnv* env,
                         jobject jo,
                         char* methodName,
                         char* type)
{
  jobject obj = 0;
  
  jclass jc = env->GetObjectClass(jo);
  if(jc) {
    jmethodID method = env->GetMethodID(jc, methodName, type);
    if(method) 
      obj = env->CallObjectMethod(jo, method);
  }
  return obj;
}

int callSetIntMethod(JNIEnv* env,
                      jobject jo,
                      char* methodName,
                      int arg)
{
  jclass jc = env->GetObjectClass(jo);
  if(!jc)
    return 0;
  
  jmethodID mid = env->GetMethodID(jc, methodName, "(I)V");
  if(!mid)
    return 0;

  env->CallVoidMethod(jo, mid, arg);
  jthrowable exc = env->ExceptionOccurred();
  if(exc) {
    env->ExceptionDescribe();
    env->Throw(exc);
    return 0;
  }
  return 1;
}


long getLongField(JNIEnv* env,
                  jobject jo,
                  char* fieldName)
{
  long ret = -1;
  
  if(jo) {
    jclass jc = env->GetObjectClass(jo);
    if(jc) {
      jfieldID jf = env->GetFieldID(jc, fieldName, "J");
      if(jf)
        ret = (long)env->GetIntField(jo, jf);
    }
  }
  return ret;
}
  
int getIntField(JNIEnv* env,
                jobject jo,
                char* fieldName)
{
  int ret = -1;
  
  if(jo) {
    jclass jc = env->GetObjectClass(jo);
    if(jc) {
      jfieldID jf = env->GetFieldID(jc, fieldName, "I");
      if(jf)
        ret = (int)env->GetIntField(jo, jf);
    }
  }
  return ret;
}

char* getStringField(JNIEnv* env,
                     jobject jo,
                     char* fieldName)
{
  char* str = 0;
  if(jo) {
    jclass jc = env->GetObjectClass(jo);
    if(jc) {
      jfieldID jf = env->GetFieldID(jc, fieldName, "Ljava/lang/String;");
      if(jf) {
        jstring js = (jstring)env->GetObjectField(jo, jf);
        str =  jstring_to_charptr(env, js);
      }
    }
  }
  return str;
}

jobject getObjectField(JNIEnv* env,
                       jobject jo,
                       char* fieldName,
                       char* type)
{
  jobject obj = 0;
  
  if(jo) {
    jclass jc = env->GetObjectClass(jo);
    if(jc) {
      jfieldID jf = env->GetFieldID(jc, fieldName, type);
      if(jf) 
        obj = env->GetObjectField(jo, jf);
    }
  }
  return obj;
}

void throw_exception(JNIEnv *env, char *reason)
{
  if(env->ExceptionCheck() == JNI_TRUE) {
    env->ExceptionDescribe();
    env->ExceptionClear();
  }
  
  jclass newExcCls =
    env->FindClass("com/sun/grid/jam/gridengine/NativeSGEException");
  if(newExcCls != NULL) 
    env->ThrowNew(newExcCls, reason);
  else {
    newExcCls = env->FindClass("java/lang/NoClassDefFoundError");
    env->ThrowNew(newExcCls, "Exception Class not Found");
  }
}

void throw_communication_exception(JNIEnv *env, char *reason)
{
  if(env->ExceptionCheck() == JNI_TRUE) {
    env->ExceptionDescribe();
    env->ExceptionClear();
  }

  jclass newExcCls =
    env->FindClass("com/sun/grid/jam/gridengine/NativeSGECommException");
  if(newExcCls != NULL) 
    env->ThrowNew(newExcCls, reason);
  else {
    newExcCls = env->FindClass("java/lang/NoClassDefFoundError");
    env->ThrowNew(newExcCls, "Exception Class not Found");
  }
}
    
void check_answer_status(JNIEnv *env, lList *answer,
                         char *fun_name, char *error_header)
{
  char error_reason[256];
  const char *an_text;
  int an_status;

//  printf("\n--%s--: chacking sge answer status\n", fun_name);

  an_status = lGetUlong(lFirst(answer), AN_status);

  switch(an_status)
  {
  case STATUS_OK:
    /* everything was fine */ 
    break;
  case STATUS_NOQMASTER:
    /* failed to reach sge_qmaster */
    an_text = lGetString(lFirst(answer), AN_text);
    sprintf(error_reason, "%s %s", error_header, an_text);
    // print_answer_status(fun_name, an_status);
    // printf("--%s--: %s\n", fun_name, error_reason);
    throw_communication_exception(env, error_reason);
    break;    
  case STATUS_NOCOMMD:
    /* failed to reach commd */
    an_text = lGetString(lFirst(answer), AN_text);
    sprintf(error_reason, "%s %s", error_header, an_text);
    // print_answer_status(fun_name, an_status);
    // printf("--%s--: %s\n", fun_name, error_reason);
    throw_communication_exception(env, error_reason);
    break;
  default:
    /* any other error */
    an_text = lGetString(lFirst(answer), AN_text);
    sprintf(error_reason, "%s %s", error_header, an_text);
    // print_answer_status(fun_name, an_status);
    // printf("--%s--: %s\n", fun_name, error_reason);
    throw_exception(env, error_reason);
    break;
  }
//  printf("--%s--: done chacking sge answer status\n", fun_name);
}

void print_answer(lList *answer, char *header) 
{
  printf("************* %s ANSWER *************\n", header);
  
  lWriteListTo(answer, stdout);
  
  int an_status = lGetUlong(lFirst(answer), AN_status);
  print_answer_status(header, an_status);  
  if(an_status != STATUS_OK) 
    fprintf(stderr, "error: %s",
            lGetString(lFirst(answer), AN_text));
  else 
    fprintf(stdout, "%s", lGetString(lFirst(answer), AN_text));
  
  printf("************* %s END ANSWER *************\n", header); 
}

void print_answer_status(char *fun_name, int an_status)
{
  printf("--%s--: answer status : %s\n", fun_name, answer_statustostr(an_status));
}

char *answer_statustostr(int status)
{
  char *strstatus;
  
  switch(status)
  {
    /* everything was fine */ 
  case STATUS_OK:
    strstatus = "ok";
    break;
    /* semantic error */
  case STATUS_ESEMANTIC:
    strstatus = "semantic error";
    break;
    /* element does not exist OR it exists for a "add" request */
  case STATUS_EEXIST:
    strstatus = "element doesn't exist";
    break;
    /* unknown error occured */
  case STATUS_EUNKNOWN:  
    strstatus = "unknown error";
    break;
    /* command not implemented for target */
  case STATUS_ENOIMP:
    strstatus = "command not implemented for target";
    break;
    /* missing key field in case of add,del,mod */
  case STATUS_ENOKEY:
    strstatus = "missing key field";
    break;    
    /* syntax error parsing a_source field */
  case STATUS_ESYNTAX:
    strstatus = "syntax error";
    break;    
    /* operation denied to this host */
  case STATUS_EDENIED2HOST:
    strstatus = "operation denied on this host";
    break;    
    /* operation needs manager privileges */
  case STATUS_ENOMGR:
    strstatus = "operation needs manager privilages";
    break;    
    /* operation needs operator privileges */
  case STATUS_ENOOPR:
    strstatus = "operation needs operator privileges";
    break;    
    /* failed to reach sge_qmaster */
  case STATUS_NOQMASTER:
    strstatus = "failed to reach qmaster";
    break;    
    /* failed to reach commd */
  case STATUS_NOCOMMD:
    strstatus = "failed to reach commd";
    break;    
    /* disk operation failed */
  case STATUS_EDISK:
    strstatus = "disk operation failed";
    break;    
    /* can't resolve user */
  case STATUS_ENOSUCHUSER:
    strstatus = "can't resolve user";
    break;
    /* can't resolve group */
  case STATUS_NOSUCHGROUP:
    strstatus = "can't resolve group";
    break;    
    /* can't allocate memory */
  case STATUS_EMALLOC:
    strstatus = "can't allocate memory";
    break;    
    /* need to be owner for this operation */
  case STATUS_ENOTOWNER:
    strstatus = "need to be owner for this operation";
    break;    
    /* too few submit host licenses */
  case STATUS_ESUBHLIC:
    strstatus = "too few submit host licenses";
    break;    
    /* not allowed to do whatever you try */
  case STATUS_DENIED:
    strstatus = "not allowed to do whatever you try";
    break;
  }
  return strstatus;
}

#ifdef __cplusplus
}
#endif  

