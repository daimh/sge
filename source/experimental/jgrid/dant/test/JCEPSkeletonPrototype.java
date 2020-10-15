/*
 * JCEPSkeletonPrototype.java
 *
 * Created on April 1, 2003, 2:56 PM
 */

package dant.test;

import java.io.*;
import java.net.*;

import com.sun.grid.jgrid.*;
import com.sun.grid.jgrid.server.*;

/**
 *
 * @author  dant
 */
public class JCEPSkeletonPrototype implements JCEP {
	private static final Object jobLock = new Object ();
	private static final Object ackLock = new Object ();
	private static boolean acknowledged = false;
	
	/** Creates a new instance of JCEPSkeletonPrototype */
	public JCEPSkeletonPrototype () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		Socket connection = new Socket ("localhost", PORT);
		InputStream in = connection.getInputStream ();
		DataInputStream din = new DataInputStream (in);
		OutputStream out = connection.getOutputStream ();
		DataOutputStream dout = new DataOutputStream (out);
		
		dout.writeInt (HANDSHAKE);
		dout.writeByte (VERSION10);
		
		byte version = din.readByte ();
		
		if (version != VERSION10) {
			System.out.println ("Version mismatch");
			connection.close ();
			System.exit (1);
		}


		Reader r = new Reader (din);
		r.start ();
		
/*		Job job = new Job ("MyJob", new ComputeTest ());
		ByteArrayOutputStream baos = new ByteArrayOutputStream ();
		ObjectOutputStream oos = new ObjectOutputStream (baos);
		
		oos.writeObject (job);
		oos.close ();
		
		byte[] serializedJob = baos.toByteArray ();
*/		
//		dout.writeByte (SUBMIT_JOB);
//		dout.writeInt (serializedJob.length);
//		dout.write (serializedJob);
//		dout.writeUTF ("MyJob");
//		
//		synchronized (jobLock) {
//			try {
//				System.out.println ("Waiting for job");
//				jobLock.wait ();
//			}
//			catch (InterruptedException e) {
//				e.printStackTrace ();
//			}
//		}
		
		dout.writeByte (SHUTDOWN);

		Thread.sleep (100);
		
		connection.close ();
	}
	
	private static class Reader extends Thread {
		private DataInputStream din = null;
		
		Reader (DataInputStream din) {
			this.din = din;
		}
		
		public void run () {
			String jobId = null;
			
			try {
				while (true) {
					byte code = din.readByte ();
					
					switch (code) {
						case LOG_MESSAGE:
							jobId = din.readUTF ();
							String message = din.readUTF ();
							System.out.println ("Message from " + jobId + ": " + message);
							break;
						case LOG_ERROR:
							jobId = din.readUTF ();
							String error = din.readUTF ();
							System.out.println ("Error from " + jobId + ": " + error);
							break;
//						case JOB_STARTED:
//							jobId = din.readUTF ();
//							System.out.println ("Job started: " + jobId);
//							break;
//						case JOB_COMPLETE:
//							jobId = din.readUTF ();
//							System.out.println ("Job completed: " + jobId);
//							
//							synchronized (jobLock) {
//								jobLock.notify ();
//							}
//							
//							break;
//						case JOB_CHECKPOINTED:
//							jobId = din.readUTF ();
//							System.out.println ("Job checkpointed: " + jobId);
//							break;
//						case JOB_EXITED:
//							jobId = din.readUTF ();
//							System.out.println ("Job canceled or errored: " + jobId);
//							
//							synchronized (jobLock) {
//								jobLock.notify ();
//							}
//							
//							break;
						case SHUTTING_DOWN:
							System.out.println ("Server shutting down");
							
							synchronized (jobLock) {
								jobLock.notify ();
							}
							
							break;
						default:
							System.err.println ("Bad code: " + code);
					}
				}
			}
			catch (IOException e) {
				e.printStackTrace ();
			}
		}
	}
	
	private static void waitForAck () {
		try {
			synchronized (ackLock) {
				while (!acknowledged) {
					ackLock.wait ();
				}

				acknowledged = false;
			}
		}
		catch (InterruptedException e) {
			//Really don't care
		}
	}
	
	private static void notifyAck () {
		synchronized (ackLock) {
			acknowledged = true;
			ackLock.notify ();
		}
	}
}