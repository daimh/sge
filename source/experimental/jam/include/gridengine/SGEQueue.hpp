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
#ifndef _SGE_QUEUE_HPP
#define _SGE_QUEUE_HPP

#include <jni.h>

#include "cull_list.h"

class SGEExecHost;

class SGEQueue 
{
private:
  // SGE queue's variables
  const char* name;// queue name 
  const char* hostname;// queue hostname  
  const char* initialState; // queue initioal state 
  const char* tmpDir; // queue tmp dir 
  const char* shell;// queue shell 
  long type;// queue type; see sge_queueL.h
  long seqNo; // queue sequence number 
  long jobSlots; // queue number of job slots 
  long state; // queue state; see sge_queueL.h
  int processors; // number of processors
  SGEExecHost* execHost; // queue's Execution Host  

  JNIEnv* env; // JNI Environment 

  lList* queues; // queue list pointer 
  lListElem* queue_elem; // queue element pointer 
  lList* answer; // answer list pointer returned by sge_gdi() 

  int fill_queue(lListElem*);
  
public:
  SGEQueue();
  SGEQueue(JNIEnv*, // JNI Environment
              char*    // name
    );
  SGEQueue(JNIEnv*, // JNI Environment
              const char*,   // name
              const char*,   // hostname
              long,    // type
              long,    // job slots
              const char*,   // initial state
              const char*,   // shell
              const char*,   // tmp dir
              long     // seq no
    );
  SGEQueue(JNIEnv*, // JNI Environment
              const char*,   // name
              const char*,   // hostname
              long,    // type
              long,    // job slots
              const char*,   // initial state
              long,    // state
              const char*,   // shell
              const char*,   // tmp dir
              long     // seq no
    );
  ~SGEQueue();

  const char* getName();
  const char* getHostname();
  const char* getTmpDir();
  const char* getShell();
  long getType();
  long getJobSlots();
  long getSeqNo();
  long getState();
  SGEExecHost* getExecHost();
  
  void setName(char*);
  void setHostname(char*);
  void setTmpDir(char*);
  void setShell(char*);
  void setType(long);
  void setJobSlots(long);
  void setState(long);
  void setExecHost(SGEExecHost *);
  
  // adds new queue to SGE RMS
  void add();

  // removes queue from a SGE RMS
  void remove();

  // update queue's settings
  // each queue is responsible for updating its own parameters
  void update();

  // upload the changes made by JAM
  void upload();
  
  // changes queue's state
  void changeState(long); // queue's new state

  void freeSGEVars();
};

#endif /* _CODEINE_QUEUE_HPP */
