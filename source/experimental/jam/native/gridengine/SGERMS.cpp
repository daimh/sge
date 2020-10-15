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
#include "SGERMS.hpp"

#include "SGEExecHost.hpp"
#include "SGEQueue.hpp"
#include "utils.h"

#include "sge_gdi.h"
#include "sge_queueL.h"

#include <stdio.h>
#include <string.h>

//================= constructors ================

SGERMS::SGERMS(JNIEnv *_env)
{
  env = _env;

  queues = NULL;
  queue_elem = NULL;
  answer = NULL;
  what = NULL;
  where = NULL;
}

//================== destructor =================

SGERMS::~SGERMS()
{
  if(queues)
    lFreeList(queues);
  if(queue_elem)
    lFreeElem(&queue_elem);
  if(answer)
    lFreeList(answer);
  if(what)
    lFreeWhat(what);
  if(where)
    lFreeWhere(&where);
}

//================== public functions =============
#define SGE_TEMPLATE_NAME  "template"
SGEQueue** SGERMS::retreiveAllQueues()
{
  char *error_header;
  
  what = lWhat("%T( ALL )", QU_Type);
  where = lWhere("%T(%I != %s)", QU_Type, QU_qname, SGE_TEMPLATE_NAME);
  answer = sge_gdi(SGE_QUEUE_LIST, SGE_GDI_GET, &queues, where, what);

  error_header = "failed to retreive all queues :";
  check_answer_status(env, answer, "N/A", error_header);
  
  if(env->ExceptionCheck() == JNI_TRUE)
    return 0;
  
  if(queues) {
    // reserve space for number of queues that have been found minus
    // 1, that's because there is one queue that is useless
    // i.e. template queue
    int length = lGetNumberOfElem(queues);
//    printf("Alocating space for %d queues\n", length);
    SGEQueue** sgeQueues = new
      SGEQueue*[length];
    for(int j = 0; j <= length; j++)
      sgeQueues[j] = 0;
      
    int i = 0;
    for_each_cpp(queue_elem, queues) {
      if(queue_elem) {
//         printf("inserting queue: %s\n",
//                lGetString(queue_elem, QU_qname));
//         printf("\tstate: %x\n",
//                lGetUlong(queue_elem, QU_state));
          
        sgeQueues[i] = new
          SGEQueue(env,
                      lGetString(queue_elem, QU_qname),
                      lGetHost(queue_elem, QU_qhostname),
                      lGetUlong(queue_elem, QU_qtype),
                      lGetUlong(queue_elem, QU_job_slots),
                      lGetString(queue_elem, QU_initial_state),
                      lGetUlong(queue_elem, QU_state),
                      lGetString(queue_elem, QU_shell),
                      lGetString(queue_elem, QU_tmpdir),
                      lGetUlong(queue_elem, QU_seq_no));
        
        SGEExecHost* execHost = new
          SGEExecHost(env, lGetHost(queue_elem, QU_qhostname));
        execHost->update();
        
        if(env->ExceptionCheck() == JNI_TRUE) 
          env->ExceptionClear();
        
        sgeQueues[i]->setExecHost(execHost);
        
        ++i;
//           printf("inserted %s queue\n", lGetString(queue_elem,
//                                                    QU_qname));

      } // if(queue_elem)
    } // for_each_cpp(...
    return sgeQueues;
  } // if(queue)
  else {
    // queues == NULL
    error_header = "failed to retreive all queues : no queues found";
    throw_exception(env, error_header);
  }
  return 0;
}
