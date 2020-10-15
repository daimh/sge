/*
 * Client.java
 *
 *  Created on May 7, 2002, 5:04 PM
 */

package dant.test;

import java.io.*;
import java.rmi.*;
import java.rmi.registry.*;

import com.sun.grid.jgrid.*;

/**
 *
 * @author unknown
 * @version 1.0
 */
public class ComputeClient extends Thread {
	private static final boolean asynch = false;
	private static ComputeEngine ce = null;
	/** Creates new Client */
	public ComputeClient() {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main(String args[]) throws Exception {
		System.setSecurityManager(new RMISecurityManager());
		
		Registry r = LocateRegistry.getRegistry(args[0], Integer.parseInt(args[1]));
		ce = (ComputeEngine)r.lookup("ComputeEngine");
		ComputeClient cc = new ComputeClient ();
		
		System.out.println ("Starting client");
		cc.run ();
		
//		for (int count = 0; count < 10; count++) {
//			System.out.println ("Starting client " + count);
//			new ComputeClient ().start ();
//		}
//	
//		Thread.yield ();
	}
	
	public void run () {
		Object returnValue = null;

		if (!asynch) {
			//Synchronous
			try {
				returnValue = ce.compute (new ComputeTest ());
			}
			catch (Exception e) {
				e.printStackTrace ();
				return;
			}
		}
		else {
			//Asynchronous
			try {
				String id = ce.computeAsynch (new ComputeTest ());

				System.out.println ("Sleeping...");

				try {
					Thread.sleep (30000);
				}
				catch (InterruptedException e) {
				}

				System.out.println ("Retrieving results...");

				if (ce.isComplete (id)) {
					returnValue = ce.getResults (id);
				}
				else {
					returnValue = "Not Finished";
				}
			}
			catch (Exception e) {
				e.printStackTrace ();
				return;
			}
		}
		
		System.out.println("Received result: " + returnValue.toString ());
	}
}