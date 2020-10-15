/*
 * AnnotationTester.java
 *
 *  Created on June 16, 2003, 1:14 PM
 */

package dant.test;

import java.rmi.*;

import dant.test.*;

/**
 *
 * @author dant
 * @version 1.0
 */
public class AnnotationTester extends Object {
	
	/** Creates new AnnotationTester */
	public AnnotationTester () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String args[]) throws Exception {
		System.setSecurityManager (new RMISecurityManager ());
		
		AnnotationTest test = (AnnotationTest)Naming.lookup ("AnnotationTestImplImpl");
		
		test.test ("test");
	}
}
