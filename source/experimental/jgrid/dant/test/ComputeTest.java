/*
 * ComputeTest.java
 *
 * Created on May 22, 2002, 3:42 PM
 */

package dant.test;

import java.io.Serializable;

import com.sun.grid.jgrid.*;
/**
 *
 * @author  unknown
 * @version
 */
public class ComputeTest implements Computable {
	private transient Job job = null;
	
	/** Creates new ComputeTest */
	public ComputeTest() {
	}
	
	public Serializable compute (Job job) throws ComputeException {
		this.job = job;
		Logger log = job.getLogger ();
		
		log.logMessage ("This is a test");
		
		return "TEST";
	}
	
	public void cancel () throws NotInterruptableException {
		job.interrupt ();
	}
	
	public void checkpoint () throws NotInterruptableException {
	}
	
	public void resume () {
	}
	
	public void suspend () throws NotInterruptableException {
	}
}
