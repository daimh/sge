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
 * the communication between JAM JINI services and
 * native resource manager systems.
 *
 * @version 1.18, 12/04/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
package com.sun.grid.jam.rms;

import java.io.*;
import java.io.IOException;
import java.util.Date;
import java.util.ArrayList;
import java.util.Vector;
import java.util.Enumeration;
import java.net.URL;
import java.net.MalformedURLException;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import net.jini.core.entry.Entry;
import net.jini.core.lookup.ServiceItem;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.transaction.TransactionException;
import net.jini.core.transaction.Transaction;
import net.jini.core.transaction.Transaction.Created;
import net.jini.core.transaction.TransactionFactory;
import net.jini.core.transaction.server.TransactionManager;
import net.jini.core.lease.Lease;
import net.jini.core.lease.LeaseDeniedException;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.discovery.LookupLocator;
import net.jini.lookup.ServiceIDListener;
import net.jini.lookup.ServiceDiscoveryManager;
import net.jini.lookup.JoinManager;
import net.jini.lookup.entry.Name;
import net.jini.discovery.DiscoveryManagement;
import net.jini.discovery.LookupDiscovery;
import net.jini.discovery.LookupDiscoveryManager;
import net.jini.lease.LeaseRenewalManager;
import net.jini.space.JavaSpace;

import com.sun.grid.jam.job.JobProxy;
import com.sun.grid.jam.job.JobType;
import com.sun.grid.jam.job.JobStatus;
import com.sun.grid.jam.job.JobObserver;
import com.sun.grid.jam.job.JobAction;
import com.sun.grid.jam.job.entry.JobInfo;
import com.sun.grid.jam.job.entry.JobParams;
import com.sun.grid.jam.job.entry.JobControlEntry;
import com.sun.grid.jam.job.entry.JobMonitorJFrameFactory;
import com.sun.grid.jam.queue.QueueInterface;
import com.sun.grid.jam.queue.QueueProxy;
import com.sun.grid.jam.queue.ExecutionEngineImpl;
import com.sun.grid.jam.queue.entry.QueueInfo;
import com.sun.grid.jam.queue.entry.QueueStatus;
import com.sun.grid.jam.queue.entry.ComputeInfoEntry;
import com.sun.grid.jam.ui.entry.ServiceUIFactoryEntry;
import com.sun.grid.jam.util.JAMAdmin;
import com.sun.grid.jam.util.JAMProxy;
import com.sun.grid.jam.util.JAMServiceRegistrar;
import com.sun.grid.jam.gridengine.RMAdapter;
import com.sun.grid.jam.gridengine.NativeSGEException;
import com.sun.grid.jam.gridengine.NativeSGECommException;
import com.sun.grid.jam.gridengine.queue.SGEQueue;
import com.sun.grid.jam.gridengine.queue.QueueState;
import com.sun.grid.jam.gridengine.queue.QueueType;
import com.sun.grid.jam.gridengine.exechost.SGEExecHost;

/**
 *
 *
 * @version 1.18, 12/04/00
 *
 * @author Nello Nellari
 * @author Rafal Piotrowski
 *
 */
public class RMServer
{
  private String rmsName;
  private ArrayList queueList;
  private ArrayList joiners;
  private Vector leaseList;
  private JavaSpace space;
  private TransactionManager txnMgr;
  private RMAdapter rm;
  private Entry controlEntry;
  private Entry killAction;
  private JobDestroyListener destroyListener;
  private ServiceCreatorListener creatorListener;
  private LookupLocator[] locators;

  /**
   * Listen for submitted jobs
   * and start new JINI job services
   * 
   */
  class ServiceCreatorListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {

    /**
     * Default constructor
     *
     */
    public ServiceCreatorListener()
      throws RemoteException
    {
      super();
    }

    /**
     * Implement RemoteEventListener interface
     *
     */
    public void notify(RemoteEvent e)
    {
      submit();
    }
  }
  
  /**
   * Listen for killing job request.
   * Destroys the Job service.
   * 
   */
  class JobDestroyListener
    extends UnicastRemoteObject
    implements RemoteEventListener 
  {

    /**
     * Default constructor
     *
     */
    public JobDestroyListener()
      throws RemoteException
    {
      super();
    }
    
    /**
     * Implement RemoteEventListener interface
     *
     */
    public void notify(RemoteEvent e)
    {
      destroyJobService();
    }
  }

  public RMServer(String rmsName)
    throws RMServerException, ClassNotFoundException
  {
    this(rmsName, null,
         new Class[] { Class.forName("net.jini.space.JavaSpace") },
         new Entry[] { new Name("JobRepository") },
         new String[] { System.getProperty("user.name") },
         null,
         new Class[] {
           Class.forName("net.jini.core.transaction.server.TransactionManager")},
         null, new String[] { System.getProperty("user.name") });
  }

  public RMServer(String rmsName, String locator)
    throws RMServerException, ClassNotFoundException
  {
    this(rmsName, locator,
         new Class[] { Class.forName("net.jini.space.JavaSpace") },
         new Entry[] { new Name("JobRepository") },
         new String[] { System.getProperty("user.name") },
         null,
         new Class[] {
           Class.forName("net.jini.core.transaction.server.TransactionManager")},
         null, new String[] { System.getProperty("user.name") });
  }

  public RMServer(String rmsName, String locator,
                  Class[] spaceClss, Entry[] spaceEntries,
                  String[] spaceGroups, ServiceID txnID,
                  Class[] txnClss, Entry[] txnEntries,
                  String[] txnGroups)
    throws RMServerException
  {
    if (locator == null) {
      locators = null;
    } else {
      locators = new LookupLocator[1];
      try {
	locators[0] = new LookupLocator(locator);
      } catch (MalformedURLException mue) {
	mue.printStackTrace();
      }
    }

    try {
      System.out.print("RMServer is coming up .");
      this.rmsName = rmsName;
      queueList = new ArrayList();
      joiners = new ArrayList();
      leaseList = new Vector();
      creatorListener = new ServiceCreatorListener();
      destroyListener = new JobDestroyListener();
      ServiceItem si;
      ServiceTemplate st;
      LookupDiscoveryManager discovery = new
        LookupDiscoveryManager(spaceGroups, locators, null);
      ServiceDiscoveryManager serviceDiscoveryManager = new
        ServiceDiscoveryManager(discovery, null);
      System.out.print("...");
      st = new ServiceTemplate(null, spaceClss, spaceEntries);
      si = serviceDiscoveryManager.lookup(st, null,
                                          10 * 1000);
      if(si != null && si.service != null) {
        space = (JavaSpace) si.service;
        si = null;
        System.out.print("...");
      } else {
        throw
          new RMServerException("JobRepository not found");
      }
      discovery.setGroups(txnGroups);
      st = new ServiceTemplate(txnID, txnClss, txnEntries);
      si = serviceDiscoveryManager.lookup(st, null,
                                          10 * 1000);
      if(si != null && si.service != null) {
        txnMgr = (TransactionManager) si.service;
        si = null;
        System.out.print("...");
      } else {
        throw
          new RMServerException("TransactionManager not found");
      }
      discovery.terminate();
      serviceDiscoveryManager.terminate();
      System.out.println(" ready.");
    } catch(Exception e) {
      throw new RMServerException(e.getMessage());
    }
  }
  
  public RMServer(String rmsName, ServiceTemplate spaceTempl,
                  String[] spaceGroups, ServiceTemplate txnTempl,
                  String[] txnGroups)
    throws InterruptedException, ClassNotFoundException,
           RemoteException, IOException, TransactionException
  {
    this.rmsName = rmsName;
    queueList = new ArrayList();
    joiners = new ArrayList();
    leaseList = new Vector();
    creatorListener = new ServiceCreatorListener();
    destroyListener = new JobDestroyListener();
    ServiceItem si;
    LookupDiscovery discovery = new
      LookupDiscovery(LookupDiscovery.NO_GROUPS);
    discovery.addGroups(spaceGroups);
    ServiceDiscoveryManager serviceDiscoveryManager = new
      ServiceDiscoveryManager(discovery, null);
    si = serviceDiscoveryManager.lookup(spaceTempl, null,
                                        30 * 1000);
    if(si != null && si.service != null) {
      space = (JavaSpace) si.service;
      si = null;
      System.out.println("JobRepository found");
    } else {
      System.out.println("Can't find JobRepository");
    }
    discovery.setGroups(txnGroups);
    si = serviceDiscoveryManager.lookup(spaceTempl, null,
                                        30 * 1000);
    if(si != null && si.service != null) {
      txnMgr = (TransactionManager) si.service;
      si = null;
      System.out.println("TransactionManager found");
    } else {
      System.out.println("Can't find TransactionManager");
    }
    discovery.terminate();
    serviceDiscoveryManager.terminate();
  }

  public void startAdapter()
    throws RemoteException, TransactionException,
           NativeSGEException
  {
    if(space != null) {
      rm = new RMAdapter(space, txnMgr);
    }
  }
  
  public void createQueueService(String name, Entry[] attrs)
    throws IOException
  {
    ExecutionEngineImpl execEng = new ExecutionEngineImpl();
    QueueProxy q = new QueueProxy(name, space, execEng, txnMgr);
    JAMServiceRegistrar jsr = new JAMServiceRegistrar(q, attrs, locators, joiners);
    jsr.run();
  }

  public void createJobService(JobInfo jobInfo, Entry[] attrs)
    throws IOException
  {
    // Create an Hastable for Job joiners
    JobObserver jObserver = new JobObserver(jobInfo, space);
    JobProxy j = new JobProxy(jobInfo, space, jObserver, txnMgr);
    // We really want to subclass JAMServiceRegistrar, so we can
    // override its destroy method to include JavaSpace cleanup. XXX
    JAMServiceRegistrar jsr = new JAMServiceRegistrar(j, attrs, locators, joiners);
    jsr.run();
  }

  private void submit()
  {
    try {
      Transaction txn = createTransaction(10 * 1000);
      try {
        JobControlEntry jce = (JobControlEntry)
          space.takeIfExists(controlEntry, txn, JavaSpace.NO_WAIT);
        submit(jce, txn);
      } catch(Exception ex) {
        System.out.println("--RMServer: Submit aborted " + ex);
      }
    } catch(RMServerException rmse) {
      System.out.println("--RMServer: Submit transaction aborted " +
                         rmse);
    }
  }

  private void submit(JobControlEntry ce, Transaction txn)
  {
    try {
      try {
        ServiceUIFactoryEntry ui = new
          ServiceUIFactoryEntry("Monitor", new
            JobMonitorJFrameFactory());
        if(ce.attributes == null) {
          Entry [] jEntry = { ui, new Name(ce.jobInfo.queueName),
                              new Name(ce.jobInfo.userID),
                              new Name(ce.jobInfo.key.toString())};
          createJobService(ce.jobInfo, jEntry);
        } else {
          int i = 0;
          Entry [] jEntry = new Entry[ce.attributes.length + 3];
          for(; i < ce.attributes.length; i++)
            jEntry[i] = ce.attributes[i];
          jEntry[i++] = ui;
          jEntry[i++] = new Name(ce.jobInfo.queueName);
          jEntry[i++] = new Name(ce.jobInfo.key.toString());
          createJobService(ce.jobInfo, jEntry);
        }
        //Writes the control entry in the job repository and
        //deletes the attribute list
        ce.jobStatus = JobStatus.JAMQUEUED;
        ce.jobAction = JobAction.SUBMIT;
        // Don't set attributes to null - it confuses the example SpaceBrowser
        // Set it to an empty Entry array instead
        ce.attributes = new Entry[0];
        space.write(ce, txn, Lease.FOREVER);
      } catch(Exception ex) {
        txn.abort();
        ex.printStackTrace();
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("-- RMServer: JobSubmission transaction failed");
    }
  }
  
//    private void submit(JobControlEntry ce)
//    {
//      try {
//        UIFactoryEntry ui = new
//          UIFactoryEntry("Monitor", "Monitoring and controlling",
//                         new JobMonitorJFrameFactory(), null);
//        if(ce.attributes == null) {
//          Entry [] jEntry = { ui, new Name(ce.jobInfo.queueName),
//                              new Name(ce.jobInfo.userID),
//                              new Name(ce.jobInfo.key.toString())};
//          createJobService(ce.jobInfo, jEntry);
//        } else {
//          int i = 0;
//          Entry [] jEntry = new Entry[ce.attributes.length + 3];
//          for(; i < ce.attributes.length; i++)
//            jEntry[i] = ce.attributes[i];
//          jEntry[i++] = ui;
//          jEntry[i++] = new Name(ce.jobInfo.queueName);
//          jEntry[i++] = new Name(ce.jobInfo.key.toString());
//          createJobService(ce.jobInfo, jEntry);
//        }
//        //Writes the control entry in the job repository and
//        //deletes the attribute list
//        ce.jobStatus = JobStatus.JAMQUEUED;
//        ce.jobAction = JobAction.SUBMIT;
//        // Don't set attributes to null - it confuses the example SpaceBrowser
//        // Set it to an empty Entry array instead
//        ce.attributes = new Entry[0];
//        space.write(ce, null, Lease.FOREVER);
//      } catch(Exception ex) {
//        ex.printStackTrace();
//      }
//    }

  private Transaction createTransaction(long leaseTime)
    throws RMServerException
  {
    Transaction.Created trc = null;
    if(txnMgr != null) {
      try {
        trc = TransactionFactory.create(txnMgr, leaseTime);
      } catch(LeaseDeniedException lde) {
        throw new RMServerException(lde.getMessage());
      } catch(RemoteException re) {
        throw new RMServerException(re.getMessage());
      }
    } else {
      throw new RMServerException("TransactionManager is null");
    }
    return trc.transaction;
  }

  private void destroyJobService()
  {
    try {
      Transaction txn = createTransaction(5 * 1000);
      try {
        JobControlEntry jce = (JobControlEntry)
          space.takeIfExists(killAction,
                             txn, JavaSpace.NO_WAIT);
        jce.jobStatus = JobStatus.KILLED;
        jce.jobAction = JobAction.NONE;
        space.write(jce, txn, Lease.FOREVER);
        //destroyJobService(jce);
      } catch(Exception ex) {
        System.out.println("--RMServer- Deleting job service aborted "
                           + ex);
        txn.abort();
      }
      txn.commit();
    } catch(Exception e) {
      System.out.println("--RMServer- Deleting job service aborted "
                         + e);
    }
  }
  
//    private void destroyJobService(JobControlEntry jce)
//    {
//      try {
//        //Unregister the JobService
//        //Code missing here
//        jce.jobStatus = JobStatus.KILLED;
//        jce.jobAction = JobAction.NONE;
//        space.write(jce, null, Lease.FOREVER);
//      } catch(Exception ex) {
//        ex.printStackTrace();
//      }
//    }
  
  private void shutdown()
  {
    //Cleaning RMServer resources
    //Delete Queue services
    //Delete Job services
    //Delete leases
    try {
      while(!leaseList.isEmpty()) {
        Lease lease = (Lease)leaseList.firstElement();
        lease.cancel();
        leaseList.remove(lease);
        //System.out.println("--RMServer--: deleting lease");
      }
    } catch(Exception ex) {
      //System.out.println("--RMServer--: Exception in shutdown");
      ex.printStackTrace();
    }
    rm.shutdown();
  }

  private static void emptyJobRepository(RMServer server)
  {
    try {
      Entry entry;
      Object ob;
      entry = new JobInfo();
      do {
        ob = server.space.take(entry, null,
                               JavaSpace.NO_WAIT);
      } while(ob != null);
      entry = new JobParams();
      do {
        ob = server.space.take(entry, null,
                               JavaSpace.NO_WAIT);
      } while(ob != null);
      entry = new JobControlEntry();
      do {
        ob = server.space.take(entry, null,
                               JavaSpace.NO_WAIT);
      } while(ob != null);
    } catch(Exception ex) {
      ex.printStackTrace();
    }
  }
  
  private static void addQMaster(RMServer server)
  {
    try {
      Entry[] attrQueue = {
        new Name(SGEQueue.QMASTER_NAME),
        new QueueInfo(SGEQueue.QMASTER_NAME,
                      QueueStatus.QUEUE_ENABLED,
                      new Boolean(true), // interactive
                      new Boolean(true), // batch
                      Boolean.FALSE, // dedicated
                      server.rmsName),
        new  ComputeInfoEntry("N/A", // manufacturer
                              "N/A", // model
                              "N/A", // architecture
                              "N/A", // OS name
                              "N/A", // OS revision
                              "N/A", // OS arch
                              new Integer(0), // processors 
                              new Integer(0)) // tiling factor
          };
      server.createQueueService(SGEQueue.QMASTER_NAME, attrQueue);
    } catch(IOException nce){
      System.out.println("-- RMServer --: Failed adding qmaster");
      nce.printStackTrace();
    }
  }

  private static void addQueues(RMServer server)
    throws RemoteException, TransactionException
  {

    SGEQueue[] queues = null;
    SGEQueue queue = null;
    SGEExecHost execHost = null;

    try {
      queues = server.rm.retreiveAllQueues();
    } catch(NativeSGEException e){
      System.out.println("--RMServer--: Failed to retreive queues from RMS");
      return;
    }

    for(int i = 0; i < queues.length; i++) {
      queue = null;
      queue = queues[i];
      execHost = null;
      execHost = queue.getExecHost();

      // Add queue-specific JobRepository listeners
      if(server.space != null) {
	server.controlEntry = server.space.snapshot
	  (new JobControlEntry(JobAction.QUEUE, queue.getName()));

	server.killAction = server.space.snapshot
	  (new JobControlEntry(JobAction.KILL, queue.getName()));

	server.leaseList.add(server.space.notify(server.controlEntry, null,
						 server.creatorListener,
						 Lease.FOREVER, null).getLease());
	server.leaseList.add(server.space.notify(server.killAction, null,
						 server.destroyListener,
						 Lease.FOREVER, null).getLease());
      }


      try {
        Entry[] attrQueue = {
          new Name(queue.getName()),
          new QueueInfo(queue.getName(),
                        new QueueStatus(queue.getState()),
                        new Boolean(queue.isInteractive()), // interactive
                        new Boolean(queue.isBatch()), // batch
                        Boolean.FALSE, // dedicated
                        server.rmsName),
          new  ComputeInfoEntry("N/A", // manufacturer
                                "N/A", // model
                                execHost.getArch(),
                                "N/A", // OS name
                                "N/A", // OS revision
                                "N/A", // OS arch
                                new Integer(execHost.getProcessors()), 
                                new Integer(0)) // tiling factor
        };
        server.createQueueService(queue.getName(), attrQueue);
      } catch(IOException nce){
        System.out.println("--RMServer--: Failed adding \""+queue.getName()+"\" queue");
        //        nce.printStackTrace();
      }
    }
  }
  
  public static void main(String[] args)
  {
    RMServer server = null;
    try {
      // Set the security manager
      System.setSecurityManager(new SecurityManager());
      // Start the RMServer - should take name from command line
      if (args.length >= 1) {
	server = new RMServer("Grid Engine", args[0]);
      } else {
	server = new RMServer("Grid Engine");
      }
      server.startAdapter();
      System.out.println("RMAdapter created.");

      // retreive queues and add them
      addQueues(server);
      // add the qmaster queue
      // addQMaster(server);
      System.out.println("RMServer STARTED, type 'q' for exit");
      BufferedReader in = new BufferedReader(new
        InputStreamReader(System.in));
      while(true) {
        String line = in.readLine();
        if(line.equalsIgnoreCase("q")) {
          for(int i = 0; i < server.joiners.size(); i ++)
            ((JoinManager)server.joiners.get(i)).terminate();
          server.shutdown();
          System.exit(0);
        }
        Thread.sleep(3000);
      }
    } catch(Exception e) {
      System.out.println("--RMServer--: RMAdapter not started");
      e.printStackTrace();
      if(server != null) {
        server.shutdown();
        server = null;
      }
      System.exit(0);
    }
  }
}

