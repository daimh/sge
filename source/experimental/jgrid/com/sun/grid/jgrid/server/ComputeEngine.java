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
 * Created on May 22, 2002, 2:30 PM
 */

package com.sun.grid.jgrid.server;

import com.sun.grid.jgrid.Job;

import java.rmi.Remote;
import java.rmi.RemoteException;

/** This class is the interface to the ComputeEngine that the
 * skeleton sees.  It allows the skeleton to control the
 * compute engines.
 * @author Daniel Templeton
 * @version 1.4
 * @deprecated No longer used as of 0.2
 * @since 0.1
 */
public interface ComputeEngine extends Remote {
	/** This method causes the execution of the Job.  Any non-RMI
	 * exceptions will be returned to the ComputeProxy via the
	 * ResultChannel.sendException method rather than being thrown.
	 * @param job the job to execute
	 * @throws RemoteException if an error occurs on the server side
	 * @see com.sun.grid.jgrid.proxy.ResultChannel#sendException
	 */	
	public abstract void compute (Job job) throws RemoteException;
	/** This method is causes the executing job to be written to
	 * disk.
	 * @param jobId the id of the job to checkpoint
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public abstract void checkpoint (String jobId) throws RemoteException;
	/** This method stops a job from executing.  The Job should be
	 * checkpointed before being stopped.
	 * @param jobId the id of the job to halt
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public abstract void halt (String jobId) throws RemoteException;
	/** This method stops all executing jobs on the ComputeEngine.
	 * Each Job should be checkpointed before being stopped.
	 * @throws RemoteException if an error occurs on the server side
	 */	
	public abstract void haltAll () throws RemoteException;
}
