/*
 * Client.java
 *
 *  Created on May 7, 2002, 5:04 PM
 */

package dant.test.rmi;

import java.io.*;
import java.rmi.*;
import java.rmi.registry.*;

/**
 *
 * @author unknown
 * @version 1.0
 */
public class Client extends Object {
	
	/** Creates new Client */
	public Client() {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main(String args[]) throws Exception {
		System.setSecurityManager(new RMISecurityManager());
		
//		ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream("registry.ser"));
		
		Registry r = LocateRegistry.getRegistry(args[0], Integer.parseInt(args[1]));
/*		
		out.writeObject(r);
		out.close();
		
		out = new ObjectOutputStream(new FileOutputStream("proxy.ser"));
*/		
		Interface i = (Interface)r.lookup("Server");
		
//		out.writeObject(i);
//		out.close();
		
//		i.voidNoParams();
//		i.voidPrimitiveParam(1);
//		i.voidMultiplePrimitiveParam(2, 3.0f);
//		i.voidStringParam("Test1");
//		i.voidMultipleStringParam("Test2", "Test3");
		i.voidSerializableParam(new SerializableObject());
//		i.voidMultipleSerializableParam(new SerializableObject(), new SerializableObject());
//		i.voidExternalizableParam(new ExternalizableObject());
//		i.voidMultipleExternalizableParam(new ExternalizableObject(), new ExternalizableObject());
//		i.primitiveNoParam();
//		i.stringNoParam();
//		i.serializableNoParam();
//		i.externalizableNoParam();
	}
}