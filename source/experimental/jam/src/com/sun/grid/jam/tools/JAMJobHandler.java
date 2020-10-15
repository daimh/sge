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
import java.rmi.*;
import java.rmi.server.UnicastRemoteObject;
import java.io.IOException;
import net.jini.core.event.RemoteEventListener;
import net.jini.core.event.RemoteEvent;
import net.jini.core.entry.Entry;
import net.jini.core.lease.Lease;
import net.jini.core.lookup.ServiceMatches;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceItem;
import net.jini.lookup.entry.Name;
import net.jini.lookup.entry.NameBean;

import com.sun.grid.jam.job.JobInterface;
import com.sun.grid.jam.ui.*;
import com.sun.grid.jam.ui.entry.*;

/**
 * Listens for a Job service.
 *
 * @version 1.4, 12/04/00
 *
 * @author Nello Nellari
 */
public class JAMJobHandler
  extends UnicastRemoteObject
  implements RemoteEventListener 
{

  private ServiceTemplate template;

  public JAMJobHandler()
    throws RemoteException
  {
    super();
  }

  public JAMJobHandler(ServiceTemplate template)
    throws RemoteException
  {
    super();
    this.template = template;
  }
    
  public void notify(RemoteEvent e)
  {
    //System.out.println(" -- JAMJobHandler Event received" +
    //                   e.getSource());
    try {
      ServiceMatches sm = ((ServiceRegistrar)
                           e.getSource()).lookup(template, 10);
      for(int h = 0; h < sm.items.length; h ++) {
        if(sm.items[h].service != null) {
          Entry attr [] = sm.items[h].attributeSets;
          // Looking for ServiceUIFactoryEntry object
          for(int i = 0 ; i < attr.length; i++) {
            if(attr[i] instanceof ServiceUIFactoryEntry) {
              ServiceUIFactory factory = ((ServiceUIFactoryEntry)
                                          attr[i]).factory;
              JFrame mon = (JFrame)factory.getServiceUI(sm.items[h].service);
              mon.pack();
              mon.setVisible(true);
            }
          }
        }
      }
    } catch(Exception re) {
      System.out.println(" -- Handler creating ui");
      re.printStackTrace();
    }
  }
}
