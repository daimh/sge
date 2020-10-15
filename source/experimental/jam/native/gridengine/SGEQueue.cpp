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
#include "SGEQueue.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SGE GDI */
#include "sge_gdi.h"
#include "sge_queueL.h" 
#include "gdi_qmod.h"
#include "sge_stringL.h"
#include "sge_requestL.h"

#include "utils.h"
#include "SGEExecHost.hpp"

extern lNameSpace nmv[];

//=============== constructors =====================
SGEQueue::SGEQueue()
{
}

SGEQueue::SGEQueue(JNIEnv* _env,
                         char* _name)
{
  name = _name;

  execHost = 0;
  
  env = _env;

  queues = 0;
  queue_elem = 0;
  answer = 0;
  
  lInit(nmv);  
}

SGEQueue::SGEQueue(JNIEnv* _env,
                         const char* _name,
                         const char* _hostname,
                         long _type, 
                         long _jobSlots,
                         const char* _initialState,
                         long _state,
                         const char* _shell,
                         const char* _tmpDir,
                         long _seqNo)
{
  name = _name;
  hostname = _hostname;
  type = _type;
  jobSlots = _jobSlots;
  initialState = _initialState;
  state = _state;
  shell = _shell;
  tmpDir = _tmpDir;
  seqNo = _seqNo;

  execHost = 0;

  env = _env;

  queues = 0;
  queue_elem = 0;
  answer = 0;
  
  lInit(nmv);
}

SGEQueue::SGEQueue(JNIEnv* _env,
                         const char* _name,
                         const char* _hostname,
                         long _type, 
                         long _jobSlots,
                         const char* _initialState,
                         const char* _shell,
                         const char* _tmpDir,
                         long _seqNo)
{
  name = _name;
  hostname = _hostname;
  type = _type;
  jobSlots = _jobSlots;
  initialState = _initialState;
  shell = _shell;
  tmpDir = _tmpDir;
  seqNo = _seqNo;

  execHost = 0;

  env = _env;

  queues = 0;
  queue_elem = 0;
  answer = 0;
  
  lInit(nmv);
}

//=============== destructor =======================

SGEQueue::~SGEQueue()
{
  if(execHost)
    delete execHost;
  
  if(queues)
    lFreeList(&queues);
  if(queue_elem)
    lFreeElem(&queue_elem);
  if(answer)
    lFreeList(&answer);
}

//=============== getters ==========================
const char* SGEQueue::getName()
{
  return name;
}

const char* SGEQueue::getHostname()
{
  return hostname;
}

const char* SGEQueue::getTmpDir()
{
  return tmpDir;
}

const char* SGEQueue::getShell()
{
  return shell;
}

long SGEQueue::getType()
{
  return type;
}

long SGEQueue::getJobSlots()
{
  return jobSlots;
}

long SGEQueue::getState()
{
  return state;
}

long SGEQueue::getSeqNo()
{
  return seqNo;
}

SGEExecHost* SGEQueue::getExecHost()
{
  return execHost;
}


//=============== setters ==========================

void SGEQueue::setName(char* _name)
{
  name = _name;
}

void SGEQueue::setHostname(char* _hostname)
{
  hostname = _hostname;
}

void SGEQueue::setState(long _state)
{
  state = _state;
}

void SGEQueue::setType(long _type)
{
  type = _type;
}

void SGEQueue::setJobSlots(long _jobSlots)
{
  jobSlots = _jobSlots;
}

void SGEQueue::setTmpDir(char* _tmpDir)
{
  tmpDir = _tmpDir;
}

void SGEQueue::setShell(char* _shell)
{
  shell = _shell;
}

void SGEQueue::setExecHost(SGEExecHost* _execHost)
{
  execHost = _execHost;
}


//=============== public functions =================

void SGEQueue::add()
{
   char error_header[256];

  queues = lCreateList("queue list", QU_Type);   
  
  queue_elem = lCreateElem(QU_Type);
  
  fill_queue(queue_elem);
  
  lAppendElem(queues, queue_elem);
  
  answer = sge_gdi(SGE_QUEUE_LIST, SGE_GDI_ADD, &queues, 0, 0);
  
  sprintf(error_header, "failed to add \"%s\" queue :", name);
  check_answer_status(env, answer, "nativeAddQueue", error_header);

  freeSGEVars();
}
  
void SGEQueue::remove()
{
  char error_header[256];
  
  queues = lCreateList("queue list", QU_Type);

  queue_elem = lCreateElem(QU_Type);
  lAppendElem(queues, queue_elem);
  lSetString(queue_elem, QU_qname, name);

  answer = sge_gdi(SGE_QUEUE_LIST, SGE_GDI_DEL, &queues, 0, 0);
  
  sprintf(error_header, "failed to delete \"%s\" queue :", name);
  check_answer_status(env, answer, "nativeDeleteQueue", error_header);

  freeSGEVars();
}

void SGEQueue::update()
{
}

void SGEQueue::upload()
{
}

void SGEQueue::changeState(long newState)
{
  char error_header[256];  

  queues = lCreateList("queue reference list", ST_Type);
  queue_elem = lCreateElem(ST_Type);

  lSetString(queue_elem, STR, name);
  
  lAppendElem(queues, queue_elem);
  
  answer = gdi_qmod(queues, 0, newState);

  sprintf(error_header, "failed to change \"%s\" queue's state :", name);
  check_answer_status(env, answer, "nativeChangeQueueState",
                      error_header);
  freeSGEVars();
}

void SGEQueue::freeSGEVars()
{
  if(queues)
    lFreeList(queues);
  if(queue_elem)
    lFreeElem(&queue_elem);
  if(answer)
    lFreeList(answer);
}


//============= private functions ==================

int SGEQueue::fill_queue(lListElem* queue_elem)
{
  lSetString(queue_elem, QU_qname, name);
  lSetHost(queue_elem, QU_qhostname, hostname);
  lSetUlong(queue_elem, QU_qtype, type);
  lSetUlong(queue_elem, QU_seq_no, seqNo);
  lSetString(queue_elem, QU_initial_state, initialState);
  lSetUlong(queue_elem, QU_job_slots, jobSlots);
  lSetString(queue_elem, QU_tmpdir, tmpDir);
  lSetString(queue_elem, QU_shell, shell);

  lSetString(queue_elem, QU_suspend_interval, "00:05:00");
  lSetUlong(queue_elem, QU_priority, 0); /* nice value for job 0..19 */
  lSetString(queue_elem, QU_enable_migr, "0");        /* ckpt only, not
                                                  * supported yet */
  lSetString(queue_elem, QU_min_cpu_interval, "0");     /* ckpt only, not
                                                  * supported yet */
  lSetString(queue_elem, QU_processors, "UNDEFINED");   /* processor sets
                                                  * only for Irix 6.5
                                                  * */
  lSetUlong(queue_elem, QU_rerun, 0);                   /* boolean value 0|1
                                                  * */
  lSetString(queue_elem, QU_notify, "0:0:30");            /* send USR1 USR2
                                                    * before
                                                    * suspend/kill */
  lSetList(queue_elem, QU_owner_list, 0);              /* list of queue
                                                    * owners */
  
  lSetString(queue_elem, QU_s_rt,    "INFINITY");
  lSetString(queue_elem, QU_s_rt,    "INFINITY");
  lSetString(queue_elem, QU_s_rt,    "INFINITY");
  lSetString(queue_elem, QU_h_rt,    "INFINITY");
  lSetString(queue_elem, QU_s_cpu,   "INFINITY");
  lSetString(queue_elem, QU_h_cpu,   "INFINITY");
  lSetString(queue_elem, QU_s_fsize, "INFINITY");
  lSetString(queue_elem, QU_h_fsize, "INFINITY");
  lSetString(queue_elem, QU_s_data,  "INFINITY");
  lSetString(queue_elem, QU_h_data,  "INFINITY");
  lSetString(queue_elem, QU_s_stack, "INFINITY");
  lSetString(queue_elem, QU_h_stack, "INFINITY");
  lSetString(queue_elem, QU_s_core,  "INFINITY");
  lSetString(queue_elem, QU_h_core,  "INFINITY");
  lSetString(queue_elem, QU_s_rss,   "INFINITY");
  lSetString(queue_elem, QU_h_rss,   "INFINITY");
  lSetString(queue_elem, QU_s_vmem,  "INFINITY");
  lSetString(queue_elem, QU_h_vmem,  "INFINITY");
  
  return 0;
}
