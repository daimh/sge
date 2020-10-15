/*
 * CodebaseTestClient.java
 *
 * Created on November 13, 2003, 11:30 AM
 */

package dant.test.rmi;

import java.rmi.*;
import java.rmi.server.*;

/**
 *
 * @author  dant
 */
public class CodebaseTestClient {
	
	/** Creates a new instance of CodebaseTestClient */
	public CodebaseTestClient () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		CodebaseTest cbt = (CodebaseTest)Naming.lookup ("test");
		
		cbt.doSomething ();
	}
	
}
