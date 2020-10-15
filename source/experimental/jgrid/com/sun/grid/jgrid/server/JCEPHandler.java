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
 * JCEPHandler.java
 *
 * Created on May 16, 2003, 10:54 AM
 */

package com.sun.grid.jgrid.server;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.logging.*;

import com.sun.grid.jgrid.Job;
import com.sun.grid.jgrid.JobCanceledException;
import com.sun.grid.jgrid.LogListener;
import com.sun.grid.jgrid.NotInterruptableException;
import com.sun.grid.jgrid.proxy.ResultChannel;

/** The JCEPHandler handles all JCEP communications.  The main method listens for
 * incoming socket requests and spins off a new thread to handle each.  The new
 * thread runs an instance of JCEPHandler which them gets a JCEPProtocolModule
 * to handle the socket communications.  When a client makes a request, the
 * JCEPProtocolModule forwards the request to the JCEPHandler, which them
 * performs the requested action.
 * @author dan.templeton@sun.com
 * @version 1.8
 * @since 0.2
 */
public class JCEPHandler implements JCEPListener, LogListener {
	/** The default registry host to use if none is specified on the command line */	
	private static final String REGISTRY_HOST = "localhost";
	/** The default registry port to use if none is specified on the command line */	
	private static final int REGISTRY_PORT = ResultChannel.PORT;
	/** The default amount of number of miliseconds to wait for jobs to complete
	 * after a shutdown before explicitly exiting the VM. */
	private static final long GRACE_PERIOD = 30*1000;
	/** The usage information String */	
	private static final String USAGE = 
		"Usage: java -Djava.rmi.server.codebase=codebase " +
		"com.sun.grid.jgrid.server.JCEPHandler ([-port port] " +
		"[-regport port] [-reghost host] [-grace sec] [-debug]) | [-help]";
	/** A Map of running and completed Jobs */	
	private static Map jobMap = null;
	/** The thread running the main() method.  Used to interrupt the server on a
	 * shutdown command.
	 */	
	private static Thread mainThread = null;
	/** The ResultChannel retreived from the registry server to be used for returning
	 * results
	 */	
	private static ResultChannel resultChannel = null;
	/** Whether debug messages should be printed */	
	private static boolean debug = false;
	/** The hostname for the RMI lookup server */	
	private static String registryHost = REGISTRY_HOST;
	/** The port number for the RMI lookup server */	
	private static int registryPort = REGISTRY_PORT;
	/** The port number on which to listen for incoming requests */	
	private static int port = JCEP.PORT;
	/** The logging mechanism */	
	private static Logger log = null;
	/** The amount of time to wait for jobs to complete after a shutdown before
	 * explicitly exiting the VM. */
	private static long gracePeriod = GRACE_PERIOD;
	/** The number of running jobs in this engine */
	private static int numberOfRunningJobs = 0;
	/** The JCEPProtocolModule which is handling the client communications */	
	private JCEPProtocolModule connection = null;
	
	/** Creates a new instance of JCEPHandler
	 * @param socket The socket connection to be used for communicating with the client
	 * @throws IOException Thrown when an error occurs while establishing a client connection
	 */
	private JCEPHandler (Socket socket) throws IOException {
		connection = JCEPProtocolModule.establishConnection (this, socket);
		connection.startConnection ();
	}
	
	/** Starts up the server, which listens for incoming socket connections.
	 * @param args The command line arguments
	 * @throws Exception Thrown when something goes wrong.  This is a bad thing and should be fixed
	 * before turning this loose on the public.
	 */
	public static void main (String[] args) throws Exception {
		prepareLogging ();
		processArguments (args);
		Runtime.getRuntime ().addShutdownHook (new LogFlusher ());
		
		log.log (Level.FINER, "Starting JCEPHandler");

		jobMap = new HashMap ();
		mainThread = Thread.currentThread ();
		
		try {
			connectToResultChannel ();
		}
		catch (RemoteException e) {
			System.err.println ("Unable to connect to the result channel.");
			System.err.println ("Please make sure the compute proxy is running.");
			
			System.exit (1);
		}
		
		log.log (Level.FINEST, "Creating server socket");
		
		ServerSocket incoming = new ServerSocket (JCEP.PORT);
		
		log.log (Level.FINEST, "Accepting connections");
		
		System.out.println ("Ready");
		
		while (true) {
			Socket socket = null;
			
			try {
				socket = incoming.accept ();
			}
			catch (InterruptedIOException e) {
				log.log (Level.FINER, "Shutting down");

				break;
			}
			
			log.log (Level.FINEST, "Connection accepted");
			
			new JCEPHandler (socket);
		}
		
		/*I break this out to its own local variable so that the number doesn't
		 * change between the if and the log entry. */
		int numJobs = getNumberOfJobs ();
		
		if (numJobs > 0) {
			/** This routine waits for a given amount of time and then exits the VM
			 * explicitly.  This is to prevent jobs that claim to be interruptable but
			 * are not from making the engine hang indefinitely. */
			log.finest ("Number of jobs still in engine: " + numJobs);
			log.finer ("Main thread beginning grace period");

			try {
				Thread.sleep (gracePeriod);
			}
			catch (InterruptedException e) {
				/* Since this grace period is rather optional, if we get interrupted
				 * just go ahead and exit. */
				log.finest ("Main thread interrupted");
			}

			log.finer ("Main thread finished grace period; exiting VM");
		}
		
		System.exit (0);
	}
	
	/** Parses the command line arguments and acts accordingly
	 * @param args The command line arguments
	 */	
	private static void processArguments (String[] args) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "processArguments");
		
		int count = 0;
		String error = null;
		
		while (count < args.length) {
			if (args[count].equals ("-help")) {
				System.out.println (USAGE);
				System.out.println ("\t-port = port number for this server");
				System.out.println ("\t-reghost = hostname of the RMI registry");
				System.out.println ("\t-regport = port number of the RMI registry");
				System.out.println ("\t-grace = number of seconds to wait for jobs to complete");
				System.out.println ("\t-debug = turn on debuging output");
				System.out.println ("\t-help = print this message");
				System.out.println ("");
				
				System.exit (0);
			}
			else if (args[count].equals ("-debug")) {
				count++;
				debug = true;
				
				Handler ha = new StreamHandler (System.out, new SimpleFormatter ());
				
				log.addHandler (ha);
				ha.setLevel (Level.ALL);
				log.setLevel (Level.ALL);
				
				log.log (Level.CONFIG, "Debugging enabled");
			}
			else if (args[count].equals ("-reghost")) {
				count++;
				
				if (count < args.length) {
					registryHost = args[count++];
				
					log.log (Level.CONFIG, "Registry server hostname set to " + registryHost);
				}
				else {
					error = "Must specify a registry server host name";
					break;
				}
			}
			else if (args[count].equals ("-regport")) {
				count++;
				
				if (count < args.length) {
					try {
						registryPort = Integer.parseInt (args[count++]);
				
						log.log (Level.CONFIG, "Registry server port set to " + registryPort);
					}
					catch (NumberFormatException e) {
						error = "Invalid registry server port number";
						break;
					}
				}
				else {
					error = "Must specify a registry server port number";
					break;
				}
			}
			else if (args[count].equals ("-port")) {
				count++;
				
				if (count < args.length) {
					try {
						port = Integer.parseInt (args[count++]);

						log.log (Level.CONFIG, "Server port number set to " + port);
					}
					catch (NumberFormatException e) {
						error = "Invalid server port number";
						break;
					}
				}
				else {
					error = "Must specify a server port number";
					break;
				}
			}
			else if (args[count].equals ("-grace")) {
				int graceSeconds = -1;
				count++;
				
				if (count < args.length) {
					try {
						graceSeconds = Integer.parseInt (args[count++]);
					}
					catch (NumberFormatException e) {
						error = "Invalid grace period";
						break;
					}
					
					gracePeriod = (long)graceSeconds * 1000L;

					log.log (Level.CONFIG, "Grace period set to " + graceSeconds + " seconds");
				}
			}
			else {
				error = "Invalid command";
				break;
			}
		}
		
		if (error != null) {
			System.err.println (error);
			System.err.println (USAGE);
			System.exit (1);
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "processArguments");
	}
	
	/** This method sets up the Logger object
	 */	
	private static void prepareLogging () {
		log = Logger.getLogger ("com.sun.grid.jgrid.server.ComputeEngine");
		log.setUseParentHandlers (false);
		
		ConsoleHandler ha = new ConsoleHandler ();
		ha.setFormatter (new Formatter () {
			public String format (LogRecord record) {
				return formatMessage (record) + "\n";
			}
		});

		log.addHandler (ha);
		ha.setLevel (Level.INFO);
		log.setLevel (Level.INFO);
	}
	
	/** Register this JCEPHandler to receive messages from an executing Job.  Called by
	 * the JCEPProtocolModule.
	 * @see com.sun.grid.jgrid.Logger
	 * @see com.sun.grid.jgrid.Job
	 * @param jobId The id of the Job to which to attach
	 * @throws CommandFailedException thrown if this method cannot complete its task due to errors or failures
	 */	
	public void register (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "register");

		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Register command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else if (job.getState () == Job.RUNNING) {
			job.getLogger ().registerHandler (this);
		
			log.fine ("Connection has been registered with job: " + jobId);
		}
		else {
			log.finer ("Register command failed for job " + jobId + ": Job is not currently running");
			
			throw new CommandFailedException (jobId, "Job is not currently running");
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "register");
	}
	
	/** Unregister for messages from a Job.  This method may be used to detach from
	 * Jobs that are no longer running.  Called by the JCEPProtocolModule.
	 * @see #register(String)
	 * @param jobId The id of the Job from which to detach
	 * @throws CommandFailedException thrown if this method cannot complete its task due to errors or failures
	 */	
	public void unregister (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "unregister");
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Unegister command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else {
			job.getLogger ().unregisterHandler (this);
		
			log.fine ("Connection has been unregistered with job: " + jobId);
		}

		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "unregister");
	}
	
	/** Cancel the executing Job.  This is done by calling the Job.cancel() method.
	 * If the Job cannot be canceled, either because it is no longer executing or it is
	 * uninterruptable this method will notify the client by throwing a
	 * CommandFailedException.
	 * @see com.sun.grid.jgrid.Job#cancel()
	 * @param jobId The id of the Job to cancel
	 * @throws CommandFailedException thrown if this method cannot complete its task due to errors or failures
	 */	
	public void cancelJob (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "cancelJob");
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Cancel command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else if (job.getState () == Job.RUNNING) {
			try {
				job.cancel ();
		
				log.fine ("Job has been canceled: " + jobId);
			}
			catch (NotInterruptableException e) {
				log.finer ("Cancel command failed for job " + jobId + ": Job cannot be interrupted");
			
				throw new CommandFailedException (jobId, "Job cannot be interrupted");
			}
		}
		else {
			log.finer ("Cancel command failed for job " + jobId + ": Job is not currently running");
			
			throw new CommandFailedException (jobId, "Job is not currently running");
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "cancelJob");
	}
	
	/** Checkpoint the executing Job.  This is done by calling the Job.checkpoint()
	 * method.
	 * If the Job cannot be checkpointed, either because it is no longer executing or
	 * it is uninterruptable this method will notify the client by throwing a
	 * CommandFailedException.
	 * Called by the JCEPProtocolModule.
	 * @see com.sun.grid.jgrid.Job#checkpoint()
	 * @param jobId The id of the job to checkpoint
	 * @throws CommandFailedException thrown if this method cannot complete its task due to errors or failures
	 */	
	public void checkpoint (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "checkpoint");
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Checkpoint command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else if (job.getState () == Job.RUNNING) {
			try {
				job.checkpoint ();
				
				log.fine ("Job has been checkpointed: " + jobId);
			}
			catch (NotInterruptableException e) {
				log.finer ("Checkpoint command failed for job " + jobId + ": Job cannot be interrupted");
				
				throw new CommandFailedException (jobId, "Job cannot be interrupted");
			}
		}
		else {
			log.finer ("Checkpoint command failed for job " + jobId + ": Job is not currently running");
			
			throw new CommandFailedException (jobId, "Job is not currently running");
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "checkpoint");
	}
	
	/** Shutdown the JCEP server.  All Jobs are first checkpointed and then canceled.
	 * Called by JCEPProtocolModule.
	 * @see com.sun.grid.jgrid.Job#cancel()
	 * @see com.sun.grid.jgrid.Job#checkpoint()
	 */	
	public void shutdown () {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "shutdown");
	
		String jobId = null;
		Iterator i = jobMap.keySet ().iterator ();
		
		while (i.hasNext ()) {
			jobId = (String)i.next ();
			Job job = (Job)jobMap.get (jobId);
	
			/* Every job will be canceled if possible.  Jobs that aren't interruptable
			 * or that don't complete before the grace period is over will be stopped
			 * prematurely by the VM exit. */
			if (job.getState () == job.RUNNING) {
				try {
					job.cancel ();
				}
				catch (NotInterruptableException e) {
					this.logError (jobId, "Job cannot be interrupted");
				}
			}
		}
		
		try {
			connection.notifyShutdown ();
			connection.closeConnection ();
		}
		catch (IOException e) {
			e.printStackTrace ();
		}
		
		mainThread.interrupt ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "shutdown");
	}
	
	/** Submit a Job for execution.  A new thread will be created for the Job, and the
	 * Job object will be stored in the Job Map.  Called by JCEPProtocolModule.
	 * @param job A Job object representing the job to be executed
	 */	
	public void submitJob (Job job) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "submitJob");
		
		com.sun.grid.jgrid.Logger logger = new com.sun.grid.jgrid.Logger (job.getJobId ());
		
		logger.registerHandler (this);
		job.setLogger (logger);
		
		Thread computeThread = new Thread (job);

		jobMap.put (job.getJobId (), job);
		
		computeThread.start ();
		
		incrementNumberOfJobs ();
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "submitJob");
	}

	/** An error has occured in an executing job.  The error is communicated to the
	 * client via JCEPProtocolModule.logError().  Called by Logger.
	 * @param jobId The id of the Job from which the error originated
	 * @param error A String describing the error
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#logError(String, String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void logError (String jobId, String error) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "logError");
		
		try {
			connection.logError (jobId, error);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to log error for job " + jobId + ": " + error);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "logError");
	}

	/** An executing Job has sent a message.  The message is communicated to the client
	 * via JCEPProtocolModule.logMessage().  Called by Logger.
	 * @param jobId The id of the Job from which the message originated
	 * @param message The text of the message
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#logMessage(String, String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void logMessage (String jobId, String message) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "logMessage");
		
		try {
			connection.logMessage (jobId, message);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to log message for job " + jobId + ": " + message);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "logMessage");
	}

	/** An executing Job has exited abnormally by being canceled .
	 * The client is notified of the state change via
	 * JCEPProtocolModule.notifyJobStopped().  Called by Logger.
	 * @param jobId The id of the Job which exited
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobStopped(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobStopped (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobStopped");
		
		try {
			connection.notifyJobStopped (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has exited: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.severe ("Job " + jobId + " has exited but no longer exists in the job map");
		}
		else {
			try {
				resultChannel.sendException (new JobCanceledException (), jobId);
			}
			catch (RemoteException e) {
				this.logError (jobId, "Unable to sends results of job to result channel");
			}
		}
		
		decrementNumberOfJobs ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobStopped");
	}

	/** An executing Job has exited abnormally by
	 * throwing an Exception.  The client is notified of the state change via
	 * JCEPProtocolModule.notifyJobFailed().  Called by Logger.
	 * @param jobId The id of the Job which exited
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobFailed(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobFailed (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobExited");
		
		try {
			connection.notifyJobFailed (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has exited: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.severe ("Job " + jobId + " has exited but no longer exists in the job map");
		}
		else {
			Exception ex = null;
			
			try {
				resultChannel.sendException ((Exception)job.getResult (), jobId);
			}
			catch (RemoteException e) {
				this.logError (jobId, "Unable to sends results of job to result channel");
			}
		}
		
		decrementNumberOfJobs ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobFailed");
	}
	
	/** An executing Job has been checkpointed.  The information is communicated to the
	 * client via JCEPProtocolModule.notifyJobCheckpointed().  Called by Logger.
	 * @param jobId The id of the Job which has been checkpointed
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobCheckpointed(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobCheckpointed (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobCheckpointed");
		
		try {
			connection.notifyJobCheckpointed (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has been checkpointed: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobCheckpointed");
	}
	
	/** An executing Job has been suspended.  The information is communicated to the
	 * client via JCEPProtocolModule.notifyJobSuspended().  Called by Logger.
	 * @param jobId The id of the Job which has been suspended
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobSuspended(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobSuspended (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobSuspended");
		
		try {
			connection.notifyJobSuspended (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has been suspended: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobSuspended");
	}
	
	/** An executing Job has been resumed.  The information is communicated to the
	 * client via JCEPProtocolModule.notifyJobResumed().  Called by Logger.
	 * @param jobId The id of the Job which has been resumed
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobResumed(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobResumed (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobResumed");
		
		try {
			connection.notifyJobResumed (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has been resumed: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobResumed");
	}
	
	/** An executing Job has completed normally.  The client is notified of the state
	 * change via JCEPProtocolModule.notifyJobCompleted().  Called by Logger.
	 * @param jobId The id of the Job which has completed
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobCompleted(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobCompleted (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobCompleted");
		
		try {
			connection.notifyJobCompleted (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has completed: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.severe ("Job " + jobId + " has completed but no longer exists in the job map");
		}
		else {
			try {
				resultChannel.sendResult (job.getResult (), jobId);
			}
			catch (RemoteException e) {
				this.logError (jobId, "Unable to sends results of job to result channel");
			}
		}
				
		decrementNumberOfJobs ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobCompleted");
	}
	
	/** A Job has begun execution.  The client is notified of the state change via
	 * JCEPProtocolModule.notifyJobStarted().  Called by Logger.
	 * @param jobId The id of the Job which has started execution
	 * @see com.sun.grid.jgrid.server.JCEPProtocolModule#notifyJobStarted(String)
	 * @see com.sun.grid.jgrid.Logger
	 */	
	public void notifyJobStarted (String jobId) {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobStarted");
		try {
			connection.notifyJobStarted (jobId);
		}
		catch (IOException e) {
			/* Our channel is dead, so just use the log file */
			log.warning ("Unable to notify that job has been started: " + jobId);
			log.warning ("Problem caused by IOException:" + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "notifyJobStarted");
	}
	
	/** This method retrieves the ResultChannel object from the RMI registry server.
	 * @throws RemoteException if an error occurs while trying to retrieve the ResultChannel object
	 */	
	private static void connectToResultChannel () throws RemoteException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "connectToResultChannel");
		
		Registry r = LocateRegistry.getRegistry (registryHost, registryPort);
		
		try {
			resultChannel = (ResultChannel)r.lookup (ResultChannel.LOOKUP_NAME);
		}
		catch (NotBoundException e) {
			log.throwing ("com.sun.grid.jgrid.server.JCEPHandler", "connectToResultChannel", e);
			
			throw new RemoteException ("Unable to bind to result channel", e);
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "connectToResultChannel");
	}
	
	/** Increment the number of running jobs by 1 */	
	private static synchronized void incrementNumberOfJobs () {
		numberOfRunningJobs++;
		
		log.finest ("Number of running jobs increased to " + numberOfRunningJobs);
	}
	
	/** Decrement the number of running jobs by 1 */	
	private static synchronized void decrementNumberOfJobs () {
		numberOfRunningJobs--;
		
		log.finest ("Number of running jobs decreased to " + numberOfRunningJobs);
	}

	/** Get the number of running jobs.
	 * @return the number of running jobs
	 */	
	private static synchronized int getNumberOfJobs () {
		return numberOfRunningJobs;
	}
	
	/** Flushes the logs.  Used in a shutdown hook to make sure all information gets
	 * displayed in the event the server is stopped.
	 */	
	private static void flushLogs () {
		Handler[] handlers = log.getHandlers ();
		
		for (int count = 0; count < handlers.length; count++) {
			handlers[count].flush ();
		}
	}
	
	/** Resume a suspended Job
	 * If the Job cannot be resume, either because it is not suspended or
	 * it is uninterruptable this method will notify the client by throwing a
	 * CommandFailedException.  Called by the JCEPProtocolModule.
	 * @param jobId The id of the Job to resume
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 * @see com.sun.grid.jgrid.Job#resume()
	 */	
	public void resume (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "resume");
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Resume command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else if (job.getState () == Job.SUSPENDED) {
			job.resume ();

			log.fine ("Job has been resumed: " + jobId);
		}
		else {
			log.finer ("Resume command failed for job " + jobId + ": Job is not suspended");
			
			throw new CommandFailedException (jobId, "Job is not suspended");
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "resume");
	}
	
	/** Suspend an executing Job
	 * If the Job cannot be suspended, either because it is no longer executing or
	 * it is uninterruptable this method will notify the client by throwing a
	 * CommandFailedException. Called by the JCEPProtocolModule.
	 * @param jobId The id of the Job to suspend
	 * @throws CommandFailedException if the command was unable to complete sucessfully
	 * @see com.sun.grid.jgrid.Job#suspend()
	 */	
	public void suspend (String jobId) throws CommandFailedException {
		log.entering ("com.sun.grid.jgrid.server.JCEPHandler", "suspend");
		
		Job job = (Job)jobMap.get (jobId);
		
		if (job == null) {
			log.finer ("Suspend command failed for job " + jobId + ": No such job");
			
			throw new CommandFailedException (jobId, "No such job");
		}
		else if (job.getState () == Job.RUNNING) {
			try {
				job.suspend ();
				
				log.fine ("Job has been suspended: " + jobId);
			}
			catch (NotInterruptableException e) {
				log.finer ("Suspend command failed for job " + jobId + ": Job cannot be interrupted");
				
				throw new CommandFailedException (jobId, "Job cannot be interrupted");
			}
		}
		else {
			log.finer ("Suspend command failed for job " + jobId + ": Job is not currently running");
			
			throw new CommandFailedException (jobId, "Job is not currently running");
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPHandler", "suspend");
	}
	
	/** A helper class used as a shutdown hook.  Its only purpose is to call
	 * flushLogs().
	 * @see com.sun.grid.jgrid.server.JCEPHandler#flushLogs()
	 */	
	private static class LogFlusher extends Thread {
		/** Used as a shutdown hook.  Its only purpose is to call flushLogs().
		 * @see com.sun.grid.jgrid.server.JCEPHandler#flushLogs()
		 */		
		public void run () {
			flushLogs ();
		}
	}
}