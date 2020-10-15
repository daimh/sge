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
 * Logger.java
 *
 * Created on April 4, 2003, 10:08 AM
 */

package com.sun.grid.jgrid;

/** This interface represents a generic Logger to be used by an executing job to log
 * events and messages.
 * @author dan.templeton@sun.com
 */

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/** The purpose of the Logger class is to attach an executing Job to one or
 * more LogListeners for logging messages and event to the client.
 * @author dan.templeton@sun.com
 * @version 1.5
 * @since 0.2
 */
public class Logger extends Object {
	/** The id of the Job to which this Logger is attached.  Used when logging events
	 * and messages to the LogListener(s).
	 */	
	private String jobId = null;
	/** The list of LogListener to which this Logger logs messages and events */	
	private Set listeners = null;
	
	/** Creates a new instance of Logger
	 * @param jobId The id of the Job to which this Logger will be attached
	 */
	public Logger (String jobId) {
		this.jobId = jobId;
		listeners = Collections.synchronizedSet (new HashSet ());
	}
	
	/** Register a new LogListener with this Logger
	 * @param listener The listener to register
	 */	
	public void registerHandler (LogListener listener) {
		listeners.add (listener);
	}
	
	/** Unregister a LogListener with this Logger.
	 * @param listener The listener to unregister
	 */	
	public void unregisterHandler (LogListener listener) {
		listeners.remove (listener);
	}
	
	/** Log an error message from the Job to the LogListener(s).
	 * @param error The text of the error message
	 */	
	public void logError (String error) {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.logError (jobId, error);
		}
	}
	
	/** Log a message from the Job to the LogListener(s)
	 * @param message The text of the message
	 */	
	public void logMessage (String message) {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.logMessage (jobId, message);
		}
	}

	/** Notify the LogListener(s) that the Job has exited abnormally, either due to an
	 * uncaught Exception or because the Job was canceled.
	 */	
	public void jobStopped () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobStopped (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has been checkpointed. */	
	public void jobCheckpointed () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobCheckpointed (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has completed normally. */	
	public void jobComplete () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobCompleted (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has been started. */	
	public void jobStarted () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobStarted (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has exited abnormally, either due to an
	 * uncaught Exception or because the Job was canceled.
	 */	
	public void jobFailed () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobFailed (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has been suspended. */	
	public void jobSuspended () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobSuspended (jobId);
		}
	}
	
	/** Notify the LogListener(s) that the Job has been resumed. */	
	public void jobResumed () {
		Iterator i = listeners.iterator ();
		
		while (i.hasNext ()) {
			LogListener listener = (LogListener)i.next ();
			listener.notifyJobResumed (jobId);
		}
	}
	
	/** This method is a convenience method used to translate an Exception into a
	 * String that looks like what the VM prints out when an uncaught Exception
	 * causes a stack trace to be printed.
	 * @param e the Exception to convert to a String
	 * @return the exception as a String that includes the message and stack tace
	 */	
	public static String getExceptionAsString (Exception e) {
		StringBuffer errorBuffer = new StringBuffer (e.getClass ().getName ());
		StackTraceElement[] trace = e.getStackTrace ();
		
		if (e.getMessage () != null) {
			errorBuffer.append (": ");
			errorBuffer.append (e.getMessage ());
		}
		
		errorBuffer.append ('\n');

		for (int count = 0; count < trace.length; count++) {
			errorBuffer.append ("\tat ");
			errorBuffer.append (trace[count]);
			errorBuffer.append ('\n');
		}
		
		return errorBuffer.toString ();
	}
}