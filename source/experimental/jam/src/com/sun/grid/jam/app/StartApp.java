/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *   and/or Swiss Center for Scientific Computing
 * 
 *   Copyright: 2002 by Sun Microsystems, Inc.
 *   Copyright: 2002 by Swiss Center for Scientific Computing
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jam.app;

import java.io.*;
import java.net.*;
import java.util.*;
import java.rmi.*;
import net.jini.core.entry.*;
import net.jini.core.discovery.*;
import net.jini.core.lookup.*;
import net.jini.entry.*;
import net.jini.lookup.entry.Name;
import net.jini.discovery.*;
import net.jini.lookup.*;
import net.jini.lease.*;
import java.rmi.server.*;

import com.sun.grid.jam.browser.*;
import com.sun.grid.jam.app.*;
import com.sun.grid.jam.util.JAMServiceRegistrar;
import com.sun.grid.jam.util.JAMProxy;

/**
 * Application service backend (wrapper).  Registers application services
 * with Jini LUS, manages their leases, and handles Administrative requests.
 * <br>
 * Reads from stdin, waiting for user to enter a "q" to cause this code to
 * unregister the services and exit.
 *
 * @version 1.21, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public class StartApp
  extends JAMServiceRegistrar
{
  public StartApp(JAMProxy proxy, Entry[] attrs,
		  LookupLocator[] locators, ArrayList joiners)
    throws RemoteException
  {
    super(proxy, attrs, locators, joiners);
  }

  // Override destroy method
  public void destroy()
  {
    System.out.println("Destroying service: service-specific cleanup");
    super.destroy();
  }

  /**
   * Usage: StartApp.main(String ClassName)
   *
   */
  public static void main(String [] args)
  {
    System.setSecurityManager(new SecurityManager());

    LookupLocator[] locators = new LookupLocator[1];

    try {
      if (args.length >= 1) {
	try {
	  locators[0] = new LookupLocator(args[0]);
	} catch (MalformedURLException mue) {
	  mue.printStackTrace();
	  locators = null;
	}
      } else {
	locators = null;
      }

      ArrayList myJoiners = new ArrayList();

      StartApp myNativeApp =
	new StartApp(new NativeApp(),
		     new Entry[]
		     {
		       new Name("NativeApp"),
			 new NativeAppUI(),
			 },
		       locators,
		       myJoiners);

      StartApp myUserdefinedApp =
	new StartApp(new UserDefinedApp(),
		     new Entry[]
		     {
		       new Name("UserDefinedApp"),
			 new UserAppUI(),
			 },
		       locators,
		       myJoiners);

      StartApp mySortApp =
	new StartApp(new SortApp(),
		     new Entry[]
		     {
		       new Name("SortApp"),
			 new SortAppUI(),
			 },
		       locators,
		       myJoiners);

      // I don't think we need to spin off separate threads for these
      // new Thread(myNativeApp).start();
      // new Thread(myUserdefinedApp).start();
      myNativeApp.run();
      myUserdefinedApp.run();
      mySortApp.run();

      System.out.println("App services coming up, type 'q' for exit");

      BufferedReader in = new BufferedReader(new
					     InputStreamReader(System.in));
      String line;

      while(true) {
	line = in.readLine();
	if(line.equalsIgnoreCase("q")) {
	  Iterator walkList = myJoiners.iterator();

	  while (walkList.hasNext()) {
	    ((JoinManager)walkList.next()).terminate();
	  }
	  Thread.sleep(1000);
	  System.exit(0);
	}
      }
    } catch (RemoteException re) {
      re.printStackTrace();
    } catch (IOException ioe) {
      ioe.printStackTrace();
    } catch (InterruptedException ie) {
      // Do nothing
    }
  }
}
