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
 * JCEPVersion10Module.java
 *
 * Created on April 3, 2003, 4:32 PM
 */

package com.sun.grid.jgrid.server;

import com.sun.grid.jgrid.Job;
import java.io.*;
import java.net.MalformedURLException;
import java.net.Socket;
import java.util.logging.Logger;



/** This class implements the 1.0 version of JCEP.
 * @author dan.templeton@sun.com
 * @see com.sun.grid.jgrid.server.JCEPProtocolModule
 * @version 1.7
 * @since 0.2
 */
/* I made a design decision to keep the logic of what happens when a notify call
 * fails in the JCEPHandler instead of refactoring it down into the protocol
 * modules.  My reasoning is that if the notify command fails, there is some
 * chance that the JCEPHandler may need to know about it.  If the fault handling
 * were done in the protocol module, the protocol module would still have to
 * throw some kind of exception to notify the handler, so it really wouldn't
 * make anything cleaner.  Plus, it wouldn't end up saving hardly any code in
 * the handler since the handler's reaction to failure is to assume the
 * connection's dead and move on. */
class JCEPVersion10Module extends JCEPProtocolModule implements JCEP {
	/** The logging mechanism */	
	private static Logger log = null;
	/** The input stream used to communicate with the client */	
	private DataInputStream din = null;
	/** The output stream used to communicate with the client */	
	private DataOutputStream dout = null;
	/** The JCEPListener at which to direct command received from the client */	
	private JCEPListener engine = null;
	/** The thread which listens for incoming client messages */	
	private ListenerThread reader = null;
	/** The id of the job with which this protocol module is registered to receive
	 * events
	 */	
	private String registeredJobId = null;
	/** The socket connected to the client */	
	private Socket socket = null;
	/** A flag used to prevent multiple shutdown notices */	
	private boolean shuttingDown = false;
	
	static {
		log = Logger.getLogger ("com.sun.grid.jgrid.server.ComputeEngine");
	}
	
	/** Creates a new instance of JCEPVersion10Module
	 * @param engine The JCEPListener at which to direct commands received from the client
	 * @param socket The socket to use for client communications
	 * @param din The socket input stream
	 * @param dout The socket output stream
	 */
	JCEPVersion10Module (JCEPListener engine, Socket socket, DataInputStream din, DataOutputStream dout) {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "<init>");
		
		this.socket = socket;
		this.engine = engine;
		this.din = din;
		this.dout = dout;
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "<init>");
	}
	
	/** This method tells the protocol module to begin reading messages from the
	 * established connection.
	 */	
	synchronized void startConnection () {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "startConnection");
		
		if (reader == null) {
			reader = new ListenerThread ();
			reader.start ();
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "startConnection");
	}
	
	/** Tells the protocol module to notify its client of an error from an executing
	 * job.  Used by the JCEPHandler.
	 * @param message The text of the error message
	 * @param jobId The id of the job from which the error originated
	 * @throws IOException Thrown if there's an error sending the error message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	synchronized void logError (String jobId, String message) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "logError");
		
		dout.writeByte (LOG_ERROR);
		dout.writeUTF (jobId);
		dout.writeUTF (message);
		dout.flush ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "logError");
	}
	
	/** Tells the protocol module to notify its client of a message from an executing
	 * job.  Used by the JCEPHandler.
	 * @param jobId The id of the job from which the message originated
	 * @param message The text of the message
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	synchronized void logMessage (String jobId, String message) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "logMessage");
		
		dout.writeByte (LOG_MESSAGE);
		dout.writeUTF (jobId);
		dout.writeUTF (message);
		dout.flush ();

		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "logMessage");
	}
	
	/** Tells the protocol module to notify its client that a job has completed
	 * execution normally.  Used by the JCEPHandler.
	 * @param jobId The id of the job which completed execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */		
	void notifyJobCompleted (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_COMPLETED);
	}
	
	/** Tells the protocol module to notify its client that a job has started execution.
	 * Used by the JCEPHandler.
	 * @param jobId The id of the job which started
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobStarted (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_RUNNING);
	}
	
	/** This method shuts down the protocol module.  It causes the protocol module to
	 * stop reading and closes the input and output streams.
	 */	
	void closeConnection () {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "closeConnection");

		/* This guarded section is to prevent NullPointerExceptions when nulling
		 * out the reader. */
		synchronized (this) {
			if (reader != null) {
				reader.stopReading ();
				reader = null;
			}
		}
		
		try {
			din.close ();
			dout.close ();
		}
		catch (IOException e) {
			//At this point, our error channel is closed, so just go to stderr
			e.printStackTrace ();
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "closeConnection");
	}
	
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally by being canceled.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobStopped (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_STOPPED);
	}
	
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally because of an error during execution.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobFailed (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_FAILED);
	}
	
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally because of an error during execution.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobSuspended (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_SUSPENDED);
	}
	
	/** Tells the protocol module to notify its client that a job has terminated
	 * execution abnormally because of an error during execution.
	 * @param jobId The id of the job which has stopped execution
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobResumed (String jobId) throws IOException {
		this.notifyJobStateChange (jobId, STATE_RUNNING);
	}
	
	/** Tells the protocol module to notify its client that server is shutting down.
	 * Used by the JCEPHandler.
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyShutdown () throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyShutdown");

		/* This guarded section is to prevent multiple shutdown attempts.  When a
		 * module gets a shutdown command, it tells all the other modules to
		 * shutdown.  Without this guarded section, the modules would recursively
		 * call each other until the stack overran.  As is, they each only call all
		 * the others once.  This is not a particularly elegant solution, and could
		 * be made much better by having only the original module propogate the
		 * shutdown command.  This could be done through another method, or through
		 * a field on the parent class. */
		synchronized (this) {
			if (!shuttingDown) {
				shuttingDown = true;
			}
			else {
				return;
			}
		}

		if (shuttingDown) {
			notifyAllModules ();

			/* This guarded section is to prevent sending garbled information to the
			 * client. It's a synchronized block instead of a synchronied method so
			 * that the call to notifyAllModules() doesn't cause a deadlock. */
			synchronized (this) {
				dout.writeByte (SHUTTING_DOWN);
				dout.flush ();
			}
		}
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyShutdown");
	}
	
	/** Tells the protocol module to notify its client that a job has been checkpointed.
	 * Used by the JCEPHandler.
	 * @param jobId The id of the job which was checkpointed
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	void notifyJobCheckpointed (String jobId) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyJobCheckpointed");

		dout.writeByte (JOB_CHECKPOINTED);
		dout.writeUTF (jobId);
		dout.flush ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyJobCheckpointed");
	}
	
	/** Tells the protocol module to notify its client that a previously submitted
	 * command has failed to execute.
	 * @param command the command that failed
	 * @param jobId The id of the job on which the command was supposed to operate
	 * @param message A message describing the reason for the command failure
	 * @throws IOException Thrown if there's an error sending the message to the client
	 * @see com.sun.grid.jgrid.server.JCEPHandler
	 */	
	private synchronized void notifyCommandFailed (byte command, String jobId, String message) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyCommandFailed");

		dout.writeByte (COMMAND_FAILED);
		dout.writeByte (command);
		dout.writeUTF (jobId);
		dout.writeUTF (message);
		dout.flush ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyCommandFailed");
	}
	
	/** Closes the client connection.  Used by the ListenerThread to tidy up when the
	 * client disconnects.
	 */	
	private synchronized void closeSocket () {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "closeSocket");

		try {
			socket.close ();
		}
		catch (IOException e) {
			//We don't really care as long as it's closed
		}
		
		socket = null;
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "closeSocket");
	}
	
	/** Tells the protocol module to notify its client that a job has changed
	 * state.
	 * @param jobId The id of the job which completed execution
	 * @param newState the job's new state
	 * @throws IOException Thrown if there's an error sending the message to the client
	 */		
	private synchronized void notifyJobStateChange (String jobId, byte newState) throws IOException {
		log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyJobStateChange");

		dout.writeByte (JOB_STATE_CHANGE);
		dout.writeUTF (jobId);
		dout.writeByte (newState);
		dout.flush ();
		
		log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module", "notifyJobStateChange");
	}
	
	/** This class reads incoming messages from the input stream and forwards the
	 * resulting commands through the protocol module.
	 */	
	private class ListenerThread extends Thread {
		private static final String NO_JOB_ID = "<unknown>";
		/** A flag to indicate whether the ListenerThread should continue running */		
		private boolean running = false;
		/** The thread which is running the ListenerThread.  Used to interrupt the
		 * ListenerThread when closing the connection.
		 */		
		private Thread thread = null;
		
		/** Creates a new ListenerThread. */		
		ListenerThread () {
			/* This is to make sure the listener threads run as often as possible.
			 * Since they spend most of their lives blocked waiting for I/O, this
			 * shouldn't cause any problems.  I can image a case, though, where enough
			 * listener threads were running to starve out the job threads.  It would
			 * take a whole lot of very active listener threads, though.  Perhaps a
			 * future idea would be to use some kind of IPC. */
			this.setPriority (Thread.MAX_PRIORITY);
		}
		
		/** When an object implementing interface <code>Runnable</code> is used
		 * to create a thread, starting the thread causes the object's
		 * <code>run</code> method to be called in that separately executing
		 * thread.
		 * <p>
		 * The general contract of the method <code>run</code> is that it may
		 * take any action whatsoever.
		 *
		 * @see     java.lang.Thread#run()
		 */
		public void run () {
			log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module$ListenerThread", "run");
			
			String jobId = null;
			thread = Thread.currentThread ();
			running = true;
			
			try {
				while (running) {
					byte code = 0;

					try {
						code = din.readByte ();
					}
					catch (EOFException e) {
						/* This just means the client has disconnected. */
						running = false;

						continue;
					}
					catch (IOException e) {
						/* If this thread has been interrupted, that means the IOException
						 * happened because the socket closed.  If that's the case, I just
						 * have the loop reevaluate the conditional, which should now be
						 * false, resulting in a clean drop out of the loop. */
						if (interrupted ()) {
							continue;
						}
						else {
							logErrorOrWarn (SYSTEM_ID, "Error while reading message id from client: " + e.getMessage ());
							return;
						}
					}

					log.finest ("Read code: 0x" + Integer.toString (code, 16));
					
					switch (code) {
						case SUBMIT_JOB:
							Job job = null;
							String location = null;

							try {
								location = (String)din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job location for submit from client: " + e.getMessage ());
								
								break;
							}

							try {
								FileInputStream fis = new FileInputStream (location);
								JCEPInputStream ois = new JCEPInputStream (fis);
								
								job = (Job)ois.readObject ();
								job.setAnnotation (ois.getLastAnnotation ());
							}
							catch (MalformedURLException e) {
								logErrorOrWarn (SYSTEM_ID, "Unable to load class from URL for submit: " + e.getMessage ());
								
								try {
									notifyCommandFailed (SUBMIT_JOB, NO_JOB_ID, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (job.getJobId (), "Error while writing submit failure message to client: " + e.getMessage ());
								
									break;
								}

								break;
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Unable to read job file for submit: " + location);
								
								try {
									notifyCommandFailed (SUBMIT_JOB, NO_JOB_ID, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (job.getJobId (), "Error while writing submit failure message to client: " + e.getMessage ());
								
									break;
								}
								
								break;
							}
							catch (ClassNotFoundException e) {
								logErrorOrWarn (SYSTEM_ID, "Unable to load job class for submit: " + e.getMessage ());
								
								try {
									notifyCommandFailed (SUBMIT_JOB, NO_JOB_ID, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (job.getJobId (), "Error while writing submit failure message to client: " + e.getMessage ());
								
									break;
								}

								break;
							}

							try {
								engine.submitJob (job);						
								registeredJobId = job.getJobId ();
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (SUBMIT_JOB, job.getJobId (), e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (job.getJobId (), "Error while writing submit failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case CANCEL_JOB:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job id for cancel from client: " + e.getMessage ());
							}

							try {
								engine.cancelJob (jobId);
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (CANCEL_JOB, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing cancel failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case CHECKPOINT_JOB:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job id for checkpoint from client: " + e.getMessage ());
							}

							try {
								engine.checkpoint (jobId);
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (CHECKPOINT_JOB, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing checkpoint failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case REGISTER:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (jobId, "Error while reading job id for register from client: " + e.getMessage ());
							}

							try {
								engine.register (jobId);
								registeredJobId = jobId;
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (REGISTER, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing register failure message to client: " + e.getMessage ());
								
									break;
								}
							}
								
							break;
						case UNREGISTER:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job id for unregister from client: " + e.getMessage ());
							}

							try {
								engine.unregister (jobId);
								registeredJobId = null;
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (UNREGISTER, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing unregister failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case SHUTDOWN:
							try {
								engine.shutdown ();
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (SHUTDOWN, SYSTEM_ID, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing shutdown failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case SUSPEND:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job id for suspend from client: " + e.getMessage ());
							}

							try {
								engine.suspend (jobId);
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (SUSPEND, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing suspend failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						case RESUME:
							try {
								jobId = din.readUTF ();
							}
							catch (IOException e) {
								logErrorOrWarn (SYSTEM_ID, "Error while reading job id for resume from client: " + e.getMessage ());
							}

							try {
								engine.resume (jobId);
							}
							catch (CommandFailedException e) {
								try {
									notifyCommandFailed (RESUME, jobId, e.getMessage ());
								}
								catch (IOException e2) {
									logErrorOrWarn (jobId, "Error while writing resume failure message to client: " + e.getMessage ());
								
									break;
								}
							}

							break;
						default:
					}
				}
			}
			finally {
				/* If we're still regsitered for messages from a job, unregister. */
				if (registeredJobId != null) {
					try {
						engine.unregister (registeredJobId);
					}
					catch (CommandFailedException e) {
						log.warning ("Unable to unregister client for job " + registeredJobId);
					}
				}

				closeSocket ();
				
				/* The next couple of lines would probably be better off in a method on
				 * the containing class. */
				JCEPVersion10Module.this.reader = null;
				moduleList.remove (JCEPVersion10Module.this);
			}
			
			log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module$ListenerThread", "run");
		}		
		
		/** Sets <CODE>running</CODE> to <CODE>false</CODE> and interrupts
		 * <CODE>thread</CODE>, causing the ListenerThread to stop reading from the input
		 * stream.  If the ListenerThread is interrupted while trying to read from the
		 * input stream, it simply exits quietly.
		 */		
		void stopReading () {
			log.entering ("com.sun.grid.jgrid.server.JCEPVersion10Module$ListenerThread", "stopReading");
			
			running = false;
			thread.interrupt ();
			
			log.exiting ("com.sun.grid.jgrid.server.JCEPVersion10Module$ListenerThread", "stopReading");
		}

		/** Convenience method to log the error to the client is possible.  If that
		 * fails, it logs the error to the log.
		 * @param jobId the id of the job that caused the error
		 * @param error the error to log
		 */
		private void logErrorOrWarn (String jobId, String error) {
			try {
				logError (jobId, error);
			}
			catch (IOException e) {
				/* Our channel is dead, so just use the log file */
				log.warning ("Unable to log error -- " + jobId + ": " + error);
				log.warning ("problem caused be exception: " + com.sun.grid.jgrid.Logger.getExceptionAsString (e));
			}
		}
	}
}