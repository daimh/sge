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
#include "SGEJob.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SGE GDI */
#include "sge_gdi.h"
#include "gdi_qmod.h"
#include "sge_jobL.h"
/* #include "sge_job_reportL.h" */
#include "sge_ja_taskL.h"
#include "sge_queueL.h" 
#include "sge_stringL.h"
#include "sge_peL.h"
#include "sge_idL.h"
#include "sge_io.h"
#include "sge_rangeL.h"
#include "sge_varL.h"

#include "StringValue.hpp"
#include "Variable.hpp"
#include "HostPath.hpp"
#include "utils.h"

extern lNameSpace nmv[];

//================= constructors ===================

SGEJob::SGEJob(JNIEnv *_env, char *_name)
{
  name = _name;
  scriptFile = 0;
  args = 0;
  envVars = 0;
  stdoutPaths = 0;
  stderrPaths = 0;
  shells = 0;
  cwd = 0;
  queueName = 0;
  force = 1;
  hold = 1;
  id = -1;
  state = -1;
  status = -1;
  
  env = _env;

  jobs = 0;
  answer = 0;
  tasks = 0;
  job_elem = 0;
  task_elem = 0;
  what = 0;
  where = 0;

  lInit(nmv);
}

SGEJob::SGEJob(JNIEnv *_env,
                     char *_name, long _id)
{
  name = _name;
  scriptFile = 0;
  args = 0;
  envVars = 0;
  stdoutPaths = 0;
  stderrPaths = 0;
  shells = 0;
  cwd = 0;
  queueName = 0;
  force = 1;
  hold = 1;
  id = _id;
  state = -1;
  status = -1;
  
  env = _env;

  jobs = 0;
  answer = 0;
  tasks = 0;
  job_elem = 0;
  task_elem = 0;
  what = 0;
  where = 0;

  lInit(nmv);
}

SGEJob::SGEJob(JNIEnv *_env,
                     char* _name,
                     char* _scriptFile,
                     StringValue** _args,
                     Variable** _envVars,
                     HostPath** _stdoutPaths,
                     HostPath** _stderrPaths,
                     HostPath** _shells,
                     char* _cwd,
                     char* _queueName)
{
  name = _name;
  scriptFile = _scriptFile;
  args = _args;
  envVars = _envVars;
  stdoutPaths = _stdoutPaths;
  stderrPaths = _stderrPaths;
  shells = _shells;
  cwd = _cwd;
  queueName = _queueName;
  force = 1;
  hold = 1;
  id = -1;
  state = -1;
  status = -1;
  
  env = _env;

  jobs = 0;
  answer = 0;
  tasks = 0;
  job_elem = 0;
  task_elem = 0;
  what = 0;
  where = 0;

  lInit(nmv);
}

//==================== destructor =====================

SGEJob::~SGEJob()
{
  delete name;
  delete scriptFile;
  if(args)
    delete[] args;
  if(envVars)
    delete[] envVars;
  if(stdoutPaths)
    delete[] stdoutPaths;
  if(stderrPaths)
    delete[] stderrPaths;
  if(shells)
    delete[] shells;
  delete cwd;
  delete queueName;
  
  if(jobs)
    lFreeList(&jobs);
  if(answer)
    lFreeList(answer);
  if(tasks)
    lFreeList(&tasks);
  if(what)
    lFreeWhat(what);
  if(where)
    lFreeWhere(&where);
}

//=================== getters ========================

long SGEJob::getId()
{
  return id;
}

long SGEJob::getState()
{
  return state;
}

long SGEJob::getStatus()
{
  return status;
}

//=================== setters ========================

// void SGEJob::setQueue(char *_queueName)
// {
//   queueName = _queueName;
// }

//=================== public functions ===============

void SGEJob::submit()
{
  char error_header[256];
  
  /* create list of JB_Type with 1 element */
  jobs = lCreateElemList("JAM job", JB_Type, 1);
  /* get pointer to the first element in the list */
  job_elem = lFirst(jobs);
  
  fill_job_for_submit(job_elem);
  
  /* submit (add) the job */
  answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, &jobs, 0, 0);

  // printf("================    SUBMITTED JOB   ================\n");
  // lWriteListTo(jobs, stdout);
  // printf("================ END of JOB LIST ================\n");
  
  sprintf(error_header, "failed to submit job \"%s\" :", name);
  check_answer_status(env, answer, "nativeSubmitJob", error_header);
}

void SGEJob::resolveId()
{
  char error_header[256];
  
  what = lWhat("%T( %I )", JB_Type, JB_job_number);
  where = lWhere("%T( %I == %s )", JB_Type, JB_job_name, name);
  
  answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_GET, &jobs, where, what);
  // printf("Called sge_gdi from resolveId()\n");

  sprintf(error_header, "failed to update job \"%s\" ID :", name);
  check_answer_status(env, answer, "nativeUpdateJobID", error_header);
    
  if(env->ExceptionCheck() == JNI_TRUE)
    return;
  
  if( jobs != 0 ) {
    job_elem = lFirst(jobs);
    if( job_elem != 0 ) {
      id = lGetUlong(job_elem, JB_job_number);
    } else {
      // job_elem == 0
      char str[256];
      sprintf(str, "Update job ID failed : Job \"%s\" not found.", name);
      throw_exception(env, str);
    }
  }
  else {
    // jobs == 0
    char str[256];
    sprintf(str, "Update job ID failed : Job \"%s\" not found.", name);
    throw_exception(env, str);
  }
}

void SGEJob::remove()
{
  char error_header[256];
  char job_idstr[2];
  
  /* covert int job_id to string job_id */
  sprintf(job_idstr, "%ld", id);
  
  jobs = lCreateList("job id list", ID_Type);
  
  job_elem = lCreateElem(ID_Type);
  lSetString(job_elem, ID_str, job_idstr);
  lAppendElem(jobs, job_elem);
  
  answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_DEL, &jobs, 0, 0);
  // printf("Called sge_gdi from remove()\n");
  
  sprintf(error_header, "failed to delete job \"%ld\" :", 
          id);
  check_answer_status(env, answer, "nativeDeleteJob", error_header);
}

void SGEJob::updateStat()
{
  char error_header[256];
  
  what = lWhat("%T( %I )", JB_Type, JB_ja_tasks);
  where = lWhere("%T( %I == %u )", JB_Type, JB_job_number, id);

  answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_GET, &jobs, where, what);
  // printf("Called sge_gdi from updateStat()\n");
  
  sprintf(error_header, "failed to update job \"%ld\" state :", 
          id);
  check_answer_status(env, answer, "nativeUpdateJobState",
                      error_header);
  
  if(env->ExceptionCheck() == JNI_TRUE) {
    return;
  }
   
  if(jobs) {
    job_elem = lFirst(jobs);
    if(job_elem) {
      tasks = lGetList(job_elem, JB_ja_tasks);
      task_elem = lFirst(tasks);
      if(task_elem) {
        state = lGetUlong(task_elem, JAT_state);
        status = lGetUlong(task_elem, JAT_status);
      }
      else {
        // task_elem == 0

	// No tasks set up - this means job not dispatched yet
	state = JHELD | JQUEUED | JWAITING;
	status = JIDLE;
	return;
      }
    } else {
      // job_elem == 0
      char str[256];
      sprintf(str,
              "Update job state failed : Job \"%s\" with ID \"%ld\", not found.",
              name, id);
      
      throw_exception(env, str);
    }
  } else {
    // jobs == 0
    char str[256];
    sprintf(str,
            "Update job state failed : Job \"%s\" with ID \"%ld\", not found.",
            name, id);
    
    throw_exception(env, str);
  }
}

void SGEJob::changeState(long newState)
{
  char job_idstr[2];
  
  /* covert int to string */
  sprintf(job_idstr, "%ld", id);
        
  jobs = lCreateList("job reference list", ST_Type);
  job_elem = lCreateElem(ST_Type);

  lSetString(job_elem, STR, job_idstr);
  
  lAppendElem(jobs, job_elem);

  answer = gdi_qmod(jobs, 0, newState);
  
  if(answer) {
    char error_header[256];
    sprintf(error_header, "failed to change job \"%ld\" state :", 
            id);
    check_answer_status(env, answer, "nativeChangeJobState", error_header);
  }
}

void SGEJob::setForce()
{
#ifdef AA
  char error_header[256];

  static lDescr force_descr[] = {
    {JB_job_number, lUlongT},
    {JB_force, lUlongT},
    {NoName, lEndT}
  };

  if(!force) {
    job_elem = lAddElemUlong(&jobs, JB_job_number, id, force_descr);
    lSetUlong(job_elem, JB_force, 1);

//   printf("==============     JOB LIST    ================\n");
//   lWriteListTo(jobs, stdout);
//   printf("============== END of JOB LIST ================\n");
  
    answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_MOD, &jobs, 0, 0);
    // printf("Called sge_gdi from setForce()\n");

    sprintf(error_header, "failed to set force flag of job \"%ld\" :", id);
    check_answer_status(env, answer, "nativeForce", error_header);
  }
  force = 1;
#endif  
}

void SGEJob::unSetForce()
{
#ifdef AA
  char error_header[256];

  static lDescr force_descr[] = {
    {JB_job_number, lUlongT},
    {JB_force, lUlongT},
    {NoName, lEndT}
  };

  if(force) {
    job_elem = lAddElemUlong(&jobs, JB_job_number, id, force_descr);
    lSetUlong(job_elem, JB_force, 0);

//   printf("==============     JOB LIST    ================\n");
//   lWriteListTo(jobs, stdout);
//   printf("============== END of JOB LIST ================\n");
  
    answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_MOD, &jobs, 0, 0);
    // printf("called sge_gdi from unSetForce()\n");
    
    sprintf(error_header, "failed to un set force flag of job \"%ld\" :", id);
    check_answer_status(env, answer, "nativeForce", error_header);
  }
  force = 0;
#endif  
}


void SGEJob::setHold()
{
  char error_header[256];

  static lDescr hold_descr[] = {
    {JB_job_number, lUlongT},
    {JB_ja_tasks, lListT},
    {JB_ja_structure, lListT},
    {NoName, lEndT}
  };
  static lDescr task_descr[] = {
    {JAT_task_number, lUlongT},
    {JAT_hold, lUlongT},
    {NoName, lEndT}
  };

  if(!hold) {
    job_elem = lAddElemUlong(&jobs, JB_job_number, id, hold_descr);
    task_elem = lAddElemUlong(&tasks, JAT_task_number, 0, task_descr);
    lSetUlong(task_elem, JAT_hold, MINUS_H_CMD_ADD | MINUS_H_TGT_USER);
    lSetList(job_elem, JB_ja_tasks, tasks);

//   printf("==============     JOB LIST    ================\n");
//   lWriteListTo(jobs, stdout);
//   printf("============== END of JOB LIST ================\n");
  
    answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_MOD, &jobs, 0, 0);
    // printf("Called sge_gdi from setHold()\n");
    
    sprintf(error_header, "failed to hold job \"%ld\" :", id);
    check_answer_status(env, answer, "nativeHold", error_header);
  }
  hold = 1;
}

void SGEJob::unSetHold()
{
  char error_header[256];

  static lDescr hold_descr[] = {
    {JB_job_number, lUlongT},
    {JB_ja_tasks, lListT},
    {JB_ja_structure, lListT},
    {NoName, lEndT}
  };
  static lDescr task_descr[] = {
    {JAT_task_number, lUlongT},
    {JAT_hold, lUlongT},
    {NoName, lEndT}
  };

  if(hold) {
    job_elem = lAddElemUlong(&jobs, JB_job_number, id, hold_descr);
    task_elem = lAddElemUlong(&tasks, JAT_task_number, 0, task_descr);
    lSetUlong(task_elem, JAT_hold, MINUS_H_CMD_SUB | MINUS_H_TGT_USER);
    lSetList(job_elem, JB_ja_tasks, tasks);
    
//   printf("==============     JOB LIST    ================\n");
//   lWriteListTo(jobs, stdout);
//   printf("============== END of JOB LIST ================\n");
    
    answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_MOD, &jobs, 0, 0);
    // printf("Called sge_gdi from unSetHold()\n");
    
    sprintf(error_header, "failed to un hold job \"%ld\" :", id);
    check_answer_status(env, answer, "nativeUnHold", error_header);
  }
  hold = 0;
}

void SGEJob::freeSGEVars()
{
  if(jobs)
    lFreeList(jobs);
  if(answer)
    lFreeList(answer);
  if(tasks)
    lFreeList(tasks);
  if(what)
    lFreeWhat(what);
  if(where)
    lFreeWhere(&where);

  // So destructor doesn't try to free again
  jobs = 0;
  answer = 0;
  tasks = 0;
  where = 0;
}


//====================== private functions ==========================

void SGEJob::fill_job_for_submit(lListElem *job_elem)
{
  int len; /* length of the content of the job script */
  char *job; /* contents of the job script */
  char *cp;

  /* Job Script/Name */  
  lSetString(job_elem, JB_job_name, name);

  lSetString(job_elem, JB_script_file, scriptFile); /* copies the string */
  job = sge_file2string(scriptFile, &len);
  lSetString(job_elem, JB_script_ptr, job);

  free((char*)job);

  lSetUlong(job_elem, JB_script_size, len);

  /* get job directive prefix */
  lSetString(job_elem, JB_directive_prefix, "#$" );
  
  /* 
   * initialize the job-array task list for the job
   */
  {
    lList *tasks;
    lListElem *task;

    tasks = lCreateList("job array task list", JAT_Type);
    task = lCreateElem(JAT_Type);
    lAppendElem(tasks, task);
    lSetList(job_elem, JB_ja_tasks, tasks);
    lSetUlong(task, JAT_task_number, 1);

    // if we want a job to not start immidiately
    // Is this needed, or is setting of JB_ja_?_h_ids fields below sufficient?
    if(hold)
      lSetUlong(task, JAT_hold, MINUS_H_CMD_SET | MINUS_H_TGT_USER);
    else
      lSetUlong(task, JAT_hold, 0);
  }
  
  /*
   * Initialize task ranges in JB_ja_structure; JB_ja_?_h_ids
   */
  {
    lList *range_list = NULL;
    lListElem *range_elem;
    lList *n_h_list;

    // Add range elements to JB_ja_structure
    range_elem = lFirst(lGetList(job_elem, JB_ja_structure));
    if (!range_elem) {
      range_elem = lCreateElem(RN_Type);
      range_list = lCreateList("task id range", RN_Type);
      lAppendElem(range_list, range_elem);
      lSetList(job_elem, JB_ja_structure, range_list);
    }
    lSetUlong(range_elem, RN_min, 1);
    lSetUlong(range_elem, RN_max, 1);
    lSetUlong(range_elem, RN_step, 1);

    // Copy list to appropriate JB_ja_?_h_ids field
    range_list = lGetList(job_elem, JB_ja_structure);
    n_h_list = lCopyList("range list", range_list);
    if (hold) {
      lSetList(job_elem, JB_ja_u_h_ids, n_h_list);
      lSetList(job_elem, JB_ja_n_h_ids, NULL);
    } else {
      lSetList(job_elem, JB_ja_n_h_ids, n_h_list);
      lSetList(job_elem, JB_ja_u_h_ids, NULL);
    }
    lSetList(job_elem, JB_ja_o_h_ids, NULL);
    lSetList(job_elem, JB_ja_s_h_ids, NULL);
  }

#ifdef AA
  /* 
   * environment 
   */
  cp = getenv("HOME");
  lSetString(job_elem, JB_sge_o_home, cp);
  cp = getenv("LOGNAME");
  lSetString(job_elem, JB_sge_o_log_name, cp);
  cp = getenv("PATH");
  lSetString(job_elem, JB_sge_o_path, cp);
  cp = getenv("MAIL");
  lSetString(job_elem, JB_sge_o_mail, cp);
  cp = getenv("SHELL");
  lSetString(job_elem, JB_sge_o_shell, cp);
  cp = getenv("TZ");
  lSetString(job_elem, JB_sge_o_tz, cp);
  cp = getenv("WORKDIR");
  lSetString(job_elem, JB_sge_o_workdir, cp);
  cp = getenv("HOST");
  lSetHost(job_elem, JB_sge_o_host, cp);
#endif

  lSetUlong(job_elem, JB_priority, (u_long32)BASE_PRIORITY);  
  
  /* normal user may only request priorities <= 0 */
  /* -p prio switch */
  int priority = 0;
  lSetUlong(job_elem, JB_priority, priority + (u_long32) BASE_PRIORITY);
    
  /* set to 0 if job should be schedules ASAP */
  /* otherwise set time in date = gettimeofday() format */
  /* -a date switch */
  int execution_time = 0;
  lSetUlong(job_elem, JB_execution_time, execution_time);
  
  /* merge stdout/stderr, BOOLEAN (1|0) value */
  /* -j j|n */
  // not used by JAM
  lSetUlong(job_elem, JB_merge_stderr, 0);
  
  /* send signal to job before suspend/delete BOOLEAN (1|0) value */
  /* -notify */
  // not used by JAM
  lSetUlong(job_elem, JB_notify, 0);
  
  /* restart job in case of failure */
  /* 0 = no request (take default from queue */
  /* 1 = request restart */
  /* 2 = no restart even queue has restart enabled */
  lSetUlong(job_elem, JB_restart, 2);

#ifdef AA
  // set force to true
  lSetUlong(job_elem, JB_force, force);
#endif

  /*
   * -cwd - start job from cwd 
   * this directory has to have read/write/execute permitions for
   * all the users
   */
  if (cwd) 
    lSetString(job_elem, JB_cwd, cwd);
  
  /*
   * JB_stdout_path_list     - path for stdout file
   * JB_stderr_path_list     - path for stderr file
   * JB_shell_list           - which shell should be used
   * JB_env_list             - environment variables set for job
   * JB_job_args             - command line arguments for job
   * JB_hard_resource_list   - "-l ..." requests 
   * JB_soft_resource_list   - soft "-l ..." requests (not very common)
   * JB_hard_queue_list      - list of queues requested with "-q"
   * JB_jid_predecessor_list - list of job id's which must finish
   *                           before this job will be scheduled
   */

  // job arguments
  if(args) {
//     printf("creating job argument list\n");
    /* request a list of args */
    lList *lp = 0;
    for(int i = 0; args[i] != 0 ; i++) 
      if (strcmp(args[i]->getValue(), "")) 
        lAddElemStr(&lp, STR, args[i]->getValue(), ST_Type);
    lSetList(job_elem, JB_job_args, lp);
  }

  // job environment variables
  if(envVars) {
//     printf("creating job environment variable list\n");
    /* request a list of environment variables */
    lList* lp = 0;
    lListElem* ep = 0;
    
    for(int i = 0; envVars[i] != 0; i++) {
      ep = lAddElemStr(&lp, VA_variable, envVars[i]->getName(),
                       VA_Type);
      lSetString(ep, VA_value, envVars[i]->getValue());
    }
    lSetList(job_elem, JB_env_list, lp);
  }
  
  // stdout path
  if (stdoutPaths) {
//    lListElem *ep = 0;
    lList *lp = 0;
    for(int i = 0; stdoutPaths[i] != 0; i++) 
      lAddElemStr(&lp, PN_path, stdoutPaths[i]->getPath(), PN_Type);
    /* PN_host does not to be set - valid for all execution hosts */
    lSetList(job_elem, JB_stdout_path_list, lp);
  }
  
  // stderr path
  /*
   * set path for stderr file, if stdout/stderr is merges,
   * file will be stdout file
   */  
  if (stderrPaths) {
    //lListElem *ep = 0;
    lList *lp = 0;
    for(int i = 0; stderrPaths[i] != 0; i++)
      lAddElemStr(&lp,  PN_path, stderrPaths[i]->getPath(), PN_Type);
    /* PN_host does not to be set - valid for all execution hosts */
    lSetList(job_elem, JB_stderr_path_list, lp);
  }          
  
  // shell
  // request shell (override default from queue) 
  if(shells) {
    //lListElem *ep = 0;
    lList *lp = 0;
    for(int i = 0; shells[i] != 0; i++)
      lAddElemStr(&lp,  PN_path, shells[i]->getPath(), PN_Type);
    /* PN_host does not to be set - valid for all execution hosts */
    lSetList(job_elem, JB_shell_list, lp);
  }
  
  // queue
  // request a list of queues 
  if(queueName) {
    lList *lp = 0;
    lAddElemStr(&lp, QR_name, queueName, QR_Type);
    lSetList(job_elem, JB_hard_queue_list, lp);
  }
}
