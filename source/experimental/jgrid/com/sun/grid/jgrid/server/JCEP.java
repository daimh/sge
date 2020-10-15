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
 * JCEP.java
 *
 * Created on April 1, 2003, 2:02 PM
 */

package com.sun.grid.jgrid.server;

/** The JCEP interface defines the codes used by the JCEP client and server
 * to communicate with each other.
 * @author dan.templeton@sun.com
 * @version 1.4
 * @since 0.2
 */

public interface JCEP {
	/** The default port number to use for a JCEP socket */	
	public static final int PORT = 13516;
	/** Passed at the beginning of a conversation */	
	public static final int HANDSHAKE = 0x74676980;
	/** Version 1.0 */	
	public static final byte VERSION10 = (byte)0x10;
	/** Submit a job for execution. */	
	public static final byte SUBMIT_JOB = (byte)0x40;
	/** Cancel an executing job. */	
	public static final byte CANCEL_JOB = (byte)0x41;
	/** Checkpointing an executing job. */	
	public static final byte CHECKPOINT_JOB = (byte)0x42;
	/** Shutdown the JCEP server. */	
	public static final byte SHUTDOWN = (byte)0x43;
	/** Register for messages from a running job. */	
	public static final byte REGISTER = (byte)0x44;
	/** Unregister for messages for a job. */	
	public static final byte UNREGISTER = (byte)0x45;
	/** Suspend an executing job */
	public static final byte SUSPEND = (byte)0x46;
	/** Resume a suspended job */
	public static final byte RESUME = (byte)0x47;
	/** A message from an executing job. */	
	public static final byte LOG_MESSAGE = (byte)0x80;
	/** An error from an executing job. */	
	public static final byte LOG_ERROR = (byte)0x81;
	/** The executing job job has changed state. */	
	public static final byte JOB_STATE_CHANGE = (byte)0x82;
	/** The JCEP command has failed. */	
	public static final byte COMMAND_FAILED = (byte)0x83;
 	/** The executing job has been sucessfully checkpointed. */	
 	public static final byte JOB_CHECKPOINTED = (byte)0x84;
	/** The JCEP server is shutting down.  All jobs will be stopped. */	
	public static final byte SHUTTING_DOWN = (byte)0x85;
	/** The job is now running */
	public static final byte STATE_RUNNING = (byte)0x01;
	/** The job is now suspended */
	public static final byte STATE_SUSPENDED = (byte)0x02;
	/** The job is now stopped */
	public static final byte STATE_STOPPED = (byte)0x03;
	/** The job is now completed */
	public static final byte STATE_COMPLETED = (byte)0x04;
	/** The job has failed */
	public static final byte STATE_FAILED = (byte)0x05;
}
