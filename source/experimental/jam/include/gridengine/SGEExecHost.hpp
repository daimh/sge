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
#ifndef _SGE_EXEC_HOST_HPP
#define _SGE_EXEC_HOST_HPP

#include <jni.h>

#include "cull_list.h"

class SGEExecHost
{
private:
  const char *name;
  char *realName;
  char *arch;
  char *load;
  long processors;

  JNIEnv *env;

  lList *exech, *loads, *answer;
  lListElem *exech_elem, *load_elem;
  lEnumeration *what;
  lCondition *where;
  
public:
  SGEExecHost(JNIEnv*, // JNI Environment
                 const char*   // execution host name
    );
  SGEExecHost(JNIEnv*, // JNI Environment
                 char*,   // execution host name
                 char*,   // architecture
                 long,    // number of processors
                 char*   // load
    );
  ~SGEExecHost();

  const char *getName();
  char *getArch();
  char *getLoad();
  long getProcessors();

  void update();

  void freeSGEVars();
};

#endif /* _CODEINE_EXEC_HOST_HPP */
