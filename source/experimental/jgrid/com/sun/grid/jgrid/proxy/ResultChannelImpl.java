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
 * ResultChannelImpl.java
 *
 * Created on May 22, 2002, 3:02 PM
 */

package com.sun.grid.jgrid.proxy;

import java.io.Serializable;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/** This class implements the ResultsChannel interface.  When
 * a result is returned, we find the job in the lockbox and
 * notify the Lock.  When then put the results in the lockbox
 * to be picked up by the ComputeProxy.
 * @author Daniel Templeton
 * @version 1.6
 * @since 0.1
 */
public class ResultChannelImpl extends UnicastRemoteObject implements ResultChannel {
	/** A Map containing ComputeProxy.Lock's for each active job.
	 */
	private static Logger log = Logger.getLogger ("com.sun.grid.jgrid.proxy.ComputeProxy");
	/** Usage to store references to running and completed jobs. */	
	private Map lockbox;
	
	/** This is a convenience method for testing purposes.  It starts a instance
	 * of the ResultChannel with a fake job id entry, &quot;MyJob&quot;
	 * @param args command line arguments -- ignored
	 * @throws Exception anything that goes wrong will result in a stack trace
	 */	
	public static void main (String[] args) throws Exception {
		HashMap lockbox = new HashMap ();
		
		lockbox.put ("MyJob", new Object ());
		
		ResultChannel resultChannel = new ResultChannelImpl (lockbox);
		Registry r = LocateRegistry.createRegistry (ResultChannel.PORT);
		r.rebind (ResultChannel.LOOKUP_NAME, resultChannel);
	}
	
	/** Constructs ResultChannelImpl object and exports it on default port.
	 * @param lockbox the Map holding the job locks.
	 * @throws RemoteException if an error occurs on the server side
	 */
	public ResultChannelImpl (Map lockbox) throws RemoteException {
		super ();
		
		this.lockbox = lockbox;
	}
	
	/** This method sends the execption resulting from a job to the
	 * ComputeProxy.
	 * @param e the exception
	 * @param processId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 */
	public void sendException (Exception e, String processId) throws RemoteException {
		log.entering ("com.sun.grid.jgrid.proxy.ResultsChannelImpl", "sendException");
		log.log (Level.FINER, "Received an exception: " + e.getMessage ());
		
		Object lock = lockbox.get (processId);
		
		lockbox.put (processId, e);
		
		log.log (Level.FINEST, "Unlocking proxy");
		synchronized (lock) {
			lock.notify ();
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ResultsChannelImpl", "sendException");
	}
	
	/** This method sends the results of a job to the ComputeProxy.
	 * @param result the results object
	 * @param processId the id of the job
	 * @throws RemoteException if an error occurs on the server
	 */
	public void sendResult (Serializable result, String processId) throws RemoteException {
		log.entering ("com.sun.grid.jgrid.proxy.ResultsChannelImpl", "sendResult");
		
		log.log (Level.FINER, "Received a result: " + result.toString ());
		Object lock = lockbox.get (processId);
		
		if (lock == null) {
			log.severe ("No lock has been set for job id " + processId);
			
			RemoteException e = new RemoteException ("No lock has been set for job id " + processId);
			
			log.throwing ("com.sun.grid.jgrid.proxy.ResultsChannelImpl", "sendResult", e);
			
			throw e;
		}
		
		lockbox.put (processId, result);
		
		log.log (Level.FINEST, "Unlocking proxy");
		synchronized (lock) {
			lock.notify ();
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ResultsChannelImpl", "sendResult");
	}
}