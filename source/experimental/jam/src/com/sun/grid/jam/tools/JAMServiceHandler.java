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
import java.util.Properties;
import java.net.MalformedURLException;
import java.io.IOException;
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.lookup.ServiceMatches;
import net.jini.core.discovery.LookupLocator;
import com.sun.grid.jam.*;
import com.sun.grid.jam.browser.ServiceBrowser;
import com.sun.grid.jam.app.*;

/**
 * This class makes sure that the text field cannot receive focus after
 * the initial lookup group has been entered.
 */
class ControlFocusJTextField
  extends JTextField
{
  private boolean allowFocusTraversal;

  public ControlFocusJTextField()
  {
    allowFocusTraversal = true;
  }

  public void removeFocus()
  {
    allowFocusTraversal = false;
    transferFocus();
  }

  /**
   * Don't allow focus traversal once removeFocus has been called
   */
  public boolean isFocusTraversable()
  {
    return allowFocusTraversal;
  }
}

/**
 * This implements the initial client UI for JAM (for
 * specifying lookup group and selecting an application).
 *
 * @version 1.15, 09/22/00
 *
 * @author Eric Sharakan
 */ 
public class JAMServiceHandler
  extends JFrame
  implements MouseListener, ActionListener
{
    String[] groups;
  ControlFocusJTextField lkLocation;
  JTextArea console;
  JList list;
  JScrollPane scroll;
  ServiceTemplate template;
  ServiceBrowser browser;
  LookupLocator[] locators;

  /**
   * Constructor does all work of setting up initial cleint UI.
   * Window is in three parts, from top to bottom:<pre>
   * Lookup (JPanel north): Jini LookupLocator URL is entered here;
   * Browser (JScrollPane scroll): Selectable list of available applications
   *				   is displayed here;
   * Console (JScrollPane scPan): Status messages are displayed here</pre>
   */
  public JAMServiceHandler(String title, ServiceTemplate templ, String locator)
  {
    super(title);
    if (locator == null) {
      locators = null;
    } else {
      locators = new LookupLocator[1];
      try {
	locators[0] = new LookupLocator(locator);
      } catch (MalformedURLException mue) {
	mue.printStackTrace();
      }
    }

    template = templ;

    getContentPane().setLayout(new BorderLayout());
    // Create Lookup panel
    JPanel north = new JPanel(new BorderLayout());
    north.setBorder(BorderFactory.createTitledBorder("Lookup"));
    north.add(new JLabel("Lookup Group: "), BorderLayout.WEST);
    lkLocation = new ControlFocusJTextField();
    // Action listener gets ServiceBrowser, which does actual Jini lookups
    lkLocation.addActionListener(this);
    north.add(lkLocation, BorderLayout.CENTER);
    // Create Browser scroll pane
    scroll = new JScrollPane();
    scroll.setBorder(BorderFactory.createTitledBorder("Browser"));
    scroll.setPreferredSize(new Dimension(300, 150));
    // Create Console's text area
    console = new JTextArea(title + " is ready: \n");
    // Place Lookup and Browser panels
    getContentPane().add(north, BorderLayout.NORTH);
    getContentPane().add(scroll, BorderLayout.CENTER);
    // Create Console scroll pane, passing it above text area
    JScrollPane scPan = new JScrollPane(console);
    scPan.setBorder(BorderFactory.createTitledBorder("Console")); 
    scPan.setPreferredSize(new Dimension(400, 60));
    // Place Console panel
    getContentPane().add(scPan, BorderLayout.SOUTH);
    // Handle window closing event by exiting
    addWindowListener(new WindowAdapter() {
      
      public void windowClosing(WindowEvent w) {
        if(browser != null)
          browser.removeListener();
        w.getWindow().setVisible(false);
        w.getWindow().dispose();
        System.exit(0);
      }
    });
  }

  /**
   *  Instantiate ServiceBrowser, which does lookups and displays results.
   */
  public void actionPerformed(ActionEvent e)
  {
    groups = new String[1];
    groups[0] = new String(lkLocation.getText());
    browser = new ServiceBrowser(groups, template, locators);
    browser.getUI();

    list = browser.getJList();
    list.addMouseListener(this);
    scroll.getViewport().setView(list);
    scroll.validate();
    repaint();

    // The next three lines prevent the user from changing the
    // group name or causing any more action events to be fired.
    lkLocation.removeActionListener(this);
    lkLocation.removeFocus();
    lkLocation.setEditable(false);

    browser.startDiscovery();
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
   * Launch application handler UI.
   *
   * @see JAMApplicationHandler
   */
  public void mouseClicked(MouseEvent e)
  {
    if(e.getClickCount() > 1) {
      int index = list.getSelectedIndex();

      try {
	JAMApplicationHandler fr = new JAMApplicationHandler
	  ("JAM Queue Browser", groups, locators, browser.getServiceItem(index));
	fr.pack();
	fr.setVisible(true);
      } catch(ClassNotFoundException cnf) {
	System.err.println(cnf);
      }
    }
  }

  /**
   * Entry point for starting JAM UI.  Optional argument specifies
   * a LookupLocator for locating a specific Jini LUS.
   */
  public static void main(String args[])
    throws ClassNotFoundException
  {
    JAMServiceHandler brw;

    System.setSecurityManager(new SecurityManager());
    Class [] cls =
    { Class.forName("com.sun.grid.jam.app.ApplicationInterface") };
    ServiceTemplate tmpl = new ServiceTemplate(null, cls, null);
    if (args.length >= 1)
      brw = new JAMServiceHandler("JAM Application Browser", tmpl, args[0]);
    else
      brw = new JAMServiceHandler("JAM Application Browser", tmpl, null);
    brw.pack();
    brw.setVisible(true);
  }
}
