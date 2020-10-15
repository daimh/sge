/*
 * JSClient.java
 *
 * Created on October 21, 2003, 1:11 PM
 */

package dant.test;

import java.rmi.*;
import net.jini.core.discovery.*;
import net.jini.core.lease.*;
import net.jini.core.lookup.*;
import net.jini.discovery.*;
import net.jini.space.*;
import net.jini.lease.*;
import net.jini.lookup.*;
import net.jini.lookup.entry.*;

/**
 *
 * @author  dant
 */
public class JSClient {
	
	/** Creates a new instance of JSClient */
	public JSClient () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		System.setSecurityManager (new RMISecurityManager ());
		LeaseRenewalManager lrm = new LeaseRenewalManager ();
		LookupDiscoveryManager ldm = new LookupDiscoveryManager (null, null, null);
		ServiceDiscoveryManager sdm = new ServiceDiscoveryManager (ldm, lrm);
		
		ServiceItem[] items = sdm.lookup (new ServiceTemplate (null, new Class[] {ServiceRegistrar.class}, null), 1, 5, null, 15000L);
		
		ldm.addLocators (new LookupLocator[] {((ServiceRegistrar)items[0].service).getLocator ()});
		
		ServiceTemplate tem = new ServiceTemplate (null, new Class[] {JavaSpace.class}, null);
		System.out.println("Getting Space");
		
		items = sdm.lookup (tem, 1, 1, (ServiceItemFilter)null, 15000L);

		System.out.println("Results of lookup");
		
		for (int count = 0; count < items.length; count++) {
			System.out.println(items[0].service.getClass ().getName ());
		}
		
		JavaSpace js = (JavaSpace)sdm.lookup (tem, 1, 1, (ServiceItemFilter)null, 15000L)[0];
		
		if (args[0].equals ("put")) {
			System.out.println("Writing location");
			Lease l = js.write (new Location ("6", "1", "ERGB01"), null, Lease.FOREVER);
			lrm.renewFor (l, Lease.FOREVER, null);
		}
		else {
			System.out.println("Reading location");
			Location l = (Location)js.read (new Location (null, null, null), null, Lease.FOREVER);
			System.out.println(l);
		}
	}
}
