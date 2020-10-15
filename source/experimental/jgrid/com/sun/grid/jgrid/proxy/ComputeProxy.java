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
 * ComputeProxy.java
 *
 * Created on June 19, 2002, 4:52 PM
 */

package com.sun.grid.jgrid.proxy;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.rmi.AccessException;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;
import java.rmi.StubNotFoundException;
import java.rmi.dgc.Lease;
import java.rmi.dgc.VMID;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.ObjID;
import java.rmi.server.UID;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.logging.*;

import sun.rmi.server.MarshalOutputStream;
import sun.rmi.server.UnicastRef;
import sun.rmi.transport.LiveRef;
import sun.rmi.transport.TransportConstants;

import com.sun.grid.jgrid.Computable;
import com.sun.grid.jgrid.Job;

/** This class pretends to be an RMI registry and an RMI server
 * in order to intercept RMI communications and feed them through
 * an instance of the Sun Grid Engine.
 * @author Daniel Templeton
 * @version 1.9
 * @deprecated This class has been superceeded by ComputeServer as of 0.2.1
 * @since 0.1
 */
public class ComputeProxy implements Runnable {
	/** The usage information String
	 */	
	private static final String USAGE = 
		"Usage: java -Djava.rmi.server.codebase=codebase " +
		"com.sun.grid.jgrid.proxy.ComputeProxy [-d job_path] " + 
		"[-sub submit_command] [-skel skeleton_command] [-p port] [-debug] [-help]";
	/** The ObjID for the registry
	 */	
	private static final ObjID REGISTRY_OID = new ObjID (ObjID.REGISTRY_ID);
	/** The number of the bind method on the Registry
	 */	
	private static final int REGISTRY_OP_BIND = 0;
	/** The number of the list method on the Registry
	 */	
	private static final int REGISTRY_OP_LIST = 1;
	/** The number of the lookup method on the Registry
	 */	
	private static final int REGISTRY_OP_LOOKUP = 2;
	/** The number of the rebind method on the Registry
	 */	
	private static final int REGISTRY_OP_REBIND = 3;
	/** The number of the unbind method on the Registry
	 */	
	private static final int REGISTRY_OP_UNBIND = 4;
	/** The ObjID of the DGC
	 */	
	private static final ObjID DGC_OID = new ObjID (ObjID.DGC_ID);
	/** The number of the clean method on the DGCRegistry
	 */	
	private static final int DGC_OP_CLEAN = 0;
	/** The number of the dirty method on the DGCRegistry
	 */	
	private static final int DGC_OP_DIRTY = 1;
	/** The method hash for the compute method
	 */	
	private static final long HASH_COMPUTE = 3908630515974072420L;
	/** The method hash for the computeAsynch method
	 */	
	private static final long HASH_COMPUTEASYNCH = 8164115102769782894L;
	/** The method hash for the getResults method
	 */	
	private static final long HASH_GETRESULTS = -728148089723471111L;
	/** The method hash for the isComplete method
	 */	
	private static final long HASH_ISCOMPLETE = -5321554526954319352L;
	/** The UID of this server
	 */	
	private static final UID uid = new UID ();
	/** A reference to this server
	 */	
	private static final LiveRef ref = new LiveRef (1099);
	/** The logging mechanism
	 */	
	private static Logger log = null;
	/** A place to store Lock objects referenced by job id.  When a
	 * job comes in, a Lock is created and placed in the lockbox
	 * under the job id.  The server thread then waits on that
	 * Lock (if synchronous).  The ResultChannelImpl notifies the
	 * Lock when the ComputeEngineImpl returns a result.
	 * @see ResultChannelImpl
	 * @see Lock
	 */	
	private static Map lockbox = Collections.synchronizedMap (new HashMap ());
	/** The next job id to be assigned
	 */	
	private static long nextProcessId = Long.MIN_VALUE;
	/** The path to the serialized job files
	 */	
	private static String jobPath = "ser/";
	/** The command used to submit job to the grid engine queue
	 */	
	private static String submitCommand = "qsub -notify";
	/** The command for the grid engine execution engine to run on
	 * the execution host
	 */	
	private static String skeletonCommand = "skel.sh submit";
	/** The port on which to start the fake server
	 */	
	private static int port = 1099;
	/** Whether debug information should be printed
	 */	
	private static boolean debug = false;
	/** The Socket over which communications will take place
	 */	
	private Socket socket;
	
	/** Creates a new instance of ComputeProxy
	 * @param socket The Socket over which communications will take place
	 */
	private ComputeProxy (Socket socket) {
		this.socket = socket;
	}
	
	/** The main method creates a registry on 1100 for return channel
	 * communications and creates a fake registry on 1099 to intercep
	 * RMI calls.
	 * @param args the command line arguments
	 */
	public static void main (String[] args) {
		prepareLogging ();
		processArguments (args);
		
		log.log (Level.FINER, "Starting ComputeProxy");
		
		log.log (Level.FINEST, "Setting SecurityManager");
		System.setSecurityManager (new RMISecurityManager ());
		
		ResultChannelImpl resultChannel = null;
		ServerSocket ss = null;
		
		//Prepare the return data channel
		log.log (Level.FINEST, "Registering ResultChannel");
		try {
			resultChannel = new ResultChannelImpl (lockbox);
			Registry r = LocateRegistry.createRegistry (ResultChannel.PORT);
			r.rebind (ResultChannel.LOOKUP_NAME, resultChannel);
		}
		catch (RemoteException e) {
			log.severe ("Unable to establish return data channel: " + e.getMessage ());
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "main", e);
			
			System.exit (1);
		}
		
		//Prepare to receive connections
		log.log (Level.FINEST, "Opening ServerSocket");
		try {
			ss = new ServerSocket (1099);
			
			System.out.println ("Ready.");
			
			//When we get a connection, spawn a thread to handle it
			while (true) {
				log.log (Level.FINE, "Accepting connections");
				Socket s = ss.accept ();
				
				log.log (Level.FINE, "Received connection -- spawing handler");
				Thread t = new Thread (new ComputeProxy (s));
				t.start ();
			}
		}
		catch (IOException e) {
			log.severe ("Error while handling connection: " + e.getMessage ());
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "main", e);
			
			System.exit (1);
		}
		finally {
			try {
				log.log (Level.FINEST, "Closing ServerSocket");
				ss.close ();
			}
			catch (IOException e2) {
				//Don't care
				log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "main", e2);
			}
		}
		
		log.log (Level.FINER, "Exiting ComputeProxy");

		flushLogs ();
	}
	
	/** This method processes the command line arguments and sets
	 * the appropriate variables
	 * @param args The args array from main()
	 */	
	private static void processArguments (String[] args) {
		int count = 0;
		String error = null;
		
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processArguments");
		
		while (count < args.length) {
			if (args[count].equals ("-d")) {
				count++;
				
				if (count < args.length) {
					String path = args[count++];
					
					File file = new File (path);
					
					if (!file.exists ()) {
						error = "Specified job path does not exist.";
						
						break;
					}
					
					if (path.endsWith ("/")) {
						jobPath = path;
					}
					else {
						jobPath = path + "/";
					}
				}
				else {
					error = USAGE;
					break;
				}
				
				log.log (Level.FINE, "Job path set to " + jobPath);
			}
			else if (args[count].equals ("-sub")) {
				count++;
				
				if (count < args.length) {
					submitCommand = args[count++];
				}
				
				log.log (Level.FINE, "Submit command set to \"" + submitCommand + "\"");
			}
			else if (args[count].equals ("-skel")) {
				count++;
				
				if (count < args.length) {
					skeletonCommand = args[count++];
				}
				
				log.log (Level.FINE, "Skeleton command set to \"" + skeletonCommand + "\"");
			}
			else if (args[count].equals ("-debug")) {
				count++;
				debug = true;
				
				Handler ha = new StreamHandler (System.out, new SimpleFormatter ());
				
				log.addHandler (ha);
				ha.setLevel (Level.ALL);
				log.setLevel (Level.ALL);
				
				log.log (Level.FINE, "Debugging enabled");
			}
			else if (args[count].equals ("-p")) {
				count++;
				
				if (count < args.length) {
					try {
						port = Integer.parseInt (args[count]);
					}
					catch (NumberFormatException e) {
						error = "Inavlid port number: " + args[count];
						
						break;
					}
					
					count++;
				}
				
				log.log (Level.FINE, "Port set to " + Integer.toString (port));
			}
			else if (args[count].equals ("-help")) {
				System.out.println (USAGE);
				System.out.println ("\t-d = set the path where job files are written");
				System.out.println ("\t\tdefaults to \"./ser/\"");
				System.out.println ("\t-sub = set the command used to submit jobs to the grid");
				System.out.println ("\t\tdefaults to \"qsub\"");
				System.out.println ("\t-skel = set the command to be run on the execution host");
				System.out.println ("\t\tdefaults to \"skel\"");
				System.out.println ("\t-p = set the port number for the RMI registry");
				System.out.println ("\t\tdefaults to 1099");
				System.out.println ("\t-debug = turn on debugging messages");
				System.out.println ("\t-help = print this message");
				System.out.println ("");
				
				break;
			}
			else {
				error = USAGE;
				break;
			}
		}
		
		if (error != null) {
			System.err.println (error);
			System.exit (1);
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processArguments");
	}
	
	/** This method sets up the Logger object
	 */	
	private static void prepareLogging () {
		log = Logger.getLogger ("com.sun.grid.jgrid.proxy.ComputeProxy");
		log.setUseParentHandlers (false);
		
		ConsoleHandler ha = new ConsoleHandler ();
		ha.setFormatter (new Formatter () {
			public String format (LogRecord record) {
				return formatMessage (record);
			}
		});

		log.addHandler (ha);
		ha.setLevel (Level.INFO);
		log.setLevel (Level.INFO);
	}
	
	/** This methods flushes the logs
	 */	
	private static void flushLogs () {
		Handler[] has = log.getHandlers ();
		
		for (int count = 0; count < has.length; count++) {
			has[0].flush ();
		}
	}
	
	/** This method begins the communications process
	 */	
	public void run () {
		byte[] buffer = new byte[256];
		
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "run");
		
		try {
			//Prepare the streams
			log.log (Level.FINEST, "Getting socket streams");
			DataInputStream in = new DataInputStream (socket.getInputStream ());
			DataOutputStream out = new DataOutputStream (new BufferedOutputStream (socket.getOutputStream ()));
			
			//Handle the JRMP handshake
			log.log (Level.FINE, "Doing handshake");
			this.doHandshake (in, out);
			
			//Open communication channel until done or error
			log.log (Level.FINE, "Handling messages");
			processConnection (in, out);
		}
		catch (EOFException e) {
			log.finer ("Connection has been closed");
		}
		catch (IOException e) {
			log.warning ("The following error occured while processing the stream: " + e.getMessage () + "\n");
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "run", e);
		}
		finally {
			//Close the socket when we're done
			log.log (Level.FINEST, "Closing socket");
			try {
				socket.close ();
			}
			catch (IOException e) {
				//Don't care
				log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "run", e);
			}
			
			socket = null;
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "run");
	}
	
	/** This method handles the initial JRMP handshake
	 * @param in the input stream
	 * @param out the outputstream
	 * @throws IOException if an error occurs during communications
	 */	
	private void doHandshake (DataInputStream in, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "doHandshake");
		
		//Read handshake
		log.log (Level.FINEST, "Reading magic, version and protocol");
		int magic = in.readInt ();
		
		if (magic != TransportConstants.Magic) {
			throw new StreamException (Integer.toString (TransportConstants.Magic), Integer.toString (magic));
		}
		
		short version = in.readShort ();
		
		if (version != TransportConstants.Version) {
			throw new StreamException (Short.toString (TransportConstants.Version), Short.toString (version));
		}
		
		byte streamType = in.readByte ();
		
		if (streamType != TransportConstants.StreamProtocol) {
			throw new StreamException (Byte.toString (TransportConstants.StreamProtocol), Byte.toString (streamType));
		}
		
		//Respond to handshake
		log.log (Level.FINEST, "Writing ip address and port");
		String socketIPAddress = socket.getInetAddress ().getHostAddress ();
		
		out.writeByte (TransportConstants.ProtocolAck);
		out.writeUTF (socketIPAddress);
		out.writeInt (socket.getPort ());
		out.flush ();
		
		//Read remainder of handshake
		log.log (Level.FINEST, "Reading ip address and port");
		String ipAddress = in.readUTF ();
		
		/* If our take on the IP address is localhost, we will accept any response
		 * for the client IP address since the client may give its actual IP address
		 * instead of localhost. */
		if (!socketIPAddress.equals (ipAddress) && !socketIPAddress.equals ("127.0.0.1")) {
			throw new StreamException (socket.getInetAddress ().getHostAddress (), ipAddress);
		}
		
		int port = in.readInt ();
		
		if (port != 0) {
			throw new StreamException ("0", Integer.toString (port));
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "doHandshake");
	}
	
	/** This method handles the connection communication
	 * @param in the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processConnection (DataInputStream in, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processConnection");
		
		while (true) {
			byte type = 0;

			//Read the type of the message
			type = in.readByte ();

			log.log (Level.FINER, "Read message of type: " + Integer.toString (type, 16));
			
			if (type == TransportConstants.Call) {
				log.log (Level.FINEST, "Handling remote method call");
				ProxyInputStream objIn = new ProxyInputStream (in);

				//Read the call header
				ObjID objId = ObjID.read (objIn);
				log.log (Level.FINER, "Read object id of " + objId.toString ());
				int opNum = objIn.readInt ();
				log.log (Level.FINER, "Read op num of " + Integer.toString (opNum));
				long hash = objIn.readLong ();				
				log.log (Level.FINER, "Read hash code of " + Long.toString (hash, 16));

				if (objId.equals (REGISTRY_OID)) {
					if (opNum == REGISTRY_OP_BIND) {
						log.log (Level.FINE, "Handling call to Registry.bind()");
						processBindCall (objIn, out);
					}
					else if (opNum == REGISTRY_OP_LIST) {
						log.log (Level.FINE, "Handling call to Registry.list()");
						processListCall (objIn, out);
					}
					else if (opNum == REGISTRY_OP_LOOKUP) {
						log.log (Level.FINE, "Handling call to Registry.lookup()");
						processLookupCall (objIn, out);
					}
					else if (opNum == REGISTRY_OP_REBIND) {
						log.log (Level.FINE, "Handling call to Registry.rebind()");
						processRebindCall (objIn, out);
					}
					else if (opNum == REGISTRY_OP_UNBIND) {
						log.log (Level.FINE, "Handling call to Registry.unbind()");
						processUnbindCall (objIn, out);
					}
				}
				else if (objId.equals (DGC_OID)) {
					if (opNum == DGC_OP_CLEAN) {
						log.log (Level.FINE, "Handling call to DGC.clean()");
						processCleanCall (objIn, out);
					}
					else if (opNum == DGC_OP_DIRTY) {
						log.log (Level.FINE, "Handling call to DGC.dirty()");
						processDirtyCall (objIn, out);
					}
				}
				else if (objId.equals (ref.getObjID ())) {
					processCall (objIn, out, opNum, hash);
				}
			}
			else if (type == TransportConstants.Ping) {
				//Acknowledge the ping
				log.log (Level.FINE, "Responding to ping");
				out.writeByte (TransportConstants.PingAck);

				out.flush ();
			}
			else {
				throw new StreamException ("a valid message type", Byte.toString (type));
			}
		}
	}
	
	/** This method handles calls to the bind method of the Registry
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processBindCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processBindCall");
		//Read arguments
		try {
			/* We don't bother casting the arguments to their proper types because
			 * we're not going to use them.  We're just reading them to get them
			 * off the stream. */
			Object name = objIn.readObject ();
			Object stub = objIn.readObject ();
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new StubNotFoundException ("Unable to find stub class", e));
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "processBindCall", e);
			
			return;
		}
		
		this.processException (out, new AccessException ("Bind operation not permitted on this server"));
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processBindCall");
	}
	
	/** This method handles calls to the list method of the Registry
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processListCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processListCall");
		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write return header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);
		
		//Write a list of valid lookup names
		log.log (Level.FINEST, "Writing list of valid lookup names");
		objOut.writeObject (new String[] {"ComputeEngine"});

		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processListCall");
	}
	
	/** This method handles calls to the lookup method of the Registry
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processLookupCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processLookupCall");
		//Read argument
		String lookupName = null;
		
		try {
			lookupName = (String)objIn.readObject ();
			log.log (Level.FINE, "Lookup call for " + lookupName);
		}
		catch (ClassNotFoundException e) {
			processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "processLookupCall", e);
			
			return;
		}
		catch (ClassCastException e) {
			this.processException (out, new RemoteException ("Incorrect argument type", e));
			log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "processLookupCall", e);
			
			return;
		}

		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write return header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);
		
		//Write a stub that points back to this port
		log.log (Level.FINEST, "Writing new stub");
		/* This used to write out a new stub, but since I've removed the stub code
		 * from the source base, I'm just writing out null here so I can get it to
		 * compile.  This won't work, but this class has been deprecated, so I don't
		 * really care. */
		objOut.writeObject (null);

		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processLookupCall");
	}
	
	/** This method handles calls to the rebind method of the Registry
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processRebindCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processRebindCall");
		//Read arguments
		try {
			/* We don't bother casting the arguments to their proper types because
			 * we're not going to use them.  We're just reading them to get them
			 * off the stream. */
			Object name = objIn.readObject ();
			Object stub = objIn.readObject ();
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new StubNotFoundException ("Unable to find stub class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processRebindCall", e);
			return;
		}
		
		this.processException (out, new AccessException ("Rebind operation not permitted on this server"));
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processRebindCall");
	}
	
	/** This method handles calls to the unbind method of the Registry
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processUnbindCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processUnbindCall");
		//Read arguments
		try {
			/* We don't bother casting the argument to its proper types because
			 * we're not going to use it.  We're just reading it to get it
			 * off the stream. */
			Object name = objIn.readObject ();
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processUnbindCall", e);
			
			return;
		}
		
		this.processException (out, new AccessException ("Unbind operation not permitted on this server"));
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processUnbindCall");
	}
	
	/** This method handles calls to the clean method of the DGC
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processCleanCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processCleanCall");
		//Read arguments
		log.log (Level.FINEST, "Reading arguments");
		try {
			Object ids = objIn.readObject ();
			long seqNumber = objIn.readLong ();
			Object vmid = objIn.readObject ();
			boolean bool = objIn.readBoolean ();
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processCleanCall", e);
			
			return;
		}

		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write message header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);
		
		/* We don't do anything with this call because we don't have any actual
		 * objects to keep track of. */

		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processCleanCall");
	}
	
	/** This method handles calls to the dirty method of the DGC
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processDirtyCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processDirtyCall");
		
		//Read arguments
		ObjID[] ids = null;
		long seqNum = -1L;
		Lease lease = null;
		
		log.log (Level.FINEST, "Reading arguments");
		try {
			ids = (ObjID[])objIn.readObject ();
			seqNum = objIn.readLong ();
			lease = (Lease)objIn.readObject ();
			
			if (ids.length > 0) {
				log.log (Level.FINE, "Dirty call for " + ids[0].toString ());
			}
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processDirtyCall", e);
			
			return;
		}
		catch (ClassCastException e) {
			this.processException (out, new RemoteException ("Incorrect argument type", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processDirtyCall", e);
			
			return;
		}

		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write message header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);

		//If the lease we were passed has a valid VM ID, write it
		if (lease.getVMID () != null) {
			log.log (Level.FINEST, "Writing old lease object");
			objOut.writeObject (lease);
		}
		//If not, create a valid lease and write it
		else {
			log.log (Level.FINEST, "Writing new lease object");
			objOut.writeObject (new Lease (new VMID (), lease.getValue ()));
		}

		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processDirtyCall");
	}
	
	/** This method handles calls to com.sun.grid.jgrid.ComputeEngine methods
	 * @param opNum The number of the method called - will always be -1
	 * @param hash The hash of the method called
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processCall (ObjectInputStream objIn, DataOutputStream out, int opNum, long hash) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processCall");
		if (((opNum == -1) && (hash == HASH_COMPUTE)) || (opNum == 0)) {
			log.log (Level.FINE, "Handling call to compute()");
			this.processComputeCall (objIn, out, false);
		}
		else if (((opNum == -1) && (hash == HASH_COMPUTEASYNCH)) || (opNum == 1)) {
			log.log (Level.FINE, "Handling call to compute(Asynch)");
			this.processComputeCall (objIn, out, true);
		}
		else if (((opNum == -1) && (hash == HASH_GETRESULTS)) || (opNum == 2)) {
			log.log (Level.FINE, "Handling call to getResults()");
			this.processGetResultsCall (objIn, out);
		}
		else if (((opNum == -1) && (hash == HASH_ISCOMPLETE)) || (opNum == 3)) {
			log.log (Level.FINE, "Handling call to isComplete()");
			this.processIsCompleteCall (objIn, out);
		}
		else {
			throw new StreamException ("a valid method hash", Long.toString (hash));
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processCall");
	}
	
	/** This method handles calls to the compute and computeAsynch methods of ComputeEngine
	 * @param asynch whether this call should be asynchronous
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processComputeCall (ObjectInputStream objIn, DataOutputStream out, boolean asynch) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processComputeCall");
		//Read argument
		Computable job = null;
		
		try {
			job = (Computable)objIn.readObject ();
			log.log (Level.FINE, "Compute call with " + job.getClass ().getName ());
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processComputeCall", e);
			
			return;
		}
		catch (ClassCastException e) {
			this.processException (out, new RemoteException ("Incorrect argument type", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processComputeCall", e);
			
			return;
		}
		
		//Get annotation information so we know where it came from
		log.log (Level.FINEST, "Getting annotation information");
		String className = job.getClass ().getName ();
		Map annotations = ((ProxyInputStream)objIn).getAnnotations ();
		String annotation = (String)annotations.get (className);
		
		//Process the compute call
		Object returnValue = this.process (job, annotation, asynch);

		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write message header
		if (returnValue instanceof Throwable) {
			log.log (Level.FINER, "Exceptional return");
			objOut.writeByte (TransportConstants.ExceptionalReturn);

			//If the exception is a RemoteException, wrap it in a RemoteException
			if (!(returnValue instanceof RemoteException)) {
				log.log (Level.FINEST, "Wrapping exception in RemoteException");
				returnValue = new RemoteException ("An error occured while processing the Computable object", (Throwable)returnValue);
			}
		}
		else {
			log.log (Level.FINER, "Normal return");
			objOut.writeByte (TransportConstants.NormalReturn);
		}

		uid.write (objOut);
		
		//Write results
		log.log (Level.FINEST, "Writing results");
		objOut.writeObject (returnValue);

		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processComputeCall");
	}
	
	/** This method handles calls to the getResults method of ComputeEngine
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processGetResultsCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processGetResultsCall");
		
		//Read argument
		String id = null;
		
		try {
			id = (String)objIn.readObject ();
			log.log (Level.FINE, "GetResults call for " + id);
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processGetResultsCall", e);
			
			return;
		}
		catch (ClassCastException e) {
			this.processException (out, new RemoteException ("Incorrect argument type", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processGetResultsCall", e);
			
			return;
		}
		
		log.log (Level.FINEST, "Retrieving lock");
		Object lock = lockbox.get (id);

		if (lock == null) {
			this.processException (out, new RemoteException ("Invalid processId"));
			
			return;
		}
		
		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write message header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);
		
		//Write results
		log.log (Level.FINEST, "Writing results");
		if (lock instanceof Lock) {
			log.log (Level.FINE, "Process is not complete");
			objOut.writeObject (null);
		}
		else {
			log.log (Level.FINE, "Process is complete");
			objOut.writeObject (lock);
			lockbox.remove (id);
		}
		
		objOut.flush ();

		log.log (Level.FINER, "Deleting job file");
		this.deleteJob (id);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processGetResultsCall");
	}
	
	/** This method handles calls to the isComplete method of ComputeEngine
	 * @param objIn the input stream
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processIsCompleteCall (ObjectInputStream objIn, DataOutputStream out) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processIsCompleteCall");
		
		//Read argument
		String id = null;
		
		try {
			id = (String)objIn.readObject ();
			log.log (Level.FINE, "IsComplete call for " + id);
		}
		catch (ClassNotFoundException e) {
			this.processException (out, new RemoteException ("Unable to find class", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processIsCompleteCall", e);
			
			return;
		}
		catch (ClassCastException e) {
			this.processException (out, new RemoteException ("Incorrect argument type", e));
			log.throwing ("com.sun.grid.jgrid.ComputeEngine", "processIsCompleteCall", e);
			
			return;
		}
		
		log.log (Level.FINEST, "Retrieving lock");
		Object lock = lockbox.get (id);

		if (lock == null) {
			this.processException (out, new RemoteException ("Invalid processId"));
			
			return;
		}
		
		//Write message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);

		MarshalOutputStream objOut = new MarshalOutputStream (out);

		//Write message header
		objOut.writeByte (TransportConstants.NormalReturn);
		uid.write (objOut);
		
		//Write results
		log.log (Level.FINEST, "Writing results");
		if (lock instanceof Lock) {
			log.log (Level.FINE, "Process is not complete");
			objOut.writeBoolean (false);
		}
		else {
			log.log (Level.FINE, "Process is complete");
			objOut.writeBoolean (true);
		}
		
		objOut.flush ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processIsCompleteCall");
	}
	
	/** This method handles exceptions
	 * @param e the exception encountered
	 * @param out the output stream
	 * @throws IOException if an error occurs during communications
	 */	
	private void processException (DataOutputStream out, Exception e) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "processException");
		
		//Write the message type
		log.log (Level.FINEST, "Writing return header");
		out.writeByte (TransportConstants.Return);
		
		ObjectOutputStream objOut = new ObjectOutputStream (out);
		
		//Write the return header
		objOut.writeByte (TransportConstants.ExceptionalReturn);
		uid.write (objOut);
		
		//Write the exception
		log.log (Level.FINEST, "Writing exception");
		objOut.writeObject (e);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "processException");
	}
		
	/** This method does the actual processing for calls to compute and computeAsynch on ComputeEngine
	 * @param computable the job to process
	 * @param annotation the source for the client class files
	 * @param asynch whether this job should be executed asynchronously
	 * @throws IOException if an error occurs during communications
	 * @return the result of the job is synchronous or null if asycnhronous
	 */	
	private Object process (Computable computable, String annotation, boolean asynch) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "process");
		
		String processId = this.getNextProcessId ();
		log.log (Level.FINE, "Process id is " + processId);
		
		Job job = new Job (processId, computable, asynch);

		Object result = this.submitJob (job);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "process");
                
		return result;
	}
	
	/** This method returns the next job id
	 * @return the next job id
	 */	
	private static String getNextProcessId () {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "getNextProcessId");
		long id;
		
		synchronized (ComputeProxy.class) {
			id = nextProcessId++;
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "getNextProcessId");
		return Long.toHexString (id);
	}

	/** This method serializes the Job object to disk at the location
	 * given by jobPath.  The job file name is the job id.
	 * @param job the job to be serialized
	 * @param annotation the location of the client class files
	 * @throws IOException if an error occurs while writing the job to disk
	 */	
	private void writeJobToDisk (Job job, String annotation) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "writeJobToDisk");
		
		log.log (Level.FINEST, "Opening file for writing");
		File file = new File (jobPath + job.getJobId ());
		ObjectOutputStream oos = new ProxyOutputStream (new FileOutputStream (file));
		ObjID tempId = new ObjID (0);
		
		job.setFilename (file.getAbsolutePath ());
		((ProxyOutputStream)oos).setAnnotation (annotation);
		
		log.log (Level.FINEST, "Writing job file");
		oos.writeObject (job);
		oos.flush ();
		oos.close ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "writeJobToDisk");
	}
	
	/** This method creates a Lock object for the job and stores it
	 * in the lockbox
	 * @param processId the id of the job for which to create a lock
	 */	
	private static void setLock (String processId) {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "setLock");
		
		Object lock = new Lock ();
		
		log.log (Level.FINEST, "Storing lock");
		lockbox.put (processId, lock);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "setLock");
	}
	
	/** This method waits for the job to complete and returns the
	 * results
	 * @param processId the job id
	 * @return results of the job
	 */	
	private static Object getResult (String processId) {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "getResult");
		
		Object lock = lockbox.get (processId);
		Object results = null;
		
		log.log (Level.FINEST, "Waiting for lock");
		
		flushLogs ();
		
		if (lock instanceof Lock) {
			synchronized (lock) {
				try {
					lock.wait ();
				}
				catch (InterruptedException e) {
					//Don't care
					log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "getResult", e);
				}
			}

			log.log (Level.FINEST, "Getting results");		
			results = lockbox.get (processId);
		}
		else {
			results = lock;
		}
		
		log.log (Level.FINEST, "Removing lock");
		lockbox.remove (processId);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "getResult");
		
		return results;
	}
	
	/** This method removes a job from the lockbox and disk.
	 * @param id the id of the job to remove
	 * @throws IOException if an error occurs while removing the job from disk
	 */	
	private void deleteJob (String id) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeProxy", "deleteJob");
		
		File file = new File (jobPath + id);
		
		log.log (Level.FINEST, "Deleting job file");
		file.delete ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeProxy", "deleteJob");
	}
	
	/** This Exception is used to signal that an error occured
	 * while reading from the input stream that was not mechanical.
	 */	
	private class StreamException extends IOException {
		
		/** What was expected
		 */
		private String wanted;
		
		/** What was actually read
		 */		
		private String  got;
		
		/** Creates a new StreamException
		 * @param wanted what was expected
		 * @param got what was actually read
		 */		
		StreamException (String wanted, String got) {
			this.wanted = wanted;
			this.got = got;
		}
		
		/** Returns the exception error message
		 * @return the exception error message
		 */		
		public String getMessage () {
			return "Expected to read: \"" + wanted + "\", actually read: \"" + got + "\".";
		}
	}
	
	private Object submitJob (Job job) throws IOException {
		Object result = null;
		
		log.log (Level.FINE, "Writing job to disk");
		this.writeJobToDisk (job, job.getAnnotation ());
		
		this.setLock (job.getJobId ());
		
		if (submitCommand == null) {
/*			SGEAdapter adapter = SGEAdapter.getSGEAdapter ();
			
			if (!adapter.isSetup ()) {
				adapter.setupGDI ();
			}
			
			adapter.submitJob (skeletonCommand, job.getJobId ());

			log.log (Level.FINER, "Waiting for results");
			result = this.getResult (job.getJobId ());

			log.log (Level.FINER, "Deleting job");
			this.deleteJob (job.getJobId ());*/
		}
		else {
			//Create command string
			String[] cmd = createCommandStringArray (new String[] {submitCommand, skeletonCommand, job.getJobId (), "-d", jobPath});

			//Execute command string
			log.log (Level.FINE, "Submitting job to queue");
			Process proc = Runtime.getRuntime ().exec (cmd);

			if (!job.isAsynch ()) {
				//Wait until the process completes
				try {
					log.log (Level.FINEST, "Waiting for process to complete");
					proc.waitFor ();
				}
				catch (InterruptedException e) {
					//Don't care
					log.throwing ("com.sun.grid.jgrid.proxy.ComputeProxy", "process", e);
				}

				if (proc.exitValue () == 1) {
					log.severe ("Unable to execute submit command");

					return null;
				}

				log.log (Level.FINER, "Waiting for results");
				result = this.getResult (job.getJobId ());

				log.log (Level.FINER, "Deleting job");
				this.deleteJob (job.getJobId ());
			}
			else {
				//computeAsynch returns the job id as a String
				result = job.getJobId ();
			}
		}
		
		return result;
	}

	private static String[] createCommandStringArray (String[] parts) {
		StringBuffer buf = new StringBuffer ();
		
		for (int count = 0; count < parts.length; count++) {
			if (count > 0) {
				buf.append (' ');
			}
			
			buf.append (parts[count]);
		}
		
		log.finest ("Using submit command: " + buf.toString ());
		
		StringTokenizer tok = new StringTokenizer (buf.toString ());		
		String[] array = new String[tok.countTokens ()];
		int count = 0;
		
		while (tok.hasMoreTokens ()) {
			array[count++] = tok.nextToken ();
		}
		
		return array;
	}
	
	/** This class is used by ComputeProxyto wait for jobs to
	 * complete.
	 */	
	private static class Lock implements Serializable {
	}
}