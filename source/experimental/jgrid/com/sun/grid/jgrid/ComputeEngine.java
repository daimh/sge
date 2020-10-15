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
 * ComputeEngine.java
 *
 * Created on May 21, 2002, 4:57 PM
 */

package com.sun.grid.jgrid;

import java.io.Serializable;
import java.rmi.Remote;
import java.rmi.RemoteException;

/** This class is the interface to the ComputeEngine that clients
 * see.  It allows clients to submit jobs both synchronously
 * and asynchronously and check on the results of asynchronous
 * jobs.
 * @author dan.templeton@sun.com
 * @version 1.4
 * @since 0.1
 */
public interface ComputeEngine extends Serializable, Remote {
	/** This method submits a job for synchrnous execution.
	 * @param job the job to be executed
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public abstract Serializable compute (Computable job) throws RemoteException, ComputeException;
	/** This method submits a job for synchrnous execution.
	 * @param job the job to be executed
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public abstract Serializable compute (Computable job, String codebase) throws RemoteException, ComputeException;
	/** This method submits a job for asynchronous execution.  Job
	 * status can be checked with the isComplete method and the
	 * results can be retrieved by the getResults method.
	 * @param job the job to be executed
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public abstract String computeAsynch (Computable job) throws RemoteException, ComputeException;
	/** This method submits a job for asynchronous execution.  Job
	 * status can be checked with the isComplete method and the
	 * results can be retrieved by the getResults method.
	 * @param job the job to be executed
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the result object
	 */	
	public abstract String computeAsynch (Computable job, String codebase) throws RemoteException, ComputeException;
	/** This method retrieves the results of a job that was
	 * executed asynchronously.
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return the results object
	 */	
	public abstract Serializable getResults (String jobId) throws RemoteException, ComputeException;
	/** This method check whether an asynchronous job has finished
	 * executing.
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return whether the job has finished
	 */	
	public abstract boolean isComplete (String jobId) throws RemoteException, ComputeException;
}