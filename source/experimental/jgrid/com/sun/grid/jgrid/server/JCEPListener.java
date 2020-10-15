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
 * JCEPListener.java
 *
 * Created on April 3, 2003, 4:15 PM
 */

package com.sun.grid.jgrid.server;

import com.sun.grid.jgrid.Job;

/** The JCEPListener interface abstracts the JCEPHandler from the
 * JCEPProtocolModule.  It defines the methods which an implemention of a JCEP
 * handler must implement to allow for inbound communications from the client.
 * @author dan.templeton@sun.com
 * @see com.sun.grid.jgrid.server.JCEPVersion10Module
 * @version 1.6
 * @since 0.2
 */
public abstract interface JCEPListener {
	/** Shutdown this JCEP server
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void shutdown () throws CommandFailedException;
	/** Cancel an executing Job
	 * @param jobId The id of the Job to cancel
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void cancelJob (String jobId) throws CommandFailedException;
	/** Submit the Job for execution
	 * @param job The Job object to execute
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void submitJob (Job job) throws CommandFailedException;
	/** Checkpoint an executing Job
	 * @param jobId The id of the Job to checkpoint
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void checkpoint (String jobId) throws CommandFailedException;
	/** Register for messages from an executing Job
	 * @param jobId The id of the Job to which to attach
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void register (String jobId) throws CommandFailedException;
	/** Unregister for messages from a Job.  This method can be used to detach from a
	 * job that is no longer running.
	 * @param jobId The id of the Job from which to detach
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void unregister (String jobId) throws CommandFailedException;
	/** Suspend an executing Job
	 * @param jobId The id of the Job to suspend
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void suspend (String jobId) throws CommandFailedException;
	/** Resume a suspended Job
	 * @param jobId The id of the Job to resume
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 */	
	public abstract void resume (String jobId) throws CommandFailedException;
}
