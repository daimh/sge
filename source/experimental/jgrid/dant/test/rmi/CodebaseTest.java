/*
 * CodebaseTest.java
 *
 * Created on November 13, 2003, 11:26 AM
 */

package dant.test.rmi;

import java.rmi.*;

/**
 *
 * @author  dant
 */
public interface CodebaseTest extends Remote {
	public abstract void doSomething () throws RemoteException;
}
