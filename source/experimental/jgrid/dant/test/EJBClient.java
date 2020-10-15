/*
 * EJBClient.java
 *
 * Created on October 31, 2003, 3:32 PM
 */

package dant.test;

import javax.naming.*;
import javax.rmi.PortableRemoteObject;

/**
 *
 * @author  dant
 */
public class EJBClient {
	private static GridEJB ge = null;
	/** Creates a new instance of EJBClient */
	public EJBClient () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) {
		try {
				Context context = new InitialContext ();
				Object homeObject = context.lookup ("ejb/GridEJB");
				GridEJBHome home = (GridEJBHome)PortableRemoteObject.narrow (homeObject, GridEJBHome.class);
				ge = home.create ();
                                ComputeTest job = new dant.test.ComputeTest ();
                                System.out.println (ge.compute (job));
				home.remove (ge.getHandle ());
		}
		catch (Exception e) {
							e.printStackTrace ();
		}
	}
	
}
