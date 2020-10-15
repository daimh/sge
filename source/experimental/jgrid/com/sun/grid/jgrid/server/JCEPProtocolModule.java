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
 * JCEPProtocolModule.java
 *
 * Created on April 3, 2003, 3:54 PM
 */

package com.sun.grid.jgrid.server;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Logger;

/** This class defines the interface for classes which which to implement a JCEP
 * service.  JCEP stands for JGrid Compute Engine Protocol.  It is a simple
 * protocol which allows a client and server to send commands and job information
 * over a socket connection.
 * @author dan.templeton@sun.com
 * @version 1.5
 * @since 0.2
 */
public abstract class JCEPProtocolModule implements JCEP {
	/** Used with logMessage() and logError() to indicated that the message comes from
	 * the JCEPProtocolModule, not from a Job.
	 */	
	public static final String SYSTEM_ID = "JCEPSystemID";
	/** The lastest version of the JCEP protocol. */	
	private static final byte CURRENT_VERSION = VERSION10;
	/** The minimum version of the JCEP protocol supported by the protocol module
	 * factory.
	 */	
	private static final byte MIN_VERSION = VERSION10;
	/** The logging mechanism */	
	private static Logger log = null;
	/** The list of JCEPProtocolModules with open client connections.  Used to notify
	 * all clients of a server shutdown.
	 */	
	protected static List moduleList = new LinkedList ();
	
	static {
		log = Logger.getLogger ("com.sun.grid.jgrid.server.ComputeEngine");
	}
	
	/** Establishes a connection to the client over the given socket and sets the
	 * handler for the connection.
	 * @param engine The JCEPListener to use for sending command requests from the client
	 * @param socket The socket on which to connect to the client
	 * @throws IOException Thrown then the connection cannot be established
	 * @return A JCEPProtocolModule object that encapsulates the client communications for the
	 * JCEPHandler
	 */	
	static JCEPProtocolModule establishConnection (JCEPListener engine, Socket socket) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPProtocolModule", "establishConnection");
		
		DataInputStream din = new DataInputStream (socket.getInputStream ());		
		DataOutputStream dout = new DataOutputStream (socket.getOutputStream ());
		
		int version = doHandshake (din, dout);
		JCEPProtocolModule module = null;
		
		/* This should actually be coming out of a file somewhere so that custom
		 * protocol handlers can be inserted without mucking with this class. */
		switch (version) {
			case VERSION10:
				module = new JCEPVersion10Module (engine, socket, din, dout);
				moduleList.add (module);
				break;
			default:
				IOException e = new IOException ("Failed handshake: invalid protocol version");
				log.throwing ("com.sun.grid.jgrid.server.JCEPProtocolModule", "establishConnection", e);
				throw e;
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPProtocolModule", "establishConnection");		

		return module;
	}
	
	/** This method confirms that the client is attempting to establish a JCEP
	 * communication channel and establishes the version of JCEP to use.
	 * @param din Input stream from the socket
	 * @param dout Output stream to the socket
	 * @throws IOException Thrown is there's an error during the handshake process
	 * @return Returns the JCEP version code to be used
	 */	
	private static byte doHandshake (DataInputStream din, DataOutputStream dout) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPProtocolModule", "doHandshake");
		
		int handshake = din.readInt ();

		if (handshake != HANDSHAKE) {
			throw new IOException ("Failed handshake: bad magic number");
		}

		byte version = din.readByte ();

		if (version < MIN_VERSION) {
			throw new IOException ("Invalid version number during handshake.");
		}
		
		version = (byte)Math.max (version, CURRENT_VERSION);
		
		dout.writeByte (version);
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPProtocolModule", "doHandshake");
		
		return version;
	}
	
	/** This method is used to propogate a shutdown notice to all open protocol modules. */	
	protected static void notifyAllModules () {
		log.entering ("com.sun.grid.jgrid.server.JCEPProtocolModule", "notifyAllModules");
		
		Iterator i = moduleList.iterator ();
		
		while (i.hasNext ()) {
			JCEPProtocolModule module = (JCEPProtocolModule)i.next ();
			
			try {
				module.notifyShutdown ();
			}
			catch (IOException e) {
				/* At this point, an exception is pretty moot.  Besides, we don't really
				 * care about other modules' exceptions. */
			}
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPProtocolModule", "notifyAllModules");
	}
	
	/** This method tells the protocol module to begin reading messages from the
	 * established connection.
	 * @throws IOException Thrown if there's an error starting up the connection
	 */	
	abstract void startConnection () throws IOException;
	/** This method shuts down the protocol module.  It causes the protocol module to
	 * stop reading and closes the input and output streams.
	 * @throws IOException Thrown is there's an error closing the input or output streams
	 */	
	abstract void closeConnection () throws IOException;
	/** Tells the protocol module to notify its client of a message from an executing
	 * job.  Used by the JCEPHandler.
	 * @param jobId The id of the job from which the message originated
	 * @param message The text of the message
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void logMessage (String jobId, String message) throws IOException;
	/** Tells the protocol module to notify its client of an error from an executing
	 * job.  Used by the JCEPHandler.
	 * @param error The text of the error message
	 * @param jobId The id of the job from which the error originated
	 * @throws IOException Thrown if there's an error sending the error message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void logError (String jobId, String error) throws IOException;
	/** Tells the protocol module to notify its client that a job has started execution.
	 * Used by the JCEPHandler.
	 * @param jobId The id of the job which started
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobStarted (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has completed
	 * execution normally.  Used by the JCEPHandler.
	 * @param jobId The id of the job which completed execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobCompleted (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has been checkpointed.
	 * Used by the JCEPHandler.
	 * @param jobId The id of the job which was checkpointed
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobCheckpointed (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally by being canceled.  Used by the JCEPHandler.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobStopped (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally because of an error during execution.  Used by the JCEPHandler.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobFailed (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has been
	 * suspended.  Used by the JCEPHandler.
	 * @param jobId The id of the job which has stopped suspended execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobSuspended (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that a job has been
	 * resumed.  Used by the JCEPHandler.
	 * @param jobId The id of the job which has resumed execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyJobResumed (String jobId) throws IOException;
	/** Tells the protocol module to notify its client that server is shutting down.
	 * Used by the JCEPHandler.
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	abstract void notifyShutdown () throws IOException;
}