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

import java.awt.event.*;
import java.awt.*;
import javax.swing.*;
import net.jini.core.entry.Entry;
import net.jini.core.discovery.LookupLocator;
import net.jini.admin.*;
import net.jini.discovery.*;
import net.jini.lookup.*;
import java.io.*;
import java.rmi.*;
import java.rmi.server.*;

import com.sun.grid.jam.util.JAMAdmin;
import com.sun.grid.jam.util.JAMAdminProxy;
import com.sun.grid.jam.queue.QueueInterface;

/**
 * Abstract superclass for application proxies.  All Application service
 * proxies should subclass this.  It is assumed that all concrete subclasses
 * will initialize appAgent appropriately (presumably in their constructor).
 *
 * This proxy is not a remote object, though it does contain a remote
 * reference to its service backend (for administration purposes).  The
 * former implies that, as a good Jini citizen, we need to override the
 * equals() and hashCode() methods of java.lang.Object.  The latter,
 * however, makes this overridding trivial: we simply delegate to the remote
 * reference's equals() and hashCode() methods, which have already been
 * properly overridden as part of RMI's remote object semantics.
 *
 * TODO: The above should really be implemented in a base class that all
 * JAM services could extend.  JAMProxy could become that common superclass.
 *
 * @version %I%, %G%
 *
 * @author Eric Sharakan
 */
public abstract class AppProxy
  implements ApplicationInterface
{
  protected JAMAdmin server;
  protected Entry[] serviceAttrs;
  protected AppUI appUI;
  protected AppAgent appAgent;
  protected AppParamsInterface appParams;
  protected LookupLocator[] locators;

  public AppProxy()
  {
  }

  public int hashCode() {
    return server.hashCode();
  }

  public boolean equals(Object obj) {
    return (obj instanceof AppProxy &&
  	    server.equals(((AppProxy)obj).server));
  }

  /**
   * This is run within service's VM (before proxy is registered with LUS)
   */
  public void setServerRef(JAMAdmin server)
  {
    this.server = server;
  }

  public Object getAdmin()
  {
    return new JAMAdminProxy(server);
  }

  public Component getUI(Entry[] attrs)
    throws ClassNotFoundException
  {
    appUI = null;

    // Do we really need to stash this away? XXX
    serviceAttrs = attrs;

    for (int i = 0; i < attrs.length; ++i) {
      if (attrs[i] instanceof AppUI) {
	appUI = (AppUI)attrs[i];
      }
      if (appUI != null)
	break;
    }
    if (appUI == null) {
      throw(new ClassNotFoundException("Missing AppUI attribute"));
    }

    // Get appParams reference from UI.
    // We need this to pass on to the AppAgent.  Subclasses implementing
    // submit() will also need this.
    appParams = appUI.getAppParams();

    // AppUI needs a reference to the AppAgent, so the UI can call
    // the agent's updateQueueList method as needed.  Is there a better way,
    // perhaps using event callbacks? XXX
    appUI.setAgentRef(appAgent);

    return appUI.getUI();
  }

  public void setLookupParams(String[] groups, LookupLocator[] locators)
  {
    // subclasses might need this
    this.locators = locators;
    appAgent.setParameters(groups, locators, appParams);
    // Not strictly a lookup parameter, but this is the best place to put this
    appAgent.setServiceAttrs();
  }

  public void addAgentListener(AgentEventListenerInterface callback)
  {
    // Just pass on to agent
    appAgent.addActionListener(callback);
  }

  public void removeAgentListener()
  {
    // Stopping agent's LookupCache listener is sufficient
    appAgent.removeListener();
  }

  public void runAgent()
  {
    // Just pass on to agent
    appAgent.run();
  }

  public abstract void submit(QueueInterface queue)
    throws RemoteException, IOException, ClassNotFoundException;
}


