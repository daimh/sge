<%
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
%>

<%
  java.beans.BeanInfo beanInfo = (java.beans.BeanInfo)params.get("beanInfo");
  Class beanClass = beanInfo.getBeanDescriptor().getBeanClass();
  String classname = beanClass.getName();
  String fullClassname = classname.replace('.','/');
  {
      int i = classname.lastIndexOf('.');
      if (i>0) {
          classname = classname.substring(i+1);
      }
  }
  classname = classname.replace('$', '_');
  
  // skip the inner EventFactory classes
  if (classname.startsWith("EventFactory_")) {
     System.out.println("Skipping " + classname);
     return;
  }
  
  java.beans.PropertyDescriptor[] props = beanInfo.getPropertyDescriptors();
  com.sun.grid.javaconv.CWrapperHelper ch = new com.sun.grid.javaconv.CWrapperHelper(beanInfo.getBeanDescriptor().getBeanClass());
%>
/*==== BEGIN  <%=fullClassname%> */

jclass <%=classname%>_find_class(JNIEnv *env, lList** alpp) {
   static jclass clazz = NULL;

   DENTER(BASIS_LAYER, "<%=classname%>_find_class");
   if (clazz == NULL) {
      clazz = find_class(env, "<%=fullClassname%>", alpp);
   }
   DRETURN(clazz);
}

<%
  // ---------------------------------------------------------------------------
  // ------------ CONSTRUCTORS -------------------------------------------------
  // ---------------------------------------------------------------------------
  for (String constructorName: ch.getConstructorNames()) { 
      java.lang.reflect.Constructor constructor = ch.getConstructor(constructorName);
      Class[] parameters = constructor.getParameterTypes();
%>
jgdi_result_t <%=classname%>_<%=constructorName%>(JNIEnv *env, jobject *obj <%
   
      for (int i = 0; i < parameters.length; i++) {
         if (String.class.equals(parameters[i])) {
            /* For strings we want a const char* not a jstring */
          %>, const char* p<%=i%> <%
         } else {
         %>, <%=ch.getCType(parameters[i])%> p<%=i%> <%
         }
      } // end of inner for
%>, lList **alpp) {
   jgdi_result_t ret = JGDI_SUCCESS;
   static jmethodID mid = NULL;
   jclass clazz = NULL;<%
   // For string parameters we need a local variable
   // which hold the jstring object
   for (int i = 0; i < parameters.length; i++) {
      if (String.class.equals(parameters[i])) {
   %>     jstring p<%=i%>_obj = NULL;<%
      }
   } // end of inner for   
   %>
   DENTER(BASIS_LAYER, "<%=classname%>_<%=constructorName%>");
      
   clazz = <%=classname%>_find_class(env, alpp);
   if (clazz == NULL) {
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   if (mid == NULL) {
      /* initialize the mid */
      mid = get_methodid(env, clazz, "<init>", "<%=ch.getSignature(constructor)%>", alpp);
      if (mid == NULL) {
         DRETURN(JGDI_ILLEGAL_STATE);
      }
   } <%
   // All string parameters comes a const char* 
   // we have to allocate the jstring object
   for (int i = 0; i < parameters.length; i++) {
      if (String.class.equals(parameters[i])) {
%>
   if ( p<%=i%> != NULL ) {
      p<%=i%>_obj = (*env)->NewStringUTF(env, p<%=i%> ); 
   }<%
      } // end of if
  } // end of inner for
%>   
   *obj = (*env)->NewObject(env, clazz, mid <%
      for (int i = 0; i < parameters.length; i++) {
         if (String.class.equals(parameters[i])) {
            // For all string parameter we pass the jstring object
            // to the method call
          %>, p<%=i%>_obj <%
         } else {
         %>, p<%=i%> <%
         }
      } // end of inner for
  %>);
   if (test_jni_error(env, "call of constructor failed", alpp)) {
      ret = JGDI_ILLEGAL_STATE;
   } 
   DRETURN(ret);
}
<%   
   } // end of for
   
  // ---------------------------------------------------------------------------
  // ------------ Static Fields ------------------------------------------------
  // ---------------------------------------------------------------------------
  for (String fieldName: ch.getStaticFieldNames()) {
    java.lang.reflect.Field field = ch.getStaticField(fieldName);
%>
jgdi_result_t <%=classname%>_static_<%=fieldName%>(JNIEnv *env, <%=ch.getCType(field.getType())%> *res, lList **alpp) {
   jgdi_result_t ret = JGDI_SUCCESS;
   jclass clazz = NULL;
   static jfieldID mid = NULL;
   /*static <%=ch.getCType(field.getType())%> field = <%=ch.getInitializer(field.getType())%>;*/
   
   DENTER(BASIS_LAYER, "<%=classname%>_static_<%=fieldName%>");

   if (env == NULL) {
      answer_list_add(alpp, "env is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   clazz = <%=classname%>_find_class(env, alpp);
   if (clazz == NULL) {
      answer_list_add(alpp, "class <%=fullClassname%> not found", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   if (mid == NULL) {
      mid = get_static_fieldid(env, clazz, "<%=field.getName()%>", "<%=ch.getSignature(field)%>", alpp);
      if (mid == NULL) {
         DRETURN(JGDI_ILLEGAL_STATE);
      }
   }   
   *res = (*env)-><%=ch.getStaticGetFieldMethod(field)%>(env, clazz, mid);
   if (test_jni_error(env, "<%=classname%>_static_<%=fieldName%> failed", alpp)) {
      ret = JGDI_ILLEGAL_STATE;
   }      
   DRETURN(ret);
}
<%    
 } // end of for
   
   
  // ---------------------------------------------------------------------------
  // ------------ Static METHODS -----------------------------------------------
  // ---------------------------------------------------------------------------
  for (String methodName: ch.getStaticMethodNames()) {
      java.lang.reflect.Method method = ch.getStaticMethod(methodName);
      Class[] parameters = method.getParameterTypes();
%>
jgdi_result_t <%=classname%>_static_<%=methodName%>(JNIEnv *env <%   
      for (int i = 0; i < parameters.length; i++) {         
         if (String.class.equals(parameters[i])) {
            /* For strings we want a const char* not a jstring */
          %>, const char* p<%=i%> <%
         } else {
         %>, <%=ch.getCType(parameters[i])%> p<%=i%> <%
         }
      }
      if (!Void.TYPE.equals(method.getReturnType())) {
         // Add a pointer to the result argument
         %>, <%=ch.getCType(method.getReturnType())%>* result<%
         if(method.getReturnType().isArray()) {
           %>, int* len<%  
         }
      }
    // And finally we need the answer list
%>, lList **alpp) {

   jgdi_result_t ret = JGDI_SUCCESS;
   static jmethodID mid = NULL; 
   static jclass clazz = NULL;
<%
  // For string parameters we need a local variable
  // which hold the jstring object
  for(int i = 0; i < parameters.length; i++) {
     if( String.class.equals(parameters[i])) {
%>   jstring p<%=i%>_obj = NULL;<%
     }
  }
  // For non void methods we temporary store the result
  // of the java method in a local variable
  if (!Void.TYPE.equals(method.getReturnType())) {
     if (method.getReturnType().isArray()) { %>
   jobject temp = NULL;    
<%} else {
%>
   <%=ch.getCType(method.getReturnType())%> temp = <%=ch.getInitializer(method.getReturnType())%>;
<%
    }
  } 
%>
   DENTER(BASIS_LAYER, "<%=classname%>_static_<%=methodName%>");
<%   
  if (!Void.TYPE.equals(method.getReturnType())) {
%>         
   if (result == NULL ) {
      answer_list_add(alpp, "result is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   /* We set the result always to the default value */
<%   
   if(method.getReturnType().isArray()) { %>   *result = NULL;    
<%   } else {
%>   *result = <%=ch.getInitializer(method.getReturnType())%>;
<%    }
  }  %>   
  if (mid == NULL) {
     if (JGDI_SUCCESS != get_static_method_id_for_fullClassname(env, &clazz, &mid, "<%=fullClassname%>", "<%=method.getName()%>", "<%=ch.getSignature(method)%>", alpp)) {
        DRETURN(JGDI_ILLEGAL_STATE);
     }
   }<%
   
   // All string parameters comes a const char* 
   // we have to allocate the jstring object
   for (int i = 0; i < parameters.length; i++) {
      if (String.class.equals(parameters[i])) {
%>   if (p<%=i%> != NULL) {
      p<%=i%>_obj = (*env)->NewStringUTF(env, p<%=i%>); 
   }<%
      } // end of if
  } // end of for
      
  if (!Void.TYPE.equals(method.getReturnType())) {
%>   temp =<%
  } else { %>
  <%
  }
%> 
 (*env)-><%=ch.getStaticCallMethod(method.getReturnType())%>(env, clazz, mid <%
      /* Add all parameter to the method call */
      for (int i = 0; i < parameters.length; i++) {  
         if (String.class.equals(parameters[i])) {
            // For all string parameter we pass the jstring object
            // to the method call
          %>, p<%=i%>_obj <%
         } else {
         %>, p<%=i%> <%
         }
      }
  %>);  
   if (test_jni_error(env, "<%=classname%>_<%=methodName%> failed", alpp)) {
      ret = JGDI_ILLEGAL_STATE;
<%  if (!Void.TYPE.equals(method.getReturnType())) {
       if (method.getReturnType().isArray()) {
%>      temp = NULL;
<%     } else { %>      temp = <%=ch.getInitializer(method.getReturnType())%>;        
<%       }    
    }
%>      
   } <%
  if (!Void.TYPE.equals(method.getReturnType())) {
     // for non void method store the temporary result
     // in the result pointer
  %> else {
  <%
     if(method.getReturnType().isArray()) {
         
       Class realType = method.getReturnType().getComponentType();
       String realCType = ch.getCType(realType);
  %>
       if (temp == NULL) {
          *result = NULL;
          *len = 0;
       } else {
           jint arrayLen = (*env)->GetArrayLength(env, (jarray)temp);
           if (test_jni_error(env, "Can not get the array length of the result", alpp)) {
              ret = JGDI_ILLEGAL_STATE;
           } if (arrayLen > 0) {
               <%=realCType%> *array = NULL;
               int i = 0;
               <% if(realType.isPrimitive()) { %>
               jboolean isCopy = false;
               <%=realCType%> *tmpArray = NULL;
               
               tmpArray = (*env)-><%=ch.getGetArrayElementsMethod(realType)%>(env, (<%=realCType%>Array)temp, &isCopy);
               if (test_jni_error(env, "Can not get the array elements of the result", alpp)) {
                  ret = JGDI_ILLEGAL_STATE;
               } else {
                  array = (<%=realCType%>*)malloc(sizeof(<%=realCType%>)* arrayLen);
                  for(i = 0; i < arrayLen; i++) {
                     array[i] = tmpArray[i];
                  }
                  (*env)-><%=ch.getReleaseArrayElementsMethod(realType)%>(env, (<%=realCType%>Array)temp, tmpArray, JNI_ABORT);
                  if (test_jni_error(env, "release the array elements of the result failed", alpp)) {
                     sge_free(&array);
                  } else {
                     *result = array;
                     *len = arrayLen;
                  }
               }
               <% } else { %>
               array = (<%=realCType%>*)malloc(sizeof(<%=realCType%>)* arrayLen);
               for(i = 0; i < arrayLen; i++) {
                  array[i] = (*env)->GetObjectArrayElement(env, (jobjectArray)temp, i); 
                  if (test_jni_error(env, "Can not get object from array", alpp)) {
                     sge_free(&array);
                     array = NULL;
                     break;
                  }
               }
               if(array != NULL) {
                  *result = array;
                  *len = arrayLen;
               }
               <% } %>
          } else {
             *result = NULL;
             *len = 0;
          }
       }
<%     } else { %>
      *result = temp;
<%   } %>      
   }
<% } %>
   DRETURN(ret);
}<%
   } // end of for
   
   
  // ---------------------------------------------------------------------------
  // ------------ METHODS ------------------------------------------------------
  // ---------------------------------------------------------------------------
  for (String methodName: ch.getMethodNames()) {
      java.lang.reflect.Method method = ch.getMethod(methodName);
      Class[] parameters = method.getParameterTypes();
%>
jgdi_result_t <%=classname%>_<%=methodName%>(JNIEnv *env, <%=ch.getCType(beanClass)%> obj <%
   
      for (int i = 0; i < parameters.length; i++) {         
         if (String.class.equals(parameters[i])) {
            /* For strings we want a const char* not a jstring */
          %>, const char* p<%=i%> <%
         } else {
         %>, <%=ch.getCType(parameters[i])%> p<%=i%> <%
         }
      }
      if (!Void.TYPE.equals(method.getReturnType())) {
         // Add a pointer to the result argument
         %>, <%=ch.getCType(method.getReturnType())%>* result<%
         if(method.getReturnType().isArray()) {
           %>, int* len<%  
         }
      }
    // And finally we need the answer lsit
%>, lList **alpp) {
   jgdi_result_t ret = JGDI_SUCCESS;
   static jmethodID mid = NULL;
<%   
  // For string parameters we need a local variable
  // which hold the jstring object
  for (int i = 0; i < parameters.length; i++) {
     if (String.class.equals(parameters[i])) {
%>
   jstring p<%=i%>_obj = NULL;<%
     }
  }
  // For non void methods we temporary store the result
  // of the java method in a local variable
  if (!Void.TYPE.equals(method.getReturnType())) {
     if (method.getReturnType().isArray()) { %>
   jobject temp = NULL;    
<%   } else {
%>
   <%=ch.getCType(method.getReturnType())%> temp = <%=ch.getInitializer(method.getReturnType())%>;
<%
    }
  } 
%>
   DENTER(BASIS_LAYER, "<%=classname%>_<%=methodName%>");
<%   
  if (!Void.TYPE.equals(method.getReturnType())) {
%>         
   if (result == NULL ) {
      answer_list_add(alpp, "result is NULL", STATUS_EUNKNOWN, ANSWER_QUALITY_ERROR);
      DRETURN(JGDI_ILLEGAL_STATE);
   }
   /* We set the result always to the default value */
<% 
   if (method.getReturnType().isArray()) { %>   *result = NULL;    
<%   } else { 
%>   *result = <%=ch.getInitializer(method.getReturnType())%>;
<%
    }
  }%>   
  if (mid == NULL) {
     if (JGDI_SUCCESS != get_method_id_for_fullClassname(env, obj, &mid, "<%=fullClassname%>", "<%=method.getName()%>", "<%=ch.getSignature(method)%>", alpp)) {
        DRETURN(JGDI_ILLEGAL_STATE);
     }
   }
<%
   // All string parameters comes a const char* 
   // we have to allocate the jstring object
   for (int i = 0; i < parameters.length; i++) {
      if (String.class.equals(parameters[i])) {
%>
   if (p<%=i%> != NULL) {
      p<%=i%>_obj = (*env)->NewStringUTF(env, p<%=i%> ); 
   }<%
      } // end of if
  } // end of for
      
  if (!Void.TYPE.equals(method.getReturnType())) {
%>
   temp =<%
  } else { %>
  <%
  }
%> (*env)-><%=ch.getCallMethod(method.getReturnType())%>(env, obj, mid<%
      // Add all parameter to the method call 
      for (int i = 0; i < parameters.length; i++) {  
         if (String.class.equals(parameters[i])) {
            // For all string parameter we pass the jstring object
            // to the method call
          %>, p<%=i%>_obj<%
         } else {
         %>, p<%=i%><%
         }
      }
  %>);  
   if (test_jni_error(env, "<%=classname%>_<%=methodName%> failed", alpp)) {
      ret = JGDI_ILLEGAL_STATE;
<%  if (!Void.TYPE.equals(method.getReturnType())) {
       if (method.getReturnType().isArray()) {
%>      temp = NULL;
<%     } else { %>      temp = <%=ch.getInitializer(method.getReturnType())%>;        
<%       }    
    }
%>      
   } <%
  if (!Void.TYPE.equals(method.getReturnType())) {
     // for non void method store the temporary result
     // in the result pointer
     if (method.getReturnType().isArray()) {
       Class realType = method.getReturnType().getComponentType();
       String realCType = ch.getCType(realType);
  %>
       if (temp == NULL) {
          *result = NULL;
          *len = 0;
       } else {
           jint arrayLen = (*env)->GetArrayLength(env, (jarray)temp);
           if (test_jni_error(env, "Can not get the array length of the result", alpp)) {
              ret = JGDI_ILLEGAL_STATE;
           } if (arrayLen > 0) {
               <%=realCType%> *array = NULL;
               int i = 0;
               <% if(realType.isPrimitive()) { %>
               jboolean isCopy = false;
               <%=realCType%> *tmpArray = NULL;
               tmpArray = (*env)-><%=ch.getGetArrayElementsMethod(realType)%>(env, (<%=realCType%>Array)temp, &isCopy);
               if (test_jni_error(env, "Can not get the array elements of the result", alpp)) {
                  ret = JGDI_ILLEGAL_STATE;
               } else {
                  array = (<%=realCType%>*)malloc(sizeof(<%=realCType%>)* arrayLen);
                  for(i = 0; i < arrayLen; i++) {
                     array[i] = tmpArray[i];
                  }
                  (*env)-><%=ch.getReleaseArrayElementsMethod(realType)%>(env, (<%=realCType%>Array)temp, tmpArray, JNI_ABORT);
                  if (test_jni_error(env, "release the array elements of the result failed", alpp)) {
                     sge_free(&array);
                  } else {
                     *result = array;
                     *len = arrayLen;
                  }
               }
               <% } else { %>
               array = (<%=realCType%>*)malloc(sizeof(<%=realCType%>)* arrayLen);
               for(i = 0; i < arrayLen; i++) {
                  array[i] = (*env)->GetObjectArrayElement(env, (jobjectArray)temp, i); 
                  if (test_jni_error(env, "Can not get object from array", alpp)) {
                     sge_free(&array);
                     array = NULL;
                     break;
                  }
               }
               if(array != NULL) {
                  *result = array;
                  *len = arrayLen;
               }
               <% } %>
          } else {
             *result = NULL;
             *len = 0;
          }
       }
<%     } else { %>
      *result = temp;
<%     } %>      
<% } // end of !Void.TYPE %>
   DRETURN(ret);
}<%
   } // end of for
%>

/*==== END  <%=fullClassname%> */
