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
package com.sun.grid.jam.tools;

import javax.swing.JFrame;
import java.io.IOException;
import java.rmi.RemoteException;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.entry.Entry;
import net.jini.core.lease.Lease;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceItem;
import net.jini.core.discovery.LookupLocator;
import net.jini.lookup.ServiceDiscoveryManager;
import net.jini.lookup.LookupCache;
import net.jini.lookup.ServiceDiscoveryListener;
import net.jini.lookup.ServiceDiscoveryEvent;
import net.jini.discovery.LookupDiscoveryManager;
import com.sun.grid.jam.ui.*;
import com.sun.grid.jam.ui.entry.*;

/**
 * Listens for a Job service.
 *
 * @version 1.7, 12/04/00
 *
 * @author Nello Nellari
 */
public class JAMServiceUILauncher
  extends Thread
  implements ServiceDiscoveryListener
{

  private ServiceDiscoveryManager clm;
  private LookupCache lookupCache;

  public JAMServiceUILauncher(ServiceDiscoveryManager clm,
                              Entry[] entry, String clName)
    throws ClassNotFoundException, RemoteException
  {
    super();
    Class [] cls = { Class.forName(clName) };
    lookupCache = clm.createLookupCache(new ServiceTemplate(null,
                                                            cls,
                                                            entry),
                                        null, this);
    //this.key = ((JobUserKey)entry[0]).key;
    //System.out.println("JAM SERVICE UIKey: " + key);
  }
  
  public JAMServiceUILauncher(String[] groups, LookupLocator[] locators,
                              Entry[] entry, String clName)
    throws ClassNotFoundException, IOException
  {
    super();
    clm = new ServiceDiscoveryManager(new
      LookupDiscoveryManager(groups, locators, null), null);
    Class [] cls = { Class.forName(clName) };
    lookupCache = clm.createLookupCache(new
      ServiceTemplate(null, cls, entry), null, this);
    //this.key = ((JobUserKey)entry[0]).key;
    //System.out.println("JAM SERVICE UIKey: " + key);
  }
  
  public void serviceAdded(ServiceDiscoveryEvent e)
  {
    startNewJFrame(e.getPostEventServiceItem());
  }
  
  public void serviceChanged(ServiceDiscoveryEvent e)
  {
  }

  public void serviceRemoved(ServiceDiscoveryEvent e)
  {
  }

  public void run()
  {
  }
  
  private void removeLookupCache()
  {
    lookupCache.removeListener(this);
    lookupCache.terminate();
    if(clm != null)
      clm.terminate();
    //System.out.println("JAM LAUNCHER terminated " + key);
  }
  
  private synchronized void startNewJFrame(ServiceItem si)
  {    
    // Looking for ServiceUIFactoryEntry object
    Entry[] attr = si.attributeSets;
    for(int i = 0 ; i < attr.length; i++) {
      if(attr[i] instanceof ServiceUIFactoryEntry) {
        try {
          ServiceUIFactory factory = ((ServiceUIFactoryEntry)attr[i]).factory;
          JFrame mon = (JFrame)factory.getServiceUI(si.service);
          mon.pack();
          mon.setVisible(true);
          //System.out.println("JAM UI LAUNCHER service ID: "
          //                   + si.serviceID);
          removeLookupCache();
        } catch(UINotAvailableException uinae) {
          uinae.printStackTrace();
        }
      }
    }
  }
}
