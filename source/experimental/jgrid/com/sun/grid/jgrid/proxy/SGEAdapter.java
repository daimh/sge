/*
 * SGEAdapter.java
 *
 * Created on May 23, 2003, 10:50 AM
 */

package com.sun.grid.jgrid.proxy;

/**
 *
 * @author  dant
 */
public class SGEAdapter implements DRMAdapter {
	private boolean setup = false;
	
	/** Creates a new instance of SGEAdapter */
	private SGEAdapter () throws DRMException {
	}
	
	public static SGEAdapter getSGEAdapter () throws DRMException {
		return new SGEAdapter ();
	}
	
	public boolean isSetup () {
		return setup;
	}
	
	public native void connect (String initializationString) throws DRMException;
	public native void submitJob (String skeleton, String jobId) throws DRMException;
	public native void disconnect () throws DRMException;
}
