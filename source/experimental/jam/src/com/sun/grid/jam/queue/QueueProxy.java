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
/**
 * This package contains classes and interfaces for
 * Queue services
 *
 * @version %I%, %G%
 *
 * @author Nello Nellari
 *
 */
package com.sun.grid.jam.queue;

import java.rmi.RemoteException;
import java.io.IOException;
import java.util.Random;
import net.jini.core.entry.Entry;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lease.Lease;
import net.jini.core.lease.LeaseDeniedException;
import net.jini.core.transaction.Transaction;
import net.jini.core.transaction.Transaction.Created;
import net.jini.core.transaction.TransactionFactory;
import net.jini.core.transaction.TransactionException;
import net.jini.core.transaction.server.TransactionManager;
import net.jini.core.event.EventRegistration;
import net.jini.core.event.RemoteEventListener;
import net.jini.space.JavaSpace;
import net.jini.admin.*;
import com.sun.grid.jam.admin.UserProperties;
import com.sun.grid.jam.app.AppParamsInterface;
import com.sun.grid.jam.job.JobType;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobAction;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.util.JAMAdmin;
import com.sun.grid.jam.util.JAMAdminProxy;

/**
 * A queue implementation for storing item
 * in the JavaSpace
 *
 * @version %I%, %G%
 *
 * @author Nello Nellari
 *
 */
public class QueueProxy
  implements QueueInterface
{
  private JavaSpace space;
  private TransactionManager txnMgr;
  private String name;
  private ExecutionEngineInterface execEng;
  private JAMAdmin server;
  
  public QueueProxy()
  {
    super();
  }
  
  public QueueProxy(String name)
  {
    this.name = name;
  }
  
  public QueueProxy(String name, JavaSpace space)
  {
    this(name);
    this.space = space;
  }

  public QueueProxy(String name, JavaSpace space,
                    ExecutionEngineInterface execEng)
  {
    this(name);
    this.space = space;
    this.execEng = execEng;
  }

  public QueueProxy(String name, JavaSpace space,
                    ExecutionEngineInterface execEng,
                    TransactionManager txnMgr)
  {
    this(name, space, execEng);
    this.txnMgr = txnMgr;
  }
  
  public int hashCode() {
    return server.hashCode();
  }

  public boolean equals(Object obj) {
    return (obj instanceof QueueProxy &&
  	    server.equals(((QueueProxy)obj).server));
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
  
  public void submit(AppParamsInterface appParams, Entry [] jobAttr,
                     UserProperties userProperties)
    throws ApplicationSubmissionException
  {
    if(appParams == null) {
      throw new ApplicationSubmissionException("AppParams is null");
    } else {
      Transaction txn = null;
      try {
        txn = createTransaction(10 * 1000);
        try {
          JobInfo info = new JobInfo(appParams.getName(),
                                     JobType.BATCH,
                                     name, userProperties.getLogin(),
                                     userProperties.getLogin(),
                                     new Random().nextInt());
          JobParams jp = new JobParams(info, -1,
                                       appParams.getScript(),
                                       appParams.getArgs());
          JobControlEntry jce = new JobControlEntry(info,
                                                    JobStatus.UNKNOWN,
                                                    JobAction.QUEUE,
                                                    jobAttr);
          space.write(info, txn, Lease.FOREVER);
          space.write(jp, txn, Lease.FOREVER);
          space.write(jce, txn, Lease.FOREVER);
          //System.out.println("--QueueProxy--: submitting:\n" + info
          //                   +"\n" + jp + "\n"+ jce);
        } catch(Exception ex) {
          txn.abort();
          throw new ApplicationSubmissionException(ex.getMessage());
        }
        txn.commit();
      } catch(Exception e) {
        //e.printStackTrace();
        throw new ApplicationSubmissionException(e.getMessage());
      }
    }
  }
  
  public EventRegistration register(AppParamsInterface api,
                                    RemoteEventListener
                                    queueListener)
    throws RemoteException
  {
    if(true)
      throw new RemoteException("Method not implemented");
    return null;
  }
  
  public int getJobsCount()
    throws RemoteException
  {
    return 0;
  }
  
  private class RunThread
    implements Runnable
  {
    private String command;

    public RunThread(String command)
    {
      this.command = command;
    }

    public void run()
    {
      try {
        String result = (execEng.execute(command)).toString();
        System.out.println("NativeApp output: \n" + result);
      } catch(RemoteException re) {
        re.printStackTrace();
      }
    }
  }
  
  public void run(String command)
  {
    RunThread rt = new RunThread(command);
    new Thread(rt).start();
  }
}

