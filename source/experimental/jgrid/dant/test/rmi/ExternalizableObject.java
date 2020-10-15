/*
 * ExternalizableObject.java
 *
 * Created on May 7, 2002, 5:00 PM
 */

package dant.test.rmi;

import java.io.Externalizable;

/**
 *
 * @author  unknown
 * @version 
 */
public class ExternalizableObject implements Externalizable {

	/** Creates new ExternalizableObject */
    public ExternalizableObject() {
    }

		public void readExternal(java.io.ObjectInput objectInput) throws java.io.IOException, java.lang.ClassNotFoundException {
			objectInput.readInt();
			objectInput.readUTF();
		}
		
		public void writeExternal(java.io.ObjectOutput objectOutput) throws java.io.IOException {
			objectOutput.writeInt(2);
			objectOutput.writeUTF("Test");
		}
}
