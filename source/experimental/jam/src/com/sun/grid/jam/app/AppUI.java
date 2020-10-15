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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.io.*;
import java.net.MalformedURLException;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.discovery.LookupLocator;
import net.jini.core.entry.Entry;
import com.sun.grid.jam.queue.QueueInterface;
import com.sun.grid.jam.tools.JAMComputeMonitor;

/**
 * Abstract superclass for application UIs.  All Application services
 * should subclass AppUI.  This class becomes an attribute of the App.
 * service, and as such, implements Entry.
 *
 * @version 1.13, 09/22/00
 *
 * @see AppParamsPanel
 * @see AppAgent
 *
 * @author Eric Sharakan
 */
public abstract class AppUI
  implements Entry
{
  protected transient AppParamsInterface params;
  protected transient AppAgent aa;
  protected transient AppParamsPanel thisPanel;

  public AppUI()
  {
  }

  /**
   * Instantiate and return an AppParams object, which hold all the
   * application-independent parameters needed to submit the application
   * to a queue service.
   */
  public AppParamsInterface getAppParams()
  {
    // Instantiate a new copy each time
    params = new AppParams();
    return params;
  }

  /**
   * Save a reference to an AppAgent object.  Concrete subclasses pass this
   * reference (along with a Ref. to the AppParams boject) to the
   * constructor of the App-specific UI panel.
   */
  public void setAgentRef(AppAgent aa)
  {
    this.aa = aa;
  }

  /**
   * Return App-specific UI Component (JPanel).
   */
  public abstract Component getUI();
}
