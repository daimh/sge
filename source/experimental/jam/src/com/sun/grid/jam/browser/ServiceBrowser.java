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
package com.sun.grid.jam.browser;

//import net.jini.core.event.* ;
import java.io.Serializable ;
import java.awt.Container;
import java.awt.BorderLayout;
import javax.swing.*;
import java.util.Vector;
import java.rmi.*;
import net.jini.core.entry.Entry;
import net.jini.core.discovery.LookupLocator;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceMatches;
import net.jini.core.lookup.ServiceItem;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.event.*;
import net.jini.discovery.*;
import net.jini.lease.*;
import net.jini.lookup.*;
import net.jini.lookup.entry.*;

/**
 * Finds, caches and displays application and queue service objects.
 * Internally utilizes the ServiceDiscoveryManager utility class for
 * discovering application services.  Handles queue services by supporting
 * external ServiceDiscoveryManager classes (instantiated and managed
 * by the AppAgents).
 *
 * @version 1.16, 09/28/00
 *
 * @author Eric Sharakan
 * @author Nello Nellari
 */
public class ServiceBrowser
{
  /**
   * Main UI element
   */
  private JApplet gui;
  private JList list;
  /**
   * List of Service names (for display).  DefaultListModel is utilized
   * because it implements the Vector API as well as being the data
   *  model for the JList.
   *
   * @see <a href=http://java.sun.com/products/jdk/1.2/docs/api/javax/swing/DefaultListModel.html>DefaultListModel</a>
   */
  private DefaultListModel serviceList;
  private Vector serviceID;
  private ServiceTemplate template;
  private ServiceDiscoveryManager clm;
  private LookupCache serviceCache;
  private DiscoveryManagement ldm;
  private LeaseRenewalManager lrm;
  private String[] groups;
  private LookupLocator[] locators;
  
  /**
   * For interacting with multiple lookup servers using discovery
   */
  public ServiceBrowser(String[] groups, ServiceTemplate templ,
			LookupLocator[] locators)
  {
    template = templ;
    this.groups = groups;
    this.locators = locators;

    serviceList = new DefaultListModel();
  }

  /**
   * If list of services already known - this provides a static list
   * (unless specifically updated by client).
   */
  public ServiceBrowser(ServiceMatches sm)
  {
    serviceList = new DefaultListModel();
    updateServiceList(sm);
  }


  /**
   * This method is used by this class's ServiceDiscoveryListener
   * callbacks.  It generates a ServiceMatches array from the list
   * of ServiceItems in the internal LookupCache (in particular,
   * application services)
   *
   * @see ServiceBrowser#updateServiceList(ServiceMatches)
   */
  private void updateServiceList()
  {
    ServiceItem[] services;
    ServiceMatches sm;

    // Massage this into the current service browser's vectors.
    services = serviceCache.lookup(null, Integer.MAX_VALUE);

    sm = new ServiceMatches(services, services.length);
    updateServiceList(sm);
  }

  /**
   * This method is used when the ServiceMatches array is already
   * available (e.g. when queue services are discovered by AppAgent).
   */
  public void updateServiceList(ServiceMatches sm)
  {
    serviceID = new Vector();
    serviceList.clear();

    NameBean bean = new NameBean();
    String nm = new String();
    if (sm != null) {
      // System.out.println("ServiceMatches contains " + sm.items.length);
      for(int h = 0; h < sm.items.length; h ++) {
	// No exception is thrown if you try to access a null
	// ServiceItem.  Must explicitly check for this case
	// (i.e. the first clause below) to properly detect a
	// service going away (at least for app agent case, which
	// builds it own ServiceMatches object).
	if(sm.items[h] != null && sm.items[h].service != null) {
	  Entry attr [] = sm.items[h].attributeSets;
	  if(attr[0] instanceof Name) {
	    bean.makeLink(attr[0]);
	    nm = bean.getName();
	  } else {
	    nm = sm.items[h].service.getClass().getName();
	  }
	  if(attr.length > 1 && attr[1] instanceof Name) {
	    bean.makeLink(attr[1]);
	    nm = nm + "  " + bean.getName();
	  }
	  serviceList.addElement(nm);
	  serviceID.addElement(sm.items[h].serviceID);

	  // HACK ALERT: This sleep is necessary to prevent discovered services
	  // from sometimes not showing up in the browser pane.  This seems to
	  // happen most often with application services registered before this
	  // pane is instantiated.  Without this sleep, it was sometimes
	  // necessary to stop and restart the applications to make them
	  // visible in the browser pane.
	  try {
	    Thread.sleep(100);
	  } catch(InterruptedException ite) {
	  // Do nothing
	  }
	} else {
	  // System.out.println("Browser got a null ServiceItem in ServiceMatches");
	}
      }
    } else {
      // System.out.println("Browser got a null ServiceMatches object!");
    }
  }

  /**
   * Instantiate and return JApplet panel.
   */
  public JApplet getUI()
  {
    if(gui == null) {
      gui = new JApplet();
      list = new JList(serviceList);
      list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
      // list.setListData(serviceList);
      Container c = gui.getContentPane();
      c.setLayout(new BorderLayout());
      c.add(list, BorderLayout.CENTER);
    }
    return gui;
  }

  /**
   * Get ServiceItem from our LookupCache which corresponds to
   * the index of the selected item in the list (keyed by serviceID
   * Vector).  Used to find selected application's ServiceItem.
   */
  public ServiceItem getServiceItem(int index)
  {
      return getServiceItem(index, this.serviceCache);
  }

  /**
   * Get ServiceItem from provided LookupCache which corresponds to
   * the index of the selected item in the list (keyed by serviceID
   * Vector).  Used by AppAgent, which provides its own LookupCache
   * of queue services.
   */
  public ServiceItem getServiceItem(int index, LookupCache serviceCache)
  {
    ServiceItem service;
    final int myIndex = index;

    service = serviceCache.lookup(new ServiceItemFilter() {
      public boolean check(ServiceItem svc) {
	return svc.serviceID.equals(getServiceID(myIndex));
      }
    });

    return service;
  }

  /**
   * Access serviceID Vector element by index.
   */
  private ServiceID getServiceID(int index)
  {
    if(serviceID != null && index < serviceID.size())
      return (ServiceID)serviceID.elementAt(index);
    return null;
  }
  
  /**
   * Accessor for UI's JList pane
   */
  public JList getJList()
  {
    return list;
  }

  /**
   * Terminate internal ServiceDiscoveryManager (usually on
   * window closing event).
   */
  public void removeListener()
  {
    if (clm != null)
      clm.terminate();
  }

  /**
   * Start internal ServiceDiscoveryManager (looking for application
   * services).
   */
  public void startDiscovery()
  {
    try {
      ldm = new LookupDiscoveryManager(groups, locators, null);
      lrm = new LeaseRenewalManager();
      clm = new ServiceDiscoveryManager(ldm, lrm);

      serviceCache = clm.createLookupCache(template, null, new ServiceDiscoveryListener() {

	public void serviceAdded(ServiceDiscoveryEvent sde) {
	  // System.out.println("Service added: " + sde.getPostEventServiceItem());
	  updateServiceList();
	}

	public void serviceChanged(ServiceDiscoveryEvent sde) {
	  updateServiceList();
	  // System.out.println("Service changed: " + sde.getPostEventServiceItem());
	}

	public void serviceRemoved(ServiceDiscoveryEvent sde) {
	  // System.out.println("Service removed: " + sde.getPostEventServiceItem());
	  updateServiceList();
	}
      });
    } catch(Exception re) {
      re.printStackTrace();
    }
  }
}

