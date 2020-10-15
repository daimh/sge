/*
 * Client.java
 *
 *  Created on May 7, 2002, 5:04 PM
 */

package dant.test;

import java.io.*;
import java.rmi.*;
import java.rmi.registry.*;

import com.sun.grid.jgrid.Job;
import com.sun.grid.jgrid.server.*;

/**
 *
 * @author unknown
 * @version 1.0
 */
public class EngineClient extends Object {
	
	/** Creates new Client */
	public EngineClient() {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main(String args[]) throws Exception {
		System.setSecurityManager(new RMISecurityManager());
		
		Registry r = LocateRegistry.getRegistry(args[0], Integer.parseInt(args[1]));
		
		ComputeEngine i = (ComputeEngine)r.lookup("ComputeEngine");
		
		i.compute (new Job ("test123", new ComputeTest ()));
	}
}