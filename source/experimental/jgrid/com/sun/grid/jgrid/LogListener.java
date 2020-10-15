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
 * JCEPLogListener.java
 *
 * Created on October 23, 2003, 3:24 PM
 */

package com.sun.grid.jgrid;

/** This interface describes classes which can listen to the information passed to
 * the Logger by the Job.
 * @author dan.templeton@sun.com
 * @see com.sun.grid.jgrid.Logger
 * @version 1.4
 * @since 0.2
 */
public interface LogListener {
	/** Log a message.
	 * @param jobId The id of the job for which this message is being logged
	 * @param message The text of the message
	 */	
	public abstract void logMessage (String jobId, String message);
	/** Log an error.
	 * @param jobId The id of the job for which this error is being logged
	 * @param error The text of the error message
	 */	
	public abstract void logError (String jobId, String error);
	/** Log that the job has started executing.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobStarted (String jobId);
	/** Log that the job has completed execution normally.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobCompleted (String jobId);
	/** Log that the job has been checkpointed.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobCheckpointed (String jobId);
	/** Log that the job has been stopped prior to completing.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobStopped (String jobId);
	/** Log that the job has exited prior to completing.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobFailed (String jobId);
	/** Log that the job has been suspended.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobSuspended (String jobId);
	/** Log that the job has been resumed.
	 * @param jobId The id of the job for which this message is being logged
	 */	
	public abstract void notifyJobResumed (String jobId);
}
