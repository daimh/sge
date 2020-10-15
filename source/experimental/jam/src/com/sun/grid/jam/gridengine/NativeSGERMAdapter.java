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
package com.sun.grid.jam.gridengine;

import com.sun.grid.jam.gridengine.job.SGEJob;
import com.sun.grid.jam.gridengine.job.JobStat;
import com.sun.grid.jam.gridengine.job.JobStatException;
import com.sun.grid.jam.gridengine.queue.QueueState;
import com.sun.grid.jam.gridengine.queue.SGEQueue;

/**
 * This class provides JAM with functionality to deal with SGE
 * through JNI and SGE GDI.
 *
 * @version 1.9, 09/22/00
 *
 * @author Rafal Piotrowski
 */
public class NativeSGERMAdapter
{
  // load the native library that implements the native methods 
  static 
  {
    System.loadLibrary("sge_rmadapter");
  }

  /**
   * Constructor tries to setup SGE GDI, if un-successful an
   * exception is thrown.
   *
   * @exception NativeSGEException
   */
  public NativeSGERMAdapter()
    throws NativeSGEException
  {
    nativeSGEGDISetup();
  }

  public void sgeGDIShutdown()
  {
    try {
      nativeSGEGDIShutdown();
    } catch(NativeSGEException e){
      //      e.printStackTrace();
    }
  }
  
  //----------------
  // JOB METHODS
  //----------------
  /**
   * If a queue is null then the job will be submitted to a queue
   * master first, and then sheduled to some queue, but if it is set, 
   * the job will be submitted to it and not to any other queue.
   * This method is for START and SUBMIT actions.
   *
   * @param job that should be submitted to RMS
   * @param queue to which the job should be submitted
   *
   * @exception NativeSGEException
   * @exception NativeSGECommException
   *
   */
  public void submitJob(SGEJob job, SGEQueue queue) 
    throws NativeSGEException, NativeSGECommException
  {
    nativeSubmitJob(job,queue);
  }

  // this is for STOP and KILL
  public void deleteJob(SGEJob job)
    throws NativeSGEException, NativeSGECommException
  {
    nativeDeleteJob(job);
  }

  // this is for JobTracer
  // updates job's state & status
  public void updateJobStat(SGEJob job)
    throws NativeSGEException, NativeSGECommException
  {
    try {
      nativeUpdateJobStat(job);
    } catch(JobStatException e) {
//       e.printStackTrace();
      throw new NativeSGEException(e.getMessage());
    }
  }

  /**
   * This method can be used to change job state to the following
   * states: JSUSPENDED, JRUNNING, (see sge.job.JobStat)
   * it is for SUSPEND and RESUME 
   *
   * @param job 
   * @param stat 
   *
   * @exception NativeSGEException
   */
  public void changeJobState(SGEJob job, JobStat stat)
    throws NativeSGEException, NativeSGECommException
  {
    nativeChangeJobState(job, stat);
    job.setStat(stat);
  }

  /**
   * Un hold the job.
   *
   * @param job - Job to be un hold
   *
   * @exception NativeSGEException
   * @exception NativeSGECommException
   */
  public void unHold(SGEJob job)
    throws NativeSGEException, NativeSGECommException
  {
    nativeUnHold(job);
  }
    
  //----------------
  // QUEUE METHODS
  //----------------

  public SGEQueue[] retreiveAllQueues()
    throws NativeSGEException, NativeSGECommException
  {
    return nativeRetreiveAllQueues();
  }
  
  public void addQueue(SGEQueue queue)
    throws NativeSGEException, NativeSGECommException
  {
    nativeAddQueue(queue);
  }

  public void deleteQueue(SGEQueue queue)
    throws NativeSGEException, NativeSGECommException
  {
    nativeDeleteQueue(queue);
  }

  public void changeQueueState(SGEQueue queue, QueueState state)
    throws NativeSGEException, NativeSGECommException
  {
    nativeChangeQueueState(queue, state);
    queue.setState(state);    
  }

  //--------------------
  // native methods
  //--------------------

  // enroll to commd
  private native void nativeSGEGDISetup()
    throws NativeSGEException;

  // leave commd
  private native void nativeSGEGDIShutdown()
    throws NativeSGEException;
  
  // job's native methods
  private native void nativeSubmitJob(SGEJob job,
                                      SGEQueue queue)
    throws NativeSGEException, NativeSGECommException;
  
  private native void nativeUpdateJobStat(SGEJob job)
    throws NativeSGEException, NativeSGECommException, JobStatException;
  
  private native void nativeChangeJobState(SGEJob job,
                                           JobStat stat)
    throws NativeSGEException, NativeSGECommException;

  private native void nativeUnHold(SGEJob job)
    throws NativeSGEException, NativeSGECommException;
  
  private native void nativeDeleteJob(SGEJob job)
    throws NativeSGEException, NativeSGECommException;  

  // queue's native methods
  private native SGEQueue[] nativeRetreiveAllQueues()
    throws NativeSGEException, NativeSGECommException;
  
  private native void nativeAddQueue(SGEQueue queue)
    throws NativeSGEException, NativeSGECommException;
  
  private native void nativeDeleteQueue(SGEQueue queue)
    throws NativeSGEException, NativeSGECommException;
  
  private native void nativeChangeQueueState(SGEQueue queue,
                                             QueueState state)
    throws NativeSGEException, NativeSGECommException;

}
