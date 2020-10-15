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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.rmi.*;
import java.net.MalformedURLException;
import java.io.IOException;
import net.jini.core.entry.Entry;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lookup.ServiceItem;
import net.jini.core.lookup.ServiceMatches;
import net.jini.core.discovery.LookupLocator;
import net.jini.lookup.*;
import net.jini.lookup.entry.*;
import net.jini.discovery.*;
import net.jini.lease.*;

import com.sun.grid.jam.queue.QueueInterface;
import com.sun.grid.jam.queue.ApplicationSubmissionException;
import com.sun.grid.jam.browser.ServiceBrowser;
import com.sun.grid.jam.job.JobInterface;
import com.sun.grid.jam.app.*;

/**
 * This implements the application launch UI for JAM.  It is used for
 * setting application- and resource-level parameters, viewing
 * and selecting an appropriate queue service, and submitting the job.
 *
 * @version 1.28, 09/22/00
 *
 * @author Eric Sharakan
 * @author Nello Nellari
 */ 
public class JAMApplicationHandler
  extends JFrame
  implements MouseListener, ActionListener
{
  private JTextArea console;
  private JList list;
  private JScrollPane scroll;
  private ServiceBrowser browser;
  private JButton exec;
  private JButton clear;
  private ApplicationInterface application;
  private JPanel center;
  private JPanel appPanel;
  private Component appUI;
  private ServiceItem appItem;
  private LookupCache serviceCache;
  private String[] groups;
  private LookupLocator[] locators;
    
  /**
   * Constructor does all the work of setting up the UI.
   * Layout of JFrame is as follows:<pre>
   *   JPanel appPanel (BorderLayout)
   *     App-specific Component goes here
   *   JPanel center (GridLayout(1,0,5,5)
   *     JScrollPane scroll
   *	   JApplet.JList (ServiceBrowser list of queue services)
   *	   JScrollPane scPan(JTextArea console) - console messages
   *   JPanel south (FlowLayout)
   *     JPanel southInt (GirdLayout(1,0,10,10)
   *	     JButton exec("Submit")
   *       JButton clear("Clear")</pre>
   */
  public JAMApplicationHandler(String title, String[] groups,
			       LookupLocator[] locators, ServiceItem appSvc)
       throws ClassNotFoundException
  {
    super(title);
    this.groups = groups;
    this.locators = locators;

    getContentPane().setLayout(new BorderLayout());
    center = new JPanel(new GridLayout(1, 0, 5, 5));
    scroll = new JScrollPane();
    scroll.setBorder(BorderFactory.createTitledBorder("Queue Browser"));
    scroll.setPreferredSize(new Dimension(200, 150));
    JPanel south = new JPanel(new FlowLayout());
    JPanel southInt = new JPanel(new GridLayout(1, 0, 10, 10));
    exec = new JButton("Submit");
    clear = new JButton("Clear");
    southInt.add(exec);
    southInt.add(clear);
    south.add(southInt);
    south.setBorder(BorderFactory.createTitledBorder("Actions"));
    exec.addActionListener(this);
    clear.addActionListener(this);
    appPanel = new JPanel(new BorderLayout());
    if (appSvc.attributeSets[0] instanceof Name) {
      // Should probably use a specially-named attribute,
      // or we could just use ServiceInfo.name for this (panel title). XXX
      NameBean nb = new NameBean();
      nb.makeLink(appSvc.attributeSets[0]);
      appPanel.setBorder(BorderFactory.createTitledBorder(nb.getName()));
    } else {
      appPanel.setBorder(BorderFactory.createTitledBorder("Application Service"));
    }
    getContentPane().add(appPanel, BorderLayout.NORTH);
    getContentPane().add(center, BorderLayout.CENTER);
    getContentPane().add(south, BorderLayout.SOUTH);

    console = new JTextArea(title);
    JScrollPane scPan = new JScrollPane(console);
    scPan.setBorder(BorderFactory.createTitledBorder("Console"));
    //scPan.setPreferredSize(new Dimension(400, 60));
    center.add(scroll);
    center.add(scPan);
    //getContentPane().add(scPan, BorderLayout.SOUTH);

    addWindowListener(new WindowAdapter() {

      public void windowClosing(WindowEvent w) {
        if (browser != null)
          browser.removeListener();
	if (application != null)
	  application.removeAgentListener();
        w.getWindow().setVisible(false);
        w.getWindow().dispose();
      }

      public void windowClosed(WindowEvent w) {
      }
    });

    appItem = appSvc;

    // Get service object from serviceItem
    application = (ApplicationInterface)appItem.service;

    // First, get the App UI and add it to the panel
    appUI = application.getUI(appItem.attributeSets);
    appPanel.add(appUI, BorderLayout.NORTH);

    // Now, tell the agent (via the proxy) which lookup
    // groups and locators it should be interested in.
    application.setLookupParams(groups, locators);

    // Register for event notification from AppAgent.
    application.addAgentListener(new AgentEventListenerInterface() {
      // Update with any dynamic service changes
      public void agentNotify(AgentEvent ae) {
	serviceCache = ae.getLookupCache();
	browser.updateServiceList(ae.getServiceMatches());
      }
    });

    // Initialize browser window whose content is controlled by
    // the AppAgent.
    setBrowserService();

    // Start agent running
    application.runAgent();

    console.append(" is ready\n");
  } // End of constructor

  /**
   * Submit the job.
   */
  public void actionPerformed(ActionEvent e)
  {
    if (e.getActionCommand().equals("Submit")) {
      int index = list.getSelectedIndex();
      if(index > -1) {
        ServiceItem queueItem = browser.getServiceItem(index, serviceCache);
        Entry queueAttrs[] = queueItem.attributeSets;
        
        // Find Name of service (there's got to be an easier way XXX)
        for (int i=0; i<queueAttrs.length; ++i) {
          if (queueAttrs[i] instanceof Name) {
            console.append("Submitting job to " +
                           ((Name)queueAttrs[i]).name + "\n");
          }
        }
        
        QueueInterface queue = (QueueInterface)queueItem.service;

        // This is it - submit the application to the selected queue
        try {
	  application.submit(queue);
        } catch(ClassNotFoundException cnfe) {
          cnfe.printStackTrace();
        } catch (ApplicationSubmissionException ase) {
          ase.printStackTrace();
        } catch (RemoteException re) {
          re.printStackTrace();
        }  catch (IOException ioe) {
          ioe.printStackTrace();
        }
      } else {
        console.append("Queue service not selected \n");
      }
    } else if (e.getActionCommand().equals("Clear")) {
      console.setText("");
    }
  }

  /**
   * Initialize browser window (whose content is controlled by
   * the AppAgent).
   */
  private void setBrowserService()
  {
    browser = new ServiceBrowser((ServiceMatches)null);
    browser.getUI();
    list = browser.getJList();
    list.addMouseListener(this);
    scroll.getViewport().setView(list);
    scroll.validate();
    repaint();
  }

  /**
   * Does nothing; needed to implement MouseListener.
   */
  public void mouseEntered(MouseEvent e)
  {
  }

  /**
   * Does nothing; needed to implement MouseListener.
   */
  public void mouseExited(MouseEvent e)
  {
  }

  /**
   * Does nothing; needed to implement MouseListener.
   */
  public void mousePressed(MouseEvent e)
  {
  }
  
  /**
   * Does nothing; needed to implement MouseListener.
   */
  public void mouseReleased(MouseEvent e)
  {
  }

  /**
   * Launch queue monitor.
   */
  public void mouseClicked(MouseEvent e)
  {
    if(e.getClickCount() > 1) {
      int index = list.getSelectedIndex();
      //Show Compute Monitor
      ServiceItem queueItem = browser.getServiceItem(index, serviceCache);
      Entry queueAttrs[] = queueItem.attributeSets;
      QueueInterface queue = (QueueInterface)queueItem.service;

      // Find Name of service (there's got to be an easier way XXX)
      for (int i=0; i<queueAttrs.length; ++i) {
	if (queueAttrs[i] instanceof Name) {
	  console.append("Start Monitoring Queue " + ((Name)queueAttrs[i]).name + "\n");
	}
      }

      JAMComputeMonitor mon = new JAMComputeMonitor("JAM Monitor", queue);
      mon.pack();
      mon.setVisible(true);
    }
  }
}
