/*
 * PrimeClient.java
 *
 * Created on June 7, 2002, 2:07 PM
 */

package dant.test;

import java.rmi.*;
import java.rmi.registry.*;

import com.sun.grid.jgrid.*;

/**
 *
 * @author  dant
 */
public class PrimeClient {
	
	/** Creates a new instance of PrimeClient */
	public PrimeClient () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		PrimeClient client = new PrimeClient ();
		
		System.setSecurityManager (new RMISecurityManager ());
		
		Registry r = LocateRegistry.getRegistry (args[0], Integer.parseInt (args[1]));
		ComputeEngine ce = (ComputeEngine)r.lookup ("ComputeEngine");
		
		client.go (ce);
	}
	
	private void go (ComputeEngine ce) {
		final Storage storage = new Storage ();
		
		
		Thread t1 = new Thread (new PrimeThread (1L, 100000L, ce, storage));
		Thread t2 = new Thread (new PrimeThread (100001L, 200000L, ce, storage));
		Thread t3 = new Thread (new PrimeThread (200001L, 300000L, ce, storage));
		
		t1.start ();
		t2.start ();
		t3.start ();
		
		System.out.println (storage.getTotal () + " primes found.");
	}
	
	class Storage {
		private int count = 0;
		private int total = 0;
		
		synchronized void add (int total) {
			this.total += total;
			this.count++;
			
			this.notify ();
		}
		
		synchronized int getTotal () {
			while (count < 3) {
				try {
					this.wait ();
				}
				catch (InterruptedException e) {
					//Don't care
				}
			}
			
			return total;
		}
	}
	
	class PrimeThread implements Runnable {
		long start, end;
		Storage storage;
		ComputeEngine ce;
		
		PrimeThread (long start, long end, ComputeEngine ce, Storage storage) {
			this.start = start;
			this.end = end;
			this.storage = storage;
			this.ce = ce;
		}
		
		public void run () {
			PrimeFinder pf = new PrimeFinder (start, end);
			int primes = 0;
			
			try {
				primes = ((Integer)ce.compute (pf)).intValue ();
			}
			catch (Exception e) {
				e.printStackTrace ();
			}
			
			storage.add (primes);
		}
	}
}