/*
 * AnnotationTestImplImpl.java
 *
 * Created on June 16, 2003, 1:10 PM
 */

package dant.test;

import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;
import java.net.MalformedURLException;

/** Unicast remote object implementing remote interface.
 *
 * @author dant
 * @version 1.0
 */
public class AnnotationTestImpl extends java.rmi.server.UnicastRemoteObject implements AnnotationTest {
	
	/** Constructs AnnotationTestImplImpl object and exports it on default port.
	 */
	public AnnotationTestImpl () throws RemoteException {
		super ();
	}
	
	/** Constructs AnnotationTestImplImpl object and exports it on specified port.
	 * @param port The port for exporting
	 */
	public AnnotationTestImpl (int port) throws RemoteException {
		super (port);
	}
	
	/** Register AnnotationTestImplImpl object with the RMI registry.
	 * @param name - name identifying the service in the RMI registry
	 * @param create - create local registry if necessary
	 * @throw RemoteException if cannot be exported or bound to RMI registry
	 * @throw MalformedURLException if name cannot be used to construct a valid URL
	 * @throw IllegalArgumentException if null passed as name
	 */
	public static void registerToRegistry (String name, Remote obj, boolean create) throws RemoteException, MalformedURLException{
		
		if (name == null) throw new IllegalArgumentException ("registration name can not be null");
		
		try {
			Naming.rebind (name, obj);
		} catch (RemoteException ex){
			if (create) {
				Registry r = LocateRegistry.createRegistry (Registry.REGISTRY_PORT);
				r.rebind (name, obj);
			} else throw ex;
		}
	}
	
	/** Main method.
	 */
	public static void main (String[] args) {
		System.setSecurityManager (new RMISecurityManager ());
		
		try {
			AnnotationTestImpl obj = new AnnotationTestImpl ();
			registerToRegistry ("AnnotationTestImplImpl", obj, true);
		} catch (RemoteException ex) {
			ex.printStackTrace ();
		} catch (MalformedURLException ex) {
			ex.printStackTrace ();
		}
	}
	
	public void test (Object obj) throws java.rmi.RemoteException {
		System.out.println("Received: " + obj.getClass ().getName () );
		
		System.out.println (RMIClassLoader.getClassAnnotation (obj.getClass ()));
	}
	
}
