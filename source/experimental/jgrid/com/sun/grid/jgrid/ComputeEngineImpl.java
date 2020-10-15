/*
 * ComputeEngineImpl.java
 *
 * Created on May 21, 2002, 5:01 PM
 */

package com.sun.grid.jgrid;

import java.io.Serializable;
import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.RMISecurityManager;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;

/** This class is a dummy implementation of the ComputeEngine
 * interface used to generate stub and skeleton classes.  The
 * functionality of this class is actually performed by the
 * ComputeProxy class.
 * @author dan.templeton@sun.com
 * @version 1.4
 * @deprecated No longer used as of 0.2.1
 * @since 0.1
 */
public class ComputeEngineImpl extends UnicastRemoteObject implements ComputeEngine {
	
	/** Constructs ComputeEngineImpl object and exports it on default port.
	 * @throws RemoteException thrown when a remote expection occurs
	 */
	public ComputeEngineImpl () throws RemoteException {
		super ();
	}
	
	/** Constructs ComputeEngineImpl object and exports it on specified port.
	 * @param port The port for exporting
	 * @throws RemoteException thrown when a remote expection occurs
	 */
	public ComputeEngineImpl (int port) throws RemoteException {
		super (port);
	}
	
	/** Register ComputeEngineImpl object with the RMI registry.
	 * @param obj the object to bind
	 * @param name - name identifying the service in the RMI registry
	 * @param create - create local registry if necessary
	 * @throws RemoteException if cannot be exported or bound to RMI registry
	 * @throws MalformedURLException if name cannot be used to construct a valid URL
	 */
	public static void registerToRegistry (String name, Remote obj, boolean create) throws RemoteException, MalformedURLException{
		
		if (name == null) throw new IllegalArgumentException ("registration name can not be null");
		
		try {
			Naming.rebind (name, obj);
		} catch (RemoteException ex){
			if (create) {
				ex.printStackTrace ();
				Registry r = LocateRegistry.createRegistry (Registry.REGISTRY_PORT);
				r.rebind (name, obj);
			} else throw ex;
		}
	}
	
	/** Main method.
	 * @param args the program arguments
	 */
	public static void main (String[] args) {
		System.setSecurityManager (new RMISecurityManager ());
		
		try {
			ComputeEngineImpl obj = new ComputeEngineImpl ();
			registerToRegistry ("ComputeEngine", obj, true);
		} catch (RemoteException ex) {
			ex.printStackTrace ();
		} catch (MalformedURLException ex) {
			ex.printStackTrace ();
		}
		
		System.out.println ("Ready.");
	}
	
	/** This method submits a job for synchrnous execution.
	 * @param job the job to be executed
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public Serializable compute (Computable job) throws RemoteException, ComputeException {
		return null;
	}
	
	/** This method submits a job for asynchrnous execution.
	 * @param job the job to be executed
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public String computeAsynch (Computable job) throws RemoteException, ComputeException {
		return null;
	}
	
	/** This method retrieves the results of a job that was
	 * executed asynchronously.
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the results object
	 */	
	public Serializable getResults (String jobId) throws RemoteException, ComputeException {
		return null;
	}
	
	/** This method check whether an asynchronous job has finished
	 * executing.
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return whether the job has finished
	 */	
	public boolean isComplete (String jobId) throws RemoteException, ComputeException {
		return false;
	}
	
	/** This method submits a job for synchrnous execution.
	 * @param job the job to be executed
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public Serializable compute (Computable job, String codebase) throws RemoteException, ComputeException {
		return null;
	}
	
	/** This method submits a job for asynchronous execution.  Job
	 * status can be checked with the isComplete method and the
	 * results can be retrieved by the getResults method.
	 * @param job the job to be executed
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public String computeAsynch (Computable job, String codebase) throws RemoteException, ComputeException {
		return null;
	}
	
}
