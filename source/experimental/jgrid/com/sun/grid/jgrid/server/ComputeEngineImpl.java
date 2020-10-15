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
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*
 * ComputeEngineImpl.java
 *
 * Created on May 22, 2002, 2:32 PM
 */

package com.sun.grid.jgrid.server;

import com.sun.grid.jgrid.Job;
import com.sun.grid.jgrid.proxy.ResultChannel;
import java.net.MalformedURLException;
import java.rmi.*;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;



/** This class implements the ComputeEngine interface.  Currently
 * only the compute method is implemented.
 * @author Daniel Templeton
 * @version 1.4
 * @deprecated No longer used as of 0.2
 * @since 0.1
 */
public class ComputeEngineImpl extends UnicastRemoteObject implements ComputeEngine {
	/** The channel for sending results back to the ComputeProxy.
	 * @see com.sun.grid.jgrid.proxy.ComputeProxy
	 * @see com.sun.grid.jgrid.proxy.ResultChannel
	 */	
	ResultChannel resultChannel = null;
	
	/** Constructs ComputeEngineImpl object and exports it on default port.
	 * @param resultHost the host with the registry containing the ResultChannel
	 * @throws RemoteException if an error occurs while trying to retrieve the ResultChannel object
	 */
	public ComputeEngineImpl(String resultHost) throws RemoteException {
		super ();
		
		this.connectToResultChannel (resultHost);
	}
	
	/** Constructs ComputeEngineImpl object and exports it on specified port.
	 * @param resultHost the host with the registry containing the ResultChannel
	 * @param port The port for exporting
	 * @throws RemoteException if an error occurs while trying to retrieve the ResultChannel object
	 */
	public ComputeEngineImpl(String resultHost, int port) throws RemoteException {
		super (port);
		
		this.connectToResultChannel (resultHost);
	}

	/** This method retrieves the ResultChannel object from the registry.
	 * @param resultHost the host with the registry containing the ResultChannel
	 * @throws RemoteException if an error occurs while trying to retrieve the ResultChannel object
	 */	
	private void connectToResultChannel (String resultHost) throws RemoteException {
		Registry r = LocateRegistry.getRegistry(resultHost, ResultChannel.PORT);
		
		try {
			resultChannel = (ResultChannel)r.lookup(ResultChannel.LOOKUP_NAME);
		}
		catch (NotBoundException e) {
			throw new RemoteException("Unable to bind to result channel");
		}
	}
	
	/** Register ComputeEngineImpl object with the RMI registry.
	 * @param obj the object to bind
	 * @param name - name identifying the service in the RMI registry
	 * @param create - create local registry if necessary
	 * @throws RemoteException if cannot be exported or bound to RMI registry
	 * @throws MalformedURLException if name cannot be used to construct a valid URL
	 * @throws IllegalArgumentException if null passed as name
   */
	public static void registerToRegistry(String name, Remote obj, boolean create) throws RemoteException, MalformedURLException {
		
		if (name == null) throw new IllegalArgumentException("registration name can not be null");
		
		try {
			Naming.rebind(name, obj);
		} catch (RemoteException ex){
			ex.printStackTrace();
			if (create) {
				System.out.println("No registry found. Creating one.");
				Registry r = LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
				r.rebind(name, obj);
			} else throw ex;
		}
	}
	
	/** Main method.
	 * @param args the command line arguments
	 * @throws Exception if an error occurs
	 */
	public static void main(String[] args) throws Exception {
		if (args.length != 1) {
			System.err.println("Usage: java dant.grid.engine.ComputeEngineImpl result_host");
			System.exit(1);
		}
		
		System.setSecurityManager(new RMISecurityManager());
		
		ComputeEngineImpl obj = new ComputeEngineImpl(args[0]);
		registerToRegistry("ComputeEngine", obj, true);
		
		System.out.println("Ready");
	}
	
	/** This method causes the execution of the Job.  Any non-RMI
	 * exceptions will be returned to the ComputeProxy via the
	 * ResultChannel.sendException method rather than being thrown.
	 * @param job the job to execute
	 * @throws RemoteException if an error occurs on the server side
	 * @see ResultChannel#sendException
	 */	
	public void compute(Job job) throws RemoteException {
		System.out.println ("Running agent...");
/* This is no longer valid with the way jobs work now, and there's really no
 * way to back port.
		try {
			System.out.println ("Agent sending result");
			resultChannel.sendResult (job.compute (), job.getJobId ());
		}
		catch (ComputeException e) {
			System.out.println ("Agent sending exception");
			resultChannel.sendException (e, job.getJobId ());
		}
 */
	}
	
	/** This method is not implemented.
	 * @param jobId the id of the job to checkpoint
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public void checkpoint (String jobId) throws RemoteException {
		System.out.println ("Checkpointing " + jobId);
	}
	
	/** This method is not implemented.
	 * @param jobId the id of the job to stop
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public void halt (String jobId) throws RemoteException {
		System.out.println ("Halting " + jobId);
	}
	
	/** This method is not implemented.
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public void haltAll () throws RemoteException {
		System.out.println ("Halting all jobs");
	}	
}