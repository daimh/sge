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
 * Job.java
 *
 * Created on June 19, 2002, 4:38 PM
 */

package com.sun.grid.jgrid;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;

//These dependencies are bad.  They need to be removed.
import com.sun.grid.jgrid.server.JCEPOutputStream;
/** This class represents a job to be executed by the compute
 * engine.
 * @author dan.templeton@sun.com
 * @version 1.5
 * @since 0.1
 */
public class Job implements Runnable, Serializable {
	/** The Job has not been started */
	public static final int NOT_STARTED = 0;
	/** The Job is currently running */
	public static final int RUNNING = 1;
	/** The Job has been suspended */
	public static final int SUSPENDED = 2;
	/** The Job has been stopped before completing */
	public static final int STOPPED = 3;
	/** The job has completed normally */
	public static final int COMPLETE = 4;
	/** The Job was stopped due to an error during execution */
	public static final int ERROR = 5;
	/** The Job is being suspended */
	public static final int SUSPENDING = 6;
	/** The Job is being stopped before completing */
	public static final int STOPPING = 7;
	/** The results returned from the executing Computable */
	private Serializable results = null;
	/** The thread under which this Job runs.  Used to interrupt the executing
	 * Computable for checkpoint and cancelation.
	 */
	private transient Thread computeThread = null;
	/** The absolute filename where the serialized Job is written */
	private String filename;
	
	/** The id of this job */
	private String jobId = null;
	/** The object to process */
	private Computable job = null;
	/** Whether this job is to be executed asynchronously or not */
	private boolean asynch;
	
	/** The logging mechanism for jobs */
	private transient Logger log = null;
	/** The current state of the Job, as represented by NOT_STARTED, RUNNING, STOPPED,
	 * COMPLETE, and ERROR
	 */
	private int state = NOT_STARTED;
	/** The URL from whence the Computable implementation was loaded */
	private transient String annotation = null;
	
	/** Creates a new instance of Job
	 * @param jobId the id of this job
	 * @param job the object to process
	 */
	public Job (String jobId, Computable job) {
		this (jobId, job, false);
	}
	
	/** Creates a new instance of Job
	 * @param jobId the id of this job
	 * @param job the object to process
	 * @param asynch whether this job should be executed asynchronously
	 */
	public Job (String jobId, Computable job, boolean asynch) {
		this.jobId = jobId;
		this.job = job;
		this.asynch = asynch;
	}
	
	/** Returns the id of this job
	 * @return this job's id
	 */
	public String getJobId () {
		return jobId;
	}
	
	/** Returns the object to process
	 * @return the object to process
	 */
	public Computable getJob () {
		return job;
	}
	
	/** Returns whether or not this job should be executed asynchronously
	 * @return whether this job should be executed asynchronously
	 */
	public boolean isAsynch () {
		return asynch;
	}
	
	/** Returns the logging mechanism to eb used by the Computable for reporting
	 * errors and logging messages.
	 * @return The Logger object for this Job
	 */
	public Logger getLogger () {
		return log;
	}
	
	/** Sets the logging mechanism to be used by this job
	 * @param log The Logger to be used by this Job
	 */
	public void setLogger (Logger log) {
		this.log = log;
	}
	
	/** This method causes the job to be executed. */
	public void run () {
		computeThread = Thread.currentThread ();
		state = RUNNING;
		log.jobStarted ();
		
		try {
			results = job.compute (this);
		}
		catch (Exception e) {
			state = ERROR;
			results = e;

			if (e instanceof java.lang.InterruptedException) {
				log.logError ("Job failed due to an improperly handled interrupt.  It is " +
									"likely that the interrupt came from a job.interrupt() call " +
									"in a suspend(), resume(), cancel(), or checkpoint() method.");
			}
			
			log.logError (Logger.getExceptionAsString (e));
			log.jobFailed ();
			
			return;
		}
		
		if (state == STOPPING) {
			state = STOPPED;
			log.jobStopped ();
			
			results = null;
		}
		else {
			state = COMPLETE;
			log.jobComplete ();
		}
	}
	
	/** Notifies this job that it should cancel execution.  This method calls
	 * Computable.cancel() and sets the state to STOPPED.
	 * @throws NotInterruptableException Thrown if the Computable does not implement interruption for cancelation.
	 */
	public synchronized void cancel () throws NotInterruptableException {
		state = STOPPING;
		job.cancel ();
	}
	
	/** Notifies this job that it should save its state for an upcoming checkpoint.
	 * This method calls Computable.checkpoint() and then serializes this Job to the
	 * location determined by <CODE>filename</CODE>.
	 * @throws NotInterruptableException Thrown if the Computable does not implement interruption for checkpointing.
	 */
	public synchronized void checkpoint () throws NotInterruptableException {
		job.checkpoint ();
		
		try {
			this.writeJobToDisk ();
			
			log.jobCheckpointed ();
		}
		catch (IOException e) {
			log.logError (e.getMessage ());
		}
	}
	
	/** Notifies this job that it should suspend execution.  This method calls
	 * Computable.suspend() and sets the state to SUSPENDED.
	 * @throws NotInterruptableException Thrown if the Computable does not implement interruption for suspension.
	 */
	public synchronized void suspend () throws NotInterruptableException {
		if (state == RUNNING) {
			state = SUSPENDING;
			job.suspend ();
			state = SUSPENDED;
			log.jobSuspended ();
		}
	}
	
	/** Notifies this job that it should resume execution.  This method calls
	 * Computable.resume() and sets the state to RUNNING.
	 */
	public synchronized void resume () {
		if (state == SUSPENDED) {
			job.resume ();
			state = RUNNING;
			log.jobResumed ();
		}
	}
	
	/** Checks whether or not this job has completed execution normally.  Functionally
	 * equivalent to <CODE>getState() == COMPLETE</CODE>.
	 * @return Whether the job completed execution normally
	 */
	public boolean completed () {
		return (state == COMPLETE);
	}
	
	/** Returns the current state of the Job.  The possible values may be:
	 * <UL>
	 * <LI>NOT_STARTED</LI>
	 * <LI>RUNNING</LI>
	 * <LI>STOPPED</LI>
	 * <LI>COMPLETE</LI>
	 * <LI>ERROR</LI>
	 * </UL>
	 * @return A code indicating the current state of the Job
	 */
	public int getState () {
		return state;
	}
	
	/** Get the result object returned by the Computable upon successful completion.
	 * @return The result object returned by the Computable upon successful completion.  If
	 * the Computable has not completed, or did not complete sucessfully,
	 * <CODE>null</CODE> is returned.
	 */
	public Serializable getResult () {
		return results;
	}
	
	/** Sets the location for the serialized Job file
	 * @param filename The absolute path to the serialized Job
	 */
	public void setFilename (String filename) {
		this.filename = filename;
	}
	
	/** Get the path to the serialized Job file
	 * @return The absolute path to the serialized Job
	 */
	public String getFilename () {
		return filename;
	}
	
	/** Sets the URL from which the Computable implementation was downloaded.
	 * @param annotation The URL from which the Computable implementation was downloaded
	 */
	public void setAnnotation (String annotation) {
		this.annotation = annotation;
	}
	
	/** Gets the URL from which the Computable implementation was downloaded.
	 * @return The URL from which the Computable implementation was downloaded
	 */
	public String getAnnotation () {
		return annotation;
	}
	
	/** Interrupts the Job thread.  Used by the Computable during cancel(),
	 * checkpoint(), suspend(), and resume() operations.  <B>Extreme care should
	 * be taken with this method</B> as it is possible to interrupt the thread
	 * during communications with the native peer, leaving the peer in an unstable
	 * state.
	 */
	public void interrupt () {
		computeThread.interrupt ();
	}
	
	/** Serializes the Job to the location given by <CODE>filename</CODE>.
	 * @throws IOException thrown if there's an error writing the Job to disk
	 */
	private void writeJobToDisk () throws IOException {
		FileOutputStream fout = new FileOutputStream (filename);
		JCEPOutputStream oout = new JCEPOutputStream (fout, annotation);
		
		oout.writeObject (this);
		
		oout.close ();
	}
}
