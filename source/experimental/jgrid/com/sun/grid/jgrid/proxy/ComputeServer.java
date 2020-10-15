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
 * ComputeServer.java
 *
 * Created on November 13, 2003, 11:40 AM
 */

package com.sun.grid.jgrid.proxy;

import java.io.*;
import java.net.*;
import java.rmi.*;
import java.rmi.registry.*;
import java.rmi.server.*;
import java.util.*;
import java.util.logging.*;

import com.sun.grid.jgrid.Computable;
import com.sun.grid.jgrid.ComputeEngine;
import com.sun.grid.jgrid.ComputeException;
import com.sun.grid.jgrid.Job;

import org.ggf.drmaa.*;

/** This class is the central contact point for clients wishing to run jobs on the
 * grid.  It offers the ability to run jobs synchonously and asynchronously and to
 * check on the results of jobs.
 * @author dan.templeton@sun.com
 * @version 1.3
 * @since 0.2.1
 */
public class ComputeServer extends UnicastRemoteObject implements ComputeEngine {
	/** The name under which the ComputeServer registers in the RMI registry */	
	public static final String LOOKUP_NAME = "ComputeEngine";
	/** The usage information String
	 */	
	private static final String USAGE = 
		"Usage: java -Djava.rmi.server.codebase=codebase " +
		"com.sun.grid.jgrid.proxy.ComputeServer [-d job_path] " + 
		"[-skel skeleton_command] [-sp server_port]" +
		"[-rp return_channel_port] [-debug] [-help]";
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
	/** The command for the grid engine execution engine to run on
	 * the execution host
	 */	
	private static String skeletonCommand = "skel.sh submit";
	/** The port on which to start the server
	 */	
	private static int server_port = Registry.REGISTRY_PORT;
	/** The port for the return data channel RMI registry */	
	private static int channel_port = ResultChannel.PORT;
	/** Whether debug information should be printed
	 */	
	private static boolean debug = false;
   private JobTemplate template = null;
   private DRMAASession drmaa = null;
	
	/** Creates a new instance of ComputeServer
	 * @throws RemoteException thrown when an RMI error occurs
	 */
	public ComputeServer () throws RemoteException {
      try {
         DRMAASessionFactory factory = DRMAASessionFactory.getFactory ();
         drmaa = factory.getSession ();
         drmaa.init (null);
         template = drmaa.createJobTemplate ();
      }
      catch (DRMAAException e) {
         throw new RemoteException ("Unable to initialize DRM interface", e);
      }
      
      StringTokenizer cmd = new StringTokenizer (skeletonCommand);
      int numTokens = cmd.countTokens ();
      
      if (numTokens >= 1) {
         try {
            template.setRemoteCommand (cmd.nextToken ());
         }
         catch (DRMAAException e) {
            throw new RemoteException ("Invalid skeleton command", e);
         }
      }
      else {
         throw new RemoteException ("Invalid skeleton command");
      }

      if (numTokens > 1) {
         //-1 for the command +3 for the additional parameters
         String[] skelArgs = new String[numTokens - 1 + 3];
         int count = 0;
         
         while (cmd.hasMoreTokens ()) {
            skelArgs[count++] = cmd.nextToken ();
         }

         //JOBID is a placeholder to be filled in during submitJob()
         skelArgs[count++] = "JOBID";
         skelArgs[count++] = "-d";
         skelArgs[count++] = jobPath;
         
         try {
            template.setInputParameters (skelArgs);
         }
         catch (DRMAAException e) {
            throw new RemoteException ("Invalid skeleton command argument(s)", e);
         }
      }                  
	}
	
	/** The main method creates a registry on 1100 for return channel
	 * communications and creates a registry on 1099 to receive RMI calls.
	 * @param args the command line arguments
	 */
	public static void main (String[] args) {
		boolean createdRegistry = false;
		
		prepareLogging ();
		processArguments (args);
		
		log.log (Level.FINER, "Starting ComputeServer");
		
		log.log (Level.FINEST, "Setting security manager");
		System.setSecurityManager (new RMISecurityManager ());
		
		ComputeServer computeServer = null;
		
		//Prepare the return data channel
		log.log (Level.FINEST, "Registering compute server");
		
		try {
			computeServer = new ComputeServer ();
		}
		catch (RemoteException e) {
			log.severe ("Unable to create compute server: " + e.getMessage ());
			
			System.exit (1);
		}
		
		log.log (Level.FINEST, "Registering with RMI registry");
		
		try {
			Registry r = LocateRegistry.getRegistry (server_port);
			r.rebind (LOOKUP_NAME, computeServer);
		}
		catch (RemoteException e) {
			log.log (Level.FINEST, "Unable to connect to registry: " + e.getMessage ());
			log.log (Level.FINEST, "Starting new registry");
			//Try starting our own registry
			try {
				Registry r = LocateRegistry.createRegistry (server_port);
				r.rebind (LOOKUP_NAME, computeServer);
				
				createdRegistry = true;
			}
			catch (RemoteException e2) {
				log.severe ("Unable to start compute server: " + e2.getMessage ());
			
				System.exit (1);
			}
		}
		
		log.log (Level.FINEST, "Registered with RMI registry");
		
		ResultChannelImpl resultChannel = null;
		
		//Prepare the return data channel
		log.log (Level.FINEST, "Registering return data channel");
		
		try {
			resultChannel = new ResultChannelImpl (lockbox);
		}
		catch (RemoteException e) {
			log.severe ("Unable to create return data channel: " + e.getMessage ());
			
			System.exit (1);
		}
		
		try {
			Registry r = LocateRegistry.getRegistry (channel_port);
			r.rebind (ResultChannel.LOOKUP_NAME, resultChannel);
		}
		catch (RemoteException e) {
			log.log (Level.FINEST, "Unable to connect to registry: " + e.getMessage ());
			log.log (Level.FINEST, "Starting new registry");
			//Try starting our own registry
			try {
				Registry r = LocateRegistry.createRegistry (channel_port);
				r.rebind (ResultChannel.LOOKUP_NAME, resultChannel);
			}
			catch (RemoteException e2) {
				if (createdRegistry) {
					log.severe ("Due to a bug (#4267864) in versions of J2SE eariler than 1.5, two " +
											"registries cannot be created withing the same VM.  Please " +
											"either set the -sp and -rp options to the same port " +
											"number or start one or both of the required RMI registries "+
											"external to the ComputeServer VM.");
				}
				else {
					log.severe ("Unable to establish return data channel: " + e2.getMessage ());
				}
				
				System.exit (1);
			}
		}		
				
		log.log (Level.FINEST, "Return data channel registered with RMI registry");
		
		System.out.println("Ready.");
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "main");
		
		flushLogs ();
	}
	
	/** This method processes the command line arguments and sets
	 * the appropriate variables
	 * @param args The args array from main()
	 */	
	private static void processArguments (String[] args) {
		int count = 0;
		String error = null;
		
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "processArguments");
		
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
			else if (args[count].equals ("-sp")) {
				count++;
				
				if (count < args.length) {
					try {
						server_port = Integer.parseInt (args[count]);
					}
					catch (NumberFormatException e) {
						error = "Inavlid server port number: " + args[count];
						
						break;
					}
					
					count++;
				}
				
				log.log (Level.FINE, "Server port set to " + Integer.toString (server_port));
			}
			else if (args[count].equals ("-rp")) {
				count++;
				
				if (count < args.length) {
					try {
						channel_port = Integer.parseInt (args[count]);
					}
					catch (NumberFormatException e) {
						error = "Inavlid return data channel port number: " + args[count];
						
						break;
					}
					
					count++;
				}
				
				log.log (Level.FINE, "Return data channel port set to " + Integer.toString (channel_port));
			}
			else if (args[count].equals ("-help")) {
				System.out.println (USAGE);
				System.out.println ("\t-d = set the path where job files are written");
				System.out.println ("\t\tdefaults to \"./ser/\"");
				System.out.println ("\t-skel = set the command to be run on the execution host");
				System.out.println ("\t\tdefaults to \"skel\"");
				System.out.println ("\t-rp = set the port number for the return data channel RMI registry");
				System.out.println ("\t-sp = set the port number for the server RMI registry");
				System.out.println ("\t\tdefaults to 1099");
				System.out.println ("\t-debug = turn on debugging messages");
				System.out.println ("\t-help = print this message");
				System.out.println ("");
				
				System.exit (0);
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
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "processArguments");
	}
	
	/** This method sets up the Logger object
	 */	
	private static void prepareLogging () {
		log = Logger.getLogger ("com.sun.grid.jgrid.proxy.ComputeServer");
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
	
	/** This method returns the next job id
	 * @return the next job id
	 */	
	private static String getNextProcessId () {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "getNextProcessId");
		long id;
		
		synchronized (ComputeServer.class) {
			id = nextProcessId++;
		}
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "getNextProcessId");
		return Long.toHexString (id);
	}

	/** Creates a command string array from the given arguments
	 * @param parts The arguments to reassemble into a command string array
	 * @return the new command string array
	 */	
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
	
	/** This method should never be called.  It is an artifact of the way the
	 * RMI works.  In order for this class to be registerable with an RMI
	 * registry, it has to implement ComputeEngine, which includes this method.
	 * However, the RMI stub and skeleton have been modified to redirect calls to
	 * the compute(Computable, String) method instead.
	 * @return never actually returns
	 * @param computable the job to compute
	 * @throws RemoteException thrown when this method is called
	 */
	public java.io.Serializable compute (Computable computable) throws RemoteException {
		throw new RemoteException ("RMI Skeleton has called an invalid method.");
	}
	
	/** This method submits a job for synchrnous execution.
	 * @return the result object
	 * @param computable the job to be executed
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 */	
	public java.io.Serializable compute (Computable computable, String codebase) throws RemoteException, ComputeException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "compute");
		
		String processId = this.getNextProcessId ();
		log.log (Level.FINE, "Process id is " + processId);
		
		Job job = new Job (processId, computable, false);

		job.setAnnotation (codebase);
		log.log (Level.FINEST, "Codebase is \"" + codebase + "\"");
		
		Object result = null;
		
		try {
			result = this.submitJob (job);
		}
		catch (IOException e) {
			throw new RemoteException ("Unable to submit job", e);
		}
      catch (DRMAAException e) {
			throw new RemoteException ("Unable to submit job", e);
      }
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "compute");
                
		return (Serializable)result;
	}
	
	/** This method should never be called.  It is an artifact of the way the
	 * RMI works.  In order for this class to be registerable with an RMI
	 * registry, it has to implement ComputeEngine, which includes this method.
	 * However, the RMI stub and skeleton have been modified to redirect calls to
	 * the compute(Computable, String) method instead.
	 * @param job the job to compute
	 * @throws RemoteException thrown when this method is called
	 * @return never actually returns
	 */
	public String computeAsynch (Computable job) throws RemoteException {
		throw new RemoteException ("RMI Skeleton has called an invalid method.");
	}
	
	/** This method submits a job for asynchronous execution.  Job
	 * status can be checked with the isComplete method and the
	 * results can be retrieved by the getResults method.
	 * @return the result object
	 * @param codebase an alternate codebase to use for loading the Computable class
	 * @param computable the job to be executed
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 */	
	public String computeAsynch (Computable computable, String codebase) throws RemoteException, ComputeException {
		log.entering ("com.sun.grid.jgrid.proxy.computeAsynch", "compute");
		
		String processId = this.getNextProcessId ();
		log.log (Level.FINE, "Process id is " + processId);
		
		Job job = new Job (processId, computable, true);

		job.setAnnotation (codebase);
		
		String result = null;
		
		try {
			result = (String)this.submitJob (job);
		}
		catch (IOException e) {
			throw new RemoteException ("Unable to submit job", e);
		}
      catch (DRMAAException e) {
			throw new RemoteException ("Unable to submit job", e);
      }
		
		log.exiting ("com.sun.grid.jgrid.proxy.computeAsynch", "compute");
                
		return result;
	}
	
	/** This method retrieves the results of a job that was
	 * executed asynchronously.
	 * @return the results object
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 */	
	public java.io.Serializable getResults (String jobId) throws RemoteException, ComputeException {
		log.log (Level.FINEST, "Retrieving lock");
		Object lock = lockbox.get (jobId);

		if (lock == null) {
			throw new RemoteException ("Invalid processId");
		}
		
		log.log (Level.FINEST, "Getting results");
		if (lock instanceof Lock) {
			log.log (Level.FINE, "Process is not complete");
			return null;
		}
		else {
			log.log (Level.FINE, "Process is complete");
			lockbox.remove (jobId);
			return (Serializable)lock;
		}
	}
	
	/** This method check whether an asynchronous job has finished
	 * executing.
	 * @param jobId the id of the job
	 * @throws RemoteException if an error occurs on the server side
	 * @throws ComputeException if an error occurs during job execution
	 * @return whether the job has finished
	 */	
	public boolean isComplete (String jobId) throws RemoteException, ComputeException {
		log.log (Level.FINEST, "Retrieving lock");
		Object lock = lockbox.get (jobId);

		if (lock == null) {
			throw new RemoteException ("Invalid processId");
		}
		
		log.log (Level.FINEST, "Getting results");
		if (lock instanceof Lock) {
			log.log (Level.FINE, "Process is not complete");
			return false;
		}
		else {
			log.log (Level.FINE, "Process is complete");
			return true;
		}
	}
	
	/** Submits the job to the DRM.
	 * @param job the object to be executed
	 * @throws IOException thrown if the job cannot be spooled
	 * @return the results of running the job
	 */	
	private Object submitJob (Job job) throws IOException, DRMAAException {
		Object result = null;
		
		log.log (Level.FINE, "Writing job to disk");
		this.writeJobToDisk (job);
		
		this.setLock (job.getJobId ());
		
		//Update command args
      String[] args = template.getInputParameters ();

      //This JOBID placeholder is 3rd from the end
      args[args.length - 3] = job.getJobId ();
      template.setInputParameters (args);      

		//Execute command string
		log.log (Level.FINE, "Submitting job to queue");
      /* I don't care what the DRMAA id is because at this point the job is
       * already written to disk, so I have nowhere to store the DRMAA id.  The
       * downside is that without the DRMAA job id, I can't offer services like
       * hold, suspend, and terminate, and I can't get the job's final exit
       * status and resource usage.  This should be handled at some point,
       * by storing the job in the lock box and the results in the job... */
      drmaa.runJob (template);

		if (!job.isAsynch ()) {
			log.log (Level.FINER, "Waiting for results");
			result = this.getResult (job.getJobId ());

			log.log (Level.FINER, "Deleting job");
			this.deleteJob (job.getJobId ());
		}
		else {
			//computeAsynch returns the job id as a String
			result = job.getJobId ();
		}
		
		return result;
	}

	/** This method serializes the Job object to disk at the location
	 * given by jobPath.  The job file name is the job id.
	 * @param job the job to be serialized
	 * @throws IOException if an error occurs while writing the job to disk
	 */	
	private void writeJobToDisk (Job job) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "writeJobToDisk");
		
		log.log (Level.FINEST, "Opening file for writing");
		File file = new File (jobPath + job.getJobId ());
		ObjectOutputStream oos = new ProxyOutputStream (new FileOutputStream (file));
		
		job.setFilename (file.getAbsolutePath ());
		((ProxyOutputStream)oos).setAnnotation (job.getAnnotation ());
		
		log.log (Level.FINEST, "Writing job file");
		oos.writeObject (job);
		oos.flush ();
		oos.close ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "writeJobToDisk");
	}
	
	/** This method creates a Lock object for the job and stores it
	 * in the lockbox
	 * @param processId the id of the job for which to create a lock
	 */	
	private static void setLock (String processId) {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "setLock");
		
		Object lock = new Lock ();
		
		log.log (Level.FINEST, "Storing lock");
		lockbox.put (processId, lock);
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "setLock");
	}
	
	/** This method waits for the job to complete and returns the
	 * results
	 * @param processId the job id
	 * @return results of the job
	 */	
	private static Object getResult (String processId) {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "getResult");
		
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
					log.throwing ("com.sun.grid.jgrid.proxy.ComputeServer", "getResult", e);
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
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "getResult");
		
		return results;
	}
	
	/** This method removes a job from the lockbox and disk.
	 * @param id the id of the job to remove
	 * @throws IOException if an error occurs while removing the job from disk
	 */	
	private void deleteJob (String id) throws IOException {
		log.entering ("com.sun.grid.jgrid.proxy.ComputeServer", "deleteJob");
		
		File file = new File (jobPath + id);
		
		log.log (Level.FINEST, "Deleting job file");
		file.delete ();
		
		log.exiting ("com.sun.grid.jgrid.proxy.ComputeServer", "deleteJob");
	}
	
	/** This class is used by ComputeServer to wait for jobs to
	 * complete.
	 */	
	private static class Lock implements Serializable {
	}
}
