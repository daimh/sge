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

import net.jini.core.entry.Entry;
import net.jini.core.event.*;
import net.jini.core.lookup.*;
import net.jini.core.lease.*;
import net.jini.core.discovery.LookupLocator;
import net.jini.discovery.*;
import net.jini.lookup.*;
import net.jini.lease.*;
import java.io.*;
import java.rmi.*;
import java.rmi.server.*;
import com.sun.jini.lease.landlord.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import com.sun.grid.jam.queue.entry.ComputeInfoEntry;

/**
 * Abstract superclass for application agents.  All Application agents
 * should subclass AppAgent.  This class becomes an attribute of the App.
 * service, and as such, implements Entry.
 *
 * Subclasses must provide a concrete setServiceAttrs method.
 *
 * @version 1.24, 12/04/00
 *
 * @author Eric Sharakan
 */
public abstract class AppAgent
  implements Entry
{
  protected transient String[] groups;
  protected transient LookupLocator[] locators;
  protected transient AppParamsInterface params;
  protected transient ServiceTemplate template;
  protected transient Lease lease;
  protected transient ServiceRegistrar[] newregs;
  protected transient LeaseRenewalManager leaseManager;
  protected Entry[] serviceAttrs;

  // for local events
  protected AgentEventListenerInterface localCallback;

  protected ServiceDiscoveryManager clm;
  protected LookupCache serviceCache;

    public AppAgent()
    {
    }

    public void run()
    {
      // For now (perhaps for good for Native App), agent simply does
      // attribute based discovery of the appropriate compute services.
      // Register for notification of appropriate LUS.
      try {
	Class[] serviceClass =
	{ Class.forName("com.sun.grid.jam.queue.QueueInterface") };
	template = new ServiceTemplate(null, serviceClass, serviceAttrs);

      serviceCache = clm.createLookupCache(template, null, new ServiceDiscoveryListener() {

	public void serviceAdded(ServiceDiscoveryEvent sde) {
	  // System.out.println("Queue Service added: " + sde.getPostEventServiceItem());
	  // Callback with existing services on newly discovered registrar
	  ServiceItem[] services = serviceCache.lookup(null, Integer.MAX_VALUE);

	  ServiceMatches sm = new ServiceMatches(services, services.length);
	  localCallback.agentNotify(new AgentEvent(AppAgent.this, serviceCache, sm));
	}

	public void serviceChanged(ServiceDiscoveryEvent sde) {
	  // System.out.println("Queue Service changed: " + sde.getPostEventServiceItem());
	  // Callback with existing services on newly discovered registrar
	  ServiceItem[] services = serviceCache.lookup(null, Integer.MAX_VALUE);

	  ServiceMatches sm = new ServiceMatches(services, services.length);
	  localCallback.agentNotify(new AgentEvent(AppAgent.this, serviceCache, sm));
	}

	public void serviceRemoved(ServiceDiscoveryEvent sde) {
	  // System.out.println("Queue Service removed: " + sde.getPostEventServiceItem());
	  // Callback with existing services on newly discovered registrar
	  ServiceItem[] services = serviceCache.lookup(null, Integer.MAX_VALUE);

	  ServiceMatches sm = new ServiceMatches(services, services.length);
	  localCallback.agentNotify(new AgentEvent(AppAgent.this, serviceCache, sm));
	}
      });
    } catch(Exception re) {
      re.printStackTrace();
    }
	return;
    }

    public void setParameters(String[] groups, LookupLocator[] locators,
			      AppParamsInterface params)
    {
	this.groups = groups;
	this.params = params;
	this.locators = locators;

	try {
	  DiscoveryManagement ldm = new LookupDiscoveryManager(groups, locators, null);
	  LeaseRenewalManager lrm = new LeaseRenewalManager();
	  clm = new ServiceDiscoveryManager(ldm, lrm);
	} catch (IOException ioe) {
	  ioe.printStackTrace();
	}
    }

  /**
   * Local event registration
   */
  public void addActionListener(AgentEventListenerInterface callback)
  {
    localCallback = callback;
  }

  /**
   * Subclasses must provide a concrete implementation of this method, which
   * sets the serviceAttrs array for template matching of appropriate queue
   * services for the specific application.
   */
  public abstract void setServiceAttrs();

  public LookupCache getServiceCache() {
    return serviceCache;
  }

    public void updateQueueList()
    {
	// Terminate current ServiceDiscoveryManager
	removeListener();
	// Empty browser list
	localCallback.agentNotify(new AgentEvent(this));
	// Restart agent
	run();
    }

    public void removeListener()
    {
	if (clm != null && serviceCache != null)
	    serviceCache.terminate();
    }
}
