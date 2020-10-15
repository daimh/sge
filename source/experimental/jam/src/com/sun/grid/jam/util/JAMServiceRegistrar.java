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
package com.sun.grid.jam.util;

import java.io.*;
import java.util.Date;
import java.util.ArrayList;
import java.net.URL;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import net.jini.space.JavaSpace;
import net.jini.core.entry.Entry;
import net.jini.core.lookup.ServiceID;
import net.jini.core.transaction.TransactionException;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lease.Lease;
import net.jini.core.discovery.LookupLocator;
import net.jini.lookup.ServiceIDListener;
import net.jini.lookup.JoinManager;
import net.jini.lookup.entry.Name;
import net.jini.lease.LeaseRenewalManager;
import net.jini.discovery.DiscoveryManagement;
import net.jini.discovery.LookupDiscovery;
import net.jini.discovery.LookupDiscoveryManager;

/**
 * This class is used to register all services in JAM.  It handles
 * registration, by utilizing JoinManager, and administration, by
 * implementing JoinAdmin & DestroyAdmin (via subinterface JAMAdmin), and
 * passing along the Admin. requests to the JoinManager.
 *
 * @version 1.5, 12/04/00
 *
 * @author Eric Sharakan
 */
public class JAMServiceRegistrar
  extends UnicastRemoteObject
  implements JAMAdmin, ServiceIDListener, Runnable
{
  protected LookupDiscoveryManager discovery;
  protected JoinManager myJM;
  protected ArrayList joiners;
  protected JAMProxy proxy;
  protected Entry[] attrs;
  protected LookupLocator[] locators;

  public JAMServiceRegistrar(JAMProxy proxy, Entry[] attrs,
			     LookupLocator[] locators, ArrayList joiners)
       throws RemoteException
  {
    this.proxy = proxy;
    this.attrs = attrs;
    this.locators = locators;
    this.joiners = joiners;
  }

  public void run()
  {
    try {
      // Due to a bug in Jini 1.1 Alpha, it's best to start off with NO_GROUPS
      // LookupDiscovery construction, and add the groups specification
      // after the JoinManager has been constructed.
      discovery = new LookupDiscoveryManager(LookupDiscovery.NO_GROUPS,
					     locators, null);

      proxy.setServerRef(this);
      myJM = new JoinManager(proxy, attrs, this, discovery,
                                       null);
      joiners.add(myJM);

      // Use user.name property as group for joining purposes
      String[] joinGroups = new String[] { System.getProperty("user.name") };
      discovery.addGroups(joinGroups);
    } catch(RemoteException re) {
      re.printStackTrace();
    } catch(IOException ioe) {
      ioe.printStackTrace();
    } 
  }

  public void serviceIDNotify(ServiceID id)
  {
    System.out.println("SeviceID is: " + id);
  }
  
  // JoinAdmin, DestroyAdmin implementation methods
  public Entry[] getLookupAttributes() throws RemoteException {
    return myJM.getAttributes();
  }

  public void addLookupAttributes(Entry[] attrSets) throws RemoteException {
    myJM.addAttributes(attrSets, true);
  }

  public void modifyLookupAttributes(Entry[] attrSetTemplates,
				     Entry[] attrSets) throws RemoteException
  {
    myJM.modifyAttributes(attrSetTemplates, attrSets, true);
  }

  public String[] getLookupGroups() throws RemoteException {
    return ((LookupDiscovery)myJM.getDiscoveryManager()).getGroups();
  }

  public void addLookupGroups(String[] groups) throws RemoteException {
    try {
      ((LookupDiscovery)myJM.getDiscoveryManager()).addGroups(groups);
    } catch(IOException ioex) {
      throw new RuntimeException(ioex.toString());
    }
  }

  public void removeLookupGroups(String[] groups) throws RemoteException {
    ((LookupDiscovery)myJM.getDiscoveryManager()).removeGroups(groups);
  }

  public void setLookupGroups(String[] groups) throws RemoteException {
    try {
      ((LookupDiscovery)myJM.getDiscoveryManager()).setGroups(groups);
    } catch(IOException ioex) {
      throw new RuntimeException(ioex.toString());
    }
  }

  public LookupLocator[] getLookupLocators() throws RemoteException {
    return locators;
  }

  public void addLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    throw new RemoteException("addLookupLocators not supported");
  }

  public void removeLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    throw new RemoteException("removeLookupLocators not supported");
  }

  public void setLookupLocators(LookupLocator[] locators)
       throws RemoteException
  {
    throw new RemoteException("setLookupLocators not supported");
  }

  protected class DestroyRun
    implements Runnable
  {
    public DestroyRun()
    {
    }

    public void run()
    {
      // TODO: remove this JoinManager reference from joiners ArrayList. XXX
      myJM.terminate();
    }
  }

  public void destroy()
  {
    DestroyRun dt = new DestroyRun();
    new Thread(dt).start();
  }
}

