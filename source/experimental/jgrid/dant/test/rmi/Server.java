/*
 * Server.java
 *
 * Created on May 7, 2002, 4:45 PM
 */

package dant.test.rmi;

import java.io.*;
import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;
import java.net.MalformedURLException;

/** Unicast remote object implementing java.rmi.Remote interface.
 *
 * @author unknown
 * @version 1.0
 */
public class Server extends UnicastRemoteObject implements Interface {

	/** Constructs Server object and exports it on default port.
     */
    public Server() throws RemoteException {
        super();
    }

		/** Constructs Server object and exports it on specified port.
     * @param port The port for exporting
     */
    public Server(int port) throws RemoteException {
        super(port);
    }

		/** Register Server object with the RMI registry.
     * @param name - name identifying the service in the RMI registry
     * @param create - create local registry if necessary
     * @throw RemoteException if cannot be exported or bound to RMI registry
     * @throw MalformedURLException if name cannot be used to construct a valid URL
     * @throw IllegalArgumentException if null passed as name
     */
    public static void registerToRegistry(String name, Remote obj, boolean create) throws RemoteException, MalformedURLException{

        if (name == null) throw new IllegalArgumentException("registration name can not be null");

        try {
            Naming.rebind(name, obj);
        } catch (RemoteException ex){
            if (create) {
                Registry r = LocateRegistry.createRegistry(Registry.REGISTRY_PORT);
                r.rebind(name, obj);
            } else throw ex;
        }
    }

    /** Main method.
     */
    public static void main(String[] args) {
        System.setSecurityManager(new RMISecurityManager());

        try {
					Server obj = new Server();
					registerToRegistry("Server", obj, false);
        } catch (RemoteException ex) {
            ex.printStackTrace();
        } catch (MalformedURLException ex) {
            ex.printStackTrace();
        }
				
				System.out.println ("Ready.");
    }
		
		public void voidPrimitiveParam(int param1) throws RemoteException {
		}
		
		public void voidMultiplePrimitiveParam(int param1, float param2) throws RemoteException {
		}
		
		public void voidExternalizableParam(Externalizable param1) throws RemoteException {
		}
		
		public String stringNoParam() throws RemoteException {
			return null;
		}
		
		public void voidMultipleStringParam(String param1, String param2) throws RemoteException {
		}
		
		public void voidNoParams() throws RemoteException {
		}
		
		public void voidStringParam(String param1) throws RemoteException {
		}
		
		public void voidMultipleExternalizableParam(Externalizable param1, Externalizable param2) throws RemoteException {
		}
		
		public Serializable serializableNoParam() throws RemoteException {
			return null;
		}
		
		public void voidSerializableParam(Serializable param1) throws RemoteException {
		}
		
		public void voidMultipleSerializableParam(Serializable param1, Serializable param2) throws RemoteException {
		}
		
		public int primitiveNoParam() throws RemoteException {
			return -1;
		}
		
		public Externalizable externalizableNoParam() throws RemoteException {
			return null;
		}		
}
