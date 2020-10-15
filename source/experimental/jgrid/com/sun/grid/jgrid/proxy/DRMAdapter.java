/*
 * DRMAdapter.java
 *
 * Created on September 12, 2003, 11:46 AM
 */

package com.sun.grid.jgrid.proxy;

/** This class represents an abstraction layer for native communications with a DRM.
 * @author dan.templeton@sun.com
 * @version 1.3
 * @since 0.1
 */
public abstract interface DRMAdapter {
	/** This method establishes a connection with the underlying DRM.
	 * @param initializationString An implementation specific initialization string
	 * @throws DRMException thrown when an error occurs connecting to the DRM
	 */	
	public abstract void connect (String initializationString) throws DRMException;
	/** This method submits a job to the underlying DRM
	 * @param skeleton the job native compute peer
	 * @param jobId the JGrid id of the job
	 * @throws DRMException thrown when there's an error submitting the job to the DRM
	 */	
	public abstract void submitJob (String skeleton, String jobId) throws DRMException;
	/** This method disconnects from the underlying DRM.
	 * @throws DRMException thrown when there is an error disconnecting from the DRM
	 */	
	public abstract void disconnect () throws DRMException;	
}
