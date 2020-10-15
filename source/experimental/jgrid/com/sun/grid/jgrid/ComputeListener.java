/*
 * ComputeListener.java
 *
 * Created on June 19, 2002, 4:45 PM
 */

package com.sun.grid.jgrid;

import java.io.Serializable;

/** This class is not yet implemented.
 * @author dan.templeton@sun.com
 * @deprecated This class was never implemented.
 * @version 1.3
 * @since 0.1
 */
public interface ComputeListener {
	/** This method sends the result of a job execution to the listener.
	 * @param jobId the id of the job
	 * @param result the results object
	 */	
	public abstract void sendResult (String jobId, Serializable result);
}
