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
#include "SGEJobJNIUtil.hpp"

#include "StringValue.hpp"
#include "Variable.hpp"
#include "HostPath.hpp"
#include "utils.h"

#include <string.h>

extern char* strdup(const char*);

//============== constructors =============

SGEJobJNIUtil::SGEJobJNIUtil(JNIEnv* _env)
{
  env = _env;
}

//============== destructor ===============

SGEJobJNIUtil::~SGEJobJNIUtil()
{
}

//============== methods ==================

StringValue** SGEJobJNIUtil::getJobArgs(jobject jobObj)
{
  if(!jobObj)
    return 0;

  jobject stringListObj = callObjectMethod(env,
                                           jobObj,
                                           "getArgs",
                                           "()Lcom/sun/grid/jam/gridengine/util/StringList;");
  if(!stringListObj)
    return 0;
  
  return getStringValues(stringListObj);
}

Variable** SGEJobJNIUtil::getJobEnvVars(jobject jobObj)
{
  if(!jobObj)
    return 0;

  jobject variableListObj = callObjectMethod(env,
                                             jobObj,
                                             "getEnvVars",
                                             "()Lcom/sun/grid/jam/gridengine/util/VariableList;");
  if(!variableListObj)
    return 0;
  
  return getVariables(variableListObj);
}

HostPath** SGEJobJNIUtil::getJobStdoutPaths(jobject jobObj)
{
  if(!jobObj)
    return 0;

  jobject hostPathListObj = callObjectMethod(env,
                                             jobObj,
                                             "getStdoutPaths",
                                             "()Lcom/sun/grid/jam/gridengine/util/HostPathList;");
  if(!hostPathListObj)
    return 0;
  
  return getHostPaths(hostPathListObj);
}

HostPath** SGEJobJNIUtil::getJobStderrPaths(jobject jobObj)
{
  if(!jobObj)
    return 0;

  jobject hostPathListObj = callObjectMethod(env,
                                             jobObj,
                                             "getStderrPaths",
                                             "()Lcom/sun/grid/jam/gridengine/util/HostPathList;");
  if(!hostPathListObj)
    return 0;
  
  return getHostPaths(hostPathListObj);
}

//=============== private methods ==============

StringValue** SGEJobJNIUtil::getStringValues(jobject stringListObj)
{
  if(!stringListObj)
    return 0;

  int size = 0;
  size = callIntMethod(env, stringListObj, "size");
  if(size <= 0)
    return 0;
  
  jclass stringListClass = env->GetObjectClass(stringListObj);
  if(!stringListClass)
    return 0;
  
  jmethodID method = env->GetMethodID(stringListClass, "get", "(I)Ljava/lang/String;");
  if(!method)
    return 0;
  
  StringValue** stringsValues = new StringValue*[size];
  for(int i = 0; i <= size; i++)
    stringsValues[i] = 0;

  for(int i = 0; i < size; i++)
  {
    char* value;
    jstring js = (jstring)env->CallObjectMethod(stringListObj, method, i);
    char* tmp = (char *)env->GetStringUTFChars(js, 0);
    value = strdup(tmp);
    env->ReleaseStringUTFChars(js, tmp);
    stringsValues[i] = new StringValue(value);
//     printf("value %d:\n", i);
//     printf("\tvalue = %s\n", stringsValues[i]->getValue());
  }

  return stringsValues;
}

Variable** SGEJobJNIUtil::getVariables(jobject variableListObj)
{
  int size = 0;
  size = callIntMethod(env, variableListObj, "size");
  if(size <= 0)
    return 0;
  
  Variable** variables = new Variable*[size];
  for(int i = 0; i <= size; i++)
    variables[i] = 0;

  for(int i = 0; i < size; i++)
  {
    variables[i] = getVariable(variableListObj, i);
//     printf("variable %d:\n", i);
//     printf("\t%s = %s\n", variables[i]->getName(), variables[i]->getValue());
  }

  return variables;
}

Variable* SGEJobJNIUtil::getVariable(jobject variableListObj,
                                        int index)
{
  jclass variableListClass = env->GetObjectClass(variableListObj);
  if(!variableListClass)
    return 0;
  
  jmethodID method = env->GetMethodID(variableListClass, "get",
                                      "(I)Lcom/sun/grid/jam/gridengine/util/Variable;");
  if(!method)
    return 0;

  jobject variableObj =
    env->CallObjectMethod(variableListObj, method, index);
  if(!variableObj)
    return 0;
  
  char* name = callStringMethod(env, variableObj, "getName");
  char* value = callStringMethod(env, variableObj, "getValue");

  if(!name || !value)
    return 0;

  return new Variable(name, value);
}

HostPath** SGEJobJNIUtil::getHostPaths(jobject hostPathListObj)
{
  int size = 0;
  size = callIntMethod(env, hostPathListObj, "size");
  if(size <= 0)
    return 0;
  
  HostPath** hostPaths = new HostPath*[size];
  for(int i = 0; i <= size; i++)
    hostPaths[i] = 0;

  for(int i = 0; i < size; i++)
  {
    hostPaths[i] = getHostPath(hostPathListObj, i);
//     printf("path %d:\n", i);
//     printf("\t%s\n", hostPaths[i]->getPath());
  }

  return hostPaths;
}

HostPath* SGEJobJNIUtil::getHostPath(jobject hostPathListObj,
                                        int index)
{
  jclass hostPathListClass = env->GetObjectClass(hostPathListObj);
  if(!hostPathListClass)
    return 0;
  
  jmethodID method = env->GetMethodID(hostPathListClass, "get",
                                      "(I)Lcom/sun/grid/jam/gridengine/util/HostPath;");
  if(!method)
    return 0;
  
  jobject hostPathObj =
    env->CallObjectMethod(hostPathListObj, method, index);
  if(!hostPathObj)
    return 0;
  
  char* path = 0;
  char* host = 0;
  
  path = callStringMethod(env, hostPathObj, "getPath");
  host = callStringMethod(env, hostPathObj, "getHost");

  if(!path)
    return 0;
  
  return new HostPath(path, host);
}
