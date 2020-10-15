/*
 * PrimeFinder.java
 *
 * Created on June 7, 2002, 1:53 PM
 */

package dant.test;

import java.io.Serializable;
import java.util.LinkedList;

import com.sun.grid.jgrid.*;

/**
 *
 * @author  dant
 */
public class PrimeFinder implements Computable {
	private long startingWith = 2L;
	private long endingWith = -1L;
	private long current = -1L;
	private transient long candidate = -1L;
	private transient Job job = null;
	private transient Logger log = null;
	private boolean continueRunning = true;
	private boolean suspend = false;
	private boolean suspended = false;
	private transient Object lock = null;
	
	/** Creates a new instance of PrimeFinder */
	public PrimeFinder (long startingWith, long endingWith) {
		this.startingWith = startingWith;
		
		if (this.startingWith % 2 == 0) {
			this.startingWith++;
		}
		
		this.endingWith = endingWith;
	}
	
	public void cancel () {
		continueRunning = false;
		
		synchronized (lock) {
			lock.notify ();
		}
	}
	
	public void checkpoint () {
		suspend ();
		
		while (!suspended) {
			System.out.println ("Job not yet suspended... yielding...");
			Thread.yield ();
		}
		
		current = candidate;
		
		resume ();
	}
	
	public Serializable compute (Job job) throws ComputeException {
		int count = 0;
		candidate = 0L;
		lock = new Object ();
		
		this.job = job;
		this.log = job.getLogger ();

		if (current > 0L) {
			startingWith = current;
		}
		
		if (startingWith < 3L) {
			candidate = 3L;
			count = 1;
		}
		else {
			candidate = startingWith;
		}

		while (continueRunning && (candidate <= endingWith)) {
//			log.logMessage ("Testing " + candidate);

			boolean isPrime = true;
			long squareRoot = (long)Math.sqrt (candidate);

			for (long tester = 3; tester <= squareRoot; tester+=2L) {
//				System.out.println ("\t" + tester);
				if (candidate % tester == 0) {
					isPrime = false;
					break;
				}
			}

			if (isPrime) {
				log.logMessage (candidate + " is prime");
				count++;
			}

			if (candidate - startingWith > 50000) {
				throw new RuntimeException ("This shit has hit the fan!");
			}
			
			candidate += 2L;

			//Extra if is to prevent too much locking & unlocking
			if (suspend) {
				synchronized (lock) {
					try {
						while (suspend && continueRunning) {
							suspended = true;
							lock.wait ();
						}
					}
					catch (InterruptedException e) {
						log.logError ("Interrupted during hold state");
					}
					finally {
						suspended = false;
					}
				}
			}
		}
		
		return new Integer (count);
	}
	
	public void resume () {
		synchronized (lock) {
			suspend = false;
			lock.notify ();
		}
	}
	
	public void suspend () {
		suspend = true;
	}
}