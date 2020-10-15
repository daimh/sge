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
import net.jini.core.discovery.LookupLocator;
import net.jini.admin.*;
import java.awt.Component;
import java.io.*;
import java.rmi.*;

import com.sun.grid.jam.util.JAMProxy;
import com.sun.grid.jam.queue.QueueInterface;

/**
 * This is the interface to Application services registered with Jini
 * by JAM.  It is implemented by the various application proxies, and called
 * by JAM's client UI code (JAMApplicationHandler).
 *
 * @see AppAgent
 * @see com.sun.grid.jam.tools.JAMApplicationHandler
 * @see com.sun.grid.jam.queue.QueueInterface
 *
 * @version 1.14, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public interface ApplicationInterface
  extends JAMProxy
{
  /**
   * Get the application-specific UI component for rendering by JAM's
   * UI client.
   */
  public Component getUI(Entry[] attrs) throws ClassNotFoundException;

  /**
   * Tell the AppProxy which groups and locators (if any) to specify when
   * looking for matching queue services.  AppProxy passes this info on to
   * AppAgent.
   *
   */
  public void setLookupParams(String[] groups, LookupLocator[] locators);

  /**
   * Tell AppAgent to start looking for matching queue services
   */
  public void runAgent();

  /**
   * Pass client-side agent listener from client to AppAgent (via
   * the AppProxy).
   */
  public void addAgentListener(AgentEventListenerInterface callback);

  /**
   * Remove listener from AppAgent.  Has side-effect of stopping agent from
   * looking for matching queue services.
   */
  public void removeAgentListener();


  /**
   * Interface to submit an application for running on a specific resource.
   * This is an abstract method at top of implementation hierarchy (AppProxy);
   * each subclass implements its own version.
   */
  public void submit(QueueInterface queue)
    throws RemoteException, IOException, ClassNotFoundException;
}
