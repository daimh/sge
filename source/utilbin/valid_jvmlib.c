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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <stdio.h>
#include <stdlib.h>

#ifndef NO_JNI

#include <string.h>

#ifdef SOLARIS
#include <link.h>
#endif

#include <jni.h>

#include "uti/sge_dlopen.h"

typedef int (*JNI_CreateJavaVM_Func)(JavaVM **pvm, void **penv, void *args);
typedef int (*JNI_GetCreatedJavaVMs_Func)(JavaVM **pvm, jsize size, jsize *real_size);

static void *libjvm_handle = NULL;
static JNI_CreateJavaVM_Func sge_JNI_CreateJavaVM = NULL;
static JNI_GetCreatedJavaVMs_Func sge_JNI_GetCreatedJavaVMs = NULL;

void usage(void) {
    printf("usage: valid_jvmlib [-help] <libpath>\n" \
           "       <libpath> ... path to the java library\n");
}
#endif /* NOJNI */

int main(int argc, char** argv) {
#ifndef NO_JNI
    const char* JNI_CreateJavaVM_FuncName;
    const char* JNI_GetCreatedJavaVMs_FuncName;
    const char *libjvm_path;
    FILE *file;
    if (argc < 2) {
        printf("Error: No argument provided!\n");
        usage();
        return 1;
    }
    libjvm_path = argv[1];
    if (strcmp("-help", libjvm_path) == 0) {
        usage();
        return 1;
    }

    if ((file = fopen(libjvm_path, "r"))) {
        fclose(file);
    } else {
        printf("Error: Could not open %s library!\n", libjvm_path);
        return 1;
    }

    /* open the shared lib */
    libjvm_handle = sge_dlopen(libjvm_path, NULL);

    if (libjvm_handle == NULL) {
        printf("Error: Unable to load %s library!\n", libjvm_path);
        return 1;
    }

#ifdef DARWIN
    /* for darwin there exists no JNI_CreateJavaVM, Why not maybe a fix in the future ??? */
    JNI_CreateJavaVM_FuncName = "JNI_CreateJavaVM_Impl";
    JNI_GetCreatedJavaVMs_FuncName = "JNI_GetCreatedJavaVMs_Impl";
#else
    JNI_CreateJavaVM_FuncName = "JNI_CreateJavaVM";
    JNI_GetCreatedJavaVMs_FuncName = "JNI_GetCreatedJavaVMs";
#endif

    sge_JNI_CreateJavaVM = (JNI_CreateJavaVM_Func) dlsym(libjvm_handle, JNI_CreateJavaVM_FuncName);
    if (sge_JNI_CreateJavaVM == NULL) {
        printf("Error: Unable to find %s function in %s library!\n", JNI_CreateJavaVM_FuncName, libjvm_path);
        return 1;
    }

    sge_JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_Func) dlsym(libjvm_handle, JNI_GetCreatedJavaVMs_FuncName);
    if (sge_JNI_GetCreatedJavaVMs == NULL) {
        printf("Error: Unable to find %s function in %s library!\n", JNI_GetCreatedJavaVMs_FuncName, libjvm_path);
        return 1;
    }
#endif
    return 0;
}

