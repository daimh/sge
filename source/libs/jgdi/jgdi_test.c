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
#include <jni.h>
#include <string.h>
#include <stdlib.h>

#include "uti/sge_stdlib.h"

#ifdef DARWIN
int JNI_CreateJavaVM_Impl(JavaVM **pvm, void **penv, void *args);
#endif

JNIEnv* create_vm(int argc, char** argv);

int invoke_main(JNIEnv* env, jclass main_class, int argc, char** argv);
int load_libs(JNIEnv *env, jclass main_class);

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
 *-------------------------------------------------------------------------*/
JNIEnv* create_vm(int argc, char** argv) {
	JavaVM* jvm;
	JNIEnv* env;
	JavaVMInitArgs args;
   int i = 0;
	JavaVMOption* options = (JavaVMOption*)malloc(argc*sizeof(JavaVMOption));
	
	/* There is a new JNI_VERSION_1_4, but it doesn't add anything for the purposes of our example. */
	args.version = JNI_VERSION_1_2;
	args.nOptions = argc;
   for(i=0; i < argc; i++) {
      /*printf("jvm_args[%d] = %s\n", i, argv[i]);*/
      options[i].optionString = argv[i];
   }
	args.options = options;
	args.ignoreUnrecognized = JNI_FALSE;

#ifdef DARWIN
   /*
   ** for darwin there exists no JNI_CreateJavaVM, Why not maybe a fix in the future ???
   */
	i = JNI_CreateJavaVM_Impl(&jvm, (void **)&env, &args);
#else   
	i = JNI_CreateJavaVM(&jvm, (void **)&env, &args);
#endif   
   if(i<0) {
      fprintf(stderr,"can not create JVM (error code %d)\n", i);
      env = NULL;
   }
   
   sge_free(&options);
	return env;
}
/*-------------------------------------------------------------------------*
 * NAME
 *   load_libs - call the loadLibs method of com.sun.grid.jgdi.TestRunner
 *
 * PARAMETER
 *  env -  JNI environment
 *
 * RETURN
 *   0 - loadLibs method successfully executed
 *   else - error
 *
 * EXTERNAL
 *
 * DESCRIPTION
 *
 *   the loadLibs method of com.sun.grid.jgdi.TestRunner load the shared
 *   library libjgdi.so. If the jgdi_test program is started a debugger
 *   after the call of this method all symbols of the shared library 
 *   are available
 *-------------------------------------------------------------------------*/
int load_libs(JNIEnv *env, jclass main_class) {
	jmethodID mid;
   
	mid = (*env)->GetStaticMethodID(env, main_class, "loadLib", "()V");
   if (mid != NULL) {
      (*env)->CallStaticVoidMethod(env, main_class, mid);
      if( (*env)->ExceptionOccurred(env)) {
         (*env)->ExceptionDescribe(env);
         return 1;
      }
   } else {
      (*env)->ExceptionClear(env);
   }
   
   return 0;
}


/*-------------------------------------------------------------------------*
 * NAME
 *   invoke_main - invoke the main method of com.sun.grid.jgdi.TestRunner
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
int invoke_main(JNIEnv* env, jclass main_class, int argc, char** argv) {
	jmethodID main_mid;
	jobjectArray main_args;
   int i = 0;
   
	main_mid = (*env)->GetStaticMethodID(env, main_class, "main", "([Ljava/lang/String;)V");
   if (main_mid == NULL) {
      fprintf(stderr, "class has no main method\n");
      return 1;
   }

	main_args = (*env)->NewObjectArray(env, argc, (*env)->FindClass(env, "java/lang/String"), NULL);
   
   for(i=0; i < argc; i++) {
      jstring str = (*env)->NewStringUTF(env, argv[i]);
      /*printf("argv[%d] = %s\n", i, argv[i]);*/
      (*env)->SetObjectArrayElement(env, main_args, i, str);
   }

	(*env)->CallStaticVoidMethod(env, main_class, main_mid, main_args);
   
   if( (*env)->ExceptionOccurred(env)) {
      (*env)->ExceptionDescribe(env);
      return 1;
   }
   
   return 0;
}


int main(int argc, char **argv) {
   
   int i = 0;
   int jvm_argc = 0;
   int main_argc = 0;
   int jvm_argv_index = 0;
   int main_argv_index = 0;
   char ** jvm_argv = (char**)malloc(argc*sizeof(char*)+1); /* plus one for classpath */
   char ** main_argv = (char**)malloc(argc*sizeof(char*));
   char *main_class_name = NULL;
   jobject main_class = NULL;
   char *p = NULL;
   int  doexit = 1;
   int result = 0;
	JNIEnv* env = NULL;
   
   int  classpath_index = -1;   
   const char* classpath_from_env = getenv("CLASSPATH");
   if(classpath_from_env != NULL) {
      
      int len = strlen("-Djava.class.path=") + strlen(classpath_from_env) + 1;
      jvm_argv[jvm_argv_index] = (char*)malloc(len);
      sprintf(jvm_argv[jvm_argv_index],"-Djava.class.path=%s", classpath_from_env);
      classpath_index = jvm_argv_index;
      jvm_argv_index++;
      jvm_argc++;
   }
   
   for(i=1; i < argc; i++) {
      if ( strcmp(argv[i], "-noexit") == 0 ) {
         doexit=0;
      } else if ( strncmp(argv[i], "-X", 2) == 0 ) {
         jvm_argc++;
         jvm_argv[jvm_argv_index++] = argv[i]+2;
      } else if (strcmp(argv[i],"-cp") == 0 ) {
         i++;
         if(i >= argc) {
            fprintf(stderr, "Missing classpath for -cp options\n");
            exit(1);
         }
         if (classpath_index < 0) {
            int len = strlen("-Djava.class.path=") + strlen(argv[i]) + 1;
            jvm_argv[jvm_argv_index] = (char*)malloc(len);
            sprintf(jvm_argv[jvm_argv_index],"-Djava.class.path=%s", argv[i]);
            classpath_index = jvm_argv_index;
            jvm_argc++;
            jvm_argv_index++;
         } else {
            char *tmp = jvm_argv[classpath_index];
            int len = strlen(tmp) + strlen(argv[i]) + 2;
            jvm_argv[classpath_index] = (char*)malloc(len);
            sprintf(jvm_argv[classpath_index],"%s:%s", tmp, argv[i]);
            sge_free(&tmp);
         }
      } else {
         if(main_class_name == NULL) {
            main_class_name = argv[i];
         } else {
            main_argc++;
            main_argv[main_argv_index++] = argv[i];
         }
      }
   }
   
   if(main_class_name == NULL) {
      fprintf(stderr,"no main class defined\n");
      exit(1);
   }
   
   
	env = create_vm(jvm_argc, jvm_argv);
   if(env == NULL) {
      exit(1);
   }
   
   for(p= main_class_name; *p != '\0'; p++) {
      if(*p == '.') {
         *p = '/';
      }
   }
   
   	main_class = (*env)->FindClass(env, main_class_name);
   if (main_class == NULL) {
      fprintf(stderr,"class %s not found\n", main_class_name);
      exit(1);
   }
   
   main_class = (jclass)(*env)->NewGlobalRef(env, main_class);
   if (main_class == NULL) {
      fprintf(stderr,"Did not get a global ref on main class\n");
      exit(1);
   }
   
   if(load_libs(env, main_class)) {
      exit(1);
   }
   
   result = invoke_main( env, main_class, main_argc, main_argv );
   
   if(doexit) {
      exit(result);
   }
   return 0;
}



