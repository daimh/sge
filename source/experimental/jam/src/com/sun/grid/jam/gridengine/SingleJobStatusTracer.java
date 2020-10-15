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

import java.rmi.RemoteException;
import net.jini.space.JavaSpace;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.entry.Entry;
import net.jini.core.entry.UnusableEntryException;
import net.jini.core.lease.Lease;
import net.jini.core.lease.LeaseDeniedException;
import net.jini.core.transaction.Transaction;
import net.jini.core.transaction.Transaction.Created;
import net.jini.core.transaction.TransactionFactory;
import net.jini.core.transaction.TransactionException;
import net.jini.core.transaction.server.TransactionManager;

import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobAction;
import com.sun.grid.jam.gridengine.job.*;


/**
 * Monitors a sge job. Stops when the
 * sge job id is not anymore valid.
 * 
 * @version 1.8, 09/22/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
public class SingleJobStatusTracer
  extends Thread
{
  private NativeSGERMAdapter nativeAdapter;
  private JavaSpace space;
  private TransactionManager txnMgr;
  private Entry jControl;
  private SGEJob job;
  private boolean run;

  public SingleJobStatusTracer(NativeSGERMAdapter nativeAdapter,
                               JavaSpace space, JobInfo jobInfo,
                               int jobID, String jobName)
  {
    super();
    this.nativeAdapter = nativeAdapter;
    this.space = space;
    job = new SGEJob(jobName, jobID);
    try {
      jControl = space.snapshot(new JobControlEntry(jobInfo));
    } catch(RemoteException e) {
      e.printStackTrace();
    }
  }

  public SingleJobStatusTracer(NativeSGERMAdapter nativeAdapter,
                               JavaSpace space, JobInfo jobInfo,
                               int jobID, String jobName,
                               TransactionManager txnMgr)
  {
    this(nativeAdapter, space, jobInfo, jobID, jobName);
    this.txnMgr = txnMgr;
  }

  private Transaction createTransaction(long leaseTime)
    throws Exception
  {
    if(txnMgr != null) {
      Transaction.Created trc = null;
      try {
        trc = TransactionFactory.create(txnMgr, leaseTime);
      } catch(LeaseDeniedException lde) {
        throw new Exception(lde.getMessage());
      } catch(RemoteException re) {
        throw new Exception(re.getMessage());
      }
      return trc.transaction;
    }
    return null;
  }
  
  public void run()
  {
    run = true;
    do {
      try {
//          System.out.println("-- Monitoring thread for " + job.getName()
//                             + " ID: " + job.getID() +
//                             " starts sleeping");
        sleep(3000);
//          System.out.println("-- Monitoring thread for " + job.getName()
//                             + " ID: " + job.getID() +
//                             " stops sleeping");

//         System.out.println("-------------");
        
        JobStatus st = null;
        try {
          nativeAdapter.updateJobStat(job);
	  //	  System.out.println("Job "+job.getID()+":\n" +
	  //			     job.getStat().toString());
          st = getNextStatus(job.getStat());
        } catch(NativeSGECommException ncce){
        } catch(NativeSGEException ncrmae) {
          st = JobStatus.COMPLETED;
          terminate();
        }
        Transaction txn = null;
        try {
          txn = createTransaction(5 * 1000);
          JobControlEntry jce =
            (JobControlEntry)space.readIfExists(jControl, null,
                                                JavaSpace.NO_WAIT);
          if((st == null) || (jce == null)) {
            continue;
          }
          if(!st.equals(jce.jobStatus)) {
            jce = (JobControlEntry)
              space.takeIfExists(jControl, txn, JavaSpace.NO_WAIT);
            if(jce == null) {
              if(txn != null)
                txn.abort();
              continue;
            } else {
              jce.jobStatus = st;
              if(jce.jobAction.equals(JobAction.BLOCK)) {
                //To be deleted
                jce.jobAction = JobAction.NONE;
              }
              if(jce.jobStatus.equals(JobStatus.STOPPED)) {
                terminate();
              }
            }
            space.write(jce, txn, Lease.FOREVER);
          }
          if(txn != null)
            txn.commit();
        } catch(Exception e) {
//            if(txn != null)
//              txn.abort();
          e.printStackTrace();
        }
      } catch(InterruptedException ie) {
        ie.printStackTrace();
      }
    } while(run);
  }
  
  public void terminate()
  {
    run = false;
  }

  private JobStatus getNextStatus(JobStat js)
  {
    JobStatus st = JobStatus.UNKNOWN;
    
    switch(js.getStatus()) {

    case JobStat.JIDLE:
      if(js.containsState(JobStat._JHELD) &&
         js.containsState(JobStat._JQUEUED) &&
         js.containsState(JobStat._JWAITING)) {
        st = JobStatus.HELD;
      } else if(js.containsState(JobStat._JSUSPENDED)) {
        st = JobStatus.SUSPENDED;
      } else if(js.containsState(JobStat._JDELETED)) {
        st = JobStatus.STOPPED;
      } else {
        st = JobStatus.IDLE;
      }
      break;  

    case JobStat.JRUNNING:
      if(js.containsState(JobStat._JDELETED)) {
        st = JobStatus.STOPPED;
      }
      else if(js.containsState(JobStat._JSUSPENDED)) {
        st = JobStatus.SUSPENDED;
      }
      else {
        st = JobStatus.RUNNING;
      }
      break;

    case JobStat.JTRANSITING:
      if(js.containsState(JobStat._JDELETED)) {
        st = JobStatus.STOPPED;
      }
      else if(js.containsState(JobStat._JSUSPENDED)) {
        st = JobStatus.SUSPENDED;
      }
      else {
        st = JobStatus.RUNNING;
      }
      break;

    case JobStat.JFINISHED:
      st = JobStatus.COMPLETED;
      break;
    }
    return st;
  }
}

