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

import java.io.*;
import java.net.URL;
import java.util.ArrayList;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import net.jini.space.JavaSpace;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.entry.Entry;
import net.jini.core.lookup.ServiceID;
import net.jini.core.transaction.Transaction;
import net.jini.core.transaction.Transaction.Created;
import net.jini.core.transaction.TransactionFactory;
import net.jini.core.transaction.TransactionException;
import net.jini.core.transaction.server.TransactionManager;
import net.jini.core.lease.Lease;
import net.jini.core.lease.LeaseDeniedException;

import com.sun.grid.jam.job.JobType;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobAction;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.gridengine.job.SGEJob;
import com.sun.grid.jam.gridengine.job.JobStat;
import com.sun.grid.jam.gridengine.job.JobStatException;
import com.sun.grid.jam.gridengine.queue.*;
import com.sun.grid.jam.gridengine.util.*;

public class RMAdapter
{
  private NativeSGERMAdapter nativeSGERMAdapter = null;
  private JavaSpace space;
  private TransactionManager txnMgr;
  private SubmissionListener submissionListener;
  private ResumeRequestListener resumeListener;
  private SuspendRequestListener suspendListener;
  private RestartRequestListener restartListener;
  private StopRequestListener stopListener;
  private StartRequestListener startListener;
  private KilledJobListener killedJobListener;
  
  private Entry submitAction;
  private Entry suspendAction;
  private Entry resumeAction;
  private Entry restartAction;
  private Entry startAction;
  private Entry stopAction;
  private Entry killedStatus;

  private String cwd;
  private Vector leaseList;
  private Hashtable tracerTable;
  private int jobCount;

  /**
   * Handle suspend requests.
   *
   */
  class SuspendRequestListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public SuspendRequestListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      changeJobState(suspendAction);
    }
  }

  /**
   * Handle resume requests.
   *
   */
  class ResumeRequestListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public ResumeRequestListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      changeJobState(resumeAction);
    }
  }
  
  class RestartRequestListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public RestartRequestListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      // to be updated
    }
  }

  class StartRequestListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public StartRequestListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      start(startAction);
    }
  }
  
  class StopRequestListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public StopRequestListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      delete(stopAction);
    }
  }
  
  /**
   * Listens for killed jobs. Deletes all job's entries
   * from the JobRepository and stops observing threads
   * 
   *
   */
  class KilledJobListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {
    
    public KilledJobListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      cleanJobResource(killedStatus);
    }
  }
  
  /**
   * Handles submission requests.
   *
   */
  class SubmissionListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {

    public SubmissionListener()
      throws RemoteException
    {
      super();
    }
    
    public void notify(RemoteEvent e)
    {
      submit(submitAction);
    }
  }
  
  public RMAdapter(JavaSpace space)
    throws TransactionException, RemoteException, NativeSGEException
  {
    cwd = System.getProperty("com.sun.grid.jam.gridengine.cwd");
    this.space = space;
    leaseList = new Vector();
    tracerTable = new Hashtable();
    jobCount = 0;
    SGEQueue[] queues;

    nativeSGERMAdapter = new NativeSGERMAdapter();
    try {
      queues = retreiveAllQueues();
    } catch(NativeSGEException e){
      System.out.println("--RMAdapter--: Failed to retreive queues from RMS");
      // Probably need to do something smarter than returning here
      return;
    }

    for(int i = 0; i < queues.length; i++) {
      // Add queue-specific JobRepository listeners
      String qname = queues[i].getName();
      submitAction = space.snapshot(new
				    JobControlEntry(JobAction.SUBMIT, qname));
      suspendAction = space.snapshot(new
				     JobControlEntry(JobAction.SUSPEND, qname));
      resumeAction = space.snapshot(new
				    JobControlEntry(JobAction.RESUME, qname));
      restartAction = space.snapshot(new
				     JobControlEntry(JobAction.RESTART, qname));
      stopAction = space.snapshot(new
                                  JobControlEntry(JobAction.STOP, qname));
      startAction = space.snapshot(new
				   JobControlEntry(JobAction.START, qname));
      killedStatus = space.snapshot(new
				    JobControlEntry(JobStatus.KILLED, qname));
      submissionListener = new SubmissionListener();
      suspendListener = new SuspendRequestListener();
      resumeListener = new ResumeRequestListener();
      restartListener = new RestartRequestListener();
      startListener = new StartRequestListener();
      stopListener = new StopRequestListener();
      killedJobListener = new KilledJobListener();
      leaseList.add(space.notify(submitAction, null, submissionListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(suspendAction, null, suspendListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(resumeAction, null, resumeListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(restartAction, null, restartListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(startAction, null, startListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(stopAction, null, stopListener,
				 Lease.FOREVER, null).getLease());
      leaseList.add(space.notify(killedStatus, null, killedJobListener,
				 Lease.FOREVER, null).getLease());
    }
  }

  public RMAdapter(JavaSpace space, TransactionManager txnMgr)
    throws TransactionException, RemoteException, NativeSGEException
  {
    this(space);
    this.txnMgr = txnMgr;
  }
  
  public SGEQueue[] retreiveAllQueues()
    throws NativeSGEException
  {
    return nativeSGERMAdapter.retreiveAllQueues();
  }

  private Transaction createTransaction(long leaseTime)
    throws Exception
  {
    Transaction.Created trc = null;
    if(txnMgr != null) {
      try {
        trc = TransactionFactory.create(txnMgr, leaseTime);
      } catch(LeaseDeniedException lde) {
        throw new Exception(lde.getMessage());
      } catch(RemoteException re) {
        throw new Exception(re.getMessage());
      }
    } else {
      throw new Exception("TransactionManager is null");
    }
    return trc.transaction;
  }

  private void changeJobState(Entry entry)
  {
    JobControlEntry jce;
    Transaction txn;
    try {
      txn = createTransaction(5 * 1000);
      jce = (JobControlEntry)
        space.takeIfExists(entry, txn, JavaSpace.NO_WAIT);
      JobStat stat = null;
      try {
        switch(jce.jobAction.intValue()) {
          
        case JobAction._SUSPEND:
          stat = new JobStat(JobStat.JSUSPENDED,
                             JobStat.JRUNNING);
          break;
          
        case JobAction._RESUME:
          stat = new JobStat(JobStat.JRUNNING,
                             JobStat.JRUNNING);
          break;
          
        default:
          txn.abort();
          return;
        }
      } catch(JobStatException jse) {
        txn.abort();
        return;
      }
      try {
        JobParams jp = (JobParams)
          space.readIfExists(new JobParams(jce.jobInfo),
                             txn, JavaSpace.NO_WAIT);
        if(jp != null) {
          SGEJob jcod = new
            SGEJob(jp.submissionName, jp.submissionID.intValue());
          jce.jobAction = JobAction.BLOCK;
          space.write(jce, txn, Lease.FOREVER);
          nativeSGERMAdapter.changeJobState(jcod, stat);
        } 
      } catch(Exception ex) {
        ex.printStackTrace();
        txn.abort();
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("--RMAdapter--: ChangeJobState failed" + e);
    }
  }
  
  private void submit(Entry entry)
  {
    JobControlEntry jce;
    Transaction txn;
    try {
      txn = createTransaction(5 * 1000);
      try {
        jce = (JobControlEntry)
          space.takeIfExists(entry, txn, JavaSpace.NO_WAIT);
        JobInfo ji = jce.jobInfo;
        JobParams jp = (JobParams)
          space.takeIfExists(new JobParams(ji),
                     txn, JavaSpace.NO_WAIT);
        if(jp != null) {
          if(jp.submissionID != null) {
            deleteFiles(jp);
          }
          String subName = ji.name.substring(ji.name.lastIndexOf('/') + 1);
          subName = "jam_" + jobCount + "_" + ji.userID + "_" + subName;
          String script = cwd + "/" + subName;
          BufferedWriter writer = new BufferedWriter(new
            FileWriter(script));
          writer.write(jp.script, 0, jp.script.length());
          writer.flush();
          writer.close();
          jp.jobID = new Integer(jobCount);
          jp.input = new URL("file:" + script + ".in");
          jp.output =  new URL("file:" + script + ".out");
          jp.error = new URL("file:" + script + ".err");
          jp.submissionName = subName;
          jp.submissionID = new Integer(nativeSubmission(jp));
          jce.jobAction = JobAction.NONE;
          SingleJobStatusTracer tracer = new
            SingleJobStatusTracer(nativeSGERMAdapter, space,
                                  ji, jp.submissionID.intValue(),
                                  jp.submissionName);
//            SingleJobStatusTracer tracer = new
//              SingleJobStatusTracer(nativeSGERMAdapter, space,
//                                    ji, jp.submissionID.intValue(),
//                                    jp.submissionName, txnMgr);
          tracer.start();
          tracerTable.put(jp.jobID, tracer);
          jobCount++;
          space.write(jp, txn, Lease.FOREVER);
          space.write(jce, txn, Lease.FOREVER);
        }
      } catch(Exception ex) {
        ex.printStackTrace();
        txn.abort();
        return;
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("-- RMAdapter --:Exception in submitting job: "
                           + jobCount);
      e.printStackTrace();
    }
  }

  private void deleteFiles(JobParams jp)
  {
    File tmp = new File(cwd + "/" + jp.submissionName);
    if(tmp.exists())
      tmp.delete();
    tmp = new File(jp.output.getFile());
    if(tmp.exists())
      tmp.delete();
    tmp = new File(jp.error.getFile());
    if(tmp.exists())
      tmp.delete();
    tmp = new File(jp.input.getFile());
    if(tmp.exists())
      tmp.delete();
  }
  
  private int nativeSubmission(JobParams jp)
    throws Exception
  {
    SGEJob jcod = new SGEJob(jp.submissionName, null, cwd);
    if(jp.args != null && jp.args.length > 0) {
      for(int i = 0; i < jp.args.length; i ++)
        jcod.addArg(jp.args[i]);
    }
    jcod.addStdoutPath(new HostPath(jp.output.getFile()));
    jcod.addStderrPath(new HostPath(jp.error.getFile()));
    if(jp.jobInfo.queueName.equals(SGEQueue.QMASTER_NAME)) {
      nativeSGERMAdapter.submitJob(jcod, null);
    } else {
      SGEQueue queue = new SGEQueue(jp.jobInfo.queueName);
      nativeSGERMAdapter.submitJob(jcod, queue);
    }
    return jcod.getID();
  }

  private void start(Entry entry)
  {
    JobControlEntry jce;
    Transaction txn;
    try {
      txn = createTransaction(5 * 1000);
      try {
        jce = (JobControlEntry)
          space.takeIfExists(entry, txn, JavaSpace.NO_WAIT);
        JobParams jp = (JobParams)
          space.readIfExists(new JobParams(jce.jobInfo),
                             txn, JavaSpace.NO_WAIT);
        if(jp != null) {
          SGEJob jcod = new SGEJob(jp.submissionName,
                                         jp.submissionID.intValue());
          nativeSGERMAdapter.unHold(jcod);
          jce.jobAction = JobAction.BLOCK;
          space.write(jce, txn, Lease.FOREVER);
        }
      } catch(Exception ex) {
        ex.printStackTrace();
        txn.abort();
        return;
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("--RMAdapter: ChangeJobState failed" + e);
      e.printStackTrace();
    }
  }

  private void delete(Entry entry)
  {
    JobControlEntry jce;
    Transaction txn;
    try {
      txn = createTransaction(5 * 1000);
      try {
        jce = (JobControlEntry)
          space.takeIfExists(entry, txn, JavaSpace.NO_WAIT);
        JobParams jp = (JobParams)
          space.readIfExists(new JobParams(jce.jobInfo),
                             txn, JavaSpace.NO_WAIT);
        if(jp != null) {
          SGEJob jcod = new SGEJob(jp.submissionName,
                                         jp.submissionID.intValue());
          nativeSGERMAdapter.deleteJob(jcod);
          jce.jobAction = JobAction.BLOCK;
          space.write(jce, txn, Lease.FOREVER);
        }
      } catch(Exception ex) {
        ex.printStackTrace();
        txn.abort();
        return;
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("--RMAdapter: ChangeJobState failed" + e);
      e.printStackTrace();
    }
  }

  private void cleanJobResource(Entry entry)
  {
    JobControlEntry jce;
    Transaction txn;
    try {
      txn = createTransaction(5 * 1000);
      try {
        jce = (JobControlEntry)
          space.takeIfExists(entry, txn, JavaSpace.NO_WAIT);
        if(jce != null && jce.jobInfo != null) {
          JobParams jp = (JobParams)
            space.takeIfExists(new JobParams(jce.jobInfo),
                               txn, JavaSpace.NO_WAIT);
          if(jp != null) {
            new File(jp.output.getFile()).delete();
            new File(jp.error.getFile()).delete();
            new File(cwd + "//" + jp.submissionName).delete();
            SingleJobStatusTracer tr = (SingleJobStatusTracer)
              tracerTable.remove(jp.jobID);
            if(tr != null) {
              tr.terminate();
            }
            SGEJob codJob = new SGEJob(jp.submissionName,
                                             jp.submissionID.intValue());
            try {
              nativeSGERMAdapter.deleteJob(codJob);
            } catch(NativeSGECommException ncce) {
            } catch(NativeSGEException ncrmae) {
            }
            space.takeIfExists(jce.jobInfo,
                               txn, JavaSpace.NO_WAIT);
          } else{
            System.out.println("--RMAdapter: Killing aborted JP is null");
            space.write(jce, txn, Lease.FOREVER);
            txn.commit();
            return;
          }
        } else {
          System.out.println("--RMAdapter--: Killing aborted JCE is null");
          txn.abort();
          return;
        }
      } catch(Exception ex) {
        ex.printStackTrace();
        txn.abort();
        return;
      }
      txn.commit();
    } catch(Exception ex) {
      ex.printStackTrace();
    }
  }
    
  public void shutdown()
  {
    try {
      for(Enumeration e = tracerTable.keys(); e.hasMoreElements();) {
        ((SingleJobStatusTracer)
         tracerTable.remove(e.nextElement())).terminate();
      }
      while(!leaseList.isEmpty()) {
        //System.out.println("--RMAdapter--: deleting lease");
        Lease lease = (Lease)leaseList.firstElement();
        lease.cancel();
        leaseList.remove(lease);
      }
      if(nativeSGERMAdapter != null) {
        nativeSGERMAdapter.sgeGDIShutdown();
        nativeSGERMAdapter = null;
      }
    } catch(Exception ex) {
      //      System.out.println("--RMAdapter--: Exception in shutdown");
      ex.printStackTrace();
    }
  } 
}

