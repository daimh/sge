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
#ifndef _UTILS_H
#define _UTILS_H

#include <jni.h>

#include "cull_list.h"

#ifdef __cplusplus
extern "C" {
#endif

int callIntMethod(JNIEnv*,
                  jobject, 
                  char*
  );
  
jobject callObjectMethod(JNIEnv*,
                          jobject,
                          char*, char*
  );
  
char* callStringMethod(JNIEnv*,
                       jobject,
                       char*
  );

int callSetIntMethod(JNIEnv*,
                      jobject,
                      char*,
                      int
  );
  
long getLongField(JNIEnv*,
                  jobject, // java object 
                  char*// field name
  );
  
int getIntField(JNIEnv*,
                jobject, // java object
                char*// field name
  );
  
char* getStringField(JNIEnv*,
                     jobject, // java object 
                     char*// field name
  );  

jobject getObjectField(JNIEnv*,
                       jobject,
                       char*,
                       char*
  );
  
char* jstring_to_charptr(JNIEnv*, jstring);
  
/* answer handling functions */
void check_answer_status(JNIEnv*, lList*, char*, char*);
void print_answer(lList*, char*);
void print_answer_status(char*, int);
char *answer_statustostr(int);

/* exceptions */
void throw_exception(JNIEnv*, char*);
void throw_communication_exception(JNIEnv*, char*);

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H */
