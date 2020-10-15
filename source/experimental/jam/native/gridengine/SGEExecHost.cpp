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
#include "SGEExecHost.hpp"

#include <stdio.h>

/* SGE GDI */
#include "sge_gdi.h"
#include "sge_hostL.h"

#include "utils.h"

#include <string.h>

extern char *strdup(const char*);
extern lNameSpace nmv[];

//================= constructors ===================
SGEExecHost::SGEExecHost(JNIEnv *_env,
                               const char *_name)
{
  name = _name;
  arch = 0;
  load = 0;
  processors = 0;
  
  env = _env;

  exech = 0;
  exech_elem = 0;
  loads = 0;
  load_elem = 0;
  answer = 0;
  what = 0;
  where = 0;
}

SGEExecHost::SGEExecHost(JNIEnv *_env,
                               char *_name,
                               char *_arch,
                               long _processors,
                               char *_load)
{
  name = _name;
  arch = _arch;
  processors = _processors;
  load = _load;

  env = _env;

  exech = 0;
  exech_elem = 0;
  loads = 0;
  load_elem = 0;
  answer = 0;
  what = 0;
  where = 0;
}

//==================== destructor =====================

SGEExecHost::~SGEExecHost()
{
  if(exech)
    lFreeList(&exech);
  if(exech_elem)
    lFreeElem(&exech_elem);
  if(loads)
    lFreeList(&loads);
  if(load_elem)
    lFreeElem(&load_elem);
  if(answer)
    lFreeList(answer);
  if(what)
    lFreeWhat(what);
  if(where)
    lFreeWhere(&where);
}

//=================== getters ========================

const char* SGEExecHost::getName()
{
  return name;
}

char* SGEExecHost::getArch()
{
  return arch; 
}

char* SGEExecHost::getLoad()
{
  return load;
}

long SGEExecHost::getProcessors()
{
  return processors;
}

//=================== public functions ===============

void SGEExecHost::update()
{
  char error_header[256];
  char error_reason[256];
  
  what = lWhat("%T( %I %I )", EH_Type, EH_processors, EH_load_list);
  where = lWhere("%T( %I == %s )", EH_Type, EH_name, name);
  answer = sge_gdi(SGE_EXECHOST_LIST, SGE_GDI_GET, &exech,
                   where, what);

  sprintf(error_header, "failed to update \"%s\" host's attrs :", name);
  check_answer_status(env, answer, "N/A", error_header);

  if(env->ExceptionCheck() == JNI_TRUE) {
    freeSGEVars();
    return;
  }
  
  if(exech) {
    // TODO:
    // need to do some null pointer checking for safety
    exech_elem = lFirst(exech);
    if(exech_elem) {
      processors = lGetUlong(exech_elem, EH_processors);
      loads = lGetList(exech_elem, EH_load_list);
      if(loads) {
        // arch
        where = lWhere("%T( %I == %s )", HL_Type, HL_name, "arch");
        load_elem = lFindFirst(loads, where);
        if(load_elem)
          arch = strdup(lGetString(load_elem, HL_value));
        else
          arch = "Unknown";

        // load
        where = lWhere("%T( %I == %s )", HL_Type, HL_name, "load_avg");
        load_elem = lFindFirst(loads, where);
        if(load_elem)
          load = strdup(lGetString(load_elem, HL_value));
        else
          load = "Unknown";
      } else {
        // loads == NULL
        arch = "Unknown";
        load = "Unknown";
      }
    } else {
      // exech_elem == NULL
      processors = 0;
      arch = "Unknown";
      load = "Unknown";
    }
  } else {
    // exech == 0
    // this means that such execution host does not exists in sge
    // any more, therefore we should raise an exception
    sprintf(error_reason, "%s execution host doesn't exist",
            error_header);
    throw_exception(env, error_reason);
  }

  freeSGEVars();
}

void SGEExecHost::freeSGEVars()
{
  if(exech)
    lFreeList(exech);
  if(exech_elem)
    lFreeElem(&exech_elem);
  if(loads)
    lFreeList(loads);
  if(load_elem)
    lFreeElem(&load_elem);
  if(answer)
    lFreeList(answer);
  if(what)
    lFreeWhat(what);
  if(where)
    lFreeWhere(&where);
}
