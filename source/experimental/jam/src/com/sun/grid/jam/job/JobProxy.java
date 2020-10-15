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
package com.sun.grid.jam.job;

import java.io.Serializable;
import java.io.Reader;
import java.net.URL;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.event.EventRegistration;
import net.jini.core.lookup.ServiceID;
import net.jini.core.entry.Entry;
import net.jini.core.transaction.Transaction;
import net.jini.core.transaction.Transaction.Created;
import net.jini.core.transaction.TransactionFactory;
import net.jini.core.transaction.TransactionException;
import net.jini.core.transaction.server.TransactionManager;
import net.jini.core.lease.Lease;
import net.jini.core.lease.UnknownLeaseException;
import net.jini.core.transaction.TransactionException;
import net.jini.space.JavaSpace;
import net.jini.lease.LeaseListener;
import net.jini.lease.LeaseRenewalManager;
import net.jini.lease.LeaseRenewalEvent;
import net.jini.core.lease.LeaseDeniedException;
import net.jini.admin.*;
import com.sun.grid.jam.util.JAMAdmin;
import com.sun.grid.jam.util.JAMAdminProxy;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.util.RemoteInputStreamInterface;

/**
 * The Job Proxy object
 *
 * @version %I%, %G%
 *
 * @author Nello Nellari
 *
 */
public class JobProxy
  implements JobInterface
{
  private JobInfo jobInfo;
  private JavaSpace space;
  private TransactionManager txnMgr;
  private Entry jpSnapshot;
  private Entry jceSnapshot;
  private RemoteJobObserverInterface observer;
  private JAMAdmin server;
  
  public JobProxy()
  {
    super();
  }

  public JobProxy(RemoteJobObserverInterface observer)
    throws RemoteException
  {
    this.observer = observer;
  }

  public JobProxy(JobInfo jobInfo, JavaSpace space,
                  RemoteJobObserverInterface observer)
    throws RemoteException
  {
    this.jobInfo = jobInfo;
    this.space = space;
    this.observer = observer;
    try {
      jpSnapshot = space.snapshot(new JobParams(jobInfo));
      jceSnapshot = space.snapshot(new JobControlEntry(jobInfo));
    } catch(Exception e) {
      throw new RemoteException(e.toString());   
    }
  }

  public JobProxy(JobInfo jobInfo, JavaSpace space,
                  RemoteJobObserverInterface observer,
                  TransactionManager txnMgr)
    throws RemoteException
  {
    this(jobInfo, space, observer);
    this.txnMgr = txnMgr;
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
    }
    return trc.transaction;
  }
  
  public int hashCode() {
    return server.hashCode();
  }

  public boolean equals(Object obj) {
    return (obj instanceof JobProxy &&
  	    server.equals(((JobProxy)obj).server));
  }

  /**
   * This is run within service's VM (before proxy is registered with LUS)
   */
  public void setServerRef(JAMAdmin server)
  {
    this.server = server;
  }

  public Object getAdmin()
  {
    return new JAMAdminProxy(server);
  }

  public JobInfo getJobInfo()
  {
    return jobInfo;
  }

  public JobParams getJobParams()
  {
    JobParams jp = null;
    try {
      jp = (JobParams)space.read(jpSnapshot, null,
                                 JavaSpace.NO_WAIT);
    } catch(Exception e) {
      System.out.println(e.toString());
    }
    return jp;
  }
  
  public JobControlEntry getJobControlEntry()
  {
    JobControlEntry jce = null;
    try {
      jce = (JobControlEntry)space.read(jceSnapshot, null,
                                        JavaSpace.NO_WAIT);
    } catch(Exception e) {
      System.out.println(e.toString());
    }
    return jce;
  }

  public RemoteInputStreamInterface getReader(URL res)
    throws RemoteException
  {
    return observer.getReader(res);
  }
  
  public void suspend()
    throws RemoteException
  {
//     System.out.println("--JobProxy-- request: " +
//                        JobAction.SUSPEND);
    requestJobAction(JobAction.SUSPEND, null);
  }

  public void resume()
    throws RemoteException
  {
//     System.out.println("--JobProxy-- request: " +
//                        JobAction.RESUME);
    requestJobAction(JobAction.RESUME, null);
  }

  public void kill()
    throws RemoteException
  {
//     System.out.println("--JobProxy-- request: " +
//                        JobAction.KILL);
    requestJobAction(JobAction.KILL, null);
  }

  public void start()
    throws RemoteException
  {
//     System.out.println("--JobProxy-- request: " +
//                        JobAction.START);
    requestJobAction(JobAction.START, null);
  }
  
  public void stop()
    throws RemoteException
  {
//     System.out.println("--JobProxy-- request: " +
//                        JobAction.STOP);
    requestJobAction(JobAction.STOP, null);
  }
  
  public void restart()
    throws RemoteException
  {
//      System.out.println("--JobProxy-- request: " +
//                         JobAction.RESTART);
    requestJobAction(JobAction.SUBMIT, null);
  }

  public void restart(JobParams jp)
    throws RemoteException
  {
    Transaction txn = null;
    try {
      txn = createTransaction(10 * 1000);
      JobParams old = (JobParams)
        space.takeIfExists(jpSnapshot, txn, JavaSpace.NO_WAIT);
      if(old == null) {
        txn.abort();
        throw new RemoteException("No JobParams in JavaSpace");
      }
      space.write(jp, txn, Lease.FOREVER);
    } catch(Exception ex) {
      throw new RemoteException(ex.toString());
    }
    requestJobAction(JobAction.SUBMIT, txn);
  }

  public EventRegistration notifyJobStatus(RemoteEventListener listener)
    throws RemoteException
  {
    try {
      return space.notify(jceSnapshot, null, listener,
                          Lease.ANY, null);
    } catch(Exception e) {
      throw new RemoteException(e.toString());
    }
  }

  public void requestJobAction(JobAction action, Transaction txn)
    throws RemoteException
  {
    try {
      if(txn == null)
        txn = createTransaction(10 * 1000);
      try {
        JobControlEntry jce =
          (JobControlEntry)space.takeIfExists(jceSnapshot, txn,
                                              JavaSpace.NO_WAIT);
        if(jce == null) {
          txn.abort();
          throw new RemoteException("No JobControlEntry in JavaSpace");
        }
        jce.jobAction = action;
        space.write(jce, txn, Lease.FOREVER);
      } catch(Exception ex) {
        txn.abort();
        throw new RemoteException(ex.toString());
      }
      txn.commit();
    } catch(Exception e) {
      throw new RemoteException(e.toString());
    }
  }
}



