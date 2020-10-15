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
#include <unistd.h>
#include <termios.h>
#include <string.h>

#include "com_sun_grid_util_SGEUtil.h"
#include "juti.h"

JNIEXPORT jint JNICALL Java_com_sun_grid_util_SGEUtil_getPID(JNIEnv *env, jobject obj) {
	return getpid();
}

   
JNIEXPORT jbyteArray JNICALL Java_com_sun_grid_util_SGEUtil_getNativePassword
(JNIEnv *env, jobject sge_util) {
   
   jcharArray pass = NULL;
   char buf[512];
   char *ret = NULL;
   int len = 0;

   setEcho(0);
   ret = fgets(buf, sizeof(buf), stdin);
   setEcho(1);
   printf("\n");
   if( ret != NULL) {
      len = strlen(ret);
      pass = (*env)->NewByteArray(env,len);
      (*env)->SetByteArrayRegion( env, pass, 0, len, (jbyte*)ret);

   }   
   return pass;
}

