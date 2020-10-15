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
#ifndef _SGE_JOB_HPP
#define _SGE_JOB_HPP

#include <jni.h>

#include "cull_list.h"

class StringValue;
class Variable;
class HostPath;

/* class that represent sge job */  
class SGEJob {
private:

  // SGE Job's variables
  char* name;
  char* scriptFile;
  char* cwd;
  char* queueName;
  int force;
  int hold;
  long id;
  long state;
  long status;
  StringValue** args;
  Variable** envVars;
  HostPath** stdoutPaths;
  HostPath** stderrPaths;
  HostPath** shells;

  
  // SGE GDI variables
  lList *jobs, *answer;
  lList *tasks;
  lListElem *job_elem;
  lListElem *task_elem;
  lEnumeration *what;
  lCondition *where;

  // JNI Environment
  JNIEnv *env;
  
  // private functions which I don't want any one to use
  void fill_job_for_submit(lListElem*);
//   void set_job_queue(lListElem*);

public:
  SGEJob(JNIEnv*,  // JNI Environment
            char*     // job name
    );
  SGEJob(JNIEnv*,  // JNI Environment
            char*,    // job name
            long      // job id
    );
  SGEJob(JNIEnv*,  // JNI Environment
            char*,    // job name
            char*,    // script file name 
            StringValue**,    // job arguments
            Variable**, // job environment variables
            HostPath**,    // stdout path 
            HostPath**,    // strerr path
            HostPath**,    // shell
            char*,    // common working directory
            char*     // queue name 
            );
  ~SGEJob();
  long getState();
  long getStatus();
  long getId();

  void freeSGEVars();
  
  // submit job to SGE RMS
  // we don't care on which queue the job will be sheduled to
  void submit();
  
  // find a corresponding id for a job with given name
  void resolveId();
  
  // remove a job with given id
  // NOTE: delete is a keyword so I use remover instead of it
  void remove();

  // update state & status of job with given id
  void updateStat();
  
  // change state of a job with given id
  void changeState(long);  // new job state

  // set the JB_force to 1
  void setForce();
  // set the JB_force to 0
  void unSetForce();
  // set the JAT_hold to MINUS_H_TGT_USER
  void setHold();
  // set the JAT_hold to MINUS_H_CMD_SET
  void unSetHold();
};

#endif /* _CODEINE_JOB_HPP */
