/*
 * CodebaseTestImpl.java
 *
 * Created on November 13, 2003, 11:27 AM
 */

package dant.test.rmi;

import java.rmi.*;
import java.rmi.server.*;

/**
 *
 * @author  dant
 */
public class CodebaseTestImpl extends UnicastRemoteObject implements CodebaseTest {
	
	/** Creates a new instance of CodebaseTestImpl */
	public CodebaseTestImpl () throws RemoteException {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		CodebaseTestImpl cbi = new CodebaseTestImpl ();
		Naming.bind ("test", cbi);
	}
	
	public void doSomething () throws RemoteException {
		//Dead stub
	}
	
	public void doSomething (String codebase) throws RemoteException {
		System.out.println("Codebase is " + codebase);
	}
	
}
