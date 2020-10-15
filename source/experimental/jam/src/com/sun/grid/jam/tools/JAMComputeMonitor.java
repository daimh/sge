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
import net.jini.core.lookup.ServiceTemplate;
import net.jini.core.lookup.ServiceID;
import net.jini.core.lookup.ServiceRegistrar;
import net.jini.core.discovery.LookupLocator;

import com.sun.grid.jam.queue.QueueInterface;

public class JAMComputeMonitor
  extends JFrame
  implements MouseListener, ActionListener
{

  JTextField host;
  JTextField cpu;
  JTextField mem;
  JTextField jobs;
  JTextField free;
  JTextField load;
  JButton ok;
  QueueInterface compute;
  Thread update;
    
  public JAMComputeMonitor(String title, QueueInterface compute)
  {
    super(title);
    this.compute = compute;
    host = new JTextField();
    cpu = new JTextField();
    mem = new JTextField();
    jobs = new JTextField();
    free = new JTextField();
    load = new JTextField();
    setTextFields();
    getContentPane().setLayout(new BorderLayout());
    JPanel north = new JPanel(new GridLayout(1,0));
    JPanel stat = new JPanel(new BorderLayout());
    JPanel labSt = new JPanel(new GridLayout(3, 0));
    labSt.add(new JLabel("Host: "));
    labSt.add(new JLabel("CPU: "));
    labSt.add(new JLabel("Tot mem: "));
    JPanel txtSt = new JPanel(new GridLayout(3, 0));
    txtSt.add(host);
    txtSt.add(cpu);
    txtSt.add(mem);
    stat.add(labSt, BorderLayout.WEST);
    stat.add(txtSt, BorderLayout.CENTER);
    JPanel dynam = new JPanel(new BorderLayout());
    JPanel labDn = new JPanel(new GridLayout(3, 0));
    labDn.add(new JLabel("Jobs: "));
    labDn.add(new JLabel("Free mem: "));
    labDn.add(new JLabel("Load: "));
    JPanel txtDn = new JPanel(new GridLayout(3, 0));
    txtDn.add(jobs);
    txtDn.add(free);
    txtDn.add(load);
    dynam.add(labDn, BorderLayout.WEST);
    dynam.add(txtDn, BorderLayout.CENTER);
    north.add(stat);
    north.add(dynam);
    north.setBorder(BorderFactory.createTitledBorder("Service Parameters"));
    JPanel south = new JPanel(new FlowLayout());
    south.setBorder(BorderFactory.createTitledBorder("Actions"));
    ok = new JButton("Ok");
    ok.addActionListener(this);
    south.add(ok);
    getContentPane().add(north, BorderLayout.NORTH);
    getContentPane().add(south, BorderLayout.SOUTH);
    update = new Thread() {

      public void run(){
        boolean r = true;
        while(r) {
          try {
            Thread.currentThread().sleep(1000);
            setTextFields();
          } catch(InterruptedException ie) {
            r = false;
            setVisible(false);
          }
        }
      }
    };
    update.start();
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent w) {
        if(update != null)
          update = null;
        dispose();
      }
      public void windowClosed(WindowEvent w) {
      }
    });
  }

  /**
   *
   *
   */
  public void actionPerformed(ActionEvent e)
  {
    setVisible(false);
    if(update != null)
      update = null;
    dispose();
  }

  private void setTextFields()
  {
    try {
      host.setText("Grid Engine Host");
      cpu.setText("N/A (fixme)");
      mem.setText("N/A (fixme)");
      jobs.setText(Integer.toString(compute.getJobsCount()) + "(canned)");
      free.setText("N/A (fixme)");
      load.setText("N/A (fixme)");
    } catch(RemoteException e) {
      setVisible(false);
    }
  }

  public void mouseEntered(MouseEvent e)
  {
  }

  /**
   *
   *
   */
  public void mouseExited(MouseEvent e)
  {
  }

  /**
   *
   *
   */
  public void mousePressed(MouseEvent e)
  {
  }
  
  /**
   *
   *
   */
  public void mouseReleased(MouseEvent e)
  {
  }

  /**
   * Launch application
   *
   */
  public void mouseClicked(MouseEvent e)
  {
  }
}
