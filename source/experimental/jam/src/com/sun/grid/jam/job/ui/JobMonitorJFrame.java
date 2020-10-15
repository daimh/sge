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
package com.sun.grid.jam.job.ui;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.grid.jam.job.JobProxy;

/**
 * Defines a frame for controlling and 
 * monitoring a job
 * 
 * @version 1.3, 09/22/00
 *
 * @author Nello Nellari
 *
 */
public class JobMonitorJFrame
  extends JFrame
  implements ActionListener
{

  private JobMonitorJPanel jobMonitor;

  public JobMonitorJFrame(JobProxy jobService)
  {
    // Possibly add a menu for exit
    super("Job Monitor");
    jobMonitor = new JobMonitorJPanel(jobService);
    getContentPane().add(jobMonitor);
    jobMonitor.registerClosingListener(this);
    JMenuBar bar = new JMenuBar();
    JMenu file = new JMenu("File");
    JMenuItem refresh = new JMenuItem("Refresh");
    JMenuItem close = new JMenuItem("Destroy");
    //JMenuItem destroy = new JMenuItem("Destroy");
    refresh.setActionCommand("8");
    refresh.addActionListener(jobMonitor);
    close.setActionCommand("0");
    close.addActionListener(this);
    file.add(refresh);
    file.add(close);
    bar.add(file);
    setJMenuBar(bar);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent w) {
        jobMonitor.terminate();
        w.getWindow().setVisible(false);
        w.getWindow().dispose();
      }
    });
  }

  public void actionPerformed(ActionEvent e)
  {
    switch(e.getActionCommand().charAt(0)) {
    case '0':
      jobMonitor.terminate();
      setVisible(false);
      dispose();
      break;
    default:
      break;
    }
  }
}

