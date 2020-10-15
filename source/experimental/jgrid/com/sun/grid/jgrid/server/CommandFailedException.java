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
 * CommandFailedException.java
 *
 * Created on October 22, 2003, 11:28 AM
 */

package com.sun.grid.jgrid.server;

import com.sun.grid.jgrid.JGridException;

/** This exception signals that a requested command (suspend,
 * resume, cancel, checkpoint, submit, or shutdown) on the
 * JCEPListener (i.e. JCEPHandler) has failed.
 * @author dan.templeton@sun.com
 * @version 1.4
 * @since 0.2
 */
public class CommandFailedException extends JGridException {
	/** The id of the job that failed */	
	private String jobId = null;
	
	/**
	 * Creates a new instance of <code>CommandFailedException</code> without detail message.
	 */
	public CommandFailedException () {
	}
	
	
	/** Constructs an instance of <code>CommandFailedException</code> with the specified detail message.
	 * @param jobId the id of the job for which the command failed
	 * @param msg the detail message.
	 */
	public CommandFailedException (String jobId, String msg) {
		super (msg);
		
		this.jobId = jobId;
	}
	
	/** Returns a message describing why the command failed.
	 * @return A message that consists of the job id followed by the reason why the command
	 * failed
	 */	
	public String getMessage () {
		return jobId + ": " + super.getMessage ();
	}
}
