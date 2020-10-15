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
#include "com_sun_grid_jam_gridengine_NativeSGERMAdapter.h"

#include "utils.h"
#include "StringValue.hpp"
#include "Variable.hpp"
#include "HostPath.hpp"
#include "SGEJob.hpp"
#include "SGEJobJNIUtil.hpp"
#include "SGEExecHost.hpp"
#include "SGEQueue.hpp"
#include "SGERMS.hpp"

#include "sge_gdi.h"

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* external string manipulation functions */
extern char *strdup(const char*);
  
/*================== SETUP & SHUTDOWN Grid Engine GDI ===================*/
  
/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeSGEGDISetup
 * Signature: (V)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeSGEGDISetup
(JNIEnv *env, jobject thisObj)
{
  /* GENERAL SGE SETUP */
  int setup_ok = sge_gdi_setup("Job & Application Manager(JAM)");
  switch(setup_ok) {
  case AE_OK:
    fprintf(stdout, "Grid Engine GDI Setup: ok\n");
    break;
  case  AE_ALREADY_SETUP:
    fprintf(stderr, "Grid Engine GDI Setup: already setup\n");
    throw_exception(env, "Grid Engine GDI Setup: already setup.");
    break;
  case AE_UNKNOWN_PARAM:
    fprintf(stderr, "Grid Engine GDI Setup: unknown parameter\n");
    throw_exception(env, "Grid Engine GDI Setup: unknown parameter.");
    break;
  case AE_QMASTER_DOWN:
    fprintf(stderr, "Grid Engine GDI Setup: qmaster not alive\n");
    throw_exception(env, "Grid Engine GDI Setup: qmaster not alive.");
    break;
  default:
    fprintf(stderr, "Grid Engine GDI Setup: unknown error\n");
    throw_exception(env, "Grid Engine GDI Setup: unknown error.");
    break;
  }
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeSGEGDIShutdown
 * Signature: (V)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeSGEGDIShutdown
(JNIEnv *env, jobject thisObj)
{
  /* GENERAL SGE SHUTDOWN */
  int shutdown_ok = sge_gdi_shutdown();
  switch(shutdown_ok) {
  case AE_OK:
    fprintf(stdout, "Grid Engine GDI Shutdown: ok\n");
    break;
  case AE_QMASTER_DOWN:
    fprintf(stderr, "Grid Engine GDI Shutdown: qmaster not alive\n");
    throw_exception(env, "Grid Engine GDI Shutdown: qmaster not alive.");
    break;
  default:
    fprintf(stderr, "Grid Engine GDI Shutdown: unknown error\n");
    throw_exception(env, "Grid Engine GDI Shutdown: unknown error.");
    break;
  }
}

/*=====================  JOB FUNCTIONS =========================*/
  
/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeSubmitJob
 * Signature: (Lcom/sun/grid/jam/gridengine/job/SGEJob;Lcom/sun/jam/sge/queue/SGEQueue;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeSubmitJob(
  JNIEnv *env, jobject thisObj, jobject job, jobject queue)
{
  char *jobName = 0; /* job name */
  char *scriptFile = 0; /* path with script file name */
  char *cwd = 0; /* common working directory */
  char *queueName = 0; /* queue name to which to submit job */

  SGEJobJNIUtil* jobUtil = new SGEJobJNIUtil(env);
  StringValue** jobArgs = 0;
  Variable** jobEnvVars = 0;
  HostPath** stdoutPaths = 0;
  HostPath** stderrPaths = 0;
  HostPath** shells = 0;
  
  /* get pointer to the job to be submitted */
  if(job) {
    /* SGEJob's name field */
    jobName = getStringField(env, job, "name");
    if(!jobName) {
      throw_exception(env, "Job submition failed : job's name is not set.");
      return;
    }
    
    /* SGEJob's script_file field */
    scriptFile = getStringField(env, job, "scriptFile");
    if(!scriptFile) {
      throw_exception(env, "Job submition failed : job's script_ile is not set.");
      return;
    }
    
    /* SGEJob's arguments field */
    jobArgs = jobUtil->getJobArgs(job);

    jobEnvVars = jobUtil->getJobEnvVars(job);

    /* SGEJob's stdout_path field */
    stdoutPaths = jobUtil->getJobStdoutPaths(job);
    if(!stdoutPaths) {
      throw_exception(env, "Job submition failed : stdout path isn't set.");
      return;
    }    

    /* SGEJob's stderr_path field */
    stderrPaths = jobUtil->getJobStderrPaths(job);
    if(!stderrPaths) {
      throw_exception(env, "Job submition failed : stderr path isn't set.");
      return;
    } 
    
    /* SGEJob's cwd field */
    cwd = getStringField(env, job, "cwd");
    if(!cwd) {
      throw_exception(env, "Job submition failed : job's cwd is not set.");
      return;
    }
    
  } else {
    throw_exception(env, "Job submition failed : Reference to SGEJob is null.");
    return;
  }
  
  /*
   * get pointer to the queue on which job has to submitted
   * it it is not set then the job is submitted into what ever queue
   * SGE decides to submit it
   */
  queueName = getStringField(env, queue, "name");
  
  SGEJob *sgeJob = new SGEJob(env,
                                       jobName, scriptFile,
                                       jobArgs, jobEnvVars,
                                       stdoutPaths, stderrPaths,
                                       shells, cwd,
                                       queueName);
  sgeJob->submit();

  if(env->ExceptionCheck() == JNI_TRUE) {
    delete sgeJob;
    return;
  }
  
  sgeJob->freeSGEVars();
  sgeJob->resolveId();

  if(env->ExceptionCheck() == JNI_TRUE) {
    delete sgeJob;
    return;
  }

  int job_id = sgeJob->getId();
  delete sgeJob;

  int ret = 0;
  ret = callSetIntMethod(env, job, "setID", job_id);
  if(!ret)
    throw_exception(env, "Update job id failed : error executing setID method.");
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeDeleteJob
 * Signature: (Lcom/sun/grid/jam/gridengine/job/SGEJob;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeDeleteJob(
  JNIEnv *env, jobject thisObj, jobject job)
{
  char* job_name;
  int id = -1;
  
  /* get pointer to the job to be deleted */
  job_name = getStringField(env, job, "name");
  if(!job_name) 
    job_name = "N/A";
  
  id = getIntField(env, job, "id");
  if(id == -1) {
    throw_exception(env, "Delete job failed : unknown job id.");
    return;
  }

  SGEJob* sgeJob = new SGEJob(env, job_name, id);
  sgeJob->remove();
  delete sgeJob;
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeUpdateJobStat
 * Signature: (Lcom/sun/grid/jam/gridengine/job/SGEJob;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeUpdateJobStat(
  JNIEnv *env, jobject thisObj, jobject job)
{
  int job_id = -1;
  int job_state;
  int job_status;
  char *job_name = 0;

  /* get pointer to the job */
  job_name = getStringField(env, job, "name");
  if(!job_name)
    job_name = "N/A";
  
  /* SGEJob's ID field */
  job_id = getIntField(env, job, "id");
  if(job_id == -1) {
    throw_exception(env, "Update job state & status failed : incorrect job id");
    return;
  }

  SGEJob *sgeJob = new SGEJob(env, job_name, job_id);
  sgeJob->updateStat();
  
  if(env->ExceptionCheck() == JNI_TRUE) {
    delete sgeJob;
    return;
  }
  
  job_state = sgeJob->getState();
  job_status = sgeJob->getStatus();
  delete sgeJob;

  // update job's state & status
  
  /* SGEJob's state field */
  jobject statObj = getObjectField(env, job,
                                   "stat",
                                   "Lcom/sun/grid/jam/gridengine/job/JobStat;");
  if(!statObj) {
    throw_exception(env, "Update job state failed : Variable field \"stat\" not found in SGEJob.");
    return;
  }

  int ret = 0;
  ret = callSetIntMethod(env, statObj, "setState", job_state);
  if(!ret) { // error occoured 
    if(env->ExceptionCheck() != JNI_TRUE) 
      throw_exception(env, "Update job state failed : error calling JobStat.setState method.");
    return;
  }

  ret = 0;
  ret = callSetIntMethod(env, statObj, "setStatus", job_status);
  if(!ret) { // error occoured 
    if(env->ExceptionCheck() != JNI_TRUE) 
      throw_exception(env, "Update job state failed : error calling JobStat.setStatus method.");
    return;
  }
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeChangeJobState
 * Signature: (Lcom/sun/grid/jam/gridengine/job/SGEJob;Lcom/sun/jam/sge/job/JobStat;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeChangeJobState(
  JNIEnv *env, jobject thisObj, jobject job, jobject stat)
{
  char *job_name = 0;
  int id = -1;
  int job_state = -1;
    
  job_name = getStringField(env, job, "name");
  if(!job_name)
    job_name = "N/A";

  /* SGEJob's ID field */
  id = getIntField(env, job, "id");
  if(id == -1) {
    throw_exception(env, "Change job state failed : invalid job id.");
    return;
  }

  job_state = getIntField(env, stat, "state");
  if( job_state == -1 ) {
    throw_exception(env, "Change job state failed : invalid job state.");
    return;
  }

  SGEJob* sgeJob = new SGEJob(env, job_name, id);
  sgeJob->changeState(job_state);
  delete sgeJob;
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeUnHold
 * Signature: (Lcom/sun/grid/jam/gridengine/job/SGEJob;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeUnHold(
  JNIEnv *env, jobject thisObj, jobject job)
{
  char* name = 0;
  int id = -1;

  name = getStringField(env, job, "name");
  if(!name)
    name = "N/A";

  id = getIntField(env, job, "id");
  if(id == -1){
    throw_exception(env, "Invalid job id.");
    return;
  }

  SGEJob* sgeJob = new SGEJob(env, name, id);
  sgeJob->unSetHold();
  delete sgeJob;
}

/*====================  QUEUE FUNCTIONS  ========================*/
/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeRetreiveAllQueues
 * Signature: ()[Lcom/sun/grid/jam/gridengine/queue/SGEQueue;
 */
JNIEXPORT jobjectArray JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeRetreiveAllQueues(
  JNIEnv* env, jobject thisObj)
{
  // JNI vars
  jclass jclassSGEQueue = 0;
  jclass jclassSGEExecHost = 0;
  
  jobject jobjectSGEQueue = 0;
  jobject jobjectSGEExecHost = 0;
  
  jobjectArray jobjectArraySGEQueue = 0;

  jmethodID jmethodIDSGEQueue = 0;
  jmethodID jmethodIDSGEQueueSetName = 0;
  jmethodID jmethodIDSGEQueueSetHostname = 0;
  jmethodID jmethodIDSGEQueueSetType = 0;
  jmethodID jmethodIDSGEQueueSetJobSlots = 0;
  jmethodID jmethodIDSGEQueueSetState = 0;
  jmethodID jmethodIDSGEQueueSetTmpDir = 0;
  jmethodID jmethodIDSGEQueueSetShell = 0;
  jmethodID jmethodIDSGEQueueSetExecHost = 0;

  jmethodID jmethodIDSGEExecHost = 0;
  jmethodID jmethodIDSGEExecHostSetName = 0;
  jmethodID jmethodIDSGEExecHostSetArch = 0;
  jmethodID jmethodIDSGEExecHostSetProcessors = 0;
  jmethodID jmethodIDSGEExecHostSetLoad = 0;
    
  jstring jstringName = 0;
  jstring jstringHostname = 0;
  jstring jstringTmpDir = 0;
  jstring jstringShell = 0;

  jstring jstringArch = 0;
  jstring jstringLoad = 0;

  // MISC vars
  int type = 0;
  int state = 0;
  long jobSlots = 0;
  long seqNo = 0;
  int processors = 0;
  
  // JAM SGE vars 
  SGERMS* rms = 0;

  SGEQueue** allQueues = 0;
  SGEExecHost* execHost = 0;
  
  //======== function =====
  
  rms = new SGERMS(env);
  if(rms)
    allQueues = rms->retreiveAllQueues();
  if(allQueues) {
    int length = 0;
    for(int i = 0; allQueues[i] != 0; i++)
      length++;
//     printf("Retreived %d queues\n", length);


    // find SGEQueue class
    jclassSGEQueue =
      env->FindClass("com/sun/grid/jam/gridengine/queue/SGEQueue");
    if(!jclassSGEQueue){
      printf("error: SGEQueue Java class not found\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    // get the init method of SGEQueue class
    jmethodIDSGEQueue =
      env->GetMethodID(jclassSGEQueue, "<init>", "()V");
    if(!jmethodIDSGEQueue) {
      printf("error: SGEQueue constructor not found\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jobjectSGEQueue =
      env->NewObject(jclassSGEQueue, jmethodIDSGEQueue);
    if(!jobjectSGEQueue){
      printf("error: allocating new Java object of SGEQueue class\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    // create array of SGEQueue Java objects
    jobjectArraySGEQueue =
      env->NewObjectArray(length,
                          jclassSGEQueue,
                          jobjectSGEQueue);
    if(!jobjectArraySGEQueue){
      printf("error: allocating new SGEQueue object array\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetName =
      env->GetMethodID(jclassSGEQueue, "setName",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEQueueSetName){
      printf("error: can't find method setName\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jmethodIDSGEQueueSetHostname =
      env->GetMethodID(jclassSGEQueue, "setHostName",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEQueueSetHostname){
      printf("error: can't find method setHostName\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetType =
      env->GetMethodID(jclassSGEQueue, "setType", "(I)V");
    if(!jmethodIDSGEQueueSetType){
      printf("error: can't find method setType\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetJobSlots =
      env->GetMethodID(jclassSGEQueue, "setJobSlots", "(J)V");
    if(!jmethodIDSGEQueueSetJobSlots){
      printf("error: can't find method setJobSlots\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetState =
      env->GetMethodID(jclassSGEQueue, "setState", "(I)V");
    if(!jmethodIDSGEQueueSetState){
      printf("error: can't find method setState\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetTmpDir =
      env->GetMethodID(jclassSGEQueue, "setTmpDir",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEQueueSetTmpDir){
      printf("error: can't find method setTmpDir\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEQueueSetShell =
      env->GetMethodID(jclassSGEQueue, "setShell",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEQueueSetShell){
      printf("error: can't find method setShell\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jmethodIDSGEQueueSetExecHost =
      env->GetMethodID(jclassSGEQueue, "setExecHost",
                       "(Lcom/sun/grid/jam/gridengine/exechost/SGEExecHost;)V");
    if(!jmethodIDSGEQueueSetExecHost){
      printf("error: can't find method setExecHost\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    // find SGEExecHost class
    jclassSGEExecHost =
      env->FindClass("com/sun/grid/jam/gridengine/exechost/SGEExecHost");
    if(!jclassSGEExecHost){
      printf("error: SGEExecHost Java class not found\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    // get the init method of SGEExecHost class
    jmethodIDSGEExecHost =
      env->GetMethodID(jclassSGEExecHost, "<init>", "()V");
    if(!jmethodIDSGEExecHost) {
      printf("error: SGEExecHost constructor not found\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }
    
    jmethodIDSGEExecHostSetName =
      env->GetMethodID(jclassSGEExecHost, "setName",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEExecHostSetName){
      printf("error: can't find method setName\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jmethodIDSGEExecHostSetArch =
      env->GetMethodID(jclassSGEExecHost, "setArch",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEExecHostSetArch){
      printf("error: can't find method setArch\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jmethodIDSGEExecHostSetLoad =
      env->GetMethodID(jclassSGEExecHost, "setLoad",
                       "(Ljava/lang/String;)V");
    if(!jmethodIDSGEExecHostSetLoad){
      printf("error: can't find method setLoad\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

    jmethodIDSGEExecHostSetProcessors =
      env->GetMethodID(jclassSGEExecHost, "setProcessors",
                       "(I)V");
    if(!jmethodIDSGEExecHostSetProcessors){
      printf("error: can't find method setProcessors\n");
      delete[] allQueues;
      delete rms;
      return 0;
    }

//     printf("creating java object array....");
    // loop through the array of allQueues and add them to jarraySGEQueue
    for(int i = 0; i < length; i++){
      // get the C++ queue's execution host
      execHost = 0;
      execHost = allQueues[i]->getExecHost();
      
 //      printf("Queue %s:\n", allQueues[i]->getName());
// //       printf("\thostname: %s\n", allQueues[i]->getHostname());
//       printf("\ttype: %lx\n", allQueues[i]->getType());
//       printf("\tjob slots: %ld\n", allQueues[i]->getJobSlots());
//       printf("\tstate: Hex(%lx), Dec(%ld)\n",
//              allQueues[i]->getState(), allQueues[i]->getState());
//       printf("\tshell: %s\n", allQueues[i]->getShell());
//       printf("\ttmp dir: %s\n", allQueues[i]->getTmpDir());
//       printf("\tseq no: %ld\n", allQueues[i]->getSeqNo());
//       printf("\tExecution Host %s:\n", execHost->getName());
//       printf("\t\tarch: %s\n", execHost->getArch());
//       printf("\t\tprocessors: %ld\n", execHost->getProcessors());
//       printf("\t\tload: %s\n", execHost->getLoad());      
      
      // create new Java object that represent SGEQueue
      // this object will be inserted into the array of SGE Queues, 
      jobjectSGEQueue = 0;
      jobjectSGEQueue =
        env->NewObject(jclassSGEQueue, jmethodIDSGEQueue);
      if(!jobjectSGEQueue){
        printf("error: allocating new Java object of SGEQueue class\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      
      // set queue's name
      jstringName = 0;
      jstringName =
        (jstring)env->NewStringUTF(allQueues[i]->getName());
      if(!jstringName){
        printf("error: can't create java string for name\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetName,
                          jstringName);

      // set queue's hostname
      jstringHostname = 0;
      jstringHostname =
        (jstring)env->NewStringUTF(allQueues[i]->getHostname());
      if(!jstringHostname){
        printf("error: can't create java string for hostname\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetHostname,
                          jstringHostname);
      
      // set queue's tem dir
      jstringTmpDir = 0;
      jstringTmpDir =
        (jstring)env->NewStringUTF(allQueues[i]->getTmpDir());
      if(!jstringTmpDir){
        printf("error: can't create java string for tmpdir\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetTmpDir,
                          jstringTmpDir);
      
      // set queue's shell
      jstringShell = 0;
      jstringShell =
        (jstring)env->NewStringUTF(allQueues[i]->getShell());
      if(!jstringShell){
        printf("error: can't create java string for shell\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetShell,
                          jstringShell);

      // set queue's type
      type = 0;
      type = allQueues[i]->getType();
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetType,
                          type);

      // set queue's state
      state = 0;
      state = allQueues[i]->getState();
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetState,
                          state);

      // set queue's sequence no
      seqNo = 0;
      seqNo = allQueues[i]->getSeqNo();
      // method needed to set seq no

      // set queue's no. of job slots
      jobSlots = 0;
      jobSlots = allQueues[i]->getJobSlots();
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetJobSlots,
                          (jlong)jobSlots);

      
      // create new object that represent Java queue's execution host
      jobjectSGEExecHost = 0;
      jobjectSGEExecHost =
        env->NewObject(jclassSGEExecHost, jmethodIDSGEExecHost);
      if(!jobjectSGEExecHost){
        printf("error: allocating new Java object of SGEExecHost class\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }

      // set execution host name
      jstringName = 0;
      jstringName =
        (jstring)env->NewStringUTF(execHost->getName());
      if(!jstringName){
        printf("error: can't create java string for execution host name\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEExecHost, jmethodIDSGEExecHostSetName,
                          jstringName);

      // set execution host architecture
      jstringArch = 0;
      jstringArch =
        (jstring)env->NewStringUTF(execHost->getArch());
      if(!jstringArch){
        printf("error: can't create java string for execution host arch\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEExecHost, jmethodIDSGEExecHostSetArch,
                          jstringArch);

      // set execution host load
      jstringLoad = 0;
      jstringLoad =
        (jstring)env->NewStringUTF(execHost->getLoad());
      if(!jstringLoad){
        printf("error: can't create java string for execution host load\n");
        delete[] allQueues;
        delete rms;
        return 0;
      }
      env->CallVoidMethod(jobjectSGEExecHost, jmethodIDSGEExecHostSetLoad,
                          jstringLoad);

      // set execution host load
      processors = 0;
      processors = execHost->getProcessors();
      env->CallVoidMethod(jobjectSGEExecHost, jmethodIDSGEExecHostSetProcessors,
                          processors);
      

      // set queue's execution host 
      env->CallVoidMethod(jobjectSGEQueue, jmethodIDSGEQueueSetExecHost,
                          jobjectSGEExecHost);
      
      // set array element
      env->SetObjectArrayElement(jobjectArraySGEQueue, i,
                                 jobjectSGEQueue);
    }
    
    delete[] allQueues;
    delete rms;
    return jobjectArraySGEQueue;
  }
  
  delete rms;
  return 0;
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeAddQueue
 * Signature: (Lcom/sun/grid/jam/gridengine/queue/SGEQueue;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeAddQueue(
  JNIEnv *env, jobject obj, jobject queue)
{
  /* MISC. variables */
  char *name;
  char *hostname;
  char *initial_state;
  char *tmpDir;
  char *shell;
  int  type = -1;
  long seqNo = -1;
  long jobSlots = -1;  
  
  /* get pointer to the queue on which job has to submitted */
  if(queue) {
    /* SGEQueue's name field */
    name = getStringField(env, queue, "name");
    if(!name) {
      throw_exception(env, "Add queue failed : queue name is not set.");
      return;
    }
    
    /* SGEQueue's qhostname field */
    hostname = getStringField(env, queue, "hostname");
    if(!hostname) {
      throw_exception(env, "Add queue failed : queue's hostname is not set.");
      return;
    } 
    
    /* SGEQueue's qstate field */
    
    jobject qstate_obj = getObjectField(env, queue,
                                        "state",
                                        "Lcom/sun/grid/jam/gridengine/queue/QueueState;");
    if(qstate_obj) {
      /* SGEState's initial_state field */
      initial_state = getStringField(env, qstate_obj, "initial_state");
      if(!initial_state) {
        throw_exception(env, "Add queue failed : queue's initial_state isn't set.");
        return;
      }
    }
    
    /* SGEQueue's qtype field */
    jobject qtype_obj = getObjectField(env,
                                       queue,
                                       "type",
                                       "Lcom/sun/grid/jam/gridengine/queue/QueueType;");
    if(qtype_obj) {
      /* SGEType's type field */
      type = getIntField(env, qtype_obj, "type");
      if(type == -1) {
        throw_exception(env, "Add queue failed : queue's type is not set.");
        return;
      }
    }
    
    /* SGEQueue's tmpdir field */
    tmpDir = getStringField(env, queue, "tmpDir");
    if(!tmpDir) {
      throw_exception(env, "Add queue failed : queue's tmpdir is not set.");
      return;
    } 
    
    /* SGEQueue's shell field */
    shell = getStringField(env, queue, "shell");
    if(!shell) {
      throw_exception(env, "Add queue failed : queue's shell is not set.");
      return;
    }
    
    /* SGEQueue's seq_no field */
    seqNo = getLongField(env, queue, "seqNo");
    if(seqNo == -1) 
      seqNo = 0;
    
    /* SGEQueue's job_slots field */
    jobSlots = getLongField(env, queue, "jobSlots");
    if(jobSlots == -1) {
      throw_exception(env, "Add queue failed : queue's job_slots is not set.");
      return;
    }
    
  } else 
    throw_exception(env, "Add queue failed : Reference to SGEQueue is null.");
  
  if(env->ExceptionCheck() == JNI_TRUE)
    return;  
  
  SGEQueue *sgeQueue = new SGEQueue(env,
                                             name, hostname,
                                             type, jobSlots,
                                             initial_state, shell,
                                             tmpDir, seqNo
    );
  sgeQueue->add();
  delete sgeQueue;
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeDeleteQueue
 * Signature: (Lcom/sun/grid/jam/gridengine/queue/SGEQueue;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeDeleteQueue(
  JNIEnv *env, jobject obj, jobject queue)
{
  char *name;

  /* get pointer to the queue on which job has to submitted */
  name = getStringField(env, queue, "name");
  if(!name) {
    throw_exception(env, "Delete queue failed : queue name isn't set.");
    return;
  }

  SGEQueue *sgeQueue = new SGEQueue(env, name);
  sgeQueue->remove();
  delete sgeQueue;
}

/*
 * Class:     com_sun_grid_jam_gridengine_NativeSGERMAdapter
 * Method:    nativeChangeQueueState
 * Signature: (Lcom/sun/grid/jam/gridengine/queue/SGEQueue;Lcom/sun/jam/sge/queue/QueueState;)V
 */
JNIEXPORT void JNICALL
Java_com_sun_grid_jam_gridengine_NativeSGERMAdapter_nativeChangeQueueState(
  JNIEnv *env, jobject obj, jobject queue, jobject state)
{
  /* MISC. variables */
  char *name = 0;
  int qstate = -1;

  /* get pointer to the queue on which job has to submitted */
  name = getStringField(env, queue, "name");
  if(!name) {
    throw_exception(env, "Set queue state failed : queue name is not set.");
    return;
  }
  
  /* get pointer to the state to which queue has to change */
  qstate = getIntField(env, state, "state");
  if(qstate == -1) {
    throw_exception(env, "Set queue state failed : invalid state.");
    return;
  }
  
  SGEQueue *sgeQueue = new SGEQueue(env, name);
  sgeQueue->changeState(qstate);
  delete sgeQueue;
}

#ifdef __cplusplus
}
#endif
