/*
 * SerializableObject.java
 *
 * Created on May 7, 2002, 5:00 PM
 */

package dant.test.rmi;

import java.io.Serializable;

/**
 *
 * @author  unknown
 * @version
 */
public class SerializableObject implements Serializable {
	private int attribute1 = 1;
	private String attribute2 = "Test";
	
	/** Creates new SerializableObject */
	public SerializableObject() {
	}
}
