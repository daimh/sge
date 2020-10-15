/*
 * PropertyTest.java
 *
 * Created on October 31, 2003, 2:39 PM
 */

package dant.test;

/**
 *
 * @author  dant
 */
public class PropertyTest {
	
	/** Creates a new instance of PropertyTest */
	public PropertyTest () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) {
		System.out.println (System.getProperty ("java.rmi.server.codebase"));
	}
	
}
