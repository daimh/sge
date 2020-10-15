/*
 * Interface.java
 *
 * Created on May 7, 2002, 4:46 PM
 */

package dant.test.rmi;

import java.io.*;
import java.rmi.*;

/** Remote interface.
 *
 * @author unknown
 * @version 1.0
 */
public abstract interface Interface extends Remote {
	public abstract void voidNoParams () throws RemoteException;
	public abstract void voidPrimitiveParam (int param1) throws RemoteException;
	public abstract void voidMultiplePrimitiveParam (int param1, float param2) throws RemoteException;
	public abstract void voidStringParam (String param1) throws RemoteException;
	public abstract void voidMultipleStringParam (String param1, String param2) throws RemoteException;
	public abstract void voidSerializableParam (Serializable param1) throws RemoteException;
	public abstract void voidMultipleSerializableParam (Serializable param1, Serializable param2) throws RemoteException;
	public abstract void voidExternalizableParam (Externalizable param1) throws RemoteException;
	public abstract void voidMultipleExternalizableParam (Externalizable param1, Externalizable param2) throws RemoteException;
	public abstract int primitiveNoParam () throws RemoteException;
	public abstract String stringNoParam () throws RemoteException;
	public abstract Serializable serializableNoParam () throws RemoteException;
	public abstract Externalizable externalizableNoParam () throws RemoteException;
}